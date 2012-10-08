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

#include <cstddef>
#include <cstring>

#include "boost/scoped_ptr.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "common.hxx"
#include "export.hxx"
#include "helper.hxx"
#include "tokens.h"
#include <iostream>
#include <rtl/strbuf.hxx>

void yyerror( const char * );
void YYWarning( const char * );

namespace {

rtl::OString sActFileName; //TODO
MergeDataFile * pMergeDataFile = 0; //TODO

namespace global {

OString prj;
OString prjRoot;
OString inputPathname;
boost::scoped_ptr< Export > exporter;

}
}

extern "C" {

FILE * init(int argc, char ** argv) {

    HandledArgs aArgs;
    if ( !Export::handleArguments(argc, argv, aArgs) )
    {
        Export::writeUsage("transex3","src/hrc");
        std::exit(EXIT_FAILURE);
    }
    Export::InitLanguages();
    global::prj =  aArgs.m_sPrj;
    global::prjRoot =  aArgs.m_sPrjRoot;
    global::inputPathname =  aArgs.m_sInputFile;

    FILE * pFile = std::fopen(global::inputPathname.getStr(), "r");
    if (pFile == 0) {
        std::fprintf(
            stderr, "Error: Cannot open file \"%s\"\n",
            global::inputPathname.getStr());
        std::exit(EXIT_FAILURE);
    }

    if (aArgs.m_bMergeMode) {
        global::exporter.reset(
            new Export(aArgs.m_sMergeSrc.getStr(), aArgs.m_sOutputFile.getStr()));
    } else {
        sActFileName =
            common::pathnameToken(
                global::inputPathname.getStr(), global::prjRoot.getStr());
        global::exporter.reset(new Export(aArgs.m_sOutputFile.getStr()));
    }

    global::exporter->Init();

    return pFile;
}

int Parse( int nTyp, const char *pTokenText ){
    global::exporter->Execute( nTyp , pTokenText );
    return 1;
}
void Close(){
    global::exporter->pParseQueue->Close();
    global::exporter.reset();
        // avoid nontrivial Export dtor being executed during exit
}

int WorkOnTokenSet( int nTyp, char *pTokenText )
{

    global::exporter->pParseQueue->Push( QueueEntry( nTyp , rtl::OString(pTokenText) ) );
    return 1;
}

} // extern

extern "C" {
/*****************************************************************************/
int SetError()
/*****************************************************************************/
{
    // set error at global instance of class Export
    global::exporter->SetError();
    return 1;
}
}

extern "C" {
/*****************************************************************************/
int GetError()
/*****************************************************************************/
{
    // get error at global instance of class Export
    if (global::exporter->GetError())
        return 1;
    return sal_False;
}
}

//
// class ResData
//

/*****************************************************************************/
sal_Bool ResData::SetId( const rtl::OString& rId, sal_uInt16 nLevel )
/*****************************************************************************/
{
    if ( nLevel > nIdLevel )
    {
        nIdLevel = nLevel;
        sId = rId;

        if ( bChild && bChildWithText )
        {
            rtl::OString sError(RTL_CONSTASCII_STRINGPARAM("ResId after child definition"));
            yyerror(sError.getStr());
            SetError();
        }

        if ( sId.getLength() > 255 )
        {
            YYWarning( "LocalId > 255 chars, truncating..." );
            sId = sId.copy(0, 255).trim();
        }

        return sal_True;
    }

    return sal_False;
}

//
// class Export
//

