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

#include <sal/config.h>

#include <rtl/ustrbuf.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <swmodule.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <redline.hxx>
#include <UndoRedline.hxx>
#include <section.hxx>
#include <tox.hxx>
#include <docsh.hxx>
#include <fmtcntnt.hxx>
#include <modcfg.hxx>
#include <frameformats.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <cstddef>
#include <memory>
#include <vector>

using namespace ::com::sun::star;

using std::vector;

namespace {

class SwCompareLine
{
    const SwNode& m_rNode;
public:
    explicit SwCompareLine( const SwNode& rNd ) : m_rNode( rNd ) {}

    sal_uLong GetHashValue() const;
    bool Compare( const SwCompareLine& rLine ) const;

    static sal_uLong GetTextNodeHashValue( const SwTextNode& rNd, sal_uLong nVal );
    static bool CompareNode( const SwNode& rDstNd, const SwNode& rSrcNd );
    static bool CompareTextNd( const SwTextNode& rDstNd,
                              const SwTextNode& rSrcNd );

    bool ChangesInLine( const SwCompareLine& rLine,
                            std::unique_ptr<SwPaM>& rpInsRing, std::unique_ptr<SwPaM>& rpDelRing ) const;

    const SwNode& GetNode() const { return m_rNode; }

    const SwNode& GetEndNode() const;

    // for debugging
    OUString GetText() const;
};


class CompareData
{
protected:
    SwDoc& m_rDoc;
private:
    std::unique_ptr<size_t[]> m_pIndex;
    std::unique_ptr<bool[]> m_pChangedFlag;

    std::unique_ptr<SwPaM> m_pInsertRing, m_pDelRing;

    static sal_uLong PrevIdx( const SwNode* pNd );
    static sal_uLong NextIdx( const SwNode* pNd );

    vector< SwCompareLine* > m_aLines;
    bool m_bRecordDiff;

    // Truncate beginning and end and add all others to the LinesArray
    void CheckRanges( CompareData& );

    virtual const SwNode& GetEndOfContent() = 0;

public:
    CompareData(SwDoc& rD, bool bRecordDiff)
        : m_rDoc( rD )
        , m_bRecordDiff(bRecordDiff)
    {
    }
    virtual ~CompareData();

    // Are there differences?
    bool HasDiffs( const CompareData& rData ) const;

    // Triggers the comparison and creation of two documents
    void CompareLines( CompareData& rData );
    // Display the differences - calls the methods ShowInsert and ShowDelete.
    // These are passed the start and end line number.
    // Displaying the actually content is to be handled by the subclass!
    sal_uLong ShowDiffs( const CompareData& rData );

    void ShowInsert( sal_uLong nStt, sal_uLong nEnd );
    void ShowDelete( const CompareData& rData, sal_uLong nStt,
                                sal_uLong nEnd, sal_uLong nInsPos );
    void CheckForChangesInLine( const CompareData& rData,
                                    sal_uLong nStt, sal_uLong nEnd,
                                    sal_uLong nThisStt, sal_uLong nThisEnd );

    // Set non-ambiguous index for a line. Same lines have the same index, even in the other CompareData!
    void SetIndex( size_t nLine, size_t nIndex );
    size_t GetIndex( size_t nLine ) const
        { return nLine < m_aLines.size() ? m_pIndex[ nLine ] : 0; }

    // Set/get of a line has changed
    void SetChanged( size_t nLine, bool bFlag = true );
    bool GetChanged( size_t nLine ) const
        {
            return (m_pChangedFlag && nLine < m_aLines.size())
                && m_pChangedFlag[ nLine ];
        }

    size_t GetLineCount() const     { return m_aLines.size(); }
    const SwCompareLine* GetLine( size_t nLine ) const
            { return m_aLines[ nLine ]; }
    void InsertLine( SwCompareLine* pLine )
        { m_aLines.push_back( pLine ); }

    void SetRedlinesToDoc( bool bUseDocInfo );
};

class CompareMainText : public CompareData
{
public:
    CompareMainText(SwDoc &rD, bool bRecordDiff)
        : CompareData(rD, bRecordDiff)
    {
    }

    virtual const SwNode& GetEndOfContent() override
    {
        return m_rDoc.GetNodes().GetEndOfContent();
    }
};

class CompareFrameFormatText : public CompareData
{
    const SwNodeIndex &m_rIndex;
public:
    CompareFrameFormatText(SwDoc &rD, const SwNodeIndex &rIndex)
        : CompareData(rD, true/*bRecordDiff*/)
        , m_rIndex(rIndex)
    {
    }

    virtual const SwNode& GetEndOfContent() override
    {
        return *m_rIndex.GetNode().EndOfSectionNode();
    }
};

class Hash
{
    struct HashData
    {
        sal_uLong nNext, nHash;
        const SwCompareLine* pLine;

        HashData()
            : nNext( 0 ), nHash( 0 ), pLine(nullptr) {}
    };

    std::unique_ptr<sal_uLong[]> m_pHashArr;
    std::unique_ptr<HashData[]> m_pDataArr;
    sal_uLong m_nCount, m_nPrime;

public:
    explicit Hash( sal_uLong nSize );

    void CalcHashValue( CompareData& rData );

    sal_uLong GetCount() const { return m_nCount; }
};

class Compare
{
public:
    class MovedData
    {
        std::unique_ptr<sal_uLong[]> m_pIndex;
        std::unique_ptr<sal_uLong[]> m_pLineNum;
        sal_uLong m_nCount;

    public:
        MovedData( CompareData& rData, const char* pDiscard );

        sal_uLong GetIndex( sal_uLong n ) const { return m_pIndex[ n ]; }
        sal_uLong GetLineNum( sal_uLong n ) const { return m_pLineNum[ n ]; }
        sal_uLong GetCount() const { return m_nCount; }
    };

private:
    /// Look for the moved lines
    class CompareSequence
    {
        CompareData &m_rData1, &m_rData2;
        const MovedData &m_rMoved1, &m_rMoved2;
        std::unique_ptr<long[]> m_pMemory;
        long *m_pFDiag, *m_pBDiag;

        void Compare( sal_uLong nStt1, sal_uLong nEnd1, sal_uLong nStt2, sal_uLong nEnd2 );
        sal_uLong CheckDiag( sal_uLong nStt1, sal_uLong nEnd1,
                        sal_uLong nStt2, sal_uLong nEnd2, sal_uLong* pCost );
    public:
        CompareSequence( CompareData& rD1, CompareData& rD2,
                        const MovedData& rMD1, const MovedData& rMD2 );
    };

    static void CountDifference( const CompareData& rData, sal_uLong* pCounts );
    static void SetDiscard( const CompareData& rData,
                            char* pDiscard, const sal_uLong* pCounts );
    static void CheckDiscard( sal_uLong nLen, char* pDiscard );
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

/// Consider two lines equal if similar enough (e.g. look like different
/// versions of the same paragraph)
class LineArrayComparator : public ArrayComparator
{
private:
    int m_nLen1, m_nLen2;
    const CompareData &m_rData1, &m_rData2;
    int m_nFirst1, m_nFirst2;

public:
    LineArrayComparator( const CompareData &rD1, const CompareData &rD2,
                            int nStt1, int nEnd1, int nStt2, int nEnd2 );

    virtual bool Compare( int nIdx1, int nIdx2 ) const override;
    virtual int GetLen1() const override { return m_nLen1; }
    virtual int GetLen2() const override { return m_nLen2; }
};

class WordArrayComparator : public ArrayComparator
{
private:
    const SwTextNode *m_pTextNode1, *m_pTextNode2;
    std::unique_ptr<int[]> m_pPos1, m_pPos2;
    int m_nCount1, m_nCount2;       // number of words

    static void CalcPositions( int *pPos, const SwTextNode *pTextNd, int &nCnt );

public:
    WordArrayComparator( const SwTextNode *pNode1, const SwTextNode *pNode2 );

    virtual bool Compare( int nIdx1, int nIdx2 ) const override;
    virtual int GetLen1() const override { return m_nCount1; }
    virtual int GetLen2() const override { return m_nCount2; }
    int GetCharSequence( const int *pWordLcs1, const int *pWordLcs2,
                        int *pSubseq1, int *pSubseq2, int nLcsLen );
};

class CharArrayComparator : public ArrayComparator
{
private:
    const SwTextNode *m_pTextNode1, *m_pTextNode2;

public:
    CharArrayComparator( const SwTextNode *pNode1, const SwTextNode *pNode2 )
        : m_pTextNode1( pNode1 ), m_pTextNode2( pNode2 )
    {
    }

    virtual bool Compare( int nIdx1, int nIdx2 ) const override;
    virtual int GetLen1() const override { return m_pTextNode1->GetText().getLength(); }
    virtual int GetLen2() const override { return m_pTextNode2->GetText().getLength(); }
};

/// Options set in Tools->Options->Writer->Comparison
struct CmpOptionsContainer
{
    SwCompareMode eCmpMode;
    int nIgnoreLen;
    bool bUseRsid;
};

}

static CmpOptionsContainer CmpOptions;

namespace {

class CommonSubseq
{
private:
    std::unique_ptr<int[]> m_pData;

protected:
    ArrayComparator &m_rComparator;

