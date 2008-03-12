/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unotbl.cxx,v $
 *
 *  $Revision: 1.116 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:35:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"




// STL includes
#include <list>

#include <float.h> // for DBL_MIN

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _UNOTBL_HXX
#include <unotbl.hxx>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif

#ifndef _UNOCRSR_HXX //autogen
#include <unocrsr.hxx>
#endif
#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _SWTBLFMT_HXX //autogen
#include <swtblfmt.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDOLE_HXX //autogen
#include <ndole.hxx>
#endif

#ifndef _FRAME_HXX //autogen
#include <frame.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#include <fmtfsize.hxx>
#ifndef _TBLAFMT_HXX //autogen
#include <tblafmt.hxx>
#endif
#ifndef _TABCOL_HXX //autogen
#include <tabcol.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#ifndef _VIEWSH_HXX //autogen
#include <viewsh.hxx>
#endif
#ifndef _TABFRM_HXX //autogen
#include <tabfrm.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _UNOREDLINE_HXX
#include <unoredline.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TABLECOLUMNSEPARATOR_HPP_
#include <com/sun/star/text/TableColumnSeparator.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWFORMAT_HPP_
#include <com/sun/star/table/ShadowFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEBORDER_HPP_
#include <com/sun/star/table/TableBorder.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PAGESTYLELAYOUT_HPP_
#include <com/sun/star/style/PageStyleLayout.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_BREAKTYPE_HPP_
#include <com/sun/star/style/BreakType.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_
#include <com/sun/star/style/GraphicLocation.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PropertyAttribute_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDATACHANGEEVENTLISTENER_HPP_
#include <com/sun/star/chart/XChartDataChangeEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATACHANGEEVENT_HPP_
#include <com/sun/star/chart/ChartDataChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XLABELEDDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLCONTENTTYPE_HPP_
#include <com/sun/star/table/CellContentType.hpp>
#endif
#ifndef _UNOTBL_HXX
#include <unotbl.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif

#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>     // SvNumberFormatter
#endif

#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _FMTTSPLT_HXX
#include <fmtlsplt.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif
#ifndef _UNOCHART_HXX
#include <unochart.hxx>
#endif

using namespace ::com::sun::star;
using ::rtl::OUString;
//collectn.cxx
BOOL lcl_IsNumeric(const String&);

//-----------------------------------------------------------------------------
// from unoobj.cxx
extern void lcl_SetTxtFmtColl(const uno::Any& rAny, SwPaM& rPaM)    throw (lang::IllegalArgumentException);
extern void lcl_setCharStyle(SwDoc* pDoc, const uno::Any aValue, SfxItemSet& rSet) throw (lang::IllegalArgumentException);

// from swtable.cxx
extern void lcl_GetTblBoxColStr( sal_uInt16 nCol, String& rNm );

#define UNO_TABLE_COLUMN_SUM    10000

/* -----------------17.07.98 15:47-------------------
 *
 * --------------------------------------------------*/
table::BorderLine lcl_SvxLineToLine(const SvxBorderLine* pLine)
{
     table::BorderLine aLine;
    if(pLine)
    {
        aLine.Color          = pLine->GetColor().GetColor() ;
        aLine.InnerLineWidth = TWIP_TO_MM100_UNSIGNED( pLine->GetInWidth() );
        aLine.OuterLineWidth = TWIP_TO_MM100_UNSIGNED( pLine->GetOutWidth() );
        aLine.LineDistance   = TWIP_TO_MM100_UNSIGNED( pLine->GetDistance() );
    }
    else
        aLine.Color          = aLine.InnerLineWidth = aLine.OuterLineWidth = aLine.LineDistance  = 0;
    return aLine;
}
/* -----------------17.07.98 15:52-------------------
 *
 * --------------------------------------------------*/
sal_Bool lcl_LineToSvxLine(const table::BorderLine& rLine, SvxBorderLine& rSvxLine)
{
    rSvxLine.SetColor(   Color(rLine.Color));
    rSvxLine.SetInWidth( MM100_TO_TWIP( rLine.InnerLineWidth ) );
    rSvxLine.SetOutWidth(MM100_TO_TWIP( rLine.OuterLineWidth ) );
    rSvxLine.SetDistance(MM100_TO_TWIP( rLine.LineDistance  ) );
    sal_Bool bRet = rLine.InnerLineWidth > 0 || rLine.OuterLineWidth > 0;
    return bRet;
}
/* -----------------11.12.98 14:22-------------------
 *
 * --------------------------------------------------*/
void lcl_SetSpecialProperty(SwFrmFmt* pFmt, const SfxItemPropertyMap* pMap, const uno::Any& aValue)
    throw (lang::IllegalArgumentException)
{
    //Sonderbehandlung fuer "Nicht-Items"
    switch(pMap->nWID)
    {
        case  FN_TABLE_HEADLINE_REPEAT:
        case  FN_TABLE_HEADLINE_COUNT:
        {
            SwTable* pTable = SwTable::FindTable( pFmt );
            {
                UnoActionContext aAction(pFmt->GetDoc());
                if( pMap->nWID == FN_TABLE_HEADLINE_REPEAT)
                {
                    sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                    pFmt->GetDoc()->SetRowsToRepeat( *pTable, bVal ? 1 : 0 );
                }
                else
                {
                    sal_Int32 nRepeat = 0;
                    aValue >>= nRepeat;
                    if( nRepeat >= 0 && nRepeat < USHRT_MAX )
                        pFmt->GetDoc()->SetRowsToRepeat( *pTable, (USHORT) nRepeat );
                }
            }
        }
        break;
        case  FN_TABLE_IS_RELATIVE_WIDTH:
        case  FN_TABLE_WIDTH:
        case  FN_TABLE_RELATIVE_WIDTH:
        {
            sal_Int32 nWidth = 0;
            SwFmtFrmSize aSz( pFmt->GetFrmSize() );
            if(FN_TABLE_WIDTH == pMap->nWID)
            {
                aValue >>= nWidth;
                aSz.SetWidthPercent(0);
                aSz.SetWidth ( MM100_TO_TWIP ( nWidth ) );
            }
            else if(FN_TABLE_RELATIVE_WIDTH == pMap->nWID)
            {
                sal_Int16 nSet = 0;
                aValue >>= nSet;
                if(nSet && nSet <=100)
                    aSz.SetWidthPercent( (BYTE)nSet );
            }
            else if(FN_TABLE_IS_RELATIVE_WIDTH == pMap->nWID)
            {
                sal_Bool bPercent = *(sal_Bool*)aValue.getValue();
                if(!bPercent)
                    aSz.SetWidthPercent(0);
                else
                {
                    lang::IllegalArgumentException aExcept;
                    aExcept.Message = C2U("relative width cannot be switched on with this property");
                    throw aExcept;
                }
            }
            pFmt->GetDoc()->SetAttr(aSz, *pFmt);
        }
        break;
        case RES_PAGEDESC:
        {
            OUString uTemp;
            aValue >>= uTemp;
            String sPageStyle = uTemp;
            const SwPageDesc* pDesc = 0;
            if(sPageStyle.Len())
            {
                SwStyleNameMapper::FillUIName(sPageStyle, sPageStyle, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, sal_True );
                pDesc = ::GetPageDescByName_Impl(*pFmt->GetDoc(), sPageStyle);
            }
            SwFmtPageDesc aDesc( pDesc );
            pFmt->GetDoc()->SetAttr(aDesc, *pFmt);
        }
        break;
        default:
            throw lang::IllegalArgumentException();
    }
}

/* -----------------27.04.98 08:50-------------------
 *
 * --------------------------------------------------*/
uno::Any lcl_GetSpecialProperty(SwFrmFmt* pFmt, const SfxItemPropertyMap* pMap )
{
    uno::Any aRet;
    switch(pMap->nWID)
    {
        case  FN_TABLE_HEADLINE_REPEAT:
        case  FN_TABLE_HEADLINE_COUNT:
        {
            SwTable* pTable = SwTable::FindTable( pFmt );
            USHORT nRepeat = pTable->GetRowsToRepeat();
            if(pMap->nWID == FN_TABLE_HEADLINE_REPEAT)
            {
                BOOL bTemp = nRepeat > 0;
                aRet.setValue(&bTemp, ::getCppuBooleanType());
            }
            else
                aRet <<= (sal_Int32)nRepeat;
        }
        break;
        case  FN_TABLE_WIDTH:
        case  FN_TABLE_IS_RELATIVE_WIDTH:
        case  FN_TABLE_RELATIVE_WIDTH:
        {
            const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
            if(FN_TABLE_WIDTH == pMap->nWID)
                rSz.QueryValue(aRet, MID_FRMSIZE_WIDTH|CONVERT_TWIPS);
            else if(FN_TABLE_RELATIVE_WIDTH == pMap->nWID)
                rSz.QueryValue(aRet, MID_FRMSIZE_REL_WIDTH);
            else
            {
                BOOL bTemp = 0 != rSz.GetWidthPercent();
                aRet.setValue(&bTemp, ::getBooleanCppuType());
            }
        }
        break;
        case RES_PAGEDESC:
        {
            const SfxItemSet& rSet = pFmt->GetAttrSet();
            const SfxPoolItem* pItem;
            String sPDesc;
            if(SFX_ITEM_SET == rSet.GetItemState(RES_PAGEDESC, sal_False, &pItem))
            {
                const SwPageDesc* pDsc = ((const SwFmtPageDesc*)pItem)->GetPageDesc();
                if(pDsc)
                {
                   sPDesc = SwStyleNameMapper::GetProgName(pDsc->GetName(), nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
                }
            }
            aRet <<= OUString(sPDesc);
        }
        break;
        case RES_ANCHOR :
            aRet <<= text::TextContentAnchorType_AT_PARAGRAPH;
        break;
        case FN_UNO_ANCHOR_TYPES :
        {
            uno::Sequence<text::TextContentAnchorType> aTypes(1);
             text::TextContentAnchorType* pArray = aTypes.getArray();
            pArray[0] = text::TextContentAnchorType_AT_PARAGRAPH;
            aRet <<= aTypes;
        }
        break;
        case FN_UNO_WRAP :
        {
            aRet <<= text::WrapTextMode_NONE;
        }
        break;
        case FN_PARAM_LINK_DISPLAY_NAME :
            aRet <<= OUString(pFmt->GetName());
        break;
        case FN_UNO_REDLINE_NODE_START:
        case FN_UNO_REDLINE_NODE_END:
        {
            SwTable* pTable = SwTable::FindTable( pFmt );
            SwNode* pTblNode = pTable->GetTableNode();
            if(FN_UNO_REDLINE_NODE_END == pMap->nWID)
                pTblNode = pTblNode->EndOfSectionNode();
            const SwRedlineTbl& rRedTbl = pFmt->GetDoc()->GetRedlineTbl();
            for(USHORT nRed = 0; nRed < rRedTbl.Count(); nRed++)
            {
                const SwRedline* pRedline = rRedTbl[nRed];
                const SwNode* pRedPointNode = pRedline->GetNode(TRUE);
                const SwNode* pRedMarkNode = pRedline->GetNode(FALSE);
                if(pRedPointNode == pTblNode || pRedMarkNode == pTblNode)
                {
                    const SwNode* pStartOfRedline = SwNodeIndex(*pRedPointNode) <= SwNodeIndex(*pRedMarkNode) ?
                        pRedPointNode : pRedMarkNode;
                    BOOL bIsStart = pStartOfRedline == pTblNode;
                    aRet <<= SwXRedlinePortion::CreateRedlineProperties(*pRedline, bIsStart);
                    break;
                }
            }
        }
        break;
    }
    return aRet;
}
/* -----------------19.10.05 08:32-------------------
 *
 * --------------------------------------------------*/


// returns the position for the cell with the specified name
// (note that the indices rColumn and rRow are 0 based here)
// Also since the implementations of tables does not really have
// columns using this function is appropriate only for tables
// that are not complex (i.e. where IsTblComplex() returns false).
//
// returns: both indices for column and row (all >= 0) if everything was Ok.
//          At least one value < 0 if sth was wrong.
//
// Sample for naming scheme of cell in a single row (in groups a 26):
// A1..Z1, a1..z1, AA1..AZ1, Aa1..Az1, BA1..BZ1, Ba1..Bz1, ...
void lcl_GetCellPosition( const String &rCellName,
        sal_Int32 &rColumn, sal_Int32 &rRow)
{
    rColumn = rRow = -1;    // default return values indicating failure
    xub_StrLen nLen = rCellName.Len();
    if (nLen)
    {
        const sal_Unicode *pBuf = rCellName.GetBuffer();
        const sal_Unicode *pEnd = pBuf + nLen;
        while (pBuf < pEnd && !('0' <= *pBuf && *pBuf <= '9'))
            ++pBuf;
        // start of number found?
        if (pBuf < pEnd && ('0' <= *pBuf && *pBuf <= '9'))
        {
            String aColTxt( rCellName.GetBuffer(), static_cast< xub_StrLen >(pBuf - rCellName.GetBuffer()) );
            String aRowTxt( pBuf, static_cast< xub_StrLen >(rCellName.GetBuffer() + nLen - pBuf) );
            if (aColTxt.Len() && aRowTxt.Len())
            {
                sal_Int32 nColIdx = 0;
                sal_Int32 nLength = aColTxt.Len();
                for (xub_StrLen i = 0;  i < nLength;  ++i)
                {
                    nColIdx = 52 * nColIdx;
                    if (i < nLength - 1)
                        ++nColIdx;
                    sal_Unicode cChar = aColTxt.GetBuffer()[i];
                    if ('A' <= cChar && cChar <= 'Z')
                        nColIdx = nColIdx + (cChar - 'A');
                    else if ('a' <= cChar && cChar <= 'z')
                        nColIdx = nColIdx + (26 + cChar - 'a');
                    else
                    {
                        nColIdx = -1;   // sth failed
                        break;
                    }
                }

                rColumn = nColIdx;
                rRow    = aRowTxt.ToInt32() - 1;    // - 1 because indices ought to be 0 based
            }
        }
    }
#if OSL_DEBUG_LEVEL > 1
    DBG_ASSERT( rColumn != -1 && rRow != -1, "failed to get column or row index" );
#endif
}


// arguments: must be non-empty strings with valid cell names
//
// returns: -1 if first cell < second cell
//           0 if both cells are equal
//          +1 if the first cell > second cell
//
// Note: this function probably also make sense only
//      for cell names of non-complex tables
int lcl_CompareCellsByRowFirst( const String &rCellName1, const String &rCellName2 )
{
    sal_Int32 nCol1 = -1, nRow1 = -1, nCol2 = -1, nRow2 = -1;
    lcl_GetCellPosition( rCellName1, nCol1, nRow1 );
    lcl_GetCellPosition( rCellName2, nCol2, nRow2 );

    if (nRow1 < nRow2 || (nRow1 == nRow2 && nCol1 < nCol2))
        return -1;
    else if (nCol1 == nCol2 && nRow1 == nRow2)
        return 0;
    else
        return +1;
}


// arguments: must be non-empty strings with valid cell names
//
// returns: -1 if first cell < second cell
//           0 if both cells are equal
//          +1 if the first cell > second cell
//
// Note: this function probably also make sense only
//      for cell names of non-complex tables
int lcl_CompareCellsByColFirst( const String &rCellName1, const String &rCellName2 )
{
    sal_Int32 nCol1 = -1, nRow1 = -1, nCol2 = -1, nRow2 = -1;
    lcl_GetCellPosition( rCellName1, nCol1, nRow1 );
    lcl_GetCellPosition( rCellName2, nCol2, nRow2 );

    if (nCol1 < nCol2 || (nCol1 == nCol2 && nRow1 < nRow2))
        return -1;
    else if (nRow1 == nRow2 && nCol1 == nCol2)
        return 0;
    else
        return +1;
}


// arguments: must be non-empty strings with valid cell names
//
// returns: -1 if first cell range < second cell range
//           0 if both cell ranges are identical
//          +1 if the first cell range > second cell range
//
// Note: this function probably also make sense only
//      for cell names of non-complex tables
int lcl_CompareCellRanges(
        const String &rRange1StartCell, const String &rRange1EndCell,
        const String &rRange2StartCell, const String &rRange2EndCell,
        sal_Bool bCmpColsFirst )
{
    int (*pCompareCells)( const String &, const String & ) =
            bCmpColsFirst ? &lcl_CompareCellsByColFirst : &lcl_CompareCellsByRowFirst;

    int nCmpResStartCells = pCompareCells( rRange1StartCell, rRange2StartCell );
    if ((-1 == nCmpResStartCells ) ||
         ( 0 == nCmpResStartCells &&
          -1 == pCompareCells( rRange1EndCell, rRange2EndCell ) ))
        return -1;
    else if (0 == nCmpResStartCells &&
             0 == pCompareCells( rRange1EndCell, rRange2EndCell ))
        return 0;
    else
        return +1;
}


/* -----------------25.06.98 08:32-------------------
 *
 * --------------------------------------------------*/

// returns the cell name for the cell at the specified position
// (note that the indices nColumn and nRow are 0 based here)
String lcl_GetCellName( sal_Int32 nColumn, sal_Int32 nRow )
{
#if OSL_DEBUG_LEVEL > 1
    {
        sal_Int32 nCol, nRow2;
        lcl_GetCellPosition( String::CreateFromAscii("z1"), nCol, nRow2);
        DBG_ASSERT( nCol == 51, "lcl_GetCellPosition failed" );
        lcl_GetCellPosition( String::CreateFromAscii("AA1"), nCol, nRow2);
        DBG_ASSERT( nCol == 52, "lcl_GetCellPosition failed" );
        lcl_GetCellPosition( String::CreateFromAscii("AB1"), nCol, nRow2);
        DBG_ASSERT( nCol == 53, "lcl_GetCellPosition failed" );
        lcl_GetCellPosition( String::CreateFromAscii("BB1"), nCol, nRow2);
        DBG_ASSERT( nCol == 105, "lcl_GetCellPosition failed" );
    }
#endif

    String sCellName;
    if (nColumn < 0 || nRow < 0)
        return sCellName;
    lcl_GetTblBoxColStr( static_cast< USHORT >(nColumn), sCellName );
    sCellName += String::CreateFromInt32( nRow + 1 );
    return sCellName;
}


/* -----------------21.11.05 14:46-------------------

 --------------------------------------------------*/
// start cell should be in the upper-left corner of the range and
// end cell in the lower-right.
// I.e. from the four possible representation
//      A1:C5, C5:A1, A5:C1, C1:A5
// only A1:C5 is the one to use
void lcl_NormalizeRange(
    String &rCell1,     // will hold the upper-left cell of the range upon return
    String &rCell2 )    // will hold the lower-right cell of the range upon return
{
    sal_Int32 nCol1 = -1, nRow1 = -1, nCol2 = -1, nRow2 = -1;
    lcl_GetCellPosition( rCell1, nCol1, nRow1 );
    lcl_GetCellPosition( rCell2, nCol2, nRow2 );
    if (nCol2 < nCol1 || nRow2 < nRow1)
    {
        rCell1  = lcl_GetCellName( Min(nCol1, nCol2), Min(nRow1, nRow2) );
        rCell2  = lcl_GetCellName( Max(nCol1, nCol2), Max(nRow1, nRow2) );
    }

}

void SwRangeDescriptor::Normalize()
{
    if (nTop > nBottom)
    {
        sal_Int32 nTmp = nTop;
        nTop = nBottom;
        nBottom = nTmp;
    }
    if (nLeft > nRight)
    {
        sal_Int32 nTmp = nLeft;
        nLeft = nRight;
        nRight = nTmp;
    }
}


/* -----------------25.06.98 08:32-------------------
 *
 * --------------------------------------------------*/
SwXCell* lcl_CreateXCell(SwFrmFmt* pFmt, sal_Int32 nColumn, sal_Int32 nRow)
{
    SwXCell* pXCell = 0;
    String sCellName = lcl_GetCellName(nColumn, nRow);
    SwTable* pTable = SwTable::FindTable( pFmt );
    SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
    if(pBox)
    {
        pXCell = SwXCell::CreateXCell(pFmt, pBox, pTable);
    }
    return pXCell;
}
/* -----------------20.07.98 12:35-------------------
 *
 * --------------------------------------------------*/
void lcl_InspectLines(SwTableLines& rLines, SvStrings& rAllNames)
{
    for( sal_uInt16 i = 0; i < rLines.Count(); i++ )
    {
        SwTableLine* pLine = rLines[i];
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for(sal_uInt16 j = 0; j < rBoxes.Count(); j++)
        {
            SwTableBox* pBox = rBoxes[j];
            if(pBox->GetName().Len())
                rAllNames.Insert(new String(pBox->GetName()), rAllNames.Count());
            SwTableLines& rBoxLines = pBox->GetTabLines();
            if(rBoxLines.Count())
            {
                lcl_InspectLines(rBoxLines, rAllNames);
            }
        }
    }
}
/* -----------------02.10.98 15:55-------------------
 *
 * --------------------------------------------------*/
void lcl_FormatTable(SwFrmFmt* pTblFmt)
{
    SwClientIter aIter( *pTblFmt );
    for( SwClient* pC = aIter.First( TYPE( SwFrm ));
            pC; pC = aIter.Next() )
    {
        if( ((SwFrm*)pC)->IsTabFrm() )
        {
            if(((SwFrm*)pC)->IsValid())
                ((SwFrm*)pC)->InvalidatePos();
            ((SwTabFrm*)pC)->SetONECalcLowers();
            ((SwTabFrm*)pC)->Calc();
        }
    }
}
/* -----------------20.07.98 13:15-------------------
 *
 * --------------------------------------------------*/
void lcl_CrsrSelect(SwPaM* pCrsr, sal_Bool bExpand)
{
    if(bExpand)
    {
        if(!pCrsr->HasMark())
            pCrsr->SetMark();
    }
    else if(pCrsr->HasMark())
        pCrsr->DeleteMark();

}
/* -----------------17.07.98 14:36-------------------
 *
 * --------------------------------------------------*/
void lcl_GetTblSeparators(uno::Any& rRet, SwTable* pTable, SwTableBox* pBox, sal_Bool bRow)
{
    SwTabCols aCols;
    aCols.SetLeftMin ( 0 );
    aCols.SetLeft    ( 0 );
    aCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    aCols.SetRightMax( UNO_TABLE_COLUMN_SUM );

    pTable->GetTabCols( aCols, pBox, sal_False, bRow );

    sal_uInt16 nSepCount = aCols.Count();
    uno::Sequence< text::TableColumnSeparator> aColSeq(nSepCount);
     text::TableColumnSeparator* pArray = aColSeq.getArray();
    sal_Bool bError = sal_False;
    for(sal_uInt16 i = 0; i < nSepCount; i++)
    {
        pArray[i].Position = static_cast< sal_Int16 >(aCols[i]);
        pArray[i].IsVisible = !aCols.IsHidden(i);
        if(!bRow && !pArray[i].IsVisible)
        {
            bError = sal_True;
            break;
        }
    }
    if(!bError)
        rRet.setValue(&aColSeq, ::getCppuType((uno::Sequence< text::TableColumnSeparator>*)0));

}
/* -----------------17.07.98 14:36-------------------
 *
 * --------------------------------------------------*/
void lcl_SetTblSeparators(const uno::Any& rVal, SwTable* pTable, SwTableBox* pBox, sal_Bool bRow, SwDoc* pDoc)
{
    SwTabCols aOldCols;

    aOldCols.SetLeftMin ( 0 );
    aOldCols.SetLeft    ( 0 );
    aOldCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    aOldCols.SetRightMax( UNO_TABLE_COLUMN_SUM );

    pTable->GetTabCols( aOldCols, pBox, sal_False, bRow );
    sal_uInt16 nOldCount = aOldCols.Count();
    //there's no use in setting tab cols if there's only one column
    if( !nOldCount )
        return;

    const uno::Sequence< text::TableColumnSeparator>* pSepSeq =
                (uno::Sequence< text::TableColumnSeparator>*) rVal.getValue();
    if(pSepSeq && pSepSeq->getLength() == nOldCount)
    {
        SwTabCols aCols(aOldCols);
        sal_Bool bError = sal_False;
        const text::TableColumnSeparator* pArray = pSepSeq->getConstArray();
        sal_Int32 nLastValue = 0;
        //sal_Int32 nTblWidth = aCols.GetRight() - aCols.GetLeft();
        for(sal_uInt16 i = 0; i < nOldCount; i++)
        {
            aCols[i] = pArray[i].Position;
            if(pArray[i].IsVisible == aCols.IsHidden(i) ||
                !bRow && aCols.IsHidden(i) ||
                long(aCols[i] - long(nLastValue)) < 0 ||
                UNO_TABLE_COLUMN_SUM < aCols[i] )
            {
                bError = sal_True;
                break;
            }
            nLastValue = aCols[i];
        }
        if(!bError)
        {
            pDoc->SetTabCols(*pTable, aCols, aOldCols, pBox, bRow );
        }
    }
}
/* -----------------30.04.02 08:00-------------------
 *
 * --------------------------------------------------*/
inline rtl::OUString lcl_getString( SwXCell &rCell )
{
    // getString is a member function of the base class...
    return rCell.getString();
}
/* -----------------30.04.02 08:00-------------------
 * non UNO function call to set string in SwXCell
 * --------------------------------------------------*/
void lcl_setString( SwXCell &rCell, const rtl::OUString &rTxt,
        BOOL bKeepNumberFmt )
{
    if(rCell.IsValid())
    {
        SwFrmFmt* pBoxFmt = rCell.pBox->ClaimFrmFmt();
        pBoxFmt->LockModify();
        pBoxFmt->ResetAttr( RES_BOXATR_FORMULA );
        pBoxFmt->ResetAttr( RES_BOXATR_VALUE );
        if (!bKeepNumberFmt)
            pBoxFmt->SetAttr( SwTblBoxNumFormat(NUMBERFORMAT_TEXT) );
        pBoxFmt->UnlockModify();
    }
    rCell.SwXText::setString(rTxt);
}
/* -----------------30.04.02 08:00-------------------
 * non UNO function call to get value from SwXCell
 * --------------------------------------------------*/
double lcl_getValue( SwXCell &rCell )
{
    double fRet = 0.0;
    if(rCell.IsValid())
    {
        fRet = rCell.pBox->GetFrmFmt()->GetTblBoxValue().GetValue();
    }
    return fRet;
}
/* -----------------30.04.02 08:00-------------------
 * non UNO function call to set value in SwXCell
 * --------------------------------------------------*/
void lcl_setValue( SwXCell &rCell, double nVal )
{
    if(rCell.IsValid())
    {
        // Der Text mu? zunaechst (vielleicht) geloescht werden
        ULONG nNdPos = rCell.pBox->IsValidNumTxtNd( sal_True );
        if(ULONG_MAX != nNdPos)
            lcl_setString( rCell, OUString(), TRUE );   // TRUE == keep number format
        SwDoc* pDoc = rCell.GetDoc();
        UnoActionContext aAction(pDoc);
        SwFrmFmt* pBoxFmt = rCell.pBox->ClaimFrmFmt();
        SfxItemSet aSet(pDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE);
        const SfxPoolItem* pItem;

        //!! do we need to set a new number format? Yes, if
        // - there is no current number format
        // - the current number format is not a number format according to the number formatter, but rather a text format
        // - the current number format is not even a valid number formatter number format, but rather Writer's own 'special' text number format
        if(SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
            ||  pDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue())
            ||  ((SwTblBoxNumFormat*)pItem)->GetValue() == NUMBERFORMAT_TEXT)
        {
            aSet.Put(SwTblBoxNumFormat(0));
        }

        SwTblBoxValue aVal(nVal);
        aSet.Put(aVal);
        pDoc->SetTblBoxFormulaAttrs( *rCell.pBox, aSet );
        //Tabelle aktualisieren
        SwTableFmlUpdate aTblUpdate( SwTable::FindTable( rCell.GetFrmFmt() ));
        pDoc->UpdateTblFlds( &aTblUpdate );
    }
}

