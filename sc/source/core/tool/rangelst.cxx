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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


//------------------------------------------------------------------------

#define SC_RANGELST_CXX         //fuer ICC

#include <tools/debug.hxx>
#include <stdlib.h>             // qsort
#include <unotools/collatorwrapper.hxx>

#include "rangelst.hxx"
#include "document.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"
#include "compiler.hxx"

using ::std::vector;
using ::std::advance;
using ::std::find_if;
using ::std::for_each;
using ::formula::FormulaGrammar;

namespace {

template<typename T>
class FindEnclosingRange : public ::std::unary_function<bool, ScRange*>
{
public:
    FindEnclosingRange(const T& rTest) : mrTest(rTest) {}
    FindEnclosingRange(const FindEnclosingRange& r) : mrTest(r.mrTest) {}
    bool operator() (const ScRange* pRange) const
    {
        return pRange->In(mrTest);
    }
private:
    const T& mrTest;
};

template<typename T>
class FindIntersectingRange : public ::std::unary_function<bool, ScRange*>
{
public:
    FindIntersectingRange(const T& rTest) : mrTest(rTest) {}
    FindIntersectingRange(const FindIntersectingRange& r) : mrTest(r.mrTest) {}
    bool operator() (const ScRange* pRange) const
    {
        return pRange->Intersects(mrTest);
    }
private:
    const T& mrTest;
};

class AppendToList : public ::std::unary_function<void, const ScRange*>
{
public:
    AppendToList(vector<ScRange*>& rRanges) : mrRanges(rRanges) {}
    AppendToList(const AppendToList& r) : mrRanges(r.mrRanges) {}
    void operator() (const ScRange* p)
    {
        mrRanges.push_back(new ScRange(*p));
    }
private:
    vector<ScRange*>& mrRanges;
};

template<typename T>
struct DeleteObject : public ::std::unary_function<void, T*>
{
    void operator() (T* p)
    {
        delete p;
    }
};

class CountCells : public ::std::unary_function<void, const ScRange*>
{
public:
    CountCells() : mnCellCount(0) {}
    CountCells(const CountCells& r) : mnCellCount(r.mnCellCount) {}

    void operator() (const ScRange* p)
    {
        mnCellCount +=
              size_t(p->aEnd.Col() - p->aStart.Col() + 1)
            * size_t(p->aEnd.Row() - p->aStart.Row() + 1)
            * size_t(p->aEnd.Tab() - p->aStart.Tab() + 1);
    }

    size_t getCellCount() const { return mnCellCount; }

private:
    size_t mnCellCount;
};

class FormatString : public ::std::unary_function<void, const ScRange*>
{
public:
    FormatString(String& rStr, sal_uInt16 nFlags, ScDocument* pDoc, FormulaGrammar::AddressConvention eConv, sal_Unicode cDelim) :
        mrStr(rStr),
        mnFlags(nFlags),
        mpDoc(pDoc),
        meConv(eConv),
        mcDelim(cDelim),
        mbFirst(true) {}

    FormatString(const FormatString& r) :
        mrStr(r.mrStr),
        mnFlags(r.mnFlags),
        mpDoc(r.mpDoc),
        meConv(r.meConv),
        mcDelim(r.mcDelim),
        mbFirst(r.mbFirst) {}

    void operator() (const ScRange* p)
    {
        String aStr;
        p->Format(aStr, mnFlags, mpDoc, meConv);
        if (mbFirst)
            mbFirst = false;
        else
            mrStr += mcDelim;
        mrStr += aStr;
    }
private:
    String& mrStr;
    sal_uInt16 mnFlags;
    ScDocument* mpDoc;
    FormulaGrammar::AddressConvention meConv;
    sal_Unicode mcDelim;
    bool mbFirst;
};

}

// === ScRangeList ====================================================

ScRangeList::~ScRangeList()
{
    RemoveAll();
}

