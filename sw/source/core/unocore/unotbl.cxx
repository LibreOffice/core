/*************************************************************************
 *
 *  $RCSfile: unotbl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:29 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO SID_ATTR_BORDER_INNER
#include <swtypes.hxx>
#include <cmdid.h>
#ifndef _UNOTBL_HXX
#include <unotbl.hxx>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif

#ifndef _UNOCRSR_HXX //autogen
#include <unocrsr.hxx>
#endif
#ifndef SW_UNOMID_HXX
#include <unomid.h>
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
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
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
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#ifndef _VIEWSH_HXX //autogen
#include <viewsh.hxx>
#endif
#ifndef _TABFRM_HXX //autogen
#include <tabfrm.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _SCH_DLL_HXX
#include <sch/schdll.hxx>
#endif
#ifndef _SCH_MEMCHRT_HXX
#include <sch/memchrt.hxx>
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
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PropertyAttribute_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _UNOTBL_HXX
#include <unotbl.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::rtl;
//collectn.cxx
BOOL lcl_IsNumeric(const String&);

//-----------------------------------------------------------------------------
//aus unoobj.cxx
extern void lcl_SetTxtFmtColl(const uno::Any& rAny, SwPaM& rPaM)    throw (IllegalArgumentException);
#define UNO_TABLE_COLUMN_SUM    10000

#define EXCEPT_ON_PROTECTION(rUnoCrsr)  \
    if((rUnoCrsr).HasReadonlySel()) \
        throw uno::RuntimeException();

#define EXCEPT_ON_TBL_PROTECTION(rUnoTblCrsr)   \
    if((rUnoTblCrsr).HasReadOnlyBoxSel()) \
        throw uno::RuntimeException();

/* -----------------17.07.98 15:47-------------------
 *
 * --------------------------------------------------*/
table::BorderLine lcl_SvxLineToLine(const SvxBorderLine* pLine)
{
     table::BorderLine aLine;
    if(pLine)
    {
        aLine.Color          = pLine->GetColor().GetColor() ;
        aLine.InnerLineWidth = pLine->GetInWidth()  ;
        aLine.OuterLineWidth = pLine->GetOutWidth() ;
        aLine.LineDistance   = pLine->GetDistance() ;
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
    rSvxLine.SetInWidth( rLine.InnerLineWidth  );
    rSvxLine.SetOutWidth(rLine.OuterLineWidth  );
    rSvxLine.SetDistance(rLine.LineDistance  );
    sal_Bool bRet = rLine.InnerLineWidth > 0 || rLine.OuterLineWidth > 0;
    return bRet;
}
/* -----------------11.12.98 14:22-------------------
 *
 * --------------------------------------------------*/
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
            }
            else if(FN_TABLE_RELATIVE_WIDTH == pMap->nWID)
            {
                sal_Int16 nSet;
                aValue >>= nSet;
                if(nSet && nSet <=100)
                    aSz.SetWidthPercent( nSet );
            }
            else if(FN_TABLE_IS_RELATIVE_WIDTH == pMap->nWID)
            {
                sal_Bool bPercent = *(sal_Bool*)aValue.getValue();
                if(!bPercent)
                    aSz.SetWidthPercent(0);
                else
                    DBG_ERROR("relative Breite kann nicht per sal_Bool eingeschaltet werden");
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
                sPageStyle = SwXStyleFamilies::GetUIName(sPageStyle, SFX_STYLE_FAMILY_PAGE);
                const SwPageDesc* pDesc = ::GetPageDescByName_Impl(*pFmt->GetDoc(), sPageStyle);
                if(pDesc)
                {
                    SwFmtPageDesc aDesc( pDesc );
//                      SwFmtPageDesc aDesc();
//                      uno::Any* pPgNo = pProps->GetProperty(UNO_NAME_PAGE_NUMBER_OFFSET );
//                      if(pPgNo)
//                      {
//                          aDesc.SetNumOffset( TypeConversion::toINT16(*pPgNo) );
//                      }
                    pFmt->GetDoc()->SetAttr(aDesc, *pFmt);
                    break;
                }
            }
        }
        break;
        default:
            throw IllegalArgumentException();
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
            sal_uInt16* nPercent = 0;
            DBG_WARNING("not implemented")
//          SwTwips nWidth = ::GetTableWidth(pFmt, TabCols?, &nPercent);
//          if(FN_TABLE_WIDTH = pMap->nWID)
//              aRet.setINT32(nWidth);
//          else if(FN_TABLE_RELATIVE_WIDTH)
//              aRet.setINT16(nPercent);
//          else
//              aRet.SetBOOL( 0 != nPercent);
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
                   sPDesc = SwXStyleFamilies::GetProgrammaticName(pDsc->GetName(), SFX_STYLE_FAMILY_PAGE);
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
    }
    return aRet;
}
/* -----------------25.06.98 08:32-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------06.10.99 14:46-------------------

 --------------------------------------------------*/
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
    rRow = sRow.ToInt32();
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
            char cChar = sCol.GetChar(nLen-1);

            if( cChar <= 'Z' )
                rCol += nBase * (cChar - 'A' + nBase > 1 ? 1 : 0  );
            else
                rCol += nBase * (cChar - 'a' + 25);

            sCol.Erase(nLen -1, 1);
            nLen = sCol.Len();
            nBase *= 50;
        }
        while(nLen);
    }
}

/* -----------------25.06.98 08:32-------------------
 *
 * --------------------------------------------------*/
SwXCell* lcl_CreateXCell(SwFrmFmt* pFmt, sal_Int16 nColumn, sal_Int16 nRow)
{
    SwXCell* pXCell = 0;
    String sCellName = lcl_GetCellName(nColumn, nRow);
    SwTable* pTable = SwTable::FindTable( pFmt );
    SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
    if(pBox)
    {
        pXCell = SwXCell::CreateXCell(pFmt, pBox, &sCellName);
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

    pTable->GetTabCols( aCols, pBox, sal_False );

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

    pTable->GetTabCols( aOldCols, pBox, sal_False );
    sal_uInt16 nOldCount = aOldCols.Count();

    const uno::Sequence< TableColumnSeparator>* pSepSeq =
                (uno::Sequence< TableColumnSeparator>*) rVal.getValue();
    if(pSepSeq && pSepSeq->getLength() == nOldCount)
    {
        SwTabCols aCols(aOldCols);
        sal_Bool bError = sal_False;
        const TableColumnSeparator* pArray = pSepSeq->getConstArray();
        sal_uInt16 nLastValue = 0;
        sal_uInt16 nTblWidth = aCols.GetRight() - aCols.GetLeft();
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
        { SW_PROP_NAME(UNO_NAME_SIZE_RELATIVE),         FN_TABLE_IS_RELATIVE_WIDTH,  &::getBooleanCppuType()  ,     PROPERTY_NONE, 0 },
        { SW_PROP_NAME(UNO_NAME_TABLE_NAME),          0,                      &::getCppuType((const OUString*)0),       PROPERTY_NONE, 0 },
        { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),            RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),         RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
        { SW_PROP_NAME(UNO_NAME_TRANSPARENT_BACKGROUND),    RES_BACKGROUND,     &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { SW_PROP_NAME(UNO_NAME_WIDTH),                 FN_TABLE_WIDTH,         &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, 0},
        { SW_PROP_NAME(UNO_NAME_CHART_ROW_AS_LABEL),        FN_UNO_RANGE_ROW_LABEL,         &::getBooleanCppuType(),            PROPERTY_NONE,  0},
        { SW_PROP_NAME(UNO_NAME_CHART_COLUMN_AS_LABEL),     FN_UNO_RANGE_COL_LABEL,         &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_TABLE_BORDER),          FN_UNO_TABLE_BORDER,            &::getCppuType((const table::TableBorder*)0),   PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
        {0,0}
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
SwXCell::SwXCell(SwFrmFmt* pTblFmt, SwTableBox* pBx, const String& rCellName) :
    SwXText(pTblFmt->GetDoc(), CURSOR_TBLTEXT),
    sCellName(rCellName),
    pBox(pBx),
    pStartNode(0),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_CELL)),
    SwClient(pTblFmt)
{
}
/* -----------------------------09.08.00 15:59--------------------------------

 ---------------------------------------------------------------------------*/
SwXCell::SwXCell(SwFrmFmt* pTblFmt, const SwStartNode& rStartNode) :
    SwXText(pTblFmt->GetDoc(), CURSOR_TBLTEXT),
    pBox(0),
    pStartNode(&rStartNode),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_CELL)),
    SwClient(pTblFmt)
{
}

