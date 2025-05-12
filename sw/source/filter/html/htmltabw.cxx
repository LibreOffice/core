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

#include <hintids.hxx>
#include <vcl/svapp.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svtools/HtmlWriter.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <fmtornt.hxx>
#include <frmfmt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swrect.hxx>
#include <cellatr.hxx>
#include <poolfmt.hxx>
#include <swtable.hxx>
#include <htmltbl.hxx>
#include "htmlnum.hxx"
#include "wrthtml.hxx"
#include <wrtswtbl.hxx>
#ifdef DBG_UTIL
#include <viewsh.hxx>
#include <viewopt.hxx>
#endif
#include <rtl/strbuf.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#define MAX_DEPTH (3)

using namespace ::com::sun::star;

namespace {

class SwHTMLWrtTable : public SwWriteTable
{
    static void Pixelize( sal_uInt16& rValue );
    void PixelizeBorders();

    /// Writes a single table cell.
    ///
    /// bCellRowSpan decides if the cell's row span should be written or not.
    void OutTableCell( SwHTMLWriter& rWrt, const SwWriteTableCell *pCell,
                       bool bOutVAlign,
                       bool bCellRowSpan ) const;

    /// Writes a single table row.
    ///
    /// rSkipRows decides if the next N rows should be skipped or written.
    void OutTableCells( SwHTMLWriter& rWrt,
                        const SwWriteTableCells& rCells,
                        const SvxBrushItem *pBrushItem,
                        sal_uInt16& rSkipRows ) const;

    virtual bool ShouldExpandSub( const SwTableBox *pBox,
                            bool bExpandedBefore, sal_uInt16 nDepth ) const override;

    static bool HasTabBackground( const SwTableLine& rLine,
                        bool bTop, bool bBottom, bool bLeft, bool bRight );
    static bool HasTabBackground( const SwTableBox& rBox,
                        bool bTop, bool bBottom, bool bLeft, bool bRight );

public:
    SwHTMLWrtTable( const SwTableLines& rLines, tools::Long nWidth, sal_uInt32 nBWidth,
                    bool bRel, sal_uInt16 nLeftSub, sal_uInt16 nRightSub,
                    sal_uInt16 nNumOfRowsToRepeat );
    explicit SwHTMLWrtTable( const SwHTMLTableLayout *pLayoutInfo );

    void Write( SwHTMLWriter& rWrt, sal_Int16 eAlign=text::HoriOrientation::NONE,
                bool bTHead=false, const SwFrameFormat *pFrameFormat=nullptr,
                const OUString *pCaption=nullptr, bool bTopCaption=false,
                sal_uInt16 nHSpace=0, sal_uInt16 nVSpace=0 ) const;
};

}

SwHTMLWrtTable::SwHTMLWrtTable( const SwTableLines& rLines, tools::Long nWidth,
                                sal_uInt32 nBWidth, bool bRel,
                                sal_uInt16 nLSub, sal_uInt16 nRSub,
                                sal_uInt16 nNumOfRowsToRepeat )
    : SwWriteTable(nullptr, rLines, nWidth, nBWidth, bRel, MAX_DEPTH, nLSub, nRSub, nNumOfRowsToRepeat)
{
    PixelizeBorders();
}

SwHTMLWrtTable::SwHTMLWrtTable( const SwHTMLTableLayout *pLayoutInfo )
    : SwWriteTable(nullptr, pLayoutInfo)
{
    // Adjust some Twip values to pixel limits
    if( m_bCollectBorderWidth )
        PixelizeBorders();
}

void SwHTMLWrtTable::Pixelize( sal_uInt16& rValue )
{
    if( rValue )
    {
        rValue = o3tl::convert(SwHTMLWriter::ToPixel(rValue), o3tl::Length::px, o3tl::Length::twip);
    }
}

void SwHTMLWrtTable::PixelizeBorders()
{
    Pixelize( m_nBorder );
    Pixelize( m_nCellSpacing );
    Pixelize( m_nCellPadding );
}

bool SwHTMLWrtTable::HasTabBackground( const SwTableBox& rBox,
                        bool bTop, bool bBottom, bool bLeft, bool bRight )
{
    OSL_ENSURE( bTop || bBottom || bLeft || bRight,
            "HasTabBackground: cannot be called" );

    bool bRet = false;
    if( rBox.GetSttNd() )
    {
        std::unique_ptr<SvxBrushItem> aBrushItem =
            rBox.GetFrameFormat()->makeBackgroundBrushItem();

        /// The table box has a background, if its background color is not "no fill"/
        /// "auto fill" or it has a background graphic.
        bRet = aBrushItem->GetColor() != COL_TRANSPARENT ||
               !aBrushItem->GetGraphicLink().isEmpty() || aBrushItem->GetGraphic();
    }
    else
    {
        const SwTableLines& rLines = rBox.GetTabLines();
        const SwTableLines::size_type nCount = rLines.size();
        bool bLeftRight = bLeft || bRight;
        for( SwTableLines::size_type i=0; !bRet && i<nCount; ++i )
        {
            bool bT = bTop && 0 == i;
            bool bB = bBottom && nCount-1 == i;
            if( bT || bB || bLeftRight )
                bRet = HasTabBackground( *rLines[i], bT, bB, bLeft, bRight);
        }
    }

    return bRet;
}

