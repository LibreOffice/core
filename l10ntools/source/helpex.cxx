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
#include <stdlib.h>

// local includes
#include "helpmerge.hxx"

// defines to parse command line
#define STATE_NON               0x0001
#define STATE_INPUT             0x0002
#define STATE_OUTPUT            0x0003
#define STATE_PRJ               0x0004
#define STATE_ROOT              0x0005
#define STATE_SDFFILE           0x0006
#define STATE_ERRORLOG          0x0007
#define STATE_BREAKHELP         0x0008
#define STATE_UNMERGE           0x0009
#define STATE_UTF8              0x000A
#define STATE_LANGUAGES         0x000B
#define STATE_FORCE_LANGUAGES   0x000C
#define STATE_OUTPUTX           0xfe
#define STATE_OUTPUTY           0xff

// set of global variables
ByteString sInputFile;
BOOL bEnableExport;
BOOL bMergeMode;
BOOL bErrorLog;
BOOL bUTF8;
ByteString sPrj;
ByteString sPrjRoot;
ByteString sOutputFile;
ByteString sOutputFileX;
ByteString sOutputFileY;
ByteString sSDFFile;

/*****************************************************************************/
BOOL ParseCommandLine( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = FALSE;
    bMergeMode = FALSE;
    bErrorLog = TRUE;
    bUTF8 = TRUE;
    sPrj = "";
    sPrjRoot = "";
    Export::sLanguages = "";
    Export::sForcedLanguages = "";

    USHORT nState = STATE_NON;
    BOOL bInput = FALSE;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        if ( ByteString( argv[ i ]).ToUpperAscii() == "-I" ) {
            nState = STATE_INPUT; // next tokens specifies source files
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii()  == "-O" ) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii()  == "-X" ) {
            nState = STATE_OUTPUTX; // next token specifies the dest file
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii()  == "-Y" ) {
            nState = STATE_OUTPUTY; // next token specifies the dest file
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-P" ) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }
         else if ( ByteString( argv[ i ]).ToUpperAscii() == "-LF" ) {
            nState = STATE_FORCE_LANGUAGES;
        }

        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-R" ) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-M" ) {
            nState = STATE_SDFFILE; // next token specifies the merge database
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-E" ) {
            nState = STATE_ERRORLOG;
            bErrorLog = FALSE;
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-UTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = TRUE;
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-NOUTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = FALSE;
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-L" ) {
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
                case STATE_OUTPUTX: {
                    sOutputFileX = argv[ i ]; // the dest. file
                }
                break;
                case STATE_OUTPUTY: {
                    sOutputFileY = argv[ i ]; // the dest. file
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
                case STATE_SDFFILE: {
                    sSDFFile = argv[ i ];
                    bMergeMode = TRUE; // activate merge mode, cause merge database found
                }
                break;
                case STATE_LANGUAGES: {
                    Export::sLanguages = argv[ i ];
                }
                case STATE_FORCE_LANGUAGES:{
                    Export::sForcedLanguages = argv[ i ];
                }
                break;
            }
        }
    }

    if ( bInput ) {
        // command line is valid
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
    fprintf( stdout, "Syntax: HELPEX[-p Prj][-r PrjRoot]-i FileIn ( -o FileOut | -x path -y relfile )[-m DataBase][-e][-b][-u][-L l1,l2,...] -LF l1,l2 \n" );
    fprintf( stdout, " Prj:      Project\n" );
    fprintf( stdout, " PrjRoot:  Path to project root (..\\.. etc.)\n" );
    fprintf( stdout, " FileIn:   Source file (*.lng)\n" );
    fprintf( stdout, " FileOut:  Destination file (*.*)\n" );
    fprintf( stdout, " DataBase: Mergedata (*.sdf)\n" );
    fprintf( stdout, " -L: Restrict the handled languages. l1,l2,... are elements of (en-US,fr,de...)\n" );
    fprintf( stdout, "     A fallback language can be defined like this: l1=f1.\n" );
    fprintf( stdout, "     f1, f2,... are also elements of (en-US,fr,de...)\n" );
    fprintf( stdout, "     Example: -L fr=en-US\n" );
    fprintf( stdout, "              Restriction to fr, en-US will be fallback for fr\n" );
    fprintf( stdout, " -LF: Force the creation of that languages\n" );

}

