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
sal_Bool bEnableExport;
sal_Bool bMergeMode;
sal_Bool bErrorLog;
sal_Bool bForce;
sal_Bool bUTF8;
ByteString sPrj;
ByteString sPrjRoot;
ByteString sInputFileName;
ByteString sActFileName;
ByteString sFullEntry;
ByteString sOutputFile;
ByteString sMergeSrc;
String sUsedTempFile;

CfgParser *pParser;

extern "C" {
// the whole interface to lexer is in this extern "C" section

/*****************************************************************************/
extern char *GetOutputFile( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport   = sal_False;
    bMergeMode      = sal_False;
    bErrorLog       = sal_True;
    bForce          = sal_False;
    bUTF8           = sal_True;
    sPrj            = "";
    sPrjRoot        = "";
    sInputFileName  = "";
    sActFileName    = "";

    sal_uInt16 nState = STATE_NON;
    sal_Bool bInput = sal_False;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        ByteString sSwitch( argv[ i ] );
        sSwitch.ToUpperAscii();

        if ( sSwitch == "-I" ) {
            nState = STATE_INPUT; // next token specifies source file
        }
        else if ( sSwitch == "-O" ) {
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
        else if ( sSwitch == "-NOUTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = sal_False;
        }
        else if ( sSwitch == "-F" ) {
            nState = STATE_FORCE;
            bForce = sal_True;
        }
        else if ( sSwitch == "-L" ) {
            nState = STATE_LANGUAGES;
        }
        else if ( sSwitch.ToUpperAscii() == "-ISO99" ) {
            nState = STATE_ISOCODE99;
        }
        else {
            switch ( nState ) {
                case STATE_NON: {
                    return NULL;    // no valid command line
                }
                case STATE_INPUT: {
                    sInputFileName = argv[ i ];
                    bInput = sal_True; // source file found
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
                    bMergeMode = sal_True; // activate merge mode, cause merge database found
                }
                break;
                case STATE_LANGUAGES: {
                    Export::sLanguages = ByteString( argv[ i ]);
                }
                break;
            }
        }
    }

    if ( bInput ) {
        // command line is valid
        bEnableExport = sal_True;
        char *pReturn = new char[ sOutputFile.Len() + 1 ];
        strcpy( pReturn, sOutputFile.GetBuffer());  // #100211# - checked
        return pReturn;
    }

    // command line is not valid
    return NULL;
}
/*****************************************************************************/
int InitCfgExport( char *pOutput , char* pFilename )
/*****************************************************************************/
{
    // instanciate Export
    ByteString sOutput( pOutput );
    ByteString sFilename( pFilename );
    Export::InitLanguages();

    if ( bMergeMode )
        pParser = new CfgMerge( sMergeSrc, sOutputFile, sFilename );
      else if ( sOutputFile.Len())
        pParser = new CfgExport( sOutputFile, sPrj, sActFileName );

    return 1;
}

/*****************************************************************************/
int EndCfgExport()
/*****************************************************************************/
{
    delete pParser;

    return 1;
}

void removeTempFile(){
    if( !sUsedTempFile.EqualsIgnoreCaseAscii( "" ) ){
        DirEntry aTempFile( sUsedTempFile );
        aTempFile.Kill();
    }
}
extern const char* getFilename()
{
    return sInputFileName.GetBuffer();
}
/*****************************************************************************/
extern FILE *GetCfgFile()
/*****************************************************************************/
{
    FILE *pFile = 0;
    // look for valid filename
    if ( sInputFileName.Len()) {
        if( Export::fileHasUTF8ByteOrderMarker( sInputFileName ) ){
            DirEntry aTempFile = Export::GetTempFile();
            DirEntry aSourceFile( String( sInputFileName , RTL_TEXTENCODING_ASCII_US ) );
            aSourceFile.CopyTo( aTempFile , FSYS_ACTION_COPYFILE );
            String sTempFile = aTempFile.GetFull();
            Export::RemoveUTF8ByteOrderMarkerFromFile( ByteString( sTempFile , RTL_TEXTENCODING_ASCII_US ) );
            pFile = fopen( ByteString( sTempFile , RTL_TEXTENCODING_ASCII_US ).GetBuffer(), "r" );
            sUsedTempFile = sTempFile;
        }else{
            // able to open file?
            pFile = fopen( sInputFileName.GetBuffer(), "r" );
            sUsedTempFile = String::CreateFromAscii("");
        }
        if ( !pFile ){
            fprintf( stderr, "Error: Could not open file %s\n",
                sInputFileName.GetBuffer());
            exit( -13 );
        }
        else {
            // this is a valid file which can be opened, so
            // create path to project root
            DirEntry aEntry( String( sInputFileName, RTL_TEXTENCODING_ASCII_US ));
            aEntry.ToAbs();
            sFullEntry= ByteString( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
            aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
            aEntry += DirEntry( sPrjRoot );
            ByteString sPrjEntry( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );

            // create file name, beginnig with project root
            // (e.g.: source\ui\src\menue.src)
//            printf("sFullEntry = %s\n",sFullEntry.GetBuffer());
            sActFileName = sFullEntry.Copy( sPrjEntry.Len() + 1 );
//            printf("sActFileName = %s\n",sActFileName.GetBuffer());

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

CfgStackData* CfgStack::Push( const ByteString &rTag, const ByteString &rId )
{
    CfgStackData *pD = new CfgStackData( rTag, rId );
    maList.push_back( pD );
    return pD;
}

//
// class CfgStack
//

/*****************************************************************************/
CfgStack::~CfgStack()
/*****************************************************************************/
{
    for ( size_t i = 0, n = maList.size(); i < n; i++ )
        delete maList[ i ];
    maList.clear();
}

/*****************************************************************************/
ByteString CfgStack::GetAccessPath( size_t nPos )
/*****************************************************************************/
{
    if ( nPos == LIST_APPEND )
        nPos = maList.size() - 1;

    ByteString sReturn;
    for ( size_t i = 0; i <= nPos; i++ ) {
        if ( i )
            sReturn += ".";
        sReturn += GetStackData( i )->GetIdentifier();
    }

    return sReturn;
}

/*****************************************************************************/
CfgStackData *CfgStack::GetStackData( size_t nPos )
/*****************************************************************************/
{
    if ( nPos == LIST_APPEND )
    {
        if (!maList.empty())
            nPos = maList.size() - 1;
        else
            return 0;
    }

    return maList[  nPos ];
}

//
// class CfgParser
//

/*****************************************************************************/
CfgParser::CfgParser()
/*****************************************************************************/
                : pStackData( NULL ),
                bLocalize( sal_False )
{
}

/*****************************************************************************/
CfgParser::~CfgParser()
/*****************************************************************************/
{
}


/*****************************************************************************/
sal_Bool CfgParser::IsTokenClosed( const ByteString &rToken )
/*****************************************************************************/
{
    return rToken.GetChar( rToken.Len() - 2 ) == '/';
}

/*****************************************************************************/
void CfgParser::AddText(
    ByteString &rText,
    const ByteString &rIsoLang,
    const ByteString &rResTyp
)
/*****************************************************************************/
{
        sal_uInt16 nTextLen = 0;
        while ( rText.Len() != nTextLen ) {
            nTextLen = rText.Len();
            rText.SearchAndReplaceAll( "\n", " " );
            rText.SearchAndReplaceAll( "\r", " " );
            rText.SearchAndReplaceAll( "\t", " " );
            rText.SearchAndReplaceAll( "  ", " " );
        }
        pStackData->sResTyp = rResTyp;
        WorkOnText( rText, rIsoLang );

        pStackData->sText[ rIsoLang ] = rText;
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

    sal_Bool bOutput = sal_True;

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
                        bLocalize = sal_True;
                    break;
                    case CFG_TOKEN_OORVALUE:
                        sSearch = "oor:value=";
                    break;
                    case CFG_TEXT_START: {
                        if ( sCurrentResTyp != sTokenName ) {
                            WorkOnRessourceEnd();
                            ByteString sCur;
                            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                                sCur = aLanguages[ n ];
                                pStackData->sText[ sCur ] = ByteString("");
                            }
                         }
                        sCurrentResTyp = sTokenName;

                        ByteString sTemp = sToken.Copy( sToken.Search( "xml:lang=" ));
                        sCurrentIsoLang = sTemp.GetToken( 1, '\"' ).GetToken( 0, '\"' );

                        if ( sCurrentIsoLang == NO_TRANSLATE_ISO )
                            bLocalize = sal_False;

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
                        ( sTemp.Search( "CFG:LOCALIZED=\"sal_True\"" ) != STRING_NOTFOUND ));
                }
            }
            else if ( sTokenName == "label" ) {
                if ( sCurrentResTyp != sTokenName ) {
                    WorkOnRessourceEnd();
                    ByteString sCur;
                    for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                        sCur = aLanguages[ n ];
                        pStackData->sText[ sCur ] = ByteString("");
                    }
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
                ByteString sError( "Misplaced close tag: " );
                ByteString sInFile(" in file ");
                sError += sToken;
                sError += sInFile;
                sError += sFullEntry;
                Error( sError );
                exit ( 13 );
            }
        break;

        case CFG_TEXTCHAR:
            sCurrentText += sToken;
            bOutput = sal_False;
        break;

        case CFG_TOKEN_NO_TRANSLATE:
            bLocalize = sal_False;
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
void CfgExport::Output( const ByteString& rOutput )
/*****************************************************************************/
{
    // Dummy operation to suppress warnings caused by poor class design
    ByteString a( rOutput );
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
    pOutputStream->SetStreamCharSet( RTL_TEXTENCODING_UTF8 );

    if ( !pOutputStream->IsOpen()) {
        ByteString sError( "ERROR: Unable to open output file: " );
        sError += rOutputFile;
        Error( sError );
        delete pOutputStream;
        pOutputStream = NULL;
        exit( -13 );
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
    Export::InitLanguages( false );
    aLanguages = Export::GetLanguages();
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
    if (( pStackData->sText[ ByteString("en-US") ].Len()
        ) ||
            ( bForce &&
                ( pStackData->sText[ ByteString("de") ].Len() ||
                    pStackData->sText[ ByteString("en-US") ].Len() )))
        {
            ByteString sFallback = pStackData->sText[ ByteString("en-US") ];
            ByteString sLocalId = pStackData->sIdentifier;
            ByteString sGroupId;
            if ( aStack.size() == 1 ) {
                sGroupId = sLocalId;
                sLocalId = "";
            }
            else {
                sGroupId = aStack.GetAccessPath( aStack.size() - 2 );
            }

            ByteString sTimeStamp( Export::GetTimeStamp());

            ByteString sCur;
            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];

                    ByteString sText = pStackData->sText[ sCur ];
                    if ( !sText.Len())
                        sText = sFallback;

                    Export::UnquotHTML( sText );

                    ByteString sOutput( sPrj ); sOutput += "\t";
                    sOutput += sPath;
                    sOutput += "\t0\t";
                    sOutput += pStackData->sResTyp; sOutput += "\t";
                    sOutput += sGroupId; sOutput += "\t";
                    sOutput += sLocalId; sOutput += "\t\t\t0\t";
                    sOutput += sCur;
                    sOutput += "\t";

                    sOutput += sText; sOutput += "\t\t\t\t";
                    sOutput += sTimeStamp;

                    pOutputStream->WriteLine( sOutput );
            }
        }
    }
}

/*****************************************************************************/
void CfgExport::WorkOnText(
    ByteString &rText,
    const ByteString &rIsoLang
)
/*****************************************************************************/
{
    if( rIsoLang.Len() ) Export::UnquotHTML( rText );
}


//
// class CfgMerge
//

/*****************************************************************************/
CfgMerge::CfgMerge(
    const ByteString &rMergeSource, const ByteString &rOutputFile,
    ByteString &rFilename )
/*****************************************************************************/
                : CfgOutputParser( rOutputFile ),
                pMergeDataFile( NULL ),
                pResData( NULL ),
                sFilename( rFilename ),
                bEnglish( sal_False )
{
    if ( rMergeSource.Len()){
        pMergeDataFile = new MergeDataFile(
        rMergeSource, sInputFileName  , bErrorLog, RTL_TEXTENCODING_MS_1252, true );
        if( Export::sLanguages.EqualsIgnoreCaseAscii("ALL") ){
            Export::SetLanguages( pMergeDataFile->GetLanguages() );
            aLanguages = pMergeDataFile->GetLanguages();
        }
        else aLanguages = Export::GetLanguages();
    }else
        aLanguages = Export::GetLanguages();
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
    const ByteString& nLangIndex
)
/*****************************************************************************/
{

    if ( pMergeDataFile && bLocalize ) {
        if ( !pResData ) {
            ByteString sLocalId = pStackData->sIdentifier;
            ByteString sGroupId;
            if ( aStack.size() == 1 ) {
                sGroupId = sLocalId;
                sLocalId = "";
            }
            else {
                sGroupId = aStack.GetAccessPath( aStack.size() - 2 );
            }

            ByteString sPlatform( "" );

            pResData = new ResData( sPlatform, sGroupId , sFilename );
            pResData->sId = sLocalId;
            pResData->sResTyp = pStackData->sResTyp;
        }

        if (( nLangIndex.EqualsIgnoreCaseAscii("en-US") ))
            bEnglish = sal_True;

        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrysCaseSensitive( pResData );
        if ( pEntrys ) {
            ByteString sContent;
            pEntrys->GetText( sContent, STRING_TYP_TEXT, nLangIndex );

            if ( Export::isAllowed( nLangIndex ) &&
                ( sContent != "-" ) && ( sContent.Len()))
            {
#ifdef MERGE_SOURCE_LANGUAGES
                    if( nLangIndex.EqualsIgnoreCaseAscii("de") || nLangIndex.EqualsIgnoreCaseAscii("en-US") )
                        rText = sContent;
#endif
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

size_t CfgStack::Push( CfgStackData *pStackData )
{
    maList.push_back( pStackData );
    return maList.size() - 1;
}

/*****************************************************************************/
void CfgMerge::WorkOnRessourceEnd()
/*****************************************************************************/
{

    if ( pMergeDataFile && pResData && bLocalize && (( bEnglish ) || bForce )) {
        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrysCaseSensitive( pResData );
        if ( pEntrys ) {
            ByteString sCur;

            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];

                ByteString sContent;
                pEntrys->GetText( sContent, STRING_TYP_TEXT, sCur , sal_True );
                if (
                    ( !sCur.EqualsIgnoreCaseAscii("en-US") ) &&

                    ( sContent != "-" ) && ( sContent.Len()))
                {

                    ByteString sText = sContent;
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
                    sReplace += sCur;
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
    bEnglish = sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
