/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include <cstring>

#include <stdio.h>

#include "common.hxx"
#include "export.hxx"
#include "po.hxx"
#include "xrmmerge.hxx"
#include "tokens.h"
#include "helper.hxx"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

void yyerror( const char * );
void YYWarning( const char * );

// set of global variables
bool bMergeMode;
sal_Bool bDisplayName;
sal_Bool bExtensionDescription;
OString sLanguage;
OString sInputFileName;
OString sOutputFile;
OString sMergeSrc;
OString sLangAttribute;
OString sResourceType;
XRMResParser *pParser = NULL;

extern "C" {
// the whole interface to lexer is in this extern "C" section

/*****************************************************************************/
extern char *GetOutputFile( int argc, char* argv[])
/*****************************************************************************/
{
    bDisplayName = sal_False;
    bExtensionDescription = sal_False;

    common::HandledArgs aArgs;
    if ( common::handleArguments(argc, argv, aArgs) )
    {
        // command line is valid
        bMergeMode = aArgs.m_bMergeMode;
        sLanguage = aArgs.m_sLanguage;
        sInputFileName = aArgs.m_sInputFile;
        sOutputFile = aArgs.m_sOutputFile;
        sMergeSrc = aArgs.m_sMergeSrc;
        char *pReturn = new char[ sOutputFile.getLength() + 1 ];
        std::strcpy( pReturn, sOutputFile.getStr());
        return pReturn;
    }
    else
    {
        // command line is not valid
        common::writeUsage("xrmex","*.xrm/*.xml");
        return NULL;
    }
}

/*****************************************************************************/
int InitXrmExport( char*, char* pFilename)
/*****************************************************************************/
{
    // instanciate Export
    OString sFilename( pFilename );

    if ( bMergeMode )
        pParser = new XRMResMerge( sMergeSrc, sOutputFile, sFilename );
      else if (!sOutputFile.isEmpty()) {
        pParser = new XRMResExport( sOutputFile, sInputFileName );
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
    OString rToken( pToken );

    switch ( nToken ) {
        case XRM_TEXT_START:{
                OString sNewGID = GetAttribute( rToken, "id" );
                if ( sNewGID != sGID ) {
                    sGID = sNewGID;
                }
                bText = sal_True;
                sCurrentText = "";
                sCurrentOpenTag = rToken;
                Output( rToken );
            }
        break;

        case XRM_TEXT_END: {
                sCurrentCloseTag = rToken;
                sResourceType = OString ( "readmeitem" );
                sLangAttribute = OString ( "xml:lang" );
                WorkOnText( sCurrentOpenTag, sCurrentText );
                Output( sCurrentText );
                EndOfText( sCurrentOpenTag, sCurrentCloseTag );
                bText = sal_False;
                rToken = OString("");
                sCurrentText  = OString("");
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
                    sGID = OString("dispname");
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
                    sResourceType = OString ( "description" );
                    sLangAttribute = OString ( "lang" );
                    WorkOnText( sCurrentOpenTag, sCurrentText );
                    Output( sCurrentText );
                    EndOfText( sCurrentOpenTag, sCurrentCloseTag );
                    bText = sal_False;
                    rToken = OString("");
                    sCurrentText  = OString("");
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
                    sGID = OString("extdesc");
                    sResourceType = OString ( "description" );
                    sLangAttribute = OString ( "lang" );
                    sCurrentOpenTag = rToken;
                    sCurrentText  = OString("");
                    Output( rToken );
                    WorkOnDesc( sCurrentOpenTag, sCurrentText );
                    sCurrentCloseTag = rToken;
                    Output( sCurrentText );
                    rToken = OString("");
                    sCurrentText  = OString("");
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
OString XRMResParser::GetAttribute( const OString &rToken, const OString &rAttribute )
/*****************************************************************************/
{
    OString sTmp( rToken );
    sTmp = sTmp.replace('\t', ' ');

    OString sSearch( " " );
    sSearch += rAttribute;
    sSearch += "=";
    sal_Int32 nPos = sTmp.indexOf( sSearch );

    if ( nPos != -1 )
    {
        sTmp = sTmp.copy( nPos );
        OString sId = sTmp.getToken(1, '"');
        return sId;
    }
    return OString();
}


/*****************************************************************************/
void XRMResParser::Error( const OString &rError )
/*****************************************************************************/
{
    yyerror(( char * ) rError.getStr());
}

//
// class XMLResExport
//

/*****************************************************************************/
XRMResExport::XRMResExport(
    const OString &rOutputFile, const OString &rFilePath )
/*****************************************************************************/
                : XRMResParser(),
                pResData( NULL ),
                sPath( rFilePath )
{
    pOutputStream.open( rOutputFile, PoOfstream::APP );
    if (!pOutputStream.isOpen())
    {
        OString sError( "Unable to open output file: " );
        sError += rOutputFile;
        Error( sError );
    }
}

/*****************************************************************************/
XRMResExport::~XRMResExport()
/*****************************************************************************/
{
    pOutputStream.close();
    delete pResData;
}

void XRMResExport::Output( const OString& ) {}

/*****************************************************************************/
void XRMResExport::WorkOnDesc(
    const OString &rOpenTag,
    OString &rText
)
/*****************************************************************************/
{
    OString sDescFileName(
        sInputFileName.replaceAll("description.xml", OString()));
    sDescFileName += GetAttribute( rOpenTag, "xlink:href" );
    ifstream file (sDescFileName.getStr(), ios::in|ios::binary|ios::ate);
    if (file.is_open()) {
        int size = static_cast<int>(file.tellg());
        char* memblock = new char [size+1];
        file.seekg (0, ios::beg);
        file.read (memblock, size);
        file.close();
        memblock[size] = '\0';
        rText = OString(memblock);
        delete[] memblock;
     }
    WorkOnText( rOpenTag, rText );
    EndOfText( rOpenTag, rOpenTag );
}

//*****************************************************************************/
void XRMResExport::WorkOnText(
    const OString &rOpenTag,
    OString &rText
)
/*****************************************************************************/
{
    OString sLang( GetAttribute( rOpenTag, sLangAttribute ));

    if ( !pResData )
    {
        pResData = new ResData( GetGID() );
    }
    pResData->sText[sLang] = rText;
}

/*****************************************************************************/
void XRMResExport::EndOfText(
    const OString &,
    const OString &
)
/*****************************************************************************/
{
    if ( pResData )
    {
        OString sAct = pResData->sText["en-US"];

        if( !sAct.isEmpty() )
            common::writePoEntry(
                "Xrmex", pOutputStream, sPath, sResourceType,
                pResData->sGId, OString(), OString(), sAct );
    }
    delete pResData;
    pResData = NULL;
}

//
// class XRMResMerge
//

/*****************************************************************************/
XRMResMerge::XRMResMerge(
    const OString &rMergeSource, const OString &rOutputFile,
    const OString &rFilename )
/*****************************************************************************/
                : XRMResParser(),
                pMergeDataFile( NULL ),
                sFilename( rFilename ) ,
                pResData( NULL )
{
    if (!rMergeSource.isEmpty())
        pMergeDataFile = new MergeDataFile(
            rMergeSource, sInputFileName, false);
    if( sLanguage.equalsIgnoreAsciiCase("ALL") )
    {
        aLanguages = pMergeDataFile->GetLanguages();
    }
    else
        aLanguages.push_back( sLanguage );
    pOutputStream.open(
        rOutputFile.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!pOutputStream.is_open()) {
        OString sError( "Unable to open output file: " );
        sError += rOutputFile;
        Error( sError );
    }
}

/*****************************************************************************/
XRMResMerge::~XRMResMerge()
/*****************************************************************************/
{
    pOutputStream.close();
    delete pMergeDataFile;
    delete pResData;
}

/*****************************************************************************/
void XRMResMerge::WorkOnDesc(
    const OString &rOpenTag,
    OString &rText
)
/*****************************************************************************/
{
    WorkOnText( rOpenTag, rText);
    if ( pMergeDataFile && pResData ) {
        MergeEntrys *pEntrys = pMergeDataFile->GetMergeEntrys( pResData );
        if ( pEntrys ) {
            OString sCur;
            OString sDescFilename = GetAttribute ( rOpenTag, "xlink:href" );
            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
                OString sContent;
                if ( !sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US"))  &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, sCur, sal_True )) &&
                    !sContent.isEmpty())
                {
                    OString sText( sContent );
                    OString sAdditionalLine( "\n        " );
                    sAdditionalLine += rOpenTag;
                    OString sSearch = sLangAttribute;
                    sSearch += "=\"";
                    OString sReplace( sSearch );

                    sSearch += GetAttribute( rOpenTag, sLangAttribute );
                    sReplace += sCur;
                    sAdditionalLine = sAdditionalLine.replaceFirst(
                        sSearch, sReplace);

                    sSearch = OString("xlink:href=\"");
                    sReplace = sSearch;

                    OString sLocDescFilename = sDescFilename;
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
                    OString sOutputDescFile(
                        sOutputFile.copy(0, i + 1) + sLocDescFilename);
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
    const OString &rOpenTag,
    OString &rText
)
/*****************************************************************************/
{
    OString sLang( GetAttribute( rOpenTag, sLangAttribute ));

    if ( pMergeDataFile ) {
        if ( !pResData ) {
            pResData = new ResData( GetGID(), sFilename );
            pResData->sResTyp = sResourceType;
        }

        MergeEntrys *pEntrys = pMergeDataFile->GetMergeEntrys( pResData );
            if ( pEntrys ) {
                OString sContent;
                if ( !sLang.equalsIgnoreAsciiCase("en-US") &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, sLang )) && !sContent.isEmpty() &&
                    helper::isWellFormedXML( sContent ))
                {
                    rText = sContent;
                }
            }
    }
}

/*****************************************************************************/
void XRMResMerge::Output( const OString& rOutput )
/*****************************************************************************/
{
    if (!rOutput.isEmpty())
        pOutputStream << rOutput.getStr();
}

/*****************************************************************************/
void XRMResMerge::EndOfText(
    const OString &rOpenTag,
    const OString &rCloseTag
)
/*****************************************************************************/
{

    Output( rCloseTag );
    if ( pMergeDataFile && pResData ) {
        MergeEntrys *pEntrys = pMergeDataFile->GetMergeEntrys( pResData );
        if ( pEntrys ) {
            OString sCur;
            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
                OString sContent;
                if (!sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")) &&
                    ( pEntrys->GetText(
                        sContent, STRING_TYP_TEXT, sCur, sal_True )) &&
                    !sContent.isEmpty() &&
                    helper::isWellFormedXML( sContent ))
                {
                    OString sText( sContent );
                    OString sAdditionalLine( "\n        " );
                    sAdditionalLine += rOpenTag;
                    OString sSearch = sLangAttribute;
                    sSearch += "=\"";
                    OString sReplace( sSearch );

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