sal_uInt16 ScRangeList::Parse( const String& rStr, ScDocument* pDoc, sal_uInt16 nMask,
                           formula::FormulaGrammar::AddressConvention eConv,
                           sal_Unicode cDelimiter )
{
    if ( rStr.Len() )
    {
        if (!cDelimiter)
            cDelimiter = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);

        nMask |= SCA_VALID;             // falls das jemand vergessen sollte
        sal_uInt16 nResult = (sal_uInt16)~0;    // alle Bits setzen
        ScRange aRange;
        String aOne;
        SCTAB nTab = 0;
        if ( pDoc )
        {
            //! erste markierte Tabelle gibts nicht mehr am Dokument
            //! -> uebergeben? oder spaeter an den Ranges setzen
        }
        else
            nTab = 0;
        sal_uInt16 nTCount = rStr.GetTokenCount( cDelimiter );
        for ( sal_uInt16 i=0; i<nTCount; i++ )
        {
            aOne = rStr.GetToken( i, cDelimiter );
            aRange.aStart.SetTab( nTab );   // Default Tab wenn nicht angegeben
            sal_uInt16 nRes = aRange.ParseAny( aOne, pDoc, eConv );
            sal_uInt16 nEndRangeBits = SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2;
            sal_uInt16 nTmp1 = ( nRes & SCA_BITS );
            sal_uInt16 nTmp2 = ( nRes & nEndRangeBits );
            // If we have a valid single range with
            // any of the address bits we are interested in
            // set - set the equiv end range bits
            if ( (nRes & SCA_VALID ) && nTmp1 && ( nTmp2 != nEndRangeBits ) )
                    nRes |= ( nTmp1 << 4 );

            if ( (nRes & nMask) == nMask )
                Append( aRange );
            nResult &= nRes;        // alle gemeinsamen Bits bleiben erhalten
        }
        return nResult;             // SCA_VALID gesetzt wenn alle ok
    }
    else
        return 0;
}


void ScRangeList::Format( String& rStr, sal_uInt16 nFlags, ScDocument* pDoc,
                          formula::FormulaGrammar::AddressConvention eConv,
                          sal_Unicode cDelimiter ) const
{
    rStr.Erase();

    if (!cDelimiter)
        cDelimiter = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);

    FormatString func(rStr, nFlags, pDoc, eConv, cDelimiter);
    for_each(maRanges.begin(), maRanges.end(), func);
}


void ScRangeList::Join( const ScRange& r, bool bIsInList )
{
    if ( maRanges.empty() )
    {
        Append( r );
        return ;
    }
    SCCOL nCol1 = r.aStart.Col();
    SCROW nRow1 = r.aStart.Row();
    SCTAB nTab1 = r.aStart.Tab();
    SCCOL nCol2 = r.aEnd.Col();
    SCROW nRow2 = r.aEnd.Row();
    SCTAB nTab2 = r.aEnd.Tab();

    ScRange* pOver = (ScRange*) &r;     // fies aber wahr wenn bInList
    size_t nOldPos = 0;
    if ( bIsInList )
    {
        // Find the current position of this range.
        for ( size_t i = 0, nRanges = maRanges.size(); i < nRanges; ++i )
        {
            if ( maRanges[i] == pOver )
            {
                nOldPos = i;
                break;
            }
        }
    }
    bool bJoinedInput = false;

    for ( size_t i = 0, nRanges = maRanges.size(); i < nRanges && pOver; ++i )
    {
        ScRange* p = maRanges[i];
        if ( p == pOver )
            continue;           // derselbe, weiter mit dem naechsten
        bool bJoined = false;
        if ( p->In( r ) )
        {   // Range r in Range p enthalten oder identisch
            if ( bIsInList )
                bJoined = true;     // weg mit Range r
            else
            {   // das war's dann
                bJoinedInput = true;    // nicht anhaengen
                break;  // for
            }
        }
        else if ( r.In( *p ) )
        {   // Range p in Range r enthalten, r zum neuen Range machen
            *p = r;
            bJoined = true;
        }
        if ( !bJoined && p->aStart.Tab() == nTab1 && p->aEnd.Tab() == nTab2 )
        {   // 2D
            if ( p->aStart.Col() == nCol1 && p->aEnd.Col() == nCol2 )
            {
                if ( p->aStart.Row() == nRow2+1 )
                {   // oben
                    p->aStart.SetRow( nRow1 );
                    bJoined = true;
                }
                else if ( p->aEnd.Row() == nRow1-1 )
                {   // unten
                    p->aEnd.SetRow( nRow2 );
                    bJoined = true;
                }
            }
            else if ( p->aStart.Row() == nRow1 && p->aEnd.Row() == nRow2 )
            {
                if ( p->aStart.Col() == nCol2+1 )
                {   // links
                    p->aStart.SetCol( nCol1 );
                    bJoined = true;
                }
                else if ( p->aEnd.Col() == nCol1-1 )
                {   // rechts
                    p->aEnd.SetCol( nCol2 );
                    bJoined = true;
                }
            }
        }
        if ( bJoined )
        {
            if ( bIsInList )
            {   // innerhalb der Liste Range loeschen
                Remove(nOldPos);
                delete pOver;
                pOver = NULL;
                if ( nOldPos )
                    nOldPos--;          // Seek richtig aufsetzen
            }
            bJoinedInput = true;
            Join( *p, true );           // rekursiv!
        }
    }
    if (  !bIsInList && !bJoinedInput )
        Append( r );
}