/******************************************************************************
 *
 ******************************************************************************/

const SfxItemPropertyMap* GetTableDescPropertyMap()
{
    static SfxItemPropertyMap aTableDescPropertyMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_BACK_COLOR )         ,  RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE,MID_BACK_COLOR         },
        { SW_PROP_NAME(UNO_NAME_BREAK_TYPE),            RES_BREAK,              &::getCppuType((const style::BreakType*)0),         PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_GRAPHIC_URL      ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { SW_PROP_NAME(UNO_NAME_GRAPHIC_FILTER  ),      RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { SW_PROP_NAME(UNO_NAME_GRAPHIC_LOCATION)    ,      RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { SW_PROP_NAME(UNO_NAME_LEFT_MARGIN),           RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_L_MARGIN},
        { SW_PROP_NAME(UNO_NAME_RIGHT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN},
        { SW_PROP_NAME(UNO_NAME_HORI_ORIENT  ),         RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
        { SW_PROP_NAME(UNO_NAME_KEEP_TOGETHER),         RES_KEEP,               &::getBooleanCppuType()  ,  PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_SPLIT    ),             RES_LAYOUT_SPLIT,       &::getBooleanCppuType()  ,  PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_PAGE_NUMBER_OFFSET),    RES_PAGEDESC,           &::getCppuType((const sal_Int16*)0),        PROPERTY_NONE, MID_PAGEDESC_PAGENUMOFFSET},
        { SW_PROP_NAME(UNO_NAME_PAGE_STYLE_NAME),           0,                      &::getCppuType((const OUString*)0),         PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_RELATIVE_WIDTH),        FN_TABLE_RELATIVE_WIDTH,&::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE, 0    },
        { SW_PROP_NAME(UNO_NAME_REPEAT_HEADLINE) ,      FN_TABLE_HEADLINE_REPEAT,&::getBooleanCppuType(),       PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_SHADOW_FORMAT),         RES_SHADOW,             &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_IS_WIDTH_RELATIVE),     FN_TABLE_IS_RELATIVE_WIDTH,  &::getBooleanCppuType()  ,     PROPERTY_NONE, 0 },
        { SW_PROP_NAME(UNO_NAME_TABLE_NAME),          0,                      &::getCppuType((const OUString*)0),       PROPERTY_NONE, 0 },
        { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),            RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),         RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT),  RES_BACKGROUND,     &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { SW_PROP_NAME(UNO_NAME_WIDTH),                 FN_TABLE_WIDTH,         &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_CHART_ROW_AS_LABEL),        FN_UNO_RANGE_ROW_LABEL,         &::getBooleanCppuType(),            PROPERTY_NONE,  0},
        { SW_PROP_NAME(UNO_NAME_CHART_COLUMN_AS_LABEL),     FN_UNO_RANGE_COL_LABEL,         &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_TABLE_BORDER),          FN_UNO_TABLE_BORDER,            &::getCppuType((const table::TableBorder*)0),   beans::PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
        {0,0,0,0,0,0}
    };
    #define TABLE_PROP_COUNT 24
    return aTableDescPropertyMap_Impl;
}
/******************************************************************
 * SwXCell
 ******************************************************************/
TYPEINIT1(SwXCell, SwClient);
/*-- 11.12.98 10:56:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXCell::SwXCell(SwFrmFmt* pTblFmt, SwTableBox* pBx, sal_uInt16 nPos ) :
    SwXText(pTblFmt->GetDoc(), CURSOR_TBLTEXT),
    SwClient(pTblFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_CELL)),
    pBox(pBx),
    pStartNode(0),
    nFndPos(nPos)
{
}
/* -----------------------------09.08.00 15:59--------------------------------

 ---------------------------------------------------------------------------*/
SwXCell::SwXCell(SwFrmFmt* pTblFmt, const SwStartNode& rStartNode) :
    SwXText(pTblFmt->GetDoc(), CURSOR_TBLTEXT),
    SwClient(pTblFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_CELL)),
    pBox(0),
    pStartNode(&rStartNode),
    nFndPos(USHRT_MAX)
{
}

/*-- 11.12.98 10:56:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXCell::~SwXCell()
{

}
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXCell::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXCell::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    else
        return SwXText::getSomething(rId);
}
/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< uno::Type > SAL_CALL SwXCell::getTypes(  ) throw(uno::RuntimeException)
{
    static uno::Sequence< uno::Type > aRetTypes;
    if(!aRetTypes.getLength())
    {
        aRetTypes = SwXCellBaseClass::getTypes();
        uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();

        long nIndex = aRetTypes.getLength();
        aRetTypes.realloc(
            aRetTypes.getLength() +
            aTextTypes.getLength());

        uno::Type* pRetTypes = aRetTypes.getArray();

        const uno::Type* pTextTypes = aTextTypes.getConstArray();
        for(long nPos = 0; nPos <aTextTypes.getLength(); nPos++)
            pRetTypes[nIndex++] = pTextTypes[nPos];
    }
    return aRetTypes;
}
/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXCell::getImplementationId(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}
/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXCell::acquire(  ) throw()
{
    SwXCellBaseClass::acquire();
}
/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXCell::release(  ) throw()
{
    SwXCellBaseClass::release();
}
/* -----------------------------18.05.00 10:23--------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SAL_CALL SwXCell::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException)
{
    uno::Any aRet = SwXCellBaseClass::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXText::queryInterface(aType);
    return aRet;
}
/*-- 11.12.98 10:56:24---------------------------------------------------

  -----------------------------------------------------------------------*/
const SwStartNode *SwXCell::GetStartNode() const
{
    const SwStartNode *pSttNd = 0;

    if( pStartNode || ((SwXCell *)this)->IsValid() )
        pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();

    return pSttNd;
}

uno::Reference< text::XTextCursor >   SwXCell::createCursor() throw (uno::RuntimeException)
{
    return createTextCursor();
}
/*-- 11.12.98 10:56:24---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool    SwXCell::IsValid()
{
    SwFrmFmt* pTblFmt = pBox ? GetFrmFmt() : 0;
    if(!pTblFmt)
        pBox = 0;
    else
    {
        SwTable* pTable = SwTable::FindTable( pTblFmt );
        const SwTableBox* pFoundBox ;
        pFoundBox =  FindBox(pTable, pBox);
        if(!pFoundBox)
            pBox = 0;
    }
    return 0 != pBox;
}
/*-- 11.12.98 10:56:25---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXCell::getFormula(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString sRet;
    if(IsValid())
    {
        SwTblBoxFormula aFormula( pBox->GetFrmFmt()->GetTblBoxFormula() );
        SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
        aFormula.PtrToBoxNm( pTable );
        sRet = aFormula.GetFormula();
    }
    return sRet;
}
/*-- 11.12.98 10:56:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::setFormula(const OUString& rFormula) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(IsValid())
    {
        // Der Text mu? zunaechst (vielleicht) geloescht werden
        sal_uInt32 nNdPos = pBox->IsValidNumTxtNd( sal_True );
        if(USHRT_MAX == nNdPos)
            lcl_setString( *this, OUString(), TRUE );
        String sFml(rFormula);
        if( sFml.EraseLeadingChars().Len() && '=' == sFml.GetChar( 0 ) )
                    sFml.Erase( 0, 1 );
        SwTblBoxFormula aFml( sFml );
        SwDoc* pMyDoc = GetDoc();
        UnoActionContext aAction(pMyDoc);
        SfxItemSet aSet(pMyDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_FORMULA);
        const SfxPoolItem* pItem;
        SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
        if(SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
            ||  pMyDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue()))
        {
            aSet.Put(SwTblBoxNumFormat(0));
        }
        aSet.Put(aFml);
        GetDoc()->SetTblBoxFormulaAttrs( *pBox, aSet );
        //Tabelle aktualisieren
        SwTableFmlUpdate aTblUpdate( SwTable::FindTable( GetFrmFmt() ));
        pMyDoc->UpdateTblFlds( &aTblUpdate );
    }
}
/*-- 11.12.98 10:56:26---------------------------------------------------

  -----------------------------------------------------------------------*/
double SwXCell::getValue(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return lcl_getValue( *this );
}
/*-- 11.12.98 10:56:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::setValue(double rValue) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    lcl_setValue( *this, rValue );
}
/*-- 11.12.98 10:56:26---------------------------------------------------

  -----------------------------------------------------------------------*/
table::CellContentType SwXCell::getType(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    table::CellContentType nRes = table::CellContentType_EMPTY;
    sal_uInt32 nNdPos = pBox->IsFormulaOrValueBox();
    switch (nNdPos)
    {
        case 0 :                    nRes = table::CellContentType_TEXT; break;
        case USHRT_MAX :            nRes = table::CellContentType_EMPTY; break;
        case RES_BOXATR_VALUE :     nRes = table::CellContentType_VALUE; break;
        case RES_BOXATR_FORMULA :   nRes = table::CellContentType_FORMULA; break;
        default :
            DBG_ERROR( "unexpected case" );
    }
    return  nRes;
}
/* -----------------27.04.99 12:06-------------------
 *
 * --------------------------------------------------*/
