/*************************************************************************
 *
 *  $RCSfile: textsearch.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-17 08:12:55 $
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
#ifndef _UNOTOOLS_TEXTSEARCH_HXX
#define _UNOTOOLS_TEXTSEARCH_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XTEXTSEARCH_HPP_
#include <com/sun/star/util/XTextSearch.hpp>
#endif

// Forward-Deklaration
class CharClass;

namespace com {
    namespace sun {
        namespace star {
            namespace util {
                struct SearchResult;
                struct SearchOptions;
            }
        }
    }
}


// SS - Klasse fuers Suchen
class SearchParam
{
public:
    enum SearchType{ SRCH_NORMAL, SRCH_REGEXP, SRCH_LEVDIST };

private:
    String sSrchStr;            // the search string
    String sReplaceStr;         // the replace string

    SearchType eSrchType;       // search normal/regular/LevDist

    int bWordOnly   : 1;        // used by normal search
    int bSrchInSel  : 1;        // search only in the selection
    int bCaseSense  : 1;        //

    // values for the "weight Levenshtein-Distance"
    int bLEV_Relaxed : 1;
    int nLEV_OtherX;
    int nLEV_ShorterY;
    int nLEV_LongerZ;

public:
    SearchParam( const String &rText,
                    SearchType eSrchType = SearchParam::SRCH_NORMAL,
                    BOOL bCaseSens = TRUE,
                    BOOL bWrdOnly = FALSE,
                    BOOL bSrchInSel = FALSE );
    SearchParam( const SearchParam& );

    const String&   GetSrchStr() const          { return sSrchStr; }
    const String&   GetReplaceStr() const       { return sReplaceStr; }
    SearchType      GetSrchType() const         { return eSrchType; }

    int             IsCaseSensitive() const     { return bCaseSense; }
    int             IsSrchInSelection() const   { return bSrchInSel; }
    int             IsSrchWordOnly() const      { return bWordOnly; }


    void SetSrchStr( const String& rStr )       { sSrchStr = rStr; }
    void SetReplaceStr( const String& rStr )    { sReplaceStr = rStr; }
    void SetSrchType( SearchType eType )        { eSrchType = eType; }

    void SetCaseSensitive( int bFlag )          { bCaseSense = bFlag; }
    void SetSrchInSelection( int bFlag )        { bSrchInSel = bFlag; }
    void SetSrchWordOnly( int bFlag )           { bWordOnly = bFlag; }

    int             IsSrchRelaxed() const       { return bLEV_Relaxed; }
    int             GetLEVOther() const         { return nLEV_OtherX; }
    int             GetLEVShorter() const       { return nLEV_ShorterY; }
    int             GetLEVLonger() const        { return nLEV_LongerZ; }

    void SetSrchRelaxed( int bFlag )            { bLEV_Relaxed = bFlag; }
    void SetLEVOther( int nValue )              { nLEV_OtherX = nValue; }
    void SetLEVShorter( int nValue )            { nLEV_ShorterY = nValue; }
    void SetLEVLonger( int nValue )             { nLEV_LongerZ = nValue; }
};

//  Klasse zum Suchen eines Strings in einem String.
//  Unterstuetzt werden folgende Verfahren:
//      - normalen Text (Bayer/Moore)
//      - regulaere Ausdruecke
//      - gewichtete Levenshtein Distanz
//
//  Es kann Vorwaerts und Rueckwaerts gesucht werden!

class TextSearch
{
    com::sun::star::uno::Reference < com::sun::star::util::XTextSearch >
            xTextSearch;

    void Init( const SearchParam & rParam,
               const ::com::sun::star::lang::Locale& rLocale );

public:
    // rText ist der zusuchende String
    // this first two CTORs are deprecated!
    TextSearch( const SearchParam & rPara, ULONG nLanguage );
    TextSearch( const SearchParam & rPara, const CharClass& rCClass );

    TextSearch( const ::com::sun::star::util::SearchOptions& rPara );
    ~TextSearch();

    /* search in the (selected) text the search string:
        rScrTxt - the text, in in which we search
        pStart  - start position for the saerch
        pEnde   - end postion for the search

        RETURN values   ==  TRUE: something is found
                        - pStart start pos of the found text,
                        - pStart end pos of the found text,
                        - pSrchResult - the search reult with all found
                             positons. Is only filled with more positions
                             if the regular expression handles goups.

                        == FALSE: nothing found, pStart,pEnde unchanged.

        Definitions: start pos always inclusive, end pos always exclusive!
                     The position must always in the right direction!
                    search forward: start <= end
                    search backward: end <= start
    */
    int SearchFrwrd( const String &rStr,
                    xub_StrLen* pStart, xub_StrLen* pEnde,
                    ::com::sun::star::util::SearchResult* pSrchResult = 0 );
    int SearchBkwrd( const String &rStr,
                    xub_StrLen* pStart, xub_StrLen* pEnde,
                    ::com::sun::star::util::SearchResult* pSrchResult = 0 );
};


#endif