bool ScRangeList::operator==( const ScRangeList& r ) const
{
    if ( this == &r )
        return true;

    if (maRanges.size() != r.maRanges.size())
        return false;

    vector<ScRange*>::const_iterator itr1 = maRanges.begin(), itrEnd = maRanges.end();
    vector<ScRange*>::const_iterator itr2 = r.maRanges.begin();
    for (; itr1 != itrEnd; ++itr1, ++itr2)
    {
        const ScRange* p1 = *itr1;
        const ScRange* p2 = *itr2;
        if (*p1 != *p2)
            return false;
    }
    return true;
}

bool ScRangeList::operator!=( const ScRangeList& r ) const
{
    return !operator==( r );
}

bool ScRangeList::UpdateReference(
    UpdateRefMode eUpdateRefMode,
    ScDocument* pDoc,
    const ScRange& rWhere,
    SCsCOL nDx,
    SCsROW nDy,
    SCsTAB nDz
)
{
    if (maRanges.empty())
        // No ranges to update.  Bail out.
        return false;

    bool bChanged = false;
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );

    vector<ScRange*>::iterator itr = maRanges.begin(), itrEnd = maRanges.end();
    for (; itr != itrEnd; ++itr)
    {
        ScRange* pR = *itr;
        SCCOL theCol1;
        SCROW theRow1;
        SCTAB theTab1;
        SCCOL theCol2;
        SCROW theRow2;
        SCTAB theTab2;
        pR->GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 );
        if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
                nDx, nDy, nDz,
                theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 )
                != UR_NOTHING )
        {
            bChanged = true;
            pR->aStart.Set( theCol1, theRow1, theTab1 );
            pR->aEnd.Set( theCol2, theRow2, theTab2 );
        }
    }
    return bChanged;
}

const ScRange* ScRangeList::Find( const ScAddress& rAdr ) const
{
    vector<ScRange*>::const_iterator itr = find_if(
        maRanges.begin(), maRanges.end(), FindEnclosingRange<ScAddress>(rAdr));
    return itr == maRanges.end() ? NULL : *itr;
}

ScRange* ScRangeList::Find( const ScAddress& rAdr )
{
    vector<ScRange*>::iterator itr = find_if(
        maRanges.begin(), maRanges.end(), FindEnclosingRange<ScAddress>(rAdr));
    return itr == maRanges.end() ? NULL : *itr;
}

ScRangeList::ScRangeList( const ScRangeList& rList ) :
    SvRefBase()
{
    maRanges.reserve(rList.maRanges.size());
    for_each(rList.maRanges.begin(), rList.maRanges.end(), AppendToList(maRanges));
}

