/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rangelst.cxx,v $
 * $Revision: 1.11.32.3 $
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


// === ScRangeList ====================================================

ScRangeList::~ScRangeList()
{
    for ( ScRangePtr pR = First(); pR; pR = Next() )
        delete pR;
}

void ScRangeList::RemoveAll()
{
    for ( ScRangePtr pR = First(); pR; pR = Next() )
        delete pR;
    Clear();
}

static void defaultDelimiter( char& cDelimiter, formula::FormulaGrammar::AddressConvention eConv)
{
    if( cDelimiter == 0)
    {
        switch( eConv )
        {
        default :
        case formula::FormulaGrammar::CONV_OOO :
            cDelimiter = ';';
            break;

        case formula::FormulaGrammar::CONV_XL_A1 :
        case formula::FormulaGrammar::CONV_XL_R1C1 :
            cDelimiter = ',';
            break;
        }
    }
}

USHORT ScRangeList::Parse( const String& rStr, ScDocument* pDoc, USHORT nMask,
                           formula::FormulaGrammar::AddressConvention eConv,
                           char cDelimiter )
{
    if ( rStr.Len() )
    {
        defaultDelimiter( cDelimiter, eConv);

        nMask |= SCA_VALID;             // falls das jemand vergessen sollte
        USHORT nResult = (USHORT)~0;    // alle Bits setzen
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
        USHORT nTCount = rStr.GetTokenCount( cDelimiter );
        for ( USHORT i=0; i<nTCount; i++ )
        {
            aOne = rStr.GetToken( i, cDelimiter );
            // FIXME : broken for Lotus
            if ( aOne.Search( ':' ) == STRING_NOTFOUND )
            {   // Range muss es sein
                String aStrTmp( aOne );
                aOne += ':';
                aOne += aStrTmp;
            }
            aRange.aStart.SetTab( nTab );   // Default Tab wenn nicht angegeben
            USHORT nRes = aRange.Parse( aOne, pDoc, eConv );
            if ( (nRes & nMask) == nMask )
                Append( aRange );
            nResult &= nRes;        // alle gemeinsamen Bits bleiben erhalten
        }
        return nResult;             // SCA_VALID gesetzt wenn alle ok
    }
    else
        return 0;
}


void ScRangeList::Format( String& rStr, USHORT nFlags, ScDocument* pDoc,
                          formula::FormulaGrammar::AddressConvention eConv,
                          char cDelimiter ) const
{
    rStr.Erase();

    defaultDelimiter( cDelimiter, eConv);

    ULONG nCnt = Count();
    for ( ULONG nIdx = 0; nIdx < nCnt; nIdx++ )
    {
        String aStr;
        GetObject( nIdx )->Format( aStr, nFlags, pDoc, eConv );
        if ( nIdx )
            rStr += cDelimiter;
        rStr += aStr;
    }
}


void ScRangeList::Join( const ScRange& r, BOOL bIsInList )
{
    if ( !Count() )
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
    ScRangePtr pOver = (ScRangePtr) &r;     // fies aber wahr wenn bInList
    ULONG nOldPos = 0;
    if ( bIsInList )
    {   // merken um ggbf. zu loeschen bzw. wiederherzustellen
        nOldPos = GetPos( pOver );
    }
    BOOL bJoinedInput = FALSE;
    for ( ScRangePtr p = First(); p && pOver; p = Next() )
    {
        if ( p == pOver )
            continue;           // derselbe, weiter mit dem naechsten
        BOOL bJoined = FALSE;
        if ( p->In( r ) )
        {   // Range r in Range p enthalten oder identisch
            if ( bIsInList )
                bJoined = TRUE;     // weg mit Range r
            else
            {   // das war's dann
                bJoinedInput = TRUE;    // nicht anhaengen
                break;  // for
            }
        }
        else if ( r.In( *p ) )
        {   // Range p in Range r enthalten, r zum neuen Range machen
            *p = r;
            bJoined = TRUE;
        }
        if ( !bJoined && p->aStart.Tab() == nTab1 && p->aEnd.Tab() == nTab2 )
        {   // 2D
            if ( p->aStart.Col() == nCol1 && p->aEnd.Col() == nCol2 )
            {
                if ( p->aStart.Row() == nRow2+1 )
                {   // oben
                    p->aStart.SetRow( nRow1 );
                    bJoined = TRUE;
                }
                else if ( p->aEnd.Row() == nRow1-1 )
                {   // unten
                    p->aEnd.SetRow( nRow2 );
                    bJoined = TRUE;
                }
            }
            else if ( p->aStart.Row() == nRow1 && p->aEnd.Row() == nRow2 )
            {
                if ( p->aStart.Col() == nCol2+1 )
                {   // links
                    p->aStart.SetCol( nCol1 );
                    bJoined = TRUE;
                }
                else if ( p->aEnd.Col() == nCol1-1 )
                {   // rechts
                    p->aEnd.SetCol( nCol2 );
                    bJoined = TRUE;
                }
            }
        }
        if ( bJoined )
        {
            if ( bIsInList )
            {   // innerhalb der Liste Range loeschen
                Remove( nOldPos );
                delete pOver;
                pOver = NULL;
                if ( nOldPos )
                    nOldPos--;          // Seek richtig aufsetzen
            }
            bJoinedInput = TRUE;
            Join( *p, TRUE );           // rekursiv!
        }
    }
    if ( bIsInList )
        Seek( nOldPos );
    else if ( !bJoinedInput )
        Append( r );
}


