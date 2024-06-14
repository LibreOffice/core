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

#include <editeng/rsiditem.hxx>
#include <sal/log.hxx>
#include <txatbase.hxx>
#include <ndhints.hxx>
#include <txtatr.hxx>

#ifdef DBG_UTIL
#include <pam.hxx>
#include <fmtautofmt.hxx>
#endif

/// sort order: Start, End (reverse), Which (reverse),
/// (char style: sort number), at last the pointer
static bool CompareSwpHtStart( const SwTextAttr* lhs, const SwTextAttr* rhs )
{
    const SwTextAttr &rHt1 = *lhs;
    const SwTextAttr &rHt2 = *rhs;
    if ( rHt1.GetStart() == rHt2.GetStart() )
    {
        const sal_Int32 nHt1 = rHt1.GetAnyEnd();
        const sal_Int32 nHt2 = rHt2.GetAnyEnd();
        if ( nHt1 == nHt2 )
        {
            const sal_uInt16 nWhich1 = rHt1.Which();
            const sal_uInt16 nWhich2 = rHt2.Which();
            if ( nWhich1 == nWhich2 )
            {
                if ( RES_TXTATR_CHARFMT == nWhich1 )
                {
                    const sal_uInt16 nS1 =
                        static_txtattr_cast<const SwTextCharFormat&>(rHt1).GetSortNumber();
                    const sal_uInt16 nS2 =
                        static_txtattr_cast<const SwTextCharFormat&>(rHt2).GetSortNumber();
                    if ( nS1 != nS2 ) // robust
                        return nS1 < nS2;
                }

                return reinterpret_cast<sal_IntPtr>(&rHt1) < reinterpret_cast<sal_IntPtr>(&rHt2);
            }
            // order is important! for requirements see hintids.hxx
            return ( nWhich1 > nWhich2 );
        }
        return ( nHt1 > nHt2 );
    }
    return ( rHt1.GetStart() < rHt2.GetStart() );
}

/// sort order: Start
/// (char style: sort number), at last the pointer
namespace {
struct CompareSwpHtStartOnly
{
    bool operator()( const SwTextAttr* lhs, sal_Int32 rhs ) const
    {
        return lhs->GetStart() < rhs;
    }
    bool operator()( sal_Int32 lhs, const SwTextAttr* rhs ) const
    {
        return lhs < rhs->GetStart();
    }
};
struct CompareSwpHtEndOnly
{
    bool operator()( const SwTextAttr* lhs, sal_Int32 rhs ) const
    {
        return lhs->GetAnyEnd() < rhs;
    }
    bool operator()( sal_Int32 lhs, const SwTextAttr* rhs ) const
    {
        return lhs < rhs->GetAnyEnd();
    }
};
}

/// sort order: Which, Start, End(reverse) at last the pointer
bool CompareSwpHtWhichStart::operator()( const SwTextAttr* lhs, const sal_uInt16 nWhich ) const
{
    return lhs->Which() < nWhich;
}
bool CompareSwpHtWhichStart::operator()( const SwTextAttr* lhs, const SwTextAttr* rhs ) const
{
    const SwTextAttr &rHt1 = *lhs;
    const SwTextAttr &rHt2 = *rhs;
    const sal_uInt16 nWhich1 = rHt1.Which();
    const sal_uInt16 nWhich2 = rHt2.Which();
    if ( nWhich1 < nWhich2 )
        return true;
    if ( nWhich1 > nWhich2 )
        return false;
    if (rHt1.GetStart() < rHt2.GetStart())
        return true;
    if (rHt1.GetStart() > rHt2.GetStart())
        return false;
    if ( RES_TXTATR_CHARFMT == nWhich1 )
    {
        const sal_uInt16 nS1 =
                        static_txtattr_cast<const SwTextCharFormat&>(rHt1).GetSortNumber();
        const sal_uInt16 nS2 =
                        static_txtattr_cast<const SwTextCharFormat&>(rHt2).GetSortNumber();
        if ( nS1 != nS2 ) // robust
            return nS1 < nS2;
    }
    const sal_Int32 nEnd1 = rHt1.GetAnyEnd();
    const sal_Int32 nEnd2 = rHt2.GetAnyEnd();
    if ( nEnd1 > nEnd2 )
        return true;
    if ( nEnd1 < nEnd2 )
        return false;
    return reinterpret_cast<sal_IntPtr>(&rHt1) < reinterpret_cast<sal_IntPtr>(&rHt2);
}
bool CompareSwpHtWhichStart::operator()( const SwTextAttr* lhs, const WhichStartPair rhs ) const
{
    if ( lhs->Which() < rhs.first )
        return true;
    if ( lhs->Which() > rhs.first )
        return false;
    return lhs->GetStart() < rhs.second;
}
bool CompareSwpHtWhichStart::operator()( const WhichStartPair lhs, const SwTextAttr* rhs ) const
{
    if ( lhs.first < rhs->Which() )
        return true;
    if ( lhs.first > rhs->Which() )
        return false;
    return lhs.second < rhs->GetStart();
}

