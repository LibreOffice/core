/*************************************************************************
 *
 *  $RCSfile: tox.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-17 16:24:04 $
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


#pragma hdrstop


#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX
#include <svx/tstpitem.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

// -> #i21237#
#include <algorithm>
#include <functional>

using namespace std;
// <- #i21237#

const sal_Char* SwForm::aFormEntry      = "<E>";
const sal_Char* SwForm::aFormTab        = "<T>";
const sal_Char* SwForm::aFormPageNums   = "<#>";
const sal_Char* SwForm::aFormLinkStt    = "<LS>";
const sal_Char* SwForm::aFormLinkEnd    = "<LE>";
const sal_Char* SwForm::aFormEntryNum   = "<E#>";
const sal_Char* SwForm::aFormEntryTxt   = "<ET>";
const sal_Char* SwForm::aFormChapterMark= "<C>";
const sal_Char* SwForm::aFormText       = "<X>";
const sal_Char* SwForm::aFormAuth       = "<A>";
BYTE SwForm::nFormTabLen            = 3;
BYTE SwForm::nFormEntryLen          = 3;
BYTE SwForm::nFormPageNumsLen       = 3;
BYTE SwForm::nFormLinkSttLen        = 4;
BYTE SwForm::nFormLinkEndLen        = 4;
BYTE SwForm::nFormEntryNumLen       = 4;
BYTE SwForm::nFormEntryTxtLen       = 4;
BYTE SwForm::nFormChapterMarkLen    = 3;
BYTE SwForm::nFormTextLen           = 3;
BYTE SwForm::nFormAuthLen           = 5;

SV_IMPL_PTRARR(SwTOXMarks, SwTOXMark*)

TYPEINIT2( SwTOXMark, SfxPoolItem, SwClient );    // fuers rtti

/* -----------------23.09.99 14:09-------------------
    includes plain text at a given position into
    the appropriate token
 --------------------------------------------------*/
USHORT lcl_ConvertTextIntoPattern( String& rPattern,
                                    xub_StrLen nStart, xub_StrLen nEnd )
{
    String sTmp; sTmp.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "<X ,,"));
    sTmp += TOX_STYLE_DELIMITER;
    sTmp += rPattern.Copy( nStart, nEnd - nStart);
    rPattern.Erase( nStart, nEnd - nStart);
    sTmp += TOX_STYLE_DELIMITER;
    sTmp += '>';
    rPattern.Insert(sTmp, nStart);
    // return the offset of the new end position
    return 8;
}

/* -----------------23.09.99 13:59-------------------

 --------------------------------------------------*/
struct PatternIni
{
    USHORT n1;
    USHORT n2;
    USHORT n3;
    USHORT n4;
    USHORT n5;
};
const PatternIni aPatternIni[] =
{
    {USHRT_MAX},    //Header - no pattern
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},   //AUTH_TYPE_ARTICLE,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_BOOK,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_BOOKLET,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CONFERENCE,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_INBOOK,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_INCOLLECTION,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_INPROCEEDINGS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_JOURNAL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_MANUAL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_MASTERSTHESIS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_MISC,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_PHDTHESIS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_PROCEEDINGS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_TECHREPORT,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_UNPUBLISHED,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_EMAIL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, AUTH_FIELD_URL, USHRT_MAX},//AUTH_TYPE_WWW,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM1,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM2,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM3,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM4,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM5,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_YEAR,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_URL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM1,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM2,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM3,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM4,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM5,
    {USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX}
};
/* -----------------23.09.99 13:58-------------------

 --------------------------------------------------*/
// -> #i21237#
SwFormTokens lcl_GetAuthPattern(USHORT nTypeId)
{
    SwFormTokens aRet; // #i21237#


    PatternIni aIni = aPatternIni[nTypeId];
    USHORT nVals[5];
    nVals[0] = aIni.n1;
    nVals[1] = aIni.n2;
    nVals[2] = aIni.n3;
    nVals[3] = aIni.n4;
    nVals[4] = aIni.n5;

    for(USHORT i = 0; i < 5 ; i++)
    {
        if(nVals[i] == USHRT_MAX)
            break;
         // -> #i21237#
        SwFormToken aToken(TOKEN_AUTHORITY);

        aToken.nAuthorityField = nVals[i];
        aRet.push_back(aToken);
        // <- #i21237#
    }

    return aRet;
}
/*--------------------------------------------------------------------
     Beschreibung:  Verzeichnis-Markierungen D/Ctor
 --------------------------------------------------------------------*/


    // Konstruktor fuers Default vom Attribut-Pool
SwTOXMark::SwTOXMark()
    : SfxPoolItem( RES_TXTATR_TOXMARK ),
    SwClient( 0 ),
    pTxtAttr( 0 ),
    bAutoGenerated(FALSE),
    bMainEntry(FALSE)
{
}


