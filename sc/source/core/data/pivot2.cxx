/*************************************************************************
 *
 *  $RCSfile: pivot2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:15 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#pragma optimize("",off)
#pragma optimize("q",off) // p-code off

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/boxitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/algitem.hxx>
#include <tools/intn.hxx>

#include "globstr.hrc"
#include "subtotal.hxx"
#include "rangeutl.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "document.hxx"
#include "userlist.hxx"
#include "pivot.hxx"
#include "rechead.hxx"
#include "compiler.hxx"                         // fuer errNoValue
#include "refupdat.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"


// STATIC DATA -----------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Hilfsmethoden von ScPivot
//--------------------------------------------------------------------------------------------------

void ScPivot::SetFrame(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nWidth)
{
    if (pDoc->pTab[nDestTab])
    {
        SvxBorderLine aLine;
        aLine.SetOutWidth(nWidth);
        SvxBoxItem aBox;
        aBox.SetLine(&aLine, BOX_LINE_LEFT);
        aBox.SetLine(&aLine, BOX_LINE_TOP);
        aBox.SetLine(&aLine, BOX_LINE_RIGHT);
        aBox.SetLine(&aLine, BOX_LINE_BOTTOM);
        SvxBoxInfoItem aBoxInfo;
        aBoxInfo.SetValid(VALID_HORI,FALSE);
        aBoxInfo.SetValid(VALID_VERT,FALSE);
        aBoxInfo.SetValid(VALID_DISTANCE,FALSE);
        pDoc->pTab[nDestTab]->ApplyBlockFrame(&aBox, &aBoxInfo, nCol1, nRow1, nCol2, nRow2);
    }
}

void ScPivot::SetFrameHor(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2)
{
    if (pDoc->pTab[nDestTab])
    {
        SvxBorderLine aLine;
        aLine.SetOutWidth(20);
        SvxBoxItem aBox;
        aBox.SetLine(&aLine, BOX_LINE_LEFT);
        aBox.SetLine(&aLine, BOX_LINE_TOP);
        aBox.SetLine(&aLine, BOX_LINE_RIGHT);
        aBox.SetLine(&aLine, BOX_LINE_BOTTOM);
        SvxBoxInfoItem aBoxInfo;
        aBoxInfo.SetValid(VALID_VERT,FALSE);
        aBoxInfo.SetValid(VALID_DISTANCE,FALSE);
        aBoxInfo.SetLine(&aLine, BOXINFO_LINE_HORI);
        pDoc->pTab[nDestTab]->ApplyBlockFrame(&aBox, &aBoxInfo, nCol1, nRow1, nCol2, nRow2);
    }
}

void ScPivot::SetFrameVer(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2)
{
    if (pDoc->pTab[nDestTab])
    {
        SvxBorderLine aLine;
        aLine.SetOutWidth(20);
        SvxBoxItem aBox;
        aBox.SetLine(&aLine, BOX_LINE_LEFT);
        aBox.SetLine(&aLine, BOX_LINE_TOP);
        aBox.SetLine(&aLine, BOX_LINE_RIGHT);
        aBox.SetLine(&aLine, BOX_LINE_BOTTOM);
        SvxBoxInfoItem aBoxInfo;
        aBoxInfo.SetValid(VALID_HORI,FALSE);
        aBoxInfo.SetValid(VALID_DISTANCE,FALSE);
        aBoxInfo.SetLine(&aLine, BOXINFO_LINE_VERT);
        pDoc->pTab[nDestTab]->ApplyBlockFrame(&aBox, &aBoxInfo, nCol1, nRow1, nCol2, nRow2);
    }
}

void ScPivot::SetFontBold(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2)
{
    if (pDoc->pTab[nDestTab])
    {
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( SvxWeightItem( WEIGHT_BOLD ) );
        pDoc->pTab[nDestTab]->ApplyPatternArea(nCol1, nRow1, nCol2, nRow2, aPattern);
    }
}

void ScPivot::SetJustifyLeft(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2)
{
    if (pDoc->pTab[nDestTab])
    {
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT ) );
        pDoc->pTab[nDestTab]->ApplyPatternArea(nCol1, nRow1, nCol2, nRow2, aPattern);
    }
}

void ScPivot::SetJustifyRight(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2)
{
    if (pDoc->pTab[nDestTab])
    {
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT ) );
        pDoc->pTab[nDestTab]->ApplyPatternArea(nCol1, nRow1, nCol2, nRow2, aPattern);
    }
}

void ScPivot::SetButton(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2)
{
    if (pDoc->pTab[nDestTab])
    {
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( ScMergeFlagAttr(SC_MF_BUTTON) );
        pDoc->pTab[nDestTab]->ApplyPatternArea(nCol1, nRow1, nCol2, nRow2, aPattern);
    }
}

void ScPivot::SetStyle(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nId)
{
    if ( nCol1 > nCol2 || nRow1 > nRow2 )
        return;                                 //  Falls Bereiche leer sind

    USHORT nStringId = 0;
    switch (nId)
    {
        case PIVOT_STYLE_INNER:     nStringId = STR_PIVOT_STYLE_INNER;      break;
        case PIVOT_STYLE_RESULT:    nStringId = STR_PIVOT_STYLE_RESULT;     break;
        case PIVOT_STYLE_CATEGORY:  nStringId = STR_PIVOT_STYLE_CATEGORY;   break;
        case PIVOT_STYLE_TITLE:     nStringId = STR_PIVOT_STYLE_TITLE;      break;
        case PIVOT_STYLE_FIELDNAME: nStringId = STR_PIVOT_STYLE_FIELDNAME;  break;
        case PIVOT_STYLE_TOP:       nStringId = STR_PIVOT_STYLE_TOP;        break;
        default:
            DBG_ERROR("falsche ID bei ScPivot::SetStyle");
            return;
    }
    String aStyleName = ScGlobal::GetRscString(nStringId);

    ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
    ScStyleSheet* pStyle = (ScStyleSheet*) pStlPool->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
    if (!pStyle)
    {
        //  neu anlegen

        pStyle = (ScStyleSheet*) &pStlPool->Make( aStyleName, SFX_STYLE_FAMILY_PARA,
                                                    SFXSTYLEBIT_USERDEF );
        pStyle->SetParent( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
        SfxItemSet& rSet = pStyle->GetItemSet();
        if ( nId==PIVOT_STYLE_RESULT || nId==PIVOT_STYLE_TITLE )
            rSet.Put( SvxWeightItem( WEIGHT_BOLD ) );
        if ( nId==PIVOT_STYLE_CATEGORY || nId==PIVOT_STYLE_TITLE )
            rSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT ) );
    }

    pDoc->pTab[nDestTab]->ApplyStyleArea( nCol1, nRow1, nCol2, nRow2, *pStyle );
}

void ScPivot::SetValue(USHORT nCol, USHORT nRow, const SubTotal& rTotal, USHORT nFunc)
{
    if ( rTotal.Valid( nFunc ) == 1)
        pDoc->SetValue(nCol, nRow, nDestTab, rTotal.Result( nFunc ));
    else if ( rTotal.Valid( nFunc ) == 0)
        pDoc->SetError(nCol, nRow, nDestTab, errNoValue);
}

//--------------------------------------------------------------------------------------------------

void ScPivot::GetParam( ScPivotParam& rParam, ScQueryParam& rQuery, ScArea& rSrcArea ) const
{
    short nCount;
    USHORT nDummy;
    GetDestArea( rParam.nCol,rParam.nRow, nDummy,nDummy, rParam.nTab );

    // Row und Col in der Bedeutung vertauscht:
    GetRowFields( rParam.aColArr, nCount );
    rParam.nColCount = (USHORT) nCount;
    GetColFields( rParam.aRowArr, nCount );
    rParam.nRowCount = (USHORT) nCount;
    GetDataFields( rParam.aDataArr, nCount );
    rParam.nDataCount = (USHORT) nCount;

    rParam.bIgnoreEmptyRows  = GetIgnoreEmpty();
    rParam.bDetectCategories = GetDetectCat();
    rParam.bMakeTotalCol     = GetMakeTotalCol();
    rParam.bMakeTotalRow     = GetMakeTotalRow();

    GetQuery(rQuery);
    GetSrcArea( rSrcArea.nColStart, rSrcArea.nRowStart,
                        rSrcArea.nColEnd, rSrcArea.nRowEnd, rSrcArea.nTab );
}

void ScPivot::SetParam( const ScPivotParam& rParam, const ScQueryParam& rQuery,
                            const ScArea& rSrcArea )
{
    SetQuery( rQuery );
    SetHeader( TRUE );
    SetSrcArea( rSrcArea.nColStart, rSrcArea.nRowStart,
                          rSrcArea.nColEnd, rSrcArea.nRowEnd, rSrcArea.nTab );
    SetDestPos( rParam.nCol, rParam.nRow, rParam.nTab );
    SetIgnoreEmpty( rParam.bIgnoreEmptyRows );
    SetDetectCat( rParam.bDetectCategories );
    SetMakeTotalCol( rParam.bMakeTotalCol );
    SetMakeTotalRow( rParam.bMakeTotalRow );

    // Row und Col in der Bedeutung vertauscht:
    SetRowFields( rParam.aColArr, rParam.nColCount );
    SetColFields( rParam.aRowArr, rParam.nRowCount );
    SetDataFields( rParam.aDataArr, rParam.nDataCount );
}

DataObject* ScPivot::Clone() const
{
    return new ScPivot(*this);
}

//--------------------------------------------------------------------------------------------------
// PivotStrCollection
//--------------------------------------------------------------------------------------------------

DataObject* PivotStrCollection::Clone() const
{
    return new PivotStrCollection(*this);
}

short PivotStrCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
{
    DBG_ASSERT(pKey1&&pKey2,"0-Zeiger bei PivotStrCollection::Compare");

    short nResult = 0;

    TypedStrData& rData1 = (TypedStrData&)*pKey1;
    TypedStrData& rData2 = (TypedStrData&)*pKey2;

    if ( rData1.nStrType > rData2.nStrType )
        nResult = 1;
    else if ( rData1.nStrType < rData2.nStrType )
        nResult = -1;
    else if ( !rData1.nStrType /* && !rData2.nStrType */ )
    {
        // Zahlen vergleichen:

        if ( rData1.nValue == rData2.nValue )
            nResult = 0;
        else if ( rData1.nValue < rData2.nValue )
            nResult = -1;
        else
            nResult = 1;
    }
    else /* if ( rData1.nStrType && rData2.nStrType ) */
    {
        // Strings vergleichen:

        StringCompare eComp;
        if (pUserData)
            eComp = pUserData->ICompare(rData1.aStrValue, rData2.aStrValue);
        else
        {
            eComp = ScGlobal::pScInternational->Compare(
                rData1.aStrValue, rData2.aStrValue, INTN_COMPARE_IGNORECASE );
        }

        if ( eComp == COMPARE_EQUAL )
            nResult = 0;
        else if ( eComp == COMPARE_LESS )
            nResult = -1;
        else
            nResult = 1;
    }

    return nResult;
}

