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

#include <stdlib.h>
#include <unotools/collatorwrapper.hxx>
#include <osl/diagnose.h>

#include "rangelst.hxx"
#include "document.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"
#include "compiler.hxx"
#include <algorithm>
#include <memory>

using ::std::vector;
using ::std::advance;
using ::std::find_if;
using ::std::for_each;
using ::formula::FormulaGrammar;

namespace {

template<typename T>
class FindEnclosingRange : public ::std::unary_function<ScRange*, bool>
{
public:
    explicit FindEnclosingRange(const T& rTest) : mrTest(rTest) {}
    FindEnclosingRange(const FindEnclosingRange& r) : mrTest(r.mrTest) {}
    bool operator() (const ScRange* pRange) const
    {
        return pRange->In(mrTest);
    }
private:
    const T& mrTest;
};

template<typename T>
class FindRangeIn : public ::std::unary_function<ScRange*, bool>
{
public:
    FindRangeIn(const T& rTest) : mrTest(rTest) {}
    FindRangeIn(const FindRangeIn& r) : mrTest(r.mrTest) {}
    bool operator() (const ScRange* pRange) const
    {
        return mrTest.In(*pRange);
    }
private:
    const T& mrTest;
};

template<typename T>
class FindIntersectingRange : public ::std::unary_function<ScRange*, bool>
{
public:
    explicit FindIntersectingRange(const T& rTest) : mrTest(rTest) {}
    FindIntersectingRange(const FindIntersectingRange& r) : mrTest(r.mrTest) {}
    bool operator() (const ScRange* pRange) const
    {
        return pRange->Intersects(mrTest);
    }
private:
    const T& mrTest;
};

class AppendToList : public ::std::unary_function<const ScRange*, void>
{
public:
    explicit AppendToList(vector<ScRange*>& rRanges) : mrRanges(rRanges) {}
    AppendToList(const AppendToList& r) : mrRanges(r.mrRanges) {}
    void operator() (const ScRange* p)
    {
        mrRanges.push_back(new ScRange(*p));
    }
private:
    vector<ScRange*>& mrRanges;
};

class CountCells : public ::std::unary_function<const ScRange*, void>
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

class FormatString : public ::std::unary_function<const ScRange*, void>
{
public:
    FormatString(OUString& rStr, ScRefFlags nFlags, ScDocument* pDoc, FormulaGrammar::AddressConvention eConv, sal_Unicode cDelim) :
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
        OUString aStr(p->Format(mnFlags, mpDoc, meConv));
        if (mbFirst)
            mbFirst = false;
        else
            mrStr += OUStringLiteral1(mcDelim);
        mrStr += aStr;
    }
private:
    OUString& mrStr;
    ScRefFlags mnFlags;
    ScDocument* mpDoc;
    FormulaGrammar::AddressConvention meConv;
    sal_Unicode mcDelim;
    bool mbFirst;
};

}

//  ScRangeList
ScRangeList::~ScRangeList()
{
    RemoveAll();
}

ScRefFlags ScRangeList::Parse( const OUString& rStr, ScDocument* pDoc,
                           formula::FormulaGrammar::AddressConvention eConv,
                           SCTAB nDefaultTab, sal_Unicode cDelimiter )
{
    if ( !rStr.isEmpty() )
    {
        if (!cDelimiter)
            cDelimiter = ScCompiler::GetNativeSymbolChar(ocSep);

        ScRefFlags nResult = ~ScRefFlags::ZERO;    // set all bits
        ScRange aRange;
        const SCTAB nTab = pDoc ? nDefaultTab : 0;

        sal_Int32 nPos = 0;
        do
        {
            const OUString aOne = rStr.getToken( 0, cDelimiter, nPos );
            aRange.aStart.SetTab( nTab );   // default tab if not specified
            ScRefFlags nRes = aRange.ParseAny( aOne, pDoc, eConv );
            ScRefFlags nEndRangeBits = ScRefFlags::COL2_VALID | ScRefFlags::ROW2_VALID | ScRefFlags::TAB2_VALID;
            ScRefFlags nTmp1 = ( nRes & ScRefFlags::BITS );
            ScRefFlags nTmp2 = ( nRes & nEndRangeBits );
            // If we have a valid single range with
            // any of the address bits we are interested in
            // set - set the equiv end range bits
            if ( (nRes & ScRefFlags::VALID ) && (nTmp1 != ScRefFlags::ZERO) && ( nTmp2 != nEndRangeBits ) )
                applyStartToEndFlags(nRes, nTmp1);

            if ( nRes & ScRefFlags::VALID )
                Append( aRange );
            nResult &= nRes;        // all common bits are preserved
        }
        while (nPos >= 0);

        return nResult;             // ScRefFlags::VALID set when all are OK
    }
    else
        return ScRefFlags::ZERO;
}