SwTOXMark::SwTOXMark( const SwTOXType* pTyp )
    : SfxPoolItem( RES_TXTATR_TOXMARK ),
    SwClient( (SwModify*)pTyp ),
    pTxtAttr( 0 ), nLevel( 0 ),
    bAutoGenerated(FALSE),
    bMainEntry(FALSE)
{
}


SwTOXMark::SwTOXMark( const SwTOXMark& rCopy )
    : SfxPoolItem( RES_TXTATR_TOXMARK ),
    SwClient((SwModify*)rCopy.GetRegisteredIn()),
    pTxtAttr( 0 ), nLevel( rCopy.nLevel ),
    aPrimaryKey( rCopy.aPrimaryKey ), aSecondaryKey( rCopy.aSecondaryKey ),
    aTextReading( rCopy.aTextReading ),
    aPrimaryKeyReading( rCopy.aPrimaryKeyReading ),
    aSecondaryKeyReading( rCopy.aSecondaryKeyReading ),
    bAutoGenerated( rCopy.bAutoGenerated),
    bMainEntry(rCopy.bMainEntry)
{
    // AlternativString kopieren
    aAltText = rCopy.aAltText;
}


SwTOXMark::~SwTOXMark()
{
}


int SwTOXMark::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return GetRegisteredIn() == ((SwTOXMark&)rAttr).GetRegisteredIn();
}


SfxPoolItem* SwTOXMark::Clone( SfxItemPool* ) const
{
    return new SwTOXMark( *this );
}


String SwTOXMark::GetText() const
{
    String aStr;
    if( aAltText.Len() )
        aStr = aAltText;
    else if( pTxtAttr && pTxtAttr->GetpTxtNd() )
    {
        xub_StrLen* pEndIdx = pTxtAttr->GetEnd();
        ASSERT( pEndIdx, "TOXMark ohne Mark!!");
        if( pEndIdx )
        {
            const xub_StrLen nStt = *pTxtAttr->GetStart();
            aStr = pTxtAttr->GetpTxtNd()->GetExpandTxt( nStt, *pEndIdx-nStt );
        }
    }
    return aStr;
}

/*--------------------------------------------------------------------
     Beschreibung: Typen von Verzeichnissen verwalten
 --------------------------------------------------------------------*/

SwTOXType::SwTOXType( TOXTypes eTyp, const String& rName )
    : SwModify(0),
    aName(rName),
    eType(eTyp)
{
}


SwTOXType::SwTOXType(const SwTOXType& rCopy)
    : SwModify( (SwModify*)rCopy.GetRegisteredIn() ),
    aName(rCopy.aName),
    eType(rCopy.eType)
{
}

/*--------------------------------------------------------------------
    Beschreibung: Formen bearbeiten
  --------------------------------------------------------------------*/

SwForm::SwForm( USHORT nTyp ) // #i21237#
    : nType( nTyp ), nFormMaxLevel( SwForm::GetFormMaxLevel( nTyp )),
//  nFirstTabPos( lNumIndent ),
    bCommaSeparated(FALSE)
{
    //bHasFirstTabPos =
    bGenerateTabPos = FALSE;
    bIsRelTabPos = TRUE;

    // Inhaltsverzeichnis hat entsprechend Anzahl Headlines + Ueberschrift
    // Benutzer hat 10 Ebenen + Ueberschrift
    // Stichwort hat 3 Ebenen + Ueberschrift + Trenner
    // indexes of tables, objects illustrations and authorities consist of a heading and one level

    USHORT nPoolId;
    switch( nType )
    {
    case TOX_INDEX:         nPoolId = STR_POOLCOLL_TOX_IDXH;    break;
    case TOX_USER:          nPoolId = STR_POOLCOLL_TOX_USERH;   break;
    case TOX_CONTENT:       nPoolId = STR_POOLCOLL_TOX_CNTNTH;  break;
    case TOX_ILLUSTRATIONS: nPoolId = STR_POOLCOLL_TOX_ILLUSH;  break;
    case TOX_OBJECTS      : nPoolId = STR_POOLCOLL_TOX_OBJECTH; break;
    case TOX_TABLES       : nPoolId = STR_POOLCOLL_TOX_TABLESH; break;
    case TOX_AUTHORITIES  : nPoolId = STR_POOLCOLL_TOX_AUTHORITIESH;    break;
    default:
        ASSERT( !this, "ungueltiger TOXTyp");
        return ;
    }

    SwFormTokens aTokens;
    if (TOX_CONTENT == nType)
    {
        aTokens.push_back(SwFormToken(TOKEN_ENTRY_NO));
        aTokens.push_back(SwFormToken(TOKEN_ENTRY_TEXT));
    }
    else
        aTokens.push_back(SwFormToken(TOKEN_ENTRY));

    if (TOX_AUTHORITIES != nType)
    {
        SwFormToken aToken(TOKEN_TAB_STOP);
        aToken.nTabStopPosition = USHRT_MAX;

        if(TOX_CONTENT == nType || TOX_TABLES == nType ||
           TOX_ILLUSTRATIONS == nType || TOX_OBJECTS == nType)
        {
            aToken.cTabFillChar = '.';
            aToken.eTabAlign = SVX_TAB_ADJUST_END;
        }

        aTokens.push_back(aToken);
        aTokens.push_back(SwFormToken(TOKEN_PAGE_NUMS));
    }

    SetTemplate( 0, SW_RESSTR( nPoolId++ ));

    if(TOX_INDEX == nType)
    {
        for( USHORT i = 1; i < 5; ++i  )
        {
            if(1 == i)
            {
                SwFormTokens aTmpTokens;
                SwFormToken aTmpToken(TOKEN_ENTRY);
                aTmpTokens.push_back(aTmpToken);

                SetPattern( i, aTmpTokens );
                SetTemplate( i, SW_RESSTR( STR_POOLCOLL_TOX_IDXBREAK    ));
            }
            else
            {
                SetPattern( i, aTokens );
                SetTemplate( i, SW_RESSTR( STR_POOLCOLL_TOX_IDX1 + i - 2 ));
            }
        }
    }
    else
        for( USHORT i = 1; i < GetFormMax(); ++i, ++nPoolId )    // Nr 0 ist der Titel
        {
            if(TOX_AUTHORITIES == nType)
                SetPattern(i, lcl_GetAuthPattern(i));
            else
                SetPattern( i, aTokens );

            if( TOX_CONTENT == nType && 6 == i )
                nPoolId = STR_POOLCOLL_TOX_CNTNT6;
            else if( TOX_USER == nType && 6 == i )
                nPoolId = STR_POOLCOLL_TOX_USER6;
            else if( TOX_AUTHORITIES == nType )
                nPoolId = STR_POOLCOLL_TOX_AUTHORITIES1;
            SetTemplate( i, SW_RESSTR( nPoolId ) );
        }
}