void SwXCell::setString(const OUString& aString) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    lcl_setString( *this, aString );
}

/*-- 11.12.98 10:56:27---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXCell::getError(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString sContent = getString();
    return sContent.equals(ViewShell::GetShellRes()->aCalc_Error);
}
/*-- 11.12.98 10:56:28---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXCell::createTextCursor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextCursor >     aRef;
    if(pStartNode || IsValid())
    {
        const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        SwXTextCursor* pCrsr = new SwXTextCursor(this, aPos, CURSOR_TBLTEXT, GetDoc());
        SwUnoCrsr* pUnoCrsr = pCrsr->GetCrsr();
        pUnoCrsr->Move(fnMoveForward, fnGoNode);
        aRef =  (text::XWordCursor*)pCrsr;
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE | nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ))
//          throw( uno::RuntimeException() );
    }
    else
        throw uno::RuntimeException();
    return aRef;
}
/*-- 11.12.98 10:56:28---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXCell::createTextCursorByRange(const uno::Reference< text::XTextRange > & xTextPosition)
                                                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextCursor >  aRef;
    SwUnoInternalPaM aPam(*GetDoc());
    if((pStartNode || IsValid()) && SwXTextRange::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
        //skip sections
        SwStartNode* p1 = aPam.GetNode()->StartOfSectionNode();
        while(p1->IsSectionNode())
            p1 = p1->StartOfSectionNode();

        if( p1 == pSttNd )
            aRef =  (text::XWordCursor*)new SwXTextCursor(this , *aPam.GetPoint(), CURSOR_TBLTEXT, GetDoc(), aPam.GetMark());
    }
    else
        throw uno::RuntimeException();
    return aRef;
}
/*-- 11.12.98 10:56:33---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXCell::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 11.12.98 10:56:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(IsValid())
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
        if( !pMap )
        {
            beans::UnknownPropertyException aEx;
            aEx.Message = rPropertyName;
            throw( aEx );
        }
        if( pMap->nWID == FN_UNO_CELL_ROW_SPAN )
        {
            sal_Int32 nRowSpan = 0;
            if( aValue >>= nRowSpan )
                pBox->setRowSpan( nRowSpan );
        }
        else
        {
            SwFrmFmt* pBoxFmt = pBox->ClaimFrmFmt();
            SwAttrSet aSet(pBoxFmt->GetAttrSet());
            aPropSet.setPropertyValue(rPropertyName, aValue, aSet);
            pBoxFmt->GetDoc()->SetAttr(aSet, *pBoxFmt);
        }
    }
}
/*-- 11.12.98 10:56:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXCell::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    if(IsValid())
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
        if( !pMap )
        {
            beans::UnknownPropertyException aEx;
            aEx.Message = rPropertyName;
            throw( aEx );
        }
        switch( pMap->nWID )
        {
            case FN_UNO_CELL_ROW_SPAN:
                aRet <<= pBox->getRowSpan();
            break;
            case FN_UNO_TEXT_SECTION:
            {
                SwFrmFmt* pTblFmt = GetFrmFmt();
                SwTable* pTable = SwTable::FindTable( pTblFmt );
                SwTableNode* pTblNode = pTable->GetTableNode();
                SwSectionNode* pSectionNode =  pTblNode->FindSectionNode();
                if(pSectionNode)
                {
                    const SwSection& rSect = pSectionNode->GetSection();
                    uno::Reference< text::XTextSection >  xSect =
                                    SwXTextSections::GetObject( *rSect.GetFmt() );
                    aRet <<= xSect;
                }
            }
            break;
            case FN_UNO_CELL_NAME:
                aRet <<= OUString ( pBox->GetName() );
            break;
            case FN_UNO_REDLINE_NODE_START:
            case FN_UNO_REDLINE_NODE_END:
            {
                //redline can only be returned if it's a living object
                aRet = SwXText::getPropertyValue(rPropertyName);
            }
            break;
            default:
            {
                const SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
                const SwAttrSet& rSet = pBoxFmt->GetAttrSet();
                aRet = aPropSet.getPropertyValue(rPropertyName, rSet);
            }
        }
    }
    return aRet;
}
/*-- 11.12.98 10:56:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 10:56:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 10:56:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 10:56:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 10:56:37---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXCell::createEnumeration(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< container::XEnumeration >  aRef;
    if(IsValid())
    {
        const SwStartNode* pSttNd = pBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        SwUnoCrsr* pUnoCrsr = GetDoc()->CreateUnoCrsr(aPos, sal_False);
        pUnoCrsr->Move( fnMoveForward, fnGoNode );

        SwXParagraphEnumeration *pEnum = new SwXParagraphEnumeration(this, pUnoCrsr, CURSOR_TBLTEXT);
        const SwTableNode* pTblNode = pSttNd->FindTableNode();
        // remember table and start node for later travelling
        // (used in export of tables in tables)
        pEnum->SetOwnTable( &pTblNode->GetTable() );
        pEnum->SetOwnStartNode( pSttNd );

        aRef = pEnum;
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE | nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ))
//          throw( uno::RuntimeException() );
    }
    return aRef;
}
/*-- 11.12.98 10:56:38---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SAL_CALL SwXCell::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<text::XTextRange>*)0);

}
/*-- 11.12.98 10:56:38---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXCell::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}
/*-- 11.12.98 10:56:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}
/* -----------------12.06.98 07:54-------------------
 *
 * --------------------------------------------------*/
SwXCell* SwXCell::CreateXCell(SwFrmFmt* pTblFmt, SwTableBox* pBox, SwTable *pTable )
{
    SwXCell* pRet = 0;
    if(pTblFmt && pBox)
    {
        if( !pTable )
            pTable = SwTable::FindTable( pTblFmt );
        sal_uInt16 nPos = USHRT_MAX;
        SwTableBox* pFoundBox =
            pTable->GetTabSortBoxes().Seek_Entry( pBox, &nPos ) ? pBox : NULL;

        //wenn es die Box gibt, dann wird auch eine Zelle zurueckgegeben
        if(pFoundBox)
        {
            SwClientIter aIter( *pTblFmt );
            SwXCell* pXCell = (SwXCell*)aIter.
                                    First( TYPE( SwXCell ));
            while( pXCell )
            {
                // gibt es eine passende Zelle bereits?
                if(pXCell->GetTblBox() == pBox)
                    break;
                pXCell = (SwXCell*)aIter.Next();
            }
            //sonst anlegen
            if(!pXCell)
                pXCell = new SwXCell(pTblFmt, pBox, nPos );
            pRet = pXCell;
        }
    }
    return pRet;
}
/* -----------------12.06.98 07:37-------------------
 *  exitstiert die Box in der angegebenen Tabelle?
 * --------------------------------------------------*/
SwTableBox* SwXCell::FindBox(SwTable* pTable, SwTableBox* pBox2)
{
    // check if nFndPos happens to point to the right table box
    if( nFndPos < pTable->GetTabSortBoxes().Count() &&
        pBox2 == pTable->GetTabSortBoxes()[ nFndPos ] )
        return pBox2;

    // if not, seek the entry (and return, if successful)
    if( pTable->GetTabSortBoxes().Seek_Entry( pBox2, &nFndPos ))
        return pBox2;

    // box not found: reset nFndPos pointer
    nFndPos = USHRT_MAX;
    return 0;
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXCell::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXCell");
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXCell::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.CellProperties");
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXCell::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.CellProperties");
    return aRet;
}

/******************************************************************
 * SwXTextTableRow
 ******************************************************************/
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextTableRow::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextTableRow");
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextTableRow::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.text.TextTableRow") == rServiceName;
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextTableRow::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextTableRow");
    return aRet;
}
TYPEINIT1(SwXTextTableRow, SwClient);
/*-- 11.12.98 12:04:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTableRow::SwXTextTableRow(SwFrmFmt* pFmt, SwTableLine* pLn) :
    SwClient(pFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE_ROW)),
    pLine(pLn)
{

}
/*-- 11.12.98 12:04:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTableRow::~SwXTextTableRow()
{

}
/*-- 11.12.98 12:04:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextTableRow::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 11.12.98 12:04:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableLine* pLn = SwXTextTableRow::FindLine(pTable, pLine);
        if(pLn)
        {
            const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
            SwDoc* pDoc = pFmt->GetDoc();
            if (!pMap)
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            if ( pMap->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            switch(pMap->nWID)
            {
                case FN_UNO_ROW_HEIGHT:
                case FN_UNO_ROW_AUTO_HEIGHT:
                {
                    SwFmtFrmSize aFrmSize(pLn->GetFrmFmt()->GetFrmSize());
                    if(FN_UNO_ROW_AUTO_HEIGHT== pMap->nWID)
                    {
                        sal_Bool bSet = *(sal_Bool*)aValue.getValue();
                        aFrmSize.SetHeightSizeType(bSet ? ATT_VAR_SIZE : ATT_FIX_SIZE);
                    }
                    else
                    {
                        sal_Int32 nHeight = 0;
                        aValue >>= nHeight;
                         Size aSz(aFrmSize.GetSize());
                        aSz.Height() = MM100_TO_TWIP(nHeight);
                        aFrmSize.SetSize(aSz);
                    }
                    pDoc->SetAttr(aFrmSize, *pLn->ClaimFrmFmt());
                }
                break;
                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    UnoActionContext aContext(pDoc);
                    SwTable* pTable2 = SwTable::FindTable( pFmt );
                    lcl_SetTblSeparators(aValue, pTable2, pLine->GetTabBoxes()[0], sal_True, pDoc);
                }
                break;
                default:
                {
                    SwFrmFmt* pLnFmt = pLn->ClaimFrmFmt();
                    SwAttrSet aSet(pLnFmt->GetAttrSet());
                    aPropSet.setPropertyValue(*pMap, aValue, aSet);
                    pDoc->SetAttr(aSet, *pLnFmt);
                }
            }
        }
    }
}
/*-- 11.12.98 12:04:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextTableRow::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableLine* pLn = SwXTextTableRow::FindLine(pTable, pLine);
        if(pLn)
        {
            const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
            if (!pMap)
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            switch(pMap->nWID)
            {
                case FN_UNO_ROW_HEIGHT:
                case FN_UNO_ROW_AUTO_HEIGHT:
                {
                    const SwFmtFrmSize& rSize = pLn->GetFrmFmt()->GetFrmSize();
                    if(FN_UNO_ROW_AUTO_HEIGHT== pMap->nWID)
                    {
                        BOOL bTmp =  ATT_VAR_SIZE == rSize.GetHeightSizeType();
                        aRet.setValue(&bTmp, ::getCppuBooleanType());
                    }
                    else
                        aRet <<= (sal_Int32)(TWIP_TO_MM100(rSize.GetSize().Height()));
                }
                break;
                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    lcl_GetTblSeparators(aRet, pTable, pLine->GetTabBoxes()[0], sal_True);
                }
                break;
                default:
                {
                    const SwAttrSet& rSet = pLn->GetFrmFmt()->GetAttrSet();
                    aRet = aPropSet.getPropertyValue(*pMap, rSet);
                }
            }
        }
    }
    return aRet;
}
/*-- 11.12.98 12:04:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:04:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:04:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:04:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:04:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}
/*-- 11.12.98 12:04:50---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTableLine* SwXTextTableRow::FindLine(SwTable* pTable, SwTableLine* pLine)
{
    SwTableLine* pRet = 0;
    SwTableLines &rLines = pTable->GetTabLines();
    for(sal_uInt16 i = 0; i < rLines.Count(); i++)
        if(rLines.GetObject(i) == pLine)
        {
            pRet = pLine;
            break;
        }
    return pRet;
}

/******************************************************************
 * SwXTextTableCursor
 ******************************************************************/
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextTableCursor::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextTableCursor");
}
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextTableCursor::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.text.TextTableCursor") == rServiceName;
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(SwXTextTableCursor,SwXTextTableCursor_Base,OTextCursorHelper)
const SwPaM*        SwXTextTableCursor::GetPaM() const  { return GetCrsr(); }
SwPaM*              SwXTextTableCursor::GetPaM()        { return GetCrsr(); }
const SwDoc*        SwXTextTableCursor::GetDoc() const  { return GetFrmFmt()->GetDoc(); }
SwDoc*              SwXTextTableCursor::GetDoc()        { return GetFrmFmt()->GetDoc(); }
const SwUnoCrsr*    SwXTextTableCursor::GetCrsr() const { return (SwUnoCrsr*)aCrsrDepend.GetRegisteredIn(); }
SwUnoCrsr*          SwXTextTableCursor::GetCrsr()       { return (SwUnoCrsr*)aCrsrDepend.GetRegisteredIn(); }
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextTableCursor::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextTableCursor");
    return aRet;
}

/*-- 11.12.98 12:16:13---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTableCursor::SwXTextTableCursor(SwFrmFmt* pFmt, SwTableBox* pBox) :
    SwClient(pFmt),
    aCrsrDepend(this, 0),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE_CURSOR))
{
    SwDoc* pDoc = pFmt->GetDoc();
    const SwStartNode* pSttNd = pBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(aPos, sal_True);
    pUnoCrsr->Move( fnMoveForward, fnGoNode );
    pUnoCrsr->Add(&aCrsrDepend);
    SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
    pTblCrsr->MakeBoxSels();
}
/*-- 11.12.98 12:16:14---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTableCursor::SwXTextTableCursor(SwFrmFmt& rTableFmt, const SwTableCursor* pTableSelection) :
    SwClient(&rTableFmt),
    aCrsrDepend(this, 0),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE_CURSOR))
{
    SwUnoCrsr* pUnoCrsr = pTableSelection->GetDoc()->CreateUnoCrsr(*pTableSelection->GetPoint(), sal_True);
    if(pTableSelection->HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pTableSelection->GetMark();
    }
    const SwSelBoxes& rBoxes = pTableSelection->GetBoxes();
    SwTableCursor* pTableCrsr = (SwTableCursor*) *pUnoCrsr;
    for(sal_uInt16 i = 0; i < rBoxes.Count(); i++)
        pTableCrsr->InsertBox( *rBoxes.GetObject(i) );

    pUnoCrsr->Add(&aCrsrDepend);
    SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
    pTblCrsr->MakeBoxSels();
}
/*-- 11.12.98 12:16:14---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTableCursor::~SwXTextTableCursor()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        delete pUnoCrsr;
}
/*-- 11.12.98 12:16:15---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextTableCursor::getRangeName(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    //!! see also SwChartDataSequence::getSourceRangeRepresentation
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        pTblCrsr->MakeBoxSels();
        const SwStartNode* pNode = pTblCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
        const SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
        const SwTableBox* pEndBox = pTable->GetTblBox( pNode->GetIndex());
        String aTmp( pEndBox->GetName() );

        if(pTblCrsr->HasMark())
        {
            pNode = pTblCrsr->GetMark()->nNode.GetNode().FindTableBoxStartNode();
            const SwTableBox* pStartBox = pTable->GetTblBox( pNode->GetIndex());
            if(pEndBox != pStartBox)
            {
                // need to switch start and end?
                if (*pTblCrsr->GetPoint() < *pTblCrsr->GetMark())
                {
                    const SwTableBox* pTmpBox = pStartBox;
                    pStartBox = pEndBox;
                    pEndBox = pTmpBox;
                }

                aTmp  = pStartBox->GetName();
                aTmp += ':';
                aTmp += pEndBox->GetName();
            }
        }
        aRet = aTmp;
    }
    return aRet;
}
/*-- 11.12.98 12:16:15---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextTableCursor::gotoCellByName(const OUString& CellName, sal_Bool Expand)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect( pTblCrsr, Expand );
        String sCellName(CellName);
        bRet = pTblCrsr->GotoTblBox(sCellName);
    }
    return bRet;
}
/*-- 11.12.98 12:16:15---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextTableCursor::goLeft(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect( pTblCrsr, Expand );
        bRet = pTblCrsr->Left( Count,CRSR_SKIP_CHARS, FALSE, FALSE);
    }
    return bRet;
}
/*-- 11.12.98 12:16:15---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextTableCursor::goRight(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect( pTblCrsr, Expand );
        bRet = pTblCrsr->Right( Count, CRSR_SKIP_CHARS, FALSE, FALSE);
    }
    return bRet;
}
/*-- 11.12.98 12:16:16---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextTableCursor::goUp(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect( pTblCrsr, Expand );
        bRet = pTblCrsr->UpDown(sal_True, Count, 0, 0);
    }
    return bRet;
}
/*-- 11.12.98 12:16:16---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextTableCursor::goDown(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect( pTblCrsr, Expand );
        bRet = pTblCrsr->UpDown(sal_False, Count, 0, 0);
    }
    return bRet;
}
/*-- 11.12.98 12:16:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::gotoStart(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect( pTblCrsr, Expand );
        pTblCrsr->MoveTable(fnTableCurr, fnTableStart);
    }
}
/*-- 11.12.98 12:16:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::gotoEnd(sal_Bool Expand) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect( pTblCrsr, Expand );
        pTblCrsr->MoveTable(fnTableCurr, fnTableEnd);
    }
}
/*-- 11.12.98 12:16:16---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextTableCursor::mergeRange(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        {
            // hier muessen die Actions aufgehoben werden
            UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
        }
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        pTblCrsr->MakeBoxSels();

        {
            UnoActionContext aContext(pUnoCrsr->GetDoc());
            bRet = TBLMERGE_OK == pTblCrsr->GetDoc()->MergeTbl(*pTblCrsr);
            if(bRet)
            {
                USHORT nCount = pTblCrsr->GetBoxesCount();
                while(nCount--)
                    pTblCrsr->DeleteBox(nCount);
            }
        }
        pTblCrsr->MakeBoxSels();
    }
    return bRet;
}
/*-- 11.12.98 12:16:16---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextTableCursor::splitRange(sal_Int16 Count, sal_Bool Horizontal) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (Count <= 0)
        throw uno::RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM( "Illegal first argument: needs to be > 0" ) ), static_cast < cppu::OWeakObject * > ( this ) );
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        {
            // hier muessen die Actions aufgehoben werden
            UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
        }
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        pTblCrsr->MakeBoxSels();
        {
            UnoActionContext aContext(pUnoCrsr->GetDoc());
            bRet = pTblCrsr->GetDoc()->SplitTbl( pTblCrsr->GetBoxes(), !Horizontal, Count );
        }
        pTblCrsr->MakeBoxSels();
    }
    return bRet;
}
/*-- 11.12.98 12:16:17---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextTableCursor::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 11.12.98 12:16:17---------------------------------------------------

  -----------------------------------------------------------------------*/
