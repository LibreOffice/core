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

typedef std::set<SwFmt*> SwpFmts;

// Special case for SvxFontItem: only compare the name
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
        // the start of an attribute has to be in the section
        if( *pTxtHt->GetStart() >= nCntntPos )
            return pTxtHt; // valid text attribute
    }
    return 0; // invalid text attribute
}


const SwTxtAttr* GetBkwrdTxtHint( const SwpHints& rHtsArr, sal_uInt16& rPos,
                                  xub_StrLen nCntntPos )
{
    while( rPos > 0 )
    {
        const SwTxtAttr *pTxtHt = rHtsArr.GetStart( --rPos );
        // the start of an attribute has to be in the section
        if( *pTxtHt->GetStart() < nCntntPos )
            return pTxtHt; // valid text attribute
    }
    return 0; // invalid text attribute
}

void lcl_SetAttrPam( SwPaM& rPam, xub_StrLen nStart, const xub_StrLen* pEnd,
                     const sal_Bool bSaveMark )
{
    xub_StrLen nCntntPos;
    if( bSaveMark )
        nCntntPos = rPam.GetMark()->nContent.GetIndex();
    else
        nCntntPos = rPam.GetPoint()->nContent.GetIndex();
    bool bTstEnd = rPam.GetPoint()->nNode == rPam.GetMark()->nNode;

    SwCntntNode* pCNd = rPam.GetCntntNode();
    rPam.GetPoint()->nContent.Assign( pCNd, nStart );
    rPam.SetMark(); // Point == GetMark

    // Point points to end of search area or end of attribute
    if( pEnd )
    {
        if( bTstEnd && *pEnd > nCntntPos )
            rPam.GetPoint()->nContent = nCntntPos;
        else
            rPam.GetPoint()->nContent = *pEnd;
    }
}

// TODO: provide documentation
/** search for a text attribute

    This function searches in a text node for a given attribute.
    If that is found then the SwPaM contains the section that surrounds the
    attribute (w.r.t. the search area).

    @param rTxtNd   Text node to search in.
    @param rPam     ???
    @param rCmpItem ???
    @param fnMove   ???
    @param bValue   ???
    @return Returns <true> if found, <false> otherwise.
*/
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

