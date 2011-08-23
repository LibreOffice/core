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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#define ITEMID_BOXINFO SID_ATTR_BORDER_INNER

#include <float.h> // for DBL_MIN

#include <swtypes.hxx>
#include <cmdid.h>

#include <errhdl.hxx>

#include <unotbl.hxx>
#include <unostyle.hxx>
#include <section.hxx>

#include <unocrsr.hxx>
#include <unomid.h>
#include <bf_svx/unomid.hxx>
#include <hints.hxx>
#include <swtblfmt.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <shellres.hxx>
#include <docary.hxx>
#include <ndole.hxx>
#include <bf_so3/ipobj.hxx>

#include <frame.hxx>
#include <vcl/svapp.hxx>
#include <fmtfsize.hxx>
#include <tblafmt.hxx>
#include <tabcol.hxx>
#include <cellatr.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <bf_svx/boxitem.hxx>
#define _SVSTDARR_STRINGS
#include <bf_svtools/svstdarr.hxx>
#include <viewsh.hxx>
#include <tabfrm.hxx>
#include <redline.hxx>
#include <unomap.hxx>
#include <unoredline.hxx>
#include <bf_sch/schdll.hxx>
#include <bf_sch/memchrt.hxx>
#include <unoprnms.hxx>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/XChartDataChangeEventListener.hpp>
#include <com/sun/star/chart/ChartDataChangeEvent.hpp>
#include <unotbl.hxx>
#include <unoobj.hxx>
#include <bf_svx/brkitem.hxx>
#include <bf_svx/shaditem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <fmtornt.hxx>
#include <bf_svx/keepitem.hxx>
#include <fmtlsplt.hxx>
#include <osl/mutex.hxx>
#include <SwStyleNameMapper.hxx>
#include <frmatr.hxx>
#include <crsskip.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::chart;

using rtl::OUString;

BOOL lcl_IsNumeric(const String& rStr)
{
          DBG_BF_ASSERT(0, "STRIP");
    for(xub_StrLen i = 0; i < rStr.Len(); i++)
    {
        sal_Unicode c = rStr.GetChar(i);
        if((c < '0') || (c > '9'))
            return FALSE;
    }
    return TRUE;
}

//aus unoobj.cxx
extern void lcl_SetTxtFmtColl(const uno::Any& rAny, SwPaM& rPaM)	throw (IllegalArgumentException);
extern void lcl_setCharStyle(SwDoc* pDoc, const uno::Any aValue, SfxItemSet& rSet) throw (lang::IllegalArgumentException);

#define UNO_TABLE_COLUMN_SUM    10000

table::BorderLine lcl_SvxLineToLine(const SvxBorderLine* pLine)
{
     table::BorderLine aLine;
    if(pLine)
    {
        aLine.Color			 = pLine->GetColor().GetColor() ;
        aLine.InnerLineWidth = TWIP_TO_MM100( pLine->GetInWidth() );
        aLine.OuterLineWidth = TWIP_TO_MM100( pLine->GetOutWidth() );
        aLine.LineDistance	 = TWIP_TO_MM100( pLine->GetDistance() );
    }
    else
        aLine.Color			 = aLine.InnerLineWidth = aLine.OuterLineWidth = aLine.LineDistance	 = 0;
    return aLine;
}

sal_Bool lcl_LineToSvxLine(const table::BorderLine& rLine, SvxBorderLine& rSvxLine)
{
    rSvxLine.SetColor(   Color(rLine.Color));
    rSvxLine.SetInWidth( MM100_TO_TWIP( rLine.InnerLineWidth ) );
    rSvxLine.SetOutWidth(MM100_TO_TWIP( rLine.OuterLineWidth ) );
    rSvxLine.SetDistance(MM100_TO_TWIP( rLine.LineDistance	) );
    sal_Bool bRet = rLine.InnerLineWidth > 0 || rLine.OuterLineWidth > 0;
    return bRet;
}

void lcl_SetSpecialProperty(SwFrmFmt* pFmt, const SfxItemPropertyMap* pMap, const uno::Any& aValue)
    throw (IllegalArgumentException)
{
    //Sonderbehandlung fuer "Nicht-Items"
    switch(pMap->nWID)
    {
        case  FN_TABLE_HEADLINE_REPEAT:

        {
            SwTable* pTable = SwTable::FindTable( pFmt );
            {
                UnoActionContext aAction(pFmt->GetDoc());
                sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                pFmt->GetDoc()->SetHeadlineRepeat( *pTable, bVal);
            }
        }
        break;
        case  FN_TABLE_IS_RELATIVE_WIDTH:
        case  FN_TABLE_WIDTH:
        case  FN_TABLE_RELATIVE_WIDTH:
        {
            sal_Int32 nWidth;
            SwFmtFrmSize aSz( pFmt->GetFrmSize() );
            if(FN_TABLE_WIDTH == pMap->nWID)
            {
                aValue >>= nWidth;
                aSz.SetWidthPercent(0);
                aSz.SetWidth ( MM100_TO_TWIP ( nWidth ) );
            }
            else if(FN_TABLE_RELATIVE_WIDTH == pMap->nWID)
            {
                sal_Int16 nSet;
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
                    IllegalArgumentException aExcept;
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
                SwStyleNameMapper::FillUIName(sPageStyle, sPageStyle, GET_POOLID_PAGEDESC, sal_True );
                pDesc = ::binfilter::GetPageDescByName_Impl(*pFmt->GetDoc(), sPageStyle);
            }
            SwFmtPageDesc aDesc( pDesc );
            pFmt->GetDoc()->SetAttr(aDesc, *pFmt);
        }
        break;
        default:
            throw IllegalArgumentException();
    }
}