short PivotStrCollection::GetIndex(TypedStrData* pData) const
{
    USHORT nIndex = 0;
    if (!Search(pData, nIndex))
        nIndex = 0;
    return (short)nIndex;
}

//--------------------------------------------------------------------------------------------------
// PivotCollection
//--------------------------------------------------------------------------------------------------

String ScPivotCollection::CreateNewName( USHORT nMin ) const
{
    String aBase = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("DataPilot"));
    //! from Resource?

    for (USHORT nAdd=0; nAdd<=nCount; nAdd++)   //  nCount+1 Versuche
    {
        String aNewName = aBase;
        aNewName += String::CreateFromInt32( nMin + nAdd );
        BOOL bFound = FALSE;
        for (USHORT i=0; i<nCount && !bFound; i++)
            if (((ScPivot*)pItems[i])->GetName() == aNewName)
                bFound = TRUE;
        if (!bFound)
            return aNewName;            // freien Namen gefunden
    }
    return String();                    // sollte nicht vorkommen
}

ScPivot* ScPivotCollection::GetPivotAtCursor(USHORT nCol, USHORT nRow, USHORT nTab) const
{
    if (pItems)
    {
        for (USHORT i = 0; i < nCount; i++)
            if (((ScPivot*)pItems[i])->IsPivotAtCursor(nCol, nRow, nTab))
            {
                return (ScPivot*)pItems[i];
            }
    }
    return NULL;
}

