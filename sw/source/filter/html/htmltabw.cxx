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
    void Pixelize( sal_uInt16& rValue );
    void PixelizeBorders();

    void OutTableCell( SwHTMLWriter& rWrt, const SwWriteTableCell *pCell,
                       sal_Bool bOutVAlign ) const;

    void OutTableCells( SwHTMLWriter& rWrt,
                        const SwWriteTableCells& rCells,
                        const SvxBrushItem *pBrushItem ) const;

    virtual sal_Bool ShouldExpandSub( const SwTableBox *pBox,
                            sal_Bool bExpandedBefore, sal_uInt16 nDepth ) const;

    static sal_Bool HasTabBackground( const SwTableLine& rLine,
                        sal_Bool bTop, sal_Bool bBottom, sal_Bool bLeft, sal_Bool bRight );
    static sal_Bool HasTabBackground( const SwTableBox& rBox,
                        sal_Bool bTop, sal_Bool bBottom, sal_Bool bLeft, sal_Bool bRight );

public:
    SwHTMLWrtTable( const SwTableLines& rLines, long nWidth, sal_uInt32 nBWidth,
                    sal_Bool bRel, sal_uInt16 nNumOfRowsToRepeat,
                    sal_uInt16 nLeftSub=0, sal_uInt16 nRightSub=0 );
    SwHTMLWrtTable( const SwHTMLTableLayout *pLayoutInfo );

    void Write( SwHTMLWriter& rWrt, sal_Int16 eAlign=text::HoriOrientation::NONE,
                sal_Bool bTHead=sal_False, const SwFrmFmt *pFrmFmt=0,
                const OUString *pCaption=0, sal_Bool bTopCaption=sal_False,
                sal_uInt16 nHSpace=0, sal_uInt16 nVSpace=0 ) const;
};


SwHTMLWrtTable::SwHTMLWrtTable( const SwTableLines& rLines, long nWidth,
                                sal_uInt32 nBWidth, sal_Bool bRel, sal_uInt16 nNumOfRowsToRepeat,
                                sal_uInt16 nLSub, sal_uInt16 nRSub )
    : SwWriteTable( rLines, nWidth, nBWidth, bRel, MAX_DEPTH, nLSub, nRSub, nNumOfRowsToRepeat )
{
    PixelizeBorders();
}

SwHTMLWrtTable::SwHTMLWrtTable( const SwHTMLTableLayout *pLayoutInfo )
    : SwWriteTable( pLayoutInfo )
{
    // Einige Twip-Werte an Pixel-Grenzen anpassen
    if( bCollectBorderWidth )
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
    Pixelize( nBorder );
    Pixelize( nCellSpacing );
    Pixelize( nCellPadding );
}

sal_Bool SwHTMLWrtTable::HasTabBackground( const SwTableBox& rBox,
                        sal_Bool bTop, sal_Bool bBottom, sal_Bool bLeft, sal_Bool bRight )
{
    OSL_ENSURE( bTop || bBottom || bLeft || bRight,
            "HasTabBackground: darf nicht aufgerufen werden" );

    sal_Bool bRet = sal_False;
    if( rBox.GetSttNd() )
    {
        const SvxBrushItem& rBrushItem =
            rBox.GetFrmFmt()->GetBackground();

        /// The table box has a background, if its background color is not "no fill"/
        /// "auto fill" or it has a background graphic.
        bRet = rBrushItem.GetColor() != COL_TRANSPARENT ||
               rBrushItem.GetGraphicLink() || rBrushItem.GetGraphic();
    }
    else
    {
        const SwTableLines& rLines = rBox.GetTabLines();
        sal_uInt16 nCount = rLines.size();
        sal_Bool bLeftRight = bLeft || bRight;
        for( sal_uInt16 i=0; !bRet && i<nCount; i++ )
        {
            sal_Bool bT = bTop && 0 == i;
            sal_Bool bB = bBottom && nCount-1 == i;
            if( bT || bB || bLeftRight )
                bRet = HasTabBackground( *rLines[i], bT, bB, bLeft, bRight);
        }
    }

    return bRet;
}

