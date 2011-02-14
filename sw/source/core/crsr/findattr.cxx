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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <i18npool/mslangid.hxx>
#include <hintids.hxx>
#include <vcl/svapp.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <fmtpdsc.hxx>
#include <txatbase.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swcrsr.hxx>
#include <editsh.hxx>
#include <ndtxt.hxx>
#include <pamtyp.hxx>
#include <swundo.hxx>
#include <crsskip.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

SV_DECL_PTRARR_SORT( SwpFmts, SwFmt*, 0, 4 )
SV_IMPL_PTRARR_SORT( SwpFmts, SwFmt* )

    // Sonderbehandlung fuer SvxFontItem, nur den Namen vergleichen:
int CmpAttr( const SfxPoolItem& rItem1, const SfxPoolItem& rItem2 )
{
    switch( rItem1.Which() )
    {
    case RES_CHRATR_FONT:
        return ((SvxFontItem&)rItem1).GetFamilyName() ==
                ((SvxFontItem&)rItem2).GetFamilyName();

    case RES_CHRATR_COLOR:
        return ((SvxColorItem&)rItem1).GetValue().IsRGBEqual(
                                ((SvxColorItem&)rItem2).GetValue() );
    case RES_PAGEDESC:
        return ((SwFmtPageDesc&)rItem1).GetNumOffset() ==
                        ((SwFmtPageDesc&)rItem2).GetNumOffset() &&
                ((SwFmtPageDesc&)rItem1).GetPageDesc() ==
                        ((SwFmtPageDesc&)rItem2).GetPageDesc();
    }
    return rItem1 == rItem2;
}


const SwTxtAttr* GetFrwrdTxtHint( const SwpHints& rHtsArr, sal_uInt16& rPos,
                                    xub_StrLen nCntntPos )
{
    while( rPos < rHtsArr.Count() )
    {
        const SwTxtAttr *pTxtHt = rHtsArr.GetStart( rPos++ );
        // der Start vom Attribut muss innerhalb des Bereiches liegen !!
        if( *pTxtHt->GetStart() >= nCntntPos )
            return pTxtHt;      // gueltiges TextAttribut
    }
    return 0;                   // kein gueltiges TextAttribut
}


const SwTxtAttr* GetBkwrdTxtHint( const SwpHints& rHtsArr, sal_uInt16& rPos,
                                  xub_StrLen nCntntPos )
{
    while( rPos > 0 )
    {
        //Hack mit cast fuer das Update
        const SwTxtAttr *pTxtHt = rHtsArr.GetStart( --rPos );
        // der Start vom Attribut muss innerhalb des Bereiches liegen !!
        if( *pTxtHt->GetStart() < nCntntPos )
            return pTxtHt;      // gueltiges TextAttribut
    }
    return 0;                   // kein gueltiges TextAttribut
}


void lcl_SetAttrPam( SwPaM & rPam, xub_StrLen nStart, const xub_StrLen* pEnde,
                        const sal_Bool bSaveMark )
{
    xub_StrLen nCntntPos;
    if( bSaveMark )
        nCntntPos = rPam.GetMark()->nContent.GetIndex();
    else
        nCntntPos = rPam.GetPoint()->nContent.GetIndex();
    sal_Bool bTstEnde = rPam.GetPoint()->nNode == rPam.GetMark()->nNode;

    SwCntntNode* pCNd = rPam.GetCntntNode();
    rPam.GetPoint()->nContent.Assign( pCNd, nStart );
    rPam.SetMark();     // Point == GetMark

    // Point zeigt auf das Ende vom SuchBereich oder Ende vom Attribut
    if( pEnde )
    {
        if( bTstEnde && *pEnde > nCntntPos )
            rPam.GetPoint()->nContent = nCntntPos;
        else
            rPam.GetPoint()->nContent = *pEnde;
    }
}

//------------------ Suche nach einem Text Attribut -----------------------

// diese Funktion sucht in einem TextNode nach dem vorgegebenen Attribut.
// Wird es gefunden, dann hat der SwPaM den Bereich der das Attribut
// umspannt, unter Beachtung des Suchbereiches


sal_Bool lcl_Search( const SwTxtNode& rTxtNd, SwPaM& rPam,
                    const SfxPoolItem& rCmpItem,
                    SwMoveFn fnMove, sal_Bool bValue )
{
    if ( !rTxtNd.HasHints() )
        return sal_False;
    const SwTxtAttr *pTxtHt = 0;
    sal_Bool bForward = fnMove == fnMoveForward;
    sal_uInt16 nPos = bForward ? 0 : rTxtNd.GetSwpHints().Count();
    xub_StrLen nCntntPos = rPam.GetPoint()->nContent.GetIndex();

    while( 0 != ( pTxtHt=(*fnMove->fnGetHint)(rTxtNd.GetSwpHints(),nPos,nCntntPos)))
        if( pTxtHt->Which() == rCmpItem.Which() &&
            ( !bValue || CmpAttr( pTxtHt->GetAttr(), rCmpItem )))
        {
            lcl_SetAttrPam( rPam, *pTxtHt->GetStart(), pTxtHt->GetEnd(), bForward );
            return sal_True;
        }
    return sal_False;
}


//------------------ Suche nach mehren Text Attributen -------------------

struct _SwSrchChrAttr
{
    sal_uInt16 nWhich;
    xub_StrLen nStt, nEnd;

    _SwSrchChrAttr( const SfxPoolItem& rItem,
                    xub_StrLen nStart, xub_StrLen nAnyEnd )
        : nWhich( rItem.Which() ), nStt( nStart ), nEnd( nAnyEnd )
    {}
};

