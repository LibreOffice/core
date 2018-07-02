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

#include <memory>
#include <hintids.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <tools/fract.hxx>
#include <wrtswtbl.hxx>
#include <swtable.hxx>
#include <frmfmt.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <frmatr.hxx>
#include <htmltbl.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

sal_Int16 SwWriteTableCell::GetVertOri() const
{
    sal_Int16 eCellVertOri = text::VertOrientation::TOP;
    if( pBox->GetSttNd() )
    {
        const SfxItemSet& rItemSet = pBox->GetFrameFormat()->GetAttrSet();
        const SfxPoolItem *pItem;
        if( SfxItemState::SET == rItemSet.GetItemState( RES_VERT_ORIENT, false, &pItem ) )
        {
            sal_Int16 eBoxVertOri =
                static_cast<const SwFormatVertOrient *>(pItem)->GetVertOrient();
            if( text::VertOrientation::CENTER==eBoxVertOri || text::VertOrientation::BOTTOM==eBoxVertOri)
                eCellVertOri = eBoxVertOri;
        }
    }

    return eCellVertOri;
}

SwWriteTableRow::SwWriteTableRow( long nPosition, bool bUseLayoutHeights )
    : pBackground(nullptr), nPos(nPosition), mbUseLayoutHeights(bUseLayoutHeights),
    nTopBorder(USHRT_MAX), nBottomBorder(USHRT_MAX), bTopBorder(true),
    bBottomBorder(true)
{
}

SwWriteTableCell *SwWriteTableRow::AddCell( const SwTableBox *pBox,
                                sal_uInt16 nRow, sal_uInt16 nCol,
                                sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                                long nHeight,
                                const SvxBrushItem *pBackgroundBrush )
{
    SwWriteTableCell *pCell =
        new SwWriteTableCell( pBox, nRow, nCol, nRowSpan, nColSpan,
                                nHeight, pBackgroundBrush );
    m_Cells.push_back(std::unique_ptr<SwWriteTableCell>(pCell));

    return pCell;
}

SwWriteTableCol::SwWriteTableCol(sal_uInt32 nPosition)
    : nPos(nPosition), nWidthOpt(0), bRelWidthOpt(false),
    bLeftBorder(true), bRightBorder(true)
{
}

sal_uInt32 SwWriteTable::GetBoxWidth( const SwTableBox *pBox )
{
    const SwFrameFormat *pFormat = pBox->GetFrameFormat();
    const SwFormatFrameSize& aFrameSize=
        pFormat->GetFormatAttr( RES_FRM_SIZE );

    return sal::static_int_cast<sal_uInt32>(aFrameSize.GetSize().Width());
}

long SwWriteTable::GetLineHeight( const SwTableLine *pLine )
{
#ifdef DBG_UTIL
    bool bOldGetLineHeightCalled = m_bGetLineHeightCalled;
    m_bGetLineHeightCalled = true;
#endif

    long nHeight = 0;
    if( m_bUseLayoutHeights )
    {
        // At first we try to get the height of the layout.
        bool bLayoutAvailable = false;
        nHeight = pLine->GetTableLineHeight(bLayoutAvailable);
        if( nHeight > 0 )
            return nHeight;

        // If no layout is found, we assume that the heights are fixed.
        // #i60390# - in some cases we still want to continue
        // to use the layout heights even if one of the rows has a height of 0
        // ('hidden' rows)
        m_bUseLayoutHeights = bLayoutAvailable;

#ifdef DBG_UTIL
        SAL_WARN_IF( !bLayoutAvailable && bOldGetLineHeightCalled, "sw", "Layout invalid?" );
#endif
    }

    const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    for( std::unique_ptr<SwTableBox> const & pBox : rBoxes )
    {
        if( pBox->GetSttNd() )
        {
            if( nHeight < ROW_DFLT_HEIGHT )
                nHeight = ROW_DFLT_HEIGHT;
        }
        else
        {
            long nTmp = 0;
            const SwTableLines &rLines = pBox->GetTabLines();
            for( size_t nLine=0; nLine<rLines.size(); nLine++ )
            {
                nTmp += GetLineHeight( rLines[nLine] );
            }
            if( nHeight < nTmp )
                nHeight = nTmp;
        }
    }

    return nHeight;
}

