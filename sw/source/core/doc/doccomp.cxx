/*************************************************************************
 *
 *  $RCSfile: doccomp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_FONTTYPE_HXX //autogen
#include <vcl/fonttype.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif

#ifdef JP_DUMP
#include <stdio.h>
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif



class CompareLine
{
public:
    CompareLine() {}
    virtual ~CompareLine();

    virtual ULONG GetHashValue() const = 0;
    virtual BOOL Compare( const CompareLine& rLine ) const = 0;
};

DECLARE_LIST( CompareList, CompareLine* )

class CompareData
{
    ULONG* pIndex;
    BOOL* pChangedFlag;

protected:
    CompareList aLines;
    ULONG nSttLineNum;

    // Anfang und Ende beschneiden und alle anderen in das
    // LinesArray setzen
    virtual void CheckRanges( CompareData& ) = 0;

public:
    CompareData();
    virtual ~CompareData();

    // gibt es unterschiede?
    BOOL HasDiffs( const CompareData& rData ) const;

    // startet das Vergleichen und Erzeugen der Unterschiede zweier
    // Dokumente
    void CompareLines( CompareData& rData );
    // lasse die Unterschiede anzeigen - ruft die beiden Methoden
    // ShowInsert / ShowDelete. Diese bekommen die Start und EndLine-Nummer
    // uebergeben. Die Abbildung auf den tatsaechline Inhalt muss die
    // Ableitung uebernehmen!
    ULONG ShowDiffs( const CompareData& rData );

    virtual void ShowInsert( ULONG nStt, ULONG nEnd );
    virtual void ShowDelete( const CompareData& rData, ULONG nStt,
                                ULONG nEnd, ULONG nInsPos );
    virtual void CheckForChangesInLine( const CompareData& rData,
                                    ULONG& nStt, ULONG& nEnd,
                                    ULONG& nThisStt, ULONG& nThisEnd );

    // Eindeutigen Index fuer eine Line setzen. Gleiche Lines haben den
    // selben Index; auch in den anderen CompareData!
    void SetIndex( ULONG nLine, ULONG nIndex );
    ULONG GetIndex( ULONG nLine ) const
        { return nLine < aLines.Count() ? pIndex[ nLine ] : 0; }

    // setze/erfrage ob eine Zeile veraendert ist
    void SetChanged( ULONG nLine, BOOL bFlag = TRUE );
    BOOL GetChanged( ULONG nLine ) const
        {
            return (pChangedFlag && nLine < aLines.Count())
                ? pChangedFlag[ nLine ]
                : 0;
        }

    ULONG GetLineCount() const      { return aLines.Count(); }
    ULONG GetLineOffset() const     { return nSttLineNum; }
    const CompareLine* GetLine( ULONG nLine ) const
            { return aLines.GetObject( nLine ); }
    void InsertLine( CompareLine* pLine )
        { aLines.Insert( pLine, LIST_APPEND ); }

#ifdef JP_DUMP
    // zum Debuggen!
    virtual void Dump();
#endif
};

class Hash
{
    struct _HashData
    {
        ULONG nNext, nHash;
        const CompareLine* pLine;

        _HashData()
            : nNext( 0 ), nHash( 0 ), pLine(0) {}
    };

    ULONG* pHashArr;
    _HashData* pDataArr;
    ULONG nCount, nPrime;

public:
    Hash( ULONG nSize );
    ~Hash();

    void CalcHashValue( CompareData& rData );

    ULONG GetCount() const { return nCount; }
};

class Compare
{
public:
    class MovedData
    {
        ULONG* pIndex;
        ULONG* pLineNum;
        ULONG nCount;

    public:
        MovedData( CompareData& rData, sal_Char* pDiscard );
        ~MovedData();

        ULONG GetIndex( ULONG n ) const { return pIndex[ n ]; }
        ULONG GetLineNum( ULONG n ) const { return pLineNum[ n ]; }
        ULONG GetCount() const { return nCount; }
    };

private:
    // Suche die verschobenen Lines
    class CompareSequence
    {
        CompareData &rData1, &rData2;
        const MovedData &rMoved1, &rMoved2;
        long *pMemory, *pFDiag, *pBDiag;

        void Compare( ULONG nStt1, ULONG nEnd1, ULONG nStt2, ULONG nEnd2 );
        ULONG CheckDiag( ULONG nStt1, ULONG nEnd1,
                        ULONG nStt2, ULONG nEnd2, ULONG* pCost );
    public:
        CompareSequence( CompareData& rData1, CompareData& rData2,
                        const MovedData& rD1, const MovedData& rD2 );
        ~CompareSequence();
    };


    static void CountDifference( const CompareData& rData, ULONG* pCounts );
    static void SetDiscard( const CompareData& rData,
                            sal_Char* pDiscard, ULONG* pCounts );
    static void CheckDiscard( ULONG nLen, sal_Char* pDiscard );
    static ULONG SetChangedFlag( CompareData& rData, sal_Char* pDiscard, int bFirst );
    static void ShiftBoundaries( CompareData& rData1, CompareData& rData2 );

public:
    Compare( ULONG nDiff, CompareData& rData1, CompareData& rData2 );
};

// ====================================================================

CompareLine::~CompareLine() {}

// ----------------------------------------------------------------------

CompareData::CompareData()
    : nSttLineNum( 0 ), pIndex( 0 ), pChangedFlag( 0 )
{
}

CompareData::~CompareData()
{
    delete pIndex;
    delete pChangedFlag;
}

void CompareData::SetIndex( ULONG nLine, ULONG nIndex )
{
    if( !pIndex )
    {
        pIndex = new ULONG[ aLines.Count() ];
        memset( pIndex, 0, aLines.Count() * sizeof( ULONG ) );
    }
    if( nLine < aLines.Count() )
        pIndex[ nLine ] = nIndex;
}

#ifdef JP_DUMP
void CompareData::Dump()
{
}
#endif

void CompareData::SetChanged( ULONG nLine, BOOL bFlag )
{
    if( !pChangedFlag )
    {
        pChangedFlag = new BOOL[ aLines.Count() +1 ];
        memset( pChangedFlag, 0, aLines.Count() +1 * sizeof( BOOL ) );
    }
    if( nLine < aLines.Count() )
        pChangedFlag[ nLine ] = bFlag;
}

void CompareData::CompareLines( CompareData& rData )
{
    CheckRanges( rData );

    ULONG nDifferent;
    {
        Hash aH( GetLineCount() + rData.GetLineCount() + 1 );
        aH.CalcHashValue( *this );
        aH.CalcHashValue( rData );
        nDifferent = aH.GetCount();
    }
    {
        Compare aComp( nDifferent, *this, rData );
    }
}

ULONG CompareData::ShowDiffs( const CompareData& rData )
{
    ULONG nLen1 = rData.GetLineCount(), nLen2 = GetLineCount();
    ULONG nStt1 = 0, nStt2 = 0;
    ULONG nCnt = 0;

    while( nStt1 < nLen1 || nStt2 < nLen2 )
    {
        if( rData.GetChanged( nStt1 ) || GetChanged( nStt2 ) )
        {
            ULONG nSav1 = nStt1, nSav2 = nStt2;
            while( nStt1 < nLen1 && rData.GetChanged( nStt1 )) ++nStt1;
            while( nStt2 < nLen2 && GetChanged( nStt2 )) ++nStt2;

            // rData ist das Original,
            // this ist das, in das die Veraenderungen sollen
            if( nSav2 != nStt2 && nSav1 != nStt1 )
                CheckForChangesInLine( rData, nSav1, nStt1, nSav2, nStt2 );

            if( nSav2 != nStt2 )
                ShowInsert( nSav2, nStt2 );

            if( nSav1 != nStt1 )
                ShowDelete( rData, nSav1, nStt1, nStt2 );
            ++nCnt;
        }
        ++nStt1, ++nStt2;
    }
    return nCnt;
}

BOOL CompareData::HasDiffs( const CompareData& rData ) const
{
    BOOL bRet = FALSE;
    ULONG nLen1 = rData.GetLineCount(), nLen2 = GetLineCount();
    ULONG nStt1 = 0, nStt2 = 0;

    while( nStt1 < nLen1 || nStt2 < nLen2 )
    {
        if( rData.GetChanged( nStt1 ) || GetChanged( nStt2 ) )
        {
            bRet = TRUE;
            break;
        }
        ++nStt1, ++nStt2;
    }
    return bRet;
}

void CompareData::ShowInsert( ULONG nStt, ULONG nEnd )
{
}

void CompareData::ShowDelete( const CompareData& rData, ULONG nStt,
                                ULONG nEnd, ULONG nInsPos )
{
}

void CompareData::CheckForChangesInLine( const CompareData& ,
                                    ULONG&, ULONG&, ULONG&, ULONG& )
{
}

// ----------------------------------------------------------------------

Hash::Hash( ULONG nSize )
    : nCount( 1 )
{

static const ULONG primes[] =
{
  509,
  1021,
  2039,
  4093,
  8191,
  16381,
  32749,
  65521,
  131071,
  262139,
  524287,
  1048573,
  2097143,
  4194301,
  8388593,
  16777213,
  33554393,
  67108859,         /* Preposterously large . . . */
  134217689,
  268435399,
  536870909,
  1073741789,
  2147483647,
  0
};

    pDataArr = new _HashData[ nSize ];
    pDataArr[0].nNext = 0;
    pDataArr[0].nHash = 0,
    pDataArr[0].pLine = 0;

    for( int i = 0; primes[i] < nSize / 3;  i++)
        if( !primes[i] )
        {
            pHashArr = 0;
            return;
        }
    nPrime = primes[ i ];
    pHashArr = new ULONG[ nPrime ];
    memset( pHashArr, 0, nPrime * sizeof( ULONG ) );
}

