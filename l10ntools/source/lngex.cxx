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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_l10ntools.hxx"
#include <stdio.h>

// local includes
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
#define STATE_UTF8      0x000A
#define STATE_ULF       0x000B
#define STATE_LANGUAGES 0x000C

// set of global variables
ByteString sInputFile;
BOOL bEnableExport;
BOOL bMergeMode;
BOOL bErrorLog;
BOOL bUTF8;
BOOL bULF; // ULF = Unicode Language File
ByteString sPrj;
ByteString sPrjRoot;
ByteString sOutputFile;
ByteString sMergeSrc;

/*****************************************************************************/
BOOL ParseCommandLine( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = FALSE;
    bMergeMode = FALSE;
    bErrorLog = TRUE;
    bUTF8 = TRUE;
    bULF = FALSE;
    sPrj = "";
    sPrjRoot = "";
    Export::sLanguages = "";

    USHORT nState = STATE_NON;
    BOOL bInput = FALSE;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        ByteString sSwitch( argv[ i ] );
        sSwitch.ToUpperAscii();
        if ( sSwitch == "-I" ) {
            nState = STATE_INPUT; // next tokens specifies source files
        }
        else if ( sSwitch  == "-O" ) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if ( sSwitch == "-P" ) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }
        else if ( sSwitch == "-R" ) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if ( sSwitch == "-M" ) {
            nState = STATE_MERGESRC; // next token specifies the merge database
        }
        else if ( sSwitch == "-E" ) {
            nState = STATE_ERRORLOG;
            bErrorLog = FALSE;
        }
        else if ( sSwitch == "-UTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = TRUE;
        }
        else if ( sSwitch == "-L" ) {
            nState = STATE_LANGUAGES;
        }
        else {
            switch ( nState ) {
                case STATE_NON: {
                    return FALSE;   // no valid command line
                }
                //break;
                case STATE_INPUT: {
                    sInputFile = argv[ i ];
                    bInput = TRUE; // source file found
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
                    bMergeMode = TRUE; // activate merge mode, cause merge database found
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
        bULF = TRUE;
        bEnableExport = TRUE;
        return TRUE;
    }

    // command line is not valid
    return FALSE;
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
    fprintf( stdout, "     A fallback language can be defined like this: l1=f1.\n" );
    fprintf( stdout, "     f1, f2,... are also elements of (de,en-US...)\n" );
    fprintf( stdout, "     Example: -L de,es=en-US\n" );
    fprintf( stdout, "              Restriction to de and es, en-US will be fallback for es\n" );
}

/*****************************************************************************/
#if defined(UNX) || defined(OS2)
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    if ( !ParseCommandLine( argc, argv )) {
        Help();
        return 1;
    }
        fprintf(stdout, ".");
        fflush( stdout );

    if ( sOutputFile.Len()) {
        LngParser aParser( sInputFile, bUTF8, bULF );
        if ( bMergeMode )
            aParser.Merge( sMergeSrc, sOutputFile , sPrj );
        else
            aParser.CreateSDF( sOutputFile, sPrj, sPrjRoot );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