SwForm::SwForm(const SwForm& rForm)
    : nType( rForm.nType )
{
    *this = rForm;
}


SwForm& SwForm::operator=(const SwForm& rForm)
{
    nType = rForm.nType;
    nFormMaxLevel = rForm.nFormMaxLevel;
//  nFirstTabPos = rForm.nFirstTabPos;
//  bHasFirstTabPos = rForm.bHasFirstTabPos;
    bGenerateTabPos = rForm.bGenerateTabPos;
    bIsRelTabPos = rForm.bIsRelTabPos;
    bCommaSeparated = rForm.bCommaSeparated;
    for(USHORT i=0; i < nFormMaxLevel; ++i)
    {
        aPattern[i] = rForm.aPattern[i];
        aTemplate[i] = rForm.aTemplate[i];
    }
    return *this;
}

USHORT SwForm::GetFormMaxLevel( USHORT nType )
{
    USHORT nRet = 0;
    switch( nType )
    {
        case TOX_INDEX:         nRet = 5;                   break;
        case TOX_USER:          nRet = MAXLEVEL+1;          break;
        case TOX_CONTENT:       nRet = MAXLEVEL+1;          break;
        case TOX_ILLUSTRATIONS:
        case TOX_OBJECTS      :
        case TOX_TABLES       : nRet = 2; break;
        case TOX_AUTHORITIES  : nRet = AUTH_TYPE_END + 1;       break;
    }
    return nRet;
}
/* -----------------15.06.99 13:39-------------------
    compatibilty methods: Version 5.0 and 5.1 need
    a value for the first tab stop
 --------------------------------------------------*/
USHORT lcl_GetPatternCount( const String& rPattern, const sal_Char* pToken )
{
    USHORT nRet = 0;
    String aSearch; aSearch.AssignAscii( pToken );
    aSearch.Erase( aSearch.Len() - 1, 1 );
    xub_StrLen nFound = rPattern.Search( aSearch );
    while( STRING_NOTFOUND != nFound )
        if( STRING_NOTFOUND != ( nFound = rPattern.Search( '>', nFound ) ))
        {
            ++nRet;
            nFound = rPattern.Search( aSearch, nFound );
        }
    return nRet;
}

String lcl_GetPattern( const String& rPattern, const sal_Char* pToken )
{
    String sRet;
    String aSearch; aSearch.AssignAscii( pToken );
    aSearch.Erase( aSearch.Len() - 1, 1 );

    xub_StrLen nEnd, nFound = rPattern.Search( aSearch );
    if( STRING_NOTFOUND != nFound &&
        STRING_NOTFOUND != ( nEnd = rPattern.Search( '>', nFound )) )
        sRet = rPattern.Copy( nFound, nEnd - nFound + 1 );
    return sRet;
}

// #i21237#
bool operator == (const SwFormToken & rToken, FormTokenType eType)
{
    return rToken.eTokenType == eType;
}

USHORT SwForm::GetFirstTabPos() const   // #i21237#
{
    DBG_WARNING("compatibility")
    USHORT nRet = 0;
    const SwFormTokens & aTokens = aPattern[1];

    if (2 <= count_if(aTokens.begin(), aTokens.end(),
                      SwFormTokenEqualToFormTokenType(TOKEN_TAB_STOP)))
    {
        SwFormTokens::const_iterator aIt =
            find_if(aTokens.begin(), aTokens.end(),
                    SwFormTokenEqualToFormTokenType(TOKEN_TAB_STOP));

        if (aIt != aTokens.end())
            nRet = aIt->nTabStopPosition;
    }

    return nRet;
}

