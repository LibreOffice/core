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
#include <vcl/wrkwin.hxx>
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

class SwHTMLWrtTable : public SwWriteTable
{
    static void Pixelize( sal_uInt16& rValue );
    void PixelizeBorders();

    void OutTableCell( SwHTMLWriter& rWrt, const SwWriteTableCell *pCell,
                       bool bOutVAlign ) const;

    void OutTableCells( SwHTMLWriter& rWrt,
                        const SwWriteTableCells& rCells,
                        const SvxBrushItem *pBrushItem ) const;

    virtual bool ShouldExpandSub( const SwTableBox *pBox,
                            bool bExpandedBefore, sal_uInt16 nDepth ) const override;

    static bool HasTabBackground( const SwTableLine& rLine,
                        bool bTop, bool bBottom, bool bLeft, bool bRight );
    static bool HasTabBackground( const SwTableBox& rBox,
                        bool bTop, bool bBottom, bool bLeft, bool bRight );

public:
    SwHTMLWrtTable( const SwTableLines& rLines, long nWidth, sal_uInt32 nBWidth,
                    bool bRel, sal_uInt16 nLeftSub, sal_uInt16 nRightSub,
                    sal_uInt16 nNumOfRowsToRepeat );
    explicit SwHTMLWrtTable( const SwHTMLTableLayout *pLayoutInfo );

    void Write( SwHTMLWriter& rWrt, sal_Int16 eAlign=text::HoriOrientation::NONE,
                bool bTHead=false, const SwFrameFormat *pFrameFormat=nullptr,
                const OUString *pCaption=nullptr, bool bTopCaption=false,
                sal_uInt16 nHSpace=0, sal_uInt16 nVSpace=0 ) const;
};

