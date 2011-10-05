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
#include "xrmmerge.hxx"
#include "tokens.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

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
#define STATE_LANGUAGES 0x000C

// set of global variables
sal_Bool bEnableExport;
sal_Bool bMergeMode;
sal_Bool bErrorLog;
sal_Bool bUTF8;
sal_Bool bDisplayName;
sal_Bool bExtensionDescription;
ByteString sPrj;
ByteString sPrjRoot;
ByteString sInputFileName;
ByteString sActFileName;
ByteString sOutputFile;
ByteString sMergeSrc;
ByteString sLangAttribute;
ByteString sResourceType;
String sUsedTempFile;
XRMResParser *pParser = NULL;

extern "C" {
// the whole interface to lexer is in this extern "C" section

/*****************************************************************************/
extern char *GetOutputFile( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = sal_False;
    bMergeMode = sal_False;
    bErrorLog = sal_True;
    bUTF8 = sal_True;
    bDisplayName = sal_False;
    bExtensionDescription = sal_False;
    sPrj = "";
    sPrjRoot = "";
    sInputFileName = "";
    sActFileName = "";
    Export::sLanguages = "";
    sal_uInt16 nState = STATE_NON;
    sal_Bool bInput = sal_False;

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
            bErrorLog = sal_False;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-L" ) {
            nState = STATE_LANGUAGES;
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
void removeTempFile(){
    if( !sUsedTempFile.EqualsIgnoreCaseAscii( "" ) ){
        DirEntry aTempFile( sUsedTempFile );
        aTempFile.Kill();
    }
}
/*****************************************************************************/
int InitXrmExport( char *pOutput , char* pFilename)
/*****************************************************************************/
{
    // instanciate Export
    ByteString sOutput( pOutput );
    ByteString sFilename( pFilename );
    Export::InitLanguages( false );

    if ( bMergeMode )
        pParser = new XRMResMerge( sMergeSrc, sOutputFile, sFilename );
      else if ( sOutputFile.Len()) {
        pParser = new XRMResExport( sOutputFile, sPrj, sActFileName );
    }

    return 1;
}

/*****************************************************************************/
int EndXrmExport()
/*****************************************************************************/
{
    delete pParser;
    return 1;
}
extern const char* getFilename()
{
    return sInputFileName.GetBuffer();
}
/*****************************************************************************/
extern FILE *GetXrmFile()
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
        }
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
    //printf("Typ = %d , text = '%s'\n",nTyp , pTokenText );
    pParser->Execute( nTyp, pTokenText );

    return 1;
}

/*****************************************************************************/
int SetError()
/*****************************************************************************/
{
    pParser->SetError();
    return 1;
}
}

extern "C" {
/*****************************************************************************/
int GetError()
/*****************************************************************************/
{
    return pParser->GetError();
}
}

//
// class XRMResParser
//


/*****************************************************************************/
XRMResParser::XRMResParser()
/*****************************************************************************/
                : bError( sal_False ),
                bText( sal_False )
{
    aLanguages = Export::GetLanguages();
}

/*****************************************************************************/
XRMResParser::~XRMResParser()
/*****************************************************************************/
{
}

