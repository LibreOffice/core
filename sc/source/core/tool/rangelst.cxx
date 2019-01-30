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
#include <sal/log.hxx>

#include <rangelst.hxx>
#include <document.hxx>
#include <refupdat.hxx>
#include <rechead.hxx>
#include <compiler.hxx>
#include <algorithm>
#include <memory>

using ::std::vector;
using ::std::advance;
using ::std::find_if;
using ::std::for_each;
using ::formula::FormulaGrammar;

namespace {

template<typename T>
class FindEnclosingRange
{
public:
    explicit FindEnclosingRange(const T& rTest) : mrTest(rTest) {}
    bool operator() (const ScRange & rRange) const
    {
        return rRange.In(mrTest);
    }
private:
    const T& mrTest;
};

template<typename T>
class FindRangeIn
{
public:
    FindRangeIn(const T& rTest) : mrTest(rTest) {}
    bool operator() (const ScRange& rRange) const
    {
        return mrTest.In(rRange);
    }
private:
    const T& mrTest;
};

template<typename T>
class FindIntersectingRange
{
public:
    explicit FindIntersectingRange(const T& rTest) : mrTest(rTest) {}
    bool operator() (const ScRange & rRange) const
    {
        return rRange.Intersects(mrTest);
    }
private:
    const T& mrTest;
};

class CountCells
{
public:
    CountCells() : mnCellCount(0) {}

    void operator() (const ScRange & r)
    {
        mnCellCount +=
              size_t(r.aEnd.Col() - r.aStart.Col() + 1)
            * size_t(r.aEnd.Row() - r.aStart.Row() + 1)
            * size_t(r.aEnd.Tab() - r.aStart.Tab() + 1);
    }

    size_t getCellCount() const { return mnCellCount; }

private:
    size_t mnCellCount;
};

class FormatString
{
public:
    FormatString(OUString& rStr, ScRefFlags nFlags, ScDocument* pDoc, FormulaGrammar::AddressConvention eConv, sal_Unicode cDelim, bool bFullAddressNotation) :
        mrStr(rStr),
        mnFlags(nFlags),
        mpDoc(pDoc),
        meConv(eConv),
        mcDelim(cDelim),
        mbFirst(true),
        mbFullAddressNotation(bFullAddressNotation) {}

    void operator() (const ScRange & r)
    {
        OUString aStr(r.Format(mnFlags, mpDoc, meConv, mbFullAddressNotation));
        if (mbFirst)
            mbFirst = false;
        else
            mrStr += OUStringLiteral1(mcDelim);
        mrStr += aStr;
    }
private:
    OUString& mrStr;
    ScRefFlags const mnFlags;
    ScDocument* const mpDoc;
    FormulaGrammar::AddressConvention const meConv;
    sal_Unicode const mcDelim;
    bool mbFirst;
    bool const mbFullAddressNotation;
};

}

//  ScRangeList
ScRangeList::~ScRangeList()
{
}

ScRefFlags ScRangeList::Parse( const OUString& rStr, const ScDocument* pDoc,
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
            ScRefFlags nTmp1 = nRes & ScRefFlags::BITS;
            ScRefFlags nTmp2 = nRes & nEndRangeBits;
            // If we have a valid single range with
            // any of the address bits we are interested in
            // set - set the equiv end range bits
            if ( (nRes & ScRefFlags::VALID ) && (nTmp1 != ScRefFlags::ZERO) && ( nTmp2 != nEndRangeBits ) )
                applyStartToEndFlags(nRes, nTmp1);

            if ( nRes & ScRefFlags::VALID )
                push_back( aRange );
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
                          sal_Unicode cDelimiter, bool bFullAddressNotation ) const
{

    if (!cDelimiter)
        cDelimiter = ScCompiler::GetNativeSymbolChar(ocSep);

    OUString aStr;
    FormatString func(aStr, nFlags, pDoc, eConv, cDelimiter, bFullAddressNotation);
    for_each(maRanges.begin(), maRanges.end(), func);
    rStr = aStr;
}