bool SwHTMLWrtTable::HasTabBackground( const SwTableLine& rLine,
                        bool bTop, bool bBottom, bool bLeft, bool bRight )
{
    OSL_ENSURE( bTop || bBottom || bLeft || bRight,
            "HasTabBackground: cannot be called" );

    std::unique_ptr<SvxBrushItem> aBrushItem = rLine.GetFrameFormat()->makeBackgroundBrushItem();
    /// The table line has a background, if its background color is not "no fill"/
    /// "auto fill" or it has a background graphic.
    bool bRet = aBrushItem->GetColor() != COL_TRANSPARENT ||
           !aBrushItem->GetGraphicLink().isEmpty() || aBrushItem->GetGraphic();

    if( !bRet )
    {
        const SwTableBoxes& rBoxes = rLine.GetTabBoxes();
        const SwTableBoxes::size_type nCount = rBoxes.size();
        bool bTopBottom = bTop || bBottom;
        for( SwTableBoxes::size_type i=0; !bRet && i<nCount; ++i )
        {
            bool bL = bLeft && 0 == i;
            bool bR = bRight && nCount-1 == i;
            if( bTopBottom || bL || bR )
                bRet = HasTabBackground( *rBoxes[i], bTop, bBottom, bL, bR );
        }
    }

    return bRet;
}

static bool lcl_TableLine_HasTabBorders( const SwTableLine* pLine, bool *pBorders );

static bool lcl_TableBox_HasTabBorders( const SwTableBox* pBox, bool *pBorders )
{
    if( *pBorders )
        return false;

    if( !pBox->GetSttNd() )
    {
        for( const auto& rpLine : pBox->GetTabLines() )
        {
            if ( lcl_TableLine_HasTabBorders( rpLine, pBorders ) )
                break;
        }
    }
    else
    {
        const SvxBoxItem& rBoxItem =
            pBox->GetFrameFormat()->GetFormatAttr( RES_BOX );

        *pBorders = rBoxItem.GetTop() || rBoxItem.GetBottom() ||
                    rBoxItem.GetLeft() || rBoxItem.GetRight();
    }

    return !*pBorders;
}

static bool lcl_TableLine_HasTabBorders( const SwTableLine* pLine, bool *pBorders )
{
    if( *pBorders )
        return false;

    for( const auto& rpBox : pLine->GetTabBoxes() )
    {
        if ( lcl_TableBox_HasTabBorders( rpBox, pBorders ) )
            break;
    }
    return !*pBorders;
}

bool SwHTMLWrtTable::ShouldExpandSub( const SwTableBox *pBox,
                                      bool bExpandedBefore,
                                      sal_uInt16 nDepth ) const
{
    bool bExpand = !pBox->GetSttNd() && nDepth>0;
    if( bExpand && bExpandedBefore )
    {
        // MIB 30.6.97: If a box was already expanded, another one is only
        // expanded when it has a border.
        bool bBorders = false;
        lcl_TableBox_HasTabBorders( pBox, &bBorders );
        if( !bBorders )
            bBorders = HasTabBackground( *pBox, true, true, true, true );
        bExpand = bBorders;
    }

    return bExpand;
}

