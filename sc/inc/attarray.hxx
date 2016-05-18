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

#ifndef INCLUDED_SC_INC_ATTARRAY_HXX
#define INCLUDED_SC_INC_ATTARRAY_HXX

#include "global.hxx"
#include "attrib.hxx"
#include <algorithm>

class ScDocument;
class ScEditDataArray;
class ScMarkArray;
class ScPatternAttr;
class ScStyleSheet;
class ScFlatBoolRowSegments;

class SfxItemPoolCache;
class SfxStyleSheetBase;
class SvxBoxItem;
class SvxBoxInfoItem;

namespace editeng { class SvxBorderLine; }

#define SC_LINE_EMPTY           0
#define SC_LINE_SET             1
#define SC_LINE_DONTCARE        2

#define SC_ATTRARRAY_DELTA      4

struct ScLineFlags
{
    sal_uInt8   nLeft;
    sal_uInt8   nRight;
    sal_uInt8   nTop;
    sal_uInt8   nBottom;
    sal_uInt8   nHori;
    sal_uInt8   nVert;

    ScLineFlags() : nLeft(SC_LINE_EMPTY),nRight(SC_LINE_EMPTY),nTop(SC_LINE_EMPTY),
                    nBottom(SC_LINE_EMPTY),nHori(SC_LINE_EMPTY),nVert(SC_LINE_EMPTY) {}
};

struct ScMergePatternState
{
    SfxItemSet* pItemSet;           ///< allocated in MergePatternArea, used for resulting ScPatternAttr
    const ScPatternAttr* pOld1;     ///< existing objects, temporary
    const ScPatternAttr* pOld2;

    ScMergePatternState() : pItemSet(nullptr), pOld1(nullptr), pOld2(nullptr) {}
};

struct ScAttrEntry
{
    SCROW                   nRow;
    const ScPatternAttr*    pPattern;
};

class ScAttrArray
{
private:
    SCCOL           nCol;
    SCTAB           nTab;
    ScDocument*     pDocument;

    SCSIZE          nCount;
    SCSIZE          nLimit;
    ScAttrEntry*    pData;

friend class ScDocument;                // for FillInfo
friend class ScDocumentIterator;
friend class ScAttrIterator;
friend class ScHorizontalAttrIterator;

    bool    ApplyFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow,
                            bool bLeft, SCCOL nDistRight, bool bTop, SCROW nDistBottom );

    void RemoveCellCharAttribs( SCROW nStartRow, SCROW nEndRow,
                              const ScPatternAttr* pPattern, ScEditDataArray* pDataArray );

    ScAttrArray(const ScAttrArray&) = delete;
    ScAttrArray& operator=(const ScAttrArray&) = delete;

public:
            ScAttrArray( SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc );
            ~ScAttrArray();

    void    SetTab(SCTAB nNewTab)   { nTab = nNewTab; }
    void    SetCol(SCCOL nNewCol)   { nCol = nNewCol; }
#if OSL_DEBUG_LEVEL > 1
    void    TestData() const;