void ScRangeList::Join( const ScRange& rNewRange, bool bIsInList )
{
    if ( maRanges.empty() )
    {
        push_back( rNewRange );
        return ;
    }

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
        const SCROW nRow1 = rNewRange.aStart.Row();
        if (nRow1 > mnMaxRowUsed + 1)
        {
            push_back( rNewRange );
            return;
        }
        else if (nRow1 == mnMaxRowUsed + 1)
        {
            // Check if we can simply enlarge the last range.
            ScRange & rLast = maRanges.back();
            if (rLast.aEnd.Row() + 1 == nRow1 &&
                    rLast.aStart.Col() == rNewRange.aStart.Col() && rLast.aEnd.Col() == rNewRange.aEnd.Col() &&
                    rLast.aStart.Tab() == rNewRange.aStart.Tab() && rLast.aEnd.Tab() == rNewRange.aEnd.Tab())
            {
                const SCROW nRow2 = rNewRange.aEnd.Row();
                rLast.aEnd.SetRow( nRow2 );
                mnMaxRowUsed = nRow2;
                return;
            }
        }
    }

    bool bJoinedInput = false;
    const ScRange* pOver = &rNewRange;

Label_Range_Join:

    assert(pOver);
    const SCCOL nCol1 = pOver->aStart.Col();
    const SCROW nRow1 = pOver->aStart.Row();
    const SCCOL nTab1 = pOver->aStart.Tab();
    const SCCOL nCol2 = pOver->aEnd.Col();
    const SCROW nRow2 = pOver->aEnd.Row();
    const SCCOL nTab2 = pOver->aEnd.Tab();

    size_t nOverPos = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < maRanges.size(); ++i)
    {
        ScRange & rRange = maRanges[i];
        if ( &rRange == pOver )
        {
            nOverPos = i;
            continue;           // the same one, continue with the next
        }
        bool bJoined = false;
        if ( rRange.In( *pOver ) )
        {   // range pOver included in or identical to range p
            // XXX if we never used Append() before Join() we could remove
            // pOver and end processing, but it is not guaranteed and there can
            // be duplicates.
            if ( bIsInList )
                bJoined = true;     // do away with range pOver
            else
            {   // that was all then
                bJoinedInput = true;    // don't append
                break;  // for
            }
        }
        else if ( pOver->In( rRange ) )
        {   // range rRange included in range pOver, make pOver the new range
            rRange = *pOver;
            bJoined = true;
        }
        if ( !bJoined && rRange.aStart.Tab() == nTab1 && rRange.aEnd.Tab() == nTab2 )
        {   // 2D
            if ( rRange.aStart.Col() == nCol1 && rRange.aEnd.Col() == nCol2 )
            {
                if ( rRange.aStart.Row() <= nRow2+1 &&
                     rRange.aStart.Row() >= nRow1 )
                {   // top
                    rRange.aStart.SetRow( nRow1 );
                    bJoined = true;
                }
                else if ( rRange.aEnd.Row() >= nRow1-1 &&
                          rRange.aEnd.Row() <= nRow2 )
                {   // bottom
                    rRange.aEnd.SetRow( nRow2 );
                    bJoined = true;
                }
            }
            else if ( rRange.aStart.Row() == nRow1 && rRange.aEnd.Row() == nRow2 )
            {
                if ( rRange.aStart.Col() <= nCol2+1 &&
                     rRange.aStart.Col() >= nCol1 )
                {   // left
                    rRange.aStart.SetCol( nCol1 );
                    bJoined = true;
                }
                else if ( rRange.aEnd.Col() >= nCol1-1 &&
                          rRange.aEnd.Col() <= nCol2 )
                {   // right
                    rRange.aEnd.SetCol( nCol2 );
                    bJoined = true;
                }
            }
        }
        if ( bJoined )
        {
            if ( bIsInList )
            {   // delete range pOver within the list
                if (nOverPos != std::numeric_limits<size_t>::max())
                {
                    Remove(nOverPos);
                    if (nOverPos < i)
                        --i;
                }
                else
                {
                    for (size_t nOver = 0, nRanges = maRanges.size(); nOver < nRanges; ++nOver)
                    {
                        if (&maRanges[nOver] == pOver)
                        {
                            Remove(nOver);
                            break;
                        }
                    }
                }
            }
            bJoinedInput = true;
            pOver = &maRanges[i];
            bIsInList = true;
            goto Label_Range_Join;
        }
    }
    if (  !bIsInList && !bJoinedInput )
        push_back( rNewRange );
}