BOOL ScPivotCollection::Load(SvStream& rStream)
{
    BOOL bSuccess = TRUE;
    USHORT nNewCount, i;
    FreeAll();

    ScMultipleReadHeader aHdr( rStream );

    rStream >> nNewCount;
    for (i=0; i<nNewCount && bSuccess; i++)
    {
        ScPivot* pPivot = new ScPivot( pDoc );
        if (pPivot)
        {
            bSuccess = pPivot->Load(rStream, aHdr);
            Insert( pPivot );
        }
        else
            bSuccess = FALSE;
    }

    //  fuer alte Dateien: eindeutige Namen vergeben

    if (bSuccess)
        for (i=0; i<nCount; i++)
            if (!((const ScPivot*)At(i))->GetName().Len())
                ((ScPivot*)At(i))->SetName( CreateNewName() );

    return bSuccess;
}

BOOL ScPivotCollection::Store(SvStream& rStream) const
{
    BOOL bSuccess = TRUE;

    ScMultipleWriteHeader aHdr( rStream );

    rStream << nCount;

    for (USHORT i=0; i<nCount && bSuccess; i++)
        bSuccess = ((const ScPivot*)At(i))->Store( rStream, aHdr );

    return bSuccess;
}

void ScPivotCollection::UpdateReference(UpdateRefMode eUpdateRefMode,
                                USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                short nDx, short nDy, short nDz )
{
    for (USHORT i=0; i<nCount; i++)
    {
        USHORT theCol1;
        USHORT theRow1;
        USHORT theTab1;
        USHORT theCol2;
        USHORT theRow2;
        USHORT theTab2;
        ScRefUpdateRes eRes;
        ScPivot* pPivot = (ScPivot*)pItems[i];

        //  Source

        pPivot->GetSrcArea( theCol1, theRow1, theCol2, theRow2, theTab1 );
        theTab2 = theTab1;

        eRes = ScRefUpdate::Update( pDoc, eUpdateRefMode,
                                                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                                theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );

        if (eRes != UR_NOTHING)
            pPivot->MoveSrcArea( theCol1, theRow1, theTab1 );

        //  Dest

        pPivot->GetDestArea( theCol1, theRow1, theCol2, theRow2, theTab1 );
        theTab2 = theTab1;

        eRes = ScRefUpdate::Update( pDoc, eUpdateRefMode,
                                                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                                theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );

        if (eRes != UR_NOTHING)
            pPivot->MoveDestArea( theCol1, theRow1, theTab1 );
    }
}

