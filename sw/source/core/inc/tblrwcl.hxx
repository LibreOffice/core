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
#include <memory>
#include <vector>

#include <swtypes.hxx>
#include <swtable.hxx>

namespace editeng { class SvxBorderLine; }

class SwDoc;
class SwTableNode;
class SwTableBoxFormat;
class SwHistory;
class SwContentNode;
class SfxPoolItem;
class SwShareBoxFormats;
class SwFormatFrameSize;

void sw_LineSetHeadCondColl( const SwTableLine* pLine );

#ifdef DBG_UTIL
void CheckBoxWidth( const SwTableLine& rLine, SwTwips nSize );
#endif

void InsTableBox( SwDoc& rDoc, SwTableNode* pTableNd,
                SwTableLine* pLine, SwTableBoxFormat* pBoxFrameFormat,
                SwTableBox* pBox, sal_uInt16 nInsPos, sal_uInt16 nCnt = 1 );

void DeleteBox_( SwTable& rTable, SwTableBox* pBox, SwUndo* pUndo,
                bool bCalcNewSize, const bool bCorrBorder,
                SwShareBoxFormats* pShareFormats = nullptr );

/**
 * Class for SplitTable
 * Collects the uppermost or lowermost Lines of a Box from a Line in an array.
 * We also store their positions.
 *
 * @see implementation in ndtbl.cxx
 */
class SwCollectTableLineBoxes
{
    std::vector<sal_uInt16> m_aPositionArr;
    std::vector<SwTableBox*> m_Boxes;
    SwHistory* m_pHistory;
    SplitTable_HeadlineOption m_nMode;
    sal_uInt16 m_nWidth;
    bool m_bGetFromTop : 1;
    bool m_bGetValues : 1;

public:
    SwCollectTableLineBoxes( bool bTop, SplitTable_HeadlineOption nMd = SplitTable_HeadlineOption::NONE, SwHistory* pHist=nullptr )
        :
        m_pHistory( pHist ), m_nMode( nMd ), m_nWidth( 0 ),
        m_bGetFromTop( bTop ), m_bGetValues( true )

    {}

    void AddBox( const SwTableBox& rBox );
    const SwTableBox* GetBoxOfPos( const SwTableBox& rBox );
    void AddToUndoHistory( const SwContentNode& rNd );

    size_t Count() const                { return m_Boxes.size(); }
    const SwTableBox& GetBox( std::size_t nPos, sal_uInt16* pWidth = nullptr ) const
        {
            // We need the EndPos of the column here!
            if( pWidth )
                *pWidth = (nPos+1 == m_aPositionArr.size()) ? m_nWidth
                                                    : m_aPositionArr[ nPos+1 ];
            return *m_Boxes[ nPos ];
        }

    bool IsGetFromTop() const           { return m_bGetFromTop; }
    bool IsGetValues() const            { return m_bGetValues; }

    SplitTable_HeadlineOption GetMode() const { return m_nMode; }
    void SetValues( bool bFlag )        { m_bGetValues = false; m_nWidth = 0;
                                          m_bGetFromTop = bFlag; }
    bool Resize( sal_uInt16 nOffset, sal_uInt16 nWidth );
};

void sw_Box_CollectBox( const SwTableBox* pBox, SwCollectTableLineBoxes* pSplPara );
bool sw_Line_CollectBox( const SwTableLine*& rpLine, void* pPara );

void sw_BoxSetSplitBoxFormats( SwTableBox* pBox, SwCollectTableLineBoxes* pSplPara );

/**
 * This structure is needed by Undo to restore row span attributes
 * when a table has been split into two tables
 */
struct SwSaveRowSpan
{
    sal_uInt16 mnSplitLine; // the line number where the table has been split
    std::vector< long > mnRowSpans; // the row span attributes in this line
    SwSaveRowSpan( SwTableBoxes& rBoxes, sal_uInt16 nSplitLn );
};

struct SwGCLineBorder
{
    const SwTableLines* pLines;
    SwShareBoxFormats* pShareFormats;
    sal_uInt16 nLinePos;

    SwGCLineBorder( const SwTable& rTable )
        : pLines( &rTable.GetTabLines() ), pShareFormats(nullptr), nLinePos( 0 )  {}

    SwGCLineBorder( const SwTableBox& rBox )
        : pLines( &rBox.GetTabLines() ), pShareFormats(nullptr), nLinePos( 0 )  {}
    bool IsLastLine() const { return nLinePos + 1 >= static_cast<sal_uInt16>(pLines->size()); }
};

class SwGCBorder_BoxBrd
{
    const editeng::SvxBorderLine* m_pBorderLine;
    bool m_bAnyBorderFind;
public:
    SwGCBorder_BoxBrd() : m_pBorderLine( nullptr ), m_bAnyBorderFind( false ) {}

    void SetBorder( const editeng::SvxBorderLine& rBorderLine )
        { m_pBorderLine = &rBorderLine; m_bAnyBorderFind = false; }

    /**
     * Check whether the left Border is the same as the set one
     * @returns false if no Border was set
     */
    bool CheckLeftBorderOfFormat( const SwFrameFormat& rFormat );

    bool IsAnyBorderFound() const { return m_bAnyBorderFind; }
};

void sw_GC_Line_Border( const SwTableLine* pLine, SwGCLineBorder* pGCPara );

class SwShareBoxFormat
{
    const SwFrameFormat* m_pOldFormat;
    std::vector<SwFrameFormat*> m_aNewFormats;

public:
    SwShareBoxFormat( const SwFrameFormat& rFormat )
        : m_pOldFormat( &rFormat )
    {}

    const SwFrameFormat& GetOldFormat() const { return *m_pOldFormat; }

    SwFrameFormat* GetFormat( long nWidth ) const;
    SwFrameFormat* GetFormat( const SfxPoolItem& rItem ) const;
    void AddFormat( SwFrameFormat& rFormat );
    /// @returns true, if we can delete
    bool RemoveFormat( const SwFrameFormat& rFormat );
};

class SwShareBoxFormats
{
    std::vector<std::unique_ptr<SwShareBoxFormat>> m_ShareArr;

    bool Seek_Entry( const SwFrameFormat& rFormat, sal_uInt16* pPos ) const;

    void ChangeFrameFormat( SwTableBox* pBox, SwTableLine* pLn, SwFrameFormat& rFormat );

public:
    SwShareBoxFormats() {}
    ~SwShareBoxFormats();

    SwFrameFormat* GetFormat( const SwFrameFormat& rFormat, long nWidth ) const;
    SwFrameFormat* GetFormat( const SwFrameFormat& rFormat, const SfxPoolItem& ) const;

    void AddFormat( const SwFrameFormat& rOld, SwFrameFormat& rNew );

    void SetSize( SwTableBox& rBox, const SwFormatFrameSize& rSz );
    void SetAttr( SwTableBox& rBox, const SfxPoolItem& rItem );
    void SetAttr( SwTableLine& rLine, const SfxPoolItem& rItem );

    void RemoveFormat( const SwFrameFormat& rFormat );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