// Write a box as single cell
void SwHTMLWrtTable::OutTableCell( SwHTMLWriter& rWrt,
                                   const SwWriteTableCell *pCell,
                                   bool bOutVAlign,
                                   bool bCellRowSpan ) const
{
    const SwTableBox *pBox = pCell->GetBox();
    sal_uInt16 nRow = pCell->GetRow();
    sal_uInt16 nCol = pCell->GetCol();
    sal_uInt16 nRowSpan = pCell->GetRowSpan();
    sal_uInt16 nColSpan = pCell->GetColSpan();

    if ( !nRowSpan )
        return;

    const SwStartNode* pSttNd = pBox->GetSttNd();
    bool bHead = false;
    if( pSttNd )
    {
        SwNodeOffset nNdPos = pSttNd->GetIndex()+1;

        // determine the type of cell (TD/TH)
        SwNode* pNd;
        while( !( pNd = rWrt.m_pDoc->GetNodes()[nNdPos])->IsEndNode() )
        {
            if( pNd->IsTextNode() )
            {
                // The only paragraphs relevant for the distinction are those
                // where the style is one of the two table related styles
                // or inherits from one of these.
                const SwFormat *pFormat = &static_cast<SwTextNode*>(pNd)->GetAnyFormatColl();
                sal_uInt16 nPoolId = pFormat->GetPoolFormatId();
                while( !pFormat->IsDefault() &&
                       RES_POOLCOLL_TABLE_HDLN!=nPoolId &&
                       RES_POOLCOLL_TABLE!=nPoolId )
                {
                    pFormat = pFormat->DerivedFrom();
                    nPoolId = pFormat->GetPoolFormatId();
                }

                if( !pFormat->IsDefault() )
                {
                    bHead = (RES_POOLCOLL_TABLE_HDLN==nPoolId);
                    break;
                }
            }
            nNdPos++;
        }
    }

    rWrt.OutNewLine();  // <TH>/<TD> in new line
    OStringBuffer sOut("<");
    OString aTag(bHead ? OOO_STRING_SVTOOLS_HTML_tableheader : OOO_STRING_SVTOOLS_HTML_tabledata);
    sOut.append(rWrt.GetNamespace() + aTag);

    // output ROW- and COLSPAN
    if (nRowSpan > 1 && bCellRowSpan)
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_rowspan
                "=\"" + OString::number(nRowSpan) + "\"");
    }
    if( nColSpan > 1 )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_colspan
                "=\"" + OString::number(nColSpan) + "\"");
    }

    tools::Long nWidth = 0;
    bool bOutWidth = true;
    sal_uInt32 nPercentWidth = SAL_MAX_UINT32;

    if( m_bLayoutExport )
    {
        if( pCell->HasPercentWidthOpt() )
        {
            nPercentWidth = pCell->GetWidthOpt();
        }
        else
        {
            nWidth = pCell->GetWidthOpt();
            if( !nWidth )
                bOutWidth = false;
        }
    }
    else
    {
        if( HasRelWidths() )
            nPercentWidth = GetPercentWidth(nCol, nColSpan);
        else
            nWidth = GetAbsWidth( nCol, nColSpan );
    }

    if (rWrt.mbReqIF)
        // ReqIF implies strict XHTML: no width for <td>.
        bOutWidth = false;

    tools::Long nHeight = pCell->GetHeight() > 0
                        ? GetAbsHeight( pCell->GetHeight(), nRow, nRowSpan )
                        : 0;
    Size aPixelSz(SwHTMLWriter::ToPixel(nWidth), SwHTMLWriter::ToPixel(nHeight));

    // output WIDTH: from layout or calculated
    if( bOutWidth )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_width "=\"");
        if( nPercentWidth != SAL_MAX_UINT32 )
        {
            sOut.append(OString::number(static_cast<sal_Int32>(nPercentWidth)) + "%");
        }
        else
        {
            sOut.append(static_cast<sal_Int32>(aPixelSz.Width()));
        }
        sOut.append("\"");
    }

    if (rWrt.mbReqIF)
    {
        // ReqIF implies strict XHTML: no height for <td>.
        nHeight = 0;
    }

    if( nHeight )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_height
                "=\"" + OString::number(aPixelSz.Height()) + "\"");
    }

    const SfxItemSet& rItemSet = pBox->GetFrameFormat()->GetAttrSet();

    // ALIGN is only outputted at the paragraphs from now on

    // output VALIGN
    if( bOutVAlign )
    {
        sal_Int16 eVertOri = pCell->GetVertOri();
        if( text::VertOrientation::TOP==eVertOri || text::VertOrientation::BOTTOM==eVertOri )
        {
            sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_valign
                    "=\"").append(text::VertOrientation::TOP==eVertOri ?
                    OOO_STRING_SVTOOLS_HTML_VA_top :
                    OOO_STRING_SVTOOLS_HTML_VA_bottom)
                .append("\"");
        }
    }

    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);

    rWrt.m_bTextAttr = false;
    rWrt.m_bOutOpts = true;
    const SvxBrushItem *pBrushItem = rItemSet.GetItemIfSet( RES_BACKGROUND, false );
    if( !pBrushItem )
        pBrushItem = pCell->GetBackground();

    if( pBrushItem )
    {
        // output background
        if (!rWrt.mbReqIF)
            // Avoid non-CSS version in the ReqIF case.
            rWrt.OutBackground( pBrushItem, false );

        if (!rWrt.m_bCfgOutStyles)
            pBrushItem = nullptr;
    }

    // tdf#132739 with rWrt.m_bCfgOutStyles of true bundle the brush item css
    // properties into the same "style" tag as the borders so there is only one
    // style tag
    rWrt.OutCSS1_TableCellBordersAndBG(*pBox->GetFrameFormat(), pBrushItem);

    sal_uInt32 nNumFormat = 0;
    double nValue = 0.0;
    bool bNumFormat = false, bValue = false;
    if( const SwTableBoxNumFormat* pItem = rItemSet.GetItemIfSet( RES_BOXATR_FORMAT, false ) )
    {
        nNumFormat = pItem->GetValue();
        bNumFormat = true;
    }
    if( const SwTableBoxValue* pItem = rItemSet.GetItemIfSet( RES_BOXATR_VALUE, false ) )
    {
        nValue = pItem->GetValue();
        bValue = true;
        if( !bNumFormat )
            nNumFormat = pBox->GetFrameFormat()->GetTableBoxNumFormat().GetValue();
    }

    if ((bNumFormat || bValue) && !rWrt.mbXHTML)
    {
        sOut.append(HTMLOutFuncs::CreateTableDataOptionsValNum(bValue, nValue,
            nNumFormat, *rWrt.m_pDoc->GetNumberFormatter()));
    }
    sOut.append('>');
    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);
    rWrt.SetLFPossible(true);

    rWrt.IncIndentLevel();  // indent the content of <TD>...</TD>

    if( pSttNd )
    {
        HTMLSaveData aSaveData( rWrt, pSttNd->GetIndex()+1,
                                pSttNd->EndOfSectionIndex() );
        rWrt.Out_SwDoc( rWrt.m_pCurrentPam.get() );
    }
    else
    {
        sal_uInt16 nTWidth;
        sal_uInt32 nBWidth;
        sal_uInt16 nLSub, nRSub;
        if( HasRelWidths() )
        {
            nTWidth = 100;
            nBWidth = GetRawWidth( nCol, nColSpan );
            nLSub = 0;
            nRSub = 0;
        }
        else
        {
            nTWidth = GetAbsWidth( nCol, nColSpan );
            nBWidth = nTWidth;
            nLSub = GetLeftSpace( nCol );
            nRSub = GetRightSpace( nCol, nColSpan );
        }

        SwHTMLWrtTable aTableWrt( pBox->GetTabLines(), nTWidth,
                                  nBWidth, HasRelWidths(), nLSub, nRSub, /*nNumOfRowsToRepeat*/0 );
        aTableWrt.Write( rWrt );
    }

    rWrt.DecIndentLevel();  // indent the content of <TD>...</TD>

    if (rWrt.IsLFPossible())
        rWrt.OutNewLine();
    aTag = bHead ? OOO_STRING_SVTOOLS_HTML_tableheader : OOO_STRING_SVTOOLS_HTML_tabledata;
    HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), Concat2View(rWrt.GetNamespace() + aTag), false);
    rWrt.SetLFPossible(true);
}