/*****************************************************************************/
int XRMResParser::Execute( int nToken, char * pToken )
/*****************************************************************************/
{
    ByteString rToken( pToken );

    switch ( nToken ) {
        case XRM_TEXT_START:{
                ByteString sNewLID = GetAttribute( rToken, "id" );
                if ( sNewLID != sLID ) {
                    sLID = sNewLID;
                }
                bText = sal_True;
                sCurrentText = "";
                sCurrentOpenTag = rToken;
                Output( rToken );
            }
        break;

        case XRM_TEXT_END: {
                sCurrentCloseTag = rToken;
                sResourceType = ByteString ( "readmeitem" );
                sLangAttribute = ByteString ( "xml:lang" );
                WorkOnText( sCurrentOpenTag, sCurrentText );
                Output( sCurrentText );
                EndOfText( sCurrentOpenTag, sCurrentCloseTag );
                bText = sal_False;
                rToken = ByteString("");
                sCurrentText  = ByteString("");
        }
        break;

        case DESC_DISPLAY_NAME_START:{
                bDisplayName = sal_True;
            }
        break;

        case DESC_DISPLAY_NAME_END:{
                bDisplayName = sal_False;
            }
        break;

        case DESC_TEXT_START:{
                if (bDisplayName) {
                    sLID = ByteString("dispname");
                    bText = sal_True;
                    sCurrentText = "";
                    sCurrentOpenTag = rToken;
                    Output( rToken );
                }
            }
        break;

        case DESC_TEXT_END: {
                if (bDisplayName) {
                    sCurrentCloseTag = rToken;
                    sResourceType = ByteString ( "description" );
                    sLangAttribute = ByteString ( "lang" );
                    WorkOnText( sCurrentOpenTag, sCurrentText );
                    Output( sCurrentText );
                    EndOfText( sCurrentOpenTag, sCurrentCloseTag );
                    bText = sal_False;
                    rToken = ByteString("");
                    sCurrentText  = ByteString("");
                }
        }
        break;

        case DESC_EXTENSION_DESCRIPTION_START: {
                bExtensionDescription = sal_True;
            }
        break;

        case DESC_EXTENSION_DESCRIPTION_END: {
                bExtensionDescription = sal_False;
            }
        break;

        case DESC_EXTENSION_DESCRIPTION_SRC: {
                if (bExtensionDescription) {
                    sLID = ByteString("extdesc");
                    sResourceType = ByteString ( "description" );
                    sLangAttribute = ByteString ( "lang" );
                    sCurrentOpenTag = rToken;
                    sCurrentText  = ByteString("");
                    Output( rToken );
                    WorkOnDesc( sCurrentOpenTag, sCurrentText );
                    sCurrentCloseTag = rToken;
                    Output( sCurrentText );
                    rToken = ByteString("");
                    sCurrentText  = ByteString("");
                }
            }
        break;

        default:
            if ( bText ) {
                sCurrentText += rToken;
            }
        break;
    }

    if ( !bText )
    {
        Output( rToken );
    }
    return 0;
}

/*****************************************************************************/
ByteString XRMResParser::GetAttribute( const ByteString &rToken, const ByteString &rAttribute )
/*****************************************************************************/
{
    ByteString sTmp( rToken );
    sTmp.SearchAndReplaceAll( "\t", " " );

    ByteString sSearch( " " );
    sSearch += rAttribute;
    sSearch += "=";
    sal_uInt16 nPos = sTmp.Search( sSearch );

    if ( nPos != STRING_NOTFOUND ) {
        sTmp = sTmp.Copy( nPos );
        ByteString sId = sTmp.GetToken( 1, '\"' );
        return sId;
    }
    return "";
}


/*****************************************************************************/
void XRMResParser::Error( const ByteString &rError )
/*****************************************************************************/
{
    yyerror(( char * ) rError.GetBuffer());
}

/*****************************************************************************/
void XRMResParser::ConvertStringToDBFormat( ByteString &rString )
/*****************************************************************************/
{
    ByteString sResult;
    do {
        sResult = rString;
        rString.EraseLeadingChars( _LF );
        rString.EraseLeadingChars( '\t' );
        rString.EraseTrailingChars( '\t' );
    } while ( sResult != rString );

    rString.SearchAndReplaceAll( "\t", "\\t" );
}

/*****************************************************************************/
void XRMResParser::ConvertStringToXMLFormat( ByteString &rString )
/*****************************************************************************/
{
    rString.SearchAndReplaceAll( "\\t", "\t" );
}



//
// class XRMResOutputParser
//

/*****************************************************************************/
XRMResOutputParser::XRMResOutputParser ( const ByteString &rOutputFile )
/*****************************************************************************/
{
    aLanguages = Export::GetLanguages();
    pOutputStream =
        new SvFileStream(
            String( rOutputFile, RTL_TEXTENCODING_ASCII_US ),
            STREAM_STD_WRITE | STREAM_TRUNC
        );
    pOutputStream->SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
    if ( !pOutputStream->IsOpen()) {
        ByteString sError( "Unable to open output file: " );
        sError += rOutputFile;
        Error( sError );
        delete pOutputStream;
        pOutputStream = NULL;
    }
}

/*****************************************************************************/
XRMResOutputParser::~XRMResOutputParser()
/*****************************************************************************/
{
    if ( pOutputStream ) {
        pOutputStream->Close();
        delete pOutputStream;
    }
}

//
// class XMLResExport
//

/*****************************************************************************/
XRMResExport::XRMResExport(
    const ByteString &rOutputFile, const ByteString &rProject,
    const ByteString &rFilePath )
/*****************************************************************************/
                : XRMResOutputParser( rOutputFile ),
                pResData( NULL ),
                sPrj( rProject ),
                sPath( rFilePath )
{
    aLanguages = Export::GetLanguages();
}

