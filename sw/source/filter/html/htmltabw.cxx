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
#include <vcl/wrkwin.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#include <com/sun/star/form/XFormController.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XSet.hpp>
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
#include <htmlnum.hxx>
#include <wrthtml.hxx>
#include <wrtswtbl.hxx>
#ifdef DBG_UTIL
#include <viewsh.hxx>
#include <viewopt.hxx>
#endif
#include <rtl/strbuf.hxx>
#include <sal/types.h>

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
                    bool bRel, sal_uInt16 nLeftSub=0, sal_uInt16 nRightSub=0,
                    sal_uInt16 nNumOfRowsToRepeat = 0 );
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
    // Einige Twip-Werte an Pixel-Grenzen anpassen
    if( m_bCollectBorderWidth )
        PixelizeBorders();
}

void SwHTMLWrtTable::Pixelize( sal_uInt16& rValue )
{
    if( rValue && Application::GetDefaultDevice() )
    {
        Size aSz( rValue, 0 );
        aSz = Application::GetDefaultDevice()->LogicToPixel( aSz, MapMode(MAP_TWIP) );
        if( !aSz.Width() )
            aSz.Width() = 1;
        aSz = Application::GetDefaultDevice()->PixelToLogic( aSz, MapMode(MAP_TWIP) );
        rValue = (sal_uInt16)aSz.Width();
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
            "HasTabBackground: darf nicht aufgerufen werden" );

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
            "HasTabBackground: darf nicht aufgerufen werden" );

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
        for( SwTableLines::const_iterator it = pBox->GetTabLines().begin();
                 it != pBox->GetTabLines().end(); ++it)
        {
            if ( lcl_TableLine_HasTabBorders( *it, pBorders ) )
                break;
        }
    }
    else
    {
        const SvxBoxItem& rBoxItem =
            static_cast<const SvxBoxItem&>(pBox->GetFrameFormat()->GetFormatAttr( RES_BOX ));

        *pBorders = rBoxItem.GetTop() || rBoxItem.GetBottom() ||
                    rBoxItem.GetLeft() || rBoxItem.GetRight();
    }

    return !*pBorders;
}

static bool lcl_TableLine_HasTabBorders( const SwTableLine* pLine, bool *pBorders )
{
    if( *pBorders )
        return false;

    for( SwTableBoxes::const_iterator it = pLine->GetTabBoxes().begin();
             it != pLine->GetTabBoxes().end(); ++it)
    {
        if ( lcl_TableBox_HasTabBorders( *it, pBorders ) )
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
        // MIB 30.6.97: Wenn schon eine Box expandiert wurde, wird eine
        // weitere nur expandiert, wenn sie Umrandungen besitzt.
        bool bBorders = false;
        lcl_TableBox_HasTabBorders( pBox, &bBorders );
        if( !bBorders )
            bBorders = HasTabBackground( *pBox, true, true, true, true );
        bExpand = bBorders;
    }

    return bExpand;
}

