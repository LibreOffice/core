/*************************************************************************
 *
 *  $RCSfile: xmltble.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:15:00 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <hintids.hxx>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTTABLE_HPP_
#include <com/sun/star/text/XTextTable.hpp>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _CNTRSRT_HXX
#include <svtools/cntnrsrt.hxx>
#endif

#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif

#ifndef _SWTABLE_HXX
#include "swtable.hxx"
#endif
#ifndef _DOC_HXX
#include "doc.hxx"
#endif
#ifndef _PAM_HXX
#include "pam.hxx"
#endif
#ifndef _FRMFMT_HXX
#include "frmfmt.hxx"
#endif
#ifndef _WRTSWTBL_HXX
#include "wrtswtbl.hxx"
#endif
#ifndef _FMTFSIZE_HXX
#include "fmtfsize.hxx"
#endif
#ifndef _FMTORNT_HXX
#include "fmtornt.hxx"
#endif

#ifndef _UNOOBJ_HXX
#include "unoobj.hxx"
#endif
#ifndef _UNOTBL_HXX
#include "unotbl.hxx"
#endif

#ifndef _XMLTEXTE_HXX
#include "xmltexte.hxx"
#endif
#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLECTXT_HXX
#include "xmlectxt.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

#ifndef XML_CORE_API
Reference < XText > lcl_xml_CreateTableBoxText(
        const SwStartNode& rBoxSttNd )
{
    SwPaM aPaM( *rBoxSttNd.EndOfSectionNode() );
    aPaM.Move( fnMoveBackward, fnGoNode );
    aPaM.SetMark();
    aPaM.GetPoint()->nNode.Assign( rBoxSttNd );
    aPaM.Move( fnMoveForward, fnGoNode );

    Reference < XTextRange > xTextRange =
        CreateTextRangeFromPosition( aPaM.GetDoc(), *aPaM.GetPoint(),
                                     aPaM.GetMark() );
    return xTextRange->getText();
}
#endif

class SwXMLTableColumn_Impl : public SwWriteTableCol
{
    OUString    sStyleName;
    sal_uInt32  nRelWidth;

public:


    SwXMLTableColumn_Impl( sal_uInt16 nPosition ) :
        SwWriteTableCol( nPosition ),
        nRelWidth( 0UL )
    {};

    void SetStyleName( const OUString& rName ) { sStyleName = rName; }
    const OUString& GetStyleName() const { return sStyleName; }

    void SetRelWidth( sal_uInt32 nSet ) { nRelWidth = nSet; }
    sal_uInt32 GetRelWidth() const { return nRelWidth; }
};

sal_Int32 SwXMLTableColumnCmpWidth_Impl( const SwXMLTableColumn_Impl& r1,
                                           const SwXMLTableColumn_Impl& r2 )
{
    sal_Int32 n = (sal_Int32)r1.GetWidthOpt() - (sal_Int32)r2.GetWidthOpt();
    if( !n )
        n = (sal_Int32)r1.GetRelWidth() - (sal_Int32)r2.GetRelWidth();
    return n;
}

// ---------------------------------------------------------------------

typedef SwXMLTableColumn_Impl *SwXMLTableColumnPtr;
SV_DECL_PTRARR_SORT_DEL( SwXMLTableColumns_Impl, SwXMLTableColumnPtr, 5, 5 )
SV_IMPL_OP_PTRARR_SORT( SwXMLTableColumns_Impl, SwXMLTableColumnPtr )

DECLARE_CONTAINER_SORT( SwXMLTableColumnsSortByWidth_Impl,
                        SwXMLTableColumn_Impl )
IMPL_CONTAINER_SORT( SwXMLTableColumnsSortByWidth_Impl, SwXMLTableColumn_Impl,
                     SwXMLTableColumnCmpWidth_Impl )

class SwXMLTableLines_Impl
{
    SwXMLTableColumns_Impl  aCols;
    const SwTableLines      *pLines;
    sal_uInt32              nWidth;

public:

    SwXMLTableLines_Impl( const SwTableLines& rLines );

    ~SwXMLTableLines_Impl() {}

    sal_uInt32 GetWidth() const { return nWidth; }
    const SwTableLines *GetLines() const { return pLines; }

    const SwXMLTableColumns_Impl& GetColumns() const { return aCols; }
};

SwXMLTableLines_Impl::SwXMLTableLines_Impl( const SwTableLines& rLines ) :
    pLines( &rLines ),
    nWidth( 0UL )
{
#ifndef PRODUCT
    sal_uInt16 nEndCPos = 0U;
#endif
    sal_uInt16 nLines = rLines.Count();
    sal_uInt16 nLine;
    for( nLine=0U; nLine<nLines; nLine++ )
    {
        const SwTableLine *pLine = rLines[nLine];
        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        sal_uInt16 nBoxes = rBoxes.Count();

        sal_uInt16 nCPos = 0U;
        for( sal_uInt16 nBox=0U; nBox<nBoxes; nBox++ )
        {
            const SwTableBox *pBox = rBoxes[nBox];

            sal_uInt16 nOldCPos = nCPos;

            if( nBox < nBoxes-1U || nWidth==0UL )
            {
                nCPos += (sal_uInt16)SwWriteTable::GetBoxWidth( pBox );
                SwXMLTableColumn_Impl *pCol =
                    new SwXMLTableColumn_Impl( nCPos );

                if( !aCols.Insert( pCol ) )
                    delete pCol;

                if( nBox==nBoxes-1U )
                {
                    ASSERT( nLine==0U && nWidth==0UL,
                            "parent width will be lost" );
                    nWidth = nCPos;
                }
            }
            else
            {
#ifndef PRODUCT
                sal_uInt16 nCheckPos =
                    nCPos + (sal_uInt16)SwWriteTable::GetBoxWidth( pBox );
                if( !nEndCPos )
                {
                    nEndCPos = nCheckPos;
                }
                else
                {
                    /*
                    ASSERT( SwXMLTableColumn_impl(nCheckPos) ==
                                        SwXMLTableColumn_Impl(nEndCPos),
                    "rows have different total widths" );
                    */
                }