/*****************************************************************************/
XRMResExport::~XRMResExport()
/*****************************************************************************/
{
    delete pResData;
}

void XRMResExport::Output( const ByteString& rOutput )
{
    // Dummy to suppress warnings caused by poor class design
    (void) rOutput;
}

/*****************************************************************************/
void XRMResExport::WorkOnDesc(
    const ByteString &rOpenTag,
    ByteString &rText
)
/*****************************************************************************/
{
    DirEntry aEntry( String( sInputFileName, RTL_TEXTENCODING_ASCII_US ));
    aEntry.ToAbs();
    ByteString sDescFileName( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
    sDescFileName.SearchAndReplaceAll( "description.xml", "" );
    sDescFileName += GetAttribute( rOpenTag, "xlink:href" );
    ifstream::pos_type size;
    char * memblock;
    ifstream file (sDescFileName.GetBuffer(), ios::in|ios::binary|ios::ate);
    if (file.is_open()) {
        size = file.tellg();
        memblock = new char [size];
        file.seekg (0, ios::beg);
        file.read (memblock, size);
        file.close();
        rText = ByteString(memblock);
        rText.SearchAndReplaceAll( "\n", "\\n" );
        delete[] memblock;
     }
    WorkOnText( rOpenTag, rText );
    EndOfText( rOpenTag, rOpenTag );
}

//*****************************************************************************/
void XRMResExport::WorkOnText(
    const ByteString &rOpenTag,
    ByteString &rText
)
/*****************************************************************************/
{
    ByteString sLang( GetAttribute( rOpenTag, sLangAttribute ));

    if ( !pResData )
    {
        ByteString sPlatform( "" );
        pResData = new ResData( sPlatform, GetGID() );
        pResData->sId = GetLID();
    }

    ByteString sText(rText);
    ConvertStringToDBFormat(sText);
    pResData->sText[sLang] = sText;
}

/*****************************************************************************/
void XRMResExport::EndOfText(
    const ByteString &rOpenTag,
    const ByteString &rCloseTag
)
/*****************************************************************************/
{

    (void) rOpenTag;        // FIXME
    (void) rCloseTag;       // FIXME

    if ( pResData && pOutputStream ) {
        ByteString sTimeStamp( Export::GetTimeStamp());
        ByteString sCur;
        for( unsigned int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];

            ByteString sAct = pResData->sText[ sCur ];
                sAct.EraseAllChars( 0x0A );

                ByteString sOutput( sPrj ); sOutput += "\t";
                sOutput += sPath;
                sOutput += "\t0\t";
                sOutput += sResourceType;
                sOutput += "\t";
                sOutput += pResData->sId;
                // USE LID AS GID OR MERGE DON'T WORK
                //sOutput += pResData->sGId;
                sOutput += "\t";
                sOutput += pResData->sId;
                sOutput += "\t\t\t0\t";
                sOutput += sCur;
                sOutput += "\t";

                sOutput += sAct; sOutput += "\t\t\t\t";
                sOutput += sTimeStamp;

                sal_Char cSearch = 0x00;
                sOutput.SearchAndReplaceAll( cSearch, '_' );
                if( sAct.Len() > 1 )
                    pOutputStream->WriteLine( sOutput );
            }
    }
    delete pResData;
    pResData = NULL;
}

//
// class XRMResMerge
//

/*****************************************************************************/
XRMResMerge::XRMResMerge(
    const ByteString &rMergeSource, const ByteString &rOutputFile,
    ByteString &rFilename)
/*****************************************************************************/
                : XRMResOutputParser( rOutputFile ),
                pMergeDataFile( NULL ),
                sFilename( rFilename ) ,
                pResData( NULL )
{
    if ( rMergeSource.Len())
        pMergeDataFile = new MergeDataFile(
            rMergeSource, sInputFileName, bErrorLog);
    if( Export::sLanguages.EqualsIgnoreCaseAscii("ALL") ){
        Export::SetLanguages( pMergeDataFile->GetLanguages() );
        aLanguages = pMergeDataFile->GetLanguages();
    }
    else aLanguages = Export::GetLanguages();
}

/*****************************************************************************/
XRMResMerge::~XRMResMerge()
/*****************************************************************************/
{
    delete pMergeDataFile;
    delete pResData;
}

