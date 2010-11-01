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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <hintids.hxx>
#include <tools/debug.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <tools/fract.hxx>
#include <wrtswtbl.hxx>
#include <swtable.hxx>
#include <frmfmt.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <frmatr.hxx>
#include <htmltbl.hxx>

using namespace ::com::sun::star;

SV_IMPL_PTRARR( SwWriteTableCells, SwWriteTableCellPtr )
SV_IMPL_OP_PTRARR_SORT( SwWriteTableRows, SwWriteTableRowPtr )
SV_IMPL_OP_PTRARR_SORT( SwWriteTableCols, SwWriteTableColPtr )

//-----------------------------------------------------------------------

sal_Int16 SwWriteTableCell::GetVertOri() const
{
    sal_Int16 eCellVertOri = text::VertOrientation::TOP;
    if( pBox->GetSttNd() )
    {
        const SfxItemSet& rItemSet = pBox->GetFrmFmt()->GetAttrSet();
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_VERT_ORIENT, FALSE, &pItem ) )
        {
            sal_Int16 eBoxVertOri =
                ((const SwFmtVertOrient *)pItem)->GetVertOrient();
            if( text::VertOrientation::CENTER==eBoxVertOri || text::VertOrientation::BOTTOM==eBoxVertOri)
                eCellVertOri = eBoxVertOri;
        }
    }

    return eCellVertOri;
}

//-----------------------------------------------------------------------

SwWriteTableRow::SwWriteTableRow( long nPosition, BOOL bUseLayoutHeights )
    : pBackground(0), nPos(nPosition), mbUseLayoutHeights(bUseLayoutHeights),
    nTopBorder(USHRT_MAX), nBottomBorder(USHRT_MAX), bTopBorder(true),
    bBottomBorder(true)
{
}

SwWriteTableCell *SwWriteTableRow::AddCell( const SwTableBox *pBox,
                                USHORT nRow, USHORT nCol,
                                USHORT nRowSpan, USHORT nColSpan,
                                long nHeight,
                                const SvxBrushItem *pBackgroundBrush )
{
    SwWriteTableCell *pCell =
        new SwWriteTableCell( pBox, nRow, nCol, nRowSpan, nColSpan,
                                nHeight, pBackgroundBrush );
    aCells.Insert( pCell, aCells.Count() );

    return pCell;
}

//-----------------------------------------------------------------------

SwWriteTableCol::SwWriteTableCol(USHORT nPosition)
    : nPos(nPosition), nWidthOpt(0), bRelWidthOpt(false), bOutWidth(true),
    bLeftBorder(true), bRightBorder(true)
{
}

//-----------------------------------------------------------------------

long SwWriteTable::GetBoxWidth( const SwTableBox *pBox )
{
    const SwFrmFmt *pFmt = pBox->GetFrmFmt();
    const SwFmtFrmSize& aFrmSize=
        (const SwFmtFrmSize&)pFmt->GetFmtAttr( RES_FRM_SIZE );

    return aFrmSize.GetSize().Width();
}

long SwWriteTable::GetLineHeight( const SwTableLine *pLine )
{
#ifdef DBG_UTIL
    BOOL bOldGetLineHeightCalled = bGetLineHeightCalled;
    bGetLineHeightCalled = TRUE;
#endif

    long nHeight = 0;
    if( bUseLayoutHeights )
    {
        // Erstmal versuchen wir die Hoehe ueber das Layout zu bekommen
        bool bLayoutAvailable = false;
        nHeight = pLine->GetTableLineHeight(bLayoutAvailable);
        if( nHeight > 0 )
            return nHeight;

        // Wenn kein Layout gefunden wurde, gehen wir von festen Hoehen aus.
        // --> FME 2007-3-26 #i60390# in some cases we still want to continue
        // to use the layout heights even if one of the rows has a height of 0
        // ('hidden' rows)
        // <--
        bUseLayoutHeights = bLayoutAvailable; /*FALSE;*/

#ifdef DBG_UTIL
        ASSERT( bLayoutAvailable || !bOldGetLineHeightCalled, "Layout ungueltig?" );
#endif
    }

    const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    USHORT nBoxes = rBoxes.Count();

    for( USHORT nBox=0; nBox<nBoxes; nBox++ )
    {
        const SwTableBox* pBox = rBoxes[nBox];
        if( pBox->GetSttNd() )
        {
            if( nHeight < ROW_DFLT_HEIGHT )
                nHeight = ROW_DFLT_HEIGHT;
        }
        else
        {
            long nTmp = 0;
            const SwTableLines &rLines = pBox->GetTabLines();
            for( USHORT nLine=0; nLine<rLines.Count(); nLine++ )
            {
                nTmp += GetLineHeight( rLines[nLine] );
            }
            if( nHeight < nTmp )
                nHeight = nTmp;
        }
    }

    return nHeight;
}