Hash::~Hash()
{
    delete pHashArr;
    delete pDataArr;
}

void Hash::CalcHashValue( CompareData& rData )
{
    if( pHashArr )
    {
        for( ULONG n = 0; n < rData.GetLineCount(); ++n )
        {
            const CompareLine* pLine = rData.GetLine( n );
            ASSERT( pLine, "wo ist die Line?" );
            ULONG nH = pLine->GetHashValue();

            ULONG* pFound = &pHashArr[ nH % nPrime ];
            for( ULONG i = *pFound;  ;  i = pDataArr[i].nNext )
                if( !i )
                {
                    i = nCount++;
                    pDataArr[i].nNext = *pFound;
                    pDataArr[i].nHash = nH;
                    pDataArr[i].pLine = pLine;
                    *pFound = i;
                    break;
                }
                else if( pDataArr[i].nHash == nH &&
                        pDataArr[i].pLine->Compare( *pLine ))
                    break;

            rData.SetIndex( n, i );
        }
    }
}

// ----------------------------------------------------------------------

Compare::Compare( ULONG nDiff, CompareData& rData1, CompareData& rData2 )
{
    MovedData *pMD1, *pMD2;
    // Suche die unterschiedlichen Lines
    {
        sal_Char* pDiscard1 = new sal_Char[ rData1.GetLineCount() ];
        sal_Char* pDiscard2 = new sal_Char[ rData2.GetLineCount() ];

        ULONG* pCount1 = new ULONG[ nDiff ];
        ULONG* pCount2 = new ULONG[ nDiff ];
        memset( pCount1, 0, nDiff * sizeof( ULONG ));
        memset( pCount2, 0, nDiff * sizeof( ULONG ));

        // stelle fest, welche Indizies in den CompareData mehrfach vergeben wurden
        CountDifference( rData1, pCount1 );
        CountDifference( rData2, pCount2 );

        // alle die jetzt nur einmal vorhanden sind, sind eingefuegt oder
        // geloescht worden. Alle die im anderen auch vorhanden sind, sind
        // verschoben worden
        SetDiscard( rData1, pDiscard1, pCount2 );
        SetDiscard( rData2, pDiscard2, pCount1 );

        // die Arrays koennen wir wieder vergessen
        delete pCount1; delete pCount2;

        CheckDiscard( rData1.GetLineCount(), pDiscard1 );
        CheckDiscard( rData2.GetLineCount(), pDiscard2 );

        pMD1 = new MovedData( rData1, pDiscard1 );
        pMD2 = new MovedData( rData2, pDiscard2 );

        // die Arrays koennen wir wieder vergessen
        delete pDiscard1; delete pDiscard2;
    }

    {
        CompareSequence aTmp( rData1, rData2, *pMD1, *pMD2 );
    }

    ShiftBoundaries( rData1, rData2 );

    delete pMD1;
    delete pMD2;
}



void Compare::CountDifference( const CompareData& rData, ULONG* pCounts )
{
    ULONG nLen = rData.GetLineCount();
    for( ULONG n = 0; n < nLen; ++n )
    {
        ULONG nIdx = rData.GetIndex( n );
        ++pCounts[ nIdx ];
    }
}

void Compare::SetDiscard( const CompareData& rData,
                            sal_Char* pDiscard, ULONG* pCounts )
{
    ULONG nLen = rData.GetLineCount();

    // berechne Max in Abhanegigkeit zur LineAnzahl
    USHORT nMax = 5;
    for( ULONG n = nLen / 64; ( n = n >> 2 ) > 0; )
        nMax <<= 1;

    for( n = 0; n < nLen; ++n )
    {
        ULONG nIdx = rData.GetIndex( n );
        if( nIdx )
        {
            nIdx = pCounts[ nIdx ];
            pDiscard[ n ] = !nIdx ? 1 : nIdx > nMax ? 2 : 0;
        }
        else
            pDiscard[ n ] = 0;
    }
}