#endif
    void    Reset( const ScPatternAttr* pPattern);
    bool    Concat(SCSIZE nPos);

    const ScPatternAttr* GetPattern( SCROW nRow ) const;
    const ScPatternAttr* GetPatternRange( SCROW& rStartRow, SCROW& rEndRow, SCROW nRow ) const;
    void    MergePatternArea( SCROW nStartRow, SCROW nEndRow, ScMergePatternState& rState, bool bDeep ) const;

    void    MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner, ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight ) const;
    void    ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow, bool bLeft, SCCOL nDistRight );

    void    SetPattern( SCROW nRow, const ScPatternAttr* pPattern, bool bPutToPool = false );
    void    SetPatternArea( SCROW nStartRow, SCROW nEndRow, const ScPatternAttr* pPattern,
                            bool bPutToPool = false, ScEditDataArray* pDataArray = nullptr );
    void    ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, ScStyleSheet* pStyle );
    void    ApplyCacheArea( SCROW nStartRow, SCROW nEndRow, SfxItemPoolCache* pCache,
                            ScEditDataArray* pDataArray = nullptr );
    bool    SetAttrEntries(ScAttrEntry* pNewData, SCSIZE nSize);
    void    ApplyLineStyleArea( SCROW nStartRow, SCROW nEndRow,
                                const ::editeng::SvxBorderLine* pLine, bool bColorOnly );

    void    AddCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex );
    /// if nIndex == 0, remove all conditional format data
    void    RemoveCondFormat( SCROW nStartRow, SCROW nEndRow, sal_uInt32 nIndex );

    void    ClearItems( SCROW nStartRow, SCROW nEndRow, const sal_uInt16* pWhich );
    void    ChangeIndent( SCROW nStartRow, SCROW nEndRow, bool bIncrement );

            /// Including current, may return -1
    SCsROW  GetNextUnprotected( SCsROW nRow, bool bUp ) const;

            /// May return -1 if not found
    SCsROW SearchStyle(
        SCsROW nRow, const ScStyleSheet* pSearchStyle, bool bUp,
        const ScMarkArray* pMarkArray = nullptr) const;

    bool SearchStyleRange(
        SCsROW& rRow, SCsROW& rEndRow, const ScStyleSheet* pSearchStyle, bool bUp,
        const ScMarkArray* pMarkArray = nullptr) const;

    bool    ApplyFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags );
    bool    RemoveFlags( SCROW nStartRow, SCROW nEndRow, ScMF nFlags );

    bool    Search( SCROW nRow, SCSIZE& nIndex ) const;

    bool    HasAttrib( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const;
    bool IsMerged( SCROW nRow ) const;
    bool    ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                bool bRefresh );
    bool    RemoveAreaMerge( SCROW nStartRow, SCROW nEndRow );

    void    FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset );
    bool    IsStyleSheetUsed( const ScStyleSheet& rStyle, bool bGatherAllStyles ) const;

    void    DeleteAreaSafe(SCROW nStartRow, SCROW nEndRow);
    void    SetPatternAreaSafe( SCROW nStartRow, SCROW nEndRow,
                                    const ScPatternAttr* pWantedPattern, bool bDefault );
    void    CopyAreaSafe( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray );

    bool    IsEmpty() const;

    bool    GetFirstVisibleAttr( SCROW& rFirstRow ) const;
    bool    GetLastVisibleAttr( SCROW& rLastRow, SCROW nLastData, bool bFullFormattedArea = false ) const;
    bool    HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const;
    bool    IsVisibleEqual( const ScAttrArray& rOther,
                            SCROW nStartRow, SCROW nEndRow ) const;
    bool    IsAllEqual( const ScAttrArray& rOther, SCROW nStartRow, SCROW nEndRow ) const;

    bool    TestInsertCol( SCROW nStartRow, SCROW nEndRow) const;
    bool    TestInsertRow( SCSIZE nSize ) const;
    void    InsertRow( SCROW nStartRow, SCSIZE nSize );
    void    DeleteRow( SCROW nStartRow, SCSIZE nSize );
    void    DeleteRange( SCSIZE nStartIndex, SCSIZE nEndIndex );
    void    DeleteArea( SCROW nStartRow, SCROW nEndRow );
    void    MoveTo( SCROW nStartRow, SCROW nEndRow, ScAttrArray& rAttrArray );
    void    CopyArea(
        SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray, ScMF nStripFlags = ScMF::NONE) const;

    void    DeleteHardAttr( SCROW nStartRow, SCROW nEndRow );

    /* i123909: Pre-calculate needed memory, and pre-reserve enough memory */
    bool    Reserve( SCSIZE nReserve );
    SCSIZE  Count() const { return nCount; }
    SCSIZE  Count( SCROW nRow1, SCROW nRow2 ) const;
};

//                              Iterator for attributes

class ScAttrIterator
{
    const ScAttrArray*  pArray;
    SCSIZE              nPos;
    SCROW               nRow;
    SCROW               nEndRow;
public:
    inline              ScAttrIterator( const ScAttrArray* pNewArray, SCROW nStart, SCROW nEnd );
    inline const ScPatternAttr* Next( SCROW& rTop, SCROW& rBottom );
    inline const ScPatternAttr* Resync( SCROW nRow, SCROW& rTop, SCROW& rBottom );
    SCROW               GetNextRow() const { return nRow; }
};

inline ScAttrIterator::ScAttrIterator( const ScAttrArray* pNewArray, SCROW nStart, SCROW nEnd ) :
    pArray( pNewArray ),
    nRow( nStart ),
    nEndRow( nEnd )
{
    if ( nStart > 0 )
        pArray->Search( nStart, nPos );
    else
        nPos = 0;
}

inline const ScPatternAttr* ScAttrIterator::Next( SCROW& rTop, SCROW& rBottom )
{
    const ScPatternAttr* pRet;
    if ( nPos < pArray->nCount && nRow <= nEndRow )
    {
        rTop = nRow;
        rBottom = std::min( pArray->pData[nPos].nRow, nEndRow );
        pRet = pArray->pData[nPos].pPattern;
        nRow = rBottom + 1;
        ++nPos;
    }
    else
        pRet = nullptr;
    return pRet;
}

inline const ScPatternAttr* ScAttrIterator::Resync( SCROW nRowP, SCROW& rTop, SCROW& rBottom )
{
    nRow = nRowP;
    // Chances are high that the pattern changed on nRowP introduced a span
    // starting right there. Assume that Next() was called so nPos already
    // advanced. Another high chance is that the change extended a previous or
    // next pattern. In all these cases we don't need to search.
    if (3 <= nPos && nPos <= pArray->nCount && pArray->pData[nPos-3].nRow < nRowP &&
            nRowP <= pArray->pData[nPos-2].nRow)
        nPos -= 2;
    else if (2 <= nPos && nPos <= pArray->nCount && pArray->pData[nPos-2].nRow < nRowP &&
            nRowP <= pArray->pData[nPos-1].nRow)
        --nPos;
    else if (pArray->nCount > 0 && nRowP <= pArray->pData[0].nRow)
        nPos = 0;
    else
        pArray->Search( nRowP, nPos );
    return Next( rTop, rBottom);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