/// sort order: End, Start(reverse), Which
/// (char style: sort number), at last the pointer(reverse)
bool CompareSwpHtEnd::operator()( sal_Int32 nEndPos, const SwTextAttr* rhs ) const
{
    return nEndPos < rhs->GetAnyEnd();
}
bool CompareSwpHtEnd::operator()( const SwTextAttr* lhs, const SwTextAttr* rhs ) const
{
    const SwTextAttr &rHt1 = *lhs;
    const SwTextAttr &rHt2 = *rhs;
    const sal_Int32 nHt1 = rHt1.GetAnyEnd();
    const sal_Int32 nHt2 = rHt2.GetAnyEnd();
    if ( nHt1 == nHt2 )
    {
        if ( rHt1.GetStart() == rHt2.GetStart() )
        {
            const sal_uInt16 nWhich1 = rHt1.Which();
            const sal_uInt16 nWhich2 = rHt2.Which();
            if ( nWhich1 == nWhich2 )
            {
                if ( RES_TXTATR_CHARFMT == nWhich1 )
                {
                    const sal_uInt16 nS1 =
                            static_txtattr_cast<const SwTextCharFormat&>(rHt1).GetSortNumber();
                    const sal_uInt16 nS2 =
                            static_txtattr_cast<const SwTextCharFormat&>(rHt2).GetSortNumber();
                    if ( nS1 != nS2 ) // robust
                        return nS1 > nS2;
                }

                return reinterpret_cast<sal_IntPtr>(&rHt1) > reinterpret_cast<sal_IntPtr>(&rHt2);
            }
            // order is important! for requirements see hintids.hxx
            return ( nWhich1 < nWhich2 );
        }
        else
            return ( rHt1.GetStart() > rHt2.GetStart() );
    }
    return ( nHt1 < nHt2 );
}

void SwpHints::Insert(SwTextAttr* pHt)
{
    assert(std::find(m_HintsByStart.begin(), m_HintsByStart.end(), pHt)
            == m_HintsByStart.end()); // "Insert: hint already in HtStart"
    assert( pHt->m_pHints == nullptr );
    pHt->m_pHints = this;

    ResortStartMap();
    ResortEndMap();
    ResortWhichMap();

    auto it1 = std::lower_bound(m_HintsByStart.begin(), m_HintsByStart.end(), pHt, CompareSwpHtStart);
    m_HintsByStart.insert(it1, pHt);

    auto it2 = std::lower_bound(m_HintsByEnd.begin(), m_HintsByEnd.end(), pHt, CompareSwpHtEnd());
    m_HintsByEnd.insert(it2, pHt);

    auto it3 = std::lower_bound(m_HintsByWhichAndStart.begin(), m_HintsByWhichAndStart.end(), pHt, CompareSwpHtWhichStart());
    m_HintsByWhichAndStart.insert(it3, pHt);
}

bool SwpHints::Contains( const SwTextAttr *pHt ) const
{
    // DO NOT use find() or CHECK here!
    // if called from SwTextNode::InsertItem, pHt has already been deleted,
    // so it cannot be dereferenced
    return std::find(m_HintsByStart.begin(), m_HintsByStart.end(), pHt)
        != m_HintsByStart.end();
}

#ifdef DBG_UTIL

#define CHECK_ERR(cond, text) \
        if(!(cond)) \
        { \
            SAL_WARN("sw.core", text); \
            Resort(); \
            return false; \
        }