// output a line as lines
void SwHTMLWrtTable::OutTableCells( SwHTMLWriter& rWrt,
                                    const SwWriteTableCells& rCells,
                                    const SvxBrushItem *pBrushItem,
                                    sal_uInt16& rSkipRows ) const
{
    // If the line contains more the one cell and all cells have the same
    // alignment, then output the VALIGN at the line instead of the cell.
    sal_Int16 eRowVertOri = text::VertOrientation::NONE;
    if( rCells.size() > 1 )
    {
        for (SwWriteTableCells::size_type nCell = 0; nCell < rCells.size(); ++nCell)
        {
            sal_Int16 eCellVertOri = rCells[nCell]->GetVertOri();
            if( 0==nCell )
            {
                eRowVertOri = eCellVertOri;
            }
            else if( eRowVertOri != eCellVertOri )
            {
                eRowVertOri = text::VertOrientation::NONE;
                break;
            }
        }
    }

    rWrt.OutNewLine();  // <TR> in new line
    rWrt.Strm().WriteChar( '<' ).WriteOString( Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_tablerow) );
    if( pBrushItem )
    {
        if (!rWrt.mbXHTML)
        {
            rWrt.OutBackground(pBrushItem, false);
        }

        rWrt.m_bTextAttr = false;
        rWrt.m_bOutOpts = true;
        if (rWrt.m_bCfgOutStyles || rWrt.mbXHTML)
            OutCSS1_TableBGStyleOpt( rWrt, *pBrushItem );
    }

    if( text::VertOrientation::TOP==eRowVertOri || text::VertOrientation::BOTTOM==eRowVertOri )
    {
        OStringBuffer sOut;
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_valign
                "=\"").append(text::VertOrientation::TOP==eRowVertOri ? OOO_STRING_SVTOOLS_HTML_VA_top : OOO_STRING_SVTOOLS_HTML_VA_bottom)
            .append("\"");
        rWrt.Strm().WriteOString( sOut );
        sOut.setLength(0);
    }

    rWrt.Strm().WriteChar( '>' );

    rWrt.IncIndentLevel(); // indent content of <TR>...</TR>

    bool bCellRowSpan = true;
    if (!rCells.empty() && rCells[0]->GetRowSpan() > 1)
    {
        // Skip the rowspan attrs of <td> elements if they are the same for every cell of this row.
        bCellRowSpan = std::adjacent_find(rCells.begin(), rCells.end(),
                                          [](const std::unique_ptr<SwWriteTableCell>& pA,
                                             const std::unique_ptr<SwWriteTableCell>& pB)
                                          { return pA->GetRowSpan() != pB->GetRowSpan(); })
                       != rCells.end();
        if (!bCellRowSpan)
        {
            // If no rowspan is written, then skip rows which would only contain covered cells, but
            // not the current row.
            rSkipRows = rCells[0]->GetRowSpan() - 1;
        }
    }

    for (const auto &rpCell : rCells)
    {
        OutTableCell(rWrt, rpCell.get(), text::VertOrientation::NONE == eRowVertOri, bCellRowSpan);
    }

    rWrt.DecIndentLevel(); // indent content of <TR>...</TR>

    rWrt.OutNewLine();  // </TR> in new line
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_tablerow), false );
}

