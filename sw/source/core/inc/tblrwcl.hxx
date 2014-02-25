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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TBLRWCL_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TBLRWCL_HXX
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
                bool bCalcNewSize = true, const bool bCorrBorder = true,
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
    bool bGetFromTop : 1;
    bool bGetValues : 1;

public:
    SwCollectTblLineBoxes( bool bTop, sal_uInt16 nMd = 0, SwHistory* pHist=0 )
        :
        pHst( pHist ), nMode( nMd ), nWidth( 0 ),
        bGetFromTop( bTop ), bGetValues( true )

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

    bool IsGetFromTop() const           { return bGetFromTop; }
    bool IsGetValues() const            { return bGetValues; }

    sal_uInt16 GetMode() const              { return nMode; }
    void SetValues( bool bFlag )        { bGetValues = false; nWidth = 0;
                                          bGetFromTop = bFlag; }
    bool Resize( sal_uInt16 nOffset, sal_uInt16 nWidth );
};

void sw_Box_CollectBox( const SwTableBox* pBox, SwCollectTblLineBoxes* pSplPara );
bool sw_Line_CollectBox( const SwTableLine*& rpLine, void* pPara );

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
    bool IsLastLine() const { return nLinePos + 1 >= (sal_uInt16)pLines->size(); }
};

class _SwGCBorder_BoxBrd
{
    const editeng::SvxBorderLine* pBrdLn;
    bool bAnyBorderFnd;
public:
    _SwGCBorder_BoxBrd() : pBrdLn( 0 ), bAnyBorderFnd( false ) {}

    void SetBorder( const editeng::SvxBorderLine& rBorderLine )
        { pBrdLn = &rBorderLine; bAnyBorderFnd = false; }

    // checke, ob die linke Border dieselbe wie die gesetzte ist
    // returnt false falls gar keine Border gesetzt ist
    bool CheckLeftBorderOfFormat( const SwFrmFmt& rFmt );

    bool IsAnyBorderFound() const { return bAnyBorderFnd; }
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
    bool Seek_Entry( const SwFrmFmt& rFmt, sal_uInt16* pPos ) const;

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