void SwForm::SetFirstTabPos( USHORT n )     // #i21237#
{
    for(USHORT i = 0; i < MAXLEVEL; i++)
    {
        SwFormTokens & aTokens = aPattern[i+1];

        if (2 <= count_if(aTokens.begin(), aTokens.end(),
                          SwFormTokenEqualToFormTokenType(TOKEN_TAB_STOP)))
        {
            SwFormTokens::iterator aIt =
                find_if(aTokens.begin(), aTokens.end(),
                        SwFormTokenEqualToFormTokenType(TOKEN_TAB_STOP));

            SwFormToken aToken(TOKEN_TAB_STOP);
            aToken.eTabAlign = aIt->eTabAlign;
            aToken.cTabFillChar = aIt->cTabFillChar;
            aToken.nTabStopPosition = n;

            *aIt = aToken;
        }
        else
        {
            SwFormToken aToken(TOKEN_TAB_STOP);

            SwFormTokens::iterator aIt = aTokens.begin();
            aIt++;
            aTokens.insert(aIt, aToken);
        }
    }
}
/* -----------------------------28.02.00 09:48--------------------------------
    if the templates contain settings of the tab positions (<5.1) then
    they must be imported into the pattern
 ---------------------------------------------------------------------------*/
BOOL lcl_FindTabToken( const String& rPattern, xub_StrLen nSearchFrom,
                    xub_StrLen& rFoundStart, xub_StrLen& rFoundEnd)
{
    // search for <T ...>
    // special handling of <TX ...>

    BOOL bRet = FALSE;
    String sToFind; sToFind.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "<T" ));
    xub_StrLen nFoundPos = rPattern.Search( sToFind, nSearchFrom );
    while( STRING_NOTFOUND != nFoundPos )
    {
        if( rPattern.GetChar(nFoundPos +1) == 'X')
        {
            // handling of text tokens
            xub_StrLen nTempDelim = rPattern.Search(TOX_STYLE_DELIMITER, nFoundPos);
            xub_StrLen nTempEndSymbol = rPattern.Search(TOX_STYLE_DELIMITER, nFoundPos);
            if( nTempEndSymbol < nTempDelim && STRING_NOTFOUND != nTempDelim )
            {
                //text delimiter must! be in pairs
                nTempDelim = rPattern.Search(TOX_STYLE_DELIMITER, nTempDelim + 1);
                nTempEndSymbol = rPattern.Search(TOX_STYLE_DELIMITER, nTempDelim + 1);
            }
            else
            {
                nFoundPos = nTempDelim;
            }
        }
        else
        {
            //tab stop token found - find end position
            rFoundStart = nFoundPos;
            rFoundEnd = rPattern.Search('>', nFoundPos +1);
            bRet = TRUE;
            break;
        }
        nFoundPos = rPattern.Search( sToFind, nSearchFrom );
    }
    return bRet;
}
//-----------------------------------------------------------------------------
void SwForm::AdjustTabStops(SwDoc& rDoc, BOOL bDefaultRightTabStop) // #i21237#
{
    for(USHORT nLevel = 1; nLevel < GetFormMax(); nLevel++)
    {
        const String& sTemplateName = GetTemplate(nLevel);

        SwTxtFmtColl* pColl = rDoc.FindTxtFmtCollByName( sTemplateName );
        if( !pColl )
        {
            USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName
                ( sTemplateName, GET_POOLID_TXTCOLL ); // #i21237#
            if( USHRT_MAX != nId )
                pColl = rDoc.GetTxtCollFromPool( nId );
        }
        if( pColl )
        {
            const SvxTabStopItem& rTabStops = pColl->GetTabStops( FALSE );
            USHORT nTabCount = rTabStops.Count();
            // #i21237#
            SwFormTokens aCurrentPattern = GetPattern(nLevel);
            SwFormTokens::iterator aIt = aCurrentPattern.begin();

            xub_StrLen nLastTabFoundEndPos = 0;
            BOOL bChanged = FALSE;

            for(USHORT nTab = 0; nTab < nTabCount; nTab++)
            {
                bChanged = TRUE;
                const SvxTabStop& rTab = rTabStops[nTab];
                xub_StrLen nStart, nEnd;

                // -> #i21237
                if (aIt != aCurrentPattern.end())
                    aIt = find_if(aIt, aCurrentPattern.end(),
                                  SwFormTokenEqualToFormTokenType
                                  (TOKEN_TAB_STOP));

                if (aIt == aCurrentPattern.end())
                {
                    SwFormToken aToken(TOKEN_TAB_STOP);
                    aToken.bWithTab = FALSE;

                    aCurrentPattern.push_back(aToken);
                    aIt = aCurrentPattern.end();
                    aIt--;
                }

                aIt->nTabStopPosition = rTab.GetTabPos();
                aIt->eTabAlign = rTab.GetAdjustment();
                aIt->cTabFillChar = rTab.GetFill();

                if( nTab == nTabCount - 1 &&
                        rTab.GetAdjustment() == SVX_TAB_ADJUST_RIGHT)
                        aIt->eTabAlign = SVX_TAB_ADJUST_END;

                if ( aIt != aCurrentPattern.end())
                    aIt++;
                // <- #i21237
            }

            // -> #i21237
            if (bDefaultRightTabStop)
            {
                SwFormTokens::reverse_iterator aIt = aCurrentPattern.rbegin();

                if (aIt == aCurrentPattern.rend() ||
                    aIt->eTabAlign != SVX_TAB_ADJUST_END)
                {
                    SwFormToken aToken(TOKEN_TAB_STOP);
                    aToken.eTabAlign = SVX_TAB_ADJUST_END;
                    aToken.bWithTab = FALSE;

                    aCurrentPattern.push_back(aToken);
                }
            }
            // <- #i21237
            if(bChanged)
                SetPattern(nLevel, aCurrentPattern); // #i21237#
        }
    }
}
//-----------------------------------------------------------------------------

