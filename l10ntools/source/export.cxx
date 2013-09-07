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
#include "boost/scoped_ptr.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "common.hxx"
#include "export.hxx"
#include "tokens.h"
#include <iostream>
#include <rtl/strbuf.hxx>

void yyerror( const char * );
void YYWarning( const char * );

namespace {

MergeDataFile * pMergeDataFile = 0; //TODO

namespace global {

OString inputPathname;
boost::scoped_ptr< Export > exporter;

}

}

extern "C" {

FILE * init(int argc, char ** argv) {

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
                                          aArgs.m_sLanguage, aArgs.m_bUTF8BOM));
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
void Close(){
    global::exporter->pParseQueue->Close();
    global::exporter.reset();
        // avoid nontrivial Export dtor being executed during exit
}

int WorkOnTokenSet( int nTyp, char *pTokenText )
{

    global::exporter->pParseQueue->Push( QueueEntry( nTyp , OString(pTokenText) ) );
    return 1;
}

} // extern

extern "C" {

int SetError()
{
    // set error at global instance of class Export
    global::exporter->SetError();
    return 1;
}
}

extern "C" {

int GetError()
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

sal_Bool ResData::SetId( const OString& rId, sal_uInt16 nLevel )
{
    if ( nLevel > nIdLevel )
    {
        nIdLevel = nLevel;
        sId = rId;

        if ( bChild && bChildWithText )
        {
            OString sError(RTL_CONSTASCII_STRINGPARAM("ResId after child definition"));
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
                bDefine( sal_False ),
                bNextMustBeDefineEOL( sal_False ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bSkipFile( false ),
                bMergeMode( false ),
                bError( sal_False ),
                bReadOver( sal_False ),
                bDontWriteOutput( sal_False ),
                sFilename( global::inputPathname ),
                sLanguages( OString() ),
                pParseQueue( new ParserQueue( *this ) )
{
    aOutput.mPo = new PoOfstream( rOutput, PoOfstream::APP );
    if (!aOutput.mPo->isOpen()) {
        fprintf(stderr, "ERROR : Can't open file %s\n", rOutput.getStr());
        exit ( -1 );
    }
}

Export::Export(
    const OString &rMergeSource, const OString &rOutput,
    const OString &rLanguage, bool bUTF8BOM)
                :
                bDefine( sal_False ),
                bNextMustBeDefineEOL( sal_False ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bSkipFile( false ),
                bMergeMode( sal_True ),
                sMergeSrc( rMergeSource ),
                bError( sal_False ),
                bReadOver( sal_False ),
                bDontWriteOutput( sal_False ),
                sFilename( global::inputPathname ),
                sLanguages( rLanguage ),
                pParseQueue( new ParserQueue( *this ) )
{
    aOutput.mSimple = new std::ofstream();
    aOutput.mSimple->open(rOutput.getStr(), std::ios_base::out | std::ios_base::trunc);

    if ( bUTF8BOM ) WriteUTF8ByteOrderMarkToOutput();
}

void Export::Init()
{
    // resets the internal status, used before parseing another file
    bDefine = sal_False;
    bNextMustBeDefineEOL = sal_False;
    nLevel = 0;
    nList = LIST_NON;
    m_sListLang = OString();
    nListIndex = 0;
    for ( size_t i = 0, n = aResStack.size(); i < n;  ++i )
        delete aResStack[ i ];
    aResStack.clear();
}

Export::~Export()
{
    if( pParseQueue )
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
            pMergeDataFile = new MergeDataFile(sMergeSrc, global::inputPathname, false);

        delete pMergeDataFile;
    }
}

int Export::Execute( int nToken, const char * pToken )
{

    OString sToken( pToken );
    OString sOrig( sToken );
    sal_Bool bWriteToMerged = bMergeMode;

    if ( nToken == CONDITION )
    {
        OString sTestToken(pToken);
        sTestToken = sTestToken.replaceAll("\t", OString()).
            replaceAll(" ", OString());
        if (( !bReadOver ) && ( sTestToken.indexOf("#ifndef__RSC_PARSER") == 0 ))
            bReadOver = sal_True;
        else if (( bReadOver ) && ( sTestToken.indexOf("#endif") == 0 ))
            bReadOver = sal_False;
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
                    Execute( LEVELDOWN, "" );
                    bNextMustBeDefineEOL = sal_False;
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
            bDontWriteOutput = sal_False;
            // this is the beginning of a new res.
            bNextMustBeDefineEOL = sal_False;
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = sal_True;
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
                nListLevel++;
            if ( nList )
                break;

            bDontWriteOutput = sal_False;
            OString sLowerTyp;
            if ( pResData )
                sLowerTyp = "unknown";
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack[ nLevel - 2 ]->bChild = sal_True;
            }

            ResData *pNewData = new ResData( FullId() , sFilename );
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
            OString sKey = sToken.getToken(0, '=', n).
                replaceAll(" ", OString()).
                replaceAll("\t", OString());
            OString sValue = sToken.getToken(0, '=', n);
            CleanValue( sValue );
            sKey = sKey.toAsciiUpperCase();
            if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("IDENTIFIER")))
            {
                OString sId(
                    sValue.replaceAll("\t", OString()).
                    replaceAll(" ", OString()));
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
                && (lcl_countOccurrences(sToken, '{')
                    > lcl_countOccurrences(sToken, '}')))
            {
                Parse( LEVELUP, "" );
            }
         }
        break;
        case UIENTRIES:
        case LISTASSIGNMENT:
        {
            bDontWriteOutput = sal_False;
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
        }
        break;
        case TEXT:
        case _LISTTEXT:
        case LISTTEXT: {
            // this is an entry for a String- or FilterList
            if ( nList ) {
                SetChildWithText();
                sal_Int32 n = 0;
                OString sEntry(sToken.getToken(1, '"', n));
                if ( lcl_countOccurrences(sToken, '"') > 2 )
                    sEntry += "\"";
                if ( sEntry == "\\\"" )
                    sEntry = "\"";
                InsertListEntry( sEntry, sOrig );
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
                    if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("TEXT")) ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("MESSAGE"))  ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("CUSTOMUNITTEXT"))  ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("SLOTNAME"))  ||
                        sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UINAME")))
                    {
                        SetChildWithText();
                        if ( sLangIndex.equalsIgnoreAsciiCase("en-US") )
                            pResData->SetId( sText, ID_LEVEL_TEXT );

                        pResData->bText = sal_True;
                        pResData->sTextTyp = sOrigKey;
                        if ( !bMergeMode )
                        {
                            if (!pResData->sText[ sLangIndex ].isEmpty())
                            {
                                OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
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
                        if ( !bMergeMode )
                        {
                            if (!pResData->sHelpText[ sLangIndex ].isEmpty())
                            {
                                OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
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
                        if ( !bMergeMode )
                        {
                            if (!pResData->sQuickHelpText[ sLangIndex ].isEmpty())
                            {
                                OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
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
                        if ( !bMergeMode )
                        {
                            if ( !pResData->sTitle[ sLangIndex ].isEmpty())
                            {
                                OStringBuffer sError(RTL_CONSTASCII_STRINGPARAM("Language "));
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
        }
        break;
        case RSCDEFINELEND:
            bDontWriteOutput = sal_False;
        break;
        case CONDITION: {
            bDontWriteOutput = sal_False;
            if ( nLevel ) {
                WriteData( pResData, sal_True );
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

sal_Bool Export::WriteData( ResData *pResData, sal_Bool bCreateNew )
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
        OString sGID = pResData->sGId;
        OString sLID;
        if (sGID.isEmpty())
            sGID = pResData->sId;
        else
            sLID = pResData->sId;

        OString sXText;
        OString sXHText;
        OString sXQHText;
        OString sXTitle;

        sXText = pResData->sText[ SOURCE_LANGUAGE ];
        if (!pResData->sText[ X_COMMENT ].isEmpty())
            sXHText = pResData->sText[ X_COMMENT ];
        else
            sXHText = pResData->sHelpText[ SOURCE_LANGUAGE ];
        sXQHText = pResData->sQuickHelpText[ SOURCE_LANGUAGE ];
        sXTitle = pResData->sTitle[ SOURCE_LANGUAGE ];

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
            pResData->sHelpText[ SOURCE_LANGUAGE ]     = "";
            pResData->sQuickHelpText[ SOURCE_LANGUAGE ]= "";
            pResData->sTitle[ SOURCE_LANGUAGE ]        = "";
        }
    }
    if ( pResData->pStringList ) {
        OString sList( "stringlist" );
        WriteExportList( pResData, pResData->pStringList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pStringList = 0;
    }
    if ( pResData->pFilterList ) {
        OString sList( "filterlist" );
        WriteExportList( pResData, pResData->pFilterList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pFilterList = 0;
    }
    if ( pResData->pItemList ) {
        OString sList( "itemlist" );
        WriteExportList( pResData, pResData->pItemList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pItemList = 0;
    }
    if ( pResData->pPairedList ) {
        OString sList( "pairedlist" );
        WriteExportList( pResData, pResData->pPairedList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pPairedList = 0;
    }
    if ( pResData->pUIEntries ) {
        OString sList( "uientries" );
        WriteExportList( pResData, pResData->pUIEntries, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pUIEntries = 0;
    }
    return sal_True;
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

sal_Bool Export::WriteExportList(ResData *pResData, ExportList *pExportList,
    const OString &rTyp, sal_Bool bCreateNew)
{
    OString sGID(pResData->sGId);
    if (sGID.isEmpty())
        sGID = pResData->sId;
    else {
        sGID += ".";
        sGID += pResData->sId;
        while (!sGID.isEmpty() && sGID[sGID.getLength() - 1] == '.') {
            sGID = sGID.copy(0, sGID.getLength() - 1);
        }
    }

    for ( size_t i = 0; pExportList != NULL && i < pExportList->size(); i++ )
    {
        ExportListEntry *pEntry = (*pExportList)[  i ];

        OString sLID;
        OString sText((*pEntry)[ SOURCE_LANGUAGE ] );

        // Strip PairList Line String
        if (rTyp.equalsIgnoreAsciiCase("pairedlist"))
        {
            sLID = GetPairedListID( sText );
            sText = GetPairedListString( sText );
        }
        else
        {
            sLID = OString::number(i + 1);
            sText = StripList( sText );
            if( sText == "\\\"" )
                sText = "\"";
        }
        ConvertExportContent(sText);
        common::writePoEntry(
            "Transex3", *aOutput.mPo, global::inputPathname,
            rTyp, sGID, sLID, OString(), sText);

        if ( bCreateNew )
            delete [] pEntry;
    }
    if ( bCreateNew )
        delete pExportList;

    return sal_True;
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
        OString sError(RTL_CONSTASCII_STRINGPARAM("GroupId > 255 chars"));
        printf("GroupID = %s\n", sFull.getStr());
        yyerror(sError.getStr());
    }

    return sFull.makeStringAndClear();
}

void Export::InsertListEntry(const OString &rText, const OString &rLine)
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
        (*pNew)[LIST_REFID] = OString::number(REFID_NONE);
        pList->push_back(pNew);
    }
    ExportListEntry *pCurEntry = (*pList)[ nListIndex ];

    // For paired list use the line to set proper lid
    if( nList == LIST_PAIRED ){
        (*pCurEntry)[ m_sListLang ] = rLine;
    }else
        (*pCurEntry)[ m_sListLang ] = rText;

    if ( m_sListLang.equalsIgnoreAsciiCase("en-US") ) {
        (*pCurEntry)[ SOURCE_LANGUAGE ] = rLine;

        pList->NewSourceLanguageListEntry();
    }

    nListIndex++;
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

bool Export::GetAllMergeEntrysOfList(ResData *pResData, std::vector<MergeEntrys*>& o_vMergeEntrys, ExportList*& o_pList )
{
    o_vMergeEntrys.clear();
    o_pList = 0;

    if (!pResData->sGId.isEmpty())
        pResData->sGId = pResData->sGId + OString('.');
    pResData->sGId = pResData->sGId + pResData->sId;

    // Find out the type of List
    MergeEntrys* pEntrysOfFirstItem = 0;
    sal_uInt16 nType = LIST_STRING;
    bool bPairedList = false;
    while( !pEntrysOfFirstItem && nType <= LIST_UIENTRIES )
    {
        switch ( nType )
        {
            case LIST_STRING : pResData->sResTyp = "stringlist"; o_pList = pResData->pStringList; bPairedList = false; break;
            case LIST_FILTER : pResData->sResTyp = "filterlist"; o_pList = pResData->pFilterList; bPairedList = false; break;
            case LIST_UIENTRIES : pResData->sResTyp = "uientries"; o_pList = pResData->pUIEntries;bPairedList = false; break;
            case LIST_ITEM : pResData->sResTyp = "itemlist"; o_pList = pResData->pItemList;       bPairedList = false; break;
            case LIST_PAIRED : pResData->sResTyp = "pairedlist"; o_pList = pResData->pPairedList; bPairedList = true;  break;
        }

        // Set matching pairedlist identifier
        if( bPairedList && pResData->pPairedList )
        {
            ExportListEntry* pListE = ( ExportListEntry* ) (*pResData->pPairedList)[ 0 ];
            pResData->sId = GetPairedListID ( (*pListE)[ SOURCE_LANGUAGE ] );
        }
        else
            pResData->sId = "1";

        pEntrysOfFirstItem = pMergeDataFile->GetMergeEntrys( pResData );
        ++nType;
    }

    if( !pEntrysOfFirstItem )
    {
        o_pList = 0;
        return false;
    }
    else
        nList = nType-1;

    sal_uInt16 nMaxIndex = 0;
    if ( o_pList )
    {
        nMaxIndex = o_pList->GetSourceLanguageListEntryCount();
    }
    /**
      * Check whether count of listentries match with count
      * of translated items. If not than write origin items
      * to the list to avoid mixed translations
      * (exclude pairedlist)
      */
    if( !bPairedList )
    {
        MergeEntrys* pEntrys;
        // MergeData contains longer list
        pResData->sId = OString::number(nMaxIndex+1);
        pEntrys = pMergeDataFile->GetMergeEntrys( pResData );
        if ( pEntrys )
            return false;
        // MergeData contains shorter list
        pResData->sId = OString::number(nMaxIndex);
        pEntrys = pMergeDataFile->GetMergeEntrys( pResData );
        if ( !pEntrys )
            return false;
        pResData->sId = "1";
    }

    o_vMergeEntrys.push_back(pEntrysOfFirstItem);

    for( sal_uInt16 nLIndex = 2; nLIndex <= nMaxIndex; ++nLIndex )
    {
        // Set matching pairedlist identifier
        if ( bPairedList )
        {
            ExportListEntry* pListE = ( ExportListEntry* )(*pResData->pPairedList)[ ( nLIndex ) -1 ];
            if( pListE )
            {
                pResData->sId = GetPairedListID ( (*pListE)[ SOURCE_LANGUAGE ] );
            }
        }
        else
            pResData->sId = OString::number(nLIndex);

        MergeEntrys* pEntrys = pMergeDataFile->GetMergeEntrys( pResData );
        if( pEntrys )
        {
           o_vMergeEntrys.push_back(pEntrys);
        }
    }
    return true;
 }

void Export::ResData2Output( MergeEntrys *pEntry, sal_uInt16 nType, const OString& rTextType )
{
    sal_Bool bAddSemicolon = sal_False;
    sal_Bool bFirst = sal_True;
    OString sCur;

    for( unsigned int n = 0; n < aLanguages.size(); n++ ){
        sCur = aLanguages[ n ];

        OString sText;
        sal_Bool bText = pEntry->GetText( sText, nType, sCur , sal_True );
        if ( bText && !sText.isEmpty() ) {
            OString sOutput;
            if ( bNextMustBeDefineEOL)  {
                if ( bFirst )
                    sOutput += "\t\\\n";
                else
                    sOutput += ";\t\\\n";
            }
            bFirst=sal_False;
            sOutput += "\t";

            sOutput += rTextType;

            if ( !sCur.equalsIgnoreAsciiCase("en-US") ) {
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
        OString sOutput( ";" );
        WriteToMerged( sOutput , false );
    }
}

void Export::MergeRest( ResData *pResData, sal_uInt16 nMode )
{
    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, global::inputPathname, false );
        aLanguages = pMergeDataFile->GetLanguages();

    }
    switch ( nMode ) {
        case MERGE_MODE_NORMAL : {
            MergeEntrys *pEntry = 0;
            if( pResData->bText || pResData->bQuickHelpText || pResData->bTitle )
                pEntry = pMergeDataFile->GetMergeEntrys( pResData );

            if ( pEntry ) {
                if ( pResData->bText )
                    ResData2Output( pEntry, STRING_TYP_TEXT, pResData->sTextTyp );

                if ( pResData->bQuickHelpText )
                    ResData2Output( pEntry, STRING_TYP_QUICKHELPTEXT, OString("QuickHelpText") );

                if ( pResData->bTitle )
                    ResData2Output( pEntry, STRING_TYP_TITLE, OString("Title") );
            }

            // Merge Lists

            if ( pResData->bList ) {
                OString sOldId = pResData->sId;
                OString sOldGId = pResData->sGId;
                OString sOldTyp = pResData->sResTyp;
                sal_uInt16 nOldListTyp = nList;

                OString sSpace;
                for ( sal_uInt16 i = 1; i < nLevel-1; i++ )
                    sSpace += "\t";

                std::vector<MergeEntrys*> vMergeEntryVector;
                ExportList* pList = 0;
                bool bTranslateList = GetAllMergeEntrysOfList(pResData, vMergeEntryVector, pList);

                if( pList )
                {
                    OString sCur;
                    for( unsigned int n = 0; n < aLanguages.size(); n++ )
                    {
                        sCur = aLanguages[ n ];

                        sal_uInt16 nLIndex = 0;
                        sal_uInt16 nMaxIndex = pList->GetSourceLanguageListEntryCount();
                        while( nLIndex < nMaxIndex )
                        {
                            if ( nLIndex == 0 )
                            {
                                OStringBuffer sHead;
                                if ( bNextMustBeDefineEOL )
                                    sHead.append("\\\n\t");
                                sHead.append(sSpace);
                                switch ( nList )
                                {
                                    case LIST_STRING:
                                        sHead.append("StringList ");
                                        break;
                                    case LIST_FILTER:
                                        sHead.append("FilterList ");
                                        break;
                                    case LIST_ITEM:
                                        sHead.append("ItemList ");
                                        break;
                                    case LIST_PAIRED:
                                        sHead.append("PairedList ");
                                        break;
                                    case LIST_UIENTRIES:
                                        sHead.append("UIEntries ");
                                        break;
                                }
                                sHead.append("[ ");
                                sHead.append(sCur);
                                sHead.append(" ] ");
                                if ( bDefine || bNextMustBeDefineEOL )
                                {
                                    sHead.append("= \\\n");
                                    sHead.append(sSpace);
                                    sHead.append("\t{\\\n\t");
                                }
                                else
                                {
                                    sHead.append("= \n");
                                    sHead.append(sSpace);
                                    sHead.append("\t{\n\t");
                                }
                                WriteToMerged(sHead.makeStringAndClear() , true);
                            }
                            OString sLine;
                            if ( pList && (*pList)[ nLIndex ] )
                                sLine = ( *(*pList)[ nLIndex ])[ SOURCE_LANGUAGE ];

                            if ( sLine.indexOf( '>' ) != -1 ) {
                                if (( nList != LIST_UIENTRIES ) &&
                                    (( sLine.indexOf( '{' ) == -1 ) ||
                                    ( sLine.indexOf( '{' ) >= sLine.indexOf( '"' ))) &&
                                    (( sLine.indexOf( '<' ) == -1 ) ||
                                    ( sLine.indexOf( '<' ) >= sLine.indexOf( '"' ))))
                                {
                                    sLine = sLine.replaceFirst("\"", "< \"" );
                                }
                            }

                            if( bTranslateList && nLIndex < vMergeEntryVector.size() )
                            {
                                OString sText;
                                sal_Bool bText;
                                bText = vMergeEntryVector[nLIndex]->GetText( sText, STRING_TYP_TEXT, sCur, sal_True );
                                if ( bText && !sText.isEmpty() )
                                {
                                    ConvertMergeContent( sText );
                                    OString sPre  = sLine.copy( 0 , sLine.indexOf('"') );
                                    OString sPost = sLine.copy( sLine.lastIndexOf('"') + 1 );
                                    sLine = sPre + sText + sPost;
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
                        if ( nLIndex > 0 ) {
                            OString sFooter;
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
                nList = nOldListTyp;
            }
        }
        break;
        case MERGE_MODE_LIST : {
        }
        break;
    }
    pParseQueue->bMflag = false;
}

void Export::SetChildWithText()
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
