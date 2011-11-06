/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
sal_Bool bEnableExport;
sal_Bool bMergeMode;
sal_Bool bErrorLog;
sal_Bool bUTF8;
sal_Bool bULF; // ULF = Unicode Language File
ByteString sPrj;
ByteString sPrjRoot;
ByteString sOutputFile;
ByteString sMergeSrc;

/*****************************************************************************/
sal_Bool ParseCommandLine( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = sal_False;
    bMergeMode = sal_False;
    bErrorLog = sal_True;
    bUTF8 = sal_True;
    bULF = sal_False;
    sPrj = "";
    sPrjRoot = "";
    Export::sLanguages = "";

    sal_uInt16 nState = STATE_NON;
    sal_Bool bInput = sal_False;

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
            bErrorLog = sal_False;
        }
        else if ( sSwitch == "-UTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = sal_True;
        }
/*      else if ( sSwitch == "-NOUTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = sal_False;
        }*/
/*      else if ( sSwitch == "-ULF" ) {
            nState = STATE_ULF;
            bULF = sal_True;
        }*/
        else if ( sSwitch == "-L" ) {
            nState = STATE_LANGUAGES;
        }
        else {
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
//                  sPrj.ToLowerAscii(); // the project
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