void SwHTMLWrtTable::Write( SwHTMLWriter& rWrt, sal_Int16 eAlign,
                            bool bTHead, const SwFrameFormat *pFrameFormat,
                            const OUString *pCaption, bool bTopCaption,
                            sal_uInt16 nHSpace, sal_uInt16 nVSpace ) const
{
    // determine value of RULES
    bool bRowsHaveBorder = false;
    bool bRowsHaveBorderOnly = true;
    assert(m_aRows.begin() != m_aRows.end());
    for (auto row = m_aRows.begin(), next = std::next(row); next < m_aRows.end(); ++row, ++next)
    {
        SwWriteTableRow* pRow = row->get();
        SwWriteTableRow* pNextRow = next->get();
        bool bBorder = ( pRow->HasBottomBorder() || pNextRow->HasTopBorder() );
        bRowsHaveBorder |= bBorder;
        bRowsHaveBorderOnly &= bBorder;

        pRow->SetBottomBorder(bBorder);
        pNextRow->SetTopBorder(bBorder);
    }

    bool bColsHaveBorder = false;
    bool bColsHaveBorderOnly = true;
    assert(m_aCols.begin() != m_aCols.end());
    for (auto col = m_aCols.begin(), next = std::next(col); next < m_aCols.end(); ++col, ++next)
    {
        SwWriteTableCol* pCol = col->get();
        SwWriteTableCol* pNextCol = next->get();
        bool bBorder = ( pCol->m_bRightBorder || pNextCol->m_bLeftBorder );
        bColsHaveBorder |= bBorder;
        bColsHaveBorderOnly &= bBorder;
        pCol->m_bRightBorder = bBorder;
        pNextCol->m_bLeftBorder = bBorder;
    }

    // close previous numbering, etc
    rWrt.ChangeParaToken( HtmlTokenId::NONE );

    if (rWrt.IsLFPossible())
        rWrt.OutNewLine();  // <TABLE> in new line
    OStringBuffer sOut("<" + rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_table);

    const SvxFrameDirection nOldDirection = rWrt.m_nDirection;
    if( pFrameFormat )
        rWrt.m_nDirection = rWrt.GetHTMLDirection( pFrameFormat->GetAttrSet() );
    if( rWrt.m_bOutFlyFrame || nOldDirection != rWrt.m_nDirection )
    {
        rWrt.Strm().WriteOString( sOut );
        sOut.setLength(0);
        rWrt.OutDirection( rWrt.m_nDirection );
    }

    // output ALIGN=
    if( text::HoriOrientation::RIGHT == eAlign )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_align
                "=\"" OOO_STRING_SVTOOLS_HTML_AL_right "\"");
    }
    else if( text::HoriOrientation::CENTER == eAlign )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_align
                "=\"" OOO_STRING_SVTOOLS_HTML_AL_center "\"");
    }
    else if( text::HoriOrientation::LEFT == eAlign )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_align
                "=\"" OOO_STRING_SVTOOLS_HTML_AL_left "\"");
    }

    // output WIDTH: from layout or calculated
    if( m_nTabWidth )
    {
        sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_width "=\"");
        if( HasRelWidths() )
            sOut.append(OString::number(static_cast<sal_Int32>(m_nTabWidth)) + "%");
        else
        {
            sal_Int32 nPixWidth =  SwHTMLWriter::ToPixel(m_nTabWidth);
            sOut.append(nPixWidth);
        }
        sOut.append("\"");
    }

    if( (nHSpace || nVSpace) && !rWrt.mbReqIF)
    {
        if (auto nPixHSpace = SwHTMLWriter::ToPixel(nHSpace))
        {
            sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_hspace
                    "=\"" + OString::number(nPixHSpace) + "\"");
        }

        if (auto nPixVSpace = SwHTMLWriter::ToPixel(nVSpace))
        {
            sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_vspace
                    "=\"" + OString::number(nPixVSpace) + "\"");
        }
    }

    // output CELLPADDING: from layout or calculated
    sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_cellpadding
            "=\"" + OString::number(SwHTMLWriter::ToPixel(m_nCellPadding)) + "\"");

    // output CELLSPACING: from layout or calculated
    sOut.append(" " OOO_STRING_SVTOOLS_HTML_O_cellspacing
            "=\"" + OString::number(SwHTMLWriter::ToPixel(m_nCellSpacing)) + "\"");

    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);

    // output background
    if( pFrameFormat )
    {
        if (!rWrt.mbXHTML)
        {
            rWrt.OutBackground(pFrameFormat->GetAttrSet(), false);
        }

        if (rWrt.m_bCfgOutStyles || rWrt.mbXHTML)
        {
            rWrt.OutCSS1_TableFrameFormatOptions( *pFrameFormat );
        }
    }

    sOut.append('>');
    rWrt.Strm().WriteOString( sOut );
    sOut.setLength(0);

    rWrt.IncIndentLevel(); // indent content of table

    // output caption
    if( pCaption && !pCaption->isEmpty() )
    {
        rWrt.OutNewLine(); // <CAPTION> in new line
        OStringBuffer sOutStr(OOO_STRING_SVTOOLS_HTML_caption);
        sOutStr.append(" " OOO_STRING_SVTOOLS_HTML_O_align "=\"")
               .append(bTopCaption ? OOO_STRING_SVTOOLS_HTML_VA_top : OOO_STRING_SVTOOLS_HTML_VA_bottom)
               .append("\"");
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + sOutStr) );
        HTMLOutFuncs::Out_String( rWrt.Strm(), *pCaption );
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_caption), false );
    }

    // output <COLGRP>/<COL>: If exporting via layout only when during import
    // some were there, otherwise always.
    bool bColGroups = (bColsHaveBorder && !bColsHaveBorderOnly);
    if( m_bColTags )
    {
        if( bColGroups )
        {
            rWrt.OutNewLine(); // <COLGRP> in new line
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_colgroup) );

            rWrt.IncIndentLevel(); // indent content of <COLGRP>
        }

        const SwWriteTableCols::size_type nCols = m_aCols.size();
        for( SwWriteTableCols::size_type nCol=0; nCol<nCols; ++nCol )
        {
            rWrt.OutNewLine(); // </COL> in new line

            const SwWriteTableCol *pColumn = m_aCols[nCol].get();

            HtmlWriter html(rWrt.Strm(), rWrt.GetNamespace());
            html.prettyPrint(false); // We add newlines ourself
            html.start(OOO_STRING_SVTOOLS_HTML_col ""_ostr);

            sal_uInt32 nWidth;
            bool bRel;
            if( m_bLayoutExport )
            {
                bRel = pColumn->HasRelWidthOpt();
                nWidth = pColumn->GetWidthOpt();
            }
            else
            {
                bRel = HasRelWidths();
                nWidth = bRel ? GetRelWidth(nCol,1) : GetAbsWidth(nCol,1);
            }

            if( bRel )
                html.attribute(OOO_STRING_SVTOOLS_HTML_O_width, Concat2View(OString::number(nWidth) + "*"));
            else
                html.attribute(OOO_STRING_SVTOOLS_HTML_O_width, OString::number(SwHTMLWriter::ToPixel(nWidth)));
            html.end();

            if( bColGroups && pColumn->m_bRightBorder && nCol<nCols-1 )
            {
                rWrt.DecIndentLevel(); // indent content of <COLGRP>
                rWrt.OutNewLine(); // </COLGRP> in new line
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_colgroup),
                                            false );
                rWrt.OutNewLine(); // <COLGRP> in new line
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_colgroup) );
                rWrt.IncIndentLevel(); // indent content of <COLGRP>
            }
        }
        if( bColGroups )
        {
            rWrt.DecIndentLevel(); // indent content of <COLGRP>

            rWrt.OutNewLine(); // </COLGRP> in new line
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_colgroup),
                                        false );
        }
    }

    // output the lines as table lines

    // Output <TBODY>?
    bool bTSections = (bRowsHaveBorder && !bRowsHaveBorderOnly);
    bool bTBody = bTSections;

    // If sections must be outputted, then a THEAD around the first line only
    // can be outputted if there is a line below the cell.
    if( bTHead &&
        (bTSections || bColGroups) &&
        m_nHeadEndRow<m_aRows.size()-1 && !m_aRows[m_nHeadEndRow]->HasBottomBorder() )
        bTHead = false;

    // Output <TBODY> only if <THEAD> is outputted.
    bTSections |= bTHead;

    if( bTSections )
    {
        rWrt.OutNewLine(); // <THEAD>/<TDATA> in new line
        OString aTag = bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody;
        HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), Concat2View(rWrt.GetNamespace() + aTag));

        rWrt.IncIndentLevel(); // indent content of <THEAD>/<TDATA>
    }

    sal_uInt16 nSkipRows = 0;
    for( SwWriteTableRows::size_type nRow = 0; nRow < m_aRows.size(); ++nRow )
    {
        const SwWriteTableRow *pRow = m_aRows[nRow].get();

        if (nSkipRows == 0)
        {
            OutTableCells(rWrt, pRow->GetCells(), pRow->GetBackground(), nSkipRows);
        }
        else
        {
            --nSkipRows;
        }
        if( ( (bTHead && nRow==m_nHeadEndRow) ||
              (bTBody && pRow->HasBottomBorder()) ) &&
            nRow < m_aRows.size()-1 )
        {
            rWrt.DecIndentLevel(); // indent content of <THEAD>/<TDATA>
            rWrt.OutNewLine(); // </THEAD>/</TDATA> in new line
            OString aTag = bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody;
            HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), Concat2View(rWrt.GetNamespace() + aTag), false);
            rWrt.OutNewLine(); // <THEAD>/<TDATA> in new line

            if( bTHead && nRow==m_nHeadEndRow )
                bTHead = false;

            aTag = bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody;
            HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), Concat2View(rWrt.GetNamespace() + aTag));
            rWrt.IncIndentLevel(); // indent content of <THEAD>/<TDATA>
        }
    }

    if( bTSections )
    {
        rWrt.DecIndentLevel(); // indent content of <THEAD>/<TDATA>

        rWrt.OutNewLine(); // </THEAD>/</TDATA> in new line
        OString aTag = bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody;
        HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), Concat2View(rWrt.GetNamespace() + aTag), false);
    }

    rWrt.DecIndentLevel(); // indent content of <TABLE>

    rWrt.OutNewLine(); // </TABLE> in new line
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_table), false );

    rWrt.m_nDirection = nOldDirection;
}

