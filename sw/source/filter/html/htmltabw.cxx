/*************************************************************************
 *
 *  $RCSfile: htmltabw.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:57:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XIMAGEPRODUCERSUPPLIER_HPP_
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLERLISTENER_HPP_
#include <com/sun/star/form/XFormControllerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_CONTAINEREVENT_HPP_
#include <com/sun/star/container/ContainerEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _HTMLTBL_HXX
#include <htmltbl.hxx>
#endif
#ifndef _HTMLNUM_HXX
#include <htmlnum.hxx>
#endif
#ifndef _WRTHTML_HXX
#include <wrthtml.hxx>
#endif
#ifndef _WRTSWTBL_HXX
#include <wrtswtbl.hxx>
#endif



#ifndef PRODUCT
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#endif


//#define MAX_DEPTH (USHRT_MAX)
#define MAX_DEPTH (3)


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
    SwHTMLWrtTable( const SwTableLines& rLines, long nWidth, sal_uInt16 nBWidth,
                    sal_Bool bRel, sal_uInt16 nLeftSub=0, sal_uInt16 nRightSub=0 );
    SwHTMLWrtTable( const SwHTMLTableLayout *pLayoutInfo );

    void Write( SwHTMLWriter& rWrt, SwHoriOrient eAlign=HORI_NONE,
                sal_Bool bTHead=sal_False, const SwFrmFmt *pFrmFmt=0,
                const String *pCaption=0, sal_Bool bTopCaption=sal_False,
                sal_uInt16 nHSpace=0, sal_uInt16 nVSpace=0 ) const;
};


SwHTMLWrtTable::SwHTMLWrtTable( const SwTableLines& rLines, long nWidth,
                                sal_uInt16 nBWidth, sal_Bool bRel,
                                sal_uInt16 nLSub, sal_uInt16 nRSub )
    : SwWriteTable( rLines, nWidth, nBWidth, bRel, MAX_DEPTH, nLSub, nRSub )
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
    ASSERT( bTop || bBottom || bLeft || bRight,
            "HasTabBackground: darf nicht aufgerufen werden" );

    sal_Bool bRet = sal_False;
    if( rBox.GetSttNd() )
    {
        const SvxBrushItem& rBrushItem =
            rBox.GetFrmFmt()->GetBackground();

        /// OD 02.09.2002 #99657#
        /// The table box has a background, if its background color is not "no fill"/
        /// "auto fill" or it has a background graphic.
        bRet = rBrushItem.GetColor() != COL_TRANSPARENT ||
               rBrushItem.GetGraphicLink() || rBrushItem.GetGraphic();
    }
    else
    {
        const SwTableLines& rLines = rBox.GetTabLines();
        sal_uInt16 nCount = rLines.Count();
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
    ASSERT( bTop || bBottom || bLeft || bRight,
            "HasTabBackground: darf nicht aufgerufen werden" );

    sal_Bool bRet = sal_False;
    const SvxBrushItem& rBrushItem = rLine.GetFrmFmt()->GetBackground();
    /// OD 02.09.2002 #99657#
    /// The table line has a background, if its background color is not "no fill"/
    /// "auto fill" or it has a background graphic.
    bRet = rBrushItem.GetColor() != COL_TRANSPARENT ||
           rBrushItem.GetGraphicLink() || rBrushItem.GetGraphic();

    if( !bRet )
    {
        const SwTableBoxes& rBoxes = rLine.GetTabBoxes();
        sal_uInt16 nCount = rBoxes.Count();
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

sal_Bool lcl_WrtHTMLTbl_HasTabBorders( const SwTableLine*& rpLine, void* pPara );

sal_Bool lcl_WrtHTMLTbl_HasTabBorders( const SwTableBox*& rpBox, void* pPara )
{
    sal_Bool *pBorders = (sal_Bool *)pPara;
    if( *pBorders )
        return sal_False;

    if( !rpBox->GetSttNd() )
    {
        ((SwTableBox *)rpBox)->GetTabLines().ForEach(
                                &lcl_WrtHTMLTbl_HasTabBorders, pPara );
    }
    else
    {
        const SvxBoxItem& rBoxItem =
            (const SvxBoxItem&)rpBox->GetFrmFmt()->GetAttr( RES_BOX );

        *pBorders = rBoxItem.GetTop() || rBoxItem.GetBottom() ||
                    rBoxItem.GetLeft() || rBoxItem.GetRight();
    }

    return !*pBorders;
}

sal_Bool lcl_WrtHTMLTbl_HasTabBorders( const SwTableLine*& rpLine, void* pPara )
{
    sal_Bool *pBorders = (sal_Bool *)pPara;
    if( *pBorders )
        return sal_False;

    ((SwTableLine *)rpLine)->GetTabBoxes().ForEach(
                                    &lcl_WrtHTMLTbl_HasTabBorders, pPara );
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
        lcl_WrtHTMLTbl_HasTabBorders( pBox, &bBorders );
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

    SwWriteTableCol *pCol = aCols[nCol];

//  sal_Bool bOutWidth = nColSpan>1 || pCol->GetOutWidth();
    sal_Bool bOutWidth = sal_True; //nColSpan==1 && pCol->GetOutWidth();

    const SwStartNode* pSttNd = pBox->GetSttNd();
    sal_Bool bHead = sal_False;
    if( pSttNd )
    {
        sal_uInt32 nNdPos = pSttNd->GetIndex()+1;

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
    ByteString sOut( '<' );
    sOut += (bHead ? sHTML_tableheader : sHTML_tabledata );

    // ROW- und COLSPAN ausgeben
    if( nRowSpan>1 )
        (((sOut += ' ' ) += sHTML_O_rowspan ) += '=')
            += ByteString::CreateFromInt32( nRowSpan );
    if( nColSpan > 1 )
        (((sOut += ' ' ) += sHTML_O_colspan ) += '=')
            += ByteString::CreateFromInt32( nColSpan );

#ifndef PURE_HTML
    long nWidth = 0;
    sal_uInt16 nPrcWidth = USHRT_MAX;
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
                    bOutWidth = sal_False;
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
        ((sOut += ' ' ) += sHTML_O_width ) += '=';
        if( nPrcWidth != USHRT_MAX )
            (sOut += ByteString::CreateFromInt32(nPrcWidth)) += '%';
        else
            sOut += ByteString::CreateFromInt32(aPixelSz.Width());
        if( !bLayoutExport && nColSpan==1 )
            pCol->SetOutWidth( sal_False );
    }

    if( nHeight )
    {
        (((sOut += ' ') += sHTML_O_height) += '=')
            += ByteString::CreateFromInt32(aPixelSz.Height());
    }
#endif

    const SfxItemSet& rItemSet = pBox->GetFrmFmt()->GetAttrSet();
    const SfxPoolItem *pItem;

    // ALIGN wird jetzt nur noch an den Absaetzen ausgegeben

    // VALIGN ausgeben
    if( bOutVAlign )
    {
        SwVertOrient eVertOri = pCell->GetVertOri();
        if( VERT_TOP==eVertOri || VERT_BOTTOM==eVertOri )
        {
            (((sOut += ' ') += sHTML_O_valign) += '=')
                += (VERT_TOP==eVertOri ? sHTML_VA_top : sHTML_VA_bottom);
        }
    }

    rWrt.Strm() << sOut.GetBuffer();
    sOut.Erase();

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
        String aDummy;
        rWrt.OutBackground( pBrushItem, aDummy, sal_False );

        if( rWrt.bCfgOutStyles )
            OutCSS1_TableBGStyleOpt( rWrt, *pBrushItem );
    }

    sal_uInt32 nNumFmt;
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
        sOut = HTMLOutFuncs::CreateTableDataOptionsValNum( sOut,
                    bValue, nValue, nNumFmt, *rWrt.pDoc->GetNumberFormatter(),
                    rWrt.eDestEnc, &rWrt.aNonConvertableCharacters );
    sOut += '>';
    rWrt.Strm() << sOut.GetBuffer();
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
        sal_uInt16 nTWidth, nBWidth, nLSub, nRSub;
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
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), bHead ? sHTML_tableheader
                                                     : sHTML_tabledata,
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
    SwVertOrient eRowVertOri = VERT_NONE;
    if( rCells.Count() > 1 )
    {
        for( sal_uInt16 nCell = 0; nCell<rCells.Count(); nCell++ )
        {
            SwVertOrient eCellVertOri = rCells[nCell]->GetVertOri();
            if( 0==nCell )
            {
                eRowVertOri = eCellVertOri;
            }
            else if( eRowVertOri != eCellVertOri )
            {
                eRowVertOri = VERT_NONE;
                break;
            }
        }
    }

    rWrt.OutNewLine();  // <TR> in neuer Zeile
    rWrt.Strm() << '<' << sHTML_tablerow;
    if( pBrushItem )
    {
        String aDummy;
        rWrt.OutBackground( pBrushItem, aDummy, sal_False );

        rWrt.bTxtAttr = sal_False;
        rWrt.bOutOpts = sal_True;
        if( rWrt.bCfgOutStyles )
            OutCSS1_TableBGStyleOpt( rWrt, *pBrushItem );
    }

    if( VERT_TOP==eRowVertOri || VERT_BOTTOM==eRowVertOri )
    {
        ByteString sOut( ' ' );
        ((sOut += sHTML_O_valign) += '=')
            += (VERT_TOP==eRowVertOri ? sHTML_VA_top : sHTML_VA_bottom);
        rWrt.Strm() << sOut.GetBuffer();
    }

    rWrt.Strm() << '>';

    rWrt.IncIndentLevel(); // Inhalt von <TR>...</TR> einruecken

    for( sal_uInt16 nCell = 0; nCell<rCells.Count(); nCell++ )
        OutTableCell( rWrt, rCells[nCell], VERT_NONE==eRowVertOri );

    rWrt.DecIndentLevel(); // Inhalt von <TR>...</TR> einruecken

    rWrt.OutNewLine();  // </TR> in neuer Zeile
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_tablerow, sal_False );
}



void SwHTMLWrtTable::Write( SwHTMLWriter& rWrt, SwHoriOrient eAlign,
                            sal_Bool bTHead, const SwFrmFmt *pFrmFmt,
                            const String *pCaption, sal_Bool bTopCaption,
                            sal_uInt16 nHSpace, sal_uInt16 nVSpace ) const
{
    sal_uInt16 nRow;

    // Wert fuer FRAME bestimmen
    sal_uInt16 nFrameMask = 15;
    if( !(aRows[0])->bTopBorder )
        nFrameMask &= ~1;
    if( !(aRows[aRows.Count()-1])->bBottomBorder )
        nFrameMask &= ~2;
    if( !(aCols[0])->bLeftBorder )
        nFrameMask &= ~4;
    if( !(aCols[aCols.Count()-1])->bRightBorder )
        nFrameMask &= ~8;

    // Wert fur RULES bestimmen
    sal_Bool bRowsHaveBorder = sal_False;
    sal_Bool bRowsHaveBorderOnly = sal_True;
    SwWriteTableRow *pRow = aRows[0];
    for( nRow=1; nRow < aRows.Count(); nRow++ )
    {
        SwWriteTableRow *pNextRow = aRows[nRow];
        sal_Bool bBorder = ( pRow->bBottomBorder || pNextRow->bTopBorder );
        bRowsHaveBorder |= bBorder;
        bRowsHaveBorderOnly &= bBorder;

        sal_uInt16 nBorder = pRow->bBottomBorder ? pRow->nBottomBorder : USHRT_MAX;
        if( pNextRow->bTopBorder && pNextRow->nTopBorder < nBorder )
            nBorder = pNextRow->nTopBorder;

        pRow->bBottomBorder = bBorder;
        pRow->nBottomBorder = nBorder;

        pNextRow->bTopBorder = bBorder;
        pNextRow->nTopBorder = nBorder;

        pRow = pNextRow;
    }

    sal_Bool bColsHaveBorder = sal_False;
    sal_Bool bColsHaveBorderOnly = sal_True;
    SwWriteTableCol *pCol = aCols[0];
    sal_uInt16 nCol;
    for( nCol=1; nCol<aCols.Count(); nCol++ )
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
    ByteString sOut( '<' );
    sOut += sHTML_table;

    sal_uInt16 nOldDirection = rWrt.nDirection;
    if( pFrmFmt )
        rWrt.nDirection = rWrt.GetHTMLDirection( pFrmFmt->GetAttrSet() );
    if( rWrt.bOutFlyFrame || nOldDirection != rWrt.nDirection )
    {
        rWrt.Strm() << sOut.GetBuffer();
        sOut.Erase();
        rWrt.OutDirection( rWrt.nDirection );
    }

    // COLS ausgeben: Nur bei Export ueber Layout, wenn es beim Import
    // vorhanden war.
    if( bColsOption )
        (((sOut += ' ') += sHTML_O_cols) += '=')
            += ByteString::CreateFromInt32( aCols.Count() );

    // ALIGN= ausgeben
    if( HORI_RIGHT == eAlign )
        (((sOut += ' ') += sHTML_O_align ) += '=') += sHTML_AL_right;
    else if( HORI_CENTER == eAlign )
        (((sOut += ' ') += sHTML_O_align ) += '=') += sHTML_AL_center;
    else if( HORI_LEFT == eAlign )
        (((sOut += ' ') += sHTML_O_align ) += '=') += sHTML_AL_left;

    // WIDTH ausgeben: Stammt aus Layout oder ist berechnet
    if( nTabWidth )
    {
        ((sOut += ' ') += sHTML_O_width ) += '=';
        if( HasRelWidths() )
            (sOut += ByteString::CreateFromInt32( nTabWidth )) += '%';
        else if( Application::GetDefaultDevice() )
        {
            long nPixWidth = Application::GetDefaultDevice()->LogicToPixel(
                        Size(nTabWidth,0), MapMode(MAP_TWIP) ).Width();
            if( !nPixWidth )
                nPixWidth = 1;

            sOut += ByteString::CreateFromInt32( nPixWidth );
        }
        else
        {
            ASSERT( Application::GetDefaultDevice(), "kein Application-Window!?" );
            sOut += "100%";
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
            (((sOut += ' ') += sHTML_O_hspace) += '=')
                += ByteString::CreateFromInt32( aPixelSpc.Width() );
        }

        if( aPixelSpc.Height() )
        {
            (((sOut += ' ') += sHTML_O_vspace) += '=')
                += ByteString::CreateFromInt32( aPixelSpc.Height() );
        }
    }

    // BORDER ausgeben, aber nur wenn wir die Umrandung selbst berechnet
    // haben oder die Umrandung 0 ist oder es irgendwelche Umrandungen gibt.
    // Anderenfalls enthaelt nBorder naemlich nur die Breite der Umrandung,
    // die genutzt wird, wenn gar kein ::com::sun::star::sheet::Border angegeben ist.
    sal_Bool bHasAnyBorders = nFrameMask || bColsHaveBorder || bRowsHaveBorder;
    if( bCollectBorderWidth || nBorder==0 || bHasAnyBorders )
        (((sOut += ' ' ) += sHTML_O_border ) += '=')
            += ByteString::CreateFromInt32( rWrt.ToPixel( nBorder ) );

    // BORDERCOLOR ausgeben

    if( (sal_uInt32)-1 != nBorderColor && rWrt.bCfgOutStyles && bHasAnyBorders )
    {
        ((sOut += ' ' ) += sHTML_O_bordercolor ) += '=';
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_Color( rWrt.Strm(), nBorderColor, rWrt.eDestEnc );
        sOut.Erase();
    }

    // CELLPADDING ausgeben: Stammt aus Layout oder ist berechnet
    (((sOut += ' ' ) += sHTML_O_cellpadding ) += '=')
        += ByteString::CreateFromInt32( rWrt.ToPixel( nCellPadding ) );

    // CELLSPACING ausgeben: Stammt aus Layout oder ist berechnet
    (((sOut += ' ' ) += sHTML_O_cellspacing ) += '=')
        += ByteString::CreateFromInt32( rWrt.ToPixel( nCellSpacing ) );

    // FRAME/RULES ausgeben (nur sinnvoll, wenn border!=0)
    if( nBorder!=0 && (bCollectBorderWidth || bHasAnyBorders) )
    {
        const sal_Char *pFrame = 0;
        switch( nFrameMask )
        {
            case 0:  pFrame = sHTML_TF_void     ;break;
            case 1:  pFrame = sHTML_TF_above    ;break;
            case 2:  pFrame = sHTML_TF_below    ;break;
            case 3:  pFrame = sHTML_TF_hsides   ;break;
            case 4:  pFrame = sHTML_TF_lhs      ;break;
            case 8:  pFrame = sHTML_TF_rhs      ;break;
            case 12: pFrame = sHTML_TF_vsides   ;break;
            //FRAME=BOX ist der default wenn BORDER>0
            //case 15:
            //default: pFrame = sHTML_TF_box        ;break; // geht nicht
        };
        if( pFrame )
            (((sOut += ' ' ) += sHTML_O_frame ) += '=') += pFrame;

        const sal_Char *pRules = 0;
        if( aCols.Count() > 1 && aRows.Count() > 1 )
        {
            if( !bColsHaveBorder )
            {
                if( !bRowsHaveBorder )
                    pRules = sHTML_TR_none;
                else if( bRowsHaveBorderOnly )
                    pRules = sHTML_TR_rows;
                else
                    pRules = sHTML_TR_groups;
            }
            else if( bColsHaveBorderOnly )
            {
                if( !bRowsHaveBorder || !bRowsHaveBorderOnly )
                    pRules = sHTML_TR_cols;
            }
            else
            {
                if( !bRowsHaveBorder )
                    pRules = sHTML_TR_groups;
                else if( bRowsHaveBorderOnly )
                    pRules = sHTML_TR_rows;
                else
                    pRules = sHTML_TR_groups;
            }
        }
        else if( aRows.Count() > 1 )
        {
            if( !bRowsHaveBorder )
                pRules = sHTML_TR_none;
            else if( !bRowsHaveBorderOnly )
                pRules = sHTML_TR_groups;
        }
        else if( aCols.Count() > 1 )
        {
            if( !bColsHaveBorder )
                pRules = sHTML_TR_none;
            else if( !bColsHaveBorderOnly )
                pRules = sHTML_TR_groups;
        }

        if( pRules )
            (((sOut += ' ' ) += sHTML_O_rules ) += '=') += pRules;
    }
    rWrt.Strm() << sOut.GetBuffer();

    // Hintergrund ausgeben
    if( pFrmFmt )
    {
        String aDummy;
        rWrt.OutBackground( pFrmFmt->GetAttrSet(), aDummy, sal_False );

        if( rWrt.bCfgOutStyles && pFrmFmt )
            rWrt.OutCSS1_TableFrmFmtOptions( *pFrmFmt );
    }

    sOut = '>';
    rWrt.Strm() << sOut.GetBuffer();

    rWrt.IncIndentLevel(); // Inhalte von Table einruecken

    // Ueberschrift ausgeben
    if( pCaption && pCaption->Len() )
    {
        rWrt.OutNewLine(); // <CAPTION> in neue Zeile
        ByteString sOut( sHTML_caption );
        (((sOut += ' ') += sHTML_O_align) += '=')
            += (bTopCaption ? sHTML_VA_top : sHTML_VA_bottom);
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sOut.GetBuffer(), sal_True );
        HTMLOutFuncs::Out_String( rWrt.Strm(), *pCaption, rWrt.eDestEnc, &rWrt.aNonConvertableCharacters    );
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_caption, sal_False );
    }

    sal_uInt16 nCols = aCols.Count();

    // <COLGRP>/<COL> ausgeben: Bei Export ueber Layout nur wenn beim
    // Import welche da waren, sonst immer.
    sal_Bool bColGroups = (bColsHaveBorder && !bColsHaveBorderOnly);
    if( bColTags )
    {
        if( bColGroups )
        {
            rWrt.OutNewLine(); // <COLGRP> in neue Zeile
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_colgroup, sal_True );

            rWrt.IncIndentLevel(); // Inhalt von <COLGRP> einruecken
        }

        for( nCol=0; nCol<nCols; nCol++ )
        {
            rWrt.OutNewLine(); // <COL> in neue Zeile

            const SwWriteTableCol *pCol = aCols[nCol];

            ByteString sOut( '<' );
            sOut += sHTML_col;

            sal_uInt16 nWidth;
            sal_Bool bRel;
            if( bLayoutExport )
            {
                bRel = pCol->HasRelWidthOpt();
                nWidth = pCol->GetWidthOpt();
            }
            else
            {
                bRel = HasRelWidths();
                nWidth = bRel ? GetRelWidth(nCol,1) : GetAbsWidth(nCol,1);
            }

            ((sOut += ' ' ) += sHTML_O_width ) += '=';
            if( bRel )
            {
                (sOut += ByteString::CreateFromInt32( nWidth ) ) += '*';
            }
            else
            {
                sOut += ByteString::CreateFromInt32( rWrt.ToPixel( nWidth ) );
            }
            sOut += '>';
            rWrt.Strm() << sOut.GetBuffer();

            if( bColGroups && pCol->bRightBorder && nCol<nCols-1 )
            {
                rWrt.DecIndentLevel(); // Inhalt von <COLGRP> einruecken
                rWrt.OutNewLine(); // </COLGRP> in neue Zeile
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_colgroup,
                                            sal_False );
                rWrt.OutNewLine(); // <COLGRP> in neue Zeile
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_colgroup,
                                            sal_True );
                rWrt.IncIndentLevel(); // Inhalt von <COLGRP> einruecken
            }
        }
        if( bColGroups )
        {
            rWrt.DecIndentLevel(); // Inhalt von <COLGRP> einruecken

            rWrt.OutNewLine(); // </COLGRP> in neue Zeile
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_colgroup,
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
        nHeadEndRow<aRows.Count()-1 && !aRows[nHeadEndRow]->bBottomBorder )
        bTHead = sal_False;

    // <TBODY> aus ausgeben, wenn <THEAD> ausgegeben wird.
    bTSections |= bTHead;

    if( bTSections )
    {
        rWrt.OutNewLine(); // <THEAD>/<TDATA> in neue Zeile
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? sHTML_thead : sHTML_tbody, sal_True );

        rWrt.IncIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.
    }

    for( nRow = 0; nRow < aRows.Count(); nRow++ )
    {
        const SwWriteTableRow *pRow = aRows[nRow];
        const SwWriteTableCells& rCells = pRow->GetCells();

        OutTableCells( rWrt, pRow->GetCells(), pRow->GetBackground() );
        if( !nCellSpacing && nRow < aRows.Count()-1 && pRow->bBottomBorder &&
            pRow->nBottomBorder > DEF_LINE_WIDTH_1 )
        {
            sal_uInt16 nCnt = (pRow->nBottomBorder / DEF_LINE_WIDTH_1) - 1;
            for( ; nCnt; nCnt-- )
            {
                rWrt.OutNewLine();
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_tablerow,
                                            sal_True );
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_tablerow,
                                            sal_False );
            }
        }
        if( ( (bTHead && nRow==nHeadEndRow) ||
              (bTBody && pRow->bBottomBorder) ) &&
            nRow < aRows.Count()-1 )
        {
            rWrt.DecIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.
            rWrt.OutNewLine(); // </THEAD>/</TDATA> in neue Zeile
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? sHTML_thead : sHTML_tbody, sal_False );
            rWrt.OutNewLine(); // <THEAD>/<TDATA> in neue Zeile

            if( bTHead && nRow==nHeadEndRow )
                bTHead = sal_False;

            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? sHTML_thead : sHTML_tbody, sal_True );
            rWrt.IncIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.
        }
    }

    if( bTSections )
    {
        rWrt.DecIndentLevel(); // Inhalt von <THEAD>/<TDATA> einr.

        rWrt.OutNewLine(); // </THEAD>/</TDATA> in neue Zeile
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                            bTHead ? sHTML_thead : sHTML_tbody, sal_False );
    }

    rWrt.DecIndentLevel(); // Inhalt von <TABLE> einr.

    rWrt.OutNewLine(); // </TABLE> in neue Zeile
    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_table, sal_False );

    rWrt.nDirection = nOldDirection;
}

Writer& OutHTML_SwTblNode( Writer& rWrt, SwTableNode & rNode,
                           const SwFrmFmt *pFlyFrmFmt,
                           const String *pCaption, sal_Bool bTopCaption )
{

    SwTable& rTbl = rNode.GetTable();

    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;
    rHTMLWrt.bOutTable = sal_True;

    // die horizontale Ausrichtung des Rahmens hat (falls vorhanden)
    // Prioritaet. NONE bedeutet, dass keine horizontale
    // Ausrichtung geschrieben wird.
    SwHoriOrient eFlyHoriOri = HORI_NONE;
    SwSurround eSurround = SURROUND_NONE;
    sal_uInt8 nFlyPrcWidth = 0;
    long nFlyWidth;
    sal_uInt16 nFlyHSpace = 0;
    sal_uInt16 nFlyVSpace = 0;
    if( pFlyFrmFmt )
    {
        eSurround = pFlyFrmFmt->GetSurround().GetSurround();
        const SwFmtFrmSize& rFrmSize = pFlyFrmFmt->GetFrmSize();
        nFlyPrcWidth = rFrmSize.GetWidthPercent();
        nFlyWidth = rFrmSize.GetSize().Width();

        eFlyHoriOri = pFlyFrmFmt->GetHoriOrient().GetHoriOrient();
        if( HORI_NONE == eFlyHoriOri )
            eFlyHoriOri = HORI_LEFT;

        const SvxLRSpaceItem& rLRSpace = pFlyFrmFmt->GetLRSpace();
        nFlyHSpace = (rLRSpace.GetLeft() + rLRSpace.GetRight()) / 2;

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

    SwHoriOrient eTabHoriOri = pFmt->GetHoriOrient().GetHoriOrient();

    // HORI_NONE und HORI_FULL Tabellen benoetigen relative Breiten
    sal_uInt16 nNewDefListLvl = 0;
    sal_Bool bRelWidths = sal_False;
    sal_Bool bCheckDefList = sal_False;
    switch( eTabHoriOri )
    {
    case HORI_FULL:
        // Tabellen mit automatischer Ausrichtung werden zu Tabellen
        // mit 100%-Breite
        bRelWidths = sal_True;
        nWidth = 100;
        eTabHoriOri = HORI_LEFT;
        break;
    case HORI_NONE:
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
                    bRelWidths = sal_True;
                    nWidth = 100;
                }

            }
            else if( nPrcWidth  )
            {
                // Ohne rechten Rand bleibt die %-Breite erhalten
                nWidth = nPrcWidth;
                bRelWidths = sal_True;
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
    case HORI_LEFT_AND_WIDTH:
        eTabHoriOri = HORI_LEFT;
        bCheckDefList = sal_True;
        // no break
    default:
        // In allen anderen Faellen kann eine absolute oder relative
        // Breite direkt uebernommen werden.
        if( nPrcWidth )
        {
            bRelWidths = sal_True;
            nWidth = nPrcWidth;
        }
        break;
    }

    if( bCheckDefList )
    {
        ASSERT( !rHTMLWrt.GetNumInfo().GetNumRule() ||
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
            nNewDefListLvl =
                (aLRItem.GetLeft() + (rHTMLWrt.nDefListMargin/2)) /
                rHTMLWrt.nDefListMargin;
        }
    }

    if( !pFlyFrmFmt && nNewDefListLvl != rHTMLWrt.nDefListLvl )
        rHTMLWrt.OutAndSetDefList( nNewDefListLvl );

    if( nNewDefListLvl )
    {
        if( rHTMLWrt.bLFPossible )
            rHTMLWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_dd );
    }

    // eFlyHoriOri und eTabHoriOri besitzen nun nur noch die Werte
    // LEFT/CENTER und RIGHT!
    if( eFlyHoriOri!=HORI_NONE )
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

    SwHoriOrient eDivHoriOri = HORI_NONE;
    switch( eTabHoriOri )
    {
    case HORI_LEFT:
        // Wenn eine linksbuendigeTabelle keinen rechtsseiigen Durchlauf
        // hat, brauchen wir auch kein ALIGN=LEFT in der Tabelle.
        if( eSurround==SURROUND_NONE || eSurround==SURROUND_LEFT )
            eTabHoriOri = HORI_NONE;
        break;
    case HORI_RIGHT:
        // Aehnliches gilt fuer rechtsbuendigeTabelle, hier nehmen wir
        // stattdessen ein <DIV ALIGN=RIGHT>.
        if( eSurround==SURROUND_NONE || eSurround==SURROUND_RIGHT )
        {
            eDivHoriOri = HORI_RIGHT;
            eTabHoriOri = HORI_NONE;
        }
        break;
    case HORI_CENTER:
        // ALIGN=CENTER versteht so gut wie keiner, deshalb verzichten wir
        // daruf und nehmen ein <CENTER>.
        eDivHoriOri = HORI_CENTER;
        eTabHoriOri = HORI_NONE;
        break;
    }
    if( HORI_NONE==eTabHoriOri )
        nFlyHSpace = nFlyVSpace = 0;

    if( pFmt->GetName().Len() )
        rHTMLWrt.OutImplicitMark( pFmt->GetName(), pMarkToTable );

    if( HORI_NONE!=eDivHoriOri )
    {
        if( rHTMLWrt.bLFPossible )
            rHTMLWrt.OutNewLine();  // <CENTER> in neuer Zeile
        if( HORI_CENTER==eDivHoriOri )
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_center, sal_True );
        else
        {
            ByteString sOut( sHTML_division );
            (((sOut += ' ') += sHTML_O_align) += '=') += sHTML_AL_right;
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sOut.GetBuffer(),
                                        sal_True );
        }
        rHTMLWrt.IncIndentLevel();  // Inhalt von <CENTER> einruecken
        rHTMLWrt.bLFPossible = sal_True;
    }

    // Wenn die Tabelle in keinem Rahmen ist kann man immer ein LF ausgeben.
    if( HORI_NONE==eTabHoriOri )
        rHTMLWrt.bLFPossible = sal_True;

    const SwHTMLTableLayout *pLayout = rTbl.GetHTMLTableLayout();

#ifndef PRODUCT
    ViewShell *pSh;
    rWrt.pDoc->GetEditShell( &pSh );
    if ( pSh && pSh->GetViewOptions()->IsTest1() )
        pLayout = 0;
#endif

    if( pLayout && pLayout->IsExportable() )
    {
        SwHTMLWrtTable aTableWrt( pLayout );
        aTableWrt.Write( rHTMLWrt, eTabHoriOri, rTbl.IsHeadlineRepeat(),
                         pFmt, pCaption, bTopCaption,
                         nFlyHSpace, nFlyVSpace );
    }
    else
    {
        SwHTMLWrtTable aTableWrt( rTbl.GetTabLines(), nWidth,
                                  nBaseWidth, bRelWidths );
        aTableWrt.Write( rHTMLWrt, eTabHoriOri, rTbl.IsHeadlineRepeat(),
                         pFmt, pCaption, bTopCaption,
                         nFlyHSpace, nFlyVSpace );
    }

    // Wenn die Tabelle in keinem Rahmen war kann man immer ein LF ausgeben.
    if( HORI_NONE==eTabHoriOri )
        rHTMLWrt.bLFPossible = sal_True;

    if( HORI_NONE!=eDivHoriOri )
    {
        rHTMLWrt.DecIndentLevel();  // Inhalt von <CENTER> einruecken
        rHTMLWrt.OutNewLine();      // </CENTER> in neue Teile
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                               HORI_CENTER==eDivHoriOri ? sHTML_center
                                                        : sHTML_division, sal_False );
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