extern sal_Bool lcl_setCrsrPropertyValue(const SfxItemPropertyMap* pMap,
            SwPaM& rPam,
            SfxItemSet& rSet,
            const uno::Any& aValue ) throw (lang::IllegalArgumentException);


void SwXTextTableCursor::setPropertyValue(const OUString& rPropertyName,
                                                        const uno::Any& aValue)
            throw( beans::UnknownPropertyException,
                        beans::PropertyVetoException,
                     lang::IllegalArgumentException,
                     lang::WrappedTargetException,
                     uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode()->StartOfSectionNode();
        const SwTableNode* pTblNode = pSttNode->FindTableNode();
        lcl_FormatTable((SwFrmFmt*)pTblNode->GetTable().GetFrmFmt());
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                    aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            pTblCrsr->MakeBoxSels();
            SwDoc* pDoc = pUnoCrsr->GetDoc();
            switch(pMap->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    pDoc->GetBoxAttr( *pUnoCrsr, aBrush );
                    aBrush.PutValue(aValue, pMap->nMemberId);
                    pDoc->SetBoxAttr( *pUnoCrsr, aBrush );

                }
                break;
                case RES_BOXATR_FORMAT:
                {
                    SfxUInt32Item aNumberFormat(RES_BOXATR_FORMAT);
                    aNumberFormat.PutValue(aValue, 0);
                    pDoc->SetBoxAttr( *pUnoCrsr, aNumberFormat);
                }
                break;
                case FN_UNO_PARA_STYLE:
                    lcl_SetTxtFmtColl(aValue, *pUnoCrsr);
                break;
                default:
                {
                    SfxItemSet aItemSet( pDoc->GetAttrPool(), pMap->nWID, pMap->nWID );
                    SwXTextCursor::GetCrsrAttr( pTblCrsr->GetSelRing(), aItemSet );

                    if(!lcl_setCrsrPropertyValue( pMap, pTblCrsr->GetSelRing(), aItemSet, aValue ))
                        aPropSet.setPropertyValue( *pMap, aValue, aItemSet );
                    SwXTextCursor::SetCrsrAttr( pTblCrsr->GetSelRing(), aItemSet, CRSR_ATTR_MODE_TABLE );
                }
            }
        }
        else
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
}
/*-- 11.12.98 12:16:17---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextTableCursor::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode()->StartOfSectionNode();
        const SwTableNode* pTblNode = pSttNode->FindTableNode();
        lcl_FormatTable((SwFrmFmt*)pTblNode->GetTable().GetFrmFmt());
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                    aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            pTblCrsr->MakeBoxSels();
            switch(pMap->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    if(pTblCrsr->GetDoc()->GetBoxAttr( *pUnoCrsr, aBrush ))
                        aBrush.QueryValue(aRet, pMap->nMemberId);

                }
                break;
                case RES_BOXATR_FORMAT:
                    //GetAttr fuer Tabellenselektion am Doc fehlt noch
                    DBG_WARNING("not implemented")
                break;
                case FN_UNO_PARA_STYLE:
                {
                    SwFmtColl* pFmt = SwXTextCursor::GetCurTxtFmtColl(*pUnoCrsr, FALSE);
                    OUString sRet;
                    if(pFmt)
                        sRet = pFmt->GetName();
                    aRet <<= sRet;
                }
                break;
                default:
                {
                    SfxItemSet aSet(pTblCrsr->GetDoc()->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    // erstmal die Attribute des Cursors
                    SwXTextCursor::GetCrsrAttr(pTblCrsr->GetSelRing(), aSet);
                    aRet = aPropSet.getPropertyValue(*pMap, aSet);
                }
            }
        }
        else
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}
/*-- 11.12.98 12:16:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:16:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:16:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:16:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:16:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}
/******************************************************************
 * SwXTextTable
 ******************************************************************/
/****************************************************************************
    Tabellenbeschreibung
****************************************************************************/

class SwTableProperties_Impl
{
    const SfxItemPropertyMap*   _pMap;
    uno::Any*                   pAnyArr[TABLE_PROP_COUNT];
    sal_uInt16                      nArrLen;

public:
    SwTableProperties_Impl(const SfxItemPropertyMap* pMap);
    ~SwTableProperties_Impl();

    sal_Bool    SetProperty(const char* pName , uno::Any aVal);
    sal_Bool    GetProperty(const char* pName, uno::Any*& rpAny);

    sal_Bool    SetProperty(USHORT nId , uno::Any aVal)
    {   return SetProperty(SW_PROP_NAME_STR( nId ), aVal); }
    sal_Bool    GetProperty(USHORT nId, uno::Any*& rpAny)
    {   return GetProperty(SW_PROP_NAME_STR( nId ), rpAny); }

    const SfxItemPropertyMap*   GetMap() const {return _pMap;}
    void                        ApplyTblAttr(const SwTable& rTbl, SwDoc& rDoc);
};

/* -----------------22.06.98 09:43-------------------
 *
 * --------------------------------------------------*/
SwTableProperties_Impl::SwTableProperties_Impl(const SfxItemPropertyMap* pMap) :
    _pMap(pMap),
    nArrLen(TABLE_PROP_COUNT)
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        pAnyArr[i] = 0;
}
/* -----------------22.06.98 09:51-------------------
 *
 * --------------------------------------------------*/
SwTableProperties_Impl::~SwTableProperties_Impl()
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        delete pAnyArr[i];
}
/* -----------------22.06.98 09:51-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwTableProperties_Impl::SetProperty(const char* pName, uno::Any aVal)
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = _pMap;
    String aName(C2S( pName ));
    while( pTemp->pName )
    {
        if( aName.EqualsAscii(pTemp->pName))
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
    {
        delete pAnyArr[nPos];
        pAnyArr[nPos] = new uno::Any(aVal);
    }
    return nPos < nArrLen;
}
/* -----------------22.06.98 09:51-------------------
 *
 * --------------------------------------------------*/

sal_Bool SwTableProperties_Impl::GetProperty(const char* pName, uno::Any*& rpAny )
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = _pMap;
    String aName(C2S(pName));
    while( pTemp->pName )
    {
        if(aName.EqualsAscii(pTemp->pName))
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
        rpAny = pAnyArr[nPos];
    return rpAny && nPos < nArrLen;
}
/* -----------------13.01.99 15:42-------------------
 *
 * --------------------------------------------------*/
void    SwTableProperties_Impl::ApplyTblAttr(const SwTable& rTbl, SwDoc& rDoc)
{
    SfxItemSet aSet(rDoc.GetAttrPool(),
        RES_LAYOUT_SPLIT,   RES_LAYOUT_SPLIT,
        RES_BACKGROUND,     RES_BACKGROUND,
        RES_FRM_SIZE,       RES_UL_SPACE,
        RES_HORI_ORIENT,    RES_HORI_ORIENT,
        RES_BREAK,          RES_BREAK,
        RES_KEEP,           RES_KEEP,
        RES_SHADOW,         RES_SHADOW,
        RES_PAGEDESC,       RES_PAGEDESC,
        0
        );
    uno::Any* pRepHead;
    const SwFrmFmt &rFrmFmt = *rTbl.GetFrmFmt();
    if(GetProperty(UNO_NAME_REPEAT_HEADLINE, pRepHead ))
    {
        sal_Bool bVal = *(sal_Bool*)pRepHead->getValue();
        ((SwTable&)rTbl).SetRowsToRepeat( bVal ? 1 : 0 );  // TODO MULTIHEADER
    }

    uno::Any* pBackColor    = 0;
    GetProperty(UNO_NAME_BACK_COLOR, pBackColor );
    uno::Any* pBackTrans    = 0;
    GetProperty(UNO_NAME_BACK_TRANSPARENT, pBackTrans );
    uno::Any* pGrLoc        = 0;
    GetProperty(UNO_NAME_GRAPHIC_LOCATION, pGrLoc   );
    uno::Any* pGrURL        = 0;
    GetProperty(UNO_NAME_GRAPHIC_URL, pGrURL     );
    uno::Any* pGrFilter     = 0;
    GetProperty(UNO_NAME_GRAPHIC_FILTER, pGrFilter     );

    if(pBackColor||pBackTrans||pGrURL||pGrFilter||pGrLoc)
    {
        SvxBrushItem aBrush ( rFrmFmt.GetBackground() );
        if(pBackColor)
            aBrush.PutValue(*pBackColor, MID_BACK_COLOR);
        if(pBackTrans)
            aBrush.PutValue(*pBackTrans, MID_GRAPHIC_TRANSPARENT);
        if(pGrURL)
            aBrush.PutValue(*pGrURL, MID_GRAPHIC_URL);
        if(pGrFilter)
            aBrush.PutValue(*pGrFilter, MID_GRAPHIC_FILTER);
        if(pGrLoc)
            aBrush.PutValue(*pGrLoc, MID_GRAPHIC_POSITION);
        aSet.Put(aBrush);
    }

    sal_Bool bPutBreak = sal_True;
    uno::Any* pPage;
    if(GetProperty(UNO_NAME_PAGE_STYLE_NAME, pPage))
    {
        OUString uTmp;
        (*pPage) >>= uTmp;
        String sPageStyle = uTmp;
        if(sPageStyle.Len())
        {
            SwStyleNameMapper::FillUIName(sPageStyle, sPageStyle, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, sal_True );
            const SwPageDesc* pDesc = ::GetPageDescByName_Impl(rDoc, sPageStyle);
            if(pDesc)
            {
                SwFmtPageDesc aDesc( pDesc );
                uno::Any* pPgNo;
                if(GetProperty(UNO_NAME_PAGE_NUMBER_OFFSET, pPgNo ))
                {
                    INT16 nTmp = 0;
                    (*pPgNo) >>= nTmp;
                    aDesc.SetNumOffset( nTmp );
                }
                aSet.Put(aDesc);
                bPutBreak = sal_False;
            }

        }
    }
    uno::Any* pBreak;
    if(bPutBreak && GetProperty(UNO_NAME_BREAK_TYPE, pBreak))
    {
        SvxFmtBreakItem aBreak ( rFrmFmt.GetBreak() );
        aBreak.PutValue(*pBreak, 0);
        aSet.Put(aBreak);
    }
    uno::Any* pShadow;
    if(GetProperty(UNO_NAME_SHADOW_FORMAT, pShadow))
    {
        SvxShadowItem aShd ( rFrmFmt.GetShadow() );
        aShd.PutValue(*pShadow, CONVERT_TWIPS);
        aSet.Put(aShd);
    }
    uno::Any* pKeep;
    if(GetProperty(UNO_NAME_KEEP_TOGETHER, pKeep))
    {
        SvxFmtKeepItem aKeep( rFrmFmt.GetKeep() );
        aKeep.PutValue(*pKeep, 0);
        aSet.Put(aKeep);
    }

    sal_Bool bFullAlign = sal_True;
    uno::Any* pHOrient;
    if(GetProperty(UNO_NAME_HORI_ORIENT, pHOrient))
    {
        SwFmtHoriOrient aOrient ( rFrmFmt.GetHoriOrient() );
        ((SfxPoolItem&)aOrient).PutValue(*pHOrient, MID_HORIORIENT_ORIENT|CONVERT_TWIPS);
        bFullAlign = (aOrient.GetHoriOrient() == text::HoriOrientation::FULL);
        aSet.Put(aOrient);
    }


    uno::Any* pSzRel        = 0;
    GetProperty(UNO_NAME_IS_WIDTH_RELATIVE, pSzRel  );
    uno::Any* pRelWidth     = 0;
    GetProperty(UNO_NAME_RELATIVE_WIDTH, pRelWidth);
    uno::Any* pWidth        = 0;
    GetProperty(UNO_NAME_WIDTH, pWidth  );

    sal_Bool bPutSize = pWidth != 0;
    SwFmtFrmSize aSz( ATT_VAR_SIZE);
    if(pWidth)
    {
        ((SfxPoolItem&)aSz).PutValue(*pWidth, MID_FRMSIZE_WIDTH);
        bPutSize = sal_True;
    }
    sal_Bool bTemp = pSzRel ? *(sal_Bool*)pSzRel->getValue() : FALSE;
    if(pSzRel && bTemp && pRelWidth)
    {
        ((SfxPoolItem&)aSz).PutValue(*pRelWidth, MID_FRMSIZE_REL_WIDTH|CONVERT_TWIPS);
        bPutSize = sal_True;
    }
    if(bPutSize)
    {
        if(!aSz.GetWidth())
            aSz.SetWidth(MINLAY);
        aSet.Put(aSz);
    }
    uno::Any* pL        = 0;
    GetProperty(UNO_NAME_LEFT_MARGIN, pL);
    uno::Any* pR        = 0;
    GetProperty(UNO_NAME_RIGHT_MARGIN, pR);
    if(pL||pR)
    {
        SvxLRSpaceItem aLR ( rFrmFmt.GetLRSpace() );
        if(pL)
            ((SfxPoolItem&)aLR).PutValue(*pL, MID_L_MARGIN|CONVERT_TWIPS);
        if(pR)
            ((SfxPoolItem&)aLR).PutValue(*pR, MID_R_MARGIN|CONVERT_TWIPS);
        aSet.Put(aLR);
    }
    uno::Any* pU        = 0;
    GetProperty(UNO_NAME_TOP_MARGIN, pU);
    uno::Any* pLo   = 0;
    GetProperty(UNO_NAME_BOTTOM_MARGIN, pLo);
    if(pU||pLo)
    {
        SvxULSpaceItem aUL ( rFrmFmt.GetULSpace() );
        if(pU)
            ((SfxPoolItem&)aUL).PutValue(*pU, MID_UP_MARGIN|CONVERT_TWIPS);
        if(pLo)
            ((SfxPoolItem&)aUL).PutValue(*pLo, MID_LO_MARGIN|CONVERT_TWIPS);
        aSet.Put(aUL);
    }
    uno::Any* pSplit;
    if(GetProperty(UNO_NAME_SPLIT, pSplit ))
    {
        sal_Bool bTmp = *(sal_Bool*)pSplit->getValue();
        SwFmtLayoutSplit aSp(bTmp);
        aSet.Put(aSp);
    }

    //TODO: folgende Propertiers noch impl.
//  FN_UNO_RANGE_ROW_LABEL
//  FN_UNO_RANGE_COL_LABEL
//  FN_UNO_TABLE_BORDER

    if(aSet.Count())
    {
        rDoc.SetAttr( aSet, *rTbl.GetFrmFmt() );
    }
}
/* -----------------------------11.07.00 12:14--------------------------------

 ---------------------------------------------------------------------------*/
SwXTextTable* SwXTextTable::GetImplementation(uno::Reference< XInterface> xRef )
{
    uno::Reference<lang::XUnoTunnel> xTunnel( xRef, uno::UNO_QUERY);
    if(xTunnel.is())
        return reinterpret_cast< SwXTextTable * >(
                sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(SwXTextTable::getUnoTunnelId()) ));
    return 0;
}
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextTable::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextTable::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}
/*-- 11.12.98 12:42:43---------------------------------------------------

  -----------------------------------------------------------------------*/
TYPEINIT1(SwXTextTable, SwClient)

/*-- 11.12.98 12:42:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTable::SwXTextTable() :
    aLstnrCntnr( (text::XTextTable*)this),
    aChartLstnrCntnr( (text::XTextTable*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    pTableProps(new SwTableProperties_Impl(GetTableDescPropertyMap())),
    bIsDescriptor(sal_True),
    nRows(2),
    nColumns(2),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False)
{

}
/*-- 11.12.98 12:42:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTable::SwXTextTable(SwFrmFmt& rFrmFmt) :
    SwClient( &rFrmFmt ),
    aLstnrCntnr( (text::XTextTable*)this),
    aChartLstnrCntnr( (text::XTextTable*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    pTableProps(0),
    bIsDescriptor(sal_False),
    nRows(0),
    nColumns(0),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False)
{

}
/*-- 11.12.98 12:42:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTable::~SwXTextTable()
{
    delete pTableProps;
}
/*-- 11.12.98 12:42:44---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::initialize(sal_Int32 nR, sal_Int32 nC) throw( uno::RuntimeException )
{
    if(!bIsDescriptor || nR <= 0 || nC <= 0 || nR >= USHRT_MAX || nC >= USHRT_MAX )
        throw uno::RuntimeException();
    else
    {
        nRows = (sal_uInt16)nR;
        nColumns = (sal_uInt16)nC;
    }
}
/*-- 11.12.98 12:42:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XTableRows >  SwXTextTable::getRows(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XTableRows >  xRet;
    if (SwFrmFmt* pFmt = GetFrmFmt())
    {
        SwXTableRows* pRows = (SwXTableRows*)SwClientIter(*pFmt).
            First(TYPE(SwXTableRows));
        if (!pRows)
            pRows = new SwXTableRows(*pFmt);
        xRet = pRows;
    }
    if (!xRet.is())
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 12:42:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XTableColumns >  SwXTextTable::getColumns(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XTableColumns >  xRet;
    if (SwFrmFmt* pFmt = GetFrmFmt())
    {
        SwXTableColumns* pCols = (SwXTableColumns*)SwClientIter(*pFmt).
            First(TYPE(SwXTableColumns));
        if (!pCols)
            pCols = new SwXTableColumns(*pFmt);
        xRet = pCols;
    }
    if (!xRet.is())
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 12:42:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCell >  SwXTextTable::getCellByName(const OUString& CellName) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XCell >  xRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        String sCellName(CellName);
        SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
        if(pBox)
        {
            xRet = SwXCell::CreateXCell(pFmt, pBox);
        }
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 12:42:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextTable::getCellNames(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
          // gibts an der Tabelle und an allen Boxen
        SwTableLines& rTblLines = pTable->GetTabLines();
        SvStrings aAllNames;
        lcl_InspectLines(rTblLines, aAllNames);
        uno::Sequence< OUString > aRet(aAllNames.Count());
        OUString* pArray = aRet.getArray();
        for(sal_uInt16 i = aAllNames.Count(); i; i--)
        {
            String* pObject = aAllNames.GetObject(i-1);
            pArray[i - 1] = *pObject;
            aAllNames.Remove(i - 1);
            delete pObject;
        }
        return aRet;
    }
    return uno::Sequence< OUString >();
}
/*-- 11.12.98 12:42:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextTableCursor >  SwXTextTable::createCursorByCellName(const OUString& CellName)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextTableCursor >  xRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        String sCellName(CellName);
        SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
        if(pBox)
        {
            xRet = new SwXTextTableCursor(pFmt, pBox);
        }
    }
    if(!xRet.is())
        throw uno::RuntimeException();
    return xRet;
}
/* -----------------18.02.99 13:36-------------------
 *
 * --------------------------------------------------*/