void ScRangeList::Format( OUString& rStr, ScRefFlags nFlags, ScDocument* pDoc,
                          formula::FormulaGrammar::AddressConvention eConv,
                          sal_Unicode cDelimiter ) const
{

    if (!cDelimiter)
        cDelimiter = ScCompiler::GetNativeSymbolChar(ocSep);

    OUString aStr;
    FormatString func(aStr, nFlags, pDoc, eConv, cDelimiter);
    for_each(maRanges.begin(), maRanges.end(), func);
    rStr = aStr;
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

    // One common usage is to join ranges that actually are top to bottom
    // appends but the caller doesn't exactly know about it, e.g. when invoked
    // by ScMarkData::FillRangeListWithMarks(), check for this special case
    // first and speed up things by not looping over all ranges for each range
    // to be joined. We don't remember the exact encompassing range that would
    // have to be updated on refupdates and insertions and deletions, instead
    // remember just the maximum row used, even independently of the sheet.
    // This satisfies most use cases.

    if (!bIsInList)
    {
        if (nRow1 > mnMaxRowUsed + 1)
        {
            Append( r );
            return;
        }
        else if (nRow1 == mnMaxRowUsed + 1)
        {
            // Check if we can simply enlarge the last range.
            ScRange* p = maRanges.back();
            if (p->aEnd.Row() + 1 == nRow1 &&
                    p->aStart.Col() == nCol1 && p->aEnd.Col() == nCol2 &&
                    p->aStart.Tab() == nTab1 && p->aEnd.Tab() == nTab2)
            {
                p->aEnd.SetRow( nRow2 );
                mnMaxRowUsed = nRow2;
                return;
            }
        }
    }

    ScRange* pOver = const_cast<ScRange*>(&r);     // fies aber wahr wenn bInList
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

    // We need to query the size of the container dynamically since its size
    // may change during the loop.
    for ( size_t i = 0; i < maRanges.size() && pOver; ++i )
    {
        ScRange* p = maRanges[i];
        if ( p == pOver )
            continue;           // the same one, continue with the next
        bool bJoined = false;
        if ( p->In( r ) )
        {   //range r included in or identical to range p
            if ( bIsInList )
                bJoined = true;     // do away with range r
            else
            {   // that was all then
                bJoinedInput = true;    // don't attach
                break;  // for
            }
        }
        else if ( r.In( *p ) )
        {   // range p included in range r, make r the new range
            *p = r;
            bJoined = true;
        }
        if ( !bJoined && p->aStart.Tab() == nTab1 && p->aEnd.Tab() == nTab2 )
        {   // 2D
            if ( p->aStart.Col() == nCol1 && p->aEnd.Col() == nCol2 )
            {
                if ( p->aStart.Row() == nRow2+1 )
                {   // top
                    p->aStart.SetRow( nRow1 );
                    bJoined = true;
                }
                else if ( p->aEnd.Row() == nRow1-1 )
                {   // bottom
                    p->aEnd.SetRow( nRow2 );
                    bJoined = true;
                }
            }
            else if ( p->aStart.Row() == nRow1 && p->aEnd.Row() == nRow2 )
            {
                if ( p->aStart.Col() == nCol2+1 )
                {   // left
                    p->aStart.SetCol( nCol1 );
                    bJoined = true;
                }
                else if ( p->aEnd.Col() == nCol1-1 )
                {   // right
                    p->aEnd.SetCol( nCol2 );
                    bJoined = true;
                }
            }
        }
        if ( bJoined )
        {
            if ( bIsInList )
            {   // delete range within the list
                Remove(nOldPos);
                i--;
                pOver = nullptr;
                if ( nOldPos )
                    nOldPos--;          // configure seek correctly
            }
            bJoinedInput = true;
            Join( *p, true );           // recursive!
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

    if(eUpdateRefMode == URM_INSDEL)
    {
        // right now this only works for nTab1 == nTab2
        if(nTab1 == nTab2)
        {
            if(nDx < 0)
            {
                bChanged = DeleteArea(nCol1+nDx, nRow1, nTab1, nCol1-1, nRow2, nTab2);
            }
            if(nDy < 0)
            {
                bChanged = DeleteArea(nCol1, nRow1+nDy, nTab1, nCol2, nRow1-1, nTab2);
            }
            SAL_WARN_IF(nDx < 0 && nDy < 0, "sc", "nDx and nDy are negative, check why");
        }
    }

    if(maRanges.empty())
        return true;

    iterator itr = maRanges.begin(), itrEnd = maRanges.end();
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
            if (mnMaxRowUsed < theRow2)
                mnMaxRowUsed = theRow2;
        }
    }

    if(eUpdateRefMode == URM_INSDEL)
    {
        if( nDx < 0 || nDy < 0 )
        {
            size_t n = maRanges.size();
            Join(*maRanges[n-1], true);
        }
    }

    return bChanged;
}

void ScRangeList::InsertRow( SCTAB nTab, SCCOL nColStart, SCCOL nColEnd, SCROW nRowPos, SCSIZE nSize )
{
    std::vector<ScRange> aNewRanges;
    for(iterator it = maRanges.begin(), itEnd = maRanges.end(); it != itEnd;
            ++it)
    {
        ScRange* pRange = *it;
        if(pRange->aStart.Tab() <= nTab && pRange->aEnd.Tab() >= nTab)
        {
            if(pRange->aEnd.Row() == nRowPos - 1 && (nColStart <= pRange->aEnd.Col() || nColEnd >= pRange->aStart.Col()))
            {
                SCCOL nNewRangeStartCol = std::max<SCCOL>(nColStart, pRange->aStart.Col());
                SCCOL nNewRangeEndCol = std::min<SCCOL>(nColEnd, pRange->aEnd.Col());
                SCROW nNewRangeStartRow = pRange->aEnd.Row() + 1;
                SCROW nNewRangeEndRow = nRowPos + nSize - 1;
                aNewRanges.push_back(ScRange(nNewRangeStartCol, nNewRangeStartRow, nTab, nNewRangeEndCol,
                            nNewRangeEndRow, nTab));
                if (mnMaxRowUsed < nNewRangeEndRow)
                    mnMaxRowUsed = nNewRangeEndRow;
            }
        }
    }

    for(std::vector<ScRange>::const_iterator it = aNewRanges.begin(), itEnd = aNewRanges.end();
            it != itEnd; ++it)
    {
        if(!it->IsValid())
            continue;

        Join(*it);
    }
}