long SwWriteTable::GetLineHeight( const SwTableBox *pBox ) const
{
    const SwTableLine *pLine = pBox->GetUpper();

    if( !pLine )
        return 0;

    const SwFrmFmt *pLineFrmFmt = pLine->GetFrmFmt();
    const SfxPoolItem* pItem;
    const SfxItemSet& rItemSet = pLineFrmFmt->GetAttrSet();

    long nHeight = 0;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_FRM_SIZE, TRUE, &pItem ))
        nHeight = ((SwFmtFrmSize*)pItem)->GetHeight();

    return nHeight;
}

const SvxBrushItem *SwWriteTable::GetLineBrush( const SwTableBox *pBox,
                                                  SwWriteTableRow *pRow )
{
    const SwTableLine *pLine = pBox->GetUpper();

    while( pLine )
    {
        const SwFrmFmt *pLineFrmFmt = pLine->GetFrmFmt();
        const SfxPoolItem* pItem;
        const SfxItemSet& rItemSet = pLineFrmFmt->GetAttrSet();

        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, FALSE,
                                                   &pItem ) )
        {
            if( !pLine->GetUpper() )
            {
                if( !pRow->GetBackground() )
                    pRow->SetBackground( (const SvxBrushItem *)pItem );
                pItem = 0;
            }

            return (const SvxBrushItem *)pItem;
        }

        pBox = pLine->GetUpper();
        pLine = pBox ? pBox->GetUpper() : 0;
    }

    return 0;
}


void SwWriteTable::MergeBorders( const SvxBorderLine* pBorderLine,
                                   BOOL bTable )
{
    if( (UINT32)-1 == nBorderColor )
    {
        Color aGrayColor( COL_GRAY );
        if( !pBorderLine->GetColor().IsRGBEqual( aGrayColor ) )
            nBorderColor = pBorderLine->GetColor().GetColor();
    }

    if( !bCollectBorderWidth )
        return;

    USHORT nOutWidth = pBorderLine->GetOutWidth();
    if( bTable )
    {
        if( nOutWidth && (!nBorder || nOutWidth < nBorder) )
            nBorder = nOutWidth;
    }
    else
    {
        if( nOutWidth && (!nInnerBorder || nOutWidth < nInnerBorder) )
            nInnerBorder = nOutWidth;
    }

    USHORT nDist = pBorderLine->GetInWidth() ? pBorderLine->GetDistance()
                                                : 0;
    if( nDist && (!nCellSpacing || nDist < nCellSpacing) )
        nCellSpacing = nDist;
}