void SwXTextTable::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    // attachToRange must only be called once
    if(!bIsDescriptor)  /* already attached ? */
        throw uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "SwXTextTable: already attached to range." ) ), static_cast < cppu::OWeakObject * > ( this ) );

    uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
    }
    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc && nRows && nColumns)
    {
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);

        {
            UnoActionContext aCont( pDoc );

            pDoc->StartUndo(UNDO_EMPTY, NULL);
            const SwTable *pTable = 0;
            if( 0 != aPam.Start()->nContent.GetIndex() )
            {
                pDoc->SplitNode(*aPam.Start(), false );
            }
            //TODO: wenn es der letzte Absatz ist, dann muss noch ein Absatz angehaengt werden!
            if( aPam.HasMark() )
            {
                pDoc->DeleteAndJoin(aPam);
                aPam.DeleteMark();
            }
            pTable = pDoc->InsertTable( SwInsertTableOptions( tabopts::HEADLINE | tabopts::DEFAULT_BORDER | tabopts::SPLIT_LAYOUT, 0 ),
                                        *aPam.GetPoint(),
                                        nRows,
                                        nColumns,
                                        text::HoriOrientation::FULL );
            if(pTable)
            {
                // hier muessen die Properties des Descriptors ausgewertet werden
                pTableProps->ApplyTblAttr(*pTable, *pDoc);
                SwFrmFmt* pTblFmt = pTable->GetFrmFmt();
                SwClientIter aIter( *pTblFmt );
                for( SwClient* pC = aIter.First( TYPE( SwFrm ));
                        pC; pC = aIter.Next() )
                {
                    if( ((SwFrm*)pC)->IsTabFrm() )
                    {
                        if(((SwFrm*)pC)->IsValid())
                            ((SwFrm*)pC)->InvalidatePos();
                        ((SwTabFrm*)pC)->SetONECalcLowers();
                        ((SwTabFrm*)pC)->Calc();
                    }
                }

                pTblFmt->Add(this);
                if(m_sTableName.Len())
                {
                    sal_uInt16 nIndex = 1;
                    const String sTmpName(m_sTableName);
                    String sTmpNameIndex(sTmpName);
                    while(pDoc->FindTblFmtByName( sTmpNameIndex, sal_True ) && nIndex < USHRT_MAX)
                    {
                        sTmpNameIndex = sTmpName;
                        sTmpNameIndex += nIndex++;
                    }
                    pDoc->SetTableName( *pTblFmt, sTmpNameIndex);
                }

                uno::Any* pName;
                if(pTableProps->GetProperty(UNO_NAME_TABLE_NAME, pName))
                {
                    OUString sTmp;
                    (*pName) >>= sTmp;
                    setName(sTmp);
                }
                bIsDescriptor = sal_False;
                DELETEZ(pTableProps);
            }
            pDoc->EndUndo( UNDO_END, NULL );
        }

    }
    else
        throw lang::IllegalArgumentException();
}
/*-- 11.12.98 12:42:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::attach(const uno::Reference< text::XTextRange > & xTextRange)
        throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    attachToRange( xTextRange );
}
/*-- 11.12.98 12:42:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextTable::getAnchor(void)
        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!pFmt)
        throw uno::RuntimeException();
    uno::Reference< text::XTextRange >  xRet = new SwXTextRange(*pFmt);
    return xRet;
}
/*-- 11.12.98 12:42:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableSortBoxes& rBoxes = pTable->GetTabSortBoxes();
        SwSelBoxes aSelBoxes;
        aSelBoxes.Insert(rBoxes.GetData(), rBoxes.Count());
        pFmt->GetDoc()->DeleteRowCol(aSelBoxes);
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 12:42:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 11.12.98 12:42:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 11.12.98 12:42:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCell >  SwXTextTable::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XCell >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    // Sheet interessiert nicht
    if(nColumn >= 0 && nRow >= 0 && nColumn < USHRT_MAX && nRow < USHRT_MAX && pFmt)
    {
        SwXCell* pXCell = lcl_CreateXCell(pFmt, nColumn, nRow);
        if(pXCell)
            aRef = pXCell;
    }
    if(!aRef.is())
        throw lang::IndexOutOfBoundsException();
    return aRef;

}
/* -----------------11.12.98 13:26-------------------
 *
 * --------------------------------------------------*/
uno::Reference< table::XCellRange >  SwXTextTable::GetRangeByName(SwFrmFmt* pFmt, SwTable* pTable,
                    const String& rTLName, const String& rBRName,
                    SwRangeDescriptor& rDesc)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XCellRange >  aRef;
    String sTLName(rTLName);
    String sBRName(rBRName);
    const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
    if(pTLBox)
    {
        // hier muessen die Actions aufgehoben werden
        UnoActionRemoveContext aRemoveContext(pFmt->GetDoc());
        const SwStartNode* pSttNd = pTLBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        // Cursor in die obere linke Zelle des Ranges setzen
        SwUnoCrsr* pUnoCrsr = pFmt->GetDoc()->CreateUnoCrsr(aPos, sal_True);
        pUnoCrsr->Move( fnMoveForward, fnGoNode );
        pUnoCrsr->SetRemainInSection( sal_False );
        const SwTableBox* pBRBox = pTable->GetTblBox( sBRName );
        if(pBRBox)
        {
            pUnoCrsr->SetMark();
            pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
            pUnoCrsr->Move( fnMoveForward, fnGoNode );
            SwUnoTableCrsr* pCrsr = *pUnoCrsr;
            pCrsr->MakeBoxSels();
            // pUnoCrsr wird uebergeben und nicht geloescht
            SwXCellRange* pCellRange = new SwXCellRange(pUnoCrsr, *pFmt, rDesc);
            aRef = pCellRange;
        }
        else
            delete pUnoCrsr;
    }
    return aRef;
}
/*-- 11.12.98 12:42:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCellRange >  SwXTextTable::getCellRangeByPosition(sal_Int32 nLeft, sal_Int32 nTop,
                sal_Int32 nRight, sal_Int32 nBottom)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XCellRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt && nRight < USHRT_MAX && nBottom < USHRT_MAX &&
        nLeft <= nRight && nTop <= nBottom &&
            nLeft >= 0 && nRight >= 0 && nTop >= 0 && nBottom >= 0 )
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            SwRangeDescriptor aDesc;
            aDesc.nTop    = nTop;
            aDesc.nBottom = nBottom;
            aDesc.nLeft   = nLeft;
            aDesc.nRight  = nRight;
            String sTLName = lcl_GetCellName(aDesc.nLeft, aDesc.nTop);
            String sBRName = lcl_GetCellName(aDesc.nRight, aDesc.nBottom);

            // please note that according to the 'if' statement at the begin
            // sTLName:sBRName already denotes the normalized range string

            aRef = GetRangeByName(pFmt, pTable, sTLName, sBRName, aDesc);
        }
    }
    if(!aRef.is())
        throw lang::IndexOutOfBoundsException();
    return aRef;
}
/*-- 11.12.98 12:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCellRange >  SwXTextTable::getCellRangeByName(const OUString& aRange)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XCellRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            String sRange(aRange);
            String sTLName(sRange.GetToken(0, ':'));
            String sBRName(sRange.GetToken(1, ':'));
            if(!sTLName.Len() || !sBRName.Len())
                throw uno::RuntimeException();
            SwRangeDescriptor aDesc;
            aDesc.nTop = aDesc.nLeft = aDesc.nBottom = aDesc.nRight = -1;
            lcl_GetCellPosition(sTLName, aDesc.nLeft, aDesc.nTop );
            lcl_GetCellPosition(sBRName, aDesc.nRight, aDesc.nBottom );

            // we should normalize the range now (e.g. A5:C1 will become A1:C5)
            // since (depending on what is done later) it will be troublesome
            // elsewhere when the cursor in the implementation does not
            // point to the top-left and bottom-right cells
            aDesc.Normalize();

            aRef = GetRangeByName(pFmt, pTable, sTLName, sBRName, aDesc);
        }
    }
    if(!aRef.is())
        throw uno::RuntimeException();
    return aRef;
}
/*-- 29.04.02 11:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Sequence< uno::Any > > SAL_CALL SwXTextTable::getDataArray()
    throw (uno::RuntimeException)
{
    // see SwXTextTable::getData(...) also

    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    uno::Sequence< uno::Sequence< uno::Any > > aRowSeq(nRowCount);
    if(pFmt)
    {
        uno::Sequence< uno::Any > * pRowArray = aRowSeq.getArray();
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            uno::Sequence< uno::Any >  aColSeq(nColCount);
            uno::Any * pColArray = aColSeq.getArray();
            uno::Reference< table::XCell > xCellRef;
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell* pXCell = lcl_CreateXCell(pFmt, nCol, nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    // check if table box value item is set
                    SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
                    BOOL bIsNum = pBoxFmt->GetItemState( RES_BOXATR_VALUE, FALSE ) == SFX_ITEM_SET;
                    //const SfxPoolItem* pItem;
                    //SwDoc* pDoc = pXCell->GetDoc();
                    //BOOL bIsText = (SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
                    //          ||  pDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue())
                    //          ||  ((SwTblBoxNumFormat*)pItem)->GetValue() == NUMBERFORMAT_TEXT);

                    if(!bIsNum/*bIsText*/)
                        pColArray[nCol] <<= lcl_getString(*pXCell);
                    else
                        pColArray[nCol] <<= lcl_getValue(*pXCell);
                }
            }
            pRowArray[nRow] = aColSeq;
        }
    }
    else
        throw uno::RuntimeException();
    return aRowSeq;
}
/*-- 29.04.02 11:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextTable::setDataArray(
        const uno::Sequence< uno::Sequence< uno::Any > >& rArray )
    throw (uno::RuntimeException)
{
    // see SwXTextTable::setData(...) also

    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();

    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(pTable->IsTblComplex())
        {
            uno::RuntimeException aRuntime;
            aRuntime.Message = C2U("Table too complex");
            throw aRuntime;
        }

        if(rArray.getLength() != nRowCount)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< uno::Any >* pRowArray = rArray.getConstArray();
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< uno::Any >& rColSeq = pRowArray[nRow];
            if(rColSeq.getLength() != nColCount)
            {
                throw uno::RuntimeException();
            }
            const uno::Any * pColArray = rColSeq.getConstArray();
            uno::Reference< table::XCell > xCellRef;
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell* pXCell = lcl_CreateXCell(pFmt, nCol, nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    const uno::Any &rAny = pColArray[nCol];
                    if (uno::TypeClass_STRING == rAny.getValueTypeClass())
                        lcl_setString( *pXCell, *(rtl::OUString *) rAny.getValue() );
                    else
                    {
                        double d = 0;
                        // #i20067# don't throw exception just do nothing if
                        // there is no value set
                        if( (rAny >>= d) )
                            lcl_setValue( *pXCell, d );
                        else
                            lcl_setString( *pXCell, OUString(), TRUE );

                    }
                }
            }
        }
    }
}
/*-- 11.12.98 12:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Sequence< double > > SwXTextTable::getData(void)
                                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    //
    SwFrmFmt* pFmt = GetFrmFmt();
    uno::Sequence< uno::Sequence< double > > aRowSeq(bFirstRowAsLabel ? nRowCount - 1 : nRowCount);
    if(pFmt)
    {
        uno::Sequence< double >* pArray = aRowSeq.getArray();

        sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            uno::Sequence< double >  aColSeq(bFirstColumnAsLabel ? nColCount - 1 : nColCount);
            double* pColArray = aColSeq.getArray();
            sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                pColArray[nCol - nColStart] = xCell->getValue();
            }
            pArray[nRow - nRowStart] = aColSeq;
        }
    }
    else
        throw uno::RuntimeException();
    return aRowSeq;
}
/*-- 11.12.98 12:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::setData(const uno::Sequence< uno::Sequence< double > >& rData)
                                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    sal_Bool bChanged = sal_False;

    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        if(rData.getLength() < nRowCount - nRowStart)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< double >* pRowArray = rData.getConstArray();
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< double >& rColSeq = pRowArray[nRow - nRowStart];
            sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
            if(rColSeq.getLength() < nColCount - nColStart)
            {
                throw uno::RuntimeException();
            }
            const double * pColArray = rColSeq.getConstArray();
            for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                xCell->setValue(pColArray[nCol - nColStart]);
                bChanged=sal_True;
            }
        }
        if ( bChanged )
            aChartLstnrCntnr.ChartDataChanged();
    }
}
/*-- 11.12.98 12:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextTable::getRowDescriptions(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    if(!nRowCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    uno::Sequence< OUString > aRet(bFirstColumnAsLabel ? nRowCount - 1 : nRowCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        OUString* pArray = aRet.getArray();
        if(bFirstColumnAsLabel)
        {
            sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nRowCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(0, i);
                if(!xCell.is())
                {
                    //exception ...
                    break;
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?")
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 12:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::setRowDescriptions(const uno::Sequence< OUString >& rRowDesc) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        sal_Int16 nRowCount = getRowCount();
        if(!nRowCount || rRowDesc.getLength() < (bFirstRowAsLabel ? nRowCount - 1 : nRowCount))
        {
            throw uno::RuntimeException();
        }
        const OUString* pArray = rRowDesc.getConstArray();
        if(bFirstColumnAsLabel)
        {
            sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nRowCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(0, i);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            DBG_ERROR("Wohin mit den Labels?")
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 12:42:48---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextTable::getColumnDescriptions(void)
                                                throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nColCount = getColumnCount();
    if(!nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    uno::Sequence< OUString > aRet(bFirstRowAsLabel ? nColCount - 1 : nColCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        OUString* pArray = aRet.getArray();
        if(bFirstRowAsLabel)
        {
            sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nColCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(i, 0);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);

                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?")
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 12:42:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::setColumnDescriptions(const uno::Sequence< OUString >& rColumnDesc) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nColCount = getColumnCount();
    if(!nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const OUString* pArray = rColumnDesc.getConstArray();
        if(bFirstRowAsLabel && rColumnDesc.getLength() >= nColCount - bFirstColumnAsLabel ? 1 : 0)
        {
            sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nColCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(i, 0);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?")
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 12:42:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::addChartDataChangeEventListener(
    const uno::Reference< chart::XChartDataChangeEventListener > & aListener)
        throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aChartLstnrCntnr.AddListener(aListener.get());
}
/*-- 11.12.98 12:42:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::removeChartDataChangeEventListener(
    const uno::Reference< chart::XChartDataChangeEventListener > & aListener)
        throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aChartLstnrCntnr.RemoveListener(aListener.get()))
        throw uno::RuntimeException();
}
/* -----------------08.03.99 15:33-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextTable::isNotANumber(double nNumber) throw( uno::RuntimeException )
{
    // We use DBL_MIN because starcalc does (which uses it because chart
    // wants it that way!)
    return ( nNumber == DBL_MIN );
}
/* -----------------08.03.99 15:34-------------------
 *
 * --------------------------------------------------*/
