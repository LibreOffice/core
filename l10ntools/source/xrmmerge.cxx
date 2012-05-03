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

#include <cstring>

#include <stdio.h>

#include "common.hxx"
#include "export.hxx"
#include "xrmmerge.hxx"
#include "tokens.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

void yyerror( const char * );
void YYWarning( const char * );

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
rtl::OString sPrj;
rtl::OString sPrjRoot;
rtl::OString sInputFileName;
rtl::OString sActFileName;
rtl::OString sOutputFile;
rtl::OString sMergeSrc;
rtl::OString sLangAttribute;
rtl::OString sResourceType;
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
        if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-I" ) {
            nState = STATE_INPUT; // next token specifies source file
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-O" ) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-P" ) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-R" ) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-M" ) {
            nState = STATE_MERGESRC; // next token specifies the merge database
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-E" ) {
            nState = STATE_ERRORLOG;
            bErrorLog = sal_False;
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-L" ) {
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
                    sPrj = rtl::OString( argv[ i ]);
                }
                break;
                case STATE_ROOT: {
                    sPrjRoot = rtl::OString( argv[ i ]); // path to project root
                }
                break;
                case STATE_MERGESRC: {
                    sMergeSrc = rtl::OString( argv[ i ]);
                    bMergeMode = sal_True; // activate merge mode, cause merge database found
                }
                break;
                case STATE_LANGUAGES: {
                    Export::sLanguages = rtl::OString( argv[ i ]);
                }
                break;
            }
        }
    }

    if ( bInput ) {
        // command line is valid
        bEnableExport = sal_True;
        char *pReturn = new char[ sOutputFile.getLength() + 1 ];
        std::strcpy( pReturn, sOutputFile.getStr());  // #100211# - checked
        return pReturn;
    }

    // command line is not valid
    return NULL;
}