SwHTMLWrtTable::SwHTMLWrtTable( const SwTableLines& rLines, long nWidth,
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
    if( rValue && Application::GetDefaultDevice() )
    {
        Size aSz( rValue, 0 );
        aSz = Application::GetDefaultDevice()->LogicToPixel( aSz, MapMode(MapUnit::MapTwip) );
        if( !aSz.Width() )
            aSz.setWidth( 1 );
        aSz = Application::GetDefaultDevice()->PixelToLogic( aSz, MapMode(MapUnit::MapTwip) );
        rValue = static_cast<sal_uInt16>(aSz.Width());
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
        SvxBrushItem aBrushItem =
            rBox.GetFrameFormat()->makeBackgroundBrushItem();

        /// The table box has a background, if its background color is not "no fill"/
        /// "auto fill" or it has a background graphic.
        bRet = aBrushItem.GetColor() != COL_TRANSPARENT ||
               !aBrushItem.GetGraphicLink().isEmpty() || aBrushItem.GetGraphic();
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

    SvxBrushItem aBrushItem = rLine.GetFrameFormat()->makeBackgroundBrushItem();
    /// The table line has a background, if its background color is not "no fill"/
    /// "auto fill" or it has a background graphic.
    bool bRet = aBrushItem.GetColor() != COL_TRANSPARENT ||
           !aBrushItem.GetGraphicLink().isEmpty() || aBrushItem.GetGraphic();

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
                                   bool bOutVAlign ) const
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
        sal_uLong nNdPos = pSttNd->GetIndex()+1;

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
    OStringBuffer sOut;
    sOut.append('<');
    OString aTag(bHead ? OOO_STRING_SVTOOLS_HTML_tableheader : OOO_STRING_SVTOOLS_HTML_tabledata);
    sOut.append(rWrt.GetNamespace() + aTag);

    // output ROW- and COLSPAN
    if( nRowSpan>1 )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_rowspan).
            append("=\"").append(static_cast<sal_Int32>(nRowSpan)).append("\"");
    }
    if( nColSpan > 1 )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_colspan).
            append("=\"").append(static_cast<sal_Int32>(nColSpan)).append("\"");
    }

    long nWidth = 0;
    bool bOutWidth = true;
    sal_uInt32 nPrcWidth = SAL_MAX_UINT32;

    if( m_bLayoutExport )
    {
        if( pCell->HasPrcWidthOpt() )
        {
            nPrcWidth = pCell->GetWidthOpt();
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
            nPrcWidth = GetPrcWidth(nCol, nColSpan);
        else
            nWidth = GetAbsWidth( nCol, nColSpan );
    }

    if (rWrt.mbReqIF)
        // ReqIF implies strict XHTML: no width for <td>.
        bOutWidth = false;

    long nHeight = pCell->GetHeight() > 0
                        ? GetAbsHeight( pCell->GetHeight(), nRow, nRowSpan )
                        : 0;
    Size aPixelSz( nWidth, nHeight );

    // output WIDTH (Argh: only for Netscape)
    if( (aPixelSz.Width() || aPixelSz.Height()) && Application::GetDefaultDevice() )
    {
        Size aOldSz( aPixelSz );
        aPixelSz = Application::GetDefaultDevice()->LogicToPixel( aPixelSz,
                                                        MapMode(MapUnit::MapTwip) );
        if( aOldSz.Width() && !aPixelSz.Width() )
            aPixelSz.setWidth( 1 );
        if( aOldSz.Height() && !aPixelSz.Height() )
            aPixelSz.setHeight( 1 );
    }

    // output WIDTH: from layout or calculated
    if( bOutWidth )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width).
            append("=\"");
        if( nPrcWidth != SAL_MAX_UINT32 )
        {
            sOut.append(static_cast<sal_Int32>(nPrcWidth)).append('%');
        }
        else
        {
            sOut.append(static_cast<sal_Int32>(aPixelSz.Width()));
        }
        sOut.append("\"");
    }

    if( nHeight )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_height)
            .append("=\"").append(static_cast<sal_Int32>(aPixelSz.Height())).append("\"");
    }

    const SfxItemSet& rItemSet = pBox->GetFrameFormat()->GetAttrSet();
    const SfxPoolItem *pItem;

    // ALIGN is only outputted at the paragraphs from now on

    // output VALIGN
    if( bOutVAlign )
    {
        sal_Int16 eVertOri = pCell->GetVertOri();
        if( text::VertOrientation::TOP==eVertOri || text::VertOrientation::BOTTOM==eVertOri )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_valign)
                .append("=\"").append(text::VertOrientation::TOP==eVertOri ?
                    OOO_STRING_SVTOOLS_HTML_VA_top :
                    OOO_STRING_SVTOOLS_HTML_VA_bottom)
                .append("\"");
        }
    }

    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    rWrt.m_bTextAttr = false;
    rWrt.m_bOutOpts = true;
    const SvxBrushItem *pBrushItem = nullptr;
    if( SfxItemState::SET==rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
    {
        pBrushItem = static_cast<const SvxBrushItem *>(pItem);
    }
    if( !pBrushItem )
        pBrushItem = pCell->GetBackground();

    if( pBrushItem )
    {
        // output background
        if (!rWrt.mbReqIF)
            // Avoid non-CSS version in the ReqIF case.
            rWrt.OutBackground( pBrushItem, false );

        if( rWrt.m_bCfgOutStyles )
            OutCSS1_TableBGStyleOpt( rWrt, *pBrushItem );
    }

    rWrt.OutCSS1_TableCellBorderHack(*pBox->GetFrameFormat());

    sal_uInt32 nNumFormat = 0;
    double nValue = 0.0;
    bool bNumFormat = false, bValue = false;
    if( SfxItemState::SET==rItemSet.GetItemState( RES_BOXATR_FORMAT, false, &pItem ) )
    {
        nNumFormat = static_cast<const SwTableBoxNumFormat *>(pItem)->GetValue();
        bNumFormat = true;
    }
    if( SfxItemState::SET==rItemSet.GetItemState( RES_BOXATR_VALUE, false, &pItem ) )
    {
        nValue = static_cast<const SwTableBoxValue *>(pItem)->GetValue();
        bValue = true;
        if( !bNumFormat )
            nNumFormat = pBox->GetFrameFormat()->GetTableBoxNumFormat().GetValue();
    }

    if( bNumFormat || bValue )
    {
        sOut.append(HTMLOutFuncs::CreateTableDataOptionsValNum(bValue, nValue,
            nNumFormat, *rWrt.m_pDoc->GetNumberFormatter(), rWrt.m_eDestEnc,
            &rWrt.m_aNonConvertableCharacters));
    }
    sOut.append('>');
    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    rWrt.m_bLFPossible = true;

    rWrt.IncIndentLevel();  // indent the content of <TD>...</TD>

    if( pSttNd )
    {
        HTMLSaveData aSaveData( rWrt, pSttNd->GetIndex()+1,
                                pSttNd->EndOfSectionIndex() );
        rWrt.Out_SwDoc( rWrt.m_pCurrentPam );
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

    if( rWrt.m_bLFPossible )
        rWrt.OutNewLine();
    aTag = bHead ? OOO_STRING_SVTOOLS_HTML_tableheader : OOO_STRING_SVTOOLS_HTML_tabledata;
    HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), rWrt.GetNamespace() + aTag, false);
    rWrt.m_bLFPossible = true;
}