sal_Bool SwHTMLWrtTable::HasTabBackground( const SwTableLine& rLine,
                        sal_Bool bTop, sal_Bool bBottom, sal_Bool bLeft, sal_Bool bRight )
{
    OSL_ENSURE( bTop || bBottom || bLeft || bRight,
            "HasTabBackground: darf nicht aufgerufen werden" );

    const SvxBrushItem& rBrushItem = rLine.GetFrmFmt()->GetBackground();
    /// The table line has a background, if its background color is not "no fill"/
    /// "auto fill" or it has a background graphic.
    sal_Bool bRet = rBrushItem.GetColor() != COL_TRANSPARENT ||
           rBrushItem.GetGraphicLink() || rBrushItem.GetGraphic();

    if( !bRet )
    {
        const SwTableBoxes& rBoxes = rLine.GetTabBoxes();
        sal_uInt16 nCount = rBoxes.size();
        sal_Bool bTopBottom = bTop || bBottom;
        for( sal_uInt16 i=0; !bRet && i<nCount; i++ )
        {
            sal_Bool bL = bLeft && 0 == i;
            sal_Bool bR = bRight && nCount-1 == i;
            if( bTopBottom || bL || bR )
                bRet = HasTabBackground( *rBoxes[i], bTop, bBottom, bL, bR );
        }
    }

    return bRet;
}

static sal_Bool lcl_TableLine_HasTabBorders( const SwTableLine* pLine, sal_Bool *pBorders );

static sal_Bool lcl_TableBox_HasTabBorders( const SwTableBox* pBox, sal_Bool *pBorders )
{
    if( *pBorders )
        return sal_False;

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
            (const SvxBoxItem&)pBox->GetFrmFmt()->GetFmtAttr( RES_BOX );

        *pBorders = rBoxItem.GetTop() || rBoxItem.GetBottom() ||
                    rBoxItem.GetLeft() || rBoxItem.GetRight();
    }

    return !*pBorders;
}

static sal_Bool lcl_TableLine_HasTabBorders( const SwTableLine* pLine, sal_Bool *pBorders )
{
    if( *pBorders )
        return sal_False;

    for( SwTableBoxes::const_iterator it = pLine->GetTabBoxes().begin();
             it != pLine->GetTabBoxes().end(); ++it)
    {
        if ( lcl_TableBox_HasTabBorders( *it, pBorders ) )
            break;
    }
    return !*pBorders;
}


sal_Bool SwHTMLWrtTable::ShouldExpandSub( const SwTableBox *pBox,
                                      sal_Bool bExpandedBefore,
                                      sal_uInt16 nDepth ) const
{
    sal_Bool bExpand = !pBox->GetSttNd() && nDepth>0;
    if( bExpand && bExpandedBefore )
    {
        // MIB 30.6.97: Wenn schon eine Box expandiert wurde, wird eine
        // weitere nur expandiert, wenn sie Umrandungen besitzt.
        sal_Bool bBorders = sal_False;
        lcl_TableBox_HasTabBorders( pBox, &bBorders );
        if( !bBorders )
            bBorders = HasTabBackground( *pBox, sal_True, sal_True, sal_True, sal_True );
        bExpand = bBorders;
    }

    return bExpand;
}


