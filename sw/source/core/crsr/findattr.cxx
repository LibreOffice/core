/*************************************************************************
 *
 *  $RCSfile: findattr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif



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


const SwTxtAttr* GetFrwrdTxtHint( const SwpHints& rHtsArr, USHORT& rPos,
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


const SwTxtAttr* GetBkwrdTxtHint( const SwpHints& rHtsArr, USHORT& rPos,
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
                        const BOOL bSaveMark )
{
    xub_StrLen nCntntPos;
    if( bSaveMark )
        nCntntPos = rPam.GetMark()->nContent.GetIndex();
    else
        nCntntPos = rPam.GetPoint()->nContent.GetIndex();
    FASTBOOL bTstEnde = rPam.GetPoint()->nNode == rPam.GetMark()->nNode;

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


FASTBOOL lcl_Search( const SwTxtNode& rTxtNd, SwPaM& rPam,
                    const SfxPoolItem& rCmpItem,
                    SwMoveFn fnMove, BOOL bValue )
{
    if ( !rTxtNd.HasHints() )
        return FALSE;
    const SwTxtAttr *pTxtHt = 0;
    FASTBOOL bForward = fnMove == fnMoveForward;
    USHORT nPos = bForward ? 0 : rTxtNd.GetSwpHints().Count();
    xub_StrLen nCntntPos = rPam.GetPoint()->nContent.GetIndex();

    while( 0 != ( pTxtHt=(*fnMove->fnGetHint)(rTxtNd.GetSwpHints(),nPos,nCntntPos)))
        if( pTxtHt->Which() == rCmpItem.Which() &&
            ( !bValue || CmpAttr( pTxtHt->GetAttr(), rCmpItem )))
        {
            lcl_SetAttrPam( rPam, *pTxtHt->GetStart(), pTxtHt->GetEnd(), bForward );
            return TRUE;
        }
    return FALSE;
}


//------------------ Suche nach mehren Text Attributen -------------------

struct _SwSrchChrAttr
{
    USHORT nWhich;
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
    USHORT nArrStart, nArrLen;
    USHORT nFound, nStackCnt;
    SfxItemSet aCmpSet;
    BOOL bNoColls;
    BOOL bForward;

public:
    SwAttrCheckArr( const SfxItemSet& rSet, int bForward, int bNoCollections );
    ~SwAttrCheckArr();

    void SetNewSet( const SwTxtNode& rTxtNd, const SwPaM& rPam );

    // wieviele Attribute ueberhaupt ??
    USHORT Count() const    { return aCmpSet.Count(); }
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
    aCmpSet.Put( rSet, FALSE );
    bNoColls = bNoCollections;

    bForward = bFwd;

    // Bestimmen den Bereich des Fnd/Stack-Arrays (Min/Max)
    SfxItemIter aIter( aCmpSet );
    nArrStart = aCmpSet.GetWhichByPos( aIter.GetFirstPos() );
    nArrLen = aCmpSet.GetWhichByPos( aIter.GetLastPos() ) - nArrStart+1;

    pFndArr = (_SwSrchChrAttr*)new char[ nArrLen * sizeof(_SwSrchChrAttr) ];
    pStackArr = (_SwSrchChrAttr*)new char[ nArrLen * sizeof(_SwSrchChrAttr) ];
}


SwAttrCheckArr::~SwAttrCheckArr()
{
    delete (void*) pFndArr;
    delete (void*) pStackArr;
}



#pragma optimize( "e", off )

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

    if( bNoColls && !rTxtNd.GetpSwAttrSet() )
        return ;

    const SfxItemSet& rSet = rTxtNd.GetSwAttrSet();
//  if( !rSet.Count() )
//      return;

    SfxItemIter aIter( aCmpSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    const SfxPoolItem* pFndItem;
    USHORT nWhich;

    while( TRUE )
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
#pragma optimize( "", on )


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
    USHORT nWhch = rAttr.Which();
    SfxWhichIter* pIter = NULL;
    const SfxPoolItem* pTmpItem;
    const SwAttrSet* pSet;
    if( RES_TXTATR_CHARFMT == nWhch )
    {
        if( bNoColls )
            return Found();
        SwCharFmt* pFmt = rAttr.GetCharFmt().GetCharFmt();
        if( pFmt )
        {
            pSet = &pFmt->GetAttrSet();
            pIter = new SfxWhichIter( *pSet );
            nWhch = pIter->FirstWhich();
            while( nWhch &&
                SFX_ITEM_SET != pSet->GetItemState( nWhch, TRUE, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                pTmpItem = NULL;
        }
        else
            pTmpItem = NULL;
    }
    else
        pTmpItem = &rAttr.GetAttr();
    while( pTmpItem )
    {
        SfxItemState eState = aCmpSet.GetItemState( nWhch, FALSE, &pItem );
        if( SFX_ITEM_DONTCARE == eState || SFX_ITEM_SET == eState )
        {
            register USHORT n;
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

            BOOL bContinue = FALSE;

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

                    bContinue = TRUE;
                }
            }
            // wird Attribut gueltig ?
            else if(  CmpAttr( *pItem, *pTmpItem ) )
            {
                pFndArr[ nWhch - nArrStart ] = aTmp;
                ++nFound;
                bContinue = TRUE;
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
                SFX_ITEM_SET != pSet->GetItemState( nWhch, TRUE, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                break;
        }
        else
            break;
    }
// --------------------------------------------------------------
#ifdef USED
    {
        SfxItemState eState = aCmpSet.GetItemState( rAttr.Which(), FALSE, &pItem );
        if( SFX_ITEM_DONTCARE != eState && SFX_ITEM_SET != eState )
            return Found();

        register USHORT n, nWhich = rAttr.Which();
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


        if( SFX_ITEM_DONTCARE == eState  )
        {
            // wird Attribut gueltig ?
            if( !CmpAttr( aCmpSet.GetPool()->GetDefaultItem( nWhich ),
                rAttr.GetAttr() ))
            {
                // suche das Attribut und erweiter es gegebenenfalls
                if( !( pCmp = &pFndArr[ nWhich - nArrStart ])->nWhich )
                {
                    *pCmp = aTmp;               // nicht gefunden, eintragen
                    nFound++;
                }
                else if( pCmp->nEnd < aTmp.nEnd )       // erweitern ?
                    pCmp->nEnd = aTmp.nEnd;

                return Found();
            }
        }
        // wird Attribut gueltig ?
        else if(  CmpAttr( *pItem, rAttr.GetAttr() ) )
        {
            pFndArr[ nWhich - nArrStart ] = aTmp;
            return ++nFound == aCmpSet.Count();
        }

        // tja, dann muss es auf den Stack
        if( ( pCmp = &pFndArr[ nWhich - nArrStart ])->nWhich )
        {
            // vorhanden, auf den Stack. Aber nur wenn es noch grosser ist
            if( pCmp->nEnd > aTmp.nEnd )
            {
                ASSERT( !pStackArr[ nWhich - nArrStart ].nWhich,
                                "Stack-Platz ist noch belegt" );

    // ---------
    // JP 22.08.96: nur Ende manipulieren reicht nicht. Bug 30547
    //          pCmp->nStt = aTmp.nEnd;
                if( aTmp.nStt <= pCmp->nStt )
                    pCmp->nStt = aTmp.nEnd;
                else
                    pCmp->nEnd = aTmp.nStt;
    // ---------

                pStackArr[ nWhich - nArrStart ] = *pCmp;
                nStackCnt++;
            }
            pCmp->nWhich = 0;
            nFound--;
        }
    }
#endif
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
    USHORT nWhch = rAttr.Which();
    SfxWhichIter* pIter = NULL;
    const SfxPoolItem* pTmpItem;
    const SwAttrSet* pSet;
    if( RES_TXTATR_CHARFMT == nWhch )
    {
        if( bNoColls )
            return Found();
        SwCharFmt* pFmt = rAttr.GetCharFmt().GetCharFmt();
        if( pFmt )
        {
            pSet = &pFmt->GetAttrSet();
            pIter = new SfxWhichIter( *pSet );
            nWhch = pIter->FirstWhich();
            while( nWhch &&
                SFX_ITEM_SET != pSet->GetItemState( nWhch, TRUE, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                pTmpItem = NULL;
        }
        else
            pTmpItem = NULL;
    }
    else
        pTmpItem = &rAttr.GetAttr();
    while( pTmpItem )
    {
        SfxItemState eState = aCmpSet.GetItemState( nWhch, FALSE, &pItem );
        if( SFX_ITEM_DONTCARE == eState || SFX_ITEM_SET == eState )
        {
            register USHORT n;
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

            BOOL bContinue = FALSE;
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

                    bContinue = TRUE;
                }
            }
            // wird Attribut gueltig ?
            else if( CmpAttr( *pItem, *pTmpItem ))
            {
                pFndArr[ nWhch - nArrStart ] = aTmp;
                ++nFound;
                bContinue = TRUE;
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
                SFX_ITEM_SET != pSet->GetItemState( nWhch, TRUE, &pTmpItem ) )
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
    for( USHORT n = 0; n < nArrLen; ++n, ++pArrPtr )
        if( pArrPtr->nWhich && pArrPtr->nStt > nStart )
            nStart = pArrPtr->nStt;

    return nStart;
}


xub_StrLen SwAttrCheckArr::End() const
{
    _SwSrchChrAttr* pArrPtr = pFndArr;
    xub_StrLen nEnd = nNdEnd;
    for( USHORT n = 0; n < nArrLen; ++n, ++pArrPtr )
        if( pArrPtr->nWhich && pArrPtr->nEnd < nEnd )
            nEnd = pArrPtr->nEnd;

    return nEnd;
}


int SwAttrCheckArr::CheckStack()
{
    if( !nStackCnt )
        return FALSE;

    USHORT n;
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
            return FALSE;
        nEndPos = rCmpArr.GetNdEnd();
        lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, TRUE );
        return TRUE;
    }

    // dann gehe mal durch das nach "Start" sortierte Array
    const SwpHints& rHtArr = rTxtNd.GetSwpHints();
    const SwTxtAttr* pAttr;
    USHORT nPos = 0;

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
                                pAttr->GetStart(), TRUE );
                    return TRUE;
                }
                // ansonsten muessen wir weiter suchen
                break;
            }

        if( nPos == rHtArr.Count() && rCmpArr.Found() )
        {
            // dann haben wir unseren Bereich
            nEndPos = rCmpArr.GetNdEnd();
            lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, TRUE );
            return TRUE;
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
                return FALSE;
            lcl_SetAttrPam( rPam, nSttPos, &nEndPos, TRUE );
            return TRUE;
        }

    if( !rCmpArr.CheckStack() ||
        (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
        return FALSE;
    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, TRUE );
    return TRUE;
}


int lcl_SearchBackward( const SwTxtNode& rTxtNd, SwAttrCheckArr& rCmpArr,
                            SwPaM& rPam )
{
    xub_StrLen nEndPos, nSttPos;
    rCmpArr.SetNewSet( rTxtNd, rPam );
    if( !rTxtNd.HasHints() )
    {
        if( !rCmpArr.Found() )
            return FALSE;
        nEndPos = rCmpArr.GetNdEnd();
        lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, FALSE );
        return TRUE;
    }

    // dann gehe mal durch das nach "Start" sortierte Array
    const SwpHints& rHtArr = rTxtNd.GetSwpHints();
    const SwTxtAttr* pAttr;
    USHORT nPos = rHtArr.Count();

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
                    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, FALSE );
                    return TRUE;
                }

                // ansonsten muessen wir weiter suchen
                break;
            }

        if( !nPos && rCmpArr.Found() )
        {
            // dann haben wir unseren Bereich
            nEndPos = rCmpArr.GetNdEnd();
            lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, FALSE );
            return TRUE;
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
                return FALSE;
            lcl_SetAttrPam( rPam, nSttPos, &nEndPos, FALSE );
            return TRUE;
        }

    if( !rCmpArr.CheckStack() ||
        (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
        return FALSE;
    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, FALSE );
    return TRUE;
}


int lcl_Search( const SwCntntNode& rCNd, SwPaM& rPam,
            const SfxItemSet& rCmpSet, BOOL bNoColls  )
{
    // nur die harte Attributierung suchen ?
    if( bNoColls && !rCNd.GetpSwAttrSet() )
        return FALSE;

    const SfxItemSet& rNdSet = rCNd.GetSwAttrSet();
    SfxItemIter aIter( rCmpSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    const SfxPoolItem* pNdItem;
    USHORT nWhich;

    while( TRUE )
    {
        // nur testen, ob vorhanden ist ?
        if( IsInvalidItem( pItem ))
        {
            nWhich = rCmpSet.GetWhichByPos( aIter.GetCurPos() );
            if( SFX_ITEM_SET != rNdSet.GetItemState( nWhich, !bNoColls, &pNdItem )
                || CmpAttr( *pNdItem, rNdSet.GetPool()->GetDefaultItem( nWhich ) ))
                return FALSE;
        }
        else
        {
            nWhich = pItem->Which();
//JP 27.02.95: wenn nach defaults gesucht wird, dann muss man bis zum Pool
//              runter
//          if( SFX_ITEM_SET != rNdSet.GetItemState( nWhich, !bNoColls, &pNdItem )
//              || *pNdItem != *pItem )
            if( !CmpAttr( rNdSet.Get( nWhich, !bNoColls ), *pItem ))
                return FALSE;
        }

        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }
    return TRUE;            // wurde gefunden
}


FASTBOOL SwPaM::Find( const SfxPoolItem& rAttr, FASTBOOL bValue, SwMoveFn fnMove,
                    const SwPaM *pRegion, FASTBOOL bInReadOnly )
{
    // stelle fest welches Attribut gesucht wird:
    USHORT nWhich = rAttr.Which();
    int bCharAttr = RES_CHRATR_BEGIN <= nWhich && nWhich < RES_TXTATR_END;

    SwPaM* pPam = MakeRegion( fnMove, pRegion );

    FASTBOOL bFound = FALSE;
    FASTBOOL bFirst = TRUE;
    FASTBOOL bSrchForward = fnMove == fnMoveForward;
    SwCntntNode * pNode;
    const SfxPoolItem* pItem;
    SwpFmts aFmtArr;

    // Wenn am Anfang/Ende, aus dem Node moven
    if( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetCntntNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( !(*fnMove->fnNds)( &pPam->GetPoint()->nNode, FALSE ))
        {
            delete pPam;
            return FALSE;
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
                bFound = TRUE;
                break;
            }
            else if( RES_TXTATR_BEGIN < nWhich )   // TextAttribut
                continue;               // --> also weiter
        }

        // keine harte Attributierung, dann pruefe, ob die Vorlage schon
        // mal nach dem Attribut befragt wurde
        if( !pNode->GetpSwAttrSet() )
        {
            const SwFmt* pTmpFmt = pNode->GetFmtColl();
            if( aFmtArr.Count() && aFmtArr.Seek_Entry( pTmpFmt ))
                continue;   // die Collection wurde schon mal befragt
            aFmtArr.Insert( pTmpFmt );
        }

        if( SFX_ITEM_SET == pNode->GetSwAttrSet().GetItemState( nWhich,
            TRUE, &pItem ) && ( !bValue || *pItem == rAttr ) )
        {
            // FORWARD:  Point an das Ende, GetMark zum Anfanf vom Node
            // BACKWARD: Point zum Anfang,  GetMark an das Ende vom Node
            // und immer nach der Logik: inkl. Start, exkl. End !!!
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            Move( fnMoveForward, fnGoCntnt );
            bFound = TRUE;
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

FASTBOOL SwPaM::Find( const SfxItemSet& rSet, FASTBOOL bNoColls, SwMoveFn fnMove,
                    const SwPaM *pRegion, FASTBOOL bInReadOnly )
{
    SwPaM* pPam = MakeRegion( fnMove, pRegion );

    FASTBOOL bFound = FALSE;
    FASTBOOL bFirst = TRUE;
    FASTBOOL bSrchForward = fnMove == fnMoveForward;
    SwCntntNode * pNode;
    SwpFmts aFmtArr;

    // teste doch mal welche Text/Char-Attribute gesucht werden
    SwAttrCheckArr aCmpArr( rSet, bSrchForward, bNoColls );
    SfxItemSet aOtherSet( GetDoc()->GetAttrPool(),
                            RES_PARATR_BEGIN, RES_GRFATR_END-1 );
    aOtherSet.Put( rSet, FALSE );   // alle Invalid-Items erhalten!

    FnSearchAttr fnSearch = bSrchForward
                                ? (&::lcl_SearchForward)
                                : (&::lcl_SearchBackward);

    // Wenn am Anfang/Ende, aus dem Node moven
    // Wenn am Anfang/Ende, aus dem Node moven
    if( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetCntntNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( !(*fnMove->fnNds)( &pPam->GetPoint()->nNode, FALSE ))
        {
            delete pPam;
            return FALSE;
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
                lcl_Search( *pNode, *pPam, aOtherSet, bNoColls )) &&
                (*fnSearch)( *(SwTxtNode*)pNode, aCmpArr, *pPam ))
            {
                // setze auf die Werte vom Attribut
                SetMark();
                *GetPoint() = *pPam->GetPoint();
                *GetMark() = *pPam->GetMark();
                bFound = TRUE;
                break;
            }
            continue;       // TextAttribute
        }

        if( !aOtherSet.Count() )
            continue;

        // keine harte Attributierung, dann pruefe, ob die Vorlage schon
        // mal nach dem Attribut befragt wurde
        if( !pNode->GetpSwAttrSet() )
        {
            const SwFmt* pTmpFmt = pNode->GetFmtColl();
            if( aFmtArr.Count() && aFmtArr.Seek_Entry( pTmpFmt ))
                continue;   // die Collection wurde schon mal befragt
            aFmtArr.Insert( pTmpFmt );
        }

        if( lcl_Search( *pNode, *pPam, aOtherSet, bNoColls ))
        {
            // FORWARD:  Point an das Ende, GetMark zum Anfanf vom Node
            // BACKWARD: Point zum Anfang,  GetMark an das Ende vom Node
            // und immer nach der Logik: inkl. Start, exkl. End !!!
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            Move( fnMoveForward, fnGoCntnt );
            bFound = TRUE;
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
    BOOL bValue;
    const SfxItemSet *pSet, *pReplSet;
    const SearchParam *pTxtPara;
    SwCursor& rCursor;
    SearchText* pSTxt;

    SwFindParaAttr( const SfxItemSet& rSet, BOOL bNoCollection,
                    const SearchParam* pTextParam, const SfxItemSet* pRSet,
                    SwCursor& rCrsr )
        : pSet( &rSet ), pReplSet( pRSet ), rCursor( rCrsr ),
            bValue( bNoCollection ), pTxtPara( pTextParam ), pSTxt( 0 )
        {}
    ~SwFindParaAttr()   { delete pSTxt; }

    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, FASTBOOL bInReadOnly );
    virtual int IsReplaceMode() const;
};


int SwFindParaAttr::Find( SwPaM* pCrsr, SwMoveFn fnMove, const SwPaM* pRegion,
                            FASTBOOL bInReadOnly )
{
    // String ersetzen ?? (nur wenn Text angegeben oder nicht attributiert
    //                      gesucht wird)
    BOOL bReplaceTxt = pTxtPara && ( pTxtPara->GetReplaceStr().Len() ||
                                    !pSet->Count() );
    BOOL bReplaceAttr = pReplSet && pReplSet->Count();
    if( bInReadOnly && (bReplaceAttr || bReplaceTxt ))
        bInReadOnly = FALSE;

    // wir suchen nach Attributen, soll zusaetzlich Text gesucht werden ?
    {
        SwPaM aRegion( *pRegion->GetMark(), *pRegion->GetPoint() );
        SwPaM* pTextRegion = &aRegion;

        while( TRUE )
        {
            if( pSet->Count() )         // gibts ueberhaupt Attributierung?
            {
                // zuerst die Attributierung
                if( !pCrsr->Find( *pSet, bValue, fnMove, &aRegion, bInReadOnly ) )
//JP 17.11.95: was ist mit Attributen in leeren Absaetzen !!
//                  || *pCrsr->GetMark() == *pCrsr->GetPoint() )    // kein Bereich ??
                    return FIND_NOT_FOUND;

                if( !pTxtPara )
                    break;      // ok, nur Attribute, also gefunden

                pTextRegion = pCrsr;
            }
            else if( !pTxtPara )
                return FIND_NOT_FOUND;

            // dann darin den Text
            if( !pSTxt )
            {
                SearchParam aTmp( *pTxtPara );
                aTmp.SetSrchInSelection( TRUE );
                pSTxt = new SearchText( aTmp, Application::GetAppInternational());
            }
            // Bug 24665: suche im richtigen Bereich weiter (pTextRegion!)
            if( pCrsr->Find( *pTxtPara, *pSTxt, fnMove, pTextRegion, bInReadOnly ) &&
                *pCrsr->GetMark() != *pCrsr->GetPoint() )   // gefunden ?
                break;                                      // also raus
            else if( !pSet->Count() )
                return FIND_NOT_FOUND;      // nur Text und nicht gefunden

            // und wieder neu aufsetzen, aber eine Position weiter
            //JP 04.11.97: Bug 44897 - aber den Mark wieder aufheben, damit
            //              weiterbewegt werden kann!
            {
                BOOL bCheckRegion = TRUE;
                SwPosition* pPos = pCrsr->GetPoint();
                if( !(*fnMove->fnNd)( &pPos->nNode.GetNode(), &pPos->nContent ))
                {
                    if( (*fnMove->fnNds)( &pPos->nNode, FALSE ))
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
                        bCheckRegion = FALSE;
                }
                if( !bCheckRegion || *aRegion.GetPoint() <= *pPos )
                    return FIND_NOT_FOUND;      // nicht gefunden
            }
            *aRegion.GetMark() = *pCrsr->GetPoint();
        }
    }

    if( bReplaceTxt )
    {
        int bRegExp = SearchParam::SRCH_REGEXP == pTxtPara->GetSrchType();
        SwIndex& rSttCntIdx = pCrsr->Start()->nContent;
        xub_StrLen nSttCnt = rSttCntIdx.GetIndex();

        // damit die Region auch verschoben wird, in den Shell-Cursr-Ring
        // mit aufnehmen !!
        Ring *pPrev;
        if( bRegExp )
        {
            pPrev = pRegion->GetPrev();
            ((Ring*)pRegion)->MoveRingTo( &rCursor );
        }

        rCursor.GetDoc()->Replace( *pCrsr, pTxtPara->GetReplaceStr(), bRegExp );
        rCursor.SaveTblBoxCntnt( pCrsr->GetPoint() );

        if( bRegExp )
        {
            // und die Region wieder herausnehmen:
            Ring *p, *pNext = (Ring*)pRegion;
            do {
                p = pNext;
                pNext = p->GetNext();
                p->MoveTo( (Ring*)pRegion );
            } while( p != pPrev );
        }
        rSttCntIdx = nSttCnt;
    }

    if( bReplaceAttr )
    {
        // --- Ist die Selection noch da ??????

        // und noch die Attribute setzen
#ifdef OLD
        pCrsr->GetDoc()->Insert( *pCrsr, *pReplSet );
#else
        //JP 13.07.95: alle gesuchten Attribute werden, wenn nicht im
        //              ReplaceSet angegeben, auf Default zurueck gesetzt

        if( !pSet->Count() )
            pCrsr->GetDoc()->Insert( *pCrsr, *pReplSet );
        else
        {
            SfxItemPool* pPool = pReplSet->GetPool();
            SfxItemSet aSet( *pPool, pReplSet->GetRanges() );

            SfxItemIter aIter( *pSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            while( TRUE )
            {
                // alle die nicht gesetzt sind mit Pool-Defaults aufuellen
                if( !IsInvalidItem( pItem ) && SFX_ITEM_SET !=
                    pReplSet->GetItemState( pItem->Which(), FALSE ))
                    aSet.Put( pPool->GetDefaultItem( pItem->Which() ));

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
            aSet.Put( *pReplSet );
            pCrsr->GetDoc()->Insert( *pCrsr, aSet );
        }
#endif
        return FIND_NO_RING;
    }

    else
        return FIND_FOUND;
}


int SwFindParaAttr::IsReplaceMode() const
{
    return ( pTxtPara && pTxtPara->GetReplaceStr().Len() ) ||
           ( pReplSet && pReplSet->Count() );
}

// Suchen nach Attributen


ULONG SwCursor::Find( const SfxItemSet& rSet, FASTBOOL bNoCollections,
                    SwDocPositions nStart, SwDocPositions nEnde,
                    FindRanges eFndRngs,
                    const SearchParam* pTextPara, const SfxItemSet* pReplSet )
{
    // OLE-Benachrichtigung abschalten !!
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    BOOL bReplace = ( pTextPara && ( pTextPara->GetReplaceStr().Len() ||
                                    !rSet.Count() ) ) ||
                    (pReplSet && pReplSet->Count());
    BOOL bSttUndo = pDoc->DoesUndo() && bReplace;
    if( bSttUndo )
        pDoc->StartUndo( UNDO_REPLACE );

    SwFindParaAttr aSwFindParaAttr( rSet, bNoCollections, pTextPara,
                                    pReplSet, *this );

    ULONG nRet = FindAll(aSwFindParaAttr, nStart, nEnde, eFndRngs );
    pDoc->SetOle2Link( aLnk );
    if( nRet && bReplace )
        pDoc->SetModified();

    if( bSttUndo )
        pDoc->EndUndo( UNDO_REPLACE );

    return nRet;
}



