/*************************************************************************
 *
 *  $RCSfile: textsearch.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-16 21:43:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HDL_
#include <com/sun/star/util/SearchFlags.hdl>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#include <comphelper/processfactory.hxx>
#include <unotools/textsearch.hxx>

#pragma hdrstop

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

SearchParam::SearchParam( const String &rText,
                                SearchType eType,
                                BOOL bCaseSensitive,
                                BOOL bWrdOnly,
                                BOOL bSearchInSel )
{
    sSrchStr        = rText;
    eSrchType       = eType;

    bWordOnly       = bWrdOnly;
    bSrchInSel      = bSearchInSel;
    bCaseSense      = bCaseSensitive;

    // Werte fuer "Gewichtete Levenshtein-Distanz"
    bLEV_Relaxed    = TRUE;
    nLEV_OtherX     = 2;
    nLEV_ShorterY   = 1;
    nLEV_LongerZ    = 3;
}

SearchParam::SearchParam( const SearchParam& rParam )
{
    sSrchStr        = rParam.sSrchStr;
    sReplaceStr     = rParam.sReplaceStr;
    eSrchType       = rParam.eSrchType;

    bWordOnly       = rParam.bWordOnly;
    bSrchInSel      = rParam.bSrchInSel;
    bCaseSense      = rParam.bCaseSense;

    bLEV_Relaxed    = rParam.bLEV_Relaxed;
    nLEV_OtherX     = rParam.nLEV_OtherX;
    nLEV_ShorterY   = rParam.nLEV_ShorterY;
    nLEV_LongerZ    = rParam.nLEV_LongerZ;
}

//  Klasse zum Suchen eines Strings in einem Text. Es wird genau nach
//  dem String gesucht.
//  ( Die Unterscheidung der Gross/Klein-Schreibung kann mit einen Flag
//  unterdrueckt werden )

TextSearch::TextSearch(const SearchParam & rParam, ULONG eLang )
{
    if( LANGUAGE_NONE == eLang )
        eLang = LANGUAGE_SYSTEM;
    String aLangStr, aCtryStr;
    ConvertLanguageToIsoNames( eLang, aLangStr, aCtryStr );

    Init( rParam,
        ::com::sun::star::lang::Locale( aLangStr, aCtryStr, String() ) );
}

TextSearch::TextSearch(const SearchParam & rParam, const CharClass& rCClass )
{
    Init( rParam, rCClass.getLocale() );
}

TextSearch::TextSearch( const SearchOptions& rPara )
{
    try
    {
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        xTextSearch = Reference< XTextSearch > ( xMSF->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.util.TextSearch" ) ) ), UNO_QUERY );
        xTextSearch->setOptions( rPara );
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "TextSearch ctor: Exception caught!" );
    }
}

void TextSearch::Init( const SearchParam & rParam,
                        const ::com::sun::star::lang::Locale& rLocale )
{
    // convert SearchParam to the UNO SearchOptions
    SearchOptions aSOpt;

    switch( rParam.GetSrchType() )
    {
    case SearchParam::SRCH_REGEXP:
        aSOpt.algorithmType = SearchAlgorithms_REGEXP;
        if( rParam.IsSrchInSelection() )
            aSOpt.searchFlag |= SearchFlags::REG_NOT_BEGINOFLINE |
                                SearchFlags::REG_NOT_ENDOFLINE;
        break;

    case SearchParam::SRCH_LEVDIST:
        aSOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
        aSOpt.changedChars = rParam.GetLEVOther();
        aSOpt.deletedChars = rParam.GetLEVLonger();
        aSOpt.insertedChars = rParam.GetLEVShorter();
        if( rParam.IsSrchRelaxed() )
            aSOpt.searchFlag |= SearchFlags::LEV_RELAXED;
        break;

//  case SearchParam::SRCH_NORMAL:
    default:
        aSOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
        if( rParam.IsSrchWordOnly() )
            aSOpt.searchFlag |= SearchFlags::NORM_WORD_ONLY;
        break;
    }
    aSOpt.searchString = rParam.GetSrchStr();
    aSOpt.replaceString = rParam.GetReplaceStr();
    aSOpt.Locale = rLocale;
    if( !rParam.IsCaseSensitive() )
        aSOpt.searchFlag |= SearchFlags::ALL_IGNORE_CASE;

    try
    {
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        xTextSearch = Reference< XTextSearch > ( xMSF->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.util.TextSearch" ) ) ), UNO_QUERY );
        xTextSearch->setOptions( aSOpt );
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "TextSearch ctor: Exception caught!" );
    }
}

TextSearch::~TextSearch()
{
}

/*
 * Die allgemeinen Methoden zu Suchen. Diese rufen dann die entpsrecheden
 * Methoden fuer die normale Suche oder der Suche nach Regular-Expressions
 * ueber die MethodenPointer auf.
 */

int TextSearch::SearchFrwrd( const String & rStr, xub_StrLen* pStart,
                            xub_StrLen* pEnde, SearchResult* pRes )
{
    int nRet = 0;
    try
    {
        if( xTextSearch.is() )
        {
            SearchResult aRet( xTextSearch->searchForward(
                                                    rStr, *pStart, *pEnde ));
            if( aRet.subRegExpressions )
            {
                nRet = 1;
                // the XTextsearch returns in startOffset the higher position
                // and the endposition is allways exclusive.
                // The caller of this function will have in startPos the
                // lower pos. and end
                *pStart = aRet.startOffset[ 0 ];
                *pEnde = aRet.endOffset[ 0 ];
                if( pRes )
                    *pRes = aRet;
            }
        }
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "SearchForward: Exception caught!" );
    }
    return nRet;
}

int TextSearch::SearchBkwrd( const String & rStr, xub_StrLen* pStart,
                            xub_StrLen* pEnde, SearchResult* pRes )
{
    int nRet = 0;
    try
    {
        if( xTextSearch.is() )
        {
            SearchResult aRet( xTextSearch->searchBackward(
                                                    rStr, *pStart, *pEnde ));
            if( aRet.subRegExpressions )
            {
                nRet = 1;
                // the XTextsearch returns in startOffset the higher position
                // and the endposition is allways exclusive.
                // The caller of this function will have in startPos the
                // lower pos. and end
                *pEnde = aRet.startOffset[ 0 ];
                *pStart = aRet.endOffset[ 0 ];
                if( pRes )
                    *pRes = aRet;
            }
        }
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "SearchBackward: Exception caught!" );
    }
    return nRet;
}