// Eine Box als einzelne Zelle schreiben
void SwHTMLWrtTable::OutTableCell( SwHTMLWriter& rWrt,
                                   const SwWriteTableCell *pCell,
                                   sal_Bool bOutVAlign ) const
{
    const SwTableBox *pBox = pCell->GetBox();
    sal_uInt16 nRow = pCell->GetRow();
    sal_uInt16 nCol = pCell->GetCol();
    sal_uInt16 nRowSpan = pCell->GetRowSpan();
    sal_uInt16 nColSpan = pCell->GetColSpan();

    if ( !nRowSpan )
        return;

#ifndef PURE_HTML
    SwWriteTableCol *pCol = aCols[nCol];
#endif

    bool bOutWidth = true;

    const SwStartNode* pSttNd = pBox->GetSttNd();
    sal_Bool bHead = sal_False;
    if( pSttNd )
    {
        sal_uLong nNdPos = pSttNd->GetIndex()+1;

        // Art der Zelle (TD/TH) bestimmen
        SwNode* pNd;
        while( !( pNd = rWrt.pDoc->GetNodes()[nNdPos])->IsEndNode() )
        {
            if( pNd->IsTxtNode() )
            {
                // nur Absaetzte betrachten, an denen man was erkennt
                // Das ist der Fall, wenn die Vorlage eine der Tabellen-Vorlagen
                // ist oder von einer der beiden abgelitten ist.
                const SwFmt *pFmt = &((SwTxtNode*)pNd)->GetAnyFmtColl();
                sal_uInt16 nPoolId = pFmt->GetPoolFmtId();
                while( !pFmt->IsDefault() &&
                       RES_POOLCOLL_TABLE_HDLN!=nPoolId &&
                       RES_POOLCOLL_TABLE!=nPoolId )
                {
                    pFmt = pFmt->DerivedFrom();
                    nPoolId = pFmt->GetPoolFmtId();
                }

                if( !pFmt->IsDefault() )
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
            append('=').append(static_cast<sal_Int32>(nRowSpan));
    }
    if( nColSpan > 1 )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_colspan).
            append('=').append(static_cast<sal_Int32>(nColSpan));
    }
#ifndef PURE_HTML
    long nWidth = 0;
    sal_uInt32 nPrcWidth = USHRT_MAX;
    if( bOutWidth )
    {
        if( bLayoutExport )
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
                nPrcWidth = (sal_uInt16)GetPrcWidth(nCol,nColSpan);
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
            append('=');
        if( nPrcWidth != USHRT_MAX )
        {
            sOut.append(static_cast<sal_Int32>(nPrcWidth)).append('%');
        }
        else
        {
            sOut.append(static_cast<sal_Int32>(aPixelSz.Width()));
        }
        if( !bLayoutExport && nColSpan==1 )
            pCol->SetOutWidth( false );
    }

    if( nHeight )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_height).
            append('=').append(static_cast<sal_Int32>(aPixelSz.Height()));
    }
#endif

    const SfxItemSet& rItemSet = pBox->GetFrmFmt()->GetAttrSet();
    const SfxPoolItem *pItem;

    // ALIGN wird jetzt nur noch an den Absaetzen ausgegeben

    // VALIGN ausgeben
    if( bOutVAlign )
    {
        sal_Int16 eVertOri = pCell->GetVertOri();
        if( text::VertOrientation::TOP==eVertOri || text::VertOrientation::BOTTOM==eVertOri )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_valign).
                append('=').append(text::VertOrientation::TOP==eVertOri ?
                    OOO_STRING_SVTOOLS_HTML_VA_top :
                    OOO_STRING_SVTOOLS_HTML_VA_bottom);
        }
    }

    rWrt.Strm() << sOut.makeStringAndClear().getStr();

    rWrt.bTxtAttr = sal_False;
    rWrt.bOutOpts = sal_True;
    const SvxBrushItem *pBrushItem = 0;
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BACKGROUND, sal_False, &pItem ) )
    {
        pBrushItem = (const SvxBrushItem *)pItem;
    }
    if( !pBrushItem )
        pBrushItem = pCell->GetBackground();

    if( pBrushItem )
    {
        // Hintergrund ausgeben
        rWrt.OutBackground( pBrushItem, sal_False );

        if( rWrt.bCfgOutStyles )
            OutCSS1_TableBGStyleOpt( rWrt, *pBrushItem );
    }

    rWrt.OutCSS1_TableCellBorderHack(*pBox->GetFrmFmt());

    sal_uInt32 nNumFmt = 0;
    double nValue = 0.0;
    sal_Bool bNumFmt = sal_False, bValue = sal_False;
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BOXATR_FORMAT, sal_False, &pItem ) )
    {
        nNumFmt = ((const SwTblBoxNumFormat *)pItem)->GetValue();
        bNumFmt = sal_True;
    }
    if( SFX_ITEM_SET==rItemSet.GetItemState( RES_BOXATR_VALUE, sal_False, &pItem ) )
    {
        nValue = ((const SwTblBoxValue *)pItem)->GetValue();
        bValue = sal_True;
        if( !bNumFmt )
            nNumFmt = pBox->GetFrmFmt()->GetTblBoxNumFmt().GetValue();
    }

    if( bNumFmt || bValue )
    {
        sOut.append(HTMLOutFuncs::CreateTableDataOptionsValNum(bValue, nValue,
            nNumFmt, *rWrt.pDoc->GetNumberFormatter(), rWrt.eDestEnc,
            &rWrt.aNonConvertableCharacters));
    }
    sOut.append('>');
    rWrt.Strm() << sOut.makeStringAndClear().getStr();
    rWrt.bLFPossible = sal_True;

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

    if( rWrt.bLFPossible )
        rWrt.OutNewLine();
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), bHead ? OOO_STRING_SVTOOLS_HTML_tableheader
                                                     : OOO_STRING_SVTOOLS_HTML_tabledata,
                                sal_False );
    rWrt.bLFPossible = sal_True;
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
        for( sal_uInt16 nCell = 0; nCell<rCells.size(); nCell++ )
        {
            sal_Int16 eCellVertOri = rCells[nCell].GetVertOri();
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
    rWrt.Strm() << '<' << OOO_STRING_SVTOOLS_HTML_tablerow;
    if( pBrushItem )
    {
        rWrt.OutBackground( pBrushItem, sal_False );

        rWrt.bTxtAttr = sal_False;
        rWrt.bOutOpts = sal_True;
        if( rWrt.bCfgOutStyles )
            OutCSS1_TableBGStyleOpt( rWrt, *pBrushItem );
    }

    if( text::VertOrientation::TOP==eRowVertOri || text::VertOrientation::BOTTOM==eRowVertOri )
    {
        OStringBuffer sOut;
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_valign)
            .append('=').append(text::VertOrientation::TOP==eRowVertOri ? OOO_STRING_SVTOOLS_HTML_VA_top : OOO_STRING_SVTOOLS_HTML_VA_bottom);
        rWrt.Strm() << sOut.makeStringAndClear().getStr();
    }

    rWrt.Strm() << '>';

    rWrt.IncIndentLevel(); // Inhalt von <TR>...</TR> einruecken

    for( sal_uInt16 nCell = 0; nCell<rCells.size(); nCell++ )
        OutTableCell( rWrt, &rCells[nCell], text::VertOrientation::NONE==eRowVertOri );

    rWrt.DecIndentLevel(); // Inhalt von <TR>...</TR> einruecken

    rWrt.OutNewLine();  // </TR> in neuer Zeile
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_tablerow, sal_False );
}