    CommonSubseq( ArrayComparator &rComparator, int nMaxSize )
        : m_rComparator( rComparator )
    {
        m_pData.reset( new int[ nMaxSize ] );
    }

    int FindLCS( int *pLcs1, int *pLcs2, int nStt1,
                 int nEnd1, int nStt2, int nEnd2 );

public:
    static int IgnoreIsolatedPieces( int *pLcs1, int *pLcs2, int nLen1, int nLen2,
                                int nLcsLen, int nPieceLen );
};

/// Use Hirschberg's algorithm to find LCS in linear space
class LgstCommonSubseq: public CommonSubseq
{
private:
    static const int CUTOFF = 1<<20; // Stop recursion at this value

    std::unique_ptr<int[]> m_pL1, m_pL2;
    std::unique_ptr<int[]> m_pBuff1, m_pBuff2;

    void FindL( int *pL, int nStt1, int nEnd1, int nStt2, int nEnd2  );
    int HirschbergLCS( int *pLcs1, int *pLcs2, int nStt1, int nEnd1,
                                                int nStt2, int nEnd2 );

public:
    explicit LgstCommonSubseq( ArrayComparator &rComparator );

    int Find( int *pSubseq1, int *pSubseq2 );
};

/// Find a common subsequence in linear time
class FastCommonSubseq: private CommonSubseq
{
private:
    static const int CUTOFF = 2056;

    int FindFastCS( int *pSeq1, int *pSeq2, int nStt1, int nEnd1,
                                             int nStt2, int nEnd2  );

public:
    explicit FastCommonSubseq( ArrayComparator &rComparator )
        : CommonSubseq( rComparator, CUTOFF )
    {
    }

    int Find( int *pSubseq1, int *pSubseq2 )
    {
        return FindFastCS( pSubseq1, pSubseq2, 0, m_rComparator.GetLen1(),
                                                0, m_rComparator.GetLen2() );
    }
};

}

CompareData::~CompareData()
{
    if( m_pDelRing )
    {
        while( m_pDelRing->GetNext() != m_pDelRing.get() )
            delete m_pDelRing->GetNext();
        m_pDelRing.reset();
    }
    if( m_pInsertRing )
    {
        while( m_pInsertRing->GetNext() != m_pInsertRing.get() )
            delete m_pInsertRing->GetNext();
        m_pInsertRing.reset();
    }
}

void CompareData::SetIndex( size_t nLine, size_t nIndex )
{
    if( !m_pIndex )
    {
        m_pIndex.reset( new size_t[ m_aLines.size() ] );
        memset( m_pIndex.get(), 0, m_aLines.size() * sizeof( size_t ) );
    }
    if( nLine < m_aLines.size() )
        m_pIndex[ nLine ] = nIndex;
}

void CompareData::SetChanged( size_t nLine, bool bFlag )
{
    if( !m_pChangedFlag )
    {
        m_pChangedFlag.reset( new bool[ m_aLines.size() +1 ] );
        memset( m_pChangedFlag.get(), 0, (m_aLines.size() +1) * sizeof( bool ) );
    }
    if( nLine < m_aLines.size() )
        m_pChangedFlag[ nLine ] = bFlag;
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

            if (m_bRecordDiff)
            {
                // Check if there are changed lines (only slightly different) and
                // compare them in detail.
                CheckForChangesInLine( rData, nSav1, nStt1, nSav2, nStt2 );
            }

            ++nCnt;
        }
        ++nStt1;
        ++nStt2;
    }
    return nCnt;
}

bool CompareData::HasDiffs( const CompareData& rData ) const
{
    bool bRet = false;
    sal_uLong nLen1 = rData.GetLineCount(), nLen2 = GetLineCount();
    sal_uLong nStt1 = 0, nStt2 = 0;

    while( nStt1 < nLen1 || nStt2 < nLen2 )
    {
        if( rData.GetChanged( nStt1 ) || GetChanged( nStt2 ) )
        {
            bRet = true;
            break;
        }
        ++nStt1;
        ++nStt2;
    }
    return bRet;
}

Hash::Hash( sal_uLong nSize )
    : m_nCount(1)
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

    m_pDataArr.reset( new HashData[ nSize ] );
    m_pDataArr[0].nNext = 0;
    m_pDataArr[0].nHash = 0;
    m_pDataArr[0].pLine = nullptr;
    m_nPrime = primes[0];

    for( i = 0; primes[i] < nSize / 3;  i++)
        if( !primes[i] )
        {
            m_pHashArr = nullptr;
            return;
        }
    m_nPrime = primes[ i ];
    m_pHashArr.reset( new sal_uLong[ m_nPrime ] );
    memset( m_pHashArr.get(), 0, m_nPrime * sizeof( sal_uLong ) );
}

void Hash::CalcHashValue( CompareData& rData )
{
    if( !m_pHashArr )
        return;

    for( size_t n = 0; n < rData.GetLineCount(); ++n )
    {
        const SwCompareLine* pLine = rData.GetLine( n );
        OSL_ENSURE( pLine, "where is the line?" );
        sal_uLong nH = pLine->GetHashValue();

        sal_uLong* pFound = &m_pHashArr[ nH % m_nPrime ];
        size_t i;
        for( i = *pFound;  ;  i = m_pDataArr[i].nNext )
            if( !i )
            {
                i = m_nCount++;
                m_pDataArr[i].nNext = *pFound;
                m_pDataArr[i].nHash = nH;
                m_pDataArr[i].pLine = pLine;
                *pFound = i;
                break;
            }
            else if( m_pDataArr[i].nHash == nH &&
                    m_pDataArr[i].pLine->Compare( *pLine ))
                break;

        rData.SetIndex( n, i );
    }
}