Export::Export(const rtl::OString &rOutput)
                :
                pWordTransformer( NULL ),
                bDefine( sal_False ),
                bNextMustBeDefineEOL( sal_False ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bSkipFile( false ),
                sProject( global::prj ),
                sRoot( global::prjRoot ),
                bMergeMode( false ),
                bError( sal_False ),
                bReadOver( sal_False ),
                bDontWriteOutput( sal_False ),
                sFilename( global::inputPathname )
{
    pParseQueue = new ParserQueue( *this );

    if( !isInitialized ) InitLanguages();
    // used when export is enabled

    // open output stream
    aOutput.open(rOutput.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!aOutput.is_open()) {
        fprintf(stderr, "ERROR : Can't open file %s\n", rOutput.getStr());
        exit ( -1 );
    }
}

Export::Export(const rtl::OString &rMergeSource, const rtl::OString &rOutput)
                :
                pWordTransformer( NULL ),
                bDefine( sal_False ),
                bNextMustBeDefineEOL( sal_False ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bSkipFile( false ),
                sProject( global::prj ),
                sRoot( global::prjRoot ),
                bMergeMode( sal_True ),
                sMergeSrc( rMergeSource ),
                bError( sal_False ),
                bReadOver( sal_False ),
                bDontWriteOutput( sal_False ),
                sFilename( global::inputPathname )
{
    pParseQueue = new ParserQueue( *this );
    if( !isInitialized ) InitLanguages( bMergeMode );
    // used when merge is enabled

    // open output stream
    aOutput.open(rOutput.getStr(), std::ios_base::out | std::ios_base::trunc);
}

/*****************************************************************************/
void Export::Init()
/*****************************************************************************/
{
    // resets the internal status, used before parseing another file
    sActPForm = "";
    bDefine = sal_False;
    bNextMustBeDefineEOL = sal_False;
    nLevel = 0;
    nList = LIST_NON;
    m_sListLang = rtl::OString();
    nListIndex = 0;
    for ( size_t i = 0, n = aResStack.size(); i < n;  ++i )
        delete aResStack[ i ];
    aResStack.clear();
}

/*****************************************************************************/
Export::~Export()
/*****************************************************************************/
{
    if( pParseQueue )
        delete pParseQueue;
    aOutput.close();
    for ( size_t i = 0, n = aResStack.size(); i < n;  ++i )
        delete aResStack[ i ];
    aResStack.clear();

    if ( bMergeMode ) {
        if ( !pMergeDataFile )
            pMergeDataFile = new MergeDataFile(sMergeSrc, global::inputPathname, false);

        delete pMergeDataFile;
    }
}

/*****************************************************************************/
int Export::Execute( int nToken, const char * pToken )
/*****************************************************************************/
{

    rtl::OString sToken( pToken );
    rtl::OString sOrig( sToken );
    sal_Bool bWriteToMerged = bMergeMode;

    if ( nToken == CONDITION )
    {
        rtl::OString sTestToken(pToken);
        sTestToken = sTestToken.replaceAll("\t", rtl::OString()).
            replaceAll(" ", rtl::OString());
        if (( !bReadOver ) && ( sTestToken.indexOf("#ifndef__RSC_PARSER") == 0 ))
            bReadOver = sal_True;
        else if (( bReadOver ) && ( sTestToken.indexOf("#endif") == 0 ))
            bReadOver = sal_False;
    }
    if ((( nToken < FILTER_LEVEL ) || ( bReadOver )) &&
        (!(( bNextMustBeDefineEOL ) && ( sOrig == "\n" )))) {
        // this tokens are not mandatory for parsing, so ignore them ...
        if ( bMergeMode )
            WriteToMerged( sOrig , false ); // ... ore whrite them directly to dest.
        return 0;
    }

    ResData *pResData = NULL;
    if ( nLevel ) {
        // res. exists at cur. level
        pResData = ( (nLevel-1) < aResStack.size() ) ? aResStack[ nLevel-1 ] : NULL;
    }
    else if (( nToken != RESOURCE ) &&
            ( nToken != RESOURCEEXPR ) &&
            ( nToken != SMALRESOURCE ) &&
            ( nToken != LEVELUP ) &&
            ( nToken != NORMDEFINE ) &&
            ( nToken != RSCDEFINE ) &&
            ( nToken != CONDITION ) &&
            ( nToken != PRAGMA ))
    {
        // no res. exists at cur. level so return
        if ( bMergeMode )
            WriteToMerged( sOrig , false );
        return 0;
    }
    // #define NO_LOCALIZE_EXPORT
    if( bSkipFile ){
        if ( bMergeMode ) {
            WriteToMerged( sOrig , false );
        }
        return 1;
    }


    if ( bDefine ) {
        if (( nToken != EMPTYLINE ) && ( nToken != LEVELDOWN ) && ( nToken != LEVELUP )) {
            // cur. res. defined in macro
            if ( bNextMustBeDefineEOL ) {
                if ( nToken != RSCDEFINELEND ) {
                    // end of macro found, so destroy res.
                    bDefine = sal_False;
                    if ( bMergeMode ) {
                        MergeRest( pResData );
                    }
                    bNextMustBeDefineEOL = sal_False;
                    Execute( LEVELDOWN, "" );
                }
                else {
                    // next line also in macro definition
                    bNextMustBeDefineEOL = sal_False;
                    if ( bMergeMode )
                        WriteToMerged( sOrig , false );
                    return 1;
                }
            }
        }
    }

    sal_Bool bExecuteDown = sal_False;
    if ( nToken != LEVELDOWN ) {
        sal_uInt16 nOpen = 0;
        sal_uInt16 nClose = 0;
        sal_Bool bReadOver1 = sal_False;
        sal_uInt16 i = 0;
        for ( i = 0; i < sToken.getLength(); i++ ) {
            if ( sToken[i] == '"' )
                bReadOver1 = !bReadOver1;
            if ( !bReadOver1 && ( sToken[i] == '{' ))
                nOpen++;
        }

        bReadOver1 = sal_False;
        for ( i = 0; i < sToken.getLength(); i++ ) {
            if ( sToken[i] == '"' )
                bReadOver1 = !bReadOver1;
            if ( !bReadOver1 && ( sToken[i] == '}' ))
                nClose++;
        }

        if ( nOpen < nClose )
            bExecuteDown = sal_True;
    }
    switch ( nToken ) {

        case NORMDEFINE:
            sToken = sToken.replace('\r', ' ').replace('\t', ' ');
            for (;;) {
                sal_Int32 n = 0;
                sToken = sToken.replaceFirst("  ", " ", &n);
                if (n == -1) {
                    break;
                }
            }
            if( sToken.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("#define NO_LOCALIZE_EXPORT")) ){
                            bSkipFile = true;
                            return 0;
                        }
                        if ( bMergeMode )
                          WriteToMerged( sOrig , false );

                        return 0;


        case RSCDEFINE:
            bDefine = sal_True; // res. defined in macro

        case RESOURCE:
        case RESOURCEEXPR: {
            bDontWriteOutput = sal_False;
            if ( nToken != RSCDEFINE )
                bNextMustBeDefineEOL = sal_False;
            // this is the beginning of a new res.
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = sal_True;
            }

            // create new instance for this res. and fill mandatory fields

            pResData = new ResData( sActPForm, FullId() , sFilename );
            aResStack.push_back( pResData );
            sToken = sToken.replaceAll("\n", rtl::OString()).
                replaceAll("\r", rtl::OString()).
                replaceAll("{", rtl::OString()).replace('\t', ' ');
            sToken = sToken.trim();
            rtl::OString sTLower = sToken.getToken(0, ' ').toAsciiLowerCase();
            pResData->sResTyp = sTLower;
            rtl::OString sId( sToken.copy( pResData->sResTyp.getLength() + 1 ));
            rtl::OString sCondition;
            if ( sId.indexOf( '#' ) != -1 )
            {
                // between ResTyp, Id and paranthes is a precomp. condition
                sCondition = "#";
                sal_Int32 n = 0;
                sId = sId.getToken(0, '#', n);
                sCondition += sId.getToken(0, '#', n);
            }
            sId = sId.getToken(0, '/');
            CleanValue( sId );
            sId = sId.replaceAll("\t", rtl::OString());
            pResData->SetId( sId, ID_LEVEL_IDENTIFIER );
            if (!sCondition.isEmpty())
            {
                Execute( CONDITION, "");  // execute the precomp. condition
            }
        }
        break;
        case SMALRESOURCE: {
            bDontWriteOutput = sal_False;
            // this is the beginning of a new res.
            bNextMustBeDefineEOL = sal_False;
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = sal_True;
            }

            // create new instance for this res. and fill mandatory fields

            pResData = new ResData( sActPForm, FullId() , sFilename );
            aResStack.push_back( pResData );
            sToken = sToken.replaceAll("\n", rtl::OString()).
                replaceAll("\r", rtl::OString()).
                replaceAll("{", rtl::OString()).
                replaceAll("\t", rtl::OString()).
                replaceAll(" ", rtl::OString()).
                replaceAll("\\", rtl::OString()).toAsciiLowerCase();
            pResData->sResTyp = sToken;
        }
        break;
        case LEVELUP: {
            // push
            if ( nList )
                nListLevel++;
            if ( nList )
                break;

            bDontWriteOutput = sal_False;
            rtl::OString sLowerTyp;
            if ( pResData )
                sLowerTyp = "unknown";
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = sal_True;
            }

            ResData *pNewData = new ResData( sActPForm, FullId() , sFilename );
            pNewData->sResTyp = sLowerTyp;
            aResStack.push_back( pNewData );
        }
        break;
        case LEVELDOWN: {
            // pop
            if ( !nList  ) {
                bDontWriteOutput = sal_False;
                if ( nLevel ) {
                    if ( bDefine && (nLevel == 1 )) {
                        bDefine = sal_False;
                        bNextMustBeDefineEOL = sal_False;
                    }
                    WriteData( pResData );
                    ResStack::iterator it = aResStack.begin();
                    ::std::advance( it, nLevel-1 );
                    delete *it;
                    aResStack.erase( it );
                    nLevel--;
                }
            }
            else {
                if ( bDefine )
                    bNextMustBeDefineEOL = sal_True;
                if ( !nListLevel ) {
                    if ( bMergeMode )
                        MergeRest( pResData, MERGE_MODE_LIST );
                    nList = LIST_NON;
                }
                else
                    nListLevel--;
            }
        }
        break;
        case ASSIGNMENT:
        {
            bDontWriteOutput = sal_False;
            // interpret different types of assignement
            sal_Int32 n = 0;
            rtl::OString sKey = sToken.getToken(0, '=', n).
                replaceAll(" ", rtl::OString()).
                replaceAll("\t", rtl::OString());
            rtl::OString sValue = sToken.getToken(0, '=', n);
            CleanValue( sValue );
            sKey = sKey.toAsciiUpperCase();
            if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("IDENTIFIER")))
            {
                rtl::OString sId(
                    sValue.replaceAll("\t", rtl::OString()).
                    replaceAll(" ", rtl::OString()));
                pResData->SetId(sId, ID_LEVEL_IDENTIFIER);
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("HELPID")))
            {
                pResData->sHelpId = sValue;
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("STRINGLIST")))
            {
                pResData->bList = sal_True;
                nList = LIST_STRING;
                m_sListLang = SOURCE_LANGUAGE;
                nListIndex = 0;
                nListLevel = 0;
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("FILTERLIST")))
            {
                pResData->bList = sal_True;
                nList = LIST_FILTER;
                m_sListLang = SOURCE_LANGUAGE;
                nListIndex = 0;
                nListLevel = 0;
            }
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UIENTRIES")))
            {
                pResData->bList = sal_True;
                nList = LIST_UIENTRIES;
                m_sListLang = SOURCE_LANGUAGE;
                nListIndex = 0;
                nListLevel = 0;
            }
            if (sToken.indexOf( '{' ) != -1
                && (helper::countOccurrences(sToken, '{')
                    > helper::countOccurrences(sToken, '}')))
            {
                Parse( LEVELUP, "" );
            }
         }
        break;
        case UIENTRIES:
        case LISTASSIGNMENT:
        {
            bDontWriteOutput = sal_False;
            rtl::OString sTmpToken(
                sToken.replaceAll(" ", rtl::OString()).toAsciiLowerCase());
            sal_Int32 nPos = sTmpToken.indexOf("[en-us]=");
            if (nPos != -1) {
                rtl::OString sKey(
                    sTmpToken.copy(0 , nPos).replaceAll(" ", rtl::OString()).
                    replaceAll("\t", rtl::OString()));
                rtl::OString sValue = sToken.getToken(1, '=');
                CleanValue( sValue );
                sKey = sKey.toAsciiUpperCase();
                if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("STRINGLIST")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_STRING;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("FILTERLIST")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_FILTER;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                // PairedList
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("PAIREDLIST")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_PAIRED;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("ITEMLIST")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_ITEM;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UIENTRIES")))
                {
                    pResData->bList = sal_True;
                    nList = LIST_UIENTRIES;
                    m_sListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
            }
            else
            {
                // new res. is a String- or FilterList
                sal_Int32 n = 0;
                rtl::OString sKey(
                    sToken.getToken(0, '[', n).replaceAll(" ", rtl::OString()).
                    replaceAll("\t", rtl::OString()).toAsciiUpperCase());
                if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("STRINGLIST")))
                    nList = LIST_STRING;
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("FILTERLIST")))
                    nList = LIST_FILTER;
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("PAIREDLIST")))
                    nList = LIST_PAIRED;                // abcd
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("ITEMLIST")))
                    nList = LIST_ITEM;
                else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UIENTRIES")))
                    nList = LIST_UIENTRIES;
                if ( nList ) {
                    rtl::OString sLang = sToken.getToken(0, ']', n);
                    CleanValue( sLang );
                    m_sListLang = sLang;
                    nListIndex = 0;
                    nListLevel = 0;
                }
            }
        }
        break;
        case TEXT:
        case _LISTTEXT:
        case LISTTEXT: {
            // this is an entry for a String- or FilterList
            if ( nList ) {
                SetChildWithText();
                sal_Int32 n = 0;
                rtl::OString sEntry(sToken.getToken(1, '"', n));
                if ( helper::countOccurrences(sToken, '"') > 2 )
                    sEntry += "\"";
                if ( sEntry == "\\\"" )
                    sEntry = "\"";
                InsertListEntry( sEntry, sOrig );
                if ( bMergeMode && ( sEntry != "\"" ))
                {
                    PrepareTextToMerge( sOrig, nList, m_sListLang, pResData );
                }
            }
        }
        break;
        case LONGTEXTLINE:
        case TEXTLINE:
            bDontWriteOutput = sal_False;
            if ( nLevel )
            {
                CutComment( sToken );

                // this is a text line!!!
                rtl::OString t(sToken.getToken(0, '='));
                rtl::OString sKey(
                    t.getToken(0, '[').replaceAll(" ", rtl::OString()).
                    replaceAll("\t", rtl::OString()));
                rtl::OString sText( GetText( sToken, nToken ));
                rtl::OString sLang;
                if ( sToken.getToken(0, '=').indexOf('[') != -1 )
                {
                    sLang = sToken.getToken(0, '=').getToken(1, '[').
                        getToken(0, ']');
                    CleanValue( sLang );
                }
                rtl::OString sLangIndex = sLang;
                rtl::OString sOrigKey = sKey;
                if ( !sText.isEmpty() && !sLang.isEmpty() )
                {
                    sKey = sKey.toAsciiUpperCase();
                    if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("TEXT")) ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("MESSAGE"))  ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("CUSTOMUNITTEXT"))  ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("SLOTNAME"))  ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UINAME")))
                    {
                        SetChildWithText();
                        if ( Export::isSourceLanguage( sLangIndex ) )
                            pResData->SetId( sText, ID_LEVEL_TEXT );

                        pResData->bText = sal_True;
                        pResData->sTextTyp = sOrigKey;
                        if ( bMergeMode ) {
                            PrepareTextToMerge( sOrig, STRING_TYP_TEXT, sLangIndex, pResData );
                        }
                        else
                        {
                            if (!pResData->sText[ sLangIndex ].isEmpty())
                            {
                                rtl::OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
                                sError.append(sLangIndex);
                                sError.append(RTL_CONSTASCII_STRINGPARAM("defined twice"));
                                yyerror(sError.getStr());
                            }
                            pResData->sText[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "HELPTEXT" ) {
                        SetChildWithText();
                        pResData->bHelpText = sal_True;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_HELPTEXT, sLangIndex, pResData );
                        else
                        {
                            if (!pResData->sHelpText[ sLangIndex ].isEmpty())
                            {
                                rtl::OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
                                sError.append(sLangIndex);
                                sError.append(" defined twice");
                                YYWarning(sError.getStr());
                            }
                            pResData->sHelpText[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "QUICKHELPTEXT" ) {
                        SetChildWithText();
                        pResData->bQuickHelpText = sal_True;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_QUICKHELPTEXT, sLangIndex, pResData );
                        else
                        {
                            if (!pResData->sQuickHelpText[ sLangIndex ].isEmpty())
                            {
                                rtl::OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
                                sError.append(sLangIndex);
                                sError.append(RTL_CONSTASCII_STRINGPARAM(" defined twice"));
                                YYWarning(sError.getStr());
                            }
                            pResData->sQuickHelpText[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "TITLE" ) {
                        SetChildWithText();
                        pResData->bTitle = sal_True;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_TITLE, sLangIndex, pResData );
                        else
                        {
                            if ( !pResData->sTitle[ sLangIndex ].isEmpty())
                            {
                                rtl::OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
                                sError.append(sLangIndex);
                                sError.append(RTL_CONSTASCII_STRINGPARAM(" defined twice"));
                                YYWarning(sError.getStr());
                            }
                            pResData->sTitle[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "ACCESSPATH" ) {
                        pResData->SetId( sText, ID_LEVEL_ACCESSPATH );
                    }
                    else if ( sKey == "FIELDNAME" ) {
                        pResData->SetId( sText, ID_LEVEL_FIELDNAME );
                    }
                }
            }
        break;
        case NEWTEXTINRES: {
            bDontWriteOutput = sal_True;
        }
        break;
        case APPFONTMAPPING:
        {
            bDontWriteOutput = sal_False;
            // this is a AppfontMapping, so look if its a definition
            // of field size
            sal_Int32 n = 0;
            rtl::OString sKey(
                sToken.getToken(0, '=', n).replaceAll(" ", rtl::OString()).
                replaceAll("\t", rtl::OString()));
            rtl::OString sMapping = sToken.getToken(0, '=', n);
            sMapping = sMapping.getToken(1, '(');
            sMapping = sMapping.getToken(0, ')').
                replaceAll(rtl::OString(' '), rtl::OString()).
                replaceAll(rtl::OString('\t'), rtl::OString()).
                toAsciiUpperCase();
            if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("SIZE"))) {
                pResData->nWidth = sMapping.getToken(0, ',').toInt32();
            } else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("POSSIZE"))) {
                pResData->nWidth = sMapping.getToken(2, ',').toInt32();
            }
        }
        break;
        case RSCDEFINELEND:
            bDontWriteOutput = sal_False;
        break;
        case CONDITION: {
            bDontWriteOutput = sal_False;
            sToken = sToken.replace('\r', ' ').replace('\t', ' ');
            for (;;) {
                sal_Int32 n = 0;
                sToken = sToken.replaceFirst("  ", " ", &n);
                if (n == -1) {
                    break;
                }
            }
            sal_Int32 n = 0;
            rtl::OString sCondition(sToken.getToken(0, ' ', n));
            if ( sCondition == "#ifndef" ) {
                sActPForm = "!defined ";
                sActPForm += sToken.getToken(0, ' ', n);
            }
            else if ( sCondition == "#ifdef" ) {
                sActPForm = "defined ";
                sActPForm += sToken.getToken(0, ' ', n);
            }
            else if ( sCondition == "#if" ) {
                sActPForm = sToken.copy( 4 ).replaceAll("||", "\\or");
            }
            else if ( sCondition == "#elif" ) {
                sActPForm = sToken.copy( 6 ).replaceAll("||", "\\or");
            }
            else if ( sCondition == "#else" ) {
                sActPForm = sCondition;
            }
            else if ( sCondition == "#endif" ) {
                sActPForm = "";
            }
            else break;
            if ( nLevel ) {
                WriteData( pResData, sal_True );
                pResData->sPForm = sActPForm;
            }
        }
        break;
        case EMPTYLINE : {
            bDontWriteOutput = sal_False;
            if ( bDefine ) {
                bNextMustBeDefineEOL = sal_False;
                bDefine = sal_False;
                while ( nLevel )
                    Parse( LEVELDOWN, "" );
            }
        }
        break;
        case PRAGMA : {
            bDontWriteOutput = sal_False;
            fprintf(stderr, "ERROR: archaic PRAGMA %s\n", sToken.getStr());
            exit(-1);
        }
        break;
        case TEXTREFID : {
            bDontWriteOutput = sal_True;
        }
        }
    if ( bWriteToMerged ) {
        // the current token must be written to dest. without merging

        if( bDefine && sOrig.getLength() > 2 ){
            for( sal_uInt16 n = 0 ; n < sOrig.getLength() ; n++ ){
                if( sOrig[n] == '\n' && sOrig[n-1] != '\\'){
                    sOrig = sOrig.replaceAt(n++, 0, "\\");
                }
            }
        }
        WriteToMerged( sOrig , false);
    }

    if ( bExecuteDown ) {
        Parse( LEVELDOWN, "" );
    }

    return 1;
}