void SwHTMLWrtTable::Write( SwHTMLWriter& rWrt, sal_Int16 eAlign,
                            sal_Bool bTHead, const SwFrmFmt *pFrmFmt,
                            const OUString *pCaption, sal_Bool bTopCaption,
                            sal_uInt16 nHSpace, sal_uInt16 nVSpace ) const
{
    sal_uInt16 nRow;

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
    sal_Bool bRowsHaveBorder = sal_False;
    sal_Bool bRowsHaveBorderOnly = sal_True;
    SwWriteTableRow *pRow = aRows[0];
    for( nRow=1; nRow < aRows.size(); nRow++ )
    {
        SwWriteTableRow *pNextRow = aRows[nRow];
        sal_Bool bBorder = ( pRow->bBottomBorder || pNextRow->bTopBorder );
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

    sal_Bool bColsHaveBorder = sal_False;
    sal_Bool bColsHaveBorderOnly = sal_True;
    SwWriteTableCol *pCol = aCols[0];
    sal_uInt16 nCol;
    for( nCol=1; nCol<aCols.size(); nCol++ )
    {
        SwWriteTableCol *pNextCol = aCols[nCol];
        sal_Bool bBorder = ( pCol->bRightBorder || pNextCol->bLeftBorder );
        bColsHaveBorder |= bBorder;
        bColsHaveBorderOnly &= bBorder;
        pCol->bRightBorder = bBorder;
        pNextCol->bLeftBorder = bBorder;
        pCol = pNextCol;
    }


    // vorhergende Aufzaehlung etc. beenden
    rWrt.ChangeParaToken( 0 );

    if( rWrt.bLFPossible )
        rWrt.OutNewLine();  // <TABLE> in neue Zeile
    OStringBuffer sOut;
    sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_table);

    sal_uInt16 nOldDirection = rWrt.nDirection;
    if( pFrmFmt )
        rWrt.nDirection = rWrt.GetHTMLDirection( pFrmFmt->GetAttrSet() );
    if( rWrt.bOutFlyFrame || nOldDirection != rWrt.nDirection )
    {
        rWrt.Strm() << sOut.makeStringAndClear().getStr();
        rWrt.OutDirection( rWrt.nDirection );
    }

    // COLS ausgeben: Nur bei Export ueber Layout, wenn es beim Import
    // vorhanden war.
    if( bColsOption )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cols).
            append('=').append(static_cast<sal_Int32>(aCols.size()));
    }

    // ALIGN= ausgeben
    if( text::HoriOrientation::RIGHT == eAlign )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).
            append('=').append(OOO_STRING_SVTOOLS_HTML_AL_right);
    }
    else if( text::HoriOrientation::CENTER == eAlign )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).
            append('=').append(OOO_STRING_SVTOOLS_HTML_AL_center);
    }
    else if( text::HoriOrientation::LEFT == eAlign )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).
            append('=').append(OOO_STRING_SVTOOLS_HTML_AL_left);
    }

    // WIDTH ausgeben: Stammt aus Layout oder ist berechnet
    if( nTabWidth )
    {
        sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_width).
            append('=');
        if( HasRelWidths() )
            sOut.append(static_cast<sal_Int32>(nTabWidth)).append('%');
        else if( Application::GetDefaultDevice() )
        {
            sal_Int32 nPixWidth = Application::GetDefaultDevice()->LogicToPixel(
                        Size(nTabWidth,0), MapMode(MAP_TWIP) ).Width();
            if( !nPixWidth )
                nPixWidth = 1;

            sOut.append(nPixWidth);
        }
        else
        {
            OSL_ENSURE( Application::GetDefaultDevice(), "kein Application-Window!?" );
            sOut.append(RTL_CONSTASCII_STRINGPARAM("100%"));
        }
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
                append('=').append(static_cast<sal_Int32>(aPixelSpc.Width()));
        }

        if( aPixelSpc.Height() )
        {
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_vspace).
                append('=').append(static_cast<sal_Int32>(aPixelSpc.Height()));
        }
    }

    // CELLPADDING ausgeben: Stammt aus Layout oder ist berechnet
    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cellpadding).
        append('=').append(static_cast<sal_Int32>(rWrt.ToPixel(nCellPadding,false)));

    // CELLSPACING ausgeben: Stammt aus Layout oder ist berechnet
    sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_cellspacing).
        append('=').append(static_cast<sal_Int32>(rWrt.ToPixel(nCellSpacing,false)));

    rWrt.Strm() << sOut.makeStringAndClear().getStr();

    // Hintergrund ausgeben
    if( pFrmFmt )
    {
        rWrt.OutBackground( pFrmFmt->GetAttrSet(), sal_False );

        if( rWrt.bCfgOutStyles && pFrmFmt )
            rWrt.OutCSS1_TableFrmFmtOptions( *pFrmFmt );
    }

    sOut.append('>');
    rWrt.Strm() << sOut.makeStringAndClear().getStr();

    rWrt.IncIndentLevel(); // Inhalte von Table einruecken

    // Ueberschrift ausgeben
    if( pCaption && !pCaption->isEmpty() )
    {
        rWrt.OutNewLine(); // <CAPTION> in neue Zeile
        OStringBuffer sOutStr(RTL_CONSTASCII_STRINGPARAM(OOO_STRING_SVTOOLS_HTML_caption));
        sOutStr.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).append('=')
               .append(bTopCaption ? OOO_STRING_SVTOOLS_HTML_VA_top : OOO_STRING_SVTOOLS_HTML_VA_bottom);
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sOutStr.getStr(), sal_True );
        HTMLOutFuncs::Out_String( rWrt.Strm(), *pCaption, rWrt.eDestEnc, &rWrt.aNonConvertableCharacters    );
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_caption, sal_False );
    }

    sal_uInt16 nCols = aCols.size();

    // <COLGRP>/<COL> ausgeben: Bei Export ueber Layout nur wenn beim
    // Import welche da waren, sonst immer.
    sal_Bool bColGroups = (bColsHaveBorder && !bColsHaveBorderOnly);
    if( bColTags )
    {
        if( bColGroups )
        {
            rWrt.OutNewLine(); // <COLGRP> in neue Zeile
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_colgroup, sal_True );

            rWrt.IncIndentLevel(); // Inhalt von <COLGRP> einruecken
        }

        for( nCol=0; nCol<nCols; nCol++ )
        {
            rWrt.OutNewLine(); // <COL> in neue Zeile

            const SwWriteTableCol *pColumn = aCols[nCol];

            OStringBuffer sOutStr;
            sOutStr.append('<').append(OOO_STRING_SVTOOLS_HTML_col);

            sal_uInt32 nWidth;
            bool bRel;
            if( bLayoutExport )
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
                append('=');
            if( bRel )
                sOutStr.append(static_cast<sal_Int32>(nWidth)).append('*');
            else
                sOutStr.append(static_cast<sal_Int32>(rWrt.ToPixel(nWidth,false)));
            sOutStr.append('>');
            rWrt.Strm() << sOutStr.makeStringAndClear().getStr();

            if( bColGroups && pColumn->bRightBorder && nCol<nCols-1 )
            {
                rWrt.DecIndentLevel(); // Inhalt von <COLGRP> einruecken
                rWrt.OutNewLine(); // </COLGRP> in neue Zeile
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_colgroup,
                                            sal_False );
                rWrt.OutNewLine(); // <COLGRP> in neue Zeile
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_colgroup,
                                            sal_True );
                rWrt.IncIndentLevel(); // Inhalt von <COLGRP> einruecken
            }
        }
        if( bColGroups )
        {
            rWrt.DecIndentLevel(); // Inhalt von <COLGRP> einruecken

            rWrt.OutNewLine(); // </COLGRP> in neue Zeile
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_colgroup,
                                        sal_False );
        }
    }

    // die Lines als Tabellenzeilen rausschreiben

    // <TBODY> ausgeben?
    sal_Bool bTSections = (bRowsHaveBorder && !bRowsHaveBorderOnly);
    sal_Bool bTBody = bTSections;

    // Wenn Sections ausgegeben werden muessen darf ein THEAD um die erste
    // Zeile nur ausgegeben werden, wenn unter der Zeile eine Linie ist
    if( bTHead &&
        (bTSections || bColGroups) &&
        nHeadEndRow<aRows.size()-1 && !aRows[nHeadEndRow]->bBottomBorder )
        bTHead = sal_False;

    // <TBODY> aus ausgeben, wenn <THEAD> ausgegeben wird.
    bTSections |= bTHead;

    if( bTSections )
    {
        rWrt.OutNewLine(); // <THEAD>/<TDATA> in neue Zeile
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody, sal_True );

        rWrt.IncIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.
    }

    for( nRow = 0; nRow < aRows.size(); nRow++ )
    {
        const SwWriteTableRow *pRow2 = aRows[nRow];

        OutTableCells( rWrt, pRow2->GetCells(), pRow2->GetBackground() );
        if( !nCellSpacing && nRow < aRows.size()-1 && pRow2->bBottomBorder &&
            pRow2->nBottomBorder > DEF_LINE_WIDTH_1 )
        {
            sal_uInt16 nCnt = (pRow2->nBottomBorder / DEF_LINE_WIDTH_1) - 1;
            for( ; nCnt; nCnt-- )
            {
                rWrt.OutNewLine();
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_tablerow,
                                            sal_True );
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_tablerow,
                                            sal_False );
            }
        }
        if( ( (bTHead && nRow==nHeadEndRow) ||
              (bTBody && pRow2->bBottomBorder) ) &&
            nRow < aRows.size()-1 )
        {
            rWrt.DecIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.
            rWrt.OutNewLine(); // </THEAD>/</TDATA> in neue Zeile
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody, sal_False );
            rWrt.OutNewLine(); // <THEAD>/<TDATA> in neue Zeile

            if( bTHead && nRow==nHeadEndRow )
                bTHead = sal_False;

            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody, sal_True );
            rWrt.IncIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.
        }
    }

    if( bTSections )
    {
        rWrt.DecIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.

        rWrt.OutNewLine(); // </THEAD>/</TDATA> in neue Zeile
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? OOO_STRING_SVTOOLS_HTML_thead : OOO_STRING_SVTOOLS_HTML_tbody, sal_False );
    }

    rWrt.DecIndentLevel(); // Inhalt von <TABLE> einr.

    rWrt.OutNewLine(); // </TABLE> in neue Zeile
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_table, sal_False );

    rWrt.nDirection = nOldDirection;
}

