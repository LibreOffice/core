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


#include <bf_svx/svdorect.hxx>

#include "schattr.hxx"
#include "memchrt.hxx"

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE	        0
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTTEXTORIENT	SCHATTR_TEXT_ORIENT


#endif

#ifndef _CHTMODEL_HXX
#endif
#include <bf_svx/svxids.hrc>
#include <math.h>
#include <float.h>
#include "glob.hrc"

#include "globfunc.hxx"
#include "pairs.hxx"

#ifndef _ZFORLIST_HXX
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <bf_svtools/zforlist.hxx>
#endif
namespace binfilter {


/*************************************************************************
|*
|* Chart-Objekt ermitteln;
|* Es koennen nur Objekte ermittelt werden, von denen es jeweils
|* EIN Exemplar gibt.
|*
\************************************************************************/


/*************************************************************************
|*
|* Chartdaten auslesen //Angepasst an Umsortierunng
|*
\************************************************************************/

/*N*/ double ChartModel::GetData(long nCol,long nRow,BOOL  bPercent,BOOL  bRowData) const
/*N*/ {
/*N*/ 	double fData = IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/ 					   ? pChartData->GetTransData(nRow, nCol)
/*N*/ 					   : pChartData->GetTransData(nCol, nRow);
/*N*/ 
/*N*/ 	if ((fData == DBL_MIN) || !bPercent) return fData;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		return IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/ 					? pChartData->GetTransDataInPercent(nRow,nCol,!bRowData)
/*N*/ 					: pChartData->GetTransDataInPercent(nCol,nRow,bRowData);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Datenpunkt-Objekt ermitteln
|*
\************************************************************************/

/*N*/ SdrObject* ChartModel::GetDataPointObj(long nCol, long nRow)
/*N*/ {
/*?*/ 	SdrPage* pPage = GetPage(0);
/*?*/ 	DBG_ASSERT(pPage, "ChartModel::GetDataPointObj:Keine Seite vorhanden!");
/*?*/ 
/*?*/ 	SdrObjGroup* pDiagram =
/*?*/ 		(SdrObjGroup*)GetObjWithId(CHOBJID_DIAGRAM, *pPage);
/*?*/ 	DBG_ASSERT(pDiagram, "ChartModel::GetDataPointObj:Kein Diagramm-Objekt vorhanden!");
/*?*/ 
/*?*/ 	SdrObject* pObj =
/*?*/ 		GetObjWithColRow(nCol, nRow, *pDiagram->GetSubList());
/*?*/ 
/*?*/ 	if (!pObj)
/*?*/ 	{
/*?*/ 		SdrObjGroup* pDataRow =
/*?*/ 			(SdrObjGroup*)GetObjWithRow(nRow, *pDiagram->GetSubList());
/*?*/ 
/*?*/ 		if(pDataRow)
/*?*/ 			pObj = GetObjWithColRow(nCol, nRow, *pDataRow->GetSubList());
/*?*/ 		else
/*?*/ 			DBG_TRACE("ChartModel::GetDataPointObj:Datenreihen-Objekt nicht gefunden!");
/*?*/ 	}
/*?*/ 
/*?*/ 	return pObj;
/*N*/ }
/*************************************************************************
|*
|* für SP2 #66110# bzw. #61907#
|*
\************************************************************************/
/*N*/ void ChartModel::SetChartDataBuffered(SchMemChart& rData, BOOL bNewTitles)
/*N*/ {
/*N*/ 	if( pChartDataBuffered )
/*N*/ 		delete pChartDataBuffered;
/*N*/ 
/*N*/ 	pChartDataBuffered = NULL;
/*N*/ 
/*N*/ 		SetChartData(*new SchMemChart(rData), bNewTitles);
/*N*/ 		BuildChart (TRUE);
/*N*/ }



/*************************************************************************
|*
|* neue Chart-Datenstruktur setzen; bisherige wird ggf. geloescht
|*
\************************************************************************/

/*N*/ void ChartModel::SetChartData(SchMemChart& rData, BOOL bNewTitles)
/*N*/ {
/*N*/ 	if (pChartData != &rData)
/*N*/ 	{
/*N*/ 		BOOL bWasData;
/*N*/ 		long nOldRowCount = 0;
/*N*/ 		long nOldColumnCount = 0;
/*N*/ 
/*N*/ 		if (pChartData)
/*N*/ 		{
/*N*/ 			nOldRowCount	= pChartData->GetRowCount();
/*N*/ 			nOldColumnCount	= pChartData->GetColCount();
/*N*/ 			if(pChartData->DecreaseRefCount())
/*N*/ 				delete pChartData;
/*N*/ 
/*N*/ 			bWasData = TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bWasData = FALSE;
/*N*/ 
/*N*/ 		pChartData  = &rData;
/*N*/ 		pChartData->IncreaseRefCount();
/*N*/ 		long nColCnt = GetColCount();
/*N*/ 		long nRowCnt = GetRowCount();
/*N*/ 
/*N*/         // #102853# the adaption of min/max should be (and is) handled by the
/*N*/         // axis themselves.  (It should at least check, if the min/max values
/*N*/         // are 'auto')
/*N*/ 
/*N*/ // 		for (long nCol = 0; nCol < nColCnt; nCol++)
/*N*/ // 		{
/*N*/ // 			for (long nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ // 			{
/*N*/ // 				double fData = GetData(nCol, nRow);
/*N*/ 
/*N*/ // 				if ((nCol == 0) && (nRow == 0))
/*N*/ // 				{
/*N*/ 					//ToDo: wozu das hier? klären!
/*N*/ //					pChartYAxis->SetMin(fData);
/*N*/ //					pChartYAxis->SetMax(fData);
/*N*/ // 				}
/*N*/ 
/*N*/ // 				if (fData < pChartYAxis->GetMin())
/*N*/ // 					pChartYAxis->SetMin(fData);
/*N*/ // 				if (fData > pChartYAxis->GetMax())
/*N*/ // 					pChartYAxis->SetMax(fData);
/*N*/ // 			}
/*N*/ // 		}
/*N*/ 
/*N*/ 		if (!bWasData || bNewTitles)
/*N*/ 		{
/*N*/ 			aMainTitle  = pChartData->GetMainTitle();
/*N*/ 			aSubTitle   = pChartData->GetSubTitle();
/*N*/ 			aXAxisTitle = pChartData->GetXAxisTitle();
/*N*/ 			aYAxisTitle = pChartData->GetYAxisTitle();
/*N*/ 			aZAxisTitle = pChartData->GetZAxisTitle();
/*N*/ 		}
/*N*/ 
/*N*/ 		InitDataAttrs();
/*N*/ 
/*N*/ 		if( rData.GetNumberFormatter() )
/*N*/ 		{
/*N*/ 			SvNumberFormatter* pNewFormatter = rData.GetNumberFormatter();
/*N*/ 			if( pNewFormatter != pNumFormatter )
/*N*/ 			{
/*N*/ 				// merge numberformatters:
/*N*/ 				// merge old one to new one and set new one as member
/*N*/ 				if( pNewFormatter && pNumFormatter )
/*N*/ 				{
/*N*/ 					SvNumberFormatterIndexTable* pTransTable =
/*N*/ 						pNewFormatter->MergeFormatter( *pNumFormatter );
/*N*/ 
/*N*/ 					SetNumberFormatter( pNewFormatter );
/*N*/ 
/*N*/ 					if( pTransTable && pTransTable->Count() )
/*N*/ 					{
/*N*/ 						TranslateAllNumFormatIds( pTransTable );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if( pNewFormatter )
/*N*/ 				{
/*?*/ 					SetNumberFormatter( pNewFormatter );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			CheckForNewAxisNumFormat();
/*N*/ 		}
/*N*/ 
/*N*/ 		if( (nOldRowCount != nRowCnt) ||
/*N*/ 			(nOldColumnCount != nColCnt ))
/*N*/ 		{
/*N*/ 			DataRangeChanged( nOldRowCount, nOldColumnCount );
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|* Erstelle Diagrammtitel
|*
\************************************************************************/

/*N*/ SdrTextObj *ChartModel::CreateTitle (SfxItemSet   *pTitleAttr,
/*N*/ 									 short        nID,
/*N*/ 									 BOOL         bSwitchColRow,
/*N*/ 									 const String &rText,
/*N*/ 									 BOOL         bVert,
/*N*/ 									 ChartAdjust  *pTextDirection)
/*N*/ {
/*N*/ 	DBG_ASSERT (pTextDirection, "ChartModel::CreateTitle:Titel-Erzeugung ohne Text-Ausrichtungsinformation");
/*N*/ 	if (pTextDirection == NULL) return NULL;  //FG: sonst Absturz
/*N*/ 
/*N*/ 	SfxItemSet aTextAttr(*pItemPool, nTitleWhichPairs);
/*N*/ 	SvxChartTextOrient eOrient = ((const SvxChartTextOrientItem&)pTitleAttr->Get(SCHATTR_TEXT_ORIENT)).GetValue();
/*N*/ 
/*N*/ 	if (bVert)
/*N*/ 	{
/*N*/ 		if (bSwitchColRow)
/*N*/ 		{
/*N*/ 			*pTextDirection = CHADJUST_BOTTOM_CENTER;
/*N*/ 			if (eOrient == CHTXTORIENT_AUTOMATIC) eOrient = CHTXTORIENT_STANDARD;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			*pTextDirection= CHADJUST_CENTER_LEFT;
/*N*/ 			if (eOrient == CHTXTORIENT_AUTOMATIC) eOrient = CHTXTORIENT_BOTTOMTOP;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		*pTextDirection = bSwitchColRow ? CHADJUST_CENTER_LEFT : CHADJUST_BOTTOM_CENTER;
/*N*/ 		if (eOrient == CHTXTORIENT_AUTOMATIC)
/*N*/ 		{
/*N*/ 			eOrient = bSwitchColRow ? CHTXTORIENT_BOTTOMTOP : CHTXTORIENT_STANDARD;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	aTextAttr.Put(*pTitleAttr);
/*N*/ 	aTextAttr.Put(SvxChartTextOrientItem(eOrient));
/*N*/ 
/*N*/ 	// Seit 4/1998 koennen Texte frei gedreht werden: SCHATTR_TEXT_DEGREES
/*N*/ 	// Hier wird ein Wert nachgetragen, falls CHTXTORIENT_AUTOMATIC
/*N*/ 	// im Attribut stehen sollte und noch kein Winkel gesetzt wurde
/*N*/ 	// ... bisher ohne auswirkung ...? ... evtl. an dieser Stelle unnoetig
/*N*/ 	GetTextRotation(aTextAttr,eOrient);
/*N*/ 
/*N*/ 	return CreateTextObj (nID, Point(), rText,
/*N*/ 						  aTextAttr, TRUE, *pTextDirection);
/*N*/ }
/*************************************************************************
|*
|* Extractor fuer SvStream zum Laden
|*
\************************************************************************/

/*N*/ BOOL ChartModel::ChangeStatistics (const SfxItemSet &rInAttrs)
/*N*/ {
/*N*/     long nRowCnt = IsPieChart()
/*N*/         ? GetColCount()
/*N*/         : GetRowCount();
/*N*/ 
/*N*/     for( long nRow = 0;
/*N*/          nRow < nRowCnt;
/*N*/          nRow++ )
/*N*/         PutDataRowAttr( nRow, rInAttrs );
/*N*/ 
/*N*/     const SfxPoolItem *pPoolItem = NULL;
/*N*/ 	BOOL              bChanged   = FALSE;
/*N*/ 
/*N*/ 	if (rInAttrs.GetItemState(SCHATTR_STAT_AVERAGE, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		bShowAverage = ((const SfxBoolItem*) pPoolItem)->GetValue();
/*N*/ 		bChanged     = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rInAttrs.GetItemState(SCHATTR_STAT_KIND_ERROR, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		eErrorKind = (SvxChartKindError) ((const SfxInt32Item*) pPoolItem)->GetValue();
/*N*/ 		bChanged   = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rInAttrs.GetItemState(SCHATTR_STAT_PERCENT, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		fIndicatePercent = ((const SvxDoubleItem*) pPoolItem)->GetValue();
/*N*/ 		bChanged         = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rInAttrs.GetItemState(SCHATTR_STAT_BIGERROR, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		fIndicateBigError = ((const SvxDoubleItem*) pPoolItem)->GetValue();
/*N*/ 		bChanged          = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rInAttrs.GetItemState(SCHATTR_STAT_CONSTPLUS, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		fIndicatePlus = ((const SvxDoubleItem*) pPoolItem)->GetValue();
/*N*/ 		bChanged      = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rInAttrs.GetItemState(SCHATTR_STAT_CONSTMINUS, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		fIndicateMinus = ((const SvxDoubleItem*) pPoolItem)->GetValue();
/*N*/ 		bChanged       = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rInAttrs.GetItemState(SCHATTR_STAT_INDICATE, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 	{
/*?*/ 		eIndicate = (SvxChartIndicate) ((const SfxInt32Item*) pPoolItem)->GetValue();
/*?*/ 		bChanged  = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rInAttrs.GetItemState(SCHATTR_STAT_REGRESSTYPE, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		eRegression = (SvxChartRegress) ((const SfxInt32Item*) pPoolItem)->GetValue();
/*N*/ 		bChanged    = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bChanged )
/*N*/     {
/*N*/         BuildChart( FALSE );
/*N*/     }
/*N*/ 
/*N*/     return bChanged;
/*N*/ }

/*N*/ void ChartModel::DataRangeChanged( long _nOldRowCnt , long _nOldColCnt )
/*N*/ {
/*N*/ 	if( Is3DChart() )
/*N*/ 	{
/*N*/ 		// delete depth-attributes for 3d charts
/*N*/ 		long i, nCount = aDataRowAttrList.Count();
/*N*/ 		SfxItemSet	*	pAttributes;
/*N*/ 
/*N*/ 		for( i=0; i<nCount; i++ )
/*N*/ //-/			aDataRowAttrList.GetObject( i )->ClearItem( SID_ATTR_3D_DEPTH );
/*N*/ 			aDataRowAttrList.GetObject( i )->ClearItem(SDRATTR_3DOBJ_DEPTH);
/*N*/ 
/*N*/ 		nCount = aDataPointAttrList.Count();
/*N*/ 		for( i=0; i < nCount; i++ )
/*N*/ 		{
/*N*/ 			pAttributes = aDataPointAttrList.GetObject( i );
/*N*/ 			if (pAttributes != NULL)
/*?*/ 				pAttributes->ClearItem(SDRATTR_3DOBJ_DEPTH);
/*N*/ 		}
/*N*/ 		
/*N*/ 		nCount = aSwitchDataPointAttrList.Count();
/*N*/ 		for( i=0; i < nCount; i++ )
/*N*/ 		{
/*N*/ 			pAttributes = aSwitchDataPointAttrList.GetObject( i );
/*N*/ 			if (pAttributes != NULL)
/*?*/ 				pAttributes->ClearItem(SDRATTR_3DOBJ_DEPTH);
/*N*/ 		}
/*N*/ 	}
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