bool ScRangeList::operator==( const ScRangeList& r ) const
{
    if ( this == &r )
        return true;

    return maRanges == r.maRanges;
}

bool ScRangeList::operator!=( const ScRangeList& r ) const
{
    return !operator==( r );
}

bool ScRangeList::UpdateReference(
    UpdateRefMode eUpdateRefMode,
    const ScDocument* pDoc,
    const ScRange& rWhere,
    SCCOL nDx,
    SCROW nDy,
    SCTAB nDz
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

    for (auto& rR : maRanges)
    {
        SCCOL theCol1;
        SCROW theRow1;
        SCTAB theTab1;
        SCCOL theCol2;
        SCROW theRow2;
        SCTAB theTab2;
        rR.GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 );
        if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
                nDx, nDy, nDz,
                theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 )
                != UR_NOTHING )
        {
            bChanged = true;
            rR.aStart.Set( theCol1, theRow1, theTab1 );
            rR.aEnd.Set( theCol2, theRow2, theTab2 );
            if (mnMaxRowUsed < theRow2)
                mnMaxRowUsed = theRow2;
        }
    }

    if(eUpdateRefMode == URM_INSDEL)
    {
        if( nDx < 0 || nDy < 0 )
        {
            size_t n = maRanges.size();
            Join(maRanges[n-1], true);
        }
    }

    return bChanged;
}

void ScRangeList::InsertRow( SCTAB nTab, SCCOL nColStart, SCCOL nColEnd, SCROW nRowPos, SCSIZE nSize )
{
    std::vector<ScRange> aNewRanges;
    for(auto & rRange : maRanges)
    {
        if(rRange.aStart.Tab() <= nTab && rRange.aEnd.Tab() >= nTab)
        {
            if(rRange.aEnd.Row() == nRowPos - 1 && (nColStart <= rRange.aEnd.Col() || nColEnd >= rRange.aStart.Col()))
            {
                SCCOL nNewRangeStartCol = std::max<SCCOL>(nColStart, rRange.aStart.Col());
                SCCOL nNewRangeEndCol = std::min<SCCOL>(nColEnd, rRange.aEnd.Col());
                SCROW nNewRangeStartRow = rRange.aEnd.Row() + 1;
                SCROW nNewRangeEndRow = nRowPos + nSize - 1;
                aNewRanges.emplace_back(nNewRangeStartCol, nNewRangeStartRow, nTab, nNewRangeEndCol,
                            nNewRangeEndRow, nTab);
                if (mnMaxRowUsed < nNewRangeEndRow)
                    mnMaxRowUsed = nNewRangeEndRow;
            }
        }
    }

    for(auto & rRange : aNewRanges)
    {
        if(!rRange.IsValid())
            continue;

        Join(rRange);
    }
}

