/*************************************************************************
 *
 *  $RCSfile: cfgmerge.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nf $ $Date: 2000-11-22 10:53:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>
#include <tools/string.hxx>
#include <tools/fsys.hxx>

// local includes
#include "export.hxx"
#include "cfgmerge.hxx"
#include "tokens.h"
#include "utf8conv.hxx"

extern "C" { yyerror( char * ); }
extern "C" { YYWarning( char * ); }

// defines to parse command line
#define STATE_NON       0x0001
#define STATE_INPUT     0x0002
#define STATE_OUTPUT    0x0003
#define STATE_PRJ       0x0004
#define STATE_ROOT      0x0005
#define STATE_MERGESRC  0x0006
#define STATE_ERRORLOG  0x0007

// set of global variables
BOOL bEnableExport;
BOOL bMergeMode;
BOOL bErrorLog;
ByteString sPrj;
ByteString sPrjRoot;
ByteString sInputFileName;
ByteString sActFileName;
ByteString sOutputFile;
ByteString sMergeSrc;

CfgParser *pParser;

extern "C" {
// the whole interface to lexer is in this extern "C" section

/*****************************************************************************/
extern char *GetOutputFile( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = FALSE;
    bMergeMode = FALSE;
    bErrorLog = TRUE;
    sPrj = "";
    sPrjRoot = "";
    sInputFileName = "";
    sActFileName = "";

    USHORT nState = STATE_NON;
    BOOL bInput = FALSE;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        if ( ByteString( argv[ i ] ).ToUpperAscii() == "-I" ) {
            nState = STATE_INPUT; // next token specifies source file
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-O" ) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-P" ) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-R" ) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-M" ) {
            nState = STATE_MERGESRC; // next token specifies the merge database
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-E" ) {
            nState = STATE_ERRORLOG;
            bErrorLog = FALSE;
        }
        else {
            switch ( nState ) {
                case STATE_NON: {
                    return NULL;    // no valid command line
                }
                break;
                case STATE_INPUT: {
                    sInputFileName = argv[ i ];
                    bInput = TRUE; // source file found
                }
                break;
                case STATE_OUTPUT: {
                    sOutputFile = argv[ i ]; // the dest. file
                }
                break;
                case STATE_PRJ: {
                    sPrj = ByteString( argv[ i ]);
                    sPrj.ToLowerAscii(); // the project
                }
                break;
                case STATE_ROOT: {
                    sPrjRoot = ByteString( argv[ i ]); // path to project root
                }
                break;
                case STATE_MERGESRC: {
                    sMergeSrc = ByteString( argv[ i ]);
                    bMergeMode = TRUE; // activate merge mode, cause merge database found
                }
            }
        }
    }

    if ( bInput ) {
        // command line is valid
        bEnableExport = TRUE;
        char *pReturn = new char[ sOutputFile.Len() + 1 ];
        strcpy( pReturn, sOutputFile.GetBuffer());
        return pReturn;
    }

    // command line is not valid
    return NULL;
}

/*****************************************************************************/
int InitCfgExport( char *pOutput )
/*****************************************************************************/
{
    // instanciate Export
    ByteString sOutput( pOutput );

    pParser = new CfgParser();


    return 1;
}

/*****************************************************************************/
int EndCfgExport()
/*****************************************************************************/
{
    delete pParser;

    return 1;
}

/*****************************************************************************/
extern FILE *GetCfgFile()
/*****************************************************************************/
{
    // look for valid filename
    if ( sInputFileName.Len()) {

        // able to open file?
        FILE *pFile = fopen( sInputFileName.GetBuffer(), "r" );
        if ( !pFile )
            fprintf( stderr, "Error: Could not open file %s\n",
                sInputFileName.GetBuffer());
        else {
            // this is a valid file which can be opened, so
            // create path to project root
            DirEntry aEntry( String( sInputFileName, RTL_TEXTENCODING_ASCII_US ));
            aEntry.ToAbs();
            ByteString sFullEntry( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
            aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
            aEntry += DirEntry( sPrjRoot );
            ByteString sPrjEntry( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );

            // create file name, beginnig with project root
            // (e.g.: source\ui\src\menue.src)
            sActFileName = sFullEntry.Copy( sPrjEntry.Len() + 1 );
            sActFileName.ToLowerAscii();

            fprintf( stdout, "\nProcessing File %s ...\n", sInputFileName.GetBuffer());
            return pFile;
        }
    }
    // this means the file could not be opened
    return NULL;
}

/*****************************************************************************/
int WorkOnTokenSet( int nTyp, char *pTokenText )
/*****************************************************************************/
{
    pParser->Execute( nTyp, pTokenText );

    return 1;
}


/*****************************************************************************/
int SetError()
/*****************************************************************************/
{
    return 1;
}

/*****************************************************************************/
int GetError()
/*****************************************************************************/
{
    return 0;
}
}

//
// class CfgStack
//

/*****************************************************************************/
CfgStack::~CfgStack()
/*****************************************************************************/
{
    for ( ULONG i = 0; i < Count(); i++ )
        delete GetObject( i );
}