BOOL ScRangeList::operator==( const ScRangeList& r ) const
{
    if ( this == &r )
        return TRUE;                // identische Referenz
    if ( Count() != r.Count() )
        return FALSE;
    ULONG nCnt = Count();
    for ( ULONG nIdx = 0; nIdx < nCnt; nIdx++ )
    {
        if ( *GetObject( nIdx ) != *r.GetObject( nIdx ) )
            return FALSE;           // auch andere Reihenfolge ist ungleich
    }
    return TRUE;
}

BOOL ScRangeList::UpdateReference( UpdateRefMode eUpdateRefMode,
                                    ScDocument* pDoc, const ScRange& rWhere,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    BOOL bChanged = FALSE;
    if ( Count() )
    {
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
        rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        for ( ScRange* pR = First(); pR; pR = Next() )
        {
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
                bChanged = TRUE;
                pR->aStart.Set( theCol1, theRow1, theTab1 );
                pR->aEnd.Set( theCol2, theRow2, theTab2 );
            }
        }
    }
    return bChanged;
}


ScRange* ScRangeList::Find( const ScAddress& rAdr ) const
{
    ULONG nListCount = Count();
    for ( ULONG j = 0; j < nListCount; j++ )
    {
        ScRange* pR = GetObject( j );
        if ( pR->In( rAdr ) )
            return pR;
    }
    return NULL;
}


ScRangeList::ScRangeList( const ScRangeList& rList ) :
    ScRangeListBase(),
    SvRefBase()
{
    ULONG nListCount = rList.Count();
    for ( ULONG j = 0; j < nListCount; j++ )
        Append( *rList.GetObject( j ) );
}


ScRangeList& ScRangeList::operator=(const ScRangeList& rList)
{
    RemoveAll();

    ULONG nListCount = rList.Count();
    for ( ULONG j = 0; j < nListCount; j++ )
        Append( *rList.GetObject( j ) );

    return *this;
}


BOOL ScRangeList::Intersects( const ScRange& rRange ) const
{
    ULONG nListCount = Count();
    for ( ULONG j = 0; j < nListCount; j++ )
        if ( GetObject(j)->Intersects( rRange ) )
            return TRUE;

    return FALSE;
}


BOOL ScRangeList::In( const ScRange& rRange ) const
{
    ULONG nListCount = Count();
    for ( ULONG j = 0; j < nListCount; j++ )
        if ( GetObject(j)->In( rRange ) )
            return TRUE;

    return FALSE;
}


ULONG ScRangeList::GetCellCount() const
{
    ULONG nCellCount = 0;
    ULONG nListCount = Count();
    for ( ULONG j = 0; j < nListCount; j++ )
    {
        ScRange* pR = GetObject( j );
        nCellCount += ULONG(pR->aEnd.Col() - pR->aStart.Col() + 1)
            * ULONG(pR->aEnd.Row() - pR->aStart.Row() + 1)
            * ULONG(pR->aEnd.Tab() - pR->aStart.Tab() + 1);
    }
    return nCellCount;
}


// === ScRangePairList ====================================================

ScRangePairList::~ScRangePairList()
{
    for ( ScRangePair* pR = First(); pR; pR = Next() )
        delete pR;
}