void ScRangeList::InsertCol( SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, SCCOL nColPos, SCSIZE nSize )
{
    std::vector<ScRange> aNewRanges;
    for(auto & rRange : maRanges)
    {
        if(rRange.aStart.Tab() <= nTab && rRange.aEnd.Tab() >= nTab)
        {
            if(rRange.aEnd.Col() == nColPos - 1 && (nRowStart <= rRange.aEnd.Row() || nRowEnd >= rRange.aStart.Row()))
            {
                SCROW nNewRangeStartRow = std::max<SCROW>(nRowStart, rRange.aStart.Row());
                SCROW nNewRangeEndRow = std::min<SCROW>(nRowEnd, rRange.aEnd.Row());
                SCCOL nNewRangeStartCol = rRange.aEnd.Col() + 1;
                SCCOL nNewRangeEndCol = nColPos + nSize - 1;
                aNewRanges.emplace_back(nNewRangeStartCol, nNewRangeStartRow, nTab, nNewRangeEndCol,
                            nNewRangeEndRow, nTab);
            }
        }
    }

    for(auto & rRange : aNewRanges)
    {
        if(!rRange.IsValid())
            continue;

        Join(rRange);
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
    return nDeleteX1 <= nX1 && nX2 <= nDeleteX2 && (nDeleteY1 <= nY1 || nY2 <= nDeleteY2);
}

bool handleOneRange( const ScRange& rDeleteRange, ScRange& r )
{
    const ScAddress& rDelStart = rDeleteRange.aStart;
    const ScAddress& rDelEnd = rDeleteRange.aEnd;
    ScAddress aPStart = r.aStart;
    ScAddress aPEnd = r.aEnd;
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

            r.aStart.SetRow(nDeleteRow1+1);
            return true;
        }
        else if (nRow2 <= nDeleteRow2)
        {
            // +------+
            // |      |
            // +------+
            // |xxxxxx|
            // +------+ (xxx) = deleted region

            r.aEnd.SetRow(nDeleteRow1-1);
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

            r.aStart.SetCol(nDeleteCol2+1);
            return true;
        }
        else if (nCol2 <= nDeleteCol2)
        {
            // +--+--+
            // |  |xx|
            // |  |xx|
            // |  |xx|
            // +--+--+ (xxx) = deleted region

            r.aEnd.SetCol(nDeleteCol1-1);
            return true;
        }
    }
    return false;
}

bool handleTwoRanges( const ScRange& rDeleteRange, ScRange& r, std::vector<ScRange>& rNewRanges )
{
    const ScAddress& rDelStart = rDeleteRange.aStart;
    const ScAddress& rDelEnd = rDeleteRange.aEnd;
    ScAddress aPStart = r.aStart;
    ScAddress aPEnd = r.aEnd;
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

            r.aEnd.SetRow(nDeleteRow1-1); // 1
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

            r.aStart.SetRow(nDeleteRow2+1); // 2
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

            r.aEnd.SetRow(nDeleteRow1-1); // 1
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

            r.aStart.SetRow(nDeleteRow2+1); // 2
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

        r.aStart.SetRow(nDeleteRow2+1); // 2
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

        r.aStart.SetCol(nDeleteCol2+1); // 2
        return true;
    }

    return false;
}