void ScRangeList::InsertCol( SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, SCCOL nColPos, SCSIZE nSize )
{
    std::vector<ScRange> aNewRanges;
    for(iterator it = maRanges.begin(), itEnd = maRanges.end(); it != itEnd;
            ++it)
    {
        ScRange* pRange = *it;
        if(pRange->aStart.Tab() <= nTab && pRange->aEnd.Tab() >= nTab)
        {
            if(pRange->aEnd.Col() == nColPos - 1 && (nRowStart <= pRange->aEnd.Row() || nRowEnd >= pRange->aStart.Row()))
            {
                SCROW nNewRangeStartRow = std::max<SCROW>(nRowStart, pRange->aStart.Row());
                SCROW nNewRangeEndRow = std::min<SCROW>(nRowEnd, pRange->aEnd.Row());
                SCCOL nNewRangeStartCol = pRange->aEnd.Col() + 1;
                SCCOL nNewRangeEndCol = nColPos + nSize - 1;
                aNewRanges.push_back(ScRange(nNewRangeStartCol, nNewRangeStartRow, nTab, nNewRangeEndCol,
                            nNewRangeEndRow, nTab));
            }
        }
    }

    for(std::vector<ScRange>::const_iterator it = aNewRanges.begin(), itEnd = aNewRanges.end();
            it != itEnd; ++it)
    {
        if(!it->IsValid())
            continue;

        Join(*it);
    }
}