#endif
                nCPos = nWidth;
#ifndef PRODUCT
                SwXMLTableColumn_Impl aCol( nWidth );
                ASSERT( aCols.Seek_Entry(&aCol), "couldn't find last column" );
                ASSERT( SwXMLTableColumn_Impl(nCheckPos) ==
                                            SwXMLTableColumn_Impl(nCPos),
                        "rows have different total widths" );
#endif
            }
        }
    }
}

typedef SwXMLTableLines_Impl *SwXMLTableLinesPtr;
DECLARE_LIST( SwXMLTableLinesCache_Impl, SwXMLTableLinesPtr )

// ---------------------------------------------------------------------

typedef SwFrmFmt *SwFrmFmtPtr;
DECLARE_LIST( SwXMLFrmFmts_Impl, SwFrmFmtPtr )

class SwXMLTableFrmFmtsSort_Impl : public SwXMLFrmFmts_Impl
{
public:
    SwXMLTableFrmFmtsSort_Impl ( sal_uInt16 nInit, sal_uInt16 nGrow ) :
        SwXMLFrmFmts_Impl( nInit, nGrow )
    {}

    sal_Bool AddRow( SwFrmFmt& rFrmFmt, const OUString& rNamePrefix, sal_uInt32 nLine );
    sal_Bool AddCell( SwFrmFmt& rFrmFmt, const OUString& rNamePrefix,
                  sal_uInt32 nCol, sal_uInt32 nRow, sal_Bool bTop );
};