USHORT SwWriteTable::MergeBoxBorders( const SwTableBox *pBox,
                                        USHORT nRow, USHORT nCol,
                                        USHORT nRowSpan, USHORT nColSpan,
                                        USHORT& rTopBorder,
                                        USHORT &rBottomBorder )
{
    USHORT nBorderMask = 0;

    const SwFrmFmt *pFrmFmt = pBox->GetFrmFmt();
    const SvxBoxItem& rBoxItem = (const SvxBoxItem&)pFrmFmt->GetFmtAttr( RES_BOX );

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
        MergeBorders( rBoxItem.GetBottom(), nRow+nRowSpan==aRows.Count() );
        rBottomBorder = rBoxItem.GetBottom()->GetOutWidth();
    }

    if( rBoxItem.GetRight() )
    {
        nBorderMask |= 8;
        MergeBorders( rBoxItem.GetRight(), nCol+nColSpan==aCols.Count() );
    }

    // If any distance is set, the smallest one is used. This holds for
    // the four distance of a box as well as for the distances of different
    // boxes.
    if( bCollectBorderWidth )
    {
        USHORT nDist = rBoxItem.GetDistance( BOX_LINE_TOP );
        if( nDist && (!nCellPadding || nDist < nCellPadding) )
            nCellPadding = nDist;
        nDist = rBoxItem.GetDistance( BOX_LINE_BOTTOM );
        if( nDist && (!nCellPadding || nDist < nCellPadding) )
            nCellPadding = nDist;
        nDist = rBoxItem.GetDistance( BOX_LINE_LEFT );
        if( nDist && (!nCellPadding || nDist < nCellPadding) )
            nCellPadding = nDist;
        nDist = rBoxItem.GetDistance( BOX_LINE_RIGHT );
        if( nDist && (!nCellPadding || nDist < nCellPadding) )
            nCellPadding = nDist;
    }

    return nBorderMask;
}


USHORT SwWriteTable::GetRawWidth( USHORT nCol, USHORT nColSpan ) const
{
    USHORT nWidth = aCols[nCol+nColSpan-1]->GetPos();
    if( nCol > 0 )
        nWidth = nWidth - aCols[nCol-1]->GetPos();

    return nWidth;
}

USHORT SwWriteTable::GetLeftSpace( USHORT nCol ) const
{
    USHORT nSpace = nCellPadding + nCellSpacing;

    // In der ersten Spalte auch noch die Liniendicke abziehen
    if( nCol==0 )
    {
        nSpace = nSpace + nLeftSub;

        const SwWriteTableCol *pCol = aCols[nCol];
        if( pCol->HasLeftBorder() )
            nSpace = nSpace + nBorder;
    }

    return nSpace;
}

USHORT SwWriteTable::GetRightSpace( USHORT nCol, USHORT nColSpan ) const
{
    USHORT nSpace = nCellPadding;

    // In der letzten Spalte noch einmal zusaetzlich CELLSPACING und
    // und die Liniendicke abziehen
    if( nCol+nColSpan==aCols.Count() )
    {
        nSpace += (nCellSpacing + nRightSub);

        const SwWriteTableCol *pCol = aCols[nCol+nColSpan-1];
        if( pCol->HasRightBorder() )
            nSpace = nSpace + nBorder;
    }

    return nSpace;
}

USHORT SwWriteTable::GetAbsWidth( USHORT nCol, USHORT nColSpan ) const
{
    long nWidth = GetRawWidth( nCol, nColSpan );
    if( nBaseWidth != nTabWidth )
    {
        nWidth *= nTabWidth;
        nWidth /= nBaseWidth;
    }

    nWidth -= GetLeftSpace( nCol ) + GetRightSpace( nCol, nColSpan );

    ASSERT( nWidth > 0, "Spaltenbreite <= 0. OK?" );
    return nWidth > 0 ? (USHORT)nWidth : 0;
}

USHORT SwWriteTable::GetRelWidth( USHORT nCol, USHORT nColSpan ) const
{
    long nWidth = GetRawWidth( nCol, nColSpan );

    return (USHORT)(long)Fraction( nWidth*256 + GetBaseWidth()/2,
                                   GetBaseWidth() );
}

USHORT SwWriteTable::GetPrcWidth( USHORT nCol, USHORT nColSpan ) const
{
    long nWidth = GetRawWidth( nCol, nColSpan );

    // sieht komisch aus, ist aber nichts anderes als
    //  [(100 * nWidth) + .5] ohne Rundungsfehler
    return (USHORT)(long)Fraction( nWidth*100 + GetBaseWidth()/2,
                                   GetBaseWidth() );
}