void ScRangePairList::Join( const ScRangePair& r, BOOL bIsInList )
{
    if ( !Count() )
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
    ULONG nOldPos = 0;
    if ( bIsInList )
    {   // merken um ggbf. zu loeschen bzw. wiederherzustellen
        nOldPos = GetPos( pOver );
    }
    BOOL bJoinedInput = FALSE;
    for ( ScRangePair* p = First(); p && pOver; p = Next() )
    {
        if ( p == pOver )
            continue;           // derselbe, weiter mit dem naechsten
        BOOL bJoined = FALSE;
        ScRange& rp1 = p->GetRange(0);
        ScRange& rp2 = p->GetRange(1);
        if ( rp2 == r2 )
        {   // nur wenn Range2 gleich ist
            if ( rp1.In( r1 ) )
            {   // RangePair r in RangePair p enthalten oder identisch
                if ( bIsInList )
                    bJoined = TRUE;     // weg mit RangePair r
                else
                {   // das war's dann
                    bJoinedInput = TRUE;    // nicht anhaengen
                    break;  // for
                }
            }
            else if ( r1.In( rp1 ) )
            {   // RangePair p in RangePair r enthalten, r zum neuen RangePair machen
                *p = r;
                bJoined = TRUE;
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
                    bJoined = TRUE;
                }
                else if ( rp1.aEnd.Row() == nRow1-1
                  && rp2.aEnd.Row() == r2.aStart.Row()-1 )
                {   // unten
                    rp1.aEnd.SetRow( nRow2 );
                    rp2.aEnd.SetRow( r2.aEnd.Row() );
                    bJoined = TRUE;
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
                    bJoined = TRUE;
                }
                else if ( rp1.aEnd.Col() == nCol1-1
                  && rp2.aEnd.Col() == r2.aEnd.Col()-1 )
                {   // rechts
                    rp1.aEnd.SetCol( nCol2 );
                    rp2.aEnd.SetCol( r2.aEnd.Col() );
                    bJoined = TRUE;
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
            bJoinedInput = TRUE;
            Join( *p, TRUE );           // rekursiv!
        }
    }
    if ( bIsInList )
        Seek( nOldPos );
    else if ( !bJoinedInput )
        Append( r );
}


BOOL ScRangePairList::operator==( const ScRangePairList& r ) const
{
    if ( this == &r )
        return TRUE;                // identische Referenz
    if ( Count() != r.Count() )
        return FALSE;
    ULONG nCnt = Count();
    for ( ULONG nIdx = 0; nIdx < nCnt; nIdx++ )
    {
        if ( *GetObject( nIdx ) != *r.GetObject( nIdx ) )
            return FALSE;           // auch andere Reihenfolge ist ungleich
    }
    return TRUE;
}


BOOL ScRangePairList::UpdateReference( UpdateRefMode eUpdateRefMode,
                                    ScDocument* pDoc, const ScRange& rWhere,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    BOOL bChanged = FALSE;
    if ( Count() )
    {
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
        rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        for ( ScRangePair* pR = First(); pR; pR = Next() )
        {
            for ( USHORT j=0; j<2; j++ )
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
                    bChanged = TRUE;
                    rRange.aStart.Set( theCol1, theRow1, theTab1 );
                    rRange.aEnd.Set( theCol2, theRow2, theTab2 );
                }
            }
        }
    }
    return bChanged;
}


void ScRangePairList::DeleteOnTab( SCTAB nTab )
{
    // Delete entries that have the labels (first range) on nTab

    ULONG nListCount = Count();
    ULONG nPos = 0;
    while ( nPos < nListCount )
    {
        ScRangePair* pR = GetObject( nPos );
        ScRange aRange = pR->GetRange(0);
        if ( aRange.aStart.Tab() == nTab && aRange.aEnd.Tab() == nTab )
        {
            Remove( nPos );
            delete pR;
            nListCount = Count();
        }
        else
            ++nPos;
    }
}


ScRangePair* ScRangePairList::Find( const ScAddress& rAdr ) const
{
    ULONG nListCount = Count();
    for ( ULONG j = 0; j < nListCount; j++ )
    {
        ScRangePair* pR = GetObject( j );
        if ( pR->GetRange(0).In( rAdr ) )
            return pR;
    }
    return NULL;
}


ScRangePair* ScRangePairList::Find( const ScRange& rRange ) const
{
    ULONG nListCount = Count();
    for ( ULONG j = 0; j < nListCount; j++ )
    {
        ScRangePair* pR = GetObject( j );
        if ( pR->GetRange(0) == rRange )
            return pR;
    }
    return NULL;
}


ScRangePairList* ScRangePairList::Clone() const
{
    ScRangePairList* pNew = new ScRangePairList;
    ULONG nListCount = Count();
    for ( ULONG j = 0; j < nListCount; j++ )
    {
        pNew->Append( *GetObject( j ) );
    }
    return pNew;
}


struct ScRangePairNameSort
{
    ScRangePair*    pPair;
    ScDocument*     pDoc;
};


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


ScRangePair** ScRangePairList::CreateNameSortedArray( ULONG& nListCount,
        ScDocument* pDoc ) const
{
    nListCount = Count();
    DBG_ASSERT( nListCount * sizeof(ScRangePairNameSort) <= (size_t)~0x1F,
        "ScRangePairList::CreateNameSortedArray nListCount * sizeof(ScRangePairNameSort) > (size_t)~0x1F" );
    ScRangePairNameSort* pSortArray = (ScRangePairNameSort*)
        new BYTE [ nListCount * sizeof(ScRangePairNameSort) ];
    ULONG j;
    for ( j=0; j < nListCount; j++ )
    {
        pSortArray[j].pPair = GetObject( j );
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




