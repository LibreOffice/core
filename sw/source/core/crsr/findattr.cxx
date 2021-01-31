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
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/searchopt.hxx>
#include <osl/diagnose.h>
#include <unotools/syslocale.hxx>
#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <svl/srchitem.hxx>
#include <svl/whiter.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <fmtpdsc.hxx>
#include <txatbase.hxx>
#include <charfmt.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <swcrsr.hxx>
#include <ndtxt.hxx>
#include <pamtyp.hxx>
#include <txtfrm.hxx>
#include <swundo.hxx>
#include <optional>

#include <algorithm>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

// Special case for SvxFontItem: only compare the name
static bool CmpAttr( const SfxPoolItem& rItem1, const SfxPoolItem& rItem2 )
{
    switch( rItem1.Which() )
    {
    case RES_CHRATR_FONT:
        return rItem1.StaticWhichCast(RES_CHRATR_FONT).GetFamilyName() == rItem2.StaticWhichCast(RES_CHRATR_FONT).GetFamilyName();

    case RES_CHRATR_COLOR:
        return rItem1.StaticWhichCast(RES_CHRATR_COLOR).GetValue().IsRGBEqual(rItem2.StaticWhichCast(RES_CHRATR_COLOR).GetValue());
    case RES_PAGEDESC:
        ::std::optional<sal_uInt16> const oNumOffset1 = rItem1.StaticWhichCast(RES_PAGEDESC).GetNumOffset();
        ::std::optional<sal_uInt16> const oNumOffset2 = rItem2.StaticWhichCast(RES_PAGEDESC).GetNumOffset();

        if (oNumOffset1 != oNumOffset2)
            return false;

        return rItem1.StaticWhichCast(RES_PAGEDESC).GetPageDesc() == rItem2.StaticWhichCast(RES_PAGEDESC).GetPageDesc();
    }
    return rItem1 == rItem2;
}

