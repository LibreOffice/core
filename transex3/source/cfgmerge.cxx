/*************************************************************************
 *
 *  $RCSfile: cfgmerge.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nf $ $Date: 2000-11-22 12:57:00 $
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

/*  if ( bMergeMode )
        pParser = new CfgMerge( sMergeSrc, sOutputFile, bErrorLog );
      else  */
    if ( sOutputFile.Len()) {
        pParser = new CfgExport( sOutputFile, sPrj, sActFileName );
    }
    else
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
    USHORT nLang = Export::GetLangByIsoLang( rIsoLang );
    if ( nLang ) {
         pStackData->sText[ Export::GetLangIndex( nLang )] = rText;
    }
    else {
        ByteString sError( "Unknown language code: " );
        sError += rIsoLang;
        Error( sError );
    }
}

/*****************************************************************************/
void CfgParser::WorkOnRessourceEnd( const ByteString &rResTyp )
/*****************************************************************************/
{
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
void CfgParser::Error( const ByteString &rError )
/*****************************************************************************/
{
    yyerror(( char * ) rError.GetBuffer());
}


//
// class CfgOutputParser
//

/*****************************************************************************/
CfgOutputParser::CfgOutputParser( const ByteString &rOutputFile )
/*****************************************************************************/
{
    pOutputStream =
        new SvFileStream(
            String( rOutputFile, RTL_TEXTENCODING_ASCII_US ),
            STREAM_STD_WRITE | STREAM_TRUNC
        );

    if ( !pOutputStream->IsOpen()) {
        ByteString sError( "Unable to open output file: " );
        sError += rOutputFile;
        Error( sError );
        delete pOutputStream;
        pOutputStream = NULL;
    }
}

/*****************************************************************************/
CfgOutputParser::~CfgOutputParser()
/*****************************************************************************/
{
    if ( pOutputStream ) {
        pOutputStream->Close();
        delete pOutputStream;
    }
}

//
// class CfgExport
//

/*****************************************************************************/
CfgExport::CfgExport(
        const ByteString &rOutputFile,
        const ByteString &rProject,
        const ByteString &rFilePath
)
/*****************************************************************************/
                : CfgOutputParser( rOutputFile ),
                sPrj( rProject ),
                sPath( rFilePath )
{
}

/*****************************************************************************/
CfgExport::~CfgExport()
/*****************************************************************************/
{
}

/*****************************************************************************/
void CfgExport::WorkOnRessourceEnd( const ByteString &rResTyp )
/*****************************************************************************/
{
    if ( pOutputStream ) {
        if ( pStackData->sText[ GERMAN_INDEX ].Len() &&
            ( pStackData->sText[ ENGLISH_US_INDEX ].Len() ||
                pStackData->sText[ ENGLISH_INDEX ].Len())
            )
        {
            ByteString sFallback = pStackData->sText[ GERMAN_INDEX ];
            if ( pStackData->sText[ ENGLISH_US_INDEX ].Len())
                sFallback = pStackData->sText[ ENGLISH_US_INDEX ];
            else if ( pStackData->sText[ ENGLISH_INDEX ].Len())
                sFallback = pStackData->sText[ ENGLISH_INDEX ];

            ByteString sLocalId = pStackData->sIdentifier;
            ByteString sGroupId;
            if ( aStack.Count() == 1 ) {
                sGroupId = sLocalId;
                sLocalId = "";
            }
            else {
                sGroupId = aStack.GetAccessPath( aStack.Count() - 2 );
            }

            Time aTime;
            ByteString sTimeStamp( ByteString::CreateFromInt64( Date().GetDate()));
            sTimeStamp += " ";
            sTimeStamp += ByteString::CreateFromInt32( aTime.GetHour());
            sTimeStamp += ":";
            sTimeStamp += ByteString::CreateFromInt32( aTime.GetMin());
            sTimeStamp += ":";
            sTimeStamp += ByteString::CreateFromInt32( aTime.GetSec());

            for ( ULONG i = 0; i < LANGUAGES; i++ ) {
                if ( LANGUAGE_ALLOWED( i )) {
                    ByteString sText = pStackData->sText[ i ];
                    if ( !sText.Len())
                        sText = sFallback;

                    Export::UnquotHTML( sText );
                    USHORT nLangId = Export::LangId[ i ];
                    sText = UTF8Converter::ConvertFromUTF8(
                        sText, Export::GetCharSet( nLangId ));

                    ByteString sOutput( sPrj ); sOutput += "\t";
                    sOutput += sPath;
                    sOutput += "\t0\t";
                    sOutput += rResTyp; sOutput += "\t";
                    sOutput += sGroupId; sOutput += "\t";
                    sOutput += sLocalId; sOutput += "\t\t\t0\t";
                    sOutput += ByteString::CreateFromInt64( nLangId );
                    sOutput += "\t";
                    sOutput += sText; sOutput += "\t\t\t\t";
                    sOutput += sTimeStamp;

                    pOutputStream->WriteLine( sOutput );
                }
            }
        }
    }
}