BOOL SwForm::IsFirstTabPosFlag() const      //{ return bHasFirstTabPos; }
{
    //rturn true if the first level contains two ore more tabstops
    // #i21237#
    return 2 <= count_if(aPattern[1].begin(), aPattern[1].end(),
                         SwFormTokenEqualToFormTokenType(TOKEN_TAB_STOP));
}
void SwForm::SetFirstTabPosFlag( BOOL b )   //{ bHasFirstTabPos = b; }
{
    //only used in the old index dialog and in Sw3IoImp::InTOXs()
    DBG_WARNING("obsolete, isn't it?")
}
/* -----------------29.07.99 14:37-------------------

 --------------------------------------------------*/
// #i21237#
String  SwForm::ConvertPatternTo51(const SwFormTokens & rSource)
{
    SwFormTokens::const_iterator aIt = rSource.begin();
    String sRet;
    while(aIt != rSource.end())
    {
        SwFormToken  aToken = *aIt;
        switch(aToken.eTokenType)
        {
            case TOKEN_ENTRY_NO     :
                sRet.AppendAscii( SwForm::aFormEntryNum );
            break;
            case TOKEN_ENTRY_TEXT   :
                sRet.AppendAscii( SwForm::aFormEntryTxt );
            break;
            case TOKEN_ENTRY        :
                sRet.AppendAscii( SwForm::aFormEntry );
            break;
            case TOKEN_TAB_STOP     :
                sRet.AppendAscii( SwForm::aFormTab );
            break;
            case TOKEN_TEXT         :
                sRet += aToken.sText;
            break;
            case TOKEN_PAGE_NUMS    :
                sRet.AppendAscii( SwForm::aFormPageNums );
            break;
            case TOKEN_CHAPTER_INFO :
                // not available in 5.1
            break;
            case TOKEN_LINK_START   :
                sRet.AppendAscii( SwForm::aFormLinkStt );
            break;
            case TOKEN_LINK_END     :
                sRet.AppendAscii( SwForm::aFormLinkEnd );
            break;
            case TOKEN_AUTHORITY :
                // no conversion available
                sRet.AppendAscii( SwForm::aFormEntry );
            break;
        }

        aIt++;
    }
    return sRet;
}

/* -----------------29.07.99 14:37-------------------

 --------------------------------------------------*/