const SwTextAttr* GetFrwrdTextHint( const SwpHints& rHtsArr, size_t& rPos,
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

const SwTextAttr* GetBkwrdTextHint( const SwpHints& rHtsArr, size_t& rPos,
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
    @return Returns <true> if found, <false> otherwise.
*/
static bool lcl_SearchAttr( const SwTextNode& rTextNd, SwPaM& rPam,
                    const SfxPoolItem& rCmpItem,
                    SwMoveFnCollection const & fnMove)
{
    if ( !rTextNd.HasHints() )
        return false;

    const SwTextAttr *pTextHt = nullptr;
    bool bForward = &fnMove == &fnMoveForward;
    size_t nPos = bForward ? 0 : rTextNd.GetSwpHints().Count();
    sal_Int32 nContentPos = rPam.GetPoint()->nContent.GetIndex();

    while( nullptr != ( pTextHt=(*fnMove.fnGetHint)(rTextNd.GetSwpHints(),nPos,nContentPos)))
        if (pTextHt->Which() == rCmpItem.Which())
        {
            lcl_SetAttrPam( rPam, pTextHt->GetStart(), pTextHt->End(), bForward );
            return true;
        }
    return false;
}

namespace {

/// search for multiple text attributes
struct SwSrchChrAttr
{
    sal_uInt16 nWhich;
    sal_Int32 nStt;
    sal_Int32 nEnd;

    SwSrchChrAttr(): nWhich(0), nStt(0), nEnd(0) {}

    SwSrchChrAttr( const SfxPoolItem& rItem,
                    sal_Int32 nStart, sal_Int32 nAnyEnd )
        : nWhich( rItem.Which() ), nStt( nStart ), nEnd( nAnyEnd )
    {}
};

class SwAttrCheckArr
{
    SwSrchChrAttr *m_pFindArr, *m_pStackArr;
    sal_Int32 m_nNodeStart;
    sal_Int32 m_nNodeEnd;
    sal_uInt16 m_nArrStart, m_nArrLen;
    sal_uInt16 m_nFound, m_nStackCount;
    SfxItemSet m_aComapeSet;
    bool m_bNoColls;
    bool m_bForward;

public:
    SwAttrCheckArr( const SfxItemSet& rSet, bool bForward, bool bNoCollections );
    ~SwAttrCheckArr();

    void SetNewSet( const SwTextNode& rTextNd, const SwPaM& rPam );

    /// how many attributes are there in total?
    sal_uInt16 Count() const    { return m_aComapeSet.Count(); }
    bool Found() const       { return m_nFound == m_aComapeSet.Count(); }
    bool CheckStack();

    sal_Int32 Start() const;
    sal_Int32 End() const;

    sal_Int32 GetNdStt() const { return m_nNodeStart; }
    sal_Int32 GetNdEnd() const { return m_nNodeEnd; }

    bool SetAttrFwd( const SwTextAttr& rAttr );
    bool SetAttrBwd( const SwTextAttr& rAttr );
};

}

SwAttrCheckArr::SwAttrCheckArr( const SfxItemSet& rSet, bool bFwd,
                                bool bNoCollections )
    : m_nNodeStart(0)
    , m_nNodeEnd(0)
    , m_nFound(0)
    , m_nStackCount(0)
    , m_aComapeSet( *rSet.GetPool(), svl::Items<RES_CHRATR_BEGIN, RES_TXTATR_END-1>{} )
    , m_bNoColls(bNoCollections)
    , m_bForward(bFwd)
{
    m_aComapeSet.Put( rSet, false );

    // determine area of Fnd/Stack array (Min/Max)
    SfxItemIter aIter( m_aComapeSet );
    m_nArrStart = m_aComapeSet.GetWhichByPos( aIter.GetFirstPos() );
    m_nArrLen = m_aComapeSet.GetWhichByPos( aIter.GetLastPos() ) - m_nArrStart+1;

    char* pFndChar  = new char[ m_nArrLen * sizeof(SwSrchChrAttr) ];
    char* pStackChar = new char[ m_nArrLen * sizeof(SwSrchChrAttr) ];

    m_pFindArr = reinterpret_cast<SwSrchChrAttr*>(pFndChar);
    m_pStackArr = reinterpret_cast<SwSrchChrAttr*>(pStackChar);
}

SwAttrCheckArr::~SwAttrCheckArr()
{
    delete[] reinterpret_cast<char*>(m_pFindArr);
    delete[] reinterpret_cast<char*>(m_pStackArr);
}

void SwAttrCheckArr::SetNewSet( const SwTextNode& rTextNd, const SwPaM& rPam )
{
    std::fill(m_pFindArr, m_pFindArr + m_nArrLen, SwSrchChrAttr());
    std::fill(m_pStackArr, m_pStackArr + m_nArrLen, SwSrchChrAttr());
    m_nFound = 0;
    m_nStackCount = 0;

    if( m_bForward )
    {
        m_nNodeStart = rPam.GetPoint()->nContent.GetIndex();
        m_nNodeEnd = rPam.GetPoint()->nNode == rPam.GetMark()->nNode
                ? rPam.GetMark()->nContent.GetIndex()
                : rTextNd.GetText().getLength();
    }
    else
    {
        m_nNodeEnd = rPam.GetPoint()->nContent.GetIndex();
        m_nNodeStart = rPam.GetPoint()->nNode == rPam.GetMark()->nNode
                ? rPam.GetMark()->nContent.GetIndex()
                : 0;
    }

    if( m_bNoColls && !rTextNd.HasSwAttrSet() )
        return ;

    const SfxItemSet& rSet = rTextNd.GetSwAttrSet();

    SfxItemIter aIter( m_aComapeSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    const SfxPoolItem* pFndItem;
    sal_uInt16 nWhich;

    do
    {
        if( IsInvalidItem( pItem ) )
        {
            nWhich = m_aComapeSet.GetWhichByPos( aIter.GetCurPos() );
            if( RES_TXTATR_END <= nWhich )
                break; // end of text attributes

            if( SfxItemState::SET == rSet.GetItemState( nWhich, !m_bNoColls, &pFndItem )
                && !CmpAttr( *pFndItem, rSet.GetPool()->GetDefaultItem( nWhich ) ))
            {
                m_pFindArr[ nWhich - m_nArrStart ] =
                    SwSrchChrAttr( *pFndItem, m_nNodeStart, m_nNodeEnd );
                m_nFound++;
            }
        }
        else
        {
            nWhich = pItem->Which();
            if( RES_TXTATR_END <= nWhich )
                break; // end of text attributes

            if( CmpAttr( rSet.Get( nWhich, !m_bNoColls ), *pItem ) )
            {
                m_pFindArr[ nWhich - m_nArrStart ] =
                    SwSrchChrAttr( *pItem, m_nNodeStart, m_nNodeEnd );
                m_nFound++;
            }
        }

        pItem = aIter.NextItem();
    } while (pItem);
}

static bool
lcl_IsAttributeIgnorable(sal_Int32 const nNdStart, sal_Int32 const nNdEnd,
        SwSrchChrAttr const& rTmp)
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
    SwSrchChrAttr aTmp( rAttr.GetAttr(), rAttr.GetStart(), rAttr.GetAnyEnd() );

    // ignore all attributes not in search range
    if (lcl_IsAttributeIgnorable(m_nNodeStart, m_nNodeEnd, aTmp))
    {
        return Found();
    }

    const SfxPoolItem* pItem;
    // here we explicitly also search in character templates
    sal_uInt16 nWhch = rAttr.Which();
    std::unique_ptr<SfxWhichIter> pIter;
    const SfxPoolItem* pTmpItem = nullptr;
    const SfxItemSet* pSet = nullptr;
    if( RES_TXTATR_CHARFMT == nWhch || RES_TXTATR_AUTOFMT == nWhch )
    {
        if( m_bNoColls && RES_TXTATR_CHARFMT == nWhch )
            return Found();
        pTmpItem = nullptr;
        pSet = CharFormat::GetItemSet( rAttr.GetAttr() );
        if ( pSet )
        {
            pIter.reset(new SfxWhichIter( *pSet ));
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
        SfxItemState eState = m_aComapeSet.GetItemState( nWhch, false, &pItem );
        if( SfxItemState::DONTCARE == eState || SfxItemState::SET == eState )
        {
            sal_uInt16 n;
            SwSrchChrAttr* pCmp;

            // first delete all up to start position that are already invalid
            SwSrchChrAttr* pArrPtr;
            if( m_nFound )
                for( pArrPtr = m_pFindArr, n = 0; n < m_nArrLen;
                    ++n, ++pArrPtr )
                    if( pArrPtr->nWhich && pArrPtr->nEnd <= aTmp.nStt )
                    {
                        pArrPtr->nWhich = 0; // deleted
                        m_nFound--;
                    }

            // delete all up to start position that are already invalid and
            // move all "open" ones (= stick out over start position) from stack
            // into FndSet
            if( m_nStackCount )
                for( pArrPtr = m_pStackArr, n=0; n < m_nArrLen; ++n, ++pArrPtr )
                {
                    if( !pArrPtr->nWhich )
                        continue;

                    if( pArrPtr->nEnd <= aTmp.nStt )
                    {
                        pArrPtr->nWhich = 0; // deleted
                        if( !--m_nStackCount )
                            break;
                    }
                    else if( pArrPtr->nStt <= aTmp.nStt )
                    {
                        pCmp = &m_pFindArr[ n ];
                        if( pCmp->nWhich )
                        {
                            if( pCmp->nEnd < pArrPtr->nEnd ) // extend
                                pCmp->nEnd = pArrPtr->nEnd;
                        }
                        else
                        {
                            *pCmp = *pArrPtr;
                            m_nFound++;
                        }
                        pArrPtr->nWhich = 0;
                        if( !--m_nStackCount )
                            break;
                    }
                }

            bool bContinue = false;

            if( SfxItemState::DONTCARE == eState  )
            {
                // Will the attribute become valid?
                if( !CmpAttr( m_aComapeSet.GetPool()->GetDefaultItem( nWhch ),
                    *pTmpItem ))
                {
                    // search attribute and extend if needed
                    pCmp = &m_pFindArr[ nWhch - m_nArrStart ];
                    if( !pCmp->nWhich )
                    {
                        *pCmp = aTmp; // not found, insert
                        m_nFound++;
                    }
                    else if( pCmp->nEnd < aTmp.nEnd ) // extend?
                        pCmp->nEnd = aTmp.nEnd;

                    bContinue = true;
                }
            }
            // Will the attribute become valid?
            else if(  CmpAttr( *pItem, *pTmpItem ) )
            {
                m_pFindArr[ nWhch - m_nArrStart ] = aTmp;
                ++m_nFound;
                bContinue = true;
            }

            // then is has to go on the stack
            if( !bContinue )
            {
                pCmp = &m_pFindArr[ nWhch - m_nArrStart ];
                if (pCmp->nWhich )
                {
                    // exists on stack, only if it is even bigger
                    if( pCmp->nEnd > aTmp.nEnd )
                    {
                        OSL_ENSURE( !m_pStackArr[ nWhch - m_nArrStart ].nWhich,
                                        "slot on stack is still in use" );

                        if( aTmp.nStt <= pCmp->nStt )
                            pCmp->nStt = aTmp.nEnd;
                        else
                            pCmp->nEnd = aTmp.nStt;

                        m_pStackArr[ nWhch - m_nArrStart ] = *pCmp;
                        m_nStackCount++;
                    }
                    pCmp->nWhich = 0;
                    m_nFound--;
                }
            }
        }
        if( pIter )
        {
            assert(pSet && "otherwise no pIter");
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
    pIter.reset();
    return Found();
}

bool SwAttrCheckArr::SetAttrBwd( const SwTextAttr& rAttr )
{
    SwSrchChrAttr aTmp( rAttr.GetAttr(), rAttr.GetStart(), rAttr.GetAnyEnd() );

    // ignore all attributes not in search range
    if (lcl_IsAttributeIgnorable(m_nNodeStart, m_nNodeEnd, aTmp))
    {
        return Found();
    }

    const SfxPoolItem* pItem;
    // here we explicitly also search in character templates
    sal_uInt16 nWhch = rAttr.Which();
    std::unique_ptr<SfxWhichIter> pIter;
    const SfxPoolItem* pTmpItem = nullptr;
    const SfxItemSet* pSet = nullptr;
    if( RES_TXTATR_CHARFMT == nWhch || RES_TXTATR_AUTOFMT == nWhch )
    {
        if( m_bNoColls && RES_TXTATR_CHARFMT == nWhch )
            return Found();

        pSet = CharFormat::GetItemSet( rAttr.GetAttr() );
        if ( pSet )
        {
            pIter.reset( new SfxWhichIter( *pSet ) );
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
        SfxItemState eState = m_aComapeSet.GetItemState( nWhch, false, &pItem );
        if( SfxItemState::DONTCARE == eState || SfxItemState::SET == eState )
        {
            sal_uInt16 n;
            SwSrchChrAttr* pCmp;

            // first delete all up to start position that are already invalid
            SwSrchChrAttr* pArrPtr;
            if( m_nFound )
                for( pArrPtr = m_pFindArr, n = 0; n < m_nArrLen; ++n, ++pArrPtr )
                    if( pArrPtr->nWhich && pArrPtr->nStt >= aTmp.nEnd )
                    {
                        pArrPtr->nWhich = 0; // deleted
                        m_nFound--;
                    }

            // delete all up to start position that are already invalid and
            // move all "open" ones (= stick out over start position) from stack
            // into FndSet
            if( m_nStackCount )
                for( pArrPtr = m_pStackArr, n = 0; n < m_nArrLen; ++n, ++pArrPtr )
                {
                    if( !pArrPtr->nWhich )
                        continue;

                    if( pArrPtr->nStt >= aTmp.nEnd )
                    {
                        pArrPtr->nWhich = 0; // deleted
                        if( !--m_nStackCount )
                            break;
                    }
                    else if( pArrPtr->nEnd >= aTmp.nEnd )
                    {
                        pCmp = &m_pFindArr[ n ];
                        if( pCmp->nWhich )
                        {
                            if( pCmp->nStt > pArrPtr->nStt ) // extend
                                pCmp->nStt = pArrPtr->nStt;
                        }
                        else
                        {
                            *pCmp = *pArrPtr;
                            m_nFound++;
                        }
                        pArrPtr->nWhich = 0;
                        if( !--m_nStackCount )
                            break;
                    }
                }

            bool bContinue = false;
            if( SfxItemState::DONTCARE == eState  )
            {
                // Will the attribute become valid?
                if( !CmpAttr( m_aComapeSet.GetPool()->GetDefaultItem( nWhch ),
                    *pTmpItem ) )
                {
                    // search attribute and extend if needed
                    pCmp = &m_pFindArr[ nWhch - m_nArrStart ];
                    if( !pCmp->nWhich )
                    {
                        *pCmp = aTmp; // not found, insert
                        m_nFound++;
                    }
                    else if( pCmp->nStt > aTmp.nStt ) // extend?
                        pCmp->nStt = aTmp.nStt;

                    bContinue = true;
                }
            }
            // Will the attribute become valid?
            else if( CmpAttr( *pItem, *pTmpItem ))
            {
                m_pFindArr[ nWhch - m_nArrStart ] = aTmp;
                ++m_nFound;
                bContinue = true;
            }

            // then is has to go on the stack
            if( !bContinue )
            {
                pCmp = &m_pFindArr[ nWhch - m_nArrStart ];
                if( pCmp->nWhich )
                {
                    // exists on stack, only if it is even bigger
                    if( pCmp->nStt < aTmp.nStt )
                    {
                        OSL_ENSURE( !m_pStackArr[ nWhch - m_nArrStart ].nWhich,
                                "slot on stack is still in use" );

                        if( aTmp.nEnd <= pCmp->nEnd )
                            pCmp->nEnd = aTmp.nStt;
                        else
                            pCmp->nStt = aTmp.nEnd;

                        m_pStackArr[ nWhch - m_nArrStart ] = *pCmp;
                        m_nStackCount++;
                    }
                    pCmp->nWhich = 0;
                    m_nFound--;
                }
            }
        }
        if( pIter )
        {
            assert(pSet && "otherwise no pIter");
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
    pIter.reset();
    return Found();
}

sal_Int32 SwAttrCheckArr::Start() const
{
    sal_Int32 nStart = m_nNodeStart;
    SwSrchChrAttr* pArrPtr = m_pFindArr;
    for( sal_uInt16 n = 0; n < m_nArrLen; ++n, ++pArrPtr )
        if( pArrPtr->nWhich && pArrPtr->nStt > nStart )
            nStart = pArrPtr->nStt;

    return nStart;
}

sal_Int32 SwAttrCheckArr::End() const
{
    SwSrchChrAttr* pArrPtr = m_pFindArr;
    sal_Int32 nEnd = m_nNodeEnd;
    for( sal_uInt16 n = 0; n < m_nArrLen; ++n, ++pArrPtr )
        if( pArrPtr->nWhich && pArrPtr->nEnd < nEnd )
            nEnd = pArrPtr->nEnd;

    return nEnd;
}

bool SwAttrCheckArr::CheckStack()
{
    if( !m_nStackCount )
        return false;

    sal_uInt16 n;
    const sal_Int32 nSttPos = Start();
    const sal_Int32 nEndPos = End();
    SwSrchChrAttr* pArrPtr;
    for( pArrPtr = m_pStackArr, n = 0; n < m_nArrLen; ++n, ++pArrPtr )
    {
        if( !pArrPtr->nWhich )
            continue;

        if( m_bForward ? pArrPtr->nEnd <= nSttPos : pArrPtr->nStt >= nEndPos )
        {
            pArrPtr->nWhich = 0; // deleted
            if( !--m_nStackCount )
                return m_nFound == m_aComapeSet.Count();
        }
        else if( m_bForward ? pArrPtr->nStt < nEndPos : pArrPtr->nEnd > nSttPos )
        {
            // move all "open" ones (= stick out over start position) into FndSet
            OSL_ENSURE( !m_pFindArr[ n ].nWhich, "slot in array is already in use" );
            m_pFindArr[ n ] = *pArrPtr;
            pArrPtr->nWhich = 0;
            m_nFound++;
            if( !--m_nStackCount )
                return m_nFound == m_aComapeSet.Count();
        }
    }
    return m_nFound == m_aComapeSet.Count();
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
        {
            pAttr = rHtArr.Get( nPos );
            if( !rCmpArr.SetAttrFwd( *pAttr ) )
            {
                if( rCmpArr.GetNdStt() < pAttr->GetStart() )
                {
                    // found end
                    auto nTmpStart = pAttr->GetStart();
                    lcl_SetAttrPam( rPam, rCmpArr.GetNdStt(),
                                &nTmpStart, true );
                    return true;
                }
                // continue search
                break;
            }
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
    {
        pAttr = rHtArr.Get( nPos );
        if( rCmpArr.SetAttrFwd( *pAttr ) )
        {
            // Do multiple start at that position? Do also check those:
            nSttPos = pAttr->GetStart();
            while( ++nPos < rHtArr.Count() )
            {
                pAttr = rHtArr.Get( nPos );
                if( nSttPos != pAttr->GetStart() || !rCmpArr.SetAttrFwd( *pAttr ) )
                    break;
            }

            if( !rCmpArr.Found() )
                continue;

            // then we have our search area
            nSttPos = rCmpArr.Start();
            nEndPos = rCmpArr.End();
            if( nSttPos > nEndPos )
                return false;

            lcl_SetAttrPam( rPam, nSttPos, &nEndPos, true );
            return true;
        }
    }

    if( !rCmpArr.CheckStack() )
        return false;
    nSttPos = rCmpArr.Start();
    nEndPos = rCmpArr.End();
    if( nSttPos > nEndPos )
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
        {
            pAttr = rHtArr.GetSortedByEnd( --nPos );
            if( !rCmpArr.SetAttrBwd( *pAttr ) )
            {
                nSttPos = pAttr->GetAnyEnd();
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
    {
        pAttr = rHtArr.GetSortedByEnd( --nPos );
        if( rCmpArr.SetAttrBwd( *pAttr ) )
        {
            // Do multiple start at that position? Do also check those:
            if( nPos )
            {
                nEndPos = pAttr->GetAnyEnd();
                while( --nPos )
                {
                    pAttr = rHtArr.GetSortedByEnd( nPos );
                    if( nEndPos != pAttr->GetAnyEnd() || !rCmpArr.SetAttrBwd( *pAttr ) )
                        break;
                }
            }
            if( !rCmpArr.Found() )
                continue;

            // then we have our search area
            nSttPos = rCmpArr.Start();
            nEndPos = rCmpArr.End();
            if( nSttPos > nEndPos )
                return false;

            lcl_SetAttrPam( rPam, nSttPos, &nEndPos, false );
            return true;
        }
    }

    if( !rCmpArr.CheckStack() )
        return false;
    nSttPos = rCmpArr.Start();
    nEndPos = rCmpArr.End();
    if( nSttPos > nEndPos )
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

    do
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

        pItem = aIter.NextItem();
    } while (pItem);
    return true; // found
}

namespace sw {

bool FindAttrImpl(SwPaM & rSearchPam,
        const SfxPoolItem& rAttr, SwMoveFnCollection const & fnMove,
        const SwPaM & rRegion, bool bInReadOnly,
        SwRootFrame const*const pLayout)
{
    // determine which attribute is searched:
    const sal_uInt16 nWhich = rAttr.Which();
    bool bCharAttr = isCHRATR(nWhich) || isTXTATR(nWhich);
    assert(isTXTATR(nWhich)); // sw_redlinehide: only works for non-formatting hints such as needed in UpdateFields; use FindAttrsImpl for others

    std::unique_ptr<SwPaM> pPam(sw::MakeRegion(fnMove, rRegion));

    bool bFound = false;
    bool bFirst = true;
    const bool bSrchForward = &fnMove == &fnMoveForward;
    SwContentNode * pNode;

    // if at beginning/end then move it out of the node
    if( bSrchForward
        ? pPam->GetPoint()->nContent.GetIndex() == pPam->GetContentNode()->Len()
        : !pPam->GetPoint()->nContent.GetIndex() )
    {
        if( !(*fnMove.fnNds)( &pPam->GetPoint()->nNode, false ))
        {
            return false;
        }
        SwContentNode *pNd = pPam->GetContentNode();
        pPam->GetPoint()->nContent.Assign( pNd, bSrchForward ? 0 : pNd->Len() );
    }

    while (nullptr != (pNode = ::GetNode(*pPam, bFirst, fnMove, bInReadOnly, pLayout)))
    {
        if( bCharAttr )
        {
            if( !pNode->IsTextNode() ) // CharAttr are only in text nodes
                continue;

            SwTextFrame const*const pFrame(pLayout
                ? static_cast<SwTextFrame const*>(pNode->getLayoutFrame(pLayout))
                : nullptr);
            if (pFrame)
            {
                SwTextNode const* pAttrNode(nullptr);
                SwTextAttr const* pAttr(nullptr);
                if (bSrchForward)
                {
                    sw::MergedAttrIter iter(*pFrame);
                    do
                    {
                        pAttr = iter.NextAttr(&pAttrNode);
                    }
                    while (pAttr
                        && (pAttrNode->GetIndex() < pPam->GetPoint()->nNode.GetIndex()
                            || (pAttrNode->GetIndex() == pPam->GetPoint()->nNode.GetIndex()
                                && pAttr->GetStart() < pPam->GetPoint()->nContent.GetIndex())
                            || pAttr->Which() != nWhich));
                }
                else
                {
                    sw::MergedAttrIterReverse iter(*pFrame);
                    do
                    {
                        pAttr = iter.PrevAttr(&pAttrNode);
                    }
                    while (pAttr
                        && (pPam->GetPoint()->nNode.GetIndex() < pAttrNode->GetIndex()
                            || (pPam->GetPoint()->nNode.GetIndex() == pAttrNode->GetIndex()
                                && pPam->GetPoint()->nContent.GetIndex() <= pAttr->GetStart())
                            || pAttr->Which() != nWhich));
                }
                if (pAttr)
                {
                    assert(pAttrNode);
                    pPam->GetPoint()->nNode = *pAttrNode;
                    lcl_SetAttrPam(*pPam, pAttr->GetStart(), pAttr->End(), bSrchForward);
                    bFound = true;
                    break;
                }
            }
            else if (!pLayout && pNode->GetTextNode()->HasHints() &&
                lcl_SearchAttr(*pNode->GetTextNode(), *pPam, rAttr, fnMove))
            {
                bFound = true;
            }
            if (bFound)
            {
                // set to the values of the attribute
                rSearchPam.SetMark();
                *rSearchPam.GetPoint() = *pPam->GetPoint();
                *rSearchPam.GetMark() = *pPam->GetMark();
                break;
            }
            else if (isTXTATR(nWhich))
                continue;
        }

#if 0
        // no hard attribution, so check if node was asked for this attr before
        if( !pNode->HasSwAttrSet() )
        {
            SwFormat* pTmpFormat = pNode->GetFormatColl();
            if( !aFormatArr.insert( pTmpFormat ).second )
                continue; // collection was requested earlier
        }

        if( SfxItemState::SET == pNode->GetSwAttrSet().GetItemState( nWhich,
                true, &pItem ))
        {
            // FORWARD:  SPoint at the end, GetMark at the beginning of the node
            // BACKWARD: SPoint at the beginning, GetMark at the end of the node
            // always: incl. start and incl. end
            *rSearchPam.GetPoint() = *pPam->GetPoint();
            rSearchPam.SetMark();
            pNode->MakeEndIndex( &rSearchPam.GetPoint()->nContent );
            bFound = true;
            break;
        }
#endif
    }

    // if backward search, switch point and mark
    if( bFound && !bSrchForward )
        rSearchPam.Exchange();

    return bFound;
}

} // namespace sw

typedef bool (*FnSearchAttr)( const SwTextNode&, SwAttrCheckArr&, SwPaM& );

static bool FindAttrsImpl(SwPaM & rSearchPam,
        const SfxItemSet& rSet, bool bNoColls, SwMoveFnCollection const & fnMove,
        const SwPaM & rRegion, bool bInReadOnly, bool bMoveFirst,
        SwRootFrame const*const pLayout)
{
    std::unique_ptr<SwPaM> pPam(sw::MakeRegion(fnMove, rRegion));

    bool bFound = false;
    bool bFirst = true;
    const bool bSrchForward = &fnMove == &fnMoveForward;
    SwContentNode * pNode;
    o3tl::sorted_vector<SwFormat*> aFormatArr;

    // check which text/char attributes are searched
    SwAttrCheckArr aCmpArr( rSet, bSrchForward, bNoColls );
    SfxItemSet aOtherSet( rSearchPam.GetDoc().GetAttrPool(),
                            svl::Items<RES_PARATR_BEGIN, RES_GRFATR_END-1>{} );
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
        if( !(*fnMove.fnNds)( &pPam->GetPoint()->nNode, false ))
        {
            return false;
        }
        SwContentNode *pNd = pPam->GetContentNode();
        pPam->GetPoint()->nContent.Assign( pNd, bSrchForward ? 0 : pNd->Len() );
    }

    while (nullptr != (pNode = ::GetNode(*pPam, bFirst, fnMove, bInReadOnly, pLayout)))
    {
        SwTextFrame const*const pFrame(pLayout && pNode->IsTextNode()
            ? static_cast<SwTextFrame const*>(pNode->getLayoutFrame(pLayout))
            : nullptr);
        assert(!pLayout || !pNode->IsTextNode() || pFrame);
        // sw_redlinehide: it's apparently not possible to find break items
        // with the UI, so checking one node is enough
        SwContentNode const& rPropsNode(*(pFrame
            ? pFrame->GetTextNodeForParaProps()
            : pNode));

        if( aCmpArr.Count() )
        {
            if( !pNode->IsTextNode() ) // CharAttr are only in text nodes
                continue;

            if (aOtherSet.Count() &&
                !lcl_Search(rPropsNode, aOtherSet, bNoColls))
            {
                continue;
            }
            sw::MergedPara const*const pMergedPara(pFrame ? pFrame->GetMergedPara() : nullptr);
            if (pMergedPara)
            {
                SwPosition const& rStart(*pPam->Start());
                SwPosition const& rEnd(*pPam->End());
                // no extents? fall back to searching index 0 of propsnode
                // to find its node items
                if (pMergedPara->extents.empty())
                {
                    if (rStart.nNode.GetIndex() <= rPropsNode.GetIndex()
                        && rPropsNode.GetIndex() <= rEnd.nNode.GetIndex())
                    {
                        SwPaM tmp(rPropsNode, 0, rPropsNode, 0);
                        bFound = (*fnSearch)(*pNode->GetTextNode(), aCmpArr, tmp);
                        if (bFound)
                        {
                            *pPam = tmp;
                        }
                    }
                }
                else
                {
                    // iterate the extents, and intersect with input pPam:
                    // the found ranges should never include delete redlines
                    // so that subsequent Replace will not affect them
                    for (size_t i = 0; i < pMergedPara->extents.size(); ++i)
                    {
                        auto const rExtent(pMergedPara->extents[bSrchForward
                                ? i
                                : pMergedPara->extents.size() - i - 1]);
                        if (rExtent.pNode->GetIndex() < rStart.nNode.GetIndex()
                            || rEnd.nNode.GetIndex() < rExtent.pNode->GetIndex())
                        {
                            continue;
                        }
                        sal_Int32 const nStart(rExtent.pNode == &rStart.nNode.GetNode()
                                ? rStart.nContent.GetIndex()
                                : 0);
                        if (rExtent.nEnd <= nStart)
                        {
                            continue;
                        }
                        sal_Int32 const nEnd(rExtent.pNode == &rEnd.nNode.GetNode()
                                ? rEnd.nContent.GetIndex()
                                : rExtent.pNode->Len());
                        if (nEnd < rExtent.nStart
                            || (nStart != nEnd && nEnd == rExtent.nStart))
                        {
                            continue;
                        }
                        SwPaM tmp(*rExtent.pNode, std::max(nStart, rExtent.nStart),
                            *rExtent.pNode, std::min(nEnd, rExtent.nEnd));
                        tmp.Normalize(bSrchForward);
                        bFound = (*fnSearch)(*rExtent.pNode, aCmpArr, tmp);
                        if (bFound)
                        {
                            *pPam = tmp;
                            break;
                        }
                    }
                }
            }
            else
            {
                bFound = (*fnSearch)(*pNode->GetTextNode(), aCmpArr, *pPam);
            }
            if (bFound)
            {
                // set to the values of the attribute
                rSearchPam.SetMark();
                *rSearchPam.GetPoint() = *pPam->GetPoint();
                *rSearchPam.GetMark() = *pPam->GetMark();
                break;
            }
            continue; // text attribute
        }

        if( !aOtherSet.Count() )
            continue;

        // no hard attribution, so check if node was asked for this attr before
        // (as an optimisation)
        if (!rPropsNode.HasSwAttrSet())
        {
            SwFormat* pTmpFormat = rPropsNode.GetFormatColl();
            if( !aFormatArr.insert( pTmpFormat ).second )
                continue; // collection was requested earlier
        }

        if (lcl_Search(rPropsNode, aOtherSet, bNoColls))
        {
            // FORWARD:  SPoint at the end, GetMark at the beginning of the node
            // BACKWARD: SPoint at the beginning, GetMark at the end of the node
            if (pFrame)
            {
                *rSearchPam.GetPoint() = *pPam->GetPoint();
                rSearchPam.SetMark();
                *rSearchPam.GetMark() = pFrame->MapViewToModelPos(
                    TextFrameIndex(bSrchForward ? pFrame->GetText().getLength() : 0));
            }
            else
            {
                *rSearchPam.GetPoint() = *pPam->GetPoint();
                rSearchPam.SetMark();
                if (bSrchForward)
                {
                    pNode->MakeEndIndex( &rSearchPam.GetPoint()->nContent );
                }
                else
                {
                    pNode->MakeStartIndex( &rSearchPam.GetPoint()->nContent );
                }
            }
            bFound = true;
            break;
        }
    }

    // in search direction, mark precedes point, because the next iteration
    // starts at point
    if (bFound)
    {
        rSearchPam.Normalize(!bSrchForward);
    }

    return bFound;
}

namespace {

/// parameters for search for attributes
struct SwFindParaAttr : public SwFindParas
{
    bool m_bNoCollection;
    const SfxItemSet *pSet, *pReplSet;
    const i18nutil::SearchOptions2 *pSearchOpt;
    SwCursor& m_rCursor;
    SwRootFrame const* m_pLayout;
    std::unique_ptr<utl::TextSearch> pSText;

    SwFindParaAttr( const SfxItemSet& rSet, bool bNoCollection,
                    const i18nutil::SearchOptions2* pOpt, const SfxItemSet* pRSet,
                    SwCursor& rCursor, SwRootFrame const*const pLayout)
        : m_bNoCollection(bNoCollection)
        , pSet( &rSet )
        , pReplSet( pRSet )
        , pSearchOpt( pOpt )
        , m_rCursor(rCursor)
        , m_pLayout(pLayout)
    {}

    virtual ~SwFindParaAttr()   {}

    virtual int DoFind(SwPaM &, SwMoveFnCollection const &, const SwPaM &, bool bInReadOnly,
                       std::unique_ptr<SvxSearchItem>& xSearchItem) override;
    virtual bool IsReplaceMode() const override;
};

}

int SwFindParaAttr::DoFind(SwPaM & rCursor, SwMoveFnCollection const & fnMove,
        const SwPaM & rRegion, bool bInReadOnly,
        std::unique_ptr<SvxSearchItem>& xSearchItem)
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
        SwPaM aRegion( *rRegion.GetMark(), *rRegion.GetPoint() );
        SwPaM* pTextRegion = &aRegion;
        SwPaM aSrchPam( *rCursor.GetPoint() );

        while( true )
        {
            if( pSet->Count() ) // any attributes?
            {
                // first attributes
                if (!FindAttrsImpl(aSrchPam, *pSet, m_bNoCollection, fnMove, aRegion, bInReadOnly, bMoveFirst, m_pLayout))
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
                i18nutil::SearchOptions2 aTmp( *pSearchOpt );

                // search in selection
                aTmp.searchFlag |= (SearchFlags::REG_NOT_BEGINOFLINE |
                                    SearchFlags::REG_NOT_ENDOFLINE);

                aTmp.Locale = SvtSysLocale().GetLanguageTag().getLocale();

                pSText.reset( new utl::TextSearch( aTmp ) );
            }

            // TODO: searching for attributes in Outliner text?!

            // continue search in correct section (pTextRegion)
            if (sw::FindTextImpl(aSrchPam, *pSearchOpt, false/*bSearchInNotes*/, *pSText, fnMove, *pTextRegion, bInReadOnly, m_pLayout, xSearchItem) &&
                *aSrchPam.GetMark() != *aSrchPam.GetPoint() )
                break; // found
            else if( !pSet->Count() )
                return FIND_NOT_FOUND; // only text and nothing found

            *aRegion.GetMark() = *aSrchPam.GetPoint();
        }

        *rCursor.GetPoint() = *aSrchPam.GetPoint();
        rCursor.SetMark();
        *rCursor.GetMark() = *aSrchPam.GetMark();
    }

    if( bReplaceText )
    {
        const bool bRegExp(
                SearchAlgorithms2::REGEXP == pSearchOpt->AlgorithmType2);
        SwIndex& rSttCntIdx = rCursor.Start()->nContent;
        const sal_Int32 nSttCnt = rSttCntIdx.GetIndex();

        // add to shell-cursor-ring so that the regions will be moved eventually
        SwPaM* pPrevRing(nullptr);
        if( bRegExp )
        {
            pPrevRing = const_cast<SwPaM &>(rRegion).GetPrev();
            const_cast<SwPaM &>(rRegion).GetRingContainer().merge( m_rCursor.GetRingContainer() );
        }

        std::optional<OUString> xRepl;
        if (bRegExp)
            xRepl = sw::ReplaceBackReferences(*pSearchOpt, &rCursor, m_pLayout);
        sw::ReplaceImpl(rCursor,
                xRepl ? *xRepl : pSearchOpt->replaceString, bRegExp,
                m_rCursor.GetDoc(), m_pLayout);

        m_rCursor.SaveTableBoxContent( rCursor.GetPoint() );

        if( bRegExp )
        {
            // and remove region again
            SwPaM* p;
            SwPaM* pNext = const_cast<SwPaM*>(&rRegion);
            do {
                p = pNext;
                pNext = p->GetNext();
                p->MoveTo(const_cast<SwPaM*>(&rRegion));
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
            rCursor.GetDoc().getIDocumentContentOperations().InsertItemSet(
                    rCursor, *pReplSet, SetAttrMode::DEFAULT, m_pLayout);
        }
        else
        {
            SfxItemPool* pPool = pReplSet->GetPool();
            SfxItemSet aSet( *pPool, pReplSet->GetRanges() );

            SfxItemIter aIter( *pSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            do
            {
                // reset all that are not set with pool defaults
                if( !IsInvalidItem( pItem ) && SfxItemState::SET !=
                    pReplSet->GetItemState( pItem->Which(), false ))
                    aSet.Put( pPool->GetDefaultItem( pItem->Which() ));

                pItem = aIter.NextItem();
            } while (pItem);
            aSet.Put( *pReplSet );
            rCursor.GetDoc().getIDocumentContentOperations().InsertItemSet(
                    rCursor, aSet, SetAttrMode::DEFAULT, m_pLayout);
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
sal_uLong SwCursor::FindAttrs( const SfxItemSet& rSet, bool bNoCollections,
                          SwDocPositions nStart, SwDocPositions nEnd,
                          bool& bCancel, FindRanges eFndRngs,
                          const i18nutil::SearchOptions2* pSearchOpt,
                          const SfxItemSet* pReplSet,
                          SwRootFrame const*const pLayout)
{
    // switch off OLE-notifications
    SwDoc& rDoc = GetDoc();
    Link<bool,void> aLnk( rDoc.GetOle2Link() );
    rDoc.SetOle2Link( Link<bool,void>() );

    bool bReplace = ( pSearchOpt && ( !pSearchOpt->replaceString.isEmpty() ||
                                    !rSet.Count() ) ) ||
                    (pReplSet && pReplSet->Count());
    bool const bStartUndo = rDoc.GetIDocumentUndoRedo().DoesUndo() && bReplace;
    if (bStartUndo)
    {
        rDoc.GetIDocumentUndoRedo().StartUndo( SwUndoId::REPLACE, nullptr );
    }

    SwFindParaAttr aSwFindParaAttr( rSet, bNoCollections, pSearchOpt,
                                    pReplSet, *this, pLayout );

    sal_uLong nRet = FindAll( aSwFindParaAttr, nStart, nEnd, eFndRngs, bCancel );
    rDoc.SetOle2Link( aLnk );
    if( nRet && bReplace )
        rDoc.getIDocumentState().SetModified();

    if (bStartUndo)
    {
        rDoc.GetIDocumentUndoRedo().EndUndo( SwUndoId::REPLACE, nullptr );
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
