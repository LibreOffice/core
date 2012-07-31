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


#include "txatbase.hxx"
#include "ndhints.hxx"
#include <txtatr.hxx>

#ifdef DBG_UTIL
#include <pam.hxx>
#endif


inline void DumpHints(const SwpHtStart &, const SwpHtEnd &) { }

/*************************************************************************
 *                      IsLessStart()
 *************************************************************************/

// Sortierreihenfolge: Start, Ende (umgekehrt!), Which-Wert (umgekehrt!),
//                     als letztes die Adresse selbst

static sal_Bool lcl_IsLessStart( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
{
    if ( *rHt1.GetStart() == *rHt2.GetStart() )
    {
        const xub_StrLen nHt1 = *rHt1.GetAnyEnd();
        const xub_StrLen nHt2 = *rHt2.GetAnyEnd();
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
                    OSL_ENSURE( nS1 != nS2, "AUTOSTYLES: lcl_IsLessStart trouble" );
                    if ( nS1 != nS2 ) // robust
                        return nS1 < nS2;
                }

                return (long)&rHt1 < (long)&rHt2;
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
static sal_Bool lcl_IsLessEnd( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
{
    const xub_StrLen nHt1 = *rHt1.GetAnyEnd();
    const xub_StrLen nHt2 = *rHt2.GetAnyEnd();
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
                    OSL_ENSURE( nS1 != nS2, "AUTOSTYLES: lcl_IsLessEnd trouble" );
                    if ( nS1 != nS2 ) // robust
                        return nS1 > nS2;
                }

                return (long)&rHt1 > (long)&rHt2;
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
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(
        m_HintStarts.find(const_cast<SwTxtAttr*>(pHt)) == m_HintStarts.end(),
            "Insert: hint already in HtStart");
    OSL_ENSURE(
        m_HintEnds.find(const_cast<SwTxtAttr*>(pHt)) == m_HintEnds.end(),
            "Insert: hint already in HtEnd");
#endif
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
            OSL_ENSURE(!this, text); \
            DumpHints(m_HintStarts, m_HintEnds); \
            return !(const_cast<SwpHintsArray*>(this))->Resort(); \
        }

bool SwpHintsArray::Check() const
{
    // 1) gleiche Anzahl in beiden Arrays
    CHECK_ERR( m_HintStarts.size() == m_HintEnds.size(),
        "HintsCheck: wrong sizes" );
    xub_StrLen nLastStart = 0;
    xub_StrLen nLastEnd   = 0;

    const SwTxtAttr *pLastStart = 0;
    const SwTxtAttr *pLastEnd = 0;

    for( sal_uInt16 i = 0; i < Count(); ++i )
    {
        // --- Start-Kontrolle ---

        // 2a) gueltiger Pointer? vgl. DELETEFF
        const SwTxtAttr *pHt = m_HintStarts[i];
        CHECK_ERR( 0xFF != *(unsigned char*)pHt, "HintsCheck: start ptr was deleted" );

        // 3a) Stimmt die Start-Sortierung?
        xub_StrLen nIdx = *pHt->GetStart();
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
            nIdx = STRING_LEN;

        CHECK_ERR( STRING_LEN != nIdx, "HintsCheck: no GetStartOf" );

        // 6) gleiche Pointer in beiden Arrays
        if (m_HintEnds.find(const_cast<SwTxtAttr*>(pHt)) == m_HintEnds.end())
            nIdx = STRING_LEN;

        CHECK_ERR( STRING_LEN != nIdx, "HintsCheck: no GetEndOf" );

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
        CHECK_ERR( 0 == i ||
                    ( RES_TXTATR_CHARFMT != pHtLast->Which() && RES_TXTATR_AUTOFMT != pHtLast->Which() ) ||
                    ( RES_TXTATR_CHARFMT != pHtThis->Which() && RES_TXTATR_AUTOFMT != pHtThis->Which() ) ||
                    ( *pHtThis->GetStart() >= *pHtLast->GetEnd() ) ||
                    (   (   (   (*pHtThis->GetStart() == *pHtLast->GetStart())
                            &&  (*pHtThis->GetEnd()   == *pHtLast->GetEnd())
                            ) // same range
                        ||  (*pHtThis->GetStart() == *pHtThis->GetEnd())
                        )
                    &&  (   (pHtThis->Which() != RES_TXTATR_AUTOFMT)
                        ||  (pHtLast->Which() != RES_TXTATR_AUTOFMT)
                        ) // never two AUTOFMT on same range
                    ),
                   "HintsCheck: Portion inconsistency. "
                   "This can be temporarily ok during undo operations" );

        // 9) nesting portion check
        if (pHtThis->IsNesting())
        {
            for ( sal_uInt16 j = 0; j < Count(); ++j )
            {
                SwTxtAttr const * const pOther( m_HintStarts[j] );
                if ( pOther->IsNesting() &&  (i != j) )
                {
                    SwComparePosition cmp = ComparePosition(
                        *pHtThis->GetStart(), *pHtThis->GetEnd(),
                        *pOther->GetStart(), *pOther->GetEnd());
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

// Resort() wird vor jedem Insert und Delete gerufen.
// Wenn Textmasse geloescht wird, so werden die Indizes in
// ndtxt.cxx angepasst. Leider erfolgt noch keine Neusortierung
// auf gleichen Positionen.

bool SwpHintsArray::Resort()
{
    bool bResort = false;
    const SwTxtAttr *pLast = 0;
    sal_uInt16 i;

    for ( i = 0; i < m_HintStarts.size(); ++i )
    {
        SwTxtAttr *pHt = m_HintStarts[i];
        if( pLast && !lcl_IsLessStart( *pLast, *pHt ) )
        {
            m_HintStarts.erase( m_HintStarts.begin() + i );
            m_HintStarts.insert( pHt );
            pHt = m_HintStarts[i];
            if ( pHt != pLast )
                --i;
            bResort = true;
        }
        pLast = pHt;
    }

    pLast = 0;
    for ( i = 0; i < m_HintEnds.size(); ++i )
    {
        SwTxtAttr *pHt = m_HintEnds[i];
        if( pLast && !lcl_IsLessEnd( *pLast, *pHt ) )
        {
            m_HintEnds.erase( m_HintEnds.begin() + i );
            m_HintEnds.insert( pHt );
            pHt = m_HintEnds[i]; // normalerweise == pLast
            // Wenn die Unordnung etwas groesser ist (24200),
            // muessen wir Position i erneut vergleichen.
            if ( pLast != pHt )
                --i;
            bResort = true;
        }
        pLast = pHt;
    }
    return bResort;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
