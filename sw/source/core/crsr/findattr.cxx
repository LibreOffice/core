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

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <hintids.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <fmtpdsc.hxx>
#include <txatbase.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <swcrsr.hxx>
#include <editsh.hxx>
#include <ndtxt.hxx>
#include <pamtyp.hxx>
#include <swundo.hxx>
#include <crsskip.hxx>
#include <boost/optional.hpp>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

typedef std::set<SwFormat*> SwpFormats;

// Special case for SvxFontItem: only compare the name
bool CmpAttr( const SfxPoolItem& rItem1, const SfxPoolItem& rItem2 )
{
    switch( rItem1.Which() )
    {
    case RES_CHRATR_FONT:
        return static_cast<const SvxFontItem&>(rItem1).GetFamilyName() ==
                static_cast<const SvxFontItem&>(rItem2).GetFamilyName();

    case RES_CHRATR_COLOR:
        return static_cast<const SvxColorItem&>(rItem1).GetValue().IsRGBEqual(
                                static_cast<const SvxColorItem&>(rItem2).GetValue() );
    case RES_PAGEDESC:
        bool bNumOffsetEqual = false;
        ::boost::optional<sal_uInt16> const oNumOffset1 =
            static_cast<const SwFormatPageDesc&>(rItem1).GetNumOffset();
        ::boost::optional<sal_uInt16> const oNumOffset2 =
            static_cast<const SwFormatPageDesc&>(rItem2).GetNumOffset();
        if (!oNumOffset1 && !oNumOffset2)
        {
            bNumOffsetEqual = true;
        }
        else if (oNumOffset1 && oNumOffset2)
        {
            bNumOffsetEqual = oNumOffset1.get() == oNumOffset2.get();
        }
        else
        {
            bNumOffsetEqual = false;
        }

        if (!bNumOffsetEqual)
            return false;

        return static_cast<const SwFormatPageDesc&>(rItem1).GetPageDesc() == static_cast<const SwFormatPageDesc&>(rItem2).GetPageDesc();
    }
    return rItem1 == rItem2;
}

const SwTextAttr* GetFrwrdTextHint( const SwpHints& rHtsArr, sal_uInt16& rPos,
                                    sal_Int32 nContentPos )
{
    while( rPos < rHtsArr.Count() )
    {
        const SwTextAttr *pTextHt = rHtsArr.Get( rPos++ );
        // the start of an attribute has to be in the section
        if( pTextHt->GetStart() >= nContentPos )
            return pTextHt; // valid text attribute
    }
    return nullptr; // invalid text attribute
}

const SwTextAttr* GetBkwrdTextHint( const SwpHints& rHtsArr, sal_uInt16& rPos,
                                  sal_Int32 nContentPos )
{
    while( rPos > 0 )
    {
        const SwTextAttr *pTextHt = rHtsArr.Get( --rPos );
        // the start of an attribute has to be in the section
        if( pTextHt->GetStart() < nContentPos )
            return pTextHt; // valid text attribute
    }
    return nullptr; // invalid text attribute
}

static void lcl_SetAttrPam( SwPaM& rPam, sal_Int32 nStart, const sal_Int32* pEnd,
                     const bool bSaveMark )
{
    sal_Int32 nContentPos;
    if( bSaveMark )
        nContentPos = rPam.GetMark()->nContent.GetIndex();
    else
        nContentPos = rPam.GetPoint()->nContent.GetIndex();
    bool bTstEnd = rPam.GetPoint()->nNode == rPam.GetMark()->nNode;

    SwContentNode* pCNd = rPam.GetContentNode();
    rPam.GetPoint()->nContent.Assign( pCNd, nStart );
    rPam.SetMark(); // Point == GetMark

    // Point points to end of search area or end of attribute
    if( pEnd )
    {
        if( bTstEnd && *pEnd > nContentPos )
            rPam.GetPoint()->nContent = nContentPos;
        else
            rPam.GetPoint()->nContent = *pEnd;
    }
}