Writer& OutHTML_SwTblNode( Writer& rWrt, SwTableNode & rNode,
                           const SwFrmFmt *pFlyFrmFmt,
                           const OUString *pCaption, sal_Bool bTopCaption )
{

    SwTable& rTbl = rNode.GetTable();

    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;
    rHTMLWrt.bOutTable = sal_True;

    // die horizontale Ausrichtung des Rahmens hat (falls vorhanden)
    // Prioritaet. NONE bedeutet, dass keine horizontale
    // Ausrichtung geschrieben wird.
    sal_Int16 eFlyHoriOri = text::HoriOrientation::NONE;
    SwSurround eSurround = SURROUND_NONE;
    sal_uInt8 nFlyPrcWidth = 0;
    long nFlyWidth = 0;
    sal_uInt16 nFlyHSpace = 0;
    sal_uInt16 nFlyVSpace = 0;
    if( pFlyFrmFmt )
    {
        eSurround = pFlyFrmFmt->GetSurround().GetSurround();
        const SwFmtFrmSize& rFrmSize = pFlyFrmFmt->GetFrmSize();
        nFlyPrcWidth = rFrmSize.GetWidthPercent();
        nFlyWidth = rFrmSize.GetSize().Width();

        eFlyHoriOri = pFlyFrmFmt->GetHoriOrient().GetHoriOrient();
        if( text::HoriOrientation::NONE == eFlyHoriOri )
            eFlyHoriOri = text::HoriOrientation::LEFT;

        const SvxLRSpaceItem& rLRSpace = pFlyFrmFmt->GetLRSpace();
        nFlyHSpace = static_cast< sal_uInt16 >((rLRSpace.GetLeft() + rLRSpace.GetRight()) / 2);

        const SvxULSpaceItem& rULSpace = pFlyFrmFmt->GetULSpace();
        nFlyVSpace = (rULSpace.GetUpper() + rULSpace.GetLower()) / 2;
    }

    // ggf. eine FORM oeffnen
    sal_Bool bPreserveForm = sal_False;
    if( !rHTMLWrt.bPreserveForm )
    {
        rHTMLWrt.OutForm( sal_True, &rNode );
        bPreserveForm = (rHTMLWrt.pxFormComps && rHTMLWrt.pxFormComps->is() );
        rHTMLWrt.bPreserveForm = bPreserveForm;
    }

    SwFrmFmt *pFmt = rTbl.GetFrmFmt();

    const SwFmtFrmSize& rFrmSize = pFmt->GetFrmSize();
    long nWidth = rFrmSize.GetSize().Width();
    sal_uInt8 nPrcWidth = rFrmSize.GetWidthPercent();
    sal_uInt16 nBaseWidth = (sal_uInt16)nWidth;

    sal_Int16 eTabHoriOri = pFmt->GetHoriOrient().GetHoriOrient();

    // text::HoriOrientation::NONE und text::HoriOrientation::FULL Tabellen benoetigen relative Breiten
    sal_uInt16 nNewDefListLvl = 0;
    bool bRelWidths = false;
    sal_Bool bCheckDefList = sal_False;
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
            const SvxLRSpaceItem& aLRItem = pFmt->GetLRSpace();
            if( aLRItem.GetRight() )
            {
                // Die Tabellenbreite wird anhand des linken und rechten
                // Abstandes bestimmt. Deshalb versuchen wir die
                // tatsaechliche Breite der Tabelle zu bestimmen. Wenn
                // das nicht geht, machen wir eine 100% breite Tabelle
                // draus.
                nWidth = pFmt->FindLayoutRect(sal_True).Width();
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
                long nRealWidth = pFmt->FindLayoutRect(sal_True).Width();
                if( nRealWidth )
                    nWidth = nRealWidth;
            }
            bCheckDefList = sal_True;
        }
        break;
    case text::HoriOrientation::LEFT_AND_WIDTH:
        eTabHoriOri = text::HoriOrientation::LEFT;
        bCheckDefList = sal_True;
        // no break
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
        const SvxLRSpaceItem& aLRItem = pFmt->GetLRSpace();
        if( aLRItem.GetLeft() > 0 && rHTMLWrt.nDefListMargin > 0 &&
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
                (aLRItem.GetLeft() + (rHTMLWrt.nDefListMargin/2)) /
                rHTMLWrt.nDefListMargin );
        }
    }

    if( !pFlyFrmFmt && nNewDefListLvl != rHTMLWrt.nDefListLvl )
        rHTMLWrt.OutAndSetDefList( nNewDefListLvl );

    if( nNewDefListLvl )
    {
        if( rHTMLWrt.bLFPossible )
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

    if( !pFmt->GetName().isEmpty() )
        rHTMLWrt.OutImplicitMark( pFmt->GetName(), pMarkToTable );

    if( text::HoriOrientation::NONE!=eDivHoriOri )
    {
        if( rHTMLWrt.bLFPossible )
            rHTMLWrt.OutNewLine();  // <CENTER> in neuer Zeile
        if( text::HoriOrientation::CENTER==eDivHoriOri )
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_center, sal_True );
        else
        {
            OStringBuffer sOut(RTL_CONSTASCII_STRINGPARAM(OOO_STRING_SVTOOLS_HTML_division));
            sOut.append(' ').append(OOO_STRING_SVTOOLS_HTML_O_align).append('=')
                .append(OOO_STRING_SVTOOLS_HTML_AL_right);
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sOut.getStr(),
                                        sal_True );
        }
        rHTMLWrt.IncIndentLevel();  // Inhalt von <CENTER> einruecken
        rHTMLWrt.bLFPossible = sal_True;
    }

    // Wenn die Tabelle in keinem Rahmen ist kann man immer ein LF ausgeben.
    if( text::HoriOrientation::NONE==eTabHoriOri )
        rHTMLWrt.bLFPossible = sal_True;

    const SwHTMLTableLayout *pLayout = rTbl.GetHTMLTableLayout();

