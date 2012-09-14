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

#define SC_RANGELST_CXX         //fuer ICC

#include <stdlib.h>             // qsort
#include <comphelper/string.hxx>
#include <unotools/collatorwrapper.hxx>

#include "rangelst.hxx"
#include "document.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"
#include "compiler.hxx"
#include "stlalgorithm.hxx"

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
    FindIntersectingRange(const T& rTest) : mrTest(rTest) {}
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
    AppendToList(vector<ScRange*>& rRanges) : mrRanges(rRanges) {}
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

class FindDeletedRange : public ::std::unary_function<const ScRange*, bool>
{
public:
    FindDeletedRange( SCsCOL nDx, SCsROW nDy): mnDx(nDx), mnDy(nDy) {}
    FindDeletedRange( const FindDeletedRange& r) : mnDx(r.mnDx), mnDy(r.mnDy) {}
    bool operator() (const ScRange* p)
    {
        const ScAddress& rStart = p->aStart;
        const ScAddress& rEnd = p->aEnd;

        if( rEnd.Col() +mnDx < rStart.Col() )
            return true;
        if( rEnd.Row() + mnDy < rStart.Row() )
            return true;

        return false;
    }

private:
    SCsCOL mnDx;
    SCsROW mnDy;
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
        sal_uInt16 nTCount = comphelper::string::getTokenCount(rStr, cDelimiter);
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


void ScRangeList::Format( rtl::OUString& rStr, sal_uInt16 nFlags, ScDocument* pDoc,
                          formula::FormulaGrammar::AddressConvention eConv,
                          sal_Unicode cDelimiter ) const
{

    if (!cDelimiter)
        cDelimiter = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);

    String aStr;
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

    // We need to query the size of the container dynamically since its size
    // may change during the loop.
    for ( size_t i = 0; i < maRanges.size() && pOver; ++i )
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
                i--;
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