uno::Any lcl_GetSpecialProperty(SwFrmFmt* pFmt, const SfxItemPropertyMap* pMap )
{
    uno::Any aRet;
    switch(pMap->nWID)
    {
        case  FN_TABLE_HEADLINE_REPEAT:
        {
            SwTable* pTable = SwTable::FindTable( pFmt );
            BOOL bTemp = pTable->IsHeadlineRepeat();
            aRet.setValue(&bTemp, ::getCppuBooleanType());
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
                   sPDesc = SwStyleNameMapper::GetProgName(pDsc->GetName(), GET_POOLID_PAGEDESC );
                }
            }
            aRet <<= OUString(sPDesc);
        }
        break;
        case RES_ANCHOR :
            aRet <<= TextContentAnchorType_AT_PARAGRAPH;
        break;
        case FN_UNO_ANCHOR_TYPES :
        {
            uno::Sequence<TextContentAnchorType> aTypes(1);
             TextContentAnchorType* pArray = aTypes.getArray();
            pArray[0] = TextContentAnchorType_AT_PARAGRAPH;
            aRet <<= aTypes;
        }
        break;
        case FN_UNO_WRAP :
        {
            aRet <<= WrapTextMode_NONE;
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

String lcl_GetCellName(sal_Int16 nColumn, sal_Int16 nRow)
{
        String sCellName;
        sal_uInt16 nDiv = nColumn;
        sal_uInt16 nMod = 0;
        sal_Bool bFirst = sal_True;
        while( 0 != (nDiv -= nMod) || bFirst )
        {
            nMod = nDiv % 52;
            sal_uInt16 nMod2 = nDiv % 26;
            char cCol = nMod < 26 ? 'A' : 'a';
            cCol += nMod2;
            sCellName.Insert(cCol, 0);
            bFirst = sal_False;
        }
        sCellName += String::CreateFromInt32(++nRow);
        return sCellName;
}

void lcl_GetRowCol(const String& rCellName, sal_uInt16& rRow, sal_uInt16& rCol)
{
    //make parts out of the cell name
    // examples: B5, Aa34,  Cf97 ...
    xub_StrLen nLen = rCellName.Len();
    xub_StrLen nFirstPart = 1;
    while(nFirstPart < nLen && !lcl_IsNumeric(String(rCellName.GetChar(nFirstPart))))
    {
        nFirstPart ++;
    }
    String sRow(rCellName.Copy(nFirstPart,nLen - nFirstPart));
    String sCol(rCellName.Copy(0, nFirstPart));
    rRow = (sal_uInt16)sRow.ToInt32();
    rRow -= 1;

    rCol = 0;
    nLen = sCol.Len();
    if(!nLen)
    {
        rRow = -1;
        rCol = -1;
    }
    else
    {
        sal_uInt16 nBase = 1;
        do
        {
            sal_Unicode cChar = sCol.GetChar(nLen-1);

            if( cChar <= 'Z' )
                rCol += nBase * ((cChar - 'A') + (nBase > 1 ? 1 : 0));
            else
                rCol += nBase * ((cChar - 'a') + (nBase > 1 ? 1 : 0));

            sCol.Erase(nLen -1, 1);
            nLen = sCol.Len();
            nBase *= 50;
        }
        while(nLen);
    }
}

SwXCell* lcl_CreateXCell(SwFrmFmt* pFmt, sal_Int16 nColumn, sal_Int16 nRow)
{
    SwXCell* pXCell = 0;
    String sCellName = lcl_GetCellName(nColumn, nRow);
    SwTable* pTable = SwTable::FindTable( pFmt );
    SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
    if(pBox)
    {
        pXCell = SwXCell::CreateXCell(pFmt, pBox, &sCellName, pTable );
    }
    return pXCell;
}

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

void lcl_GetTblSeparators(uno::Any& rRet, SwTable* pTable, SwTableBox* pBox, sal_Bool bRow)
{
    SwTabCols aCols;
    aCols.SetLeftMin ( 0 );
    aCols.SetLeft    ( 0 );
    aCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    aCols.SetRightMax( UNO_TABLE_COLUMN_SUM );

    pTable->GetTabCols( aCols, pBox, sal_False, bRow );

    sal_uInt16 nSepCount = aCols.Count();
    uno::Sequence< TableColumnSeparator> aColSeq(nSepCount);
     TableColumnSeparator* pArray = aColSeq.getArray();
    sal_Bool bError = sal_False;
    for(sal_uInt16 i = 0; i < nSepCount; i++)
    {
        pArray[i].Position = aCols[i];
        pArray[i].IsVisible = !aCols.IsHidden(i);
        if(!bRow && !pArray[i].IsVisible)
        {
            bError = sal_True;
            break;
        }
    }
    if(!bError)
        rRet.setValue(&aColSeq, ::getCppuType((uno::Sequence< TableColumnSeparator>*)0));

}

void lcl_SetTblSeparators(const uno::Any& rVal, SwTable* pTable, SwTableBox* pBox, sal_Bool bRow, SwDoc* pDoc)
{
    SwTabCols aOldCols;

    aOldCols.SetLeftMin ( 0 );
    aOldCols.SetLeft    ( 0 );
    aOldCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    aOldCols.SetRightMax( UNO_TABLE_COLUMN_SUM );

    pTable->GetTabCols( aOldCols, pBox, sal_False, bRow );
    sal_uInt16 nOldCount = aOldCols.Count();

    const uno::Sequence< TableColumnSeparator>* pSepSeq =
                (uno::Sequence< TableColumnSeparator>*) rVal.getValue();
    if(pSepSeq && pSepSeq->getLength() == nOldCount)
    {
        SwTabCols aCols(aOldCols);
        sal_Bool bError = sal_False;
        const TableColumnSeparator* pArray = pSepSeq->getConstArray();
        sal_Int32 nLastValue = 0;
        sal_Int32 nTblWidth = aCols.GetRight() - aCols.GetLeft();
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

inline ::rtl::OUString lcl_getString( SwXCell &rCell )
{
    // getString is a member function of the base class...
    return rCell.getString();
}
/* -----------------30.04.02 08:00-------------------
 * non UNO function call to set string in SwXCell
 * --------------------------------------------------*/
void lcl_setString( SwXCell &rCell, const ::rtl::OUString &rTxt )
{
    if(rCell.IsValid())
    {
        SwFrmFmt* pBoxFmt = rCell.pBox->ClaimFrmFmt();
        pBoxFmt->LockModify();
        pBoxFmt->ResetAttr( RES_BOXATR_FORMULA );
        pBoxFmt->ResetAttr( RES_BOXATR_VALUE );
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
        // Der Text mu� zunaechst (vielleicht) geloescht werden
        sal_uInt32 nNdPos = rCell.pBox->IsValidNumTxtNd( sal_True );
        if(USHRT_MAX == nNdPos)
            lcl_setString( rCell, OUString() );
        SwDoc* pDoc = rCell.GetDoc();
        UnoActionContext aAction(pDoc);
        SwFrmFmt* pBoxFmt = rCell.pBox->ClaimFrmFmt();
        SfxItemSet aSet(pDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE);
        const SfxPoolItem* pItem;
        if(SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
            ||  pDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue()))
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

const SfxItemPropertyMap* GetTableDescPropertyMap()
{
    static SfxItemPropertyMap aTableDescPropertyMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_BACK_COLOR )		 ,	RES_BACKGROUND, 		&::getCppuType((const sal_Int32*)0),			PROPERTY_NONE,MID_BACK_COLOR		 },
        { SW_PROP_NAME(UNO_NAME_BREAK_TYPE), 			RES_BREAK, 				&::getCppuType((const style::BreakType*)0), 		PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_GRAPHIC_URL      ), 		RES_BACKGROUND, 		&::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { SW_PROP_NAME(UNO_NAME_GRAPHIC_FILTER  ), 		RES_BACKGROUND, 		&::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { SW_PROP_NAME(UNO_NAME_GRAPHIC_LOCATION)	 , 		RES_BACKGROUND, 		&::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { SW_PROP_NAME(UNO_NAME_LEFT_MARGIN), 			RES_LR_SPACE, 			&::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_L_MARGIN},
        { SW_PROP_NAME(UNO_NAME_RIGHT_MARGIN), 			RES_LR_SPACE, 			&::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN},
        { SW_PROP_NAME(UNO_NAME_HORI_ORIENT	 ), 		RES_HORI_ORIENT,		&::getCppuType((const sal_Int16*)0),			PROPERTY_NONE ,MID_HORIORIENT_ORIENT	},
        { SW_PROP_NAME(UNO_NAME_KEEP_TOGETHER),  		RES_KEEP,				&::getBooleanCppuType()  ,  PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_SPLIT    ),  			RES_LAYOUT_SPLIT,		&::getBooleanCppuType()  ,  PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_PAGE_NUMBER_OFFSET), 	RES_PAGEDESC,  			&::getCppuType((const sal_Int16*)0),  		PROPERTY_NONE, MID_PAGEDESC_PAGENUMOFFSET},
        { SW_PROP_NAME(UNO_NAME_PAGE_STYLE_NAME), 			0,                      &::getCppuType((const OUString*)0),         PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_RELATIVE_WIDTH), 		FN_TABLE_RELATIVE_WIDTH,&::getCppuType((const sal_Int16*)0)  ,  		PROPERTY_NONE, 0	},
        { SW_PROP_NAME(UNO_NAME_REPEAT_HEADLINE) ,  	FN_TABLE_HEADLINE_REPEAT,&::getBooleanCppuType(), 		PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_SHADOW_FORMAT), 		RES_SHADOW, 			&::getCppuType((const table::ShadowFormat*)0), 	PROPERTY_NONE, CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_IS_WIDTH_RELATIVE),     FN_TABLE_IS_RELATIVE_WIDTH,  &::getBooleanCppuType()  ,     PROPERTY_NONE, 0 },
        { SW_PROP_NAME(UNO_NAME_TABLE_NAME),          0,                      &::getCppuType((const OUString*)0), 		PROPERTY_NONE, 0 },
        { SW_PROP_NAME(UNO_NAME_TOP_MARGIN), 			RES_UL_SPACE, 			&::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN), 		RES_UL_SPACE, 			&::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT),	RES_BACKGROUND, 	&::getBooleanCppuType(),			PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT		 },
        { SW_PROP_NAME(UNO_NAME_WIDTH), 				FN_TABLE_WIDTH,   		&::getCppuType((const sal_Int32*)0)  ,  		PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_CHART_ROW_AS_LABEL),    	FN_UNO_RANGE_ROW_LABEL,			&::getBooleanCppuType(),			PROPERTY_NONE, 	0},
        { SW_PROP_NAME(UNO_NAME_CHART_COLUMN_AS_LABEL), 	FN_UNO_RANGE_COL_LABEL,			&::getBooleanCppuType()  ,  		PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_TABLE_BORDER),			FN_UNO_TABLE_BORDER,    		&::getCppuType((const table::TableBorder*)0),	PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
        {0,0}
    };
    #define TABLE_PROP_COUNT 24
    return aTableDescPropertyMap_Impl;
}

/******************************************************************
 * SwXCell
 ******************************************************************/
TYPEINIT1(SwXCell, SwClient);