String  SwForm::ConvertPatternFrom51(const String& rSource, TOXTypes eType)
{
    String sRet( rSource );
    DBG_ASSERT(rSource.GetTokenCount('>') == rSource.GetTokenCount('<'),
                "count of '<' and '>' not identical")
//  simple task - convert tokens
//  <E> -> <E >
//  <E#> -> <E# >
//  <ET> -> <ET >
//  <#> -> <# >
//  <T> -> <T >
//  <LS> -> <LS >
//  <LE> -> <LE >
    for( xub_StrLen nFound = 0;
        STRING_NOTFOUND != ( nFound = sRet.Search( '>', nFound )); )
    {
        sRet.Insert( ' ', nFound );
        nFound += 2;                    // skip over the blank and '>'
    }

    //more complicated: convert text between >TEXT< to <X \0xffTEXT\0xff>
    xub_StrLen nOpenStart = 0;
    xub_StrLen nCloseStart = 0;
    xub_StrLen nClosePos = sRet.Search('>', nCloseStart);
    xub_StrLen nOpenPos = sRet.Search('<', nOpenStart);
    if(nOpenPos != STRING_NOTFOUND && nOpenPos > 0)
    {
        USHORT nOffset = lcl_ConvertTextIntoPattern( sRet, 0, nOpenPos);
        nCloseStart += nOffset;
        nClosePos += nOffset;
        nOpenStart = nClosePos;
    }
    else
        nOpenStart = nClosePos;
    nOpenPos = sRet.Search('<', ++nOpenStart);
    while(nOpenPos != STRING_NOTFOUND)
    {
        if(nClosePos < nOpenPos - 1)
        {
            USHORT nOffset = lcl_ConvertTextIntoPattern(sRet, nClosePos + 1, nOpenPos);
            nOpenStart += nOffset;
            nCloseStart = nOpenStart;
        }
        else
        {
            nCloseStart = nClosePos;
            nOpenStart = nOpenPos;
        }
        nClosePos = sRet.Search('>', ++nCloseStart);
        nOpenPos = sRet.Search('<', ++nOpenStart);
    }
    //is there any text at the end?
    if(nClosePos != STRING_NOTFOUND && nClosePos < sRet.Len() - 1)
        lcl_ConvertTextIntoPattern(sRet, nClosePos + 1, sRet.Len());
    if(eType != TOX_INDEX)
    {
        // set most left tab stop to right alignment and FillChar == '.'
        String sTabSearch;
        sTabSearch.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "<T "));
        xub_StrLen nOldTabPos = sRet.Search(sTabSearch);
        if(STRING_NOTFOUND != nOldTabPos)
        {
            xub_StrLen nNewTabPos = nOldTabPos;
            do {
                nOldTabPos = nNewTabPos;
                nNewTabPos = sRet.Search(sTabSearch, nOldTabPos + 1);
            } while(STRING_NOTFOUND != nNewTabPos);
            sRet.InsertAscii( ",65535,0,5,.", nOldTabPos + 3 );
        }
    }
    return sRet;
}

/*--------------------------------------------------------------------
     Beschreibung: Ctor TOXBase
 --------------------------------------------------------------------*/


SwTOXBase::SwTOXBase(const SwTOXType* pTyp, const SwForm& rForm,
                     USHORT nCreaType, const String& rTitle )
    : SwClient((SwModify*)pTyp),
    nCreateType(nCreaType),
    aTitle(rTitle),
    aForm(rForm),
    eCaptionDisplay(CAPTION_COMPLETE),
    eLanguage((LanguageType)::GetAppLanguage()),
    bProtected( TRUE ),
    bFromChapter(FALSE),
    bFromObjectNames(FALSE),
    bLevelFromChapter(FALSE),
    nOLEOptions(0)
{
    aData.nOptions = 0;
}


SwTOXBase::SwTOXBase( const SwTOXBase& rSource, SwDoc* pDoc )
    : SwClient( rSource.pRegisteredIn )
{
    CopyTOXBase( pDoc, rSource );
}

SwTOXBase& SwTOXBase::CopyTOXBase( SwDoc* pDoc, const SwTOXBase& rSource )
{
    SwTOXType* pType = (SwTOXType*)rSource.GetTOXType();
    if( pDoc && USHRT_MAX == pDoc->GetTOXTypes().GetPos( pType ))
    {
        // type not in pDoc, so create it now
        const SwTOXTypes& rTypes = pDoc->GetTOXTypes();
        BOOL bFound = FALSE;
        for( USHORT n = rTypes.Count(); n; )
        {
            const SwTOXType* pCmp = rTypes[ --n ];
            if( pCmp->GetType() == pType->GetType() &&
                pCmp->GetTypeName() == pType->GetTypeName() )
            {
                pType = (SwTOXType*)pCmp;
                bFound = TRUE;
                break;
            }
        }

        if( !bFound )
            pType = (SwTOXType*)pDoc->InsertTOXType( *pType );
    }
    pType->Add( this );

    nCreateType = rSource.nCreateType;
    aTitle      = rSource.aTitle;
    aForm       = rSource.aForm;
    bProtected  = rSource.bProtected;
    bFromChapter = rSource.bFromChapter;
    bFromObjectNames = rSource.bFromObjectNames;
    sMainEntryCharStyle = rSource.sMainEntryCharStyle;
    sSequenceName = rSource.sSequenceName;
    eCaptionDisplay = rSource.eCaptionDisplay;
    nOLEOptions = rSource.nOLEOptions;
    eLanguage = rSource.eLanguage;
    sSortAlgorithm = rSource.sSortAlgorithm;

    for( USHORT i = 0; i < MAXLEVEL; ++i )
        aStyleNames[i] = rSource.aStyleNames[i];

    // its the same data type!
    aData.nOptions =  rSource.aData.nOptions;

    if( !pDoc || pDoc->IsCopyIsMove() )
        aName = rSource.GetTOXName();
    else
        aName = pDoc->GetUniqueTOXBaseName( *pType, &rSource.GetTOXName() );

    return *this;
}

/* -----------------30.06.99 14:46-------------------
    Check if any style names are set in the array
 --------------------------------------------------*/
BOOL    SwTOXBase::HasAnyStyleNames() const
{
    for(USHORT i = 0; i < MAXLEVEL; i++)
        if(aStyleNames[i].Len())
            return TRUE;
    return FALSE;
}

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnisspezifische Funktionen
 --------------------------------------------------------------------*/