/*****************************************************************************/
void XRMResMerge::WorkOnDesc(
    const ByteString &rOpenTag,
    ByteString &rText
)
/*****************************************************************************/
{
    WorkOnText( rOpenTag, rText);
    if ( pMergeDataFile && pResData ) {
        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
        if ( pEntrys ) {
            ByteString sCur;
            ByteString sDescFilename = GetAttribute ( rOpenTag, "xlink:href" );
            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
                ByteString sContent;
                if ( !sCur.EqualsIgnoreCaseAscii("en-US")  &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, sCur, sal_True )) &&
                    ( sContent != "-" ) && ( sContent.Len()))
                {
                    ByteString sText( sContent );
                    ByteString sAdditionalLine( "\n" );
                    sAdditionalLine += rOpenTag;
                    ByteString sSearch = sLangAttribute;
                    sSearch += "=\"";
                    ByteString sReplace( sSearch );

                    sSearch += GetAttribute( rOpenTag, sLangAttribute );
                    sReplace += sCur;
                    sAdditionalLine.SearchAndReplace( sSearch, sReplace );

                    sSearch = ByteString("xlink:href=\"");
                    sReplace = sSearch;

                    ByteString sLocDescFilename = sCur;
                    sLocDescFilename += ByteString("-");
                    sLocDescFilename += sDescFilename;

                    sSearch += sDescFilename;
                    sReplace += sLocDescFilename;
                    sAdditionalLine.SearchAndReplace( sSearch, sReplace );

                    Output( sAdditionalLine );

                    DirEntry aEntry( String( sOutputFile, RTL_TEXTENCODING_ASCII_US ));
                    aEntry.ToAbs();
                    ByteString sOutputDescFile( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
                    sOutputDescFile.SearchAndReplaceAll( "description.xml", "" );
                    sOutputDescFile += sLocDescFilename;
                    sText.SearchAndReplaceAll( "\\n", "\n" );
                    ofstream file ( sOutputDescFile.GetBuffer() );
                    if (file.is_open()) {
                        file << sText.GetBuffer();
                        file.close();
                    }
                }
            }
        }
    }
    delete pResData;
    pResData = NULL;
}

/*****************************************************************************/
void XRMResMerge::WorkOnText(
    const ByteString &rOpenTag,
    ByteString &rText
)
/*****************************************************************************/
{
    ByteString sLang( GetAttribute( rOpenTag, sLangAttribute ));

    if ( pMergeDataFile ) {
        if ( !pResData ) {
            ByteString sPlatform( "" );
            pResData = new ResData( sPlatform, GetLID() , sFilename );
            pResData->sId = GetLID();
            pResData->sResTyp = sResourceType;
        }

        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
            if ( pEntrys ) {
                ByteString sContent;
                if ( Export::isAllowed( sLang ) &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, sLang )) &&
                    ( sContent != "-" ) && ( sContent.Len()))

                {
                    rText = sContent;
                    ConvertStringToXMLFormat( rText );
                }
            }
    }
}

/*****************************************************************************/
void XRMResMerge::Output( const ByteString& rOutput )
/*****************************************************************************/
{
    //printf("W: %s\n",rOutput.GetBuffer());
    if ( pOutputStream && rOutput.Len() > 0 )
        pOutputStream->Write( rOutput.GetBuffer(), rOutput.Len());
}

/*****************************************************************************/
void XRMResMerge::EndOfText(
    const ByteString &rOpenTag,
    const ByteString &rCloseTag
)
/*****************************************************************************/
{

    Output( rCloseTag );
    if ( pMergeDataFile && pResData ) {
        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
        if ( pEntrys ) {
            ByteString sCur;
            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
                ByteString sContent;
                if ( !sCur.EqualsIgnoreCaseAscii("en-US")  &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, sCur, sal_True )) &&
                    ( sContent != "-" ) && ( sContent.Len()))
                {
                    ByteString sText( sContent );
                    ByteString sAdditionalLine( "\n" );
                    sAdditionalLine += rOpenTag;
                    ByteString sSearch = sLangAttribute;
                    sSearch += "=\"";
                    ByteString sReplace( sSearch );

                    sSearch += GetAttribute( rOpenTag, sLangAttribute );
                    sReplace += sCur;

                    sAdditionalLine.SearchAndReplace( sSearch, sReplace );

                    sAdditionalLine += sText;
                    sAdditionalLine += rCloseTag;

                    Output( sAdditionalLine );
                }
            }
        }
    }
    delete pResData;
    pResData = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
