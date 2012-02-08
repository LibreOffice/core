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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "boost/scoped_ptr.hpp"
#include "osl/process.h"
#include "rtl/uri.hxx"

#include <comphelper/string.hxx>

#include "helper.hxx"
#include "export.hxx"
#include "cfgmerge.hxx"
#include "tokens.h"

using comphelper::string::getToken;

extern "C" { int yyerror(char const *); }

namespace {

namespace global {

bool mergeMode = false;
bool errorLog = true;
char const * prj = 0;
char const * prjRoot = 0;
char const * inputPathname = 0;
char const * outputPathname = 0;
char const * mergeSrc;
boost::scoped_ptr< CfgParser > parser;

}

void handleArguments(int argc, char ** argv) {
    enum State {
        STATE_NONE, STATE_INPUT, STATE_OUTPUT, STATE_PRJ, STATE_ROOT,
        STATE_MERGESRC, STATE_LANGUAGES };
    State state = STATE_NONE;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-i") == 0) {
            state = STATE_INPUT;
        } else if (std::strcmp(argv[i], "-o") == 0) {
            state = STATE_OUTPUT;
        } else if (std::strcmp(argv[i], "-p") == 0) {
            state = STATE_PRJ;
        } else if (std::strcmp(argv[i], "-r") == 0) {
            state = STATE_ROOT;
        } else if (std::strcmp(argv[i], "-m") == 0) {
            state = STATE_MERGESRC;
        } else if (std::strcmp(argv[i], "-e") == 0) {
            state = STATE_NONE;
            global::errorLog = false;
        } else if (std::strcmp(argv[i], "-l") == 0) {
            state = STATE_LANGUAGES;
        } else {
            switch (state) {
            default:
                global::inputPathname = 0; // no valid command line
                goto done;
            case STATE_INPUT:
                global::inputPathname = argv[i];
                break;
            case STATE_OUTPUT:
                global::outputPathname = argv[i];
                break;
            case STATE_PRJ:
                global::prj = argv[i];
                break;
            case STATE_ROOT:
                global::prjRoot = argv[i];
                break;
            case STATE_MERGESRC:
                global::mergeSrc = argv[i];
                global::mergeMode = true;
                break;
            case STATE_LANGUAGES:
                Export::sLanguages = argv[i];
                break;
            }
            state = STATE_NONE;
        }
    }
done:
    if (global::inputPathname == 0 || global::outputPathname == 0) {
        std::fprintf(
            stderr,
            ("Syntax: cfgex [-p Prj] [-r PrjRoot] -i FileIn -o FileOut"
             " [-m DataBase] [-e] [-l l1,l2,...]\n"
             " Prj:      Project\n"
             " PrjRoot:  Path to project root (../.. etc.)\n"
             " FileIn:   Source files (*.src)\n"
             " FileOut:  Destination file (*.*)\n"
             " DataBase: Mergedata (*.sdf)\n"
             " -e: Disable writing errorlog\n"
             " -l: Restrict the handled languages; l1, l2, ... are elements of"
             " (de, en-US, ...)\n"));
        std::exit(EXIT_FAILURE);
    }
    Export::InitLanguages();
}

}