double SwXTextTable::getNotANumber(void) throw( uno::RuntimeException )
{
    // We use DBL_MIN because starcalc does (which uses it because chart
    // wants it that way!)
    return DBL_MIN;
}
/*-- 11.12.98 12:42:48---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SwXTextTable::createSortDescriptor(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXTextCursor::createSortDescriptor(sal_True);
}
/*-- 11.12.98 12:42:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSortOptions aSortOpt;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt &&
        SwXTextCursor::convertSortProperties(rDescriptor, aSortOpt))
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwSelBoxes aBoxes;
        const SwTableSortBoxes& rTBoxes = pTable->GetTabSortBoxes();
        for( sal_uInt16 n = 0; n < rTBoxes.Count(); ++n )
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.Insert( pBox );
        }
        UnoActionContext aContext( pFmt->GetDoc() );
        pFmt->GetDoc()->SortTbl(aBoxes, aSortOpt);
    }
}
/*-- 11.12.98 12:42:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::autoFormat(const OUString& aName) throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {

            String sAutoFmtName(aName);
            SwTableAutoFmtTbl aAutoFmtTbl;
            aAutoFmtTbl.Load();
            for( sal_uInt16 i = aAutoFmtTbl.Count(); i; )
                if( sAutoFmtName == aAutoFmtTbl[ --i ]->GetName() )
                {
                    SwSelBoxes aBoxes;
                    const SwTableSortBoxes& rTBoxes = pTable->GetTabSortBoxes();
                    for( sal_uInt16 n = 0; n < rTBoxes.Count(); ++n )
                    {
                        SwTableBox* pBox = rTBoxes[ n ];
                        aBoxes.Insert( pBox );
                    }
                    UnoActionContext aContext( pFmt->GetDoc() );
                    pFmt->GetDoc()->SetTableAutoFmt( aBoxes, *aAutoFmtTbl[i] );
                    break;
                }
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 12:42:49---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextTable::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 11.12.98 12:42:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::setPropertyValue(const OUString& rPropertyName,
                                                    const uno::Any& aValue)
        throw( beans::UnknownPropertyException, beans::PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!aValue.hasValue())
        throw lang::IllegalArgumentException();
    if(pFmt)
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    _pMap, rPropertyName);
        if (!pMap)
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        if ( pMap->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        if(0xFF == pMap->nMemberId)
        {
            lcl_SetSpecialProperty(pFmt, pMap, aValue);
        }
        else
        {
            switch(pMap->nWID)
            {
                case FN_UNO_RANGE_ROW_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstRowAsLabel != bTmp)
                    {
                        aChartLstnrCntnr.ChartDataChanged();
                        bFirstRowAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstColumnAsLabel != bTmp)
                    {
                        aChartLstnrCntnr.ChartDataChanged();
                        bFirstColumnAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_TABLE_BORDER:
                {
                    const table::TableBorder* pBorder =
                            (const table::TableBorder* )aValue.getValue();
                    if(aValue.getValueType() == ::getCppuType((const table::TableBorder* )0)
                        && pBorder)
                    {
                        SwDoc* pDoc = pFmt->GetDoc();
                        SwClientIter aIter( *pFmt );
                        //Tabellen ohne Layout (unsichtbare Header/Footer )
                        if(0 != aIter.First( TYPE( SwFrm )))
                        {
                            lcl_FormatTable(pFmt);
                            SwTable* pTable = SwTable::FindTable( pFmt );
                            SwTableLines &rLines = pTable->GetTabLines();


                            // hier muessen die Actions aufgehoben werden
                            UnoActionRemoveContext aRemoveContext(pDoc);
                            SwTableBox* pTLBox = rLines[0]->GetTabBoxes()[0];
                            const SwStartNode* pSttNd = pTLBox->GetSttNd();
                            SwPosition aPos(*pSttNd);
                            // Cursor in die obere linke Zelle des Ranges setzen
                            SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(aPos, sal_True);
                            pUnoCrsr->Move( fnMoveForward, fnGoNode );
                            pUnoCrsr->SetRemainInSection( sal_False );

                            SwTableLine* pLastLine = rLines[rLines.Count() - 1];
                            SwTableBoxes &rBoxes = pLastLine->GetTabBoxes();
                            const SwTableBox* pBRBox = rBoxes[rBoxes.Count() -1];
                            pUnoCrsr->SetMark();
                            pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                            pUnoCrsr->Move( fnMoveForward, fnGoNode );
                            SwUnoTableCrsr* pCrsr = *pUnoCrsr;
                            pCrsr->MakeBoxSels();

                            SfxItemSet aSet(pDoc->GetAttrPool(),
                                            RES_BOX, RES_BOX,
                                            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                            0);

                            SvxBoxItem aBox( RES_BOX );
                            SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
                            SvxBorderLine aLine;

                            sal_Bool bSet = lcl_LineToSvxLine(pBorder->TopLine, aLine);
                            aBox.SetLine(bSet ? &aLine : 0, BOX_LINE_TOP);
                            aBoxInfo.SetValid(VALID_TOP, pBorder->IsTopLineValid);

                            bSet = lcl_LineToSvxLine(pBorder->BottomLine, aLine);
                            aBox.SetLine(bSet ? &aLine : 0, BOX_LINE_BOTTOM);
                            aBoxInfo.SetValid(VALID_BOTTOM, pBorder->IsBottomLineValid);

                            bSet = lcl_LineToSvxLine(pBorder->LeftLine, aLine);
                            aBox.SetLine(bSet ? &aLine : 0, BOX_LINE_LEFT);
                            aBoxInfo.SetValid(VALID_LEFT, pBorder->IsLeftLineValid);

                            bSet = lcl_LineToSvxLine(pBorder->RightLine, aLine);
                            aBox.SetLine(bSet ? &aLine : 0, BOX_LINE_RIGHT);
                            aBoxInfo.SetValid(VALID_RIGHT, pBorder->IsRightLineValid);

                            bSet = lcl_LineToSvxLine(pBorder->HorizontalLine, aLine);
                            aBoxInfo.SetLine(bSet ? &aLine : 0, BOXINFO_LINE_HORI);
                            aBoxInfo.SetValid(VALID_HORI, pBorder->IsHorizontalLineValid);

                            bSet = lcl_LineToSvxLine(pBorder->VerticalLine, aLine);
                            aBoxInfo.SetLine(bSet ? &aLine : 0, BOXINFO_LINE_VERT);
                            aBoxInfo.SetValid(VALID_VERT, pBorder->IsVerticalLineValid);

                            aBox.SetDistance((sal_uInt16)MM100_TO_TWIP(pBorder->Distance));
                            aBoxInfo.SetValid(VALID_DISTANCE, pBorder->IsDistanceValid);

                            aSet.Put(aBox);
                            aSet.Put(aBoxInfo);

                            pDoc->SetTabBorders(*pCrsr, aSet);
                            delete pUnoCrsr;
                        }
                    }
                }
                break;
                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    UnoActionContext aContext(pFmt->GetDoc());
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    lcl_SetTblSeparators(aValue, pTable, pTable->GetTabLines()[0]->GetTabBoxes()[0], sal_False, pFmt->GetDoc());
                }
                break;
                case FN_UNO_TABLE_COLUMN_RELATIVE_SUM:/*_readonly_*/ break;
                default:
                {
                    SwAttrSet aSet(pFmt->GetAttrSet());
                    aPropSet.setPropertyValue(*pMap, aValue, aSet);
                    pFmt->GetDoc()->SetAttr(aSet, *pFmt);
                }
            }
        }
    }
    else if(bIsDescriptor)
    {
        String aPropertyName(rPropertyName);
        if(!pTableProps->SetProperty(
             ByteString( aPropertyName, RTL_TEXTENCODING_ASCII_US).GetBuffer(),
            aValue))
            throw lang::IllegalArgumentException();
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 12:42:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextTable::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    _pMap, rPropertyName);
        if (!pMap)
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        if(0xFF == pMap->nMemberId)
        {
            aRet = lcl_GetSpecialProperty(pFmt, pMap );
        }
        else
        {
            switch(pMap->nWID)
            {
                case  FN_UNO_ANCHOR_TYPES:
                case  FN_UNO_TEXT_WRAP:
                case  FN_UNO_ANCHOR_TYPE:
                    SwXParagraph::getDefaultTextContentValue(aRet, OUString(), pMap->nWID);
                break;
                case FN_UNO_RANGE_ROW_LABEL:
                {
                    sal_Bool bTemp = bFirstRowAsLabel;
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTemp = bFirstColumnAsLabel;
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                case FN_UNO_TABLE_BORDER:
                {
                    SwDoc* pDoc = pFmt->GetDoc();
                    SwClientIter aIter( *pFmt );
                    //Tabellen ohne Layout (unsichtbare Header/Footer )
                    if(0 != aIter.First( TYPE( SwFrm )))
                    {
                        lcl_FormatTable(pFmt);
                        SwTable* pTable = SwTable::FindTable( pFmt );
                        SwTableLines &rLines = pTable->GetTabLines();

                        // hier muessen die Actions aufgehoben werden
                        UnoActionRemoveContext aRemoveContext(pDoc);
                        SwTableBox* pTLBox = rLines[0]->GetTabBoxes()[0];
                        const SwStartNode* pSttNd = pTLBox->GetSttNd();
                        SwPosition aPos(*pSttNd);
                        // Cursor in die obere linke Zelle des Ranges setzen
                        SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(aPos, sal_True);
                        pUnoCrsr->Move( fnMoveForward, fnGoNode );
                        pUnoCrsr->SetRemainInSection( sal_False );

                        SwTableLine* pLastLine = rLines[rLines.Count() - 1];
                        SwTableBoxes &rBoxes = pLastLine->GetTabBoxes();
                        const SwTableBox* pBRBox = rBoxes[rBoxes.Count() -1];
                        pUnoCrsr->SetMark();
                        pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                        pUnoCrsr->Move( fnMoveForward, fnGoNode );
                        SwUnoTableCrsr* pCrsr = *pUnoCrsr;
                        pCrsr->MakeBoxSels();

                        SfxItemSet aSet(pDoc->GetAttrPool(),
                                        RES_BOX, RES_BOX,
                                        SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                        0);
                        aSet.Put(SvxBoxInfoItem( SID_ATTR_BORDER_INNER ));
                        pDoc->GetTabBorders(*pCrsr, aSet);
                        const SvxBoxInfoItem& rBoxInfoItem = (const SvxBoxInfoItem&)aSet.Get(SID_ATTR_BORDER_INNER);
                        const SvxBoxItem& rBox = (const SvxBoxItem&)aSet.Get(RES_BOX);

                         table::TableBorder aTableBorder;
                        aTableBorder.TopLine                = lcl_SvxLineToLine(rBox.GetTop());
                        aTableBorder.IsTopLineValid         = rBoxInfoItem.IsValid(VALID_TOP);
                        aTableBorder.BottomLine             = lcl_SvxLineToLine(rBox.GetBottom());
                        aTableBorder.IsBottomLineValid      = rBoxInfoItem.IsValid(VALID_BOTTOM);
                        aTableBorder.LeftLine               = lcl_SvxLineToLine(rBox.GetLeft());
                        aTableBorder.IsLeftLineValid        = rBoxInfoItem.IsValid(VALID_LEFT);
                        aTableBorder.RightLine              = lcl_SvxLineToLine(rBox.GetRight());
                        aTableBorder.IsRightLineValid       = rBoxInfoItem.IsValid(VALID_RIGHT );
                        aTableBorder.HorizontalLine         = lcl_SvxLineToLine(rBoxInfoItem.GetHori());
                        aTableBorder.IsHorizontalLineValid  = rBoxInfoItem.IsValid(VALID_HORI);
                        aTableBorder.VerticalLine           = lcl_SvxLineToLine(rBoxInfoItem.GetVert());
                        aTableBorder.IsVerticalLineValid    = rBoxInfoItem.IsValid(VALID_VERT);
                        aTableBorder.Distance               = TWIP_TO_MM100_UNSIGNED( rBox.GetDistance() );
                        aTableBorder.IsDistanceValid        = rBoxInfoItem.IsValid(VALID_DISTANCE);
                        aRet.setValue(&aTableBorder, ::getCppuType((const table::TableBorder*)0));
                        delete pUnoCrsr;
                    }
                }
                break;
                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    lcl_GetTblSeparators(aRet, pTable, pTable->GetTabLines()[0]->GetTabBoxes()[0], sal_False);
                }
                break;
                case FN_UNO_TABLE_COLUMN_RELATIVE_SUM:
                    aRet <<= (INT16) UNO_TABLE_COLUMN_SUM;
                break;
                case RES_ANCHOR:
                    //AnchorType ist readonly und maybevoid und wird nicht geliefert
                break;
                case FN_UNO_TEXT_SECTION:
                {
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    SwTableNode* pTblNode = pTable->GetTableNode();
                    SwSectionNode* pSectionNode =  pTblNode->FindSectionNode();
                    if(pSectionNode)
                    {
                        const SwSection& rSect = pSectionNode->GetSection();
                        uno::Reference< text::XTextSection >  xSect =
                                        SwXTextSections::GetObject( *rSect.GetFmt() );
                        aRet <<= xSect;
                    }
                }
                break;
                default:
                {
                    const SwAttrSet& rSet = pFmt->GetAttrSet();
                    aRet = aPropSet.getPropertyValue(*pMap, rSet);
                }
            }
        }
    }
    else if(bIsDescriptor)
    {
        uno::Any* pAny = 0;
        String aPropertyName(rPropertyName);
        if(!pTableProps->GetProperty(ByteString(aPropertyName, RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                                                                                                pAny))
            throw lang::IllegalArgumentException();
        else if(pAny)
            aRet = *pAny;
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 12:42:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:42:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:42:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:42:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:42:58---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextTable::getName(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String sRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!pFmt && !bIsDescriptor)
        throw uno::RuntimeException();
    if(pFmt)
    {
        sRet = pFmt->GetName();
    }
    else
        sRet = m_sTableName;
    return sRet;
}
/*-- 11.12.98 12:42:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::setName(const OUString& rName) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    String sNewTblName(rName);
    if(!pFmt && !bIsDescriptor ||
        !sNewTblName.Len() ||
            STRING_NOTFOUND != sNewTblName.Search('.') ||
                STRING_NOTFOUND != sNewTblName.Search(' ')  )
        throw uno::RuntimeException();

    if(pFmt)
    {
        const String aOldName( pFmt->GetName() );
        sal_Bool bNameFound = sal_False;
        SwFrmFmt* pTmpFmt;
        const SwFrmFmts* pTbl = pFmt->GetDoc()->GetTblFrmFmts();
        for( sal_uInt16 i = pTbl->Count(); i; )
            if( !( pTmpFmt = (*pTbl)[ --i ] )->IsDefault() &&
                pTmpFmt->GetName() == sNewTblName &&
                            pFmt->GetDoc()->IsUsed( *pTmpFmt ))
            {
                bNameFound = sal_True;
                break;
            }

        if(bNameFound)
        {
            throw uno::RuntimeException();
        }
        pFmt->SetName( sNewTblName );


        SwStartNode *pStNd;
        SwNodeIndex aIdx( *pFmt->GetDoc()->GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
        while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
        {
            aIdx++;
            SwNode *pNd = pFmt->GetDoc()->GetNodes()[aIdx];
            if ( pNd->IsOLENode() &&
                aOldName == ((SwOLENode*)pNd)->GetChartTblName() )
            {
                ((SwOLENode*)pNd)->SetChartTblName( sNewTblName );

                ((SwOLENode*)pNd)->GetOLEObj();

                SwTable* pTable = SwTable::FindTable( pFmt );
                //TL_CHART2: chart needs to be notfied about name changes
                pFmt->GetDoc()->UpdateCharts( pTable->GetFrmFmt()->GetName() );
            }
            aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
        }
        pFmt->GetDoc()->SetModified();
    }
    else
        m_sTableName = sNewTblName;
}
/*-----------------11.02.98 09:58-------------------

--------------------------------------------------*/
sal_uInt16 SwXTextTable::getRowCount(void)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRet = 0;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            nRet = pTable->GetTabLines().Count();
        }
    }
    return nRet;
}
/*-----------------11.02.98 09:58-------------------

--------------------------------------------------*/
sal_uInt16 SwXTextTable::getColumnCount(void)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    sal_Int16 nRet = 0;
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableLines& rLines = pTable->GetTabLines();
            SwTableLine* pLine = rLines.GetObject(0);
            nRet = pLine->GetTabBoxes().Count();
        }
    }
    return nRet;
}
/*-- 11.12.98 12:42:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    if(pOld && pOld->Which() == RES_REMOVE_UNO_OBJECT &&
        (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
    else
        ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        aLstnrCntnr.Disposing();
        aChartLstnrCntnr.Disposing();
    }
    else
        aChartLstnrCntnr.ChartDataChanged();
}
/* -----------------25.10.99 15:12-------------------

 --------------------------------------------------*/
OUString SAL_CALL SwXTextTable::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextTable");
}
/* -----------------25.10.99 15:12-------------------

 --------------------------------------------------*/
sal_Bool SwXTextTable::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    String sServiceName(rServiceName);
    return (sServiceName.EqualsAscii("com.sun.star.document.LinkTarget")  ||
            sServiceName.EqualsAscii("com.sun.star.text.TextTable")  ||
            sServiceName.EqualsAscii("com.sun.star.text.TextContent") ||
            sServiceName.EqualsAscii("com.sun.star.text.TextSortable"));
}
/* -----------------25.10.99 15:12-------------------

 --------------------------------------------------*/
uno::Sequence< OUString > SwXTextTable::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(4);
    OUString* pArr = aRet.getArray();
    pArr[0] = C2U("com.sun.star.document.LinkTarget");
    pArr[1] = C2U("com.sun.star.text.TextTable");
    pArr[2] = C2U("com.sun.star.text.TextContent");
    pArr[2] = C2U("com.sun.star.text.TextSortable");
    return aRet;
}

/******************************************************************
 *
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXCellRange::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXCellRange::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}
/* -----------------28.04.98 10:29-------------------
 *
 * --------------------------------------------------*/
TYPEINIT1(SwXCellRange, SwClient);
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXCellRange::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXCellRange");
}
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXCellRange::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return
        rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.text.CellRange" ) ) ||
         rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.style.CharacterProperties" ) ) ||
        rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.style.CharacterPropertiesAsian" ) ) ||
        rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.style.CharacterPropertiesComplex") ) ||
        rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.style.ParagraphProperties" ) ) ||
        rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.style.ParagraphPropertiesAsian" ) ) ||
        rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.style.ParagraphPropertiesComplex" ) );
}
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXCellRange::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(7);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.CellRange");
     pArray[1] = C2U("com.sun.star.style.CharacterProperties");
    pArray[2] = C2U("com.sun.star.style.CharacterPropertiesAsian");
    pArray[3] = C2U("com.sun.star.style.CharacterPropertiesComplex");
    pArray[4] = C2U("com.sun.star.style.ParagraphProperties");
    pArray[5] = C2U("com.sun.star.style.ParagraphPropertiesAsian");
    pArray[6] = C2U("com.sun.star.style.ParagraphPropertiesComplex");
    return aRet;
}