long SwWriteTable::GetLineHeight( const SwTableBox *pBox )
{
    const SwTableLine *pLine = pBox->GetUpper();

    if( !pLine )
        return 0;

    const SwFrameFormat *pLineFrameFormat = pLine->GetFrameFormat();
    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = pLineFrameFormat->GetAttrSet();

    long nHeight = 0;
    if( SfxItemState::SET == rItemSet.GetItemState( RES_FRM_SIZE, true, &pItem ))
        nHeight = static_cast<const SwFormatFrameSize*>(pItem)->GetHeight();

    return nHeight;
}

const SvxBrushItem *SwWriteTable::GetLineBrush( const SwTableBox *pBox,
                                                  SwWriteTableRow *pRow )
{
    const SwTableLine *pLine = pBox->GetUpper();

    while( pLine )
    {
        const SwFrameFormat *pLineFrameFormat = pLine->GetFrameFormat();
        const SfxPoolItem* pItem;
        const SfxItemSet& rItemSet = pLineFrameFormat->GetAttrSet();

        if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            if( !pLine->GetUpper() )
            {
                if( !pRow->GetBackground() )
                    pRow->SetBackground( static_cast<const SvxBrushItem *>(pItem) );
                pItem = nullptr;
            }

            return static_cast<const SvxBrushItem *>(pItem);
        }

        pBox = pLine->GetUpper();
        pLine = pBox ? pBox->GetUpper() : nullptr;
    }

    return nullptr;
}

void SwWriteTable::MergeBorders( const SvxBorderLine* pBorderLine,
                                   bool bTable )
{
    if( Color(0xffffffff) == m_nBorderColor )
    {
        if( !pBorderLine->GetColor().IsRGBEqual( COL_GRAY ) )
            m_nBorderColor = pBorderLine->GetColor();
    }

    if( !m_bCollectBorderWidth )
        return;

    const sal_uInt16 nOutWidth = pBorderLine->GetOutWidth();
    if( bTable )
    {
        if( nOutWidth && (!m_nBorder || nOutWidth < m_nBorder) )
            m_nBorder = nOutWidth;
    }
    else
    {
        if( nOutWidth && (!m_nInnerBorder || nOutWidth < m_nInnerBorder) )
            m_nInnerBorder = nOutWidth;
    }

    const sal_uInt16 nDist = pBorderLine->GetInWidth() ? pBorderLine->GetDistance()
                                                : 0;
    if( nDist && (!m_nCellSpacing || nDist < m_nCellSpacing) )
        m_nCellSpacing = nDist;
}

sal_uInt16 SwWriteTable::MergeBoxBorders( const SwTableBox *pBox,
                                        size_t const nRow, size_t const nCol,
                                        sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                                        sal_uInt16& rTopBorder,
                                        sal_uInt16 &rBottomBorder )
{
    sal_uInt16 nBorderMask = 0;

    const SwFrameFormat *pFrameFormat = pBox->GetFrameFormat();
    const SvxBoxItem& rBoxItem = pFrameFormat->GetFormatAttr( RES_BOX );

    if( rBoxItem.GetTop() )
    {
        nBorderMask |= 1;
        MergeBorders( rBoxItem.GetTop(), nRow==0 );
        rTopBorder = rBoxItem.GetTop()->GetOutWidth();
    }

    if( rBoxItem.GetLeft() )
    {
        nBorderMask |= 4;
        MergeBorders( rBoxItem.GetLeft(), nCol==0 );
    }

    if( rBoxItem.GetBottom() )
    {
        nBorderMask |= 2;
        MergeBorders( rBoxItem.GetBottom(), nRow+nRowSpan==m_aRows.size() );
        rBottomBorder = rBoxItem.GetBottom()->GetOutWidth();
    }

    if( rBoxItem.GetRight() )
    {
        nBorderMask |= 8;
        MergeBorders( rBoxItem.GetRight(), nCol+nColSpan==m_aCols.size() );
    }

    // If any distance is set, the smallest one is used. This holds for
    // the four distance of a box as well as for the distances of different
    // boxes.
    if( m_bCollectBorderWidth )
    {
        sal_uInt16 nDist = rBoxItem.GetDistance( SvxBoxItemLine::TOP );
        if( nDist && (!m_nCellPadding || nDist < m_nCellPadding) )
            m_nCellPadding = nDist;
        nDist = rBoxItem.GetDistance( SvxBoxItemLine::BOTTOM );
        if( nDist && (!m_nCellPadding || nDist < m_nCellPadding) )
            m_nCellPadding = nDist;
        nDist = rBoxItem.GetDistance( SvxBoxItemLine::LEFT );
        if( nDist && (!m_nCellPadding || nDist < m_nCellPadding) )
            m_nCellPadding = nDist;
        nDist = rBoxItem.GetDistance( SvxBoxItemLine::RIGHT );
        if( nDist && (!m_nCellPadding || nDist < m_nCellPadding) )
            m_nCellPadding = nDist;
    }

    return nBorderMask;
}