SwXCell::SwXCell(SwFrmFmt* pTblFmt, SwTableBox* pBx, sal_uInt16 nPos ) :
    SwXText(pTblFmt->GetDoc(), CURSOR_TBLTEXT),
    pBox(pBx),
    pStartNode(0),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_CELL)),
    SwClient(pTblFmt),
    nFndPos(nPos)
{
}

SwXCell::SwXCell(SwFrmFmt* pTblFmt, const SwStartNode& rStartNode) :
    SwXText(pTblFmt->GetDoc(), CURSOR_TBLTEXT),
    pBox(0),
    pStartNode(&rStartNode),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_CELL)),
    SwClient(pTblFmt),
    nFndPos(USHRT_MAX)
{
}

SwXCell::~SwXCell()
{

}

const uno::Sequence< sal_Int8 > & SwXCell::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXCell::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    else
        return SwXText::getSomething(rId);
}

Sequence< uno::Type > SAL_CALL SwXCell::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static Sequence< uno::Type > aRetTypes;
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

Sequence< sal_Int8 > SAL_CALL SwXCell::getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

void SAL_CALL SwXCell::acquire(  ) throw()
{
    SwXCellBaseClass::acquire();
}

void SAL_CALL SwXCell::release(  ) throw()
{
    SwXCellBaseClass::release();
}

uno::Any SAL_CALL SwXCell::queryInterface( const uno::Type& aType )
    throw (RuntimeException)
{
    uno::Any aRet = SwXCellBaseClass::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXText::queryInterface(aType);
    return aRet;
}

const SwStartNode *SwXCell::GetStartNode() const
{
    const SwStartNode *pSttNd = 0;

    if( pStartNode || ((SwXCell *)this)->IsValid() )
        pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();

    return pSttNd;
}

uno::Reference< XTextCursor >   SwXCell::createCursor() throw ( ::com::sun::star::uno::RuntimeException)
{
    return createTextCursor();
}

sal_Bool	SwXCell::IsValid()
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

OUString SwXCell::getFormula(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

void SwXCell::setFormula(const OUString& rFormula) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        // Der Text mu� zunaechst (vielleicht) geloescht werden
        sal_uInt32 nNdPos = pBox->IsValidNumTxtNd( sal_True );
        if(USHRT_MAX == nNdPos)
            setString(OUString());
        String sFml(rFormula);
        if( sFml.EraseLeadingChars().Len() && '=' == sFml.GetChar( 0 ) )
                    sFml.Erase( 0, 1 );
        SwTblBoxFormula aFml( sFml );
        SwDoc* pDoc = GetDoc();
        UnoActionContext aAction(pDoc);
        SfxItemSet aSet(pDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_FORMULA);
        const SfxPoolItem* pItem;
        SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
        if(SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
            ||  pDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue()))
        {
            aSet.Put(SwTblBoxNumFormat(0));
        }
        aSet.Put(aFml);
        GetDoc()->SetTblBoxFormulaAttrs( *pBox, aSet );
        //Tabelle aktualisieren
        SwTableFmlUpdate aTblUpdate( SwTable::FindTable( GetFrmFmt() ));
        pDoc->UpdateTblFlds( &aTblUpdate );
    }
}

double SwXCell::getValue(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return lcl_getValue( *this );
}

void SwXCell::setValue(double rValue) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    lcl_setValue( *this, rValue );
}

table::CellContentType SwXCell::getType(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return  (table::CellContentType)0;
}

void SwXCell::setString(const OUString& aString) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    lcl_setString( *this, aString );
}

sal_Int32 SwXCell::getError(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OUString sContent = getString();
    return sContent.equals(ViewShell::GetShellRes()->aCalc_Error);
}

uno::Reference< XTextCursor >  SwXCell::createTextCursor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextCursor >  	aRef;
    if(pStartNode || IsValid())
    {
        const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        SwXTextCursor* pCrsr = new SwXTextCursor(this, aPos, CURSOR_TBLTEXT, GetDoc());
        SwUnoCrsr* pUnoCrsr = pCrsr->GetCrsr();
        pUnoCrsr->Move(fnMoveForward, fnGoNode);
        aRef =  (XWordCursor*)pCrsr;
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

uno::Reference< XTextCursor >  SwXCell::createTextCursorByRange(const uno::Reference< XTextRange > & xTextPosition)
                                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextCursor >  aRef;
    SwUnoInternalPaM aPam(*GetDoc());
    if((pStartNode || IsValid()) && SwXTextRange::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
        //skip sections
        SwStartNode* p1 = aPam.GetNode()->FindStartNode();
        while(p1->IsSectionNode())
            p1 = p1->FindStartNode();

        if( p1 == pSttNd )
            aRef =  (XWordCursor*)new SwXTextCursor(this , *aPam.GetPoint(), CURSOR_TBLTEXT, GetDoc(), aPam.GetMark());
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

uno::Reference< beans::XPropertySetInfo >  SwXCell::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}

void SwXCell::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue) throw( beans::UnknownPropertyException, beans::PropertyVetoException, IllegalArgumentException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        SwFrmFmt* pBoxFmt = pBox->ClaimFrmFmt();
        SwAttrSet aSet(pBoxFmt->GetAttrSet());
        aPropSet.setPropertyValue(rPropertyName, aValue, aSet);
        pBoxFmt->GetDoc()->SetAttr(aSet, *pBoxFmt);
    }
}

uno::Any SwXCell::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_TEXT_SECTION)))
        {
            SwFrmFmt* pTblFmt = GetFrmFmt();
            SwDoc* pDoc = pTblFmt->GetDoc();
            SwTable* pTable = SwTable::FindTable( pTblFmt );
            SwTableNode* pTblNode = pTable->GetTableNode();
            SwSectionNode* pSectionNode =  pTblNode->FindSectionNode();
            if(pSectionNode)
            {
                const SwSection& rSect = pSectionNode->GetSection();
                Reference< XTextSection >  xSect =
                                SwXTextSections::GetObject( *rSect.GetFmt() );
                aRet <<= xSect;
            }
        }
        else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_CELL_NAME)))
        {
            aRet <<= OUString ( pBox->GetName() );
        }
        else if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_START_REDLINE))||
                rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_END_REDLINE)))
        {
            //redline can only be returned if it's a living object
            aRet = SwXText::getPropertyValue(rPropertyName);
        }
        else
        {
            const SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
            const SwAttrSet& rSet = pBoxFmt->GetAttrSet();
            aRet = aPropSet.getPropertyValue(rPropertyName, rSet);
        }
    }
    return aRet;
}

void SwXCell::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXCell::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXCell::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXCell::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

uno::Reference< container::XEnumeration >  SwXCell::createEnumeration(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XEnumeration >  aRef;
    if(IsValid())
    {
        const SwStartNode* pSttNd = pBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        SwUnoCrsr* pUnoCrsr = GetDoc()->CreateUnoCrsr(aPos, sal_False);
        pUnoCrsr->Move( fnMoveForward, fnGoNode );
        aRef = new SwXParagraphEnumeration(this, pUnoCrsr, CURSOR_TBLTEXT);
    }
    return aRef;
}

uno::Type SAL_CALL SwXCell::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<XTextRange>*)0);

}

sal_Bool SwXCell::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}

void SwXCell::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

SwXCell* SwXCell::CreateXCell(SwFrmFmt* pTblFmt, SwTableBox* pBox, const String* pCellName, SwTable *pTable )
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
 * 	exitstiert die Box in der angegebenen Tabelle?
 * --------------------------------------------------*/
SwTableBox* SwXCell::FindBox(SwTable* pTable, SwTableBox* pBox)
{
    // check if nFndPos happens to point to the right table box
    if( nFndPos < pTable->GetTabSortBoxes().Count() &&
        pBox == pTable->GetTabSortBoxes()[ nFndPos ] )
        return pBox;

    // if not, seek the entry (and return, if successful)
    if( pTable->GetTabSortBoxes().Seek_Entry( pBox, &nFndPos ))
        return pBox;

    // box not found: reset nFndPos pointer
    nFndPos = USHRT_MAX;
    return 0;
}

OUString SwXCell::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXCell");
}

BOOL SwXCell::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.CellProperties");
}

Sequence< OUString > SwXCell::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.CellProperties");
    return aRet;
}

/******************************************************************
 * SwXTextTableRow
 ******************************************************************/
OUString SwXTextTableRow::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextTableRow");
}

BOOL SwXTextTableRow::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextTableRow") == rServiceName;
}

Sequence< OUString > SwXTextTableRow::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextTableRow");
    return aRet;
}
TYPEINIT1(SwXTextTableRow, SwClient);