/*-- 11.12.98 10:56:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXCell::~SwXCell()
{

}
/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< uno::Type > SAL_CALL SwXCell::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Sequence< uno::Type > aCellTypes = SwXCellBaseClass::getTypes();
    uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();

    long nIndex = aCellTypes.getLength();
    aCellTypes.realloc(
        aCellTypes.getLength() +
        aTextTypes.getLength());

    uno::Type* pCellTypes = aCellTypes.getArray();

    const uno::Type* pTextTypes = aTextTypes.getConstArray();
    for(long nPos = 0; nPos <aTextTypes.getLength(); nPos++)
        pCellTypes[nIndex++] = pTextTypes[nPos];

    return aCellTypes;
}
/* -----------------------------18.05.00 10:18--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< sal_Int8 > SAL_CALL SwXCell::getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId( 16 );
    static BOOL bInit = FALSE;
    if(!bInit)
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
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
    throw (RuntimeException)
{
    uno::Any aRet = SwXText::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXCellBaseClass::queryInterface(aType);
    return aRet;
}
/*-- 11.12.98 10:56:24---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextCursor >   SwXCell::createCursor()
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
        const SwTableBox* pFoundBox = sCellName.Len() ?
                        pTable->GetTblBox( sCellName ) : SwXCell::FindBox(pTable, pBox);
        if(pFoundBox != pBox)
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
        SwAttrSet aSet(pBox->ClaimFrmFmt()->GetAttrSet());
        SwTblBoxFormula aFormula((const SwTblBoxFormula&)  aSet.Get( RES_BOXATR_FORMULA ));
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
        // Der Text muß zunaechst (vielleicht) geloescht werden
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
/*-- 11.12.98 10:56:26---------------------------------------------------

  -----------------------------------------------------------------------*/
double SwXCell::getValue(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    double fRet = 0.;
    if(IsValid())
    {

        SwAttrSet aSet(pBox->ClaimFrmFmt()->GetAttrSet());
        fRet = ((SwTblBoxValue&)aSet.Get(RES_BOXATR_VALUE)).GetValue();
    }
    return fRet;
}
/*-- 11.12.98 10:56:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::setValue(double rValue) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(IsValid())
    {
        // Der Text muß zunaechst (vielleicht) geloescht werden
        sal_uInt32 nNdPos = pBox->IsValidNumTxtNd( sal_True );
        if(USHRT_MAX == nNdPos)
            setString(OUString());
        SwDoc* pDoc = GetDoc();
        UnoActionContext aAction(pDoc);
        SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
        SfxItemSet aSet(pDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE);
        const SfxPoolItem* pItem;
        if(SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
            ||  pDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue()))
        {
            aSet.Put(SwTblBoxNumFormat(0));
        }

        SwTblBoxValue aVal(rValue);
        aSet.Put(aVal);
        pDoc->SetTblBoxFormulaAttrs( *pBox, aSet );
        //Tabelle aktualisieren
        SwTableFmlUpdate aTblUpdate( SwTable::FindTable( GetFrmFmt() ));
        pDoc->UpdateTblFlds( &aTblUpdate );
    }
}
/*-- 11.12.98 10:56:26---------------------------------------------------

  -----------------------------------------------------------------------*/
table::CellContentType SwXCell::getType(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")
    return  (table::CellContentType)0;
}
/* -----------------27.04.99 12:06-------------------
 *
 * --------------------------------------------------*/
void SwXCell::setString(const OUString& aString) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(IsValid())
    {
        SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
        pBoxFmt->LockModify();
        pBoxFmt->ResetAttr( RES_BOXATR_FORMULA );
        pBoxFmt->ResetAttr( RES_BOXATR_VALUE );
        pBoxFmt->UnlockModify();
    }
    SwXText::setString(aString);
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
uno::Reference< XTextCursor >  SwXCell::createTextCursor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextCursor >   aRef;
    if(pStartNode || IsValid())
    {
        const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        SwXTextCursor* pCrsr = new SwXTextCursor(this, aPos, CURSOR_TBLTEXT, GetDoc());
        SwUnoCrsr* pUnoCrsr = pCrsr->GetCrsr();
        pUnoCrsr->Move(fnMoveForward, fnGoNode);
        aRef =  (XWordCursor*)pCrsr;
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS ))
//          throw( uno::RuntimeException() );
    }
    else
        throw uno::RuntimeException();
    return aRef;
}
/*-- 11.12.98 10:56:28---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextCursor >  SwXCell::createTextCursorByRange(const uno::Reference< XTextRange > & xTextPosition)
                                                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextCursor >  aRef;
    SwUnoInternalPaM aPam(*GetDoc());
    if((pStartNode || IsValid()) && SwXTextRange::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
        if(aPam.GetNode()->FindStartNode() == pSttNd)
            aRef =  (XWordCursor*)new SwXTextCursor(this , *aPam.GetPoint(), CURSOR_TBLTEXT, GetDoc(), aPam.GetMark());
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
void SwXCell::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue) throw( beans::UnknownPropertyException, beans::PropertyVetoException, IllegalArgumentException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(IsValid())
    {
        SwFrmFmt* pBoxFmt = pBox->ClaimFrmFmt();
        SwAttrSet aSet(pBoxFmt->GetAttrSet());
        aPropSet.setPropertyValue(rPropertyName, aValue, aSet);
        pBoxFmt->GetDoc()->SetAttr(aSet, *pBoxFmt);
    }
}
/*-- 11.12.98 10:56:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXCell::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    if(IsValid())
    {
        SwFrmFmt* pBoxFmt = pBox->ClaimFrmFmt();
        const SwAttrSet& rSet = pBoxFmt->GetAttrSet();
        aRet = aPropSet.getPropertyValue(rPropertyName, rSet);
    }
    return aRet;
}
/*-- 11.12.98 10:56:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 10:56:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 10:56:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 10:56:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCell::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
        aRef = new SwXParagraphEnumeration(this, pUnoCrsr, CURSOR_TBLTEXT);
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS ))
//          throw( uno::RuntimeException() );
    }
    return aRef;
}
/*-- 11.12.98 10:56:38---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SAL_CALL SwXCell::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<XTextRange>*)0);

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
SwXCell* SwXCell::CreateXCell(SwFrmFmt* pTblFmt, SwTableBox* pBox, const String* pCellName)
{
    SwXCell* pRet = 0;
    if(pTblFmt && pBox)
    {
        SwTable* pTable = SwTable::FindTable( pTblFmt );
        SwTableBox* pFoundBox = SwXCell::FindBox(pTable, pBox);
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
                pXCell = new SwXCell(pTblFmt, pBox, pCellName? *pCellName : aEmptyStr);
            pRet = pXCell;
        }
    }
    return pRet;
}
/* -----------------12.06.98 07:37-------------------
 *  exitstiert die Box in der angegebenen Tabelle?
 * --------------------------------------------------*/