bool SwpHints::Check(bool bPortionsMerged) const
{
    // 1) both arrays have same size
    CHECK_ERR( m_HintsByStart.size() == m_HintsByEnd.size(),
        "HintsCheck: wrong sizes" );
    sal_Int32 nLastStart = 0;
    sal_Int32 nLastEnd   = 0;

    const SwTextAttr *pLastStart = nullptr;
    const SwTextAttr *pLastEnd = nullptr;
    o3tl::sorted_vector<SwTextAttr const*> RsidOnlyAutoFormats;
    if (bPortionsMerged)
    {
        for (size_t i = 0; i < Count(); ++i)
        {
            SwTextAttr const*const pHint(m_HintsByStart[i]);
            if (RES_TXTATR_AUTOFMT == pHint->Which())
            {
                std::shared_ptr<SfxItemSet> const & pSet(
                        pHint->GetAutoFormat().GetStyleHandle());
                if (pSet->Count() == 1 && pSet->GetItem(RES_CHRATR_RSID, false))
                {
                    RsidOnlyAutoFormats.insert(pHint);
                }
            }
        }
    }

    // --- cross checks ---
    // same pointers in both arrays
    auto tmpHintsByEnd = m_HintsByEnd;
    std::sort(tmpHintsByEnd.begin(), tmpHintsByEnd.end(), CompareSwpHtStart);
    CHECK_ERR( tmpHintsByEnd == m_HintsByStart, "HintsCheck: the two arrays do not contain the same set of pointers" );

    for( size_t i = 0; i < Count(); ++i )
    {
        // --- check Starts ---

        // 2a) valid pointer? depends on overwriting freed mem with 0xFF
        const SwTextAttr *pHt = m_HintsByStart[i];
        CHECK_ERR( 0xFF != *reinterpret_cast<unsigned char const *>(pHt), "HintsCheck: start ptr was deleted" );

        // 3a) start sort order?
        sal_Int32 nIdx = pHt->GetStart();
        CHECK_ERR( nIdx >= nLastStart, "HintsCheck: starts are unsorted" );

        // 4a) IsLessStart consistency
        if( pLastStart )
            CHECK_ERR( CompareSwpHtStart( pLastStart, pHt ), "HintsCheck: IsLastStart" );

        nLastStart = nIdx;
        pLastStart = pHt;

        // --- check Ends ---

        // 2b) valid pointer? see DELETEFF
        const SwTextAttr *pHtEnd = m_HintsByEnd[i];
        CHECK_ERR( 0xFF != *reinterpret_cast<unsigned char const *>(pHtEnd), "HintsCheck: end ptr was deleted" );

        // 3b) end sort order?
        nIdx = pHtEnd->GetAnyEnd();
        CHECK_ERR( nIdx >= nLastEnd, "HintsCheck: ends are unsorted" );

        // 4b) IsLessEnd consistency
        if( pLastEnd )
            CHECK_ERR( CompareSwpHtEnd()( pLastEnd, pHtEnd ), "HintsCheck: IsLastEnd" );

        nLastEnd = nIdx;
        pLastEnd = pHtEnd;

        CHECK_ERR( COMPLETE_STRING != nIdx, "HintsCheck: no GetEndOf" );

        // 7a) character attributes in array?
        sal_uInt16 nWhich = pHt->Which();
        CHECK_ERR( !isCHRATR(nWhich),
                   "HintsCheck: Character attribute in start array" );

        // 7b) character attributes in array?
        nWhich = pHtEnd->Which();
        CHECK_ERR( !isCHRATR(nWhich),
                   "HintsCheck: Character attribute in end array" );

        // 8) style portion check
        const SwTextAttr* pHtThis = m_HintsByStart[i];
        const SwTextAttr* pHtLast = i > 0 ? m_HintsByStart[i-1] : nullptr;
        CHECK_ERR( (0 == i)
            ||  (   (RES_TXTATR_CHARFMT != pHtLast->Which())
                &&  (RES_TXTATR_AUTOFMT != pHtLast->Which()))
            ||  (   (RES_TXTATR_CHARFMT != pHtThis->Which())
                &&  (RES_TXTATR_AUTOFMT != pHtThis->Which()))
            ||  (pHtThis->GetStart() >= *pHtLast->End()) // no overlap
            ||  (   (   (pHtThis->GetStart() == pHtLast->GetStart())
                    &&  (*pHtThis->End()   == *pHtLast->End())
                    ) // same range
                &&  (   (pHtThis->Which() != RES_TXTATR_AUTOFMT)
                    ||  (pHtLast->Which() != RES_TXTATR_AUTOFMT)
                    ) // never two AUTOFMT on same range
                &&  (   (pHtThis->Which() != RES_TXTATR_CHARFMT)
                    ||  (pHtLast->Which() != RES_TXTATR_CHARFMT)
                    ||  (static_txtattr_cast<const SwTextCharFormat *>(pHtThis)
                                ->GetSortNumber() !=
                         static_txtattr_cast<const SwTextCharFormat *>(pHtLast)
                                ->GetSortNumber())
                    ) // multiple CHARFMT on same range need distinct sorter
                )
            ||  (pHtThis->GetStart() == *pHtThis->End()), // this empty
                   "HintsCheck: Portion inconsistency. "
                   "This can be temporarily ok during undo operations" );

        // 8 1/2) format ignore start/end flag check
        // (problems because MergePortions buggy or not called)
        if (bPortionsMerged)
        {
            if (RES_TXTATR_AUTOFMT == pHt->Which() ||
                RES_TXTATR_CHARFMT == pHt->Which())
            {
                // mostly ignore the annoying no-length hints
                // BuildPortions inserts these in the middle of an existing one
                bool const bNoLength(pHt->GetStart() == *pHt->End());
                bool bNeedContinuation(!bNoLength && pHt->IsFormatIgnoreEnd());
                bool bForbidContinuation(!bNoLength && !bNeedContinuation);
                if (RES_TXTATR_AUTOFMT == pHt->Which())
                {
                    if (RsidOnlyAutoFormats.find(pHt) != RsidOnlyAutoFormats.end())
                    {
                        assert(pHt->IsFormatIgnoreStart());
                        bNeedContinuation = false;
                        // don't forbid continuation - may be other hint here!
                    }
                }
                if (bNeedContinuation || bForbidContinuation)
                {
                    bool bFound(false);
                    for (size_t j = i + 1; j < Count(); ++j)
                    {
                        SwTextAttr *const pOther(m_HintsByStart[j]);
                        if (pOther->GetStart() > *pHt->End())
                        {
                            break; // done
                        }
                        else if (pOther->GetStart() == pOther->GetAnyEnd())
                        {
                            continue; // empty hint: ignore
                        }
                        else if (pOther->GetStart() == *pHt->End())
                        {
                            if (RES_TXTATR_AUTOFMT == pOther->Which() ||
                                RES_TXTATR_CHARFMT == pOther->Which())
                            {   // multiple charfmt on same range must all match
                                if (bNeedContinuation)
                                {
                                    assert(pOther->IsFormatIgnoreStart());
                                    bFound = true;
                                }
                                else if (bForbidContinuation &&
                                         (RsidOnlyAutoFormats.find(pOther) ==
                                          RsidOnlyAutoFormats.end()))
                                {
                                    assert(!pOther->IsFormatIgnoreStart());
                                }
                            }
                        }
                    }
                    if (bNeedContinuation)
                    {
                        assert(bFound); // ? can this happen temp. during undo?
                    }
                }
            }
            else
            {
                assert(!pHt->IsFormatIgnoreStart());
                assert(!pHt->IsFormatIgnoreEnd());
            }
        }

        // 9) nesting portion check
        if (pHtThis->IsNesting())
        {
            for (size_t j = 0; j < i; ++j)
            {
                SwTextAttr const * const pOther( m_HintsByStart[j] );
                if (pOther->IsNesting())
                {
                    SwComparePosition cmp = ComparePosition(
                        pHtThis->GetStart(), *pHtThis->End(),
                        pOther->GetStart(), *pOther->End());
                    CHECK_ERR( (SwComparePosition::OverlapBefore != cmp) &&
                               (SwComparePosition::OverlapBehind != cmp),
                        "HintsCheck: overlapping nesting hints!!!" );
                }
            }
        }

        // 10) dummy char check (unfortunately cannot check SwTextNode::m_Text)
        if (pHtThis->HasDummyChar())
        {
            for ( size_t j = 0; j < i; ++j )
            {
                SwTextAttr const * const pOther( m_HintsByStart[j] );
                if (pOther->HasDummyChar())
                {
                    CHECK_ERR( (pOther->GetStart() != pHtThis->GetStart()),
                        "HintsCheck: multiple hints claim same CH_TXTATR!");
                }
            }
        }
    }
    return true;
}