sal_uInt32  SwWriteTable::GetRawWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const
{
    sal_uInt32 nWidth = m_aCols[nCol+nColSpan-1]->GetPos();
    if( nCol > 0 )
        nWidth = nWidth - m_aCols[nCol-1]->GetPos();

    return nWidth;
}

sal_uInt16 SwWriteTable::GetLeftSpace( sal_uInt16 nCol ) const
{
    sal_uInt16 nSpace = m_nCellPadding + m_nCellSpacing;

    // Additional subtract the line thickness in the first column.
    if( nCol==0 )
    {
        nSpace = nSpace + m_nLeftSub;

        const SwWriteTableCol *pCol = m_aCols[nCol];
        if( pCol->HasLeftBorder() )
            nSpace = nSpace + m_nBorder;
    }

    return nSpace;
}

sal_uInt16
SwWriteTable::GetRightSpace(size_t const nCol, sal_uInt16 nColSpan) const
{
    sal_uInt16 nSpace = m_nCellPadding;

    // Additional subtract in the last column CELLSPACING and
    // line thickness once again.
    if( nCol+nColSpan==m_aCols.size() )
    {
        nSpace += (m_nCellSpacing + m_nRightSub);

        const SwWriteTableCol *pCol = m_aCols[nCol+nColSpan-1];
        if( pCol->HasRightBorder() )
            nSpace = nSpace + m_nBorder;
    }

    return nSpace;
}

sal_uInt16 SwWriteTable::GetAbsWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const
{
    sal_uInt32 nWidth = GetRawWidth( nCol, nColSpan );
    if( m_nBaseWidth != m_nTabWidth )
    {
        nWidth *= m_nTabWidth;
        nWidth /= m_nBaseWidth;
    }

    nWidth -= GetLeftSpace( nCol ) + GetRightSpace( nCol, nColSpan );

    OSL_ENSURE( nWidth > 0, "Column Width <= 0. OK?" );
    return nWidth > 0 ? static_cast<sal_uInt16>(nWidth) : 0;
}

sal_uInt16 SwWriteTable::GetRelWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const
{
    long nWidth = GetRawWidth( nCol, nColSpan );

    return static_cast<sal_uInt16>(static_cast<long>(Fraction( nWidth*256 + GetBaseWidth()/2,
                                   GetBaseWidth() )));
}

sal_uInt16 SwWriteTable::GetPrcWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const
{
    long nWidth = GetRawWidth( nCol, nColSpan );

    // Looks funny, but is nothing more than
    // [(100 * nWidth) + .5] without rounding errors
    return static_cast<sal_uInt16>(static_cast<long>(Fraction( nWidth*100 + GetBaseWidth()/2,
                                   GetBaseWidth() )));
}

long SwWriteTable::GetAbsHeight(long nRawHeight, size_t const nRow,
                                   sal_uInt16 nRowSpan ) const
{
    nRawHeight -= (2*m_nCellPadding + m_nCellSpacing);

    // Additional subtract in the first column CELLSPACING and
    // line thickness once again.
    const SwWriteTableRow *pRow = nullptr;
    if( nRow==0 )
    {
        nRawHeight -= m_nCellSpacing;
        pRow = m_aRows[nRow];
        if( pRow->HasTopBorder() )
            nRawHeight -= m_nBorder;
    }

    // Subtract the line thickness in the last column
    if( nRow+nRowSpan==m_aRows.size() )
    {
        if( !pRow || nRowSpan > 1 )
            pRow = m_aRows[nRow+nRowSpan-1];
        if( pRow->HasBottomBorder() )
            nRawHeight -= m_nBorder;
    }

    OSL_ENSURE( nRawHeight > 0, "Row Height <= 0. OK?" );
    return std::max<long>(nRawHeight, 0);
}