sal_Bool SwXMLTableFrmFmtsSort_Impl::AddRow( SwFrmFmt& rFrmFmt,
                                         const OUString& rNamePrefix,
                                            sal_uInt32 nLine )
{
    const SwFmtFrmSize *pFrmSize = 0;
    const SvxBrushItem *pBrush = 0;

    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_FRM_SIZE, sal_False, &pItem ) )
        pFrmSize = (const SwFmtFrmSize *)pItem;

    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, sal_False, &pItem ) )
        pBrush = (const SvxBrushItem *)pItem;

    // empty styles have not to be exported
    if( !pFrmSize && !pBrush )
        return sal_False;

    // order is: -/brush, size/-, size/brush
    sal_uInt32 nCount = Count();
    sal_Bool bInsert = sal_True;
    sal_uInt32 i;
    for( i=0; i<nCount; i++ )
    {
        const SwFmtFrmSize *pTestFrmSize = 0;
        const SvxBrushItem *pTestBrush = 0;
        const SwFrmFmt *pTestFmt = GetObject(i);
        const SfxItemSet& rTestSet = pTestFmt->GetAttrSet();
        if( SFX_ITEM_SET == rTestSet.GetItemState( RES_FRM_SIZE, sal_False,
                                                  &pItem ) )
        {
            if( !pFrmSize )
                break;

            pTestFrmSize = (const SwFmtFrmSize *)pItem;
        }
        else
        {
            if( pFrmSize )
                continue;
        }

        if( SFX_ITEM_SET == rTestSet.GetItemState( RES_BACKGROUND, sal_False,
                                                  &pItem ) )
        {
            if( !pBrush )
                break;

            pTestBrush = (const SvxBrushItem *)pItem;
        }
        else
        {
            if( pBrush )
                continue;
        }

        if( pFrmSize &&
            ( pFrmSize->GetSizeType() != pTestFrmSize->GetSizeType() ||
              pFrmSize->GetHeight() != pTestFrmSize->GetHeight() ) )
            continue;

        if( pBrush && !pBrush->equalsXML( *pTestBrush ) )
            continue;

        // found!
        const String& rFmtName = pTestFmt->GetName();
        rFrmFmt.SetName( rFmtName );
        bInsert = sal_False;
        break;
    }

    if( bInsert )
    {
        OUStringBuffer sBuffer( rNamePrefix.getLength() + 4UL );
        sBuffer.append( rNamePrefix );
        sBuffer.append( (sal_Unicode)'.' );
        sBuffer.append( (sal_Int32)(nLine+1UL) );

        rFrmFmt.SetName( sBuffer.makeStringAndClear() );
        Insert( &rFrmFmt, i );
    }

    return bInsert;
}

void lcl_GetTblBoxColStr( sal_uInt16 nCol, String& rNm );
void lcl_xmltble_appendBoxPrefix( OUStringBuffer& rBuffer,
                                  const OUString& rNamePrefix,
                                  sal_uInt16 nCol, sal_uInt16 nRow, sal_Bool bTop )
{
    rBuffer.append( rNamePrefix );
    rBuffer.append( (sal_Unicode)'.' );
    if( bTop )
    {
        String sTmp;
        lcl_GetTblBoxColStr( nCol, sTmp );
        rBuffer.append( sTmp );
    }
    else
    {
        rBuffer.append( (sal_Int32)(nCol + 1));
        rBuffer.append( (sal_Unicode)'.' );
    }
    rBuffer.append( (sal_Int32)(nRow + 1));
}

sal_Bool SwXMLTableFrmFmtsSort_Impl::AddCell( SwFrmFmt& rFrmFmt,
                                         const OUString& rNamePrefix,
                                            sal_uInt32 nCol, sal_uInt32 nRow, sal_Bool bTop )
{
    const SwFmtVertOrient *pVertOrient = 0;
    const SvxBrushItem *pBrush = 0;
    const SvxBoxItem *pBox = 0;

    const SfxItemSet& rItemSet = rFrmFmt.GetAttrSet();
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_VERT_ORIENT, sal_False,
                                               &pItem ) )
        pVertOrient = (const SwFmtVertOrient *)pItem;

    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, sal_False, &pItem ) )
        pBrush = (const SvxBrushItem *)pItem;

    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BOX, sal_False, &pItem ) )
        pBox = (const SvxBoxItem *)pItem;

    // empty styles have not to be exported
    if( !pVertOrient && !pBrush && !pBox )
        return sal_False;

    // order is: -/-/box, -/brush/-, -/brush/box,
    //           vert/-/-, vert/-/box, vert/brush/-, vert/brush/box
    sal_uInt32 nCount = Count();
    sal_Bool bInsert = sal_True;
    sal_uInt32 i;
    for( i=0; i<nCount; i++ )
    {
        const SwFmtVertOrient *pTestVertOrient = 0;
        const SvxBrushItem *pTestBrush = 0;
        const SvxBoxItem *pTestBox = 0;
        const SwFrmFmt *pTestFmt = GetObject(i);
        const SfxItemSet& rTestSet = pTestFmt->GetAttrSet();
        if( SFX_ITEM_SET == rTestSet.GetItemState( RES_VERT_ORIENT, sal_False,
                                                  &pItem ) )
        {
            if( !pVertOrient )
                break;

            pTestVertOrient = (const SwFmtVertOrient *)pItem;
        }
        else
        {
            if( pVertOrient )
                continue;
        }

        if( SFX_ITEM_SET == rTestSet.GetItemState( RES_BACKGROUND, sal_False,
                                                  &pItem ) )
        {
            if( !pBrush )
                break;

            pTestBrush = (const SvxBrushItem *)pItem;
        }
        else
        {
            if( pBrush )
                continue;
        }

        if( SFX_ITEM_SET == rTestSet.GetItemState( RES_BOX, sal_False, &pItem ) )
        {
            if( !pBox )
                break;

            pTestBox = (const SvxBoxItem *)pItem;
        }
        else
        {
            if( pBox )
                continue;
        }

        if( pVertOrient &&
            pVertOrient->GetVertOrient() != pTestVertOrient->GetVertOrient() )
            continue;

        if( pBrush && !pBrush->equalsXML( *pTestBrush ) )
            continue;

        if( pBox && !pBox->equalsXML( *pTestBox ) )
            continue;

        // found!
        const String& rFmtName = pTestFmt->GetName();
        rFrmFmt.SetName( rFmtName );
        bInsert = sal_False;
        break;
    }

    if( bInsert )
    {
        OUStringBuffer sBuffer( rNamePrefix.getLength() + 8UL );
        lcl_xmltble_appendBoxPrefix( sBuffer, rNamePrefix, nCol, nRow, bTop );
        rFrmFmt.SetName( sBuffer.makeStringAndClear() );
        Insert( &rFrmFmt, i );
    }

    return bInsert;
}
// ---------------------------------------------------------------------


