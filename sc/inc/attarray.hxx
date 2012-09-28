/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_ATRARR_HXX
#define SC_ATRARR_HXX

#include "global.hxx"
#include "attrib.hxx"

class ScDocument;
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

friend class ScDocument;                // fuer FillInfo
friend class ScDocumentIterator;
friend class ScAttrIterator;
friend class ScHorizontalAttrIterator;
friend void lcl_IterGetNumberFormat( sal_uLong& nFormat,
        const ScAttrArray*& rpArr, SCROW& nAttrEndRow,
        const ScAttrArray* pNewArr, SCROW nRow, ScDocument* pDoc );

    sal_Bool    ApplyFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow,
                            sal_Bool bLeft, SCCOL nDistRight, sal_Bool bTop, SCROW nDistBottom );

public:
            ScAttrArray( SCCOL nNewCol, SCTAB nNewTab, ScDocument* pDoc );
            ~ScAttrArray();

    void    SetTab(SCTAB nNewTab)   { nTab = nNewTab; }
    void    SetCol(SCCOL nNewCol)   { nCol = nNewCol; }
#ifdef DBG_UTIL
    void    TestData() const;
#endif
    void    Reset( const ScPatternAttr* pPattern, sal_Bool bAlloc = sal_True );
    sal_Bool    Concat(SCSIZE nPos);

    const ScPatternAttr* GetPattern( SCROW nRow ) const;
    const ScPatternAttr* GetPatternRange( SCROW& rStartRow, SCROW& rEndRow, SCROW nRow ) const;
    void    MergePatternArea( SCROW nStartRow, SCROW nEndRow, ScMergePatternState& rState, sal_Bool bDeep ) const;

    void    MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner, ScLineFlags& rFlags,
                            SCROW nStartRow, SCROW nEndRow, sal_Bool bLeft, SCCOL nDistRight ) const;
    void    ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                            SCROW nStartRow, SCROW nEndRow, sal_Bool bLeft, SCCOL nDistRight );

    void    SetPattern( SCROW nRow, const ScPatternAttr* pPattern, sal_Bool bPutToPool = sal_False );
    void    SetPatternArea( SCROW nStartRow, SCROW nEndRow, const ScPatternAttr* pPattern, sal_Bool bPutToPool = sal_False);
    void    ApplyStyleArea( SCROW nStartRow, SCROW nEndRow, ScStyleSheet* pStyle );
    void    ApplyCacheArea( SCROW nStartRow, SCROW nEndRow, SfxItemPoolCache* pCache );
    void    ApplyLineStyleArea( SCROW nStartRow, SCROW nEndRow,
                                const SvxBorderLine* pLine, sal_Bool bColorOnly );

    void    ClearItems( SCROW nStartRow, SCROW nEndRow, const sal_uInt16* pWhich );
    void    ChangeIndent( SCROW nStartRow, SCROW nEndRow, sal_Bool bIncrement );

            /// Including current, may return -1
    SCsROW  GetNextUnprotected( SCsROW nRow, sal_Bool bUp ) const;

            /// May return -1 if not found
    SCsROW  SearchStyle( SCsROW nRow, const ScStyleSheet* pSearchStyle,
                            sal_Bool bUp, ScMarkArray* pMarkArray = NULL );
    sal_Bool    SearchStyleRange( SCsROW& rRow, SCsROW& rEndRow, const ScStyleSheet* pSearchStyle,
                            sal_Bool bUp, ScMarkArray* pMarkArray = NULL );

    sal_Bool    ApplyFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags );
    sal_Bool    RemoveFlags( SCROW nStartRow, SCROW nEndRow, sal_Int16 nFlags );

    sal_Bool    Search( SCROW nRow, SCSIZE& nIndex ) const;

    sal_Bool    HasLines( SCROW nRow1, SCROW nRow2, Rectangle& rSizes,
                        sal_Bool bLeft, sal_Bool bRight ) const;
    bool    HasAttrib( SCROW nRow1, SCROW nRow2, sal_uInt16 nMask ) const;
    sal_Bool    ExtendMerge( SCCOL nThisCol, SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rPaintCol, SCROW& rPaintRow,
                                sal_Bool bRefresh, sal_Bool bAttrs );
    sal_Bool    RemoveAreaMerge( SCROW nStartRow, SCROW nEndRow );

    void    FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, ScFlatBoolRowSegments& rUsedRows, bool bReset );
    sal_Bool    IsStyleSheetUsed( const ScStyleSheet& rStyle, sal_Bool bGatherAllStyles ) const;

    void    DeleteAreaSafe(SCROW nStartRow, SCROW nEndRow);
    void    SetPatternAreaSafe( SCROW nStartRow, SCROW nEndRow,
                                    const ScPatternAttr* pWantedPattern, sal_Bool bDefault );
    void    CopyAreaSafe( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray );

    sal_Bool    IsEmpty() const;

//UNUSED2008-05  SCROW  GetFirstEntryPos() const;
//UNUSED2008-05  SCROW  GetLastEntryPos( sal_Bool bIncludeBottom ) const;

    sal_Bool    GetFirstVisibleAttr( SCROW& rFirstRow ) const;
    sal_Bool    GetLastVisibleAttr( SCROW& rLastRow, SCROW nLastData ) const;
    /*
    Get the last cell's row number , which have visual atribute or visual data in attribute list
    */
    sal_Bool    GetLastAttr( SCROW& rLastRow, SCROW nLastData ) const;
    sal_Bool    HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const;
    sal_Bool    IsVisibleEqual( const ScAttrArray& rOther,
                            SCROW nStartRow, SCROW nEndRow ) const;
    sal_Bool    IsAllEqual( const ScAttrArray& rOther, SCROW nStartRow, SCROW nEndRow ) const;

    sal_Bool    TestInsertCol( SCROW nStartRow, SCROW nEndRow) const;
    sal_Bool    TestInsertRow( SCSIZE nSize ) const;
    void    InsertRow( SCROW nStartRow, SCSIZE nSize );
    void    DeleteRow( SCROW nStartRow, SCSIZE nSize );
    void    DeleteRange( SCSIZE nStartIndex, SCSIZE nEndIndex );
    void    DeleteArea( SCROW nStartRow, SCROW nEndRow );
    void    MoveTo( SCROW nStartRow, SCROW nEndRow, ScAttrArray& rAttrArray );
    void    CopyArea( SCROW nStartRow, SCROW nEndRow, long nDy, ScAttrArray& rAttrArray,
                        sal_Int16 nStripFlags = 0 );

    void    DeleteHardAttr( SCROW nStartRow, SCROW nEndRow );

//UNUSED2008-05  void    ConvertFontsAfterLoad();     // old binary file format
};


//  ------------------------------------------------------------------------------
//                              Iterator fuer Attribute
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


