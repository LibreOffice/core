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


#ifndef _TBLRWCL_HXX
#define _TBLRWCL_HXX
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
#include <vector>
#include <swtypes.hxx>
#include <tblsel.hxx>
#include <swtable.hxx>

class SvxBorderLine;
class SwDoc;
class SwTableNode;
class _FndLine;
class _FndBox;
class SwTableLine;
class SwTableBox;
class SwTableBoxFmt;
class SwHistory;
class SwCntntNode;
class SfxPoolItem;
class SwShareBoxFmts;
class SwFmtFrmSize;

// Funktions Deklarationen:
sal_Bool lcl_CopyRow( const _FndLine*& rpFndLine, void* pPara );
sal_Bool lcl_CopyCol( const _FndBox*& rpFndBox, void* pPara );

sal_Bool lcl_MergeGCBox( const SwTableBox*& rpBox, void* pPara );
sal_Bool lcl_MergeGCLine( const SwTableLine*& rpLine, void* pPara );

sal_Bool lcl_Merge_MoveBox( const _FndBox*& rpFndBox, void* pPara );
sal_Bool lcl_Merge_MoveLine( const _FndLine*& rpFndLine, void* pPara );

sal_Bool lcl_CopyBoxToDoc( const _FndBox*& rpFndBox, void* pPara );
sal_Bool lcl_CopyLineToDoc( const _FndLine*& rpFndLn, void* pPara );

sal_Bool lcl_BoxSetHeadCondColl( const SwTableBox*& rpBox, void* pPara );
sal_Bool lcl_LineSetHeadCondColl( const SwTableLine*& rpLine, void* pPara );


#ifdef DBG_UTIL
void _CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize );
#endif

void _InsTblBox( SwDoc* pDoc, SwTableNode* pTblNd,
                SwTableLine* pLine, SwTableBoxFmt* pBoxFrmFmt,
                SwTableBox* pBox, sal_uInt16 nInsPos, sal_uInt16 nCnt = 1 );

SW_DLLPUBLIC void _DeleteBox( SwTable& rTbl, SwTableBox* pBox, SwUndo* pUndo = 0,
                sal_Bool bCalcNewSize = sal_True, const sal_Bool bCorrBorder = sal_True,
                SwShareBoxFmts* pShareFmts = 0 );

// Klasse fuers SplitTable
// sammelt von einer Line die Boxen aller obersten oder untersten Lines
// in einem Array. Zusaetzlich werden die Positionen vermerkt.
// ( die Implementierung steht im ndtbl.cxx)

class SwCollectTblLineBoxes
{
    SvUShorts aPosArr;
    SwSelBoxes_SAR aBoxes;
    SwHistory* pHst;
    sal_uInt16 nMode, nWidth;
    sal_Bool bGetFromTop : 1;
    sal_Bool bGetValues : 1;

public:
    SwCollectTblLineBoxes( sal_Bool bTop, sal_uInt16 nMd = 0, SwHistory* pHist=0 )
        : aPosArr( 16, 16 ), aBoxes( 16, 16 ),
        pHst( pHist ), nMode( nMd ), nWidth( 0 ),
        bGetFromTop( bTop ), bGetValues( sal_True )

    {}

    void AddBox( const SwTableBox& rBox );
    const SwTableBox* GetBoxOfPos( const SwTableBox& rBox );
    void AddToUndoHistory( const SwCntntNode& rNd );

    sal_uInt16 Count() const                { return aBoxes.Count(); }
    const SwTableBox& GetBox( sal_uInt16 nPos, sal_uInt16* pWidth = 0 ) const
        {
            // hier wird die EndPos der Spalte benoetigt!
            if( pWidth )
                *pWidth = nPos+1 == aPosArr.Count() ? nWidth
                                                    : aPosArr[ nPos+1 ];
            return *aBoxes[ nPos ];
        }

    sal_Bool IsGetFromTop() const           { return bGetFromTop; }
    sal_Bool IsGetValues() const            { return bGetValues; }

    sal_uInt16 GetMode() const              { return nMode; }
    void SetValues( sal_Bool bFlag )        { bGetValues = sal_False; nWidth = 0;
                                          bGetFromTop = bFlag; }
    sal_Bool Resize( sal_uInt16 nOffset, sal_uInt16 nWidth );
};

sal_Bool lcl_Box_CollectBox( const SwTableBox*& rpBox, void* pPara );
sal_Bool lcl_Line_CollectBox( const SwTableLine*& rpLine, void* pPara );