// Eine Box als einzelne Zelle schreiben
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

    SwWriteTableCol *pCol = m_aCols[nCol];
    bool bOutWidth = true;

    const SwStartNode* pSttNd = pBox->GetSttNd();
    bool bHead = false;
    if( pSttNd )
    {
        sal_uLong nNdPos = pSttNd->GetIndex()+1;

        // Art der Zelle (TD/TH) bestimmen
        SwNode* pNd;
        while( !( pNd = rWrt.pDoc->GetNodes()[nNdPos])->IsEndNode() )
        {
            if( pNd->IsTextNode() )
            {
                // nur Absaetzte betrachten, an denen man was erkennt
                // Das ist der Fall, wenn die Vorlage eine der Tabellen-Vorlagen
                // ist oder von einer der beiden abgelitten ist.
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

    rWrt.OutNewLine();  // <TH>/<TD> in neue Zeile
    OStringBuffer sOut;
    sOut.append('<');
    sOut.append(bHead ? OOO_STRING_SVTOOLS_HTML_tableheader : OOO_STRING_SVTOOLS_HTML_tabledata);

    // ROW- und COLSPAN ausgeben
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
    sal_uInt32 nPrcWidth = SAL_MAX_UINT32;
    if( bOutWidth )
    {
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
    }

    long nHeight = pCell->GetHeight() > 0
                        ? GetAbsHeight( pCell->GetHeight(), nRow, nRowSpan )
                        : 0;
    Size aPixelSz( nWidth, nHeight );

    // WIDTH ausgeben (Grrr: nur fuer Netscape)
    if( (aPixelSz.Width() || aPixelSz.Height()) && Application::GetDefaultDevice() )
    {
        Size aOldSz( aPixelSz );
        aPixelSz = Application::GetDefaultDevice()->LogicToPixel( aPixelSz,
                                                        MapMode(MAP_TWIP) );
        if( aOldSz.Width() && !aPixelSz.Width() )
            aPixelSz.Width() = 1;
        if( aOldSz.Height() && !aPixelSz.Height() )
            aPixelSz.Height() = 1;
    }

    // WIDTH ausgeben: Aus Layout oder berechnet
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
        if( !m_bLayoutExport && nColSpan==1 )
            pCol->SetOutWidth( false );
    }

    if( nHeight )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_height)
            .append("=\"").append(static_cast<sal_Int32>(aPixelSz.Height())).append("\"");
    }

    const SfxItemSet& rItemSet = pBox->GetFrameFormat()->GetAttrSet();
    const SfxPoolItem *pItem;

    // ALIGN wird jetzt nur noch an den Absaetzen ausgegeben

    // VALIGN ausgeben
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
        // Hintergrund ausgeben
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
            nNumFormat, *rWrt.pDoc->GetNumberFormatter(), rWrt.m_eDestEnc,
            &rWrt.m_aNonConvertableCharacters));
    }
    sOut.append('>');
    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
    rWrt.m_bLFPossible = true;

    rWrt.IncIndentLevel();  // den Inhalt von <TD>...</TD> einruecken

    if( pSttNd )
    {
        HTMLSaveData aSaveData( rWrt, pSttNd->GetIndex()+1,
                                pSttNd->EndOfSectionIndex() );
        rWrt.Out_SwDoc( rWrt.pCurPam );
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
                                  nBWidth, HasRelWidths(), nLSub, nRSub );
        aTableWrt.Write( rWrt );
    }

    rWrt.DecIndentLevel();  // den Inhalt von <TD>...</TD> einruecken

    if( rWrt.m_bLFPossible )
        rWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), bHead ? OOO_STRING_SVTOOLS_HTML_tableheader
                                                     : OOO_STRING_SVTOOLS_HTML_tabledata,
                                false );
    rWrt.m_bLFPossible = true;
}

// Eine Line als Zeilen ausgeben
void SwHTMLWrtTable::OutTableCells( SwHTMLWriter& rWrt,
                                    const SwWriteTableCells& rCells,
                                    const SvxBrushItem *pBrushItem ) const
{
    // Wenn die Zeile mehr als eine Zelle nethaelt und alle Zellen
    // die gleiche Ausrichtung besitzen, das VALIGN an der Zeile statt der
    // Zelle ausgeben
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

    rWrt.OutNewLine();  // <TR> in neuer Zeile
    rWrt.Strm().WriteChar( '<' ).WriteCharPtr( OOO_STRING_SVTOOLS_HTML_tablerow );
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

    rWrt.IncIndentLevel(); // Inhalt von <TR>...</TR> einruecken

    for (const auto &rpCell : rCells)
    {
        OutTableCell(rWrt, rpCell.get(), text::VertOrientation::NONE == eRowVertOri);
    }

    rWrt.DecIndentLevel(); // Inhalt von <TR>...</TR> einruecken

    rWrt.OutNewLine();  // </TR> in neuer Zeile
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_tablerow, false );
}