/**
 * Check if any of the following applies:
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

bool handleThreeRanges( const ScRange& rDeleteRange, ScRange& r, std::vector<ScRange>& rNewRanges )
{
    const ScAddress& rDelStart = rDeleteRange.aStart;
    const ScAddress& rDelEnd = rDeleteRange.aEnd;
    ScAddress aPStart = r.aStart;
    ScAddress aPEnd = r.aEnd;
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

            r.aEnd.SetCol(nDeleteCol1-1); // 1
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

            r.aStart.SetCol(nDeleteCol2+1); // 2
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

            r.aEnd.SetRow(nDeleteRow1-1); // 1
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

            r.aStart.SetRow(nDeleteRow2+1); // 3
        }
        return true;
    }

    return false;
}

bool handleFourRanges( const ScRange& rDelRange, ScRange& r, std::vector<ScRange>& rNewRanges )
{
    const ScAddress& rDelStart = rDelRange.aStart;
    const ScAddress& rDelEnd = rDelRange.aEnd;
    ScAddress aPStart = r.aStart;
    ScAddress aPEnd = r.aEnd;
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

        r.aEnd.SetRow(nDeleteRow1-1); // 1

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

    for(auto & rRange : maRanges)
    {
        // we have two basic cases here:
        // 1. Delete area and pRange intersect
        // 2. Delete area and pRange are not intersecting
        // checking for 2 and if true skip this range
        if(!rRange.Intersects(aRange))
            continue;

        // We get between 1 and 4 ranges from the difference of the first with the second

        // X either Col or Row and Y then the opposite
        // r = deleteRange, p = entry from ScRangeList

        // getting exactly one range is the simple case
        // r.aStart.X() <= p.aStart.X() && r.aEnd.X() >= p.aEnd.X()
        // && ( r.aStart.Y() <= p.aStart.Y() || r.aEnd.Y() >= r.aEnd.Y() )
        if(handleOneRange( aRange, rRange ))
        {
            bChanged = true;
            continue;
        }

        // getting two ranges
        // r.aStart.X()
        else if(handleTwoRanges( aRange, rRange, aNewRanges ))
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
        else if(handleThreeRanges( aRange, rRange, aNewRanges ))
        {
            bChanged = true;
            continue;
        }

        // getting 4 ranges
        // r.aStart.X() > p.aStart.X() && r.aEnd().X() < p.aEnd.X()
        // && r.aStart.Y() > p.aStart.Y() && r.aEnd().Y() < p.aEnd.Y()
        else if(handleFourRanges( aRange, rRange, aNewRanges ))
        {
            bChanged = true;
            continue;
        }
    }
    for(auto & rRange : aNewRanges)
        Join(rRange);

    return bChanged;
}

const ScRange* ScRangeList::Find( const ScAddress& rAdr ) const
{
    auto itr = find_if(
        maRanges.cbegin(), maRanges.cend(), FindEnclosingRange<ScAddress>(rAdr));
    return itr == maRanges.end() ? nullptr : &*itr;
}

ScRange* ScRangeList::Find( const ScAddress& rAdr )
{
    auto itr = find_if(
        maRanges.begin(), maRanges.end(), FindEnclosingRange<ScAddress>(rAdr));
    return itr == maRanges.end() ? nullptr : &*itr;
}

ScRangeList::ScRangeList() : mnMaxRowUsed(-1) {}

ScRangeList::ScRangeList( const ScRangeList& rList ) :
    SvRefBase(rList),
    maRanges(rList.maRanges),
    mnMaxRowUsed(rList.mnMaxRowUsed)
{
}

ScRangeList::ScRangeList( const ScRangeList&& rList ) :
    SvRefBase(),
    maRanges(rList.maRanges),
    mnMaxRowUsed(rList.mnMaxRowUsed)
{
}

ScRangeList::ScRangeList( const ScRange& rRange ) :
    mnMaxRowUsed(-1)
{
    maRanges.reserve(1);
    push_back(rRange);
}

ScRangeList& ScRangeList::operator=(const ScRangeList& rList)
{
    maRanges = rList.maRanges;
    mnMaxRowUsed = rList.mnMaxRowUsed;
    return *this;
}

ScRangeList& ScRangeList::operator=(ScRangeList&& rList)
{
    maRanges = std::move(rList.maRanges);
    mnMaxRowUsed = rList.mnMaxRowUsed;
    return *this;
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
    maRanges.erase(maRanges.begin() + nPos);
}

void ScRangeList::RemoveAll()
{
    maRanges.clear();
    mnMaxRowUsed = -1;
}

ScRange ScRangeList::Combine() const
{
    if (maRanges.empty())
        return ScRange();

    auto itr = maRanges.cbegin(), itrEnd = maRanges.cend();
    ScRange aRet = *itr;
    ++itr;
    for (; itr != itrEnd; ++itr)
    {
        const ScRange& r = *itr;
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

void ScRangeList::push_back(const ScRange & r)
{
    maRanges.push_back(r);
    if (mnMaxRowUsed < r.aEnd.Row())
        mnMaxRowUsed = r.aEnd.Row();
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

    ScAddress const * pAddr = &maRanges[0].aStart;
    for(size_t i = 1, n = size(); i < n; ++i)
    {
        if(maRanges[i].aStart < *pAddr)
            pAddr = &maRanges[i].aStart;
    }

    return *pAddr;
}

ScRangeList ScRangeList::GetIntersectedRange(const ScRange& rRange) const
{
    ScRangeList aReturn;
    for(auto& rR : maRanges)
    {
        if(rR.Intersects(rRange))
        {
            SCCOL nColStart1, nColEnd1, nColStart2, nColEnd2;
            SCROW nRowStart1, nRowEnd1, nRowStart2, nRowEnd2;
            SCTAB nTabStart1, nTabEnd1, nTabStart2, nTabEnd2;
            rR.GetVars(nColStart1, nRowStart1, nTabStart1,
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
}

void ScRangePairList::Remove(size_t nPos)
{
    if (maPairs.size() <= nPos)
        // Out-of-bound condition.  Bail out.
        return;
    maPairs.erase(maPairs.begin() + nPos);
}

void ScRangePairList::Remove( const ScRangePair & rAdr)
{
    auto itr = std::find_if(maPairs.begin(), maPairs.end(), [&rAdr](const ScRangePair& rPair) { return &rAdr == &rPair; });
    if (itr != maPairs.end())
    {
        maPairs.erase( itr );
        return;
    }
    assert(false);
}

ScRangePair & ScRangePairList::operator [](size_t idx)
{
    return maPairs[idx];
}

const ScRangePair & ScRangePairList::operator [](size_t idx) const
{
    return maPairs[idx];
}

size_t ScRangePairList::size() const
{
    return maPairs.size();
}

void ScRangePairList::UpdateReference( UpdateRefMode eUpdateRefMode,
                                    const ScDocument* pDoc, const ScRange& rWhere,
                                    SCCOL nDx, SCROW nDy, SCTAB nDz )
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
        for (ScRangePair & rR : maPairs)
        {
            for ( sal_uInt16 j=0; j<2; j++ )
            {
                ScRange& rRange = rR.GetRange(j);
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
    maPairs.erase(std::remove_if(maPairs.begin(), maPairs.end(),
        [&nTab](const ScRangePair& rR) {
            const ScRange & rRange = rR.GetRange(0);
            return (rRange.aStart.Tab() == nTab) && (rRange.aEnd.Tab() == nTab);
        }),
        maPairs.end());
}

ScRangePair* ScRangePairList::Find( const ScAddress& rAdr )
{
    for (ScRangePair & rR : maPairs)
    {
        if ( rR.GetRange(0).In( rAdr ) )
            return &rR;
    }
    return nullptr;
}

ScRangePair* ScRangePairList::Find( const ScRange& rRange )
{
    for (ScRangePair & rR : maPairs)
    {
        if ( rR.GetRange(0) == rRange )
            return &rR;
    }
    return nullptr;
}

ScRangePairList* ScRangePairList::Clone() const
{
    ScRangePairList* pNew = new ScRangePairList;
    for (const ScRangePair & rR : maPairs)
    {
        pNew->Append( rR );
    }
    return pNew;
}

class ScRangePairList_sortNameCompare
{
public:
    ScRangePairList_sortNameCompare(ScDocument *pDoc) : mpDoc(pDoc) {}

    bool operator()( const ScRangePair *ps1, const ScRangePair* ps2 ) const
    {
        const ScAddress& rStartPos1 = ps1->GetRange(0).aStart;
        const ScAddress& rStartPos2 = ps2->GetRange(0).aStart;
        OUString aStr1, aStr2;
        sal_Int32 nComp;
        if ( rStartPos1.Tab() == rStartPos2.Tab() )
            nComp = 0;
        else
        {
            mpDoc->GetName( rStartPos1.Tab(), aStr1 );
            mpDoc->GetName( rStartPos2.Tab(), aStr2 );
            nComp = ScGlobal::GetCollator()->compareString( aStr1, aStr2 );
        }
        if (nComp < 0)
        {
            return true; // -1;
        }
        else if (nComp > 0)
        {
            return false; // 1;
        }

        // equal tabs
        if ( rStartPos1.Col() < rStartPos2.Col() )
            return true; // -1;
        if ( rStartPos1.Col() > rStartPos2.Col() )
            return false; // 1;
        // equal cols
        if ( rStartPos1.Row() < rStartPos2.Row() )
            return true; // -1;
        if ( rStartPos1.Row() > rStartPos2.Row() )
            return false; // 1;

        // first corner equal, second corner
        const ScAddress& rEndPos1 = ps1->GetRange(0).aEnd;
        const ScAddress& rEndPos2 = ps2->GetRange(0).aEnd;
        if ( rEndPos1.Tab() == rEndPos2.Tab() )
            nComp = 0;
        else
        {
            mpDoc->GetName( rEndPos1.Tab(), aStr1 );
            mpDoc->GetName( rEndPos2.Tab(), aStr2 );
            nComp = ScGlobal::GetCollator()->compareString( aStr1, aStr2 );
        }
        if (nComp < 0)
        {
            return true; // -1;
        }
        else if (nComp > 0)
        {
            return false; // 1;
        }

        // equal tabs
        if ( rEndPos1.Col() < rEndPos2.Col() )
            return true; // -1;
        if ( rEndPos1.Col() > rEndPos2.Col() )
            return false; // 1;
        // equal cols
        if ( rEndPos1.Row() < rEndPos2.Row() )
            return true; // -1;
        if ( rEndPos1.Row() > rEndPos2.Row() )
            return false; // 1;

        return false;
    }
private:
    ScDocument * const mpDoc;
};

void ScRangePairList::Join( const ScRangePair& r, bool bIsInList )
{
    if ( maPairs.empty() )
    {
        Append( r );
        return ;
    }

    bool bJoinedInput = false;
    const ScRangePair* pOver = &r;

Label_RangePair_Join:

    assert(pOver);
    const ScRange& r1 = pOver->GetRange(0);
    const ScRange& r2 = pOver->GetRange(1);
    const SCCOL nCol1 = r1.aStart.Col();
    const SCROW nRow1 = r1.aStart.Row();
    const SCTAB nTab1 = r1.aStart.Tab();
    const SCCOL nCol2 = r1.aEnd.Col();
    const SCROW nRow2 = r1.aEnd.Row();
    const SCTAB nTab2 = r1.aEnd.Tab();

    size_t nOverPos = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < maPairs.size(); ++i)
    {
        ScRangePair & rPair = maPairs[ i ];
        if ( &rPair == pOver )
        {
            nOverPos = i;
            continue;           // the same one, continue with the next
        }
        bool bJoined = false;
        ScRange& rp1 = rPair.GetRange(0);
        ScRange& rp2 = rPair.GetRange(1);
        if ( rp2 == r2 )
        {   // only if Range2 is equal
            if ( rp1.In( r1 ) )
            {   // RangePair pOver included in or identical to RangePair p
                if ( bIsInList )
                    bJoined = true;     // do away with RangePair pOver
                else
                {   // that was all then
                    bJoinedInput = true;    // don't append
                    break;  // for
                }
            }
            else if ( r1.In( rp1 ) )
            {   // RangePair p included in RangePair pOver, make pOver the new RangePair
                rPair = *pOver;
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
            {   // delete RangePair pOver within the list
                if (nOverPos != std::numeric_limits<size_t>::max())
                {
                    Remove(nOverPos);
                    if (nOverPos < i)
                        --i;
                }
                else
                {
                    for (size_t nOver = 0, nRangePairs = maPairs.size(); nOver < nRangePairs; ++nOver)
                    {
                        if (&maPairs[nOver] == pOver)
                        {
                            maPairs.erase(maPairs.begin() + nOver);
                            break;
                        }
                    }
                    assert(false);
                }
            }
            bJoinedInput = true;
            pOver = &maPairs[i];
            bIsInList = true;
            goto Label_RangePair_Join;
        }
    }
    if ( !bIsInList && !bJoinedInput )
        Append( r );
}

std::vector<const ScRangePair*> ScRangePairList::CreateNameSortedArray( ScDocument* pDoc ) const
{
    std::vector<const ScRangePair*> aSortedVec(maPairs.size());
    size_t i = 0;
    for ( auto const & rPair : maPairs)
    {
        aSortedVec[i++] = &rPair;
    }

    std::sort( aSortedVec.begin(), aSortedVec.end(), ScRangePairList_sortNameCompare(pDoc) );

    return aSortedVec;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