/*****************************************************************************/
ByteString CfgStack::GetAccessPath( ULONG nPos )
/*****************************************************************************/
{
    if ( nPos == LIST_APPEND )
        nPos = Count() - 1;

    ByteString sReturn;
    for ( ULONG i = 0; i <= nPos; i++ ) {
        if ( i )
            sReturn += ".";
        sReturn += GetStackData( i )->GetIdentifier();
    }

    return sReturn;
}

/*****************************************************************************/
CfgStackData *CfgStack::GetStackData( ULONG nPos )
/*****************************************************************************/
{
    if ( nPos == LIST_APPEND )
        nPos = Count() - 1;

    return GetObject( nPos );
}

//
// class CfgParser
//

/*****************************************************************************/
CfgParser::CfgParser()
/*****************************************************************************/
                : pStackData( NULL )
{
}

/*****************************************************************************/
CfgParser::~CfgParser()
/*****************************************************************************/
{
}


/*****************************************************************************/
BOOL CfgParser::IsTokenClosed( const ByteString &rToken )
/*****************************************************************************/
{
    return rToken.GetChar( rToken.Len() - 2 ) == '/';
}

/*****************************************************************************/
void CfgParser::WorkOnText(
    const ByteString &rText,
    const ByteString &rIsoLang
)
/*****************************************************************************/
{
//  fprintf( stdout, "%s %s %s %s\n", rCurrentGid.GetBuffer(), rCurrentLid.GetBuffer(), rIsoLang.GetBuffer(), rText.GetBuffer());

    USHORT nLang = GetLang( rIsoLang );
    if ( nLang ) {
         pStackData->sText[ Export::GetLangIndex( nLang )] = rText;
    }
}

/*****************************************************************************/
void CfgParser::WorkOnRessourceEnd( const ByteString &rResTyp )
/*****************************************************************************/
{
    if ( pStackData->sText[ ENGLISH_US_INDEX ].Len()) {
        pStackData->sResTyp = rResTyp;

        fprintf( stdout, "%s\n", aStack.GetAccessPath().GetBuffer());
        fprintf( stdout, "%s\n", pStackData->sResTyp.GetBuffer());
        for ( ULONG i = 0; i < LANGUAGES; i++ ) {
            if ( pStackData->sText[ i ].Len())
                fprintf( stdout, "\t%s%s%s\n", pStackData->sTextTag.GetBuffer(), pStackData->sText[ i ].GetBuffer(), pStackData->sEndTextTag.GetBuffer());
        }
    }
}

/*****************************************************************************/
int CfgParser::ExecuteAnalyzedToken( int nToken, char *pToken )
/*****************************************************************************/
{
    ByteString sToken( pToken );

    ByteString sTokenName;
    ByteString sTokenId;

    switch ( nToken ) {
        case CFG_TOKEN_PACKAGE:
        case CFG_TOKEN_COMPONENT:
        case CFG_TOKEN_TEMPLATE:
        case CFG_TOKEN_CONFIGNAME:
        case CFG_TAG:
        case ANYTOKEN:
        case CFG_TEXT_START:
        {
              if ( !IsTokenClosed( sToken )) {
                sTokenName = sToken.GetToken( 1, '<' ).GetToken( 0, '>' ).GetToken( 0, ' ' );

                ByteString sSearch;
                switch ( nToken ) {
                    case CFG_TOKEN_PACKAGE:
                        sSearch = "package-id=";
                    break;
                    case CFG_TOKEN_COMPONENT:
                        sSearch = "component-id=";
                    break;
                    case CFG_TOKEN_TEMPLATE:
                        sSearch = "template-id=";
                    break;
                    case CFG_TOKEN_CONFIGNAME:
                        sSearch = "cfg:name=";
                    break;
                    case CFG_TEXT_START: {
                        sCurrentResTyp = sTokenName;

                        ByteString sTemp = sToken.Copy( sToken.Search( "xml:lang=" ));
                        sCurrentIsoLang = sTemp.GetToken( 1, '\"' ).GetToken( 0, '\"' );

                        pStackData->sTextTag = sToken;

                        sCurrentText = "";
                    }
                    break;
                }
                if ( sSearch.Len()) {
                    ByteString sTemp = sToken.Copy( sToken.Search( sSearch ));
                    sTokenId = sTemp.GetToken( 1, '\"' ).GetToken( 0, '\"' );
                }
                pStackData = aStack.Push( sTokenName, sTokenId );
            }
        }
        break;
        case CFG_CLOSETAG:
            sTokenName = sToken.GetToken( 1, '/' ).GetToken( 0, '>' ).GetToken( 0, ' ' );
               if ( aStack.GetStackData() && ( aStack.GetStackData()->GetTagType() == sTokenName )) {
                WorkOnRessourceEnd( sCurrentResTyp );
                aStack.Pop();
                pStackData = aStack.GetStackData();
            }
            else {
                ByteString sError( "Missplaced close tag: " );
                sError += sToken;
                Error( sError );
            }
        break;

        case CFG_TEXTCHAR:
            sCurrentText += sToken;
        break;
    }

    if ( sCurrentText.Len() && nToken != CFG_TEXTCHAR ) {
        WorkOnText( sCurrentText, sCurrentIsoLang );
        sCurrentText = "";
        pStackData->sEndTextTag = sToken;
    }

    return 1;
}