/*****************************************************************************/
void Export::CutComment( rtl::OString &rText )
/*****************************************************************************/
{
    if (rText.indexOf("//") != -1) {
        rtl::OString sWork(rText.replaceAll("\\\"", "XX"));
        bool bInner = false;
        for (sal_Int32 i = 0; i < sWork.getLength() - 1; ++i) {
            if (sWork[i] == '"') {
                bInner = !bInner;
            } else if (sWork[i] == '/' && !bInner && sWork[i + 1] == '/' ) {
                rText = rText.copy(0, i);
                break;
            }
        }
    }
}

/*****************************************************************************/
sal_Bool Export::WriteData( ResData *pResData, sal_Bool bCreateNew )
/*****************************************************************************/
{
    if ( bMergeMode ) {
        MergeRest( pResData );
        return sal_True;
    }

       // mandatory to export: en-US

     if (( !pResData->sText[ SOURCE_LANGUAGE ].isEmpty())
        ||
        (  !pResData->sHelpText[ SOURCE_LANGUAGE ].isEmpty())
        ||
        (  !pResData->sQuickHelpText[ SOURCE_LANGUAGE ].isEmpty())
         ||
        (  !pResData->sTitle[ SOURCE_LANGUAGE ].isEmpty()))

       {
        rtl::OString sGID = pResData->sGId;
        rtl::OString sLID;
        if (sGID.isEmpty())
            sGID = pResData->sId;
        else
            sLID = pResData->sId;

        rtl::OString sXText;
        rtl::OString sXHText;
        rtl::OString sXQHText;
        rtl::OString sXTitle;

        sXText = pResData->sText[ SOURCE_LANGUAGE ];
        if (!pResData->sText[ X_COMMENT ].isEmpty())
            sXHText = pResData->sText[ X_COMMENT ];
        else
            sXHText = pResData->sHelpText[ SOURCE_LANGUAGE ];
        sXQHText = pResData->sQuickHelpText[ SOURCE_LANGUAGE ];
        sXTitle = pResData->sTitle[ SOURCE_LANGUAGE ];

        if (sXText.isEmpty())
            sXText = "-";

        rtl::OString sOutput( sProject ); sOutput += "\t";
        if ( !sRoot.isEmpty())
            sOutput += sActFileName;
        sOutput += "\t0\t";
        sOutput += pResData->sResTyp; sOutput += "\t";
        sOutput += sGID; sOutput += "\t";
        sOutput += sLID; sOutput += "\t";
        sOutput += pResData->sHelpId; sOutput   += "\t";
        sOutput += pResData->sPForm; sOutput    += "\t";
        sOutput += rtl::OString::valueOf(pResData->nWidth); sOutput += "\t";
        sOutput += "en-US"; sOutput += "\t";


        sOutput += sXText; sOutput  += "\t";
        sOutput += sXHText; sOutput += "\t";
        sOutput += sXQHText; sOutput+= "\t";
        sOutput += sXTitle; sOutput += "\t";

        aOutput << sOutput.getStr() << '\n';

        if ( bCreateNew ) {
            pResData->sText[ SOURCE_LANGUAGE ]         = "";
            pResData->sHelpText[ SOURCE_LANGUAGE ]     = "";
            pResData->sQuickHelpText[ SOURCE_LANGUAGE ]= "";
            pResData->sTitle[ SOURCE_LANGUAGE ]        = "";
        }
    }
    if ( pResData->pStringList ) {
        rtl::OString sList( "stringlist" );
        WriteExportList( pResData, pResData->pStringList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pStringList = 0;
    }
    if ( pResData->pFilterList ) {
        rtl::OString sList( "filterlist" );
        WriteExportList( pResData, pResData->pFilterList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pFilterList = 0;
    }
    if ( pResData->pItemList ) {
        rtl::OString sList( "itemlist" );
        WriteExportList( pResData, pResData->pItemList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pItemList = 0;
    }
    if ( pResData->pPairedList ) {
        rtl::OString sList( "pairedlist" );
        WriteExportList( pResData, pResData->pPairedList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pItemList = 0;
    }
    if ( pResData->pUIEntries ) {
        rtl::OString sList( "uientries" );
        WriteExportList( pResData, pResData->pUIEntries, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pUIEntries = 0;
    }
    return sal_True;
}

rtl::OString Export::GetPairedListID(const rtl::OString& rText)
{
// < "STRING" ; IDENTIFIER ; > ;
    return rText.getToken(1, ';').toAsciiUpperCase().replace('\t', ' ').trim();
}

rtl::OString Export::GetPairedListString(const rtl::OString& rText)
{
// < "STRING" ; IDENTIFIER ; > ;
    rtl::OString sString(rText.getToken(0, ';').replace('\t', ' '));
    sString = sString.trim();
    rtl::OString s1(sString.copy(sString.indexOf('"') + 1));
    sString = s1.copy(0, s1.lastIndexOf('"'));
    return sString.trim();
}

rtl::OString Export::StripList(const rtl::OString & rText)
{
    rtl::OString s1 = rText.copy( rText.indexOf('\"') + 1);
    return s1.copy( 0 , s1.lastIndexOf('\"'));
}

sal_Bool Export::WriteExportList(ResData *pResData, ExportList *pExportList,
    const rtl::OString &rTyp, sal_Bool bCreateNew)
{
    rtl::OString sGID(pResData->sGId);
    if (sGID.isEmpty())
        sGID = pResData->sId;
    else {
        sGID += ".";
        sGID += pResData->sId;
        while (sGID.getLength() != 0 && sGID[sGID.getLength() - 1] == '.') {
            sGID = sGID.copy(0, sGID.getLength() - 1);
        }
    }

    rtl::OString sCur;
    for ( size_t i = 0; pExportList != NULL && i < pExportList->size(); i++ )
    {
        ExportListEntry *pEntry = (*pExportList)[  i ];

        rtl::OString sLID(rtl::OString::valueOf(static_cast<sal_Int64>(i + 1)));
        for (unsigned int n = 0; n < aLanguages.size(); ++n)
        {
            sCur = aLanguages[ n ];
            if (!(*pEntry)[ SOURCE_LANGUAGE ].isEmpty())
            {
                rtl::OString sText((*pEntry)[ SOURCE_LANGUAGE ] );

                // Strip PairList Line String
                if (rTyp.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("pairedlist")))
                {
                    sLID = GetPairedListID( sText );
                    if (!(*pEntry)[ sCur ].isEmpty())
                        sText = (*pEntry)[ sCur ];
                    sText = GetPairedListString( sText );
                }
                else
                {
                    sText = StripList( (*pEntry)[ sCur ] );
                    if( sText == "\\\"" )
                        sText = "\"";
                }

                rtl::OStringBuffer sOutput(sProject);
                sOutput.append('\t');
                if ( !sRoot.isEmpty())
                    sOutput.append(sActFileName);
                sOutput.append("\t0\t");
                sOutput.append(rTyp).append('\t');
                sOutput.append(sGID).append('\t');
                sOutput.append(sLID).append("\t\t");
                sOutput.append(pResData->sPForm).append("\t0\t");
                sOutput.append(sCur).append('\t');

                sOutput.append(sText).append("\t\t\t\t");

                aOutput << sOutput.makeStringAndClear().getStr() << '\n';
            }
        }
        if ( bCreateNew )
            delete [] pEntry;
    }
    if ( bCreateNew )
        delete pExportList;

    return sal_True;
}

rtl::OString Export::FullId()
{
    rtl::OStringBuffer sFull;
    if ( nLevel > 1 )
    {
        sFull.append(aResStack[ 0 ]->sId);
        for ( size_t i = 1; i < nLevel - 1; ++i )
        {
            rtl::OString sToAdd = aResStack[ i ]->sId;
            if (!sToAdd.isEmpty())
                sFull.append('.').append(sToAdd);
        }
    }
    if (sFull.getLength() > 255)
    {
        rtl::OString sError(RTL_CONSTASCII_STRINGPARAM("GroupId > 255 chars"));
        printf("GroupID = %s\n", sFull.getStr());
        yyerror(sError.getStr());
    }

    return sFull.makeStringAndClear();
}

void Export::InsertListEntry(const rtl::OString &rText, const rtl::OString &rLine)
{
    ResData *pResData = ( nLevel-1 < aResStack.size() ) ? aResStack[ nLevel-1 ] : NULL;

    ExportList *pList = NULL;
    if ( nList == LIST_STRING ) {
        pList = pResData->pStringList;
        if ( !pList ) {
            pResData->pStringList = new ExportList();
            pList = pResData->pStringList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_FILTER ) {
        pList = pResData->pFilterList;
        if ( !pList ) {
            pResData->pFilterList = new ExportList();
            pList = pResData->pFilterList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_ITEM ) {
        pList = pResData->pItemList;
        if ( !pList ) {
            pResData->pItemList = new ExportList();
            pList = pResData->pItemList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_PAIRED ) {
        pList = pResData->pPairedList;
        if ( !pList ) {
            pResData->pPairedList = new ExportList();
            pList = pResData->pPairedList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_UIENTRIES ) {
        pList = pResData->pUIEntries;
        if ( !pList ) {
            pResData->pUIEntries = new ExportList();
            pList = pResData->pUIEntries;
            nListIndex = 0;
        }
    }
    else
        return;

    if ( nListIndex + 1 > pList->size())
    {
        ExportListEntry *pNew = new ExportListEntry();
        (*pNew)[LIST_REFID] = rtl::OString::valueOf(static_cast<sal_Int32>(REFID_NONE));
        pList->push_back(pNew);
    }
    ExportListEntry *pCurEntry = (*pList)[ nListIndex ];

    // For paired list use the line to set proper lid
    if( nList == LIST_PAIRED ){
        (*pCurEntry)[ m_sListLang ] = rLine;
    }else
        (*pCurEntry)[ m_sListLang ] = rText;

    if ( Export::isSourceLanguage( m_sListLang ) ) {
        (*pCurEntry)[ SOURCE_LANGUAGE ] = rLine;

        pList->NewSourceLanguageListEntry();
    }

    nListIndex++;
}

/*****************************************************************************/
void Export::CleanValue( rtl::OString &rValue )
/*****************************************************************************/
{
    while ( !rValue.isEmpty()) {
        if (( rValue[0] == ' ' ) || ( rValue[0] == '\t' ))
            rValue = rValue.copy( 1 );
        else
            break;
    }

    if ( !rValue.isEmpty()) {
        for ( sal_Int32 i = rValue.getLength() - 1; i > 0; i-- ) {
            if (( rValue[i] == ' ' ) || ( rValue[i] == '\t' ) ||
                ( rValue[i] == '\n' ) || ( rValue[i] == ';' ) ||
                ( rValue[i] == '{' ) || ( rValue[i] == '\\' ) ||
                ( rValue[i] == '\r' ))
                rValue = rValue.copy(0, i);
            else
                break;
        }
    }
}

#define TXT_STATE_NON   0x000
#define TXT_STATE_TEXT  0x001
#define TXT_STATE_MACRO 0x002

rtl::OString Export::GetText(const rtl::OString &rSource, int nToken)
{
    rtl::OString sReturn;
    switch ( nToken )
    {
        case TEXTLINE:
        case LONGTEXTLINE:
        {
            rtl::OString sTmp(rSource.copy(rSource.indexOf('=')));
            CleanValue( sTmp );
            sTmp = sTmp.replaceAll("\n", rtl::OString()).
                replaceAll("\r", rtl::OString()).
                replaceAll("\\\\\"", "-=<[BSlashBSlashHKom]>=-\"").
                replaceAll("\\\"", "-=<[Hochkomma]>=-").
                replaceAll("\\", "-=<[0x7F]>=-").
                replaceAll("\\0x7F", "-=<[0x7F]>=-");

            sal_uInt16 nState = TXT_STATE_TEXT;
            for (sal_Int32 i = 1; i <= helper::countOccurrences(sTmp, '"'); ++i)
            {
                rtl::OString sToken(sTmp.getToken(i, '"'));
                if (!sToken.isEmpty()) {
                    if ( nState == TXT_STATE_TEXT ) {
                        sReturn += sToken;
                        nState = TXT_STATE_MACRO;
                    }
                    else {
                        sToken = sToken.replace('\t', ' ');
                        for (;;) {
                            sal_Int32 n = 0;
                            sToken = sToken.replaceFirst("  ", " ", &n);
                            if (n == -1) {
                                break;
                            }
                        }
                        sToken = sToken.trim();
                        if (!sToken.isEmpty()) {
                            sReturn += "\\\" ";
                            sReturn += sToken;
                            sReturn += " \\\"";
                        }
                        nState = TXT_STATE_TEXT;
                    }
                }
            }

            sReturn = sReturn.replaceAll("-=<[0x7F]>=-", "").
                replaceAll("-=<[Hochkomma]>=-", "\"").
                replaceAll("-=<[BSlashBSlashHKom]>=-", "\\\\").
                replaceAll("\\\\", "-=<[BSlashBSlash]>=-").
                replaceAll("-=<[BSlashBSlash]>=-", "\\");
        }
        break;
    }
    return sReturn;
}

void Export::WriteToMerged(const rtl::OString &rText , bool bSDFContent)
{
    rtl::OString sText(rText);
    for (;;) {
        sal_Int32 n = 0;
        sText = sText.replaceFirst(" \n", "\n", &n);
        if (n == -1) {
            break;
        }
    }
    if (pParseQueue->bNextIsM && bSDFContent && sText.getLength() > 2) {
        for (sal_Int32 n = 0; n < sText.getLength(); ++n) {
            if (sText[n] == '\n' && sText[n - 1] != '\\') {
                sText = sText.replaceAt(n++, 0, "\\");
            }
        }
    } else if (pParseQueue->bLastWasM && sText.getLength() > 2) {
        for (sal_Int32 n = 0; n < sText.getLength(); ++n) {
            if (sText[n] == '\n' && sText[n - 1] != '\\') {
                sText = sText.replaceAt(n++, 0, "\\");
            }
            if (sText[n] == '\n') {
                pParseQueue->bMflag = true;
            }
        }
    } else if (pParseQueue->bCurrentIsM && bSDFContent && sText.getLength() > 2)
    {
        for (sal_Int32 n = 0; n < sText.getLength(); ++n) {
            if (sText[n] == '\n' && sText[n - 1] != '\\') {
                sText = sText.replaceAt(n++, 0, "\\");
                pParseQueue->bMflag = true;
            }
        }
    } else if (pParseQueue->bMflag) {
        for (sal_Int32 n = 1; n < sText.getLength(); ++n) {
            if (sText[n] == '\n' && sText[n - 1] != '\\') {
                sText = sText.replaceAt(n++, 0, "\\");
            }
        }
    } for (sal_Int32 i = 0; i < sText.getLength(); ++i) {
        if (sText[i] == '\n') {
            aOutput << '\n';
        } else {
            char cChar = sText[i];
            aOutput << cChar;
        }
    }
}

/*****************************************************************************/
void Export::ConvertMergeContent( rtl::OString &rText )
/*****************************************************************************/
{
    sal_Bool bNoOpen = ( rText.indexOf( "\\\"" ) != 0 );
    sal_Bool bNoClose = !rText.endsWithL(RTL_CONSTASCII_STRINGPARAM("\\\""));
    rtl::OStringBuffer sNew;
    for ( sal_Int32 i = 0; i < rText.getLength(); i++ )
    {
        rtl::OString sChar( rText[i]);
        if (sChar.equalsL(RTL_CONSTASCII_STRINGPARAM("\\")))
        {
            if (( i + 1 ) < rText.getLength())
            {
                sal_Char cNext = rText[i + 1];
                if ( cNext == '\"' )
                {
                    sChar = rtl::OString('\"');
                    i++;
                }
                else if ( cNext == 'n' )
                {
                    sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\n"));
                    i++;
                }
                else if ( cNext == 't' )
                {
                    sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\t"));
                    i++;
                }
                else if ( cNext == '\'' )
                {
                    sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\\'"));
                    i++;
                }
                else
                {
                    sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\\\"));
                }
            }
            else
            {
                sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\\\"));
            }
        }
        else if (sChar.equalsL(RTL_CONSTASCII_STRINGPARAM("\"")))
        {
            sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\\""));
        }
        else if (sChar.equalsL(RTL_CONSTASCII_STRINGPARAM("")))
        {
            sChar = rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\0x7F"));
        }
        sNew.append(sChar);
    }

    rText = sNew.makeStringAndClear();

    if ( bNoOpen ) {
        rtl::OString sTmp( rText );
        rText = "\"";
        rText += sTmp;
    }
    if ( bNoClose )
        rText += "\"";
}

sal_Bool Export::PrepareTextToMerge(rtl::OString &rText, sal_uInt16 nTyp,
    rtl::OString &rLangIndex, ResData *pResData)
{
    // position to merge in:
    sal_Int32 nStart = 0;
    sal_Int32 nEnd = 0;
    rtl::OString sOldId = pResData->sId;
    rtl::OString sOldGId = pResData->sGId;
    rtl::OString sOldTyp = pResData->sResTyp;

    rtl::OString sOrigText( rText );

    switch ( nTyp ) {
        case LIST_STRING :
        case LIST_UIENTRIES :
        case LIST_FILTER :
        case LIST_PAIRED:
        case LIST_ITEM :
        {
            ExportList *pList = NULL;
            switch ( nTyp ) {
                case LIST_STRING : {
                    pResData->sResTyp = "stringlist";
                    pList = pResData->pStringList;
                }
                break;
                case LIST_UIENTRIES : {
                    pResData->sResTyp = "uientries";
                    pList = pResData->pUIEntries;
                }
                break;
                case LIST_FILTER : {
                    pResData->sResTyp = "filterlist";
                    pList = pResData->pFilterList;
                }
                break;
                case LIST_ITEM : {
                    pResData->sResTyp = "itemlist";
                    pList = pResData->pItemList;
                }
                break;
                case LIST_PAIRED : {
                    pResData->sResTyp = "pairedlist";
                    pList = pResData->pPairedList;
                }
                break;

            }
            if (pList)
            {
                ExportListEntry *pCurEntry = (*pList)[ nListIndex - 1 ];
                if ( pCurEntry )
                    rText = (*pCurEntry)[ SOURCE_LANGUAGE ];
            }

            nStart = rText.indexOf( '"' );
            if ( nStart == -1 ) {
                rText = sOrigText;
                return sal_False;
            }

            sal_Bool bFound = sal_False;
            for ( nEnd = nStart + 1; nEnd < rText.getLength() && !bFound; nEnd++ ) {
                if ( rText[nEnd] == '\"' )
                    bFound = sal_True;
            }
            if ( !bFound ) {
                rText = sOrigText;
                return sal_False;
            }

            nEnd --;
            sLastListLine = rText;
            if (( sLastListLine.indexOf( '>' ) != -1 ) &&
                ( sLastListLine.indexOf( '<' ) == -1 ))
            {
                rtl::OString sTmp = sLastListLine;
                sLastListLine = "<";
                sLastListLine += sTmp;
            }
            if ( pResData->sResTyp.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("pairedlist")) )
            {
               pResData->sId = GetPairedListID( sLastListLine );
            }
            else pResData->sId = rtl::OString::valueOf(static_cast<sal_Int32>(nListIndex));

            if (!pResData->sGId.isEmpty())
                pResData->sGId = pResData->sGId + rtl::OString('.');
            pResData->sGId = pResData->sGId + sOldId;
            nTyp = STRING_TYP_TEXT;
        }
        break;
        case STRING_TYP_TEXT :
        case STRING_TYP_HELPTEXT :
        case STRING_TYP_QUICKHELPTEXT :
        case STRING_TYP_TITLE :
        {
            nStart = rText.indexOf( '=' );
            if ( nStart == -1 ) {
                rText = sOrigText;
                return sal_False;
            }

            nStart++;
            sal_Bool bFound = sal_False;
            while(( nStart < rText.getLength()) && !bFound ) {
                if (( rText[nStart] != ' ' ) && ( rText[nStart] != '\t' ))
                    bFound = sal_True;
                else
                    nStart ++;
            }

            // no start position found
            if ( !bFound ) {
                rText = sOrigText;
                return sal_False;
            }

            // position to end mergeing in
            nEnd = rText.getLength() - 1;
            bFound = sal_False;

            while (( nEnd > nStart ) && !bFound ) {
                if (( rText[nEnd] != ' ' ) && ( rText[nEnd] != '\t' ) &&
                    ( rText[nEnd] != '\n' ) && ( rText[nEnd] != ';' ) &&
                    ( rText[nEnd] != '{' ) && ( rText[nEnd] != '\\' ))
                {
                    bFound = sal_True;
                }
                else
                    nEnd --;
            }
        }
        break;
    }

    // search for merge data
    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, global::inputPathname, false );

        // Init Languages
        if( Export::sLanguages.equalsIgnoreAsciiCase("ALL") )
            SetLanguages( pMergeDataFile->GetLanguages() );
        else if( !isInitialized )InitLanguages();

    }

    PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
    pResData->sId = sOldId;
    pResData->sGId = sOldGId;
    pResData->sResTyp = sOldTyp;

    if ( !pEntrys ) {
        rText = sOrigText;
        return sal_False; // no data found
    }

    rtl::OString sContent;
    pEntrys->GetTransex3Text(sContent, nTyp, rLangIndex);
    if (sContent.isEmpty() && (!Export::isSourceLanguage(rLangIndex)))
    {
        rText = sOrigText;
        return sal_False; // no data found
    }

    if (Export::isSourceLanguage(rLangIndex))
        return sal_False;

    rtl::OString sPostFix( rText.copy( ++nEnd ));
    rText = rText.copy(0, nStart);

    ConvertMergeContent( sContent );



    // merge new res. in text line
    rText += sContent;
    rText += sPostFix;

    return sal_True;
}

void Export::ResData2Output( PFormEntrys *pEntry, sal_uInt16 nType, const rtl::OString& rTextType )
{
    sal_Bool bAddSemicolon = sal_False;
    sal_Bool bFirst = sal_True;
    rtl::OString sCur;

    for( unsigned int n = 0; n < aLanguages.size(); n++ ){
        sCur = aLanguages[ n ];

        rtl::OString sText;
        sal_Bool bText = pEntry->GetTransex3Text( sText, nType, sCur , sal_True );
        if ( bText && !sText.isEmpty() && sText != "-" ) {
            rtl::OString sOutput;
            if ( bNextMustBeDefineEOL)  {
                if ( bFirst )
                    sOutput += "\t\\\n";
                else
                    sOutput += ";\t\\\n";
            }
            bFirst=sal_False;
            sOutput += "\t";

            sOutput += rTextType;

            if ( ! Export::isSourceLanguage( sCur ) ) {
                sOutput += "[ ";
                sOutput += sCur;
                sOutput += " ] ";
            }
            sOutput += "= ";
            ConvertMergeContent( sText );
            sOutput += sText;

            if ( bDefine )
                sOutput += ";\\\n";
            else if ( !bNextMustBeDefineEOL )
                sOutput += ";\n";
            else
                bAddSemicolon = sal_True;
            for ( sal_uInt16 j = 1; j < nLevel; j++ )
                sOutput += "\t";
            WriteToMerged( sOutput , true );
        }
    }


    if ( bAddSemicolon ) {
        rtl::OString sOutput( ";" );
        WriteToMerged( sOutput , false );
    }
}

/*****************************************************************************/
void Export::MergeRest( ResData *pResData, sal_uInt16 nMode )
/*****************************************************************************/
{
    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, global::inputPathname, false );

        // Init Languages
        if (Export::sLanguages.equalsIgnoreAsciiCase("ALL"))
            SetLanguages( pMergeDataFile->GetLanguages() );
        else if( !isInitialized )InitLanguages();

    }
    switch ( nMode ) {
        case MERGE_MODE_NORMAL : {
            PFormEntrys *pEntry = pMergeDataFile->GetPFormEntrys( pResData );

            if ( pEntry ) {
                if ( pResData->bText )
                    ResData2Output( pEntry, STRING_TYP_TEXT, pResData->sTextTyp );

                if ( pResData->bQuickHelpText )
                    ResData2Output( pEntry, STRING_TYP_QUICKHELPTEXT, rtl::OString("QuickHelpText") );

                if ( pResData->bTitle )
                    ResData2Output( pEntry, STRING_TYP_TITLE, rtl::OString("Title") );
            }

            // Merge Lists

            if ( pResData->bList ) {
                bool bPairedList = false;
                rtl::OString sOldId = pResData->sId;
                rtl::OString sOldGId = pResData->sGId;
                rtl::OString sOldTyp = pResData->sResTyp;
                if (!pResData->sGId.isEmpty())
                    pResData->sGId = pResData->sGId + rtl::OString('.');
                pResData->sGId = pResData->sGId + sOldId;
                rtl::OString sSpace;
                for ( sal_uInt16 i = 1; i < nLevel-1; i++ )
                    sSpace += "\t";
                for ( sal_uInt16 nT = LIST_STRING; nT <= LIST_UIENTRIES; nT++ ) {
                    ExportList *pList = NULL;
                    switch ( nT ) {
                        case LIST_STRING : pResData->sResTyp = "stringlist"; pList = pResData->pStringList; bPairedList = false; break;
                        case LIST_FILTER : pResData->sResTyp = "filterlist"; pList = pResData->pFilterList; bPairedList = false; break;
                        case LIST_UIENTRIES : pResData->sResTyp = "uientries"; pList = pResData->pUIEntries;bPairedList = false; break;
                        case LIST_ITEM : pResData->sResTyp = "itemlist"; pList = pResData->pItemList;       bPairedList = false; break;
                        case LIST_PAIRED : pResData->sResTyp = "pairedlist"; pList = pResData->pPairedList; bPairedList = true;  break;
                    }
                    rtl::OString sCur;
                    for( unsigned int n = 0; n < aLanguages.size(); n++ )
                    {
                        sCur = aLanguages[ n ];
                        sal_uInt16 nIdx = 1;

                        // Set matching pairedlist identifier
                        if( bPairedList && pResData->pPairedList && ( nIdx == 1 ) ){
                            ExportListEntry* pListE = ( ExportListEntry* ) (*pResData->pPairedList)[ nIdx-1 ];
                            pResData->sId = GetPairedListID ( (*pListE)[ SOURCE_LANGUAGE ] );
                        }
                        else
                            pResData->sId = "1";

                        PFormEntrys *pEntrys;
                        std::size_t nLIndex = 0;
                        std::size_t nMaxIndex = 0;
                        if ( pList )
                            nMaxIndex = pList->GetSourceLanguageListEntryCount();
                        pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
                        while( pEntrys  && ( nLIndex < nMaxIndex )) {
                            rtl::OString sText;
                            sal_Bool bText;
                            bText = pEntrys->GetTransex3Text( sText, STRING_TYP_TEXT, sCur, sal_True );
                            if( !bText )
                                bText = pEntrys->GetTransex3Text( sText , STRING_TYP_TEXT, SOURCE_LANGUAGE , sal_False );

                            if ( bText && !sText.isEmpty())
                            {
                                if ( nIdx == 1 )
                                {
                                    rtl::OStringBuffer sHead;
                                    if ( bNextMustBeDefineEOL )
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("\\\n\t"));
                                    sHead.append(sSpace);
                                    switch ( nT )
                                    {
                                        case LIST_STRING:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("StringList "));
                                            break;
                                        case LIST_FILTER:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("FilterList "));
                                            break;
                                        case LIST_ITEM:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("ItemList "));
                                            break;
                                        case LIST_PAIRED:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("PairedList "));
                                            break;
                                        case LIST_UIENTRIES:
                                            sHead.append(RTL_CONSTASCII_STRINGPARAM("UIEntries "));
                                            break;
                                    }
                                    sHead.append(RTL_CONSTASCII_STRINGPARAM("[ "));
                                    sHead.append(sCur);
                                    sHead.append(RTL_CONSTASCII_STRINGPARAM(" ] "));
                                    //}
                                    if ( bDefine || bNextMustBeDefineEOL )
                                    {
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("= \\\n"));
                                        sHead.append(sSpace);
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("\t{\\\n\t"));
                                    }
                                    else
                                    {
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("= \n"));
                                        sHead.append(sSpace);
                                        sHead.append(RTL_CONSTASCII_STRINGPARAM("\t{\n\t"));
                                    }
                                    WriteToMerged(sHead.makeStringAndClear() , true);
                                }
                                rtl::OString sLine;
                                if ( pList && (*pList)[ nLIndex ] )
                                    sLine = ( *(*pList)[ nLIndex ])[ SOURCE_LANGUAGE ];
                                if ( sLine.isEmpty())
                                    sLine = sLastListLine;

                                if ( sLastListLine.indexOf( '<' ) != -1 ) {
                                    if (( nT != LIST_UIENTRIES ) &&
                                        (( sLine.indexOf( '{' ) == -1 ) ||
                                        ( sLine.indexOf( '{' ) >= sLine.indexOf( '"' ))) &&
                                        (( sLine.indexOf( '<' ) == -1 ) ||
                                        ( sLine.indexOf( '<' ) >= sLine.indexOf( '"' ))))
                                    {
                                        sLine = sLine.replaceFirst("\"", "< \"" );
                                    }
                                }

                                sal_Int32 nStart, nEnd;
                                nStart = sLine.indexOf( '"' );

                                rtl::OString sPostFix;
                                if( !bPairedList ){
                                    nEnd = sLine.lastIndexOf( '"' );
                                    sPostFix = sLine.copy( ++nEnd );
                                    sLine = sLine.copy(0, nStart);
                                }


                                ConvertMergeContent( sText );

                                // merge new res. in text line
                                if( bPairedList ){
                                    sLine = MergePairedList( sLine , sText );
                                }
                                else{
                                    sLine += sText;
                                    sLine += sPostFix;
                                }

                                rtl::OString sText1( "\t" );
                                sText1 += sLine;
                                if ( bDefine || bNextMustBeDefineEOL )
                                    sText1 += " ;\\\n";
                                else
                                    sText1 += " ;\n";
                                sText1 += sSpace;
                                sText1 += "\t";
                                WriteToMerged( sText1 ,true );

                                // Set matching pairedlist identifier
                                if ( bPairedList ){
                                    nIdx++;
                                    ExportListEntry* pListE = ( ExportListEntry* )(*pResData->pPairedList)[ ( nIdx ) -1 ];
                                    if( pListE ){
                                        pResData->sId = GetPairedListID ( (*pListE)[ SOURCE_LANGUAGE ] );
                                    }
                                }
                                else
                                    pResData->sId = rtl::OString::valueOf(static_cast<sal_Int32>(++nIdx));
                            }
                            else
                                break;
                            nLIndex ++;
                            PFormEntrys *oldEntry = pEntrys;
                            pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
                            if( !pEntrys )
                                pEntrys = oldEntry;
                        }
                        if ( nIdx > 1 ) {
                            rtl::OString sFooter;
                            if (!sSpace.isEmpty()) {
                                sFooter = sSpace.copy(1);
                            }
                            if ( bNextMustBeDefineEOL )
                                sFooter += "};";
                            else if ( !bDefine )
                                sFooter += "};\n\t";
                            else
                                sFooter += "\n\n";
                            WriteToMerged( sFooter ,true );
                        }
                    }
                }

                pResData->sId = sOldId;
                pResData->sGId = sOldGId;
                pResData->sResTyp = sOldTyp;
            }
        }
        break;
        case MERGE_MODE_LIST : {
            ExportList *pList = NULL;
            switch ( nList ) {
                // PairedList
                case LIST_STRING : pList = pResData->pStringList; break;
                case LIST_FILTER : pList = pResData->pFilterList; break;
                case LIST_UIENTRIES : pList = pResData->pUIEntries; break;
                case LIST_ITEM : pList = pResData->pItemList; break;
                case LIST_PAIRED : pList = pResData->pPairedList; break;

            }

            nListIndex++;
            std::size_t nMaxIndex = 0;
            if ( pList )
                nMaxIndex = pList->GetSourceLanguageListEntryCount();
            rtl::OString sLine;
            if ( pList && (*pList)[ nListIndex ] )
                sLine = ( *(*pList)[ nListIndex ])[ SOURCE_LANGUAGE ];
            if ( sLine.isEmpty())
                sLine = sLastListLine;

            if ( sLastListLine.indexOf( '<' ) != -1 ) {
                if (( nList != LIST_UIENTRIES ) &&
                    (( sLine.indexOf( '{' ) == -1 ) ||
                    ( sLine.indexOf( '{' ) >= sLine.indexOf( '"' ))) &&
                    (( sLine.indexOf( '<' ) == -1 ) ||
                    ( sLine.indexOf( '<' ) >= sLine.indexOf( '"' ))))
                {
                    sLine = sLine.replaceFirst("\"", "< \"");
                }
            }

            while( PrepareTextToMerge( sLine, nList, m_sListLang, pResData ) && ( nListIndex <= nMaxIndex )) {
                rtl::OString sText( "\t" );
                sText += sLine;
                sText += " ;";
                sText += "\n";
                for ( sal_uInt16 i = 0; i < nLevel; i++ )
                    sText += "\t";
                WriteToMerged( sText ,false );
                nListIndex++;
                if ( pList && (*pList)[ nListIndex ])
                    sLine = ( *(*pList)[ nListIndex ])[ SOURCE_LANGUAGE ];
                if ( sLine.isEmpty())
                    sLine = sLastListLine;
                sLine += " ;";
            }
        }
        break;
    }
    pParseQueue->bMflag = false;
}