SwTOXBase::~SwTOXBase()
{
//    if( GetTOXType()->GetType() == TOX_USER  )
//        delete aData.pTemplateName;
}

String SwFormToken::GetString() const
{
    String sRet;

    BOOL bAppend = TRUE;
    switch( eTokenType )
    {
        case TOKEN_ENTRY_NO:
            sRet.AssignAscii( SwForm::aFormEntryNum );
        break;
        case TOKEN_ENTRY_TEXT:
            sRet.AssignAscii( SwForm::aFormEntryTxt );
        break;
        case TOKEN_ENTRY:
            sRet.AssignAscii( SwForm::aFormEntry );
        break;
        case TOKEN_TAB_STOP:
            sRet.AssignAscii( SwForm::aFormTab );
        break;
        case TOKEN_TEXT:
            sRet.AssignAscii( SwForm::aFormText );
        break;
        case TOKEN_PAGE_NUMS:
            sRet.AssignAscii( SwForm::aFormPageNums );
        break;
        case TOKEN_CHAPTER_INFO:
            sRet.AssignAscii( SwForm::aFormChapterMark );
        break;
        case TOKEN_LINK_START:
            sRet.AssignAscii( SwForm::aFormLinkStt );
        break;
        case TOKEN_LINK_END:
            sRet.AssignAscii( SwForm::aFormLinkEnd );
        break;
        case TOKEN_AUTHORITY:
        {
            sRet.AssignAscii( SwForm::aFormAuth );
            String sTmp( String::CreateFromInt32( nAuthorityField ));
            if( sTmp.Len() < 2 )
                sTmp.Insert('0', 0);
            sRet.Insert( sTmp, 2 );
        }
        break;
    }
    sRet.Erase( sRet.Len() - 1 );
    sRet += ' ';
    sRet += sCharStyleName;
    sRet += ',';
    sRet += String::CreateFromInt32( nPoolId );
    sRet += ',';

    // TabStopPosition and TabAlign or ChapterInfoFormat
    if(TOKEN_TAB_STOP == eTokenType)
    {
        sRet += String::CreateFromInt32( nTabStopPosition );
        sRet += ',';
        sRet += String::CreateFromInt32( eTabAlign );
        sRet += ',';
        sRet += cTabFillChar;
        sRet += ',';
        sRet += String::CreateFromInt32( bWithTab );
    }
    else if(TOKEN_CHAPTER_INFO == eTokenType)
    {
        sRet += String::CreateFromInt32( nChapterFormat );
    }
    else if(TOKEN_TEXT == eTokenType)
    {
        //append Text if Len() > 0 only!
        if( sText.Len() )
        {
            sRet += TOX_STYLE_DELIMITER;
            String sTmp( sText );
            sTmp.EraseAllChars( TOX_STYLE_DELIMITER );
            sRet += sTmp;
            sRet += TOX_STYLE_DELIMITER;
        }
        else
            bAppend = FALSE;
    }
    if(bAppend)
    {
        sRet += '>';
    }
    else
    {
        // don't append empty text tokens
        sRet.Erase();
    }

    return sRet;
}

// -> #i21237#
SwFormTokensHelper::SwFormTokensHelper(const String & rPattern)
{
    xub_StrLen nCurPatternPos = 0;
    xub_StrLen nCurPatternLen = 0;

    while (nCurPatternPos < rPattern.Len())
    {
        nCurPatternPos += nCurPatternLen;

        SwFormToken aToken = BuildToken(rPattern, nCurPatternPos);
        aTokens.push_back(aToken);
    }
}

SwFormToken SwFormTokensHelper::BuildToken( const String & sPattern,
                                           xub_StrLen & nCurPatternPos ) const
{
    String sToken( SearchNextToken(sPattern, nCurPatternPos) );
    nCurPatternPos += sToken.Len();
    xub_StrLen nTokenLen;
    FormTokenType eTokenType = GetTokenType(sToken, &nTokenLen);

    // at this point sPattern contains the
    // character style name, the PoolId, tab stop position, tab stop alignment, chapter info format
    // the form is: CharStyleName, PoolId[, TabStopPosition|ChapterInfoFormat[, TabStopAlignment[, TabFillChar]]]
    // in text tokens the form differs from the others: CharStyleName, PoolId[,\0xffinserted text\0xff]
    SwFormToken eRet( eTokenType );
    String sAuthFieldEnum = sToken.Copy( 2, 2 );
    sToken = sToken.Copy( nTokenLen, sToken.Len() - nTokenLen - 1);

    eRet.sCharStyleName = sToken.GetToken( 0, ',');
    String sTmp( sToken.GetToken( 1, ',' ));
    if( sTmp.Len() )
        eRet.nPoolId = sTmp.ToInt32();

    switch( eTokenType )
    {
    case TOKEN_TEXT:
        {
            xub_StrLen nStartText = sToken.Search( TOX_STYLE_DELIMITER );
            if( STRING_NOTFOUND != nStartText )
            {
                xub_StrLen nEndText = sToken.Search( TOX_STYLE_DELIMITER,
                                                nStartText + 1);
                if( STRING_NOTFOUND != nEndText )
                {
                    eRet.sText = sToken.Copy( nStartText + 1,
                                                nEndText - nStartText - 1);
                }
            }
        }
        break;

    case TOKEN_TAB_STOP:
        if( (sTmp = sToken.GetToken( 2, ',' ) ).Len() )
            eRet.nTabStopPosition = sTmp.ToInt32();

        if( (sTmp = sToken.GetToken( 3, ',' ) ).Len() )
            eRet.eTabAlign = sTmp.ToInt32();

        if( (sTmp = sToken.GetToken( 4, ',' ) ).Len() )
            eRet.cTabFillChar = sTmp.GetChar(0);

        if( (sTmp = sToken.GetToken( 5, ',' ) ).Len() )
            eRet.bWithTab = 0 != sTmp.ToInt32();
        break;

    case TOKEN_CHAPTER_INFO:
        if( (sTmp = sToken.GetToken( 2, ',' ) ).Len() )
            eRet.nChapterFormat = sTmp.ToInt32(); //SwChapterFormat;
        break;

    case TOKEN_AUTHORITY:
        eRet.nAuthorityField = sAuthFieldEnum.ToInt32();
        break;
    }
    return eRet;
}