SwTableBox* SwXCell::FindBox(SwTable* pTable, SwTableBox* pBox)
{
    if( pTable->GetTabSortBoxes().Seek_Entry( pBox ))
        return pBox;
    return 0;
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXCell::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXCell");
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXCell::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.table.Cell")||
            sServiceName.EqualsAscii("com.sun.star.table.CellProperties");
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXCell::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.table.Cell");
    pArray[1] = C2U("com.sun.star.table.CellProperties");
    return aRet;
}

/******************************************************************
 * SwXTextTableRow
 ******************************************************************/
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextTableRow::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextTableRow");
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextTableRow::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextTableRow") == rServiceName;
}
/* -----------------------------19.04.00 15:20--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextTableRow::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
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
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, IllegalArgumentException, WrappedTargetException, uno::RuntimeException )
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
            if(!pMap)
                throw beans::UnknownPropertyException();
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
                    aPropSet.setPropertyValue(rPropertyName, aValue, aSet);
                    pDoc->SetAttr(aSet, *pLnFmt);
                }
            }
        }
    }
}
/*-- 11.12.98 12:04:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextTableRow::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
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
            if(!pMap)
                throw beans::UnknownPropertyException();
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
                    aRet = aPropSet.getPropertyValue(rPropertyName, rSet);
                }
            }
        }
    }
    return aRet;
}
/*-- 11.12.98 12:04:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:04:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:04:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:04:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableRow::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
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
OUString SwXTextTableCursor::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextTableCursor");
}
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextTableCursor::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextTableCursor") == rServiceName;
}
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextTableCursor::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextTableCursor");
    return aRet;
}

/*-- 11.12.98 12:16:13---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 11.12.98 12:16:14---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTableCursor::SwXTextTableCursor(SwFrmFmt& rTableFmt, const SwTableCursor* pTableSelection) :
    SwClient(&rTableFmt),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE_CURSOR)),
    aCrsrDepend(this, 0)
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
        sCellName.ToUpperAscii();
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
        bRet = pTblCrsr->LeftRight(sal_True, Count);
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
        bRet = pTblCrsr->LeftRight(sal_False, Count);
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
        bRet = pTblCrsr->UpDown(sal_True, Count);
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
        bRet = pTblCrsr->UpDown(sal_False, Count);
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
        EXCEPT_ON_TBL_PROTECTION(*pTblCrsr)

        bRet = TBLMERGE_OK == pTblCrsr->GetDoc()->MergeTbl(*pTblCrsr);
        pTblCrsr->MakeBoxSels();
    }
    return bRet;
}
/*-- 11.12.98 12:16:16---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextTableCursor::splitRange(sal_Int16 Count, sal_Bool Horizontal) throw( uno::RuntimeException )
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
        EXCEPT_ON_TBL_PROTECTION(*pTblCrsr)

        bRet = pTblCrsr->GetDoc()->SplitTbl( pTblCrsr->GetBoxes(), !Horizontal, Count );
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
void SwXTextTableCursor::setPropertyValue(const OUString& rPropertyName,
                                                        const uno::Any& aValue)
            throw( beans::UnknownPropertyException,
                        beans::PropertyVetoException,
                     IllegalArgumentException,
                     WrappedTargetException,
                     uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode()->FindStartNode();
        const SwTableNode* pTblNode = pSttNode->FindTableNode();
        lcl_FormatTable((SwFrmFmt*)pTblNode->GetTable().GetFrmFmt());
        SwUnoTableCrsr* pTblCrsr = *pUnoCrsr;
        EXCEPT_ON_TBL_PROTECTION(*pTblCrsr)
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                    aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
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
                    EXCEPT_ON_PROTECTION(*pUnoCrsr)

                    SfxItemSet rSet(pDoc->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    SwXTextCursor::GetCrsrAttr(pTblCrsr->GetSelRing(), rSet);
                    aPropSet.setPropertyValue(rPropertyName, aValue, rSet);
                    SwXTextCursor::SetCrsrAttr(pTblCrsr->GetSelRing(), rSet, sal_True);
                }
            }
        }
    }
}
/*-- 11.12.98 12:16:17---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextTableCursor::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode()->FindStartNode();
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
                    SvxBrushItem aBrush;
                    if(pTblCrsr->GetDoc()->GetBoxBackground( *pUnoCrsr, aBrush ))
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
                    aRet = aPropSet.getPropertyValue(rPropertyName, aSet);
                }
            }
        }
    }
    return aRet;
}
/*-- 11.12.98 12:16:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:16:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:16:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:16:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTableCursor::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
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

    sal_Bool    SetProperty(const char* pName, uno::Any aVal);
    sal_Bool    GetProperty(const char* pName, uno::Any*& rpAny);

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
    const SfxItemPropertyMap* pTemp = _pMap;
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
    String aName(C2S(pName));
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
    if(GetProperty(UNO_NAME_REPEAT_HEADLINE, pRepHead ))
    {
        sal_Bool bVal = *(sal_Bool*)pRepHead->getValue();
        ((SwTable&)rTbl).SetHeadlineRepeat(bVal);
    }

    uno::Any* pBackColor    = 0;
    GetProperty(UNO_NAME_BACK_COLOR, pBackColor );
    uno::Any* pBackTrans    = 0;
    GetProperty(UNO_NAME_TRANSPARENT_BACKGROUND, pBackTrans );
    uno::Any* pGrLoc        = 0;
    GetProperty(UNO_NAME_GRAPHIC_LOCATION, pGrLoc   );
    uno::Any* pGrURL        = 0;
    GetProperty(UNO_NAME_GRAPHIC_URL, pGrURL     );
    uno::Any* pGrFilter     = 0;
    GetProperty(UNO_NAME_GRAPHIC_FILTER, pGrFilter     );

    if(pBackColor||pBackTrans||pGrURL||pGrFilter||pGrLoc)
    {
        SvxBrushItem aBrush(RES_BACKGROUND);
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
            sPageStyle = SwXStyleFamilies::GetUIName(sPageStyle, SFX_STYLE_FAMILY_PAGE);
            const SwPageDesc* pDesc = ::GetPageDescByName_Impl(rDoc, sPageStyle);
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
        SvxFmtBreakItem aBreak;
        aBreak.PutValue(*pBreak, 0);
        aSet.Put(aBreak);
    }
    uno::Any* pShadow;
    if(GetProperty(UNO_NAME_SHADOW_FORMAT, pShadow))
    {
        SvxShadowItem aShd(RES_SHADOW);
        aShd.PutValue(*pShadow, CONVERT_TWIPS);
        aSet.Put(aShd);
    }
    uno::Any* pKeep;
    if(GetProperty(UNO_NAME_KEEP_TOGETHER, pKeep))
    {
        SvxFmtKeepItem aKeep(RES_KEEP);
        aKeep.PutValue(*pKeep, 0);
        aSet.Put(aKeep);
    }

    sal_Bool bFullAlign = sal_True;
    uno::Any* pHOrient;
    if(GetProperty(UNO_NAME_HORI_ORIENT, pHOrient))
    {
        SwFmtHoriOrient aOrient;
        ((SfxPoolItem&)aOrient).PutValue(*pHOrient, MID_HORIORIENT_ORIENT|CONVERT_TWIPS);
        bFullAlign = (aOrient.GetHoriOrient() == HORI_FULL);
        aSet.Put(aOrient);
    }


    uno::Any* pSzRel        = 0;
    GetProperty(UNO_NAME_SIZE_RELATIVE, pSzRel  );
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
        SvxLRSpaceItem aLR(RES_LR_SPACE);
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
        SvxULSpaceItem aUL(RES_UL_SPACE);
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
SwXTextTable* SwXTextTable::GetImplementation(Reference< XInterface> xRef )
{
    uno::Reference<lang::XUnoTunnel> xTunnel( xRef, uno::UNO_QUERY);
    if(xTunnel.is())
        return (SwXTextTable*)xTunnel->getSomething(SwXTextTable::getUnoTunnelId());
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
            return (sal_Int64)this;
    }
    return 0;
}
/*-- 11.12.98 12:42:43---------------------------------------------------

  -----------------------------------------------------------------------*/
