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


#include <hintids.hxx>
#include <vcl/vclenum.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/udlnitem.hxx>
#include <swmodule.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <redline.hxx>
#include <UndoRedline.hxx>
#include <section.hxx>
#include <tox.hxx>
#include <docsh.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <vector>

#include <set>
#include <cctype>

using namespace ::com::sun::star;

using ::std::vector;

class CompareLine
{
public:
    CompareLine() {}
    virtual ~CompareLine();

    virtual sal_uLong GetHashValue() const = 0;
    virtual sal_Bool Compare( const CompareLine& rLine ) const = 0;
};

class CompareData
{
    size_t* pIndex;
    bool* pChangedFlag;

protected:
    vector< CompareLine* > aLines;
    sal_uLong nSttLineNum;

    // Truncate beginning and end and add all others to the LinesArray
    virtual void CheckRanges( CompareData& ) = 0;

public:
    CompareData();
    virtual ~CompareData();

    // Are there differences?
    sal_Bool HasDiffs( const CompareData& rData ) const;

    // Triggers the comparison and creation of two documents
    void CompareLines( CompareData& rData );
    // Display the differences - calls the methods ShowInsert and ShowDelete.
    // These are passed the start and end line number.
    // Displaying the actually content is to be handled by the subclass!
    sal_uLong ShowDiffs( const CompareData& rData );

    virtual void ShowInsert( sal_uLong nStt, sal_uLong nEnd );
    virtual void ShowDelete( const CompareData& rData, sal_uLong nStt,
                                sal_uLong nEnd, sal_uLong nInsPos );
    virtual void CheckForChangesInLine( const CompareData& rData,
                                    sal_uLong& nStt, sal_uLong& nEnd,
                                    sal_uLong& nThisStt, sal_uLong& nThisEnd );

    // Set non-ambiguous index for a line. Same lines have the same index, even in the other CompareData!
    void SetIndex( size_t nLine, size_t nIndex );
    size_t GetIndex( size_t nLine ) const
        { return nLine < aLines.size() ? pIndex[ nLine ] : 0; }

    // Set/get of a line has changed
    void SetChanged( size_t nLine, bool bFlag = true );
    bool GetChanged( size_t nLine ) const
        {
            return (pChangedFlag && nLine < aLines.size())
                ? pChangedFlag[ nLine ]
                : 0;
        }

    size_t GetLineCount() const     { return aLines.size(); }
    sal_uLong GetLineOffset() const     { return nSttLineNum; }
    const CompareLine* GetLine( size_t nLine ) const
            { return aLines[ nLine ]; }
    void InsertLine( CompareLine* pLine )
        { aLines.push_back( pLine ); }
};

class Hash
{
    struct _HashData
    {
        sal_uLong nNext, nHash;
        const CompareLine* pLine;

        _HashData()
            : nNext( 0 ), nHash( 0 ), pLine(0) {}
    };

    sal_uLong* pHashArr;
    _HashData* pDataArr;
    sal_uLong nCount, nPrime;

public:
    Hash( sal_uLong nSize );
    ~Hash();

    void CalcHashValue( CompareData& rData );

    sal_uLong GetCount() const { return nCount; }
};

class Compare
{
public:
    class MovedData
    {
        sal_uLong* pIndex;
        sal_uLong* pLineNum;
        sal_uLong nCount;

    public:
        MovedData( CompareData& rData, sal_Char* pDiscard );
        ~MovedData();

        sal_uLong GetIndex( sal_uLong n ) const { return pIndex[ n ]; }
        sal_uLong GetLineNum( sal_uLong n ) const { return pLineNum[ n ]; }
        sal_uLong GetCount() const { return nCount; }
    };

private:
    // Look for the moved lines
    class CompareSequence
    {
        CompareData &rData1, &rData2;
        const MovedData &rMoved1, &rMoved2;
        long *pMemory, *pFDiag, *pBDiag;

        void Compare( sal_uLong nStt1, sal_uLong nEnd1, sal_uLong nStt2, sal_uLong nEnd2 );
        sal_uLong CheckDiag( sal_uLong nStt1, sal_uLong nEnd1,
                        sal_uLong nStt2, sal_uLong nEnd2, sal_uLong* pCost );
    public:
        CompareSequence( CompareData& rData1, CompareData& rData2,
                        const MovedData& rD1, const MovedData& rD2 );
        ~CompareSequence();
    };


    static void CountDifference( const CompareData& rData, sal_uLong* pCounts );
    static void SetDiscard( const CompareData& rData,
                            sal_Char* pDiscard, sal_uLong* pCounts );
    static void CheckDiscard( sal_uLong nLen, sal_Char* pDiscard );
    static sal_uLong SetChangedFlag( CompareData& rData, sal_Char* pDiscard, int bFirst );
    static void ShiftBoundaries( CompareData& rData1, CompareData& rData2 );

public:
    Compare( sal_uLong nDiff, CompareData& rData1, CompareData& rData2 );
};

class ArrayComparator
{
public:
    virtual bool Compare( int nIdx1, int nIdx2 ) const = 0;
    virtual int GetLen1() const = 0;
    virtual int GetLen2() const = 0;
    virtual ~ArrayComparator() {}
};

// Consider two lines equal if similar enough (e.g. look like different
// versions of the same paragraph)
class LineArrayComparator : public ArrayComparator
{
private:
    int nLen1, nLen2;
    const CompareData &rData1, &rData2;
    int nFirst1, nFirst2;

public:
    LineArrayComparator( const CompareData &rD1, const CompareData &rD2,
                            int nStt1, int nEnd1, int nStt2, int nEnd2 );

    virtual bool Compare( int nIdx1, int nIdx2 ) const;
    virtual int GetLen1() const { return nLen1; }
    virtual int GetLen2() const { return nLen2; }
};

class WordArrayComparator : public ArrayComparator
{
private:
    const SwTxtNode *pTxtNd1, *pTxtNd2;
    int *pPos1, *pPos2;
    int nCnt1, nCnt2;		// number of words

    void CalcPositions( int *pPos, const SwTxtNode *pTxtNd, int &nCnt );

public:
    WordArrayComparator( const SwTxtNode *pNode1, const SwTxtNode *pNode2 );
    ~WordArrayComparator();

    virtual bool Compare( int nIdx1, int nIdx2 ) const;
    virtual int GetLen1() const { return nCnt1; }
    virtual int GetLen2() const { return nCnt2; }
    int GetCharSequence( const int *pWordLcs1, const int *pWordLcs2,
                        int *pSubseq1, int *pSubseq2, int nLcsLen );
};

class CharArrayComparator : public ArrayComparator
{
private:
    const SwTxtNode *pTxtNd1, *pTxtNd2;

public:
    CharArrayComparator( const SwTxtNode *pNode1, const SwTxtNode *pNode2 )
        : pTxtNd1( pNode1 ), pTxtNd2( pNode2 )
    {
    }

    virtual bool Compare( int nIdx1, int nIdx2 ) const;
    virtual int GetLen1() const { return pTxtNd1->GetTxt().Len(); }
    virtual int GetLen2() const { return pTxtNd2->GetTxt().Len(); }
};

// Options set in Tools->Options->Writer->Comparison
struct CmpOptionsContainer
{
    SvxCompareMode eCmpMode;
    int nIgnoreLen;
    bool bUseRsid;
} CmpOptions;

class CommonSubseq
{
private:
    int *pData;
    int nSize;

protected:
    ArrayComparator &rCmp;

    CommonSubseq( ArrayComparator &rComparator, int nMaxSize )
        : nSize( nMaxSize ), rCmp( rComparator )
    {
        pData = new int[ nSize ];
    }

    ~CommonSubseq()
    {
        delete[] pData;
    }

    int FindLCS( int *pLcs1 = 0, int *pLcs2 = 0, int nStt1 = 0,
                    int nEnd1 = 0, int nStt2 = 0, int nEnd2 = 0 );

public:
    int IgnoreIsolatedPieces( int *pLcs1, int *pLcs2, int nLen1, int nLen2,
                                int nLcsLen, int nPieceLen );
};

// Use Hirschberg's algrithm to find LCS in linear space
class LgstCommonSubseq: public CommonSubseq
{
private:
    static const int CUTOFF = 1<<20; // Stop recursion at this value

    int *pL1, *pL2;
    int *pBuff1, *pBuff2;

    void FindL( int *pL, int nStt1, int nEnd1, int nStt2, int nEnd2  );
    int HirschbergLCS( int *pLcs1, int *pLcs2, int nStt1, int nEnd1,
                                                int nStt2, int nEnd2 );

public:
    LgstCommonSubseq( ArrayComparator &rComparator );
    ~LgstCommonSubseq();

    int Find( int *pSubseq1, int *pSubseq2 );
};

// Find a common subsequence in linear time
class FastCommonSubseq: private CommonSubseq
{
private:
    static const int CUTOFF = 2056;

    int FindFastCS( int *pSeq1, int *pSeq2, int nStt1, int nEnd1,
                                             int nStt2, int nEnd2  );

public:
    FastCommonSubseq( ArrayComparator &rComparator )
        : CommonSubseq( rComparator, CUTOFF )
    {
    }

    int Find( int *pSubseq1, int *pSubseq2 )
    {
        return FindFastCS( pSubseq1, pSubseq2, 0, rCmp.GetLen1(),
                                                0, rCmp.GetLen2() );
    }
};

CompareLine::~CompareLine() {}

CompareData::CompareData()
    : pIndex( 0 ), pChangedFlag( 0 ), nSttLineNum( 0 )
{
}

CompareData::~CompareData()
{
    delete[] pIndex;
    delete[] pChangedFlag;
}