Compare::Compare( sal_uLong nDiff, CompareData& rData1, CompareData& rData2 )
{
    std::unique_ptr<MovedData> pMD1, pMD2;
    // Look for the differing lines
    {
        std::unique_ptr<char[]> pDiscard1( new char[ rData1.GetLineCount() ] );
        std::unique_ptr<char[]> pDiscard2( new char[ rData2.GetLineCount() ] );

        std::unique_ptr<sal_uLong[]> pCount1(new sal_uLong[ nDiff ]);
        std::unique_ptr<sal_uLong[]> pCount2(new sal_uLong[ nDiff ]);
        memset( pCount1.get(), 0, nDiff * sizeof( sal_uLong ));
        memset( pCount2.get(), 0, nDiff * sizeof( sal_uLong ));

        // find indices in CompareData which have been assigned multiple times
        CountDifference( rData1, pCount1.get() );
        CountDifference( rData2, pCount2.get() );

        // All which occur only once now have either been inserted or deleted.
        // All which are also contained in the other one have been moved.
        SetDiscard( rData1, pDiscard1.get(), pCount2.get() );
        SetDiscard( rData2, pDiscard2.get(), pCount1.get() );

        CheckDiscard( rData1.GetLineCount(), pDiscard1.get() );
        CheckDiscard( rData2.GetLineCount(), pDiscard2.get() );

        pMD1.reset(new MovedData( rData1, pDiscard1.get() ));
        pMD2.reset(new MovedData( rData2, pDiscard2.get() ));
    }

    {
        CompareSequence aTmp( rData1, rData2, *pMD1, *pMD2 );
    }

    ShiftBoundaries( rData1, rData2 );
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
                            char* pDiscard, const sal_uLong* pCounts )
{
    const sal_uLong nLen = rData.GetLineCount();

    // calculate Max with respect to the line count
    sal_uLong nMax = 5;

    for( sal_uLong n = nLen / 64; ( n = n >> 2 ) > 0; )
        nMax <<= 1;

    for( sal_uLong n = 0; n < nLen; ++n )
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

void Compare::CheckDiscard( sal_uLong nLen, char* pDiscard )
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
            {
                pDiscard[ --j ] = 0;
                --provisional;
            }

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
                    {
                        consec = 0;
                        pDiscard[n + j] = 0;
                    }
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
                    {
                        consec = 0;
                        pDiscard[n - j] = 0;
                    }
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

Compare::MovedData::MovedData( CompareData& rData, const char* pDiscard )
    : m_nCount( 0 )
{
    sal_uLong nLen = rData.GetLineCount();
    sal_uLong n;

    for( n = 0; n < nLen; ++n )
        if( pDiscard[ n ] )
            rData.SetChanged( n );
        else
            ++m_nCount;

    if( m_nCount )
    {
        m_pIndex.reset( new sal_uLong[ m_nCount ] );
        m_pLineNum.reset( new sal_uLong[ m_nCount ] );

        for( n = 0, m_nCount = 0; n < nLen; ++n )
            if( !pDiscard[ n ] )
            {
                m_pIndex[ m_nCount ] = rData.GetIndex( n );
                m_pLineNum[ m_nCount++ ] = n;
            }
    }
}

/// Find the differing lines
Compare::CompareSequence::CompareSequence(
                            CompareData& rD1, CompareData& rD2,
                            const MovedData& rMD1, const MovedData& rMD2 )
    : m_rData1( rD1 ), m_rData2( rD2 ), m_rMoved1( rMD1 ), m_rMoved2( rMD2 )
{
    sal_uLong nSize = rMD1.GetCount() + rMD2.GetCount() + 3;
    m_pMemory.reset( new long[ nSize * 2 ] );
    m_pFDiag = m_pMemory.get() + ( rMD2.GetCount() + 1 );
    m_pBDiag = m_pMemory.get() + ( nSize + rMD2.GetCount() + 1 );

    Compare( 0, rMD1.GetCount(), 0, rMD2.GetCount() );
}

void Compare::CompareSequence::Compare( sal_uLong nStt1, sal_uLong nEnd1,
                                        sal_uLong nStt2, sal_uLong nEnd2 )
{
    /* Slide down the bottom initial diagonal. */
    while( nStt1 < nEnd1 && nStt2 < nEnd2 &&
        m_rMoved1.GetIndex( nStt1 ) == m_rMoved2.GetIndex( nStt2 ))
    {
        ++nStt1;
        ++nStt2;
    }

    /* Slide up the top initial diagonal. */
    while( nEnd1 > nStt1 && nEnd2 > nStt2 &&
        m_rMoved1.GetIndex( nEnd1 - 1 ) == m_rMoved2.GetIndex( nEnd2 - 1 ))
    {
        --nEnd1;
        --nEnd2;
    }

    /* Handle simple cases. */
    if( nStt1 == nEnd1 )
        while( nStt2 < nEnd2 )
            m_rData2.SetChanged( m_rMoved2.GetLineNum( nStt2++ ));

    else if (nStt2 == nEnd2)
        while (nStt1 < nEnd1)
            m_rData1.SetChanged( m_rMoved1.GetLineNum( nStt1++ ));

    else
    {
        sal_uLong c, d, b;

        /* Find a point of correspondence in the middle of the files.  */

        d = CheckDiag( nStt1, nEnd1, nStt2, nEnd2, &c );
        b = m_pBDiag[ d ];

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

    m_pFDiag[fmid] = nStt1;
    m_pBDiag[bmid] = nEnd1;

    for (c = 1;; ++c)
    {
        long d;         /* Active diagonal. */

        /* Extend the top-down search by an edit step in each diagonal. */
        if (fmin > dmin)
            m_pFDiag[--fmin - 1] = -1;
        else
            ++fmin;
        if (fmax < dmax)
            m_pFDiag[++fmax + 1] = -1;
        else
            --fmax;
        for (d = fmax; d >= fmin; d -= 2)
        {
            long x, y, tlo = m_pFDiag[d - 1], thi = m_pFDiag[d + 1];

            if (tlo >= thi)
                x = tlo + 1;
            else
                x = thi;
            y = x - d;
            while( o3tl::make_unsigned(x) < nEnd1 && o3tl::make_unsigned(y) < nEnd2 &&
                m_rMoved1.GetIndex( x ) == m_rMoved2.GetIndex( y ))
            {
                ++x;
                ++y;
            }
            m_pFDiag[d] = x;
            if( odd && bmin <= d && d <= bmax && m_pBDiag[d] <= m_pFDiag[d] )
            {
                *pCost = 2 * c - 1;
                return d;
            }
        }

        /* Similar extend the bottom-up search. */
        if (bmin > dmin)
            m_pBDiag[--bmin - 1] = INT_MAX;
        else
            ++bmin;
        if (bmax < dmax)
            m_pBDiag[++bmax + 1] = INT_MAX;
        else
            --bmax;
        for (d = bmax; d >= bmin; d -= 2)
        {
            long x, y, tlo = m_pBDiag[d - 1], thi = m_pBDiag[d + 1];

            if (tlo < thi)
                x = tlo;
            else
                x = thi - 1;
            y = x - d;
            while( o3tl::make_unsigned(x) > nStt1 && o3tl::make_unsigned(y) > nStt2 &&
                m_rMoved1.GetIndex( x - 1 ) == m_rMoved2.GetIndex( y - 1 ))
            {
                --x;
                --y;
            }
            m_pBDiag[d] = x;
            if (!odd && fmin <= d && d <= fmax && m_pBDiag[d] <= m_pFDiag[d])
            {
                *pCost = 2 * c;
                return d;
            }
        }
    }
}

namespace
{
    void lcl_ShiftBoundariesOneway( CompareData* const pData, CompareData const * const pOtherData)
    {
        sal_uLong i = 0;
        sal_uLong j = 0;
        sal_uLong i_end = pData->GetLineCount();
        sal_uLong preceding = ULONG_MAX;
        sal_uLong other_preceding = ULONG_MAX;

        while (true)
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

            while (true)
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
                    start != preceding && other_start != other_preceding )
                {
                    pData->SetChanged( start++, false );
                    pData->SetChanged(  i );
                    /* Since one line-that-matches is now before this run
                       instead of after, we must advance in the other file
                       to keep in sync.  */
                    ++j;
                }
                else
                    break;
            }

            preceding = i;
            other_preceding = j;
        }
    }
}

void Compare::ShiftBoundaries( CompareData& rData1, CompareData& rData2 )
{
    lcl_ShiftBoundariesOneway(&rData1, &rData2);
    lcl_ShiftBoundariesOneway(&rData2, &rData1);
}

sal_uLong SwCompareLine::GetHashValue() const
{
    sal_uLong nRet = 0;
    switch( m_rNode.GetNodeType() )
    {
    case SwNodeType::Text:
        nRet = GetTextNodeHashValue( *m_rNode.GetTextNode(), nRet );
        break;

    case SwNodeType::Table:
        {
            const SwNode* pEndNd = m_rNode.EndOfSectionNode();
            SwNodeIndex aIdx( m_rNode );
            while( &aIdx.GetNode() != pEndNd )
            {
                if( aIdx.GetNode().IsTextNode() )
                    nRet = GetTextNodeHashValue( *aIdx.GetNode().GetTextNode(), nRet );
                ++aIdx;
            }
        }
        break;

    case SwNodeType::Section:
        {
            OUString sStr( GetText() );
            for( sal_Int32 n = 0; n < sStr.getLength(); ++n )
                nRet = (nRet << 1) + sStr[ n ];
        }
        break;

    case SwNodeType::Grf:
    case SwNodeType::Ole:
        // Fixed ID? Should never occur ...
        break;
    default: break;
    }
    return nRet;
}

const SwNode& SwCompareLine::GetEndNode() const
{
    const SwNode* pNd = &m_rNode;
    switch( m_rNode.GetNodeType() )
    {
    case SwNodeType::Table:
        pNd = m_rNode.EndOfSectionNode();
        break;

    case SwNodeType::Section:
        {
            const SwSectionNode& rSNd = static_cast<const SwSectionNode&>(m_rNode);
            const SwSection& rSect = rSNd.GetSection();
            if( SectionType::Content != rSect.GetType() || rSect.IsProtect() )
                pNd = m_rNode.EndOfSectionNode();
        }
        break;
    default: break;
    }
    return *pNd;
}

bool SwCompareLine::Compare( const SwCompareLine& rLine ) const
{
    return CompareNode( m_rNode, rLine.m_rNode );
}

namespace
{
    OUString SimpleTableToText(const SwNode &rNode)
    {
        OUStringBuffer sRet;
        const SwNode* pEndNd = rNode.EndOfSectionNode();
        SwNodeIndex aIdx( rNode );
        while (&aIdx.GetNode() != pEndNd)
        {
            if (aIdx.GetNode().IsTextNode())
            {
                if (sRet.getLength())
                {
                    sRet.append( '\n' );
                }
                sRet.append( aIdx.GetNode().GetTextNode()->GetExpandText(nullptr) );
            }
            ++aIdx;
        }
        return sRet.makeStringAndClear();
    }
}