// TODO: provide documentation
/** search for a text attribute

    This function searches in a text node for a given attribute.
    If that is found then the SwPaM contains the section that surrounds the
    attribute (w.r.t. the search area).

    @param rTextNd   Text node to search in.
    @param rPam     ???
    @param rCmpItem ???
    @param fnMove   ???
    @param bValue   ???
    @return Returns <true> if found, <false> otherwise.
*/
static bool lcl_Search( const SwTextNode& rTextNd, SwPaM& rPam,
                    const SfxPoolItem& rCmpItem,
                    SwMoveFn fnMove, bool bValue )
{
    if ( !rTextNd.HasHints() )
        return false;

    const SwTextAttr *pTextHt = nullptr;
    bool bForward = fnMove == fnMoveForward;
    sal_uInt16 nPos = bForward ? 0 : rTextNd.GetSwpHints().Count();
    sal_Int32 nContentPos = rPam.GetPoint()->nContent.GetIndex();

    while( nullptr != ( pTextHt=(*fnMove->fnGetHint)(rTextNd.GetSwpHints(),nPos,nContentPos)))
        if( pTextHt->Which() == rCmpItem.Which() &&
            ( !bValue || CmpAttr( pTextHt->GetAttr(), rCmpItem )))
        {
            lcl_SetAttrPam( rPam, pTextHt->GetStart(), pTextHt->End(), bForward );
            return true;
        }
    return false;
}

/// search for multiple text attributes
struct _SwSrchChrAttr
{
    sal_uInt16 nWhich;
    sal_Int32 nStt;
    sal_Int32 nEnd;

    _SwSrchChrAttr( const SfxPoolItem& rItem,
                    sal_Int32 nStart, sal_Int32 nAnyEnd )
        : nWhich( rItem.Which() ), nStt( nStart ), nEnd( nAnyEnd )
    {}
};

class SwAttrCheckArr
{
    _SwSrchChrAttr *pFndArr, *pStackArr;
    sal_Int32 nNdStt;
    sal_Int32 nNdEnd;
    sal_uInt16 nArrStart, nArrLen;
    sal_uInt16 nFound, nStackCnt;
    SfxItemSet aCmpSet;
    bool bNoColls;
    bool bForward;

public:
    SwAttrCheckArr( const SfxItemSet& rSet, bool bForward, bool bNoCollections );
    ~SwAttrCheckArr();

    void SetNewSet( const SwTextNode& rTextNd, const SwPaM& rPam );

    /// how many attributes are there in total?
    sal_uInt16 Count() const    { return aCmpSet.Count(); }
    bool Found() const       { return nFound == aCmpSet.Count(); }
    bool CheckStack();

    sal_Int32 Start() const;
    sal_Int32 End() const;

    sal_Int32 GetNdStt() const { return nNdStt; }
    sal_Int32 GetNdEnd() const { return nNdEnd; }

    bool SetAttrFwd( const SwTextAttr& rAttr );
    bool SetAttrBwd( const SwTextAttr& rAttr );
};

SwAttrCheckArr::SwAttrCheckArr( const SfxItemSet& rSet, bool bFwd,
                                bool bNoCollections )
    : nNdStt(0)
    , nNdEnd(0)
    , nFound(0)
    , nStackCnt(0)
    , aCmpSet( *rSet.GetPool(), RES_CHRATR_BEGIN, RES_TXTATR_END-1 )
    , bNoColls(bNoCollections)
    , bForward(bFwd)
{
    aCmpSet.Put( rSet, false );

    // determine area of Fnd/Stack array (Min/Max)
    SfxItemIter aIter( aCmpSet );
    nArrStart = aCmpSet.GetWhichByPos( aIter.GetFirstPos() );
    nArrLen = aCmpSet.GetWhichByPos( aIter.GetLastPos() ) - nArrStart+1;

    char* pFndChar  = new char[ nArrLen * sizeof(_SwSrchChrAttr) ];
    char* pStackChar = new char[ nArrLen * sizeof(_SwSrchChrAttr) ];

    pFndArr = reinterpret_cast<_SwSrchChrAttr*>(pFndChar);
    pStackArr = reinterpret_cast<_SwSrchChrAttr*>(pStackChar);
}