TYPEINIT1(SwXTextTable, SwClient)

/*-- 11.12.98 12:42:43---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTable::SwXTextTable() :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    aLstnrCntnr( (XTextTable*)this),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False),
    pLastSortOptions(0),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    bIsDescriptor(sal_True),
    nRows(2),
    nColumns(2),
    pTableProps(new SwTableProperties_Impl(GetTableDescPropertyMap()))
{

}
/*-- 11.12.98 12:42:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTable::SwXTextTable(SwFrmFmt& rFrmFmt) :
    SwClient( &rFrmFmt ),
    aLstnrCntnr( (XTextTable*)this),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    pLastSortOptions(0),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_TABLE)),
    bIsDescriptor(sal_False),
    nRows(0),
    nColumns(0),
    pTableProps(0)
{

}
/*-- 11.12.98 12:42:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextTable::~SwXTextTable()
{
    delete pLastSortOptions;
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
        nRows = nR;
        nColumns = nC;
    }
}
/*-- 11.12.98 12:42:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XTableRows >  SwXTextTable::getRows(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XTableRows >  xRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
        xRet = new SwXTableRows(*pFmt);
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 12:42:45---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XTableColumns >  SwXTextTable::getColumns(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XTableColumns >  xRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
        xRet = new SwXTableColumns(*pFmt);
    else
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
            xRet = SwXCell::CreateXCell(pFmt, pBox, &sCellName);
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
uno::Reference< XTextTableCursor >  SwXTextTable::createCursorByCellName(const OUString& CellName)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/* -----------------18.02.99 13:36-------------------
 *
 * --------------------------------------------------*/
void SwXTextTable::attachToRange(const uno::Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, uno::RuntimeException )
{
    uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    SwXTextCursor* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
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

            pDoc->StartUndo();
            const SwTable *pTable = 0;
            if( 0 != aPam.Start()->nContent.GetIndex() )
            {
                pDoc->SplitNode(*aPam.Start() );
            }
            //TODO: wenn es der letzte Absatz ist, dann muss noch ein Absatz angehaengt werden!
            pDoc->DeleteAndJoin(aPam);
            aPam.DeleteMark();
            pTable = pDoc->InsertTable(
                                        *aPam.GetPoint(),
                                        nRows,
                                        nColumns,
                                        HORI_FULL,
                                        HEADLINE|DEFAULT_BORDER);
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
            pDoc->EndUndo( UNDO_END );
        }
    }
    else
        throw IllegalArgumentException();
}
/*-- 11.12.98 12:42:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::attach(const uno::Reference< XTextRange > & xTextRange)
        throw( IllegalArgumentException, uno::RuntimeException )
{
}
/*-- 11.12.98 12:42:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextTable::getAnchor(void)
        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!pFmt)
        throw uno::RuntimeException();
    uno::Reference< XTextRange >  xRet = new SwXTextRange(*pFmt);
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
void SwXTextTable::addEventListener(const uno::Reference< XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 11.12.98 12:42:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 11.12.98 12:42:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCell >  SwXTextTable::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
        throw uno::RuntimeException();
    return aRef;

}
/* -----------------11.12.98 13:26-------------------
 *
 * --------------------------------------------------*/
uno::Reference< table::XCellRange >  SwXTextTable::GetRangeByName(SwFrmFmt* pFmt, SwTable* pTable,
                    const String& sTLName, const String& sBRName,
                    SwRangeDescriptor& rDesc)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XCellRange >  aRef;
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
                throw( uno::RuntimeException )
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
            aRef = GetRangeByName(pFmt, pTable, sTLName, sBRName, aDesc);
        }
    }
    if(!aRef.is())
        throw uno::RuntimeException();
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
/*-- 11.12.98 12:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Sequence< double > > SwXTextTable::getData(void)
                                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    //werden hier uno::Sequence<s fuer jede Zeile in einer Spaltenuno::Sequence< geliefert oder umgekehrt?
    uno::Sequence< uno::Sequence< double > > aColSeq(bFirstColumnAsLabel ? nColCount - 1 : nColCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        uno::Sequence< double >* pColArray = aColSeq.getArray();

        sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
        for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
        {
            uno::Sequence< double > aRowSeq(bFirstRowAsLabel ? nRowCount - 1 : nRowCount);
            double * pArray = aRowSeq.getArray();
            sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                pArray[nRow - nRowStart] = xCell->getValue();
            }
            pColArray[nCol - nColStart] = aRowSeq;
        }
    }
    else
        throw uno::RuntimeException();
    return aColSeq;
}
/*-- 11.12.98 12:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::setData(const uno::Sequence< uno::Sequence< double > >& rData)
                                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
        if(rData.getLength() < nColCount - nColStart)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< double >* pColArray = rData.getConstArray();
        for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
        {
            const uno::Sequence< double >& rRowSeq = pColArray[nCol - nColStart];
            sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
            if(rRowSeq.getLength() < nRowCount - nRowStart)
            {
                throw uno::RuntimeException();
            }
            const double * pRowArray = rRowSeq.getConstArray();
            for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                xCell->setValue(pRowArray[nRow - nRowStart]);
            }
        }
    }
}
/*-- 11.12.98 12:42:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextTable::getRowDescriptions(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
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
        if(rRowDesc.getLength() < bFirstRowAsLabel ? nRowCount - 1 : nRowCount)
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
            DBG_ERROR("Wo kommen die Labels her?")
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 12:42:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::addChartDataChangeEventListener(const uno::Reference< chart::XChartDataChangeEventListener > & aListener) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:42:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::removeChartDataChangeEventListener(const uno::Reference< chart::XChartDataChangeEventListener > & aListener) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/* -----------------08.03.99 15:33-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextTable::isNotANumber(double nNumber) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return sal_False;

}
/* -----------------08.03.99 15:34-------------------
 *
 * --------------------------------------------------*/
