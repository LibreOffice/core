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

#include "helper.hxx"
#include "srclex.hxx"

#include <cstdio>
#include <cstdlib>
#include "common.hxx"
#include "export.hxx"
#include "tokens.h"
#include <iostream>
#include <memory>
#include <rtl/strbuf.hxx>

void yyerror( const char * );
void YYWarning( const char * );

namespace {

MergeDataFile * pMergeDataFile = 0; //TODO

namespace global {

OString inputPathname;
std::unique_ptr< Export > exporter;

}

static OString lcl_GetListTyp( const sal_uInt16 nTyp, const bool bUpperCamelCase )
{
    OString sType;
    switch (nTyp)
    {
        case LIST_STRING:
            sType = bUpperCamelCase ? "StringList" : "stringlist";
            break;
        case LIST_FILTER:
            sType = bUpperCamelCase ? "FilterList" : "filterlist";
            break;
        case LIST_ITEM:
            sType = bUpperCamelCase ? "ItemList" : "itemlist";
            break;
        case LIST_PAIRED:
            sType = bUpperCamelCase ? "PairedList" : "pairedlist";
            break;
        default: break;
    }
    return sType;
}

}

extern "C" {

FILE * init(int argc, char ** argv)
{
    common::HandledArgs aArgs;
    if ( !common::handleArguments(argc, argv, aArgs) )
    {
        common::writeUsage("transex3","*.src/*.hrc");
        std::exit(EXIT_FAILURE);
    }
    global::inputPathname =  aArgs.m_sInputFile;

    FILE * pFile = std::fopen(global::inputPathname.getStr(), "r");
    if (pFile == 0) {
        std::fprintf(
            stderr, "Error: Cannot open file \"%s\"\n",
            global::inputPathname.getStr());
        std::exit(EXIT_FAILURE);
    }

    if (aArgs.m_bMergeMode) {
        global::exporter.reset(new Export(aArgs.m_sMergeSrc, aArgs.m_sOutputFile,
                                          aArgs.m_bUTF8BOM));
    } else {
        global::exporter.reset(new Export(aArgs.m_sOutputFile));
    }

    global::exporter->Init();

    return pFile;
}

int Parse( int nTyp, const char *pTokenText ){
    global::exporter->Execute( nTyp , pTokenText );
    return 1;
}

void Close()
{
    global::exporter->GetParseQueue()->Close();
    global::exporter.reset();
        // avoid nontrivial Export dtor being executed during exit
}

int WorkOnTokenSet( int nTyp, char *pTokenText )
{
    global::exporter->GetParseQueue()->Push( QueueEntry( nTyp , OString(pTokenText) ) );
    return 1;
}

int SetError()
{
    // set error at global instance of class Export
    global::exporter->SetError();
    return 1;
}

int GetError()
{
    // get error at global instance of class Export
    if (global::exporter->GetError())
        return 1;
    return sal_False;
}

} // extern "C"


// class ResData


bool ResData::SetId( const OString& rId, sal_uInt16 nLevel )
{
    if ( nLevel > nIdLevel )
    {
        nIdLevel = nLevel;
        sId = rId;

        if ( bChild && bChildWithText )
        {
            OString sError("ResId after child definition");
            yyerror(sError.getStr());
            SetError();
        }

        if ( sId.getLength() > 255 )
        {
            YYWarning( "LocalId > 255 chars, truncating..." );
            sId = sId.copy(0, 255).trim();
        }

        return true;
    }

    return false;
}


// class Export


namespace
{

static sal_Int32 lcl_countOccurrences(const OString& text, char c)
{
    sal_Int32 n = 0;
    for (sal_Int32 i = 0;; ++i) {
        i = text.indexOf(c, i);
        if (i == -1) {
            break;
        }
        ++n;
    }
    return n;
}

}

