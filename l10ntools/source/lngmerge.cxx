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
#include <fstream>
#include <iterator>
#include <string>

#include "po.hxx"
#include "lngmerge.hxx"

namespace {

OString getBracketedContent(OString text) {
    return text.getToken(1, '[').getToken(0, ']');
}

static void lcl_RemoveUTF8ByteOrderMarker( OString &rString )
{
    if( rString.getLength() >= 3 && rString[0] == '\xEF' &&
        rString[1] == '\xBB' && rString[2] == '\xBF' )
    {
        rString = rString.copy(3);
    }
}

}

//
// class LngParser
//
LngParser::LngParser(const OString &rLngFile,
    sal_Bool bULFFormat)
    : nError( LNG_OK )
    , pLines( NULL )
    , sSource( rLngFile )
    , bULF( bULFFormat )
{
    pLines = new LngLineList();
    std::ifstream aStream(sSource.getStr());
    if (aStream.is_open())
    {
        bool bFirstLine = true;
        std::string s;
        std::getline(aStream, s);
        while (!aStream.eof())
        {
            OString sLine(s.data(), s.length());

            if( bFirstLine )
            {
                // Always remove UTF8 BOM from the first line
                lcl_RemoveUTF8ByteOrderMarker( sLine );
                bFirstLine = false;
            }

            pLines->push_back( new OString(sLine) );
            std::getline(aStream, s);
        }
        pLines->push_back( new OString() );
    }
    else
        nError = LNG_COULD_NOT_OPEN;
}

LngParser::~LngParser()
{
    for ( size_t i = 0, n = pLines->size(); i < n; ++i )
        delete (*pLines)[ i ];
    pLines->clear();
    delete pLines;
}

sal_Bool LngParser::CreatePO(
    const OString &rPOFile,
    const OString &rLanguage )
{
    aLanguages.push_back(rLanguage);
    PoOfstream aPOStream( rPOFile, PoOfstream::APP );
    if (!aPOStream.isOpen()) {
        std::cerr << "Ulfex error: Can't open po file:" << rPOFile.getStr() << "\n";
    }

    size_t nPos  = 0;
    sal_Bool bStart = true;
    OString sGroup, sLine;
    OStringHashMap Text;
    OString sID;

    while( nPos < pLines->size() ) {
        sLine = *(*pLines)[ nPos++ ];
        while( nPos < pLines->size() && !isNextGroup( sGroup , sLine ) ) {
            ReadLine( sLine , Text );
            sID = sGroup;
            sLine = *(*pLines)[ nPos++ ];
        };
        if( bStart ) {
            bStart = false;
            sID = sGroup;
        }
        else {
            WritePO( aPOStream , Text , sSource , sID );
        }
    }
    aPOStream.close();
    return true;
}

void LngParser::WritePO(PoOfstream &aPOStream,
    OStringHashMap &rText_inout, const OString &rActFileName,
    const OString &rID)
{

   sal_Bool bExport = true;
   if ( bExport ) {
       OString sCur;
       for( unsigned int n = 0; n < aLanguages.size(); n++ ){
           sCur = aLanguages[ n ];
           OString sAct = rText_inout[ sCur ];
           if ( sAct.isEmpty() && !sCur.isEmpty() )
               sAct = rText_inout[ OString("en-US") ];

           common::writePoEntry(
                "Ulfex", aPOStream, rActFileName, "LngText",
                rID, OString(), OString(), sAct);
       }
   }
}

bool LngParser::isNextGroup(OString &sGroup_out, const OString &sLine_in)
{
    const OString sLineTrim = sLine_in.trim();
    if ((sLineTrim[0] == '[') && (sLineTrim[sLineTrim.getLength() - 1] == ']'))
    {
        sGroup_out = getBracketedContent(sLineTrim).trim();
        return true;
    }
    return false;
}

void LngParser::ReadLine(const OString &rLine_in,
        OStringHashMap &rText_inout)
{
    if (!rLine_in.match(" *") && !rLine_in.match("/*"))
    {
        OString sLang(rLine_in.getToken(0, '=').trim());
        if (!sLang.isEmpty()) {
            OString sText(rLine_in.getToken(1, '"'));
            rText_inout[sLang] = sText;
        }
    }
}