SwXTextTableRow::SwXTextTableRow(SwFrmFmt* pFmt, SwTableLine* pLn) :
    SwClient(pFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE_ROW)),
    pLine(pLn)
{

}

SwXTextTableRow::~SwXTextTableRow()
{

}

uno::Reference< beans::XPropertySetInfo >  SwXTextTableRow::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}

void SwXTextTableRow::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, IllegalArgumentException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableLine* pLn = SwXTextTableRow::FindLine(pTable, pLine);
        if(pLn)
        {
            const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
            SwDoc* pDoc = pFmt->GetDoc();
            if (!pMap)
                throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            switch(pMap->nWID)
            {
                case FN_UNO_ROW_HEIGHT:
                case FN_UNO_ROW_AUTO_HEIGHT:
                {
                    SwFmtFrmSize aFrmSize(pLn->GetFrmFmt()->GetFrmSize());
                    if(FN_UNO_ROW_AUTO_HEIGHT== pMap->nWID)
                    {
                        sal_Bool bSet = *(sal_Bool*)aValue.getValue();
                        aFrmSize.SetSizeType(bSet ? ATT_VAR_SIZE : ATT_FIX_SIZE);
                    }
                    else
                    {
                        sal_Int32 nHeight;
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
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    lcl_SetTblSeparators(aValue, pTable, pLine->GetTabBoxes()[0], sal_True, pDoc);
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

uno::Any SwXTextTableRow::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableLine* pLn = SwXTextTableRow::FindLine(pTable, pLine);
        if(pLn)
        {
            const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
            if (!pMap)
                throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            switch(pMap->nWID)
            {
                case FN_UNO_ROW_HEIGHT:
                case FN_UNO_ROW_AUTO_HEIGHT:
                {
                    const SwFmtFrmSize& rSize = pLn->GetFrmFmt()->GetFrmSize();
                    if(FN_UNO_ROW_AUTO_HEIGHT== pMap->nWID)
                    {
                        BOOL bTmp =  ATT_VAR_SIZE == rSize.GetSizeType();
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

void SwXTextTableRow::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTableRow::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTableRow::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTableRow::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTableRow::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

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
OUString SwXTextTableCursor::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextTableCursor");
}

BOOL SwXTextTableCursor::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextTableCursor") == rServiceName;
}

IMPLEMENT_FORWARD_XINTERFACE2(SwXTextTableCursor,SwXTextTableCursor_Base,OTextCursorHelper)
const SwPaM*		SwXTextTableCursor::GetPaM() const	{ return GetCrsr(); }
SwPaM*				SwXTextTableCursor::GetPaM()		{ return GetCrsr(); }
const SwDoc* 		SwXTextTableCursor::GetDoc() const	{ return GetFrmFmt()->GetDoc(); }
SwDoc* 				SwXTextTableCursor::GetDoc()		{ return GetFrmFmt()->GetDoc(); }
const SwUnoCrsr*	SwXTextTableCursor::GetCrsr() const	{ return (SwUnoCrsr*)aCrsrDepend.GetRegisteredIn(); }
SwUnoCrsr*			SwXTextTableCursor::GetCrsr()		{ return (SwUnoCrsr*)aCrsrDepend.GetRegisteredIn(); }

Sequence< OUString > SwXTextTableCursor::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextTableCursor");
    return aRet;
}

SwXTextTableCursor::SwXTextTableCursor(SwFrmFmt* pFmt, SwTableBox* pBox) :
    SwClient(pFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE_CURSOR)),
    aCrsrDepend(this, 0)
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

SwXTextTableCursor::~SwXTextTableCursor()
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        delete pUnoCrsr;
}

OUString SwXTextTableCursor::getRangeName(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OUString aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        pTblCrsr->MakeBoxSels();
        const SwStartNode* pStart = pTblCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
        const SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
        const SwTableBox* pBox = pTable->GetTblBox( pStart->GetIndex());
        String sRet = pBox->GetName();

        if(pTblCrsr->HasMark())
        {
            pStart = pTblCrsr->GetMark()->nNode.GetNode().FindTableBoxStartNode();
            const SwTableBox* pEBox = pTable->GetTblBox( pStart->GetIndex());
            if(pEBox != pBox)
            {
                sRet += ':';
                sRet += pEBox->GetName();
            }
        }
        aRet = sRet;
    }
    return aRet;
}

sal_Bool SwXTextTableCursor::gotoCellByName(const OUString& CellName, sal_Bool Expand)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect(	pTblCrsr, Expand );
        String sCellName(CellName);
        bRet = pTblCrsr->GotoTblBox(sCellName);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goLeft(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect(	pTblCrsr, Expand );
        bRet = pTblCrsr->Left( Count,CRSR_SKIP_CHARS);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goRight(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect(	pTblCrsr, Expand );
        bRet = pTblCrsr->Right( Count, CRSR_SKIP_CHARS);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goUp(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect(	pTblCrsr, Expand );
        bRet = pTblCrsr->UpDown(sal_True, Count);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goDown(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect(	pTblCrsr, Expand );
        bRet = pTblCrsr->UpDown(sal_False, Count);
    }
    return bRet;
}

void SwXTextTableCursor::gotoStart(sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect(	pTblCrsr, Expand );
        pTblCrsr->MoveTable(fnTableCurr, fnTableStart);
    }
}

void SwXTextTableCursor::gotoEnd(sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        lcl_CrsrSelect(	pTblCrsr, Expand );
        pTblCrsr->MoveTable(fnTableCurr, fnTableEnd);
    }
}

sal_Bool SwXTextTableCursor::mergeRange(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

sal_Bool SwXTextTableCursor::splitRange(sal_Int16 Count, sal_Bool Horizontal) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if (Count <= 0)
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM( "Illegal first argument: needs to be > 0" ) ), static_cast < cppu::OWeakObject * > ( this ) );
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

uno::Reference< beans::XPropertySetInfo >  SwXTextTableCursor::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}

void SwXTextTableCursor::setPropertyValue(const OUString& rPropertyName,
                                                        const uno::Any& aValue)
            throw( beans::UnknownPropertyException,
                        beans::PropertyVetoException,
                     IllegalArgumentException,
                     WrappedTargetException,
                     uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode()->FindStartNode();
        const SwTableNode* pTblNode = pSttNode->FindTableNode();
        lcl_FormatTable((SwFrmFmt*)pTblNode->GetTable().GetFrmFmt());
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                    aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            pTblCrsr->MakeBoxSels();
            SwDoc* pDoc = pUnoCrsr->GetDoc();
            switch(pMap->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush;
                    pDoc->GetBoxBackground( *pUnoCrsr, aBrush );
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
                    SfxItemSet rSet(pDoc->GetAttrPool(),
                        RES_CHRATR_BEGIN, 		RES_FRMATR_END -1,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    SwXTextCursor::GetCrsrAttr(pTblCrsr->GetSelRing(), rSet);
                    aPropSet.setPropertyValue(*pMap, aValue, rSet);
                    SwXTextCursor::SetCrsrAttr(pTblCrsr->GetSelRing(), rSet, CRSR_ATTR_MODE_TABLE);
                }
            }
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
}

uno::Any SwXTextTableCursor::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode()->FindStartNode();
        const SwTableNode* pTblNode = pSttNode->FindTableNode();
        lcl_FormatTable((SwFrmFmt*)pTblNode->GetTable().GetFrmFmt());
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                    aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            pTblCrsr->MakeBoxSels();
            switch(pMap->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush;
                    if(pTblCrsr->GetDoc()->GetBoxBackground( *pUnoCrsr, aBrush ))
                        aBrush.QueryValue(aRet, pMap->nMemberId);

                }
                break;
                case RES_BOXATR_FORMAT:
                    //GetAttr fuer Tabellenselektion am Doc fehlt noch
                    DBG_WARNING("not implemented");
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
                        RES_CHRATR_BEGIN, 		RES_FRMATR_END -1,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    // erstmal die Attribute des Cursors
                    SwXTextCursor::GetCrsrAttr(pTblCrsr->GetSelRing(), aSet);
                    aRet = aPropSet.getPropertyValue(*pMap, aSet);
                }
            }
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}

void SwXTextTableCursor::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTableCursor::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTableCursor::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTableCursor::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

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
    const SfxItemPropertyMap* 	_pMap;
    uno::Any* 					pAnyArr[TABLE_PROP_COUNT];
    sal_uInt16 						nArrLen;