namespace {

/**
 * Check if the deleting range cuts the test range exactly into a single
 * piece.
 *
 * X = column ; Y = row
 * +------+    +------+
 * |xxxxxx|    |      |
 * +------+ or +------+
 * |      |    |xxxxxx|
 * +------+    +------+
 *
 * X = row; Y = column
 * +--+--+    +--+--+
 * |xx|  |    |  |xx|
 * |xx|  | or |  |xx|
 * |xx|  |    |  |xx|
 * +--+--+    +--+--+
 * where xxx is the deleted region.
 */
template<typename X, typename Y>
bool checkForOneRange(
   X nDeleteX1, X nDeleteX2, Y nDeleteY1, Y nDeleteY2, X nX1, X nX2, Y nY1, Y nY2)
{
    if (nDeleteX1 <= nX1 && nX2 <= nDeleteX2 && (nDeleteY1 <= nY1 || nY2 <= nDeleteY2))
        return true;

    return false;
}

bool handleOneRange( const ScRange& rDeleteRange, ScRange* p )
{
    const ScAddress& rDelStart = rDeleteRange.aStart;
    const ScAddress& rDelEnd = rDeleteRange.aEnd;
    ScAddress aPStart = p->aStart;
    ScAddress aPEnd = p->aEnd;
    SCCOL nDeleteCol1 = rDelStart.Col();
    SCCOL nDeleteCol2 = rDelEnd.Col();
    SCROW nDeleteRow1 = rDelStart.Row();
    SCROW nDeleteRow2 = rDelEnd.Row();
    SCCOL nCol1 = aPStart.Col();
    SCCOL nCol2 = aPEnd.Col();
    SCROW nRow1 = aPStart.Row();
    SCROW nRow2 = aPEnd.Row();

    if (checkForOneRange(nDeleteCol1, nDeleteCol2, nDeleteRow1, nDeleteRow2, nCol1, nCol2, nRow1, nRow2))
    {
        // Deleting range fully overlaps the column range.  Adjust the row span.
        if (nDeleteRow1 <= nRow1)
        {
            // +------+
            // |xxxxxx|
            // +------+
            // |      |
            // +------+ (xxx) = deleted region

            p->aStart.SetRow(nDeleteRow1+1);
            return true;
        }
        else if (nRow2 <= nDeleteRow2)
        {
            // +------+
            // |      |
            // +------+
            // |xxxxxx|
            // +------+ (xxx) = deleted region

            p->aEnd.SetRow(nDeleteRow1-1);
            return true;
        }
    }
    else if (checkForOneRange(nDeleteRow1, nDeleteRow2, nDeleteCol1, nDeleteCol2, nRow1, nRow2, nCol1, nCol2))
    {
        // Deleting range fully overlaps the row range.  Adjust the column span.
        if (nDeleteCol1 <= nCol1)
        {
            // +--+--+
            // |xx|  |
            // |xx|  |
            // |xx|  |
            // +--+--+ (xxx) = deleted region

            p->aStart.SetCol(nDeleteCol2+1);
            return true;
        }
        else if (nCol2 <= nDeleteCol2)
        {
            // +--+--+
            // |  |xx|
            // |  |xx|
            // |  |xx|
            // +--+--+ (xxx) = deleted region

            p->aEnd.SetCol(nDeleteCol1-1);
            return true;
        }
    }
    return false;
}

bool handleTwoRanges( const ScRange& rDeleteRange, ScRange* p, std::vector<ScRange>& rNewRanges )
{
    const ScAddress& rDelStart = rDeleteRange.aStart;
    const ScAddress& rDelEnd = rDeleteRange.aEnd;
    ScAddress aPStart = p->aStart;
    ScAddress aPEnd = p->aEnd;
    SCCOL nDeleteCol1 = rDelStart.Col();
    SCCOL nDeleteCol2 = rDelEnd.Col();
    SCROW nDeleteRow1 = rDelStart.Row();
    SCROW nDeleteRow2 = rDelEnd.Row();
    SCCOL nCol1 = aPStart.Col();
    SCCOL nCol2 = aPEnd.Col();
    SCROW nRow1 = aPStart.Row();
    SCROW nRow2 = aPEnd.Row();
    SCTAB nTab = aPStart.Tab();

    if (nCol1 < nDeleteCol1 && nDeleteCol1 <= nCol2 && nCol2 <= nDeleteCol2)
    {
        // column deleted :     |-------|
        // column original: |-------|
        if (nRow1 < nDeleteRow1 && nDeleteRow1 <= nRow2 && nRow2 <= nDeleteRow2)
        {
            // row deleted:     |------|
            // row original: |------|
            //
            // +-------+
            // |   1   |
            // +---+---+---+
            // | 2 |xxxxxxx|
            // +---+xxxxxxx|
            //     |xxxxxxx|
            //     +-------+ (xxx) deleted region

            ScRange aNewRange( nCol1, nDeleteRow1, nTab, nDeleteCol1-1, nRow2, nTab ); // 2
            rNewRanges.push_back(aNewRange);

            p->aEnd.SetRow(nDeleteRow1-1); // 1
            return true;
        }
        else if (nRow1 <= nDeleteRow2 && nDeleteRow2 < nRow2 && nDeleteRow1 <= nRow1)
        {
            // row deleted:  |------|
            // row original:    |------|
            //
            //     +-------+
            //     |xxxxxxx|
            // +---+xxxxxxx|
            // | 1 |xxxxxxx|
            // +---+---+---+
            // |   2   |    (xxx) deleted region
            // +-------+

            ScRange aNewRange( aPStart, ScAddress(nDeleteCol1-1, nRow2, nTab) ); // 1
            rNewRanges.push_back(aNewRange);

            p->aStart.SetRow(nDeleteRow2+1); // 2
            return true;
        }
    }
    else if (nCol1 <= nDeleteCol2 && nDeleteCol2 < nCol2 && nDeleteCol1 <= nCol1)
    {
        // column deleted : |-------|
        // column original:     |-------|
        if (nRow1 < nDeleteRow1 && nDeleteRow1 <= nRow2 && nRow2 <= nDeleteRow2)
        {
            // row deleted:     |------|
            // row original: |------|
            //
            //     +-------+
            //     |   1   |
            // +-------+---+
            // |xxxxxxx| 2 |
            // |xxxxxxx+---+
            // |xxxxxxx|
            // +-------+
            //  (xxx) deleted region

            ScRange aNewRange( ScAddress( nDeleteCol2+1, nDeleteRow1, nTab ), aPEnd ); // 2
            rNewRanges.push_back(aNewRange);

            p->aEnd.SetRow(nDeleteRow1-1); // 1
            return true;
        }
        else if (nRow1 <= nDeleteRow2 && nDeleteRow2 < nRow2 && nDeleteRow1 <= nRow1)
        {
            // row deleted:  |-------|
            // row original:     |--------|
            //
            // +-------+
            // |xxxxxxx|
            // |xxxxxxx+---+
            // |xxxxxxx| 1 |
            // +-------+---+
            //     |   2   |
            //     +-------+ (xxx) deleted region

            ScRange aNewRange(nDeleteCol2+1, nRow1, nTab, nCol2, nDeleteRow2, nTab); // 1
            rNewRanges.push_back(aNewRange);

            p->aStart.SetRow(nDeleteRow2+1); // 2
            return true;
        }
    }
    else if (nRow1 < nDeleteRow1 && nDeleteRow2 < nRow2 && nDeleteCol1 <= nCol1 && nCol2 <= nDeleteCol2)
    {
        // +--------+
        // |   1    |
        // +--------+
        // |xxxxxxxx| (xxx) deleted region
        // +--------+
        // |   2    |
        // +--------+

        ScRange aNewRange( aPStart, ScAddress(nCol2, nDeleteRow1-1, nTab) ); // 1
        rNewRanges.push_back(aNewRange);

        p->aStart.SetRow(nDeleteRow2+1); // 2
        return true;
    }
    else if (nCol1 < nDeleteCol1 && nDeleteCol2 < nCol2 && nDeleteRow1 <= nRow1 && nRow2 <= nDeleteRow2)
    {
        // +---+-+---+
        // |   |x|   |
        // |   |x|   |
        // | 1 |x| 2 | (xxx) deleted region
        // |   |x|   |
        // |   |x|   |
        // +---+-+---+

        ScRange aNewRange( aPStart, ScAddress(nDeleteCol1-1, nRow2, nTab) ); // 1
        rNewRanges.push_back(aNewRange);

        p->aStart.SetCol(nDeleteCol2+1); // 2
        return true;
    }

    return false;
}

/**
 * Check if any of the followings applies:
 *
 * X = column; Y = row
 * +----------+           +----------+
 * |          |           |          |
 * |  +-------+---+    +--+-------+  |
 * |  |xxxxxxxxxxx| or |xxxxxxxxxx|  |
 * |  +-------+---+    +--+-------+  |
 * |          |           |          |
 * +----------+           +----------+
 *
 * X = row; Y = column
 *     +--+
 *     |xx|
 * +---+xx+---+    +----------+
 * |   |xx|   |    |          |
 * |   |xx|   | or |   +--+   |
 * |   +--+   |    |   |xx|   |
 * |          |    |   |xx|   |
 * +----------+    +---+xx+---+
 *                     |xx|
 *                     +--+     (xxx) deleted region
 */
template<typename X, typename Y>
bool checkForThreeRanges(
   X nDeleteX1, X nDeleteX2, Y nDeleteY1, Y nDeleteY2, X nX1, X nX2, Y nY1, Y nY2)
{
    if (nX1 <= nDeleteX1 && nX2 <= nDeleteX2 && nY1 < nDeleteY1 && nDeleteY2 < nY2)
        return true;

    if (nDeleteX1 <= nX1 && nDeleteX2 <= nX2 && nY1 < nDeleteY1 && nDeleteY2 < nY2)
        return true;

    return false;
}

bool handleThreeRanges( const ScRange& rDeleteRange, ScRange* p, std::vector<ScRange>& rNewRanges )
{
    const ScAddress& rDelStart = rDeleteRange.aStart;
    const ScAddress& rDelEnd = rDeleteRange.aEnd;
    ScAddress aPStart = p->aStart;
    ScAddress aPEnd = p->aEnd;
    SCCOL nDeleteCol1 = rDelStart.Col();
    SCCOL nDeleteCol2 = rDelEnd.Col();
    SCROW nDeleteRow1 = rDelStart.Row();
    SCROW nDeleteRow2 = rDelEnd.Row();
    SCCOL nCol1 = aPStart.Col();
    SCCOL nCol2 = aPEnd.Col();
    SCROW nRow1 = aPStart.Row();
    SCROW nRow2 = aPEnd.Row();
    SCTAB nTab = aPStart.Tab();

    if (checkForThreeRanges(nDeleteCol1, nDeleteCol2, nDeleteRow1, nDeleteRow2, nCol1, nCol2, nRow1, nRow2))
    {
        if (nCol1 < nDeleteCol1)
        {
            // +---+------+
            // |   |  2   |
            // |   +------+---+
            // | 1 |xxxxxxxxxx|
            // |   +------+---+
            // |   |  3   |
            // +---+------+

            ScRange aNewRange(nDeleteCol1, nRow1, nTab, nCol2, nDeleteRow1-1, nTab); // 2
            rNewRanges.push_back(aNewRange);

            aNewRange = ScRange(ScAddress(nDeleteCol1, nDeleteRow2+1, nTab), aPEnd); // 3
            rNewRanges.push_back(aNewRange);

            p->aEnd.SetCol(nDeleteCol1-1); // 1
        }
        else
        {
            //     +------+---+
            //     |  1   |   |
            // +---+------+   |
            // |xxxxxxxxxx| 2 |
            // +---+------+   |
            //     |  3   |   |
            //     +------+---+

            ScRange aNewRange(aPStart, ScAddress(nDeleteCol2, nDeleteRow1-1, nTab)); // 1
            rNewRanges.push_back(aNewRange);

            aNewRange = ScRange(nCol1, nDeleteRow2+1, nTab, nDeleteCol2, nRow2, nTab); // 3
            rNewRanges.push_back(aNewRange);

            p->aStart.SetCol(nDeleteCol2+1); // 2
        }
        return true;
    }
    else if (checkForThreeRanges(nDeleteRow1, nDeleteRow2, nDeleteCol1, nDeleteCol2, nRow1, nRow2, nCol1, nCol2))
    {
        if (nRow1 < nDeleteRow1)
        {
            // +----------+
            // |    1     |
            // +---+--+---+
            // |   |xx|   |
            // | 2 |xx| 3 |
            // |   |xx|   |
            // +---+xx+---+
            //     |xx|
            //     +--+

            ScRange aNewRange(nCol1, nDeleteRow1, nTab, nDeleteCol1-1, nRow2, nTab); // 2
            rNewRanges.push_back( aNewRange );

            aNewRange = ScRange(ScAddress(nDeleteCol2+1, nDeleteRow1, nTab), aPEnd); // 3
            rNewRanges.push_back( aNewRange );

            p->aEnd.SetRow(nDeleteRow1-1); // 1
        }
        else
        {
            //     +--+
            //     |xx|
            // +---+xx+---+
            // | 1 |xx| 2 |
            // |   |xx|   |
            // +---+--+---+
            // |    3     |
            // +----------+

            ScRange aNewRange(aPStart, ScAddress(nDeleteCol1-1, nDeleteRow2, nTab)); // 1
            rNewRanges.push_back(aNewRange);

            aNewRange = ScRange(nDeleteCol2+1, nRow1, nTab, nCol2, nDeleteRow2, nTab); // 2
            rNewRanges.push_back( aNewRange );

            p->aStart.SetRow(nDeleteRow2+1); // 3
        }
        return true;
    }

    return false;
}

bool handleFourRanges( const ScRange& rDelRange, ScRange* p, std::vector<ScRange>& rNewRanges )
{
    const ScAddress& rDelStart = rDelRange.aStart;
    const ScAddress& rDelEnd = rDelRange.aEnd;
    ScAddress aPStart = p->aStart;
    ScAddress aPEnd = p->aEnd;
    SCCOL nDeleteCol1 = rDelStart.Col();
    SCCOL nDeleteCol2 = rDelEnd.Col();
    SCROW nDeleteRow1 = rDelStart.Row();
    SCROW nDeleteRow2 = rDelEnd.Row();
    SCCOL nCol1 = aPStart.Col();
    SCCOL nCol2 = aPEnd.Col();
    SCROW nRow1 = aPStart.Row();
    SCROW nRow2 = aPEnd.Row();
    SCTAB nTab = aPStart.Tab();

    if (nCol1 < nDeleteCol1 && nDeleteCol2 < nCol2 && nRow1 < nDeleteRow1 && nDeleteRow2 < nRow2)
    {

        // +---------------+
        // |       1       |
        // +---+-------+---+
        // |   |xxxxxxx|   |
        // | 2 |xxxxxxx| 3 |
        // |   |xxxxxxx|   |
        // +---+-------+---+
        // |       4       |
        // +---------------+

        ScRange aNewRange(ScAddress(nCol1, nDeleteRow2+1, nTab), aPEnd); // 4
        rNewRanges.push_back( aNewRange );

        aNewRange = ScRange(nCol1, nDeleteRow1, nTab, nDeleteCol1-1, nDeleteRow2, nTab); // 2
        rNewRanges.push_back( aNewRange );

        aNewRange = ScRange(nDeleteCol2+1, nDeleteRow1, nTab, nCol2, nDeleteRow2, nTab); // 3
        rNewRanges.push_back( aNewRange );

        p->aEnd.SetRow(nDeleteRow1-1); // 1

        return true;
    }

    return false;
}

}