ScRangeList& ScRangeList::operator=(const ScRangeList& rList)
{
    RemoveAll();
    maRanges.reserve(rList.maRanges.size());
    for_each(rList.maRanges.begin(), rList.maRanges.end(), AppendToList(maRanges));
    return *this;
}

bool ScRangeList::Intersects( const ScRange& rRange ) const
{
    vector<ScRange*>::const_iterator itrEnd = maRanges.end();
    vector<ScRange*>::const_iterator itr =
        find_if(maRanges.begin(), itrEnd, FindIntersectingRange<ScRange>(rRange));
    return itr != itrEnd;
}

bool ScRangeList::In( const ScRange& rRange ) const
{
    vector<ScRange*>::const_iterator itrEnd = maRanges.end();
    vector<ScRange*>::const_iterator itr =
        find_if(maRanges.begin(), itrEnd, FindEnclosingRange<ScRange>(rRange));
    return itr != itrEnd;
}


size_t ScRangeList::GetCellCount() const
{
    CountCells func;
    return for_each(maRanges.begin(), maRanges.end(), func).getCellCount();
}

ScRange* ScRangeList::Remove(size_t nPos)
{
    if (maRanges.size() <= nPos)
        // Out-of-bound condition.  Bail out.
        return NULL;

    vector<ScRange*>::iterator itr = maRanges.begin();
    advance(itr, nPos);
    ScRange* p = *itr;
    maRanges.erase(itr);
    return p;
}

void ScRangeList::RemoveAll()
{
    for_each(maRanges.begin(), maRanges.end(), DeleteObject<ScRange>());
    maRanges.clear();
}

bool ScRangeList::empty() const
{
    return maRanges.empty();
}

size_t ScRangeList::size() const
{
    return maRanges.size();
}

ScRange* ScRangeList::operator [](size_t idx)
{
    return maRanges[idx];
}

const ScRange* ScRangeList::operator [](size_t idx) const
{
    return maRanges[idx];
}

ScRange* ScRangeList::front()
{
    return maRanges.front();
}

const ScRange* ScRangeList::front() const
{
    return maRanges.front();
}

ScRange* ScRangeList::back()
{
    return maRanges.back();
}

const ScRange* ScRangeList::back() const
{
    return maRanges.back();
}

void ScRangeList::push_back(ScRange* p)
{
    maRanges.push_back(p);
}

// === ScRangePairList ========================================================

ScRangePairList::~ScRangePairList()
{
    for_each( maPairs.begin(), maPairs.end(), DeleteObject<ScRangePair>() );
    maPairs.clear();
}

//-----------------------------------------------------------------------------
ScRangePair* ScRangePairList::Remove(size_t nPos)
{
    if (maPairs.size() <= nPos)
        // Out-of-bound condition.  Bail out.
        return NULL;

    vector<ScRangePair*>::iterator itr = maPairs.begin();
    advance(itr, nPos);
    ScRangePair* p = *itr;
    maPairs.erase(itr);
    return p;
}

//-----------------------------------------------------------------------------
ScRangePair* ScRangePairList::Remove( ScRangePair* Adr)
{
    ScRangePair* p = NULL;

    if (Adr == NULL) return NULL;

    for ( vector<ScRangePair*>::iterator itr = maPairs.begin(); itr < maPairs.end(); ++itr )
    {
        if ( Adr == (p = *itr) )
        {
            maPairs.erase( itr );
            break;
        }
    }
    return p;
}

bool ScRangePairList::operator==( const ScRangePairList& r ) const
{
    if ( this == &r )
        return true;                // identische Referenz
    if ( maPairs.size() != r.size() )
        return false;
    for ( size_t nIdx = 0, nCnt = maPairs.size(); nIdx < nCnt; ++nIdx )
    {
        if ( *maPairs[ nIdx ] != *r[ nIdx ] )
            return false;           // auch andere Reihenfolge ist ungleich
    }
    return true;
}

ScRangePair* ScRangePairList::operator [](size_t idx)
{
    return maPairs[idx];
}

const ScRangePair* ScRangePairList::operator [](size_t idx) const
{
    return maPairs[idx];
}