rtl::OString Export::MergePairedList( rtl::OString const & sLine , rtl::OString const & sText ){
// < "xy" ; IDENTIFIER ; >
    rtl::OString sPre  = sLine.copy( 0 , sLine.indexOf('"') );
    rtl::OString sPost = sLine.copy( sLine.lastIndexOf('"') + 1 );
    sPre += sText;
    sPre += sPost;
    return sPre;
}

/*****************************************************************************/
void Export::SetChildWithText()
/*****************************************************************************/
{
    if ( aResStack.size() > 1 ) {
        for ( size_t i = 0; i < aResStack.size() - 1; i++ ) {
            aResStack[ i ]->bChildWithText = sal_True;
        }
    }
}

void ParserQueue::Push( const QueueEntry& aEntry )
{
    sal_Int32 nLen = aEntry.sLine.getLength();

    if( !bStart ){
        aQueueCur->push( aEntry );
        if( nLen > 1 && aEntry.sLine[nLen-1] == '\n' )
            bStart = true;
        else if ( aEntry.nTyp != IGNOREDTOKENS ){
            if( nLen > 1 && ( aEntry.sLine[nLen-1] == '\\') ){
                // Next is Macro
                bCurrentIsM = true;
             }else{
                // Next is no Macro
                bCurrentIsM = false;
             }
        }
    }
    else{
        aQueueNext->push( aEntry );
        if( nLen > 1 && aEntry.sLine[nLen-1] != '\n' ){
            if( nLen > 1 && ( aEntry.sLine[nLen-1] == '\\') ){
                // Next is Macro
                bNextIsM = true;
            }
            else{
                // Next is no Macro
                bNextIsM = false;
            }
        }else if( nLen > 2 && aEntry.sLine[nLen-1] == '\n' ){
            if( aEntry.nTyp != IGNOREDTOKENS ){
                if( nLen > 2 && ( aEntry.sLine[nLen-2] == '\\') ){
                    // Next is Macro
                    bNextIsM = true;
                }
                else{
                    // Next is no Macro
                    bNextIsM = false;
                }
            }
            // Pop current
            Pop( *aQueueCur );
            bLastWasM = bCurrentIsM;
            // next -> current
            bCurrentIsM = bNextIsM;
            aQref = aQueueCur;
            aQueueCur = aQueueNext;
            aQueueNext = aQref;

        }

        else{
            // Pop current
            Pop( *aQueueCur );
            bLastWasM = bCurrentIsM;
            // next -> current
            bCurrentIsM = bNextIsM;
            aQref = aQueueCur;
            aQueueCur = aQueueNext;
            aQueueNext = aQref;
        }
    }
}

void ParserQueue::Close(){
    // Pop current
    Pop( *aQueueCur );
    // next -> current
    bLastWasM = bCurrentIsM;
    bCurrentIsM = bNextIsM;
    aQref = aQueueCur;
    aQueueCur = aQueueNext;
    aQueueNext = aQref;
    bNextIsM = false;
    Pop( *aQueueNext );
};

void ParserQueue::Pop( std::queue<QueueEntry>& aQueue )
{
    while (!aQueue.empty())
    {
        QueueEntry aEntry = aQueue.front();
        aQueue.pop();
        aExport.Execute(aEntry.nTyp, aEntry.sLine.getStr());
    }
}

ParserQueue::ParserQueue( Export& aExportObj )
        :
          bCurrentIsM( false ),
          bNextIsM( false ) ,
          bLastWasM( false ),
          bMflag( false ) ,
          aExport( aExportObj ) ,
          bStart( false )
{
          aQueueNext = new std::queue<QueueEntry>;
          aQueueCur  = new std::queue<QueueEntry>;
}


ParserQueue::~ParserQueue(){
    if( aQueueNext )    delete aQueueNext;
    if( aQueueCur )     delete aQueueCur;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