bool SwCompareLine::CompareNode( const SwNode& rDstNd, const SwNode& rSrcNd )
{
    if( rSrcNd.GetNodeType() != rDstNd.GetNodeType() )
        return false;

    bool bRet = false;

    switch( rDstNd.GetNodeType() )
    {
    case SwNodeType::Text:
        bRet = CompareTextNd( *rDstNd.GetTextNode(), *rSrcNd.GetTextNode() )
            && ( !CmpOptions.bUseRsid || rDstNd.GetTextNode()->CompareParRsid( *rSrcNd.GetTextNode() ) );
        break;

    case SwNodeType::Table:
        {
            const SwTableNode& rTSrcNd = static_cast<const SwTableNode&>(rSrcNd);
            const SwTableNode& rTDstNd = static_cast<const SwTableNode&>(rDstNd);

            bRet = ( rTSrcNd.EndOfSectionIndex() - rTSrcNd.GetIndex() ) ==
                   ( rTDstNd.EndOfSectionIndex() - rTDstNd.GetIndex() );

            // --> #i107826#: compare actual table content
            if (bRet)
            {
                bRet = (SimpleTableToText(rSrcNd) == SimpleTableToText(rDstNd));
            }
        }
        break;

    case SwNodeType::Section:
        {
            const SwSectionNode& rSSrcNd = static_cast<const SwSectionNode&>(rSrcNd),
                               & rSDstNd = static_cast<const SwSectionNode&>(rDstNd);
            const SwSection& rSrcSect = rSSrcNd.GetSection(),
                           & rDstSect = rSDstNd.GetSection();
            SectionType eSrcSectType = rSrcSect.GetType(),
                        eDstSectType = rDstSect.GetType();
            switch( eSrcSectType )
            {
            case SectionType::Content:
                bRet = SectionType::Content == eDstSectType &&
                        rSrcSect.IsProtect() == rDstSect.IsProtect();
                if( bRet && rSrcSect.IsProtect() )
                {
                    // the only have they both the same size
                    bRet = ( rSSrcNd.EndOfSectionIndex() - rSSrcNd.GetIndex() ) ==
                              ( rSDstNd.EndOfSectionIndex() - rSDstNd.GetIndex() );
                }
                break;

            case SectionType::ToxHeader:
            case SectionType::ToxContent:
                if( SectionType::ToxHeader == eDstSectType ||
                    SectionType::ToxContent == eDstSectType )
                {
                    // the same type of TOX?
                    const SwTOXBase* pSrcTOX = rSrcSect.GetTOXBase();
                    const SwTOXBase* pDstTOX = rDstSect.GetTOXBase();
                    bRet =  pSrcTOX && pDstTOX
                            && pSrcTOX->GetType() == pDstTOX->GetType()
                            && pSrcTOX->GetTitle() == pDstTOX->GetTitle()
                            && pSrcTOX->GetTypeName() == pDstTOX->GetTypeName()
                            ;
                }
                break;

            case SectionType::DdeLink:
            case SectionType::FileLink:
                bRet = eSrcSectType == eDstSectType &&
                        rSrcSect.GetLinkFileName() ==
                        rDstSect.GetLinkFileName();
                break;
            }
        }
        break;

    case SwNodeType::End:
        bRet = rSrcNd.StartOfSectionNode()->GetNodeType() ==
               rDstNd.StartOfSectionNode()->GetNodeType();

        // --> #i107826#: compare actual table content
        if (bRet && rSrcNd.StartOfSectionNode()->GetNodeType() == SwNodeType::Table)
        {
            bRet = CompareNode(
                *rSrcNd.StartOfSectionNode(), *rDstNd.StartOfSectionNode());
        }

        break;

    default: break;
    }
    return bRet;
}

OUString SwCompareLine::GetText() const
{
    OUString sRet;
    switch( m_rNode.GetNodeType() )
    {
    case SwNodeType::Text:
        sRet = m_rNode.GetTextNode()->GetExpandText(nullptr);
        break;

    case SwNodeType::Table:
        {
            sRet = "Tabelle: " + SimpleTableToText(m_rNode);
        }
        break;

    case SwNodeType::Section:
        {
            sRet = "Section - Node:";

            const SwSectionNode& rSNd = static_cast<const SwSectionNode&>(m_rNode);
            const SwSection& rSect = rSNd.GetSection();
            switch( rSect.GetType() )
            {
            case SectionType::Content:
                if( rSect.IsProtect() )
                    sRet += OUString::number(
                            rSNd.EndOfSectionIndex() - rSNd.GetIndex() );
                break;

            case SectionType::ToxHeader:
            case SectionType::ToxContent:
                {
                    const SwTOXBase* pTOX = rSect.GetTOXBase();
                    if( pTOX )
                        sRet += pTOX->GetTitle() + pTOX->GetTypeName() +
                            OUString::number(pTOX->GetType());
                }
                break;

            case SectionType::DdeLink:
            case SectionType::FileLink:
                sRet += rSect.GetLinkFileName();
                break;
            }
        }
        break;

    case SwNodeType::Grf:
        sRet = "Grafik - Node:";
        break;
    case SwNodeType::Ole:
        sRet = "OLE - Node:";
        break;
    default: break;
    }
    return sRet;
}

sal_uLong SwCompareLine::GetTextNodeHashValue( const SwTextNode& rNd, sal_uLong nVal )
{
    OUString sStr( rNd.GetExpandText(nullptr) );
    for( sal_Int32 n = 0; n < sStr.getLength(); ++n )
        nVal = (nVal << 1 ) + sStr[ n ];
    return nVal;
}

bool SwCompareLine::CompareTextNd( const SwTextNode& rDstNd,
                                  const SwTextNode& rSrcNd )
{
    bool bRet = false;
    // Very simple at first
    if( rDstNd.GetText() == rSrcNd.GetText() )
    {
        // The text is the same, but are the "special attributes" (0xFF) also the same?
        bRet = true;
    }
    return bRet;
}

bool SwCompareLine::ChangesInLine( const SwCompareLine& rLine,
                            std::unique_ptr<SwPaM>& rpInsRing, std::unique_ptr<SwPaM>& rpDelRing ) const
{
    bool bRet = false;

    // Only compare textnodes
    if( SwNodeType::Text == m_rNode.GetNodeType() &&
        SwNodeType::Text == rLine.GetNode().GetNodeType() )
    {
        SwTextNode& rDstNd = *const_cast<SwTextNode*>(m_rNode.GetTextNode());
        const SwTextNode& rSrcNd = *rLine.GetNode().GetTextNode();
        SwDoc& rDstDoc = rDstNd.GetDoc();

        int nLcsLen = 0;

        int nDstLen = rDstNd.GetText().getLength();
        int nSrcLen = rSrcNd.GetText().getLength();

        int nMinLen = std::min( nDstLen , nSrcLen );
        int nAvgLen = ( nDstLen + nSrcLen )/2;

        std::vector<int> aLcsDst( nMinLen + 1 );
        std::vector<int> aLcsSrc( nMinLen + 1 );

        if( CmpOptions.eCmpMode == SwCompareMode::ByWord )
        {
            std::vector<int> aTmpLcsDst( nMinLen + 1 );
            std::vector<int> aTmpLcsSrc( nMinLen + 1 );

            WordArrayComparator aCmp( &rDstNd, &rSrcNd );

            LgstCommonSubseq aSeq( aCmp );

            nLcsLen = aSeq.Find( aTmpLcsDst.data(), aTmpLcsSrc.data() );

            if( CmpOptions.nIgnoreLen )
            {
                nLcsLen = CommonSubseq::IgnoreIsolatedPieces( aTmpLcsDst.data(), aTmpLcsSrc.data(),
                                                aCmp.GetLen1(), aCmp.GetLen2(),
                                                nLcsLen, CmpOptions.nIgnoreLen );
            }

            nLcsLen = aCmp.GetCharSequence( aTmpLcsDst.data(), aTmpLcsSrc.data(),
                                            aLcsDst.data(), aLcsSrc.data(), nLcsLen );
        }
        else
        {
            CharArrayComparator aCmp( &rDstNd, &rSrcNd );
            LgstCommonSubseq aSeq( aCmp );

            nLcsLen = aSeq.Find( aLcsDst.data(), aLcsSrc.data() );

            if( CmpOptions.nIgnoreLen )
            {
                nLcsLen = CommonSubseq::IgnoreIsolatedPieces( aLcsDst.data(), aLcsSrc.data(), nDstLen,
                                                    nSrcLen, nLcsLen,
                                                    CmpOptions.nIgnoreLen );
            }
        }

        // find the sum of the squares of the continuous substrings
        int nSqSum = 0;
        int nCnt = 1;
        for( int i = 0; i < nLcsLen; i++ )
        {
            if( i != nLcsLen - 1 && aLcsDst[i] + 1 == aLcsDst[i + 1]
                                && aLcsSrc[i] + 1 == aLcsSrc[i + 1] )
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
            return false;
        }

        // Show the differences
        int nSkip = 0;
        for( int i = 0; i <= nLcsLen; i++ )
        {
            int nDstFrom = i ? (aLcsDst[i - 1] + 1) : 0;
            int nDstTo = ( i == nLcsLen ) ? nDstLen : aLcsDst[i];
            int nSrcFrom = i ? (aLcsSrc[i - 1] + 1) : 0;
            int nSrcTo = ( i == nLcsLen ) ? nSrcLen : aLcsSrc[i];

            SwPaM aPam( rDstNd, nDstTo + nSkip );

            if ( nDstFrom < nDstTo )
            {
                SwPaM* pTmp = new SwPaM( *aPam.GetPoint(), rpInsRing.get() );
                if( !rpInsRing )
                    rpInsRing.reset(pTmp);
                pTmp->SetMark();
                pTmp->GetMark()->nContent = nDstFrom + nSkip;
            }

            if ( nSrcFrom < nSrcTo )
            {
                bool bUndo = rDstDoc.GetIDocumentUndoRedo().DoesUndo();
                rDstDoc.GetIDocumentUndoRedo().DoUndo( false );
                SwPaM aCpyPam( rSrcNd, nSrcFrom );
                aCpyPam.SetMark();
                aCpyPam.GetPoint()->nContent = nSrcTo;
                aCpyPam.GetDoc()->getIDocumentContentOperations().CopyRange( aCpyPam, *aPam.GetPoint(),
                    SwCopyFlags::CheckPosInFly);
                rDstDoc.GetIDocumentUndoRedo().DoUndo( bUndo );

                SwPaM* pTmp = new SwPaM( *aPam.GetPoint(), rpDelRing.get() );
                if( !rpDelRing )
                    rpDelRing.reset(pTmp);

                pTmp->SetMark();
                pTmp->GetMark()->nContent = nDstTo + nSkip;
                nSkip += nSrcTo - nSrcFrom;

                if( rpInsRing )
                {
                    SwPaM* pCorr = rpInsRing->GetPrev();
                    if( *pCorr->GetPoint() == *pTmp->GetPoint() )
                        *pCorr->GetPoint() = *pTmp->GetMark();
                }
            }
        }

        bRet = true;
    }

    return bRet;
}