double SwXTextTable::getNotANumber(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return 0.;
}
/*-- 11.12.98 12:42:48---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SwXTextTable::createSortDescriptor(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< beans::PropertyValue > aRet;
    //XTextSortDescriptor noch nicht wieder definiert
    DBG_WARNING("not implemented")
    return aRet;
    /*SwXTextSortDescriptor* pDesc = new SwXTextSortDescriptor(sal_True);
    uno::Reference< XSortDescriptor >  xRet = pDesc;
    if(!bEmpty && pLastSortOptions)
        pDesc->SetSortOptions(*pLastSortOptions);
    return xRet;*/
}
/*-- 11.12.98 12:42:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::sort(const uno::Sequence< beans::PropertyValue >& xDescriptor)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")
/*
    SwFrmFmt* pFmt = GetFrmFmt();
    SwXTextSortDescriptor* pDesc = (SwXTextSortDescriptor*)
                        xDescriptor->getImplementation(::getCppuType((const SwXTextSortDescriptor*)0));

    if(pFmt && pDesc && pDesc->GetSortOptions().aKeys.Count())
    {
        delete pLastSortOptions;
        pLastSortOptions = new SwSortOptions(pDesc->GetSortOptions());
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwSelBoxes aBoxes;
        const SwTableSortBoxes& rTBoxes = pTable->GetTabSortBoxes();
        for( sal_uInt16 n = 0; n < rTBoxes.Count(); ++n )
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.Insert( pBox );
        }
        UnoActionContext aContext( pFmt->GetDoc() );
        pFmt->GetDoc()->SortTbl(aBoxes, pDesc->GetSortOptions());
    }

 * */
}
/*-- 11.12.98 12:42:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::autoFormat(const OUString& aName) throw( IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {

            String sAutoFmtName(aName);

            DBG_ERROR("AutoFormat: Dafuer sollte es eine autom. Aktualisierung geben!");

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
                IllegalArgumentException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!aValue.hasValue())
        throw IllegalArgumentException();
    if(pFmt)
    {
        lcl_FormatTable(pFmt);
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    _pMap, rPropertyName);
        if(!pMap)
            throw beans::UnknownPropertyException();
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
                        DBG_ERROR("Listener am CharData-interface benachrichtigen")
                        bFirstRowAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstColumnAsLabel != bTmp)
                    {
                        DBG_ERROR("Listener am CharData-interface benachrichtigen")
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

                            aBox.SetDistance(MM100_TO_TWIP(pBorder->Distance));
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
                    aPropSet.setPropertyValue(rPropertyName, aValue, aSet);
                    pFmt->GetDoc()->SetAttr(aSet, *pFmt);
                }
            }
        }
    }
    else if(bIsDescriptor)
    {
        String aPropertyName(rPropertyName);
        if(!pTableProps->SetProperty(ByteString(aPropertyName, RTL_TEXTENCODING_ASCII_US).GetBuffer(), aValue))
            throw IllegalArgumentException();
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 12:42:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextTable::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        lcl_FormatTable(pFmt);
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    _pMap, rPropertyName);
        if(!pMap)
            throw beans::UnknownPropertyException();
        if(0xFF == pMap->nMemberId)
        {
            aRet = lcl_GetSpecialProperty(pFmt, pMap );
        }
        else
        {
            switch(pMap->nWID)
            {
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
                        aTableBorder.Distance               = rBox.GetDistance();
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
                default:
                {
                    const SwAttrSet& rSet = pFmt->GetAttrSet();
                    aRet = aPropSet.getPropertyValue(rPropertyName, rSet);
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
/*-- 11.12.98 12:42:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:42:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:42:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 12:42:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextTable::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
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
        aLstnrCntnr.Disposing();
}
/* -----------------14.01.99 10:43-------------------
 *
 * --------------------------------------------------*/
/*uno::Sequence< uno::Uik > SwXTextTable::getUiks()
{
    uno::Sequence< uno::Uik > aUiks(8);
 uno::Uik* pArray = aUiks.getArray();
    pArray[0] = ::getCppuType((const uno::Reference< XTextTable >*)0);
    pArray[1] = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
    pArray[2] = ::getCppuType((const uno::Reference< container::XNamed >*)0);
    pArray[3] = ::getCppuType((const uno::Reference< table::XCellRange >*)0);
    pArray[4] = ::getCppuType((const uno::Reference< table::XAutoFormattable >*)0);
    pArray[5] = ::getCppuType((const uno::Reference< util::XSortable >*)0);
    pArray[6] = ::getCppuType((const uno::Reference< table::XAutoFormattable >*)0);
    pArray[7] = ::getCppuType((const uno::Reference< chart::XChartData >*)0);
    return aUiks;
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
                sServiceName.EqualsAscii("com.sun.star.text.TextContent"));
}
/* -----------------25.10.99 15:12-------------------

 --------------------------------------------------*/
uno::Sequence< OUString > SwXTextTable::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(3);
    OUString* pArr = aRet.getArray();
    pArr[0] = C2U("com.sun.star.text.TextTable");
    pArr[1] = C2U("com.sun.star.document.LinkTarget");
    pArr[2] = C2U("com.sun.star.text.TextContent");
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
            return (sal_Int64)this;
    }
    return 0;
}
/* -----------------28.04.98 10:29-------------------
 *
 * --------------------------------------------------*/
