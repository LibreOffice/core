/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "cmdline.hxx"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>


char C_sUseText[] = "Use: xml2cmp.exe \n"
                    "        [-dep <xml-component-descriptions-root-directory> \n"
                    "        [-func funcFile] \n"
                    "        [-html htmlFile] \n"
                    "        [-types typeFile] \n"
                    "        [-idlpath idlPath] \n"
                    "        Xml_FileName\n";


char C_sCmdFunc[]       = "-func";
char C_sCmdDep[]       = "-dep";
char C_sCmdHtml[]       = "-html";
char C_sCmdType[]       = "-types";
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

    if (bDisplayUse)
    {
        std::cout << C_sUseText << std::endl;
        bIsOk = false;
        exit(0);
    }

    ParseSingleFileCommand(argc,argv);

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
              int           argc,              char *        argv[] )
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
        else if ( strcmp( argv[nCountArg], C_sCmdDep ) == 0 )
        {
            bIsOk = GetParameter(
                                sDepPath,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