bool ScRangeList::DeleteArea( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2 )
{
    bool bChanged = false;
    ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    for(size_t i = 0; i < maRanges.size();)
    {
        if(FindRangeIn< ScRange >(aRange)(maRanges[i]))
        {
            Remove(i);
            bChanged = true;
        }
        else
            ++i;
    }

    std::vector<ScRange> aNewRanges;

    for(iterator itr = maRanges.begin(); itr != maRanges.end(); ++itr)
    {
        // we have two basic cases here:
        // 1. Delete area and pRange intersect
        // 2. Delete area and pRange are not intersecting
        // checking for 2 and if true skip this range
        if(!(*itr)->Intersects(aRange))
            continue;

        // We get between 1 and 4 ranges from the difference of the first with the second

        // X either Col or Row and Y then the opposite
        // r = deleteRange, p = entry from ScRangeList

        // getting exactly one range is the simple case
        // r.aStart.X() <= p.aStart.X() && r.aEnd.X() >= p.aEnd.X()
        // && ( r.aStart.Y() <= p.aStart.Y() || r.aEnd.Y() >= r.aEnd.Y() )
        if(handleOneRange( aRange, *itr ))
        {
            bChanged = true;
            continue;
        }

        // getting two ranges
        // r.aStart.X()
        else if(handleTwoRanges( aRange, *itr, aNewRanges ))
        {
            bChanged = true;
            continue;
        }

        // getting 3 ranges
        // r.aStart.X() > p.aStart.X() && r.aEnd.X() >= p.aEnd.X()
        // && r.aStart.Y() > p.aStart.Y() && r.aEnd.Y() < p.aEnd.Y()
        // or
        // r.aStart.X() <= p.aStart.X() && r.aEnd.X() < p.aEnd.X()
        // && r.aStart.Y() > p.aStart.Y() && r.aEnd.Y() < p.aEnd.Y()
        else if(handleThreeRanges( aRange, *itr, aNewRanges ))
        {
            bChanged = true;
            continue;
        }

        // getting 4 ranges
        // r.aStart.X() > p.aStart.X() && r.aEnd().X() < p.aEnd.X()
        // && r.aStart.Y() > p.aStart.Y() && r.aEnd().Y() < p.aEnd.Y()
        else if(handleFourRanges( aRange, *itr, aNewRanges ))
        {
            bChanged = true;
            continue;
        }
    }
    for(vector<ScRange>::iterator itr = aNewRanges.begin(); itr != aNewRanges.end(); ++itr)
        Join( *itr);

    return bChanged;
}

