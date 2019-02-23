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

#include <cfglex.hxx>
#include <common.hxx>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <memory>
#include <rtl/strbuf.hxx>

#include <helper.hxx>
#include <export.hxx>
#include <cfgmerge.hxx>
#include <tokens.h>

namespace {

namespace global {

OString inputPathname;
std::unique_ptr< CfgParser > parser;

}
}

extern "C" {

FILE * init(int argc, char ** argv) {

    common::HandledArgs aArgs;
    if ( !common::handleArguments(argc, argv, aArgs) )
    {
        common::writeUsage("cfgex","*.xcu");
        std::exit(EXIT_FAILURE);
    }
    global::inputPathname = aArgs.m_sInputFile;

    FILE * pFile = std::fopen(global::inputPathname.getStr(), "r");
    if (pFile == nullptr) {
        std::fprintf(
            stderr, "Error: Cannot open file \"%s\"\n",
            global::inputPathname.getStr() );
        std::exit(EXIT_FAILURE);
    }

    if (aArgs.m_bMergeMode) {
        global::parser.reset(
            new CfgMerge(
                aArgs.m_sMergeSrc, aArgs.m_sOutputFile,
                global::inputPathname, aArgs.m_sLanguage ));
    } else {
        global::parser.reset(
            new CfgExport(
                aArgs.m_sOutputFile, global::inputPathname ));
    }

    return pFile;
}

void workOnTokenSet(int nTyp, char * pTokenText) {
    global::parser->Execute( nTyp, pTokenText );
}

}


// class CfgStackData


CfgStackData* CfgStack::Push(const OString &rTag, const OString &rId)
{
    CfgStackData *pD = new CfgStackData( rTag, rId );
    maList.push_back( pD );
    return pD;
}


// class CfgStack


CfgStack::~CfgStack()
{
}

OString CfgStack::GetAccessPath( size_t nPos )
{
    OStringBuffer sReturn;
    for (size_t i = 0; i <= nPos; ++i)
    {
        if (i)
            sReturn.append('.');
        sReturn.append(maList[i]->GetIdentifier());
    }

    return sReturn.makeStringAndClear();
}

CfgStackData *CfgStack::GetStackData()
{
    if (!maList.empty())
        return maList[maList.size() - 1];
    else
        return nullptr;
}


// class CfgParser


CfgParser::CfgParser()
                : pStackData( nullptr ),
                bLocalize( false )
{
}

CfgParser::~CfgParser()
{
}

bool CfgParser::IsTokenClosed(const OString &rToken)
{
    return rToken[rToken.getLength() - 2] == '/';
}

void CfgParser::AddText(
    OString &rText,
    const OString &rIsoLang,
    const OString &rResTyp )
{
    rText = rText.replaceAll(OString('\n'), OString()).
        replaceAll(OString('\r'), OString()).
        replaceAll(OString('\t'), OString());
    pStackData->sResTyp = rResTyp;
    WorkOnText( rText, rIsoLang );
    pStackData->sText[ rIsoLang ] = rText;
}