void SwXMLExport::ExportTableColumnStyle( const SwXMLTableColumn_Impl& rCol )
{
    // <style:style ...>
    CheckAttrList();

    // style:name="..."
    AddAttribute( XML_NAMESPACE_STYLE, sXML_name, rCol.GetStyleName() );

    // style:family="table-column"
    AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_family, sXML_table_column );

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_STYLE, sXML_style, sal_True,
                                  sal_True );
        OUStringBuffer sValue;
        if( rCol.GetWidthOpt() )
        {
            GetTwipUnitConverter().convertMeasure( sValue, rCol.GetWidthOpt() );
            AddAttribute( XML_NAMESPACE_STYLE, sXML_column_width,
                          sValue.makeStringAndClear() );
        }
        if( rCol.GetRelWidth() )
        {
            sValue.append( (sal_Int32)rCol.GetRelWidth() );
            sValue.append( (sal_Unicode)'*' );
            AddAttribute( XML_NAMESPACE_STYLE, sXML_rel_column_width,
                          sValue.makeStringAndClear() );
        }

        {
            SvXMLElementExport aElem( *this, XML_NAMESPACE_STYLE,
                                      sXML_properties, sal_True, sal_True );
        }
    }
}

void SwXMLExport::ExportTableLinesAutoStyles( const SwTableLines& rLines,
                                    sal_uInt32 nAbsWidth, sal_uInt32 nBaseWidth,
                                    const OUString& rNamePrefix,
                                    SwXMLTableColumnsSortByWidth_Impl& rExpCols,
                                    SwXMLTableFrmFmtsSort_Impl& rExpRows,
                                    SwXMLTableFrmFmtsSort_Impl& rExpCells,
                                      sal_Bool bTop )
{
    // pass 1: calculate columns
    SwXMLTableLines_Impl *pLines =
        new SwXMLTableLines_Impl( rLines );
    if( !pTableLines )
        pTableLines = new SwXMLTableLinesCache_Impl( 5, 5 );
    pTableLines->Insert( pLines, pTableLines->Count() );

    OUStringBuffer sBuffer( rNamePrefix.getLength() + 8L );

    // pass 2: export column styles
    {
        const SwXMLTableColumns_Impl& rCols = pLines->GetColumns();
        sal_uInt16 nCPos = 0U;
        sal_uInt16 nColumns = rCols.Count();
        for( sal_uInt16 nColumn=0U; nColumn<nColumns; nColumn++ )
        {
            SwXMLTableColumn_Impl *pColumn = rCols[nColumn];

            sal_uInt16 nOldCPos = nCPos;
            nCPos = pColumn->GetPos();

            sal_uInt32 nWidth = nCPos - nOldCPos;

            // If a base width is given, the table has either an automatic
            // or margin alignment, or an percentage width. In either case,
            // relative widths should be exported.
            if( nBaseWidth )
            {
                pColumn->SetRelWidth( nWidth );
            }

            // If an absolute width is given, the table either has a fixed
            // width, or the current width is known from the layout. In the
            // later case, a base width is set in addition and must be used
            // to "absoultize" the relative column width.
            if( nAbsWidth )
            {
                sal_uInt32 nColAbsWidth = nWidth;
                if( nBaseWidth )
                {
                    nColAbsWidth *= nAbsWidth;
                    nColAbsWidth += (nBaseWidth/2UL);
                    nColAbsWidth /= nBaseWidth;
                }
                pColumn->SetWidthOpt( nColAbsWidth, sal_False );
            }

            sal_uInt32 nExpPos = 0;
            if( rExpCols.Seek_Entry( pColumn, &nExpPos ) )
            {
                pColumn->SetStyleName(
                        rExpCols.GetObject(nExpPos)->GetStyleName() );
            }
            else
            {
                sBuffer.append( rNamePrefix );
                sBuffer.append( (sal_Unicode)'.' );
                if( bTop )
                {
                    String sTmp;
                    lcl_GetTblBoxColStr( nColumn, sTmp );
                    sBuffer.append( sTmp );
                }
                else
                {
                    sBuffer.append( (sal_Int32)(nColumn + 1U) );
                }

                pColumn->SetStyleName( sBuffer.makeStringAndClear() );
                ExportTableColumnStyle( *pColumn );
                rExpCols.Insert( pColumn );
            }
        }
    }

    // pass 3: export line/rows
    sal_uInt16 nLines = rLines.Count();
    for( sal_uInt16 nLine=0U; nLine<nLines; nLine++ )
    {
        SwTableLine *pLine = rLines[nLine];

        SwFrmFmt *pFrmFmt = pLine->GetFrmFmt();
        if( rExpRows.AddRow( *pFrmFmt, rNamePrefix, nLine ) )
            ExportFmt( *pFrmFmt, sXML_table_row );

        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        sal_uInt16 nBoxes = rBoxes.Count();

        sal_uInt16 nCPos = 0U;
        sal_uInt16 nCol = 0U;
        for( sal_uInt16 nBox=0U; nBox<nBoxes; nBox++ )
        {
            const SwTableBox *pBox = rBoxes[nBox];

            sal_uInt16 nOldCPos = nCPos;
            if( nBox < nBoxes-1U )
                nCPos += (sal_uInt16)SwWriteTable::GetBoxWidth( pBox );
            else
                nCPos = pLines->GetWidth();


            // Und ihren Index
            sal_uInt16 nOldCol = nCol;
            SwXMLTableColumn_Impl aCol( nCPos );
            sal_Bool bFound = pLines->GetColumns().Seek_Entry( &aCol, &nCol );
            ASSERT( bFound, "couldn't find column" );

            const SwStartNode *pBoxSttNd = pBox->GetSttNd();
            if( pBoxSttNd )
            {
#ifdef XML_CORE_API
                SwNode *pStartNd = pDoc->GetNodes()[pBoxSttNd->GetIndex()+1];
                SwNode *pEndNd =
                            pDoc->GetNodes()[pBoxSttNd->EndOfSectionIndex()-1];
                SwXMLExpContext aContext( *this, *pStartNd, *pEndNd,
                                          0, STRING_LEN );
#endif

                SwFrmFmt *pFrmFmt = pBox->GetFrmFmt();
                if( rExpCells.AddCell( *pFrmFmt, rNamePrefix, nOldCol, nLine,
                                       bTop) )
                    ExportFmt( *pFrmFmt, sXML_table_cell );

#ifdef XML_CORE_API
                ExportCurPaMAutoStyles();
#else
                GetTextParagraphExport()->collectTextAutoStyles(
                        lcl_xml_CreateTableBoxText( *pBoxSttNd ) );
#endif
            }
            else
            {
                lcl_xmltble_appendBoxPrefix( sBuffer, rNamePrefix, nOldCol,
                                             nLine, bTop );

                ExportTableLinesAutoStyles( pBox->GetTabLines(),
                                            nAbsWidth, nBaseWidth,
                                            sBuffer.makeStringAndClear(),
                                            rExpCols, rExpRows, rExpCells );
            }

            nCol++;
        }
    }
}