Export::Export(const OString &rOutput)
                :
                bDefine( false ),
                bNextMustBeDefineEOL( false ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bMergeMode( false ),
                bError( false ),
                bReadOver( false ),
                sFilename( global::inputPathname ),
                pParseQueue( new ParserQueue( *this ) )
{
    aOutput.mPo = new PoOfstream( rOutput, PoOfstream::APP );
    if (!aOutput.mPo->isOpen())
    {
        std::fprintf(stderr, "ERROR : Can't open file %s\n", rOutput.getStr());
        std::exit(EXIT_FAILURE);
    }
}

Export::Export(
    const OString &rMergeSource, const OString &rOutput,
    bool bUTF8BOM)
                :
                bDefine( false ),
                bNextMustBeDefineEOL( false ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bMergeMode( true ),
                sMergeSrc( rMergeSource ),
                bError( false ),
                bReadOver( false ),
                sFilename( global::inputPathname ),
                pParseQueue( new ParserQueue( *this ) )
{
    aOutput.mSimple = new std::ofstream();
    aOutput.mSimple->open(rOutput.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!aOutput.mSimple->is_open())
    {
        std::fprintf(stderr, "ERROR : Can't open file %s\n", rOutput.getStr());
        std::exit(EXIT_FAILURE);
    }

    if ( bUTF8BOM ) WriteUTF8ByteOrderMarkToOutput();
}

void Export::Init()
{
    // resets the internal status, used before parseing another file
    bDefine = false;
    bNextMustBeDefineEOL = false;
    nLevel = 0;
    nList = LIST_NON;
    nListIndex = 0;
    for ( size_t i = 0, n = aResStack.size(); i < n;  ++i )
        delete aResStack[ i ];
    aResStack.clear();
}

Export::~Export()
{
    delete pParseQueue;
    if ( bMergeMode )
    {
        aOutput.mSimple->close();
        delete aOutput.mSimple;
    }
    else
    {
        aOutput.mPo->close();
        delete aOutput.mPo;
    }
    for ( size_t i = 0, n = aResStack.size(); i < n;  ++i )
        delete aResStack[ i ];
    aResStack.clear();

    if ( bMergeMode ) {
        if ( !pMergeDataFile )
            pMergeDataFile = new MergeDataFile(sMergeSrc, global::inputPathname, true);

        delete pMergeDataFile;
    }
}

int Export::Execute( int nToken, const char * pToken )
{

    OString sToken( pToken );
    OString sOrig( sToken );

    if ( nToken == CONDITION )
    {
        OString sTestToken(pToken);
        sTestToken = sTestToken.replaceAll("\t", OString()).
            replaceAll(" ", OString());
        if (( !bReadOver ) && ( sTestToken.startsWith("#ifndef__RSC_PARSER")))
            bReadOver = true;
        else if (( bReadOver ) && ( sTestToken.startsWith("#endif") ))
            bReadOver = false;
    }
    if ((( nToken < FILTER_LEVEL ) || ( bReadOver )) &&
        (!(( bNextMustBeDefineEOL ) && ( sOrig == "\n" )))) {
        // this tokens are not mandatory for parsing, so ignore them ...
        if ( bMergeMode )
            WriteToMerged( sOrig , false ); // ... or write them directly to dest.
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

    if ( bDefine ) {
        if (( nToken != EMPTYLINE ) && ( nToken != LEVELDOWN ) && ( nToken != LEVELUP )) {
            // cur. res. defined in macro
            if ( bNextMustBeDefineEOL ) {
                if ( nToken != RSCDEFINELEND ) {
                    // end of macro found, so destroy res.
                    bDefine = false;
                    Execute( LEVELDOWN, "" );
                    bNextMustBeDefineEOL = false;
                }
                else {
                    // next line also in macro definition
                    bNextMustBeDefineEOL = false;
                    if ( bMergeMode )
                        WriteToMerged( sOrig , false );
                    return 1;
                }
            }
        }
    }

    bool bExecuteDown = false;
    if ( nToken != LEVELDOWN ) {
        sal_uInt16 nOpen = 0;
        sal_uInt16 nClose = 0;
        bool bReadOver1 = false;
        sal_uInt16 i = 0;
        for ( i = 0; i < sToken.getLength(); i++ ) {
            if ( sToken[i] == '"' )
                bReadOver1 = !bReadOver1;
            if ( !bReadOver1 && ( sToken[i] == '{' ))
                nOpen++;
        }

        bReadOver1 = false;
        for ( i = 0; i < sToken.getLength(); i++ ) {
            if ( sToken[i] == '"' )
                bReadOver1 = !bReadOver1;
            if ( !bReadOver1 && ( sToken[i] == '}' ))
                nClose++;
        }

        if ( nOpen < nClose )
            bExecuteDown = true;
    }

    bool bWriteToMerged = bMergeMode;
    switch ( nToken ) {

        case NORMDEFINE:
            if ( bMergeMode )
                WriteToMerged( sOrig , false );
            return 0;
        case RSCDEFINE:
            bDefine = true; // res. defined in macro

        case RESOURCE:
        case RESOURCEEXPR: {
            if ( nToken != RSCDEFINE )
                bNextMustBeDefineEOL = false;
            // this is the beginning of a new res.
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = true;
            }

            // create new instance for this res. and fill mandatory fields

            pResData = new ResData( FullId() , sFilename );
            aResStack.push_back( pResData );
            sToken = sToken.replaceAll("\n", OString()).
                replaceAll("\r", OString()).
                replaceAll("{", OString()).replace('\t', ' ');
            sToken = sToken.trim();
            OString sTLower = sToken.getToken(0, ' ').toAsciiLowerCase();
            pResData->sResTyp = sTLower;
            OString sId( sToken.copy( pResData->sResTyp.getLength() + 1 ));
            OString sCondition;
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
            sId = sId.replaceAll("\t", OString());
            pResData->SetId( sId, ID_LEVEL_IDENTIFIER );
            if (!sCondition.isEmpty())
            {
                Execute( CONDITION, "");  // execute the precomp. condition
            }
        }
        break;
        case SMALRESOURCE: {
            // this is the beginning of a new res.
            bNextMustBeDefineEOL = false;
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = true;
            }

            // create new instance for this res. and fill mandatory fields

            pResData = new ResData( FullId() , sFilename );
            aResStack.push_back( pResData );
            sToken = sToken.replaceAll("\n", OString()).
                replaceAll("\r", OString()).
                replaceAll("{", OString()).
                replaceAll("\t", OString()).
                replaceAll(" ", OString()).
                replaceAll("\\", OString()).toAsciiLowerCase();
            pResData->sResTyp = sToken;
        }
        break;
        case LEVELUP: {
            // push
            if ( nList )
            {
                nListLevel++;
                break;
            }

            OString sLowerTyp;
            if ( pResData )
                sLowerTyp = "unknown";
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = true;
            }

            ResData *pNewData = new ResData( FullId() , sFilename );
            pNewData->sResTyp = sLowerTyp;
            aResStack.push_back( pNewData );
        }
        break;
        case LEVELDOWN: {
            // pop
            if ( !nList || !nListLevel ) {
                if ( nLevel ) {
                    if ( bDefine && (nLevel == 1 )) {
                        bDefine = false;
                        bNextMustBeDefineEOL = false;
                    }
                    WriteData( pResData );
                    ResStack::iterator it = aResStack.begin();
                    ::std::advance( it, nLevel-1 );
                    delete *it;
                    aResStack.erase( it );
                    nLevel--;
                }
                if( nList )
                {
                    nList = LIST_NON;
                    nListLevel = 1;
                }
            }
            else
            {
                if ( bDefine )
                    bNextMustBeDefineEOL = true;
                nListLevel--;
            }
        }
        break;
        case ASSIGNMENT:
        {
            // interpret different types of assignement
            sal_Int32 n = 0;
            OString sKey = sToken.getToken(0, '=', n).
                replaceAll(" ", OString()).
                replaceAll("\t", OString());
            OString sValue = sToken.getToken(0, '=', n);
            CleanValue( sValue );
            sKey = sKey.toAsciiUpperCase();
            if (sKey == "IDENTIFIER")
            {
                OString sId(
                    sValue.replaceAll("\t", OString()).
                    replaceAll(" ", OString()));
                pResData->SetId(sId, ID_LEVEL_IDENTIFIER);
            }
            else if (sKey =="STRINGLIST")
            {
                nList = LIST_STRING;
                nListIndex = 0;
                nListLevel = 1;
            }
            else if (sKey == "FILTERLIST")
            {
                nList = LIST_FILTER;
                nListIndex = 0;
                nListLevel = 1;
            }
            if (sToken.indexOf( '{' ) != -1
                && (lcl_countOccurrences(sToken, '{')
                    > lcl_countOccurrences(sToken, '}')))
            {
                Parse( LEVELUP, "" );
            }
         }
        break;
        case LISTASSIGNMENT:
        {
            OString sTmpToken(
                sToken.replaceAll(" ", OString()).toAsciiLowerCase());
            sal_Int32 nPos = sTmpToken.indexOf("[en-us]=");
            if (nPos != -1) {
                OString sKey(
                    sTmpToken.copy(0 , nPos).replaceAll(" ", OString()).
                    replaceAll("\t", OString()));
                OString sValue = sToken.getToken(1, '=');
                CleanValue( sValue );
                sKey = sKey.toAsciiUpperCase();
                if (sKey == "STRINGLIST")
                {
                    nList = LIST_STRING;
                }
                else if (sKey == "FILTERLIST")
                {
                    nList = LIST_FILTER;
                }
                else if (sKey == "PAIREDLIST")
                {
                    nList = LIST_PAIRED;
                }
                else if (sKey == "ITEMLIST")
                {
                    nList = LIST_ITEM;
                }
                if( nList )
                {
                    nListIndex = 0;
                    nListLevel = 1;
                }
            }
        }
        break;
        case TEXT:
        case _LISTTEXT:
        case LISTTEXT: {
            // this is an entry for a List
            if ( nList )
            {
                SetChildWithText();
                InsertListEntry( sOrig );
            }
        }
        break;
        case LONGTEXTLINE:
        case TEXTLINE:
            if ( nLevel )
            {
                CutComment( sToken );

                // this is a text line!!!
                OString t(sToken.getToken(0, '='));
                OString sKey(
                    t.getToken(0, '[').replaceAll(" ", OString()).
                    replaceAll("\t", OString()));
                OString sText( GetText( sToken, nToken ));
                OString sLang;
                if ( sToken.getToken(0, '=').indexOf('[') != -1 )
                {
                    sLang = sToken.getToken(0, '=').getToken(1, '[').
                        getToken(0, ']');
                    CleanValue( sLang );
                }
                OString sLangIndex = sLang;
                OString sOrigKey = sKey;
                if ( !sText.isEmpty() && !sLang.isEmpty() )
                {
                    sKey = sKey.toAsciiUpperCase();
                    if (sKey == "TEXT" || sKey == "MESSAGE"  || sKey == "CUSTOMUNITTEXT")
                    {
                        SetChildWithText();
                        if ( sLangIndex.equalsIgnoreAsciiCase("en-US") )
                            pResData->SetId( sText, ID_LEVEL_TEXT );

                        pResData->bText = true;
                        pResData->sTextTyp = sOrigKey;
                        if ( !bMergeMode )
                        {
                            pResData->sText[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "QUICKHELPTEXT" ) {
                        SetChildWithText();
                        pResData->bQuickHelpText = true;
                        if ( !bMergeMode )
                        {
                            pResData->sQuickHelpText[ sLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "TITLE" ) {
                        SetChildWithText();
                        pResData->bTitle = true;
                        if ( !bMergeMode )
                        {
                            pResData->sTitle[ sLangIndex ] = sText;
                        }
                    }
                }
            }
        break;
        case APPFONTMAPPING:
        break;
        case RSCDEFINELEND:
        break;
        case CONDITION: {
            if ( nLevel && pResData ) {
                WriteData( pResData, true );
            }
        }
        break;
        case EMPTYLINE : {
            if ( bDefine ) {
                bNextMustBeDefineEOL = false;
                bDefine = false;
                while ( nLevel )
                    Parse( LEVELDOWN, "" );
            }
        }
        break;
        case PRAGMA : {
            std::fprintf(stderr, "ERROR: archaic PRAGMA %s\n", sToken.getStr());
            std::exit(EXIT_FAILURE);
        }
        break;
        }
    if ( bWriteToMerged ) {
        // the current token must be written to dest. without merging

        if( bDefine && sOrig.getLength() > 2 ){
            for( sal_Int32 n = 0; n < sOrig.getLength(); n++ ){
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

void Export::CutComment( OString &rText )
{
    if (rText.indexOf("//") != -1) {
        OString sWork(rText.replaceAll("\\\"", "XX"));
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

bool Export::WriteData( ResData *pResData, bool bCreateNew )
{
    if ( bMergeMode ) {
        MergeRest( pResData );
        return true;
    }

       // mandatory to export: en-US

     if (( !pResData->sText[ SOURCE_LANGUAGE ].isEmpty())
        ||
        (  !pResData->sQuickHelpText[ SOURCE_LANGUAGE ].isEmpty())
         ||
        (  !pResData->sTitle[ SOURCE_LANGUAGE ].isEmpty()))

       {
        OString sGID = pResData->sGId;
        OString sLID;
        if (sGID.isEmpty())
            sGID = pResData->sId;
        else
            sLID = pResData->sId;

        OString sXText = pResData->sText[ SOURCE_LANGUAGE ];
        OString sXHText = pResData->sText[ X_COMMENT ];
        OString sXQHText = pResData->sQuickHelpText[ SOURCE_LANGUAGE ];
        OString sXTitle = pResData->sTitle[ SOURCE_LANGUAGE ];

        if( !sXText.isEmpty() )
        {
            ConvertExportContent(sXText);
            ConvertExportContent(sXHText);
            common::writePoEntry(
                "Transex3", *aOutput.mPo, global::inputPathname,
                pResData->sResTyp, sGID, sLID, sXHText, sXText);
        }
        if( !sXQHText.isEmpty() )
        {
            ConvertExportContent(sXQHText);
            common::writePoEntry(
                "Transex3", *aOutput.mPo, global::inputPathname, pResData->sResTyp,
                sGID, sLID, OString(), sXQHText, PoEntry::TQUICKHELPTEXT );
        }
        if( !sXTitle.isEmpty() )
        {
            ConvertExportContent(sXTitle);
            common::writePoEntry(
                "Transex3", *aOutput.mPo, global::inputPathname, pResData->sResTyp,
                sGID, sLID, OString(), sXTitle, PoEntry::TTITLE );
        }

        if ( bCreateNew ) {
            pResData->sText[ SOURCE_LANGUAGE ]         = "";
            pResData->sQuickHelpText[ SOURCE_LANGUAGE ]= "";
            pResData->sTitle[ SOURCE_LANGUAGE ]        = "";
        }
    }

    if( nList )
    {
        WriteExportList( pResData, pResData->m_aList, nList );
        if ( bCreateNew )
            pResData->m_aList.clear();
    }
    return true;
}

OString Export::GetPairedListID(const OString& rText)
{
// < "STRING" ; IDENTIFIER ; > ;
    return rText.getToken(1, ';').toAsciiUpperCase().replace('\t', ' ').trim();
}

OString Export::GetPairedListString(const OString& rText)
{
// < "STRING" ; IDENTIFIER ; > ;
    OString sString(rText.getToken(0, ';').replace('\t', ' '));
    sString = sString.trim();
    OString s1(sString.copy(sString.indexOf('"') + 1));
    sString = s1.copy(0, s1.lastIndexOf('"'));
    return sString.trim();
}

OString Export::StripList(const OString & rText)
{
    OString s1 = rText.copy( rText.indexOf('\"') + 1);
    return s1.copy( 0 , s1.lastIndexOf('\"'));
}

bool Export::WriteExportList(ResData *pResData, ExportList& rExportList,
    const sal_uInt16 nTyp)
{
    OString sGID(pResData->sGId);
    if (sGID.isEmpty())
        sGID = pResData->sId;
    else {
        sGID += ".";
        sGID += pResData->sId;
        while (sGID.endsWith(".")) {
            sGID = sGID.copy(0, sGID.getLength() - 1);
        }
    }

    for ( size_t i = 0; i < rExportList.size(); i++ )
    {
        OString sLID;
        OString sText(rExportList[ i ]);

        // Strip PairList Line String
        if (nTyp == LIST_PAIRED)
        {
            sLID = GetPairedListID( sText );
            sText = GetPairedListString( sText );
        }
        else
        {
            sText = StripList( sText );
            if( sText == "\\\"" )
                sText = "\"";
        }
        ConvertExportContent(sText);

        if (nTyp != LIST_PAIRED)
            sLID = sText;

        OString sType = lcl_GetListTyp( nList, false );

        common::writePoEntry(
            "Transex3", *aOutput.mPo, global::inputPathname,
            sType, sGID, sLID, OString(), sText);
    }

    return true;
}

OString Export::FullId()
{
    OStringBuffer sFull;
    if ( nLevel > 1 )
    {
        sFull.append(aResStack[ 0 ]->sId);
        for ( size_t i = 1; i < nLevel - 1; ++i )
        {
            OString sToAdd = aResStack[ i ]->sId;
            if (!sToAdd.isEmpty())
                sFull.append('.').append(sToAdd);
        }
    }
    if (sFull.getLength() > 255)
    {
        OString sError("GroupId > 255 chars");
        printf("GroupID = %s\n", sFull.getStr());
        yyerror(sError.getStr());
    }

    return sFull.makeStringAndClear();
}

void Export::InsertListEntry(const OString &rLine)
{
    ResData *pResData = ( nLevel-1 < aResStack.size() ) ? aResStack[ nLevel-1 ] : NULL;

    if (!pResData)
        std::exit(EXIT_FAILURE);

    if( pResData->m_aList.empty() )
        nListIndex = 0;

    pResData->m_aList.push_back(rLine);
}

void Export::CleanValue( OString &rValue )
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

#define TXT_STATE_TEXT  0x001
#define TXT_STATE_MACRO 0x002

OString Export::GetText(const OString &rSource, int nToken)
{
    OString sReturn;
    switch ( nToken )
    {
        case TEXTLINE:
        case LONGTEXTLINE:
        {
            OString sTmp(rSource.copy(rSource.indexOf('=')));
            CleanValue( sTmp );
            sTmp = sTmp.replaceAll("\n", OString()).
                replaceAll("\r", OString()).
                replaceAll("\\\\\"", "-=<[BSlashBSlashHKom]>=-\"").
                replaceAll("\\\"", "-=<[Hochkomma]>=-").
                replaceAll("\\", "-=<[0x7F]>=-").
                replaceAll("\\0x7F", "-=<[0x7F]>=-");

            sal_uInt16 nState = TXT_STATE_TEXT;
            for (sal_Int32 i = 1; i <= lcl_countOccurrences(sTmp, '"'); ++i)
            {
                OString sToken(sTmp.getToken(i, '"'));
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

void Export::WriteToMerged(const OString &rText , bool bSDFContent)
{
    OString sText(rText);
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
            *aOutput.mSimple << '\n';
        } else {
            char cChar = sText[i];
            *aOutput.mSimple << cChar;
        }
    }
}

void Export::ConvertMergeContent( OString &rText )
{
    rText = rText.replaceAll("\\\'","\'"); // Temporary: until PO files contain escaped single quotes
                                            // (Maybe next PO update solve this)
    rText =
        helper::escapeAll(
            rText.replaceAll("","\\0x7F"),
            "\n""\t""\\""\"","\\n""\\t""\\\\""\\\"");

    rText = "\"" + rText + "\"";
}

void Export::ConvertExportContent( OString& rText )
{
    rText = helper::unEscapeAll(rText,"\\n""\\t""\\\\""\\\"","\n""\t""\\""\"");
}

void Export::ResData2Output( MergeEntrys *pEntry, sal_uInt16 nType, const OString& rTextType )
{
    bool bAddSemicolon = false;
    bool bFirst = true;
    OString sCur;

    for( size_t n = 0; n < aLanguages.size(); n++ ){
        sCur = aLanguages[ n ];

        OString sText;
        bool bText = pEntry->GetText( sText, nType, sCur , true );
        if ( bText && !sText.isEmpty() ) {
            OStringBuffer sOutput;
            if ( bNextMustBeDefineEOL)  {
                if ( bFirst )
                    sOutput.append("\t\\\n");
                else
                    sOutput.append(";\t\\\n");
            }
            bFirst=false;
            sOutput.append("\t" + rTextType);

            if ( !sCur.equalsIgnoreAsciiCase("en-US") ) {
                sOutput.append("[ " + sCur + " ] ");
            }

            ConvertMergeContent( sText );
            sOutput.append("= " + sText);

            if ( bDefine )
                sOutput.append(";\\\n");
            else if ( !bNextMustBeDefineEOL )
                sOutput.append(";\n");
            else
                bAddSemicolon = true;
            for ( size_t j = 1; j < nLevel; j++ )
                sOutput.append("\t");
            WriteToMerged( sOutput.makeStringAndClear() , true );
        }
    }


    if ( bAddSemicolon ) {
        WriteToMerged( ";" , false );
    }
}

void Export::MergeRest( ResData *pResData )
{
    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, global::inputPathname, true );
        aLanguages = pMergeDataFile->GetLanguages();

    }

    MergeEntrys *pEntry = 0;
    if( pResData->bText || pResData->bQuickHelpText || pResData->bTitle )
        pEntry = pMergeDataFile->GetMergeEntrysCaseSensitive( pResData );

    if ( pEntry )
    {
        if ( pResData->bText )
            ResData2Output( pEntry, STRING_TYP_TEXT, pResData->sTextTyp );

        if ( pResData->bQuickHelpText )
            ResData2Output( pEntry, STRING_TYP_QUICKHELPTEXT, OString("QuickHelpText") );

        if ( pResData->bTitle )
            ResData2Output( pEntry, STRING_TYP_TITLE, OString("Title") );
    }

    // Merge Lists
    if ( nList )
    {
        OString sOldId = pResData->sId;
        OString sOldGId = pResData->sGId;
        OString sOldTyp = pResData->sResTyp;

        // Set pResData so we can find the corresponding string
        if (!pResData->sGId.isEmpty())
            pResData->sGId = pResData->sGId + OString('.');
        pResData->sGId = pResData->sGId + pResData->sId;

        pResData->sResTyp = lcl_GetListTyp( nList, false );

        OString sSpace;
        for ( sal_uInt16 i = 1; i < nLevel-1; i++ )
            sSpace += "\t";

        OString sCur;
        for( size_t n = 0; n < aLanguages.size(); n++ )
        {
            sCur = aLanguages[ n ];

            sal_uInt16 nLIndex = 0;
            sal_uInt16 nMaxIndex = pResData->m_aList.size();
            while( nLIndex < nMaxIndex )
            {
                if ( nLIndex == 0 )
                {
                    OStringBuffer sHead;
                    if ( bNextMustBeDefineEOL )
                        sHead.append("\\\n\t");
                    sHead.append(sSpace + lcl_GetListTyp( nList, true ) + " [ " + sCur + " ] ");

                    if ( bDefine || bNextMustBeDefineEOL )
                    {
                        sHead.append("= \\\n" + sSpace + "\t{\\\n\t");
                    }
                    else
                    {
                        sHead.append("= \n" + sSpace + "\t{\n\t");
                    }
                    WriteToMerged(sHead.makeStringAndClear() , true);
                }

                OString sLine = pResData->m_aList[ nLIndex ];
                if ( sLine.indexOf( '>' ) != -1 )
                {
                    if ((( sLine.indexOf( '{' ) == -1 ) ||
                        ( sLine.indexOf( '{' ) >= sLine.indexOf( '"' ))) &&
                        (( sLine.indexOf( '<' ) == -1 ) ||
                        ( sLine.indexOf( '<' ) >= sLine.indexOf( '"' ))))
                    {
                        sLine = sLine.replaceFirst("\"", "< \"" );
                    }
                }

                // Set matching identifier
                if ( nList == LIST_PAIRED )
                {
                    pResData->sId = GetPairedListID ( sLine );
                }
                else
                {
                    pResData->sId =
                        sLine.copy(
                        sLine.indexOf('"')+1,
                        sLine.lastIndexOf('"')-sLine.indexOf('"')-1);
                    ConvertExportContent( pResData->sId );
                }

                MergeEntrys* pEntrys = pMergeDataFile->GetMergeEntrysCaseSensitive( pResData );

                if( pEntrys )
                {
                    OString sText;
                    pEntrys->GetText( sText, STRING_TYP_TEXT, sCur );
                    if( !sText.isEmpty())
                    {
                        ConvertMergeContent( sText );
                        sLine =
                            sLine.copy( 0 , sLine.indexOf('"') ) +
                            sText +
                            sLine.copy( sLine.lastIndexOf('"') + 1 );
                    }
                }

                OString sText1( "\t" );
                sText1 += sLine;
                if ( bDefine || bNextMustBeDefineEOL )
                    sText1 += " ;\\\n";
                else
                    sText1 += " ;\n";
                sText1 += sSpace;
                sText1 += "\t";
                WriteToMerged( sText1 ,true );
                ++nLIndex;
            }

            if ( nLIndex > 0 )
            {
                OString sFooter;
                if (!sSpace.isEmpty())
                    sFooter = sSpace.copy(1);

                if ( bNextMustBeDefineEOL )
                    sFooter += "};";
                else if ( !bDefine )
                    sFooter += "};\n\t";
                else
                    sFooter += "\n\n";
                WriteToMerged( sFooter ,true );
            }
        }
        pResData->sId = sOldId;
        pResData->sGId = sOldGId;
        pResData->sResTyp = sOldTyp;
    }
    pParseQueue->bMflag = false;
}

void Export::SetChildWithText()
{
    if ( aResStack.size() > 1 ) {
        for ( size_t i = 0; i < aResStack.size() - 1; i++ ) {
            aResStack[ i ]->bChildWithText = true;
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
            std::queue<QueueEntry>* aQref = aQueueCur;
            aQueueCur = aQueueNext;
            aQueueNext = aQref;

        }

        else{
            // Pop current
            Pop( *aQueueCur );
            bLastWasM = bCurrentIsM;
            // next -> current
            bCurrentIsM = bNextIsM;
            std::queue<QueueEntry>* aQref = aQueueCur;
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
    std::queue<QueueEntry>* aQref = aQueueCur;
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


ParserQueue::~ParserQueue()
{
    delete aQueueNext;
    delete aQueueCur;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