long SwWriteTable::GetAbsHeight( long nRawHeight, USHORT nRow,
                                   USHORT nRowSpan ) const
{
    nRawHeight -= (2*nCellPadding + nCellSpacing);

    // In der ersten Zeile noch einmal zusaetzlich CELLSPACING und
    // und die Liniendicke abziehen
    const SwWriteTableRow *pRow = 0;
    if( nRow==0 )
    {
        nRawHeight -= nCellSpacing;
        pRow = aRows[nRow];
        if( pRow->HasTopBorder() )
            nRawHeight -= nBorder;
    }

    // In der letzten Zeile noch die Liniendicke abziehen
    if( nRow+nRowSpan==aRows.Count() )
    {
        if( !pRow || nRowSpan > 1 )
            pRow = aRows[nRow+nRowSpan-1];
        if( pRow->HasBottomBorder() )
            nRawHeight -= nBorder;
    }

    ASSERT( nRawHeight > 0, "Zeilenheohe <= 0. OK?" );
    return nRawHeight > 0 ? nRawHeight : 0;
}

BOOL SwWriteTable::ShouldExpandSub(const SwTableBox *pBox, BOOL /*bExpandedBefore*/,
    USHORT nDepth) const
{
    return !pBox->GetSttNd() && nDepth > 0;
}

void SwWriteTable::CollectTableRowsCols( long nStartRPos,
                                           USHORT nStartCPos,
                                           long nParentLineHeight,
                                           USHORT nParentLineWidth,
                                           const SwTableLines& rLines,
                                           USHORT nDepth )
{
    BOOL bSubExpanded = FALSE;
    USHORT nLines = rLines.Count();

#ifdef DBG_UTIL
    USHORT nEndCPos = 0;
#endif

    long nRPos = nStartRPos;
    for( USHORT nLine = 0; nLine < nLines; nLine++ )
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
                ASSERT( FALSE, "Corrupt line height I" );
                nRPos -= nLineHeight;
                nLineHeight = nStartRPos + nParentLineHeight - nRPos; // remaining parent height
                nLineHeight /= nLines - nLine; // divided through the number of remaining sub rows
                nRPos += nLineHeight;
            }
            SwWriteTableRow *pRow = new SwWriteTableRow( nRPos, bUseLayoutHeights);
            USHORT nRow;
            if( aRows.Seek_Entry( pRow, &nRow ) )
                delete pRow;
            else
                aRows.Insert( pRow );
        }
        else
        {
#ifdef DBG_UTIL
            long nCheckPos = nRPos + GetLineHeight( pLine );
#endif
            nRPos = nStartRPos + nParentLineHeight;
#ifdef DBG_UTIL
            SwWriteTableRow aRow( nStartRPos + nParentLineHeight, bUseLayoutHeights );
            ASSERT( aRows.Seek_Entry(&aRow),
                    "Parent-Zeile nicht gefunden" );
            SwWriteTableRow aRowCheckPos(nCheckPos,bUseLayoutHeights);
            SwWriteTableRow aRowRPos(nRPos,bUseLayoutHeights);
            ASSERT( !bUseLayoutHeights ||
                    aRowCheckPos == aRowRPos,
                    "Hoehe der Zeilen stimmt nicht mit Parent ueberein" );
#endif
        }

        // Fuer alle Boxen der Zeile ggf. eine Spalte einfuegen
        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        USHORT nBoxes = rBoxes.Count();

        USHORT nCPos = nStartCPos;
        for( USHORT nBox=0; nBox<nBoxes; nBox++ )
        {
            const SwTableBox *pBox = rBoxes[nBox];

            USHORT nOldCPos = nCPos;

            if( nBox < nBoxes-1 || (nParentLineWidth==0 && nLine==0)  )
            {
                nCPos = nCPos + (USHORT)GetBoxWidth( pBox );
                SwWriteTableCol *pCol = new SwWriteTableCol( nCPos );

                USHORT nCol;
                if( aCols.Seek_Entry( pCol, &nCol ) )
                    delete pCol;
                else
                    aCols.Insert( pCol );

                if( nBox==nBoxes-1 )
                {
                    ASSERT( nLine==0 && nParentLineWidth==0,
                            "Jetzt wird die Parent-Breite plattgemacht!" );
                    nParentLineWidth = nCPos-nStartCPos;
                }
            }
            else
            {
#ifdef DBG_UTIL
                USHORT nCheckPos = nCPos + (USHORT)GetBoxWidth( pBox );
                if( !nEndCPos )
                {
                    nEndCPos = nCheckPos;
                }
                else
                {
                    ASSERT( SwWriteTableCol(nCheckPos) ==
                                                SwWriteTableCol(nEndCPos),
                    "Zelle enthaelt unterschiedlich breite Zeilen" );
                }
#endif
                nCPos = nStartCPos + nParentLineWidth;
#ifdef DBG_UTIL
                SwWriteTableCol aCol( nStartCPos + nParentLineWidth );
                ASSERT( aCols.Seek_Entry(&aCol),
                        "Parent-Zelle nicht gefunden" );
                ASSERT( SwWriteTableCol(nCheckPos) ==
                                            SwWriteTableCol(nCPos),
                        "Breite der Zellen stimmt nicht mit Parent ueberein" );
#endif
            }

            if( ShouldExpandSub( pBox, bSubExpanded, nDepth ) )
            {
                CollectTableRowsCols( nOldRPos, nOldCPos,
                                        nRPos - nOldRPos,
                                        nCPos - nOldCPos,
                                        pBox->GetTabLines(),
                                        nDepth-1 );
                bSubExpanded = TRUE;
            }
        }
    }
}