public:
    SwTableProperties_Impl(const SfxItemPropertyMap* pMap);
    ~SwTableProperties_Impl();

    sal_Bool 	SetProperty(const char* pName , uno::Any aVal);
    sal_Bool 	GetProperty(const char* pName, uno::Any*& rpAny);

    sal_Bool 	SetProperty(USHORT nId , uno::Any aVal)
    {	return SetProperty(SW_PROP_NAME_STR( nId ), aVal); }
    sal_Bool 	GetProperty(USHORT nId, uno::Any*& rpAny)
    {	return GetProperty(SW_PROP_NAME_STR( nId ), rpAny); }

    const SfxItemPropertyMap* 	GetMap() const {return _pMap;}
    void  						ApplyTblAttr(const SwTable& rTbl, SwDoc& rDoc);
};

SwTableProperties_Impl::SwTableProperties_Impl(const SfxItemPropertyMap* pMap) :
    _pMap(pMap),
    nArrLen(TABLE_PROP_COUNT)
{
    const SfxItemPropertyMap* pTemp = _pMap;
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        pAnyArr[i] = 0;

}

SwTableProperties_Impl::~SwTableProperties_Impl()
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        delete pAnyArr[i];
}

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

void	SwTableProperties_Impl::ApplyTblAttr(const SwTable& rTbl, SwDoc& rDoc)
{
    SfxItemSet aSet(rDoc.GetAttrPool(),
        RES_LAYOUT_SPLIT, 	RES_LAYOUT_SPLIT,
        RES_BACKGROUND,		RES_BACKGROUND,
        RES_FRM_SIZE,		RES_UL_SPACE,
        RES_HORI_ORIENT,	RES_HORI_ORIENT,
        RES_BREAK, 			RES_BREAK,
        RES_KEEP,			RES_KEEP,
        RES_SHADOW, 		RES_SHADOW,
        RES_PAGEDESC, 		RES_PAGEDESC,
        0
        );
    uno::Any* pRepHead;
    const SwFrmFmt &rFrmFmt = *rTbl.GetFrmFmt();
    if(GetProperty(UNO_NAME_REPEAT_HEADLINE, pRepHead ))
    {
        sal_Bool bVal = *(sal_Bool*)pRepHead->getValue();
        ((SwTable&)rTbl).SetHeadlineRepeat(bVal);
    }

    uno::Any* pBackColor 	= 0;
    GetProperty(UNO_NAME_BACK_COLOR, pBackColor );
    uno::Any* pBackTrans 	= 0;
    GetProperty(UNO_NAME_BACK_TRANSPARENT, pBackTrans );
    uno::Any* pGrLoc 		= 0;
    GetProperty(UNO_NAME_GRAPHIC_LOCATION, pGrLoc	);
    uno::Any* pGrURL 		= 0;
    GetProperty(UNO_NAME_GRAPHIC_URL, pGrURL     );
    uno::Any* pGrFilter 	= 0;
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
        const SwPageDesc* pDesc = 0;
        if(sPageStyle.Len())
        {
            SwStyleNameMapper::FillUIName(sPageStyle, sPageStyle, GET_POOLID_PAGEDESC, sal_True );
            const SwPageDesc* pDesc = ::binfilter::GetPageDescByName_Impl(rDoc, sPageStyle);
            if(pDesc)
            {
                SwFmtPageDesc aDesc( pDesc );
                uno::Any* pPgNo;
                if(GetProperty(UNO_NAME_PAGE_NUMBER_OFFSET, pPgNo ))
                {
                    INT16 nTmp;
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
        bFullAlign = (aOrient.GetHoriOrient() == HORI_FULL);
        aSet.Put(aOrient);
    }


    uno::Any* pSzRel 		= 0;
    GetProperty(UNO_NAME_IS_WIDTH_RELATIVE, pSzRel  );
    uno::Any* pRelWidth 	= 0;
    GetProperty(UNO_NAME_RELATIVE_WIDTH, pRelWidth);
    uno::Any* pWidth 		= 0;
    GetProperty(UNO_NAME_WIDTH,	pWidth 	);

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
    uno::Any* pL 		= 0;
    GetProperty(UNO_NAME_LEFT_MARGIN, pL);
    uno::Any* pR 		= 0;
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
    uno::Any* pU 		= 0;
    GetProperty(UNO_NAME_TOP_MARGIN, pU);
    uno::Any* pLo 	= 0;
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
        sal_Bool bTemp = *(sal_Bool*)pSplit->getValue();
        SwFmtLayoutSplit aSp(bTemp);
        aSet.Put(aSp);
    }

    //TODO: folgende Propertiers noch impl.
//	FN_UNO_RANGE_ROW_LABEL
//	FN_UNO_RANGE_COL_LABEL
//	FN_UNO_TABLE_BORDER

    if(aSet.Count())
    {
        rDoc.SetAttr( aSet, *rTbl.GetFrmFmt() );
    }
}

SwXTextTable* SwXTextTable::GetImplementation(Reference< XInterface> xRef )
{
    uno::Reference<lang::XUnoTunnel> xTunnel( xRef, uno::UNO_QUERY);
    if(xTunnel.is())
        return (SwXTextTable*)xTunnel->getSomething(SwXTextTable::getUnoTunnelId());
    return 0;
}

const uno::Sequence< sal_Int8 > & SwXTextTable::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXTextTable::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

TYPEINIT1(SwXTextTable, SwClient)

SwXTextTable::SwXTextTable() :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    aLstnrCntnr( (XTextTable*)this),
    aChartLstnrCntnr( (XTextTable*)this),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    bIsDescriptor(sal_True),
    nRows(2),
    nColumns(2),
    pTableProps(new SwTableProperties_Impl(GetTableDescPropertyMap()))
{

}

SwXTextTable::SwXTextTable(SwFrmFmt& rFrmFmt) :
    SwClient( &rFrmFmt ),
    aLstnrCntnr( (XTextTable*)this),
    aChartLstnrCntnr( (XTextTable*)this),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    bIsDescriptor(sal_False),
    nRows(0),
    nColumns(0),
    pTableProps(0)
{

}

SwXTextTable::~SwXTextTable()
{
    delete pTableProps;
}

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

uno::Reference< table::XTableRows >  SwXTextTable::getRows(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

uno::Reference< table::XTableColumns >  SwXTextTable::getColumns(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

uno::Reference< table::XCell >  SwXTextTable::getCellByName(const OUString& CellName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCell >  xRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        String sCellName(CellName);
        SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
        if(pBox)
        {
            xRet = SwXCell::CreateXCell(pFmt, pBox, &sCellName);
        }
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

uno::Sequence< OUString > SwXTextTable::getCellNames(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

uno::Reference< XTextTableCursor >  SwXTextTable::createCursorByCellName(const OUString& CellName)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextTableCursor >  xRet;
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

void SwXTextTable::attachToRange(const uno::Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, uno::RuntimeException )
{
    // attachToRange must only be called once
    if(!bIsDescriptor)  /* already attached ? */
        throw uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "SwXTextTable: already attached to range." ) ), static_cast < cppu::OWeakObject * > ( this ) );

    uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                OTextCursorHelper::getUnoTunnelId());
    }
    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc && nRows && nColumns)
    {
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        //keine Tabellen in Tabellen!
        if(!aPam.GetNode()->FindTableNode())
        {
            UnoActionContext aCont( pDoc );

            const SwTable *pTable = 0;
            if( 0 != aPam.Start()->nContent.GetIndex() )
            {
                pDoc->SplitNode(*aPam.Start() );
            }
            //TODO: wenn es der letzte Absatz ist, dann muss noch ein Absatz angehaengt werden!
            if( aPam.HasMark() )
            {
                pDoc->DeleteAndJoin(aPam);
                aPam.DeleteMark();
            }
            pTable = pDoc->InsertTable(
                                        *aPam.GetPoint(),
                                        nRows,
                                        nColumns,
                                        HORI_FULL,
                                        HEADLINE|DEFAULT_BORDER|SPLIT_LAYOUT);
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
        }
        else
        {
            IllegalArgumentException aExcept;
            aExcept.Message = C2U("tables cannot be inserted into tables");
            throw aExcept;
        }
    }
    else
        throw IllegalArgumentException();
}

void SwXTextTable::attach(const uno::Reference< XTextRange > & xTextRange)
        throw( IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

uno::Reference< XTextRange >  SwXTextTable::getAnchor(void)
        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!pFmt)
        throw uno::RuntimeException();
    uno::Reference< XTextRange >  xRet = new SwXTextRange(*pFmt);
    return xRet;
}

void SwXTextTable::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

void SwXTextTable::addEventListener(const uno::Reference< XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}

void SwXTextTable::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}