sal_uLong CompareData::NextIdx( const SwNode* pNd )
{
    if( pNd->IsStartNode() )
    {
        if( pNd->IsTableNode() )
            pNd = pNd->EndOfSectionNode();
        else
        {
            const SwSectionNode* pSNd = pNd->GetSectionNode();
            if( pSNd &&
                ( SectionType::Content != pSNd->GetSection().GetType() ||
                    pSNd->GetSection().IsProtect() ) )
                pNd = pNd->EndOfSectionNode();
        }
    }
    return pNd->GetIndex() + 1;
}

sal_uLong CompareData::PrevIdx( const SwNode* pNd )
{
    if( pNd->IsEndNode() )
    {
        if( pNd->StartOfSectionNode()->IsTableNode() )
            pNd = pNd->StartOfSectionNode();
        else
        {
            const SwSectionNode* pSNd = pNd->StartOfSectionNode()->GetSectionNode();
            if( pSNd &&
                ( SectionType::Content != pSNd->GetSection().GetType() ||
                    pSNd->GetSection().IsProtect() ) )
                pNd = pNd->StartOfSectionNode();
        }
    }
    return pNd->GetIndex() - 1;
}

void CompareData::CheckRanges( CompareData& rData )
{
    const SwNodes& rSrcNds = rData.m_rDoc.GetNodes();
    const SwNodes& rDstNds = m_rDoc.GetNodes();

    const SwNode& rSrcEndNd = rData.GetEndOfContent();
    const SwNode& rDstEndNd = GetEndOfContent();

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

void CompareData::ShowInsert( sal_uLong nStt, sal_uLong nEnd )
{
    SwPaM* pTmp = new SwPaM( GetLine( nStt )->GetNode(), 0,
                             GetLine( nEnd-1 )->GetEndNode(), 0,
                             m_pInsertRing.get() );
    if( !m_pInsertRing )
        m_pInsertRing.reset( pTmp );

    // #i65201#: These SwPaMs are calculated smaller than needed, see comment below
}

void CompareData::ShowDelete(
    const CompareData& rData,
    sal_uLong nStt,
    sal_uLong nEnd,
    sal_uLong nInsPos )
{
    SwNodeRange aRg(
        rData.GetLine( nStt )->GetNode(), 0,
        rData.GetLine( nEnd-1 )->GetEndNode(), 1 );

    sal_uInt16 nOffset = 0;
    const SwCompareLine* pLine = nullptr;
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
            pLineNd = &pLine->GetEndNode();
        else
            pLineNd = &pLine->GetNode();
    }
    else
    {
        pLineNd = &GetEndOfContent();
        nOffset = 0;
    }

    SwNodeIndex aInsPos( *pLineNd, nOffset );
    SwNodeIndex aSavePos( aInsPos, -1 );

    rData.m_rDoc.GetDocumentContentOperationsManager().CopyWithFlyInFly(aRg, aInsPos);
    m_rDoc.getIDocumentState().SetModified();
    ++aSavePos;

    // #i65201#: These SwPaMs are calculated when the (old) delete-redlines are hidden,
    // they will be inserted when the delete-redlines are shown again.
    // To avoid unwanted insertions of delete-redlines into these new redlines, what happens
    // especially at the end of the document, I reduce the SwPaM by one node.
    // Before the new redlines are inserted, they have to expand again.
    SwPaM* pTmp = new SwPaM( aSavePos.GetNode(), aInsPos.GetNode(), 0, -1, m_pDelRing.get() );
    if( !m_pDelRing )
        m_pDelRing.reset(pTmp);

    if( m_pInsertRing )
    {
        SwPaM* pCorr = m_pInsertRing->GetPrev();
        if( *pCorr->GetPoint() == *pTmp->GetPoint() )
        {
            SwNodeIndex aTmpPos( pTmp->GetMark()->nNode, -1 );
            *pCorr->GetPoint() = SwPosition( aTmpPos );
        }
    }
}

void CompareData::CheckForChangesInLine( const CompareData& rData,
                                    sal_uLong nStt, sal_uLong nEnd,
                                    sal_uLong nThisStt, sal_uLong nThisEnd )
{
    LineArrayComparator aCmp( *this, rData, nThisStt, nThisEnd,
                              nStt, nEnd );

    int nMinLen = std::min( aCmp.GetLen1(), aCmp.GetLen2() );
    std::unique_ptr<int[]> pLcsDst(new int[ nMinLen ]);
    std::unique_ptr<int[]> pLcsSrc(new int[ nMinLen ]);

    FastCommonSubseq subseq( aCmp );
    int nLcsLen = subseq.Find( pLcsDst.get(), pLcsSrc.get() );
    for (int i = 0; i <= nLcsLen; i++)
    {
        // Beginning of inserted lines (inclusive)
        int nDstFrom = i ? pLcsDst[i - 1] + 1 : 0;
        // End of inserted lines (exclusive)
        int nDstTo = ( i == nLcsLen ) ? aCmp.GetLen1() : pLcsDst[i];
        // Beginning of deleted lines (inclusive)
        int nSrcFrom = i ? pLcsSrc[i - 1] + 1 : 0;
        // End of deleted lines (exclusive)
        int nSrcTo = ( i == nLcsLen ) ? aCmp.GetLen2() : pLcsSrc[i];

        if( i )
        {
            const SwCompareLine* pDstLn = GetLine( nThisStt + nDstFrom - 1 );
            const SwCompareLine* pSrcLn = rData.GetLine( nStt + nSrcFrom - 1 );

            // Show differences in detail for lines that
            // were matched as only slightly different
            if( !pDstLn->ChangesInLine( *pSrcLn, m_pInsertRing, m_pDelRing ) )
            {
                ShowInsert( nThisStt + nDstFrom - 1, nThisStt + nDstFrom );
                ShowDelete( rData, nStt + nSrcFrom - 1, nStt + nSrcFrom,
                                                    nThisStt + nDstFrom );
            }
        }

        // Lines missing from source are inserted
        if( nDstFrom != nDstTo )
        {
            ShowInsert( nThisStt + nDstFrom, nThisStt + nDstTo );
        }

        // Lines missing from destination are deleted
        if( nSrcFrom != nSrcTo )
        {
            ShowDelete( rData, nStt + nSrcFrom, nStt + nSrcTo, nThisStt + nDstTo );
        }
    }
}