/*-- 11.12.98 14:27:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXCellRange::SwXCellRange(SwUnoCrsr* pCrsr, SwFrmFmt& rFrmFmt,
    SwRangeDescriptor& rDesc)
    :
    SwClient(&rFrmFmt),
    aCursorDepend(this, pCrsr),
    aChartLstnrCntnr((cppu::OWeakObject*)this),
    aRgDesc(rDesc),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_RANGE)),
    pTblCrsr(pCrsr),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_RANGE)),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False)
{
    aRgDesc.Normalize();
}
/*-- 11.12.98 14:27:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXCellRange::~SwXCellRange()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    delete pTblCrsr;
}
/*-- 11.12.98 14:27:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCell >  SwXCellRange::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XCell >  aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        if(nColumn >= 0 && nRow >= 0 &&
             getColumnCount() > nColumn && getRowCount() > nRow )
        {
            SwXCell* pXCell = lcl_CreateXCell(pFmt,
                    aRgDesc.nLeft + nColumn, aRgDesc.nTop + nRow);
            if(pXCell)
                aRet = pXCell;
        }
    }
    if(!aRet.is())
        throw lang::IndexOutOfBoundsException();
    return aRet;
}
/*-- 11.12.98 14:27:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCellRange >  SwXCellRange::getCellRangeByPosition(
        sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XCellRange >  aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt && getColumnCount() > nRight && getRowCount() > nBottom &&
        nLeft <= nRight && nTop <= nBottom
        && nLeft >= 0 && nRight >= 0 && nTop >= 0 && nBottom >= 0 )
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            SwRangeDescriptor aNewDesc;
            aNewDesc.nTop    = nTop + aRgDesc.nTop;
            aNewDesc.nBottom = nBottom + aRgDesc.nTop;
            aNewDesc.nLeft   = nLeft + aRgDesc.nLeft;
            aNewDesc.nRight  = nRight + aRgDesc.nLeft;
            aNewDesc.Normalize();
            String sTLName = lcl_GetCellName(aNewDesc.nLeft, aNewDesc.nTop);
            String sBRName = lcl_GetCellName(aNewDesc.nRight, aNewDesc.nBottom);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                // hier muessen die Actions aufgehoben
                UnoActionRemoveContext aRemoveContext(pFmt->GetDoc());
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // Cursor in die obere linke Zelle des Ranges setzen
                SwUnoCrsr* pUnoCrsr = pFmt->GetDoc()->CreateUnoCrsr(aPos, sal_True);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( sal_False );
                const SwTableBox* pBRBox = pTable->GetTblBox( sBRName );
                if(pBRBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr = *pUnoCrsr;
                    pCrsr->MakeBoxSels();
                    // pUnoCrsr wird uebergeben und nicht geloescht
                    SwXCellRange* pCellRange = new SwXCellRange(pUnoCrsr, *pFmt, aNewDesc);
                    aRet = pCellRange;
                }
                else
                    delete pUnoCrsr;
            }
        }
    }
    if(!aRet.is())
        throw lang::IndexOutOfBoundsException();
    return aRet;

}
/*-- 11.12.98 14:27:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCellRange >  SwXCellRange::getCellRangeByName(const OUString& rRange)
        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String sRange(rRange);
    String sTLName(sRange.GetToken(0, ':'));
    String sBRName(sRange.GetToken(1, ':'));
    if(!sTLName.Len() || !sBRName.Len())
        throw uno::RuntimeException();
    SwRangeDescriptor aDesc;
    aDesc.nTop = aDesc.nLeft = aDesc.nBottom = aDesc.nRight = -1;
    lcl_GetCellPosition( sTLName, aDesc.nLeft, aDesc.nTop );
    lcl_GetCellPosition( sBRName, aDesc.nRight, aDesc.nBottom );
    aDesc.Normalize();
    return getCellRangeByPosition(aDesc.nLeft - aRgDesc.nLeft, aDesc.nTop - aRgDesc.nTop,
                aDesc.nRight - aRgDesc.nLeft, aDesc.nBottom - aRgDesc.nTop);
}
/*-- 11.12.98 14:27:35---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXCellRange::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
/*-- 11.12.98 14:27:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue) throw( beans::UnknownPropertyException,
        beans::PropertyVetoException, lang::IllegalArgumentException,
            lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        /* ASK OLIVER
        lcl_FormatTable(pFmt);*/
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            SwDoc* pDoc = pTblCrsr->GetDoc();
            {
                // remove actions to enable box selection
                UnoActionRemoveContext aRemoveContext(pDoc);
            }
            SwUnoTableCrsr* pCrsr = *pTblCrsr;
            pCrsr->MakeBoxSels();
            switch(pMap->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    pDoc->GetBoxAttr( *pTblCrsr, aBrush );
                    ((SfxPoolItem&)aBrush).PutValue(aValue, pMap->nMemberId);
                    pDoc->SetBoxAttr( *pTblCrsr, aBrush );

                }
                break;
                case RES_BOX :
                {
                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);
                    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
                    aBoxInfo.SetValid(0xff, FALSE);
                    BYTE nValid = 0;
                    switch(pMap->nMemberId & ~CONVERT_TWIPS)
                    {
                        case  LEFT_BORDER :             nValid = VALID_LEFT; break;
                        case  RIGHT_BORDER:             nValid = VALID_RIGHT; break;
                        case  TOP_BORDER  :             nValid = VALID_TOP; break;
                        case  BOTTOM_BORDER:            nValid = VALID_BOTTOM; break;
                        case  LEFT_BORDER_DISTANCE :
                        case  RIGHT_BORDER_DISTANCE:
                        case  TOP_BORDER_DISTANCE  :
                        case  BOTTOM_BORDER_DISTANCE:
                            nValid = VALID_DISTANCE;
                        break;
                    }
                    aBoxInfo.SetValid(nValid, TRUE);


                    aSet.Put(aBoxInfo);
                    pDoc->GetTabBorders(*pCrsr, aSet);

                    aSet.Put(aBoxInfo);
                    SvxBoxItem aBoxItem((const SvxBoxItem&)aSet.Get(RES_BOX));
                    ((SfxPoolItem&)aBoxItem).PutValue(aValue, pMap->nMemberId);
                    aSet.Put(aBoxItem);
                    pDoc->SetTabBorders( *pTblCrsr, aSet );
                }
                break;
                case RES_BOXATR_FORMAT:
                {
                    SfxUInt32Item aNumberFormat(RES_BOXATR_FORMAT);
                    ((SfxPoolItem&)aNumberFormat).PutValue(aValue, 0);
                    pDoc->SetBoxAttr( *pCrsr, aNumberFormat);
                }
                break;
                case FN_UNO_RANGE_ROW_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstRowAsLabel != bTmp)
                    {
                        aChartLstnrCntnr.ChartDataChanged();
                        bFirstRowAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstColumnAsLabel != bTmp)
                    {
                        aChartLstnrCntnr.ChartDataChanged();
                        bFirstColumnAsLabel = bTmp;
                    }
                }
                break;
                default:
                {
                    SfxItemSet aItemSet( pDoc->GetAttrPool(), pMap->nWID, pMap->nWID );
                    SwXTextCursor::GetCrsrAttr( pCrsr->GetSelRing(), aItemSet );

                    if(!lcl_setCrsrPropertyValue( pMap, pCrsr->GetSelRing(), aItemSet, aValue ))
                        aPropSet.setPropertyValue(*pMap, aValue, aItemSet );
                    SwXTextCursor::SetCrsrAttr(pCrsr->GetSelRing(), aItemSet, CRSR_ATTR_MODE_TABLE );
                }
            }
        }
        else
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
}
/*-- 11.12.98 14:27:35---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXCellRange::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        /* ASK OLIVER
        lcl_FormatTable(pFmt);*/
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
        if(pMap)
        {
            switch(pMap->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    if(pTblCrsr->GetDoc()->GetBoxAttr( *pTblCrsr, aBrush ))
                        aBrush.QueryValue(aRet, pMap->nMemberId);

                }
                break;
                case RES_BOX :
                {
                    SwDoc* pDoc = pTblCrsr->GetDoc();
                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);
                    aSet.Put(SvxBoxInfoItem( SID_ATTR_BORDER_INNER ));
                    pDoc->GetTabBorders(*pTblCrsr, aSet);
                    const SvxBoxItem& rBoxItem = ((const SvxBoxItem&)aSet.Get(RES_BOX));
                    rBoxItem.QueryValue(aRet, pMap->nMemberId);
                }
                break;
                case RES_BOXATR_FORMAT:
                    //GetAttr fuer Tabellenselektion am Doc fehlt noch
                    DBG_WARNING("not implemented")
                break;
                case FN_UNO_PARA_STYLE:
                {
                    SwFmtColl* pTmpFmt = SwXTextCursor::GetCurTxtFmtColl(*pTblCrsr, FALSE);
                    OUString sRet;
                    if(pFmt)
                        sRet = pTmpFmt->GetName();
                    aRet <<= sRet;
                }
                break;
                case FN_UNO_RANGE_ROW_LABEL:
                {
                    sal_Bool bTemp = bFirstRowAsLabel;
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTemp = bFirstColumnAsLabel;
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                default:
                {
                    SfxItemSet aSet(pTblCrsr->GetDoc()->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    // erstmal die Attribute des Cursors
                    SwUnoTableCrsr* pCrsr = *pTblCrsr;
                    SwXTextCursor::GetCrsrAttr(pCrsr->GetSelRing(), aSet);
                    aRet = aPropSet.getPropertyValue(*pMap, aSet);
                }
            }
        }
        else
           throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}
/*-- 11.12.98 14:27:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 14:27:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 14:27:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 14:27:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}

/*-----------------------------------------------------------------------

  -----------------------------------------------------------------------*/

void SwXCellRange::GetDataSequence(
        uno::Sequence< uno::Any >   *pAnySeq,   //-> first pointer != 0 is used
        uno::Sequence< OUString >   *pTxtSeq,   //-> as output sequence
        uno::Sequence< double >     *pDblSeq,   //-> (previous data gets overwritten)
        sal_Bool bForceNumberResults )          //-> when 'true' requires to make an
                                                // extra effort to return a value different
                                                // from 0 even if the cell is formatted to text
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // compare to SwXCellRange::getDataArray (note different return types though)

    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    //
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }

    sal_Int32 nSize = nRowCount * nColCount;
    if (pAnySeq)
        pAnySeq->realloc( nSize );
    else if (pTxtSeq)
        pTxtSeq->realloc( nSize );
    else if (pDblSeq)
        pDblSeq->realloc( nSize );
    else
    {
        DBG_ERROR( "argument missing" );
        return;
    }
    uno::Any   *pAnyData = pAnySeq ? pAnySeq->getArray() : 0;
    OUString   *pTxtData = pTxtSeq ? pTxtSeq->getArray() : 0;
    double     *pDblData = pDblSeq ? pDblSeq->getArray() : 0;

    sal_Int32 nDtaCnt = 0;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        uno::Reference< table::XCell > xCellRef;
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell * pXCell = lcl_CreateXCell(pFmt,
                                    aRgDesc.nLeft + nCol,
                                    aRgDesc.nTop + nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    if (pAnyData)
                    {
                        // check if table box value item is set
                        BOOL bIsNum = pBox->GetFrmFmt()->GetItemState( RES_BOXATR_VALUE, FALSE ) == SFX_ITEM_SET;
                        //ULONG nNdPos = pBox->IsValidNumTxtNd( sal_True );
                        if (!bIsNum/* && ULONG_MAX == nNdPos*/)
                            pAnyData[nDtaCnt++] <<= lcl_getString(*pXCell);
                        else
                            pAnyData[nDtaCnt++] <<= lcl_getValue(*pXCell);
                    }
                    else if (pTxtData)
                        pTxtData[nDtaCnt++] = lcl_getString(*pXCell);
                    else if (pDblData)
                    {
                        double fVal = 0.0;
                        if (!bForceNumberResults || table::CellContentType_TEXT != pXCell->getType())
                            fVal = lcl_getValue(*pXCell);
                        else
                        {
                            DBG_ASSERT( table::CellContentType_TEXT == pXCell->getType(),
                                    "this branch of 'if' is only for text formatted cells" )

                            // now we'll try to get a useful numerical value
                            // from the text in the cell...

                            sal_uInt32 nFIndex;
                            SvNumberFormatter* pNumFormatter = pTblCrsr->GetDoc()->GetNumberFormatter();

                            // look for SwTblBoxNumFormat value in parents as well
                            const SfxPoolItem* pItem;
                            SwFrmFmt *pBoxFmt = pXCell->GetTblBox()->GetFrmFmt();
                            SfxItemState eState = pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem);

                            if (eState == SFX_ITEM_SET)
                            {
                                // please note that the language of the numberformat
                                // is implicitly coded into the below value as well
                                nFIndex = ((SwTblBoxNumFormat*)pItem)->GetValue();

                                // since the current value indicates a text format but the call
                                // to 'IsNumberFormat' below won't work for text formats
                                // we need to get rid of the part that indicates the text format.
                                // According to ER this can be done like this:
                                nFIndex -= (nFIndex % SV_COUNTRY_LANGUAGE_OFFSET);
                            }
                            else
                            {
                                // system language is probably not the best possible choice
                                // but since we have to guess anyway (because the language of at
                                // the text is NOT the one used for the number format!)
                                // it is at least conform to to what is used in
                                // SwTableShell::Execute when
                                // SID_ATTR_NUMBERFORMAT_VALUE is set...
                                LanguageType eLang = LANGUAGE_SYSTEM;
                                nFIndex = pNumFormatter->GetStandardIndex( eLang );
                            }

                            OUString aTxt( lcl_getString(*pXCell) );
                            double fTmp;
                            if (pNumFormatter->IsNumberFormat( aTxt, nFIndex, fTmp ))
                                fVal = fTmp;
                        }
                        pDblData[nDtaCnt++] = fVal;
                    }
                    else
                        DBG_ERROR( "output sequence missing" );
                }
            }
        }
    }
    DBG_ASSERT( nDtaCnt == nSize, "size mismatch. Invalid cell range?" );
    if (pAnySeq)
        pAnySeq->realloc( nDtaCnt );
    else if (pTxtSeq)
        pTxtSeq->realloc( nDtaCnt );
    else if (pDblSeq)
        pDblSeq->realloc( nDtaCnt );
}

/*-- 04.06.04 11:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/

SwUnoCrsr * lcl_CreateCursor( SwFrmFmt &rTblFmt,
        SwTableBox *pStartBox,      // should be top-left cell of cell range
        SwTableBox *pEndBox )       // should be bottom right-cell cell range
{
    // create a *new* UNO cursor spanning the cell range defined by
    // the start and end box. Both boxes must be belong to the same table!

    SwUnoCrsr *pUnoCrsr = 0;
    if (pStartBox && pEndBox)
    {
        // hier muessen die Actions aufgehoben werden um
        // (zB dem Layout zu ermöglichen die Tabelle zu formatieren, da
        // sonst kein Tabellen Cursor aufgespannt werden kann.)
        UnoActionRemoveContext aRemoveContext(rTblFmt.GetDoc());

        // set point of cursor to top left box of range
        const SwStartNode* pSttNd = pStartBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        pUnoCrsr = rTblFmt.GetDoc()->CreateUnoCrsr(aPos, sal_True);
        pUnoCrsr->Move( fnMoveForward, fnGoNode );
        pUnoCrsr->SetRemainInSection( sal_False );
        pUnoCrsr->SetMark();
        pUnoCrsr->GetPoint()->nNode = *pEndBox->GetSttNd();
        pUnoCrsr->Move( fnMoveForward, fnGoNode );
        SwUnoTableCrsr *pCrsr = *pUnoCrsr;
        pCrsr->MakeBoxSels();
    }
    return pUnoCrsr;
}


uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > SwXCellRange::GetLabeledDataSequences(
        SwChartDataProvider &rProvider,
        BOOL bBuildColumnSeqs,      // if false build sequences of rows
        BOOL bFirstCellIsLabel )
    throw (uno::RuntimeException)
{
    sal_Int32 nRowCount = getRowCount();
    sal_Int32 nColCount = getColumnCount();
    //
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }

    sal_Int32 nNumLabelCells = bFirstCellIsLabel ? 1 : 0;
    sal_Int32 nNumRowLabelCells = bBuildColumnSeqs ? 0 : nNumLabelCells;
    sal_Int32 nNumColLabelCells = bBuildColumnSeqs ? nNumLabelCells : 0;

    sal_Int32 nSecondaryCount   = bBuildColumnSeqs ? nColCount : nRowCount;
    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSecondarySeq( nSecondaryCount );
    uno::Reference< chart2::data::XLabeledDataSequence > *pSecondaryArray = aSecondarySeq.getArray();
    SwFrmFmt* pTblFmt = GetFrmFmt();
    if (pTblFmt)
    {
        sal_Int32 nColOff   = 0;
        sal_Int32 nRowOff   = 0;
        sal_Int32 &rSecondaryOffset = bBuildColumnSeqs ? nColOff : nRowOff;
        for(rSecondaryOffset = 0;  rSecondaryOffset < nSecondaryCount;  ++rSecondaryOffset)
        {
            SwTableBox * pLabelStartBox     = 0;
            SwTableBox * pLabelEndBox       = 0;
            SwTableBox * pValuesStartBox    = 0;
            SwTableBox * pValuesEndBox      = 0;
            SwTable* pTable = SwTable::FindTable( pTblFmt );
            DBG_ASSERT( pTable, "table not found" )
            if (!pTable)
                throw uno::RuntimeException();
            if (nNumLabelCells != 0)
            {
                String aLabelStartBoxName = lcl_GetCellName(
                        aRgDesc.nLeft + nColOff,
                        aRgDesc.nTop  + nRowOff );
                pLabelStartBox = (SwTableBox*)pTable->GetTblBox( aLabelStartBoxName );
                String aLabelEndBoxName = lcl_GetCellName(
                        aRgDesc.nLeft + nColOff + Max((sal_Int32)0, nNumRowLabelCells - 1),
                        aRgDesc.nTop  + nRowOff + Max((sal_Int32)0, nNumColLabelCells - 1) );
                pLabelEndBox = (SwTableBox*)pTable->GetTblBox( aLabelEndBoxName );
            }
            if (nColCount > nNumColLabelCells  &&  nRowCount > nNumRowLabelCells)
            {
                String aValuesStartBoxName = lcl_GetCellName(
                        aRgDesc.nLeft + nColOff + nNumRowLabelCells,
                        aRgDesc.nTop  + nRowOff + nNumColLabelCells );
                pValuesStartBox = (SwTableBox*)pTable->GetTblBox( aValuesStartBoxName );

                String aValuesEndBoxName;
                if (bBuildColumnSeqs)
                {
                    aValuesEndBoxName = lcl_GetCellName(
                            aRgDesc.nLeft + nColOff,
                            aRgDesc.nTop  + nRowCount - 1 );
                }
                else
                {
                    aValuesEndBoxName = lcl_GetCellName(
                            aRgDesc.nLeft + nColCount - 1,
                            aRgDesc.nTop  + nRowOff );
                }
                pValuesEndBox = (SwTableBox*)pTable->GetTblBox( aValuesEndBoxName );
            }


            uno::Reference< chart2::data::XLabeledDataSequence > xLDS;
            uno::Reference< chart2::data::XDataSequence > xLabelSeq;
            uno::Reference< chart2::data::XDataSequence > xValuesSeq;
            if (pValuesStartBox && pValuesEndBox)
            {
                SwUnoCrsr *pTblCursor = lcl_CreateCursor( *pTblFmt,
                                                pValuesStartBox, pValuesEndBox );
                xValuesSeq = new SwChartDataSequence( rProvider, *pTblFmt, pTblCursor );
            }
            if (pLabelStartBox && pLabelEndBox)
            {
                SwUnoCrsr *pTblCursor = lcl_CreateCursor( *pTblFmt,
                                                pLabelStartBox, pLabelEndBox );
                xLabelSeq =  new SwChartDataSequence( rProvider, *pTblFmt, pTblCursor );
            }
            if (!xValuesSeq.is())
                throw uno::RuntimeException();
            else
            {
                xLDS = new SwChartLabeledDataSequence();
                xLDS->setLabel( xLabelSeq );
                xLDS->setValues( xValuesSeq );
            }

            pSecondaryArray[ rSecondaryOffset ] = xLDS;
        }
    }
    else
        throw uno::RuntimeException();

    return aSecondarySeq;
}

/*-- 29.04.02 11:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Sequence< uno::Any > > SAL_CALL SwXCellRange::getDataArray()
    throw (uno::RuntimeException)
{
    // see SwXCellRange::getData also
    // also see SwXCellRange::GetDataSequence

    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    //
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    uno::Sequence< uno::Sequence< uno::Any > > aRowSeq(nRowCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        uno::Sequence< uno::Any >* pRowArray = aRowSeq.getArray();
        uno::Reference< table::XCell > xCellRef;
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            uno::Sequence< uno::Any > aColSeq(nColCount);
            uno::Any * pColArray = aColSeq.getArray();
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell * pXCell = lcl_CreateXCell(pFmt,
                                    aRgDesc.nLeft + nCol,
                                    aRgDesc.nTop + nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    // check if table box value item is set
                    SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
                    BOOL bIsNum = pBoxFmt->GetItemState( RES_BOXATR_VALUE, FALSE ) == SFX_ITEM_SET;
                    //const SfxPoolItem* pItem;
                    //SwDoc* pDoc = pXCell->GetDoc();
                    //BOOL bIsText = (SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
                    //          ||  pDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue())
                    //          ||  ((SwTblBoxNumFormat*)pItem)->GetValue() == NUMBERFORMAT_TEXT);

                    if(!bIsNum/*bIsText*/)
                        pColArray[nCol] <<= lcl_getString(*pXCell);
                    else
                        pColArray[nCol] <<= lcl_getValue(*pXCell);
                }
            }
            pRowArray[nRow] = aColSeq;
        }
    }
    return aRowSeq;
}
/*-- 29.04.02 11:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXCellRange::setDataArray(
        const uno::Sequence< uno::Sequence< uno::Any > >& rArray )
    throw (uno::RuntimeException)
{
    // see SwXCellRange::setData also

    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        if(rArray.getLength() != nRowCount)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< uno::Any >* pRowArray = rArray.getConstArray();
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< uno::Any >& rColSeq = pRowArray[nRow];
            if(rColSeq.getLength() != nColCount)
            {
                throw uno::RuntimeException();
            }
            const uno::Any * pColArray = rColSeq.getConstArray();
            uno::Reference< table::XCell > xCellRef;
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell * pXCell = lcl_CreateXCell(pFmt,
                                    aRgDesc.nLeft + nCol,
                                    aRgDesc.nTop + nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    const uno::Any &rAny = pColArray[nCol];
                    if (uno::TypeClass_STRING == rAny.getValueTypeClass())
                        lcl_setString( *pXCell, *(rtl::OUString *) rAny.getValue() );
                    else
                    {
                        double d = 0;
                        // #i20067# don't throw exception just do nothing if
                        // there is no value set
                        if( (rAny >>= d) )
                            lcl_setValue( *pXCell, d );
                        else
                            lcl_setString( *pXCell, OUString(), TRUE );
                    }
                }
            }
        }
    }
}
/*-- 11.12.98 14:27:36---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Sequence< double > > SwXCellRange::getData(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    //
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    uno::Sequence< uno::Sequence< double > > aRowSeq(bFirstRowAsLabel ? nRowCount - 1 : nRowCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        uno::Sequence< double >* pRowArray = aRowSeq.getArray();

        sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            uno::Sequence< double > aColSeq(bFirstColumnAsLabel ? nColCount - 1 : nColCount);
            double * pArray = aColSeq.getArray();
            sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                pArray[nCol - nColStart] = xCell->getValue();
            }
            pRowArray[nRow - nRowStart] = aColSeq;
        }
    }
    return aRowSeq;
}
/*-- 11.12.98 14:27:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::setData(const uno::Sequence< uno::Sequence< double > >& rData)
                                                throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        if(rData.getLength() < nRowCount - nRowStart)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< double >* pRowArray = rData.getConstArray();
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< double >& rColSeq = pRowArray[nRow - nRowStart];
            sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
            if(rColSeq.getLength() < nColCount - nColStart)
            {
                throw uno::RuntimeException();
            }
            const double * pColArray = rColSeq.getConstArray();
            for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                xCell->setValue(pColArray[nCol - nColStart]);
            }
        }
    }
}
/*-- 11.12.98 14:27:37---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXCellRange::getRowDescriptions(void)
                                            throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    if(!nRowCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    uno::Sequence< OUString > aRet(bFirstColumnAsLabel ? nRowCount - 1 : nRowCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        OUString* pArray = aRet.getArray();
        if(bFirstColumnAsLabel)
        {
            sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nRowCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(0, i);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?")
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 14:27:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::setRowDescriptions(const uno::Sequence< OUString >& rRowDesc)
                                                    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        sal_Int16 nRowCount = getRowCount();
        if(!nRowCount || rRowDesc.getLength() < bFirstRowAsLabel ? nRowCount - 1 : nRowCount)
        {
            throw uno::RuntimeException();
        }
        const OUString* pArray = rRowDesc.getConstArray();
        if(bFirstColumnAsLabel)
        {
            sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nRowCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(0, i);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            DBG_ERROR("Wohin mit den Labels?")
        }
    }
}
/*-- 11.12.98 14:27:37---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXCellRange::getColumnDescriptions(void)
                                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nColCount = getColumnCount();
    if(!nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    uno::Sequence< OUString > aRet(bFirstRowAsLabel ? nColCount - 1 : nColCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        OUString* pArray = aRet.getArray();
        if(bFirstRowAsLabel)
        {
            sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nColCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(i, 0);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?")
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 14:27:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::setColumnDescriptions(const uno::Sequence< OUString >& ColumnDesc)
                                                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nColCount = getColumnCount();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const OUString* pArray = ColumnDesc.getConstArray();
        if(bFirstRowAsLabel && ColumnDesc.getLength() >= nColCount - bFirstColumnAsLabel ? 1 : 0)
        {
            sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nColCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(i, 0);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);

                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?")
        }
    }
}
/*-- 11.12.98 14:27:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::addChartDataChangeEventListener(const uno::Reference< chart::XChartDataChangeEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aChartLstnrCntnr.AddListener(aListener.get());
}
/*-- 11.12.98 14:27:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::removeChartDataChangeEventListener(const uno::Reference< chart::XChartDataChangeEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aChartLstnrCntnr.RemoveListener(aListener.get()))
        throw uno::RuntimeException();
}
/* -----------------08.03.99 15:36-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXCellRange::isNotANumber(double /*fNumber*/) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return sal_False;

}
/* -----------------08.03.99 15:36-------------------
 *
 * --------------------------------------------------*/