#if defined _MSC_VER
#pragma warning(disable: 4702) // unreachable code, bug in MSVC2015, it thinks the std::exit is unreachable
#endif
void CfgParser::ExecuteAnalyzedToken( int nToken, char *pToken )
{
    OString sToken( pToken );

    if ( sToken == " " || sToken == "\t" )
        sLastWhitespace += sToken;

    OString sTokenName;
    OString sTokenId;

    bool bOutput = true;

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
            sTokenName = sToken.getToken(1, '<').getToken(0, '>').
                getToken(0, ' ');

            if ( !IsTokenClosed( sToken )) {
                OString sSearch;
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
                        bLocalize = true;
                    break;
                    case CFG_TOKEN_OORVALUE:
                        sSearch = "oor:value=";
                    break;
                    case CFG_TEXT_START: {
                        if ( sCurrentResTyp != sTokenName ) {
                            WorkOnResourceEnd();
                         }
                        sCurrentResTyp = sTokenName;

                        OString sTemp = sToken.copy( sToken.indexOf( "xml:lang=" ));
                        sCurrentIsoLang = sTemp.getToken(1, '"');

                        if ( sCurrentIsoLang == NO_TRANSLATE_ISO )
                            bLocalize = false;

                        pStackData->sTextTag = sToken;

                        sCurrentText = "";
                    }
                    break;
                }
                if ( !sSearch.isEmpty())
                {
                    OString sTemp = sToken.copy( sToken.indexOf( sSearch ));
                    sTokenId = sTemp.getToken(1, '"');
                }
                pStackData = aStack.Push( sTokenName, sTokenId );

                if ( sSearch == "cfg:name=" ) {
                    OString sTemp( sToken.toAsciiUpperCase() );
                    bLocalize = sTemp.indexOf("CFG:TYPE=\"STRING\"")>=0
                        && sTemp.indexOf( "CFG:LOCALIZED=\"TRUE\"" )>=0;
                }
            }
            else if ( sTokenName == "label" ) {
                if ( sCurrentResTyp != sTokenName ) {
                    WorkOnResourceEnd();
                }
                sCurrentResTyp = sTokenName;
            }
        }
        break;
        case CFG_CLOSETAG:
        {
            sTokenName = sToken.getToken(1, '/').getToken(0, '>').
                getToken(0, ' ');
            if ( aStack.GetStackData() && ( aStack.GetStackData()->GetTagType() == sTokenName ))
            {
                if (sCurrentText.isEmpty())
                    WorkOnResourceEnd();
                aStack.Pop();
                pStackData = aStack.GetStackData();
            }
            else
            {
                OString sError( "Misplaced close tag: " );
                sError += sToken;
                sError += " in file ";
                sError += global::inputPathname;
                yyerror(sError.getStr());
                std::exit(EXIT_FAILURE);
            }
        }
        break;

        case CFG_TEXTCHAR:
            sCurrentText += sToken;
            bOutput = false;
        break;

        case CFG_TOKEN_NO_TRANSLATE:
            bLocalize = false;
        break;
    }

    if ( !sCurrentText.isEmpty() && nToken != CFG_TEXTCHAR )
    {
        AddText( sCurrentText, sCurrentIsoLang, sCurrentResTyp );
        Output( sCurrentText );
        sCurrentText.clear();
        pStackData->sEndTextTag = sToken;
    }

    if ( bOutput )
        Output( sToken );

    if ( sToken != " " && sToken != "\t" )
        sLastWhitespace = "";
}

void CfgExport::Output(const OString&)
{
}

void CfgParser::Execute( int nToken, char * pToken )
{
    OString sToken( pToken );

    switch ( nToken ) {
        case CFG_TAG:
            if ( sToken.indexOf( "package-id=" ) != -1 ) {
                ExecuteAnalyzedToken( CFG_TOKEN_PACKAGE, pToken );
                return;
            } else if ( sToken.indexOf( "component-id=" ) != -1 ) {
                ExecuteAnalyzedToken( CFG_TOKEN_COMPONENT, pToken );
                return;
            } else if ( sToken.indexOf( "template-id=" ) != -1 ) {
                ExecuteAnalyzedToken( CFG_TOKEN_TEMPLATE, pToken );
                return;
            } else if ( sToken.indexOf( "cfg:name=" ) != -1 ) {
                ExecuteAnalyzedToken( CFG_TOKEN_OORNAME, pToken );
                return;
            } else if ( sToken.indexOf( "oor:name=" ) != -1 ) {
                ExecuteAnalyzedToken( CFG_TOKEN_OORNAME, pToken );
                return;
            } else if ( sToken.indexOf( "oor:value=" ) != -1 ) {
                ExecuteAnalyzedToken( CFG_TOKEN_OORVALUE, pToken );
                return;
            }
        break;
    }
    ExecuteAnalyzedToken( nToken, pToken );
}


// class CfgExport


CfgExport::CfgExport(
        const OString &rOutputFile,
        const OString &rFilePath )

                : sPath( rFilePath )
{
    pOutputStream.open( rOutputFile, PoOfstream::APP );
    if (!pOutputStream.isOpen())
    {
        std::cerr << "ERROR: Unable to open output file: " << rOutputFile << "\n";
        std::exit(EXIT_FAILURE);
    }
}

CfgExport::~CfgExport()
{
    pOutputStream.close();
}


