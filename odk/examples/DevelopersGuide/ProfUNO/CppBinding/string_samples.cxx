#include <stdio.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>

using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OString;

int main( int argc, char * argv [] )
{
    // string concatination

    sal_Int32 n = 42;
    double pi = 3.14159;

    // give it an initial size, should be a good guess.
    // stringbuffer extends if necessary
    OUStringBuffer buf( 128 );

    // append an ascii string
    buf.appendAscii( "pi ( here " );

    // numbers can be simply appended
    buf.append( pi );

    // lets the compiler count the stringlength, so this is more efficient than
    // the above appendAscii call, where length of the string must be calculated at
    // runtime
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" ) multiplied with " ) );
    buf.append( n );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" gives ") );
    buf.append( (double)( n * pi ) );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "." ) );

    // now transfer the buffer into the string.
    // afterwards buffer is empty and may be reused again !
    OUString string = buf.makeStringAndClear();

    // I could of course also used the OStringBuffer directly
    OString oString = rtl::OUStringToOString( string , RTL_TEXTENCODING_ASCII_US );

    // just to print something
    printf( "%s\n" ,oString.getStr() );

    return 0;
}