size_t ScRangePairList::size() const
{
    return maPairs.size();
}

bool ScRangePairList::UpdateReference( UpdateRefMode eUpdateRefMode,
                                    ScDocument* pDoc, const ScRange& rWhere,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    bool bChanged = false;
    if ( !maPairs.empty() )
    {
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
        rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        for ( size_t i = 0, nPairs = maPairs.size(); i < nPairs; ++i )
        {
            ScRangePair* pR = maPairs[ i ];
            for ( sal_uInt16 j=0; j<2; j++ )
            {
                ScRange& rRange = pR->GetRange(j);
                SCCOL theCol1;
                SCROW theRow1;
                SCTAB theTab1;
                SCCOL theCol2;
                SCROW theRow2;
                SCTAB theTab2;
                rRange.GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 );
                if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                        nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
                        nDx, nDy, nDz,
                        theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 )
                        != UR_NOTHING )
                {
                    bChanged = true;
                    rRange.aStart.Set( theCol1, theRow1, theTab1 );
                    rRange.aEnd.Set( theCol2, theRow2, theTab2 );
                }
            }
        }
    }
    return bChanged;
}

//-----------------------------------------------------------------------------
// Delete entries that have the labels (first range) on nTab
void ScRangePairList::DeleteOnTab( SCTAB nTab )
{
    size_t nListCount = maPairs.size();
    size_t nPos = 0;
    while ( nPos < nListCount )
    {
        ScRangePair* pR = maPairs[  nPos ];
        ScRange aRange = pR->GetRange(0);
        if ( aRange.aStart.Tab() == nTab && aRange.aEnd.Tab() == nTab )
        {
            Remove( nPos );
            delete pR;
            nListCount = maPairs.size();
        }
        else
            ++nPos;
    }
}

//-----------------------------------------------------------------------------
ScRangePair* ScRangePairList::Find( const ScAddress& rAdr ) const
{
    for ( size_t j = 0, nListCount = maPairs.size(); j < nListCount; j++ )
    {
        ScRangePair* pR = maPairs[ j ];
        if ( pR->GetRange(0).In( rAdr ) )
            return pR;
    }
    return NULL;
}

//-----------------------------------------------------------------------------
ScRangePair* ScRangePairList::Find( const ScRange& rRange ) const
{
    for ( size_t j = 0, nListCount = maPairs.size(); j < nListCount; j++ )
    {
        ScRangePair* pR = maPairs[ j ];
        if ( pR->GetRange(0) == rRange )
            return pR;
    }
    return NULL;
}


//-----------------------------------------------------------------------------
ScRangePairList* ScRangePairList::Clone() const
{
    ScRangePairList* pNew = new ScRangePairList;
    for ( size_t j = 0, nListCount = maPairs.size(); j < nListCount; j++ )
    {
        pNew->Append( *maPairs[ j ] );
    }
    return pNew;
}

//-----------------------------------------------------------------------------
struct ScRangePairNameSort
{
    ScRangePair*    pPair;
    ScDocument*     pDoc;
};