uno::Reference< table::XCell >  SwXTextTable::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCell >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    // Sheet interessiert nicht
    if(nColumn >= 0 && nRow >= 0 && nColumn < USHRT_MAX && nRow < USHRT_MAX && pFmt)
    {
        SwXCell* pXCell = lcl_CreateXCell(pFmt,
                        (sal_uInt16)nColumn, (sal_uInt16)nRow);
        if(pXCell)
            aRef = pXCell;
    }
    if(!aRef.is())
        throw lang::IndexOutOfBoundsException();
    return aRef;

}

uno::Reference< table::XCellRange >  SwXTextTable::GetRangeByName(SwFrmFmt* pFmt, SwTable* pTable,
                    const String& rTLName, const String& rBRName,
                    SwRangeDescriptor& rDesc)
{
    SolarMutexGuard aGuard;
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

uno::Reference< table::XCellRange >  SwXTextTable::getCellRangeByPosition(sal_Int32 nLeft, sal_Int32 nTop,
                sal_Int32 nRight, sal_Int32 nBottom)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
{
    SolarMutexGuard aGuard;
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
            aDesc.nTop    = (sal_uInt16)nTop;
            aDesc.nBottom = (sal_uInt16)nBottom;
            aDesc.nLeft   = (sal_uInt16)nLeft;
            aDesc.nRight  = (sal_uInt16)nRight;
            String sTLName = lcl_GetCellName(aDesc.nLeft, aDesc.nTop);
            String sBRName = lcl_GetCellName(aDesc.nRight, aDesc.nBottom);
            aRef = GetRangeByName(pFmt, pTable, sTLName, sBRName, aDesc);
        }
    }
    if(!aRef.is())
        throw lang::IndexOutOfBoundsException();
    return aRef;
}

uno::Reference< table::XCellRange >  SwXTextTable::getCellRangeByName(const OUString& aRange)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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
            aDesc.nTop = aDesc.nLeft =  aDesc.nBottom = aDesc.nRight = -1;
            lcl_GetRowCol(sTLName, aDesc.nTop, aDesc.nLeft);
            lcl_GetRowCol(sBRName, aDesc.nBottom, aDesc.nRight);
            aRef = GetRangeByName(pFmt, pTable, sTLName, sBRName, aDesc);
        }
    }
    if(!aRef.is())
        throw uno::RuntimeException();
    return aRef;
}

uno::Sequence< uno::Sequence< uno::Any > > SAL_CALL SwXTextTable::getDataArray()
    throw (uno::RuntimeException)
{
    // see SwXTextTable::getData(...) also

    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        RuntimeException aRuntime;
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
            Reference< XCell > xCellRef;
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
                    sal_uInt32 nNdPos = pBox->IsValidNumTxtNd( sal_True );
                    if(USHRT_MAX == nNdPos)
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

void SAL_CALL SwXTextTable::setDataArray(
        const uno::Sequence< uno::Sequence< uno::Any > >& rArray )
    throw (uno::RuntimeException)
{
    // see SwXTextTable::setData(...) also

    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = nRows;
    sal_Int16 nColCount = nColumns;

    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(pTable->IsTblComplex())
        {
            RuntimeException aRuntime;
            aRuntime.Message = C2U("Table too complex");
            throw aRuntime;
        }

        if(rArray.getLength() != nRowCount)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< Any >* pRowArray = rArray.getConstArray();
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< Any >& rColSeq = pRowArray[nRow];
            if(rColSeq.getLength() != nColCount)
            {
                throw uno::RuntimeException();
            }
            const Any * pColArray = rColSeq.getConstArray();
            Reference< XCell > xCellRef;
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
                    if (TypeClass_STRING == rAny.getValueTypeClass())
                        lcl_setString( *pXCell, *(::rtl::OUString *) rAny.getValue() );
                    else
                    {
                        double d;
                        if( !(rAny >>= d) )
                            throw uno::RuntimeException();
                        lcl_setValue( *pXCell, d );
                    }
                }
            }
        }
    }
}

uno::Sequence< uno::Sequence< double > > SwXTextTable::getData(void)
                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
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

void SwXTextTable::setData(const uno::Sequence< uno::Sequence< double > >& rData)
                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    sal_Bool bChanged = sal_False;

    if(!nRowCount || !nColCount)
    {
        RuntimeException aRuntime;
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

uno::Sequence< OUString > SwXTextTable::getRowDescriptions(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    if(!nRowCount)
    {
        RuntimeException aRuntime;
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
                Reference< XText >  xText(xCell, UNO_QUERY);
                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextTable::setRowDescriptions(const uno::Sequence< OUString >& rRowDesc) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        sal_Int16 nRowCount = getRowCount();
        if(!nRowCount || rRowDesc.getLength() < (bFirstRowAsLabel ? nRowCount - 1 : nRowCount))
        {
            throw uno::RuntimeException();
            return;
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
                    break;
                }
                uno::Reference< XText >  xText(xCell, UNO_QUERY);
                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            DBG_ERROR("Wohin mit den Labels?");
        }
    }
    else
        throw uno::RuntimeException();
}

uno::Sequence< OUString > SwXTextTable::getColumnDescriptions(void)
                                                throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nColCount = getColumnCount();
    if(!nColCount)
    {
        RuntimeException aRuntime;
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
                uno::Reference< XText >  xText(xCell, UNO_QUERY);

                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextTable::setColumnDescriptions(const uno::Sequence< OUString >& rColumnDesc) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nColCount = getColumnCount();
    if(!nColCount)
    {
        RuntimeException aRuntime;
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
                    break;
                }
                uno::Reference< XText >  xText(xCell, UNO_QUERY);
                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
}

void SwXTextTable::addChartDataChangeEventListener(
    const uno::Reference< chart::XChartDataChangeEventListener > & aListener)
        throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aChartLstnrCntnr.AddListener(aListener.get());
}

void SwXTextTable::removeChartDataChangeEventListener(
    const uno::Reference< chart::XChartDataChangeEventListener > & aListener)
        throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aChartLstnrCntnr.RemoveListener(aListener.get()))
        throw uno::RuntimeException();
}

sal_Bool SwXTextTable::isNotANumber(double nNumber) throw( uno::RuntimeException )
{
    // We use DBL_MIN because starcalc does (which uses it because chart
    // wants it that way!)
    return ( nNumber == DBL_MIN );
}

double SwXTextTable::getNotANumber(void) throw( uno::RuntimeException )
{
    // We use DBL_MIN because starcalc does (which uses it because chart
    // wants it that way!)
    return DBL_MIN;
}

uno::Sequence< beans::PropertyValue > SwXTextTable::createSortDescriptor(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return SwXTextCursor::createSortDescriptor(sal_False);
}

void SwXTextTable::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

void SwXTextTable::autoFormat(const OUString& aName) throw( IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {

        }
    }
    else
        throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySetInfo >  SwXTextTable::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}

