/*************************************************************************
 *
 *  $RCSfile: cfgmerge.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: nf $ $Date: 2003-07-14 08:39:00 $
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

extern "C" { int yyerror( char * ); }
extern "C" { int YYWarning( char * ); }

// defines to parse command line
#define STATE_NON       0x0001
#define STATE_INPUT     0x0002
#define STATE_OUTPUT    0x0003
#define STATE_PRJ       0x0004
#define STATE_ROOT      0x0005
#define STATE_MERGESRC  0x0006
#define STATE_ERRORLOG  0x0007
#define STATE_UTF8      0x0008
#define STATE_LANGUAGES 0X0009
#define STATE_ISOCODE99 0x000A
#define STATE_FORCE     0x000B

// set of global variables
BOOL bEnableExport;
BOOL bMergeMode;
BOOL bErrorLog;
BOOL bForce;
BOOL bUTF8;
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
    bForce = FALSE;
    bUTF8 = TRUE;
    sPrj = "";
    sPrjRoot = "";
    sInputFileName = "";
    sActFileName = "";
    Export::sLanguages = "";

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
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-UTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = TRUE;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-NOUTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = FALSE;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-F" ) {
            nState = STATE_FORCE;
            bForce = TRUE;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-L" ) {
            nState = STATE_LANGUAGES;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-ISO99" ) {
            nState = STATE_ISOCODE99;
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
//                  sPrj.ToLowerAscii(); // the project
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
                break;
                case STATE_LANGUAGES: {
                    Export::sLanguages = ByteString( argv[ i ]);
                }
                break;
                case STATE_ISOCODE99: {
                    Export::sIsoCode99 = ByteString( argv[ i ]);
                }
                break;
            }
        }
    }

    if ( bInput ) {
        // command line is valid
        bEnableExport = TRUE;
        char *pReturn = new char[ sOutputFile.Len() + 1 ];
        strcpy( pReturn, sOutputFile.GetBuffer());  // #100211# - checked
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

    if ( bMergeMode )
        pParser = new CfgMerge( sMergeSrc, sOutputFile, bErrorLog );
      else if ( sOutputFile.Len()) {
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
//          sActFileName.ToLowerAscii();

            fprintf( stdout, "\nProcessing File %s ...\n", sInputFileName.GetBuffer());

            sActFileName.SearchAndReplaceAll( "/", "\\" );

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
// class CfgStackData
//

/*****************************************************************************/
void CfgStackData::FillInFallbacks()
/*****************************************************************************/
{
    for ( USHORT i = 0; i < LANGUAGES; i++ ) {
        if (( i != GERMAN_INDEX ) && ( i != ENGLISH_INDEX )) {
            USHORT nFallbackIndex =
                Export::GetLangIndex(
                    Export::GetFallbackLanguage( Export::LangId[ i ] ));
            if (( nFallbackIndex < LANGUAGES ) && !sText[ i ].Len() && i != nFallbackIndex ) {
                CharSet eSource =
                    Export::GetCharSet( Export::LangId[ nFallbackIndex ] );
                CharSet eDest =
                    Export::GetCharSet( Export::LangId[ i ] );

                if ( eSource != eDest ) {
                    ByteString sFallback =
                        UTF8Converter::ConvertToUTF8( sText[ nFallbackIndex ],
                            eSource );
                    sText[ i ] =
                        UTF8Converter::ConvertFromUTF8( sFallback,
                            eDest );
                }
            }
        }
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
                : pStackData( NULL ),
                bLocalize( FALSE )
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
    ByteString &rText,
    USHORT nLangIndex,
    const ByteString &rResTyp
)
/*****************************************************************************/
{
}

/*****************************************************************************/
void CfgParser::AddText(
    ByteString &rText,
    const ByteString &rIsoLang,
    const ByteString &rResTyp
)
/*****************************************************************************/
{
    USHORT nLang = Export::GetLangByIsoLang( rIsoLang );
    if ( nLang != 0xFFFF ) {
        USHORT nTextLen = 0;
        while ( rText.Len() != nTextLen ) {
            nTextLen = rText.Len();
            rText.SearchAndReplaceAll( "\n", " " );
            rText.SearchAndReplaceAll( "\r", " " );
            rText.SearchAndReplaceAll( "\t", " " );
            rText.SearchAndReplaceAll( "  ", " " );
        }
        USHORT nLangIndex = Export::GetLangIndex( nLang );
        pStackData->sResTyp = rResTyp;
        WorkOnText( rText, nLangIndex, rResTyp );
         pStackData->sText[ nLangIndex ] = rText;
    }
    else if ( rIsoLang != NO_TRANSLATE_ISO ) {
        ByteString sError( "Unknown language code: " );
        sError += rIsoLang;
        Error( sError );
    }
}

/*****************************************************************************/
void CfgParser::WorkOnRessourceEnd()
/*****************************************************************************/
{
}

/*****************************************************************************/
int CfgParser::ExecuteAnalyzedToken( int nToken, char *pToken )
/*****************************************************************************/
{
    ByteString sToken( pToken );

    if ( sToken == " " || sToken == "\t" )
        sLastWhitespace += sToken;

    ByteString sTokenName;
    ByteString sTokenId;

    BOOL bOutput = TRUE;

    switch ( nToken ) {
        case CFG_TOKEN_PACKAGE:
        case CFG_TOKEN_COMPONENT:
        case CFG_TOKEN_TEMPLATE:
        case CFG_TOKEN_CONFIGNAME:
        case CFG_TOKEN_OORNAME:
        case CFG_TOKEN_OORVALUE:
        case CFG_TAG:
        case ANYTOKEN:
        case CFG_TEXT_START:
        {
            sTokenName = sToken.GetToken( 1, '<' ).GetToken( 0, '>' ).GetToken( 0, ' ' );

              if ( !IsTokenClosed( sToken )) {
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
                    case CFG_TOKEN_OORNAME:
                        sSearch = "oor:name=";
                        bLocalize = TRUE;
                    break;
                    case CFG_TOKEN_OORVALUE:
                        sSearch = "oor:value=";
                    break;
                    case CFG_TEXT_START: {
                        if ( sCurrentResTyp != sTokenName ) {
                            WorkOnRessourceEnd();
                            for ( ULONG i = 0; i < LANGUAGES; i++ )
                                if ( LANGUAGE_ALLOWED( i ))
                                    pStackData->sText[ i ] = "";
                        }
                        sCurrentResTyp = sTokenName;

                        ByteString sTemp = sToken.Copy( sToken.Search( "xml:lang=" ));
                        sCurrentIsoLang = sTemp.GetToken( 1, '\"' ).GetToken( 0, '\"' );

                        if ( sCurrentIsoLang == NO_TRANSLATE_ISO )
                            bLocalize = FALSE;

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
                if ( sSearch == "cfg:name=" ) {
                    ByteString sTemp( sToken );
                    sTemp.ToUpperAscii();
                    bLocalize = (( sTemp.Search( "CFG:TYPE=\"STRING\"" ) != STRING_NOTFOUND ) &&
                        ( sTemp.Search( "CFG:LOCALIZED=\"TRUE\"" ) != STRING_NOTFOUND ));
                }
            }
            else if ( sTokenName == "label" ) {
                if ( sCurrentResTyp != sTokenName ) {
                    WorkOnRessourceEnd();
                    for ( ULONG i = 0; i < LANGUAGES; i++ )
                        if ( LANGUAGE_ALLOWED( i ))
                            pStackData->sText[ i ] = "";
                }
                sCurrentResTyp = sTokenName;
            }
        }
        break;
        case CFG_CLOSETAG:
            sTokenName = sToken.GetToken( 1, '/' ).GetToken( 0, '>' ).GetToken( 0, ' ' );
               if ( aStack.GetStackData() && ( aStack.GetStackData()->GetTagType() == sTokenName )) {
                if ( ! sCurrentText.Len())
                    WorkOnRessourceEnd();
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
            bOutput = FALSE;
        break;

        case CFG_TOKEN_NO_TRANSLATE:
            bLocalize = FALSE;
        break;
    }

    if ( sCurrentText.Len() && nToken != CFG_TEXTCHAR ) {
        AddText( sCurrentText, sCurrentIsoLang, sCurrentResTyp );
        Output( sCurrentText );
        sCurrentText = "";
        pStackData->sEndTextTag = sToken;
    }

    if ( bOutput )
        Output( sToken );

    if ( sToken != " " && sToken != "\t" )
        sLastWhitespace = "";

    return 1;
}

/*****************************************************************************/
void CfgParser::Output( const ByteString& rOutput )
/*****************************************************************************/
{
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
                return ExecuteAnalyzedToken( CFG_TOKEN_OORNAME, pToken );
            else if ( sToken.Search( "oor:name=" ) != STRING_NOTFOUND )
                return ExecuteAnalyzedToken( CFG_TOKEN_OORNAME, pToken );
            else if ( sToken.Search( "oor:value=" ) != STRING_NOTFOUND )
                return ExecuteAnalyzedToken( CFG_TOKEN_OORVALUE, pToken );
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
void CfgExport::WorkOnRessourceEnd()
/*****************************************************************************/
{
    if ( pOutputStream && bLocalize ) {
        if (( pStackData->sText[ GERMAN_INDEX ].Len() &&
            ( pStackData->sText[ ENGLISH_US_INDEX ].Len() ||
                pStackData->sText[ ENGLISH_INDEX ].Len())) ||
            ( bForce &&
                ( pStackData->sText[ GERMAN_INDEX ].Len() ||
                    pStackData->sText[ ENGLISH_INDEX ].Len() ||
                    pStackData->sText[ ENGLISH_US_INDEX ].Len())
                )
            )
        {
            pStackData->FillInFallbacks();

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

            ByteString sTimeStamp( Export::GetTimeStamp());

            for ( ULONG i = 0; i < LANGUAGES; i++ ) {
                if ( LANGUAGE_ALLOWED( i )) {
                    ByteString sText = pStackData->sText[ i ];
                    if ( !sText.Len())
                        sText = sFallback;

                    ByteString sOutput( sPrj ); sOutput += "\t";
                    sOutput += sPath;
                    sOutput += "\t0\t";
                    sOutput += pStackData->sResTyp; sOutput += "\t";
                    sOutput += sGroupId; sOutput += "\t";
                    sOutput += sLocalId; sOutput += "\t\t\t0\t";
                    sOutput += ByteString::CreateFromInt64( Export::LangId[ i ]);
                    sOutput += "\t";
                    sOutput += sText; sOutput += "\t\t\t\t";
                    sOutput += sTimeStamp;

                    if ( bUTF8 )
                        sOutput = UTF8Converter::ConvertToUTF8( sOutput, Export::GetCharSet( Export::LangId[ i ] ));

                    pOutputStream->WriteLine( sOutput );
                }
            }
        }
    }
}

/*****************************************************************************/
void CfgExport::WorkOnText(
    ByteString &rText,
    USHORT nLangIndex,
    const ByteString &rResTyp
)
/*****************************************************************************/
{
    Export::UnquotHTML( rText );
    USHORT nLangId = Export::LangId[ nLangIndex ];
    rText = UTF8Converter::ConvertFromUTF8(
        rText, Export::GetCharSet( nLangId ));
}


//
// class CfgMerge
//

/*****************************************************************************/
CfgMerge::CfgMerge(
    const ByteString &rMergeSource, const ByteString &rOutputFile,
    BOOL bErrorLog )
/*****************************************************************************/
                : CfgOutputParser( rOutputFile ),
                pMergeDataFile( NULL ),
                pResData( NULL ),
                bGerman( FALSE ),
                bEnglish( FALSE )
{
    if ( rMergeSource.Len())
        pMergeDataFile = new MergeDataFile(
            rMergeSource, bErrorLog, RTL_TEXTENCODING_MS_1252, bUTF8 );
}

/*****************************************************************************/
CfgMerge::~CfgMerge()
/*****************************************************************************/
{
    delete pMergeDataFile;
    delete pResData;
}

/*****************************************************************************/
void CfgMerge::WorkOnText(
    ByteString &rText,
    USHORT nLangIndex,
    const ByteString &rResTyp
)
/*****************************************************************************/
{
    if ( pMergeDataFile && bLocalize ) {
        if ( !pResData ) {
            ByteString sLocalId = pStackData->sIdentifier;
            ByteString sGroupId;
            if ( aStack.Count() == 1 ) {
                sGroupId = sLocalId;
                sLocalId = "";
            }
            else {
                sGroupId = aStack.GetAccessPath( aStack.Count() - 2 );
            }

            ByteString sPlatform( "" );

            pResData = new ResData( sPlatform, sGroupId );
            pResData->sId = sLocalId;
            pResData->sResTyp = pStackData->sResTyp;
        }

        if ( nLangIndex == GERMAN_INDEX )
            bGerman = TRUE;
        if (( nLangIndex == ENGLISH_INDEX ) || ( nLangIndex == ENGLISH_US_INDEX ))
            bEnglish = TRUE;

        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
        if ( pEntrys ) {
            ByteString sContent;
            if (( nLangIndex != GERMAN_INDEX ) &&
                ( nLangIndex != ENGLISH_INDEX ) &&
                ( pEntrys->GetText(
                    sContent, STRING_TYP_TEXT, nLangIndex )) &&
                ( sContent != "-" ) && ( sContent.Len()))
            {
                rText = UTF8Converter::ConvertToUTF8(
                    sContent, Export::GetCharSet( Export::LangId[ nLangIndex ]));
                Export::QuotHTML( rText );
            }
        }
    }
}

/*****************************************************************************/
void CfgMerge::Output( const ByteString& rOutput )
/*****************************************************************************/
{
    if ( pOutputStream )
        pOutputStream->Write( rOutput.GetBuffer(), rOutput.Len());
}

/*****************************************************************************/
void CfgMerge::WorkOnRessourceEnd()
/*****************************************************************************/
{
    if ( pMergeDataFile && pResData && bLocalize && (( bGerman && bEnglish ) || bForce )) {
        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
        if ( pEntrys ) {
            for ( ULONG nIndex = 0; nIndex < LANGUAGES; nIndex++ ) {
                ByteString sContent;
                if (( nIndex != GERMAN_INDEX ) &&
                    ( nIndex != ENGLISH_INDEX ) &&
                    ( LANGUAGE_ALLOWED( nIndex )) &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, ( USHORT ) nIndex, TRUE )) &&
                    ( sContent != "-" ) && ( sContent.Len()))
                {
                    ByteString sText = UTF8Converter::ConvertToUTF8(
                        sContent, Export::GetCharSet( Export::LangId[ nIndex ]));

                    Export::QuotHTML( sText );

                    ByteString sAdditionalLine( "\t" );

                    ByteString sTextTag = pStackData->sTextTag;
                    ByteString sTemp = sTextTag.Copy( sTextTag.Search( "xml:lang=" ));

                    ByteString sSearch = sTemp.GetToken( 0, '\"' );
                    sSearch += "\"";
                    sSearch += sTemp.GetToken( 1, '\"' );
                    sSearch += "\"";

                    ByteString sReplace = sTemp.GetToken( 0, '\"' );
                    sReplace += "\"";
                    sReplace += Export::GetIsoLangByIndex(( USHORT ) nIndex );
                    sReplace += "\"";

                    sTextTag.SearchAndReplace( sSearch, sReplace );

                    sAdditionalLine += sTextTag;
                    sAdditionalLine += sText;
                    sAdditionalLine += pStackData->sEndTextTag;

                    sAdditionalLine += "\n";
                    sAdditionalLine += sLastWhitespace;

                    Output( sAdditionalLine );
                }
            }
        }
    }
    delete pResData;
    pResData = NULL;
    bGerman = FALSE;
    bEnglish = FALSE;
}