void SwHTMLWrtTable::Write( SwHTMLWriter& rWrt, sal_Int16 eAlign,
                            bool bTHead, const SwFrameFormat *pFrameFormat,
                            const OUString *pCaption, bool bTopCaption,
                            sal_uInt16 nHSpace, sal_uInt16 nVSpace ) const
{
    // Wert fuer FRAME bestimmen
    sal_uInt16 nFrameMask = 15;
    if( !(aRows.front())->bTopBorder )
        nFrameMask &= ~1;
    if( !(aRows.back())->bBottomBorder )
        nFrameMask &= ~2;
    if( !(aCols.front())->bLeftBorder )
        nFrameMask &= ~4;
    if( !(aCols.back())->bRightBorder )
        nFrameMask &= ~8;

    // Wert fur RULES bestimmen
    bool bRowsHaveBorder = false;
    bool bRowsHaveBorderOnly = true;
    SwWriteTableRow *pRow = m_aRows[0];
    for( SwWriteTableRows::size_type nRow=1; nRow < m_aRows.size(); ++nRow )
    {
        SwWriteTableRow *pNextRow = m_aRows[nRow];
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
    SwWriteTableCol *pCol = m_aCols[0];
    for( SwWriteTableCols::size_type nCol=1; nCol<m_aCols.size(); ++nCol )
    {
        SwWriteTableCol *pNextCol = m_aCols[nCol];
        bool bBorder = ( pCol->bRightBorder || pNextCol->bLeftBorder );
        bColsHaveBorder |= bBorder;
        bColsHaveBorderOnly &= bBorder;
        pCol->bRightBorder = bBorder;
        pNextCol->bLeftBorder = bBorder;
        pCol = pNextCol;
    }

    // vorhergende Aufzaehlung etc. beenden
    rWrt.ChangeParaToken( 0 );

    if( rWrt.m_bLFPossible )
        rWrt.OutNewLine();  // <TABLE> in neue Zeile
    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_table);

    const sal_uInt16 nOldDirection = rWrt.m_nDirection;
    if( pFrameFormat )
        rWrt.m_nDirection = rWrt.GetHTMLDirection( pFrameFormat->GetAttrSet() );
    if( rWrt.m_bOutFlyFrame || nOldDirection != rWrt.m_nDirection )
    {
        rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );
        rWrt.OutDirection( rWrt.m_nDirection );
    }

    // ALIGN= ausgeben
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

    // WIDTH ausgeben: Stammt aus Layout oder ist berechnet
    if( m_nTabWidth )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width).
            append("=\"");
        if( HasRelWidths() )
            sOut.append(static_cast<sal_Int32>(m_nTabWidth)).append('%');
        else if( Application::GetDefaultDevice() )
        {
            sal_Int32 nPixWidth = Application::GetDefaultDevice()->LogicToPixel(
                        Size(m_nTabWidth,0), MapMode(MAP_TWIP) ).Width();
            if( !nPixWidth )
                nPixWidth = 1;

            sOut.append(nPixWidth);
        }
        else
        {
            OSL_ENSURE( Application::GetDefaultDevice(), "kein Application-Window!?" );
            sOut.append("100%");
        }
        sOut.append("\"");
    }

    if( (nHSpace || nVSpace) && Application::GetDefaultDevice())
    {
        Size aPixelSpc =
            Application::GetDefaultDevice()->LogicToPixel( Size(nHSpace,nVSpace),
                                                   MapMode(MAP_TWIP) );
        if( !aPixelSpc.Width() && nHSpace )
            aPixelSpc.Width() = 1;
        if( !aPixelSpc.Height() && nVSpace )
            aPixelSpc.Height() = 1;

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

    // CELLPADDING ausgeben: Stammt aus Layout oder ist berechnet
    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cellpadding).
        append("=\"").append(static_cast<sal_Int32>(SwHTMLWriter::ToPixel(m_nCellPadding,false))).append("\"");

    // CELLSPACING ausgeben: Stammt aus Layout oder ist berechnet
    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cellspacing).
        append("=\"").append(static_cast<sal_Int32>(SwHTMLWriter::ToPixel(m_nCellSpacing,false))).append("\"");

    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    // Hintergrund ausgeben
    if( pFrameFormat )
    {
        rWrt.OutBackground( pFrameFormat->GetAttrSet(), false );

        if( rWrt.m_bCfgOutStyles && pFrameFormat )
            rWrt.OutCSS1_TableFrameFormatOptions( *pFrameFormat );
    }

    sOut.append('>');
    rWrt.Strm().WriteCharPtr( sOut.makeStringAndClear().getStr() );

    rWrt.IncIndentLevel(); // Inhalte von Table einruecken

    // Ueberschrift ausgeben
    if( pCaption && !pCaption->isEmpty() )
    {
        rWrt.OutNewLine(); // <CAPTION> in neue Zeile
        OStringBuffer sOutStr(OOO_STRING_SVTOOLS_HTML_caption);
        sOutStr.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).append("=\"")
               .append(bTopCaption ? OOO_STRING_SVTOOLS_HTML_VA_top : OOO_STRING_SVTOOLS_HTML_VA_bottom)
               .append("\"");
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sOutStr.getStr() );
        HTMLOutFuncs::Out_String( rWrt.Strm(), *pCaption, rWrt.m_eDestEnc, &rWrt.m_aNonConvertableCharacters );
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_caption, false );
    }

    const SwWriteTableCols::size_type nCols = m_aCols.size();

    // <COLGRP>/<COL> ausgeben: Bei Export ueber Layout nur wenn beim
    // Import welche da waren, sonst immer.
    bool bColGroups = (bColsHaveBorder && !bColsHaveBorderOnly);
    if( m_bColTags )
    {
        if( bColGroups )
        {
            rWrt.OutNewLine(); // <COLGRP> in neue Zeile
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_colgroup );

            rWrt.IncIndentLevel(); // Inhalt von <COLGRP> einruecken
        }

        for( SwWriteTableCols::size_type nCol=0; nCol<nCols; ++nCol )
        {
            rWrt.OutNewLine(); // <COL> in neue Zeile

            const SwWriteTableCol *pColumn = m_aCols[nCol];

            OStringBuffer sOutStr;
            sOutStr.append('<').append(OOO_STRING_SVTOOLS_HTML_col);

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

            sOutStr.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width).
                append("=\"");
            if( bRel )
                sOutStr.append(static_cast<sal_Int32>(nWidth)).append('*');
            else
                sOutStr.append(static_cast<sal_Int32>(SwHTMLWriter::ToPixel(nWidth,false)));
            sOutStr.append("\">");
            rWrt.Strm().WriteCharPtr( sOutStr.makeStringAndClear().getStr() );

            if( bColGroups && pColumn->bRightBorder && nCol<nCols-1 )
            {
                rWrt.DecIndentLevel(); // Inhalt von <COLGRP> einruecken
                rWrt.OutNewLine(); // </COLGRP> in neue Zeile
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_colgroup,
                                            false );
                rWrt.OutNewLine(); // <COLGRP> in neue Zeile
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_colgroup );
                rWrt.IncIndentLevel(); // Inhalt von <COLGRP> einruecken
            }
        }
        if( bColGroups )
        {
            rWrt.DecIndentLevel(); // Inhalt von <COLGRP> einruecken

            rWrt.OutNewLine(); // </COLGRP> in neue Zeile
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_colgroup,
                                        false );
        }
    }

    // die Lines als Tabellenzeilen rausschreiben

    // <TBODY> ausgeben?
    bool bTSections = (bRowsHaveBorder && !bRowsHaveBorderOnly);
    bool bTBody = bTSections;

    // Wenn Sections ausgegeben werden muessen darf ein THEAD um die erste
    // Zeile nur ausgegeben werden, wenn unter der Zeile eine Linie ist
    if( bTHead &&
        (bTSections || bColGroups) &&
        m_nHeadEndRow<m_aRows.size()-1 && !m_aRows[m_nHeadEndRow]->bBottomBorder )
        bTHead = false;

    // <TBODY> aus ausgeben, wenn <THEAD> ausgegeben wird.
    bTSections |= bTHead;

    if( bTSections )
    {
        rWrt.OutNewLine(); // <THEAD>/<TDATA> in neue Zeile
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody );

        rWrt.IncIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.
    }

    for( SwWriteTableRows::size_type nRow = 0; nRow < m_aRows.size(); ++nRow )
    {
        const SwWriteTableRow *pRow2 = m_aRows[nRow];

        OutTableCells( rWrt, pRow2->GetCells(), pRow2->GetBackground() );
        if( !m_nCellSpacing && nRow < m_aRows.size()-1 && pRow2->bBottomBorder &&
            pRow2->nBottomBorder > DEF_LINE_WIDTH_1 )
        {
            for( auto nCnt = (pRow2->nBottomBorder / DEF_LINE_WIDTH_1) - 1; nCnt; --nCnt )
            {
                rWrt.OutNewLine();
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_tablerow );
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_tablerow,
                                            false );
            }
        }
        if( ( (bTHead && nRow==m_nHeadEndRow) ||
              (bTBody && pRow2->bBottomBorder) ) &&
            nRow < m_aRows.size()-1 )
        {
            rWrt.DecIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.
            rWrt.OutNewLine(); // </THEAD>/</TDATA> in neue Zeile
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody, false );
            rWrt.OutNewLine(); // <THEAD>/<TDATA> in neue Zeile

            if( bTHead && nRow==m_nHeadEndRow )
                bTHead = false;

            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody );
            rWrt.IncIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.
        }
    }

    if( bTSections )
    {
        rWrt.DecIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.

        rWrt.OutNewLine(); // </THEAD>/</TDATA> in neue Zeile
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody, false );
    }

    rWrt.DecIndentLevel(); // Inhalt von <TABLE> einr.

    rWrt.OutNewLine(); // </TABLE> in neue Zeile
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_table, false );

    rWrt.m_nDirection = nOldDirection;
}

