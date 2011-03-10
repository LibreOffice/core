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

#ifndef SC_ATRARR_HXX
#define SC_ATRARR_HXX

#include "global.hxx"
#include "attrib.hxx"

class ScDocument;
class ScEditDataArray;
class ScMarkArray;
class ScPatternAttr;
class ScStyleSheet;
class ScFlatBoolRowSegments;

class Rectangle;
class SfxItemPoolCache;
class SfxStyleSheetBase;
class SvxBorderLine;
class SvxBoxItem;
class SvxBoxInfoItem;

#define SC_LINE_EMPTY           0
#define SC_LINE_SET             1
#define SC_LINE_DONTCARE        2

#define SC_ATTRARRAY_DELTA      4

struct ScLineFlags
{
    BYTE    nLeft;
    BYTE    nRight;
    BYTE    nTop;
    BYTE    nBottom;
    BYTE    nHori;
    BYTE    nVert;

    ScLineFlags() : nLeft(SC_LINE_EMPTY),nRight(SC_LINE_EMPTY),nTop(SC_LINE_EMPTY),
                    nBottom(SC_LINE_EMPTY),nHori(SC_LINE_EMPTY),nVert(SC_LINE_EMPTY) {}
};

struct ScMergePatternState
{
    SfxItemSet* pItemSet;           // allocated in MergePatternArea, used for resulting ScPatternAttr
    const ScPatternAttr* pOld1;     // existing objects, temporary
    const ScPatternAttr* pOld2;

    ScMergePatternState() : pItemSet(NULL), pOld1(NULL), pOld2(NULL) {}
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
friend void lcl_IterGetNumberFormat( ULONG& nFormat,
        const ScAttrArray*& rpArr, SCROW& nAttrEndRow,
        const ScAttrArray* pNewArr, SCROW nRow, ScDocument* pDoc );

    BOOL    ApplyFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow,
                            BOOL bLeft, SCCOL nDistRight, BOOL bTop, SCROW nDistBottom );

    void RemoveCellCharAttribs( SCROW nStartRow, SCROW nEndRow,
                              const ScPatternAttr* pPattern, ScEditDataArray* pDataArray );

public:
            ScAttrArray( SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc );
            ~ScAttrArray();

    void    SetTab(SCTAB nNewTab)   { nTab = nNewTab; }
    void    SetCol(SCCOL nNewCol)   { nCol = nNewCol; }
#ifdef DBG_UTIL
    void    TestData() const;
