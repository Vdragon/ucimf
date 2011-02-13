#include "uchewing.h"
#include <imf/widget.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <string>
#include <iconv.h>
using namespace std;


Imf* UcimfChewingHandler::_instance = 0;

Imf* UcimfChewingHandler::getInstance()
{
  if( _instance == 0 )
    _instance = new UcimfChewingHandler;

  return _instance;
}

UcimfChewingHandler::UcimfChewingHandler()
{
	/* Initialize libchewing */
	/* for the sake of testing, we should not change existing hash data */
	char *prefix = const_cast<char*>(CHEWING_DATA_PREFIX);
	chewing_Init( prefix, TEST_HASH_DIR );

	/* Request handle to ChewingContext */
	ctx = chewing_new();

	/* Set keyboard type */
	chewing_set_KBType( ctx, chewing_KBStr2Num( const_cast<char*>("KB_DEFAULT") ) );

	/* Fill configuration values */
	chewing_set_candPerPage( ctx, 9 );
	chewing_set_maxChiSymbolLen( ctx, 16 );
	chewing_set_addPhraseDirection( ctx, 1 );
	//chewing_set_selKey( ctx, selKey_define, 10 );
	chewing_set_spaceAsSelection( ctx, 1 );
	chewing_set_autoShiftCur( ctx, 1 );
	chewing_set_phraseChoiceRearward( ctx, 1 );

}

UcimfChewingHandler::~UcimfChewingHandler()
{
	/* Free Chewing IM handle */
	chewing_delete( ctx );

	/* Termate Chewing services */
	chewing_Terminate();

}

string UcimfChewingHandler::commit_buf="";


void UcimfChewingHandler::refresh()
{
	Status *stts = Status::getInstance();
	stts->set_imf_status( const_cast<char*>("Chewing"), const_cast<char*>("新酷音"), const_cast<char*>(""));
}

string UcimfChewingHandler::process_input( const string& buf )
{
	string result("");

	int buf_len = buf.size();                                                                     

	if( buf_len == 1 ) {
		switch( buf[0] ){
			case 9:   chewing_handle_Tab( ctx ); break;
			case 13:  chewing_handle_Enter( ctx ); break;
			case 27:  chewing_handle_Esc( ctx ); break;
			case 32:  chewing_handle_Space( ctx ); break;
			case 126: chewing_handle_Del( ctx ); break;
			case 127: chewing_handle_Backspace( ctx ); break;
			default: {
					 chewing_handle_Default( ctx, buf[0] ); 
					 break;
				 }
		}
	}
	else if( buf_len == 3 && buf[0]==27 && buf[1]==91 ) {
		switch( buf[2] )
		{
			case 65: chewing_handle_Up( ctx ); break;
			case 66: chewing_handle_Down( ctx ); break;
			case 67: chewing_handle_Right( ctx ); break;
			case 68: chewing_handle_Left( ctx ); break;
			default:
			break;
		}
	}
	else if( buf_len == 4 && buf[0]==27 && buf[1]==91 && buf[4]==126 ) {
		switch( buf[3] )
		{
			case 49: chewing_handle_Home( ctx ); break;
			case 52: chewing_handle_End( ctx ); break;
			case 53: break; // PAGE_UP
			case 54: break; // PAGE_DOWN
			default:
			break;
		}
	}
	else {
		; // do nothing
	}

	int *prdt_count;
	char *prdt_text = chewing_zuin_String( ctx, prdt_count);
	char *prdt_text2 = chewing_buffer_String( ctx );

	Preedit *prdt = Preedit::getInstance();
	prdt->clear();
	prdt->append( prdt_text2 );
	prdt->append( prdt_text );
	prdt->render();

	char *lkc_text = chewing_cand_String( ctx );
	LookupChoice *lkc = LookupChoice::getInstance();
	lkc->clear();
	lkc->append( lkc_text );
	lkc->render();

	if ( chewing_commit_Check( ctx ) ) {
		char *s;
		s = chewing_commit_String( ctx );
		commit_buf = s;
		free( s );
	}

	if( commit_buf.size() > 0 )
	{
		result = commit_buf;
		commit_buf.clear();
	}

	return result;
}

Imf* createImf() { 
	return UcimfChewingHandler::getInstance(); 
}

void destroyImf( Imf* imf ){ 
	delete imf; 
}