//-----------------------------------------------------------------------------
extern "C" int
#ifdef WNT
__cdecl
#endif
ScRangePairList_QsortNameCompare( const void* p1, const void* p2 )
{
    const ScRangePairNameSort* ps1 = (const ScRangePairNameSort*)p1;
    const ScRangePairNameSort* ps2 = (const ScRangePairNameSort*)p2;
    const ScAddress& rStartPos1 = ps1->pPair->GetRange(0).aStart;
    const ScAddress& rStartPos2 = ps2->pPair->GetRange(0).aStart;
    String aStr1, aStr2;
    sal_Int32 nComp;
    if ( rStartPos1.Tab() == rStartPos2.Tab() )
        nComp = COMPARE_EQUAL;
    else
    {
        ps1->pDoc->GetName( rStartPos1.Tab(), aStr1 );
        ps2->pDoc->GetName( rStartPos2.Tab(), aStr2 );
        nComp = ScGlobal::GetCollator()->compareString( aStr1, aStr2 );
    }
    switch ( nComp )
    {
        case COMPARE_LESS:
            return -1;
        //break;
        case COMPARE_GREATER:
            return 1;
        //break;
        default:
            // gleiche Tabs
            if ( rStartPos1.Col() < rStartPos2.Col() )
                return -1;
            if ( rStartPos1.Col() > rStartPos2.Col() )
                return 1;
            // gleiche Cols
            if ( rStartPos1.Row() < rStartPos2.Row() )
                return -1;
            if ( rStartPos1.Row() > rStartPos2.Row() )
                return 1;
            // erste Ecke gleich, zweite Ecke
            {
                const ScAddress& rEndPos1 = ps1->pPair->GetRange(0).aEnd;
                const ScAddress& rEndPos2 = ps2->pPair->GetRange(0).aEnd;
                if ( rEndPos1.Tab() == rEndPos2.Tab() )
                    nComp = COMPARE_EQUAL;
                else
                {
                    ps1->pDoc->GetName( rEndPos1.Tab(), aStr1 );
                    ps2->pDoc->GetName( rEndPos2.Tab(), aStr2 );
                    nComp = ScGlobal::GetCollator()->compareString( aStr1, aStr2 );
                }
                switch ( nComp )
                {
                    case COMPARE_LESS:
                        return -1;
                    //break;
                    case COMPARE_GREATER:
                        return 1;
                    //break;
                    default:
                        // gleiche Tabs
                        if ( rEndPos1.Col() < rEndPos2.Col() )
                            return -1;
                        if ( rEndPos1.Col() > rEndPos2.Col() )
                            return 1;
                        // gleiche Cols
                        if ( rEndPos1.Row() < rEndPos2.Row() )
                            return -1;
                        if ( rEndPos1.Row() > rEndPos2.Row() )
                            return 1;
                        return 0;
                }
            }
            return 0;
    }
    return 0; // just in case
}

