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
#include <fstream>
#include <iterator>
#include <string>

#include "common.hxx"
#include "lngmerge.hxx"

namespace {

rtl::OString getBracketedContent(rtl::OString text) {
    return text.getToken(1, '[').getToken(0, ']');
}

}

//
// class LngParser
//
LngParser::LngParser(const rtl::OString &rLngFile,
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
        while (!aStream.eof())
        {
            std::string s;
            std::getline(aStream, s);
            rtl::OString sLine(s.data(), s.length());

            if( bFirstLine )
            {
                // Always remove UTF8 BOM from the first line
                Export::RemoveUTF8ByteOrderMarker( sLine );
                bFirstLine = false;
            }

            pLines->push_back( new rtl::OString(sLine) );
        }
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

sal_Bool LngParser::CreateSDF(const rtl::OString &rSDFFile,
    const rtl::OString &rPrj, const rtl::OString &rRoot)
{

    Export::InitLanguages( false );
    aLanguages = Export::GetLanguages();
    std::ofstream aSDFStream(
        rSDFFile.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!aSDFStream.is_open()) {
        nError = SDF_COULD_NOT_OPEN;
    }
    nError = SDF_OK;
    rtl::OString sActFileName(
        common::pathnameToken(sSource.getStr(), rRoot.getStr()));

    size_t nPos  = 0;
    sal_Bool bStart = true;
    rtl::OString sGroup, sLine;
    OStringHashMap Text;
    rtl::OString sID;

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
            WriteSDF( aSDFStream , Text , rPrj , rRoot , sActFileName , sID );
        }
    }
    aSDFStream.close();
    return true;
}

void LngParser::WriteSDF(std::ofstream &aSDFStream,
    OStringHashMap &rText_inout, const rtl::OString &rPrj,
    const rtl::OString &rRoot, const rtl::OString &rActFileName,
    const rtl::OString &rID)
{

   sal_Bool bExport = true;
   if ( bExport ) {
       rtl::OString sCur;
       for( unsigned int n = 0; n < aLanguages.size(); n++ ){
           sCur = aLanguages[ n ];
           rtl::OString sAct = rText_inout[ sCur ];
           if ( sAct.isEmpty() && !sCur.isEmpty() )
               sAct = rText_inout[ rtl::OString("en-US") ];

           rtl::OString sOutput( rPrj ); sOutput += "\t";
           if (rRoot.getLength())
               sOutput += rActFileName;
           sOutput += "\t0\t";
           sOutput += "LngText\t";
           sOutput += rID; sOutput += "\t\t\t\t0\t";
           sOutput += sCur; sOutput += "\t";
           sOutput += sAct; sOutput += "\t\t\t\t";
           aSDFStream << sOutput.getStr() << '\n';
       }
   }
}

bool LngParser::isNextGroup(rtl::OString &sGroup_out, rtl::OString &sLine_in)
{
    sLine_in = sLine_in.trim();
    if ((sLine_in[0] == '[') && (sLine_in[sLine_in.getLength() - 1] == ']'))
    {
        sGroup_out = getBracketedContent(sLine_in).trim();
        return true;
    }
    return false;
}

void LngParser::ReadLine(const rtl::OString &rLine_in,
        OStringHashMap &rText_inout)
{
    rtl::OString sLang(rLine_in.getToken(0, '=').trim());
    if (!sLang.isEmpty()) {
        rtl::OString sText(rLine_in.getToken(1, '"'));
        rText_inout[sLang] = sText;
    }
}

sal_Bool LngParser::Merge(
    const rtl::OString &rSDFFile,
    const rtl::OString &rDestinationFile)
{
    Export::InitLanguages( true );
    std::ofstream aDestination(
        rDestinationFile.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!aDestination.is_open()) {
        nError = LNG_COULD_NOT_OPEN;
    }
    nError = LNG_OK;

    MergeDataFile aMergeDataFile( rSDFFile, sSource, false );
    rtl::OString sTmp( Export::sLanguages );
    if( sTmp.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("ALL")) )
        Export::SetLanguages( aMergeDataFile.GetLanguages() );
    aLanguages = Export::GetLanguages();

    size_t nPos = 0;
    sal_Bool bGroup = sal_False;
    rtl::OString sGroup;

    // seek to next group
    while ( nPos < pLines->size() && !bGroup )
    {
        rtl::OString sLine( *(*pLines)[ nPos ] );
        sLine = sLine.trim();
        if (( sLine[0] == '[' ) &&
            ( sLine[sLine.getLength() - 1] == ']' ))
        {
            sGroup = getBracketedContent(sLine).trim();
            bGroup = sal_True;
        }
        nPos ++;
    }

    while ( nPos < pLines->size()) {
        OStringHashMap Text;
        rtl::OString sID( sGroup );
        std::size_t nLastLangPos = 0;

        ResData  *pResData = new ResData( "", sID , sSource );
        pResData->sResTyp = "LngText";
        PFormEntrys *pEntrys = aMergeDataFile.GetPFormEntrys( pResData );
        // read languages
        bGroup = sal_False;

        rtl::OString sLanguagesDone;

        while ( nPos < pLines->size() && !bGroup )
        {
            rtl::OString sLine( *(*pLines)[ nPos ] );
            sLine = sLine.trim();
            if (( sLine[0] == '[' ) &&
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
                rtl::OString sLang(sLine.getToken(0, '=', n));
                if (n == -1)
                {
                    ++nPos;
                }
                else
                {
                    sLang = sLang.trim();

                    rtl::OString sSearch( ";" );
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
                            rtl::OString sNewText;
                            pEntrys->GetText( sNewText, STRING_TYP_TEXT, sLang, sal_True );

                            if ( !sNewText.isEmpty()) {
                                rtl::OString *pLine = (*pLines)[ nPos ];

                                rtl::OString sText1( sLang );
                                sText1 += " = \"";
                                sText1 += sNewText;
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
        rtl::OString sCur;
        if ( nLastLangPos )
        {
            for(size_t n = 0; n < aLanguages.size(); ++n)
            {
                sCur = aLanguages[ n ];
                if( !sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")) && Text[sCur].isEmpty() && pEntrys )
                {

                    rtl::OString sNewText;
                    pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur, sal_True );
                    if (( !sNewText.isEmpty()) &&
                        !(( sCur.equalsL(RTL_CONSTASCII_STRINGPARAM("x-comment"))) && ( sNewText == "-" )))
                    {
                        rtl::OString sLine;
                        sLine += sCur;
                        sLine += " = \"";
                        sLine += sNewText;
                        sLine += "\"";

                        nLastLangPos++;
                        nPos++;

                        if ( nLastLangPos < pLines->size() ) {
                            LngLineList::iterator it = pLines->begin();
                            std::advance( it, nLastLangPos );
                            pLines->insert( it, new rtl::OString(sLine) );
                        } else {
                            pLines->push_back( new rtl::OString(sLine) );
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