// output a line as lines
void SwHTMLWrtTable::OutTableCells( SwHTMLWriter& rWrt,
                                    const SwWriteTableCells& rCells,
                                    const SvxBrushItem *pBrushItem ) const
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
    rWrt.Strm().WriteChar( '<' ).WriteOString( rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_tablerow );
    if( pBrushItem )
    {
        rWrt.OutBackground( pBrushItem, false );

        rWrt.m_bTextAttr = false;
        rWrt.m_bOutOpts = true;
        if( rWrt.m_bCfgOutStyles )
            OutCSS1_TableBGStyleOpt( rWrt, *pBrushItem );
    }

    if( text::VertOrientation::TOP==eRowVertOri || text::VertOrientation::BOTTOM==eRowVertOri )
    {
        OStringBuffer sOut;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_valign)
            .append("=\"").append(text::VertOrientation::TOP==eRowVertOri ? OOO_STRING_SVTOOLS_HTML_VA_top : OOO_STRING_SVTOOLS_HTML_VA_bottom)
            .append("\"");
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    }

    rWrt.Strm().WriteChar( '>' );

    rWrt.IncIndentLevel(); // indent content of <TR>...</TR>

    for (const auto &rpCell : rCells)
    {
        OutTableCell(rWrt, rpCell.get(), text::VertOrientation::NONE == eRowVertOri);
    }

    rWrt.DecIndentLevel(); // indent content of <TR>...</TR>

    rWrt.OutNewLine();  // </TR> in new line
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_tablerow, false );
}