//-----------------------------------------------------------------------------
void ScRangePairList::Join( const ScRangePair& r, bool bIsInList )
{
    if ( maPairs.empty() )
    {
        Append( r );
        return ;
    }
    const ScRange& r1 = r.GetRange(0);
    const ScRange& r2 = r.GetRange(1);
    SCCOL nCol1 = r1.aStart.Col();
    SCROW nRow1 = r1.aStart.Row();
    SCTAB nTab1 = r1.aStart.Tab();
    SCCOL nCol2 = r1.aEnd.Col();
    SCROW nRow2 = r1.aEnd.Row();
    SCTAB nTab2 = r1.aEnd.Tab();
    ScRangePair* pOver = (ScRangePair*) &r;     // fies aber wahr wenn bInList
    size_t nOldPos = 0;
    if ( bIsInList )
    {
        // Find the current position of this range.
        for ( size_t i = 0, nPairs = maPairs.size(); i < nPairs; ++i )
        {
            if ( maPairs[i] == pOver )
            {
                nOldPos = i;
                break;
            }
        }
    }
    bool bJoinedInput = false;

    for ( size_t i = 0, nPairs = maPairs.size(); i < nPairs && pOver; ++i )
    {
        ScRangePair* p = maPairs[ i ];
        if ( p == pOver )
            continue;           // derselbe, weiter mit dem naechsten
        bool bJoined = false;
        ScRange& rp1 = p->GetRange(0);
        ScRange& rp2 = p->GetRange(1);
        if ( rp2 == r2 )
        {   // nur wenn Range2 gleich ist
            if ( rp1.In( r1 ) )
            {   // RangePair r in RangePair p enthalten oder identisch
                if ( bIsInList )
                    bJoined = true;     // weg mit RangePair r
                else
                {   // das war's dann
                    bJoinedInput = true;    // nicht anhaengen
                    break;  // for
                }
            }
            else if ( r1.In( rp1 ) )
            {   // RangePair p in RangePair r enthalten, r zum neuen RangePair machen
                *p = r;
                bJoined = true;
            }
        }
        if ( !bJoined && rp1.aStart.Tab() == nTab1 && rp1.aEnd.Tab() == nTab2
          && rp2.aStart.Tab() == r2.aStart.Tab()
          && rp2.aEnd.Tab() == r2.aEnd.Tab() )
        {   // 2D, Range2 muss genauso nebeneinander liegen wie Range1
            if ( rp1.aStart.Col() == nCol1 && rp1.aEnd.Col() == nCol2
              && rp2.aStart.Col() == r2.aStart.Col()
              && rp2.aEnd.Col() == r2.aEnd.Col() )
            {
                if ( rp1.aStart.Row() == nRow2+1
                  && rp2.aStart.Row() == r2.aEnd.Row()+1 )
                {   // oben
                    rp1.aStart.SetRow( nRow1 );
                    rp2.aStart.SetRow( r2.aStart.Row() );
                    bJoined = true;
                }
                else if ( rp1.aEnd.Row() == nRow1-1
                  && rp2.aEnd.Row() == r2.aStart.Row()-1 )
                {   // unten
                    rp1.aEnd.SetRow( nRow2 );
                    rp2.aEnd.SetRow( r2.aEnd.Row() );
                    bJoined = true;
                }
            }
            else if ( rp1.aStart.Row() == nRow1 && rp1.aEnd.Row() == nRow2
              && rp2.aStart.Row() == r2.aStart.Row()
              && rp2.aEnd.Row() == r2.aEnd.Row() )
            {
                if ( rp1.aStart.Col() == nCol2+1
                  && rp2.aStart.Col() == r2.aEnd.Col()+1 )
                {   // links
                    rp1.aStart.SetCol( nCol1 );
                    rp2.aStart.SetCol( r2.aStart.Col() );
                    bJoined = true;
                }
                else if ( rp1.aEnd.Col() == nCol1-1
                  && rp2.aEnd.Col() == r2.aEnd.Col()-1 )
                {   // rechts
                    rp1.aEnd.SetCol( nCol2 );
                    rp2.aEnd.SetCol( r2.aEnd.Col() );
                    bJoined = true;
                }
            }
        }
        if ( bJoined )
        {
            if ( bIsInList )
            {   // innerhalb der Liste RangePair loeschen
                Remove( nOldPos );
                delete pOver;
                pOver = NULL;
                if ( nOldPos )
                    nOldPos--;          // Seek richtig aufsetzen
            }
            bJoinedInput = true;
            Join( *p, true );           // rekursiv!
        }
    }
    if ( !bIsInList && !bJoinedInput )
        Append( r );
}

//-----------------------------------------------------------------------------
ScRangePair** ScRangePairList::CreateNameSortedArray( size_t& nListCount,
        ScDocument* pDoc ) const
{
    nListCount = maPairs.size();
    DBG_ASSERT( nListCount * sizeof(ScRangePairNameSort) <= (size_t)~0x1F,
        "ScRangePairList::CreateNameSortedArray nListCount * sizeof(ScRangePairNameSort) > (size_t)~0x1F" );
    ScRangePairNameSort* pSortArray = (ScRangePairNameSort*)
        new sal_uInt8 [ nListCount * sizeof(ScRangePairNameSort) ];
    sal_uLong j;
    for ( j=0; j < nListCount; j++ )
    {
        pSortArray[j].pPair = maPairs[ j ];
        pSortArray[j].pDoc = pDoc;
    }
#if !(defined(ICC ) && defined(OS2))
    qsort( (void*)pSortArray, nListCount, sizeof(ScRangePairNameSort), &ScRangePairList_QsortNameCompare );
#else
    qsort( (void*)pSortArray, nListCount, sizeof(ScRangePairNameSort), ICCQsortRPairCompare );
#endif
    // ScRangePair Pointer aufruecken
    ScRangePair** ppSortArray = (ScRangePair**)pSortArray;
    for ( j=0; j < nListCount; j++ )
    {
        ppSortArray[j] = pSortArray[j].pPair;
    }
    return ppSortArray;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