const ScRange* ScRangeList::Find( const ScAddress& rAdr ) const
{
    const_iterator itr = find_if(
        maRanges.begin(), maRanges.end(), FindEnclosingRange<ScAddress>(rAdr));
    return itr == maRanges.end() ? nullptr : *itr;
}

ScRange* ScRangeList::Find( const ScAddress& rAdr )
{
    iterator itr = find_if(
        maRanges.begin(), maRanges.end(), FindEnclosingRange<ScAddress>(rAdr));
    return itr == maRanges.end() ? nullptr : *itr;
}

ScRangeList::ScRangeList() : mnMaxRowUsed(-1) {}

ScRangeList::ScRangeList( const ScRangeList& rList ) :
    SvRefBase(),
    mnMaxRowUsed(-1)
{
    maRanges.reserve(rList.maRanges.size());
    for_each(rList.maRanges.begin(), rList.maRanges.end(), AppendToList(maRanges));
    mnMaxRowUsed = rList.mnMaxRowUsed;
}

ScRangeList::ScRangeList( const ScRange& rRange ) :
    mnMaxRowUsed(-1)
{
    maRanges.reserve(1);
    Append(rRange);
}

ScRangeList& ScRangeList::operator=(const ScRangeList& rList)
{
    RemoveAll();
    maRanges.reserve(rList.maRanges.size());
    for_each(rList.maRanges.begin(), rList.maRanges.end(), AppendToList(maRanges));
    mnMaxRowUsed = rList.mnMaxRowUsed;
    return *this;
}

void ScRangeList::Append( const ScRange& rRange )
{
    ScRange* pR = new ScRange( rRange );
    push_back( pR );
}

bool ScRangeList::Intersects( const ScRange& rRange ) const
{
    return std::any_of(maRanges.begin(), maRanges.end(), FindIntersectingRange<ScRange>(rRange));
}

bool ScRangeList::In( const ScRange& rRange ) const
{
    return std::any_of(maRanges.begin(), maRanges.end(), FindEnclosingRange<ScRange>(rRange));
}

size_t ScRangeList::GetCellCount() const
{
    CountCells func;
    return for_each(maRanges.begin(), maRanges.end(), func).getCellCount();
}

void ScRangeList::Remove(size_t nPos)
{
    if (maRanges.size() <= nPos)
        // Out-of-bound condition.  Bail out.
        return;

    iterator itr = maRanges.begin();
    advance(itr, nPos);
    delete *itr;
    maRanges.erase(itr);
}

void ScRangeList::RemoveAll()
{
    for_each(maRanges.begin(), maRanges.end(), std::default_delete<ScRange>());
    maRanges.clear();
}