void SwHTMLWrtTable::Write( SwHTMLWriter& rWrt, sal_Int16 eAlign,
                            bool bTHead, const SwFrameFormat *pFrameFormat,
                            const OUString *pCaption, bool bTopCaption,
                            sal_uInt16 nHSpace, sal_uInt16 nVSpace ) const
{
    // determine value of RULES
    bool bRowsHaveBorder = false;
    bool bRowsHaveBorderOnly = true;
    SwWriteTableRow *pRow = m_aRows[0].get();
    for( SwWriteTableRows::size_type nRow=1; nRow < m_aRows.size(); ++nRow )
    {
        SwWriteTableRow *pNextRow = m_aRows[nRow].get();
        bool bBorder = ( pRow->bBottomBorder || pNextRow->bTopBorder );
        bRowsHaveBorder |= bBorder;
        bRowsHaveBorderOnly &= bBorder;

        sal_uInt16 nBorder2 = pRow->bBottomBorder ? pRow->nBottomBorder : USHRT_MAX;
        if( pNextRow->bTopBorder && pNextRow->nTopBorder < nBorder2 )
            nBorder2 = pNextRow->nTopBorder;

        pRow->bBottomBorder = bBorder;
        pRow->nBottomBorder = nBorder2;

        pNextRow->bTopBorder = bBorder;
        pNextRow->nTopBorder = nBorder2;

        pRow = pNextRow;
    }

    bool bColsHaveBorder = false;
    bool bColsHaveBorderOnly = true;
    SwWriteTableCol *pCol = m_aCols[0].get();
    for( SwWriteTableCols::size_type nCol=1; nCol<m_aCols.size(); ++nCol )
    {
        SwWriteTableCol *pNextCol = m_aCols[nCol].get();
        bool bBorder = ( pCol->bRightBorder || pNextCol->bLeftBorder );
        bColsHaveBorder |= bBorder;
        bColsHaveBorderOnly &= bBorder;
        pCol->bRightBorder = bBorder;
        pNextCol->bLeftBorder = bBorder;
        pCol = pNextCol;
    }

    // close previous numbering, etc
    rWrt.ChangeParaToken( HtmlTokenId::NONE );

    if( rWrt.m_bLFPossible )
        rWrt.OutNewLine();  // <TABLE> in new line
    OStringBuffer sOut;
    sOut.append('<').append(rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_table);

    const SvxFrameDirection nOldDirection = rWrt.m_nDirection;
    if( pFrameFormat )
        rWrt.m_nDirection = rWrt.GetHTMLDirection( pFrameFormat->GetAttrSet() );
    if( rWrt.m_bOutFlyFrame || nOldDirection != rWrt.m_nDirection )
    {
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        rWrt.OutDirection( rWrt.m_nDirection );
    }

    // output ALIGN=
    if( text::HoriOrientation::RIGHT == eAlign )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).
            append("=\"").append(OOO_STRING_SVTOOLS_HTML_AL_right).append("\"");
    }
    else if( text::HoriOrientation::CENTER == eAlign )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).
            append("=\"").append(OOO_STRING_SVTOOLS_HTML_AL_center).append("\"");
    }
    else if( text::HoriOrientation::LEFT == eAlign )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).
            append("=\"").append(OOO_STRING_SVTOOLS_HTML_AL_left).append("\"");
    }

    // output WIDTH: from layout or calculated
    if( m_nTabWidth )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width).
            append("=\"");
        if( HasRelWidths() )
            sOut.append(static_cast<sal_Int32>(m_nTabWidth)).append('%');
        else if( Application::GetDefaultDevice() )
        {
            sal_Int32 nPixWidth = Application::GetDefaultDevice()->LogicToPixel(
                        Size(m_nTabWidth,0), MapMode(MapUnit::MapTwip) ).Width();
            if( !nPixWidth )
                nPixWidth = 1;

            sOut.append(nPixWidth);
        }
        else
        {
            OSL_ENSURE( Application::GetDefaultDevice(), "no Application-Window!?" );
            sOut.append("100%");
        }
        sOut.append("\"");
    }

    if( (nHSpace || nVSpace) && Application::GetDefaultDevice())
    {
        Size aPixelSpc =
            Application::GetDefaultDevice()->LogicToPixel( Size(nHSpace,nVSpace),
                                                   MapMode(MapUnit::MapTwip) );
        if( !aPixelSpc.Width() && nHSpace )
            aPixelSpc.setWidth( 1 );
        if( !aPixelSpc.Height() && nVSpace )
            aPixelSpc.setHeight( 1 );

        if( aPixelSpc.Width() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_hspace).
                append("=\"").append(static_cast<sal_Int32>(aPixelSpc.Width())).append("\"");
        }

        if( aPixelSpc.Height() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_vspace).
                append("=\"").append(static_cast<sal_Int32>(aPixelSpc.Height())).append("\"");
        }
    }

    // output CELLPADDING: from layout or calculated
    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cellpadding).
        append("=\"").append(static_cast<sal_Int32>(SwHTMLWriter::ToPixel(m_nCellPadding,false))).append("\"");

    // output CELLSPACING: from layout or calculated
    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cellspacing).
        append("=\"").append(static_cast<sal_Int32>(SwHTMLWriter::ToPixel(m_nCellSpacing,false))).append("\"");

    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // output background
    if( pFrameFormat )
    {
        rWrt.OutBackground( pFrameFormat->GetAttrSet(), false );

        if (rWrt.m_bCfgOutStyles)
            rWrt.OutCSS1_TableFrameFormatOptions( *pFrameFormat );
    }

    sOut.append('>');
    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    rWrt.IncIndentLevel(); // indent content of table

    // output caption
    if( pCaption && !pCaption->isEmpty() )
    {
        rWrt.OutNewLine(); // <CAPTION> in new line
        OStringBuffer sOutStr(OOO_STRING_SVTOOLS_HTML_caption);
        sOutStr.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).append("=\"")
               .append(bTopCaption ? OOO_STRING_SVTOOLS_HTML_VA_top : OOO_STRING_SVTOOLS_HTML_VA_bottom)
               .append("\"");
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + sOutStr.getStr() );
        HTMLOutFuncs::Out_String( rWrt.Strm(), *pCaption, rWrt.m_eDestEnc, &rWrt.m_aNonConvertableCharacters );
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_caption, false );
    }

    const SwWriteTableCols::size_type nCols = m_aCols.size();

    // output <COLGRP>/<COL>: If exporting via layout only when during import
    // some were there, otherwise always.
    bool bColGroups = (bColsHaveBorder && !bColsHaveBorderOnly);
    if( m_bColTags )
    {
        if( bColGroups )
        {
            rWrt.OutNewLine(); // <COLGRP> in new line
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_colgroup );

            rWrt.IncIndentLevel(); // indent content of <COLGRP>
        }

        for( SwWriteTableCols::size_type nCol=0; nCol<nCols; ++nCol )
        {
            rWrt.OutNewLine(); // </COL> in new line

            const SwWriteTableCol *pColumn = m_aCols[nCol].get();

            HtmlWriter html(rWrt.Strm(), rWrt.maNamespace);
            html.start(OOO_STRING_SVTOOLS_HTML_col);

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
                html.attribute(OOO_STRING_SVTOOLS_HTML_O_width, OString::number(nWidth) + "*");
            else
                html.attribute(OOO_STRING_SVTOOLS_HTML_O_width, OString::number(SwHTMLWriter::ToPixel(nWidth,false)));
            html.end();

            if( bColGroups && pColumn->bRightBorder && nCol<nCols-1 )
            {
                rWrt.DecIndentLevel(); // indent content of <COLGRP>
                rWrt.OutNewLine(); // </COLGRP> in new line
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_colgroup,
                                            false );
                rWrt.OutNewLine(); // <COLGRP> in new line
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_colgroup );
                rWrt.IncIndentLevel(); // indent content of <COLGRP>
            }
        }
        if( bColGroups )
        {
            rWrt.DecIndentLevel(); // indent content of <COLGRP>

            rWrt.OutNewLine(); // </COLGRP> in new line
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_colgroup,
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
        m_nHeadEndRow<m_aRows.size()-1 && !m_aRows[m_nHeadEndRow]->bBottomBorder )
        bTHead = false;

    // Output <TBODY> only if <THEAD> is outputted.
    bTSections |= bTHead;

    if( bTSections )
    {
        rWrt.OutNewLine(); // <THEAD>/<TDATA> in new line
        OString aTag = bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody;
        HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), rWrt.GetNamespace() + aTag);

        rWrt.IncIndentLevel(); // indent content of <THEAD>/<TDATA>
    }

    for( SwWriteTableRows::size_type nRow = 0; nRow < m_aRows.size(); ++nRow )
    {
        const SwWriteTableRow *pRow2 = m_aRows[nRow].get();

        OutTableCells( rWrt, pRow2->GetCells(), pRow2->GetBackground() );
        if( !m_nCellSpacing && nRow < m_aRows.size()-1 && pRow2->bBottomBorder &&
            pRow2->nBottomBorder > DEF_LINE_WIDTH_1 )
        {
            for( auto nCnt = (pRow2->nBottomBorder / DEF_LINE_WIDTH_1) - 1; nCnt; --nCnt )
            {
                rWrt.OutNewLine();
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_tablerow );
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_tablerow,
                                            false );
            }
        }
        if( ( (bTHead && nRow==m_nHeadEndRow) ||
              (bTBody && pRow2->bBottomBorder) ) &&
            nRow < m_aRows.size()-1 )
        {
            rWrt.DecIndentLevel(); // indent content of <THEAD>/<TDATA>
            rWrt.OutNewLine(); // </THEAD>/</TDATA> in new line
            OString aTag = bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody;
            HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), rWrt.GetNamespace() + aTag, false);
            rWrt.OutNewLine(); // <THEAD>/<TDATA> in new line

            if( bTHead && nRow==m_nHeadEndRow )
                bTHead = false;

            aTag = bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody;
            HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), rWrt.GetNamespace() + aTag);
            rWrt.IncIndentLevel(); // indent content of <THEAD>/<TDATA>
        }
    }

    if( bTSections )
    {
        rWrt.DecIndentLevel(); // indent content of <THEAD>/<TDATA>

        rWrt.OutNewLine(); // </THEAD>/</TDATA> in new line
        OString aTag = bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody;
        HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), rWrt.GetNamespace() + aTag, false);
    }

    rWrt.DecIndentLevel(); // indent content of <TABLE>

    rWrt.OutNewLine(); // </TABLE> in new line
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_table, false );

    rWrt.m_nDirection = nOldDirection;
}