/*****************************************************************************/
int CfgParser::Execute( int nToken, char * pToken )
/*****************************************************************************/
{
    ByteString sToken( pToken );

    switch ( nToken ) {
        case CFG_TAG:
            if ( sToken.Search( "package-id=" ) != STRING_NOTFOUND )
                return ExecuteAnalyzedToken( CFG_TOKEN_PACKAGE, pToken );
            else if ( sToken.Search( "component-id=" ) != STRING_NOTFOUND )
                return ExecuteAnalyzedToken( CFG_TOKEN_COMPONENT, pToken );
            else if ( sToken.Search( "template-id=" ) != STRING_NOTFOUND )
                return ExecuteAnalyzedToken( CFG_TOKEN_TEMPLATE, pToken );
            else if ( sToken.Search( "cfg:name=" ) != STRING_NOTFOUND )
                return ExecuteAnalyzedToken( CFG_TOKEN_CONFIGNAME, pToken );
        break;
    }
    return ExecuteAnalyzedToken( nToken, pToken );
}

/*****************************************************************************/
USHORT CfgParser::GetLang( const ByteString &rIsoLang )
/*****************************************************************************/
{
    ByteString sLang( rIsoLang );

    sLang.ToUpperAscii();

    if ( sLang == ByteString( COMMENT_ISO ).ToUpperAscii())
        return COMMENT;
    else if ( sLang == ByteString( ENGLISH_US_ISO ).ToUpperAscii())
        return ENGLISH_US;
    else if ( sLang == ByteString( PORTUGUESE_ISO ).ToUpperAscii())
        return PORTUGUESE;
    else if ( sLang == ByteString( RUSSIAN_ISO ).ToUpperAscii())
        return RUSSIAN;
    else if ( sLang == ByteString( GREEK_ISO ).ToUpperAscii())
        return GREEK;
    else if ( sLang == ByteString( DUTCH_ISO ).ToUpperAscii())
        return DUTCH;
    else if ( sLang == ByteString( FRENCH_ISO ).ToUpperAscii())
        return FRENCH;
    else if ( sLang == ByteString( SPANISH_ISO ).ToUpperAscii())
        return SPANISH;
    else if ( sLang == ByteString( FINNISH_ISO ).ToUpperAscii())
        return FINNISH;
    else if ( sLang == ByteString( HUNGARIAN_ISO ).ToUpperAscii())
        return HUNGARIAN;
    else if ( sLang == ByteString( ITALIAN_ISO ).ToUpperAscii())
        return ITALIAN;
    else if ( sLang == ByteString( CZECH_ISO ).ToUpperAscii())
        return CZECH;
    else if ( sLang == ByteString( SLOVAK_ISO ).ToUpperAscii())
        return SLOVAK;
    else if ( sLang == ByteString( ENGLISH_ISO ).ToUpperAscii())
        return ENGLISH;
    else if ( sLang == ByteString( DANISH_ISO ).ToUpperAscii())
        return DANISH;
    else if ( sLang == ByteString( SWEDISH_ISO ).ToUpperAscii())
        return SWEDISH;
    else if ( sLang == ByteString( NORWEGIAN_ISO ).ToUpperAscii())
        return NORWEGIAN;
    else if ( sLang == ByteString( POLISH_ISO ).ToUpperAscii())
        return POLISH;
    else if ( sLang == ByteString( GERMAN_ISO ).ToUpperAscii())
        return GERMAN;
    else if ( sLang == ByteString( PORTUGUESE_BRAZILIAN_ISO ).ToUpperAscii())
        return PORTUGUESE_BRAZILIAN;
    else if ( sLang == ByteString( JAPANESE_ISO ).ToUpperAscii())
        return JAPANESE;
    else if ( sLang == ByteString( KOREAN_ISO ).ToUpperAscii())
        return KOREAN;
    else if ( sLang == ByteString( CHINESE_SIMPLIFIED_ISO ).ToUpperAscii())
        return CHINESE_SIMPLIFIED;
    else if ( sLang == ByteString( CHINESE_TRADITIONAL_ISO ).ToUpperAscii())
        return CHINESE_TRADITIONAL;
    else if ( sLang == ByteString( TURKISH_ISO ).ToUpperAscii())
        return TURKISH;
    else if ( sLang == ByteString( ARABIC_ISO ).ToUpperAscii())
        return ARABIC;
    else if ( sLang == ByteString( HEBREW_ISO ).ToUpperAscii())
        return HEBREW;

    ByteString sError( "Unknown language code: " );
    sError += rIsoLang;
    Error( sError );
    return 0;
}

/*****************************************************************************/
void CfgParser::Error( const ByteString &rError )
/*****************************************************************************/
{
    yyerror(( char * ) rError.GetBuffer());
}