void SwXMLExport::ExportTableAutoStyles( const SwTableNode& rTblNd )
{
    const SwTable& rTbl = rTblNd.GetTable();
    const SwFrmFmt *pTblFmt = rTbl.GetFrmFmt();

    if( pTblFmt )
    {
        SwHoriOrient eTabHoriOri = pTblFmt->GetHoriOrient().GetHoriOrient();
        const SwFmtFrmSize& rFrmSize = pTblFmt->GetFrmSize();

        sal_uInt32 nAbsWidth = rFrmSize.GetSize().Width();
        sal_uInt32 nBaseWidth = 0UL;
        sal_Int8 nPrcWidth = rFrmSize.GetWidthPercent();

        sal_Bool bFixAbsWidth = nPrcWidth != 0 || HORI_NONE == eTabHoriOri
                                           || HORI_FULL == eTabHoriOri;
        if( bFixAbsWidth )
        {
            nBaseWidth = nAbsWidth;
            nAbsWidth = pTblFmt->FindLayoutRect(sal_True).Width();
            if( !nAbsWidth )
            {
                // TODO???
            }
        }
        ExportTableFmt( *pTblFmt, nAbsWidth );

        OUString sName( pTblFmt->GetName() );
        SwXMLTableColumnsSortByWidth_Impl aExpCols( 10, 10 );
        SwXMLTableFrmFmtsSort_Impl aExpRows( 10, 10 );
        SwXMLTableFrmFmtsSort_Impl aExpCells( 10, 10 );
        ExportTableLinesAutoStyles( rTbl.GetTabLines(), nAbsWidth, nBaseWidth,
                                    sName, aExpCols, aExpRows, aExpCells,
                                    sal_True );
    }

#ifdef XML_CORE_API
    pCurPaM->GetPoint()->nNode = *rTblNd.EndOfSectionNode();
#endif
}