class SwAttrCheckArr
{
    _SwSrchChrAttr *pFndArr, *pStackArr;
    xub_StrLen nNdStt, nNdEnd;
    sal_uInt16 nArrStart, nArrLen;
    sal_uInt16 nFound, nStackCnt;
    SfxItemSet aCmpSet;
    sal_Bool bNoColls;
    sal_Bool bForward;

public:
    SwAttrCheckArr( const SfxItemSet& rSet, int bForward, int bNoCollections );
    ~SwAttrCheckArr();

    void SetNewSet( const SwTxtNode& rTxtNd, const SwPaM& rPam );

    // wieviele Attribute ueberhaupt ??
    sal_uInt16 Count() const    { return aCmpSet.Count(); }
    int Found() const       { return nFound == aCmpSet.Count(); }
    int CheckStack();

    xub_StrLen Start() const;
    xub_StrLen End() const;

    xub_StrLen GetNdStt() const { return nNdStt; }
    xub_StrLen GetNdEnd() const { return nNdEnd; }

    int SetAttrFwd( const SwTxtAttr& rAttr );
    int SetAttrBwd( const SwTxtAttr& rAttr );
};



SwAttrCheckArr::SwAttrCheckArr( const SfxItemSet& rSet, int bFwd,
                                int bNoCollections )
    : aCmpSet( *rSet.GetPool(), RES_CHRATR_BEGIN, RES_TXTATR_END-1 )
{
    aCmpSet.Put( rSet, sal_False );
    bNoColls = 0 != bNoCollections;

    bForward = 0 != bFwd;

    // Bestimmen den Bereich des Fnd/Stack-Arrays (Min/Max)
    SfxItemIter aIter( aCmpSet );
    nArrStart = aCmpSet.GetWhichByPos( aIter.GetFirstPos() );
    nArrLen = aCmpSet.GetWhichByPos( aIter.GetLastPos() ) - nArrStart+1;

    char* pFndChar  = new char[ nArrLen * sizeof(_SwSrchChrAttr) ];
    char* pStackChar = new char[ nArrLen * sizeof(_SwSrchChrAttr) ];

    pFndArr = (_SwSrchChrAttr*)pFndChar;
    pStackArr = (_SwSrchChrAttr*)pStackChar;
}

SwAttrCheckArr::~SwAttrCheckArr()
{
    delete[] (char*)pFndArr;
    delete[] (char*)pStackArr;
}