bool SwWriteTable::ShouldExpandSub(const SwTableBox *pBox, bool /*bExpandedBefore*/,
    sal_uInt16 nDepth) const
{
    return !pBox->GetSttNd() && nDepth > 0;
}

// FIXME: the degree of coupling between this method and
// FillTableRowsCols which is called immediately afterwards
// is -extremely- unpleasant and potentially problematic.

void SwWriteTable::CollectTableRowsCols( long nStartRPos,
                                           sal_uInt32 nStartCPos,
                                           long nParentLineHeight,
                                           sal_uInt32 nParentLineWidth,
                                           const SwTableLines& rLines,
                                           sal_uInt16 nDepth )
{
    bool bSubExpanded = false;
    const SwTableLines::size_type nLines = rLines.size();

#if OSL_DEBUG_LEVEL > 0
    sal_uInt32 nEndCPos = 0;
#endif

    long nRPos = nStartRPos;
    for( SwTableLines::size_type nLine = 0; nLine < nLines; ++nLine )
    {
        /*const*/ SwTableLine *pLine = rLines[nLine];

        long nOldRPos = nRPos;

        if( nLine < nLines-1 || nParentLineHeight==0  )
        {
            long nLineHeight = GetLineHeight( pLine );
            nRPos += nLineHeight;
            if( nParentLineHeight && nStartRPos + nParentLineHeight <= nRPos )
            {
                /* If you have corrupt line height information, e.g. breaking rows in complex table
                layout, you may run into this robust code.
                It's not allowed that subrows leaves their parentrow. If this would happen the line
                height of subrow is reduced to a part of the remaining height */
                OSL_FAIL( "Corrupt line height I" );
                nRPos -= nLineHeight;
                nLineHeight = nStartRPos + nParentLineHeight - nRPos; // remaining parent height
                nLineHeight /= nLines - nLine; // divided through the number of remaining sub rows
                nRPos += nLineHeight;
            }
            SwWriteTableRow *pRow = new SwWriteTableRow( nRPos, m_bUseLayoutHeights);
            if( !m_aRows.insert( pRow ).second )
                delete pRow;
        }
        else
        {
#if OSL_DEBUG_LEVEL > 0
            long nCheckPos = nRPos + GetLineHeight( pLine );
#endif
            nRPos = nStartRPos + nParentLineHeight;
#if OSL_DEBUG_LEVEL > 0
            SwWriteTableRow aSrchRow( nRPos, m_bUseLayoutHeights );
            OSL_ENSURE( m_aRows.find( &aSrchRow ) != m_aRows.end(), "Parent-Row not found" );
            SwWriteTableRow aRowCheckPos(nCheckPos,m_bUseLayoutHeights);
            SwWriteTableRow aRowRPos(nRPos,m_bUseLayoutHeights);
            OSL_ENSURE( !m_bUseLayoutHeights ||
                    aRowCheckPos == aRowRPos,
                    "Height of the rows does not correspond with the parent" );
#endif
        }

        // If necessary insert a column for all boxes of the row
        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        const SwTableBoxes::size_type nBoxes = rBoxes.size();

        sal_uInt32 nCPos = nStartCPos;
        for( SwTableBoxes::size_type nBox=0; nBox<nBoxes; ++nBox )
        {
            const SwTableBox *pBox = rBoxes[nBox].get();

            sal_uInt32 nOldCPos = nCPos;

            if( nBox < nBoxes-1 || (nParentLineWidth==0 && nLine==0)  )
            {
                nCPos = nCPos + GetBoxWidth( pBox );
                SwWriteTableCol *pCol = new SwWriteTableCol( nCPos );

                if( !m_aCols.insert( pCol ).second )
                    delete pCol;

                if( nBox==nBoxes-1 )
                {
                    OSL_ENSURE( nLine==0 && nParentLineWidth==0,
                            "Now the parent width will be flattened!" );
                    nParentLineWidth = nCPos-nStartCPos;
                }
            }
            else
            {
#if OSL_DEBUG_LEVEL > 0
                sal_uInt32 nCheckPos = nCPos + GetBoxWidth( pBox );
                if( !nEndCPos )
                {
                    nEndCPos = nCheckPos;
                }
                else
                {
                    OSL_ENSURE( SwWriteTableCol(nCheckPos) ==
                                                SwWriteTableCol(nEndCPos),
                    "Cell includes rows of different widths" );
                }
#endif
                nCPos = nStartCPos + nParentLineWidth;

#if OSL_DEBUG_LEVEL > 0
                SwWriteTableCol aSrchCol( nCPos );
                OSL_ENSURE( m_aCols.find( &aSrchCol ) != m_aCols.end(),
                        "Parent-Cell not found" );
                OSL_ENSURE( SwWriteTableCol(nCheckPos) ==
                                            SwWriteTableCol(nCPos),
                        "Width of the cells does not correspond with the parent" );
#endif
            }

            if( ShouldExpandSub( pBox, bSubExpanded, nDepth ) )
            {
                CollectTableRowsCols( nOldRPos, nOldCPos,
                                        nRPos - nOldRPos,
                                        nCPos - nOldCPos,
                                        pBox->GetTabLines(),
                                        nDepth-1 );
                bSubExpanded = true;
            }
        }
    }
}

