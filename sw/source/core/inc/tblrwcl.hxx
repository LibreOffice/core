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
#ifndef _TBLRWCL_HXX
#define _TBLRWCL_HXX
#include <cstddef>
#include <vector>
#include <swtypes.hxx>
#include <tblsel.hxx>
#include <swtable.hxx>

namespace editeng { class SvxBorderLine; }

class SwDoc;
class SwTableNode;
class SwTableLine;
class SwTableBox;
class SwTableBoxFmt;
class SwHistory;
class SwCntntNode;
class SfxPoolItem;
class SwShareBoxFmts;
class SwFmtFrmSize;
struct _CpyPara;
struct _InsULPara;

void sw_LineSetHeadCondColl( const SwTableLine* pLine );


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
    std::vector<sal_uInt16> aPosArr;
    std::vector<SwTableBox*> m_Boxes;
    SwHistory* pHst;
    sal_uInt16 nMode, nWidth;
    sal_Bool bGetFromTop : 1;
    sal_Bool bGetValues : 1;

public:
    SwCollectTblLineBoxes( sal_Bool bTop, sal_uInt16 nMd = 0, SwHistory* pHist=0 )
        :
        pHst( pHist ), nMode( nMd ), nWidth( 0 ),
        bGetFromTop( bTop ), bGetValues( sal_True )

    {}

    void AddBox( const SwTableBox& rBox );
    const SwTableBox* GetBoxOfPos( const SwTableBox& rBox );
    void AddToUndoHistory( const SwCntntNode& rNd );

    size_t Count() const                { return m_Boxes.size(); }
    const SwTableBox& GetBox( std::size_t nPos, sal_uInt16* pWidth = 0 ) const
        {
            // hier wird die EndPos der Spalte benoetigt!
            if( pWidth )
                *pWidth = (nPos+1 == aPosArr.size()) ? nWidth
                                                    : aPosArr[ nPos+1 ];
            return *m_Boxes[ nPos ];
        }

    sal_Bool IsGetFromTop() const           { return bGetFromTop; }
    sal_Bool IsGetValues() const            { return bGetValues; }

    sal_uInt16 GetMode() const              { return nMode; }
    void SetValues( sal_Bool bFlag )        { bGetValues = sal_False; nWidth = 0;
                                          bGetFromTop = bFlag; }
    sal_Bool Resize( sal_uInt16 nOffset, sal_uInt16 nWidth );
};

void sw_Box_CollectBox( const SwTableBox* pBox, SwCollectTblLineBoxes* pSplPara );
sal_Bool sw_Line_CollectBox( const SwTableLine*& rpLine, void* pPara );

void sw_BoxSetSplitBoxFmts( SwTableBox* pBox, SwCollectTblLineBoxes* pSplPara );

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
    sal_Bool IsLastLine() const { return nLinePos + 1 >= (sal_uInt16)pLines->size(); }
};

class _SwGCBorder_BoxBrd
{
    const editeng::SvxBorderLine* pBrdLn;
    sal_Bool bAnyBorderFnd;
public:
    _SwGCBorder_BoxBrd() : pBrdLn( 0 ), bAnyBorderFnd( sal_False ) {}

    void SetBorder( const editeng::SvxBorderLine& rBorderLine )
        { pBrdLn = &rBorderLine; bAnyBorderFnd = sal_False; }

    // checke, ob die linke Border dieselbe wie die gesetzte ist
    // returnt sal_False falls gar keine Border gesetzt ist
    sal_Bool CheckLeftBorderOfFormat( const SwFrmFmt& rFmt );

    sal_Bool IsAnyBorderFound() const { return bAnyBorderFnd; }
};

void sw_GC_Line_Border( const SwTableLine* pLine, _SwGCLineBorder* pGCPara );


class SwShareBoxFmt
{
    const SwFrmFmt* pOldFmt;
    std::vector<SwFrmFmt*> aNewFmts;

public:
    SwShareBoxFmt( const SwFrmFmt& rFmt )
        : pOldFmt( &rFmt )
    {}

    const SwFrmFmt& GetOldFormat() const { return *pOldFmt; }

    SwFrmFmt* GetFormat( long nWidth ) const;
    SwFrmFmt* GetFormat( const SfxPoolItem& rItem ) const;
    void AddFormat( SwFrmFmt& rFmt );
    // returnt sal_True, wenn geloescht werden kann
    bool RemoveFormat( const SwFrmFmt& rFmt );
};


typedef boost::ptr_vector<SwShareBoxFmt> _SwShareBoxFmts;

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

    void AddFormat( const SwFrmFmt& rOld, SwFrmFmt& rNew );

    void SetSize( SwTableBox& rBox, const SwFmtFrmSize& rSz );
    void SetAttr( SwTableBox& rBox, const SfxPoolItem& rItem );
    void SetAttr( SwTableLine& rLine, const SfxPoolItem& rItem );

    void RemoveFormat( const SwFrmFmt& rFmt );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