sal_Bool lcl_BoxSetSplitBoxFmts( const SwTableBox*& rpBox, void* pPara );

// This structure is needed by Undo to restore row span attributes
// when a table has been splitted into two tables
struct SwSaveRowSpan
{
    sal_uInt16 mnSplitLine; // the line number where the table has been splitted
    std::vector< long > mnRowSpans; // the row span attributes in this line
    SwSaveRowSpan( SwTableBoxes& rBoxes, sal_uInt16 nSplitLn );
};

struct _SwGCLineBorder
{
    const SwTableLines* pLines;
    SwShareBoxFmts* pShareFmts;
    sal_uInt16 nLinePos;

    _SwGCLineBorder( const SwTable& rTable )
        : pLines( &rTable.GetTabLines() ), pShareFmts(0), nLinePos( 0 )  {}

    _SwGCLineBorder( const SwTableBox& rBox )
        : pLines( &rBox.GetTabLines() ), pShareFmts(0), nLinePos( 0 )  {}
    sal_Bool IsLastLine() const { return nLinePos + 1 >= pLines->Count(); }
};

class _SwGCBorder_BoxBrd
{
    const SvxBorderLine* pBrdLn;
    sal_Bool bAnyBorderFnd;
public:
    _SwGCBorder_BoxBrd() : pBrdLn( 0 ), bAnyBorderFnd( sal_False ) {}

    void SetBorder( const SvxBorderLine& rBorderLine )
        { pBrdLn = &rBorderLine; bAnyBorderFnd = sal_False; }

    // checke, ob die linke Border dieselbe wie die gesetzte ist
    // returnt sal_False falls gar keine Border gesetzt ist
    sal_Bool CheckLeftBorderOfFormat( const SwFrmFmt& rFmt );

    sal_Bool IsAnyBorderFound() const { return bAnyBorderFnd; }
};

sal_Bool lcl_GC_Line_Border( const SwTableLine*& , void* pPara );
sal_Bool lcl_GC_Box_Border( const SwTableBox*& , void* pPara );

sal_Bool lcl_GCBorder_ChkBoxBrd_L( const SwTableLine*& , void* pPara );
sal_Bool lcl_GCBorder_ChkBoxBrd_B( const SwTableBox*& , void* pPara );

sal_Bool lcl_GCBorder_GetLastBox_L( const SwTableLine*& , void* pPara );
sal_Bool lcl_GCBorder_GetLastBox_B( const SwTableBox*& , void* pPara );


class SwShareBoxFmt
{
    const SwFrmFmt* pOldFmt;
    SvPtrarr aNewFmts;

public:
    SwShareBoxFmt( const SwFrmFmt& rFmt )
        : pOldFmt( &rFmt ), aNewFmts( 1, 4 )
    {}

    const SwFrmFmt& GetOldFormat() const { return *pOldFmt; }

    SwFrmFmt* GetFormat( long nWidth ) const;
    SwFrmFmt* GetFormat( const SfxPoolItem& rItem ) const;
    void AddFormat( const SwFrmFmt& rFmt );
    // returnt sal_True, wenn geloescht werden kann
    sal_Bool RemoveFormat( const SwFrmFmt& rFmt );
};


SV_DECL_PTRARR_DEL( _SwShareBoxFmts, SwShareBoxFmt*, 8, 8 )

class SwShareBoxFmts
{
    _SwShareBoxFmts aShareArr;
    sal_Bool Seek_Entry( const SwFrmFmt& rFmt, sal_uInt16* pPos ) const;

    void ChangeFrmFmt( SwTableBox* pBox, SwTableLine* pLn, SwFrmFmt& rFmt );

public:
    SwShareBoxFmts() {}
    ~SwShareBoxFmts();

    SwFrmFmt* GetFormat( const SwFrmFmt& rFmt, long nWidth ) const;
    SwFrmFmt* GetFormat( const SwFrmFmt& rFmt, const SfxPoolItem& ) const;

    void AddFormat( const SwFrmFmt& rOld, const SwFrmFmt& rNew );

    void SetSize( SwTableBox& rBox, const SwFmtFrmSize& rSz );
    void SetAttr( SwTableBox& rBox, const SfxPoolItem& rItem );
    void SetAttr( SwTableLine& rLine, const SfxPoolItem& rItem );

    void RemoveFormat( const SwFrmFmt& rFmt );
};



#endif