#endif      /* DBG_UTIL */

// Resort() is called before every Insert and Delete.
// Various SwTextNode methods modify hints in a way that violates the
// sort order of the m_HintsByStart, m_HintsByEnd arrays, so this method is needed
// to restore the order.

void SwpHints::Resort() const
{
    ResortStartMap();
    ResortEndMap();
    ResortWhichMap();
}

void SwpHints::ResortStartMap() const
{
    if (m_StartMapNeedsSortingRange.first != SAL_MAX_INT32)
    {
        auto & rStartMap = const_cast<SwpHints*>(this)->m_HintsByStart;
        if (m_StartMapNeedsSortingRange.first == -1)
            std::sort(rStartMap.begin(), rStartMap.end(), CompareSwpHtStart);
        else
        {
            // only need to sort a partial range of the array
            auto it1 = std::lower_bound(rStartMap.begin(), rStartMap.end(), m_StartMapNeedsSortingRange.first, CompareSwpHtStartOnly());
            auto it2 = std::upper_bound(rStartMap.begin(), rStartMap.end(), m_StartMapNeedsSortingRange.second, CompareSwpHtStartOnly());
            std::sort(rStartMap.begin() + std::distance(rStartMap.begin(), it1),
                      rStartMap.begin() + std::distance(rStartMap.begin(), it2), CompareSwpHtStart);
        }
        m_StartMapNeedsSortingRange = { SAL_MAX_INT32, -1 };
    }
}