TYPEINIT1(SwXCellRange, SwClient);
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXCellRange::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXCellRange");
}
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXCellRange::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.table.CellRange") == rServiceName;
}
/* -----------------------------19.04.00 15:21--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXCellRange::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.table.CellRange");
    return aRet;
}
/*-- 11.12.98 14:27:32---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXCellRange::SwXCellRange() :
    aCursorDepend(this, 0),
    pTblCrsr(0),
    pLastSortOptions(0),
    aPropSet(0),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False),
    _pMap(0)
{

}
/*-- 11.12.98 14:27:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXCellRange::SwXCellRange(SwUnoCrsr* pCrsr, SwFrmFmt& rFrmFmt,
                                            SwRangeDescriptor& rDesc) :
    SwClient(&rFrmFmt),
    aCursorDepend(this, pCrsr),
    aRgDesc(rDesc),
    pTblCrsr(pCrsr),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_RANGE)),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False),
    pLastSortOptions(0),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TABLE_RANGE))
{

}
/*-- 11.12.98 14:27:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXCellRange::~SwXCellRange()
{
    delete pLastSortOptions;
    delete pTblCrsr;
}
/*-- 11.12.98 14:27:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCell >  SwXCellRange::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
                                            throw( uno::RuntimeException )
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
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 14:27:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCellRange >  SwXCellRange::getCellRangeByPosition(
        sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom)
        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< table::XCellRange >  aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt && getColumnCount() > nRight && getRowCount() > nBottom &&
        nLeft <= nRight && nTop <= nBottom)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            SwRangeDescriptor aNewDesc;
            aNewDesc.nTop    = nTop + aRgDesc.nTop;
            aNewDesc.nBottom = nBottom + aRgDesc.nTop;
            aNewDesc.nLeft   = nLeft +  aRgDesc.nLeft;
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
        throw uno::RuntimeException();
    return aRet;

}
/*-- 11.12.98 14:27:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< table::XCellRange >  SwXCellRange::getCellRangeByName(const OUString& aRange) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return uno::Reference< table::XCellRange > ();
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
        beans::PropertyVetoException, IllegalArgumentException,
            WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        lcl_FormatTable(pFmt);
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
        if(pMap)
        {
            SwDoc* pDoc = pTblCrsr->GetDoc();
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
                        DBG_ERROR("Listener am CharData-interface benachrichtigen")
                        bFirstRowAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstColumnAsLabel != bTmp)
                    {
                        DBG_ERROR("Listener am CharData-interface benachrichtigen")
                        bFirstColumnAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_PARA_STYLE:
                    lcl_SetTxtFmtColl(aValue, *pTblCrsr);
                break;
                default:
                {
                    SfxItemSet rSet(pDoc->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        0L);
                    SwUnoTableCrsr* pCrsr = *pTblCrsr;
                    SwXTextCursor::GetCrsrAttr(pCrsr->GetSelRing(), rSet);
                    aPropSet.setPropertyValue(rPropertyName, aValue, rSet);
                    SwXTextCursor::SetCrsrAttr(pCrsr->GetSelRing(), rSet, sal_True);
                }
            }
        }
    }
}
/*-- 11.12.98 14:27:35---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXCellRange::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        lcl_FormatTable(pFmt);
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
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
                case RES_BOXATR_FORMAT:
                    //GetAttr fuer Tabellenselektion am Doc fehlt noch
                    DBG_WARNING("not implemented")
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
                default:
                {
                    SfxItemSet aSet(pTblCrsr->GetDoc()->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        0L);
                    // erstmal die Attribute des Cursors
                    SwUnoTableCrsr* pCrsr = *pTblCrsr;
                    SwXTextCursor::GetCrsrAttr(pCrsr->GetSelRing(), aSet);
                    aRet = aPropSet.getPropertyValue(rPropertyName, aSet);
                }
            }
        }
    }
    return aRet;
}
/*-- 11.12.98 14:27:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 14:27:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 14:27:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 14:27:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 14:27:36---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Sequence< double > > SwXCellRange::getData(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    //werden hier uno::Sequence<s fuer jede Zeile in einer Spaltenuno::Sequence< geliefert oder umgekehrt?
    uno::Sequence< uno::Sequence< double > > aColSeq(bFirstColumnAsLabel ? nColCount - 1 : nColCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        uno::Sequence< double >* pColArray = aColSeq.getArray();

        sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
        for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
        {
            uno::Sequence< double > aRowSeq(bFirstRowAsLabel ? nRowCount - 1 : nRowCount);
            double * pArray = aRowSeq.getArray();
            sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    //exception ...
                    break;
                }
                pArray[nRow - nRowStart] = xCell->getValue();
            }
            pColArray[nCol - nColStart] = aRowSeq;
        }
    }
    return aColSeq;
}
/*-- 11.12.98 14:27:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::setData(const uno::Sequence< uno::Sequence< double > >& rData)
                                                throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
        if(rData.getLength() < nColCount - nColStart)
        {
            throw uno::RuntimeException();
            return;
        }
        const uno::Sequence< double >* pColArray = rData.getConstArray();
        for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
        {
            const uno::Sequence< double >& rRowSeq = pColArray[nCol - nColStart];
            sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
            if(rRowSeq.getLength() < nRowCount - nRowStart)
            {
                throw uno::RuntimeException();
                return;
            }
            const double * pRowArray = rRowSeq.getConstArray();
            for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                    break;
                }
                xCell->setValue(pRowArray[nRow - nRowStart]);
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
        if(rRowDesc.getLength() < bFirstRowAsLabel ? nRowCount - 1 : nRowCount)
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
                    break;
                }
                uno::Reference< XText >  xText(xCell, UNO_QUERY);

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
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 14:27:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::removeChartDataChangeEventListener(const uno::Reference< chart::XChartDataChangeEventListener > & aListener) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/* -----------------08.03.99 15:36-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXCellRange::isNotANumber(double nNumber) throw( uno::RuntimeException )
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
    DBG_WARNING("not implemented")
    uno::Sequence< beans::PropertyValue > aRet;
    return aRet;
    /*SwXTextSortDescriptor* pDesc = new SwXTextSortDescriptor(sal_True);
    uno::Reference< XSortDescriptor >  xRet = pDesc;
    if(!bEmpty && pLastSortOptions)
        pDesc->SetSortOptions(*pLastSortOptions);
    return xRet;*/
}
/*-- 11.12.98 14:27:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXCellRange::sort(const uno::Sequence< beans::PropertyValue >& xDescriptor)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_WARNING("not implemented")
    /*
    SwFrmFmt* pFmt = GetFrmFmt();
    SwXTextSortDescriptor* pDesc = (SwXTextSortDescriptor*)
//                      xDescriptor->getImplementation(::getCppuType((const SwXTextSortDescriptor*)0));

    if(pFmt && pDesc && pDesc->GetSortOptions().aKeys.Count())
    {
        delete pLastSortOptions;
        pLastSortOptions = new SwSortOptions(pDesc->GetSortOptions());
        SwUnoTableCrsr* pTableCrsr = *pTblCrsr;
        pTableCrsr->MakeBoxSels();
        UnoActionContext aContext( pFmt->GetDoc() );
        pFmt->GetDoc()->SortTbl(pTableCrsr->GetBoxes(), pDesc->GetSortOptions());
    }*/
}
/* -----------------27.04.98 16:54-------------------
 *
 * --------------------------------------------------*/
sal_uInt16 SwXCellRange::getColumnCount(void)
{
    return aRgDesc.nRight - aRgDesc.nLeft + 1;
}
/* -----------------27.04.98 16:54-------------------
 *
 * --------------------------------------------------*/
sal_uInt16 SwXCellRange::getRowCount(void)
{
    return aRgDesc.nBottom - aRgDesc.nTop + 1;
}
/*-- 11.12.98 14:27:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXCellRange::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew );
    if(!aCursorDepend.GetRegisteredIn())
        pTblCrsr = 0;
}
/******************************************************************
 *  SwXTableRows
 ******************************************************************/
