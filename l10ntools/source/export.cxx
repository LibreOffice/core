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

#include <cstddef>
#include <cstring>

#include "comphelper/string.hxx"

#include "boost/scoped_ptr.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "common.hxx"
#include "export.hxx"
#include "helper.hxx"
#include "tokens.h"
#include <iostream>
#include <rtl/strbuf.hxx>

extern "C" { int yyerror( const char * ); }
extern "C" { int YYWarning( const char * ); }

using comphelper::string::getToken;
using comphelper::string::getTokenCount;

namespace {

rtl::OString sActFileName; //TODO
MergeDataFile * pMergeDataFile = 0; //TODO

namespace global {

bool mergeMode = false;
bool errorLog = true;
char const * prj = 0;
char const * prjRoot = 0;
char const * inputPathname = 0;
char const * outputPathname = 0;
char const * mergeSrc;
boost::scoped_ptr< Export > exporter;

}

void handleArguments(int argc, char ** argv) {
    for (int i = 1; i != argc; ++i) {
        if (std::strcmp(argv[i], "-e") == 0) {
            global::errorLog = false;
        } else if (std::strcmp(argv[i], "-i") == 0) {
            if (++i == argc) {
                global::inputPathname = 0; // no valid command line
                break;
            }
            global::inputPathname = argv[i];
        } else if (std::strcmp(argv[i], "-l") == 0) {
            if (++i == argc) {
                global::inputPathname = 0; // no valid command line
                break;
            }
            Export::sLanguages = argv[i];
        } else if (std::strcmp(argv[i], "-m") == 0) {
            if (++i == argc) {
                global::inputPathname = 0; // no valid command line
                break;
            }
            global::mergeSrc = argv[i];
            global::mergeMode = true;
        } else if (std::strcmp(argv[i], "-o") == 0) {
            if (++i == argc) {
                global::inputPathname = 0; // no valid command line
                break;
            }
            global::outputPathname = argv[i];
        } else if (std::strcmp(argv[i], "-p") == 0) {
            if (++i == argc) {
                global::inputPathname = 0; // no valid command line
                break;
            }
            global::prj = argv[i];
        } else if (std::strcmp(argv[i], "-r") == 0) {
            if (++i == argc) {
                global::inputPathname = 0; // no valid command line
                break;
            }
            global::prjRoot = argv[i];
        } else {
            global::inputPathname = 0; // no valid command line
            break;
        }
    }
    if (global::inputPathname == 0 || global::outputPathname == 0) {
        std::fprintf(
            stderr,
            ("Syntax: transex3 [-p Prj] [-r PrjRoot] -i FileIn -o FileOut"
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

    if (global::mergeMode) {
        global::exporter.reset(
            new Export(global::mergeSrc, global::outputPathname));
    } else {
        sActFileName =
            common::pathnameToken(global::inputPathname, global::prjRoot);
        global::exporter.reset(new Export(global::outputPathname));
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

void ResData::Dump(){
    printf("**************\nResData\n");
    printf("sPForm = %s , sResTyp = %s , sId = %s , sGId = %s , sHelpId = %s\n",
        sPForm.getStr(), sResTyp.getStr(), sId.getStr(),
        sGId.getStr(), sHelpId.getStr());

    rtl::OString a("*pStringList");
    rtl::OString b("*pUIEntries");
    rtl::OString c("*pFilterList");
    rtl::OString d("*pItemList");
    rtl::OString e("*pPairedList");
    rtl::OString f("sText");

    Export::DumpMap( f , sText );

    if( pStringList )   Export::DumpExportList( a , *pStringList );
    if( pUIEntries )    Export::DumpExportList( b , *pUIEntries );
    if( pFilterList )   Export::DumpExportList( c , *pFilterList );
    if( pItemList )     Export::DumpExportList( d , *pItemList );
    if( pPairedList )   Export::DumpExportList( e , *pPairedList );
    printf("\n");
}

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
            sId = helper::trimAscii(sId.copy(0, 255));
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
            pMergeDataFile = new MergeDataFile(sMergeSrc, global::inputPathname, global::errorLog);

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
        helper::searchAndReplaceAll(&sTestToken, "\t", rtl::OString());
        helper::searchAndReplaceAll(&sTestToken, " ", rtl::OString());
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
    else if (( nToken != RESSOURCE ) &&
            ( nToken != RESSOURCEEXPR ) &&
            ( nToken != SMALRESSOURCE ) &&
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
            while( helper::searchAndReplace(&sToken, "\r", " " ) != -1 ) {};
            while( helper::searchAndReplace(&sToken,  "\t", " " ) != -1 ) {};
            while( helper::searchAndReplace(&sToken, "  ", " " ) != -1 ) {};
            if( sToken.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("#define NO_LOCALIZE_EXPORT")) ){
                            bSkipFile = true;
                            return 0;
                        }
                        if ( bMergeMode )
                          WriteToMerged( sOrig , false );

                        return 0;


        case RSCDEFINE:
            bDefine = sal_True; // res. defined in macro

        case RESSOURCE:
        case RESSOURCEEXPR: {
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
            helper::searchAndReplaceAll(&sToken, "\n", rtl::OString());
            helper::searchAndReplaceAll(&sToken, "\r", rtl::OString());
            helper::searchAndReplaceAll(&sToken, "{", rtl::OString());
            helper::searchAndReplaceAll(&sToken, "\t", " ");
            sToken = helper::trimAscii(sToken);
            sal_Int32 n = 0;
            rtl::OString sTLower = sToken.getToken(0, ' ', n).toAsciiLowerCase();
            pResData->sResTyp = sTLower;
            rtl::OString sId( sToken.copy( pResData->sResTyp.getLength() + 1 ));
            rtl::OString sCondition;
            if ( sId.indexOf( '#' ) != -1 )
            {
                // between ResTyp, Id and paranthes is a precomp. condition
                sCondition = "#";
                n = 0;
                sId = sId.getToken(0, '#', n);
                sCondition += sId.getToken(0, '#', n);
            }
            n = 0;
            sId = sId.getToken(0, '/', n);
            CleanValue( sId );
            helper::searchAndReplaceAll(&sId, "\t", rtl::OString());
            pResData->SetId( sId, ID_LEVEL_IDENTIFIER );
            if (!sCondition.isEmpty())
            {
                Execute( CONDITION, "");  // execute the precomp. condition
            }
        }
        break;
        case SMALRESSOURCE: {
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
            helper::searchAndReplaceAll(&sToken, "\n", rtl::OString());
            helper::searchAndReplaceAll(&sToken, "\r", rtl::OString());
            helper::searchAndReplaceAll(&sToken, "{", rtl::OString());
            helper::searchAndReplaceAll(&sToken, "\t", rtl::OString());
            helper::searchAndReplaceAll(&sToken, " ", rtl::OString());
            helper::searchAndReplaceAll(&sToken, "\\", rtl::OString());
            sToken = sToken.toAsciiLowerCase();
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
            rtl::OString sKey = sToken.getToken(0, '=', n);
            helper::searchAndReplaceAll(&sKey, " ", rtl::OString());
            helper::searchAndReplaceAll(&sKey, "\t", rtl::OString());
            rtl::OString sValue = sToken.getToken(0, '=', n);
            CleanValue( sValue );
            sKey = sKey.toAsciiUpperCase();
            if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("IDENTIFIER")))
            {
                rtl::OString sId(sValue);
                helper::searchAndReplaceAll(&sId, "\t", rtl::OString());
                helper::searchAndReplaceAll(&sId, " ", rtl::OString());
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
            if ( (sToken.indexOf( '{' ) != -1) &&
                ( getTokenCount(sToken, '{') > getTokenCount(sToken, '}') ))
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
                comphelper::string::remove(sToken, ' ').toAsciiLowerCase());
            sal_Int32 nPos = sTmpToken.indexOf("[en-us]=");
            if (nPos != -1) {
                rtl::OString sKey(sTmpToken.copy(0 , nPos));
                sKey = comphelper::string::remove(sKey, ' ');
                sKey = comphelper::string::remove(sKey, '\t');
                rtl::OString sValue = getToken(sToken, 1, '=');
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
                rtl::OString sKey = getToken(sToken, 0, '[');
                sKey = comphelper::string::remove(sKey, ' ');
                sKey = comphelper::string::remove(sKey, '\t');
                sKey = sKey.toAsciiUpperCase();
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
                    rtl::OString sLang = getToken(getToken(sToken, 1, '['), 0, ']');
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
                rtl::OString sEntry(getToken(sToken, 1, '\"'));
                if ( getTokenCount(sToken, '\"') > 3 )
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
                rtl::OString sKey = getToken(getToken(sToken, 0, '='), 0, '[');
                sKey = comphelper::string::remove(sKey, ' ');
                sKey = comphelper::string::remove(sKey, '\t');
                rtl::OString sText( GetText( sToken, nToken ));
                rtl::OString sLang;
                if ( getToken(sToken, 0, '=').indexOf('[') != -1 )
                {
                    sLang = getToken(getToken(getToken(sToken, 0, '='), 1, '['), 0, ']');
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
            using comphelper::string::replace;

            bDontWriteOutput = sal_False;
            // this is a AppfontMapping, so look if its a definition
            // of field size
            rtl::OString sKey = getToken(sToken, 0, '=');
            sKey = comphelper::string::remove(sKey, ' ');
            sKey = comphelper::string::remove(sKey, '\t');
            rtl::OString sMapping = getToken(sToken, 1, '=');
            sMapping = getToken(sMapping, 1, '(');
            sMapping = getToken(sMapping, 0, ')');
            sMapping = replace(sMapping, rtl::OString(' '), rtl::OString());
            sMapping = replace(sMapping, rtl::OString('\t'), rtl::OString());
            sKey = sKey.toAsciiUpperCase();
            if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("SIZE")))
                pResData->nWidth = getToken(sMapping, 0, ',').toInt32();
            else if (sKey.equalsL(RTL_CONSTASCII_STRINGPARAM("POSSIZE")))
                pResData->nWidth = getToken(sMapping, 2, ',').toInt32();
        }
        break;
        case RSCDEFINELEND:
            bDontWriteOutput = sal_False;
        break;
        case CONDITION: {
            bDontWriteOutput = sal_False;
            while( helper::searchAndReplace(&sToken, "\r", " " ) != -1 ) {};
            while( helper::searchAndReplace(&sToken, "\t", " " ) != -1 ) {};
            while( helper::searchAndReplace(&sToken, "  ", " " ) != -1 ) {};
            rtl::OString sCondition(getToken(sToken, 0, ' '));
            if ( sCondition == "#ifndef" ) {
                sActPForm = "!defined ";
                sActPForm += getToken(sToken, 1, ' ');
            }
            else if ( sCondition == "#ifdef" ) {
                sActPForm = "defined ";
                sActPForm += getToken(sToken, 1, ' ');
            }
            else if ( sCondition == "#if" ) {
                sActPForm = sToken.copy( 4 );
                while ( helper::searchAndReplace(&sActPForm, "||", "\\or" ) != -1 ) {};
            }
            else if ( sCondition == "#elif" ) {
                sActPForm = sToken.copy( 6 );
                while ( helper::searchAndReplace(&sActPForm, "||", "\\or" ) != -1 ) {};
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
        rtl::OString sWork(rText);
        helper::searchAndReplaceAll(&sWork, "\\\"", "XX");
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

        rtl::OString sCur;

        for( unsigned int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];
                if (!sCur.equalsIgnoreAsciiCase("x-comment") ){
                    if (!pResData->sText[ sCur ].isEmpty())
                        sXText = pResData->sText[ sCur ];
                    else {
                        sXText = pResData->sText[ SOURCE_LANGUAGE ];
                    }

                    if (!pResData->sHelpText[ sCur ].isEmpty())
                        sXHText = pResData->sHelpText[ sCur ];
                    else {
                        sXHText = pResData->sHelpText[ SOURCE_LANGUAGE ];
                    }

                    if (!pResData->sQuickHelpText[ sCur ].isEmpty())
                        sXQHText = pResData->sQuickHelpText[ sCur ];
                    else {
                        sXQHText = pResData->sQuickHelpText[ SOURCE_LANGUAGE ];
                    }

                    if (!pResData->sTitle[ sCur ].isEmpty())
                        sXTitle = pResData->sTitle[ sCur ];
                    else
                        sXTitle = pResData->sTitle[ SOURCE_LANGUAGE ];

                    if (sXText.isEmpty())
                        sXText = "-";

                    if (sXHText.isEmpty())
                    {
                        if (!pResData->sHelpText[ SOURCE_LANGUAGE ].isEmpty())
                            sXHText = pResData->sHelpText[ SOURCE_LANGUAGE ];
                    }
                }
                else
                    sXText = pResData->sText[ sCur ];

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
                sOutput += sCur; sOutput += "\t";


                sOutput += sXText; sOutput  += "\t";
                sOutput += sXHText; sOutput += "\t";
                sOutput += sXQHText; sOutput+= "\t";
                sOutput += sXTitle; sOutput += "\t";

                aOutput << sOutput.getStr() << '\n';

                if ( bCreateNew ) {
                    pResData->sText[ sCur ]         = "";
                    pResData->sHelpText[ sCur ]     = "";
                    pResData->sQuickHelpText[ sCur ]= "";
                    pResData->sTitle[ sCur ]        = "";
                }
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
    return helper::trimAscii(
        getToken(rText, 1, ';').toAsciiUpperCase().replace('\t', ' '));
}

rtl::OString Export::GetPairedListString(const rtl::OString& rText)
{
// < "STRING" ; IDENTIFIER ; > ;
    rtl::OString sString(getToken(rText, 0, ';').replace('\t', ' '));
    sString = comphelper::string::stripEnd(sString, ' ');
    rtl::OString s1(sString.copy(sString.indexOf('"') + 1));
    sString = s1.copy(0, s1.lastIndexOf('"'));
    sString = comphelper::string::stripEnd(sString, ' ');
    sString = comphelper::string::stripStart(sString, ' ');
    return sString;
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
        sGID = comphelper::string::stripEnd(sGID, '.');
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
        if( nList == LIST_PAIRED ){
            (*pCurEntry)[ SOURCE_LANGUAGE ] = rLine;
        }
        else
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
            rtl::OString sTmp(rSource.copy(rSource.indexOf("=")));
            CleanValue( sTmp );
            sTmp = comphelper::string::remove(sTmp, '\n');
            sTmp = comphelper::string::remove(sTmp, '\r');

            helper::searchAndReplaceAll(
                &sTmp, "\\\\\"", "-=<[BSlashBSlashHKom]>=-\"");
            helper::searchAndReplaceAll(&sTmp, "\\\"", "-=<[Hochkomma]>=-");
            helper::searchAndReplaceAll(&sTmp, "\\", "-=<[0x7F]>=-");
            helper::searchAndReplaceAll(&sTmp, "\\0x7F", "-=<[0x7F]>=-");

            sal_uInt16 nState = TXT_STATE_TEXT;
            for (sal_Int32 i = 1; i < getTokenCount(sTmp, '"'); ++i)
            {
                rtl::OString sToken(getToken(sTmp, i, '"'));
                if (!sToken.isEmpty()) {
                    if ( nState == TXT_STATE_TEXT ) {
                        sReturn += sToken;
                        nState = TXT_STATE_MACRO;
                    }
                    else {
                        sToken = sToken.replace('\t', ' ');
                        while (helper::searchAndReplace(&sToken, "  ", " ")
                               != -1)
                        {}
                        sToken = comphelper::string::stripStart(sToken, ' ');
                        sToken = comphelper::string::stripEnd(sToken, ' ');
                        if (!sToken.isEmpty()) {
                            sReturn += "\\\" ";
                            sReturn += sToken;
                            sReturn += " \\\"";
                        }
                        nState = TXT_STATE_TEXT;
                    }
                }
            }

            helper::searchAndReplaceAll(&sReturn, "-=<[0x7F]>=-", "");
            helper::searchAndReplaceAll(&sReturn, "-=<[Hochkomma]>=-", "\"");
            helper::searchAndReplaceAll(
                &sReturn, "-=<[BSlashBSlashHKom]>=-", "\\\\");
            helper::searchAndReplaceAll(
                &sReturn, "\\\\", "-=<[BSlashBSlash]>=-");
            helper::searchAndReplaceAll(&sReturn, "-=<[BSlashBSlash]>=-", "\\");
        }
        break;
    }
    return sReturn;
}

void Export::WriteToMerged(const rtl::OString &rText , bool bSDFContent)
{
    rtl::OString sText(rText);
    while (helper::searchAndReplace(&sText, " \n", "\n") != -1) {}
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
    sal_Bool bNoClose = !helper::endsWith(rText, "\\\"");
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
            nStart = rText.indexOf( "=" );
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
        pMergeDataFile = new MergeDataFile( sMergeSrc, global::inputPathname, global::errorLog );

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
        pMergeDataFile = new MergeDataFile( sMergeSrc, global::inputPathname, global::errorLog );

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
                                        helper::searchAndReplace(&sLine, "\"", "< \"" );
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
                    helper::searchAndReplace(&sLine, "\"", "< \"" );
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
          bStart( false ) ,
          bStartNext( false )
{
          aQueueNext = new std::queue<QueueEntry>;
          aQueueCur  = new std::queue<QueueEntry>;
}


ParserQueue::~ParserQueue(){
    if( aQueueNext )    delete aQueueNext;
    if( aQueueCur )     delete aQueueCur;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