    // delete all entries that are fully deleted
    if( eUpdateRefMode == URM_INSDEL && (nDx < 0 || nDy < 0) )
    {
        vector<ScRange*>::iterator itr = std::remove_if(maRanges.begin(), maRanges.end(), FindDeletedRange(nDx, nDy));
        for_each(itr, maRanges.end(), ScDeleteObjectByPtr<ScRange>());
        maRanges.erase(itr, maRanges.end());
    }

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

namespace {
    //
        // r.aStart.X() <= p.aStart.X() && r.aEnd.X() >= p.aEnd.X()
        // && ( r.aStart.Y() <= p.aStart.Y() || r.aEnd.Y() >= r.aEnd.Y() )

template<typename X, typename Y>
bool checkForOneRange( X rStartX, X rEndX, Y rStartY, Y rEndY,
                    X pStartX, X pEndX, Y pStartY, Y pEndY )
{
    if( rStartX <= pStartX && rEndX >= pEndX
            && ( rStartY <= pStartY || rEndY >= pEndY ) )
        return true;

    return false;
}

bool handleOneRange( const ScRange& rDeleteRange, ScRange* p )
{
    ScAddress rDelStart = rDeleteRange.aStart;
    ScAddress rDelEnd = rDeleteRange.aEnd;
    ScAddress rPStart = p->aStart;
    ScAddress rPEnd = p->aEnd;
    if(checkForOneRange(rDelStart.Col(), rDelEnd.Col(), rDelStart.Row(), rDelEnd.Row(),
                rPStart.Col(), rPEnd.Col(), rPStart.Row(), rPEnd.Row()))
    {
        // X = Col
        // Y = Row
        if(rDelStart.Row() <= rPStart.Row())
        {
            p->aStart.SetRow(rDelEnd.Row()+1);
        }
        else if(rDelEnd.Row() >= rPEnd.Row())
        {
            p->aEnd.SetRow(rDelStart.Row()-1);
        }

        return true;
    }
    else if(checkForOneRange(rDelStart.Row(), rDelEnd.Row(), rDelStart.Col(), rDelEnd.Col(),
                rPStart.Row(), rPEnd.Row(), rPStart.Col(), rPEnd.Col()))
    {
        // X = Row
        // Y = Col
        if(rDelStart.Col() <= rPStart.Col())
            p->aStart.SetCol(rDelEnd.Col()+1);
        else if(rDelEnd.Col() >= rPEnd.Col())
            p->aEnd.SetCol(rDelStart.Col()-1);

        return true;
    }
    return false;
}

template<typename X, typename Y>
bool checkForTwoRangesCase2( X rStartX, X rEndX, Y rStartY, Y rEndY,
                    X pStartX, X pEndX, Y pStartY, Y pEndY )
{
    if(rStartY > pStartY && rStartX <= pStartX
            && rEndY < pEndY && rEndX >= pEndX)
        return true;

    return false;
}


bool handleTwoRanges( const ScRange& rDeleteRange, ScRange* p, std::vector<ScRange>& rNewRanges )
{
    ScAddress rDelStart = rDeleteRange.aStart;
    ScAddress rDelEnd = rDeleteRange.aEnd;
    ScAddress rPStart = p->aStart;
    ScAddress rPEnd = p->aEnd;
    SCCOL rStartCol = rDelStart.Col();
    SCCOL rEndCol = rDelEnd.Col();
    SCCOL pStartCol = rPStart.Col();
    SCCOL pEndCol = rPEnd.Col();
    SCROW rStartRow = rDelStart.Row();
    SCROW rEndRow = rDelEnd.Row();
    SCROW pStartRow = rPStart.Row();
    SCROW pEndRow = rPEnd.Row();
    SCTAB nTab = rPStart.Tab();
    if(rStartCol > pStartCol && rStartCol < pEndCol && rEndCol >= pEndCol)
    {
        if(rStartRow > pStartRow && rStartRow < pEndRow && rEndRow >= pEndRow)
        {
            ScRange aNewRange( pStartCol, rStartRow, nTab, rStartCol-1, pEndRow, nTab );
            rNewRanges.push_back(aNewRange);

            p->aEnd.SetRow(rStartRow -1);
            return true;
        }
        else if(rEndRow > pStartRow && rEndRow < pEndRow && rStartRow <= pStartRow)
        {
            ScRange aNewRange( rPStart, ScAddress( pStartCol -1, pEndRow, nTab ) );
            rNewRanges.push_back(aNewRange);

            p->aStart.SetRow(rEndRow+1);
            return true;
        }
    }
    else if(rEndCol > pStartCol && rEndCol < pEndCol && rStartCol <= pStartCol)
    {
        if(rStartRow > pStartRow && rStartRow < pEndRow)
        {
            ScRange aNewRange( ScAddress( rEndCol +1, rStartRow, nTab ), rPEnd );
            rNewRanges.push_back(aNewRange);

            p->aEnd.SetRow(rStartRow-1);
            return true;
        }
        else if(rEndRow > pStartRow && rEndRow < pEndRow)
        {
            ScRange aNewRange( rEndCol +1, pStartRow, nTab, rEndCol, rEndRow, nTab );
            rNewRanges.push_back(aNewRange);

            p->aStart.SetRow(rEndRow+1);
            return true;
        }
    }
    else if(checkForTwoRangesCase2(rDelStart.Col(), rDelEnd.Col(), rDelStart.Row(), rDelEnd.Row(),
                rPStart.Col(), rPEnd.Col(), rPStart.Row(), rPEnd.Row()))
    {
        ScRange aNewRange( rPStart, ScAddress( rPEnd.Col(), rDelStart.Row() -1, nTab ) );
        rNewRanges.push_back(aNewRange);

        p->aStart.SetRow(rEndRow+1);
        return true;
    }
    else if(checkForTwoRangesCase2(rDelStart.Row(), rDelEnd.Row(), rDelStart.Col(), rDelEnd.Col(),
                rPStart.Row(), rPEnd.Row(), rPStart.Col(), rPEnd.Col()))
    {
        ScRange aNewRange( rPStart, ScAddress( rDelStart.Col() -1, rPEnd.Row(), nTab ) );
        rNewRanges.push_back(aNewRange);

        p->aStart.SetCol(rEndCol+1);
        return true;
    }

    return false;
}

