
#include <stdio.h>
#include "share.h"


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static void some_more(  t_throws_exc p )
{
    (*p)();
}

extern "C" void SAL_CALL start( t_throws_exc p )
{
    try
    {
        some_more( p );
    }
    catch (lang::IllegalArgumentException & exc)
    {
        OString msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        printf( "starter.cxx: caught IllegalArgumentException: %s\n", msg.getStr() );
    }
    catch (Exception & exc)
    {
        OString msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        printf( "starter.cxx: caught some UNO exc: %s\n", msg.getStr() );
    }
    catch (...)
    {
        printf( "starter.cxx: caught something\n" );
    }
}