Writer& OutHTML_SwTableNode( Writer& rWrt, SwTableNode & rNode,
                           const SwFrameFormat *pFlyFrameFormat,
                           const OUString *pCaption, bool bTopCaption )
{

    SwTable& rTable = rNode.GetTable();

    SwHTMLWriter & rHTMLWrt = static_cast<SwHTMLWriter&>(rWrt);
    rHTMLWrt.m_bOutTable = true;

    // die horizontale Ausrichtung des Rahmens hat (falls vorhanden)
    // Prioritaet. NONE bedeutet, dass keine horizontale
    // Ausrichtung geschrieben wird.
    sal_Int16 eFlyHoriOri = text::HoriOrientation::NONE;
    SwSurround eSurround = SURROUND_NONE;
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

    // ggf. eine FORM oeffnen
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
    sal_uInt16 nBaseWidth = (sal_uInt16)nWidth;

    sal_Int16 eTabHoriOri = pFormat->GetHoriOrient().GetHoriOrient();

    // text::HoriOrientation::NONE und text::HoriOrientation::FULL Tabellen benoetigen relative Breiten
    sal_uInt16 nNewDefListLvl = 0;
    bool bRelWidths = false;
    bool bCheckDefList = false;
    switch( eTabHoriOri )
    {
    case text::HoriOrientation::FULL:
        // Tabellen mit automatischer Ausrichtung werden zu Tabellen
        // mit 100%-Breite
        bRelWidths = true;
        nWidth = 100;
        eTabHoriOri = text::HoriOrientation::LEFT;
        break;
    case text::HoriOrientation::NONE:
        {
            const SvxLRSpaceItem& aLRItem = pFormat->GetLRSpace();
            if( aLRItem.GetRight() )
            {
                // Die Tabellenbreite wird anhand des linken und rechten
                // Abstandes bestimmt. Deshalb versuchen wir die
                // tatsaechliche Breite der Tabelle zu bestimmen. Wenn
                // das nicht geht, machen wir eine 100% breite Tabelle
                // draus.
                nWidth = pFormat->FindLayoutRect(true).Width();
                if( !nWidth )
                {
                    bRelWidths = true;
                    nWidth = 100;
                }

            }
            else if( nPrcWidth  )
            {
                // Ohne rechten Rand bleibt die %-Breite erhalten
                nWidth = nPrcWidth;
                bRelWidths = true;
            }
            else
            {
                // Ohne rechten Rand bleibt auch eine absolute Breite erhalten
                // Wir versuchen aber trotzdem ueber das Layout die
                // tatsachliche Breite zu ermitteln.
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
        SAL_FALLTHROUGH;
    default:
        // In allen anderen Faellen kann eine absolute oder relative
        // Breite direkt uebernommen werden.
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
                "NumInfo fuer naechsten Absatz fehlt!" );
        const SvxLRSpaceItem& aLRItem = pFormat->GetLRSpace();
        if( aLRItem.GetLeft() > 0 && rHTMLWrt.m_nDefListMargin > 0 &&
            ( !rHTMLWrt.GetNumInfo().GetNumRule() ||
              ( rHTMLWrt.GetNextNumInfo() &&
                (rHTMLWrt.GetNextNumInfo()->IsRestart() ||
                 rHTMLWrt.GetNumInfo().GetNumRule() !=
                    rHTMLWrt.GetNextNumInfo()->GetNumRule()) ) ) )
        {
            // Wenn der Absatz vor der Tabelle nicht numeriert ist oder
            // der Absatz nach der Tabelle mit einer anderen oder
            // (gar keiner) Regel numeriert ist, koennen wir
            // die Einrueckung ueber eine DL regeln. Sonst behalten wir
            // die Einrueckung der Numerierung bei.
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
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_dd );
    }

    // eFlyHoriOri und eTabHoriOri besitzen nun nur noch die Werte
    // LEFT/CENTER und RIGHT!
    if( eFlyHoriOri!=text::HoriOrientation::NONE )
    {
        eTabHoriOri = eFlyHoriOri;
        // MIB 4.7.97: Wenn die Tabelle eine relative Breite besitzt,
        // dann richtet sich ihre Breite nach der des Rahmens, also
        // exportieren wir dessen Breite. Bei fixer Breite ist die Breite
        // der Tabelle massgeblich. Wer Tabellen mit relativer Breite <100%
        // in Rahmen steckt, ist selber schuld wenn nix Gutes bei rauskommt.
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
        // Wenn eine linksbuendigeTabelle keinen rechtsseiigen Durchlauf
        // hat, brauchen wir auch kein ALIGN=LEFT in der Tabelle.
        if( eSurround==SURROUND_NONE || eSurround==SURROUND_LEFT )
            eTabHoriOri = text::HoriOrientation::NONE;
        break;
    case text::HoriOrientation::RIGHT:
        // Aehnliches gilt fuer rechtsbuendigeTabelle, hier nehmen wir
        // stattdessen ein <DIV ALIGN=RIGHT>.
        if( eSurround==SURROUND_NONE || eSurround==SURROUND_RIGHT )
        {
            eDivHoriOri = text::HoriOrientation::RIGHT;
            eTabHoriOri = text::HoriOrientation::NONE;
        }
        break;
    case text::HoriOrientation::CENTER:
        // ALIGN=CENTER versteht so gut wie keiner, deshalb verzichten wir
        // daruf und nehmen ein <CENTER>.
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
            rHTMLWrt.OutNewLine();  // <CENTER> in neuer Zeile
        if( text::HoriOrientation::CENTER==eDivHoriOri )
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_center );
        else
        {
            OStringBuffer sOut(OOO_STRING_SVTOOLS_HTML_division);
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).append("=\"")
                .append(OOO_STRING_SVTOOLS_HTML_AL_right).append("\"");
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sOut.getStr() );
        }
        rHTMLWrt.IncIndentLevel();  // Inhalt von <CENTER> einruecken
        rHTMLWrt.m_bLFPossible = true;
    }

    // Wenn die Tabelle in keinem Rahmen ist kann man immer ein LF ausgeben.
    if( text::HoriOrientation::NONE==eTabHoriOri )
        rHTMLWrt.m_bLFPossible = true;

    const SwHTMLTableLayout *pLayout = rTable.GetHTMLTableLayout();

