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

#include <sal/config.h>

#include <cstring>

#include <stdio.h>

#include <common.hxx>
#include <export.hxx>
#include <po.hxx>
#include <xrmlex.hxx>
#include <xrmmerge.hxx>
#include <tokens.h>
#include <helper.hxx>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

using namespace std;

// set of global variables
static bool bMergeMode;
static bool bDisplayName;
static bool bExtensionDescription;
static OString sLanguage;
static OString sInputFileName;
static OString sOutputFile;
static OString sMergeSrc;
static OString sLangAttribute;
static OString sResourceType;
static XRMResParser *pParser = nullptr;

extern "C" {
// the whole interface to lexer is in this extern "C" section

extern bool GetOutputFile( int argc, char* argv[])
{
    bDisplayName = false;
    bExtensionDescription = false;

    common::HandledArgs aArgs;
    if ( common::handleArguments(argc, argv, aArgs) )
    {
        bMergeMode = aArgs.m_bMergeMode;
        sLanguage = aArgs.m_sLanguage;
        sInputFileName = aArgs.m_sInputFile;
        sOutputFile = aArgs.m_sOutputFile;
        sMergeSrc = aArgs.m_sMergeSrc;
        return true;
    }
    else
    {
        // command line is not valid
        common::writeUsage("xrmex","*.xrm/*.xml");
        return false;
    }
}

int InitXrmExport( const char* pFilename)
{
    // instantiate Export
    OString sFilename( pFilename );

    if ( bMergeMode )
        pParser = new XRMResMerge( sMergeSrc, sOutputFile, sFilename );
    else if (!sOutputFile.isEmpty())
        pParser = new XRMResExport( sOutputFile, sInputFileName );

    return 1;
}

int EndXrmExport()
{
    delete pParser;
    return 1;
}
extern const char* getFilename()
{
    return sInputFileName.getStr();
}

extern FILE *GetXrmFile()
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
    return nullptr;
}

int WorkOnTokenSet( int nTyp, char *pTokenText )
{
    //printf("Typ = %d , text = '%s'\n",nTyp , pTokenText );
    pParser->Execute( nTyp, pTokenText );

    return 1;
}

int SetError()
{
    pParser->SetError();
    return 1;
}
}

extern "C" {

int GetError()
{
    return pParser->GetError();
}
}


// class XRMResParser


XRMResParser::XRMResParser()
    : bError( false ),
    bText( false )
{
}

XRMResParser::~XRMResParser()
{
}