void CompareData::SetRedlinesToDoc( bool bUseDocInfo )
{
    SwPaM* pTmp = m_pDelRing.get();

    // get the Author / TimeStamp from the "other" document info
    std::size_t nAuthor = m_rDoc.getIDocumentRedlineAccess().GetRedlineAuthor();
    DateTime aTimeStamp( DateTime::SYSTEM );
    SwDocShell *pDocShell(m_rDoc.GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");

        if( bUseDocInfo && xDocProps.is() ) {
            OUString aTmp( 1 == xDocProps->getEditingCycles()
                                ? xDocProps->getAuthor()
                                : xDocProps->getModifiedBy() );
            util::DateTime uDT( 1 == xDocProps->getEditingCycles()
                                ? xDocProps->getCreationDate()
                                : xDocProps->getModificationDate() );

            if( !aTmp.isEmpty() )
            {
                nAuthor = m_rDoc.getIDocumentRedlineAccess().InsertRedlineAuthor( aTmp );
                aTimeStamp = DateTime(uDT);
            }
        }
    }

    if( pTmp )
    {
        SwRedlineData aRedlnData( RedlineType::Delete, nAuthor, aTimeStamp,
                                    OUString(), nullptr );
        do {
            // #i65201#: Expand again, see comment above.
            if( pTmp->GetPoint()->nContent == 0 )
            {
                ++pTmp->GetPoint()->nNode;
                pTmp->GetPoint()->nContent.Assign( pTmp->GetContentNode(), 0 );
            }
            // #i101009#
            // prevent redlines that end on structural end node
            if (& GetEndOfContent() ==
                & pTmp->GetPoint()->nNode.GetNode())
            {
                --pTmp->GetPoint()->nNode;
                SwContentNode *const pContentNode( pTmp->GetContentNode() );
                pTmp->GetPoint()->nContent.Assign( pContentNode,
                        pContentNode ? pContentNode->Len() : 0 );
                // tdf#106218 try to avoid losing a paragraph break here:
                if (pTmp->GetMark()->nContent == 0)
                {
                    SwNodeIndex const prev(pTmp->GetMark()->nNode, -1);
                    if (prev.GetNode().IsTextNode())
                    {
                        *pTmp->GetMark() = SwPosition(
                            *prev.GetNode().GetTextNode(),
                            prev.GetNode().GetTextNode()->Len());
                    }
                }
            }

            m_rDoc.getIDocumentRedlineAccess().DeleteRedline( *pTmp, false, RedlineType::Any );

            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoCompDoc>( *pTmp, false ));
            }
            m_rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( aRedlnData, *pTmp ), true );

        } while( m_pDelRing.get() != ( pTmp = pTmp->GetNext()) );
    }

    pTmp = m_pInsertRing.get();
    if( !pTmp )
        return;

    do {
        if( pTmp->GetPoint()->nContent == 0 )
        {
            ++pTmp->GetPoint()->nNode;
            pTmp->GetPoint()->nContent.Assign( pTmp->GetContentNode(), 0 );
        }
        // #i101009#
        // prevent redlines that end on structural end node
        if (& GetEndOfContent() ==
            & pTmp->GetPoint()->nNode.GetNode())
        {
            --pTmp->GetPoint()->nNode;
            SwContentNode *const pContentNode( pTmp->GetContentNode() );
            pTmp->GetPoint()->nContent.Assign( pContentNode,
                    pContentNode ? pContentNode->Len() : 0 );
            // tdf#106218 try to avoid losing a paragraph break here:
            if (pTmp->GetMark()->nContent == 0)
            {
                SwNodeIndex const prev(pTmp->GetMark()->nNode, -1);
                if (prev.GetNode().IsTextNode())
                {
                    *pTmp->GetMark() = SwPosition(
                        *prev.GetNode().GetTextNode(),
                        prev.GetNode().GetTextNode()->Len());
                }
            }
        }
    } while( m_pInsertRing.get() != ( pTmp = pTmp->GetNext()) );
    SwRedlineData aRedlnData( RedlineType::Insert, nAuthor, aTimeStamp,
                                OUString(), nullptr );

    // combine consecutive
    if( pTmp->GetNext() != m_pInsertRing.get() )
    {
        do {
            SwPosition& rSttEnd = *pTmp->End(),
                      & rEndStt = *pTmp->GetNext()->Start();
            const SwContentNode* pCNd;
            if( rSttEnd == rEndStt ||
                (!rEndStt.nContent.GetIndex() &&
                rEndStt.nNode.GetIndex() - 1 == rSttEnd.nNode.GetIndex() &&
                nullptr != ( pCNd = rSttEnd.nNode.GetNode().GetContentNode() ) &&
                rSttEnd.nContent.GetIndex() == pCNd->Len()))
            {
                if( pTmp->GetNext() == m_pInsertRing.get() )
                {
                    // are consecutive, so combine
                    rEndStt = *pTmp->Start();
                    delete pTmp;
                    pTmp = m_pInsertRing.get();
                }
                else
                {
                    // are consecutive, so combine
                    rSttEnd = *pTmp->GetNext()->End();
                    delete pTmp->GetNext();
                }
            }
            else
                pTmp = pTmp->GetNext();
        } while( m_pInsertRing.get() != pTmp );
    }

    do {
        if (IDocumentRedlineAccess::AppendResult::APPENDED ==
                m_rDoc.getIDocumentRedlineAccess().AppendRedline(
                    new SwRangeRedline(aRedlnData, *pTmp), true) &&
            m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoCompDoc>( *pTmp, true ));
        }
    } while( m_pInsertRing.get() != ( pTmp = pTmp->GetNext()) );
}

typedef std::shared_ptr<CompareData> CompareDataPtr;
typedef std::pair<CompareDataPtr, CompareDataPtr> CompareDataPtrPair;
typedef std::vector<CompareDataPtrPair> Comparators;

namespace
{
    Comparators buildComparators(SwDoc &rSrcDoc, SwDoc &rDestDoc)
    {
        Comparators aComparisons;
        //compare main text
        aComparisons.emplace_back(std::make_shared<CompareMainText>(rSrcDoc, true),
                                  std::make_shared<CompareMainText>(rDestDoc, true));

        //if we have the same number of frames then try to compare within them
        const SwFrameFormats *pSrcFrameFormats = rSrcDoc.GetSpzFrameFormats();
        const SwFrameFormats *pDestFrameFormats = rDestDoc.GetSpzFrameFormats();
        if (pSrcFrameFormats->size() == pDestFrameFormats->size())
        {
            for (size_t i = 0; i < pSrcFrameFormats->size(); ++i)
            {
                const SwFrameFormat& rSrcFormat = *(*pSrcFrameFormats)[i];
                const SwFrameFormat& rDestFormat = *(*pDestFrameFormats)[i];
                const SwNodeIndex* pSrcIdx = rSrcFormat.GetContent().GetContentIdx();
                const SwNodeIndex* pDestIdx = rDestFormat.GetContent().GetContentIdx();
                if (!pSrcIdx && !pDestIdx)
                    continue;
                if (!pSrcIdx || !pDestIdx)
                    break;
                const SwNode* pSrcNode = pSrcIdx->GetNode().EndOfSectionNode();
                const SwNode* pDestNode = pDestIdx->GetNode().EndOfSectionNode();
                if (!pSrcNode && !pDestNode)
                    continue;
                if (!pSrcNode || !pDestNode)
                    break;
                if (pSrcIdx->GetNodes()[pSrcIdx->GetIndex() + 1]->IsNoTextNode()
                    || pDestIdx->GetNodes()[pDestIdx->GetIndex() + 1]->IsNoTextNode())
                {
                    continue; // tdf#125660 don't redline GrfNode/OLENode
                }
                aComparisons.emplace_back(std::make_shared<CompareFrameFormatText>(rSrcDoc, *pSrcIdx),
                                          std::make_shared<CompareFrameFormatText>(rDestDoc, *pDestIdx));
            }
        }
        return aComparisons;
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
    if( CmpOptions.eCmpMode == SwCompareMode::Auto )
    {
        if( getRsidRoot() == rDoc.getRsidRoot() )
        {
            CmpOptions.eCmpMode = SwCompareMode::ByChar;
            CmpOptions.bUseRsid = true;
            CmpOptions.nIgnoreLen = 2;
        }
        else
        {
            CmpOptions.eCmpMode = SwCompareMode::ByWord;
            CmpOptions.bUseRsid = false;
            CmpOptions.nIgnoreLen = 3;
        }
    }
    else
    {
        CmpOptions.bUseRsid = getRsidRoot() == rDoc.getRsidRoot() && SW_MOD()->IsUseRsid();
        CmpOptions.nIgnoreLen = SW_MOD()->IsIgnorePieces() ? SW_MOD()->GetPieceLen() : 0;
    }

    GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);
    bool bDocWasModified = getIDocumentState().IsModified();
    SwDoc& rSrcDoc = const_cast<SwDoc&>(rDoc);
    bool bSrcModified = rSrcDoc.getIDocumentState().IsModified();

    RedlineFlags eSrcRedlMode = rSrcDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    rSrcDoc.getIDocumentRedlineAccess().SetRedlineFlags( RedlineFlags::ShowInsert );
    getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert);

    Comparators aComparisons(buildComparators(rSrcDoc, *this));

    for (auto& a : aComparisons)
    {
        CompareData& rD0 = *a.first;
        CompareData& rD1 = *a.second;
        rD1.CompareLines( rD0 );
        nRet |= rD1.ShowDiffs( rD0 );
    }

    if( nRet )
    {
        getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On |
                       RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);

        for (auto& a : aComparisons)
        {
            CompareData& rD1 = *a.second;
            rD1.SetRedlinesToDoc( !bDocWasModified );
        }
        getIDocumentState().SetModified();
    }

    rSrcDoc.getIDocumentRedlineAccess().SetRedlineFlags( eSrcRedlMode );
    getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);

    if( !bSrcModified )
        rSrcDoc.getIDocumentState().ResetModified();

    GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);

    return nRet;
}

namespace
{
    struct SaveMergeRedline
    {
        const SwRangeRedline* pSrcRedl;
        SwRangeRedline* pDestRedl;
        SaveMergeRedline( const SwNode& rDstNd, const SwRangeRedline& rSrcRedl);
        sal_uInt16 InsertRedline(SwPaM* pLastDestRedline);
    };
}

SaveMergeRedline::SaveMergeRedline( const SwNode& rDstNd,
                        const SwRangeRedline& rSrcRedl)
    : pSrcRedl( &rSrcRedl )
{
    SwPosition aPos( rDstNd );

    const SwPosition* pStt = rSrcRedl.Start();
    if( rDstNd.IsContentNode() )
        aPos.nContent.Assign( const_cast<SwContentNode*>(static_cast<const SwContentNode*>(&rDstNd)), pStt->nContent.GetIndex() );
    pDestRedl = new SwRangeRedline( rSrcRedl.GetRedlineData(), aPos );

    if( RedlineType::Delete != pDestRedl->GetType() )
        return;

    // mark the area as deleted
    const SwPosition* pEnd = pStt == rSrcRedl.GetPoint()
                                        ? rSrcRedl.GetMark()
                                        : rSrcRedl.GetPoint();

    pDestRedl->SetMark();
    pDestRedl->GetPoint()->nNode += pEnd->nNode.GetIndex() -
                                    pStt->nNode.GetIndex();
    pDestRedl->GetPoint()->nContent.Assign( pDestRedl->GetContentNode(),
                                            pEnd->nContent.GetIndex() );
}