        // r.aStart.X() > p.aStart.X() && r.aEnd.X() >= p.aEnd.X()
        // && r.aStart.Y() > p.aStart.Y() && r.aEnd.Y() < p.aEnd.Y()
        // or
        // r.aStart.X() <= p.aStart.X() && r.aEnd.X() < p.aEnd.X()
        // && r.aStart.Y() > p.aStart.Y() && r.aEnd.Y() < p.aEnd.Y()
template<typename X, typename Y>
bool checkForThreeRanges( X rStartX, X rEndX, Y rStartY, Y rEndY,
                                X pStartX, X pEndX, Y pStartY, Y pEndY )
{
    if(rStartX > pStartX && rEndX >= pEndX
            && rStartY > pStartY && rEndY < pEndY )
        return true;
    else if( rStartX <= pStartX && rEndX < pEndX
            && rStartY > pStartY && rEndY < pEndY )
        return true;

    return false;
}

bool handleThreeRanges( const ScRange& rDeleteRange, ScRange* p, std::vector<ScRange>& rNewRanges )
{
    ScAddress rDelStart = rDeleteRange.aStart;
    ScAddress rDelEnd = rDeleteRange.aEnd;
    ScAddress rPStart = p->aStart;
    ScAddress rPEnd = p->aEnd;
    SCTAB nTab = rDelStart.Tab();
    if(checkForThreeRanges(rDelStart.Col(), rDelEnd.Col(), rDelStart.Row(), rDelEnd.Row(),
                rPStart.Col(), rPEnd.Col(), rPStart.Row(), rPEnd.Row()))
    {
        if(rDelStart.Col() > rPStart.Col())
        {
            SCCOL nCol1 = rDelStart.Col();

            ScRange aNewRange( nCol1, rPStart.Row(), nTab, rPEnd.Col(), rDelStart.Row()-1, nTab);
            rNewRanges.push_back(aNewRange);

            aNewRange = ScRange( ScAddress(nCol1, rDelEnd.Row()+1, nTab), rPEnd);
            rNewRanges.push_back(aNewRange);

            p->aEnd.SetCol(nCol1-1);
        }
        else
        {
            SCCOL nCol1 = rDelEnd.Col();

            ScRange aNewRange( rPStart, ScAddress( nCol1 - 1, rDelStart.Row() -1, nTab ) );
            rNewRanges.push_back(aNewRange);

            aNewRange = ScRange( rPStart.Col(), rDelEnd.Row() + 1, nTab, rDelEnd.Col() +1, rPEnd.Row(), nTab );
            rNewRanges.push_back(aNewRange);

            p->aStart.SetCol(nCol1+1);
        }
        return true;
    }
    else if(checkForThreeRanges(rDelStart.Row(), rDelEnd.Row(), rDelStart.Col(), rDelEnd.Col(),
                rPStart.Row(), rPEnd.Row(), rPStart.Col(), rPEnd.Col()))
    {
        if(rDelStart.Row() > rPStart.Row())
        {
            SCROW nRow1 = rDelStart.Row();

            ScRange aNewRange( rPStart.Col(), nRow1, nTab, rDelStart.Col() -1, rPEnd.Row(), nTab );
            rNewRanges.push_back( aNewRange );

            aNewRange = ScRange( ScAddress(rDelEnd.Col() +1, nRow1, nTab), rPEnd );
            rNewRanges.push_back( aNewRange );

            p->aEnd.SetRow(nRow1-1);
        }
        else
        {
            SCROW nRow1 = rDelEnd.Row();

            ScRange aNewRange( rPStart, ScAddress( rDelStart.Col() -1, nRow1, nTab ) );
            rNewRanges.push_back(aNewRange);

            aNewRange = ScRange( rDelEnd.Col() +1, rPStart.Col(), nTab, rPEnd.Col(), nRow1, nTab );
            rNewRanges.push_back( aNewRange );

            p->aStart.SetRow(nRow1+1);
        }
        return true;
    }

    return false;
}

bool handleFourRanges( const ScRange& rDelRange, ScRange* p, std::vector<ScRange>& rNewRanges )
{
    ScAddress rDelStart = rDelRange.aStart;
    ScAddress rDelEnd = rDelRange.aEnd;
    ScAddress rPStart = p->aStart;
    ScAddress rPEnd = p->aEnd;
    if( rDelRange.aStart.Col() > p->aStart.Col() && rDelRange.aEnd.Col() < p->aEnd.Col()
            && rDelRange.aStart.Row() > p->aStart.Row() && rDelRange.aEnd.Row() < p->aEnd.Row() )
    {
        SCTAB nTab = rDelStart.Tab();

        ScRange aNewRange( ScAddress( rPStart.Col(), rDelEnd.Row(), nTab ), rDelEnd );
        rNewRanges.push_back( aNewRange );

        aNewRange = ScRange( rPStart.Col(), rDelStart.Row(), nTab, rDelStart.Col() -1, rDelEnd.Row(), nTab );
        rNewRanges.push_back( aNewRange );

        aNewRange = ScRange( rDelEnd.Col() +1, rDelStart.Row(), nTab, rPEnd.Col(), rDelEnd.Row(), nTab );
        rNewRanges.push_back( aNewRange );

        rPEnd.SetRow(rDelStart.Row()-1);
        p->aEnd = rPEnd;

        return true;
    }

    return false;
}

}

void ScRangeList::DeleteArea( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2 )
{
    ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    iterator itrDel = std::remove_if(maRanges.begin(), maRanges.end(), FindRangeIn<ScRange>(aRange));
    for_each(itrDel, maRanges.end(), ScDeleteObjectByPtr<ScRange>());
    maRanges.erase(itrDel, maRanges.end());

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
            continue;

        // getting two ranges
        // r.aStart.X()
        else if(handleTwoRanges( aRange, *itr, aNewRanges ))
            continue;

        // getting 3 ranges
        // r.aStart.X() > p.aStart.X() && r.aEnd.X() >= p.aEnd.X()
        // && r.aStart.Y() > p.aStart.Y() && r.aEnd.Y() < p.aEnd.Y()
        // or
        // r.aStart.X() <= p.aStart.X() && r.aEnd.X() < p.aEnd.X()
        // && r.aStart.Y() > p.aStart.Y() && r.aEnd.Y() < p.aEnd.Y()
        else if(handleThreeRanges( aRange, *itr, aNewRanges ))
            continue;

        // getting 4 ranges
        // r.aStart.X() > p.aStart.X() && r.aEnd().X() < p.aEnd.X()
        // && r.aStart.Y() > p.aStart.Y() && r.aEnd().Y() < p.aEnd.Y()
        else if(handleFourRanges( aRange, *itr, aNewRanges ))
            continue;
    }
    for(vector<ScRange>::iterator itr = aNewRanges.begin(); itr != aNewRanges.end(); ++itr)
        Join( *itr, false);
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

ScRangeList::ScRangeList() {}

ScRangeList::ScRangeList( const ScRangeList& rList ) :
    SvRefBase()
{
    maRanges.reserve(rList.maRanges.size());
    for_each(rList.maRanges.begin(), rList.maRanges.end(), AppendToList(maRanges));
}

ScRangeList::ScRangeList( const ScRange& rRange )
{
    maRanges.reserve(1);
    Append(rRange);
}

ScRangeList& ScRangeList::operator=(const ScRangeList& rList)
{
    RemoveAll();
    maRanges.reserve(rList.maRanges.size());
    for_each(rList.maRanges.begin(), rList.maRanges.end(), AppendToList(maRanges));
    return *this;
}

void ScRangeList::Append( const ScRange& rRange )
{
    ScRange* pR = new ScRange( rRange );
    maRanges.push_back( pR );
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
    for_each(maRanges.begin(), maRanges.end(), ScDeleteObjectByPtr<ScRange>());
    maRanges.clear();
}

ScRange ScRangeList::Combine() const
{
    if (maRanges.empty())
        return ScRange();

    vector<ScRange*>::const_iterator itr = maRanges.begin(), itrEnd = maRanges.end();
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
}

// === ScRangePairList ========================================================

ScRangePairList::~ScRangePairList()
{
    for_each( maPairs.begin(), maPairs.end(), ScDeleteObjectByPtr<ScRangePair>() );
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
extern "C"
int SAL_CALL ScRangePairList_QsortNameCompare( const void* p1, const void* p2 )
{
    const ScRangePairNameSort* ps1 = (const ScRangePairNameSort*)p1;
    const ScRangePairNameSort* ps2 = (const ScRangePairNameSort*)p2;
    const ScAddress& rStartPos1 = ps1->pPair->GetRange(0).aStart;
    const ScAddress& rStartPos2 = ps2->pPair->GetRange(0).aStart;
    rtl::OUString aStr1, aStr2;
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
    }
#ifndef _MSC_VER // MSVC is good enough to warn about unreachable code here.
                 // Or stupid enough to bother warning about it, depending
                 // on your point of view.
    return 0; // just in case
#endif
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

    for ( size_t i = 0; i < maPairs.size() && pOver; ++i )
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
                i--;
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
    OSL_ENSURE( nListCount * sizeof(ScRangePairNameSort) <= (size_t)~0x1F,
        "ScRangePairList::CreateNameSortedArray nListCount * sizeof(ScRangePairNameSort) > (size_t)~0x1F" );
    ScRangePairNameSort* pSortArray = (ScRangePairNameSort*)
        new sal_uInt8 [ nListCount * sizeof(ScRangePairNameSort) ];
    sal_uLong j;
    for ( j=0; j < nListCount; j++ )
    {
        pSortArray[j].pPair = maPairs[ j ];
        pSortArray[j].pDoc = pDoc;
    }
    qsort( (void*)pSortArray, nListCount, sizeof(ScRangePairNameSort), &ScRangePairList_QsortNameCompare );
    // ScRangePair Pointer aufruecken
    ScRangePair** ppSortArray = (ScRangePair**)pSortArray;
    for ( j=0; j < nListCount; j++ )
    {
        ppSortArray[j] = pSortArray[j].pPair;
    }
    return ppSortArray;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