// ---------------------------------------------------------------------

void SwXMLExport::ExportTableBox( const SwTableBox& rBox, sal_uInt16 nColSpan )
{
    const SwStartNode *pBoxSttNd = rBox.GetSttNd();
    if( pBoxSttNd )
    {
        const SwFrmFmt *pFrmFmt = rBox.GetFrmFmt();
        if( pFrmFmt )
        {
            const String& rName = pFrmFmt->GetName();
            if( rName.Len() )
            {
                AddAttribute( XML_NAMESPACE_TABLE, sXML_style_name, rName );
            }
        }
    }

    if( nColSpan != 1 )
    {
        OUStringBuffer sTmp;
        sTmp.append( (sal_Int32)nColSpan );
        AddAttribute( XML_NAMESPACE_TABLE, sXML_number_columns_spanned,
                      sTmp.makeStringAndClear() );
    }

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE,
                                  sXML_table_cell, sal_True, sal_True );
        if( pBoxSttNd )
        {
#ifdef XML_CORE_API
            SwNode *pStartNd = pDoc->GetNodes()[pBoxSttNd->GetIndex()+1];
            SwNode *pEndNd = pDoc->GetNodes()[pBoxSttNd->EndOfSectionIndex()-1];
            SwXMLExpContext aContext( *this, *pStartNd, *pEndNd,
                                      0, STRING_LEN );
            ExportCurPaM();
#else
            GetTextParagraphExport()->exportText(
                lcl_xml_CreateTableBoxText( *pBoxSttNd ) );
#endif
        }
        else
        {
            SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE,
                                      sXML_sub_table, sal_True, sal_True );
            ExportTableLines( rBox.GetTabLines() );
        }
    }
}