void ScPivotCollection::UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY )
{
    //  nur Quell-Bereich

    for (USHORT i=0; i<nCount; i++)
    {
        ScPivot* pPivot = (ScPivot*)pItems[i];
        ScRange aSrc = pPivot->GetSrcArea();
        ScRefUpdateRes eRes = ScRefUpdate::DoGrow( rArea, nGrowX, nGrowY, aSrc );
        if (eRes != UR_NOTHING)
            pPivot->ExtendSrcArea( aSrc.aEnd.Col(), aSrc.aEnd.Row() );
    }
}

BOOL ScPivotCollection::operator==(const ScPivotCollection& rCmp) const
{
    if (nCount != rCmp.nCount)
        return FALSE;

    if (!nCount)
        return TRUE;            // beide leer - nicht erst die Param's anlegen!

    ScPivotParam aMyParam, aCmpParam;
    ScQueryParam aMyQuery, aCmpQuery;
    ScArea aMyArea, aCmpArea;

    for (USHORT i=0; i<nCount; i++)
    {
        ScPivot* pMyPivot = (ScPivot*)pItems[i];
        pMyPivot->GetParam( aMyParam, aMyQuery, aMyArea );
        ScPivot* pCmpPivot = (ScPivot*)rCmp.pItems[i];
        pCmpPivot->GetParam( aCmpParam, aCmpQuery, aCmpArea );
        if (!( aMyArea==aCmpArea && aMyParam==aCmpParam && aMyQuery==aCmpQuery ))
            return FALSE;
    }

    return TRUE;
}

DataObject* ScPivotCollection::Clone() const
{
    return new ScPivotCollection(*this);
}




