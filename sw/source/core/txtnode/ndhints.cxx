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

#include "txatbase.hxx"
#include "ndhints.hxx"
#include <txtatr.hxx>

#ifdef DBG_UTIL
#include <pam.hxx>
#include <fmtautofmt.hxx>
#include <set>
#endif

// Sortierreihenfolge: Start, Ende (umgekehrt!), Which-Wert (umgekehrt!),
//                     als letztes die Adresse selbst

static bool lcl_IsLessStart( const SwTextAttr &rHt1, const SwTextAttr &rHt2 )
{
    if ( rHt1.GetStart() == rHt2.GetStart() )
    {
        const sal_Int32 nHt1 = *rHt1.GetAnyEnd();
        const sal_Int32 nHt2 = *rHt2.GetAnyEnd();
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

// Zuerst nach Ende danach nach Ptr
static bool lcl_IsLessEnd( const SwTextAttr &rHt1, const SwTextAttr &rHt2 )
{
    const sal_Int32 nHt1 = *rHt1.GetAnyEnd();
    const sal_Int32 nHt2 = *rHt2.GetAnyEnd();
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

bool CompareSwpHtStart::operator()(SwTextAttr* const lhs, SwTextAttr* const rhs) const
{
  return lcl_IsLessStart( *lhs, *rhs );
}

bool CompareSwpHtEnd::operator()(SwTextAttr* const lhs, SwTextAttr* const rhs) const
{
  return lcl_IsLessEnd( *lhs, *rhs );
}

void SwpHints::Insert( const SwTextAttr *pHt )
{
    Resort();
    assert(m_HintsByStart.find(const_cast<SwTextAttr*>(pHt))
            == m_HintsByStart.end()); // "Insert: hint already in HtStart"
    assert(m_HintsByEnd.find(const_cast<SwTextAttr*>(pHt))
            == m_HintsByEnd.end());   // "Insert: hint already in HtEnd"
    m_HintsByStart.insert( const_cast<SwTextAttr*>(pHt) );
    m_HintsByEnd  .insert( const_cast<SwTextAttr*>(pHt) );
}

bool SwpHints::Contains( const SwTextAttr *pHt ) const
{
    // DO NOT use find() or CHECK here!
    // if called from SwTextNode::InsertItem, pHt has already been deleted,
    // so it cannot be dereferenced
    for (size_t i = 0; i < m_HintsByStart.size(); ++i)
    {
        if (m_HintsByStart[i] == pHt)
        {
            return true;
        }
    }
    return false;
}

#ifdef DBG_UTIL

#define CHECK_ERR(cond, text) \
        if(!(cond)) \
        { \
            SAL_WARN("sw.core", text); \
            (const_cast<SwpHints*>(this))->Resort(); \
            return false; \
        }

bool SwpHints::Check(bool bPortionsMerged) const
{
    // 1) gleiche Anzahl in beiden Arrays
    CHECK_ERR( m_HintsByStart.size() == m_HintsByEnd.size(),
        "HintsCheck: wrong sizes" );
    sal_Int32 nLastStart = 0;
    sal_Int32 nLastEnd   = 0;

    const SwTextAttr *pLastStart = nullptr;
    const SwTextAttr *pLastEnd = nullptr;
    std::set<SwTextAttr const*> RsidOnlyAutoFormats;
    if (bPortionsMerged)
    {
        for (size_t i = 0; i < Count(); ++i)
        {
            SwTextAttr const*const pHint(m_HintsByStart[i]);
            if (RES_TXTATR_AUTOFMT == pHint->Which())
            {
                std::shared_ptr<SfxItemSet> const pSet(
                        pHint->GetAutoFormat().GetStyleHandle());
                if (pSet->Count() == 1 && pSet->GetItem(RES_CHRATR_RSID, false))
                {
                    RsidOnlyAutoFormats.insert(pHint);
                }
            }
        }
    }

    for( size_t i = 0; i < Count(); ++i )
    {
        // --- Start-Kontrolle ---

        // 2a) gueltiger Pointer? vgl. DELETEFF
        const SwTextAttr *pHt = m_HintsByStart[i];
        CHECK_ERR( 0xFF != *reinterpret_cast<unsigned char const *>(pHt), "HintsCheck: start ptr was deleted" );

        // 3a) Stimmt die Start-Sortierung?
        sal_Int32 nIdx = pHt->GetStart();
        CHECK_ERR( nIdx >= nLastStart, "HintsCheck: starts are unsorted" );

        // 4a) IsLessStart-Konsistenz
        if( pLastStart )
            CHECK_ERR( lcl_IsLessStart( *pLastStart, *pHt ), "HintsCheck: IsLastStart" );

        nLastStart = nIdx;
        pLastStart = pHt;

        // --- End-Kontrolle ---

        // 2b) gueltiger Pointer? vgl. DELETEFF
        const SwTextAttr *pHtEnd = m_HintsByEnd[i];
        CHECK_ERR( 0xFF != *reinterpret_cast<unsigned char const *>(pHtEnd), "HintsCheck: end ptr was deleted" );

        // 3b) Stimmt die End-Sortierung?
        nIdx = *pHtEnd->GetAnyEnd();
        CHECK_ERR( nIdx >= nLastEnd, "HintsCheck: ends are unsorted" );
        nLastEnd = nIdx;

        // 4b) IsLessEnd-Konsistenz
        if( pLastEnd )
            CHECK_ERR( lcl_IsLessEnd( *pLastEnd, *pHtEnd ), "HintsCheck: IsLastEnd" );

        nLastEnd = nIdx;
        pLastEnd = pHtEnd;

        // --- Ueberkreuzungen ---

        // 5) gleiche Pointer in beiden Arrays
        if (m_HintsByStart.find(const_cast<SwTextAttr*>(pHt)) == m_HintsByStart.end())
            nIdx = COMPLETE_STRING;

        CHECK_ERR( COMPLETE_STRING != nIdx, "HintsCheck: no GetStartOf" );

        // 6) gleiche Pointer in beiden Arrays
        if (m_HintsByEnd.find(const_cast<SwTextAttr*>(pHt)) == m_HintsByEnd.end())
            nIdx = COMPLETE_STRING;

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
                    ) // multiple CHARFMT on same range need distinct sortnr
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
                        else if (pOther->GetStart() == *pOther->GetAnyEnd())
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
            for ( size_t j = 0; j < Count(); ++j )
            {
                SwTextAttr const * const pOther( m_HintsByStart[j] );
                if ( pOther->IsNesting() &&  (i != j) )
                {
                    SwComparePosition cmp = ComparePosition(
                        pHtThis->GetStart(), *pHtThis->End(),
                        pOther->GetStart(), *pOther->End());
                    CHECK_ERR( (POS_OVERLAP_BEFORE != cmp) &&
                               (POS_OVERLAP_BEHIND != cmp),
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

void SwpHints::Resort()
{
    m_HintsByStart.Resort();
    m_HintsByEnd.Resort();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