void SwXMLExport::ExportTableLine( const SwTableLine& rLine,
                                   const SwXMLTableLines_Impl& rLines )
{
    const SwFrmFmt *pFrmFmt = rLine.GetFrmFmt();
    if( pFrmFmt )
    {
        const String& rName = pFrmFmt->GetName();
        if( rName.Len() )
        {
            AddAttribute( XML_NAMESPACE_TABLE, sXML_style_name, rName );
        }
    }

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE,
                                  sXML_table_row, sal_True, sal_True );
        const SwTableBoxes& rBoxes = rLine.GetTabBoxes();
        sal_uInt16 nBoxes = rBoxes.Count();

        sal_uInt16 nCPos = 0U;
        sal_uInt16 nCol = 0U;
        for( sal_uInt16 nBox=0U; nBox<nBoxes; nBox++ )
        {
            const SwTableBox *pBox = rBoxes[nBox];

            sal_uInt16 nOldCPos = nCPos;
            if( nBox < nBoxes-1U )
                nCPos += (sal_uInt16)SwWriteTable::GetBoxWidth( pBox );
            else
                nCPos = rLines.GetWidth();

            // Und ihren Index
            sal_uInt16 nOldCol = nCol;
            SwXMLTableColumn_Impl aCol( nCPos );
            sal_Bool bFound = rLines.GetColumns().Seek_Entry( &aCol, &nCol );
            ASSERT( bFound, "couldn't find column" );

            sal_uInt16 nColSpan = nCol - nOldCol + 1U;
            ExportTableBox( *pBox, nColSpan );
            for( sal_uInt16 i=nOldCol; i<nCol; i++ )
            {
                SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE,
                                          sXML_covered_table_cell, sal_True,
                                          sal_False );
            }

            nCol++;
        }
    }
}

void SwXMLExport::ExportTableLines( const SwTableLines& rLines,
                                    sal_Bool bHeadline )
{
    ASSERT( pTableLines && pTableLines->Count(),
            "SwXMLExport::ExportTableLines: table columns infos missing" );
    if( !pTableLines || 0 == pTableLines->Count() )
        return;

    SwXMLTableLines_Impl *pLines = 0;
    sal_uInt16 nInfoPos;
    for( nInfoPos=0; nInfoPos < pTableLines->Count(); nInfoPos++ )
    {
        if( pTableLines->GetObject( nInfoPos )->GetLines() == &rLines )
        {
            pLines = pTableLines->GetObject( nInfoPos );
            break;
        }
    }
    ASSERT( pLines,
            "SwXMLExport::ExportTableLines: table columns info missing" );
    ASSERT( 0==nInfoPos,
            "SwXMLExport::ExportTableLines: table columns infos are unsorted" );
    if( !pLines )
        return;

    // pass 2: export columns
    const SwXMLTableColumns_Impl& rCols = pLines->GetColumns();
    sal_uInt16 nColumn = 0U;
    sal_uInt16 nColumns = rCols.Count();
    sal_uInt16 nColRep = 1U;
    SwXMLTableColumn_Impl *pColumn = (nColumns > 0) ? rCols[0U] : 0;
    while( pColumn )
    {
        nColumn++;
        SwXMLTableColumn_Impl *pNextColumn =
            (nColumn < nColumns) ? rCols[nColumn] : 0;
        if( pNextColumn &&
            pNextColumn->GetStyleName() == pColumn->GetStyleName() )
        {
            nColRep++;
        }
        else
        {
            AddAttribute( XML_NAMESPACE_TABLE, sXML_style_name,
                          pColumn->GetStyleName() );

            if( nColRep > 1U )
            {
                OUStringBuffer sTmp(4);
                sTmp.append( (sal_Int32)nColRep );
                AddAttribute( XML_NAMESPACE_TABLE, sXML_number_columns_repeated,
                              sTmp.makeStringAndClear() );
            }

            {
                SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE,
                                          sXML_table_column, sal_True, sal_True );
            }

            nColRep = 1U;
        }
        pColumn = pNextColumn;
    }

    // pass 3: export line/rows
    sal_uInt16 nLines = rLines.Count();
    for( sal_uInt16 nLine=0U; nLine<nLines; nLine++ )
    {
        const SwTableLine *pLine = rLines[nLine];
        if( bHeadline && 0U==nLine )
        {
            SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE,
                                      sXML_table_header_rows, sal_True, sal_True );
            ExportTableLine( *pLine, *pLines );
        }
        else
        {
            ExportTableLine( *pLine, *pLines );
        }
    }

    pTableLines->Remove( nInfoPos );
    if( 0 == pTableLines->Count() )
    {
        delete pTableLines ;
        pTableLines = 0;
    }
    delete pLines;
}

sal_Bool lcl_xmltble_ClearName_Line( const SwTableLine*& rpLine, void* );