void SwWriteTable::FillTableRowsCols( long nStartRPos, USHORT nStartRow,
                                        USHORT nStartCPos, USHORT nStartCol,
                                        long nParentLineHeight,
                                        USHORT nParentLineWidth,
                                        const SwTableLines& rLines,
                                        const SvxBrushItem* pParentBrush,
                                        USHORT nDepth,
                                        sal_uInt16 nNumOfHeaderRows )
{
    USHORT nLines = rLines.Count();
    BOOL bSubExpanded = FALSE;

    // Festlegen der Umrandung
    long nRPos = nStartRPos;
    USHORT nRow = nStartRow;

    for( USHORT nLine = 0; nLine < nLines; nLine++ )
    {
        const SwTableLine *pLine = rLines[nLine];

        // Position der letzten ueberdeckten Zeile ermitteln
        long nOldRPos = nRPos;
        if( nLine < nLines-1 || nParentLineHeight==0 )
        {
            long nLineHeight = GetLineHeight( pLine );
            nRPos += nLineHeight;
            if( nParentLineHeight && nStartRPos + nParentLineHeight <= nRPos )
            {
                /* See comment in CollectTableRowCols */
                ASSERT( FALSE, "Corrupt line height II" );
                nRPos -= nLineHeight;
                nLineHeight = nStartRPos + nParentLineHeight - nRPos; // remaining parent height
                nLineHeight /= nLines - nLine; // divided through the number of remaining sub rows
                nRPos += nLineHeight;
            }
        }
        else
            nRPos = nStartRPos + nParentLineHeight;

        // Und ihren Index
        USHORT nOldRow = nRow;
        SwWriteTableRow aRow( nRPos,bUseLayoutHeights );
#ifdef DBG_UTIL
        BOOL bFound =
#endif
            aRows.Seek_Entry( &aRow, &nRow );
        ASSERT( bFound, "Wo ist die Zeile geblieben?" );

        ASSERT( nOldRow <= nRow, "Don't look back!" );
        if( nOldRow > nRow )
        {
            nOldRow = nRow;
            if( nOldRow )
                --nOldRow;
        }


        SwWriteTableRow *pRow = aRows[nOldRow];
        SwWriteTableRow *pEndRow = aRows[nRow];
//      if( nLine==0 && nParentLineHeight==0 )
        if( nLine+1==nNumOfHeaderRows && nParentLineHeight==0 )
            nHeadEndRow = nRow;

        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();

        const SwFrmFmt *pLineFrmFmt = pLine->GetFrmFmt();
        const SfxPoolItem* pItem;
        const SfxItemSet& rItemSet = pLineFrmFmt->GetAttrSet();

        long nHeight = 0;
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_FRM_SIZE, TRUE, &pItem ))
            nHeight = ((SwFmtFrmSize*)pItem)->GetHeight();


        const SvxBrushItem *pBrushItem, *pLineBrush = pParentBrush;
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, FALSE,
                                                   &pItem ) )
        {
            pLineBrush = (const SvxBrushItem *)pItem;

            // Wenn die Zeile die gesamte Tabelle umspannt, koennen
            // Wir den Hintergrund an der Zeile ausgeben. Sonst muessen
            // wir in an den Zelle ausgeben.
            BOOL bOutAtRow = !nParentLineWidth;
            if( !bOutAtRow && nStartCPos==0 )
            {
                USHORT nEndCol;
                SwWriteTableCol aCol( nParentLineWidth );
                bOutAtRow = aCols.Seek_Entry(&aCol,&nEndCol) &&
                            nEndCol == aCols.Count()-1;
            }
            if( bOutAtRow )
            {
                pRow->SetBackground( pLineBrush );
                pBrushItem = 0;
            }
            else
                pBrushItem = pLineBrush;
        }
        else
        {
            pRow->SetBackground( pLineBrush );
            pBrushItem = 0;
        }

        USHORT nBoxes = rBoxes.Count();
        USHORT nCPos = nStartCPos;
        USHORT nCol = nStartCol;

        for( USHORT nBox=0; nBox<nBoxes; nBox++ )
        {
            const SwTableBox *pBox = rBoxes[nBox];

            // Position der letzten ueberdeckten Spalte ermitteln
            USHORT nOldCPos = nCPos;
            if( nBox < nBoxes-1 || (nParentLineWidth==0 && nLine==0) )
            {
                nCPos = nCPos + (USHORT)GetBoxWidth( pBox );
                if( nBox==nBoxes-1 )
                    nParentLineWidth = nCPos - nStartCPos;
            }
            else
                nCPos = nStartCPos + nParentLineWidth;

            // Und ihren Index
            USHORT nOldCol = nCol;
            SwWriteTableCol aCol( nCPos );
#ifdef DBG_UTIL
            BOOL bFound2 =
#endif
                aCols.Seek_Entry( &aCol, &nCol );
            ASSERT( bFound2, "Wo ist die Spalte geblieben?" );

            if( !ShouldExpandSub( pBox, bSubExpanded, nDepth ) )
            {
                USHORT nRowSpan = nRow - nOldRow + 1;

                // The new table model may have true row span attributes
                const long nAttrRowSpan = pBox->getRowSpan();
                if ( 1 < nAttrRowSpan )
                    nRowSpan = (USHORT)nAttrRowSpan;
                else if ( nAttrRowSpan < 1 )
                    nRowSpan = 0;

                USHORT nColSpan = nCol - nOldCol + 1;
                pRow->AddCell( pBox, nOldRow, nOldCol,
                               nRowSpan, nColSpan, nHeight,
                               pBrushItem );
                nHeight = 0; // Die Hoehe braucht nur einmal geschieben werden

                if( pBox->GetSttNd() )
                {
                    USHORT nTopBorder = USHRT_MAX, nBottomBorder = USHRT_MAX;
                    USHORT nBorderMask = MergeBoxBorders(pBox, nOldRow, nOldCol,
                        nRowSpan, nColSpan, nTopBorder, nBottomBorder);

                    // #i30094# add a sanity check here to ensure that
                    // we don't access an invalid aCols[] as &nCol
                    // above can be changed.
                    if (!(nBorderMask & 4) && nOldCol < aCols.Count())
                    {
                        SwWriteTableCol *pCol = aCols[nOldCol];
                        ASSERT(pCol, "No TableCol found, panic!");
                        if (pCol)
                            pCol->bLeftBorder = FALSE;
                    }

                    if (!(nBorderMask & 8))
                    {
                        SwWriteTableCol *pCol = aCols[nCol];
                        ASSERT(pCol, "No TableCol found, panic!");
                        if (pCol)
                            pCol->bRightBorder = FALSE;
                    }

                    if (!(nBorderMask & 1))
                        pRow->bTopBorder = FALSE;
                    else if (!pRow->nTopBorder || nTopBorder < pRow->nTopBorder)
                        pRow->nTopBorder = nTopBorder;

                    if (!(nBorderMask & 2))
                        pEndRow->bBottomBorder = FALSE;
                    else if (
                                !pEndRow->nBottomBorder ||
                                nBottomBorder < pEndRow->nBottomBorder
                            )
                    {
                        pEndRow->nBottomBorder = nBottomBorder;
                    }
                }
//              MIB: 13.12.2000: Why should a cell that contains a subtable
//              not have borders? Moreover, switching them, off switches off
//              the fill border lines between the columns and rows. (#74222#)
//              else
//              {
//                  aCols[nOldCol]->bLeftBorder = FALSE;
//                  aCols[nCol]->bRightBorder = FALSE;
//                  pRow->bTopBorder = FALSE;
//                  pEndRow->bBottomBorder = FALSE;
//              }
            }
            else
            {
                FillTableRowsCols( nOldRPos, nOldRow, nOldCPos, nOldCol,
                                    nRPos-nOldRPos, nCPos-nOldCPos,
                                    pBox->GetTabLines(),
                                    pLineBrush, nDepth-1,
                                    nNumOfHeaderRows );
                bSubExpanded = TRUE;
            }

            nCol++; // Die naechste Zelle faengt in der nachten Spalte an
        }

        nRow++;
    }
}