ScRange ScRangeList::Combine() const
{
    if (maRanges.empty())
        return ScRange();

    const_iterator itr = maRanges.begin(), itrEnd = maRanges.end();
    ScRange aRet = **itr;
    ++itr;
    for (; itr != itrEnd; ++itr)
    {
        const ScRange& r = **itr;
        SCROW nRow1 = r.aStart.Row(), nRow2 = r.aEnd.Row();
        SCCOL nCol1 = r.aStart.Col(), nCol2 = r.aEnd.Col();
        SCTAB nTab1 = r.aStart.Tab(), nTab2 = r.aEnd.Tab();
        if (aRet.aStart.Row() > nRow1)
            aRet.aStart.SetRow(nRow1);
        if (aRet.aStart.Col() > nCol1)
            aRet.aStart.SetCol(nCol1);
        if (aRet.aStart.Tab() > nTab1)
            aRet.aStart.SetTab(nTab1);
        if (aRet.aEnd.Row() < nRow2)
            aRet.aEnd.SetRow(nRow2);
        if (aRet.aEnd.Col() < nCol2)
            aRet.aEnd.SetCol(nCol2);
        if (aRet.aEnd.Tab() < nTab2)
            aRet.aEnd.SetTab(nTab2);
    }
    return aRet;
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
    if (mnMaxRowUsed < p->aEnd.Row())
        mnMaxRowUsed = p->aEnd.Row();
}

void ScRangeList::swap( ScRangeList& r )
{
    maRanges.swap(r.maRanges);
    std::swap(mnMaxRowUsed, r.mnMaxRowUsed);
}

ScAddress ScRangeList::GetTopLeftCorner() const
{
    if(empty())
        return ScAddress();

    ScAddress aAddr = maRanges[0]->aStart;
    for(size_t i = 1, n = size(); i < n; ++i)
    {
        if(maRanges[i]->aStart < aAddr)
            aAddr = maRanges[i]->aStart;
    }

    return aAddr;
}

ScRangeList ScRangeList::GetIntersectedRange(const ScRange& rRange) const
{
    ScRangeList aReturn;
    for(const_iterator itr = maRanges.begin(), itrEnd = maRanges.end();
            itr != itrEnd; ++itr)
    {
        if((*itr)->Intersects(rRange))
        {
            SCCOL nColStart1, nColEnd1, nColStart2, nColEnd2;
            SCROW nRowStart1, nRowEnd1, nRowStart2, nRowEnd2;
            SCTAB nTabStart1, nTabEnd1, nTabStart2, nTabEnd2;
            (*itr)->GetVars(nColStart1, nRowStart1, nTabStart1,
                        nColEnd1, nRowEnd1, nTabEnd1);
            rRange.GetVars(nColStart2, nRowStart2, nTabStart2,
                        nColEnd2, nRowEnd2, nTabEnd2);

            ScRange aNewRange(std::max<SCCOL>(nColStart1, nColStart2), std::max<SCROW>(nRowStart1, nRowStart2),
                    std::max<SCTAB>(nTabStart1, nTabStart2), std::min<SCCOL>(nColEnd1, nColEnd2),
                    std::min<SCROW>(nRowEnd1, nRowEnd2), std::min<SCTAB>(nTabEnd1, nTabEnd2));
            aReturn.Join(aNewRange);
        }
    }

    return aReturn;
}

//  ScRangePairList
ScRangePairList::~ScRangePairList()
{
    for_each( maPairs.begin(), maPairs.end(), std::default_delete<ScRangePair>() );
    maPairs.clear();
}

void ScRangePairList::Remove(size_t nPos)
{
    if (maPairs.size() <= nPos)
        // Out-of-bound condition.  Bail out.
        return;

    vector<ScRangePair*>::iterator itr = maPairs.begin();
    advance(itr, nPos);
    delete *itr;
    maPairs.erase(itr);
}

void ScRangePairList::Remove( ScRangePair* Adr)
{
    if (Adr == nullptr)
        return;

    for ( vector<ScRangePair*>::iterator itr = maPairs.begin(); itr != maPairs.end(); ++itr )
    {
        if (Adr == *itr)
        {
            delete *itr;
            maPairs.erase( itr );
            return;
        }
    }
    assert(false);
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

void ScRangePairList::UpdateReference( UpdateRefMode eUpdateRefMode,
                                    ScDocument* pDoc, const ScRange& rWhere,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    if ( !maPairs.empty() )
    {
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
        rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        for (ScRangePair* pR : maPairs)
        {
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
                    rRange.aStart.Set( theCol1, theRow1, theTab1 );
                    rRange.aEnd.Set( theCol2, theRow2, theTab2 );
                }
            }
        }
    }
}

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
            nListCount = maPairs.size();
        }
        else
            ++nPos;
    }
}

ScRangePair* ScRangePairList::Find( const ScAddress& rAdr ) const
{
    for (ScRangePair* pR : maPairs)
    {
        if ( pR->GetRange(0).In( rAdr ) )
            return pR;
    }
    return nullptr;
}

ScRangePair* ScRangePairList::Find( const ScRange& rRange ) const
{
    for (ScRangePair* pR : maPairs)
    {
        if ( pR->GetRange(0) == rRange )
            return pR;
    }
    return nullptr;
}

ScRangePairList* ScRangePairList::Clone() const
{
    ScRangePairList* pNew = new ScRangePairList;
    for (const ScRangePair* pR : maPairs)
    {
        pNew->Append( *pR );
    }
    return pNew;
}

struct ScRangePairNameSort
{
    ScRangePair*    pPair;
    ScDocument*     pDoc;
};