sal_uInt16 SaveMergeRedline::InsertRedline(SwPaM* pLastDestRedline)
{
    sal_uInt16 nIns = 0;
    SwDoc* pDoc = pDestRedl->GetDoc();

    if( RedlineType::Insert == pDestRedl->GetType() )
    {
        // the part was inserted so copy it from the SourceDoc
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

        SwNodeIndex aSaveNd( pDestRedl->GetPoint()->nNode, -1 );
        const sal_Int32 nSaveCnt = pDestRedl->GetPoint()->nContent.GetIndex();

        RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);

        pSrcRedl->GetDoc()->getIDocumentContentOperations().CopyRange(
                *const_cast<SwPaM*>(static_cast<const SwPaM*>(pSrcRedl)),
                *pDestRedl->GetPoint(), SwCopyFlags::CheckPosInFly);

        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );

        pDestRedl->SetMark();
        ++aSaveNd;
        pDestRedl->GetMark()->nNode = aSaveNd;
        pDestRedl->GetMark()->nContent.Assign( aSaveNd.GetNode().GetContentNode(),
                                                nSaveCnt );

        if( pLastDestRedline && *pLastDestRedline->GetPoint() == *pDestRedl->GetPoint() )
            *pLastDestRedline->GetPoint() = *pDestRedl->GetMark();
    }
    else
    {
        //JP 21.09.98: Bug 55909
        // If there already is a deleted or inserted one at the same position, we have to split it!
        SwPosition* pDStt = pDestRedl->GetMark(),
                  * pDEnd = pDestRedl->GetPoint();
        SwRedlineTable::size_type n = 0;

            // find the first redline for StartPos
        if( !pDoc->getIDocumentRedlineAccess().GetRedline( *pDStt, &n ) && n )
            --n;

        const SwRedlineTable& rRedlineTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
        for( ; n < rRedlineTable.size(); ++n )
        {
            SwRangeRedline* pRedl = rRedlineTable[ n ];
            SwPosition* pRStt = pRedl->Start(),
                      * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                           : pRedl->GetPoint();
            if( RedlineType::Delete == pRedl->GetType() ||
                RedlineType::Insert == pRedl->GetType() )
            {
                SwComparePosition eCmpPos = ComparePosition( *pDStt, *pDEnd, *pRStt, *pREnd );
                switch( eCmpPos )
                {
                case SwComparePosition::CollideStart:
                case SwComparePosition::Behind:
                    break;

                case SwComparePosition::Inside:
                case SwComparePosition::Equal:
                    delete pDestRedl;
                    pDestRedl = nullptr;
                    [[fallthrough]];

                case SwComparePosition::CollideEnd:
                case SwComparePosition::Before:
                    n = rRedlineTable.size();
                    break;

                case SwComparePosition::Outside:
                    assert(pDestRedl && "is this actually impossible");
                    if (pDestRedl)
                    {
                        SwRangeRedline* pCpyRedl = new SwRangeRedline(
                            pDestRedl->GetRedlineData(), *pDStt );
                        pCpyRedl->SetMark();
                        *pCpyRedl->GetPoint() = *pRStt;

                        std::unique_ptr<SwUndoCompDoc> pUndo;
                        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
                            pUndo.reset(new SwUndoCompDoc( *pCpyRedl ));

                        // now modify doc: append redline, undo (and count)
                        pDoc->getIDocumentRedlineAccess().AppendRedline( pCpyRedl, true );
                        if( pUndo )
                        {
                            pDoc->GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
                        }
                        ++nIns;

                        *pDStt = *pREnd;

                        // we should start over now
                        n = SwRedlineTable::npos;
                    }
                    break;

                case SwComparePosition::OverlapBefore:
                    *pDEnd = *pRStt;
                    break;

                case SwComparePosition::OverlapBehind:
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
        std::unique_ptr<SwUndoCompDoc> pUndo;
        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
            pUndo.reset(new SwUndoCompDoc( *pDestRedl ));

        // now modify doc: append redline, undo (and count)
        IDocumentRedlineAccess::AppendResult const result(
            pDoc->getIDocumentRedlineAccess().AppendRedline(pDestRedl, true));
        if( pUndo )
        {
            pDoc->GetIDocumentUndoRedo().AppendUndo( std::move(pUndo) );
        }
        ++nIns;

        // if AppendRedline has deleted our redline, we may not keep a
        // reference to it
        if (IDocumentRedlineAccess::AppendResult::APPENDED != result)
            pDestRedl = nullptr;
    }
    return nIns;
}

/// Merge two documents
long SwDoc::MergeDoc( const SwDoc& rDoc )
{
    if( &rDoc == this )
        return 0;

    long nRet = 0;

    GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

    SwDoc& rSrcDoc = const_cast<SwDoc&>(rDoc);
    bool bSrcModified = rSrcDoc.getIDocumentState().IsModified();

    RedlineFlags eSrcRedlMode = rSrcDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    rSrcDoc.getIDocumentRedlineAccess().SetRedlineFlags( RedlineFlags::ShowDelete );
    getIDocumentRedlineAccess().SetRedlineFlags( RedlineFlags::ShowDelete );

    CompareMainText aD0(rSrcDoc, false);
    CompareMainText aD1(*this, false);
    aD1.CompareLines( aD0 );
    if( !aD1.HasDiffs( aD0 ) )
    {
        // we want to get all redlines from the SourceDoc

        // look for all insert redlines from the SourceDoc and determine their position in the DestDoc
        std::vector<SaveMergeRedline> vRedlines;
        const SwRedlineTable& rSrcRedlTable = rSrcDoc.getIDocumentRedlineAccess().GetRedlineTable();
        sal_uLong nEndOfExtra = rSrcDoc.GetNodes().GetEndOfExtras().GetIndex();
        sal_uLong nMyEndOfExtra = GetNodes().GetEndOfExtras().GetIndex();
        for(const SwRangeRedline* pRedl : rSrcRedlTable)
        {
            sal_uLong nNd = pRedl->GetPoint()->nNode.GetIndex();
            RedlineType eType = pRedl->GetType();
            if( nEndOfExtra < nNd &&
                ( RedlineType::Insert == eType || RedlineType::Delete == eType ))
            {
                const SwNode* pDstNd = GetNodes()[
                                        nMyEndOfExtra + nNd - nEndOfExtra ];

                // Found the position.
                // Then we also have to insert the redline to the line in the DestDoc.
                vRedlines.emplace_back(*pDstNd, *pRedl);
            }
        }

        if( !vRedlines.empty() )
        {
            // Carry over all into DestDoc
            rSrcDoc.getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);

            getIDocumentRedlineAccess().SetRedlineFlags(
                                      RedlineFlags::On |
                                      RedlineFlags::ShowInsert |
                                      RedlineFlags::ShowDelete);

            SwPaM* pLastDestRedline(nullptr);
            for(SaveMergeRedline& rRedline: vRedlines)
            {
                nRet += rRedline.InsertRedline(pLastDestRedline);
                pLastDestRedline = rRedline.pDestRedl;
            }
        }
    }

    rSrcDoc.getIDocumentRedlineAccess().SetRedlineFlags( eSrcRedlMode );
    if( !bSrcModified )
        rSrcDoc.getIDocumentState().ResetModified();

    getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);

    GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);

    return nRet;
}

LineArrayComparator::LineArrayComparator( const CompareData &rD1,
                                            const CompareData &rD2, int nStt1,
                                            int nEnd1, int nStt2, int nEnd2 )
    : m_rData1( rD1 ), m_rData2( rD2 ), m_nFirst1( nStt1 ), m_nFirst2( nStt2 )
{
    m_nLen1 = nEnd1 - nStt1;
    m_nLen2 = nEnd2 - nStt2;
}

bool LineArrayComparator::Compare( int nIdx1, int nIdx2 ) const
{
    if( nIdx1 < 0 || nIdx2 < 0 || nIdx1 >= m_nLen1 || nIdx2 >= m_nLen2 )
    {
        OSL_ENSURE( false, "Index out of range!" );
        return false;
    }

    const SwTextNode *pTextNd1 = m_rData1.GetLine( m_nFirst1 + nIdx1 )->GetNode().GetTextNode();
    const SwTextNode *pTextNd2 = m_rData2.GetLine( m_nFirst2 + nIdx2 )->GetNode().GetTextNode();

    if( !pTextNd1 || !pTextNd2
        || ( CmpOptions.bUseRsid && !pTextNd1->CompareParRsid( *pTextNd2 ) ) )
    {
        return false;
    }

    const sal_Int32 nPar1Len = pTextNd1->Len();
    const sal_Int32 nPar2Len = pTextNd2->Len();

    if( std::min( nPar1Len, nPar2Len ) * 3 < std::max( nPar1Len, nPar2Len ) )
    {
        return false;
    }

    sal_Int32 nBorderLen = ( nPar1Len + nPar2Len )/16;

    if( nBorderLen < 3 )
    {
        nBorderLen = std::min<sal_Int32>( 3, std::min( nPar1Len, nPar2Len ) );
    }

    std::set<unsigned> aHashes;
    unsigned nHash = 0;
    unsigned nMul = 251;
    unsigned nPow = 1;
    sal_Int32 i;

    for( i = 0; i < nBorderLen - 1; i++ )
    {
        nPow *= nMul;
    }
    for( i = 0; i < nBorderLen; i++ )
    {
        nHash = nHash*nMul + pTextNd1->GetText()[i];
    }
    aHashes.insert( nHash );
    for( ; i < nPar1Len; i++ )
    {
        nHash = nHash - nPow*pTextNd1->GetText()[ i - nBorderLen ];
        nHash = nHash*nMul + pTextNd1->GetText()[ i ];

        aHashes.insert( nHash );
    }

    nHash = 0;
    for( i = 0; i < nBorderLen; i++ )
    {
        nHash = nHash*nMul + pTextNd2->GetText()[ i ];
    }

    if( aHashes.find( nHash ) != aHashes.end() )
    {
        return true;
    }

    for( ; i < nPar2Len; i++ )
    {
        nHash = nHash - nPow*pTextNd2->GetText()[ i - nBorderLen ];
        nHash = nHash*nMul + pTextNd2->GetText()[ i ];
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
        OSL_ENSURE( false, "Index out of range!" );
        return false;
    }

    return ( !CmpOptions.bUseRsid
            || m_pTextNode1->CompareRsid(  *m_pTextNode2, nIdx1 + 1, nIdx2 + 1 ) )
            && m_pTextNode1->GetText()[ nIdx1 ] == m_pTextNode2->GetText()[ nIdx2 ];
}