SwHTMLWriter& OutHTML_SwTableNode( SwHTMLWriter& rWrt, SwTableNode & rNode,
                           const SwFrameFormat *pFlyFrameFormat,
                           const OUString *pCaption, bool bTopCaption )
{

    SwTable& rTable = rNode.GetTable();

    rWrt.m_bOutTable = true;

    // The horizontal alignment of the frame (if exists) has priority.
    // NONE means that no horizontal alignment was outputted.
    sal_Int16 eFlyHoriOri = text::HoriOrientation::NONE;
    css::text::WrapTextMode eSurround = css::text::WrapTextMode_NONE;
    sal_uInt8 nFlyPercentWidth = 0;
    tools::Long nFlyWidth = 0;
    sal_uInt16 nFlyHSpace = 0;
    sal_uInt16 nFlyVSpace = 0;
    if( pFlyFrameFormat )
    {
        eSurround = pFlyFrameFormat->GetSurround().GetSurround();
        const SwFormatFrameSize& rFrameSize = pFlyFrameFormat->GetFrameSize();
        nFlyPercentWidth = rFrameSize.GetWidthPercent();
        nFlyWidth = rFrameSize.GetSize().Width();

        eFlyHoriOri = pFlyFrameFormat->GetHoriOrient().GetHoriOrient();
        if( text::HoriOrientation::NONE == eFlyHoriOri )
            eFlyHoriOri = text::HoriOrientation::LEFT;

        const SvxLRSpaceItem& rLRSpace = pFlyFrameFormat->GetLRSpace();
        nFlyHSpace
            = static_cast<sal_uInt16>((rLRSpace.ResolveLeft({}) + rLRSpace.ResolveRight({})) / 2);

        const SvxULSpaceItem& rULSpace = pFlyFrameFormat->GetULSpace();
        nFlyVSpace = (rULSpace.GetUpper() + rULSpace.GetLower()) / 2;
    }

    // maybe open a FORM
    bool bPreserveForm = false;
    if( !rWrt.m_bPreserveForm )
    {
        rWrt.OutForm( true, &rNode );
        bPreserveForm = rWrt.mxFormComps.is();
        rWrt.m_bPreserveForm = bPreserveForm;
    }

    SwFrameFormat *pFormat = rTable.GetFrameFormat();

    const SwFormatFrameSize& rFrameSize = pFormat->GetFrameSize();
    tools::Long nWidth = rFrameSize.GetSize().Width();
    sal_uInt8 nPercentWidth = rFrameSize.GetWidthPercent();
    sal_uInt16 nBaseWidth = o3tl::narrowing<sal_uInt16>(nWidth);

    sal_Int16 eTabHoriOri = pFormat->GetHoriOrient().GetHoriOrient();

    // text::HoriOrientation::NONE and text::HoriOrientation::FULL tables need relative widths
    sal_uInt16 nNewDefListLvl = 0;
    bool bRelWidths = false;
    bool bCheckDefList = false;
    switch( eTabHoriOri )
    {
    case text::HoriOrientation::FULL:
        // Tables with automatic alignment become tables with 100% width.
        bRelWidths = true;
        nWidth = 100;
        eTabHoriOri = text::HoriOrientation::LEFT;
        break;
    case text::HoriOrientation::NONE:
        {
            const SvxLRSpaceItem& aLRItem = pFormat->GetLRSpace();
            if (aLRItem.ResolveRight({}))
            {
                // The table width is defined on the basis of the left and
                // right margin. Therefore we try to define the actual
                // width of the table. If that's not possible we transform
                // it to a table with width 100%.
                nWidth = pFormat->FindLayoutRect(true).Width();
                if( !nWidth )
                {
                    bRelWidths = true;
                    nWidth = 100;
                }

            }
            else if( nPercentWidth  )
            {
                // Without a right border the %-width is maintained.
                nWidth = nPercentWidth;
                bRelWidths = true;
            }
            else
            {
                // Without a right margin also an absolute width is maintained.
                // We still try to define the actual width via the layout.
                tools::Long nRealWidth = pFormat->FindLayoutRect(true).Width();
                if( nRealWidth )
                    nWidth = nRealWidth;
            }
            bCheckDefList = true;
        }
        break;
    case text::HoriOrientation::LEFT_AND_WIDTH:
        eTabHoriOri = text::HoriOrientation::LEFT;
        bCheckDefList = true;
        [[fallthrough]];
    default:
        // In all other case it's possible to use directly an absolute
        // or relative width.
        if( nPercentWidth )
        {
            bRelWidths = true;
            nWidth = nPercentWidth;
        }
        break;
    }

    // In ReqIF case, do not emulate indentation with fake description list
    if( bCheckDefList && !rWrt.mbReqIF )
    {
        OSL_ENSURE( !rWrt.GetNumInfo().GetNumRule() ||
                rWrt.GetNextNumInfo(),
                "NumInfo for next paragraph is missing!" );
        const SvxLRSpaceItem& aLRItem = pFormat->GetLRSpace();
        if (aLRItem.ResolveLeft({}) > 0 && rWrt.m_nDefListMargin > 0
            && (!rWrt.GetNumInfo().GetNumRule()
                || (rWrt.GetNextNumInfo()
                    && (rWrt.GetNumInfo().GetNumRule() != rWrt.GetNextNumInfo()->GetNumRule()
                        || rWrt.GetNextNumInfo()->IsRestart(rWrt.GetNumInfo())))))
        {
            // If the paragraph before the table is not numbered or the
            // paragraph after the table starts with a new numbering or with
            // a different rule, we can maintain the indentation with a DL.
            // Otherwise we keep the indentation of the numbering.
            nNewDefListLvl = static_cast<sal_uInt16>(
                (aLRItem.ResolveLeft({}) + (rWrt.m_nDefListMargin / 2)) / rWrt.m_nDefListMargin);
        }
    }

    if( !pFlyFrameFormat && !rWrt.mbReqIF && nNewDefListLvl != rWrt.m_nDefListLvl )
        rWrt.OutAndSetDefList( nNewDefListLvl );

    // eFlyHoriOri and eTabHoriOri now only contain the values of
    // LEFT/CENTER and RIGHT!
    if( eFlyHoriOri!=text::HoriOrientation::NONE )
    {
        eTabHoriOri = eFlyHoriOri;
        // MIB 4.7.97: If the table has a relative width, then the width is
        // adjusted to the width of the frame, therefore we export its width.
        // If fixed width, the table width is relevant. Whoever puts tables with
        // relative width <100% into frames is to blame when the result looks bad.
        if( bRelWidths )
        {
            nWidth = nFlyPercentWidth ? nFlyPercentWidth : nFlyWidth;
            bRelWidths = nFlyPercentWidth > 0;
        }
    }

    sal_Int16 eDivHoriOri = text::HoriOrientation::NONE;
    switch( eTabHoriOri )
    {
    case text::HoriOrientation::LEFT:
        // If a left-aligned table has no right sided flow, then we don't need
        // an ALIGN=LEFT in the table.
        if( eSurround==css::text::WrapTextMode_NONE || eSurround==css::text::WrapTextMode_LEFT )
            eTabHoriOri = text::HoriOrientation::NONE;
        break;
    case text::HoriOrientation::RIGHT:
        // Something like that also applies to right-aligned tables,
        // here we use a <DIV ALIGN=RIGHT> instead.
        if( eSurround==css::text::WrapTextMode_NONE || eSurround==css::text::WrapTextMode_RIGHT )
        {
            eDivHoriOri = text::HoriOrientation::RIGHT;
            eTabHoriOri = text::HoriOrientation::NONE;
        }
        break;
    case text::HoriOrientation::CENTER:
        // Almost nobody understands ALIGN=CENTER, therefore we abstain
        // from it and use a <CENTER>.
        eDivHoriOri = text::HoriOrientation::CENTER;
        eTabHoriOri = text::HoriOrientation::NONE;
        break;
    default:
        ;
    }
    if( text::HoriOrientation::NONE==eTabHoriOri )
        nFlyHSpace = nFlyVSpace = 0;

    if( !pFormat->GetName().isEmpty() )
        rWrt.OutImplicitMark( pFormat->GetName().toString(), "table" );

    if( text::HoriOrientation::NONE!=eDivHoriOri )
    {
        if (rWrt.IsLFPossible())
            rWrt.OutNewLine();  // <CENTER> in new line
        if( text::HoriOrientation::CENTER==eDivHoriOri )
        {
            if (!rWrt.mbXHTML)
            {
                // Not XHTML's css center: start <center>.
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), Concat2View(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_center) );
            }
        }
        else
        {
            if (rWrt.mbReqIF)
            {
                // In ReqIF, div cannot have an 'align' attribute. For now, use 'style' only
                // for ReqIF; maybe it makes sense to use it in both cases?
                static constexpr char sOut[] = OOO_STRING_SVTOOLS_HTML_division
                    " style=\"display: flex; flex-direction: column; align-items: flex-end\"";
                HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), Concat2View(rWrt.GetNamespace() + sOut));
            }
            else
            {
                static constexpr char sOut[] = OOO_STRING_SVTOOLS_HTML_division
                    " " OOO_STRING_SVTOOLS_HTML_O_align "=\"" OOO_STRING_SVTOOLS_HTML_AL_right "\"";
                HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), Concat2View(rWrt.GetNamespace() + sOut));
            }
        }
        rWrt.IncIndentLevel();  // indent content of <CENTER>
        rWrt.SetLFPossible(true);
    }

    // If the table isn't in a frame, then you always can output a LF.
    if( text::HoriOrientation::NONE==eTabHoriOri )
        rWrt.SetLFPossible(true);

    const SwHTMLTableLayout *pLayout = rTable.GetHTMLTableLayout();