void Compare::CheckDiscard( ULONG nLen, sal_Char* pDiscard )
{
    for( ULONG n = 0; n < nLen; ++n )
    {
        if( 2 == pDiscard[ n ] )
            pDiscard[n] = 0;
        else if( pDiscard[ n ] )
        {
            register ULONG j;
            ULONG length;
            ULONG provisional = 0;

            /* Find end of this run of discardable lines.
                Count how many are provisionally discardable.  */
            for (j = n; j < nLen; j++)
            {
                if( !pDiscard[j] )
                    break;
                if( 2 == pDiscard[j] )
                    ++provisional;
            }

            /* Cancel provisional discards at end, and shrink the run.  */
            while( j > n && 2 == pDiscard[j - 1] )
                pDiscard[ --j ] = 0, --provisional;

            /* Now we have the length of a run of discardable lines
               whose first and last are not provisional.  */
            length = j - n;

            /* If 1/4 of the lines in the run are provisional,
               cancel discarding of all provisional lines in the run.  */
            if (provisional * 4 > length)
            {
                while (j > n)
                    if (pDiscard[--j] == 2)
                        pDiscard[j] = 0;
            }
            else
            {
                register ULONG consec;
                ULONG minimum = 1;
                ULONG tem = length / 4;

                /* MINIMUM is approximate square root of LENGTH/4.
                   A subrun of two or more provisionals can stand
                   when LENGTH is at least 16.
                   A subrun of 4 or more can stand when LENGTH >= 64.  */
                while ((tem = tem >> 2) > 0)
                    minimum *= 2;
                minimum++;

                /* Cancel any subrun of MINIMUM or more provisionals
                   within the larger run.  */
                for (j = 0, consec = 0; j < length; j++)
                    if (pDiscard[n + j] != 2)
                        consec = 0;
                    else if (minimum == ++consec)
                        /* Back up to start of subrun, to cancel it all.  */
                        j -= consec;
                    else if (minimum < consec)
                        pDiscard[n + j] = 0;

                /* Scan from beginning of run
                   until we find 3 or more nonprovisionals in a row
                   or until the first nonprovisional at least 8 lines in.
                   Until that point, cancel any provisionals.  */
                for (j = 0, consec = 0; j < length; j++)
                {
                    if (j >= 8 && pDiscard[n + j] == 1)
                        break;
                    if (pDiscard[n + j] == 2)
                        consec = 0, pDiscard[n + j] = 0;
                    else if (pDiscard[n + j] == 0)
                        consec = 0;
                    else
                        consec++;
                    if (consec == 3)
                        break;
                }

                /* I advances to the last line of the run.  */
                n += length - 1;

                /* Same thing, from end.  */
                for (j = 0, consec = 0; j < length; j++)
                {
                    if (j >= 8 && pDiscard[n - j] == 1)
                        break;
                    if (pDiscard[n - j] == 2)
                        consec = 0, pDiscard[n - j] = 0;
                    else if (pDiscard[n - j] == 0)
                        consec = 0;
                    else
                        consec++;
                    if (consec == 3)
                        break;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------

Compare::MovedData::MovedData( CompareData& rData, sal_Char* pDiscard )
    : pIndex( 0 ), pLineNum( 0 ), nCount( 0 )
{
    ULONG nLen = rData.GetLineCount();
    for( ULONG n = 0; n < nLen; ++n )
        if( pDiscard[ n ] )
            rData.SetChanged( n );
        else
            ++nCount;

    if( nCount )
    {
        pIndex = new ULONG[ nCount ];
        pLineNum = new ULONG[ nCount ];

        for( n = 0, nCount = 0; n < nLen; ++n )
            if( !pDiscard[ n ] )
            {
                pIndex[ nCount ] = rData.GetIndex( n );
                pLineNum[ nCount++ ] = n;
            }
    }
}

Compare::MovedData::~MovedData()
{
    delete pIndex;
    delete pLineNum;
}

// ----------------------------------------------------------------------

    // Suche die verschobenen Lines
Compare::CompareSequence::CompareSequence(
                            CompareData& rD1, CompareData& rD2,
                            const MovedData& rMD1, const MovedData& rMD2 )
    : rData1( rD1 ), rData2( rD2 ), rMoved1( rMD1 ), rMoved2( rMD2 )
{
    ULONG nSize = rMD1.GetCount() + rMD2.GetCount() + 3;
    pMemory = new long[ nSize * 2 ];
    pFDiag = pMemory + ( rMD2.GetCount() + 1 );
    pBDiag = pMemory + ( nSize + rMD2.GetCount() + 1 );

#ifdef JP_DUMP
    rD1.Dump(), rD2.Dump();
#endif
    Compare( 0, rMD1.GetCount(), 0, rMD2.GetCount() );
}

Compare::CompareSequence::~CompareSequence()
{
    delete pMemory;
}

void Compare::CompareSequence::Compare( ULONG nStt1, ULONG nEnd1,
                                        ULONG nStt2, ULONG nEnd2 )
{
    /* Slide down the bottom initial diagonal. */
    while( nStt1 < nEnd1 && nStt2 < nEnd2 &&
        rMoved1.GetIndex( nStt1 ) == rMoved2.GetIndex( nStt2 ))
        ++nStt1, ++nStt2;

    /* Slide up the top initial diagonal. */
    while( nEnd1 > nStt1 && nEnd2 > nStt2 &&
        rMoved1.GetIndex( nEnd1 - 1 ) == rMoved2.GetIndex( nEnd2 - 1 ))
        --nEnd1, --nEnd2;

    /* Handle simple cases. */
    if( nStt1 == nEnd1 )
        while( nStt2 < nEnd2 )
            rData2.SetChanged( rMoved2.GetLineNum( nStt2++ ));

    else if (nStt2 == nEnd2)
        while (nStt1 < nEnd1)
            rData1.SetChanged( rMoved1.GetLineNum( nStt1++ ));

    else
    {
        ULONG c, d, b;

        /* Find a point of correspondence in the middle of the files.  */

        d = CheckDiag( nStt1, nEnd1, nStt2, nEnd2, &c );
        b = pBDiag[ d ];

        if( 1 != c )
        {
            /* Use that point to split this problem into two subproblems.  */
            Compare( nStt1, b, nStt2, b - d );
            /* This used to use f instead of b,
               but that is incorrect!
               It is not necessarily the case that diagonal d
               has a snake from b to f.  */
            Compare( b, nEnd1, b - d, nEnd2 );
        }
    }
}

ULONG Compare::CompareSequence::CheckDiag( ULONG nStt1, ULONG nEnd1,
                                    ULONG nStt2, ULONG nEnd2, ULONG* pCost )
{
    const long dmin = nStt1 - nEnd2;    /* Minimum valid diagonal. */
    const long dmax = nEnd1 - nStt2;    /* Maximum valid diagonal. */
    const long fmid = nStt1 - nStt2;    /* Center diagonal of top-down search. */
    const long bmid = nEnd1 - nEnd2;    /* Center diagonal of bottom-up search. */

    long fmin = fmid, fmax = fmid;  /* Limits of top-down search. */
    long bmin = bmid, bmax = bmid;  /* Limits of bottom-up search. */

    long c;         /* Cost. */
    long odd = (fmid - bmid) & 1;   /* True if southeast corner is on an odd
                     diagonal with respect to the northwest. */

    pFDiag[fmid] = nStt1;
    pBDiag[bmid] = nEnd1;

    for (c = 1;; ++c)
    {
        long d;         /* Active diagonal. */
        long big_snake = 0;

        /* Extend the top-down search by an edit step in each diagonal. */
        fmin > dmin ? pFDiag[--fmin - 1] = -1 : ++fmin;
        fmax < dmax ? pFDiag[++fmax + 1] = -1 : --fmax;
        for (d = fmax; d >= fmin; d -= 2)
        {
            long x, y, oldx, tlo = pFDiag[d - 1], thi = pFDiag[d + 1];

            if (tlo >= thi)
                x = tlo + 1;
            else
                x = thi;
            oldx = x;
            y = x - d;
            while( ULONG(x) < nEnd1 && ULONG(y) < nEnd2 &&
                rMoved1.GetIndex( x ) == rMoved2.GetIndex( y ))
                ++x, ++y;
            if (x - oldx > 20)
                big_snake = 1;
            pFDiag[d] = x;
            if( odd && bmin <= d && d <= bmax && pBDiag[d] <= pFDiag[d] )
            {
                *pCost = 2 * c - 1;
                return d;
            }
        }

        /* Similar extend the bottom-up search. */
        bmin > dmin ? pBDiag[--bmin - 1] = INT_MAX : ++bmin;
        bmax < dmax ? pBDiag[++bmax + 1] = INT_MAX : --bmax;
        for (d = bmax; d >= bmin; d -= 2)
        {
            long x, y, oldx, tlo = pBDiag[d - 1], thi = pBDiag[d + 1];

            if (tlo < thi)
                x = tlo;
            else
                x = thi - 1;
            oldx = x;
            y = x - d;
            while( ULONG(x) > nStt1 && ULONG(y) > nStt2 &&
                rMoved1.GetIndex( x - 1 ) == rMoved2.GetIndex( y - 1 ))
                --x, --y;
            if (oldx - x > 20)
                big_snake = 1;
            pBDiag[d] = x;
            if (!odd && fmin <= d && d <= fmax && pBDiag[d] <= pFDiag[d])
            {
                *pCost = 2 * c;
                return d;
            }
        }
    }
}

void Compare::ShiftBoundaries( CompareData& rData1, CompareData& rData2 )
{
    for( int i = 0; i < 2; ++i )
    {
        CompareData* pData = &rData1;
        CompareData* pOtherData = &rData2;

        ULONG i = 0;
        ULONG j = 0;
        ULONG i_end = pData->GetLineCount();
        ULONG preceding = ULONG_MAX;
        ULONG other_preceding = ULONG_MAX;

        while (1)
        {
            ULONG start, other_start;

            /* Scan forwards to find beginning of another run of changes.
               Also keep track of the corresponding point in the other file.  */

            while( i < i_end && !pData->GetChanged( i ) )
            {
                while( pOtherData->GetChanged( j++ ))
                    /* Non-corresponding lines in the other file
                       will count as the preceding batch of changes.  */
                    other_preceding = j;
                i++;
            }

            if (i == i_end)
                break;

            start = i;
            other_start = j;

            while (1)
            {
                /* Now find the end of this run of changes.  */

                while( pData->GetChanged( ++i ))
                    ;

                /* If the first changed line matches the following unchanged one,
                   and this run does not follow right after a previous run,
                   and there are no lines deleted from the other file here,
                   then classify the first changed line as unchanged
                   and the following line as changed in its place.  */

                /* You might ask, how could this run follow right after another?
                   Only because the previous run was shifted here.  */

                if( i != i_end &&
                    pData->GetIndex( start ) == pData->GetIndex( i ) &&
                    !pOtherData->GetChanged( j ) &&
                    !( start == preceding || other_start == other_preceding ))
                {
                    pData->SetChanged( start++, 0 );
                    pData->SetChanged(  i );
                    /* Since one line-that-matches is now before this run
                       instead of after, we must advance in the other file
                       to keep in synch.  */
                    ++j;
                }
                else
                    break;
            }

            preceding = i;
            other_preceding = j;
        }

        pData = &rData2;
        pOtherData = &rData1;
    }
}

/*  */

inline ULONG NextIdx( const SwNode* pNd )
{
    if( pNd->IsStartNode() )
        pNd = pNd->EndOfSectionNode();
    return pNd->GetIndex() + 1;
}

inline ULONG PrevIdx( const SwNode* pNd )
{
    if( pNd->IsEndNode() )
        pNd = pNd->StartOfSectionNode();
    return pNd->GetIndex() - 1;
}

class SwCompareLine : public CompareLine
{
    const SwNode& rNode;
public:
    SwCompareLine( const SwNode& rNd );
    virtual ~SwCompareLine();

    virtual ULONG GetHashValue() const;
    virtual BOOL Compare( const CompareLine& rLine ) const;

    static ULONG GetTxtNodeHashValue( const SwTxtNode& rNd, ULONG nVal );
    static BOOL CompareNode( const SwNode& rDstNd, const SwNode& rSrcNd );
    static BOOL CompareTxtNd( const SwTxtNode& rDstNd,
                              const SwTxtNode& rSrcNd );

    BOOL ChangesInLine( const SwCompareLine& rLine,
                            SwPaM *& rpInsRing, SwPaM*& rpDelRing ) const;

    const SwNode& GetNode() const { return rNode; }

    const SwNode& GetEndNode() const;

    // fuers Debugging!
    String GetText() const;
};

class SwCompareData : public CompareData
{
    SwDoc& rDoc;
    SwPaM *pInsRing, *pDelRing;

    virtual void CheckRanges( CompareData& );
    virtual void ShowInsert( ULONG nStt, ULONG nEnd );
    virtual void ShowDelete( const CompareData& rData, ULONG nStt,
                                ULONG nEnd, ULONG nInsPos );

    virtual void CheckForChangesInLine( const CompareData& rData,
                                    ULONG& nStt, ULONG& nEnd,
                                    ULONG& nThisStt, ULONG& nThisEnd );

public:
    SwCompareData( SwDoc& rD ) : rDoc( rD ), pInsRing(0), pDelRing(0) {}
    virtual ~SwCompareData();

    void SetRedlinesToDoc();

#ifdef JP_DUMP
    // zum Debuggen!
    virtual void Dump();
#endif
};

// ----------------------------------------------------------------

SwCompareLine::SwCompareLine( const SwNode& rNd )
    : rNode( rNd )
{
}

SwCompareLine::~SwCompareLine()
{
}

ULONG SwCompareLine::GetHashValue() const
{
    ULONG nRet = 0;
    switch( rNode.GetNodeType() )
    {
    case ND_TEXTNODE:
        nRet = GetTxtNodeHashValue( (SwTxtNode&)rNode, nRet );
        break;

    case ND_TABLENODE:
        {
            const SwNode* pEndNd = rNode.EndOfSectionNode();
            SwNodeIndex aIdx( rNode );
            while( &aIdx.GetNode() != pEndNd )
            {
                if( aIdx.GetNode().IsTxtNode() )
                    nRet = GetTxtNodeHashValue( (SwTxtNode&)aIdx.GetNode(), nRet );
                aIdx++;
            }
        }
        break;

    case ND_SECTIONNODE:
        // sollte nie auftauchen (oder?)
        break;

    case ND_GRFNODE:
    case ND_OLENODE:
        // feste Id ? sollte aber nie auftauchen
        break;
    }
    return nRet;
}

const SwNode& SwCompareLine::GetEndNode() const
{
    const SwNode* pNd = &rNode;
    switch( rNode.GetNodeType() )
    {
    case ND_TABLENODE:
        pNd = rNode.EndOfSectionNode();
        break;

    case ND_SECTIONNODE:
        // sollte nie auftauchen (oder?)
        break;
    }
    return *pNd;
}

BOOL SwCompareLine::Compare( const CompareLine& rLine ) const
{
    return CompareNode( rNode, ((SwCompareLine&)rLine).rNode );
}

BOOL SwCompareLine::CompareNode( const SwNode& rDstNd, const SwNode& rSrcNd )
{
    BOOL bRet = FALSE;

    switch( ( rSrcNd.GetNodeType() * 256 ) + rDstNd.GetNodeType() )
    {
    case ( ND_TEXTNODE * 256 ) + ND_TEXTNODE:
        bRet = CompareTxtNd( (SwTxtNode&)rDstNd, (SwTxtNode&)rSrcNd );
        break;

    case ( ND_TABLENODE * 256 ) + ND_TABLENODE:
        {
            const SwTableNode& rTSrcNd = (SwTableNode&)rSrcNd;
            const SwTableNode& rTDstNd = (SwTableNode&)rDstNd;

            bRet = ( rTSrcNd.EndOfSectionIndex() - rTSrcNd.GetIndex() ) ==
                   ( rTDstNd.EndOfSectionIndex() - rTDstNd.GetIndex() );
        }
        break;

    case ( ND_SECTIONNODE * 256 ) + ND_SECTIONNODE:
        {
            const SwSectionNode& rSSrcNd = (SwSectionNode&)rSrcNd;
            const SwSectionNode& rSDstNd = (SwSectionNode&)rDstNd;

            bRet = ( rSSrcNd.EndOfSectionIndex() - rSSrcNd.GetIndex() ) ==
                   ( rSDstNd.EndOfSectionIndex() - rSDstNd.GetIndex() );
        }
        break;

    case ( ND_ENDNODE * 256 ) + ND_ENDNODE:
        if( rSrcNd.FindStartNode()->IsTableNode() &&
            rDstNd.FindStartNode()->IsTableNode() )
        {

        }
        else if( rSrcNd.FindStartNode()->IsTableNode() &&
                 rDstNd.FindStartNode()->IsTableNode() )
        {
        }
        break;
    }
    return bRet;
}

String SwCompareLine::GetText() const
{
    String sRet;
    switch( rNode.GetNodeType() )
    {
    case ND_TEXTNODE:
        sRet = ((SwTxtNode&)rNode).GetExpandTxt();
        break;

    case ND_TABLENODE:
        {
            const SwNode* pEndNd = rNode.EndOfSectionNode();
            SwNodeIndex aIdx( rNode );
            while( &aIdx.GetNode() != pEndNd )
            {
                if( aIdx.GetNode().IsTxtNode() )
                {
                    if( sRet.Len() )
                        sRet.Append( '\n' );
                    sRet.Append( ((SwTxtNode&)rNode).GetExpandTxt() );
                }
                aIdx++;
            }
            sRet.InsertAscii( "Tabelle: ", 0 );
        }
        break;

    case ND_SECTIONNODE:
        sRet.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Section - Node:" ));
        break;

    case ND_GRFNODE:
        sRet.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Grafik - Node:" ));
        break;
    case ND_OLENODE:
        sRet.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "OLE - Node:" ));
        break;
    }
    return sRet;
}

ULONG SwCompareLine::GetTxtNodeHashValue( const SwTxtNode& rNd, ULONG nVal )
{
    String sStr( rNd.GetExpandTxt() );
    for( xub_StrLen n = 0; n < sStr.Len(); ++n )
        ( nVal <<= 1 ) += sStr.GetChar( n );
    return nVal;
}

BOOL SwCompareLine::CompareTxtNd( const SwTxtNode& rDstNd,
                                  const SwTxtNode& rSrcNd )
{
    BOOL bRet = FALSE;
    // erstmal ganz einfach!
    if( rDstNd.GetTxt() == rSrcNd.GetTxt() )
    {
        // der Text ist gleich, aber sind die "Sonderattribute" (0xFF) auch
        // dieselben??
        bRet = TRUE;
    }
    return bRet;
}

BOOL SwCompareLine::ChangesInLine( const SwCompareLine& rLine,
                            SwPaM *& rpInsRing, SwPaM*& rpDelRing ) const
{
    BOOL bRet = FALSE;
    if( ND_TEXTNODE == rNode.GetNodeType() &&
        ND_TEXTNODE == rLine.GetNode().GetNodeType() )
    {
        SwTxtNode& rDestNd = *(SwTxtNode*)rNode.GetTxtNode();
        const SwTxtNode& rSrcNd = *rLine.GetNode().GetTxtNode();

        xub_StrLen nDEnd = rDestNd.GetTxt().Len(), nSEnd = rSrcNd.GetTxt().Len();
        for( xub_StrLen nStt = 0, nEnd = Min( nDEnd, nSEnd );
            nStt < nEnd; ++nStt )
            if( rDestNd.GetTxt().GetChar( nStt ) !=
                rSrcNd.GetTxt().GetChar( nStt ) )
                break;

        while( nStt < nDEnd && nStt < nSEnd )
        {
            --nDEnd, --nSEnd;
            if( rDestNd.GetTxt().GetChar( nDEnd ) !=
                rSrcNd.GetTxt().GetChar( nSEnd ) )
            {
                ++nDEnd, ++nSEnd;
                break;
            }
        }

        if( nStt || !nDEnd || !nSEnd || nDEnd < rDestNd.GetTxt().Len() ||
            nSEnd < rSrcNd.GetTxt().Len() )
        {
            // jetzt ist zwischen nStt bis nDEnd das neu eingefuegte
            // und zwischen nStt und nSEnd das geloeschte
            SwDoc* pDoc = rDestNd.GetDoc();
            SwPaM aPam( rDestNd, nDEnd );
            if( nStt != nDEnd )
            {
                SwPaM* pTmp = new SwPaM( *aPam.GetPoint(), rpInsRing );
                if( !rpInsRing )
                    rpInsRing = pTmp;

                pTmp->SetMark();
                pTmp->GetMark()->nContent = nStt;
            }

            if( nStt != nSEnd )
            {
                {
                    BOOL bUndo = pDoc->DoesUndo();
                    pDoc->DoUndo( FALSE );
                    SwPaM aCpyPam( rSrcNd, nStt );
                    aCpyPam.SetMark();
                    aCpyPam.GetPoint()->nContent = nSEnd;
                    aCpyPam.GetDoc()->Copy( aCpyPam, *aPam.GetPoint() );
                    pDoc->DoUndo( bUndo );
                }

                SwPaM* pTmp = new SwPaM( *aPam.GetPoint(), rpDelRing );
                if( !rpDelRing )
                    rpDelRing = pTmp;

                pTmp->SetMark();
                pTmp->GetMark()->nContent = nDEnd;

                if( rpInsRing )
                {
                    SwPaM* pCorr = (SwPaM*)rpInsRing->GetPrev();
                    if( *pCorr->GetPoint() == *pTmp->GetPoint() )
                        *pCorr->GetPoint() = *pTmp->GetMark();
                }
            }
            bRet = TRUE;
        }
    }
    return bRet;
}

// ----------------------------------------------------------------

SwCompareData::~SwCompareData()
{
    if( pDelRing )
    {
        while( pDelRing->GetNext() != pDelRing )
            delete pDelRing->GetNext();
        delete pDelRing;
    }
    if( pInsRing )
    {
        while( pInsRing->GetNext() != pInsRing )
            delete pInsRing->GetNext();
        delete pInsRing;
    }
}

void SwCompareData::CheckRanges( CompareData& rData )
{
    const SwNodes& rSrcNds = ((SwCompareData&)rData).rDoc.GetNodes();
    const SwNodes& rDstNds = rDoc.GetNodes();

    const SwNode& rSrcEndNd = rSrcNds.GetEndOfContent();
    const SwNode& rDstEndNd = rDstNds.GetEndOfContent();

    SwNodeIndex aSrcIdx( *rSrcEndNd.FindStartNode(), 1 );
    SwNodeIndex aDstIdx( *rDstEndNd.FindStartNode(), 1 );

    ULONG nSrcSttIdx = aSrcIdx.GetIndex();
    ULONG nSrcEndIdx = rSrcEndNd.GetIndex();

    ULONG nDstSttIdx = aDstIdx.GetIndex();
    ULONG nDstEndIdx = rDstEndNd.GetIndex();

    while( nSrcSttIdx < nSrcEndIdx && nDstSttIdx < nDstEndIdx )
    {
        const SwNode* pSrcNd = rSrcNds[ nSrcSttIdx ];
        const SwNode* pDstNd = rDstNds[ nDstSttIdx ];
        if( !SwCompareLine::CompareNode( *pSrcNd, *pDstNd ))
            break;

        nSrcSttIdx = NextIdx( pSrcNd );
        nDstSttIdx = NextIdx( pDstNd );
    }

    --nSrcEndIdx;
    --nDstEndIdx;
    while( nSrcSttIdx < nSrcEndIdx && nDstSttIdx < nDstEndIdx )
    {
        const SwNode* pSrcNd = rSrcNds[ nSrcEndIdx ];
        const SwNode* pDstNd = rDstNds[ nDstEndIdx ];
        if( !SwCompareLine::CompareNode( *pSrcNd, *pDstNd ))
            break;

        nSrcEndIdx = PrevIdx( pSrcNd );
        nDstEndIdx = PrevIdx( pDstNd );
    }

    while( nSrcSttIdx <= nSrcEndIdx )
    {
        const SwNode* pNd = rSrcNds[ nSrcSttIdx ];
        rData.InsertLine( new SwCompareLine( *pNd ) );
        nSrcSttIdx = NextIdx( pNd );
    }

    while( nDstSttIdx <= nDstEndIdx )
    {
        const SwNode* pNd = rDstNds[ nDstSttIdx ];
        InsertLine( new SwCompareLine( *pNd ) );
        nDstSttIdx = NextIdx( pNd );
    }
}


void SwCompareData::ShowInsert( ULONG nStt, ULONG nEnd )
{
    SwPaM* pTmp = new SwPaM( ((SwCompareLine*)GetLine( nStt ))->GetNode(), 0,
                            ((SwCompareLine*)GetLine( nEnd-1 ))->GetEndNode(), 0,
                             pInsRing );
    if( !pInsRing )
        pInsRing = pTmp;

// vom Anfang des 1. Absatzes (1. Buchstabe) bis
// zum letzten Absatz (letzter Buchstabe!)
    pTmp->GetPoint()->nNode++;
    pTmp->GetPoint()->nContent.Assign( pTmp->GetCntntNode(), 0 );
}

void SwCompareData::ShowDelete( const CompareData& rData, ULONG nStt,
                                ULONG nEnd, ULONG nInsPos )
{
    SwNodeRange aRg(
        ((SwCompareLine*)rData.GetLine( nStt ))->GetNode(), 0,
        ((SwCompareLine*)rData.GetLine( nEnd-1 ))->GetEndNode(), 1 );

    USHORT nOffset = 0;
    const CompareLine* pLine;
    if( GetLineCount() == nInsPos )
    {
        pLine = GetLine( nInsPos-1 );
        nOffset = 1;
    }
    else
        pLine = GetLine( nInsPos );

    const SwNode* pLineNd;
    if( pLine )
        pLineNd = &((SwCompareLine*)pLine)->GetNode();
    else
    {
        pLineNd = &rDoc.GetNodes().GetEndOfContent();
        nOffset = 0;
    }

    SwNodeIndex aInsPos( *pLineNd, nOffset );
    SwNodeIndex aSavePos( aInsPos, -1 );

    ((SwCompareData&)rData).rDoc.CopyWithFlyInFly( aRg, aInsPos );

    aSavePos++;

    SwPaM* pTmp = new SwPaM( aSavePos.GetNode(), aInsPos.GetNode(), 0, 0,
                                pDelRing );
    if( !pDelRing )
        pDelRing = pTmp;

    if( pInsRing )
    {
        SwPaM* pCorr = (SwPaM*)pInsRing->GetPrev();
        if( *pCorr->GetPoint() == *pTmp->GetPoint() )
            *pCorr->GetPoint() = *pTmp->GetMark();
    }
}

void SwCompareData::CheckForChangesInLine( const CompareData& rData,
                                    ULONG& rStt, ULONG& rEnd,
                                    ULONG& rThisStt, ULONG& rThisEnd )
{
    while( rStt < rEnd && rThisStt < rThisEnd )
    {
        SwCompareLine* pDstLn = (SwCompareLine*)GetLine( rThisStt );
        SwCompareLine* pSrcLn = (SwCompareLine*)rData.GetLine( rStt );
        if( !pDstLn->ChangesInLine( *pSrcLn, pInsRing, pDelRing ) )
            break;

        ++rStt;
        ++rThisStt;
    }
}

void SwCompareData::SetRedlinesToDoc()
{
    SwPaM* pTmp = pDelRing;
    if( pTmp )
        do {
            rDoc.DeleteRedline( *pTmp, FALSE );

            if( rDoc.DoesUndo() )
                rDoc.AppendUndo( new SwUndoCompDoc( *pTmp, FALSE ));
            rDoc.AppendRedline( new SwRedline( REDLINE_DELETE, *pTmp ) );

        } while( pDelRing != ( pTmp = (SwPaM*)pTmp->GetNext() ));

    pTmp = pInsRing;
    if( pTmp )
    {
        // zusammenhaengende zusammenfassen
        if( pTmp->GetNext() != pInsRing )
        {
            const SwCntntNode* pCNd;
            do {
                SwPosition& rSttEnd = *pTmp->End(),
                          & rEndStt = *((SwPaM*)pTmp->GetNext())->Start();
                if( rSttEnd == rEndStt ||
                    (!rEndStt.nContent.GetIndex() &&
                    rEndStt.nNode.GetIndex() - 1 == rSttEnd.nNode.GetIndex() &&
                    0 != ( pCNd = rSttEnd.nNode.GetNode().GetCntntNode() )
                        ? rSttEnd.nContent.GetIndex() == pCNd->Len()
                        : 0 ))
                {
                    if( pTmp->GetNext() == pInsRing )
                    {
                        // liegen hintereinander also zusammen fassen
                        rEndStt = *pTmp->Start();
                        delete pTmp;
                        pTmp = pInsRing;
                    }
                    else
                    {
                        // liegen hintereinander also zusammen fassen
                        rSttEnd = *((SwPaM*)pTmp->GetNext())->End();
                        delete pTmp->GetNext();
                    }
                }
                else
                    pTmp = (SwPaM*)pTmp->GetNext();
            } while( pInsRing != pTmp );
        }

        do {
            if( rDoc.AppendRedline( new SwRedline( REDLINE_INSERT, *pTmp )) &&
                rDoc.DoesUndo() )
                rDoc.AppendUndo( new SwUndoCompDoc( *pTmp, TRUE ));
        } while( pInsRing != ( pTmp = (SwPaM*)pTmp->GetNext() ));
    }
}

#ifdef JP_DUMP
void SwCompareData::Dump()
{
    static int nFirst = 1;
    SvFileStream aStrm( "d:\\tmp\\compare.dmp", nFirst
                            ? STREAM_WRITE | STREAM_TRUNC
                            : STREAM_WRITE | STREAM_NOCREATE );
    if( !nFirst )
        aStrm.Seek( STREAM_SEEK_TO_END );

    nFirst = 0;

    aStrm << "\n";
    ULONG nLCount = aLines.Count();
    for( ULONG n = 0; n < nLCount; ++n )
    {
        SwCompareLine* pLine = (SwCompareLine*)GetLine( n );
        String sTxt( pLine->GetText() );
        char sBuffer[ 20 ];
        sprintf( sBuffer, "[%4ld][%3ld][%1d]",
                                pLine->GetNode().GetIndex(),
                                GetIndex(  n ), GetChanged(  n ) );
        ( aStrm << sBuffer ).WriteByteString( sTxt ) << '\n';
    }
}

#endif

/*  */



    // returnt (?die Anzahl der Unterschiede?) ob etwas unterschiedlich ist
long SwDoc::CompareDoc( const SwDoc& rDoc )
{
    if( &rDoc == this )
        return 0;

    long nRet = 0;

    StartUndo();

    SwDoc& rSrcDoc = (SwDoc&)rDoc;
    BOOL bSrcModified = rSrcDoc.IsModified();

    SwRedlineMode eSrcRedlMode = rSrcDoc.GetRedlineMode();
    rSrcDoc.SetRedlineMode( REDLINE_SHOW_INSERT );
    SetRedlineMode( REDLINE_ON | REDLINE_SHOW_INSERT );

    SwCompareData aD0( rSrcDoc );
    SwCompareData aD1( *this );

    aD1.CompareLines( aD0 );

    nRet = aD1.ShowDiffs( aD0 );

    if( nRet )
    {
        SetRedlineMode( REDLINE_ON | REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE );
        aD1.SetRedlinesToDoc();
    }

#ifdef JP_DUMP
    aD0.Dump(), aD1.Dump();
#endif

    rSrcDoc.SetRedlineMode( eSrcRedlMode );
    SetRedlineMode( REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE );

    if( !bSrcModified )
        rSrcDoc.ResetModified();

    EndUndo();

    return nRet;
}


typedef void (SwDoc::*FNInsUndo)( SwUndo* );

class _SaveMergeRedlines : public Ring
{
    const SwRedline* pSrcRedl;
    SwRedline* pDestRedl;
public:
    _SaveMergeRedlines( const SwNode& rDstNd,
                        const SwRedline& rSrcRedl, Ring* pRing );
    USHORT InsertRedline( FNInsUndo pFn );

    SwRedline* GetDestRedline() { return pDestRedl; }
};

_SaveMergeRedlines::_SaveMergeRedlines( const SwNode& rDstNd,
                        const SwRedline& rSrcRedl, Ring* pRing )
    : Ring( pRing ), pSrcRedl( &rSrcRedl )
{
    SwPosition aPos( rDstNd );

    const SwPosition* pStt = rSrcRedl.Start();
    if( rDstNd.IsCntntNode() )
        aPos.nContent.Assign( ((SwCntntNode*)&rDstNd), pStt->nContent.GetIndex() );
    pDestRedl = new SwRedline( rSrcRedl.GetRedlineData(), aPos );

    if( REDLINE_DELETE == pDestRedl->GetType() )
    {
        // den Bereich als geloescht kennzeichnen
        const SwPosition* pEnd = pStt == rSrcRedl.GetPoint()
                                            ? rSrcRedl.GetMark()
                                            : rSrcRedl.GetPoint();

        pDestRedl->SetMark();
        pDestRedl->GetPoint()->nNode += pEnd->nNode.GetIndex() -
                                        pStt->nNode.GetIndex();
        pDestRedl->GetPoint()->nContent.Assign( pDestRedl->GetCntntNode(),
                                                pEnd->nContent.GetIndex() );
    }
}

USHORT _SaveMergeRedlines::InsertRedline( FNInsUndo pFn )
{
    USHORT nIns = 0;
    SwDoc* pDoc = pDestRedl->GetDoc();

    if( REDLINE_INSERT == pDestRedl->GetType() )
    {
        // der Teil wurde eingefuegt, also kopiere ihn aus dem SourceDoc
        BOOL bUndo = pDoc->DoesUndo();
        pDoc->DoUndo( FALSE );

        SwNodeIndex aSaveNd( pDestRedl->GetPoint()->nNode, -1 );
        xub_StrLen nSaveCnt = pDestRedl->GetPoint()->nContent.GetIndex();

        SwRedlineMode eOld = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode_intern( eOld | REDLINE_IGNORE );

        pSrcRedl->GetDoc()->Copy( *(SwPaM*)pSrcRedl, *pDestRedl->GetPoint() );

        pDoc->SetRedlineMode_intern( eOld );
        pDoc->DoUndo( bUndo );

        pDestRedl->SetMark();
        aSaveNd++;
        pDestRedl->GetMark()->nNode = aSaveNd;
        pDestRedl->GetMark()->nContent.Assign( aSaveNd.GetNode().GetCntntNode(),
                                                nSaveCnt );

        SwPaM* pPrev = ((_SaveMergeRedlines*)GetPrev())->pDestRedl;
        if( pPrev && *pPrev->GetPoint() == *pDestRedl->GetPoint() )
            *pPrev->GetPoint() == *pDestRedl->GetMark();

    }
    else
    {
        //JP 21.09.98: Bug 55909
        // falls im Doc auf gleicher Pos aber schon ein geloeschter oder
        // eingefuegter ist, dann muss dieser gesplittet werden!
        SwPosition* pDStt = pDestRedl->GetMark(),
                  * pDEnd = pDestRedl->GetPoint();
        USHORT n = 0;

            // zur StartPos das erste Redline suchen
        if( !pDoc->GetRedline( *pDStt, &n ) && n )
            --n;

        const SwRedlineTbl& rRedlineTbl = pDoc->GetRedlineTbl();
        for( ; n < rRedlineTbl.Count(); ++n )
        {
            SwRedline* pRedl = rRedlineTbl[ n ];
            SwPosition* pRStt = pRedl->Start(),
                      * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                           : pRedl->GetPoint();
            if( REDLINE_DELETE == pRedl->GetType() ||
                REDLINE_INSERT == pRedl->GetType() )
            {
                SwComparePosition eCmpPos = ComparePosition( *pDStt, *pDEnd, *pRStt, *pREnd );
                switch( eCmpPos )
                {
                case POS_BEHIND:
                    break;

                case POS_INSIDE:
                case POS_EQUAL:
                    delete pDestRedl, pDestRedl = 0;
                    // break; -> kein break !!!!

                case POS_BEFORE:
                    n = rRedlineTbl.Count();
                    break;

                case POS_OUTSIDE:
                    {
                        SwRedline* pCpyRedl = new SwRedline(
                            pDestRedl->GetRedlineData(), *pDStt );
                        pCpyRedl->SetMark();
                        *pCpyRedl->GetPoint() = *pRStt;

                        SwUndoCompDoc* pUndo = pDoc->DoesUndo()
                                    ? new SwUndoCompDoc( *pCpyRedl ) : 0;
                        if( pDoc->AppendRedline( pCpyRedl ) )
                        {
                            ++nIns;
                            if( pUndo )
                                (pDoc->*pFn)( pUndo );
                        }
                        else
                            delete pUndo;

                        *pDStt = *pREnd;

                        // dann solle man neu anfangen
                        n = USHRT_MAX;
                    }
                    break;

                case POS_OVERLAP_BEFORE:
                    *pDEnd = *pRStt;
                    break;

                case POS_OVERLAP_BEHIND:
                    *pDStt = *pREnd;
                    break;
                }
            }
            else if( *pDEnd <= *pRStt )
                break;
        }

    }

    if( pDestRedl )
    {
        SwUndoCompDoc* pUndo = pDoc->DoesUndo() ? new SwUndoCompDoc( *pDestRedl ) : 0;
        if( pDoc->AppendRedline( pDestRedl ) )
        {
            ++nIns;
            if( pUndo )
                (pDoc->*pFn)( pUndo );
        }
        else
            delete pUndo;
    }
    return nIns;
}

// merge zweier Dokumente
long SwDoc::MergeDoc( const SwDoc& rDoc )
{
    if( &rDoc == this )
        return 0;

    long nRet = 0;

    StartUndo();

    SwDoc& rSrcDoc = (SwDoc&)rDoc;
    BOOL bSrcModified = rSrcDoc.IsModified();

    SwRedlineMode eSrcRedlMode = rSrcDoc.GetRedlineMode();
    rSrcDoc.SetRedlineMode( REDLINE_SHOW_DELETE );
    SetRedlineMode( REDLINE_SHOW_DELETE );

    SwCompareData aD0( rSrcDoc );
    SwCompareData aD1( *this );

    aD1.CompareLines( aD0 );

    if( !aD1.HasDiffs( aD0 ) )
    {
        // jetzt wollen wir alle Redlines aus dem SourceDoc zu uns bekommen

        // suche alle Insert - Redlines aus dem SourceDoc und bestimme
        // deren Position im DestDoc
        _SaveMergeRedlines* pRing = 0;
        const SwRedlineTbl& rSrcRedlTbl = rSrcDoc.GetRedlineTbl();
        ULONG nEndOfExtra = rSrcDoc.GetNodes().GetEndOfExtras().GetIndex();
        ULONG nMyEndOfExtra = GetNodes().GetEndOfExtras().GetIndex();
        for( USHORT n = 0; n < rSrcRedlTbl.Count(); ++n )
        {
            const SwRedline* pRedl = rSrcRedlTbl[ n ];
            ULONG nNd = pRedl->GetPoint()->nNode.GetIndex();
            SwRedlineType eType = pRedl->GetType();
            if( nEndOfExtra < nNd &&
                ( REDLINE_INSERT == eType || REDLINE_DELETE == eType ))
            {
                const SwNode* pDstNd = GetNodes()[
                                        nMyEndOfExtra + nNd - nEndOfExtra ];

                // Position gefunden. Dann muss im DestDoc auch
                // in der Line das Redline eingefuegt werden
                _SaveMergeRedlines* pTmp = new _SaveMergeRedlines(
                                                    *pDstNd, *pRedl, pRing );
                if( !pRing )
                    pRing = pTmp;
            }
        }

        if( pRing )
        {
            // dann alle ins DestDoc ueber nehmen
            rSrcDoc.SetRedlineMode( REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE );
            SetRedlineMode( REDLINE_ON | REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE );
            _SaveMergeRedlines* pTmp = pRing;

            do {
                nRet += pTmp->InsertRedline( &SwDoc::AppendUndo );
            } while( pRing != ( pTmp = (_SaveMergeRedlines*)pTmp->GetNext() ));

            while( pRing != pRing->GetNext() )
                delete pRing->GetNext();
            delete pRing;
        }
    }

    rSrcDoc.SetRedlineMode( eSrcRedlMode );
    if( !bSrcModified )
        rSrcDoc.ResetModified();

    SetRedlineMode( REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE );

    EndUndo();

    return nRet;
}