WordArrayComparator::WordArrayComparator( const SwTextNode *pNode1,
                                            const SwTextNode *pNode2 )
    : m_pTextNode1( pNode1 ), m_pTextNode2( pNode2 )
{
    m_pPos1.reset( new int[ m_pTextNode1->GetText().getLength() + 1 ] );
    m_pPos2.reset( new int[ m_pTextNode2->GetText().getLength() + 1 ] );

    CalcPositions( m_pPos1.get(), m_pTextNode1, m_nCount1 );
    CalcPositions( m_pPos2.get(), m_pTextNode2, m_nCount2 );
}

bool WordArrayComparator::Compare( int nIdx1, int nIdx2 ) const
{
    int nLen = m_pPos1[ nIdx1 + 1 ] - m_pPos1[ nIdx1 ];
    if( nLen != m_pPos2[ nIdx2 + 1 ] - m_pPos2[ nIdx2 ] )
    {
        return false;
    }
    for( int i = 0; i < nLen; i++)
    {
        if( m_pTextNode1->GetText()[ m_pPos1[ nIdx1 ] + i ]
            != m_pTextNode2->GetText()[ m_pPos2[ nIdx2 ] + i ]
            || ( CmpOptions.bUseRsid && !m_pTextNode1->CompareRsid( *m_pTextNode2,
                                m_pPos1[ nIdx1 ] + i, m_pPos2[ nIdx2 ] + i ) ) )
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
        if( m_pPos1[ pWordLcs1[i] + 1 ] - m_pPos1[ pWordLcs1[i] ]
            != m_pPos2[ pWordLcs2[i] + 1 ] - m_pPos2[ pWordLcs2[i] ] )
        {
            continue;
        }
        for( int j = 0; j < m_pPos1[pWordLcs1[i]+1] - m_pPos1[pWordLcs1[i]]; j++)
        {
            pSubseq1[ nLen ] = m_pPos1[ pWordLcs1[i] ] + j;
            pSubseq2[ nLen ] = m_pPos2[ pWordLcs2[i] ] + j;

            if( m_pTextNode1->GetText()[ m_pPos1[ pWordLcs1[i] ] + j ]
             != m_pTextNode2->GetText()[ m_pPos2[ pWordLcs2[i] ] + j ] )
            {
                nLen -= j;
                break;
            }

            nLen++;
        }
    }
    return nLen;
}

void WordArrayComparator::CalcPositions( int *pPos, const SwTextNode *pTextNd,
                                         int &nCnt )
{
    nCnt = -1;
    for (int i = 0; i <= pTextNd->GetText().getLength(); ++i)
    {
        if (i == 0 || i == pTextNd->GetText().getLength()
                    || !rtl::isAsciiAlphanumeric( pTextNd->GetText()[ i - 1 ])
                    || !rtl::isAsciiAlphanumeric( pTextNd->GetText()[ i ]))
        { // Begin new word
            nCnt++;
            pPos[ nCnt ] = i;
        }
    }
}

int CommonSubseq::FindLCS( int *pLcs1, int *pLcs2, int nStt1, int nEnd1,
                                                    int nStt2, int nEnd2 )
{
    int nLen1 = nEnd1 ? nEnd1 - nStt1 : m_rComparator.GetLen1();
    int nLen2 = nEnd2 ? nEnd2 - nStt2 : m_rComparator.GetLen2();

    assert( nLen1 >= 0 );
    assert( nLen2 >= 0 );

    std::unique_ptr<int*[]> pLcs( new int*[ nLen1 + 1 ] );
    pLcs[ 0 ] = m_pData.get();

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
            if( m_rComparator.Compare( nStt1 + i - 1, nStt2 + j - 1 ) )
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
                nIdx1--;
                nIdx2--;
                pLcs1[ nIdx ] = nIdx1 + nStt1;
                pLcs2[ nIdx ] = nIdx2 + nStt2;
                nIdx--;
            }
        }
    }

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
    m_pBuff1.reset( new int[ rComparator.GetLen2() + 1 ] );
    m_pBuff2.reset( new int[ rComparator.GetLen2() + 1 ] );

    m_pL1.reset( new int[ rComparator.GetLen2() + 1 ] );
    m_pL2.reset( new int[ rComparator.GetLen2() + 1 ] );
}

void LgstCommonSubseq::FindL( int *pL, int nStt1, int nEnd1,
                                        int nStt2, int nEnd2  )
{
    int nLen1 = nEnd1 ? nEnd1 - nStt1 : m_rComparator.GetLen1();
    int nLen2 = nEnd2 ? nEnd2 - nStt2 : m_rComparator.GetLen2();

    int *currL = m_pBuff1.get();
    int *prevL = m_pBuff2.get();

    // Avoid memory corruption
    if( nLen2 > m_rComparator.GetLen2() )
    {
        assert( false );
        return;
    }

    memset( m_pBuff1.get(), 0, sizeof( m_pBuff1[0] ) * ( nLen2 + 1 ) );
    memset( m_pBuff2.get(), 0, sizeof( m_pBuff2[0] ) * ( nLen2 + 1 ) );

    // Find lcs
    for( int i = 1; i <= nLen1; i++ )
    {
        for( int j = 1; j <= nLen2; j++ )
        {
            if( m_rComparator.Compare( nStt1 + i - 1, nStt2 + j - 1 ) )
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

    FindL( m_pL1.get(), nStt1, nStt1 + nMid, nStt2, nEnd2 );
    FindL( m_pL2.get(), nStt1 + nMid, nEnd1, nStt2, nEnd2 );

    int nMaxPos = 0;
    static int nMaxVal;
    nMaxVal = -1;

    static int i;
    for( i = 0; i <= nLen2; i++ )
    {
        if( m_pL1[i] + ( m_pL2[nLen2] - m_pL2[i] ) > nMaxVal )
        {
            nMaxPos = i;
            nMaxVal = m_pL1[i]+( m_pL2[nLen2] - m_pL2[i] );
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
    int nEnd1 = m_rComparator.GetLen1();
    int nEnd2 = m_rComparator.GetLen2();

    // Check for corresponding lines in the beginning of the sequences
    while( nStt < nEnd1 && nStt < nEnd2 && m_rComparator.Compare( nStt, nStt ) )
    {
        pSubseq1[ nStt ] = nStt;
        pSubseq2[ nStt ] = nStt;
        nStt++;
    }

    pSubseq1 += nStt;
    pSubseq2 += nStt;

    // Check for corresponding lines in the end of the sequences
    while( nStt < nEnd1 && nStt < nEnd2
                        && m_rComparator.Compare( nEnd1 - 1, nEnd2 - 1 ) )
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
    while( nStt1 < nEnd1 && nStt2 < nEnd2 && m_rComparator.Compare( nStt1, nStt2 ) )
    {
        pSeq1[ nCutBeg ] = nStt1++;
        pSeq2[ nCutBeg ] = nStt2++;
        nCutBeg++;
    }

    pSeq1 += nCutBeg;
    pSeq2 += nCutBeg;

    // Check for corresponding lines in the end of the sequences
    while( nStt1 < nEnd1 && nStt2 < nEnd2
                        && m_rComparator.Compare( nEnd1 - 1, nEnd2 - 1 ) )
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
            if( m_rComparator.Compare( nStt1 + i, nStt2 + nMid2 - nRad ) )
            {
                nPos1 = nStt1 + i;
                nPos2 = nStt2 + nMid2 - nRad;
                break;
            }
            if( m_rComparator.Compare( nStt1 + i, nStt2 + nMid2 + nRad ) )
            {
                nPos1 = nStt1 + i;
                nPos2 = nStt2 + nMid2 - nRad;
                break;
            }
        }
        // Search to the left and to the right of the middle of the second sequence
        for( int i = nMid2 - nRad; i <= nMid2 + nRad; i++ )
        {
            if( m_rComparator.Compare( nStt2 + nMid2 - nRad, nStt2 + i ) )
            {
                nPos2 = nStt2 + i;
                nPos1 = nStt1 + nMid1 - nRad;
                break;
            }
            if( m_rComparator.Compare( nStt2 + nMid2 - nRad, nStt2 + i ) )
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