#endif
    void    Reset( const ScPatternAttr* pPattern, BOOL bAlloc = TRUE );
    BOOL    Concat(SCSIZE nPos);

    const ScPatternAttr* GetPattern( SCROW nRow ) const;
    const ScPatternAttr* GetPatternRange( SCROW& rStartRow, SCROW& rEndRow, SCROW nRow ) const;
    void    MergePatternArea( SCROW nStartRow, SCROW nEndRow, ScMergePatternState& rState, BOOL bDeep ) const;

    void    MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner, ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, BOOL bLeft, SCCOL nDistRight ) const;
    void    ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow, BOOL bLeft, SCCOL nDistRight );

    void    SetPattern( SCROW nRow, const ScPatternAttr* pPattern, BOOL bPutToPool = FALSE );
    void    SetPatternArea( SCROW nStartRow, SCROW nEndRow, const ScPatternAttr* pPattern,
                            BOOL bPutToPool = FALSE, ScEditDataArray* pDataArray = NULL );
    void    ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, ScStyleSheet* pStyle );
    void    ApplyCacheArea( SCROW nStartRow, SCROW nEndRow, SfxItemPoolCache* pCache,
                            ScEditDataArray* pDataArray = NULL );
    bool    SetAttrEntries(ScAttrEntry* pNewData, SCSIZE nSize);
    void    ApplyLineStyleArea( SCROW nStartRow, SCROW nEndRow,
                                const SvxBorderLine* pLine, BOOL bColorOnly );

    void    ClearItems( SCROW nStartRow, SCROW nEndRow, const USHORT* pWhich );
    void    ChangeIndent( SCROW nStartRow, SCROW nEndRow, BOOL bIncrement );

            /// Including current, may return -1
    SCsROW  GetNextUnprotected( SCsROW nRow, BOOL bUp ) const;

            /// May return -1 if not found
    SCsROW  SearchStyle( SCsROW nRow, const ScStyleSheet* pSearchStyle,
                            BOOL bUp, ScMarkArray* pMarkArray = NULL );
    BOOL    SearchStyleRange( SCsROW& rRow, SCsROW& rEndRow, const ScStyleSheet* pSearchStyle,
                            BOOL bUp, ScMarkArray* pMarkArray = NULL );

    BOOL    ApplyFlags( SCROW nStartRow, SCROW nEndRow, INT16 nFlags );
    BOOL    RemoveFlags( SCROW nStartRow, SCROW nEndRow, INT16 nFlags );

    BOOL    Search( SCROW nRow, SCSIZE& nIndex ) const;

    BOOL    HasLines( SCROW nRow1, SCROW nRow2, Rectangle& rSizes,
                        BOOL bLeft, BOOL bRight ) const;
    bool    HasAttrib( SCROW nRow1, SCROW nRow2, USHORT nMask ) const;
    BOOL    ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                BOOL bRefresh, BOOL bAttrs );
    BOOL    RemoveAreaMerge( SCROW nStartRow, SCROW nEndRow );

    void    FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset );
    BOOL    IsStyleSheetUsed( const ScStyleSheet& rStyle, BOOL bGatherAllStyles ) const;

    void    DeleteAreaSafe(SCROW nStartRow, SCROW nEndRow);
    void    SetPatternAreaSafe( SCROW nStartRow, SCROW nEndRow,
                                    const ScPatternAttr* pWantedPattern, BOOL bDefault );
    void    CopyAreaSafe( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray );

    BOOL    IsEmpty() const;

    BOOL    GetFirstVisibleAttr( SCROW& rFirstRow ) const;
    BOOL    GetLastVisibleAttr( SCROW& rLastRow, SCROW nLastData ) const;
    BOOL    HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const;
    BOOL    IsVisibleEqual( const ScAttrArray& rOther,
                            SCROW nStartRow, SCROW nEndRow ) const;
    BOOL    IsAllEqual( const ScAttrArray& rOther, SCROW nStartRow, SCROW nEndRow ) const;

    BOOL    TestInsertCol( SCROW nStartRow, SCROW nEndRow) const;
    BOOL    TestInsertRow( SCSIZE nSize ) const;
    void    InsertRow( SCROW nStartRow, SCSIZE nSize );
    void    DeleteRow( SCROW nStartRow, SCSIZE nSize );
    void    DeleteRange( SCSIZE nStartIndex, SCSIZE nEndIndex );
    void    DeleteArea( SCROW nStartRow, SCROW nEndRow );
    void    MoveTo( SCROW nStartRow, SCROW nEndRow, ScAttrArray& rAttrArray );
    void    CopyArea( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray,
                        INT16 nStripFlags = 0 );

    void    DeleteHardAttr( SCROW nStartRow, SCROW nEndRow );
};


//  ------------------------------------------------------------------------------
//                              Iterator for attributes
//  ------------------------------------------------------------------------------

class ScAttrIterator
{
    const ScAttrArray*  pArray;
    SCSIZE              nPos;
    SCROW               nRow;
    SCROW               nEndRow;
public:
    inline              ScAttrIterator( const ScAttrArray* pNewArray, SCROW nStart, SCROW nEnd );
    inline const ScPatternAttr* Next( SCROW& rTop, SCROW& rBottom );
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
        rBottom = Min( pArray->pData[nPos].nRow, nEndRow );
        pRet = pArray->pData[nPos].pPattern;
        nRow = rBottom + 1;
        ++nPos;
    }
    else
        pRet = NULL;
    return pRet;
}



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