#ifdef DBG_UTIL
    {
    ViewShell *pSh;
    rWrt.pDoc->GetEditShell( &pSh );
    if ( pSh && pSh->GetViewOptions()->IsTest1() )
        pLayout = 0;
    }
#endif

    if( pLayout && pLayout->IsExportable() )
    {
        SwHTMLWrtTable aTableWrt( pLayout );
        aTableWrt.Write( rHTMLWrt, eTabHoriOri, rTbl.GetRowsToRepeat() > 0,
                         pFmt, pCaption, bTopCaption,
                         nFlyHSpace, nFlyVSpace );
    }
    else
    {
        SwHTMLWrtTable aTableWrt( rTbl.GetTabLines(), nWidth,
                                  nBaseWidth, bRelWidths, rTbl.GetRowsToRepeat() );
        aTableWrt.Write( rHTMLWrt, eTabHoriOri, rTbl.GetRowsToRepeat() > 0,
                         pFmt, pCaption, bTopCaption,
                         nFlyHSpace, nFlyVSpace );
    }

    // Wenn die Tabelle in keinem Rahmen war kann man immer ein LF ausgeben.
    if( text::HoriOrientation::NONE==eTabHoriOri )
        rHTMLWrt.bLFPossible = sal_True;

    if( text::HoriOrientation::NONE!=eDivHoriOri )
    {
        rHTMLWrt.DecIndentLevel();  // Inhalt von <CENTER> einruecken
        rHTMLWrt.OutNewLine();      // </CENTER> in neue Teile
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                               text::HoriOrientation::CENTER==eDivHoriOri ? OOO_STRING_SVTOOLS_HTML_center
                                                        : OOO_STRING_SVTOOLS_HTML_division, sal_False );
        rHTMLWrt.bLFPossible = sal_True;
    }

    // Pam hinter die Tabelle verschieben
    rHTMLWrt.pCurPam->GetPoint()->nNode = *rNode.EndOfSectionNode();

    if( bPreserveForm )
    {
        rHTMLWrt.bPreserveForm = sal_False;
        rHTMLWrt.OutForm( sal_False );
    }

    rHTMLWrt.bOutTable = sal_False;

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
