/*************************************************************************
 *
 *  $RCSfile: txtcmp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:54 $
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
#ifndef _TXTCMP_HXX
#define _TXTCMP_HXX

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
class International;
class CharClass;

// SS - Klasse fuers Suchen
class SearchParam
{
public:
    enum SearchType{ SRCH_NORMAL, SRCH_REGEXP, SRCH_LEVDIST };

private:
    friend class SearchText;

    String sSrchStr;            // der Such-String
    String sReplaceStr;         // nur fuer SS - der Ersetzungs-String

    SearchType eSrchType;       // suche normal/regulaer/LevDist

    int bWordOnly   : 1;        // nur bei normaler Suche
    int bSrchInSel  : 1;        // Suche in vorgebener Grenze
    int bCaseSense  : 1;        // Gross-/Klein-Schreibung beachten

    // Werte fuer "Gewichtete Levenshtein-Distanz"
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


    // Methoden fuer "Gewichtete Levenshtein-Distanz"
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

class SearchText
{
    com::sun::star::uno::Reference < com::sun::star::util::XTextSearch >
            xTextSearch;
    BOOL bSearchRegular;

    void Init( const SearchParam & rParam,
               const ::com::sun::star::lang::Locale& rLocale );

public:
    // rText ist der zusuchende String
    SearchText(const SearchParam & rPara, const International & rTheIntnl);
    SearchText(const SearchParam & rPara, const CharClass& rCClass );
    ~SearchText();

    /* Suche im Text den initialen String:
        rScrTxt - der Text, worin geuscht wird.
        pStart  - Startposition im Text fuers Suchen
        nEnde   - Endposition im Text

        RETURN Werte    ==  TRUE: wenn gefunden gibt,
                        - pStart zeigt auf die Startposition im Text,
                        - pEnde zeigt auf die Endposition im Text

                        == FALSE: nicht gefunden, pStart,pEnde unveraendert.
    */
    int SearchFrwrd( const String &rStr, xub_StrLen* pStart, xub_StrLen* pEnde );
    int SearchBkwrd( const String &rStr, xub_StrLen* pStart, xub_StrLen* pEnde );
};

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/svtools/inc/txtcmp.hxx,v 1.1.1.1 2000-09-18 16:58:54 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.13  2000/09/18 14:13:18  willem.vandorp
      OpenOffice header added.

      Revision 1.12  2000/09/01 13:52:29  jp
      move search into INT - DLL

      Revision 1.11  2000/08/31 13:41:16  willem.vandorp
      Header and footer replaced

      Revision 1.10  2000/08/28 11:55:35  jp
      new: use CharClass instead of International

      Revision 1.9  2000/03/31 10:48:39  jp
      Changes for unicode

      Revision 1.8  2000/02/09 16:16:19  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.7  1999/08/24 13:56:27  sb
      #66082# Make use of International explicitly through ctor.

      Revision 1.6  1997/07/30 08:36:52  MH
      chg: header


      Rev 1.5   30 Jul 1997 10:36:52   MH
   chg: header

      Rev 1.4   18 Jun 1996 14:56:14   WP
   richtig includen

      Rev 1.3   25 Mar 1996 09:54:06   MT
   CLOOK-Vorbeugung

      Rev 1.2   27 Nov 1995 18:27:46   AB
   Unicode-Anpassung

      Rev 1.1   23 Dec 1994 11:44:02   JP
   endif ohne Kommentar

      Rev 1.0   01 Dec 1994 16:35:46   MT
   Initial revision.

*************************************************************************/

#endif
    // _TXTCMP_HXX