SwWriteTable::SwWriteTable(const SwTableLines& rLines, long nWidth,
    USHORT nBWidth, BOOL bRel, USHORT nMaxDepth, USHORT nLSub, USHORT nRSub, sal_uInt32 nNumOfRowsToRepeat)
    : nBorderColor((UINT32)-1), nCellSpacing(0), nCellPadding(0), nBorder(0),
    nInnerBorder(0), nBaseWidth(nBWidth), nHeadEndRow(USHRT_MAX),
     nLeftSub(nLSub), nRightSub(nRSub), nTabWidth(nWidth), bRelWidths(bRel),
    bUseLayoutHeights(true),
#ifdef DBG_UTIL
    bGetLineHeightCalled(false),
#endif
    bColsOption(false), bColTags(true), bLayoutExport(false),
    bCollectBorderWidth(true)
{
    USHORT nParentWidth = nBaseWidth + nLeftSub + nRightSub;

    // Erstmal die Tabellen-Struktur festlegen. Hinter der Tabelle ist in
    // jedem Fall eine Spalte zu Ende
    SwWriteTableCol *pCol = new SwWriteTableCol( nParentWidth );
    aCols.Insert( pCol );
    CollectTableRowsCols( 0, 0, 0, nParentWidth, rLines, nMaxDepth - 1 );

    // Und jetzt mit leben fuellen
    FillTableRowsCols( 0, 0, 0, 0, 0, nParentWidth, rLines, 0, nMaxDepth - 1, static_cast< sal_uInt16 >(nNumOfRowsToRepeat) );

    // Einige Twip-Werte an Pixel-Grenzen anpassen
    if( !nBorder )
        nBorder = nInnerBorder;
}