void SwWriteTable::FillTableRowsCols( long nStartRPos, sal_uInt16 nStartRow,
                                        sal_uInt32 nStartCPos, sal_uInt16 nStartCol,
                                        long nParentLineHeight,
                                        sal_uInt32 nParentLineWidth,
                                        const SwTableLines& rLines,
                                        const SvxBrushItem* pParentBrush,
                                        sal_uInt16 nDepth,
                                        sal_uInt16 nNumOfHeaderRows )
{
    const SwTableLines::size_type nLines = rLines.size();
    bool bSubExpanded = false;

    // Specifying the border
    long nRPos = nStartRPos;
    sal_uInt16 nRow = nStartRow;

    for( SwTableLines::size_type nLine = 0; nLine < nLines; ++nLine )
    {
        const SwTableLine *pLine = rLines[nLine];

        // Determine the position of the last covered row
        long nOldRPos = nRPos;
        if( nLine < nLines-1 || nParentLineHeight==0 )
        {
            long nLineHeight = GetLineHeight( pLine );
            nRPos += nLineHeight;
            if( nParentLineHeight && nStartRPos + nParentLineHeight <= nRPos )
            {
                /* See comment in CollectTableRowCols */
                OSL_FAIL( "Corrupt line height II" );
                nRPos -= nLineHeight;
                nLineHeight = nStartRPos + nParentLineHeight - nRPos; // remaining parent height
                nLineHeight /= nLines - nLine; // divided through the number of remaining sub rows
                nRPos += nLineHeight;
            }
        }
        else
            nRPos = nStartRPos + nParentLineHeight;

        // And their index
        sal_uInt16 nOldRow = nRow;
        SwWriteTableRow aSrchRow( nRPos,m_bUseLayoutHeights );
        SwWriteTableRows::const_iterator it2 = m_aRows.find( &aSrchRow );

        // coupled methods out of sync ...
        assert( it2 != m_aRows.end() );
        nRow = it2 - m_aRows.begin();

        OSL_ENSURE( nOldRow <= nRow, "Don't look back!" );
        if( nOldRow > nRow )
        {
            nOldRow = nRow;
            if( nOldRow )
                --nOldRow;
        }

        SwWriteTableRow *pRow = m_aRows[nOldRow];
        SwWriteTableRow *pEndRow = m_aRows[nRow];
        if( nLine+1==nNumOfHeaderRows && nParentLineHeight==0 )
            m_nHeadEndRow = nRow;

        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();

        const SwFrameFormat *pLineFrameFormat = pLine->GetFrameFormat();
        const SfxPoolItem* pItem;
        const SfxItemSet& rItemSet = pLineFrameFormat->GetAttrSet();

        long nHeight = 0;
        if( SfxItemState::SET == rItemSet.GetItemState( RES_FRM_SIZE, true, &pItem ))
            nHeight = static_cast<const SwFormatFrameSize*>(pItem)->GetHeight();

        const SvxBrushItem *pBrushItem, *pLineBrush = pParentBrush;
        if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            pLineBrush = static_cast<const SvxBrushItem *>(pItem);

            // If the row spans the entire table, we can
            // print out the background to the row. Otherwise
            // we have to print out into the cell.
            bool bOutAtRow = !nParentLineWidth;
            if( !bOutAtRow && nStartCPos==0 )
            {
                SwWriteTableCol aCol( nParentLineWidth );
                bOutAtRow = m_aCols.find( &aCol ) == (m_aCols.end() - 1);
            }
            if( bOutAtRow )
            {
                pRow->SetBackground( pLineBrush );
                pBrushItem = nullptr;
            }
            else
                pBrushItem = pLineBrush;
        }
        else
        {
            pRow->SetBackground( pLineBrush );
            pBrushItem = nullptr;
        }

        const SwTableBoxes::size_type nBoxes = rBoxes.size();
        sal_uInt32 nCPos = nStartCPos;
        sal_uInt16 nCol = nStartCol;

        for( SwTableBoxes::size_type nBox=0; nBox<nBoxes; ++nBox )
        {
            const SwTableBox *pBox = rBoxes[nBox].get();

            // Determine the position of the last covered column
            sal_uInt32 nOldCPos = nCPos;
            if( nBox < nBoxes-1 || (nParentLineWidth==0 && nLine==0) )
            {
                nCPos = nCPos + GetBoxWidth( pBox );
                if( nBox==nBoxes-1 )
                    nParentLineWidth = nCPos - nStartCPos;
            }
            else
                nCPos = nStartCPos + nParentLineWidth;

            // And their index
            sal_uInt16 nOldCol = nCol;
            SwWriteTableCol aSrchCol( nCPos );
            SwWriteTableCols::const_iterator it = m_aCols.find( &aSrchCol );
            OSL_ENSURE( it != m_aCols.end(), "missing column" );
            if(it != m_aCols.end())
            {
                // if find fails for some nCPos value then it used to set nCol value with size of aCols.
                nCol = it - m_aCols.begin();
            }

            if( !ShouldExpandSub( pBox, bSubExpanded, nDepth ) )
            {
                sal_uInt16 nRowSpan = nRow - nOldRow + 1;

                // The new table model may have true row span attributes
                const long nAttrRowSpan = pBox->getRowSpan();
                if ( 1 < nAttrRowSpan )
                    nRowSpan = static_cast<sal_uInt16>(nAttrRowSpan);
                else if ( nAttrRowSpan < 1 )
                    nRowSpan = 0;

                sal_uInt16 nColSpan = nCol - nOldCol + 1;
                pRow->AddCell( pBox, nOldRow, nOldCol,
                               nRowSpan, nColSpan, nHeight,
                               pBrushItem );
                nHeight = 0; // The height requires only to be written once

                if( pBox->GetSttNd() )
                {
                    sal_uInt16 nTopBorder = USHRT_MAX, nBottomBorder = USHRT_MAX;
                    sal_uInt16 nBorderMask = MergeBoxBorders(pBox, nOldRow, nOldCol,
                        nRowSpan, nColSpan, nTopBorder, nBottomBorder);

                    // #i30094# add a sanity check here to ensure that
                    // we don't access an invalid aCols[] as &nCol
                    // above can be changed.
                    if (!(nBorderMask & 4) && nOldCol < m_aCols.size())
                    {
                        SwWriteTableCol *pCol = m_aCols[nOldCol];
                        OSL_ENSURE(pCol, "No TableCol found, panic!");
                        if (pCol)
                            pCol->bLeftBorder = false;
                    }

                    if (!(nBorderMask & 8))
                    {
                        SwWriteTableCol *pCol = m_aCols[nCol];
                        OSL_ENSURE(pCol, "No TableCol found, panic!");
                        if (pCol)
                            pCol->bRightBorder = false;
                    }

                    if (!(nBorderMask & 1))
                        pRow->bTopBorder = false;
                    else if (!pRow->nTopBorder || nTopBorder < pRow->nTopBorder)
                        pRow->nTopBorder = nTopBorder;

                    if (!(nBorderMask & 2))
                        pEndRow->bBottomBorder = false;
                    else if (
                                !pEndRow->nBottomBorder ||
                                nBottomBorder < pEndRow->nBottomBorder
                            )
                    {
                        pEndRow->nBottomBorder = nBottomBorder;
                    }
                }
            }
            else
            {
                FillTableRowsCols( nOldRPos, nOldRow, nOldCPos, nOldCol,
                                    nRPos-nOldRPos, nCPos-nOldCPos,
                                    pBox->GetTabLines(),
                                    pLineBrush, nDepth-1,
                                    nNumOfHeaderRows );
                bSubExpanded = true;
            }

            nCol++; // The next cell begins in the next column
        }

        nRow++;
    }
}