/*****************************************************************************/
int InitXrmExport( char *pOutput , char* pFilename)
/*****************************************************************************/
{
    // instanciate Export
    rtl::OString sOutput( pOutput );
    rtl::OString sFilename( pFilename );
    Export::InitLanguages( false );

    if ( bMergeMode )
        pParser = new XRMResMerge( sMergeSrc, sOutputFile, sFilename );
      else if (!sOutputFile.isEmpty()) {
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
    return sInputFileName.getStr();
}
/*****************************************************************************/
extern FILE *GetXrmFile()
/*****************************************************************************/
{
    // look for valid filename
    if (!sInputFileName.isEmpty()) {
        //TODO: explicit BOM handling?
        FILE * pFile = fopen(sInputFileName.getStr(), "r");
        if ( !pFile ){
            fprintf( stderr, "Error: Could not open file %s\n",
                sInputFileName.getStr());
        }
        else {
            if (!bMergeMode) {
                sActFileName = common::pathnameToken(
                    sInputFileName.getStr(), sPrjRoot.getStr());
            }
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
    rtl::OString rToken( pToken );

    switch ( nToken ) {
        case XRM_TEXT_START:{
                rtl::OString sNewLID = GetAttribute( rToken, "id" );
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
                sResourceType = rtl::OString ( "readmeitem" );
                sLangAttribute = rtl::OString ( "xml:lang" );
                WorkOnText( sCurrentOpenTag, sCurrentText );
                Output( sCurrentText );
                EndOfText( sCurrentOpenTag, sCurrentCloseTag );
                bText = sal_False;
                rToken = rtl::OString("");
                sCurrentText  = rtl::OString("");
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
                    sLID = rtl::OString("dispname");
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
                    sResourceType = rtl::OString ( "description" );
                    sLangAttribute = rtl::OString ( "lang" );
                    WorkOnText( sCurrentOpenTag, sCurrentText );
                    Output( sCurrentText );
                    EndOfText( sCurrentOpenTag, sCurrentCloseTag );
                    bText = sal_False;
                    rToken = rtl::OString("");
                    sCurrentText  = rtl::OString("");
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
                    sLID = rtl::OString("extdesc");
                    sResourceType = rtl::OString ( "description" );
                    sLangAttribute = rtl::OString ( "lang" );
                    sCurrentOpenTag = rToken;
                    sCurrentText  = rtl::OString("");
                    Output( rToken );
                    WorkOnDesc( sCurrentOpenTag, sCurrentText );
                    sCurrentCloseTag = rToken;
                    Output( sCurrentText );
                    rToken = rtl::OString("");
                    sCurrentText  = rtl::OString("");
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
rtl::OString XRMResParser::GetAttribute( const rtl::OString &rToken, const rtl::OString &rAttribute )
/*****************************************************************************/
{
    rtl::OString sTmp( rToken );
    sTmp = sTmp.replace('\t', ' ');

    rtl::OString sSearch( " " );
    sSearch += rAttribute;
    sSearch += "=";
    sal_Int32 nPos = sTmp.indexOf( sSearch );

    if ( nPos != -1 )
    {
        sTmp = sTmp.copy( nPos );
        rtl::OString sId = sTmp.getToken(1, '"');
        return sId;
    }
    return rtl::OString();
}


/*****************************************************************************/
void XRMResParser::Error( const rtl::OString &rError )
/*****************************************************************************/
{
    yyerror(( char * ) rError.getStr());
}

/*****************************************************************************/
void XRMResParser::ConvertStringToDBFormat( rtl::OString &rString )
/*****************************************************************************/
{
    rString = rString.trim().replaceAll("\t", "\\t");
}

/*****************************************************************************/
void XRMResParser::ConvertStringToXMLFormat( rtl::OString &rString )
/*****************************************************************************/
{
    rString = rString.replaceAll("\\t", "\t");
}



//
// class XRMResOutputParser
//

/*****************************************************************************/
XRMResOutputParser::XRMResOutputParser ( const rtl::OString &rOutputFile )
/*****************************************************************************/
{
    aLanguages = Export::GetLanguages();
    pOutputStream.open(
        rOutputFile.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!pOutputStream.is_open()) {
        rtl::OString sError( "Unable to open output file: " );
        sError += rOutputFile;
        Error( sError );
    }
}

/*****************************************************************************/
XRMResOutputParser::~XRMResOutputParser()
/*****************************************************************************/
{
    pOutputStream.close();
}

//
// class XMLResExport
//

/*****************************************************************************/
XRMResExport::XRMResExport(
    const rtl::OString &rOutputFile, const rtl::OString &rProject,
    const rtl::OString &rFilePath )
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

void XRMResExport::Output( const rtl::OString& ) {}

/*****************************************************************************/
void XRMResExport::WorkOnDesc(
    const rtl::OString &rOpenTag,
    rtl::OString &rText
)
/*****************************************************************************/
{
    rtl::OString sDescFileName(
        sInputFileName.replaceAll("description.xml", rtl::OString()));
    sDescFileName += GetAttribute( rOpenTag, "xlink:href" );
    int size;
    char * memblock;
    ifstream file (sDescFileName.getStr(), ios::in|ios::binary|ios::ate);
    if (file.is_open()) {
        size = static_cast<int>(file.tellg());
        memblock = new char [size+1];
        file.seekg (0, ios::beg);
        file.read (memblock, size);
        file.close();
        memblock[size] = '\0';
        rText = rtl::OString(memblock).replaceAll("\n", "\\n");
        delete[] memblock;
     }
    WorkOnText( rOpenTag, rText );
    EndOfText( rOpenTag, rOpenTag );
}

//*****************************************************************************/
void XRMResExport::WorkOnText(
    const rtl::OString &rOpenTag,
    rtl::OString &rText
)
/*****************************************************************************/
{
    rtl::OString sLang( GetAttribute( rOpenTag, sLangAttribute ));

    if ( !pResData )
    {
        rtl::OString sPlatform( "" );
        pResData = new ResData( sPlatform, GetGID() );
        pResData->sId = GetLID();
    }

    rtl::OString sText(rText);
    ConvertStringToDBFormat(sText);
    pResData->sText[sLang] = sText;
}

/*****************************************************************************/
void XRMResExport::EndOfText(
    const rtl::OString &,
    const rtl::OString &
)
/*****************************************************************************/
{
    if ( pResData )
    {
        rtl::OString sCur;
        for( unsigned int n = 0; n < aLanguages.size(); n++ )
        {
            sCur = aLanguages[ n ];

            rtl::OString sAct(
                pResData->sText[sCur].replaceAll("\x0A", rtl::OString()));

            rtl::OString sOutput( sPrj ); sOutput += "\t";
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

            sOutput += sAct;
            sOutput += "\t\t\t\t";

            sOutput = sOutput.replace('\0', '_');
            if( sAct.getLength() > 1 )
                pOutputStream << sOutput.getStr() << '\n';
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
    const rtl::OString &rMergeSource, const rtl::OString &rOutputFile,
    const rtl::OString &rFilename)
/*****************************************************************************/
                : XRMResOutputParser( rOutputFile ),
                pMergeDataFile( NULL ),
                sFilename( rFilename ) ,
                pResData( NULL )
{
    if (!rMergeSource.isEmpty())
        pMergeDataFile = new MergeDataFile(
            rMergeSource, sInputFileName, bErrorLog);
    if( Export::sLanguages.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("ALL")))
    {
        Export::SetLanguages( pMergeDataFile->GetLanguages() );
        aLanguages = pMergeDataFile->GetLanguages();
    }
    else
        aLanguages = Export::GetLanguages();
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
    const rtl::OString &rOpenTag,
    rtl::OString &rText
)
/*****************************************************************************/
{
    WorkOnText( rOpenTag, rText);
    if ( pMergeDataFile && pResData ) {
        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
        if ( pEntrys ) {
            rtl::OString sCur;
            rtl::OString sDescFilename = GetAttribute ( rOpenTag, "xlink:href" );
            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
                rtl::OString sContent;
                if ( !sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US"))  &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, sCur, sal_True )) &&
                    ( sContent != "-" ) && !sContent.isEmpty())
                {
                    rtl::OString sText( sContent );
                    rtl::OString sAdditionalLine( "\n        " );
                    sAdditionalLine += rOpenTag;
                    rtl::OString sSearch = sLangAttribute;
                    sSearch += "=\"";
                    rtl::OString sReplace( sSearch );

                    sSearch += GetAttribute( rOpenTag, sLangAttribute );
                    sReplace += sCur;
                    sAdditionalLine = sAdditionalLine.replaceFirst(
                        sSearch, sReplace);

                    sSearch = rtl::OString("xlink:href=\"");
                    sReplace = sSearch;

                    rtl::OString sLocDescFilename = sDescFilename;
                    sLocDescFilename = sLocDescFilename.replaceFirst(
                        "en-US", sCur);

                    sSearch += sDescFilename;
                    sReplace += sLocDescFilename;
                    sAdditionalLine = sAdditionalLine.replaceFirst(
                        sSearch, sReplace);

                    Output( sAdditionalLine );

                    sal_Int32 i = sOutputFile.lastIndexOf('/');
                    if (i == -1) {
                        std::cerr
                            << "Error: output file " << sOutputFile.getStr()
                            << " does not contain any /\n";
                        throw false; //TODO
                    }
                    rtl::OString sOutputDescFile(
                        sOutputFile.copy(0, i + 1) + sLocDescFilename);
                    sText = sText.replaceAll("\\n", "\n");
                    ofstream file(sOutputDescFile.getStr());
                    if (file.is_open()) {
                        file << sText.getStr();
                        file.close();
                    } else {
                        std::cerr
                            << "Error: cannot write "
                            << sOutputDescFile.getStr() << '\n';
                        throw false; //TODO
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
    const rtl::OString &rOpenTag,
    rtl::OString &rText
)
/*****************************************************************************/
{
    rtl::OString sLang( GetAttribute( rOpenTag, sLangAttribute ));

    if ( pMergeDataFile ) {
        if ( !pResData ) {
            rtl::OString sPlatform( "" );
            pResData = new ResData( sPlatform, GetLID() , sFilename );
            pResData->sId = GetLID();
            pResData->sResTyp = sResourceType;
        }

        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
            if ( pEntrys ) {
                rtl::OString sContent;
                if ( Export::isAllowed( sLang ) &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, sLang )) &&
                    ( sContent != "-" ) && !sContent.isEmpty())

                {
                    rText = sContent;
                    ConvertStringToXMLFormat( rText );
                }
            }
    }
}

/*****************************************************************************/
void XRMResMerge::Output( const rtl::OString& rOutput )
/*****************************************************************************/
{
    if (!rOutput.isEmpty())
        pOutputStream << rOutput.getStr();
}

/*****************************************************************************/
void XRMResMerge::EndOfText(
    const rtl::OString &rOpenTag,
    const rtl::OString &rCloseTag
)
/*****************************************************************************/
{

    Output( rCloseTag );
    if ( pMergeDataFile && pResData ) {
        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
        if ( pEntrys ) {
            rtl::OString sCur;
            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
                rtl::OString sContent;
                if (!sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")) &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, sCur, sal_True )) &&
                    ( sContent != "-" ) && !sContent.isEmpty())
                {
                    rtl::OString sText( sContent );
                    rtl::OString sAdditionalLine( "\n        " );
                    sAdditionalLine += rOpenTag;
                    rtl::OString sSearch = sLangAttribute;
                    sSearch += "=\"";
                    rtl::OString sReplace( sSearch );

                    sSearch += GetAttribute( rOpenTag, sLangAttribute );
                    sReplace += sCur;

                    sAdditionalLine = sAdditionalLine.replaceFirst(
                        sSearch, sReplace);

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