/// search for multiple text attributes
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

    /// how many attributes are there in total?
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

    // determine area of Fnd/Stack array (Min/Max)
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

    SfxItemIter aIter( aCmpSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    const SfxPoolItem* pFndItem;
    sal_uInt16 nWhich;

    while( sal_True )
    {
        if( IsInvalidItem( pItem ) )
        {
            nWhich = aCmpSet.GetWhichByPos( aIter.GetCurPos() );
            if( RES_TXTATR_END <= nWhich )
                break; // end of text attributes

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
                break; // end of text attributes

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

static bool
lcl_IsAttributeIgnorable(xub_StrLen const nNdStart, xub_StrLen const nNdEnd,
        _SwSrchChrAttr const& rTmp)
{
    // #i115528#: if there is a paragraph attribute, it has been added by the
    // SwAttrCheckArr ctor, and nFound is 1.
    // if the paragraph is entirely covered by hints that override the paragraph
    // attribute, then this function must find an attribute to decrement nFound!
    // so check for an empty search range, let attributes that start/end there
    // cover it, and hope for the best...
    return ((nNdEnd == nNdStart)
            ? ((rTmp.nEnd <  nNdStart) || (nNdEnd <  rTmp.nStt))
            : ((rTmp.nEnd <= nNdStart) || (nNdEnd <= rTmp.nStt)));
}

int SwAttrCheckArr::SetAttrFwd( const SwTxtAttr& rAttr )
{
    _SwSrchChrAttr aTmp( rAttr.GetAttr(), *rAttr.GetStart(), *rAttr.GetAnyEnd() );

    // ignore all attributes not in search range
    if (lcl_IsAttributeIgnorable(nNdStt, nNdEnd, aTmp))
    {
        return Found();
    }

    const SfxPoolItem* pItem;
    // here we explicitly also search in character templates
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

            // first delete all up to start position that are already invalid
            _SwSrchChrAttr* pArrPtr;
            if( nFound )
                for( pArrPtr = pFndArr, n = 0; n < nArrLen;
                    ++n, ++pArrPtr )
                    if( pArrPtr->nWhich && pArrPtr->nEnd <= aTmp.nStt )
                    {
                        pArrPtr->nWhich = 0; // deleted
                        nFound--;
                    }

            // delete all up to start position that are already invalid and
            // move all "open" ones (= stick out over start position) from stack
            // into FndSet
            if( nStackCnt )
                for( pArrPtr = pStackArr, n=0; n < nArrLen; ++n, ++pArrPtr )
                {
                    if( !pArrPtr->nWhich )
                        continue;

                    if( pArrPtr->nEnd <= aTmp.nStt )
                    {
                        pArrPtr->nWhich = 0; // deleted
                        if( !--nStackCnt )
                            break;
                    }
                    else if( pArrPtr->nStt <= aTmp.nStt )
                    {
                        if( ( pCmp = &pFndArr[ n ])->nWhich )
                        {
                            if( pCmp->nEnd < pArrPtr->nEnd ) // extend
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

            bool bContinue = false;

            if( SFX_ITEM_DONTCARE == eState  )
            {
                // Will the attribute become valid?
                if( !CmpAttr( aCmpSet.GetPool()->GetDefaultItem( nWhch ),
                    *pTmpItem ))
                {
                    // search attribute and extend if needed
                    if( !( pCmp = &pFndArr[ nWhch - nArrStart ])->nWhich )
                    {
                        *pCmp = aTmp; // not found, insert
                        nFound++;
                    }
                    else if( pCmp->nEnd < aTmp.nEnd ) // extend?
                        pCmp->nEnd = aTmp.nEnd;

                    bContinue = true;
                }
            }
            // Will the attribute become valid?
            else if(  CmpAttr( *pItem, *pTmpItem ) )
            {
                pFndArr[ nWhch - nArrStart ] = aTmp;
                ++nFound;
                bContinue = true;
            }

            // then is has to go on the stack
            if( !bContinue && ( pCmp = &pFndArr[ nWhch - nArrStart ])->nWhich )
            {
                // exists on stack, only if it is even bigger
                if( pCmp->nEnd > aTmp.nEnd )
                {
                    OSL_ENSURE( !pStackArr[ nWhch - nArrStart ].nWhich,
                                    "slot on stack is still in use" );

                    if( aTmp.nStt <= pCmp->nStt )
                        pCmp->nStt = aTmp.nEnd;
                    else
                        pCmp->nEnd = aTmp.nStt;

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

    // ignore all attributes not in search range
    if (lcl_IsAttributeIgnorable(nNdStt, nNdEnd, aTmp))
    {
        return Found();
    }

    const SfxPoolItem* pItem;
    // here we explicitly also search in character templates
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

            // first delete all up to start position that are already invalid
            _SwSrchChrAttr* pArrPtr;
            if( nFound )
                for( pArrPtr = pFndArr, n = 0; n < nArrLen; ++n, ++pArrPtr )
                    if( pArrPtr->nWhich && pArrPtr->nStt >= aTmp.nEnd )
                    {
                        pArrPtr->nWhich = 0; // deleted
                        nFound--;
                    }

            // delete all up to start position that are already invalid and
            // move all "open" ones (= stick out over start position) from stack
            // into FndSet
            if( nStackCnt )
                for( pArrPtr = pStackArr, n = 0; n < nArrLen; ++n, ++pArrPtr )
                {
                    if( !pArrPtr->nWhich )
                        continue;

                    if( pArrPtr->nStt >= aTmp.nEnd )
                    {
                        pArrPtr->nWhich = 0; // deleted
                        if( !--nStackCnt )
                            break;
                    }
                    else if( pArrPtr->nEnd >= aTmp.nEnd )
                    {
                        if( ( pCmp = &pFndArr[ n ])->nWhich )
                        {
                            if( pCmp->nStt > pArrPtr->nStt ) // extend
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
                // Will the attribute become valid?
                if( !CmpAttr( aCmpSet.GetPool()->GetDefaultItem( nWhch ),
                    *pTmpItem ) )
                {
                    // search attribute and extend if needed
                    if( !( pCmp = &pFndArr[ nWhch - nArrStart ])->nWhich )
                    {
                        *pCmp = aTmp; // not found, insert
                        nFound++;
                    }
                    else if( pCmp->nStt > aTmp.nStt ) // extend?
                        pCmp->nStt = aTmp.nStt;

                    bContinue = sal_True;
                }
            }
            // Will the attribute become valid?
            else if( CmpAttr( *pItem, *pTmpItem ))
            {
                pFndArr[ nWhch - nArrStart ] = aTmp;
                ++nFound;
                bContinue = sal_True;
            }

            // then is has to go on the stack
            if( !bContinue && ( pCmp = &pFndArr[ nWhch - nArrStart ])->nWhich )
            {
                // exists on stack, only if it is even bigger
                if( pCmp->nStt < aTmp.nStt )
                {
                    OSL_ENSURE( !pStackArr[ nWhch - nArrStart ].nWhich,
                            "slot on stack is still in use" );

                    if( aTmp.nEnd <= pCmp->nEnd )
                        pCmp->nEnd = aTmp.nStt;
                    else
                        pCmp->nStt = aTmp.nEnd;

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
            pArrPtr->nWhich = 0; // deleted
            if( !--nStackCnt )
                return nFound == aCmpSet.Count();
        }
        else if( bForward ? pArrPtr->nStt < nEndPos : pArrPtr->nEnd > nSttPos )
        {
            // move all "open" ones (= stick out over start position) into FndSet
            OSL_ENSURE( !pFndArr[ n ].nWhich, "slot in array is already in use" );
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

    const SwpHints& rHtArr = rTxtNd.GetSwpHints();
    const SwTxtAttr* pAttr;
    sal_uInt16 nPos = 0;

    // if everything is already there then check with which it will be ended
    if( rCmpArr.Found() )
    {
        for( ; nPos < rHtArr.Count(); ++nPos )
            if( !rCmpArr.SetAttrFwd( *( pAttr = rHtArr.GetStart( nPos )) ) )
            {
                if( rCmpArr.GetNdStt() < *pAttr->GetStart() )
                {
                    // found end
                    lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(),
                                pAttr->GetStart(), sal_True );
                    return sal_True;
                }
                // continue search
                break;
            }

        if( nPos == rHtArr.Count() && rCmpArr.Found() )
        {
            // found
            nEndPos = rCmpArr.GetNdEnd();
            lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, sal_True );
            return sal_True;
        }
    }

    for( ; nPos < rHtArr.Count(); ++nPos )
        if( rCmpArr.SetAttrFwd( *( pAttr = rHtArr.GetStart( nPos )) ) )
        {
            // Do multiple start at that position? Do also check those:
            nSttPos = *pAttr->GetStart();
            while( ++nPos < rHtArr.Count() && nSttPos ==
                    *( pAttr = rHtArr.GetStart( nPos ))->GetStart() &&
                    rCmpArr.SetAttrFwd( *pAttr ) )
                ;
            if( !rCmpArr.Found() )
                continue;

            // then we have our search area
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

    const SwpHints& rHtArr = rTxtNd.GetSwpHints();
    const SwTxtAttr* pAttr;
    sal_uInt16 nPos = rHtArr.Count();

    // if everything is already there then check with which it will be ended
    if( rCmpArr.Found() )
    {
        while( nPos )
            if( !rCmpArr.SetAttrBwd( *( pAttr = rHtArr.GetEnd( --nPos )) ) )
            {
                nSttPos = *pAttr->GetAnyEnd();
                if( nSttPos < rCmpArr.GetNdEnd() )
                {
                    // found end
                    nEndPos = rCmpArr.GetNdEnd();
                    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, sal_False );
                    return sal_True;
                }

                // continue search
                break;
            }

        if( !nPos && rCmpArr.Found() )
        {
            // found
            nEndPos = rCmpArr.GetNdEnd();
            lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, sal_False );
            return sal_True;
        }
    }

    while( nPos )
        if( rCmpArr.SetAttrBwd( *( pAttr = rHtArr.GetEnd( --nPos )) ) )
        {
            // Do multiple start at that position? Do also check those:
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

            // then we have our search area
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
    // search only hard attribution?
    if( bNoColls && !rCNd.HasSwAttrSet() )
        return sal_False;

    const SfxItemSet& rNdSet = rCNd.GetSwAttrSet();
    SfxItemIter aIter( rCmpSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    const SfxPoolItem* pNdItem;
    sal_uInt16 nWhich;

    while( sal_True )
    {
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

            if( !CmpAttr( rNdSet.Get( nWhich, !bNoColls ), *pItem ))
                return sal_False;
        }

        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }
    return sal_True; // found
}


sal_Bool SwPaM::Find( const SfxPoolItem& rAttr, sal_Bool bValue, SwMoveFn fnMove,
                    const SwPaM *pRegion, sal_Bool bInReadOnly )
{
    // determine which attribute is searched:
    sal_uInt16 nWhich = rAttr.Which();
    int bCharAttr = isCHRATR(nWhich) || isTXTATR(nWhich);

    SwPaM* pPam = MakeRegion( fnMove, pRegion );

    sal_Bool bFound = sal_False;
    sal_Bool bFirst = sal_True;
    sal_Bool bSrchForward = fnMove == fnMoveForward;
    SwCntntNode * pNode;
    const SfxPoolItem* pItem;
    SwpFmts aFmtArr;

    // if at beginning/end then move it out of the node
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
            if( !pNode->IsTxtNode() ) // CharAttr are only in text nodes
                continue;

            if( ((SwTxtNode*)pNode)->HasHints() &&
                lcl_Search( *(SwTxtNode*)pNode, *pPam, rAttr, fnMove,  bValue ))
            {
                // set to the values of the attribute
                SetMark();
                *GetPoint() = *pPam->GetPoint();
                *GetMark() = *pPam->GetMark();
                bFound = sal_True;
                break;
            }
            else if (isTXTATR(nWhich))
                continue;
        }

        // no hard attribution, so check if node was asked for this attr before
        if( !pNode->HasSwAttrSet() )
        {
            SwFmt* pTmpFmt = pNode->GetFmtColl();
            if( aFmtArr.find( pTmpFmt ) != aFmtArr.end() )
                continue; // collection was requested earlier
            aFmtArr.insert( pTmpFmt );
        }

        if( SFX_ITEM_SET == pNode->GetSwAttrSet().GetItemState( nWhich,
            sal_True, &pItem ) && ( !bValue || *pItem == rAttr ) )
        {
            // FORWARD:  SPoint at the end, GetMark at the beginning of the node
            // BACKWARD: SPoint at the beginning, GetMark at the end of the node
            // always: incl. start and incl. end
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            bFound = sal_True;
            break;
        }
    }

    // if backward search, switch point and mark
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

    // check which text/char attributes are searched
    SwAttrCheckArr aCmpArr( rSet, bSrchForward, bNoColls );
    SfxItemSet aOtherSet( GetDoc()->GetAttrPool(),
                            RES_PARATR_BEGIN, RES_GRFATR_END-1 );
    aOtherSet.Put( rSet, sal_False );   // got all invalid items

    FnSearchAttr fnSearch = bSrchForward
                                ? (&::lcl_SearchForward)
                                : (&::lcl_SearchBackward);

    // if at beginning/end then move it out of the node
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
            if( !pNode->IsTxtNode() ) // CharAttr are only in text nodes
                continue;

            if( (!aOtherSet.Count() ||
                lcl_Search( *pNode, aOtherSet, bNoColls )) &&
                (*fnSearch)( *(SwTxtNode*)pNode, aCmpArr, *pPam ))
            {
                // set to the values of the attribute
                SetMark();
                *GetPoint() = *pPam->GetPoint();
                *GetMark() = *pPam->GetMark();
                bFound = sal_True;
                break;
            }
            continue; // text attribute
        }

        if( !aOtherSet.Count() )
            continue;

        // no hard attribution, so check if node was asked for this attr before
        if( !pNode->HasSwAttrSet() )
        {
            SwFmt* pTmpFmt = pNode->GetFmtColl();
            if( aFmtArr.find( pTmpFmt ) != aFmtArr.end() )
                continue; // collection was requested earlier
            aFmtArr.insert( pTmpFmt );
        }

        if( lcl_Search( *pNode, aOtherSet, bNoColls ))
        {
            // FORWARD:  SPoint at the end, GetMark at the beginning of the node
            // BACKWARD: SPoint at the beginning, GetMark at the end of the node
            // always: incl. start and incl. end
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            bFound = sal_True;
            break;
        }
    }

    // if backward search, switch point and mark
    if( bFound && !bSrchForward )
        Exchange();

    delete pPam;
    return bFound;
}

/// parameters for search for attributes
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
    // replace string (only if text given and search is not parameterized)?
    sal_Bool bReplaceTxt = pSearchOpt && ( !pSearchOpt->replaceString.isEmpty() ||
                                    !pSet->Count() );
    sal_Bool bReplaceAttr = pReplSet && pReplSet->Count();
    sal_Bool bMoveFirst = !bReplaceAttr;
    if( bInReadOnly && (bReplaceAttr || bReplaceTxt ))
        bInReadOnly = sal_False;

    // We search for attributes, should we search for text as well?
    {
        SwPaM aRegion( *pRegion->GetMark(), *pRegion->GetPoint() );
        SwPaM* pTextRegion = &aRegion;
        SwPaM aSrchPam( *pCrsr->GetPoint() );

        while( sal_True )
        {
            if( pSet->Count() ) // any attributes?
            {
                // first attributes
                if( !aSrchPam.Find( *pSet, bValue, fnMove, &aRegion, bInReadOnly, bMoveFirst ) )
                    return FIND_NOT_FOUND;
                bMoveFirst = sal_True;

                if( !pSearchOpt )
                    break; // ok, only attributes, so found

                pTextRegion = &aSrchPam;
            }
            else if( !pSearchOpt )
                return FIND_NOT_FOUND;

            // then search in text of it
            if( !pSTxt )
            {
                SearchOptions aTmp( *pSearchOpt );

                // search in selection
                aTmp.searchFlag |= (SearchFlags::REG_NOT_BEGINOFLINE |
                                    SearchFlags::REG_NOT_ENDOFLINE);

                MsLangId::convertLanguageToLocale( LANGUAGE_SYSTEM, aTmp.Locale );

                pSTxt = new utl::TextSearch( aTmp );
            }

            // TODO: searching for attributes in Outliner text?!
            sal_Bool bSearchInNotes = sal_False;

            // continue search in correct section (pTextRegion)
            if( aSrchPam.Find( *pSearchOpt, bSearchInNotes, *pSTxt, fnMove, pTextRegion, bInReadOnly ) &&
                *aSrchPam.GetMark() != *aSrchPam.GetPoint() )
                break; // found
            else if( !pSet->Count() )
                return FIND_NOT_FOUND; // only text and nothing found

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

        // add to shell-cursor-ring so that the regions will be moved enventually
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
            // and remove region again
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
        // is the selection still existent?
        // all searched attributes are reset to default if
        // they are not in ReplaceSet
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
                // reset all that are not set with pool defaults
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

        return FIND_NO_RING;
    }

    else
        return FIND_FOUND;
}


int SwFindParaAttr::IsReplaceMode() const
{
    return ( pSearchOpt && !pSearchOpt->replaceString.isEmpty() ) ||
           ( pReplSet && pReplSet->Count() );
}

/// search for attributes
sal_uLong SwCursor::Find( const SfxItemSet& rSet, sal_Bool bNoCollections,
                          SwDocPositions nStart, SwDocPositions nEnd,
                          sal_Bool& bCancel, FindRanges eFndRngs,
                          const SearchOptions* pSearchOpt,
                          const SfxItemSet* pReplSet )
{
    // switch off OLE-notifications
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    sal_Bool bReplace = ( pSearchOpt && ( !pSearchOpt->replaceString.isEmpty() ||
                                    !rSet.Count() ) ) ||
                    (pReplSet && pReplSet->Count());
    bool const bStartUndo = pDoc->GetIDocumentUndoRedo().DoesUndo() && bReplace;
    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_REPLACE, NULL );
    }

    SwFindParaAttr aSwFindParaAttr( rSet, bNoCollections, pSearchOpt,
                                    pReplSet, *this );

    sal_uLong nRet = FindAll( aSwFindParaAttr, nStart, nEnd, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );
    if( nRet && bReplace )
        pDoc->SetModified();

    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_REPLACE, NULL );
    }

    return nRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
