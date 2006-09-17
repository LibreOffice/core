/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lngex.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 08:18:41 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_transex3.hxx"
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
bool bQuiet;
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
    bQuiet = false;
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
        else if ( sSwitch == "-QQ" ) {
            bQuiet = true;
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
/*      else if ( sSwitch == "-NOUTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = FALSE;
        }*/
/*      else if ( sSwitch == "-ULF" ) {
            nState = STATE_ULF;
            bULF = TRUE;
        }*/
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
//                  sPrj.ToLowerAscii(); // the project
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
    //fprintf( stdout, "Syntax:ULFEX[-p Prj][-r PrjRoot]-i FileIn -o FileOut[-m DataBase][-e][-b][-u][-NOUTF8][-ULF][-L l1,l2,...]\n" );
    fprintf( stdout, "Syntax:ULFEX[-p Prj][-r PrjRoot]-i FileIn -o FileOut[-m DataBase][-L l1,l2,...]\n" );
    fprintf( stdout, " Prj:      Project\n" );
    fprintf( stdout, " PrjRoot:  Path to project root (..\\.. etc.)\n" );
    fprintf( stdout, " FileIn:   Source file (*.lng)\n" );
    fprintf( stdout, " FileOut:  Destination file (*.*)\n" );
    fprintf( stdout, " DataBase: Mergedata (*.sdf)\n" );
    fprintf( stdout, " -QQ: quite output\n" );
    //fprintf( stdout, " -NOUTF8: disable UTF8 as language independent encoding\n" );
    //fprintf( stdout, " -ULF: enables Unicode Language File format, leads to UTF8 encoded version of lng files" );
    fprintf( stdout, " -L: Restrict the handled languages. l1,l2,... are elements of (de,en-US...)\n" );
    fprintf( stdout, "     A fallback language can be defined like this: l1=f1.\n" );
    fprintf( stdout, "     f1, f2,... are also elements of (de,en-US...)\n" );
    fprintf( stdout, "     Example: -L de,es=en-US\n" );
    fprintf( stdout, "              Restriction to de and es, en-US will be fallback for es\n" );
}

/*****************************************************************************/
#if defined( UNX ) || defined( MAC )
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
    if( !bQuiet ){
        fprintf( stdout, "\nUlfEx 1 Copyright 2000 Sun Microsystems, Inc. All Rights Reserved.\n" );
        fprintf( stdout, "====================================================================\n" );
        fprintf( stdout, "\nProcessing File %s ...\n", sInputFile.GetBuffer());
    }else
    {
        fprintf(stdout, ".");
        fflush( stdout );
    }

    if ( sOutputFile.Len()) {
        LngParser aParser( sInputFile, bUTF8, bULF , bQuiet );
        if ( bMergeMode )
            aParser.Merge( sMergeSrc, sOutputFile , sPrj );
        else
            aParser.CreateSDF( sOutputFile, sPrj, sPrjRoot );
    }

    if( !bQuiet ) fprintf( stdout, "\n=================================================\n\n" );

    return 0;
}
