#include <stdio.h>
#include <stdlib.h>

#include "versionhelper.hxx"

#include <rtl/ustring.hxx>

#include <iostream>

/**
 * display usage screen
 */

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    static char* optionSet[] = {
        "-h:s,       display help or help on option",
        "-help:s,    see -h",
        NULL
    };

    GetOpt opt( argv, optionSet );

    // someone indicates that he needs help
    if ( opt.hasOpt( "-h" ) || opt.hasOpt( "-help" ) )
    {
        opt.showUsage();
        exit(0);
    }

    rtl::OUString suLibraryName = rtl::OStringToOUString(opt.getFirstParam(), RTL_TEXTENCODING_ASCII_US );
    VersionHelper aHelper(suLibraryName, opt);

    std::cout << aHelper << std::endl;
    return 0;
}