sal_Bool lcl_xmltble_ClearName_Box( const SwTableBox*& rpBox, void* )
{
    if( !rpBox->GetSttNd() )
    {
        ((SwTableBox *)rpBox)->GetTabLines().ForEach(
                                            &lcl_xmltble_ClearName_Line, 0 );
    }
    else
    {
        SwFrmFmt *pFrmFmt = ((SwTableBox *)rpBox)->GetFrmFmt();
        if( pFrmFmt && pFrmFmt->GetName().Len() )
            pFrmFmt->SetName( aEmptyStr );
    }

    return sal_True;
}

sal_Bool lcl_xmltble_ClearName_Line( const SwTableLine*& rpLine, void* )
{
    ((SwTableLine *)rpLine)->GetTabBoxes().ForEach(
                                            &lcl_xmltble_ClearName_Box, 0 );

    return sal_True;
}

void SwXMLExport::ExportTable( const SwTableNode& rTblNd )
{
    const SwTable& rTbl = rTblNd.GetTable();
    const SwFrmFmt *pTblFmt = rTbl.GetFrmFmt();
    if( pTblFmt && pTblFmt->GetName().Len() )
    {
        AddAttribute( XML_NAMESPACE_TABLE, sXML_name, pTblFmt->GetName() );
        AddAttribute( XML_NAMESPACE_TABLE, sXML_style_name,
                      pTblFmt->GetName() );
    }

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, sXML_table,
                                  sal_True, sal_True );
        ExportTableLines( rTbl.GetTabLines(), rTbl.IsHeadlineRepeat() );

        ((SwTable &)rTbl).GetTabLines().ForEach( &lcl_xmltble_ClearName_Line,
                                                 0 );
    }

#ifdef XML_CORE_API
    pCurPaM->GetPoint()->nNode = *rTblNd.EndOfSectionNode();
#endif
}

#ifndef XML_CORE_API
void SwXMLTextParagraphExport::exportTable(
        const Reference < XTextContent > & rTextContent,
        sal_Bool bAutoStyles )
{
//  Reference < XPropertySet > xPropSet( rTextContent, UNO_QUERY );
//  Any aAny = xPropSet->getPropertyValue( sTextTable );

    Reference < XTextTable > xTxtTbl( rTextContent, UNO_QUERY );
//  aAny >>= xTxtTbl;
    DBG_ASSERT( xTxtTbl.is(), "text table missing" );
    if( xTxtTbl.is() )
    {
        const SwXTextTable *pXTable = 0;
        Reference<XUnoTunnel> xTableTunnel( rTextContent, UNO_QUERY);
        if( xTableTunnel.is() )
        {
            pXTable = (SwXTextTable*)xTableTunnel->getSomething(
                                            SwXTextTable::getUnoTunnelId() );
            ASSERT( pXTable, "SwXTextTable missing" );
        }
        if( pXTable )
        {
            SwFrmFmt *pFmt = pXTable->GetFrmFmt();
            ASSERT( pFmt, "table format missing" );
            const SwTable *pTbl = SwTable::FindTable( pFmt );
            ASSERT( pTbl, "table missing" );
            const SwTableNode *pTblNd = pTbl->GetTableNode();
            ASSERT( pTblNd, "table node missing" );
            if( bAutoStyles )
            {
                ((SwXMLExport&)GetExport()).ExportTableAutoStyles( *pTblNd );
            }
            else
            {
                ((SwXMLExport&)GetExport()).ExportTable( *pTblNd );
            }
        }
    }
}
#endif

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmltble.cxx,v 1.1.1.1 2000-09-18 17:15:00 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.11  2000/09/18 16:05:07  willem.vandorp
      OpenOffice header added.

      Revision 1.10  2000/09/05 09:51:10  mib
      #78063#: table lines cache is sorted now

      Revision 1.9  2000/07/31 09:42:35  mib
      text export continued

      Revision 1.8  2000/07/21 12:55:15  mib
      text import/export using StarOffice API

      Revision 1.7  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.6  2000/05/03 12:08:05  mib
      unicode

      Revision 1.5  2000/03/13 14:33:44  mib
      UNO3

      Revision 1.4  2000/02/11 17:08:37  kz
      #65293# cast Syntax

      Revision 1.3  2000/02/11 14:42:15  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.2  2000/02/08 06:47:09  mib
      #70271#: remove temporary table cell format names

      Revision 1.1  2000/02/07 10:02:55  mib
      #70271#: table export


*************************************************************************/