sal_Bool LngParser::Merge(
    const OString &rPOFile,
    const OString &rDestinationFile,
    const OString &rLanguage )
{
    std::ofstream aDestination(
        rDestinationFile.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!aDestination.is_open()) {
        nError = LNG_COULD_NOT_OPEN;
    }
    nError = LNG_OK;

    MergeDataFile aMergeDataFile( rPOFile, sSource, false, true );
    if( rLanguage.equalsIgnoreAsciiCase("ALL") )
        aLanguages = aMergeDataFile.GetLanguages();

    size_t nPos = 0;
    sal_Bool bGroup = sal_False;
    OString sGroup;

    // seek to next group
    while ( nPos < pLines->size() && !bGroup )
    {
        OString sLine( *(*pLines)[ nPos ] );
        sLine = sLine.trim();
        if (!sLine.isEmpty() &&
            ( sLine[0] == '[' ) &&
            ( sLine[sLine.getLength() - 1] == ']' ))
        {
            sGroup = getBracketedContent(sLine).trim();
            bGroup = sal_True;
        }
        nPos ++;
    }

    while ( nPos < pLines->size()) {
        OStringHashMap Text;
        OString sID( sGroup );
        std::size_t nLastLangPos = 0;

        ResData  *pResData = new ResData( "", sID , sSource );
        pResData->sResTyp = "LngText";
        PFormEntrys *pEntrys = aMergeDataFile.GetPFormEntrys( pResData );
        // read languages
        bGroup = sal_False;

        OString sLanguagesDone;

        while ( nPos < pLines->size() && !bGroup )
        {
            OString sLine( *(*pLines)[ nPos ] );
            sLine = sLine.trim();
            if (!sLine.isEmpty() &&
                ( sLine[0] == '[' ) &&
                ( sLine[sLine.getLength() - 1] == ']' ))
            {
                sGroup = getBracketedContent(sLine).trim();
                bGroup = sal_True;
                nPos ++;
                sLanguagesDone = "";
            }
            else
            {
                sal_Int32 n = 0;
                OString sLang(sLine.getToken(0, '=', n));
                if (n == -1 || static_cast<bool>(sLine.match("/*")))
                {
                    ++nPos;
                }
                else
                {
                    sLang = sLang.trim();

                    OString sSearch( ";" );
                    sSearch += sLang;
                    sSearch += ";";

                    if (( sLanguagesDone.indexOf( sSearch ) != -1 )) {
                        LngLineList::iterator it = pLines->begin();
                        std::advance( it, nPos );
                        pLines->erase( it );
                    }
                    if( bULF && pEntrys )
                    {
                        if( !sLang.isEmpty() )
                        {
                            OString sNewText;
                            pEntrys->GetText( sNewText, STRING_TYP_TEXT, sLang, sal_True );
                            if( sLang == "qtz" )
                                sNewText = sNewText.copy(sNewText.indexOf("|") + 2);

                            if ( !sNewText.isEmpty()) {
                                OString *pLine = (*pLines)[ nPos ];

                                OString sText1( sLang );
                                sText1 += " = \"";
                                // escape quotes, unescape double escaped quotes fdo#56648
                                sText1 += sNewText.replaceAll("\"","\\\"").replaceAll("\\\\\"","\\\"");
                                sText1 += "\"";
                                *pLine = sText1;
                                Text[ sLang ] = sNewText;
                            }
                        }
                        nLastLangPos = nPos;
                        nPos ++;
                        sLanguagesDone += sSearch;
                    }
                    else {
                        nLastLangPos = nPos;
                        nPos ++;
                        sLanguagesDone += sSearch;
                    }
                }
            }
        }
        OString sCur;
        if ( nLastLangPos )
        {
            for(size_t n = 0; n < aLanguages.size(); ++n)
            {
                sCur = aLanguages[ n ];
                if( !sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")) && Text[sCur].isEmpty() && pEntrys )
                {

                    OString sNewText;
                    pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur, sal_True );
                    if( sCur == "qtz" )
                        sNewText = sNewText.copy(sNewText.indexOf("|") + 2);
                    if (( !sNewText.isEmpty()) &&
                        !(( sCur.equalsL(RTL_CONSTASCII_STRINGPARAM("x-comment"))) && ( sNewText == "-" )))
                    {
                        OString sLine;
                        sLine += sCur;
                        sLine += " = \"";
                        // escape quotes, unescape double escaped quotes fdo#56648
                        sLine += sNewText.replaceAll("\"","\\\"").replaceAll("\\\\\"","\\\"");
                        sLine += "\"";

                        nLastLangPos++;
                        nPos++;

                        if ( nLastLangPos < pLines->size() ) {
                            LngLineList::iterator it = pLines->begin();
                            std::advance( it, nLastLangPos );
                            pLines->insert( it, new OString(sLine) );
                        } else {
                            pLines->push_back( new OString(sLine) );
                        }
                    }
                }
            }
        }

        delete pResData;
    }

    for ( size_t i = 0; i < pLines->size(); ++i )
        aDestination << (*pLines)[i]->getStr() << '\n';

    aDestination.close();
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
