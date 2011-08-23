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
#pragma optimize("",off)

#pragma hdrstop
#endif
#ifndef _SVDORECT_HXX //autogen
#include <bf_svx/svdorect.hxx>
#endif

#ifndef _SVDOPATH_HXX //autogen
#include <bf_svx/svdopath.hxx>
#endif


#ifndef _SVX_XLNCLIT_HXX //autogen
#include <bf_svx/xlnclit.hxx>
#endif

#ifndef _ZFORLIST_HXX //autogen
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#endif


#include <list>

#ifndef _EEITEM_HXX //autogen
#include <bf_svx/eeitem.hxx>
#endif

#ifndef _SCHATTR_HXX
#include "schattr.hxx"
#endif
#ifndef _SCH_MEMCHRT_HXX
#include "memchrt.hxx"
#endif

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_CHARTDATADESCR	SCHATTR_DATADESCR_DESCR
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTTEXTORIENT	SCHATTR_TEXT_ORIENT


#endif

#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#define ITEMID_FONTWIDTH   EE_CHAR_FONTWIDTH
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/fwdtitem.hxx>

// header for class SdrOutliner
// header for GetDraftFillColor()
#ifndef _SVDETC_HXX
#include <bf_svx/svdetc.hxx>
#endif

#ifndef _CHTMODEL_HXX
#include "globfunc.hxx"
#endif

#include "ChXChartDocument.hxx"

#include <float.h>
#include "glob.hrc"

