/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cmdline.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:55:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "cmdline.hxx"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>


char C_sUseText[] = "Use: xml2cmp.exe \n"
                    "        [-func funcFile] \n"
                    "        [-html htmlFile] \n"
                    "        [-types typeFile] \n"
                    "        [-idlpath idlPath] \n"
                    "        Xml_FileName\n"
                    " or: xml2cmp.exe \n"
                    "        -ix \n"
                    "        sourceDirectory \n"
                    "        outputDirectory \n"
                    "        [-idlpath idlPath] \n"
                    "        tagname [tagname ...]";


char C_sCmdFunc[]       = "-func";
char C_sCmdHtml[]       = "-html";
char C_sCmdType[]       = "-types";
char C_sCmdIndex[]      = "-ix";
char C_sCmdIdlPath[]    = "-idlpath";



bool GetParameter( Simstr & o_rMemory, int & io_nCountArg, int argc, char * argv[] );


CommandLine::CommandLine( int           argc,
                          char *        argv[] )
    :   bIsOk(true)
{
    bool bDisplayUse = false;

    /* Check command line: */
    if ( argc < 2 )
        bDisplayUse = true;
    else if ( argc == 2 && ! isalnum(argv[1][0]) )
        bDisplayUse = true;
    else if ( strcmp( argv[1], C_sCmdIndex ) == 0 && argc < 5 )
        bDisplayUse = true;

    if (bDisplayUse)
    {
        std::cout << C_sUseText << std::endl;
        bIsOk = false;
        exit(0);
    }

    if ( strcmp( argv[1], C_sCmdIndex ) == 0 )
    {
        ParseIndexCommand(argc,argv);
    }
    else
    {
        ParseSingleFileCommand(argc,argv);
    }

    if ( sXmlSourceFile.l() == 0
         && sXmlSourceDirectory.l() == 0 )
    {
        bIsOk = false;
    }
    else if ( sXmlSourceFile.l() > 0
              && sXmlSourceDirectory.l() > 0 )
    {
        bIsOk = false;
    }
    else if ( sIndexFile.l() > 0
              && ( sXmlSourceDirectory.l() == 0
                   || aTagsInIndex.size() == 0
                 )
            )
    {
        bIsOk = false;
    }

}

CommandLine::~CommandLine()
{
}


const char *
CommandLine::ErrorText() const
{
    static char cOut[] = "Error:  Command line was incorrect. Probably there was a flag without\n"
                         "        the corresponding parameter. Or there was no XML-sourcefile specified.\n";
    return cOut;
}

bool
GetParameter( Simstr &      o_pMemory,
              int &         io_pCountArg,
              int           argc,
              char *        argv[] )
{
    io_pCountArg++;
    if( io_pCountArg < argc )
    {
        o_pMemory = argv[io_pCountArg];
        return true;
    }
    return false;
}


void
CommandLine::ParseIndexCommand( int                 argc,
                                char *              argv[] )
{
    int nCountArg = 1;
    bIsOk = GetParameter(
                sXmlSourceDirectory,
                nCountArg,
                argc,
                argv  );
    if (bIsOk)
        bIsOk = GetParameter(
                    sOutputDirectory,
                    nCountArg,
                    argc,
                    argv  );
    if (bIsOk && strcmp( argv[nCountArg+1], C_sCmdIdlPath ) == 0 )
        bIsOk = GetParameter(
                    sIdlRootPath,
                    ++nCountArg,
                    argc,
                    argv  );

    sIndexFile = sOutputDirectory;
#if defined(WNT) || defined(OS2)
    sIndexFile+= "\\xmlindex.html";
#elif defined(UNX)
    sIndexFile+= "/xmlindex.html";
#endif

    for ( ++nCountArg; nCountArg < argc; ++nCountArg )
    {
        Simstr sElementName(argv[nCountArg]);
        aTagsInIndex.push_back( sElementName );
    }
}


void
CommandLine::ParseSingleFileCommand( int                argc,
                                     char *             argv[] )
{
    for ( int nCountArg = 1; nCountArg < argc && bIsOk; ++nCountArg )
    {
        if ( strcmp( argv[nCountArg], C_sCmdFunc ) == 0 )
        {
            bIsOk = GetParameter(
                                sFuncFile,
                                nCountArg,
                                argc,
                                argv  );
        }
        else if ( strcmp( argv[nCountArg], C_sCmdHtml ) == 0 )
        {
            bIsOk = GetParameter(
                                sHtmlFile,
                                nCountArg,
                                argc,
                                argv  );
        }
        else if ( strcmp( argv[nCountArg], C_sCmdType ) == 0 )
        {
            bIsOk = GetParameter(
                                sTypeInfoFile,
                                nCountArg,
                                argc,
                                argv  );
        }
        else if ( strcmp( argv[nCountArg], C_sCmdIdlPath ) == 0 )
        {
            bIsOk = GetParameter(
                                sIdlRootPath,
                                nCountArg,
                                argc,
                                argv  );
        }
        else
        {
            sXmlSourceFile = argv[nCountArg];
        }
    }   /* end for */
}
