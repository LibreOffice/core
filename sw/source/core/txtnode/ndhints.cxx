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

inline void DumpHints(const SwpHtStart &, const SwpHtEnd &) { }

/*************************************************************************
 *                      IsLessStart()
 *************************************************************************/

// Sortierreihenfolge: Start, Ende (umgekehrt!), Which-Wert (umgekehrt!),
//                     als letztes die Adresse selbst

static bool lcl_IsLessStart( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
{
    if ( *rHt1.GetStart() == *rHt2.GetStart() )
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
                    const sal_uInt16 nS1 = static_cast<const SwTxtCharFmt&>(rHt1).GetSortNumber();
                    const sal_uInt16 nS2 = static_cast<const SwTxtCharFmt&>(rHt2).GetSortNumber();
                    if ( nS1 != nS2 ) // robust
                        return nS1 < nS2;
                }

                return (sal_IntPtr)&rHt1 < (sal_IntPtr)&rHt2;
            }
            // order is important! for requirements see hintids.hxx
            return ( nWhich1 > nWhich2 );
        }
        return ( nHt1 > nHt2 );
    }
    return ( *rHt1.GetStart() < *rHt2.GetStart() );
}

/*************************************************************************
 *                      inline IsLessEnd()
 *************************************************************************/

// Zuerst nach Ende danach nach Ptr
static bool lcl_IsLessEnd( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
{
    const sal_Int32 nHt1 = *rHt1.GetAnyEnd();
    const sal_Int32 nHt2 = *rHt2.GetAnyEnd();
    if ( nHt1 == nHt2 )
    {
        if ( *rHt1.GetStart() == *rHt2.GetStart() )
        {
            const sal_uInt16 nWhich1 = rHt1.Which();
            const sal_uInt16 nWhich2 = rHt2.Which();
            if ( nWhich1 == nWhich2 )
            {
                if ( RES_TXTATR_CHARFMT == nWhich1 )
                {
                    const sal_uInt16 nS1 = static_cast<const SwTxtCharFmt&>(rHt1).GetSortNumber();
                    const sal_uInt16 nS2 = static_cast<const SwTxtCharFmt&>(rHt2).GetSortNumber();
                    if ( nS1 != nS2 ) // robust
                        return nS1 > nS2;
                }

                return (sal_IntPtr)&rHt1 > (sal_IntPtr)&rHt2;
            }
            // order is important! for requirements see hintids.hxx
            return ( nWhich1 < nWhich2 );
        }
        else
            return ( *rHt1.GetStart() > *rHt2.GetStart() );
    }
    return ( nHt1 < nHt2 );
}

bool CompareSwpHtStart::operator()(SwTxtAttr* const lhs, SwTxtAttr* const rhs) const
{
  return lcl_IsLessStart( *lhs, *rhs );
}

bool CompareSwpHtEnd::operator()(SwTxtAttr* const lhs, SwTxtAttr* const rhs) const
{
  return lcl_IsLessEnd( *lhs, *rhs );
}

/*************************************************************************
 *                      class SwpHintsArr
 *************************************************************************/

void SwpHintsArray::Insert( const SwTxtAttr *pHt )
{
    Resort();
    assert(m_HintStarts.find(const_cast<SwTxtAttr*>(pHt))
            == m_HintStarts.end()); // "Insert: hint already in HtStart"
    assert(m_HintEnds.find(const_cast<SwTxtAttr*>(pHt))
            == m_HintEnds.end());   // "Insert: hint already in HtEnd"
    m_HintStarts.insert( const_cast<SwTxtAttr*>(pHt) );
    m_HintEnds  .insert( const_cast<SwTxtAttr*>(pHt) );
}

void SwpHintsArray::DeleteAtPos( const sal_uInt16 nPos )
{
    // optimization: nPos is the position in the Starts array
    SwTxtAttr *pHt = m_HintStarts[ nPos ];
    m_HintStarts.erase( m_HintStarts.begin() + nPos );

    Resort();

    bool const done = m_HintEnds.erase(pHt);
    assert(done);
    (void) done; // unused in NDEBUG
}

sal_uInt16 SwpHintsArray::GetPos( const SwTxtAttr *pHt ) const
{
    // DO NOT use find() here!
    // if called from SwTxtNode::InsertItem, pHt has already been deleted,
    // so it cannot be dereferenced
    for (size_t i = 0; i < m_HintStarts.size(); ++i)
    {
        if (m_HintStarts[i] == pHt)
        {
            return i;
        }
    }
    return USHRT_MAX;
}

#ifdef DBG_UTIL

/*************************************************************************
 *                      SwpHintsArray::Check()
 *************************************************************************/

#define CHECK_ERR(cond, text) \
        if(!(cond)) \
        { \
            SAL_WARN("sw.core", text); \
            DumpHints(m_HintStarts, m_HintEnds); \
            (const_cast<SwpHintsArray*>(this))->Resort(); \
            return false; \
        }

bool SwpHintsArray::Check(bool bPortionsMerged) const
{
    // 1) gleiche Anzahl in beiden Arrays
    CHECK_ERR( m_HintStarts.size() == m_HintEnds.size(),
        "HintsCheck: wrong sizes" );
    sal_Int32 nLastStart = 0;
    sal_Int32 nLastEnd   = 0;

    const SwTxtAttr *pLastStart = 0;
    const SwTxtAttr *pLastEnd = 0;
    std::set<SwTxtAttr const*> RsidOnlyAutoFmts;
    if (bPortionsMerged)
    {
        for (sal_uInt16 i = 0; i < Count(); ++i)
        {
            SwTxtAttr const*const pHint(m_HintStarts[i]);
            if (RES_TXTATR_AUTOFMT == pHint->Which())
            {
                boost::shared_ptr<SfxItemSet> const pSet(
                        pHint->GetAutoFmt().GetStyleHandle());
                if (pSet->Count() == 1 && pSet->GetItem(RES_CHRATR_RSID, false))
                {
                    RsidOnlyAutoFmts.insert(pHint);
                }
            }
        }
    }

    for( sal_uInt16 i = 0; i < Count(); ++i )
    {
        // --- Start-Kontrolle ---

        // 2a) gueltiger Pointer? vgl. DELETEFF
        const SwTxtAttr *pHt = m_HintStarts[i];
        CHECK_ERR( 0xFF != *(unsigned char*)pHt, "HintsCheck: start ptr was deleted" );

        // 3a) Stimmt die Start-Sortierung?
        sal_Int32 nIdx = *pHt->GetStart();
        CHECK_ERR( nIdx >= nLastStart, "HintsCheck: starts are unsorted" );

        // 4a) IsLessStart-Konsistenz
        if( pLastStart )
            CHECK_ERR( lcl_IsLessStart( *pLastStart, *pHt ), "HintsCheck: IsLastStart" );

        nLastStart = nIdx;
        pLastStart = pHt;

        // --- End-Kontrolle ---

        // 2b) gueltiger Pointer? vgl. DELETEFF
        const SwTxtAttr *pHtEnd = m_HintEnds[i];
        CHECK_ERR( 0xFF != *(unsigned char*)pHtEnd, "HintsCheck: end ptr was deleted" );

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
        if (m_HintStarts.find(const_cast<SwTxtAttr*>(pHt)) == m_HintStarts.end())
            nIdx = COMPLETE_STRING;

        CHECK_ERR( COMPLETE_STRING != nIdx, "HintsCheck: no GetStartOf" );

        // 6) gleiche Pointer in beiden Arrays
        if (m_HintEnds.find(const_cast<SwTxtAttr*>(pHt)) == m_HintEnds.end())
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
        const SwTxtAttr* pHtThis = m_HintStarts[i];
        const SwTxtAttr* pHtLast = i > 0 ? m_HintStarts[i-1] : 0;
        CHECK_ERR( (0 == i)
            ||  (   (RES_TXTATR_CHARFMT != pHtLast->Which())
                &&  (RES_TXTATR_AUTOFMT != pHtLast->Which()))
            ||  (   (RES_TXTATR_CHARFMT != pHtThis->Which())
                &&  (RES_TXTATR_AUTOFMT != pHtThis->Which()))
            ||  (*pHtThis->GetStart() >= *pHtLast->End()) // no overlap
            ||  (   (   (*pHtThis->GetStart() == *pHtLast->GetStart())
                    &&  (*pHtThis->End()   == *pHtLast->End())
                    ) // same range
                &&  (   (pHtThis->Which() != RES_TXTATR_AUTOFMT)
                    ||  (pHtLast->Which() != RES_TXTATR_AUTOFMT)
                    ) // never two AUTOFMT on same range
                &&  (   (pHtThis->Which() != RES_TXTATR_CHARFMT)
                    ||  (pHtLast->Which() != RES_TXTATR_CHARFMT)
                    ||  (static_cast<const SwTxtCharFmt *>(pHtThis)
                                ->GetSortNumber() !=
                         static_cast<const SwTxtCharFmt *>(pHtLast)
                                ->GetSortNumber())
                    ) // multiple CHARFMT on same range need distinct sortnr
                )
            ||  (*pHtThis->GetStart() == *pHtThis->End()), // this empty
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
                // BuildPortions inserts these in the middle of an exsiting one
                bool const bNoLength(*pHt->GetStart() == *pHt->End());
                bool bNeedContinuation(!bNoLength && pHt->IsFormatIgnoreEnd());
                bool bForbidContinuation(!bNoLength && !bNeedContinuation);
                if (RES_TXTATR_AUTOFMT == pHt->Which())
                {
                    if (RsidOnlyAutoFmts.find(pHt) != RsidOnlyAutoFmts.end())
                    {
                        assert(pHt->IsFormatIgnoreStart());
                        bNeedContinuation = false;
                        // don't forbid continuation - may be other hint here!
                    }
                }
                if (bNeedContinuation || bForbidContinuation)
                {
                    bool bFound(false);
                    for (sal_uInt16 j = i + 1; j < Count(); ++j)
                    {
                        SwTxtAttr *const pOther(m_HintStarts[j]);
                        if (*pOther->GetStart() > *pHt->End())
                        {
                            break; // done
                        }
                        else if (*pOther->GetStart() == *pOther->GetAnyEnd())
                        {
                            continue; // empty hint: ignore
                        }
                        else if (*pOther->GetStart() == *pHt->End())
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
                                         (RsidOnlyAutoFmts.find(pOther) ==
                                          RsidOnlyAutoFmts.end()))
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
            for ( sal_uInt16 j = 0; j < Count(); ++j )
            {
                SwTxtAttr const * const pOther( m_HintStarts[j] );
                if ( pOther->IsNesting() &&  (i != j) )
                {
                    SwComparePosition cmp = ComparePosition(
                        *pHtThis->GetStart(), *pHtThis->End(),
                        *pOther->GetStart(), *pOther->End());
                    CHECK_ERR( (POS_OVERLAP_BEFORE != cmp) &&
                               (POS_OVERLAP_BEHIND != cmp),
                        "HintsCheck: overlapping nesting hints!!!" );
                }
            }
        }

        // 10) dummy char check (unfortunately cannot check SwTxtNode::m_Text)
        if (pHtThis->HasDummyChar())
        {
            for ( sal_uInt16 j = 0; j < i; ++j )
            {
                SwTxtAttr const * const pOther( m_HintStarts[j] );
                if (pOther->HasDummyChar())
                {
                    CHECK_ERR( (*pOther->GetStart() != *pHtThis->GetStart()),
                        "HintsCheck: multiple hints claim same CH_TXTATR!");
                }
            }
        }
    }
    return true;
}

#endif      /* DBG_UTIL */

/*************************************************************************
 *                          SwpHintsArray::Resort()
 *************************************************************************/

// Resort() is called before every Insert and Delete.
// Various SwTxtNode methods modify hints in a way that violates the
// sort order of the m_HintStarts, m_HintEnds arrays, so this method is needed
// to restore the order.

void SwpHintsArray::Resort()
{
    m_HintStarts.Resort();
    m_HintEnds.Resort();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