String SwFormTokensHelper::SearchNextToken( const String & sPattern,
                                            xub_StrLen nStt ) const
{
    //it's not so easy - it doesn't work if the text part contains a '>'
    //USHORT nTokenEnd = sPattern.Search('>');

    String aResult;

    xub_StrLen nEnd = sPattern.Search( '>', nStt );
    if( STRING_NOTFOUND == nEnd )
    {
        nEnd = sPattern.Len();
    }
    else
    {
        xub_StrLen nTextSeparatorFirst = sPattern.Search( TOX_STYLE_DELIMITER, nStt );
        if( STRING_NOTFOUND != nTextSeparatorFirst )
        {
            xub_StrLen nTextSeparatorSecond = sPattern.Search( TOX_STYLE_DELIMITER,
                                                               nTextSeparatorFirst + 1 );
            if( STRING_NOTFOUND != nTextSeparatorSecond &&
                nEnd > nTextSeparatorFirst )
                nEnd = sPattern.Search( '>', nTextSeparatorSecond );
        }

        ++nEnd;

        aResult = sPattern.Copy( nStt, nEnd - nStt );
    }

    return aResult;
}

FormTokenType SwFormTokensHelper::GetTokenType(const String & sToken,
                                               xub_StrLen * pTokenLen) const
{
    static struct
    {
        const sal_Char* pNm;
        USHORT nLen;
        USHORT nOffset;
        FormTokenType eToken;
    } __READONLY_DATA aTokenArr[] = {
        SwForm::aFormTab,       SwForm::nFormEntryLen,      1, TOKEN_TAB_STOP,
        SwForm::aFormPageNums,  SwForm::nFormPageNumsLen,   1, TOKEN_PAGE_NUMS,
        SwForm::aFormLinkStt,   SwForm::nFormLinkSttLen,    1, TOKEN_LINK_START,
        SwForm::aFormLinkEnd,   SwForm::nFormLinkEndLen,    1, TOKEN_LINK_END,
        SwForm::aFormEntryNum,  SwForm::nFormEntryNumLen,   1, TOKEN_ENTRY_NO,
        SwForm::aFormEntryTxt,  SwForm::nFormEntryTxtLen,   1, TOKEN_ENTRY_TEXT,
        SwForm::aFormChapterMark,SwForm::nFormChapterMarkLen,1,TOKEN_CHAPTER_INFO,
        SwForm::aFormText,      SwForm::nFormTextLen,       1, TOKEN_TEXT,
        SwForm::aFormEntry,     SwForm::nFormEntryLen,      1, TOKEN_ENTRY,
        SwForm::aFormAuth,      SwForm::nFormAuthLen,       3, TOKEN_AUTHORITY,
        0,                      0,                          0, TOKEN_END
    };

    FormTokenType eTokenType = TOKEN_TEXT;
    xub_StrLen nTokenLen = 0;
    const sal_Char* pNm;
    for( int i = 0; 0 != ( pNm = aTokenArr[ i ].pNm ); ++i )
        if( COMPARE_EQUAL == sToken.CompareToAscii( pNm,
                            aTokenArr[ i ].nLen - aTokenArr[ i ].nOffset ))
        {
            eTokenType = aTokenArr[ i ].eToken;
            nTokenLen = aTokenArr[ i ].nLen;
            break;
        }

    ASSERT( pNm, "wrong token" );
    if (pTokenLen)
        *pTokenLen = nTokenLen;

    return eTokenType;
}

String SwFormTokensHelper::GetPatternString() const
{
    String aResult;

    for_each(aTokens.begin(), aTokens.end(), SwFormTokenToString(aResult));

    return aResult;
}
// <- #i21237#