extern "C" {

FILE * init(int argc, char ** argv) {
    handleArguments(argc, argv);

    FILE * pFile = std::fopen(global::inputPathname, "r");
    if (pFile == 0) {
        std::fprintf(
            stderr, "Error: Cannot open file \"%s\"\n",
            global::inputPathname);
        std::exit(EXIT_FAILURE);
    }

    // Skip UTF-8 BOM:
    unsigned char buf[3];
    if (std::fread(buf, 1, 3, pFile) != 3 ||
        buf[0] != 0xEF || buf[1] != 0xBB || buf[2] != 0xBF)
    {
        std::rewind(pFile);
    }

    if (global::mergeMode) {
        global::parser.reset(
            new CfgMerge(
                global::mergeSrc, global::outputPathname,
                global::inputPathname));
    } else {
        rtl::OUString cwd;
        if (osl_getProcessWorkingDir(&cwd.pData) != osl_Process_E_None) {
            std::fprintf(stderr, "Error: Cannot determine cwd\n");
            std::exit(EXIT_FAILURE);
        }
        rtl::OUString full;
        if (!rtl_convertStringToUString(
                &full.pData, global::inputPathname,
                rtl_str_getLength(global::inputPathname),
                osl_getThreadTextEncoding(),
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
        {
            std::fprintf(
                stderr, "Error: Cannot convert input pathname to UTF-16\n");
            std::exit(EXIT_FAILURE);
        }
        if (osl::FileBase::getAbsoluteFileURL(cwd, full, full)
            != osl::FileBase::E_None)
        {
            std::fprintf(
                stderr,
                "Error: Cannot convert input pathname to absolute URL\n");
            std::exit(EXIT_FAILURE);
        }
        if (global::prjRoot == 0) {
            std::fprintf(stderr, "Error: No project root argument\n");
            std::exit(EXIT_FAILURE);
        }
        rtl::OUString base;
        if (!rtl_convertStringToUString(
                &base.pData, global::prjRoot,
                rtl_str_getLength(global::prjRoot),
                osl_getThreadTextEncoding(),
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                 | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
        {
            std::fprintf(
                stderr, "Error: Cannot convert project root to UTF-16\n");
            std::exit(EXIT_FAILURE);
        }
        base = rtl::Uri::convertRelToAbs(full, base);
        if (full.getLength() <= base.getLength() || base.isEmpty()
            || base[base.getLength() - 1] != '/'
            || full[base.getLength() - 1] != '/')
        {
            std::fprintf(
                stderr, "Error: Cannot extract suffix from input pathname\n");
            std::exit(EXIT_FAILURE);
        }
        full = full.copy(base.getLength()).replace('/', '\\');
        rtl::OString suffix;
        if (!full.convertToString(
                &suffix, osl_getThreadTextEncoding(),
                (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                 | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
        {
            std::fprintf(
                stderr, "Error: Cannot convert suffix from UTF-16\n");
            std::exit(EXIT_FAILURE);
        }
        global::parser.reset(
            new CfgExport(global::outputPathname, global::prj, suffix));
    }

    return pFile;
}

void workOnTokenSet(int nTyp, char * pTokenText) {
    global::parser->Execute( nTyp, pTokenText );
}

}

//
// class CfgStackData
//

CfgStackData* CfgStack::Push(const rtl::OString &rTag, const rtl::OString &rId)
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

rtl::OString CfgStack::GetAccessPath( size_t nPos )
{
    rtl::OStringBuffer sReturn;
    for (size_t i = 0; i <= nPos; ++i)
    {
        if (i)
            sReturn.append('.');
        sReturn.append(maList[i]->GetIdentifier());
    }

    return sReturn.makeStringAndClear();
}

/*****************************************************************************/
CfgStackData *CfgStack::GetStackData()
/*****************************************************************************/
{
    if (!maList.empty())
        return maList[maList.size() - 1];
    else
        return 0;
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

CfgParser::~CfgParser()
{
}

sal_Bool CfgParser::IsTokenClosed(const rtl::OString &rToken)
{
    return rToken[rToken.getLength() - 2] == '/';
}

/*****************************************************************************/
void CfgParser::AddText(
    rtl::OString &rText,
    const rtl::OString &rIsoLang,
    const rtl::OString &rResTyp
)
/*****************************************************************************/
{
        sal_Int32 nTextLen = 0;
        while ( rText.getLength() != nTextLen ) {
            nTextLen = rText.getLength();
            rText = rText.replace( '\n', ' ' );
            rText = rText.replace( '\r', ' ' );
            rText = rText.replace( '\t', ' ' );
            while (helper::searchAndReplace(&rText, "  ", " ") != -1) {}
        }
        pStackData->sResTyp = rResTyp;
        WorkOnText( rText, rIsoLang );

        pStackData->sText[ rIsoLang ] = rText;
}

/*****************************************************************************/
int CfgParser::ExecuteAnalyzedToken( int nToken, char *pToken )
/*****************************************************************************/
{
    rtl::OString sToken( pToken );

    if ( sToken == " " || sToken == "\t" )
        sLastWhitespace += sToken;

    rtl::OString sTokenName;
    rtl::OString sTokenId;

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
            sTokenName = getToken(getToken(getToken(sToken, 1, '<'), 0, '>'), 0, ' ');

              if ( !IsTokenClosed( sToken )) {
                rtl::OString sSearch;
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
                            rtl::OString sCur;
                            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                                sCur = aLanguages[ n ];
                                pStackData->sText[ sCur ] = rtl::OString();
                            }
                         }
                        sCurrentResTyp = sTokenName;

                        rtl::OString sTemp = sToken.copy( sToken.indexOf( "xml:lang=" ));
                        sCurrentIsoLang = getToken(getToken(sTemp, 1, '\"'), 0, '\"');

                        if ( sCurrentIsoLang == NO_TRANSLATE_ISO )
                            bLocalize = sal_False;

                        pStackData->sTextTag = sToken;

                        sCurrentText = "";
                    }
                    break;
                }
                if ( !sSearch.isEmpty())
                {
                    rtl::OString sTemp = sToken.copy( sToken.indexOf( sSearch ));
                    sTokenId = getToken(getToken(sTemp, 1, '\"'), 0, '\"');
                }
                pStackData = aStack.Push( sTokenName, sTokenId );

                if ( sSearch == "cfg:name=" ) {
                    rtl::OString sTemp( sToken.toAsciiUpperCase() );
                    bLocalize = (( sTemp.indexOf( "CFG:TYPE=\"STRING\"" ) != -1 ) &&
                        ( sTemp.indexOf( "CFG:LOCALIZED=\"sal_True\"" ) != -1 ));
                }
            }
            else if ( sTokenName == "label" ) {
                if ( sCurrentResTyp != sTokenName ) {
                    WorkOnRessourceEnd();
                    rtl::OString sCur;
                    for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                        sCur = aLanguages[ n ];
                        pStackData->sText[ sCur ] = rtl::OString();
                    }
                }
                sCurrentResTyp = sTokenName;
            }
        }
        break;
        case CFG_CLOSETAG:
            sTokenName = getToken(getToken(getToken(sToken, 1, '/'), 0, '>'), 0, ' ');
            if ( aStack.GetStackData() && ( aStack.GetStackData()->GetTagType() == sTokenName ))
            {
                if (sCurrentText.isEmpty())
                    WorkOnRessourceEnd();
                aStack.Pop();
                pStackData = aStack.GetStackData();
            }
            else
            {
                rtl::OString sError( "Misplaced close tag: " );
                rtl::OString sInFile(" in file ");
                sError += sToken;
                sError += sInFile;
                sError += global::inputPathname;
                Error( sError );
                std::exit(EXIT_FAILURE);
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

    if ( !sCurrentText.isEmpty() && nToken != CFG_TEXTCHAR )
    {
        AddText( sCurrentText, sCurrentIsoLang, sCurrentResTyp );
        Output( sCurrentText );
        sCurrentText = rtl::OString();
        pStackData->sEndTextTag = sToken;
    }

    if ( bOutput )
        Output( sToken );

    if ( sToken != " " && sToken != "\t" )
        sLastWhitespace = "";

    return 1;
}

void CfgExport::Output(const rtl::OString&)
{
}

/*****************************************************************************/
int CfgParser::Execute( int nToken, char * pToken )
/*****************************************************************************/
{
    rtl::OString sToken( pToken );

    switch ( nToken ) {
        case CFG_TAG:
            if ( sToken.indexOf( "package-id=" ) != -1 )
                return ExecuteAnalyzedToken( CFG_TOKEN_PACKAGE, pToken );
            else if ( sToken.indexOf( "component-id=" ) != -1 )
                return ExecuteAnalyzedToken( CFG_TOKEN_COMPONENT, pToken );
            else if ( sToken.indexOf( "template-id=" ) != -1 )
                return ExecuteAnalyzedToken( CFG_TOKEN_TEMPLATE, pToken );
            else if ( sToken.indexOf( "cfg:name=" ) != -1 )
                return ExecuteAnalyzedToken( CFG_TOKEN_OORNAME, pToken );
            else if ( sToken.indexOf( "oor:name=" ) != -1 )
                return ExecuteAnalyzedToken( CFG_TOKEN_OORNAME, pToken );
            else if ( sToken.indexOf( "oor:value=" ) != -1 )
                return ExecuteAnalyzedToken( CFG_TOKEN_OORVALUE, pToken );
        break;
    }
    return ExecuteAnalyzedToken( nToken, pToken );
}

void CfgParser::Error(const rtl::OString& rError)
{
    yyerror(rError.getStr());
}

//
// class CfgOutputParser
//

CfgOutputParser::CfgOutputParser(const rtl::OString &rOutputFile)
{
    pOutputStream.open(
        rOutputFile.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!pOutputStream.is_open())
    {
        rtl::OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("ERROR: Unable to open output file: "));
        sError.append(rOutputFile);
        Error(sError.makeStringAndClear());
        std::exit(EXIT_FAILURE);
    }
}

/*****************************************************************************/
CfgOutputParser::~CfgOutputParser()
/*****************************************************************************/
{
    pOutputStream.close();
}

//
// class CfgExport
//

/*****************************************************************************/
CfgExport::CfgExport(
        const rtl::OString &rOutputFile,
        const rtl::OString &rProject,
        const rtl::OString &rFilePath
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
    if ( bLocalize ) {
    if ( pStackData->sText[rtl::OString(RTL_CONSTASCII_STRINGPARAM("en-US"))].getLength() )
        {
            rtl::OString sFallback = pStackData->sText[rtl::OString(RTL_CONSTASCII_STRINGPARAM("en-US"))];
            rtl::OString sLocalId = pStackData->sIdentifier;
            rtl::OString sGroupId;
            if ( aStack.size() == 1 ) {
                sGroupId = sLocalId;
                sLocalId = "";
            }
            else {
                sGroupId = aStack.GetAccessPath( aStack.size() - 2 );
            }

            rtl::OString sTimeStamp( Export::GetTimeStamp());

            for (size_t n = 0; n < aLanguages.size(); n++)
            {
                rtl::OString sCur = aLanguages[ n ];

                rtl::OString sText = pStackData->sText[ sCur ];
                if ( sText.isEmpty())
                    sText = sFallback;

                sText = Export::UnquoteHTML( sText );

                rtl::OString sOutput( sPrj ); sOutput += "\t";
                sOutput += sPath;
                sOutput += "\t0\t";
                sOutput += pStackData->sResTyp; sOutput += "\t";
                sOutput += sGroupId; sOutput += "\t";
                sOutput += sLocalId; sOutput += "\t\t\t0\t";
                sOutput += sCur;
                sOutput += "\t";

                sOutput += sText; sOutput += "\t\t\t\t";
                sOutput += sTimeStamp;

                pOutputStream << sOutput.getStr() << '\n';
            }
        }
    }
}

void CfgExport::WorkOnText(
    rtl::OString &rText,
    const rtl::OString &rIsoLang
)
{
    if( rIsoLang.getLength() ) rText = Export::UnquoteHTML( rText );
}


//
// class CfgMerge
//

CfgMerge::CfgMerge(
    const rtl::OString &rMergeSource, const rtl::OString &rOutputFile,
    const rtl::OString &rFilename)
                : CfgOutputParser( rOutputFile ),
                pMergeDataFile( NULL ),
                pResData( NULL ),
                sFilename( rFilename ),
                bEnglish( sal_False )
{
    if (rMergeSource.getLength())
    {
        pMergeDataFile = new MergeDataFile(
            rMergeSource, global::inputPathname, global::errorLog, true );
        if (Export::sLanguages.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("ALL")))
        {
            Export::SetLanguages( pMergeDataFile->GetLanguages() );
            aLanguages = pMergeDataFile->GetLanguages();
        }
        else aLanguages = Export::GetLanguages();
    }
    else
        aLanguages = Export::GetLanguages();
}

/*****************************************************************************/
CfgMerge::~CfgMerge()
/*****************************************************************************/
{
    delete pMergeDataFile;
    delete pResData;
}

void CfgMerge::WorkOnText(rtl::OString &rText, const rtl::OString& rLangIndex)
{

    if ( pMergeDataFile && bLocalize ) {
        if ( !pResData ) {
            rtl::OString sLocalId = pStackData->sIdentifier;
            rtl::OString sGroupId;
            if ( aStack.size() == 1 ) {
                sGroupId = sLocalId;
                sLocalId = rtl::OString();
            }
            else {
                sGroupId = aStack.GetAccessPath( aStack.size() - 2 );
            }

            rtl::OString sPlatform;

            pResData = new ResData( sPlatform, sGroupId , sFilename );
            pResData->sId = sLocalId;
            pResData->sResTyp = pStackData->sResTyp;
        }

        if (rLangIndex.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")))
            bEnglish = sal_True;

        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrysCaseSensitive( pResData );
        if ( pEntrys ) {
            rtl::OString sContent;
            pEntrys->GetText( sContent, STRING_TYP_TEXT, rLangIndex );

            if ( Export::isAllowed( rLangIndex ) &&
                ( sContent != "-" ) && !sContent.isEmpty())
            {
                rText = Export::QuoteHTML( rText );
            }
        }
    }
}

void CfgMerge::Output(const rtl::OString& rOutput)
{
    pOutputStream << rOutput.getStr();
}

/*****************************************************************************/
void CfgMerge::WorkOnRessourceEnd()
/*****************************************************************************/
{

    if ( pMergeDataFile && pResData && bLocalize && bEnglish ) {
        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrysCaseSensitive( pResData );
        if ( pEntrys ) {
            rtl::OString sCur;

            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];

                rtl::OString sContent;
                pEntrys->GetText( sContent, STRING_TYP_TEXT, sCur , sal_True );
                if (
                    ( !sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")) ) &&

                    ( sContent != "-" ) && !sContent.isEmpty())
                {

                    rtl::OString sText = Export::QuoteHTML( sContent);

                    rtl::OString sAdditionalLine( "\t" );

                    rtl::OString sTextTag = pStackData->sTextTag;
                    rtl::OString sTemp = sTextTag.copy( sTextTag.indexOf( "xml:lang=" ));

                    rtl::OString sSearch = getToken(sTemp, 0, '\"');
                    sSearch += "\"";
                    sSearch += getToken(sTemp, 1, '\"');
                    sSearch += "\"";

                    rtl::OString sReplace = getToken(sTemp, 0, '\"');
                    sReplace += "\"";
                    sReplace += sCur;
                    sReplace += "\"";

                    helper::searchAndReplace(&sTextTag, sSearch, sReplace);

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