SwWriteTable::SwWriteTable( const SwHTMLTableLayout *pLayoutInfo )
    : nBorderColor((UINT32)-1), nCellSpacing(0), nCellPadding(0), nBorder(0),
    nInnerBorder(0), nBaseWidth(pLayoutInfo->GetWidthOption()), nHeadEndRow(0),
    nLeftSub(0), nRightSub(0), nTabWidth(pLayoutInfo->GetWidthOption()),
    bRelWidths(pLayoutInfo->HasPrcWidthOption()), bUseLayoutHeights(false),
#ifdef DBG_UTIL
    bGetLineHeightCalled(false),
#endif
    bColsOption(pLayoutInfo->HasColsOption()),
    bColTags(pLayoutInfo->HasColTags()), bLayoutExport(true),
    bCollectBorderWidth(pLayoutInfo->HaveBordersChanged())
{
    if( !bCollectBorderWidth )
    {
        nBorder = pLayoutInfo->GetBorder();
        nCellPadding = pLayoutInfo->GetCellPadding();
        nCellSpacing = pLayoutInfo->GetCellSpacing();
    }

    USHORT nRow, nCol;
    USHORT nCols = pLayoutInfo->GetColCount();
    USHORT nRows = pLayoutInfo->GetRowCount();

    // Erstmal die Tabellen-Struktur festlegen.
    for( nCol=0; nCol<nCols; nCol++ )
    {
        SwWriteTableCol *pCol =
            new SwWriteTableCol( (nCol+1)*COL_DFLT_WIDTH );

        if( bColTags )
        {
            const SwHTMLTableLayoutColumn *pLayoutCol =
                pLayoutInfo->GetColumn( nCol );
            pCol->SetWidthOpt( pLayoutCol->GetWidthOption(),
                               pLayoutCol->IsRelWidthOption() );
        }

        aCols.Insert( pCol );
    }

    for( nRow=0; nRow<nRows; nRow++ )
    {
        SwWriteTableRow *pRow =
            new SwWriteTableRow( (nRow+1)*ROW_DFLT_HEIGHT, bUseLayoutHeights );
        pRow->nTopBorder = 0;
        pRow->nBottomBorder = 0;
        aRows.Insert( pRow );
    }

    // Und jetzt mit leben fuellen
    for( nRow=0; nRow<nRows; nRow++ )
    {
        SwWriteTableRow *pRow = aRows[nRow];

        BOOL bHeightExported = FALSE;
        for( nCol=0; nCol<nCols; nCol++ )
        {
            const SwHTMLTableLayoutCell *pLayoutCell =
                pLayoutInfo->GetCell( nRow, nCol );

            const SwHTMLTableLayoutCnts *pLayoutCnts =
                pLayoutCell->GetContents();

            // Beginnt die Zelle eigentlich eine Zeile weiter oben oder
            // weiter vorne?
            if( ( nRow>0 && pLayoutCnts == pLayoutInfo->GetCell(nRow-1,nCol)
                                                      ->GetContents() ) ||
                ( nCol>0 && pLayoutCnts == pLayoutInfo->GetCell(nRow,nCol-1)
                                                      ->GetContents() ) )
            {
                continue;
            }

            USHORT nRowSpan = pLayoutCell->GetRowSpan();
            USHORT nColSpan = pLayoutCell->GetColSpan();
            const SwTableBox *pBox = pLayoutCnts->GetTableBox();
            ASSERT( pBox,
                    "Tabelle in Tabelle kann nicht ueber Layout exportiert werden" );

            long nHeight = bHeightExported ? 0 : GetLineHeight( pBox );
            const SvxBrushItem *pBrushItem = GetLineBrush( pBox, pRow );

            SwWriteTableCell *pCell =
                pRow->AddCell( pBox, nRow, nCol, nRowSpan, nColSpan,
                               nHeight, pBrushItem );
            pCell->SetWidthOpt( pLayoutCell->GetWidthOption(),
                                pLayoutCell->IsPrcWidthOption() );

            USHORT nTopBorder = USHRT_MAX, nBottomBorder = USHRT_MAX;
            USHORT nBorderMask =
            MergeBoxBorders( pBox, nRow, nCol, nRowSpan, nColSpan,
                                nTopBorder, nBottomBorder );

            SwWriteTableCol *pCol = aCols[nCol];
            if( !(nBorderMask & 4) )
                pCol->bLeftBorder = FALSE;

            pCol = aCols[nCol+nColSpan-1];
            if( !(nBorderMask & 8) )
                pCol->bRightBorder = FALSE;

            if( !(nBorderMask & 1) )
                pRow->bTopBorder = FALSE;

            SwWriteTableRow *pEndRow = aRows[nRow+nRowSpan-1];
            if( !(nBorderMask & 2) )
                pEndRow->bBottomBorder = FALSE;

            // Die Hoehe braucht nur einmal geschieben werden
            if( nHeight )
                bHeightExported = TRUE;
        }
    }

    // Einige Twip-Werte an Pixel-Grenzen anpassen
    if( bCollectBorderWidth && !nBorder )
        nBorder = nInnerBorder;
}

SwWriteTable::~SwWriteTable()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