extern "C"
int SAL_CALL ScRangePairList_QsortNameCompare( const void* p1, const void* p2 )
{
    const ScRangePairNameSort* ps1 = static_cast<const ScRangePairNameSort*>(p1);
    const ScRangePairNameSort* ps2 = static_cast<const ScRangePairNameSort*>(p2);
    const ScAddress& rStartPos1 = ps1->pPair->GetRange(0).aStart;
    const ScAddress& rStartPos2 = ps2->pPair->GetRange(0).aStart;
    OUString aStr1, aStr2;
    sal_Int32 nComp;
    if ( rStartPos1.Tab() == rStartPos2.Tab() )
        nComp = 0;
    else
    {
        ps1->pDoc->GetName( rStartPos1.Tab(), aStr1 );
        ps2->pDoc->GetName( rStartPos2.Tab(), aStr2 );
        nComp = ScGlobal::GetCollator()->compareString( aStr1, aStr2 );
    }
    if (nComp < 0)
    {
        return -1;
    }
    else if (nComp > 0)
    {
        return 1;
    }

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
    const ScAddress& rEndPos1 = ps1->pPair->GetRange(0).aEnd;
    const ScAddress& rEndPos2 = ps2->pPair->GetRange(0).aEnd;
    if ( rEndPos1.Tab() == rEndPos2.Tab() )
        nComp = 0;
    else
    {
        ps1->pDoc->GetName( rEndPos1.Tab(), aStr1 );
        ps2->pDoc->GetName( rEndPos2.Tab(), aStr2 );
        nComp = ScGlobal::GetCollator()->compareString( aStr1, aStr2 );
    }
    if (nComp < 0)
    {
        return -1;
    }
    else if (nComp > 0)
    {
        return 1;
    }

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
    ScRangePair* pOver = const_cast<ScRangePair*>(&r);     // fies aber wahr wenn bInList
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

    for ( size_t i = 0; i < maPairs.size() && pOver; ++i )
    {
        ScRangePair* p = maPairs[ i ];
        if ( p == pOver )
            continue;           // the same one, continue with the next
        bool bJoined = false;
        ScRange& rp1 = p->GetRange(0);
        ScRange& rp2 = p->GetRange(1);
        if ( rp2 == r2 )
        {   // only if Range2 is equal
            if ( rp1.In( r1 ) )
            {   // RangePair r included in or identical to RangePair p
                if ( bIsInList )
                    bJoined = true;     // do away with RangePair r
                else
                {   // that was all then
                    bJoinedInput = true;    // don't attach
                    break;  // for
                }
            }
            else if ( r1.In( rp1 ) )
            {   // RangePair p included in RangePair r enthalten, make r the new RangePair
                *p = r;
                bJoined = true;
            }
        }
        if ( !bJoined && rp1.aStart.Tab() == nTab1 && rp1.aEnd.Tab() == nTab2
          && rp2.aStart.Tab() == r2.aStart.Tab()
          && rp2.aEnd.Tab() == r2.aEnd.Tab() )
        {   // 2D, Range2 must be located side-by-side just like Range1
            if ( rp1.aStart.Col() == nCol1 && rp1.aEnd.Col() == nCol2
              && rp2.aStart.Col() == r2.aStart.Col()
              && rp2.aEnd.Col() == r2.aEnd.Col() )
            {
                if ( rp1.aStart.Row() == nRow2+1
                  && rp2.aStart.Row() == r2.aEnd.Row()+1 )
                {   // top
                    rp1.aStart.SetRow( nRow1 );
                    rp2.aStart.SetRow( r2.aStart.Row() );
                    bJoined = true;
                }
                else if ( rp1.aEnd.Row() == nRow1-1
                  && rp2.aEnd.Row() == r2.aStart.Row()-1 )
                {   // bottom
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
                {   // left
                    rp1.aStart.SetCol( nCol1 );
                    rp2.aStart.SetCol( r2.aStart.Col() );
                    bJoined = true;
                }
                else if ( rp1.aEnd.Col() == nCol1-1
                  && rp2.aEnd.Col() == r2.aEnd.Col()-1 )
                {   // right
                    rp1.aEnd.SetCol( nCol2 );
                    rp2.aEnd.SetCol( r2.aEnd.Col() );
                    bJoined = true;
                }
            }
        }
        if ( bJoined )
        {
            if ( bIsInList )
            {   // delete RangePair within the list
                Remove( nOldPos );
                i--;
                pOver = nullptr;
                if ( nOldPos )
                    nOldPos--;          // configure seek correctly
            }
            bJoinedInput = true;
            Join( *p, true );           // recursive!
        }
    }
    if ( !bIsInList && !bJoinedInput )
        Append( r );
}

ScRangePair** ScRangePairList::CreateNameSortedArray( size_t& nListCount,
        ScDocument* pDoc ) const
{
    nListCount = maPairs.size();
    OSL_ENSURE( nListCount * sizeof(ScRangePairNameSort) <= (size_t)~0x1F,
        "ScRangePairList::CreateNameSortedArray nListCount * sizeof(ScRangePairNameSort) > (size_t)~0x1F" );
    ScRangePairNameSort* pSortArray = reinterpret_cast<ScRangePairNameSort*>(
        new sal_uInt8 [ nListCount * sizeof(ScRangePairNameSort) ]);
    sal_uLong j;
    for ( j=0; j < nListCount; j++ )
    {
        pSortArray[j].pPair = maPairs[ j ];
        pSortArray[j].pDoc = pDoc;
    }
    qsort( static_cast<void*>(pSortArray), nListCount, sizeof(ScRangePairNameSort), &ScRangePairList_QsortNameCompare );
    // move ScRangePair pointer up
    ScRangePair** ppSortArray = reinterpret_cast<ScRangePair**>(pSortArray);
    for ( j=0; j < nListCount; j++ )
    {
        ppSortArray[j] = pSortArray[j].pPair;
    }
    return ppSortArray;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