void SwXTextTable::setPropertyValue(const OUString& rPropertyName,
                                                    const uno::Any& aValue)
        throw( beans::UnknownPropertyException, beans::PropertyVetoException,
                IllegalArgumentException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!aValue.hasValue())
        throw IllegalArgumentException();
    if(pFmt)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    _pMap, rPropertyName);
        if (!pMap)
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        if ( pMap->nFlags & PropertyAttribute::READONLY)
            throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

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

                            SvxBoxItem aBox;
                            SvxBoxInfoItem aBoxInfo;
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
                    UnoActionContext(pFmt->GetDoc());
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
            throw IllegalArgumentException();
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXTextTable::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    _pMap, rPropertyName);
        if (!pMap)
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

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
                        aSet.Put(SvxBoxInfoItem());
                        pDoc->GetTabBorders(*pCrsr, aSet);
                        const SvxBoxInfoItem& rBoxInfoItem = (const SvxBoxInfoItem&)aSet.Get(SID_ATTR_BORDER_INNER);
                        const SvxBoxItem& rBox = (const SvxBoxItem&)aSet.Get(RES_BOX);

                         table::TableBorder aTableBorder;
                        aTableBorder.TopLine 				= lcl_SvxLineToLine(rBox.GetTop());
                        aTableBorder.IsTopLineValid 		= rBoxInfoItem.IsValid(VALID_TOP);
                        aTableBorder.BottomLine				= lcl_SvxLineToLine(rBox.GetBottom());
                        aTableBorder.IsBottomLineValid		= rBoxInfoItem.IsValid(VALID_BOTTOM);
                        aTableBorder.LeftLine				= lcl_SvxLineToLine(rBox.GetLeft());
                        aTableBorder.IsLeftLineValid		= rBoxInfoItem.IsValid(VALID_LEFT);
                        aTableBorder.RightLine				= lcl_SvxLineToLine(rBox.GetRight());
                        aTableBorder.IsRightLineValid		= rBoxInfoItem.IsValid(VALID_RIGHT );
                        aTableBorder.HorizontalLine			= lcl_SvxLineToLine(rBoxInfoItem.GetHori());
                        aTableBorder.IsHorizontalLineValid 	= rBoxInfoItem.IsValid(VALID_HORI);
                        aTableBorder.VerticalLine			= lcl_SvxLineToLine(rBoxInfoItem.GetVert());
                        aTableBorder.IsVerticalLineValid	= rBoxInfoItem.IsValid(VALID_VERT);
                        aTableBorder.Distance 				= TWIP_TO_MM100( rBox.GetDistance() );
                        aTableBorder.IsDistanceValid 		= rBoxInfoItem.IsValid(VALID_DISTANCE);
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
                    SwDoc* pDoc = pFmt->GetDoc();
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    SwTableNode* pTblNode = pTable->GetTableNode();
                    SwSectionNode* pSectionNode =  pTblNode->FindSectionNode();
                    if(pSectionNode)
                    {
                        const SwSection& rSect = pSectionNode->GetSection();
                        Reference< XTextSection >  xSect =
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
            throw IllegalArgumentException();
        else if(pAny)
            aRet = *pAny;
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextTable::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTable::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTable::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXTextTable::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

OUString SwXTextTable::getName(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

void SwXTextTable::setName(const OUString& rName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

                SwOLEObj& rOObj = ((SwOLENode*)pNd)->GetOLEObj();
                SchMemChart *pData = SchDLL::GetChartData( rOObj.GetOleRef() );
                if ( pData )
                {
                    if ( aOldName == pData->GetMainTitle() )
                    {
                        pData->SetMainTitle( sNewTblName );
//Window??
                        SchDLL::Update( rOObj.GetOleRef(), pData, 0/*GetWin()*/ );
                    }
                    SwFrm *pFrm;
                    SwClientIter aIter( *((SwOLENode*)pNd) );
                    for( pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) ); pFrm;
                            pFrm = (SwFrm*)aIter.Next() )
                    {
//InvalidateWindows?
//                        if ( pFrm->Frm().HasArea() )
//                            ((ViewShell*)this)->InvalidateWindows( pFrm->Frm() );
                    }
                }
            }
            aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
        }
        pFmt->GetDoc()->SetModified();
    }
    else
        m_sTableName = sNewTblName;
}

sal_uInt16 SwXTextTable::getRowCount(void)
{
    SolarMutexGuard aGuard;
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

sal_uInt16 SwXTextTable::getColumnCount(void)
{
    SolarMutexGuard aGuard;
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

OUString SAL_CALL SwXTextTable::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextTable");
}

sal_Bool SwXTextTable::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    String sServiceName(rServiceName);
    return (sServiceName.EqualsAscii("com.sun.star.document.LinkTarget")  ||
            sServiceName.EqualsAscii("com.sun.star.text.TextTable")  ||
            sServiceName.EqualsAscii("com.sun.star.text.TextContent") ||
            sServiceName.EqualsAscii("com.sun.star.text.TextSortable"));
}

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

const uno::Sequence< sal_Int8 > & SwXCellRange::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXCellRange::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

TYPEINIT1(SwXCellRange, SwClient);

OUString SwXCellRange::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXCellRange");
}

BOOL SwXCellRange::supportsService(const OUString& rServiceName) throw( RuntimeException )
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

Sequence< OUString > SwXCellRange::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(7);
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

SwXCellRange::SwXCellRange(SwUnoCrsr* pCrsr, SwFrmFmt& rFrmFmt,
                                            SwRangeDescriptor& rDesc) :
    SwClient(&rFrmFmt),
    aChartLstnrCntnr((cppu::OWeakObject*)this),
    aCursorDepend(this, pCrsr),
    aRgDesc(rDesc),
    pTblCrsr(pCrsr),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_RANGE)),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_RANGE))
{

}

SwXCellRange::~SwXCellRange()
{
    delete pTblCrsr;
}

uno::Reference< table::XCell >  SwXCellRange::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCell >  aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        if(nColumn >= 0 && nRow >= 0 &&
             getColumnCount() > nColumn && getRowCount() > nRow )
        {
            SwXCell* pXCell = lcl_CreateXCell(pFmt,
                    sal_Int16(aRgDesc.nLeft + nColumn), sal_Int16(aRgDesc.nTop + nRow));
            if(pXCell)
                aRet = pXCell;
        }
    }
    if(!aRet.is())
        throw lang::IndexOutOfBoundsException();
    return aRet;
}

uno::Reference< table::XCellRange >  SwXCellRange::getCellRangeByPosition(
        sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
{
    SolarMutexGuard aGuard;
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
            aNewDesc.nLeft   = nLeft + 	aRgDesc.nLeft;
            aNewDesc.nRight  = nRight + aRgDesc.nLeft;
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

Reference< XCellRange >  SwXCellRange::getCellRangeByName(const OUString& rRange)
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    String sRange(rRange);
    String sTLName(sRange.GetToken(0, ':'));
    String sBRName(sRange.GetToken(1, ':'));
    if(!sTLName.Len() || !sBRName.Len())
        throw uno::RuntimeException();
    SwRangeDescriptor aDesc;
    aDesc.nTop = aDesc.nLeft =  aDesc.nBottom = aDesc.nRight = -1;
    lcl_GetRowCol(sTLName, aDesc.nTop, aDesc.nLeft);
    lcl_GetRowCol(sBRName, aDesc.nBottom, aDesc.nRight);
    return getCellRangeByPosition(aDesc.nLeft - aRgDesc.nLeft, aDesc.nTop - aRgDesc.nTop,
                aDesc.nRight - aRgDesc.nLeft, aDesc.nBottom - aRgDesc.nTop);
}

uno::Reference< beans::XPropertySetInfo >  SwXCellRange::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = aPropSet.getPropertySetInfo();
    return xRef;
}

void SwXCellRange::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue) throw( beans::UnknownPropertyException,
        beans::PropertyVetoException, IllegalArgumentException,
            WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

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
                    SvxBrushItem aBrush;
                    pDoc->GetBoxBackground( *pTblCrsr, aBrush );
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
                    SvxBoxInfoItem aBoxInfo;
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
                    SwUnoTableCrsr* pCrsr = *pTblCrsr;
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
                    SfxItemSet rSet(pDoc->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        0L);
                    SwCursor& rSelCrsr = pCrsr->GetSelRing();
                    SwXTextCursor::GetCrsrAttr(rSelCrsr, rSet);
                    SwXTextCursor::SetPropertyValue(
                        rSelCrsr, aPropSet, rPropertyName,
                        aValue, pMap, CRSR_ATTR_MODE_TABLE);
                }
            }
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
}

uno::Any SwXCellRange::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
        if(pMap)
        {
            switch(pMap->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush;
                    if(pTblCrsr->GetDoc()->GetBoxBackground( *pTblCrsr, aBrush ))
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
                    aSet.Put(SvxBoxInfoItem());
                    pDoc->GetTabBorders(*pTblCrsr, aSet);
                    const SvxBoxItem& rBoxItem = ((const SvxBoxItem&)aSet.Get(RES_BOX));
                    rBoxItem.QueryValue(aRet, pMap->nMemberId);
                }
                break;
                case RES_BOXATR_FORMAT:
                    //GetAttr fuer Tabellenselektion am Doc fehlt noch
                    DBG_WARNING("not implemented");
                break;
                case FN_UNO_PARA_STYLE:
                {
                    SwFmtColl* pFmt = SwXTextCursor::GetCurTxtFmtColl(*pTblCrsr, FALSE);
                    OUString sRet;
                    if(pFmt)
                        sRet = pFmt->GetName();
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
                        RES_CHRATR_BEGIN, 		RES_FRMATR_END -1,
                        0L);
                    // erstmal die Attribute des Cursors
                    SwUnoTableCrsr* pCrsr = *pTblCrsr;
                    SwXTextCursor::GetCrsrAttr(pCrsr->GetSelRing(), aSet);
                    aRet = aPropSet.getPropertyValue(*pMap, aSet);
                }
            }
        }
        else
           throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}

void SwXCellRange::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXCellRange::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXCellRange::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXCellRange::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