#ifdef DBG_UTIL
    {
    SwViewShell *pSh = rWrt.m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    if ( pSh && pSh->GetViewOptions()->IsTest1() )
        pLayout = nullptr;
    }
#endif

    if( pLayout && pLayout->IsExportable() )
    {
        SwHTMLWrtTable aTableWrt( pLayout );
        aTableWrt.Write( rWrt, eTabHoriOri, rTable.GetRowsToRepeat() > 0,
                         pFormat, pCaption, bTopCaption,
                         nFlyHSpace, nFlyVSpace );
    }
    else
    {
        SwHTMLWrtTable aTableWrt( rTable.GetTabLines(), nWidth,
                                  nBaseWidth, bRelWidths, 0, 0, rTable.GetRowsToRepeat() );
        aTableWrt.Write( rWrt, eTabHoriOri, rTable.GetRowsToRepeat() > 0,
                         pFormat, pCaption, bTopCaption,
                         nFlyHSpace, nFlyVSpace );
    }

    // If the table wasn't in a frame, then you always can output a LF.
    if( text::HoriOrientation::NONE==eTabHoriOri )
        rWrt.SetLFPossible(true);

    if( text::HoriOrientation::NONE!=eDivHoriOri )
    {
        rWrt.DecIndentLevel();  // indent content of <CENTER>
        rWrt.OutNewLine();      // </CENTER> in new line
        OString aTag = text::HoriOrientation::CENTER == eDivHoriOri
                           ? OOO_STRING_SVTOOLS_HTML_center
                           : OOO_STRING_SVTOOLS_HTML_division;
        if (!rWrt.mbXHTML || eDivHoriOri != text::HoriOrientation::CENTER)
        {
            // Not XHTML's css center: end <center>.
            HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), Concat2View(rWrt.GetNamespace() + aTag), false);
        }
        rWrt.SetLFPossible(true);
    }

    // move Pam behind the table
    rWrt.m_pCurrentPam->GetPoint()->Assign( *rNode.EndOfSectionNode() );

    if( bPreserveForm )
    {
        rWrt.m_bPreserveForm = false;
        rWrt.OutForm( false );
    }

    rWrt.m_bOutTable = false;

    if( rWrt.GetNextNumInfo() &&
        rWrt.GetNextNumInfo()->GetNumRule() == rWrt.GetNumInfo().GetNumRule() &&
        !rWrt.GetNextNumInfo()->IsRestart(rWrt.GetNumInfo()) )
    {
        // If the paragraph after the table is numbered with the same rule as the
        // one before, then the NumInfo of the next paragraph holds the level of
        // paragraph before the table. Therefore NumInfo must be fetched again
        // to maybe close the Num list.
        rWrt.ClearNextNumInfo();
        rWrt.FillNextNumInfo();
        OutHTML_NumberBulletListEnd( rWrt, *rWrt.GetNextNumInfo() );
    }
    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