/* -----------------------------19.04.00 15:22--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTableRows::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTableRows");
}
/* -----------------------------19.04.00 15:22--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTableRows::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TableRows") == rServiceName;
}
/* -----------------------------19.04.00 15:22--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTableRows::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TableRows");
    return aRet;
}
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
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 )
        throw IndexOutOfBoundsException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(pTable->GetTabLines().Count() > nIndex)
        {
            SwTableLine* pLine = pTable->GetTabLines().GetObject(nIndex);
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
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 || nCount <= 0)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
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
                pFrmFmt->GetDoc()->InsertRow(*pUnoCrsr, nCount, bAppend);
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
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            String sTLName = lcl_GetCellName(0, nIndex);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, sal_True);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                {
                    // Die Klammer ist wichtig!
                    UnoActionContext aAction(pFrmFmt->GetDoc());
                    pFrmFmt->GetDoc()->DeleteRow(*pUnoCrsr);
                    delete pUnoCrsr;
                }
                // hier muessen die Actions aufgehoben werden
                UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
            }
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
OUString SwXTableColumns::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTableColumns");
}
/* -----------------------------19.04.00 15:23--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTableColumns::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TableColumns") == rServiceName;
}
/* -----------------------------19.04.00 15:23--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTableColumns::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TableColumns");
    return aRet;
}
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
    throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
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
            throw IndexOutOfBoundsException();
        xRet = *new cppu::OWeakObject();
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
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt|| nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            String sTLName = lcl_GetCellName(nIndex, 0);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            sal_Bool bAppend = sal_False;
            if(!pTLBox)
            {
                bAppend = sal_True;
                // am Ende anfuegen, dazu muss der Cursor in die letzte Spalte!
                SwTableLines& rLines = pTable->GetTabLines();
                SwTableLine* pLine = rLines.GetObject(0);
                SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                pTLBox = rBoxes.GetObject(0);
            }
            if(pTLBox)
            {
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                UnoActionContext aAction(pFrmFmt->GetDoc());
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, sal_True);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pFrmFmt->GetDoc()->InsertCol(*pUnoCrsr, nCount, bAppend);
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
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt|| nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            String sTLName = lcl_GetCellName(nIndex, 0);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, sal_True);
                //TODO: hier wird nur eine Spalte geloescht!
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                {   // Die Klammer ist wichtig
                    UnoActionContext aAction(pFrmFmt->GetDoc());
                    pFrmFmt->GetDoc()->DeleteCol(*pUnoCrsr);
                    delete pUnoCrsr;
                }
                // hier muessen die Actions aufgehoben werden
                UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
            }
        }
    }
}
/*-- 03.02.99 07:37:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTableColumns::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.123  2000/09/18 16:04:35  willem.vandorp
    OpenOffice header added.

    Revision 1.122  2000/09/15 07:24:04  os
    #78563# survive setPropertyValue with void

    Revision 1.121  2000/09/12 13:03:59  os
    #78700# types of text content properties corrected

    Revision 1.120  2000/09/05 15:15:18  os
    string length available again

    Revision 1.119  2000/08/24 11:14:34  mib
    bug fixes for XML import

    Revision 1.118  2000/08/18 15:01:26  os
    #77094# getCellRangeByName corrected

    Revision 1.117  2000/08/18 10:25:43  os
    #77790# obsolete type casts removed

    Revision 1.116  2000/08/15 10:09:19  os
    throw exception if cell doesn't exist in CreateCursorByCellName

    Revision 1.115  2000/08/09 14:50:16  os
    create cell objects without SwTableBox*

    Revision 1.114  2000/08/09 13:50:02  os
    #77094# SwXCellRange: check for illegal parameters

    Revision 1.113  2000/07/19 11:26:27  os
    #76861# getError - implemented via string compare

    Revision 1.112  2000/07/19 11:01:49  os
    properties added/renamed

    Revision 1.111  2000/07/11 13:43:44  os
    #76708# insert/remove paragraphs before/behind tables

    Revision 1.110  2000/07/10 12:32:18  os
    chg: acquire/release don't throw exceptions

    Revision 1.109  2000/07/03 14:18:36  os
    #76590# ::getCellByName: reject negative positions

    Revision 1.108  2000/07/03 13:26:34  os
    #76457# support service TextContent

    Revision 1.107  2000/07/03 10:46:46  os
    #76404# SwXTableColumns::getByIndex: check index value

    Revision 1.106  2000/06/30 12:45:52  os
    getByIndex: return type correced

    Revision 1.105  2000/06/27 16:33:57  os
    #76407# IndexOutOfBoundsException instead of RuntimeException

    Revision 1.104  2000/06/27 12:10:33  os
    #76423# programmatic style names

    Revision 1.103  2000/06/13 13:42:23  os
    #75719# special handling of AnchorType of text tables

    Revision 1.102  2000/05/30 11:27:26  os
    #76028# service CellProperties

    Revision 1.101  2000/05/26 07:21:48  os
    old SW Basic API Slots removed

    Revision 1.100  2000/05/18 08:36:32  os
    XInterface - mothods

    Revision 1.99  2000/05/16 17:21:40  jp
    Changes for Unicode

    Revision 1.98  2000/05/16 09:14:55  os
    project usr removed

    Revision 1.97  2000/05/04 15:16:37  os
    reduce size of unoobj.cxx

    Revision 1.96  2000/04/27 10:46:56  os
    UNICODE

    Revision 1.95  2000/04/26 11:35:20  os
    GetName() returns String&

    Revision 1.94  2000/04/19 13:35:31  os
    UNICODE

    Revision 1.93  2000/04/12 13:26:02  os
    SfxItemPropertyMap/Set use OUString

    Revision 1.92  2000/03/27 10:21:11  os
    UNO III

    Revision 1.91  2000/03/21 15:42:25  os
    UNOIII

    Revision 1.90  2000/03/09 21:53:59  jp
    Changes: SchMemChart in new headerfile

    Revision 1.89  2000/02/17 12:13:55  os
    #70258# Range corrected

    Revision 1.88  2000/02/11 14:36:00  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.87  2000/01/07 12:56:43  os
    #67022# SwXTextTable: react to deleted table

    Revision 1.86  1999/11/25 15:43:27  os
    headers corrected

    Revision 1.85  1999/11/22 10:37:58  os
    missing headers added

    Revision 1.84  1999/11/19 16:40:19  os
    modules renamed

    Revision 1.83  1999/10/26 14:35:33  os
    LinkTargetSupplier

    Revision 1.82  1999/10/13 11:52:34  os
    #69150# convert Distance in TableBorder

    Revision 1.81  1999/10/11 09:45:01  os
    #69081# missing break

    Revision 1.80  1999/10/08 11:37:00  os
    call MakeBoxSels after Merge or Split

    Revision 1.79  1999/10/07 12:54:09  os
    #69017# call MakeBoxSels before any other operation

    Revision 1.78  1999/10/06 14:12:05  os
    #69022# GetRangeByName: use a Reference not a pointer

    Revision 1.77  1999/09/23 11:49:30  os
    #68802# getTableNames - dont include empty boxes

    Revision 1.76  1999/07/19 06:43:46  OS
    syntax error, SwWait removed


      Rev 1.75   19 Jul 1999 08:43:46   OS
   syntax error, SwWait removed

      Rev 1.74   13 Jul 1999 16:05:24   MA
   use internal object to toggle wait cursor

      Rev 1.73   08 Jul 1999 11:26:14   OS
   #67500# cursor handling in protected sections

      Rev 1.72   27 Apr 1999 13:01:02   OS
   #65400# Formel zuruecksetzen, wenn setString an der Zelle gerufen wird

      Rev 1.71   26 Apr 1999 11:52:02   OS
   #65199# Tabellenname aus dem Descriptor uebernehmen

      Rev 1.70   23 Apr 1999 08:26:36   OS
   #65194# Semikolon muss weg

      Rev 1.69   22 Apr 1999 16:13:58   OS
   #65194# throw -> throw; #65124# not impl. nur noch warning EventListener

      Rev 1.68   31 Mar 1999 06:45:28   OS
   #63852# AnchorType fuer Tabelle

      Rev 1.67   16 Mar 1999 10:36:52   OS
   #63310# Tabelle mit den richtigen Flags einfuegen

      Rev 1.66   15 Mar 1999 14:37:56   OS
   #62845# Makro fuer ServiceInfo jetzt auch fuer OS/2

      Rev 1.65   12 Mar 1999 09:41:34   OS
   #62845# XServiceInfo impl.

      Rev 1.64   11 Mar 1999 10:14:04   OS
   #63101# SetValue/Formula: richtiges Format uebergeben

      Rev 1.63   11 Mar 1999 08:33:00   OS
   #63110# Text-Intefaces fuer XCell im queryInterface


      Rev 1.62   09 Mar 1999 12:41:30   OS
   #62008# Solar-Mutex

      Rev 1.61   08 Mar 1999 13:51:42   OS
   #62751# XChartDataArray

      Rev 1.60   08 Mar 1999 07:42:42   MH
   update 515

      Rev 1.59   05 Mar 1999 08:43:10   OS
   #62191# INT statt UINT

      Rev 1.58   04 Mar 1999 15:01:28   OS
   #62191# UINT nicht mehr verwenden

      Rev 1.57   25 Feb 1999 13:32:52   OS
   #62405# richtiges Split fuer Tabellen mit korrektem Namen

      Rev 1.56   19 Feb 1999 17:01:08   OS
   #62124# ParagraphEnumeration soll sich in Tabellen nicht verlaufen, #62118# eigener TextRange fuer Tabelle

      Rev 1.55   18 Feb 1999 14:25:54   OS
   #52654# insertTextContent statt attach

      Rev 1.54   11 Feb 1999 16:43:52   OS
   #52654# fehlendes break

      Rev 1.53   11 Feb 1999 16:16:12   OS
   #52654# GetProperty ohne const

      Rev 1.52   05 Feb 1999 14:31:14   OS
   #56371# XCellRange interface veraendert

      Rev 1.51   03 Feb 1999 11:45:12   OS
   #56371# SwXTableRows/Columns

      Rev 1.50   28 Jan 1999 16:46:26   OS
   #56371# keine Objekte fuer DEBUG anlegen

      Rev 1.49   27 Jan 1999 12:06:50   OS
   #56371# TF_ONE51

      Rev 1.48   22 Jan 1999 15:09:24   OS
   #56371# Draw wieder verfuegbar

      Rev 1.47   14 Jan 1999 16:21:48   OS
   #56371# TF_ONE51

      Rev 1.46   15 Dec 1998 10:09:56   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.45   10 Dec 1998 15:53:38   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.44   09 Nov 1998 14:47:22   OS
   #58785# getSelection an der XTextView vollstaendig

      Rev 1.43   04 Nov 1998 13:10:26   OS
   #58229# Zugriffe auf das Layout von unsichtbaren Tabellen verhindern

      Rev 1.42   15 Oct 1998 12:19:32   OS
   #52654# Tabellenformatierung mit SetONECalcLowers

      Rev 1.41   02 Oct 1998 16:09:18   OS
   #52654# CalcLayout fuer Tabelle, Range und TableCursor

      Rev 1.40   30 Sep 1998 10:30:02   OS
   #52654# Sortable und SortDescriptor eingebaut

      Rev 1.39   15 Sep 1998 16:15:10   OS
   #52654# Cursor nach setPropertyValue wieder loeschen

      Rev 1.38   29 Jul 1998 13:05:16   OS
   Height und IsAutoHeight an der TableRow #52654#

      Rev 1.37   24 Jul 1998 10:51:36   OS
   leere BorderLines muessen mit Null gesetzt werden #52654#

      Rev 1.36   21 Jul 1998 17:03:34   OS
   ParagraphStyle am Range und am TextTableCursor  #52654#

      Rev 1.35   21 Jul 1998 14:23:30   OS
   include #52654#

      Rev 1.34   21 Jul 1998 09:15:58   OS
   beans::XPropertySet fuer TextTableCursor #52654#

      Rev 1.33   20 Jul 1998 17:30:04   OS
   neu: SwXTextTableCursor; chg: SwXTextTable erwietert #52654#

      Rev 1.32   18 Jul 1998 16:12:18   OS
   Border und Tabellenspaltenbreiten fuer UNO #54654#

      Rev 1.31   15 Jul 1998 13:18:02   OS
   XChartData an der Table und am Range unterstuetzen #52656#

      Rev 1.30   10 Jul 1998 18:09:04   OS
   PropertySetInfo und IdlClass static

      Rev 1.29   09 Jul 1998 09:14:44   OS
   UnoActionRemoveContext

      Rev 1.28   29 Jun 1998 17:24:10   OS
   Properties Tabellenzeile

      Rev 1.27   27 Jun 1998 16:22:36   OS
   SwXTextTableRow angefangen

      Rev 1.26   26 Jun 1998 18:17:54   OS
   PageDesc setzen

      Rev 1.25   25 Jun 1998 11:14:56   OS
   PreopertyMaps nur noch vom PropertyMapProvider

      Rev 1.24   23 Jun 1998 09:51:26   TRI
   ICC brauchte temp. Objekte

      Rev 1.23   18 Jun 1998 08:10:50   OS
   Property-Namen veraendert

      Rev 1.22   16 Jun 1998 16:53:46   OS
   Bold/Italic/Underlined ersetzt durch Weight, Posture, Underline

      Rev 1.21   15 Jun 1998 09:08:38   OS
   setPropertyValue am CellRange berichtigt

      Rev 1.20   12 Jun 1998 09:46:44   OS
   Tabellenboxen anders suchen

      Rev 1.19   10 Jun 1998 15:57:52   OS
   SwXCellRange::getColumn/RowCount und ::getSubRange berichtigt

      Rev 1.18   10 Jun 1998 08:20:42   OS
   get/setFormula/Value eingebaut

      Rev 1.17   08 Jun 1998 16:29:02   OS
   get/setText korrigiert

      Rev 1.16   08 Jun 1998 10:34:32   OS
   Map-Reihenfolge

      Rev 1.15   04 Jun 1998 09:40:04   OS
   getIdlClasses


      Rev 1.14   03 Jun 1998 09:53:44   OS
   ParagraphEnumeratable

      Rev 1.13   02 Jun 1998 10:33:58   JP
   TabellenAutoFormat: Load/Save ohne Stream - erzeugen diesen selbst

      Rev 1.12   29 May 1998 13:49:18   OS
   UNO_NAME_REGISTER -> _REGISTER_MODE_ACTIVE

      Rev 1.11   22 May 1998 15:10:38   OS
   CreateTextCursorAtPos eingebaut

      Rev 1.10   18 May 1998 12:22:54   OS
   Container fuer Suchergebnis

      Rev 1.9   05 May 1998 10:04:30   OS
   Actions

      Rev 1.8   04 May 1998 08:55:14   OS
   Zeilen+Spalten einfuegen und loeschen

      Rev 1.7   29 Apr 1998 08:00:56   OS
   SwXCellRange

      Rev 1.6   22 Apr 1998 13:43:06   OS
   Tabellenname nur ohne Space und Punkt

      Rev 1.5   17 Apr 1998 13:35:28   OS
   Tabellennamen setzen

      Rev 1.4   09 Apr 1998 15:10:52   OS
   Uno-Umstellung

      Rev 1.3   07 Apr 1998 17:06:22   OS
   ParagraphEnumerator eingebaut

      Rev 1.2   07 Apr 1998 14:07:38   OS
   SwXTextTable hierher verschoben

      Rev 1.1   12 Mar 1998 16:44:06   TJ
   include

      Rev 1.0   10 Mar 1998 12:12:34   OS
   Initial revision.


------------------------------------------------------------------------*/