uno::Sequence< uno::Sequence< uno::Any > > SAL_CALL SwXCellRange::getDataArray()
    throw (uno::RuntimeException)
{
    // see SwXCellRange::getData also
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    uno::Sequence< uno::Sequence< uno::Any > > aRowSeq(nRowCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        uno::Sequence< uno::Any >* pRowArray = aRowSeq.getArray();
        Reference< XCell > xCellRef;
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            uno::Sequence< uno::Any > aColSeq(nColCount);
            uno::Any * pColArray = aColSeq.getArray();
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell * pXCell = lcl_CreateXCell(pFmt,
                                    sal_Int16(aRgDesc.nLeft + nCol),
                                    sal_Int16(aRgDesc.nTop + nRow));
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
                    sal_uInt32 nNdPos = pBox->IsValidNumTxtNd( sal_True );
                    if(USHRT_MAX == nNdPos)
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

void SAL_CALL SwXCellRange::setDataArray(
        const uno::Sequence< uno::Sequence< uno::Any > >& rArray )
    throw (uno::RuntimeException)
{
    // see SwXCellRange::setData also
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        RuntimeException aRuntime;
        aRuntime.Message = C2U("Table too complex");
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        if(rArray.getLength() != nRowCount)
        {
            throw uno::RuntimeException();
            return;
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
            Reference< XCell > xCellRef;
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell * pXCell = lcl_CreateXCell(pFmt,
                                    sal_Int16(aRgDesc.nLeft + nCol),
                                    sal_Int16(aRgDesc.nTop + nRow));
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
                    if (TypeClass_STRING == rAny.getValueTypeClass())
                        lcl_setString( *pXCell, *(::rtl::OUString *) rAny.getValue() );
                    else
                    {
                        double d;
                        if( !(rAny >>= d) )
                            throw uno::RuntimeException();
                        lcl_setValue( *pXCell, d );
                    }
                }
            }
        }
    }
}

uno::Sequence< uno::Sequence< double > > SwXCellRange::getData(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        RuntimeException aRuntime;
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
                    throw RuntimeException();
                }
                pArray[nCol - nColStart] = xCell->getValue();
            }
            pRowArray[nRow - nRowStart] = aColSeq;
        }
    }
    return aRowSeq;
}

void SwXCellRange::setData(const uno::Sequence< uno::Sequence< double > >& rData)
                                                throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        RuntimeException aRuntime;
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
            return;
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

uno::Sequence< OUString > SwXCellRange::getRowDescriptions(void)
                                            throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    if(!nRowCount)
    {
        RuntimeException aRuntime;
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
                    break;
                }
                uno::Reference< XText >  xText(xCell, UNO_QUERY);
                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXCellRange::setRowDescriptions(const uno::Sequence< OUString >& rRowDesc)
                                                    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        sal_Int16 nRowCount = getRowCount();
        if(!nRowCount || rRowDesc.getLength() < bFirstRowAsLabel ? nRowCount - 1 : nRowCount)
        {
            throw uno::RuntimeException();
            return;
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
                    break;
                }
                uno::Reference< XText >  xText(xCell, UNO_QUERY);
                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            DBG_ERROR("Wohin mit den Labels?");
        }
    }
}

uno::Sequence< OUString > SwXCellRange::getColumnDescriptions(void)
                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nColCount = getColumnCount();
    if(!nColCount)
    {
        RuntimeException aRuntime;
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
                    break;
                }
                uno::Reference< XText >  xText(xCell, UNO_QUERY);
                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXCellRange::setColumnDescriptions(const uno::Sequence< OUString >& ColumnDesc)
                                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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
                    break;
                }
                uno::Reference< XText >  xText(xCell, UNO_QUERY);

                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            DBG_ERROR("Wo kommen die Labels her?");
        }
    }
}

void SwXCellRange::addChartDataChangeEventListener(const uno::Reference< chart::XChartDataChangeEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aChartLstnrCntnr.AddListener(aListener.get());
}

void SwXCellRange::removeChartDataChangeEventListener(const uno::Reference< chart::XChartDataChangeEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aChartLstnrCntnr.RemoveListener(aListener.get()))
        throw uno::RuntimeException();
}

sal_Bool SwXCellRange::isNotANumber(double nNumber) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return sal_False;

}

double SwXCellRange::getNotANumber(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return 0.;
}

uno::Sequence< beans::PropertyValue > SwXCellRange::createSortDescriptor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return SwXTextCursor::createSortDescriptor(sal_False);
}

void SAL_CALL SwXCellRange::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

sal_uInt16 SwXCellRange::getColumnCount(void)
{
    return aRgDesc.nRight - aRgDesc.nLeft + 1;
}

sal_uInt16 SwXCellRange::getRowCount(void)
{
    return aRgDesc.nBottom - aRgDesc.nTop + 1;
}

void SwXCellRange::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew );
    if(!GetRegisteredIn() || !aCursorDepend.GetRegisteredIn())
    {
        pTblCrsr = 0;
        aChartLstnrCntnr.Disposing();
    }
    else
        aChartLstnrCntnr.ChartDataChanged();
}

/******************************************************************
 *	SwXTableRows
 ******************************************************************/
OUString SwXTableRows::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTableRows");
}

BOOL SwXTableRows::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TableRows") == rServiceName;
}

Sequence< OUString > SwXTableRows::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TableRows");
    return aRet;
}
TYPEINIT1(SwXTableRows, SwClient);

SwXTableRows::SwXTableRows(SwFrmFmt& rFrmFmt) :
    SwClient(&rFrmFmt)
{
}

SwXTableRows::~SwXTableRows()
{
}

sal_Int32 SwXTableRows::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

uno::Any SwXTableRows::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 )
        throw IndexOutOfBoundsException();
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
            throw IndexOutOfBoundsException();
    }
    return aRet;
}

uno::Type SAL_CALL SwXTableRows::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SwXTableRows::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    //es gibt keine Tabelle ohne Zeilen
    return sal_True;
}

void SwXTableRows::insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 || nCount <= 0)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            String sTLName = lcl_GetCellName(0, (sal_Int16)nIndex);
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

void SwXTableRows::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        BOOL bSuccess = FALSE;
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            String sTLName = lcl_GetCellName(0, (sal_Int16)nIndex);
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
                String sBLName = lcl_GetCellName(0, (sal_Int16)nIndex + nCount - 1);
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

void SwXTableRows::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

/******************************************************************
 * SwXTableColumns
 ******************************************************************/
OUString SwXTableColumns::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTableColumns");
}

BOOL SwXTableColumns::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TableColumns") == rServiceName;
}

Sequence< OUString > SwXTableColumns::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TableColumns");
    return aRet;
}
TYPEINIT1(SwXTableColumns, SwClient);

SwXTableColumns::SwXTableColumns(SwFrmFmt& rFrmFmt) :
    SwClient(&rFrmFmt)
{
}

SwXTableColumns::~SwXTableColumns()
{
}

sal_Int32 SwXTableColumns::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

uno::Any SwXTableColumns::getByIndex(sal_Int32 nIndex)
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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
            throw IndexOutOfBoundsException();
        xRet = *new cppu::OWeakObject();
    }
    return uno::Any(&xRet, ::getCppuType((const uno::Reference<uno::XInterface>*)0));
}

uno::Type SAL_CALL SwXTableColumns::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference<uno::XInterface>*)0);
}

sal_Bool SwXTableColumns::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    return sal_True;
}

void SwXTableColumns::insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt|| nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            String sTLName = lcl_GetCellName((sal_Int16)nIndex, 0);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            sal_Bool bAppend = sal_False;
            if(!pTLBox)
            {
                bAppend = sal_True;
                // am Ende anfuegen, dazu muss der Cursor in die letzte Spalte!
                SwTableLines& rLines = pTable->GetTabLines();
                SwTableLine* pLine = rLines.GetObject(0);
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

void SwXTableColumns::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt|| nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        BOOL bSuccess = FALSE;
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            String sTLName = lcl_GetCellName((sal_Int16)nIndex, 0);
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
                String sTRName = lcl_GetCellName((sal_Int16)nIndex + nCount - 1, 0);
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

void SwXTableColumns::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

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
            XEventListenerPtr pElem = pListenerArr->GetObject(i);
            try
            {
                XEventListenerPtr pElem = pListenerArr->GetObject(i);
                Reference<XEventListener> xEventListener = *pElem;
                Reference<XChartDataChangeEventListener> XChartEventListener = (XChartDataChangeEventListener*)(*pElem).get();
                XChartEventListener->chartDataChanged( aEvent );
            }
            catch(Exception&)
            {
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