void SwAttrCheckArr::SetNewSet( const SwTxtNode& rTxtNd, const SwPaM& rPam )
{
    memset( pFndArr, 0, nArrLen * sizeof(_SwSrchChrAttr) );
    memset( pStackArr, 0, nArrLen * sizeof(_SwSrchChrAttr) );
    nFound = 0;
    nStackCnt = 0;

    if( bForward )
    {
        nNdStt = rPam.GetPoint()->nContent.GetIndex();
        nNdEnd = rPam.GetPoint()->nNode == rPam.GetMark()->nNode
                ? rPam.GetMark()->nContent.GetIndex()
                : rTxtNd.GetTxt().Len();
    }
    else
    {
        nNdEnd = rPam.GetPoint()->nContent.GetIndex();
        nNdStt = rPam.GetPoint()->nNode == rPam.GetMark()->nNode
                ? rPam.GetMark()->nContent.GetIndex()
                : 0;
    }

    if( bNoColls && !rTxtNd.HasSwAttrSet() )
        return ;

    const SfxItemSet& rSet = rTxtNd.GetSwAttrSet();
//  if( !rSet.Count() )
//      return;

    SfxItemIter aIter( aCmpSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    const SfxPoolItem* pFndItem;
    sal_uInt16 nWhich;

    while( sal_True )
    {
        // nur testen, ob vorhanden ist ?
        if( IsInvalidItem( pItem ) )
        {
            nWhich = aCmpSet.GetWhichByPos( aIter.GetCurPos() );
            if( RES_TXTATR_END <= nWhich )
                break;              // Ende der TextAttribute

            if( SFX_ITEM_SET == rSet.GetItemState( nWhich, !bNoColls, &pFndItem )
                && !CmpAttr( *pFndItem, rSet.GetPool()->GetDefaultItem( nWhich ) ))
            {
                pFndArr[ nWhich - nArrStart ] =
                    _SwSrchChrAttr( *pFndItem, nNdStt, nNdEnd );
                nFound++;
            }
        }
        else
        {
            if( RES_TXTATR_END <= (nWhich = pItem->Which() ))
                break;              // Ende der TextAttribute

//JP 27.02.95: wenn nach defaults gesucht wird, dann muss man bis zum Pool
//              runter
//          if( SFX_ITEM_SET == rSet.GetItemState( nWhich, !bNoColls, &pFndItem )
//                && *pFndItem == *pItem )
            if( CmpAttr( rSet.Get( nWhich, !bNoColls ), *pItem ) )
            {
                pFndArr[ nWhich - nArrStart ] =
                    _SwSrchChrAttr( *pItem, nNdStt, nNdEnd );
                nFound++;
            }
        }

        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }
}
int SwAttrCheckArr::SetAttrFwd( const SwTxtAttr& rAttr )
{
    _SwSrchChrAttr aTmp( rAttr.GetAttr(), *rAttr.GetStart(), *rAttr.GetAnyEnd() );
    // alle die nicht im Bereich sind -> ignorieren
    if( aTmp.nEnd <= nNdStt || aTmp.nStt >= nNdEnd )
        return Found();

    const SfxPoolItem* pItem;

// --------------------------------------------------------------
// Hier wird jetzt ausdruecklich auch in Zeichenvorlagen gesucht
// --------------------------------------------------------------
    sal_uInt16 nWhch = rAttr.Which();
    SfxWhichIter* pIter = NULL;
    const SfxPoolItem* pTmpItem = NULL;
    const SfxItemSet* pSet = NULL;
    if( RES_TXTATR_CHARFMT == nWhch || RES_TXTATR_AUTOFMT == nWhch )
    {
        if( bNoColls && RES_TXTATR_CHARFMT == nWhch )
            return Found();
        pTmpItem = NULL;
        pSet = CharFmt::GetItemSet( rAttr.GetAttr() );
        if ( pSet )
        {
            pIter = new SfxWhichIter( *pSet );
            nWhch = pIter->FirstWhich();
            while( nWhch &&
                SFX_ITEM_SET != pSet->GetItemState( nWhch, sal_True, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                pTmpItem = NULL;
        }
    }
    else
        pTmpItem = &rAttr.GetAttr();
    while( pTmpItem )
    {
        SfxItemState eState = aCmpSet.GetItemState( nWhch, sal_False, &pItem );
        if( SFX_ITEM_DONTCARE == eState || SFX_ITEM_SET == eState )
        {
            sal_uInt16 n;
            _SwSrchChrAttr* pCmp;

            // loesche erstmal alle, die bis zu der Start Position schon wieder
            // ungueltig sind:

            _SwSrchChrAttr* pArrPtr;
            if( nFound )
                for( pArrPtr = pFndArr, n = 0; n < nArrLen;
                    ++n, ++pArrPtr )
                    if( pArrPtr->nWhich && pArrPtr->nEnd <= aTmp.nStt )
                    {
                        pArrPtr->nWhich = 0;        // geloescht
                        nFound--;
                    }

            // loesche erstmal alle, die bis zu der Start Position schon wieder
            // ungueltig sind. Und verschiebe alle die "offen" sind, heisst ueber
            // die Start Position ragen, vom Stack in den FndSet

            if( nStackCnt )
                for( pArrPtr = pStackArr, n=0; n < nArrLen; ++n, ++pArrPtr )
                {
                    if( !pArrPtr->nWhich )
                        continue;

                    if( pArrPtr->nEnd <= aTmp.nStt )
                    {
                        pArrPtr->nWhich = 0;        // geloescht
                        if( !--nStackCnt )
                            break;
                    }
                    else if( pArrPtr->nStt <= aTmp.nStt )
                    {
                        if( ( pCmp = &pFndArr[ n ])->nWhich )
                        {
                            if( pCmp->nEnd < pArrPtr->nEnd )        // erweitern
                                pCmp->nEnd = pArrPtr->nEnd;
                        }
                        else
                        {
                            *pCmp = *pArrPtr;
                            nFound++;
                        }
                        pArrPtr->nWhich = 0;
                        if( !--nStackCnt )
                            break;
                    }
                }

            sal_Bool bContinue = sal_False;

            if( SFX_ITEM_DONTCARE == eState  )
            {
                // wird Attribut gueltig ?
                if( !CmpAttr( aCmpSet.GetPool()->GetDefaultItem( nWhch ),
                    *pTmpItem ))
                {
                    // suche das Attribut und erweiter es gegebenenfalls
                    if( !( pCmp = &pFndArr[ nWhch - nArrStart ])->nWhich )
                    {
                        *pCmp = aTmp;               // nicht gefunden, eintragen
                        nFound++;
                    }
                    else if( pCmp->nEnd < aTmp.nEnd )       // erweitern ?
                        pCmp->nEnd = aTmp.nEnd;

                    bContinue = sal_True;
                }
            }
            // wird Attribut gueltig ?
            else if(  CmpAttr( *pItem, *pTmpItem ) )
            {
                pFndArr[ nWhch - nArrStart ] = aTmp;
                ++nFound;
                bContinue = sal_True;
            }

            // tja, dann muss es auf den Stack
            if( !bContinue && ( pCmp = &pFndArr[ nWhch - nArrStart ])->nWhich )
            {
                // vorhanden, auf den Stack. Aber nur wenn es noch grosser ist
                if( pCmp->nEnd > aTmp.nEnd )
                {
                    ASSERT( !pStackArr[ nWhch - nArrStart ].nWhich,
                                    "Stack-Platz ist noch belegt" );

        // ---------
        // JP 22.08.96: nur Ende manipulieren reicht nicht. Bug 30547
        //          pCmp->nStt = aTmp.nEnd;
                    if( aTmp.nStt <= pCmp->nStt )
                        pCmp->nStt = aTmp.nEnd;
                    else
                        pCmp->nEnd = aTmp.nStt;
        // ---------

                    pStackArr[ nWhch - nArrStart ] = *pCmp;
                    nStackCnt++;
                }
                pCmp->nWhich = 0;
                nFound--;
            }
        }
        if( pIter )
        {
            nWhch = pIter->NextWhich();
            while( nWhch &&
                SFX_ITEM_SET != pSet->GetItemState( nWhch, sal_True, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                break;
        }
        else
            break;
    }
    return Found();
}


int SwAttrCheckArr::SetAttrBwd( const SwTxtAttr& rAttr )
{
    _SwSrchChrAttr aTmp( rAttr.GetAttr(), *rAttr.GetStart(), *rAttr.GetAnyEnd() );
    // alle die nicht im Bereich sind -> ignorieren
    if( aTmp.nEnd < nNdStt || aTmp.nStt >= nNdEnd )
        return Found();

    const SfxPoolItem* pItem;
// --------------------------------------------------------------
// Hier wird jetzt ausdruecklich auch in Zeichenvorlagen gesucht
// --------------------------------------------------------------
    sal_uInt16 nWhch = rAttr.Which();
    SfxWhichIter* pIter = NULL;
    const SfxPoolItem* pTmpItem = NULL;
    const SfxItemSet* pSet = NULL;
    if( RES_TXTATR_CHARFMT == nWhch || RES_TXTATR_AUTOFMT == nWhch )
    {
        if( bNoColls && RES_TXTATR_CHARFMT == nWhch )
            return Found();

        pSet = CharFmt::GetItemSet( rAttr.GetAttr() );
        if ( pSet )
        {
            pIter = new SfxWhichIter( *pSet );
            nWhch = pIter->FirstWhich();
            while( nWhch &&
                SFX_ITEM_SET != pSet->GetItemState( nWhch, sal_True, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                pTmpItem = NULL;
        }
    }
    else
        pTmpItem = &rAttr.GetAttr();
    while( pTmpItem )
    {
        SfxItemState eState = aCmpSet.GetItemState( nWhch, sal_False, &pItem );
        if( SFX_ITEM_DONTCARE == eState || SFX_ITEM_SET == eState )
        {
            sal_uInt16 n;
            _SwSrchChrAttr* pCmp;

            // loesche erstmal alle, die bis zu der Start Position schon wieder
            // ungueltig sind:

            _SwSrchChrAttr* pArrPtr;
            if( nFound )
                for( pArrPtr = pFndArr, n = 0; n < nArrLen; ++n, ++pArrPtr )
                    if( pArrPtr->nWhich && pArrPtr->nStt >= aTmp.nEnd )
                    {
                        pArrPtr->nWhich = 0;        // geloescht
                        nFound--;
                    }

            // loesche erstmal alle, die bis zu der Start Position schon wieder
            // ungueltig sind. Und verschiebe alle die "offen" sind, heisst ueber
            // die Start Position ragen, vom Stack in den FndSet

            if( nStackCnt )
                for( pArrPtr = pStackArr, n = 0; n < nArrLen; ++n, ++pArrPtr )
                {
                    if( !pArrPtr->nWhich )
                        continue;

                    if( pArrPtr->nStt >= aTmp.nEnd )
                    {
                        pArrPtr->nWhich = 0;        // geloescht
                        if( !--nStackCnt )
                            break;
                    }
                    else if( pArrPtr->nEnd >= aTmp.nEnd )
                    {
                        if( ( pCmp = &pFndArr[ n ])->nWhich )
                        {
                            if( pCmp->nStt > pArrPtr->nStt )        // erweitern
                                pCmp->nStt = pArrPtr->nStt;
                        }
                        else
                        {
                            *pCmp = *pArrPtr;
                            nFound++;
                    }
                    pArrPtr->nWhich = 0;
                    if( !--nStackCnt )
                        break;
                }
            }

            sal_Bool bContinue = sal_False;
            if( SFX_ITEM_DONTCARE == eState  )
            {
                // wird Attribut gueltig ?
                if( !CmpAttr( aCmpSet.GetPool()->GetDefaultItem( nWhch ),
                    *pTmpItem ) )
                {
                    // suche das Attribut und erweiter es gegebenenfalls
                    if( !( pCmp = &pFndArr[ nWhch - nArrStart ])->nWhich )
                    {
                        *pCmp = aTmp;               // nicht gefunden, eintragen
                        nFound++;
                    }
                    else if( pCmp->nStt > aTmp.nStt )       // erweitern ?
                        pCmp->nStt = aTmp.nStt;

                    bContinue = sal_True;
                }
            }
            // wird Attribut gueltig ?
            else if( CmpAttr( *pItem, *pTmpItem ))
            {
                pFndArr[ nWhch - nArrStart ] = aTmp;
                ++nFound;
                bContinue = sal_True;
            }

            // tja, dann muss es auf den Stack
            if( !bContinue && ( pCmp = &pFndArr[ nWhch - nArrStart ])->nWhich )
            {
                // vorhanden, auf den Stack. Aber nur wenn es noch grosser ist
                if( pCmp->nStt < aTmp.nStt )
                {
                    ASSERT( !pStackArr[ nWhch - nArrStart ].nWhich,
                            "Stack-Platz ist noch belegt" );

// ---------
// JP 22.08.96: nur Ende manipulieren reicht nicht. Bug 30547
//          pCmp->nEnd = aTmp.nStt;
                    if( aTmp.nEnd <= pCmp->nEnd )
                        pCmp->nEnd = aTmp.nStt;
                    else
                        pCmp->nStt = aTmp.nEnd;
// ---------

                    pStackArr[ nWhch - nArrStart ] = *pCmp;
                    nStackCnt++;
                }
                pCmp->nWhich = 0;
                nFound--;
            }
        }
        if( pIter )
        {
            nWhch = pIter->NextWhich();
            while( nWhch &&
                SFX_ITEM_SET != pSet->GetItemState( nWhch, sal_True, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                break;
        }
        else
            break;
    }
    return Found();
}


xub_StrLen SwAttrCheckArr::Start() const
{
    xub_StrLen nStart = nNdStt;
    _SwSrchChrAttr* pArrPtr = pFndArr;
    for( sal_uInt16 n = 0; n < nArrLen; ++n, ++pArrPtr )
        if( pArrPtr->nWhich && pArrPtr->nStt > nStart )
            nStart = pArrPtr->nStt;

    return nStart;
}


xub_StrLen SwAttrCheckArr::End() const
{
    _SwSrchChrAttr* pArrPtr = pFndArr;
    xub_StrLen nEnd = nNdEnd;
    for( sal_uInt16 n = 0; n < nArrLen; ++n, ++pArrPtr )
        if( pArrPtr->nWhich && pArrPtr->nEnd < nEnd )
            nEnd = pArrPtr->nEnd;

    return nEnd;
}


int SwAttrCheckArr::CheckStack()
{
    if( !nStackCnt )
        return sal_False;

    sal_uInt16 n;
    xub_StrLen nSttPos = Start(), nEndPos = End();
    _SwSrchChrAttr* pArrPtr;
    for( pArrPtr = pStackArr, n = 0; n < nArrLen; ++n, ++pArrPtr )
    {
        if( !pArrPtr->nWhich )
            continue;

        if( bForward ? pArrPtr->nEnd <= nSttPos : pArrPtr->nStt >= nEndPos )
        {
            pArrPtr->nWhich = 0;        // geloescht
            if( !--nStackCnt )
                return nFound == aCmpSet.Count();
        }
        else if( bForward ? pArrPtr->nStt < nEndPos : pArrPtr->nEnd > nSttPos )
        {
            // alle die "offen" sind, heisst ueber die Start Position ragen,
            // im FndSet setzen
            ASSERT( !pFndArr[ n ].nWhich, "Array-Platz ist noch belegt" );
            pFndArr[ n ] = *pArrPtr;
            pArrPtr->nWhich = 0;
            nFound++;
            if( !--nStackCnt )
                return nFound == aCmpSet.Count();
        }
    }
    return nFound == aCmpSet.Count();
}



int lcl_SearchForward( const SwTxtNode& rTxtNd, SwAttrCheckArr& rCmpArr,
                            SwPaM& rPam )
{
    xub_StrLen nEndPos, nSttPos;
    rCmpArr.SetNewSet( rTxtNd, rPam );
    if( !rTxtNd.HasHints() )
    {
        if( !rCmpArr.Found() )
            return sal_False;
        nEndPos = rCmpArr.GetNdEnd();
        lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, sal_True );
        return sal_True;
    }

    // dann gehe mal durch das nach "Start" sortierte Array
    const SwpHints& rHtArr = rTxtNd.GetSwpHints();
    const SwTxtAttr* pAttr;
    sal_uInt16 nPos = 0;

    // sollte jetzt schon alles vorhanden sein, dann teste, mit welchem
    // das wieder beendet wird.
    if( rCmpArr.Found() )
    {
        for( ; nPos < rHtArr.Count(); ++nPos )
            if( !rCmpArr.SetAttrFwd( *( pAttr = rHtArr.GetStart( nPos )) ) )
            {
                if( rCmpArr.GetNdStt() < *pAttr->GetStart() )
                {
                    // dann haben wir unser Ende:
                    lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(),
                                pAttr->GetStart(), sal_True );
                    return sal_True;
                }
                // ansonsten muessen wir weiter suchen
                break;
            }

        if( nPos == rHtArr.Count() && rCmpArr.Found() )
        {
            // dann haben wir unseren Bereich
            nEndPos = rCmpArr.GetNdEnd();
            lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, sal_True );
            return sal_True;
        }
    }

    for( ; nPos < rHtArr.Count(); ++nPos )
        if( rCmpArr.SetAttrFwd( *( pAttr = rHtArr.GetStart( nPos )) ) )
        {
            // sollten noch mehr auf der gleichen Position anfangen ??
            // auch die noch mit testen !!
            nSttPos = *pAttr->GetStart();
            while( ++nPos < rHtArr.Count() && nSttPos ==
                    *( pAttr = rHtArr.GetStart( nPos ))->GetStart() &&
                    rCmpArr.SetAttrFwd( *pAttr ) )
                ;
            if( !rCmpArr.Found() )
                continue;

            // dann haben wir den Bereich zusammen
            if( (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
                return sal_False;
            lcl_SetAttrPam( rPam, nSttPos, &nEndPos, sal_True );
            return sal_True;
        }

    if( !rCmpArr.CheckStack() ||
        (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
        return sal_False;
    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, sal_True );
    return sal_True;
}


int lcl_SearchBackward( const SwTxtNode& rTxtNd, SwAttrCheckArr& rCmpArr,
                            SwPaM& rPam )
{
    xub_StrLen nEndPos, nSttPos;
    rCmpArr.SetNewSet( rTxtNd, rPam );
    if( !rTxtNd.HasHints() )
    {
        if( !rCmpArr.Found() )
            return sal_False;
        nEndPos = rCmpArr.GetNdEnd();
        lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, sal_False );
        return sal_True;
    }

    // dann gehe mal durch das nach "Start" sortierte Array
    const SwpHints& rHtArr = rTxtNd.GetSwpHints();
    const SwTxtAttr* pAttr;
    sal_uInt16 nPos = rHtArr.Count();

    // sollte jetzt schon alles vorhanden sein, dann teste, mit welchem
    // das wieder beendet wird.
    if( rCmpArr.Found() )
    {
        while( nPos )
            if( !rCmpArr.SetAttrBwd( *( pAttr = rHtArr.GetEnd( --nPos )) ) )
            {
                nSttPos = *pAttr->GetAnyEnd();
                if( nSttPos < rCmpArr.GetNdEnd() )
                {
                    // dann haben wir unser Ende:
                    nEndPos = rCmpArr.GetNdEnd();
                    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, sal_False );
                    return sal_True;
                }

                // ansonsten muessen wir weiter suchen
                break;
            }

        if( !nPos && rCmpArr.Found() )
        {
            // dann haben wir unseren Bereich
            nEndPos = rCmpArr.GetNdEnd();
            lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, sal_False );
            return sal_True;
        }
    }

    while( nPos )
        if( rCmpArr.SetAttrBwd( *( pAttr = rHtArr.GetEnd( --nPos )) ) )
        {
            // sollten noch mehr auf der gleichen Position anfangen ??
            // auch die noch mit testen !!
            if( nPos )
            {
                nEndPos = *pAttr->GetAnyEnd();
                while( --nPos && nEndPos ==
                        *( pAttr = rHtArr.GetEnd( nPos ))->GetAnyEnd() &&
                        rCmpArr.SetAttrBwd( *pAttr ) )
                    ;
            }
            if( !rCmpArr.Found() )
                continue;


            // dann haben wir den Bereich zusammen
            if( (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
                return sal_False;
            lcl_SetAttrPam( rPam, nSttPos, &nEndPos, sal_False );
            return sal_True;
        }

    if( !rCmpArr.CheckStack() ||
        (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
        return sal_False;
    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, sal_False );
    return sal_True;
}


int lcl_Search( const SwCntntNode& rCNd, const SfxItemSet& rCmpSet, sal_Bool bNoColls )
{
    // nur die harte Attributierung suchen ?
    if( bNoColls && !rCNd.HasSwAttrSet() )
        return sal_False;

    const SfxItemSet& rNdSet = rCNd.GetSwAttrSet();
    SfxItemIter aIter( rCmpSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    const SfxPoolItem* pNdItem;
    sal_uInt16 nWhich;

    while( sal_True )
    {
        // nur testen, ob vorhanden ist ?
        if( IsInvalidItem( pItem ))
        {
            nWhich = rCmpSet.GetWhichByPos( aIter.GetCurPos() );
            if( SFX_ITEM_SET != rNdSet.GetItemState( nWhich, !bNoColls, &pNdItem )
                || CmpAttr( *pNdItem, rNdSet.GetPool()->GetDefaultItem( nWhich ) ))
                return sal_False;
        }
        else
        {
            nWhich = pItem->Which();
//JP 27.02.95: wenn nach defaults gesucht wird, dann muss man bis zum Pool
//              runter
//          if( SFX_ITEM_SET != rNdSet.GetItemState( nWhich, !bNoColls, &pNdItem )
//              || *pNdItem != *pItem )
            if( !CmpAttr( rNdSet.Get( nWhich, !bNoColls ), *pItem ))
                return sal_False;
        }

        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }
    return sal_True;            // wurde gefunden
}


sal_Bool SwPaM::Find( const SfxPoolItem& rAttr, sal_Bool bValue, SwMoveFn fnMove,
                    const SwPaM *pRegion, sal_Bool bInReadOnly )
{
    // stelle fest welches Attribut gesucht wird:
    sal_uInt16 nWhich = rAttr.Which();
    int bCharAttr = isCHRATR(nWhich) || isTXTATR(nWhich);

    SwPaM* pPam = MakeRegion( fnMove, pRegion );

    sal_Bool bFound = sal_False;
    sal_Bool bFirst = sal_True;
    sal_Bool bSrchForward = fnMove == fnMoveForward;
    SwCntntNode * pNode;
    const SfxPoolItem* pItem;
    SwpFmts aFmtArr;

    // Wenn am Anfang/Ende, aus dem Node moven
    if( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetCntntNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( !(*fnMove->fnNds)( &pPam->GetPoint()->nNode, sal_False ))
        {
            delete pPam;
            return sal_False;
        }
        SwCntntNode *pNd = pPam->GetCntntNode();
        xub_StrLen nTmpPos = bSrchForward ? 0 : pNd->Len();
        pPam->GetPoint()->nContent.Assign( pNd, nTmpPos );
    }

    while( 0 != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ) )
    {
        if( bCharAttr )
        {
            if( !pNode->IsTxtNode() )       // CharAttr sind nur in TextNodes
                continue;

            if( ((SwTxtNode*)pNode)->HasHints() &&
                lcl_Search( *(SwTxtNode*)pNode, *pPam, rAttr, fnMove,  bValue ))
            {
                // setze auf die Werte vom Attribut
                SetMark();
                *GetPoint() = *pPam->GetPoint();
                *GetMark() = *pPam->GetMark();
                bFound = sal_True;
                break;
            }
            else if (isTXTATR(nWhich))
                continue;               // --> also weiter
        }

        // keine harte Attributierung, dann pruefe, ob die Vorlage schon
        // mal nach dem Attribut befragt wurde
        if( !pNode->HasSwAttrSet() )
        {
            const SwFmt* pTmpFmt = pNode->GetFmtColl();
            if( aFmtArr.Count() && aFmtArr.Seek_Entry( pTmpFmt ))
                continue;   // die Collection wurde schon mal befragt
            aFmtArr.Insert( pTmpFmt );
        }

        if( SFX_ITEM_SET == pNode->GetSwAttrSet().GetItemState( nWhich,
            sal_True, &pItem ) && ( !bValue || *pItem == rAttr ) )
        {
            // FORWARD:  Point an das Ende, GetMark zum Anfanf vom Node
            // BACKWARD: Point zum Anfang,  GetMark an das Ende vom Node
            // und immer nach der Logik: inkl. Start, exkl. End !!!
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            bFound = sal_True;
            break;
        }
    }

    // beim rueckwaerts Suchen noch Point und Mark vertauschen
    if( bFound && !bSrchForward )
        Exchange();

    delete pPam;
    return bFound;
}


typedef int (*FnSearchAttr)( const SwTxtNode&, SwAttrCheckArr&, SwPaM& );

sal_Bool SwPaM::Find( const SfxItemSet& rSet, sal_Bool bNoColls, SwMoveFn fnMove,
                    const SwPaM *pRegion, sal_Bool bInReadOnly, sal_Bool bMoveFirst )
{
    SwPaM* pPam = MakeRegion( fnMove, pRegion );

    sal_Bool bFound = sal_False;
    sal_Bool bFirst = sal_True;
    sal_Bool bSrchForward = fnMove == fnMoveForward;
    SwCntntNode * pNode;
    SwpFmts aFmtArr;

    // teste doch mal welche Text/Char-Attribute gesucht werden
    SwAttrCheckArr aCmpArr( rSet, bSrchForward, bNoColls );
    SfxItemSet aOtherSet( GetDoc()->GetAttrPool(),
                            RES_PARATR_BEGIN, RES_GRFATR_END-1 );
    aOtherSet.Put( rSet, sal_False );   // alle Invalid-Items erhalten!

    FnSearchAttr fnSearch = bSrchForward
                                ? (&::lcl_SearchForward)
                                : (&::lcl_SearchBackward);

    // Wenn am Anfang/Ende, aus dem Node moven
    // Wenn am Anfang/Ende, aus dem Node moven
    if( bMoveFirst &&
        ( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetCntntNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() ) )
    {
        if( !(*fnMove->fnNds)( &pPam->GetPoint()->nNode, sal_False ))
        {
            delete pPam;
            return sal_False;
        }
        SwCntntNode *pNd = pPam->GetCntntNode();
        xub_StrLen nTmpPos = bSrchForward ? 0 : pNd->Len();
        pPam->GetPoint()->nContent.Assign( pNd, nTmpPos );
    }


    while( 0 != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ) )
    {
        if( aCmpArr.Count() )
        {
            if( !pNode->IsTxtNode() )       // CharAttr sind nur in TextNodes
                continue;

            if( (!aOtherSet.Count() ||
                lcl_Search( *pNode, aOtherSet, bNoColls )) &&
                (*fnSearch)( *(SwTxtNode*)pNode, aCmpArr, *pPam ))
            {
                // setze auf die Werte vom Attribut
                SetMark();
                *GetPoint() = *pPam->GetPoint();
                *GetMark() = *pPam->GetMark();
                bFound = sal_True;
                break;
            }
            continue;       // TextAttribute
        }

        if( !aOtherSet.Count() )
            continue;

        // keine harte Attributierung, dann pruefe, ob die Vorlage schon
        // mal nach dem Attribut befragt wurde
        if( !pNode->HasSwAttrSet() )
        {
            const SwFmt* pTmpFmt = pNode->GetFmtColl();
            if( aFmtArr.Count() && aFmtArr.Seek_Entry( pTmpFmt ))
                continue;   // die Collection wurde schon mal befragt
            aFmtArr.Insert( pTmpFmt );
        }

        if( lcl_Search( *pNode, aOtherSet, bNoColls ))
        {
            // FORWARD:  Point an das Ende, GetMark zum Anfanf vom Node
            // BACKWARD: Point zum Anfang,  GetMark an das Ende vom Node
            // und immer nach der Logik: inkl. Start, exkl. End !!!
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            bFound = sal_True;
            break;
        }
    }

    // beim rueckwaerts Suchen noch Point und Mark vertauschen
    if( bFound && !bSrchForward )
        Exchange();

    delete pPam;
    return bFound;
}

//------------------ Methoden vom SwCursor ---------------------------

// Parameter fuer das Suchen vom Attributen
struct SwFindParaAttr : public SwFindParas
{
    sal_Bool bValue;
    const SfxItemSet *pSet, *pReplSet;
    const SearchOptions *pSearchOpt;
    SwCursor& rCursor;
    utl::TextSearch* pSTxt;

    SwFindParaAttr( const SfxItemSet& rSet, sal_Bool bNoCollection,
                    const SearchOptions* pOpt, const SfxItemSet* pRSet,
                    SwCursor& rCrsr )
        : bValue( bNoCollection ), pSet( &rSet ), pReplSet( pRSet ),
          pSearchOpt( pOpt ), rCursor( rCrsr ),pSTxt( 0 ) {}

    virtual ~SwFindParaAttr()   { delete pSTxt; }

    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, sal_Bool bInReadOnly );
    virtual int IsReplaceMode() const;
};


int SwFindParaAttr::Find( SwPaM* pCrsr, SwMoveFn fnMove, const SwPaM* pRegion,
                            sal_Bool bInReadOnly )
{
    // String ersetzen ?? (nur wenn Text angegeben oder nicht attributiert
    //                      gesucht wird)
    sal_Bool bReplaceTxt = pSearchOpt && ( pSearchOpt->replaceString.getLength() ||
                                    !pSet->Count() );
    sal_Bool bReplaceAttr = pReplSet && pReplSet->Count();
    sal_Bool bMoveFirst = !bReplaceAttr;
    if( bInReadOnly && (bReplaceAttr || bReplaceTxt ))
        bInReadOnly = sal_False;

    // wir suchen nach Attributen, soll zusaetzlich Text gesucht werden ?
    {
        SwPaM aRegion( *pRegion->GetMark(), *pRegion->GetPoint() );
        SwPaM* pTextRegion = &aRegion;
        SwPaM aSrchPam( *pCrsr->GetPoint() );

        while( sal_True )
        {
            if( pSet->Count() )         // gibts ueberhaupt Attributierung?
            {
                // zuerst die Attributierung
                if( !aSrchPam.Find( *pSet, bValue, fnMove, &aRegion, bInReadOnly, bMoveFirst ) )
//JP 17.11.95: was ist mit Attributen in leeren Absaetzen !!
//                  || *pCrsr->GetMark() == *pCrsr->GetPoint() )    // kein Bereich ??
                    return FIND_NOT_FOUND;
                bMoveFirst = sal_True;

                if( !pSearchOpt )
                    break;      // ok, nur Attribute, also gefunden

                pTextRegion = &aSrchPam;
            }
            else if( !pSearchOpt )
                return FIND_NOT_FOUND;

            // dann darin den Text
            if( !pSTxt )
            {
                SearchOptions aTmp( *pSearchOpt );

                // search in selection
                aTmp.searchFlag |= (SearchFlags::REG_NOT_BEGINOFLINE |
                                    SearchFlags::REG_NOT_ENDOFLINE);

                MsLangId::convertLanguageToLocale( LANGUAGE_SYSTEM, aTmp.Locale );

                pSTxt = new utl::TextSearch( aTmp );
            }

            // todo/mba: searching for attributes in Outliner text?!
            sal_Bool bSearchInNotes = sal_False;

            // Bug 24665: suche im richtigen Bereich weiter (pTextRegion!)
            if( aSrchPam.Find( *pSearchOpt, bSearchInNotes, *pSTxt, fnMove, pTextRegion, bInReadOnly ) &&
                *aSrchPam.GetMark() != *aSrchPam.GetPoint() )   // gefunden ?
                break;                                      // also raus
            else if( !pSet->Count() )
                return FIND_NOT_FOUND;      // nur Text und nicht gefunden

/*          // --> FME 2007-4-12 #i74765 # Why should we move the position?
            Moving the position results in bugs when there are two adjacent
            portions which both have the requested attributes set. I suspect this
            should be only be an optimization. Therefore I boldly remove it now!

            // JP: und wieder neu aufsetzen, aber eine Position weiter
            //JP 04.11.97: Bug 44897 - aber den Mark wieder aufheben, damit
            //              weiterbewegt werden kann!
            {
                sal_Bool bCheckRegion = sal_True;
                SwPosition* pPos = aSrchPam.GetPoint();
                if( !(*fnMove->fnNd)( &pPos->nNode.GetNode(),
                                        &pPos->nContent, CRSR_SKIP_CHARS ))
                {
                    if( (*fnMove->fnNds)( &pPos->nNode, sal_False ))
                    {
                        SwCntntNode *pNd = pPos->nNode.GetNode().GetCntntNode();
                        xub_StrLen nCPos;
                        if( fnMove == fnMoveForward )
                            nCPos = 0;
                        else
                            nCPos = pNd->Len();
                        pPos->nContent.Assign( pNd, nCPos );
                    }
                    else
                        bCheckRegion = sal_False;
                }
                if( !bCheckRegion || *aRegion.GetPoint() <= *pPos )
                    return FIND_NOT_FOUND;      // nicht gefunden
            }*/
            *aRegion.GetMark() = *aSrchPam.GetPoint();
        }

        *pCrsr->GetPoint() = *aSrchPam.GetPoint();
        pCrsr->SetMark();
        *pCrsr->GetMark() = *aSrchPam.GetMark();
    }

    if( bReplaceTxt )
    {
        const bool bRegExp(
                SearchAlgorithms_REGEXP == pSearchOpt->algorithmType);
        SwIndex& rSttCntIdx = pCrsr->Start()->nContent;
        xub_StrLen nSttCnt = rSttCntIdx.GetIndex();

        // damit die Region auch verschoben wird, in den Shell-Cursr-Ring
        // mit aufnehmen !!
        Ring *pPrevRing = 0;
        if( bRegExp )
        {
            pPrevRing = pRegion->GetPrev();
            ((Ring*)pRegion)->MoveRingTo( &rCursor );
        }

        ::std::auto_ptr<String> pRepl( (bRegExp) ?
                ReplaceBackReferences( *pSearchOpt, pCrsr ) : 0 );
        rCursor.GetDoc()->ReplaceRange( *pCrsr,
            (pRepl.get()) ? *pRepl : String(pSearchOpt->replaceString),
            bRegExp );
        rCursor.SaveTblBoxCntnt( pCrsr->GetPoint() );

        if( bRegExp )
        {
            // und die Region wieder herausnehmen:
            Ring *p, *pNext = (Ring*)pRegion;
            do {
                p = pNext;
                pNext = p->GetNext();
                p->MoveTo( (Ring*)pRegion );
            } while( p != pPrevRing );
        }
        rSttCntIdx = nSttCnt;
    }

    if( bReplaceAttr )
    {
        // --- Ist die Selection noch da ??????

        // und noch die Attribute setzen
#ifdef OLD
        pCrsr->GetDoc()->Insert( *pCrsr, *pReplSet, 0 );
#else
        //JP 13.07.95: alle gesuchten Attribute werden, wenn nicht im
        //              ReplaceSet angegeben, auf Default zurueck gesetzt

        if( !pSet->Count() )
        {
            pCrsr->GetDoc()->InsertItemSet( *pCrsr, *pReplSet, 0 );
        }
        else
        {
            SfxItemPool* pPool = pReplSet->GetPool();
            SfxItemSet aSet( *pPool, pReplSet->GetRanges() );

            SfxItemIter aIter( *pSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            while( sal_True )
            {
                // alle die nicht gesetzt sind mit Pool-Defaults aufuellen
                if( !IsInvalidItem( pItem ) && SFX_ITEM_SET !=
                    pReplSet->GetItemState( pItem->Which(), sal_False ))
                    aSet.Put( pPool->GetDefaultItem( pItem->Which() ));

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
            aSet.Put( *pReplSet );
            pCrsr->GetDoc()->InsertItemSet( *pCrsr, aSet, 0 );
        }
#endif
        return FIND_NO_RING;
    }

    else
        return FIND_FOUND;
}


int SwFindParaAttr::IsReplaceMode() const
{
    return ( pSearchOpt && pSearchOpt->replaceString.getLength() ) ||
           ( pReplSet && pReplSet->Count() );
}

// Suchen nach Attributen


sal_uLong SwCursor::Find( const SfxItemSet& rSet, sal_Bool bNoCollections,
                    SwDocPositions nStart, SwDocPositions nEnde, sal_Bool& bCancel,
                    FindRanges eFndRngs,
                    const SearchOptions* pSearchOpt, const SfxItemSet* pReplSet )
{
    // OLE-Benachrichtigung abschalten !!
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    sal_Bool bReplace = ( pSearchOpt && ( pSearchOpt->replaceString.getLength() ||
                                    !rSet.Count() ) ) ||
                    (pReplSet && pReplSet->Count());
    bool const bStartUndo = pDoc->GetIDocumentUndoRedo().DoesUndo() && bReplace;
    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_REPLACE, NULL );
    }

    SwFindParaAttr aSwFindParaAttr( rSet, bNoCollections, pSearchOpt,
                                    pReplSet, *this );

    sal_uLong nRet = FindAll(aSwFindParaAttr, nStart, nEnde, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );
    if( nRet && bReplace )
        pDoc->SetModified();

    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_REPLACE, NULL );
    }

    return nRet;
}



