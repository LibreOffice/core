/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