SwAttrCheckArr::~SwAttrCheckArr()
{
    delete[] reinterpret_cast<char*>(pFndArr);
    delete[] reinterpret_cast<char*>(pStackArr);
}

void SwAttrCheckArr::SetNewSet( const SwTextNode& rTextNd, const SwPaM& rPam )
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
                : rTextNd.GetText().getLength();
    }
    else
    {
        nNdEnd = rPam.GetPoint()->nContent.GetIndex();
        nNdStt = rPam.GetPoint()->nNode == rPam.GetMark()->nNode
                ? rPam.GetMark()->nContent.GetIndex()
                : 0;
    }

    if( bNoColls && !rTextNd.HasSwAttrSet() )
        return ;

    const SfxItemSet& rSet = rTextNd.GetSwAttrSet();

    SfxItemIter aIter( aCmpSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    const SfxPoolItem* pFndItem;
    sal_uInt16 nWhich;

    while( true )
    {
        if( IsInvalidItem( pItem ) )
        {
            nWhich = aCmpSet.GetWhichByPos( aIter.GetCurPos() );
            if( RES_TXTATR_END <= nWhich )
                break; // end of text attributes

            if( SfxItemState::SET == rSet.GetItemState( nWhich, !bNoColls, &pFndItem )
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
lcl_IsAttributeIgnorable(sal_Int32 const nNdStart, sal_Int32 const nNdEnd,
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

bool SwAttrCheckArr::SetAttrFwd( const SwTextAttr& rAttr )
{
    _SwSrchChrAttr aTmp( rAttr.GetAttr(), rAttr.GetStart(), *rAttr.GetAnyEnd() );

    // ignore all attributes not in search range
    if (lcl_IsAttributeIgnorable(nNdStt, nNdEnd, aTmp))
    {
        return Found();
    }

    const SfxPoolItem* pItem;
    // here we explicitly also search in character templates
    sal_uInt16 nWhch = rAttr.Which();
    SfxWhichIter* pIter = nullptr;
    const SfxPoolItem* pTmpItem = nullptr;
    const SfxItemSet* pSet = nullptr;
    if( RES_TXTATR_CHARFMT == nWhch || RES_TXTATR_AUTOFMT == nWhch )
    {
        if( bNoColls && RES_TXTATR_CHARFMT == nWhch )
            return Found();
        pTmpItem = nullptr;
        pSet = CharFormat::GetItemSet( rAttr.GetAttr() );
        if ( pSet )
        {
            pIter = new SfxWhichIter( *pSet );
            nWhch = pIter->FirstWhich();
            while( nWhch &&
                SfxItemState::SET != pSet->GetItemState( nWhch, true, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                pTmpItem = nullptr;
        }
    }
    else
        pTmpItem = &rAttr.GetAttr();

    while( pTmpItem )
    {
        SfxItemState eState = aCmpSet.GetItemState( nWhch, false, &pItem );
        if( SfxItemState::DONTCARE == eState || SfxItemState::SET == eState )
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

            if( SfxItemState::DONTCARE == eState  )
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
                SfxItemState::SET != pSet->GetItemState( nWhch, true, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                break;
        }
        else
            break;
    }
    delete pIter;
    return Found();
}

bool SwAttrCheckArr::SetAttrBwd( const SwTextAttr& rAttr )
{
    _SwSrchChrAttr aTmp( rAttr.GetAttr(), rAttr.GetStart(), *rAttr.GetAnyEnd() );

    // ignore all attributes not in search range
    if (lcl_IsAttributeIgnorable(nNdStt, nNdEnd, aTmp))
    {
        return Found();
    }

    const SfxPoolItem* pItem;
    // here we explicitly also search in character templates
    sal_uInt16 nWhch = rAttr.Which();
    SfxWhichIter* pIter = nullptr;
    const SfxPoolItem* pTmpItem = nullptr;
    const SfxItemSet* pSet = nullptr;
    if( RES_TXTATR_CHARFMT == nWhch || RES_TXTATR_AUTOFMT == nWhch )
    {
        if( bNoColls && RES_TXTATR_CHARFMT == nWhch )
            return Found();

        pSet = CharFormat::GetItemSet( rAttr.GetAttr() );
        if ( pSet )
        {
            pIter = new SfxWhichIter( *pSet );
            nWhch = pIter->FirstWhich();
            while( nWhch &&
                SfxItemState::SET != pSet->GetItemState( nWhch, true, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                pTmpItem = nullptr;
        }
    }
    else
        pTmpItem = &rAttr.GetAttr();

    while( pTmpItem )
    {
        SfxItemState eState = aCmpSet.GetItemState( nWhch, false, &pItem );
        if( SfxItemState::DONTCARE == eState || SfxItemState::SET == eState )
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

            bool bContinue = false;
            if( SfxItemState::DONTCARE == eState  )
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

                    bContinue = true;
                }
            }
            // Will the attribute become valid?
            else if( CmpAttr( *pItem, *pTmpItem ))
            {
                pFndArr[ nWhch - nArrStart ] = aTmp;
                ++nFound;
                bContinue = true;
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
                SfxItemState::SET != pSet->GetItemState( nWhch, true, &pTmpItem ) )
                nWhch = pIter->NextWhich();
            if( !nWhch )
                break;
        }
        else
            break;
    }
    delete pIter;
    return Found();
}

sal_Int32 SwAttrCheckArr::Start() const
{
    sal_Int32 nStart = nNdStt;
    _SwSrchChrAttr* pArrPtr = pFndArr;
    for( sal_uInt16 n = 0; n < nArrLen; ++n, ++pArrPtr )
        if( pArrPtr->nWhich && pArrPtr->nStt > nStart )
            nStart = pArrPtr->nStt;

    return nStart;
}

sal_Int32 SwAttrCheckArr::End() const
{
    _SwSrchChrAttr* pArrPtr = pFndArr;
    sal_Int32 nEnd = nNdEnd;
    for( sal_uInt16 n = 0; n < nArrLen; ++n, ++pArrPtr )
        if( pArrPtr->nWhich && pArrPtr->nEnd < nEnd )
            nEnd = pArrPtr->nEnd;

    return nEnd;
}

bool SwAttrCheckArr::CheckStack()
{
    if( !nStackCnt )
        return false;

    sal_uInt16 n;
    const sal_Int32 nSttPos = Start();
    const sal_Int32 nEndPos = End();
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

static bool lcl_SearchForward( const SwTextNode& rTextNd, SwAttrCheckArr& rCmpArr,
                            SwPaM& rPam )
{
    sal_Int32 nEndPos;
    rCmpArr.SetNewSet( rTextNd, rPam );
    if( !rTextNd.HasHints() )
    {
        if( !rCmpArr.Found() )
            return false;
        nEndPos = rCmpArr.GetNdEnd();
        lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, true );
        return true;
    }

    const SwpHints& rHtArr = rTextNd.GetSwpHints();
    const SwTextAttr* pAttr;
    size_t nPos = 0;

    // if everything is already there then check with which it will be ended
    if( rCmpArr.Found() )
    {
        for( ; nPos < rHtArr.Count(); ++nPos )
            if( !rCmpArr.SetAttrFwd( *( pAttr = rHtArr.Get( nPos )) ) )
            {
                if( rCmpArr.GetNdStt() < pAttr->GetStart() )
                {
                    // found end
                    lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(),
                                &pAttr->GetStart(), true );
                    return true;
                }
                // continue search
                break;
            }

        if( nPos == rHtArr.Count() && rCmpArr.Found() )
        {
            // found
            nEndPos = rCmpArr.GetNdEnd();
            lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, true );
            return true;
        }
    }

    sal_Int32 nSttPos;
    for( ; nPos < rHtArr.Count(); ++nPos )
        if( rCmpArr.SetAttrFwd( *( pAttr = rHtArr.Get( nPos )) ) )
        {
            // Do multiple start at that position? Do also check those:
            nSttPos = pAttr->GetStart();
            while( ++nPos < rHtArr.Count() && nSttPos ==
                    ( pAttr = rHtArr.Get( nPos ))->GetStart() &&
                    rCmpArr.SetAttrFwd( *pAttr ) )
                ;

            if( !rCmpArr.Found() )
                continue;

            // then we have our search area
            if( (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
                return false;

            lcl_SetAttrPam( rPam, nSttPos, &nEndPos, true );
            return true;
        }

    if( !rCmpArr.CheckStack() ||
        (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
        return false;

    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, true );
    return true;
}

static bool lcl_SearchBackward( const SwTextNode& rTextNd, SwAttrCheckArr& rCmpArr,
                            SwPaM& rPam )
{
    sal_Int32 nEndPos;
    rCmpArr.SetNewSet( rTextNd, rPam );
    if( !rTextNd.HasHints() )
    {
        if( !rCmpArr.Found() )
            return false;
        nEndPos = rCmpArr.GetNdEnd();
        lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, false );
        return true;
    }

    const SwpHints& rHtArr = rTextNd.GetSwpHints();
    const SwTextAttr* pAttr;
    size_t nPos = rHtArr.Count();
    sal_Int32 nSttPos;

    // if everything is already there then check with which it will be ended
    if( rCmpArr.Found() )
    {
        while( nPos )
            if( !rCmpArr.SetAttrBwd( *( pAttr = rHtArr.GetSortedByEnd( --nPos )) ) )
            {
                nSttPos = *pAttr->GetAnyEnd();
                if( nSttPos < rCmpArr.GetNdEnd() )
                {
                    // found end
                    nEndPos = rCmpArr.GetNdEnd();
                    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, false );
                    return true;
                }

                // continue search
                break;
            }

        if( !nPos && rCmpArr.Found() )
        {
            // found
            nEndPos = rCmpArr.GetNdEnd();
            lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(), &nEndPos, false );
            return true;
        }
    }

    while( nPos )
        if( rCmpArr.SetAttrBwd( *( pAttr = rHtArr.GetSortedByEnd( --nPos )) ) )
        {
            // Do multiple start at that position? Do also check those:
            if( nPos )
            {
                nEndPos = *pAttr->GetAnyEnd();
                while( --nPos && nEndPos ==
                        *( pAttr = rHtArr.GetSortedByEnd( nPos ))->GetAnyEnd() &&
                        rCmpArr.SetAttrBwd( *pAttr ) )
                    ;
            }
            if( !rCmpArr.Found() )
                continue;

            // then we have our search area
            if( (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
                return false;

            lcl_SetAttrPam( rPam, nSttPos, &nEndPos, false );
            return true;
        }

    if( !rCmpArr.CheckStack() ||
        (nSttPos = rCmpArr.Start()) > (nEndPos = rCmpArr.End()) )
        return false;

    lcl_SetAttrPam( rPam, nSttPos, &nEndPos, false );
    return true;
}

static bool lcl_Search( const SwContentNode& rCNd, const SfxItemSet& rCmpSet, bool bNoColls )
{
    // search only hard attribution?
    if( bNoColls && !rCNd.HasSwAttrSet() )
        return false;

    const SfxItemSet& rNdSet = rCNd.GetSwAttrSet();
    SfxItemIter aIter( rCmpSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    const SfxPoolItem* pNdItem;
    sal_uInt16 nWhich;

    while( true )
    {
        if( IsInvalidItem( pItem ))
        {
            nWhich = rCmpSet.GetWhichByPos( aIter.GetCurPos() );
            if( SfxItemState::SET != rNdSet.GetItemState( nWhich, !bNoColls, &pNdItem )
                || CmpAttr( *pNdItem, rNdSet.GetPool()->GetDefaultItem( nWhich ) ))
                return false;
        }
        else
        {
            nWhich = pItem->Which();

            if( !CmpAttr( rNdSet.Get( nWhich, !bNoColls ), *pItem ))
                return false;
        }

        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }
    return true; // found
}

bool SwPaM::Find( const SfxPoolItem& rAttr, bool bValue, SwMoveFn fnMove,
                  const SwPaM *pRegion, bool bInReadOnly )
{
    // determine which attribute is searched:
    const sal_uInt16 nWhich = rAttr.Which();
    bool bCharAttr = isCHRATR(nWhich) || isTXTATR(nWhich);

    std::unique_ptr<SwPaM> pPam(MakeRegion( fnMove, pRegion ));

    bool bFound = false;
    bool bFirst = true;
    const bool bSrchForward = fnMove == fnMoveForward;
    SwContentNode * pNode;
    const SfxPoolItem* pItem;
    SwpFormats aFormatArr;

    // if at beginning/end then move it out of the node
    if( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetContentNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( !(*fnMove->fnNds)( &pPam->GetPoint()->nNode, false ))
        {
            return false;
        }
        SwContentNode *pNd = pPam->GetContentNode();
        pPam->GetPoint()->nContent.Assign( pNd, bSrchForward ? 0 : pNd->Len() );
    }

    while( nullptr != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ) )
    {
        if( bCharAttr )
        {
            if( !pNode->IsTextNode() ) // CharAttr are only in text nodes
                continue;

            if( pNode->GetTextNode()->HasHints() &&
                lcl_Search( *pNode->GetTextNode(), *pPam, rAttr, fnMove,  bValue ))
            {
                // set to the values of the attribute
                SetMark();
                *GetPoint() = *pPam->GetPoint();
                *GetMark() = *pPam->GetMark();
                bFound = true;
                break;
            }
            else if (isTXTATR(nWhich))
                continue;
        }

        // no hard attribution, so check if node was asked for this attr before
        if( !pNode->HasSwAttrSet() )
        {
            SwFormat* pTmpFormat = pNode->GetFormatColl();
            if( aFormatArr.find( pTmpFormat ) != aFormatArr.end() )
                continue; // collection was requested earlier
            aFormatArr.insert( pTmpFormat );
        }

        if( SfxItemState::SET == pNode->GetSwAttrSet().GetItemState( nWhich,
            true, &pItem ) && ( !bValue || *pItem == rAttr ) )
        {
            // FORWARD:  SPoint at the end, GetMark at the beginning of the node
            // BACKWARD: SPoint at the beginning, GetMark at the end of the node
            // always: incl. start and incl. end
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            bFound = true;
            break;
        }
    }

    // if backward search, switch point and mark
    if( bFound && !bSrchForward )
        Exchange();

    return bFound;
}

typedef bool (*FnSearchAttr)( const SwTextNode&, SwAttrCheckArr&, SwPaM& );

bool SwPaM::Find( const SfxItemSet& rSet, bool bNoColls, SwMoveFn fnMove,
                  const SwPaM *pRegion, bool bInReadOnly, bool bMoveFirst )
{
    std::unique_ptr<SwPaM> pPam(MakeRegion( fnMove, pRegion ));

    bool bFound = false;
    bool bFirst = true;
    const bool bSrchForward = fnMove == fnMoveForward;
    SwContentNode * pNode;
    SwpFormats aFormatArr;

    // check which text/char attributes are searched
    SwAttrCheckArr aCmpArr( rSet, bSrchForward, bNoColls );
    SfxItemSet aOtherSet( GetDoc()->GetAttrPool(),
                            RES_PARATR_BEGIN, RES_GRFATR_END-1 );
    aOtherSet.Put( rSet, false );   // got all invalid items

    FnSearchAttr fnSearch = bSrchForward
                                ? (&::lcl_SearchForward)
                                : (&::lcl_SearchBackward);

    // if at beginning/end then move it out of the node
    if( bMoveFirst &&
        ( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetContentNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() ) )
    {
        if( !(*fnMove->fnNds)( &pPam->GetPoint()->nNode, false ))
        {
            return false;
        }
        SwContentNode *pNd = pPam->GetContentNode();
        pPam->GetPoint()->nContent.Assign( pNd, bSrchForward ? 0 : pNd->Len() );
    }

    while( nullptr != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ) )
    {
        if( aCmpArr.Count() )
        {
            if( !pNode->IsTextNode() ) // CharAttr are only in text nodes
                continue;

            if( (!aOtherSet.Count() ||
                lcl_Search( *pNode, aOtherSet, bNoColls )) &&
                (*fnSearch)( *pNode->GetTextNode(), aCmpArr, *pPam ))
            {
                // set to the values of the attribute
                SetMark();
                *GetPoint() = *pPam->GetPoint();
                *GetMark() = *pPam->GetMark();
                bFound = true;
                break;
            }
            continue; // text attribute
        }

        if( !aOtherSet.Count() )
            continue;

        // no hard attribution, so check if node was asked for this attr before
        if( !pNode->HasSwAttrSet() )
        {
            SwFormat* pTmpFormat = pNode->GetFormatColl();
            if( aFormatArr.find( pTmpFormat ) != aFormatArr.end() )
                continue; // collection was requested earlier
            aFormatArr.insert( pTmpFormat );
        }

        if( lcl_Search( *pNode, aOtherSet, bNoColls ))
        {
            // FORWARD:  SPoint at the end, GetMark at the beginning of the node
            // BACKWARD: SPoint at the beginning, GetMark at the end of the node
            // always: incl. start and incl. end
            *GetPoint() = *pPam->GetPoint();
            SetMark();
            pNode->MakeEndIndex( &GetPoint()->nContent );
            bFound = true;
            break;
        }
    }

    // if backward search, switch point and mark
    if( bFound && !bSrchForward )
        Exchange();

    return bFound;
}

/// parameters for search for attributes
struct SwFindParaAttr : public SwFindParas
{
    bool bValue;
    const SfxItemSet *pSet, *pReplSet;
    const SearchOptions *pSearchOpt;
    SwCursor& rCursor;
    utl::TextSearch* pSText;

    SwFindParaAttr( const SfxItemSet& rSet, bool bNoCollection,
                    const SearchOptions* pOpt, const SfxItemSet* pRSet,
                    SwCursor& rCrsr )
        : bValue( bNoCollection ), pSet( &rSet ), pReplSet( pRSet ),
          pSearchOpt( pOpt ), rCursor( rCrsr ),pSText( nullptr ) {}

    virtual ~SwFindParaAttr()   { delete pSText; }

    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, bool bInReadOnly ) override;
    virtual bool IsReplaceMode() const override;
};

int SwFindParaAttr::Find( SwPaM* pCrsr, SwMoveFn fnMove, const SwPaM* pRegion,
                          bool bInReadOnly )
{
    // replace string (only if text given and search is not parameterized)?
    bool bReplaceText = pSearchOpt && ( !pSearchOpt->replaceString.isEmpty() ||
                                    !pSet->Count() );
    bool bReplaceAttr = pReplSet && pReplSet->Count();
    bool bMoveFirst = !bReplaceAttr;
    if( bInReadOnly && (bReplaceAttr || bReplaceText ))
        bInReadOnly = false;

    // We search for attributes, should we search for text as well?
    {
        SwPaM aRegion( *pRegion->GetMark(), *pRegion->GetPoint() );
        SwPaM* pTextRegion = &aRegion;
        SwPaM aSrchPam( *pCrsr->GetPoint() );

        while( true )
        {
            if( pSet->Count() ) // any attributes?
            {
                // first attributes
                if( !aSrchPam.Find( *pSet, bValue, fnMove, &aRegion, bInReadOnly, bMoveFirst ) )
                    return FIND_NOT_FOUND;
                bMoveFirst = true;

                if( !pSearchOpt )
                    break; // ok, only attributes, so found

                pTextRegion = &aSrchPam;
            }
            else if( !pSearchOpt )
                return FIND_NOT_FOUND;

            // then search in text of it
            if( !pSText )
            {
                SearchOptions aTmp( *pSearchOpt );

                // search in selection
                aTmp.searchFlag |= (SearchFlags::REG_NOT_BEGINOFLINE |
                                    SearchFlags::REG_NOT_ENDOFLINE);

                aTmp.Locale = SvtSysLocale().GetLanguageTag().getLocale();

                pSText = new utl::TextSearch( aTmp );
            }

            // TODO: searching for attributes in Outliner text?!
            bool bSearchInNotes = false;

            // continue search in correct section (pTextRegion)
            if( aSrchPam.Find( *pSearchOpt, bSearchInNotes, *pSText, fnMove, pTextRegion, bInReadOnly ) &&
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

    if( bReplaceText )
    {
        const bool bRegExp(
                SearchAlgorithms_REGEXP == pSearchOpt->algorithmType);
        SwIndex& rSttCntIdx = pCrsr->Start()->nContent;
        const sal_Int32 nSttCnt = rSttCntIdx.GetIndex();

        // add to shell-cursor-ring so that the regions will be moved eventually
        SwPaM* pPrevRing(nullptr);
        if( bRegExp )
        {
            pPrevRing = const_cast< SwPaM* >(pRegion)->GetPrev();
            const_cast< SwPaM* >(pRegion)->GetRingContainer().merge( rCursor.GetRingContainer() );
        }

        std::unique_ptr<OUString> pRepl( (bRegExp) ?
                ReplaceBackReferences( *pSearchOpt, pCrsr ) : nullptr );
        rCursor.GetDoc()->getIDocumentContentOperations().ReplaceRange( *pCrsr,
            (pRepl.get()) ? *pRepl : pSearchOpt->replaceString,
            bRegExp );
        rCursor.SaveTableBoxContent( pCrsr->GetPoint() );

        if( bRegExp )
        {
            // and remove region again
            SwPaM* p;
            SwPaM* pNext = const_cast<SwPaM*>(pRegion);
            do {
                p = pNext;
                pNext = p->GetNext();
                p->MoveTo( const_cast<SwPaM*>(pRegion) );
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
            pCrsr->GetDoc()->getIDocumentContentOperations().InsertItemSet( *pCrsr, *pReplSet );
        }
        else
        {
            SfxItemPool* pPool = pReplSet->GetPool();
            SfxItemSet aSet( *pPool, pReplSet->GetRanges() );

            SfxItemIter aIter( *pSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            while( true )
            {
                // reset all that are not set with pool defaults
                if( !IsInvalidItem( pItem ) && SfxItemState::SET !=
                    pReplSet->GetItemState( pItem->Which(), false ))
                    aSet.Put( pPool->GetDefaultItem( pItem->Which() ));

                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
            aSet.Put( *pReplSet );
            pCrsr->GetDoc()->getIDocumentContentOperations().InsertItemSet( *pCrsr, aSet );
        }

        return FIND_NO_RING;
    }
    else
        return FIND_FOUND;
}

bool SwFindParaAttr::IsReplaceMode() const
{
    return ( pSearchOpt && !pSearchOpt->replaceString.isEmpty() ) ||
           ( pReplSet && pReplSet->Count() );
}

/// search for attributes
sal_uLong SwCursor::Find( const SfxItemSet& rSet, bool bNoCollections,
                          SwDocPositions nStart, SwDocPositions nEnd,
                          bool& bCancel, FindRanges eFndRngs,
                          const SearchOptions* pSearchOpt,
                          const SfxItemSet* pReplSet )
{
    // switch off OLE-notifications
    SwDoc* pDoc = GetDoc();
    Link<bool,void> aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link<bool,void>() );

    bool bReplace = ( pSearchOpt && ( !pSearchOpt->replaceString.isEmpty() ||
                                    !rSet.Count() ) ) ||
                    (pReplSet && pReplSet->Count());
    bool const bStartUndo = pDoc->GetIDocumentUndoRedo().DoesUndo() && bReplace;
    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_REPLACE, nullptr );
    }

    SwFindParaAttr aSwFindParaAttr( rSet, bNoCollections, pSearchOpt,
                                    pReplSet, *this );

    sal_uLong nRet = FindAll( aSwFindParaAttr, nStart, nEnd, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );
    if( nRet && bReplace )
        pDoc->getIDocumentState().SetModified();

    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_REPLACE, nullptr );
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