void CompareData::SetIndex( size_t nLine, size_t nIndex )
{
    if( !pIndex )
    {
        pIndex = new size_t[ aLines.size() ];
        memset( pIndex, 0, aLines.size() * sizeof( size_t ) );
    }
    if( nLine < aLines.size() )
        pIndex[ nLine ] = nIndex;
}

void CompareData::SetChanged( size_t nLine, bool bFlag )
{
    if( !pChangedFlag )
    {
        pChangedFlag = new bool[ aLines.size() +1 ];
        memset( pChangedFlag, 0, (aLines.size() +1) * sizeof( bool ) );
    }
    if( nLine < aLines.size() )
        pChangedFlag[ nLine ] = bFlag;
}

void CompareData::CompareLines( CompareData& rData )
{
    CheckRanges( rData );

    sal_uLong nDifferent;
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

sal_uLong CompareData::ShowDiffs( const CompareData& rData )
{
    sal_uLong nLen1 = rData.GetLineCount(), nLen2 = GetLineCount();
    sal_uLong nStt1 = 0, nStt2 = 0;
    sal_uLong nCnt = 0;

    while( nStt1 < nLen1 || nStt2 < nLen2 )
    {
        if( rData.GetChanged( nStt1 ) || GetChanged( nStt2 ) )
        {
            // Find a region of different lines between two pairs of identical
            // lines.
            sal_uLong nSav1 = nStt1, nSav2 = nStt2;
            while( nStt1 < nLen1 && rData.GetChanged( nStt1 )) ++nStt1;
            while( nStt2 < nLen2 && GetChanged( nStt2 )) ++nStt2;

            // Check if there are changed lines (only slightly different) and
            // compare them in detail.
            CheckForChangesInLine( rData, nSav1, nStt1, nSav2, nStt2 );

            ++nCnt;
        }
        ++nStt1, ++nStt2;
    }
    return nCnt;
}

sal_Bool CompareData::HasDiffs( const CompareData& rData ) const
{
    sal_Bool bRet = sal_False;
    sal_uLong nLen1 = rData.GetLineCount(), nLen2 = GetLineCount();
    sal_uLong nStt1 = 0, nStt2 = 0;

    while( nStt1 < nLen1 || nStt2 < nLen2 )
    {
        if( rData.GetChanged( nStt1 ) || GetChanged( nStt2 ) )
        {
            bRet = sal_True;
            break;
        }
        ++nStt1, ++nStt2;
    }
    return bRet;
}

void CompareData::ShowInsert( sal_uLong, sal_uLong )
{
}

void CompareData::ShowDelete( const CompareData&, sal_uLong, sal_uLong, sal_uLong )
{
}

void CompareData::CheckForChangesInLine( const CompareData& ,
                                    sal_uLong&, sal_uLong&, sal_uLong&, sal_uLong& )
{
}

Hash::Hash( sal_uLong nSize )
    : nCount( 1 )
{

static const sal_uLong primes[] =
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
    int i;

    pDataArr = new _HashData[ nSize ];
    pDataArr[0].nNext = 0;
    pDataArr[0].nHash = 0,
    pDataArr[0].pLine = 0;

    for( i = 0; primes[i] < nSize / 3;  i++)
        if( !primes[i] )
        {
            pHashArr = 0;
            return;
        }
    nPrime = primes[ i ];
    pHashArr = new sal_uLong[ nPrime ];
    memset( pHashArr, 0, nPrime * sizeof( sal_uLong ) );
}

Hash::~Hash()
{
    delete[] pHashArr;
    delete[] pDataArr;
}

void Hash::CalcHashValue( CompareData& rData )
{
    if( pHashArr )
    {
        for( size_t n = 0; n < rData.GetLineCount(); ++n )
        {
            const CompareLine* pLine = rData.GetLine( n );
            OSL_ENSURE( pLine, "wo ist die Line?" );
            sal_uLong nH = pLine->GetHashValue();

            sal_uLong* pFound = &pHashArr[ nH % nPrime ];
            size_t i;
            for( i = *pFound;  ;  i = pDataArr[i].nNext )
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

Compare::Compare( sal_uLong nDiff, CompareData& rData1, CompareData& rData2 )
{
    MovedData *pMD1, *pMD2;
    // Look for the differing lines
    {
        sal_Char* pDiscard1 = new sal_Char[ rData1.GetLineCount() ];
        sal_Char* pDiscard2 = new sal_Char[ rData2.GetLineCount() ];

        sal_uLong* pCount1 = new sal_uLong[ nDiff ];
        sal_uLong* pCount2 = new sal_uLong[ nDiff ];
        memset( pCount1, 0, nDiff * sizeof( sal_uLong ));
        memset( pCount2, 0, nDiff * sizeof( sal_uLong ));

        // find indices in CompareData which have been assigned multiple times
        CountDifference( rData1, pCount1 );
        CountDifference( rData2, pCount2 );

        // All which occur only once now have either been inserted or deleted.
        // All which are also contained in the other one have been moved.
        SetDiscard( rData1, pDiscard1, pCount2 );
        SetDiscard( rData2, pDiscard2, pCount1 );

        // forget the arrays again
        delete [] pCount1; delete [] pCount2;

        CheckDiscard( rData1.GetLineCount(), pDiscard1 );
        CheckDiscard( rData2.GetLineCount(), pDiscard2 );

        pMD1 = new MovedData( rData1, pDiscard1 );
        pMD2 = new MovedData( rData2, pDiscard2 );

        // forget the arrays again
        delete [] pDiscard1; delete [] pDiscard2;
    }

    {
        CompareSequence aTmp( rData1, rData2, *pMD1, *pMD2 );
    }

    ShiftBoundaries( rData1, rData2 );

    delete pMD1;
    delete pMD2;
}

void Compare::CountDifference( const CompareData& rData, sal_uLong* pCounts )
{
    sal_uLong nLen = rData.GetLineCount();
    for( sal_uLong n = 0; n < nLen; ++n )
    {
        sal_uLong nIdx = rData.GetIndex( n );
        ++pCounts[ nIdx ];
    }
}

void Compare::SetDiscard( const CompareData& rData,
                            sal_Char* pDiscard, sal_uLong* pCounts )
{
    sal_uLong nLen = rData.GetLineCount();

    // calculate Max with respect to the line count
    sal_uInt16 nMax = 5;
    sal_uLong n;

    for( n = nLen / 64; ( n = n >> 2 ) > 0; )
        nMax <<= 1;

    for( n = 0; n < nLen; ++n )
    {
        sal_uLong nIdx = rData.GetIndex( n );
        if( nIdx )
        {
            nIdx = pCounts[ nIdx ];
            pDiscard[ n ] = !nIdx ? 1 : nIdx > nMax ? 2 : 0;
        }
        else
            pDiscard[ n ] = 0;
    }
}

void Compare::CheckDiscard( sal_uLong nLen, sal_Char* pDiscard )
{
    for( sal_uLong n = 0; n < nLen; ++n )
    {
        if( 2 == pDiscard[ n ] )
            pDiscard[n] = 0;
        else if( pDiscard[ n ] )
        {
            sal_uLong j;
            sal_uLong length;
            sal_uLong provisional = 0;

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
                sal_uLong consec;
                sal_uLong minimum = 1;
                sal_uLong tem = length / 4;

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

Compare::MovedData::MovedData( CompareData& rData, sal_Char* pDiscard )
    : pIndex( 0 ), pLineNum( 0 ), nCount( 0 )
{
    sal_uLong nLen = rData.GetLineCount();
    sal_uLong n;

    for( n = 0; n < nLen; ++n )
        if( pDiscard[ n ] )
            rData.SetChanged( n );
        else
            ++nCount;

    if( nCount )
    {
        pIndex = new sal_uLong[ nCount ];
        pLineNum = new sal_uLong[ nCount ];

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
    delete [] pIndex;
    delete [] pLineNum;
}

// Find the differing lines
Compare::CompareSequence::CompareSequence(
                            CompareData& rD1, CompareData& rD2,
                            const MovedData& rMD1, const MovedData& rMD2 )
    : rData1( rD1 ), rData2( rD2 ), rMoved1( rMD1 ), rMoved2( rMD2 )
{
    sal_uLong nSize = rMD1.GetCount() + rMD2.GetCount() + 3;
    pMemory = new long[ nSize * 2 ];
    pFDiag = pMemory + ( rMD2.GetCount() + 1 );
    pBDiag = pMemory + ( nSize + rMD2.GetCount() + 1 );

    Compare( 0, rMD1.GetCount(), 0, rMD2.GetCount() );
}

Compare::CompareSequence::~CompareSequence()
{
    delete [] pMemory;
}

void Compare::CompareSequence::Compare( sal_uLong nStt1, sal_uLong nEnd1,
                                        sal_uLong nStt2, sal_uLong nEnd2 )
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
        sal_uLong c, d, b;

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

sal_uLong Compare::CompareSequence::CheckDiag( sal_uLong nStt1, sal_uLong nEnd1,
                                    sal_uLong nStt2, sal_uLong nEnd2, sal_uLong* pCost )
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

        /* Extend the top-down search by an edit step in each diagonal. */
        fmin > dmin ? pFDiag[--fmin - 1] = -1 : ++fmin;
        fmax < dmax ? pFDiag[++fmax + 1] = -1 : --fmax;
        for (d = fmax; d >= fmin; d -= 2)
        {
            long x, y, tlo = pFDiag[d - 1], thi = pFDiag[d + 1];

            if (tlo >= thi)
                x = tlo + 1;
            else
                x = thi;
            y = x - d;
            while( sal_uLong(x) < nEnd1 && sal_uLong(y) < nEnd2 &&
                rMoved1.GetIndex( x ) == rMoved2.GetIndex( y ))
                ++x, ++y;
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
            long x, y, tlo = pBDiag[d - 1], thi = pBDiag[d + 1];

            if (tlo < thi)
                x = tlo;
            else
                x = thi - 1;
            y = x - d;
            while( sal_uLong(x) > nStt1 && sal_uLong(y) > nStt2 &&
                rMoved1.GetIndex( x - 1 ) == rMoved2.GetIndex( y - 1 ))
                --x, --y;
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
    for( int iz = 0; iz < 2; ++iz )
    {
        CompareData* pData = &rData1;
        CompareData* pOtherData = &rData2;

        sal_uLong i = 0;
        sal_uLong j = 0;
        sal_uLong i_end = pData->GetLineCount();
        sal_uLong preceding = ULONG_MAX;
        sal_uLong other_preceding = ULONG_MAX;

        while (1)
        {
            sal_uLong start, other_start;

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

class SwCompareLine : public CompareLine
{
    const SwNode& rNode;
public:
    SwCompareLine( const SwNode& rNd );
    virtual ~SwCompareLine();

    virtual sal_uLong GetHashValue() const;
    virtual sal_Bool Compare( const CompareLine& rLine ) const;

    static sal_uLong GetTxtNodeHashValue( const SwTxtNode& rNd, sal_uLong nVal );
    static sal_Bool CompareNode( const SwNode& rDstNd, const SwNode& rSrcNd );
    static sal_Bool CompareTxtNd( const SwTxtNode& rDstNd,
                              const SwTxtNode& rSrcNd );

    sal_Bool ChangesInLine( const SwCompareLine& rLine,
                            SwPaM *& rpInsRing, SwPaM*& rpDelRing ) const;

    const SwNode& GetNode() const { return rNode; }

    const SwNode& GetEndNode() const;

    // for debugging
    String GetText() const;
};

class SwCompareData : public CompareData
{
    SwDoc& rDoc;
    SwPaM *pInsRing, *pDelRing;

    sal_uLong PrevIdx( const SwNode* pNd );
    sal_uLong NextIdx( const SwNode* pNd );

    virtual void CheckRanges( CompareData& );
    virtual void ShowInsert( sal_uLong nStt, sal_uLong nEnd );
    virtual void ShowDelete( const CompareData& rData, sal_uLong nStt,
                                sal_uLong nEnd, sal_uLong nInsPos );

    virtual void CheckForChangesInLine( const CompareData& rData,
                                    sal_uLong& nStt, sal_uLong& nEnd,
                                    sal_uLong& nThisStt, sal_uLong& nThisEnd );

public:
    SwCompareData( SwDoc& rD ) : rDoc( rD ), pInsRing(0), pDelRing(0) {}
    virtual ~SwCompareData();

    void SetRedlinesToDoc( sal_Bool bUseDocInfo );
};

SwCompareLine::SwCompareLine( const SwNode& rNd )
    : rNode( rNd )
{
}

SwCompareLine::~SwCompareLine()
{
}

sal_uLong SwCompareLine::GetHashValue() const
{
    sal_uLong nRet = 0;
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
        {
            String sStr( GetText() );
            for( xub_StrLen n = 0; n < sStr.Len(); ++n )
                ( nRet <<= 1 ) += sStr.GetChar( n );
        }
        break;

    case ND_GRFNODE:
    case ND_OLENODE:
        // Fixed ID? Should never occur ...
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
        {
            const SwSectionNode& rSNd = (SwSectionNode&)rNode;
            const SwSection& rSect = rSNd.GetSection();
            if( CONTENT_SECTION != rSect.GetType() || rSect.IsProtect() )
                pNd = rNode.EndOfSectionNode();
        }
        break;
    }
    return *pNd;
}

sal_Bool SwCompareLine::Compare( const CompareLine& rLine ) const
{
    return CompareNode( rNode, ((SwCompareLine&)rLine).rNode );
}

namespace
{
    static String SimpleTableToText(const SwNode &rNode)
    {
        String sRet;
        const SwNode* pEndNd = rNode.EndOfSectionNode();
        SwNodeIndex aIdx( rNode );
        while (&aIdx.GetNode() != pEndNd)
        {
            if (aIdx.GetNode().IsTxtNode())
            {
                if (sRet.Len())
                {
                    sRet.Append( '\n' );
                }
                sRet.Append( aIdx.GetNode().GetTxtNode()->GetExpandTxt() );
            }
            aIdx++;
        }
        return sRet;
    }
}

sal_Bool SwCompareLine::CompareNode( const SwNode& rDstNd, const SwNode& rSrcNd )
{
    if( rSrcNd.GetNodeType() != rDstNd.GetNodeType() )
        return sal_False;

    sal_Bool bRet = sal_False;

    switch( rDstNd.GetNodeType() )
    {
    case ND_TEXTNODE:
        bRet = CompareTxtNd( (SwTxtNode&)rDstNd, (SwTxtNode&)rSrcNd )
            && ( !CmpOptions.bUseRsid || ((SwTxtNode&)rDstNd).CompareParRsid( (SwTxtNode&)rSrcNd ) );
        break;

    case ND_TABLENODE:
        {
            const SwTableNode& rTSrcNd = (SwTableNode&)rSrcNd;
            const SwTableNode& rTDstNd = (SwTableNode&)rDstNd;

            bRet = ( rTSrcNd.EndOfSectionIndex() - rTSrcNd.GetIndex() ) ==
                   ( rTDstNd.EndOfSectionIndex() - rTDstNd.GetIndex() );

            // --> #i107826#: compare actual table content
            if (bRet)
            {
                bRet = (SimpleTableToText(rSrcNd) == SimpleTableToText(rDstNd));
            }
        }
        break;

    case ND_SECTIONNODE:
        {
            const SwSectionNode& rSSrcNd = (SwSectionNode&)rSrcNd,
                               & rSDstNd = (SwSectionNode&)rDstNd;
            const SwSection& rSrcSect = rSSrcNd.GetSection(),
                           & rDstSect = rSDstNd.GetSection();
            SectionType eSrcSectType = rSrcSect.GetType(),
                        eDstSectType = rDstSect.GetType();
            switch( eSrcSectType )
            {
            case CONTENT_SECTION:
                bRet = CONTENT_SECTION == eDstSectType &&
                        rSrcSect.IsProtect() == rDstSect.IsProtect();
                if( bRet && rSrcSect.IsProtect() )
                {
                    // the only have they both the same size
                    bRet = ( rSSrcNd.EndOfSectionIndex() - rSSrcNd.GetIndex() ) ==
                              ( rSDstNd.EndOfSectionIndex() - rSDstNd.GetIndex() );
                }
                break;

            case TOX_HEADER_SECTION:
            case TOX_CONTENT_SECTION:
                if( TOX_HEADER_SECTION == eDstSectType ||
                    TOX_CONTENT_SECTION == eDstSectType )
                {
                    // the same type of TOX?
                    const SwTOXBase* pSrcTOX = rSrcSect.GetTOXBase();
                    const SwTOXBase* pDstTOX = rDstSect.GetTOXBase();
                    bRet =  pSrcTOX && pDstTOX
                            && pSrcTOX->GetType() == pDstTOX->GetType()
                            && pSrcTOX->GetTitle() == pDstTOX->GetTitle()
                            && pSrcTOX->GetTypeName() == pDstTOX->GetTypeName()
//                          && pSrcTOX->GetTOXName() == pDstTOX->GetTOXName()
                            ;
                }
                break;

            case DDE_LINK_SECTION:
            case FILE_LINK_SECTION:
                bRet = eSrcSectType == eDstSectType &&
                        rSrcSect.GetLinkFileName() ==
                        rDstSect.GetLinkFileName();
                break;
            }
        }
        break;

    case ND_ENDNODE:
        bRet = rSrcNd.StartOfSectionNode()->GetNodeType() ==
               rDstNd.StartOfSectionNode()->GetNodeType();

        // --> #i107826#: compare actual table content
        if (bRet && rSrcNd.StartOfSectionNode()->GetNodeType() == ND_TABLENODE)
        {
            bRet = CompareNode(
                *rSrcNd.StartOfSectionNode(), *rDstNd.StartOfSectionNode());
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
            sRet = SimpleTableToText(rNode);
            sRet.InsertAscii( "Tabelle: ", 0 );
        }
        break;

    case ND_SECTIONNODE:
        {
            sRet.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Section - Node:" ));

            const SwSectionNode& rSNd = (SwSectionNode&)rNode;
            const SwSection& rSect = rSNd.GetSection();
            switch( rSect.GetType() )
            {
            case CONTENT_SECTION:
                if( rSect.IsProtect() )
                    sRet.Append( String::CreateFromInt32(
                            rSNd.EndOfSectionIndex() - rSNd.GetIndex() ));
                break;

            case TOX_HEADER_SECTION:
            case TOX_CONTENT_SECTION:
                {
                    const SwTOXBase* pTOX = rSect.GetTOXBase();
                    if( pTOX )
                        sRet.Append( pTOX->GetTitle() )
                            .Append( pTOX->GetTypeName() )
//                          .Append( pTOX->GetTOXName() )
                            .Append( String::CreateFromInt32( pTOX->GetType() ));
                }
                break;

            case DDE_LINK_SECTION:
            case FILE_LINK_SECTION:
                sRet += rSect.GetLinkFileName();
                break;
            }
        }
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

sal_uLong SwCompareLine::GetTxtNodeHashValue( const SwTxtNode& rNd, sal_uLong nVal )
{
    String sStr( rNd.GetExpandTxt() );
    for( xub_StrLen n = 0; n < sStr.Len(); ++n )
        ( nVal <<= 1 ) += sStr.GetChar( n );
    return nVal;
}

sal_Bool SwCompareLine::CompareTxtNd( const SwTxtNode& rDstNd,
                                  const SwTxtNode& rSrcNd )
{
    sal_Bool bRet = sal_False;
    // Very simple at first
    if( rDstNd.GetTxt() == rSrcNd.GetTxt() )
    {
        // The text is the same, but are the "special attributes" (0xFF) also the same?
        bRet = sal_True;
    }
    return bRet;
}

sal_Bool SwCompareLine::ChangesInLine( const SwCompareLine& rLine,
                            SwPaM *& rpInsRing, SwPaM*& rpDelRing ) const
{
    sal_Bool bRet = sal_False;

    // Only compare textnodes
    if( ND_TEXTNODE == rNode.GetNodeType() &&
        ND_TEXTNODE == rLine.GetNode().GetNodeType() )
    {
        SwTxtNode& rDstNd = *(SwTxtNode*)rNode.GetTxtNode();
        const SwTxtNode& rSrcNd = *rLine.GetNode().GetTxtNode();
        SwDoc* pDstDoc = rDstNd.GetDoc();

        int nLcsLen = 0;

        int nDstLen = rDstNd.GetTxt().Len();
        int nSrcLen = rSrcNd.GetTxt().Len();

        int nMinLen = std::min( nDstLen , nSrcLen );
        int nAvgLen = ( nDstLen + nSrcLen )/2;

        int *pLcsDst = new int[ nMinLen + 1 ];
        int *pLcsSrc = new int[ nMinLen + 1 ];

        if( CmpOptions.eCmpMode == SVX_CMP_BY_WORD )
        {
            int *pTmpLcsDst = new int[ nMinLen + 1 ];
            int *pTmpLcsSrc = new int[ nMinLen + 1 ];

            WordArrayComparator aCmp( &rDstNd, &rSrcNd );

            LgstCommonSubseq aSeq( aCmp );

            nLcsLen = aSeq.Find( pTmpLcsDst, pTmpLcsSrc );

            if( CmpOptions.nIgnoreLen )
            {
                nLcsLen = aSeq.IgnoreIsolatedPieces( pTmpLcsDst, pTmpLcsSrc,
                                                aCmp.GetLen1(), aCmp.GetLen2(),
                                                nLcsLen, CmpOptions.nIgnoreLen );
            }

            nLcsLen = aCmp.GetCharSequence( pTmpLcsDst, pTmpLcsSrc,
                                            pLcsDst, pLcsSrc, nLcsLen );

            delete[] pTmpLcsDst;
            delete[] pTmpLcsSrc;
        }
        else
        {
            CharArrayComparator aCmp( &rDstNd, &rSrcNd );
            LgstCommonSubseq aSeq( aCmp );

            nLcsLen = aSeq.Find( pLcsDst, pLcsSrc );

            if( CmpOptions.nIgnoreLen )
            {
                nLcsLen = aSeq.IgnoreIsolatedPieces( pLcsDst, pLcsSrc, nDstLen,
                                                    nSrcLen, nLcsLen,
                                                    CmpOptions.nIgnoreLen );
            }
        }

        // find the sum of the squares of the continuous substrings
        int nSqSum = 0;
        int nCnt = 1;
        for( int i = 0; i < nLcsLen; i++ )
        {
            if( i != nLcsLen - 1 && pLcsDst[i] + 1 == pLcsDst[i + 1]
                                && pLcsSrc[i] + 1 == pLcsSrc[i + 1] )
            {
                nCnt++;
            }
            else
            {
                nSqSum += nCnt*nCnt;
                nCnt = 1;
            }
        }

        // Don't compare if there aren't enough similarities
        if ( nAvgLen >= 8 && nSqSum*32 < nAvgLen*nAvgLen )
        {
            return sal_False;
        }

        // Show the differences
        int nSkip = 0;
        for( int i = 0; i <= nLcsLen; i++ )
        {
            int nDstFrom = i ? (pLcsDst[i - 1] + 1) : 0;
            int nDstTo = ( i == nLcsLen ) ? nDstLen : pLcsDst[i];
            int nSrcFrom = i ? (pLcsSrc[i - 1] + 1) : 0;
            int nSrcTo = ( i == nLcsLen ) ? nSrcLen : pLcsSrc[i];

            SwPaM aPam( rDstNd, nDstTo + nSkip );

            if ( nDstFrom < nDstTo )
            {
                SwPaM* pTmp = new SwPaM( *aPam.GetPoint(), rpInsRing );
                if( !rpInsRing )
                    rpInsRing = pTmp;
                pTmp->SetMark();
                pTmp->GetMark()->nContent = nDstFrom + nSkip;
            }

            if ( nSrcFrom < nSrcTo )
            {
                sal_Bool bUndo = pDstDoc->GetIDocumentUndoRedo().DoesUndo();
                pDstDoc->GetIDocumentUndoRedo().DoUndo( sal_False );
                SwPaM aCpyPam( rSrcNd, nSrcFrom );
                aCpyPam.SetMark();
                aCpyPam.GetPoint()->nContent = nSrcTo;
                aCpyPam.GetDoc()->CopyRange( aCpyPam, *aPam.GetPoint(),
                    false );
                pDstDoc->GetIDocumentUndoRedo().DoUndo( bUndo );

                SwPaM* pTmp = new SwPaM( *aPam.GetPoint(), rpDelRing );
                if( !rpDelRing )
                    rpDelRing = pTmp;

                pTmp->SetMark();
                pTmp->GetMark()->nContent = nDstTo + nSkip;
                nSkip += nSrcTo - nSrcFrom;

                if( rpInsRing )
                {
                    SwPaM* pCorr = (SwPaM*)rpInsRing->GetPrev();
                    if( *pCorr->GetPoint() == *pTmp->GetPoint() )
                        *pCorr->GetPoint() = *pTmp->GetMark();
                }
            }
        }

        delete[] pLcsDst;
        delete[] pLcsSrc;

        bRet = sal_True;
    }

    return bRet;
}

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

sal_uLong SwCompareData::NextIdx( const SwNode* pNd )
{
    if( pNd->IsStartNode() )
    {
        const SwSectionNode* pSNd;
        if( pNd->IsTableNode() ||
            ( 0 != (pSNd = pNd->GetSectionNode() ) &&
                ( CONTENT_SECTION != pSNd->GetSection().GetType() ||
                    pSNd->GetSection().IsProtect() ) ) )
            pNd = pNd->EndOfSectionNode();
    }
    return pNd->GetIndex() + 1;
}

sal_uLong SwCompareData::PrevIdx( const SwNode* pNd )
{
    if( pNd->IsEndNode() )
    {
        const SwSectionNode* pSNd;
        if( pNd->StartOfSectionNode()->IsTableNode() ||
            ( 0 != (pSNd = pNd->StartOfSectionNode()->GetSectionNode() ) &&
                ( CONTENT_SECTION != pSNd->GetSection().GetType() ||
                    pSNd->GetSection().IsProtect() ) ) )
            pNd = pNd->StartOfSectionNode();
    }
    return pNd->GetIndex() - 1;
}

void SwCompareData::CheckRanges( CompareData& rData )
{
    const SwNodes& rSrcNds = ((SwCompareData&)rData).rDoc.GetNodes();
    const SwNodes& rDstNds = rDoc.GetNodes();

    const SwNode& rSrcEndNd = rSrcNds.GetEndOfContent();
    const SwNode& rDstEndNd = rDstNds.GetEndOfContent();

    sal_uLong nSrcSttIdx = NextIdx( rSrcEndNd.StartOfSectionNode() );
    sal_uLong nSrcEndIdx = rSrcEndNd.GetIndex();

    sal_uLong nDstSttIdx = NextIdx( rDstEndNd.StartOfSectionNode() );
    sal_uLong nDstEndIdx = rDstEndNd.GetIndex();

    while( nSrcSttIdx < nSrcEndIdx && nDstSttIdx < nDstEndIdx )
    {
        const SwNode* pSrcNd = rSrcNds[ nSrcSttIdx ];
        const SwNode* pDstNd = rDstNds[ nDstSttIdx ];
        if( !SwCompareLine::CompareNode( *pSrcNd, *pDstNd ))
            break;

        nSrcSttIdx = NextIdx( pSrcNd );
        nDstSttIdx = NextIdx( pDstNd );
    }

    nSrcEndIdx = PrevIdx( &rSrcEndNd );
    nDstEndIdx = PrevIdx( &rDstEndNd );
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

void SwCompareData::ShowInsert( sal_uLong nStt, sal_uLong nEnd )
{
    SwPaM* pTmp = new SwPaM( ((SwCompareLine*)GetLine( nStt ))->GetNode(), 0,
                            ((SwCompareLine*)GetLine( nEnd-1 ))->GetEndNode(), 0,
                             pInsRing );
    if( !pInsRing )
        pInsRing = pTmp;

    // #i65201#: These SwPaMs are calculated smaller than needed, see comment below

}

void SwCompareData::ShowDelete( const CompareData& rData, sal_uLong nStt,
                                sal_uLong nEnd, sal_uLong nInsPos )
{
    SwNodeRange aRg(
        ((SwCompareLine*)rData.GetLine( nStt ))->GetNode(), 0,
        ((SwCompareLine*)rData.GetLine( nEnd-1 ))->GetEndNode(), 1 );

    sal_uInt16 nOffset = 0;
    const CompareLine* pLine = 0;
    if( nInsPos >= 1 )
    {
        if( GetLineCount() == nInsPos )
        {
            pLine = GetLine( nInsPos-1 );
            nOffset = 1;
        }
        else
            pLine = GetLine( nInsPos );
    }

    const SwNode* pLineNd;
    if( pLine )
    {
        if( nOffset )
            pLineNd = &((SwCompareLine*)pLine)->GetEndNode();
        else
            pLineNd = &((SwCompareLine*)pLine)->GetNode();
    }
    else
    {
        pLineNd = &rDoc.GetNodes().GetEndOfContent();
        nOffset = 0;
    }

    SwNodeIndex aInsPos( *pLineNd, nOffset );
    SwNodeIndex aSavePos( aInsPos, -1 );

    ((SwCompareData&)rData).rDoc.CopyWithFlyInFly( aRg, 0, aInsPos );
    rDoc.SetModified();
    aSavePos++;

    // #i65201#: These SwPaMs are calculated when the (old) delete-redlines are hidden,
    // they will be inserted when the delete-redlines are shown again.
    // To avoid unwanted insertions of delete-redlines into these new redlines, what happens
    // especially at the end of the document, I reduce the SwPaM by one node.
    // Before the new redlines are inserted, they have to expand again.
    SwPaM* pTmp = new SwPaM( aSavePos.GetNode(), aInsPos.GetNode(), 0, -1, pDelRing );
    if( !pDelRing )
        pDelRing = pTmp;

    if( pInsRing )
    {
        SwPaM* pCorr = (SwPaM*)pInsRing->GetPrev();
        if( *pCorr->GetPoint() == *pTmp->GetPoint() )
        {
            SwNodeIndex aTmpPos( pTmp->GetMark()->nNode, -1 );
            *pCorr->GetPoint() = SwPosition( aTmpPos );
        }
    }
}

void SwCompareData::CheckForChangesInLine( const CompareData& rData,
                                    sal_uLong& rStt, sal_uLong& rEnd,
                                    sal_uLong& rThisStt, sal_uLong& rThisEnd )
{
    LineArrayComparator aCmp( (CompareData&)*this, rData, rThisStt, rThisEnd,
                              rStt, rEnd );

    int nMinLen = std::min( aCmp.GetLen1(), aCmp.GetLen2() );
    int *pLcsDst = new int[ nMinLen ];
    int *pLcsSrc = new int[ nMinLen ];

    FastCommonSubseq subseq( aCmp );
    int nLcsLen = subseq.Find( pLcsDst, pLcsSrc );
    for (int i = 0; i <= nLcsLen; i++)
    {
        // Beginning of inserted lines (inclusive)
        int nDstFrom = i ? pLcsDst[i - 1] + 1 : 0;
        // End of inserted lines (exclusive)
        int nDstTo = ( i == nLcsLen ) ? aCmp.GetLen1() : pLcsDst[i];
        // Begining of deleted lines (inclusive)
        int nSrcFrom = i ? pLcsSrc[i - 1] + 1 : 0;
        // End of deleted lines (exclusive)
        int nSrcTo = ( i == nLcsLen ) ? aCmp.GetLen2() : pLcsSrc[i];

        if( i )
        {
            SwCompareLine* pDstLn = (SwCompareLine*)GetLine( rThisStt + nDstFrom - 1 );
            SwCompareLine* pSrcLn = (SwCompareLine*)rData.GetLine( rStt + nSrcFrom - 1 );

            // Show differences in detail for lines that
            // were matched as only slightly different
            if( !pDstLn->ChangesInLine( *pSrcLn, pInsRing, pDelRing ) )
            {
                ShowInsert( rThisStt + nDstFrom - 1, rThisStt + nDstFrom );
                ShowDelete( rData, rStt + nSrcFrom - 1, rStt + nSrcFrom,
                                                    rThisStt + nDstFrom );
            }
        }

        // Lines missing from source are inserted
        if( nDstFrom != nDstTo )
        {
            ShowInsert( rThisStt + nDstFrom, rThisStt + nDstTo );
        }

        // Lines missing from destination are deleted
        if( nSrcFrom != nSrcTo )
        {
            ShowDelete( rData, rStt + nSrcFrom, rStt + nSrcTo, rThisStt + nDstTo );
        }
    }
}

void SwCompareData::SetRedlinesToDoc( sal_Bool bUseDocInfo )
{
    SwPaM* pTmp = pDelRing;

    // get the Author / TimeStamp from the "other" document info
    sal_uInt16 nAuthor = rDoc.GetRedlineAuthor();
    DateTime aTimeStamp( DateTime::SYSTEM );
    SwDocShell *pDocShell(rDoc.GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");

        if( bUseDocInfo && xDocProps.is() ) {
            String aTmp( 1 == xDocProps->getEditingCycles()
                                ? xDocProps->getAuthor()
                                : xDocProps->getModifiedBy() );
            util::DateTime uDT( 1 == xDocProps->getEditingCycles()
                                ? xDocProps->getCreationDate()
                                : xDocProps->getModificationDate() );
            Date d(uDT.Day, uDT.Month, uDT.Year);
            Time t(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
            DateTime aDT(d,t);

            if( aTmp.Len() )
            {
                nAuthor = rDoc.InsertRedlineAuthor( aTmp );
                aTimeStamp = aDT;
            }
        }
    }

    if( pTmp )
    {
        SwRedlineData aRedlnData( nsRedlineType_t::REDLINE_DELETE, nAuthor, aTimeStamp,
                                    aEmptyStr, 0, 0 );
        do {
            // #i65201#: Expand again, see comment above.
            if( pTmp->GetPoint()->nContent == 0 )
            {
                pTmp->GetPoint()->nNode++;
                pTmp->GetPoint()->nContent.Assign( pTmp->GetCntntNode(), 0 );
            }
            // #i101009#
            // prevent redlines that end on structural end node
            if (& rDoc.GetNodes().GetEndOfContent() ==
                & pTmp->GetPoint()->nNode.GetNode())
            {
                pTmp->GetPoint()->nNode--;
                SwCntntNode *const pContentNode( pTmp->GetCntntNode() );
                pTmp->GetPoint()->nContent.Assign( pContentNode,
                        (pContentNode) ? pContentNode->Len() : 0 );
            }

            rDoc.DeleteRedline( *pTmp, false, USHRT_MAX );

            if (rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo(new SwUndoCompDoc( *pTmp, sal_False )) ;
                rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
            }
            rDoc.AppendRedline( new SwRedline( aRedlnData, *pTmp ), true );

        } while( pDelRing != ( pTmp = (SwPaM*)pTmp->GetNext() ));
    }

    pTmp = pInsRing;
    if( pTmp )
    {
        do {
            if( pTmp->GetPoint()->nContent == 0 )
            {
                pTmp->GetPoint()->nNode++;
                pTmp->GetPoint()->nContent.Assign( pTmp->GetCntntNode(), 0 );
            }
            // #i101009#
            // prevent redlines that end on structural end node
            if (& rDoc.GetNodes().GetEndOfContent() ==
                & pTmp->GetPoint()->nNode.GetNode())
            {
                pTmp->GetPoint()->nNode--;
                SwCntntNode *const pContentNode( pTmp->GetCntntNode() );
                pTmp->GetPoint()->nContent.Assign( pContentNode,
                        (pContentNode) ? pContentNode->Len() : 0 );
            }
        } while( pInsRing != ( pTmp = (SwPaM*)pTmp->GetNext() ));
        SwRedlineData aRedlnData( nsRedlineType_t::REDLINE_INSERT, nAuthor, aTimeStamp,
                                    aEmptyStr, 0, 0 );

        // combine consecutive
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
                        // are consecutive, so combine
                        rEndStt = *pTmp->Start();
                        delete pTmp;
                        pTmp = pInsRing;
                    }
                    else
                    {
                        // are consecutive, so combine
                        rSttEnd = *((SwPaM*)pTmp->GetNext())->End();
                        delete pTmp->GetNext();
                    }
                }
                else
                    pTmp = (SwPaM*)pTmp->GetNext();
            } while( pInsRing != pTmp );
        }

        do {
            if( rDoc.AppendRedline( new SwRedline( aRedlnData, *pTmp ), true) &&
                rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo(new SwUndoCompDoc( *pTmp, sal_True ));
                rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
            }
        } while( pInsRing != ( pTmp = (SwPaM*)pTmp->GetNext() ));
    }
}

// Returns (the difference count?) if something is different
long SwDoc::CompareDoc( const SwDoc& rDoc )
{
    if( &rDoc == this )
        return 0;

    long nRet = 0;

    // Get comparison options
    CmpOptions.eCmpMode = SW_MOD()->GetCompareMode();
    if( CmpOptions.eCmpMode == SVX_CMP_AUTO )
    {
        if( getRsidRoot() == rDoc.getRsidRoot() )
        {
            CmpOptions.eCmpMode = SVX_CMP_BY_CHAR;
            CmpOptions.bUseRsid = true;
            CmpOptions.nIgnoreLen = 2;
        }
        else
        {
            CmpOptions.eCmpMode = SVX_CMP_BY_WORD;
            CmpOptions.bUseRsid = false;
            CmpOptions.nIgnoreLen = 3;
        }
    }
    else
    {
        CmpOptions.bUseRsid = getRsidRoot() == rDoc.getRsidRoot() && SW_MOD()->IsUseRsid();
        CmpOptions.nIgnoreLen = SW_MOD()->IsIgnorePieces() ? SW_MOD()->GetPieceLen() : 0;
    }

    GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);
    sal_Bool bDocWasModified = IsModified();
    SwDoc& rSrcDoc = (SwDoc&)rDoc;
    sal_Bool bSrcModified = rSrcDoc.IsModified();

    RedlineMode_t eSrcRedlMode = rSrcDoc.GetRedlineMode();
    rSrcDoc.SetRedlineMode( nsRedlineMode_t::REDLINE_SHOW_INSERT );
    SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT));

    SwCompareData aD0( rSrcDoc );
    SwCompareData aD1( *this );

    aD1.CompareLines( aD0 );

    nRet = aD1.ShowDiffs( aD0 );

    if( nRet )
    {
      SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_ON |
                       nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE));

        aD1.SetRedlinesToDoc( !bDocWasModified );
        SetModified();
    }

    rSrcDoc.SetRedlineMode( eSrcRedlMode );
    SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE));

    if( !bSrcModified )
        rSrcDoc.ResetModified();

    GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);

    return nRet;
}

class _SaveMergeRedlines : public Ring
{
    const SwRedline* pSrcRedl;
    SwRedline* pDestRedl;
public:
    _SaveMergeRedlines( const SwNode& rDstNd,
                        const SwRedline& rSrcRedl, Ring* pRing );
    sal_uInt16 InsertRedline();

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

    if( nsRedlineType_t::REDLINE_DELETE == pDestRedl->GetType() )
    {
        // mark the area as deleted
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

sal_uInt16 _SaveMergeRedlines::InsertRedline()
{
    sal_uInt16 nIns = 0;
    SwDoc* pDoc = pDestRedl->GetDoc();

    if( nsRedlineType_t::REDLINE_INSERT == pDestRedl->GetType() )
    {
        // the part was inserted so copy it from the SourceDoc
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

        SwNodeIndex aSaveNd( pDestRedl->GetPoint()->nNode, -1 );
        xub_StrLen nSaveCnt = pDestRedl->GetPoint()->nContent.GetIndex();

        RedlineMode_t eOld = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));

        pSrcRedl->GetDoc()->CopyRange(
                *const_cast<SwPaM*>(static_cast<const SwPaM*>(pSrcRedl)),
                *pDestRedl->GetPoint(), false );

        pDoc->SetRedlineMode_intern( eOld );

        pDestRedl->SetMark();
        aSaveNd++;
        pDestRedl->GetMark()->nNode = aSaveNd;
        pDestRedl->GetMark()->nContent.Assign( aSaveNd.GetNode().GetCntntNode(),
                                                nSaveCnt );

        if( GetPrev() != this )
        {
            SwPaM* pTmpPrev = ((_SaveMergeRedlines*)GetPrev())->pDestRedl;
            if( pTmpPrev && *pTmpPrev->GetPoint() == *pDestRedl->GetPoint() )
                *pTmpPrev->GetPoint() = *pDestRedl->GetMark();
        }
    }
    else
    {
        //JP 21.09.98: Bug 55909
        // If there already is a deleted or inserted one at the same position, we have to split it!
        SwPosition* pDStt = pDestRedl->GetMark(),
                  * pDEnd = pDestRedl->GetPoint();
        sal_uInt16 n = 0;

            // find the first redline for StartPos
        if( !pDoc->GetRedline( *pDStt, &n ) && n )
            --n;

        const SwRedlineTbl& rRedlineTbl = pDoc->GetRedlineTbl();
        for( ; n < rRedlineTbl.size(); ++n )
        {
            SwRedline* pRedl = rRedlineTbl[ n ];
            SwPosition* pRStt = pRedl->Start(),
                      * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                           : pRedl->GetPoint();
            if( nsRedlineType_t::REDLINE_DELETE == pRedl->GetType() ||
                nsRedlineType_t::REDLINE_INSERT == pRedl->GetType() )
            {
                SwComparePosition eCmpPos = ComparePosition( *pDStt, *pDEnd, *pRStt, *pREnd );
                switch( eCmpPos )
                {
                case POS_COLLIDE_START:
                case POS_BEHIND:
                    break;

                case POS_INSIDE:
                case POS_EQUAL:
                    delete pDestRedl, pDestRedl = 0;
                    // break; -> no break !!!!

                case POS_COLLIDE_END:
                case POS_BEFORE:
                    n = rRedlineTbl.size();
                    break;

                case POS_OUTSIDE:
                    {
                        SwRedline* pCpyRedl = new SwRedline(
                            pDestRedl->GetRedlineData(), *pDStt );
                        pCpyRedl->SetMark();
                        *pCpyRedl->GetPoint() = *pRStt;

                        SwUndoCompDoc *const pUndo =
                            (pDoc->GetIDocumentUndoRedo().DoesUndo())
                                    ? new SwUndoCompDoc( *pCpyRedl ) : 0;

                        // now modify doc: append redline, undo (and count)
                        pDoc->AppendRedline( pCpyRedl, true );
                        if( pUndo )
                        {
                            pDoc->GetIDocumentUndoRedo().AppendUndo(pUndo);
                        }
                        ++nIns;

                        *pDStt = *pREnd;

                        // we should start over now
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
        SwUndoCompDoc *const pUndo = (pDoc->GetIDocumentUndoRedo().DoesUndo())
            ? new SwUndoCompDoc( *pDestRedl ) : 0;

        // now modify doc: append redline, undo (and count)
        bool bRedlineAccepted = pDoc->AppendRedline( pDestRedl, true );
        if( pUndo )
        {
            pDoc->GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
        ++nIns;

        // if AppendRedline has deleted our redline, we may not keep a
        // reference to it
        if( ! bRedlineAccepted )
            pDestRedl = NULL;
    }
    return nIns;
}

// Merge two documents
long SwDoc::MergeDoc( const SwDoc& rDoc )
{
    if( &rDoc == this )
        return 0;

    long nRet = 0;

    GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

    SwDoc& rSrcDoc = (SwDoc&)rDoc;
    sal_Bool bSrcModified = rSrcDoc.IsModified();

    RedlineMode_t eSrcRedlMode = rSrcDoc.GetRedlineMode();
    rSrcDoc.SetRedlineMode( nsRedlineMode_t::REDLINE_SHOW_DELETE );
    SetRedlineMode( nsRedlineMode_t::REDLINE_SHOW_DELETE );

    SwCompareData aD0( rSrcDoc );
    SwCompareData aD1( *this );

    aD1.CompareLines( aD0 );

    if( !aD1.HasDiffs( aD0 ) )
    {
        // we want to get all redlines from the SourceDoc

        // look for all insert redlines from the SourceDoc and determine their position in the DestDoc
        _SaveMergeRedlines* pRing = 0;
        const SwRedlineTbl& rSrcRedlTbl = rSrcDoc.GetRedlineTbl();
        sal_uLong nEndOfExtra = rSrcDoc.GetNodes().GetEndOfExtras().GetIndex();
        sal_uLong nMyEndOfExtra = GetNodes().GetEndOfExtras().GetIndex();
        for( sal_uInt16 n = 0; n < rSrcRedlTbl.size(); ++n )
        {
            const SwRedline* pRedl = rSrcRedlTbl[ n ];
            sal_uLong nNd = pRedl->GetPoint()->nNode.GetIndex();
            RedlineType_t eType = pRedl->GetType();
            if( nEndOfExtra < nNd &&
                ( nsRedlineType_t::REDLINE_INSERT == eType || nsRedlineType_t::REDLINE_DELETE == eType ))
            {
                const SwNode* pDstNd = GetNodes()[
                                        nMyEndOfExtra + nNd - nEndOfExtra ];

                // Found the positon.
                // Then we also have to insert the redline to the line in the DestDoc.
                _SaveMergeRedlines* pTmp = new _SaveMergeRedlines(
                                                    *pDstNd, *pRedl, pRing );
                if( !pRing )
                    pRing = pTmp;
            }
        }

        if( pRing )
        {
          // Carry over all into DestDoc
          rSrcDoc.SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE));

          SetRedlineMode((RedlineMode_t)(
                                      nsRedlineMode_t::REDLINE_ON |
                                      nsRedlineMode_t::REDLINE_SHOW_INSERT |
                                      nsRedlineMode_t::REDLINE_SHOW_DELETE));

            _SaveMergeRedlines* pTmp = pRing;

            do {
                nRet += pTmp->InsertRedline();
            } while( pRing != ( pTmp = (_SaveMergeRedlines*)pTmp->GetNext() ));

            while( pRing != pRing->GetNext() )
                delete pRing->GetNext();
            delete pRing;
        }
    }

    rSrcDoc.SetRedlineMode( eSrcRedlMode );
    if( !bSrcModified )
        rSrcDoc.ResetModified();

    SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE));

    GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);

    return nRet;
}

LineArrayComparator::LineArrayComparator( const CompareData &rD1,
                                            const CompareData &rD2, int nStt1,
                                            int nEnd1, int nStt2, int nEnd2 )
    : rData1( rD1 ), rData2( rD2 ), nFirst1( nStt1 ), nFirst2( nStt2 )
{
    nLen1 = nEnd1 - nStt1;
    nLen2 = nEnd2 - nStt2;
}

bool LineArrayComparator::Compare( int nIdx1, int nIdx2 ) const
{
    if( nIdx1 < 0 || nIdx2 < 0 || nIdx1 >= nLen1 || nIdx2 >= nLen2 )
    {
        OSL_ENSURE( 0, "Index out of range!" );
        return false;
    }

    const SwTxtNode *pTxtNd1 = ( ( SwCompareLine* )rData1.GetLine( nFirst1 + nIdx1 ) )->GetNode().GetTxtNode();
    const SwTxtNode *pTxtNd2 = ( ( SwCompareLine* )rData2.GetLine( nFirst2 + nIdx2 ) )->GetNode().GetTxtNode();

    if( !pTxtNd1 || !pTxtNd2
        || ( CmpOptions.bUseRsid && !pTxtNd1->CompareParRsid( *pTxtNd2 ) ) )
    {
        return false;
    }

    int nPar1Len = pTxtNd1->Len();
    int nPar2Len = pTxtNd2->Len();

    if( std::min( nPar1Len, nPar2Len ) * 3 < std::max( nPar1Len, nPar2Len ) )
    {
        return false;
    }

    int nBorderLen = ( nPar1Len + nPar2Len )/16;

    if( nBorderLen < 3 )
    {
        nBorderLen = std::min( 3, std::min( nPar1Len, nPar2Len ) );
    }

    std::set<unsigned> aHashes;
    unsigned nHash = 0;
    unsigned nMul = 251;
    unsigned nPow = 1;
    int i;

    for( i = 0; i < nBorderLen - 1; i++ )
    {
        nPow *= nMul;
    }
    for( i = 0; i < nBorderLen; i++ )
    {
        nHash = nHash*nMul + pTxtNd1->GetTxt().GetChar( i );
    }
    aHashes.insert( nHash );
    for( ; i < nPar1Len; i++ )
    {
        nHash = nHash - nPow*pTxtNd1->GetTxt().GetChar( i - nBorderLen );
        nHash = nHash*nMul + pTxtNd1->GetTxt().GetChar( i );

        aHashes.insert( nHash );
    }

    nHash = 0;
    for( i = 0; i < nBorderLen; i++ )
    {
        nHash = nHash*nMul + pTxtNd2->GetTxt().GetChar( i );
    }

    if( aHashes.find( nHash ) != aHashes.end() )
    {
        return true;
    }

    for( ; i < nPar2Len; i++ )
    {
        nHash = nHash - nPow*pTxtNd2->GetTxt().GetChar( i - nBorderLen );
        nHash = nHash*nMul + pTxtNd2->GetTxt().GetChar( i );
        if( aHashes.find( nHash ) != aHashes.end() )
        {
            return true;
        }
    }
    return false;
}

bool CharArrayComparator::Compare( int nIdx1, int nIdx2 ) const
{
    if( nIdx1 < 0 || nIdx2 < 0 || nIdx1 >= GetLen1() || nIdx2 >= GetLen2() )
    {
        OSL_ENSURE( 0, "Index out of range!" );
        return false;
    }

    return ( !CmpOptions.bUseRsid
            || pTxtNd1->CompareRsid(  *pTxtNd2, nIdx1 + 1, nIdx2 + 1 ) )
            && pTxtNd1->GetTxt().GetChar( nIdx1 )
            == pTxtNd2->GetTxt().GetChar( nIdx2 );
}

WordArrayComparator::WordArrayComparator( const SwTxtNode *pNode1,
                                            const SwTxtNode *pNode2 )
    : pTxtNd1( pNode1 ), pTxtNd2( pNode2 )
{
    pPos1 = new int[ pTxtNd1->GetTxt().Len() + 1 ];
    pPos2 = new int[ pTxtNd2->GetTxt().Len() + 1 ];

    CalcPositions( pPos1, pTxtNd1, nCnt1 );
    CalcPositions( pPos2, pTxtNd2, nCnt2 );
}

WordArrayComparator::~WordArrayComparator()
{
    delete[] pPos1;
    delete[] pPos2;
}

bool WordArrayComparator::Compare( int nIdx1, int nIdx2 ) const
{
    int nLen = pPos1[ nIdx1 + 1 ] - pPos1[ nIdx1 ];
    if( nLen != pPos2[ nIdx2 + 1 ] - pPos2[ nIdx2 ] )
    {
        return false;
    }
    for( int i = 0; i < nLen; i++)
    {
        if( pTxtNd1->GetTxt().GetChar( pPos1[ nIdx1 ] + i )
            != pTxtNd2->GetTxt().GetChar( pPos2[ nIdx2 ] + i )
            || ( CmpOptions.bUseRsid && !pTxtNd1->CompareRsid( *pTxtNd2,
                                pPos1[ nIdx1 ] + i, pPos2[ nIdx2 ] + i ) ) )
        {
            return false;
        }
    }
    return true;
}

int WordArrayComparator::GetCharSequence( const int *pWordLcs1,
            const int *pWordLcs2, int *pSubseq1, int *pSubseq2, int nLcsLen )
{
    int nLen = 0;
    for( int i = 0; i < nLcsLen; i++ )
    {
        // Check for hash collisions
        if( pPos1[ pWordLcs1[i] + 1 ] - pPos1[ pWordLcs1[i] ]
            != pPos2[ pWordLcs2[i] + 1 ] - pPos2[ pWordLcs2[i] ] )
        {
            continue;
        }
        for( int j = 0; j < pPos1[pWordLcs1[i]+1] - pPos1[pWordLcs1[i]]; j++)
        {
            pSubseq1[ nLen ] = pPos1[ pWordLcs1[i] ] + j;
            pSubseq2[ nLen ] = pPos2[ pWordLcs2[i] ] + j;

            if( pTxtNd1->GetTxt().GetChar( pPos1[ pWordLcs1[i] ] + j )
             != pTxtNd2->GetTxt().GetChar( pPos2[ pWordLcs2[i] ] + j ) )
            {
                nLen -= j;
                break;
            }

            nLen++;
        }
    }
    return nLen;
}

void WordArrayComparator::CalcPositions( int *pPos, const SwTxtNode *pTxtNd,
                                         int &nCnt )
{
    nCnt = -1;
    for( int i = 0; i <= pTxtNd->GetTxt().Len(); i++ )
    {
        if( i == 0 || i == pTxtNd->GetTxt().Len()
                    || !isalnum( pTxtNd->GetTxt().GetChar( i - 1 ) )
                    || !isalnum( pTxtNd->GetTxt().GetChar( i ) ) )
        { // Begin new word
            nCnt++;
            pPos[ nCnt ] = i;
        }
    }
}


int CommonSubseq::FindLCS( int *pLcs1, int *pLcs2, int nStt1, int nEnd1,
                                                    int nStt2, int nEnd2 )
{
    int nLen1 = nEnd1 ? nEnd1 - nStt1 : rCmp.GetLen1();
    int nLen2 = nEnd2 ? nEnd2 - nStt2 : rCmp.GetLen2();

    OSL_ASSERT( nLen1 >= 0 );
    OSL_ASSERT( nLen2 >= 0 );

    int **pLcs = new int*[ nLen1 + 1 ];
    pLcs[ 0 ] = pData;

    for( int i = 1; i < nLen1 + 1; i++ )
        pLcs[ i ] = pLcs[ i - 1 ] + nLen2 + 1;

    for( int i = 0; i <= nLen1; i++ )
        pLcs[i][0] = 0;

    for( int j = 0; j <= nLen2; j++ )
        pLcs[0][j] = 0;

    // Find lcs
    for( int i = 1; i <= nLen1; i++ )
    {
        for( int j = 1; j <= nLen2; j++ )
        {
            if( rCmp.Compare( nStt1 + i - 1, nStt2 + j - 1 ) )
                pLcs[i][j] = pLcs[i - 1][j - 1] + 1;
            else
                pLcs[i][j] = std::max( pLcs[i][j - 1], pLcs[i - 1][j] );
        }
    }

    int nLcsLen = pLcs[ nLen1 ][ nLen2 ];

    // Recover the lcs in the two sequences
    if( pLcs1 && pLcs2 )
    {
        int nIdx1 = nLen1;
        int nIdx2 = nLen2;
        int nIdx = nLcsLen - 1;

        while( nIdx1 > 0 && nIdx2 > 0 )
        {
            if( pLcs[ nIdx1 ][ nIdx2 ] == pLcs[ nIdx1 - 1 ][ nIdx2 ] )
                nIdx1--;
            else if( pLcs[ nIdx1 ][ nIdx2 ] == pLcs[ nIdx1 ][ nIdx2 - 1 ] )
                nIdx2--;
            else
            {
                nIdx1--, nIdx2--;
                pLcs1[ nIdx ] = nIdx1 + nStt1;
                pLcs2[ nIdx ] = nIdx2 + nStt2;
                nIdx--;
            }
        }
    }

    delete[] pLcs;

    return nLcsLen;
}

int CommonSubseq::IgnoreIsolatedPieces( int *pLcs1, int *pLcs2, int nLen1,
                                        int nLen2, int nLcsLen, int nPieceLen )
{
    if( !nLcsLen )
    {
        return 0;
    }

    int nNext = 0;

    // Don't ignore text at the beginning of the paragraphs
    if( pLcs1[ 0 ] == 0 && pLcs2[ 0 ] == 0 )
    {
        while( nNext < nLcsLen - 1 && pLcs1[ nNext ] + 1 == pLcs1[ nNext + 1 ]
                                && pLcs2[ nNext ] + 1 == pLcs2[ nNext + 1 ] )
        {
            nNext++;
        }
        nNext++;
    }

    int nCnt = 1;

    for( int i = nNext; i < nLcsLen; i++ )
    {
        if( i != nLcsLen - 1 && pLcs1[ i ] + 1 == pLcs1[ i + 1 ]
                            && pLcs2[ i ] + 1 == pLcs2[ i + 1 ] )
        {
            nCnt++;
        }
        else
        {
            if( nCnt > nPieceLen
                // Don't ignore text at the end of the paragraphs
                || ( i == nLcsLen - 1
                && pLcs1[i] == nLen1 - 1 && pLcs2[i] == nLen2 - 1 ))
            {
                for( int j = i + 1 - nCnt; j <= i; j++ )
                {
                    pLcs2[ nNext ] = pLcs2[ j ];
                    pLcs1[ nNext ] = pLcs1[ j ];
                    nNext++;
                }
            }
            nCnt = 1;
        }
    }

    return nNext;
}

LgstCommonSubseq::LgstCommonSubseq( ArrayComparator &rComparator )
    : CommonSubseq( rComparator, CUTOFF )
{
    pBuff1 = new int[ rComparator.GetLen2() + 1 ];
    pBuff2 = new int[ rComparator.GetLen2() + 1 ];

    pL1 = new int[ rComparator.GetLen2() + 1 ];
    pL2 = new int[ rComparator.GetLen2() + 1 ];
}

LgstCommonSubseq::~LgstCommonSubseq()
{
    delete[] pBuff1;
    delete[] pBuff2;

    delete[] pL1;
    delete[] pL2;
}

void LgstCommonSubseq::FindL( int *pL, int nStt1, int nEnd1,
                                        int nStt2, int nEnd2  )
{
    int nLen1 = nEnd1 ? nEnd1 - nStt1 : rCmp.GetLen1();
    int nLen2 = nEnd2 ? nEnd2 - nStt2 : rCmp.GetLen2();

    int *currL = pBuff1;
    int *prevL = pBuff2;

    // Avoid memory corruption
    if( nLen2 > rCmp.GetLen2() )
    {
        assert( false );
        return;
    }

    memset( pBuff1, 0, sizeof( *pBuff1 ) * ( nLen2 + 1 ) );
    memset( pBuff2, 0, sizeof( *pBuff2 ) * ( nLen2 + 1 ) );

    // Find lcs
    for( int i = 1; i <= nLen1; i++ )
    {
        for( int j = 1; j <= nLen2; j++ )
        {
            if( rCmp.Compare( nStt1 + i - 1, nStt2 + j - 1 ) )
                currL[j] = prevL[j - 1] + 1;
            else
                currL[j] = std::max( currL[j - 1], prevL[j] );
        }
        int *tmp = currL;
        currL = prevL;
        prevL = tmp;
    }
    memcpy( pL, prevL, ( nLen2 + 1 ) * sizeof( *prevL ) );
}

int LgstCommonSubseq::HirschbergLCS( int *pLcs1, int *pLcs2, int nStt1,
                                    int nEnd1, int nStt2, int nEnd2 )
{
    static int nLen1;
    static int nLen2;
    nLen1 = nEnd1 - nStt1;
    nLen2 = nEnd2 - nStt2;

    if( ( nLen1 + 1 ) * ( nLen2 + 1 ) <= CUTOFF )
    {
        if( !nLen1 || !nLen2 )
        {
            return 0;
        }
        return FindLCS(pLcs1, pLcs2, nStt1, nEnd1, nStt2, nEnd2);
    }

    int nMid = nLen1/2;

    FindL( pL1, nStt1, nStt1 + nMid, nStt2, nEnd2 );
    FindL( pL2, nStt1 + nMid, nEnd1, nStt2, nEnd2 );

    int nMaxPos = 0;
    static int nMaxVal;
    nMaxVal = -1;

    static int i;
    for( i = 0; i <= nLen2; i++ )
    {
        if( pL1[i] + ( pL2[nLen2] - pL2[i] ) > nMaxVal )
        {
            nMaxPos = i;
            nMaxVal = pL1[i]+( pL2[nLen2] - pL2[i] );
        }
    }

    int nRet = HirschbergLCS( pLcs1, pLcs2, nStt1, nStt1 + nMid,
                                            nStt2, nStt2 + nMaxPos );
    nRet += HirschbergLCS( pLcs1 + nRet, pLcs2 + nRet, nStt1 + nMid, nEnd1,
                                                    nStt2 + nMaxPos, nEnd2 );

    return nRet;
}

int LgstCommonSubseq::Find( int *pSubseq1, int *pSubseq2 )
{
    int nStt = 0;
    int nCutEnd = 0;
    int nEnd1 = rCmp.GetLen1();
    int nEnd2 = rCmp.GetLen2();

    // Check for corresponding lines in the beginning of the sequences
    while( nStt < nEnd1 && nStt < nEnd2 && rCmp.Compare( nStt, nStt ) )
    {
        pSubseq1[ nStt ] = nStt;
        pSubseq2[ nStt ] = nStt;
        nStt++;
    }

    pSubseq1 += nStt;
    pSubseq2 += nStt;

    // Check for corresponding lines in the end of the sequences
    while( nStt < nEnd1 && nStt < nEnd2
                        && rCmp.Compare( nEnd1 - 1, nEnd2 - 1 ) )
    {
        nCutEnd++;
        nEnd1--;
        nEnd2--;
    }

    int nLen = HirschbergLCS( pSubseq1, pSubseq2, nStt, nEnd1, nStt, nEnd2 );

    for( int i = 0; i < nCutEnd; i++ )
    {
        pSubseq1[ nLen + i ] = nEnd1 + i;
        pSubseq2[ nLen + i ] = nEnd2 + i;
    }

    return nStt + nLen + nCutEnd;
}

int FastCommonSubseq::FindFastCS( int *pSeq1, int *pSeq2, int nStt1,
                                    int nEnd1, int nStt2, int nEnd2  )
{
    int nCutBeg = 0;
    int nCutEnd = 0;

    // Check for corresponding lines in the beginning of the sequences
    while( nStt1 < nEnd1 && nStt2 < nEnd2 && rCmp.Compare( nStt1, nStt2 ) )
    {
        pSeq1[ nCutBeg ] = nStt1++;
        pSeq2[ nCutBeg ] = nStt2++;
        nCutBeg++;
    }

    pSeq1 += nCutBeg;
    pSeq2 += nCutBeg;

    // Check for corresponding lines in the end of the sequences
    while( nStt1 < nEnd1 && nStt2 < nEnd2
                        && rCmp.Compare( nEnd1 - 1, nEnd2 - 1 ) )
    {
        nCutEnd++;
        nEnd1--;
        nEnd2--;
    }

    int nLen1 = nEnd1 - nStt1;
    int nLen2 = nEnd2 - nStt2;

    // Return if a sequence is empty
    if( nLen1 <= 0 || nLen2 <= 0 )
    {
        for( int i = 0; i < nCutEnd; i++ )
        {
            pSeq1[ i ] = nEnd1 + i;
            pSeq2[ i ] = nEnd2 + i;
        }
        return nCutBeg + nCutEnd;
    }

    // Cut to LCS for small values
    if( nLen1 < 3 || nLen2 < 3 || ( nLen1 + 1 ) * ( nLen2 + 1 ) <= CUTOFF )
    {
        int nLcsLen = FindLCS( pSeq1, pSeq2, nStt1, nEnd1, nStt2, nEnd2);

        for( int i = 0; i < nCutEnd; i++ )
        {
            pSeq1[ nLcsLen + i ] = nEnd1 + i;
            pSeq2[ nLcsLen + i ] = nEnd2 + i;
        }
        return nCutBeg + nLcsLen + nCutEnd;
    }

    int nMid1 = nLen1/2;
    int nMid2 = nLen2/2;

    int nRad;
    int nPos1 = -1, nPos2 = -1;

    // Find a point of correspondence in the middle of the sequences
    for( nRad = 0; nRad*nRad < std::min( nMid1, nMid2 ); nRad++ )
    {
        // Search to the left and to the right of the middle of the first sequence
        for( int i = nMid1 - nRad; i <= nMid1 + nRad; i++ )
        {
            if( rCmp.Compare( nStt1 + i, nStt2 + nMid2 - nRad ) )
            {
                nPos1 = nStt1 + i;
                nPos2 = nStt2 + nMid2 - nRad;
                break;
            }
            if( rCmp.Compare( nStt1 + i, nStt2 + nMid2 + nRad ) )
            {
                nPos1 = nStt1 + i;
                nPos2 = nStt2 + nMid2 - nRad;
                break;
            }
        }
        // Search to the left and to the right of the middle of the second sequence
        for( int i = nMid2 - nRad; i <= nMid2 + nRad; i++ )
        {
            if( rCmp.Compare( nStt2 + nMid2 - nRad, nStt2 + i ) )
            {
                nPos2 = nStt2 + i;
                nPos1 = nStt1 + nMid1 - nRad;
                break;
            }
            if( rCmp.Compare( nStt2 + nMid2 - nRad, nStt2 + i ) )
            {
                nPos2 = nStt2 + i;
                nPos1 = nStt1 + nMid1 - nRad;
                break;
            }
        }
    }

    // return if no point of correspondence found
    if( nPos1 == -1 )
    {
        for( int i = 0; i < nCutEnd; i++ )
        {
            pSeq1[ i ] = nEnd1 + i;
            pSeq2[ i ] = nEnd2 + i;
        }
        return nCutBeg + nCutEnd;
    }

    // Run the same on the sequences to the left of the correspondence point
    int nLen = FindFastCS( pSeq1, pSeq2, nStt1, nPos1, nStt2, nPos2 );

    pSeq1[ nLen ] = nPos1;
    pSeq2[ nLen ] = nPos2;

    // Run the same on the sequences to the right of the correspondence point
    nLen += FindFastCS( pSeq1 + nLen + 1, pSeq2 + nLen + 1,
                         nPos1 + 1, nEnd1, nPos2 + 1, nEnd2 ) + 1;

    for( int i = 0; i < nCutEnd; i++ )
    {
        pSeq1[ nLen + i ] = nEnd1 + i;
        pSeq2[ nLen + i ] = nEnd2 + i;
    }

    return nLen + nCutBeg + nCutEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