SwWriteTable::SwWriteTable(const SwTable* pTable, const SwTableLines& rLines, long nWidth,
    sal_uInt32 nBWidth, bool bRel, sal_uInt16 nMaxDepth, sal_uInt16 nLSub, sal_uInt16 nRSub, sal_uInt32 nNumOfRowsToRepeat)
    : m_pTable(pTable), m_nBorderColor(sal_uInt32(-1)), m_nCellSpacing(0), m_nCellPadding(0), m_nBorder(0),
    m_nInnerBorder(0), m_nBaseWidth(nBWidth), m_nHeadEndRow(USHRT_MAX),
     m_nLeftSub(nLSub), m_nRightSub(nRSub), m_nTabWidth(nWidth), m_bRelWidths(bRel),
    m_bUseLayoutHeights(true),
#ifdef DBG_UTIL
    m_bGetLineHeightCalled(false),
#endif
    m_bColTags(true), m_bLayoutExport(false),
    m_bCollectBorderWidth(true)
{
    sal_uInt32 nParentWidth = m_nBaseWidth + m_nLeftSub + m_nRightSub;

    // First the table structure set. Behind the table is in each
    // case the end of a column
    SwWriteTableCol *pCol = new SwWriteTableCol( nParentWidth );
    m_aCols.insert( pCol );
    m_bUseLayoutHeights = true;
    CollectTableRowsCols( 0, 0, 0, nParentWidth, rLines, nMaxDepth - 1 );

    // FIXME: awfully GetLineHeight writes to this in its first call
    // and proceeds to return a rather odd number fdo#62336, we have to
    // behave identically since the code in FillTableRowsCols duplicates
    // and is highly coupled to CollectTableRowsCols - sadly.
    m_bUseLayoutHeights = true;
    // And now fill with life
    FillTableRowsCols( 0, 0, 0, 0, 0, nParentWidth, rLines, nullptr, nMaxDepth - 1, static_cast< sal_uInt16 >(nNumOfRowsToRepeat) );

    // Adjust some Twip values to pixel boundaries
    if( !m_nBorder )
        m_nBorder = m_nInnerBorder;
}