double SwXCellRange::getNotANumber(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return 0.;
}
/*-- 11.12.98 14:27:38---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SwXCellRange::createSortDescriptor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXTextCursor::createSortDescriptor(sal_True);
}
/*-- 11.12.98 14:27:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXCellRange::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSortOptions aSortOpt;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt &&
        SwXTextCursor::convertSortProperties(rDescriptor, aSortOpt))
    {
        SwUnoTableCrsr* pTableCrsr = *pTblCrsr;
        pTableCrsr->MakeBoxSels();
        UnoActionContext aContext( pFmt->GetDoc() );
        pFmt->GetDoc()->SortTbl(pTableCrsr->GetBoxes(), aSortOpt);
    }
}
/* -----------------27.04.98 16:54-------------------
 *
 * --------------------------------------------------*/
sal_uInt16 SwXCellRange::getColumnCount(void)
{
    return static_cast< sal_uInt16 >(aRgDesc.nRight - aRgDesc.nLeft + 1);
}
/* -----------------27.04.98 16:54-------------------
 *
 * --------------------------------------------------*/
sal_uInt16 SwXCellRange::getRowCount(void)
{
    return static_cast< sal_uInt16 >(aRgDesc.nBottom - aRgDesc.nTop + 1);
}
/* -----------------------------05.06.01 09:19--------------------------------

 ---------------------------------------------------------------------------*/
const SwUnoCrsr* SwXCellRange::GetTblCrsr() const
{
    const SwUnoCrsr* pRet = 0;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
        pRet = pTblCrsr;
    return pRet;
}

/*-- 11.12.98 14:27:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew );
    if(!GetRegisteredIn() || !aCursorDepend.GetRegisteredIn())
    {
        /*
         * Not sure if this will cause a memory leak - this pTblCrsr
         * is deleted in SwDoc and GPFs here when deleted again
         * if(!aCursorDepend.GetRegisteredIn())
            delete pTblCrsr;
         */
        pTblCrsr = 0;
        aChartLstnrCntnr.Disposing();
    }
    else
        aChartLstnrCntnr.ChartDataChanged();
}
/******************************************************************
 *  SwXTableRows
 ******************************************************************/
/* -----------------------------19.04.00 15:22--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTableRows::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTableRows");
}
/* -----------------------------19.04.00 15:22--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTableRows::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.text.TableRows") == rServiceName;
}
/* -----------------------------19.04.00 15:22--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTableRows::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TableRows");
    return aRet;
}
TYPEINIT1(SwXTableRows, SwClient);
/*-- 03.02.99 07:37:41---------------------------------------------------
  -----------------------------------------------------------------------*/
SwXTableRows::SwXTableRows(SwFrmFmt& rFrmFmt) :
    SwClient(&rFrmFmt)
{
}
/*-- 03.02.99 07:37:41---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTableRows::~SwXTableRows()
{
}
/*-- 03.02.99 07:37:41---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXTableRows::getCount(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nRet = 0;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        nRet = pTable->GetTabLines().Count();
    }
    return nRet;
}
/*-- 03.02.99 07:37:41---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTableRows::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 )
        throw lang::IndexOutOfBoundsException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(pTable->GetTabLines().Count() > nIndex)
        {
            SwTableLine* pLine = pTable->GetTabLines().GetObject((sal_uInt16)nIndex);
            SwClientIter aIter( *pFrmFmt );
            SwXTextTableRow* pXRow = (SwXTextTableRow*)aIter.
                                    First( TYPE( SwXTextTableRow ));
            while( pXRow )
            {
                // gibt es eine passende Zelle bereits?
                if(pXRow->GetTblRow() == pLine)
                    break;
                pXRow = (SwXTextTableRow*)aIter.Next();
            }
            //sonst anlegen
            if(!pXRow)
                pXRow = new SwXTextTableRow(pFrmFmt, pLine);
            uno::Reference< beans::XPropertySet >  xRet =
                                    (beans::XPropertySet*)pXRow;
            aRet.setValue(&xRet, ::getCppuType((const uno::Reference<beans::XPropertySet>*)0));
        }
        else
            throw lang::IndexOutOfBoundsException();
    }
    return aRet;
}
/*-- 03.02.99 07:37:42---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SAL_CALL SwXTableRows::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<beans::XPropertySet>*)0);
}
/*-- 03.02.99 07:37:42---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTableRows::hasElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    //es gibt keine Tabelle ohne Zeilen
    return sal_True;
}
/*-- 03.02.99 07:37:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTableRows::insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (nCount == 0)
        return;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            sal_uInt16 nRowCount = pTable->GetTabLines().Count();
            if (nCount <= 0 || !(0 <= nIndex && nIndex <= nRowCount))
            {
                uno::RuntimeException aExcept;
                aExcept.Message = C2U("Illegal arguments");
                throw aExcept;
            }

            String sTLName = lcl_GetCellName(0, nIndex);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            sal_Bool bAppend = sal_False;
            if(!pTLBox)
            {
                bAppend = sal_True;
                // am Ende anfuegen, dazu muss der Cursor in die letzte Zeile!
                SwTableLines& rLines = pTable->GetTabLines();
                SwTableLine* pLine = rLines.GetObject(rLines.Count() -1);
                SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                pTLBox = rBoxes.GetObject(0);
            }
            if(pTLBox)
            {
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // Cursor in die obere linke Zelle des Ranges setzen
                UnoActionContext aAction(pFrmFmt->GetDoc());
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, sal_True);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );

                {
                    // remove actions
                    UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
                }

                pFrmFmt->GetDoc()->InsertRow(*pUnoCrsr, (sal_uInt16)nCount, bAppend);
                delete pUnoCrsr;
            }
        }
    }
}
/*-- 03.02.99 07:37:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTableRows::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (nCount == 0)
        return;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        BOOL bSuccess = FALSE;
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            String sTLName = lcl_GetCellName(0, nIndex);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                {
                    // hier muessen die Actions aufgehoben werden
                    UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                }
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // Cursor in die obere linke Zelle des Ranges setzen
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, sal_True);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( sal_False );
                String sBLName = lcl_GetCellName(0, nIndex + nCount - 1);
                const SwTableBox* pBLBox = pTable->GetTblBox( sBLName );
                if(pBLBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBLBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr = *pUnoCrsr;
                    pCrsr->MakeBoxSels();
                    {   // Die Klammer ist wichtig
                        UnoActionContext aAction(pFrmFmt->GetDoc());
                        pFrmFmt->GetDoc()->DeleteRow(*pUnoCrsr);
                        delete pUnoCrsr;
                        bSuccess = TRUE;
                    }
                    {
                        // hier muessen die Actions aufgehoben werden
                        UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                    }
                }
            }
        }
        if(!bSuccess)
        {
            uno::RuntimeException aExcept;
            aExcept.Message = C2U("Illegal arguments");
            throw aExcept;
        }
    }
}
/*-- 03.02.99 07:37:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTableRows::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

/******************************************************************
 * SwXTableColumns
 ******************************************************************/
/* -----------------------------19.04.00 15:23--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTableColumns::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTableColumns");
}
/* -----------------------------19.04.00 15:23--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTableColumns::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.text.TableColumns") == rServiceName;
}
/* -----------------------------19.04.00 15:23--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTableColumns::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TableColumns");
    return aRet;
}
TYPEINIT1(SwXTableColumns, SwClient);
/*-- 03.02.99 07:37:41---------------------------------------------------
  -----------------------------------------------------------------------*/
SwXTableColumns::SwXTableColumns(SwFrmFmt& rFrmFmt) :
    SwClient(&rFrmFmt)
{
}
/*-- 03.02.99 07:37:41---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTableColumns::~SwXTableColumns()
{
}
/*-- 03.02.99 07:37:41---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXTableColumns::getCount(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nRet = 0;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableLines& rLines = pTable->GetTabLines();
            SwTableLine* pLine = rLines.GetObject(0);
            nRet = pLine->GetTabBoxes().Count();
        }
    }
    return nRet;
}
/*-- 03.02.99 07:37:41---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTableColumns::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< uno::XInterface >  xRet;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        USHORT nCount = 0;
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableLines& rLines = pTable->GetTabLines();
            SwTableLine* pLine = rLines.GetObject(0);
            nCount = pLine->GetTabBoxes().Count();
        }
        if(nCount <= nIndex || nIndex < 0)
            throw lang::IndexOutOfBoundsException();
        xRet = uno::Reference<uno::XInterface>();   //!! writer tables do not have columns !!
    }
    return uno::Any(&xRet, ::getCppuType((const uno::Reference<uno::XInterface>*)0));
}
/*-- 03.02.99 07:37:42---------------------------------------------------

  -----------------------------------------------------------------------*/

uno::Type SAL_CALL SwXTableColumns::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference<uno::XInterface>*)0);
}
/*-- 03.02.99 07:37:42---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTableColumns::hasElements(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    return sal_True;
}
/*-- 03.02.99 07:37:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTableColumns::insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (nCount == 0)
        return;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableLines& rLines = pTable->GetTabLines();
            SwTableLine* pLine = rLines.GetObject(0);
            sal_uInt16 nColCount = pLine->GetTabBoxes().Count();
            if (nCount <= 0 || !(0 <= nIndex && nIndex <= nColCount))
            {
                uno::RuntimeException aExcept;
                aExcept.Message = C2U("Illegal arguments");
                throw aExcept;
            }

            String sTLName = lcl_GetCellName(nIndex, 0);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            sal_Bool bAppend = sal_False;
            if(!pTLBox)
            {
                bAppend = sal_True;
                // am Ende anfuegen, dazu muss der Cursor in die letzte Spalte!
                SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                pTLBox = rBoxes.GetObject(rBoxes.Count() - 1);
            }
            if(pTLBox)
            {
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                UnoActionContext aAction(pFrmFmt->GetDoc());
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, sal_True);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );

                {
                    // remove actions
                    UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
                }

                pFrmFmt->GetDoc()->InsertCol(*pUnoCrsr, (sal_uInt16)nCount, bAppend);
                delete pUnoCrsr;
            }
        }
    }
}
/*-- 03.02.99 07:37:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTableColumns::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (nCount == 0)
        return;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt|| nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        BOOL bSuccess = FALSE;
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            String sTLName = lcl_GetCellName(nIndex, 0);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                {
                    // hier muessen die Actions aufgehoben werden
                    UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                }
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // Cursor in die obere linke Zelle des Ranges setzen
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, sal_True);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( sal_False );
                String sTRName = lcl_GetCellName(nIndex + nCount - 1, 0);
                const SwTableBox* pTRBox = pTable->GetTblBox( sTRName );
                if(pTRBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pTRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr = *pUnoCrsr;
                    pCrsr->MakeBoxSels();
                    {   // Die Klammer ist wichtig
                        UnoActionContext aAction(pFrmFmt->GetDoc());
                        pFrmFmt->GetDoc()->DeleteCol(*pUnoCrsr);
                        delete pUnoCrsr;
                        bSuccess = TRUE;
                    }
                    {
                        // hier muessen die Actions aufgehoben werden
                        UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                    }
                }
            }
        }
        if(!bSuccess)
        {
            uno::RuntimeException aExcept;
            aExcept.Message = C2U("Illegal arguments");
            throw aExcept;
        }
    }
}
/*-- 03.02.99 07:37:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTableColumns::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}
/* -----------------------------22.09.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
void SwChartEventListenerContainer::ChartDataChanged()
{
    if(pListenerArr)
    {
        //TODO: find appropriate settings of the Event
        lang::EventObject aObj(pxParent);
        chart::ChartDataChangeEvent aEvent;
        aEvent.Type = chart::ChartDataChangeType_ALL;
        aEvent.StartColumn = 0;
        aEvent.EndColumn = 1;
        aEvent.StartRow = 0;
        aEvent.EndRow = 1;

        for(sal_uInt16 i = 0; i < pListenerArr->Count(); i++)
        {
            try
            {
                XEventListenerPtr pElem = pListenerArr->GetObject(i);
                uno::Reference<lang::XEventListener> xEventListener = *pElem;
                uno::Reference<chart::XChartDataChangeEventListener> xChartEventListener = (chart::XChartDataChangeEventListener*)(*pElem).get();
                xChartEventListener->chartDataChanged( aEvent );
            }
            catch(uno::Exception const &)
            {
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////