void SwpHints::ResortEndMap() const
{
    if (m_EndMapNeedsSortingRange.first != SAL_MAX_INT32)
    {
        auto & rEndMap = const_cast<SwpHints*>(this)->m_HintsByEnd;
        if (m_EndMapNeedsSortingRange.first == -1)
            std::sort(rEndMap.begin(), rEndMap.end(), CompareSwpHtEnd());
        else
        {
            // only need to sort a partial range of the array
            auto it1 = std::lower_bound(rEndMap.begin(), rEndMap.end(), m_EndMapNeedsSortingRange.first, CompareSwpHtEndOnly());
            auto it2 = std::upper_bound(rEndMap.begin(), rEndMap.end(), m_EndMapNeedsSortingRange.second, CompareSwpHtEndOnly());
            std::sort(rEndMap.begin() + std::distance(rEndMap.begin(), it1),
                      rEndMap.begin() + std::distance(rEndMap.begin(), it2), CompareSwpHtEnd());
        }
        m_EndMapNeedsSortingRange = { SAL_MAX_INT32, -1 };
    }
}

void SwpHints::ResortWhichMap() const
{
    if (m_WhichMapNeedsSortingRange.first.first != SAL_MAX_INT32)
    {
        auto & rWhichStartMap = const_cast<SwpHints*>(this)->m_HintsByWhichAndStart;
        if (m_WhichMapNeedsSortingRange.first.first == -1)
            std::sort(rWhichStartMap.begin(), rWhichStartMap.end(), CompareSwpHtWhichStart());
        else
        {
            // only need to sort a partial range of the array
            auto it1 = std::lower_bound(rWhichStartMap.begin(), rWhichStartMap.end(), m_WhichMapNeedsSortingRange.first, CompareSwpHtWhichStart());
            auto it2 = std::upper_bound(rWhichStartMap.begin(), rWhichStartMap.end(), m_WhichMapNeedsSortingRange.second, CompareSwpHtWhichStart());
            std::sort(rWhichStartMap.begin() + std::distance(rWhichStartMap.begin(), it1),
                      rWhichStartMap.begin() + std::distance(rWhichStartMap.begin(), it2), CompareSwpHtWhichStart());
        }
        m_WhichMapNeedsSortingRange = { { SAL_MAX_INT32, -1 }, { -1, -1 } };
    }
}

size_t SwpHints::GetFirstPosSortedByWhichAndStart( sal_uInt16 nWhich ) const
{
    if (m_WhichMapNeedsSortingRange.first.first != SAL_MAX_INT32)
        ResortWhichMap();
    auto it = std::lower_bound(m_HintsByWhichAndStart.begin(), m_HintsByWhichAndStart.end(), nWhich, CompareSwpHtWhichStart());
    if ( it == m_HintsByWhichAndStart.end() )
        return SAL_MAX_SIZE;
    return it - m_HintsByWhichAndStart.begin();
}

int SwpHints::GetLastPosSortedByEnd( sal_Int32 nEndPos ) const
{
    if (m_EndMapNeedsSortingRange.first != SAL_MAX_INT32)
        ResortEndMap();
    auto it = std::upper_bound(m_HintsByEnd.begin(), m_HintsByEnd.end(), nEndPos, CompareSwpHtEnd());
    return it - m_HintsByEnd.begin() - 1;
}

size_t SwpHints::GetIndexOf( const SwTextAttr *pHt ) const
{
    if (m_StartMapNeedsSortingRange.first != SAL_MAX_INT32)
        ResortStartMap();
    auto it = std::lower_bound(m_HintsByStart.begin(), m_HintsByStart.end(), const_cast<SwTextAttr*>(pHt), CompareSwpHtStart);
    if ( it == m_HintsByStart.end() || *it != pHt )
        return SAL_MAX_SIZE;
    return it - m_HintsByStart.begin();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