SwWriteTable::SwWriteTable(const SwTable* pTable, const SwHTMLTableLayout *pLayoutInfo)
    : m_pTable(pTable), m_nBorderColor(sal_uInt32(-1)), m_nCellSpacing(0), m_nCellPadding(0), m_nBorder(0),
    m_nInnerBorder(0), m_nBaseWidth(pLayoutInfo->GetWidthOption()), m_nHeadEndRow(0),
    m_nLeftSub(0), m_nRightSub(0), m_nTabWidth(pLayoutInfo->GetWidthOption()),
    m_bRelWidths(pLayoutInfo->HasPrcWidthOption()), m_bUseLayoutHeights(false),
#ifdef DBG_UTIL
    m_bGetLineHeightCalled(false),
#endif
    m_bColTags(pLayoutInfo->HasColTags()), m_bLayoutExport(true),
    m_bCollectBorderWidth(pLayoutInfo->HaveBordersChanged())
{
    if( !m_bCollectBorderWidth )
    {
        m_nBorder = pLayoutInfo->GetBorder();
        m_nCellPadding = pLayoutInfo->GetCellPadding();
        m_nCellSpacing = pLayoutInfo->GetCellSpacing();
    }

    const sal_uInt16 nCols = pLayoutInfo->GetColCount();
    const sal_uInt16 nRows = pLayoutInfo->GetRowCount();

    // First set the table structure.
    for( sal_uInt16 nCol=0; nCol<nCols; ++nCol )
    {
        SwWriteTableCol *pCol =
            new SwWriteTableCol( (nCol+1)*COL_DFLT_WIDTH );

        if( m_bColTags )
        {
            const SwHTMLTableLayoutColumn *pLayoutCol =
                pLayoutInfo->GetColumn( nCol );
            pCol->SetWidthOpt( pLayoutCol->GetWidthOption(),
                               pLayoutCol->IsRelWidthOption() );
        }

        m_aCols.insert( pCol );
    }

    for( sal_uInt16 nRow=0; nRow<nRows; ++nRow )
    {
        SwWriteTableRow *pRow =
            new SwWriteTableRow( (nRow+1)*ROW_DFLT_HEIGHT, m_bUseLayoutHeights );
        pRow->nTopBorder = 0;
        pRow->nBottomBorder = 0;
        m_aRows.insert( pRow );
    }

    // And now fill with life
    for( sal_uInt16 nRow=0; nRow<nRows; ++nRow )
    {
        SwWriteTableRow *pRow = m_aRows[nRow];

        bool bHeightExported = false;
        for( sal_uInt16 nCol=0; nCol<nCols; nCol++ )
        {
            const SwHTMLTableLayoutCell *pLayoutCell =
                pLayoutInfo->GetCell( nRow, nCol );

            const SwHTMLTableLayoutCnts *pLayoutCnts =
                pLayoutCell->GetContents().get();

            // The cell begins actually a row above or further forward?
            if( ( nRow>0 && pLayoutCnts == pLayoutInfo->GetCell(nRow-1,nCol)
                                                      ->GetContents().get() ) ||
                ( nCol>0 && pLayoutCnts == pLayoutInfo->GetCell(nRow,nCol-1)
                                                      ->GetContents().get() ) )
            {
                continue;
            }

            const sal_uInt16 nRowSpan = pLayoutCell->GetRowSpan();
            const sal_uInt16 nColSpan = pLayoutCell->GetColSpan();
            const SwTableBox *pBox = pLayoutCnts->GetTableBox();
            OSL_ENSURE( pBox,
                    "Table in Table can not be exported over layout" );

            long nHeight = bHeightExported ? 0 : GetLineHeight( pBox );
            const SvxBrushItem *pBrushItem = GetLineBrush( pBox, pRow );

            SwWriteTableCell *pCell =
                pRow->AddCell( pBox, nRow, nCol, nRowSpan, nColSpan,
                               nHeight, pBrushItem );
            pCell->SetWidthOpt( pLayoutCell->GetWidthOption(),
                                pLayoutCell->IsPrcWidthOption() );

            sal_uInt16 nTopBorder = USHRT_MAX, nBottomBorder = USHRT_MAX;
            sal_uInt16 nBorderMask =
            MergeBoxBorders( pBox, nRow, nCol, nRowSpan, nColSpan,
                                nTopBorder, nBottomBorder );

            SwWriteTableCol *pCol = m_aCols[nCol];
            if( !(nBorderMask & 4) )
                pCol->bLeftBorder = false;

            pCol = m_aCols[nCol+nColSpan-1];
            if( !(nBorderMask & 8) )
                pCol->bRightBorder = false;

            if( !(nBorderMask & 1) )
                pRow->bTopBorder = false;

            SwWriteTableRow *pEndRow = m_aRows[nRow+nRowSpan-1];
            if( !(nBorderMask & 2) )
                pEndRow->bBottomBorder = false;

            // The height requires only to be written once
            if( nHeight )
                bHeightExported = true;
        }
    }

    // Adjust some Twip values to pixel boundaries
    if( m_bCollectBorderWidth && !m_nBorder )
        m_nBorder = m_nInnerBorder;
}

SwWriteTable::~SwWriteTable()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