#ifdef DBG_UTIL
    {
    SwViewShell *pSh = rWrt.pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
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

    // Wenn die Tabelle in keinem Rahmen war kann man immer ein LF ausgeben.
    if( text::HoriOrientation::NONE==eTabHoriOri )
        rHTMLWrt.m_bLFPossible = true;

    if( text::HoriOrientation::NONE!=eDivHoriOri )
    {
        rHTMLWrt.DecIndentLevel();  // Inhalt von <CENTER> einruecken
        rHTMLWrt.OutNewLine();      // </CENTER> in neue Teile
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                               text::HoriOrientation::CENTER==eDivHoriOri ? OOO_STRING_SVTOOLS_HTML_center
                                                        : OOO_STRING_SVTOOLS_HTML_division, false );
        rHTMLWrt.m_bLFPossible = true;
    }

    // Pam hinter die Tabelle verschieben
    rHTMLWrt.pCurPam->GetPoint()->nNode = *rNode.EndOfSectionNode();

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
        // Wenn der Absatz hinter der Tabelle mit der gleichen Regel
        // numeriert ist wie der Absatz vor der Tabelle, dann steht in
        // der NumInfo des naechsten Absatzes noch die Ebene des Absatzes
        // vor der Tabelle. Es muss deshalb die NumInfo noch einmal geholt
        // werden um ggf. die Num-Liste noch zu beenden.
        rHTMLWrt.ClearNextNumInfo();
        rHTMLWrt.FillNextNumInfo();
        OutHTML_NumBulListEnd( rHTMLWrt, *rHTMLWrt.GetNextNumInfo() );
    }
    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