Writer& OutHTML_SwTableNode( Writer& rWrt, SwTableNode & rNode,
                           const SwFrameFormat *pFlyFrameFormat,
                           const OUString *pCaption, bool bTopCaption )
{

    SwTable& rTable = rNode.GetTable();

    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    rHTMLWrt.m_bOutTable = true;

    // The horizontal alignment of the frame (if exists) has priority.
    // NONE means that no horizontal alignment was outputted.
    sal_Int16 eFlyHoriOri = text::HoriOrientation::NONE;
    css::text::WrapTextMode eSurround = css::text::WrapTextMode_NONE;
    sal_uInt8 nFlyPrcWidth = 0;
    long nFlyWidth = 0;
    sal_uInt16 nFlyHSpace = 0;
    sal_uInt16 nFlyVSpace = 0;
    if( pFlyFrameFormat )
    {
        eSurround = pFlyFrameFormat->GetSurround().GetSurround();
        const SwFormatFrameSize& rFrameSize = pFlyFrameFormat->GetFrameSize();
        nFlyPrcWidth = rFrameSize.GetWidthPercent();
        nFlyWidth = rFrameSize.GetSize().Width();

        eFlyHoriOri = pFlyFrameFormat->GetHoriOrient().GetHoriOrient();
        if( text::HoriOrientation::NONE == eFlyHoriOri )
            eFlyHoriOri = text::HoriOrientation::LEFT;

        const SvxLRSpaceItem& rLRSpace = pFlyFrameFormat->GetLRSpace();
        nFlyHSpace = static_cast< sal_uInt16 >((rLRSpace.GetLeft() + rLRSpace.GetRight()) / 2);

        const SvxULSpaceItem& rULSpace = pFlyFrameFormat->GetULSpace();
        nFlyVSpace = (rULSpace.GetUpper() + rULSpace.GetLower()) / 2;
    }

    // maybe open a FORM
    bool bPreserveForm = false;
    if( !rHTMLWrt.m_bPreserveForm )
    {
        rHTMLWrt.OutForm( true, &rNode );
        bPreserveForm = rHTMLWrt.mxFormComps.is();
        rHTMLWrt.m_bPreserveForm = bPreserveForm;
    }

    SwFrameFormat *pFormat = rTable.GetFrameFormat();

    const SwFormatFrameSize& rFrameSize = pFormat->GetFrameSize();
    long nWidth = rFrameSize.GetSize().Width();
    sal_uInt8 nPrcWidth = rFrameSize.GetWidthPercent();
    sal_uInt16 nBaseWidth = static_cast<sal_uInt16>(nWidth);

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
            if( aLRItem.GetRight() )
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
            else if( nPrcWidth  )
            {
                // Without a right border the %-width is maintained.
                nWidth = nPrcWidth;
                bRelWidths = true;
            }
            else
            {
                // Without a right margin also an absolute width is maintained.
                // We still try to define the actual width via the layout.
                long nRealWidth = pFormat->FindLayoutRect(true).Width();
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
        if( nPrcWidth )
        {
            bRelWidths = true;
            nWidth = nPrcWidth;
        }
        break;
    }

    if( bCheckDefList )
    {
        OSL_ENSURE( !rHTMLWrt.GetNumInfo().GetNumRule() ||
                rHTMLWrt.GetNextNumInfo(),
                "NumInfo for next paragraph is missing!" );
        const SvxLRSpaceItem& aLRItem = pFormat->GetLRSpace();
        if( aLRItem.GetLeft() > 0 && rHTMLWrt.m_nDefListMargin > 0 &&
            ( !rHTMLWrt.GetNumInfo().GetNumRule() ||
              ( rHTMLWrt.GetNextNumInfo() &&
                (rHTMLWrt.GetNextNumInfo()->IsRestart() ||
                 rHTMLWrt.GetNumInfo().GetNumRule() !=
                    rHTMLWrt.GetNextNumInfo()->GetNumRule()) ) ) )
        {
            // If the paragraph before the table is not numbered or the
            // paragraph after the table starts with a new numbering or with
            // a different rule, we can maintain the indentation with a DL.
            // Otherwise we keep the indentation of the numbering.
            nNewDefListLvl = static_cast< sal_uInt16 >(
                (aLRItem.GetLeft() + (rHTMLWrt.m_nDefListMargin/2)) /
                rHTMLWrt.m_nDefListMargin );
        }
    }

    if( !pFlyFrameFormat && nNewDefListLvl != rHTMLWrt.m_nDefListLvl )
        rHTMLWrt.OutAndSetDefList( nNewDefListLvl );

    if( nNewDefListLvl )
    {
        if( rHTMLWrt.m_bLFPossible )
            rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_dd );
    }

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
            nWidth = nFlyPrcWidth ? nFlyPrcWidth : nFlyWidth;
            bRelWidths = nFlyPrcWidth > 0;
        }
    }

    sal_Int16 eDivHoriOri = text::HoriOrientation::NONE;
    switch( eTabHoriOri )
    {
    case text::HoriOrientation::LEFT:
        // If a left-aligned table has no right sided flow, then we don't need
        // a ALIGN=LEFT in the table.
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
        rHTMLWrt.OutImplicitMark( pFormat->GetName(), "table" );

    if( text::HoriOrientation::NONE!=eDivHoriOri )
    {
        if( rHTMLWrt.m_bLFPossible )
            rHTMLWrt.OutNewLine();  // <CENTER> in new line
        if( text::HoriOrientation::CENTER==eDivHoriOri )
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + OOO_STRING_SVTOOLS_HTML_center );
        else
        {
            OStringBuffer sOut(OOO_STRING_SVTOOLS_HTML_division);
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).append("=\"")
                .append(OOO_STRING_SVTOOLS_HTML_AL_right).append("\"");
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rHTMLWrt.GetNamespace() + sOut.getStr() );
        }
        rHTMLWrt.IncIndentLevel();  // indent content of <CENTER>
        rHTMLWrt.m_bLFPossible = true;
    }

    // If the table isn't in a frame, then you always can output a LF.
    if( text::HoriOrientation::NONE==eTabHoriOri )
        rHTMLWrt.m_bLFPossible = true;

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
        aTableWrt.Write( rHTMLWrt, eTabHoriOri, rTable.GetRowsToRepeat() > 0,
                         pFormat, pCaption, bTopCaption,
                         nFlyHSpace, nFlyVSpace );
    }
    else
    {
        SwHTMLWrtTable aTableWrt( rTable.GetTabLines(), nWidth,
                                  nBaseWidth, bRelWidths, 0, 0, rTable.GetRowsToRepeat() );
        aTableWrt.Write( rHTMLWrt, eTabHoriOri, rTable.GetRowsToRepeat() > 0,
                         pFormat, pCaption, bTopCaption,
                         nFlyHSpace, nFlyVSpace );
    }

    // If the table wasn't in a frame, then you always can output a LF.
    if( text::HoriOrientation::NONE==eTabHoriOri )
        rHTMLWrt.m_bLFPossible = true;

    if( text::HoriOrientation::NONE!=eDivHoriOri )
    {
        rHTMLWrt.DecIndentLevel();  // indent content of <CENTER>
        rHTMLWrt.OutNewLine();      // </CENTER> in new line
        OString aTag = text::HoriOrientation::CENTER == eDivHoriOri
                           ? OOO_STRING_SVTOOLS_HTML_center
                           : OOO_STRING_SVTOOLS_HTML_division;
        HTMLOutFuncs::Out_AsciiTag(rWrt.Strm(), rHTMLWrt.GetNamespace() + aTag, false);
        rHTMLWrt.m_bLFPossible = true;
    }

    // move Pam behind the table
    rHTMLWrt.m_pCurrentPam->GetPoint()->nNode = *rNode.EndOfSectionNode();

    if( bPreserveForm )
    {
        rHTMLWrt.m_bPreserveForm = false;
        rHTMLWrt.OutForm( false );
    }

    rHTMLWrt.m_bOutTable = false;

    if( rHTMLWrt.GetNextNumInfo() &&
        !rHTMLWrt.GetNextNumInfo()->IsRestart() &&
        rHTMLWrt.GetNextNumInfo()->GetNumRule() ==
            rHTMLWrt.GetNumInfo().GetNumRule() )
    {
        // If the paragraph after the table is numbered with the same rule as the
        // one before, then the NumInfo of the next paragraph holds the level of
        // paragraph before the table. Therefore NumInfo must be fetched again
        // to maybe close the Num list.
        rHTMLWrt.ClearNextNumInfo();
        rHTMLWrt.FillNextNumInfo();
        OutHTML_NumBulListEnd( rHTMLWrt, *rHTMLWrt.GetNextNumInfo() );
    }
    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