#include "chaxis.hxx"
#include "chdescr.hxx"
#include "calculat.hxx"
namespace binfilter {

/*************************************************************************
|*
|* Koordinatenachsen und Rueckwand mit Unterteilung und Beschriftung
|* erzeugen; Liefert die Einfuege-Position fuer die Chart-Datenobjekte.
|*
\************************************************************************/
/*N*/ void ChartModel::Create2DXYTitles(Rectangle& rRect,BOOL bSwitchColRow)
/*N*/ {
/*N*/ 	SdrPage* pPage=GetPage(0);
/*N*/ 
/*N*/ 	SdrTextObj         *pXAxisTitleObj = NULL;
/*N*/ 	SdrTextObj         *pYAxisTitleObj = NULL;
/*N*/ 
/*N*/ 	if (bShowXAxisTitle)
/*N*/ 	{
/*N*/ 		pXAxisTitleObj = CreateTitle (pXAxisTitleAttr, CHOBJID_DIAGRAM_TITLE_X_AXIS,
/*N*/ 									  bSwitchColRow,aXAxisTitle, FALSE, &eAdjustXAxesTitle);
/*N*/ 
/*N*/ 		if (GetAdjustMarginsForXAxisTitle())
/*N*/ 		{
/*N*/ 			if (bSwitchColRow)
/*N*/ 				rRect.Left() += GetOutputSize(*pXAxisTitleObj).Width() + 200;
/*N*/ 			else
/*N*/ 				rRect.Bottom() -= GetOutputSize(*pXAxisTitleObj).Height() + 200;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (bShowYAxisTitle)
/*N*/ 	{
/*N*/ 		pYAxisTitleObj = CreateTitle (pYAxisTitleAttr, CHOBJID_DIAGRAM_TITLE_Y_AXIS,
/*N*/ 			bSwitchColRow,aYAxisTitle, TRUE, &eAdjustYAxesTitle);
/*N*/ 		if (GetAdjustMarginsForYAxisTitle())
/*N*/ 		{
/*N*/ 			if (bSwitchColRow)
/*N*/ 				rRect.Bottom() -= GetOutputSize(*pYAxisTitleObj).Height() + 200;
/*N*/ 			else
/*N*/ 				rRect.Left() += GetOutputSize(*pYAxisTitleObj).Width() + 200;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (pXAxisTitleObj)
/*N*/ 		pPage->NbcInsertObject(pXAxisTitleObj);
/*N*/ 	if (pYAxisTitleObj)
/*N*/ 		pPage->NbcInsertObject(pYAxisTitleObj);
/*N*/ }


/*N*/ BOOL ChartModel::CanAxis(long nAxisId) const
/*N*/ {
/*N*/ 	switch(nAxisId)
/*N*/ 	{
/*N*/ 	   case CHART_AXIS_PRIMARY_X:
/*?*/ 		   return ( ! ( IsPieChart() || IsDonutChart() || IsNetChart() ));
/*N*/ 	   case CHART_AXIS_PRIMARY_Y:
/*?*/ 		   return ( ! ( IsPieChart() || IsDonutChart() ));
/*N*/ 	   case CHART_AXIS_SECONDARY_X:
/*N*/ 	   case CHART_AXIS_SECONDARY_Y:
/*N*/ 		   switch(eChartStyle)
/*N*/ 		   {
/*N*/ 				case CHSTYLE_2D_LINE:
/*N*/ 				case CHSTYLE_2D_STACKEDLINE:
/*N*/ 				case CHSTYLE_2D_PERCENTLINE:
/*N*/ 				case CHSTYLE_2D_COLUMN:
/*N*/ 				case CHSTYLE_2D_STACKEDCOLUMN:
/*N*/ 				case CHSTYLE_2D_PERCENTCOLUMN:
/*N*/ 				case CHSTYLE_2D_BAR:
/*N*/ 				case CHSTYLE_2D_STACKEDBAR:
/*N*/ 				case CHSTYLE_2D_PERCENTBAR:
/*N*/ 				case CHSTYLE_2D_AREA:
/*N*/ 				case CHSTYLE_2D_STACKEDAREA:
/*N*/ 				case CHSTYLE_2D_PERCENTAREA:
/*				case CHSTYLE_2D_PIE:
                case CHSTYLE_3D_STRIPE:
                case CHSTYLE_3D_COLUMN:
                case CHSTYLE_3D_FLATCOLUMN:
                 case CHSTYLE_3D_STACKEDFLATCOLUMN:
                case CHSTYLE_3D_PERCENTFLATCOLUMN:
                case CHSTYLE_3D_AREA:
                case CHSTYLE_3D_STACKEDAREA:
                case CHSTYLE_3D_PERCENTAREA:
                case CHSTYLE_3D_SURFACE:
                case CHSTYLE_3D_PIE:
*/				case CHSTYLE_2D_XY:
/*N*/ //				case CHSTYLE_3D_XYZ:
/*N*/ 				case CHSTYLE_2D_LINESYMBOLS:
/*N*/ 				case CHSTYLE_2D_STACKEDLINESYM:
/*N*/ 				case CHSTYLE_2D_PERCENTLINESYM:
/*N*/ 				case CHSTYLE_2D_XYSYMBOLS:
/*N*/ 				case CHSTYLE_3D_XYZSYMBOLS:
/*N*/ //				case CHSTYLE_2D_DONUT1:
/*N*/ //				case CHSTYLE_2D_DONUT2:
/*N*/ 				case CHSTYLE_3D_BAR:
/*N*/ 				case CHSTYLE_3D_FLATBAR:
/*N*/ 				case CHSTYLE_3D_STACKEDFLATBAR:
/*N*/ 				case CHSTYLE_3D_PERCENTFLATBAR:
/*				case CHSTYLE_2D_PIE_SEGOF1:
                case CHSTYLE_2D_PIE_SEGOFALL:
                case CHSTYLE_2D_NET:
                case CHSTYLE_2D_NET_SYMBOLS:
                case CHSTYLE_2D_NET_STACK:
                case CHSTYLE_2D_NET_SYMBOLS_STACK:
                case CHSTYLE_2D_NET_PERCENT:
                case CHSTYLE_2D_NET_SYMBOLS_PERCENT:
*/				case CHSTYLE_2D_CUBIC_SPLINE:
/*N*/ 				case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL:
/*N*/ 				case CHSTYLE_2D_LINE_STACKEDCOLUMN:
/*N*/ 				case CHSTYLE_2D_LINE_COLUMN:
/*N*/ 				case CHSTYLE_2D_B_SPLINE:
/*N*/ 				case CHSTYLE_2D_B_SPLINE_SYMBOL:
/*N*/ 				case CHSTYLE_2D_CUBIC_SPLINE_XY:
/*N*/ 				case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY:
/*N*/ 				case CHSTYLE_2D_B_SPLINE_XY:
/*N*/ 				case CHSTYLE_2D_B_SPLINE_SYMBOL_XY:
/*N*/ 				case CHSTYLE_2D_XY_LINE:
/*N*/ 				case CHSTYLE_2D_STOCK_1:
/*N*/ 				case CHSTYLE_2D_STOCK_2:
/*N*/ 				case CHSTYLE_2D_STOCK_3:
/*N*/ 				case CHSTYLE_2D_STOCK_4:
/*N*/ 			    case CHSTYLE_ADDIN:
/*N*/ 				   return TRUE;
/*N*/ 			   default:
/*N*/ 				   return FALSE;
/*N*/ 		   }
/*N*/ 		   break;
/*N*/ 
/*N*/ 
/*N*/ 	   default:
/*?*/ 			return FALSE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }
/*N*/ BOOL ChartModel::HasSecondYAxis() const
/*N*/ {
/*N*/ 	switch(eChartStyle)
/*N*/ 	{
/*N*/ 		case CHSTYLE_2D_STOCK_3:
/*N*/ 		case CHSTYLE_2D_STOCK_4:
/*N*/ 			return TRUE;
/*N*/ 	}
/*N*/ 	if(!CanAxis(CHART_AXIS_SECONDARY_Y))
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	if(pChartBAxis->IsVisible())
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 
/*N*/ 	const SfxPoolItem *pPoolItem = NULL;
/*N*/ 
/*N*/ 	for(long n=0;n<GetRowCount();n++)
/*N*/ 		if(GetDataRowAttr(n).GetItemState(SCHATTR_AXIS,TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 			if(((const SfxInt32Item*)pPoolItem)->GetValue()==CHART_AXIS_SECONDARY_Y)
/*N*/ 				return TRUE;
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }
/*N*/ void ChartModel::Create2DBackplane(Rectangle &rRect,SdrObjList &rObjList,BOOL bPartDescr,USHORT eStackMode)
/*N*/ {
/*N*/ 	BOOL bPercent=IsPercent();
/*N*/ 	BOOL bSwitchColRow=IsXVertikal();//IsBar()
/*N*/ 
/*N*/ 	//Initialisieren:
/*N*/ 	pChartXAxis->Initialise(rRect,bSwitchColRow,eStackMode,bPercent,FALSE);
/*N*/ 	pChartYAxis->Initialise(rRect,bSwitchColRow,eStackMode,bPercent,FALSE);
/*N*/ 	pChartBAxis->Initialise(rRect,bSwitchColRow,eStackMode,bPercent,FALSE);
/*N*/ 	pChartAAxis->Initialise(rRect,bSwitchColRow,eStackMode,bPercent,FALSE);
/*N*/ 
/*N*/ 	//Position setzen (kann man eigentlich auch im ctor des Model machen?)
/*N*/ 	pChartAAxis->SetPosition(CHAXIS_POS_B); //A,B sind sekundäre Achsen => oben und rechts
/*N*/ 	pChartBAxis->SetPosition(CHAXIS_POS_B);
/*N*/ 	pChartXAxis->SetPosition(CHAXIS_POS_A); //primäre Positionen=links, unten
/*N*/ 	pChartYAxis->SetPosition(CHAXIS_POS_A);
/*N*/ 
/*N*/ 	pChartXAxis->CalcValueSteps();
/*N*/ 	pChartYAxis->CalcValueSteps();//#63904# 12%
/*N*/ 	pChartAAxis->CalcValueSteps();
/*N*/ 	pChartBAxis->CalcValueSteps();//#63904# 11%
/*N*/ 
/*N*/ 	pChartBAxis->AttachIfNoOwnData(pChartYAxis); //skalierung transferieren, falls Y keine automatische Skalierung benutzt und B keine Daten besitzt!
/*N*/ 	pChartYAxis->AttachIfNoOwnData(pChartBAxis); // und umgekehrt
/*N*/ 
/*N*/ 	//Falls  einer der beiden Y-Daten keine Reihen zugeordnet sind, soll diese die andere kopieren
/*N*/ 
/*N*/ 	long nTitleBottom	= rRect.Bottom();
/*N*/ 	long nTitleLeft 	= rRect.Left();
/*N*/ 
/*N*/ // ******* RECT_CHANGES **** Ab hier wird am rRect rumgeschraubt ******************
/*N*/ 
/*N*/ 	Create2DXYTitles(rRect,bSwitchColRow);	//abziehen der Achsen-Titel-Fläche
/*N*/ 
/*N*/ 	// FG: Ich merke mir vor jeglicher Aenderung des linken oder rechten Randes wo der Seitenrand
/*N*/ 	//     gewesen ist. Das braucht man um nun zu entscheiden ob der linke Rand wegen eines
/*N*/ 	//     überstehenden Textes nachgeregelt werden soll.
/*N*/ 	const Rectangle aOldRect(rRect);
/*N*/ 
/*N*/ 	//Zusammenfassen ???
/*N*/ 	pChartYAxis->CalcMaxTextSize();//#63904# 14%, aber Aufruf nur wenn nötig
/*N*/ 	pChartYAxis->InitDescr();
/*N*/ 	pChartYAxis->SubtractDescrSize(rRect);//Größe der Beschriftung vom Rechteck abziehen
/*N*/ 	pChartBAxis->CalcMaxTextSize();//#63904# s.o. 0% wenn nicht benutzt!
/*N*/ 	pChartBAxis->InitDescr();
/*N*/ 	pChartBAxis->SubtractDescrSize(rRect);
/*N*/ 
/*N*/ 	pChartXAxis->SetArea(rRect); //Die X-Achsenlänge ist bereits bekannt! wichtig für nDescrWidth
/*N*/ 	pChartAAxis->SetArea(rRect); //Die Y-Längen werden nachfolgend korrigiert
/*N*/ 
/*N*/ 	pChartAAxis->SetColTextMode(!IsXYChart(),!bPartDescr);
/*N*/ 	pChartXAxis->SetColTextMode(!IsXYChart(),!bPartDescr);
/*N*/ 
/*N*/ 	//für die X-Achse gibt es ein paar sonderregel, insbesondere wird
/*N*/ 	//das Rect auch in der X-Breite geändert, wenn die Spaltenunterschriften sehr lang sind
/*N*/ 	//hinzu kommt noch Umbruch (an Y-Achse nicht vorgesehen), daher mu?hier eine
/*N*/ 	//Sonderbehandlung erfolgen, auch das InitDescr erfolgt hier nachträglich!
/*N*/ 	pChartXAxis->SubtractDescrSize_X(rRect,aOldRect);
/*N*/ 
/*N*/ 	pChartAAxis->SetArea(rRect);
/*N*/ 
/*N*/ 	pChartAAxis->SubtractDescrSize_X(rRect,aOldRect); //noch falsch!
/*N*/ 
/*N*/ 	pChartXAxis->SetArea(rRect); //jetzt sind x und y länge bekannt
/*N*/ 	pChartYAxis->SetArea(rRect);
/*N*/ 	pChartAAxis->SetArea(rRect);
/*N*/ 	pChartBAxis->SetArea(rRect);
/*N*/ 
/*N*/ // ******* END RECT_CHANGES ********************************************************************
/*N*/ 
/*N*/ 	rRect.Justify();
/*N*/ 
/*N*/ 	pChartXAxis->SetArea(rRect); //jetzt sind x und y länge bekannt
/*N*/ 	pChartYAxis->SetArea(rRect);
/*N*/ 	pChartAAxis->SetArea(rRect);
/*N*/ 	pChartBAxis->SetArea(rRect);
/*N*/ 
/*N*/ 	long  nStepPartWidth=pChartXAxis->GetDescrWidth(); //Warum nochmal? sollte noch immer identisch nDescrWidth sein!
/*N*/ 
/*N*/ 	Position2DAxisTitles(rRect,bSwitchColRow,nTitleLeft,nTitleBottom);
/*N*/ 
/*N*/ 	// Diagrammwand
/*N*/ 	SdrRectObj* pWallObj = new SdrRectObj( rRect );
/*N*/ 	pWallObj->SetModel( this );
/*N*/ 	rObjList.NbcInsertObject( SetObjectAttr( pWallObj, CHOBJID_DIAGRAM_WALL,
/*N*/ 											 TRUE, TRUE, pDiagramWallAttr));
/*N*/ 
/*N*/ 	//********* Erzeugung der Achsen und Gitter *****************************
/*N*/ 
/*N*/ 	SdrObjList* pXGridMainList = NULL;
/*N*/ 	SdrObjList* pYGridMainList = NULL;
/*N*/ 	SdrObjList* pXGridHelpList = NULL;
/*N*/ 	SdrObjList* pYGridHelpList = NULL;
/*N*/ 	SdrObjList* pBAxisList	   = NULL;
/*N*/ 
/*N*/ 
/*N*/ 	BOOL bXAxis = (pChartXAxis->IsVisible() && pChartYAxis->IsOriginInRange());
/*N*/ 	BOOL bYAxis = (pChartYAxis->IsVisible() &&
/*N*/ 		(!IsXYChart() || (IsXYChart() && pChartXAxis->IsOriginInRange())));
/*N*/ 
/*N*/ 	if(bShowXGridMain)
/*N*/ 		pXGridMainList = CreateGroup (rObjList, CHOBJID_DIAGRAM_X_GRID_MAIN_GROUP);
/*N*/ 	if(bShowYGridMain)
/*N*/ 		pYGridMainList = CreateGroup (rObjList, CHOBJID_DIAGRAM_Y_GRID_MAIN_GROUP);
/*N*/ 	if(bShowXGridHelp)
/*?*/ 		pXGridHelpList = CreateGroup (rObjList, CHOBJID_DIAGRAM_X_GRID_HELP_GROUP);
/*N*/ 	if(bShowYGridHelp)
/*?*/ 		pYGridHelpList = CreateGroup (rObjList, CHOBJID_DIAGRAM_Y_GRID_HELP_GROUP);
/*N*/ 
/*N*/ 	pChartXAxis->CreateAxis( rObjList, CHOBJID_DIAGRAM_X_AXIS );
/*N*/ 	pChartYAxis->CreateAxis( rObjList, CHOBJID_DIAGRAM_Y_AXIS );
/*N*/ 
/*N*/ 	pChartXAxis->ShowAxis(bXAxis);
/*N*/ 	pChartYAxis->ShowAxis(bYAxis);
/*N*/ 
/*N*/ 	pChartXAxis->SetMainGrid(pYGridMainList,pYGridMainAttr);
/*N*/ 	pChartXAxis->SetHelpGrid(pYGridHelpList,pYGridHelpAttr);
/*N*/ 
/*N*/ 	pChartYAxis->SetHelpGrid(pXGridHelpList,pXGridHelpAttr);
/*N*/ 	pChartYAxis->SetMainGrid(pXGridMainList,pXGridMainAttr);
/*N*/ 
/*N*/ 	pChartAAxis->CreateAxis(rObjList,CHOBJID_DIAGRAM_A_AXIS);
/*N*/ 	pChartBAxis->CreateAxis(rObjList,CHOBJID_DIAGRAM_B_AXIS);
/*N*/ 
/*N*/ 	//sek. Achsen haben nie ein Gitter
/*N*/ 	pChartAAxis->SetHelpGrid(NULL,NULL);
/*N*/ 	pChartAAxis->SetMainGrid(NULL,NULL);
/*N*/ 	pChartBAxis->SetHelpGrid(NULL,NULL);
/*N*/ 	pChartBAxis->SetMainGrid(NULL,NULL);
/*N*/ 
/*N*/ 	pChartXAxis->CreateAxis(pChartYAxis->GetPosOrigin(),pChartXAxis->HasDescription(),bXAxis);
/*N*/ 
/*N*/ 	if (IsXYChart())
/*N*/ 	{
/*N*/ 		pChartYAxis->CreateAxis(pChartXAxis->GetPosOrigin(),pChartYAxis->HasDescription(),bYAxis);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if(bYAxis)
/*N*/ 			pChartYAxis->CreateAxis();
/*N*/ 	}
/*N*/ 
/*N*/ 	pChartYAxis->DrawGrids();
/*N*/ 	pChartBAxis->DrawGrids();//eigentlich kein Grid, nur die Ticks!
/*N*/ 
/*N*/ 	if(IsXYChart())
/*N*/ 	{	//Achtung! Die X-Achse unterstützt Umbruch (nColumnTextWidth)!
/*N*/ 		pChartXAxis->InitDescr();
/*N*/ 		pChartAAxis->InitDescr();
/*N*/ 	}
/*N*/ 	else //Hier wird der ColText genutzt -> Beschränkung auf maMaxTextSize!
/*N*/ 	{
/*N*/ 		pChartXAxis->InitDescr_X();
/*N*/ 		pChartAAxis->InitDescr_X();
/*N*/ 	}
/*N*/ 	pChartAAxis->DrawGrids();
/*N*/ 	pChartXAxis->DrawGrids();
/*N*/ }
/*N*/ void ChartModel::Position2DAxisTitles(const Rectangle& rRect,BOOL bSwitchColRow,long nTitleLeft,long nTitleBottom)
/*N*/ {
/*N*/ 	SdrPage *pPage = GetPage(0);
/*N*/ 	Size aPageSize = pPage->GetSize();
/*N*/ 
/*N*/ 	if(bShowXAxisTitle)
/*N*/ 	{
/*N*/ 		SdrObject *pXAxisTitleObj = GetObjWithId(CHOBJID_DIAGRAM_TITLE_X_AXIS,*pPage);
/*N*/ 
/*N*/ 		if (pXAxisTitleObj)
/*N*/ 		{
/*N*/ 
/*N*/ 
/*N*/ 			Point aXAxesTitlePosition (rRect.Left() + (int) (rRect.GetWidth() / 2),nTitleBottom);
/*N*/ 
/*N*/ 			if (GetXAxisTitleHasBeenMoved() && GetUseRelativePositions() &&
/*N*/ 				(aXAxesTitlePosition.X() > 0) && (aXAxesTitlePosition.Y() > 0))
/*N*/ 			{
/*N*/ 				// FG: Das ist eine Variable die in BuildChart gesetzt wird, kurz bevor
/*N*/ 				//     das Objekt zerstoert wird.
/*N*/ 				double fRelativeXPosition = ((double) aTitleXAxisPosition.X()) / aInitialSize.Width();
/*N*/ 				double fRelativeYPosition = ((double) aTitleXAxisPosition.Y()) / aInitialSize.Height();
/*N*/ 				aXAxesTitlePosition.X() = (int) (aPageSize.Width() * fRelativeXPosition + 0.5);
/*N*/ 				aXAxesTitlePosition.Y() = (int) (aPageSize.Height() * fRelativeYPosition + 0.5);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if (bSwitchColRow)
/*N*/ 				{
/*N*/ 					aXAxesTitlePosition.X() = nTitleLeft;
/*N*/ 					aXAxesTitlePosition.Y() = (long)(rRect.Top() + rRect.GetHeight() / 2);
/*N*/ 				}
/*N*/ 			}
/*N*/ 			SetTextPos((SdrTextObj &) *pXAxisTitleObj, aXAxesTitlePosition,pXAxisTitleAttr);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(bShowYAxisTitle)
/*N*/ 	{
/*N*/ 		SdrObject *pYAxisTitleObj = GetObjWithId(CHOBJID_DIAGRAM_TITLE_Y_AXIS,*pPage);
/*N*/ 
/*N*/ 		if (pYAxisTitleObj)
/*N*/ 		{
/*N*/ 			Point aYAxesTitlePosition (Max(0L,nTitleLeft),
/*N*/ 				Max(0L,(long) (rRect.Top() + (int) (rRect.GetHeight() / 2))));
/*N*/ 
/*N*/ 			if (GetYAxisTitleHasBeenMoved() && GetUseRelativePositions() &&
/*N*/ 				(aYAxesTitlePosition.X() >= 0) && (aYAxesTitlePosition.Y() >= 0))
/*N*/ 			{
/*N*/ 				// FG: Das ist eine Variable die in BuildChart gesetzt wird, kurz bevor
/*N*/ 				//     das Objekt zerstoert wird.
/*N*/ 				double fRelativeXPosition = ((double) aTitleYAxisPosition.X()) / aInitialSize.Width();
/*N*/ 				double fRelativeYPosition = ((double) aTitleYAxisPosition.Y()) / aInitialSize.Height();
/*N*/ 				aYAxesTitlePosition.X() = (int) (aPageSize.Width() * fRelativeXPosition + 0.5);
/*N*/ 				aYAxesTitlePosition.Y() = (int) (aPageSize.Height() * fRelativeYPosition + 0.5);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if (bSwitchColRow)
/*N*/ 				{
/*N*/ 					aYAxesTitlePosition.X() = rRect.Left() + (int) (rRect.GetWidth() / 2);
/*N*/ 					aYAxesTitlePosition.Y() = nTitleBottom;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			SetTextPos((SdrTextObj &)*pYAxisTitleObj, aYAxesTitlePosition,pYAxisTitleAttr);
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|* create 2d column charts
|*
\************************************************************************/

/*N*/ SdrObjGroup* ChartModel::Create2DColChart(Rectangle aRect)
/*N*/ {
/*N*/ 
/*N*/ 	// Background
/*N*/ 	SchObjGroup* pGroup;
/*N*/ 	SdrObjList*  pList;
/*N*/ 	CreateChartGroup (pGroup, pList);
/*N*/ 	Create2DBackplane(aRect, *pList, TRUE,IsStacked() ? CHSTACK_MINMAX : CHSTACK_NONE);
/*N*/ 
/*N*/ 	Rectangle aClipRect( aRect );
/*N*/ 	aClipRect.Right() += 1;
/*N*/ 	aClipRect.Top() -= 1;
/*N*/ 
/*N*/ 	long nColCnt = GetColCount();
/*N*/ 	long nRowCnt = GetRowCount();
/*N*/ 
/*N*/ 	ULONG nInsert,nInsStat;
/*N*/ 	ULONG nAxisBPos=CONTAINER_APPEND;
/*N*/ 	ULONG nAxisYPos=pList->GetObjCount()-1;
/*N*/ 
/*N*/ 	//VerbundChart, Symbol der Linie, dynamische Größe:
/*N*/ 	Size aLegendSize (((SvxFontWidthItem &) pLegendAttr->Get (EE_CHAR_FONTWIDTH)).GetWidth (),
/*N*/ 					  ((SvxFontHeightItem &) pLegendAttr->Get (EE_CHAR_FONTHEIGHT)).GetHeight ());
/*N*/     OutputDevice * pRefDev = GetRefDevice();
/*N*/ 	if(pRefDev)
/*N*/ 		aLegendSize = pRefDev->PixelToLogic (pRefDev->LogicToPixel (aLegendSize));
/*N*/ 	else
/*N*/ 		DBG_ERROR("ChartModel: no RefDevice");
/*N*/ 	long nLegendHeight = aLegendSize.Height () * 9 / 10;
/*N*/ 
/*N*/ 	SdrObject   *pObj;
/*N*/ 	ChartAxis* pAxis=pChartYAxis;
/*N*/ 	short nCol, nRow;
/*N*/ 	BOOL  bPercent = IsPercent();
/*N*/ 
/*N*/ 	long nLines = Min((long)GetNumLinesColChart(),(long)(nRowCnt));;//#50212#
/*N*/ 	long nLineStart = nRowCnt-nLines; //#50212# Ab hier werden Linien gezeichnet
/*N*/ 
/*N*/ 	//BarDescriptoren erzeugen
/*N*/ 	long nR1=0,nR2=0;
/*N*/ 	if(IsStacked())//Reihenzahl in diesem Fall egal
/*N*/ 	{
/*N*/ 		nR1=nR2=1;
/*N*/ 	}
/*N*/ 	else //sonst Datenreihen mit Balken je Achse zählen: (Lines fallen weg)
/*N*/ 	{
/*N*/ 		for(nRow=0;nRow<nRowCnt;nRow++)
/*N*/ 		{
/*N*/ 			long nAxisUID=((const SfxInt32Item&)GetDataRowAttr(nRow).Get(SCHATTR_AXIS)).GetValue();
/*N*/ 			if(IsBar()||IsCol(nRow))
/*N*/ 			{
/*N*/ 				if(nAxisUID == CHART_AXIS_SECONDARY_Y)
/*N*/ 					nR2++;
/*N*/ 				else
/*N*/ 					nR1++;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(!nR1)    //Verbundcharts, Lines werden oben nicht berücksichtigt,
/*N*/ 		nR1=1;  //aber nRn=0 macht keinen Sinn -> Korrektur
/*N*/ 	if(!nR2)
/*N*/ 		nR2=1;
/*N*/ 	aBarY1.Create(aRect,nColCnt,nR1);
/*N*/ 	aBarY2.Create(aRect,nColCnt,nR2);
/*N*/ 	ChartBarDescriptor* pBar=&aBarY1;
/*N*/ 
/*N*/ 	SdrObjList      **pRowLists   = new SdrObjList*[nRowCnt];
/*N*/ 	SdrObjList      **pStatLists  = new SdrObjList*[nRowCnt];
/*N*/ 
/*N*/ 	ChartDataDescription aDescr(nColCnt,nRowCnt,pList,this,bShowDataDescr);
/*N*/ 
/*N*/ 	Point *pTracePoint=0;
/*N*/ 	if(m_nDefaultColorSet&CHSPECIAL_TRACELINES)
/*N*/ 	{
/*?*/ 		pTracePoint = new Point[nRowCnt];//#50149#
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	XPolygon *pLine = new XPolygon[nLines]; //#50149#
/*N*/ 	BOOL	bStartPointIsValid;	//	Indicates wether the first point of a line
/*N*/ 								//	segment is valid.
/*N*/ 
/*N*/ 	for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 	{
/*N*/ 		Point aTextPos;
/*N*/ 
/*N*/ 		for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 		{
/*N*/ 			const SfxItemSet& rDataRowAttr = GetDataRowAttr(nRow);
/*N*/ 			SfxItemSet aDataPointAttr(rDataRowAttr);
/*N*/ 			MergeDataPointAttr(aDataPointAttr,nCol,nRow); //#63904#
/*N*/ 
/*N*/ 			//Achse und Bardescriptor wählen, Insertreihenfolge festlegen  ->
/*N*/ 			long nAxisUID=((const SfxInt32Item&)rDataRowAttr.Get(SCHATTR_AXIS)).GetValue();
/*N*/ 			pAxis=GetAxisByUID(nAxisUID);
/*N*/ 			if(nAxisUID==CHART_AXIS_SECONDARY_Y)
/*N*/ 			{
/*?*/ 				pBar=&aBarY2;
/*?*/ 				nInsert=nAxisBPos;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pBar=&aBarY1;
/*N*/ 				nInsert=nAxisYPos;//Insert-Reihenfolge festlegen
/*N*/ 			}
/*N*/ 			BOOL bIsLine=IsLine(nRow);
/*N*/ 			if(bIsLine)
/*N*/ 				nInsert=CONTAINER_APPEND;
/*N*/ 
/*N*/ 			nInsStat = (nInsert==CONTAINER_APPEND) ? nInsert : nInsert+1;//Statistik immer hinter den Reihenobjekten, sonst wird sie verdeckt
/*N*/ 			//<- Achse und Bardescriptor wählen, Insertreihenfolge festlegen
/*N*/ 
/*N*/ 			if(!nRow)
/*N*/ 			{
/*N*/ 				pChartBAxis->InitStacking();
/*N*/ 				pChartYAxis->InitStacking();
/*N*/ 			}
/*N*/ 
/*N*/ 			if (!nCol)
/*N*/ 			{
/*N*/ 				SchObjGroup* pRowGroup = (SchObjGroup*) CreateSimpleGroup (CHOBJID_DIAGRAM_ROWGROUP, TRUE, TRUE);
/*N*/ 
/*N*/ 				pRowGroup->InsertUserData(new SchDataRow(nRow));
/*N*/ 				pList->NbcInsertObject(pRowGroup,nInsert);
/*N*/ 				pRowLists[nRow] = pRowGroup->GetSubList();
/*N*/ 
/*N*/ 				SchObjGroup* pStatGroup = (SchObjGroup*) CreateSimpleGroup (CHOBJID_DIAGRAM_STATISTICS_GROUP, TRUE, TRUE);
/*N*/ 
/*N*/ 				pStatGroup->InsertUserData(new SchDataRow(nRow));
/*N*/ 				pList->NbcInsertObject(pStatGroup,nInsStat);
/*N*/ 				pStatLists [nRow] = pStatGroup->GetSubList ();
/*N*/ 			}
/*N*/ 
/*N*/ 			long       nIndex         = nCol + nRow * nColCnt;
/*N*/ 			double     fData          = GetData(nCol, nRow, bPercent);
/*N*/ 
/*N*/ 			BOOL bLogarithm = pAxis->IsLogarithm();
/*N*/ 			switch (eChartStyle)
/*N*/ 			{
/*N*/ 				case CHSTYLE_2D_COLUMN:
/*N*/ 				case CHSTYLE_2D_LINE_COLUMN:
/*N*/ 				{
/*N*/ 					Point aTopLeft(pBar->BarLeft(),pAxis->GetUpper(fData,TRUE));
/*N*/ 					Point aBottomRight(pBar->BarRight(),pAxis->GetLower(fData,TRUE));
/*N*/ 
/*N*/ 					Rectangle aObjRect(aTopLeft, aBottomRight);
/*N*/ 
/*N*/ 					BOOL bShow = (aObjRect.Bottom() >= aObjRect.Top());
/*N*/ 					aObjRect.Justify();
/*N*/ 
/*N*/ 					if (!nCol && ((const SfxBoolItem &) rDataRowAttr.Get (SCHATTR_STAT_AVERAGE)).GetValue ())
                        {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pList->NbcInsertObject(AverageValueY(nRow,FALSE,aRect,
/*N*/ 
/*N*/ 					if (bShow && ((!bLogarithm && (fData != DBL_MIN)) ||
/*N*/ 								  (bLogarithm && (fData != DBL_MIN) && (fData > 0.0))))
/*N*/ 					{
/*N*/ 						if(bIsLine) //#50212#
/*N*/ 						{
/*N*/ 							// Verbund-Chart
/*N*/ 							// Letzte "Datenreihe" als Linie ausgeben
/*N*/ 							pLine[nRow-nLineStart][1] =
/*N*/ 								Point(pBar->Middle(),pAxis->GetPos(fData));
/*N*/ 
/*N*/ 							if (nCol == 0)
/*N*/ 							{
/*N*/ 								// Beim ersten Datenpunkt: Anfangspunkt = EndPunkt
/*N*/ 								pLine[nRow-nLineStart][0] = pLine[nRow-nLineStart][1];//#50212#
/*N*/ 								bStartPointIsValid = TRUE;
/*N*/ 							}
/*N*/ 							// Symbol einfuegen
/*N*/                             Point& rInsert = pLine[ nRow - nLineStart ][ 1 ];
/*N*/ 
/*N*/                             if( aClipRect.IsInside( rInsert ) )
/*N*/                             {
/*N*/                                 SdrObject* pNewObj = CreateSymbol( rInsert, nRow, nCol,
/*N*/                                                                    (SfxItemSet &)aDataPointAttr,
/*N*/                                                                    nLegendHeight ); //#50212#
/*N*/                                 if(pNewObj)
/*N*/                                 {
/*N*/                                     pNewObj->InsertUserData(new SchObjectId(CHOBJID_DIAGRAM_DATA));
/*N*/                                     pRowLists[nRow]->NbcInsertObject(pNewObj);
/*N*/                                 }
/*N*/                             }
/*N*/ 
/*N*/ 							//	Insert the line segment only if its starting point has a valid
/*N*/ 							//	value.
/*N*/ 							if (bStartPointIsValid)
/*N*/ 							{
/*N*/ 								// Insert line.
/*N*/                                 XPolyPolygon aResult;
/*N*/                                 SchCalculationHelper::IntersectPolygonWithRectangle( pLine[ nRow - nLineStart ],
/*N*/                                                                                      aClipRect,
/*N*/                                                                                      aResult );
/*N*/                                 if( aResult.Count())
/*N*/                                 {
/*N*/                                     SdrPathObj* pObj = new SdrPathObj( OBJ_PLIN, aResult );
/*N*/                                     pObj->InsertUserData( new SchObjectId( CHOBJID_DIAGRAM_ROWSLINE ));
/*N*/                                     pObj->InsertUserData( new SchDataRow( nRow ));
/*N*/                                     pRowLists[ nRow ]->NbcInsertObject( pObj, 0 );
/*N*/ 
/*N*/                                     // Set the line's attributes.
/*N*/                                     pObj->SetItemSet( rDataRowAttr );
/*N*/                                 }
/*N*/ 							}
/*N*/ 
/*N*/ 							// Anfangspunkt des naechsten Datenpunkts =
/*N*/ 							// Endpunkt des aktuellen Datenpunkts
/*N*/ 							pLine[nRow-nLineStart][0] = pLine[nRow-nLineStart][1];//#50212#
/*N*/ 							bStartPointIsValid = TRUE;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							pRowLists[nRow]->NbcInsertObject(CreateRect (aObjRect, nCol, nRow, aDataPointAttr));
/*N*/ 						}
/*N*/ 
/*N*/ 						if ((SvxChartKindError)
/*N*/ 							((const SfxInt32Item &) aDataPointAttr.Get (SCHATTR_STAT_KIND_ERROR)).GetValue () !=
/*N*/ 							 CHERROR_NONE)
/*?*/ 						{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	AverageErrorY(nRow,fData,aObjRect.TopCenter(),FALSE,aDataPointAttr,pStatLists[nRow],pAxis);
/*N*/ 
/*N*/ 						if(aDescr.Enabled())
/*N*/ 						{
/*N*/ 							Point aPos(bIsLine ? pLine[nRow-nLineStart][1] : aObjRect.TopCenter() );
/*N*/ 							aDescr.Insert(nCol,nRow,aDataPointAttr,aPos,FALSE,CHADJUST_BOTTOM_CENTER,pAxis);
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else if (bIsLine)
/*N*/ 					{
/*N*/ 						//	Remember that the current point may not be inserted.
/*?*/ 						bStartPointIsValid = FALSE;
/*N*/ 					}
/*N*/ 
/*N*/ 					pBar->NextBar();
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 				case CHSTYLE_2D_STACKEDCOLUMN:
/*N*/ 				case CHSTYLE_2D_PERCENTCOLUMN:
/*N*/ 				case CHSTYLE_2D_LINE_STACKEDCOLUMN:
/*N*/ 				{
/*N*/ 					Pair aTopBottom(pAxis->Stack(fData,TRUE));
/*N*/ 					Point aTopLeft(pBar->BarLeft()/*nPos*/,aTopBottom.A());
/*N*/ 					Point aBottomRight(pBar->BarRight()/*nPos + nColWidth*/,aTopBottom.B());
/*N*/ 
/*N*/ 
/*N*/ 					Rectangle aObjRect(aTopLeft, aBottomRight);
/*N*/ 
/*N*/ 					BOOL bShow =   (aObjRect.Bottom() >= aObjRect.Top());
/*N*/ 
/*N*/ 					//Stackedline-Chart benutzt dieses ObjectRect nicht => Bug #48970#
/*N*/ 					if (bIsLine)
/*?*/ 						   bShow=TRUE;//#50212#
/*N*/ 
/*N*/ 					if(fData==DBL_MIN)
/*N*/ 					{
/*?*/ 						bShow=FALSE;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						// bShow includes Top == Bottom and that may be ok, if there is no data in the chart
/*N*/ 						// and > might also happen (difference 1 from rounding errors)
/*N*/ 						// AND if bShow is TRUE the following part fixes the problem
/*N*/ 						// so ASSERT => TRACE
/*N*/ 						CHART_TRACE1( "Create2DColChart bShow = %s", bShow? "True" : "False" );
/*N*/ 					}
/*N*/ 					if(bLogarithm && (fData <= 0.0) )
/*?*/ 						bShow=FALSE;
/*N*/ 
/*N*/ 					if (bShow)
/*N*/ 					{
/*N*/ 						aObjRect.Justify();
/*N*/ 
/*N*/ 						if (bIsLine)//#50212#
/*N*/ 						{
/*?*/ 							pLine[nRow-nLineStart][1] = Point(pBar->Middle()//MIDPOS//zu umständlich: nPos + nPartWidth/2 - nGap
/*?*/ 								,pAxis->GetPos(fData));//#50212#
/*?*/ 
/*?*/ 							if (nCol == 0)
/*?*/ 							{
/*?*/ 								// Beim ersten Datenpunkt: Anfangspunkt = EndPunkt
/*?*/ 								pLine[nRow-nLineStart][0] = pLine[nRow-nLineStart][1];//#50212#
/*?*/ 							}
/*?*/                             Point& rInsert = pLine[ nRow - nLineStart ][ 1 ];
/*?*/ 
/*?*/                             if( aClipRect.IsInside( rInsert ))
/*?*/                             {
/*?*/                                 SdrObject* pNewObj = CreateSymbol( rInsert, nRow, nCol,
/*?*/                                                                    (SfxItemSet &) aDataPointAttr,
/*?*/                                                                    nLegendHeight ); //#50212#
/*?*/                                 if(pNewObj)
/*?*/                                 {
/*?*/                                     pNewObj->InsertUserData(new SchObjectId(CHOBJID_DIAGRAM_DATA));
/*?*/                                     pRowLists[nRow]->NbcInsertObject(pNewObj);
/*?*/                                 }
/*?*/                             }
/*?*/ 
/*?*/ 							// Insert line.
/*?*/                             XPolyPolygon aResult;
/*?*/                             SchCalculationHelper::IntersectPolygonWithRectangle( pLine[ nRow - nLineStart ],
/*?*/                                                                                  aClipRect,
/*?*/                                                                                  aResult );
/*?*/                             if( aResult.Count())
/*?*/                             {
/*?*/                                 SdrPathObj* pObj = new SdrPathObj( OBJ_PLIN, aResult );
/*?*/                                 pObj->InsertUserData( new SchObjectId( CHOBJID_DIAGRAM_ROWSLINE ));
/*?*/                                 pObj->InsertUserData( new SchDataRow( nRow ));
/*?*/                                 pRowLists[ nRow ]->NbcInsertObject( pObj, 0 );
/*?*/ 
/*?*/                                 // Set the line's attributes.
/*?*/                                 pObj->SetItemSet( rDataRowAttr );
/*?*/                             }
/*?*/ 
/*?*/ 							// Anfangspunkt des naechsten Datenpunkts =
/*?*/ 							// Endpunkt des aktuellen Datenpunkts
/*?*/ 							pLine[nRow-nLineStart][0] = pLine[nRow-nLineStart][1];//#50212#
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{ //#50116#
/*N*/ 							Rectangle aBarRect(aObjRect);
/*N*/ 							if(nBarPercentWidth<100 && nBarPercentWidth>0)
/*N*/ 							{
/*?*/ 								double fWidth=aBarRect.GetWidth();
/*?*/ 								fWidth=fWidth*((double)nBarPercentWidth/100.0);
/*?*/ 								long nWidth=Round(fWidth);
/*?*/ 								long nDiff=(aBarRect.GetWidth()-nWidth);
/*?*/ 
/*?*/ 								Size aSize=aBarRect.GetSize();
/*?*/ 								aSize.Width()-=nDiff;
/*?*/ 								aBarRect.SetSize(aSize);
/*?*/ 								aBarRect.Move(nDiff/2,0);
/*N*/ 							}
/*N*/ 							pRowLists[nRow]->NbcInsertObject(CreateRect (aBarRect, nCol, nRow, aDataPointAttr));
/*N*/ 
/*N*/ 							BOOL bIsDownward=(BOOL)(fData < 0.0);//FALSE;//#51471#
/*N*/ 
/*N*/ 							if(pTracePoint) //#50149#
/*N*/ 							{
/*?*/ 								if(nCol != 0)
/*?*/ 								{
/*?*/ 									Point aEndPoint=bIsDownward ? aBarRect.BottomLeft() : aBarRect.TopLeft();//#51471#
/*?*/ 									SdrPathObj* pObj = new SdrPathObj(pTracePoint[nRow],aEndPoint);
/*?*/ 									pObj->InsertUserData(new SchObjectId (0));
/*?*/ 									pList->NbcInsertObject(pObj);//immer vorne, egal welche Achse
/*?*/ 									// Linie attributieren
/*?*/ 
/*?*/ //-/									pObj->NbcSetAttributes(rDataRowAttr, FALSE);
/*?*/ 									pObj->SetItemSet(rDataRowAttr);
/*?*/ 
/*?*/ 							   }
/*?*/ 								pTracePoint[nRow]=bIsDownward ? aBarRect.BottomRight() :aBarRect.TopRight();//#51471#
/*N*/ 							}
/*N*/ 						}
/*N*/ 						if(aDescr.Enabled())
/*N*/ 						{
/*N*/ 							if(bIsLine)
/*?*/ 								aDescr.Insert(nCol,nRow,aDataPointAttr,pLine[nRow-nLineStart][1],FALSE,CHADJUST_BOTTOM_CENTER,pAxis);
/*N*/ 							else
/*N*/ 								aDescr.Insert(nCol,nRow,aDataPointAttr,aObjRect.Center(),FALSE,CHADJUST_CENTER_CENTER,pAxis);
/*N*/ 						}
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				}
/*N*/ 
/*N*/ 				case CHSTYLE_2D_BAR:
/*N*/ 				{
/*N*/ 					Point aTopLeft( pAxis->GetLower(fData),pBar->BarTop() );
/*N*/ 					Point aRightBottom( pAxis->GetUpper(fData),pBar->BarBottom() );
/*N*/ 					Rectangle aObjRect(aTopLeft,aRightBottom);
/*N*/ 
/*N*/ 					BOOL bShow = (aObjRect.Right() >= aObjRect.Left());
/*N*/ 					aObjRect.Justify();
/*N*/ 
/*N*/ 					if (!nCol && ((const SfxBoolItem &) rDataRowAttr.Get (SCHATTR_STAT_AVERAGE)).GetValue ())
/*?*/ 						{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pList->NbcInsertObject (AverageValueY (nRow, TRUE, aRect,
/*N*/ 
/*N*/ 					if ((bShow) && ((!bLogarithm && (fData != DBL_MIN)) ||
/*N*/ 									(bLogarithm && (fData != DBL_MIN) && (fData > 0.0))))
/*N*/ 					{
/*N*/ 						pRowLists[nRow]->NbcInsertObject(CreateRect (aObjRect, nCol, nRow, aDataPointAttr));
/*N*/ 
/*N*/ 						if ((SvxChartKindError)
/*N*/ 							((const SfxInt32Item &) aDataPointAttr.Get (SCHATTR_STAT_KIND_ERROR)).GetValue () !=
/*N*/ 							 CHERROR_NONE)
/*?*/ 						{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	AverageErrorY(nRow,fData, aObjRect.RightCenter(),TRUE,aDataPointAttr,pStatLists[nRow],pAxis);
/*N*/ 
/*N*/ 						if(aDescr.Enabled())
/*N*/ 						{
/*N*/ 							Point aPos(aObjRect.TopRight());
/*N*/ 							aPos.Y()+=aObjRect.GetHeight() / 2;
/*N*/ 							aPos.X()+=500;
/*N*/ 							aDescr.Insert(nCol,nRow,aDataPointAttr,aPos,FALSE,CHADJUST_CENTER_LEFT,pAxis);
/*N*/ 						}
/*N*/ 					}
/*N*/ 					pBar->NextBar();
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 				case CHSTYLE_2D_STACKEDBAR:
/*N*/ 				case CHSTYLE_2D_PERCENTBAR:
/*N*/ 				{
/*N*/ 					Pair aLeftRight(pAxis->Stack(fData,TRUE));
/*N*/ 					Point aTopLeft(aLeftRight.A(),pBar->BarBottom());
/*N*/ 					Point aBottomRight(aLeftRight.B(),pBar->BarTop());
/*N*/ 
/*N*/ 					Rectangle aObjRect(aTopLeft,aBottomRight);
/*N*/ 
/*N*/ 
/*N*/ 					if ((aObjRect.Right() >= aObjRect.Left()) &&
/*N*/ 						((!bLogarithm && (fData != DBL_MIN)) ||
/*N*/ 						 (bLogarithm && (fData != DBL_MIN) && (fData > 0.0))))
/*N*/ 					{
/*N*/ 						aObjRect.Justify();
/*N*/ 
/*N*/ 						pObj = new SdrRectObj( aObjRect );
/*N*/ 						pObj->SetModel( this );
/*N*/ 						pObj = SetObjectAttr( pObj, CHOBJID_DIAGRAM_DATA, TRUE, TRUE, &aDataPointAttr );
/*N*/ 						pObj->InsertUserData(new SchDataPoint(nCol, nRow));
/*N*/ 						pRowLists[nRow]->NbcInsertObject(pObj);
/*N*/ 
/*N*/ 						if(aDescr.Enabled())
/*N*/ 							aDescr.Insert(nCol,nRow,aDataPointAttr,aObjRect.Center(),FALSE,CHADJUST_CENTER_CENTER,pAxis);
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		} //for nRow
/*N*/ 		aBarY1.NextCol();
/*N*/ 		aBarY2.NextCol();
/*N*/ 	}//for nCol
/*N*/ 
/*N*/ 	aDescr.Build(TRUE);
/*N*/ 	delete[] pTracePoint;//#50149#
/*N*/ 	delete[] pLine;//#50212#
/*N*/ 	delete[] pRowLists;
/*N*/ 	delete[] pStatLists;
/*N*/ 	return pGroup;
/*N*/ }


/*************************************************************************
|*
|* Fl„chendiagramm erzeugen
|*		case CHSTYLE_2D_LINE:
|*		case CHSTYLE_2D_STACKEDLINE:
|*		case CHSTYLE_2D_PERCENTLINE:
|*      case CHSTYLE_2D_LINESYMBOLS :
|*      case CHSTYLE_2D_STACKEDLINESYM :
|*	    case CHSTYLE_2D_PERCENTLINESYM :
|*      case CHSTYLE_2D_CUBIC_SPLINE :
|*      case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL :
|*      case CHSTYLE_2D_B_SPLINE :
|*      case CHSTYLE_2D_B_SPLINE_SYMBOL :
|*
\************************************************************************/
/*N*/ BOOL ChartModel::HasStockLines( SvxChartStyle* pStyle )
/*N*/ {
/*N*/ 	switch( pStyle? *pStyle: eChartStyle )
/*N*/ 	{
/*N*/ 		case CHSTYLE_2D_STOCK_1:
/*N*/ 		case CHSTYLE_2D_STOCK_2:
/*N*/ 		case CHSTYLE_2D_STOCK_3:
/*N*/ 		case CHSTYLE_2D_STOCK_4:
/*N*/ 		return TRUE;
/*N*/ 		default:
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ }
/*N*/ BOOL ChartModel::HasStockRects( SvxChartStyle* pStyle )
/*N*/ {
/*N*/ 	switch( pStyle? *pStyle: eChartStyle )
/*N*/ 	{
/*N*/ 		case CHSTYLE_2D_STOCK_2:
/*N*/ 		case CHSTYLE_2D_STOCK_4:
/*N*/ 		return TRUE;
/*N*/ 		default:
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ }
/*N*/ BOOL ChartModel::HasStockBars( SvxChartStyle* pStyle )
/*N*/ {
/*N*/ 	switch( pStyle? *pStyle: eChartStyle )
/*N*/ 	{
/*N*/ 		case CHSTYLE_2D_STOCK_3:
/*N*/ 		case CHSTYLE_2D_STOCK_4:
/*N*/ 		return TRUE;
/*N*/ 		default:
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ }
/*N*/ ChartAxis* ChartModel::GetAxisByUID(long nUId)
/*N*/ {
/*N*/ 	switch(nUId)
/*N*/ 	{
/*N*/ 	case CHART_AXIS_PRIMARY_X:
/*N*/ 		return pChartXAxis;
/*N*/ 	case CHART_AXIS_PRIMARY_Y:
/*N*/ 		return pChartYAxis;
/*N*/ 	case CHART_AXIS_PRIMARY_Z:
/*N*/ 		return pChartZAxis;
/*N*/ 	case CHART_AXIS_SECONDARY_Y:
/*N*/ 		return pChartBAxis;
/*N*/ 	case CHART_AXIS_SECONDARY_X:
/*N*/ 		return pChartAAxis;
/*N*/ 	}
/*N*/ 	DBG_ERROR1( "ChartAxis not found: id=%ld", nUId );
/*N*/ 	return pChartYAxis;
/*N*/ }
/*N*/ #define SchDataCol(a) SchDataRow(a)     //erst mal testen..... (wird eh ne geklonte Klasse)

/*N*/ void ChartModel::DrawStockLines( SdrObjList* pList, const Rectangle& rRect )
/*N*/ {	//und fuer Stock-Charts
/*N*/ 
/*N*/ 	long nStart=HasStockBars() ? 1:0;
/*N*/ 	if(HasStockLines())
/*N*/ 	{
/*N*/ 
/*N*/ 		long nColCnt = GetColCount();
/*N*/ 		long nRowCnt = GetRowCount();
/*N*/ 		long nCol, nRow;
/*N*/ 
/*N*/ 		SdrObject   *pObj;
/*N*/ 		SdrObjList  *pLineList,*pLossList=NULL,*pPlusList=NULL;
/*N*/ 		SchObjGroup *pLineGroup,*pLossGroup=NULL,*pPlusGroup=NULL;
/*N*/ 
/*N*/ 
/*N*/ 		ChartBarDescriptor* pBar;
/*N*/ 		aBarY2.Create(rRect,nColCnt,1);
/*N*/ 		aBarY1.Create(rRect,nColCnt,1);
/*N*/ 
/*N*/ 		pLineGroup =(SchObjGroup*)CreateSimpleGroup(CHOBJID_DIAGRAM_STOCKLINE_GROUP,TRUE, TRUE);
/*N*/ 		pList->NbcInsertObject(pLineGroup);
/*N*/ 		pLineList = pLineGroup->GetSubList();
/*N*/ 
/*N*/ 		XPolygon aPolyStock(2);
/*N*/ 
/*N*/ 		//StockRects:
/*N*/ 		long nLow,nHi;
/*N*/ 
/*N*/ 		for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 		{
/*N*/ 			BOOL bOK=FALSE;
/*N*/ 			nLow=nHi=0;
/*N*/ 			for (nRow = nStart; nRow < nRowCnt; nRow++)
/*N*/ 			{
/*N*/ 				if(GetAxisUID(nRow)==CHART_AXIS_SECONDARY_Y)
/*?*/ 					pBar=&aBarY2;
/*N*/ 				else
/*N*/ 					pBar=&aBarY1;
/*N*/ 
/*N*/ 				ChartAxis *pAxis=GetAxisByUID(((const SfxInt32Item &)GetDataRowAttr(nRow).Get(SCHATTR_AXIS)).GetValue());
/*N*/ 				double fData = GetData(nCol,nRow,FALSE);
/*N*/ 				if(fData != DBL_MIN)
/*N*/ 				{
/*N*/ 					bOK=TRUE;
/*N*/ 					long nYPos = pAxis->GetPos(fData);
/*N*/ 
/*N*/ 					aPolyStock[0].X()=aPolyStock[1].X()=(USHORT)pBar->Middle();
/*N*/ 					if(nRow==nStart)//geht auch schöner: bInitialise (ToDo:)
/*N*/ 					{
/*N*/ 						aPolyStock[0].Y()=aPolyStock[1].Y()=(USHORT) nYPos;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						if((USHORT)nYPos > aPolyStock[0].Y())
/*N*/ 							aPolyStock[0].Y()=(USHORT)nYPos;
/*N*/ 						if((USHORT)nYPos < aPolyStock[1].Y())
/*N*/ 							aPolyStock[1].Y()=(USHORT)nYPos;
/*N*/ 					}
/*N*/ 					switch(nRow-nStart)//StockRects
/*N*/ 					{
/*N*/ 						case 0:
/*N*/ 							nLow=nYPos;
/*N*/ 							break;
/*N*/ 						case 3:
/*N*/ 							nHi=nYPos;
/*N*/ 							break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if(bOK)
/*N*/ 			{
/*N*/ 				pObj = new SdrPathObj(OBJ_PLIN, aPolyStock);
/*N*/ 				pObj->InsertUserData(new SchObjectId (CHOBJID_DIAGRAM_STOCKLINE));
/*N*/ 				pObj->InsertUserData(new SchDataCol((short)nCol));
/*N*/ 				pLineList->NbcInsertObject(pObj,LIST_APPEND);
/*N*/ 
/*N*/ //-/				pObj->NbcSetAttributes(*pStockLineAttr, FALSE);
/*N*/ 				pObj->SetItemSet(*pStockLineAttr);
/*N*/ 
/*N*/ 
/*N*/ 				if(HasStockRects()&&nRowCnt>3)
/*N*/ 				{
/*N*/ 					BOOL bLoss=TRUE;
/*N*/ 					if(nLow>nHi)
/*N*/ 					{
/*N*/ 						long nTmp=nLow;
/*N*/ 						nLow=nHi;
/*N*/ 						nHi=nTmp;
/*N*/ 						bLoss=FALSE;
/*N*/ 					}
/*N*/ 
/*N*/ 					if(!pLossList&&bLoss)
/*N*/ 					{
/*N*/ 						pLossGroup =(SchObjGroup*)CreateSimpleGroup(CHOBJID_DIAGRAM_STOCKLOSS_GROUP,TRUE, TRUE);
/*N*/ 						pList->NbcInsertObject(pLossGroup);
/*N*/ 						pLossList = pLossGroup->GetSubList();
/*N*/ 					}
/*N*/ 					if(!pPlusList && !bLoss)
/*N*/ 					{
/*N*/ 						pPlusGroup =(SchObjGroup*)CreateSimpleGroup(CHOBJID_DIAGRAM_STOCKPLUS_GROUP,TRUE, TRUE);
/*N*/ 						pList->NbcInsertObject(pPlusGroup);
/*N*/ 						pPlusList = pPlusGroup->GetSubList();
/*N*/ 					}
/*N*/ 					pObj = new SdrRectObj(Rectangle(Point(pBar->BarLeft(),nLow),Size(pBar->BarWidth(),nHi-nLow)));
/*N*/ 					pObj->InsertUserData(new SchObjectId (CHOBJID_DIAGRAM_STOCKRECT));
/*N*/ 					pObj->InsertUserData(new SchDataCol((short)nCol));
/*N*/ 					if(bLoss)
/*N*/ 					{
/*N*/ 						pLossList->NbcInsertObject(pObj,LIST_APPEND);
/*N*/ 
/*N*/ //-/						pObj->NbcSetAttributes(*pStockLossAttr, FALSE);
/*N*/ 						pObj->SetItemSet(*pStockLossAttr);
/*N*/ 
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						pPlusList->NbcInsertObject(pObj,LIST_APPEND);
/*N*/ 
/*N*/ //-/						pObj->NbcSetAttributes(*pStockPlusAttr, FALSE);
/*N*/ 						pObj->SetItemSet(*pStockPlusAttr);
/*N*/ 
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 			}
/*N*/ 
/*N*/ 			aBarY1.NextCol();
/*N*/ 			aBarY2.NextCol();
/*N*/ //			nPos+=nXWidth;
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/ void ChartModel::DrawStockBars(SdrObjList* pList,const Rectangle& aRect)
/*N*/ {
/*N*/ 	const long nRow = 0;
/*N*/ 
/*N*/ 	const SfxItemSet& rDataRowAttr = GetDataRowAttr(nRow);
/*N*/ 
/*N*/ 	long nColCnt = GetColCount();
/*N*/ 
/*N*/ 	ChartBarDescriptor* pBar;
/*N*/ 	long nAxisUID=GetAxisUID(nRow);
/*N*/ 	switch(nAxisUID)
/*N*/ 	{
/*N*/ 		case CHART_AXIS_SECONDARY_Y:
/*N*/ 			aBarY2.Create(aRect,nColCnt,1);
/*N*/ 			pBar=&aBarY2;
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			aBarY1.Create(aRect,nColCnt,1);
/*N*/ 			pBar=&aBarY1;
/*N*/ 			break;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 
/*N*/ 	SchObjGroup *pStatGroup=NULL;
/*N*/ 
/*N*/ 
/*N*/ 	if(GetRowCount()&&HasStockBars())
/*N*/ 	{
/*N*/ 		SdrObjList* pDescrList = NULL;
/*N*/ 
/*N*/ 		ChartAxis *pAxis=GetAxisByUID(((const SfxInt32Item &)rDataRowAttr.Get(SCHATTR_AXIS)).GetValue());
/*N*/ 		long nCol;
/*N*/ 
/*N*/ 		SdrObjList  *pBarList;
/*N*/ 
/*N*/ 		SdrObjList* pStatList  = NULL;
/*N*/ 
/*N*/ 		SchObjGroup *pBarGroup;
/*N*/ 
/*N*/ 		pBarGroup =(SchObjGroup*)CreateSimpleGroup(CHOBJID_DIAGRAM_ROWGROUP,TRUE, TRUE);
/*N*/ 		pBarGroup->InsertUserData(new SchDataRow(nRow));
/*N*/ 
/*N*/ 		pList->NbcInsertObject(pBarGroup);
/*N*/ 		pBarList = pBarGroup->GetSubList();
/*N*/ 
/*N*/ 		//Statistik:
/*N*/ 		if ( ((const SfxBoolItem &) rDataRowAttr.Get (SCHATTR_STAT_AVERAGE)).GetValue ())
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if(!pStatList)
/*N*/ 		}
/*N*/ 
/*N*/ 		for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 		{
/*N*/ 			SfxItemSet aDataPointAttr(GetFullDataPointAttr(nCol,nRow));
/*N*/ 
/*N*/ 
/*N*/ 			double fData = GetData(nCol,nRow,FALSE);
/*N*/ 			long nHi = pAxis->GetPos(fData);
/*N*/ 			long nLow= pAxis->GetPosOrigin();
/*N*/ 
/*N*/ 			if(fData != DBL_MIN)
/*N*/ 			{
/*N*/ 				Rectangle aObjRect(Point(pBar->BarLeft(),nLow),Size(pBar->BarWidth(),nHi-nLow));
/*N*/ 
/*N*/ 				pBarList->NbcInsertObject(CreateRect(aObjRect,nCol,nRow,aDataPointAttr));
/*N*/ 
/*N*/ 				if ((SvxChartKindError)
/*N*/ 					((const SfxInt32Item &) aDataPointAttr.Get (SCHATTR_STAT_KIND_ERROR)).GetValue () !=
/*N*/ 					 CHERROR_NONE)
/*N*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if(!pStatList)
/*N*/ 				}
/*N*/ 
/*N*/ 
/*N*/ 				// ***************************** Description *****************************
/*N*/ 				SvxChartDataDescr eDescr  = ((const SvxChartDataDescrItem&)aDataPointAttr.
/*N*/ 												Get(SCHATTR_DATADESCR_DESCR)).GetValue();
/*N*/ 
/*N*/ 				if((eDescr!=CHDESCR_NONE)&&bShowDataDescr)
/*N*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 DataDescription aDescr;
/*N*/ 				}
/*N*/ 				// ************************* end description *******************************
/*N*/ 			}
/*N*/ 			pBar->NextCol();
/*N*/ 			//		nPos+=nXWidth;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ SdrObjGroup* ChartModel::Create2DRowLineChart(Rectangle aRect)
/*N*/ {
/*N*/ 	BOOL bPartDescr=FALSE;
/*N*/ 	if(HasStockLines())
/*N*/ 		bPartDescr=TRUE;
/*N*/ 
/*N*/ 	SchObjGroup *pGroup;
/*N*/ 	SdrObjList  *pList;
/*N*/ 	CreateChartGroup (pGroup, pList);
/*N*/ 	BOOL bStacked   = IsStacked();
/*N*/ 	USHORT eStackMode =  bStacked ? CHSTACK_OVERLAP : CHSTACK_NONE;
/*N*/ 	Create2DBackplane(aRect, *pList, bPartDescr, eStackMode);
/*N*/ 
/*N*/ 	Rectangle aClipRect( aRect );
/*N*/ 	aClipRect.Right() += 1;
/*N*/ 	aClipRect.Top() -= 1;
/*N*/ 
/*N*/ 	SdrObject   *pObj;
/*N*/ 	long	nColCnt = GetColCount();
/*N*/ 	long	nRowCnt = GetRowCount();
/*N*/ 	long	nCol, nRow;
/*N*/ 
/*N*/ 	BOOL bPercent   = IsPercent();
/*N*/ 
/*N*/ 	//ToDo: XAchse-Logarithmus auf FALSE erzwingen (???)
/*N*/ 
/*N*/ 	long nBackColCnt = (bPartDescr) ? nColCnt +1: nColCnt ;
/*N*/ 
/*N*/ 	if (nColCnt > 1)
/*N*/ 	{
/*N*/ 		nBackColCnt--;
/*N*/ 	}
/*N*/ 
/*N*/ 	long   nPartWidth = aRect.GetWidth() / nBackColCnt;
/*N*/ 	double fPartWidth = ((double) aRect.GetWidth() / (double) nBackColCnt);
/*N*/ 
/*N*/ 	SdrObjList** pRowLists   = new SdrObjList*[nRowCnt];
/*N*/ 	SdrObjList** pStatLists  = new SdrObjList*[nRowCnt];
/*N*/ 
/*N*/ 	ChartDataDescription aDescr(nColCnt,nRowCnt,pList,this,bShowDataDescr);
/*N*/ 
/*N*/ 	//Berechnung von nLegendHeigth:
/*N*/ 	Size aLegendSize (((SvxFontWidthItem &) pLegendAttr->Get(EE_CHAR_FONTWIDTH)).GetWidth(),
/*N*/ 					  ((SvxFontHeightItem &) pLegendAttr->Get(EE_CHAR_FONTHEIGHT)).GetHeight());
/*N*/ 
/*N*/ 	//#54884# Was soll das hier? Create2DRowLineChart
/*N*/     OutputDevice * pRefDev = GetRefDevice();
/*N*/ 	if(pRefDev)
/*N*/ 		aLegendSize = pRefDev->PixelToLogic (pRefDev->LogicToPixel (aLegendSize));
/*N*/ 	else
/*N*/ 		DBG_ERROR("ChartModel: no RefDevice");
/*N*/ 
/*N*/ 	long nLegendHeight = aLegendSize.Height () * 9 / 10;
/*N*/ 
/*N*/ 	Polygon   aPolygon( nColCnt + 16 );		// +4 -> +16: let some more points be possible. Is set to exact size later
/*N*/ 
/*N*/ 	//Ab hier speziell fuer (Sp)Line-Charts:
/*N*/ 	XPolygon   *pSpline = new XPolygon(nColCnt * nGranularity);
/*N*/ 
/*N*/ 	long nStartX=aRect.Left();
/*N*/ 	if(bPartDescr)
/*N*/ 		nStartX+=nPartWidth/2;
/*N*/ 
/*N*/ 	if(HasStockBars())
/*N*/ 		DrawStockBars( pList, aRect );
/*N*/ 	if(HasStockLines())
/*N*/ 		DrawStockLines( pList, aRect );
/*N*/ 
/*N*/ 	long nStart=HasStockBars() ? 1: 0;
/*N*/ 
/*N*/ 	long nAllObjects = pList->GetObjCount () - 1;
/*N*/ 
/*N*/ 	for (nRow = nStart; nRow < nRowCnt; nRow++)
/*N*/ 	{
/*N*/ 		BOOL bArea=IsArea(nRow);
/*N*/ 		const SfxItemSet &rDataRowAttr = GetDataRowAttr(nRow);
/*N*/ 		long nAxisUId = ((const SfxInt32Item &)rDataRowAttr.Get(SCHATTR_AXIS)).GetValue();
/*N*/ 		ChartAxis *pAxis=GetAxisByUID(nAxisUId);
/*N*/ 
/*N*/ 		long             nPos          = nStartX;
/*N*/ 		double           fPos          = nStartX;
/*N*/ 		USHORT           nPoints       = 0;
/*N*/ 
/*N*/ 		SchObjGroup *pRowGroup =(SchObjGroup*)CreateSimpleGroup(CHOBJID_DIAGRAM_ROWGROUP,TRUE, TRUE);
/*N*/ 		SchObjGroup *pStatGroup=(SchObjGroup*)CreateSimpleGroup(CHOBJID_DIAGRAM_STATISTICS_GROUP,TRUE,TRUE);
/*N*/ 
/*N*/ 		pRowGroup->InsertUserData(new SchDataRow((short)nRow));
/*N*/ 
/*N*/ 		if(bArea)
/*N*/ 			pList->NbcInsertObject(pRowGroup, nAllObjects);
/*N*/ 		else
/*N*/ 			pList->NbcInsertObject(pRowGroup);
/*N*/ 
/*N*/ 		pRowLists[nRow]   = pRowGroup->GetSubList();
/*N*/ 
/*N*/ 		pStatGroup->InsertUserData(new SchDataRow((short)nRow));
/*N*/ 		pList->NbcInsertObject(pStatGroup);
/*N*/ 		pStatLists [nRow] = pStatGroup->GetSubList();
/*N*/ 
/*N*/ 		if( ! bStacked )
/*N*/ 		{
/*N*/ 			if (((const SfxBoolItem &) rDataRowAttr.Get (SCHATTR_STAT_AVERAGE)).GetValue ())
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 	pList->NbcInsertObject (AverageValueY (nRow, FALSE, aRect,
/*N*/ 		}
/*N*/ 
/*N*/ 		for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 		{
/*N*/ 			SfxItemSet aDataPointAttr(rDataRowAttr);//#63904#
/*N*/ 			MergeDataPointAttr(aDataPointAttr,nCol,nRow);
/*N*/ 
/*N*/ 			double fData = GetData(nCol, nRow, bPercent);
/*N*/ 
/*N*/ 			BOOL bLogarithm = pAxis->IsLogarithm();
/*N*/ 			BOOL bValidData=((fData!=DBL_MIN)&&(!bLogarithm||bLogarithm&&(fData>0.0)));
/*N*/ 
/*N*/ 			if(!bValidData && bStacked)
/*N*/ 			{
/*N*/ 				fData      = 0.0;
/*N*/ 				bValidData = TRUE;
/*N*/ 			}
/*N*/ 
/*N*/ 			// *****************************end Data Valid? ***************************
/*N*/ 
/*N*/ 			if (bValidData)
/*N*/ 			{
/*N*/ 				if(eStackMode != CHSTACK_NONE)
/*N*/ 					fData = pAxis->StackColData(fData,nCol,nColCnt);
/*N*/ 
/*N*/ 				long nYPos = pAxis->GetPos(fData);
/*N*/ 
/*N*/ 				aPolygon[nPoints].X() = nPos;
/*N*/ 				aPolygon[nPoints].Y() = nYPos;
/*N*/ 
/*N*/ 				nPoints++;
/*N*/ 
/*N*/ 				if (HasSymbols(nRow))
/*N*/ 				{
/*N*/ 
/*N*/ 					Point& rInsert = aPolygon[nPoints - 1];
/*N*/ 
/*N*/ 					if( aClipRect.IsInside( rInsert ) )
/*N*/ 					{
/*N*/ 						SdrObject *pNewObj = CreateSymbol( rInsert, nRow, nCol,
/*N*/ 														   (SfxItemSet &) aDataPointAttr, nLegendHeight );
/*N*/ 						if(pNewObj)
/*N*/ 						{
/*N*/ 							pNewObj->InsertUserData(new SchObjectId(CHOBJID_DIAGRAM_DATA));
/*N*/ 							pRowLists[nRow]->NbcInsertObject(pNewObj);
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				if (!bStacked)
/*N*/ 				{
/*N*/ 					if ((SvxChartKindError)
/*N*/ 						((const SfxInt32Item &)aDataPointAttr.Get(SCHATTR_STAT_KIND_ERROR)).GetValue () != CHERROR_NONE)
/*?*/ 						 {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 AverageErrorY(nRow,fData,aPolygon[nPoints -1],FALSE,aDataPointAttr,pStatLists[nRow],pAxis);
/*N*/ 				}
/*N*/ 
/*N*/ 				if(aDescr.Enabled())
/*N*/ 				{
/*N*/ 					Point aPos(aPolygon[nPoints-1]);
/*N*/ 					if(aPos.X() == aRect.Left())
/*N*/ 					{
/*N*/ 						aPos.X()+=50;
/*N*/ 						aDescr.Insert(nCol,nRow,aDataPointAttr,aPos,FALSE,CHADJUST_BOTTOM_LEFT,pAxis);
/*N*/ 					}
/*N*/ 					else
/*N*/ 						aDescr.Insert(nCol,nRow,aDataPointAttr,aPos,FALSE,CHADJUST_BOTTOM_CENTER,pAxis);
/*N*/ 				}
/*N*/ 				// ************************* end description *******************************
/*N*/ 			}
/*N*/ 			else if( bArea )		// invalid data, area
/*N*/ 			{
/*?*/ 				if(eStackMode != CHSTACK_NONE)
/*?*/ 					fData = pAxis->StackColData(fData,nCol,nColCnt);
/*?*/ 				long nYPos = pAxis->GetPos(0.0);
/*?*/ 				//fehlende Werte werden hier als 0 interpretiert - weglassen des BlockInhalts wäre Interpolation!
/*?*/ 				aPolygon[nPoints].X() = nPos;
/*?*/ 				aPolygon[nPoints].Y() = nYPos;
/*?*/ 				nPoints++;
/*N*/ 			}
/*N*/ 			else				// invalid data, no area
/*N*/ 			{
/*?*/ 				if( nPoints )
/*?*/ 				{
/*?*/ 					if( nPoints > 1 )
/*?*/ 					{
/*?*/ 						if( IsSplineChart() )
/*?*/ 						{
/*?*/ 							Polygon aNewPoly( nPoints );
/*?*/ 							for( USHORT i = 0; i < nPoints; i++ )
/*?*/ 								aNewPoly[ i ] = aPolygon[ i ];
/*?*/ 
/*?*/ 							XPolygon aXPoly( aNewPoly );
/*?*/ 							if ((eChartStyle == CHSTYLE_2D_CUBIC_SPLINE) ||
/*?*/ 								(eChartStyle == CHSTYLE_2D_CUBIC_SPLINE_SYMBOL))
/*?*/ 							{
/*?*/ 								CubicSpline( aXPoly, nPoints - 1, nGranularity, *pSpline );
/*?*/ 							}
/*?*/ 							else
/*?*/ 							{
/*?*/ 								approxMesh( nGranularity, *pSpline, aXPoly, nPoints - 1, nSplineDepth );
/*?*/ 							}
/*?*/ 
/*?*/ 							XPolygon aSplinePoly( *pSpline );
/*?*/ 							aSplinePoly.SetSize( (nPoints - 1) * nGranularity );
/*?*/ 
/*?*/ 							// #67488# crop polygon
/*?*/ 							XPolyPolygon aResult;
/*?*/ 							SchCalculationHelper::IntersectPolygonWithRectangle( aSplinePoly, aClipRect, aResult );
/*?*/ 							pObj = new SdrPathObj( OBJ_PLIN, aResult );
/*?*/ 						}
/*?*/ 						else
/*?*/ 						{
/*?*/ 							Polygon aNewPoly( nPoints );
/*?*/ 							for( USHORT i = 0; i < nPoints; i++ )
/*?*/ 								aNewPoly[ i ] = aPolygon[ i ];
/*?*/ 
/*?*/ 							// #67488# crop polygon
/*?*/ 							XPolyPolygon aResult;
/*?*/ 							XPolygon aXPoly( aNewPoly );
/*?*/ 
/*?*/ 							SchCalculationHelper::IntersectPolygonWithRectangle( aXPoly, aClipRect, aResult );
/*?*/ 							pObj = new SdrPathObj( OBJ_PLIN, aResult );
/*?*/ 						}
/*?*/ 
/*?*/ 						pObj->InsertUserData( new SchObjectId( CHOBJID_DIAGRAM_ROWSLINE ));
/*?*/ 						pObj->InsertUserData( new SchDataRow( (short)nRow ));
/*?*/ 						pRowLists[ nRow ]->NbcInsertObject( pObj, 0 ); //#54870# put line to background, so symbol is in front
/*?*/ 
/*?*/ //-/						pObj->NbcSetAttributes( rDataRowAttr, FALSE );
/*?*/ 						pObj->SetItemSet( rDataRowAttr);
/*?*/ 
/*?*/ 					}
/*?*/ 				}
/*?*/ 				nPoints = 0;
/*N*/ 			}
/*N*/ 
/*N*/ 			fPos += fPartWidth;
/*N*/ 			nPos = long(fPos);
/*N*/ 		} //for nCol
/*N*/ 
/*N*/ 		if( nPoints )
/*N*/ 		{
/*N*/ 			pObj = NULL;
/*N*/ 
/*N*/ 			if( IsSplineChart() )
/*N*/ 			{
/*N*/ 				if( nPoints > 1 )
/*N*/ 				{
/*N*/ 					Polygon aNewPoly( nPoints );
/*N*/ 					for( USHORT i = 0; i < nPoints; i++ )
/*N*/ 						aNewPoly[ i ] = aPolygon[ i ];
/*N*/ 
/*N*/ 					XPolygon aXPoly( aNewPoly );
/*N*/ 					if ((eChartStyle == CHSTYLE_2D_CUBIC_SPLINE) ||
/*N*/ 						(eChartStyle == CHSTYLE_2D_CUBIC_SPLINE_SYMBOL))
/*N*/ 					{
/*N*/ 						CubicSpline( aXPoly, nPoints - 1,nGranularity, *pSpline );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						approxMesh( nGranularity, *pSpline, aXPoly, nPoints - 1, nSplineDepth );
/*N*/ 					}
/*N*/ 
/*N*/ 					XPolygon aSplinePoly( *pSpline );
/*N*/ 					aSplinePoly.SetSize( (nPoints - 1) * nGranularity );
/*N*/ 
/*N*/ 					// #67488# crop polygon
/*N*/ 					XPolyPolygon aResult;
/*N*/ 					SchCalculationHelper::IntersectPolygonWithRectangle( aSplinePoly, aClipRect, aResult );
/*N*/ 					pObj = new SdrPathObj( OBJ_PLIN, aResult );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if( bArea )
/*N*/ 				{
/*N*/ 					Polygon aNewPoly( nPoints + 3 );
/*N*/ 					for( USHORT i = 0; i < nPoints + 3; i++ )
/*N*/ 						aNewPoly[ i ] = aPolygon[ i ];
/*N*/ 					XPolygon aBase( 2 );
/*N*/ 
/*N*/ 					pAxis->GridLine( aBase, pAxis->GetPosOrigin() );
/*N*/ 					aNewPoly[ nPoints ]		= aBase[ 1 ];
/*N*/ 					aNewPoly[ nPoints + 1 ]	= aBase[ 0 ];
/*N*/ 					aNewPoly[ nPoints + 2 ]	= aNewPoly[ 0 ];
/*N*/ 
/*N*/ 					// #67488# crop polygon
/*N*/ 					aNewPoly.Clip( aClipRect );
/*N*/ 					XPolygon aXPoly( aNewPoly );
/*N*/ 
/*N*/ 					pObj = new SdrPathObj( OBJ_POLY, aXPoly );
/*N*/ 					pObj->SetModel( this );
/*N*/ 					SetObjectAttr( pObj,CHOBJID_DIAGRAM_ROWS, TRUE, TRUE, (SfxItemSet *)&rDataRowAttr );
/*N*/ 				}
/*N*/ 				else if( nPoints > 1 )			// line
/*N*/ 				{
/*N*/ 					// #67488# crop polygon
/*N*/ 					Polygon aNewPoly( nPoints );
/*N*/ 					for( USHORT i = 0; i < nPoints; i++ )
/*N*/ 						aNewPoly[ i ] = aPolygon[ i ];
/*N*/ 
/*N*/ 					XPolyPolygon aResult;
/*N*/ 					XPolygon aXPoly( aNewPoly );
/*N*/ 
/*N*/ 					SchCalculationHelper::IntersectPolygonWithRectangle( aXPoly, aClipRect, aResult );
/*N*/ 					pObj = new SdrPathObj( OBJ_PLIN, aResult );
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if( pObj )
/*N*/ 			{
/*N*/ 				pObj->InsertUserData( new SchObjectId( CHOBJID_DIAGRAM_ROWSLINE ));
/*N*/ 
/*N*/ //-/				pObj->NbcSetAttributes( rDataRowAttr, FALSE );
/*N*/ 				pObj->SetItemSet( rDataRowAttr);
/*N*/ 
/*N*/ 				pObj->InsertUserData( new SchDataRow( (short)nRow ));
/*N*/ 
/*N*/ 				if( ! bArea || (bArea && IsStacked()) )
/*N*/ 					pRowLists[ nRow ]->NbcInsertObject( pObj, 0 );
/*N*/ 				else
/*N*/ 					pRowLists[ nRow ]->NbcInsertObject( pObj );
/*N*/ 			}
/*N*/ 		} // if( nPoints )
/*N*/ 	} //for nRow
/*N*/ 
/*N*/ 	delete pSpline;
/*N*/ 
/*N*/ 	//Ab hier wieder wie in Create2DRow(Area)Chart:
/*N*/ 
/*N*/ 	aDescr.Build(TRUE);
/*N*/ 	delete[] pRowLists;
/*N*/ 	delete[] pStatLists;
/*N*/ 	return pGroup;
/*N*/ }

/*N*/ bool ChartModel::UsesSourceFormat( long nAxisUID, SfxItemSet** pItemSetPointer /* = NULL */ )
/*N*/ {
/*N*/     const SfxPoolItem*    pPoolItem     = NULL;
/*N*/           ChartAxis*      pAxis         = NULL;
/*N*/           SfxItemSet*     pItemSet      = NULL;
/*N*/ 
/*N*/     if( ( pAxis = GetAxisByUID( nAxisUID )) &&
/*N*/         ( pItemSet = pAxis->GetItemSet()))
/*N*/     {
/*N*/         if( pItemSetPointer )
/*N*/             *pItemSetPointer = pItemSet;
/*N*/ 
/*N*/         // return true if item is not set (default) or it is set to TRUE
/*N*/         return ( ( SFX_ITEM_SET !=
/*N*/                    pItemSet->GetItemState( SID_ATTR_NUMBERFORMAT_SOURCE, FALSE, &pPoolItem )) ||
/*N*/                  ( SAL_STATIC_CAST( const SfxBoolItem*, pPoolItem )->GetValue() == TRUE ));
/*N*/     }
/*N*/ 
/*N*/     return false;
/*N*/ }

// checks if the numberformat was set in MemChart (by Calc/Writer)
// if this is the case the axis attributes are overwritten and TRUE is returned
/*N*/ BOOL ChartModel::CheckForNewAxisNumFormat()
/*N*/ {
/*N*/     //	Don't set the numberformater for a percent chart.
/*N*/     //  or when formats are not set from the outside
/*N*/     if ( IsPercentChart() ||
/*N*/          UsesOwnNumberFormatter() )
/*N*/     	return FALSE;
/*N*/ 
/*N*/     // if source format is used, update
/*N*/     const SfxPoolItem*    pPoolItem  = NULL;
/*N*/           SfxItemSet*     pItemSet   =  NULL;
/*N*/           SfxItemSet*     pItemSetA  =  NULL;
/*N*/           bool            bRet       =  false;
/*N*/           long            nRow       =  0;
/*N*/           INT32           nFmt       = -1;
/*N*/           USHORT          nSchattr   =  IsPercentChart()
/*N*/               ? SCHATTR_AXIS_NUMFMTPERCENT
/*N*/               : SCHATTR_AXIS_NUMFMT;
/*N*/ 
/*N*/     // x axis
/*N*/     if( IsXYChart())
/*N*/     {
/*N*/         bool bXUsesSrcFmt = UsesSourceFormat( CHAXIS_AXIS_X, &pItemSet );
/*N*/         bool bAUsesSrcFmt = UsesSourceFormat( CHAXIS_AXIS_A, &pItemSetA );
/*N*/         if( bXUsesSrcFmt || bAUsesSrcFmt )
/*N*/         {
/*N*/             // use source format for x-axis
/*N*/             nFmt = IsDataSwitched()
/*N*/                 ? pChartData->GetTransNumFormatIdCol( nRow )
/*N*/                 : pChartData->GetTransNumFormatIdRow( nRow );
/*N*/         }
/*N*/ 
/*N*/         if( bXUsesSrcFmt && pItemSet && nFmt != -1 )
/*N*/         {
/*N*/             pItemSet->Put( SfxUInt32Item( nSchattr, nFmt ));
/*N*/             bRet = true;
/*N*/             nXLastNumFmt = nFmt;    // for binary file format only
/*N*/         }
/*N*/         if( bAUsesSrcFmt && pItemSetA && nFmt != -1 )
/*N*/         {
/*N*/             pItemSetA->Put( SfxUInt32Item( nSchattr, nFmt ));
/*N*/             bRet = true;
/*N*/             nXLastNumFmt = nFmt;
/*N*/         }
/*N*/         nRow++;
/*N*/     }
/*N*/ 
/*N*/ 	if( HasSecondYAxis() )	// check both y axes
/*N*/     {
/*N*/         // seek first series using second axis
/*N*/         bool    bYAxisFound     = false;
/*N*/         bool    bBAxisFound     = false;
/*N*/         long    nMaxRow         = GetRowCount();
/*N*/ 
/*N*/         for( ; nRow < nMaxRow; nRow++ )
/*N*/         {
/*N*/             if( ! bBAxisFound &&
/*N*/                 SFX_ITEM_SET ==
/*N*/                 GetDataRowAttr( nRow ).GetItemState( SCHATTR_AXIS, FALSE, &pPoolItem ) &&
/*N*/                 SAL_STATIC_CAST( const SfxInt32Item*, pPoolItem )->GetValue() == CHART_AXIS_SECONDARY_Y )
/*N*/             {
/*N*/                 if( UsesSourceFormat( CHAXIS_AXIS_B, &pItemSet ) &&
/*N*/                     pItemSet )
/*N*/                 {
/*N*/                     // use source format for secondary y-axis
/*N*/                     nFmt = IsDataSwitched()
/*N*/                         ? pChartData->GetTransNumFormatIdCol( nRow )
/*N*/                         : pChartData->GetTransNumFormatIdRow( nRow );
/*N*/ 
/*N*/                     if( nFmt != -1 )
/*N*/                     {
/*N*/                         pItemSet->Put( SfxUInt32Item( nSchattr, nFmt ));
/*N*/                         bRet = true;
/*N*/                         nBLastNumFmt = nFmt;    // for binary file format only
/*N*/                     }
/*N*/                 }
/*N*/                 bBAxisFound = true;
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 if( ! bYAxisFound &&
/*N*/                     UsesSourceFormat( CHAXIS_AXIS_Y, &pItemSet ) &&
/*N*/                     pItemSet )
/*N*/                 {
/*N*/                     // use source format for primary  y-axis
/*N*/                     nFmt = IsDataSwitched()
/*N*/                         ? pChartData->GetTransNumFormatIdCol( nRow )
/*N*/                         : pChartData->GetTransNumFormatIdRow( nRow );
/*N*/ 
/*N*/                     if( nFmt != -1 )
/*N*/                     {
/*N*/                         pItemSet->Put( SfxUInt32Item( nSchattr, nFmt ));
/*N*/                         bRet = true;
/*N*/                         nYLastNumFmt = nFmt;    // for binary file format only
/*N*/                     }
/*N*/                 }
/*N*/                 bYAxisFound = true;
/*N*/             }
/*N*/ 
/*N*/             if( bYAxisFound && bBAxisFound )
/*N*/                 break;
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         // get first setting
/*N*/         if( nRow < GetRowCount() &&
/*N*/             UsesSourceFormat( CHAXIS_AXIS_Y, &pItemSet ) &&
/*N*/             pItemSet )
/*N*/         {
/*N*/             nFmt = IsDataSwitched()
/*N*/                 ? pChartData->GetTransNumFormatIdCol( nRow )
/*N*/                 : pChartData->GetTransNumFormatIdRow( nRow );
/*N*/ 
/*N*/             if( nFmt != -1 )
/*N*/             {
/*N*/                 pItemSet->Put( SfxUInt32Item( nSchattr, nFmt ));
/*N*/                 bRet = true;
/*N*/                 nYLastNumFmt = nFmt;    // for binary file format only
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     return bRet;
/*N*/ }

/*N*/ BOOL ChartModel::HasDefaultGrayArea( SvxChartStyle* pStyle ) const
/*N*/ {
/*N*/ 	SvxChartStyle eStyle = pStyle? *pStyle: eChartStyle;
/*N*/ 
/*N*/ 	switch( eStyle )
/*N*/ 	{
/*N*/ 		case CHSTYLE_2D_NET:
/*N*/ 		case CHSTYLE_2D_NET_SYMBOLS:
/*N*/ 		case CHSTYLE_2D_NET_STACK:
/*N*/ 		case CHSTYLE_2D_NET_SYMBOLS_STACK:
/*N*/ 		case CHSTYLE_2D_NET_PERCENT:
/*N*/ 		case CHSTYLE_2D_NET_SYMBOLS_PERCENT:
/*N*/ 			return TRUE;
/*N*/ 
/*N*/ 		default:
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ChartModel::HasDefaultGrayWall( SvxChartStyle* pStyle ) const
/*N*/ {
/*N*/ 	SvxChartStyle eStyle = pStyle? *pStyle: eChartStyle;
/*N*/ 
/*N*/ 	switch( eStyle )
/*N*/ 	{
/*N*/ 		case CHSTYLE_2D_LINE:
/*N*/ 		case CHSTYLE_2D_STACKEDLINE:
/*N*/ 		case CHSTYLE_2D_PERCENTLINE:
/*N*/ 		case CHSTYLE_2D_XY:
/*N*/ 		case CHSTYLE_2D_LINESYMBOLS:
/*N*/ 		case CHSTYLE_2D_STACKEDLINESYM:
/*N*/ 		case CHSTYLE_2D_PERCENTLINESYM:
/*N*/ 		case CHSTYLE_2D_XYSYMBOLS:
/*N*/ 		case CHSTYLE_2D_CUBIC_SPLINE:
/*N*/ 		case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL:
/*N*/ 		case CHSTYLE_2D_B_SPLINE:
/*N*/ 		case CHSTYLE_2D_B_SPLINE_SYMBOL:
/*N*/ 		case CHSTYLE_2D_CUBIC_SPLINE_XY:
/*N*/ 		case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY:
/*N*/ 		case CHSTYLE_2D_B_SPLINE_XY:
/*N*/ 		case CHSTYLE_2D_B_SPLINE_SYMBOL_XY:
/*N*/ 		case CHSTYLE_2D_XY_LINE:
/*N*/ 			return TRUE;
/*N*/ 
/*N*/ 		default:
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ using namespace ::com::sun::star;

/*N*/ void ChartModel::SetNumberFormatter( SvNumberFormatter* pFormatter )
/*N*/ {
/*N*/ 	pNumFormatter = pFormatter;
/*N*/ 
/*N*/ 	pChartXAxis->SetNumberFormatter( pNumFormatter );
/*N*/ 	pChartYAxis->SetNumberFormatter( pNumFormatter );
/*N*/ 	pChartZAxis->SetNumberFormatter( pNumFormatter );
/*N*/ 	pChartAAxis->SetNumberFormatter( pNumFormatter );
/*N*/ 	pChartBAxis->SetNumberFormatter( pNumFormatter );
/*N*/ 
/*N*/     // update UNO Numberformatter Wrapper
/*N*/     SfxObjectShell* pMyDocShell = GetObjectShell();
/*N*/     if( pMyDocShell != NULL )
/*N*/     {
/*N*/         ChXChartDocument* pUnoDoc = ChXChartDocument::getImplementation( pMyDocShell->GetModel());
/*N*/         DBG_ASSERT( pUnoDoc, "Couldn't get XModel implementation" );
/*N*/         if( pUnoDoc != NULL )
/*N*/         {
/*N*/             // invalidate old number formatter
/*N*/             pUnoDoc->ClearNumberFormatter();
/*N*/         }
/*N*/ 	}
/*N*/ }

/*N*/ void ChartModel::TranslateAllNumFormatIds( SvNumberFormatterIndexTable* pTransTable )
/*N*/ {
/*N*/ 	if( pChartXAxis->TranslateMergedNumFormat( pTransTable ) &&
/*N*/         nXLastNumFmt >= 0 )
/*N*/   {DBG_BF_ASSERT(0, "STRIP");} //STRIP001       nXLastNumFmt = pChartXAxis->GetNumFormat();
/*N*/ 
/*N*/     if( pChartYAxis->TranslateMergedNumFormat( pTransTable ) &&
/*N*/         nYLastNumFmt >= 0 )
/*N*/     {DBG_BF_ASSERT(0, "STRIP");} //STRIP001     nYLastNumFmt = pChartYAxis->GetNumFormat();
/*N*/ 
/*N*/ 	pChartZAxis->TranslateMergedNumFormat( pTransTable );
/*N*/ 	pChartAAxis->TranslateMergedNumFormat( pTransTable );
/*N*/ 
/*N*/ 	if( pChartBAxis->TranslateMergedNumFormat( pTransTable ) &&
/*N*/         nBLastNumFmt >= 0 )
/*N*/   {DBG_BF_ASSERT(0, "STRIP");} //STRIP001       nBLastNumFmt = pChartBAxis->GetNumFormat();
/*N*/ }

/*N*/ void ChartModel::SetNumLinesColChart(const long nSet, BOOL bForceStyleChange)
/*N*/ {
/*N*/     const long nOldNumLines = nNumLinesInColChart;
/*N*/     const long nLastSeries = GetRowCount() - 1;
/*N*/ 
/*N*/     switch( eChartStyle  )
/*N*/ 	{
/*N*/ 		case CHSTYLE_2D_COLUMN:					// only types supporting mixing with lines
/*N*/   		case CHSTYLE_2D_STACKEDCOLUMN:
/*N*/ 		case CHSTYLE_2D_PERCENTCOLUMN:
/*N*/ 		case CHSTYLE_2D_LINE_COLUMN:
/*N*/ 		case CHSTYLE_2D_LINE_STACKEDCOLUMN:
/*N*/ 			if( bForceStyleChange )
/*N*/ 			{
/*N*/ 				if( nSet <= 0 )
/*N*/ 				{
/*N*/ 					nNumLinesInColChart = 0;
/*N*/ 					if( eChartStyle == CHSTYLE_2D_LINE_COLUMN )
/*?*/ 						eChartStyle = CHSTYLE_2D_COLUMN;
/*N*/ 					else if( eChartStyle == CHSTYLE_2D_LINE_STACKEDCOLUMN )
/*?*/ 						eChartStyle = CHSTYLE_2D_STACKEDCOLUMN;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					nNumLinesInColChart = nSet;
/*N*/ 
/*N*/ 					if( eChartStyle == CHSTYLE_2D_COLUMN )
/*N*/ 						eChartStyle = CHSTYLE_2D_LINE_COLUMN;
/*N*/ 					else if( eChartStyle == CHSTYLE_2D_STACKEDCOLUMN )
/*?*/ 						eChartStyle = CHSTYLE_2D_LINE_STACKEDCOLUMN;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				nNumLinesInColChart = nSet > 0 ? nSet : 0;
/*N*/ 			}
/*N*/             break;
/*N*/ 
/*N*/         default:
/*N*/ //             DBG_ASSERT( nSet == 0, "Trying to set number of lines to value >0 for wrong chart type" );
/*N*/             nNumLinesInColChart = nSet > 0 ? nSet : 0;
/*N*/             break;
/*N*/ 	}
/*N*/ 
/*N*/     // #101164# map fill-/line colors
/*N*/     if( nNumLinesInColChart > nOldNumLines )
/*N*/     {
/*N*/         for( int i = nOldNumLines ;
/*N*/              i < nNumLinesInColChart && i < nLastSeries ;
/*N*/              ++i )
/*N*/         {
/*N*/             SfxItemSet * pSet = aDataRowAttrList.GetObject( nLastSeries - i );
/*N*/             OSL_ASSERT( pSet );
/*N*/             pSet->Put( XLineColorItem(
/*N*/                            String(),
/*N*/                            static_cast< const XFillColorItem & >(
/*N*/                                pSet->Get( XATTR_FILLCOLOR )).GetValue() ));
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         for( int i = nNumLinesInColChart ;
/*N*/              i < nOldNumLines && i < nLastSeries ;
/*N*/              ++i )
/*N*/         {
/*?*/             SfxItemSet * pSet = aDataRowAttrList.GetObject( nLastSeries - i );
/*?*/             OSL_ASSERT( pSet );
/*?*/             pSet->Put( XFillColorItem(
/*?*/                            String(),
/*?*/                            static_cast< const XLineColorItem & >(
/*?*/                                pSet->Get( XATTR_LINECOLOR )).GetValue() ));
/*?*/             pSet->Put( XLineColorItem( String(), RGBColor( COL_BLACK ) ));
/*N*/         }
/*N*/     }
/*N*/ }

/*N*/ BOOL ChartModel::SetObjectShell( SfxObjectShell* pDocSh )
/*N*/ {
/*N*/ 	if( ! pDocShell )
/*N*/ 	{
/*N*/ 		pDocShell = pDocSh;
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ using namespace ::com::sun::star;

/*N*/ void ChartModel::SetChartAddIn( uno::Reference< util::XRefreshable >& xChartAddIn )
/*N*/ {
/*N*/ 	if( xChartAddIn.is())
/*N*/ 	{
/*N*/ 		// acquire add-in
/*?*/ 		mxChartAddIn = xChartAddIn;
/*?*/ 		ChangeChart( CHSTYLE_ADDIN );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// release add-in
/*N*/ 		mxChartAddIn = NULL;
/*N*/ 		// if base type was not set, use default
/*N*/ 		if( eChartStyle == CHSTYLE_ADDIN )
/*?*/ 			ChangeChart( CHSTYLE_2D_BAR );
/*N*/ 	}
/*N*/ }


/*N*/ void ChartModel::HandsOff()
/*N*/ {
/*N*/     mxPictureStorage = SotStorageRef();
/*N*/ 	mpDocStor = NULL;
/*N*/ }


/*N*/ uno::Sequence< uno::Sequence< sal_Int32 > > ChartModel::GetSetDataPointList()
/*N*/ {
/*N*/     const ItemSetList & rAttrList = IsDataSwitched()
/*N*/         ? aSwitchDataPointAttrList
/*N*/         : aDataPointAttrList;
/*N*/     const long nSize = rAttrList.Count();
/*N*/     long nRow, nCol;
/*N*/ 	long nColCnt = GetColCount();
/*N*/     long nRowCnt = GetRowCount();
/*N*/ 
/*N*/     DBG_ASSERT( nSize == ( nRowCnt * nColCnt ), "Data-Point list has invalid size!" );
/*N*/ 
/*N*/     // the 'outer' sequence contains one sequence for each series
/*N*/     uno::Sequence< uno::Sequence< sal_Int32 > > aResult( nRowCnt );
/*N*/ 
/*N*/     // here we need again a special treatment for pie charts as a single pie
/*N*/     // uses merged data point and data row attributes the data points are also
/*N*/     // set if data row attributes are set, which is always the case
/*N*/     if( IsPieChart())
/*N*/     {
/*N*/         aResult[ 0 ].realloc( nColCnt );
/*N*/         sal_Int32 * pArray = aResult[ 0 ].getArray();
/*N*/ 
/*N*/         for( nCol = 0; nCol < nColCnt; ++nCol )
/*N*/             pArray[ nCol ] = nCol;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         // use a vector for quick dynamic resizing
/*N*/         ::std::list< sal_Int32 > aList;
/*N*/ 
/*N*/         for( nRow = 0; nRow < nRowCnt; ++nRow )
/*N*/         {
/*N*/             for( nCol = 0; nCol < nColCnt; ++nCol )
/*N*/             {
/*N*/                 if( rAttrList.GetObject( nCol * nRowCnt + nRow ) != NULL )
/*N*/                     aList.push_back( nCol );
/*N*/             }
/*N*/ 
/*N*/             if( aList.size() > 0 )
/*N*/             {
/*N*/                 // copy list to 'inner' sequence
/*N*/                 aResult[ nRow ].realloc( aList.size());
/*N*/                 sal_Int32 * pArray = aResult[ nRow ].getArray();
/*N*/                 sal_Int32 i = 0;
/*N*/                 ::std::list< sal_Int32 >::iterator aIter;
/*N*/                 for( aIter = aList.begin(); aIter != aList.end(); ++aIter )
/*N*/                 {
/*N*/                     pArray[ i++ ] = (*aIter);
/*N*/                 }
/*N*/                 aList.clear();
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     return aResult;
/*N*/ }

/*N*/ void ChartModel::PageColorChanged( const SfxItemSet& rItems )
/*N*/ {
/*N*/     Color aColor;
/*N*/ 
/*N*/     // GetDraftFillColor is defined in bf_svx/svdetc.hxx
/*N*/     if( GetDraftFillColor( rItems, aColor ))
/*N*/     {
/*N*/         SdrOutliner *  pOut = GetOutliner();
/*N*/         if( pOut )
/*N*/             pOut->SetBackgroundColor( aColor );
/*N*/         GetDrawOutliner().SetBackgroundColor( aColor );
/*N*/     }
/*N*/ }

}