void XRMResParser::Execute( int nToken, char * pToken )
{
    OString rToken( pToken );

    switch ( nToken ) {
        case XRM_TEXT_START:{
                OString sNewGID = GetAttribute( rToken, "id" );
                if ( sNewGID != sGID ) {
                    sGID = sNewGID;
                }
                bText = true;
                sCurrentText = OString();
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
                bText = false;
                rToken = OString();
                sCurrentText  = OString();
        }
        break;

        case DESC_DISPLAY_NAME_START:{
                bDisplayName = true;
            }
        break;

        case DESC_DISPLAY_NAME_END:{
                bDisplayName = false;
            }
        break;

        case DESC_TEXT_START:{
                if (bDisplayName) {
                    sGID = OString("dispname");
                    bText = true;
                    sCurrentText = OString();
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
                    bText = false;
                    rToken = OString();
                    sCurrentText  = OString();
                }
        }
        break;

        case DESC_EXTENSION_DESCRIPTION_START: {
                bExtensionDescription = true;
            }
        break;

        case DESC_EXTENSION_DESCRIPTION_END: {
                bExtensionDescription = false;
            }
        break;

        case DESC_EXTENSION_DESCRIPTION_SRC: {
                if (bExtensionDescription) {
                    sGID = OString("extdesc");
                    sResourceType = OString ( "description" );
                    sLangAttribute = OString ( "lang" );
                    sCurrentOpenTag = rToken;
                    sCurrentText  = OString();
                    Output( rToken );
                    WorkOnDesc( sCurrentOpenTag, sCurrentText );
                    sCurrentCloseTag = rToken;
                    Output( sCurrentText );
                    rToken = OString();
                    sCurrentText  = OString();
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
}

OString XRMResParser::GetAttribute( const OString &rToken, const OString &rAttribute )
{
    const OString sSearch{ " " + rAttribute + "=" };
    OString sTmp{ rToken.replace('\t', ' ') };
    sal_Int32 nPos = sTmp.indexOf( sSearch );

    if ( nPos<0 )
        return OString();

    return sTmp.getToken(1, '"', nPos);
}


void XRMResParser::Error( const OString &rError )
{
    yyerror(rError.getStr());
}


// class XMLResExport


XRMResExport::XRMResExport(
    const OString &rOutputFile, const OString &rFilePath )
                : XRMResParser(),
                sPath( rFilePath )
{
    pOutputStream.open( rOutputFile, PoOfstream::APP );
    if (!pOutputStream.isOpen())
    {
        Error( "Unable to open output file: " + rOutputFile );
    }
}

XRMResExport::~XRMResExport()
{
    pOutputStream.close();
}

void XRMResExport::Output( const OString& ) {}

void XRMResExport::WorkOnDesc(
    const OString &rOpenTag,
    OString &rText )
{
    const OString sDescFileName{ sInputFileName.replaceAll("description.xml", OString())
        + GetAttribute( rOpenTag, "xlink:href" ) };
    ifstream file (sDescFileName.getStr(), ios::in|ios::binary|ios::ate);
    if (file.is_open()) {
        int size = static_cast<int>(file.tellg());
        std::unique_ptr<char[]> memblock(new char [size+1]);
        file.seekg (0, ios::beg);
        file.read (memblock.get(), size);
        file.close();
        memblock[size] = '\0';
        rText = OString(memblock.get());
     }
    WorkOnText( rOpenTag, rText );
    EndOfText( rOpenTag, rOpenTag );
}

void XRMResExport::WorkOnText(
    const OString &rOpenTag,
    OString &rText )
{
    OString sLang( GetAttribute( rOpenTag, sLangAttribute ));

    if ( !pResData )
    {
        pResData.reset( new ResData( GetGID() ) );
    }
    pResData->sText[sLang] = rText;
}

void XRMResExport::EndOfText(
    const OString &,
    const OString & )
{
    if ( pResData )
    {
        OString sAct = pResData->sText["en-US"];

        if( !sAct.isEmpty() )
            common::writePoEntry(
                "Xrmex", pOutputStream, sPath, sResourceType,
                pResData->sGId, OString(), OString(), sAct );
    }
    pResData.reset();
}


// class XRMResMerge


XRMResMerge::XRMResMerge(
    const OString &rMergeSource, const OString &rOutputFile,
    const OString &rFilename )
                : XRMResParser(),
                sFilename( rFilename )
{
    if (!rMergeSource.isEmpty() && sLanguage.equalsIgnoreAsciiCase("ALL"))
    {
        pMergeDataFile.reset(new MergeDataFile(
            rMergeSource, sInputFileName, false));
        aLanguages = pMergeDataFile->GetLanguages();
    }
    else
        aLanguages.push_back( sLanguage );
    pOutputStream.open(
        rOutputFile.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!pOutputStream.is_open()) {
        Error( "Unable to open output file: " + rOutputFile );
    }
}

XRMResMerge::~XRMResMerge()
{
    pOutputStream.close();
}

void XRMResMerge::WorkOnDesc(
    const OString &rOpenTag,
    OString &rText )
{
    WorkOnText( rOpenTag, rText);
    if ( pMergeDataFile && pResData ) {
        MergeEntrys *pEntrys = pMergeDataFile->GetMergeEntrys( pResData.get() );
        if ( pEntrys ) {
            OString sCur;
            OString sDescFilename = GetAttribute ( rOpenTag, "xlink:href" );
            for( size_t n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
                OString sText;
                if ( !sCur.equalsIgnoreAsciiCase("en-US")  &&
                    ( pEntrys->GetText( sText, sCur, true )) &&
                    !sText.isEmpty())
                {
                    OString sAdditionalLine{ "\n        "  + rOpenTag };
                    OString sSearch{ sLangAttribute + "=\"" };
                    OString sReplace( sSearch );

                    sSearch += GetAttribute( rOpenTag, sLangAttribute );
                    sReplace += sCur;
                    sAdditionalLine = sAdditionalLine.replaceFirst(
                        sSearch, sReplace);

                    sSearch = OString("xlink:href=\"");
                    sReplace = sSearch;

                    const OString sLocDescFilename = sDescFilename.replaceFirst( "en-US", sCur);

                    sSearch += sDescFilename;
                    sReplace += sLocDescFilename;
                    sAdditionalLine = sAdditionalLine.replaceFirst(
                        sSearch, sReplace);

                    Output( sAdditionalLine );

                    sal_Int32 i = sOutputFile.lastIndexOf('/');
                    if (i == -1) {
                        std::cerr
                            << "Error: output file " << sOutputFile
                            << " does not contain any /\n";
                        throw false; //TODO
                    }
                    OString sOutputDescFile(
                        sOutputFile.copy(0, i + 1) + sLocDescFilename);
                    ofstream file(sOutputDescFile.getStr());
                    if (file.is_open()) {
                        file << sText;
                        file.close();
                    } else {
                        std::cerr
                            << "Error: cannot write "
                            << sOutputDescFile << '\n';
                        throw false; //TODO
                    }
                }
            }
        }
    }
    pResData.reset();
}

void XRMResMerge::WorkOnText(
    const OString &,
    OString & )
{
    if ( pMergeDataFile && !pResData ) {
        pResData.reset( new ResData( GetGID(), sFilename ) );
        pResData->sResTyp = sResourceType;
    }
}

void XRMResMerge::Output( const OString& rOutput )
{
    if (!rOutput.isEmpty())
        pOutputStream << rOutput;
}

void XRMResMerge::EndOfText(
    const OString &rOpenTag,
    const OString &rCloseTag )
{

    Output( rCloseTag );
    if ( pMergeDataFile && pResData ) {
        MergeEntrys *pEntrys = pMergeDataFile->GetMergeEntrys( pResData.get() );
        if ( pEntrys ) {
            OString sCur;
            for( size_t n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
                OString sContent;
                if (!sCur.equalsIgnoreAsciiCase("en-US") &&
                    ( pEntrys->GetText( sContent, sCur, true )) &&
                    !sContent.isEmpty() &&
                    helper::isWellFormedXML( sContent ))
                {
                    const OString& sText( sContent );
                    OString sAdditionalLine{ "\n        " + rOpenTag };
                    OString sSearch{ sLangAttribute + "=\"" };
                    OString sReplace( sSearch );

                    sSearch += GetAttribute( rOpenTag, sLangAttribute );
                    sReplace += sCur;

                    sAdditionalLine = sAdditionalLine.replaceFirst(
                        sSearch, sReplace) + sText + rCloseTag;

                    Output( sAdditionalLine );
                }
            }
        }
    }
    pResData.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
