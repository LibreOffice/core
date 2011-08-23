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
#ifndef _TBLRWCL_HXX
#define _TBLRWCL_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
namespace binfilter {

class SfxPoolItem;
class SvxBorderLine;
class SwDoc;
class SwTableNode;
class _FndLine;
class _FndBox;
class SwTableLine;
class SwTableBox;
class SwTableBoxFmt;
class SwTableLineFmt;
class SwHistory;
class SwCntntNode;

class SwShareBoxFmts;
class SwFmtFrmSize;

// Funktions Deklarationen:
BOOL lcl_CopyRow( const _FndLine*& rpFndLine, void* pPara );
BOOL lcl_CopyCol( const _FndBox*& rpFndBox, void* pPara );

BOOL lcl_MergeGCBox( const SwTableBox*& rpBox, void* pPara );
BOOL lcl_MergeGCLine( const SwTableLine*& rpLine, void* pPara );

BOOL lcl_Merge_MoveBox( const _FndBox*& rpFndBox, void* pPara );
BOOL lcl_Merge_MoveLine( const _FndLine*& rpFndLine, void* pPara );

BOOL lcl_CopyBoxToDoc( const _FndBox*& rpFndBox, void* pPara );
BOOL lcl_CopyLineToDoc( const _FndLine*& rpFndLn, void* pPara );

BOOL lcl_BoxSetHeadCondColl( const SwTableBox*& rpBox, void* pPara );
BOOL lcl_LineSetHeadCondColl( const SwTableLine*& rpLine, void* pPara );


#ifdef DBG_UTIL
#endif

void _InsTblBox( SwDoc* pDoc, SwTableNode* pTblNd,
                SwTableLine* pLine, SwTableBoxFmt* pBoxFrmFmt,
                SwTableBox* pBox, USHORT nInsPos, USHORT nCnt = 1 );

void _DeleteBox( SwTable& rTbl, SwTableBox* pBox,
                BOOL bCalcNewSize = TRUE, const BOOL bCorrBorder = TRUE,
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
    USHORT nMode, nWidth;
    BOOL bGetFromTop : 1;
    BOOL bGetValues : 1;

 public:
    SwCollectTblLineBoxes( BOOL bTop, USHORT nMd = 0, SwHistory* pHist=0 )
        : aPosArr( 16, 16 ), aBoxes( 16, 16 ),
        bGetFromTop( bTop ), bGetValues( TRUE ), nMode( nMd ),
        nWidth( 0 ), pHst( pHist )
    {}

    void AddBox( const SwTableBox& rBox );
    const SwTableBox* GetBoxOfPos( const SwTableBox& rBox );

    USHORT Count() const                { return aBoxes.Count(); }
    const SwTableBox& GetBox( USHORT nPos, USHORT* pWidth = 0 ) const
        {
            // hier wird die EndPos der Spalte benoetigt!
            if( pWidth )
                *pWidth = nPos+1 == aPosArr.Count() ? nWidth
                                                    : aPosArr[ nPos+1 ];
            return *aBoxes[ nPos ];
        }

    BOOL IsGetFromTop() const           { return bGetFromTop; }
    BOOL IsGetValues() const            { return bGetValues; }

    USHORT GetMode() const              { return nMode; }
    void SetValues( BOOL bFlag )        { bGetValues = FALSE; nWidth = 0;
                                          bGetFromTop = bFlag; }
    FASTBOOL Resize( USHORT nOffset, USHORT nWidth );
 };

BOOL lcl_Box_CollectBox( const SwTableBox*& rpBox, void* pPara );
BOOL lcl_Line_CollectBox( const SwTableLine*& rpLine, void* pPara );

BOOL lcl_BoxSetSplitBoxFmts( const SwTableBox*& rpBox, void* pPara );

struct _SwGCLineBorder
{
    const SwTableLines* pLines;
    SwShareBoxFmts* pShareFmts;
    USHORT nLinePos;

    _SwGCLineBorder( const SwTable& rTable )
        : pLines( &rTable.GetTabLines() ), nLinePos( 0 ), pShareFmts(0) {}

    _SwGCLineBorder( const SwTableBox& rBox )
        : pLines( &rBox.GetTabLines() ), nLinePos( 0 ), pShareFmts(0) {}
    BOOL IsLastLine() const { return nLinePos + 1 >= pLines->Count(); }
};

class _SwGCBorder_BoxBrd
{
    const SvxBorderLine* pBrdLn;
    BOOL bAnyBorderFnd;
public:
    _SwGCBorder_BoxBrd() : pBrdLn( 0 ), bAnyBorderFnd( FALSE ) {}

    void SetBorder( const SvxBorderLine& rBorderLine )
        { pBrdLn = &rBorderLine; bAnyBorderFnd = FALSE; }

    // checke, ob die linke Border dieselbe wie die gesetzte ist
    // returnt FALSE falls gar keine Border gesetzt ist
    BOOL CheckLeftBorderOfFormat( const SwFrmFmt& rFmt );

    BOOL IsAnyBorderFound() const { return bAnyBorderFnd; }
};

BOOL lcl_GC_Line_Border( const SwTableLine*& , void* pPara );
BOOL lcl_GC_Box_Border( const SwTableBox*& , void* pPara );

BOOL lcl_GCBorder_ChkBoxBrd_L( const SwTableLine*& , void* pPara );
BOOL lcl_GCBorder_ChkBoxBrd_B( const SwTableBox*& , void* pPara );

BOOL lcl_GCBorder_GetLastBox_L( const SwTableLine*& , void* pPara );
BOOL lcl_GCBorder_GetLastBox_B( const SwTableBox*& , void* pPara );


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
//STRIP001 	// returnt TRUE, wenn geloescht werden kann
    FASTBOOL RemoveFormat( const SwFrmFmt& rFmt );
};


SV_DECL_PTRARR_DEL( _SwShareBoxFmts, SwShareBoxFmt*, 8, 8 )

class SwShareBoxFmts
{
    _SwShareBoxFmts aShareArr;
    BOOL Seek_Entry( const SwFrmFmt& rFmt, USHORT* pPos ) const;

    void ChangeFrmFmt( SwTableBox* pBox, SwTableLine* pLn, SwFrmFmt& rFmt );
public:
    SwShareBoxFmts() {}
    ~SwShareBoxFmts();

    SwFrmFmt* GetFormat( const SwFrmFmt& rFmt, long nWidth ) const;
    SwFrmFmt* GetFormat( const SwFrmFmt& rFmt, const SfxPoolItem& ) const;
    void AddFormat( const SwFrmFmt& rOld, const SwFrmFmt& rNew );
    void SetSize( SwTableBox& rBox, const SwFmtFrmSize& rSz );
    void SetAttr( SwTableBox& rBox, const SfxPoolItem& rItem );
    void RemoveFormat( const SwFrmFmt& rFmt );
};



} //namespace binfilter
#endif
