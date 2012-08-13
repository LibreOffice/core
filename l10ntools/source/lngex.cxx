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

#include "sal/config.h"

#include <stdio.h>

#include "sal/main.h"

#include "lngmerge.hxx"

// defines to parse command line
#define STATE_NON       0x0001
#define STATE_INPUT     0x0002
#define STATE_OUTPUT    0x0003
#define STATE_PRJ       0x0004
#define STATE_ROOT      0x0005
#define STATE_MERGESRC  0x0006
#define STATE_ERRORLOG  0x0007
#define STATE_BREAKHELP 0x0008
#define STATE_UNMERGE   0x0009
#define STATE_ULF       0x000A
#define STATE_LANGUAGES 0x000B

// set of global variables
rtl::OString sInputFile;
sal_Bool bEnableExport;
sal_Bool bMergeMode;
sal_Bool bUTF8;
sal_Bool bULF; // ULF = Unicode Language File
rtl::OString sPrj;
rtl::OString sPrjRoot;
rtl::OString sOutputFile;
rtl::OString sMergeSrc;

/*****************************************************************************/
sal_Bool ParseCommandLine( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = sal_False;
    bMergeMode = sal_False;
    bUTF8 = sal_True;
    bULF = sal_False;
    sPrj = "";
    sPrjRoot = "";
    Export::sLanguages = "";

    sal_uInt16 nState = STATE_NON;
    sal_Bool bInput = sal_False;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        rtl::OString sSwitch = rtl::OString(argv[i]).toAsciiUpperCase();
        if (sSwitch.equalsL(RTL_CONSTASCII_STRINGPARAM("-I")))
            nState = STATE_INPUT; // next tokens specifies source files
        else if (sSwitch.equalsL(RTL_CONSTASCII_STRINGPARAM("-O")))
            nState = STATE_OUTPUT; // next token specifies the dest file
        else if (sSwitch.equalsL(RTL_CONSTASCII_STRINGPARAM("-P")))
            nState = STATE_PRJ; // next token specifies the cur. project
        else if (sSwitch.equalsL(RTL_CONSTASCII_STRINGPARAM("-R")))
            nState = STATE_ROOT; // next token specifies path to project root
        else if (sSwitch.equalsL(RTL_CONSTASCII_STRINGPARAM("-M")))
            nState = STATE_MERGESRC; // next token specifies the merge database
        else if (sSwitch.equalsL(RTL_CONSTASCII_STRINGPARAM("-E")))
        {
            nState = STATE_ERRORLOG;
        }
        else if (sSwitch.equalsL(RTL_CONSTASCII_STRINGPARAM("-L")))
            nState = STATE_LANGUAGES;
        else
        {
            switch ( nState ) {
                case STATE_NON: {
                    return sal_False;   // no valid command line
                }
                //break;
                case STATE_INPUT: {
                    sInputFile = argv[ i ];
                    bInput = sal_True; // source file found
                }
                break;
                case STATE_OUTPUT: {
                    sOutputFile = argv[ i ]; // the dest. file
                }
                break;
                case STATE_PRJ: {
                    sPrj = argv[ i ];
                }
                break;
                case STATE_ROOT: {
                    sPrjRoot = argv[ i ]; // path to project root
                }
                break;
                case STATE_MERGESRC: {
                    sMergeSrc = argv[ i ];
                    bMergeMode = sal_True; // activate merge mode, cause merge database found
                }
                break;
                case STATE_LANGUAGES: {
                    Export::sLanguages = argv[ i ];
                }
                break;
            }
        }
    }

    if ( bInput ) {
        // command line is valid
        bULF = sal_True;
        bEnableExport = sal_True;
        return sal_True;
    }

    // command line is not valid
    return sal_False;
}


/*****************************************************************************/
void Help()
/*****************************************************************************/
{
    fprintf( stdout, "Syntax:ULFEX[-p Prj][-r PrjRoot]-i FileIn -o FileOut[-m DataBase][-L l1,l2,...]\n" );
    fprintf( stdout, " Prj:      Project\n" );
    fprintf( stdout, " PrjRoot:  Path to project root (..\\.. etc.)\n" );
    fprintf( stdout, " FileIn:   Source file (*.lng)\n" );
    fprintf( stdout, " FileOut:  Destination file (*.*)\n" );
    fprintf( stdout, " DataBase: Mergedata (*.sdf)\n" );
    fprintf( stdout, " -L: Restrict the handled languages. l1,l2,... are elements of (de,en-US...)\n" );
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    if ( !ParseCommandLine( argc, argv ))
    {
        Help();
        return 1;
    }

    if (!sOutputFile.isEmpty())
    {
        LngParser aParser( sInputFile, bULF );
        if ( bMergeMode )
            aParser.Merge(sMergeSrc, sOutputFile);
        else
            aParser.CreateSDF( sOutputFile, sPrj, sPrjRoot );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