void CfgExport::WorkOnResourceEnd()
{
    if ( bLocalize ) {
    if ( !pStackData->sText["en-US"].isEmpty() )
        {
            OString sXComment = pStackData->sText[OString("x-comment")];
            OString sLocalId = pStackData->sIdentifier;
            OString sGroupId;
            if ( aStack.size() == 1 ) {
                sGroupId = sLocalId;
                sLocalId = "";
            }
            else {
                sGroupId = aStack.GetAccessPath( aStack.size() - 2 );
            }


            OString sText = pStackData->sText[ "en-US" ];
            sText = helper::UnQuotHTML( sText );

            common::writePoEntry(
                "Cfgex", pOutputStream, sPath, pStackData->sResTyp,
                sGroupId, sLocalId, sXComment, sText);
        }
    }
}

void CfgExport::WorkOnText(
    OString &rText,
    const OString &rIsoLang
)
{
    if( !rIsoLang.isEmpty() ) rText = helper::UnQuotHTML( rText );
}


// class CfgMerge


CfgMerge::CfgMerge(
    const OString &rMergeSource, const OString &rOutputFile,
    const OString &rFilename, const OString &rLanguage )
                : sFilename( rFilename ),
                bEnglish( false )
{
    pOutputStream.open(
        rOutputFile.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!pOutputStream.is_open())
    {
        std::cerr << "ERROR: Unable to open output file: " << rOutputFile << "\n";
        std::exit(EXIT_FAILURE);
    }

    if (!rMergeSource.isEmpty())
    {
        pMergeDataFile.reset(new MergeDataFile(
            rMergeSource, global::inputPathname, true ));
        if (rLanguage.equalsIgnoreAsciiCase("ALL") )
        {
            aLanguages = pMergeDataFile->GetLanguages();
        }
        else aLanguages.push_back(rLanguage);
    }
    else
        aLanguages.push_back(rLanguage);
}

CfgMerge::~CfgMerge()
{
    pOutputStream.close();
}

void CfgMerge::WorkOnText(OString &, const OString& rLangIndex)
{

    if ( pMergeDataFile && bLocalize ) {
        if ( !pResData ) {
            OString sLocalId = pStackData->sIdentifier;
            OString sGroupId;
            if ( aStack.size() == 1 ) {
                sGroupId = sLocalId;
                sLocalId.clear();
            }
            else {
                sGroupId = aStack.GetAccessPath( aStack.size() - 2 );
            }

            pResData.reset( new ResData( sGroupId, sFilename ) );
            pResData->sId = sLocalId;
            pResData->sResTyp = pStackData->sResTyp;
        }

        if (rLangIndex.equalsIgnoreAsciiCase("en-US"))
            bEnglish = true;
    }
}

void CfgMerge::Output(const OString& rOutput)
{
    pOutputStream << rOutput;
}

void CfgMerge::WorkOnResourceEnd()
{

    if ( pMergeDataFile && pResData && bLocalize && bEnglish ) {
        MergeEntrys *pEntrys = pMergeDataFile->GetMergeEntrysCaseSensitive( pResData.get() );
        if ( pEntrys ) {
            OString sCur;

            for( size_t i = 0; i < aLanguages.size(); ++i ){
                sCur = aLanguages[ i ];

                OString sContent;
                pEntrys->GetText( sContent, sCur, true );
                if (
                    ( !sCur.equalsIgnoreAsciiCase("en-US") ) && !sContent.isEmpty())
                {

                    OString sText = helper::QuotHTML( sContent);

                    OString sAdditionalLine( "\t" );

                    OString sTextTag = pStackData->sTextTag;
                    OString sTemp = sTextTag.copy( sTextTag.indexOf( "xml:lang=" ));

                    sal_Int32 n = 0;
                    OString sSearch = sTemp.getToken(0, '"', n);
                    sSearch += "\"";
                    sSearch += sTemp.getToken(0, '"', n);
                    sSearch += "\"";

                    OString sReplace = sTemp.getToken(0, '"');
                    sReplace += "\"";
                    sReplace += sCur;
                    sReplace += "\"";

                    sTextTag = sTextTag.replaceFirst(sSearch, sReplace);

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
    pResData.reset();
    bEnglish = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