/*****************************************************************************/
#ifndef TESTDRIVER

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
    //sal_uInt32 startfull = Export::startMessure();

    bool hasInputList = sInputFile.GetBuffer()[0]=='@';
//    printf("x = %s , y = %s , o = %s\n", sOutputFileX.GetBuffer(),  sOutputFileY.GetBuffer() , sOutputFile.GetBuffer() );
    bool hasNoError = true;

    if ( sOutputFile.Len() ){                                               // Merge single file ?
        //printf("DBG: Inputfile = %s\n",sInputFile.GetBuffer());
        HelpParser aParser( sInputFile, bUTF8 , false );

        if ( bMergeMode )
        {

            //sal_uInt64 startreadloc = Export::startMessure();
            MergeDataFile aMergeDataFile( sSDFFile, sInputFile , FALSE, RTL_TEXTENCODING_MS_1252 );
            //MergeDataFile aMergeDataFile( sSDFFile, sInputFile , FALSE, RTL_TEXTENCODING_MS_1252, false );
            //Export::stopMessure( ByteString("read localize.sdf") , startreadloc );

            hasNoError = aParser.Merge( sSDFFile, sOutputFile , Export::sLanguages , aMergeDataFile );
        }
        else
            hasNoError = aParser.CreateSDF( sOutputFile, sPrj, sPrjRoot, sInputFile, new XMLFile( '0' ), "help" );
    }else if ( sOutputFileX.Len() && sOutputFileY.Len() && hasInputList ) {  // Merge multiple files ?
        if ( bMergeMode ){

            ifstream aFStream( sInputFile.Copy( 1 , sInputFile.Len() ).GetBuffer() , ios::in );

            if( !aFStream ){
                cerr << "ERROR: - helpex - Can't open the file " << sInputFile.Copy( 1 , sInputFile.Len() ).GetBuffer() << "\n";
                exit(-1);
            }

            vector<ByteString> filelist;
            rtl::OStringBuffer filename;
            sal_Char aChar;
            while( aFStream.get( aChar ) )
            {
                if( aChar == ' ' || aChar == '\n')
                    filelist.push_back( ByteString( filename.makeStringAndClear().getStr() ) );
                else
                    filename.append( aChar );
            }
            if( filename.getLength() > 0 )
                filelist.push_back( ByteString ( filename.makeStringAndClear().getStr() ) );

            aFStream.close();
            ByteString sHelpFile(""); // dummy
            //MergeDataFile aMergeDataFile( sSDFFile, sHelpFile , FALSE, RTL_TEXTENCODING_MS_1252, false );
            MergeDataFile aMergeDataFile( sSDFFile, sHelpFile , FALSE, RTL_TEXTENCODING_MS_1252 );

            //aMergeDataFile.Dump();
            std::vector<ByteString> aLanguages;
            HelpParser::parse_languages( aLanguages , aMergeDataFile );

            bool bCreateDir = true;
            for( vector<ByteString>::iterator pos = filelist.begin() ; pos != filelist.end() ; ++pos )
            {
                sHelpFile = *pos;
                cout << ".";cout.flush();

                HelpParser aParser( sHelpFile , bUTF8 , true );
                hasNoError = aParser.Merge( sSDFFile , sOutputFileX , sOutputFileY , true , aLanguages , aMergeDataFile , bCreateDir );
                bCreateDir = false;
            }
        }
    } else
        cerr << "helpex ERROR: Wrong input parameters!\n";

    //Export::stopMessure( ByteString("full cycle") , startfull );
    if( hasNoError )
        return 0;
    else
        return 1;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
