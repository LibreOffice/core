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

#define RAD2CDEG(fAngle) ( (long)(((fAngle)*18000.0/F_PI)+36000.0)%36000 )



#include "schattr.hxx"
#define ITEMID_ADJUST EE_PARA_JUST

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_CHARTDATADESCR	SCHATTR_DATADESCR_DESCR
#define ITEMID_CHARTTEXTORIENT	SCHATTR_TEXT_ORIENT

#include <bf_svtools/eitem.hxx>

#endif

#include <globfunc.hxx>
#include "axisid.hxx"
#include "chtscene.hxx"

#undef	ITEMID_COLOR		//	Defined in svx3ditems.hxx
#define ITEMID_COLOR       EE_CHAR_COLOR

#include <bf_svx/svdocirc.hxx>
#include "math.h"
#include "glob.hrc"
#include "float.h"

#ifndef _ZFORLIST_HXX //autogen
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <bf_svtools/zforlist.hxx>
#endif
#include <bf_svx/svdopath.hxx>
#include <bf_svx/xlnwtit.hxx>

#include "pairs.hxx"
#include "chmod3d.hxx"
#include "chaxis.hxx"
#include "chdescr.hxx"
#define FIXED_SIZE_FOR_3D_CHART_VOLUME			(10000)

namespace binfilter {

/*************************************************************************
|*
|* Erzeugt alle 3d-Achsentitel und setzt sie sofort ins Diagramm
|* Positioniert werden sie von der Chartszene
|*
\************************************************************************/

/*N*/ void ChartModel::CreateAndInsert3DAxesTitles (Rectangle &rRect, BOOL bSwitchColRow)
/*N*/ {
/*N*/ 	SdrTextObj *pXAxisTitleObj = NULL;
/*N*/ 	SdrTextObj *pYAxisTitleObj = NULL;
/*N*/ 	SdrTextObj *pZAxisTitleObj = NULL;
/*N*/ 
/*N*/ 	if (bShowXAxisTitle)
/*N*/ 	{
/*N*/ 		pXAxisTitleObj = CreateTitle (pXAxisTitleAttr, CHOBJID_DIAGRAM_TITLE_X_AXIS,
/*N*/ 												   bSwitchColRow,aXAxisTitle,
/*N*/ 												   FALSE, &eAdjustXAxesTitle);
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
/*N*/ 												   bSwitchColRow, aYAxisTitle,
/*N*/ 												   TRUE, &eAdjustYAxesTitle);
/*N*/ 		if (GetAdjustMarginsForYAxisTitle())
/*N*/ 		{
/*N*/ 			if (bSwitchColRow)
/*N*/ 				rRect.Bottom() -= GetOutputSize(*pYAxisTitleObj).Height() + 200;
/*N*/ 			else
/*N*/ 				rRect.Left() += GetOutputSize(*pYAxisTitleObj).Width() + 200;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (bShowZAxisTitle)
/*N*/ 	{
/*N*/ 		pZAxisTitleObj = CreateTitle (pZAxisTitleAttr, CHOBJID_DIAGRAM_TITLE_Z_AXIS,
/*N*/ 			bSwitchColRow,aZAxisTitle, FALSE, &eAdjustZAxesTitle);
/*N*/ 		if( GetAdjustMarginsForZAxisTitle())
/*N*/         {
/*N*/             rRect.Right() -= GetOutputSize(*pZAxisTitleObj).Width() + 200;
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrPage* pPage=GetPage(0);
/*N*/ 	if (pXAxisTitleObj) pPage->NbcInsertObject( pXAxisTitleObj );
/*N*/ 	if (pYAxisTitleObj) pPage->NbcInsertObject( pYAxisTitleObj );
/*N*/ 	if (pZAxisTitleObj) pPage->NbcInsertObject( pZAxisTitleObj );
/*N*/ 
/*N*/     // this should resize the scene according to the space needed
/*N*/     // by the titles. However this results in strange effects like
/*N*/     // constant resizing until the scene has 0 size and also the title
/*N*/     // objects disappear when being moved (?).
/*N*/ //     GetScene()->NbcSetSnapRect( rRect );
/*N*/ }

/*************************************************************************
|*
|* 3D-Rueckwaende mit Unterteilung und Beschriftung erzeugen
|*
\************************************************************************/
/*N*/ void ChartModel::Create3DBackplanes (Rectangle		&rRect,
/*N*/ 									 Vector3D       aPos,
/*N*/ 									 Vector3D       aSizeVec,
/*N*/ 									 ChartScene     &rScene,
/*N*/ 									 BOOL           bPartDescr,
/*N*/ 									 BOOL           bXLogarithm,
/*N*/ 									 BOOL           bYLogarithm,
/*N*/ 									 BOOL           bZLogarithm,
/*N*/ 									 USHORT         eStackMode,
/*N*/ 									 BOOL           bPercent,
/*N*/ 									 BOOL           bFlatChart,
/*N*/ 									 BOOL           bSwitchColRow)
/*N*/ {
/*N*/ 	//vorlaeufiger Hack, bis logarithmus-Ueberpruefung eingebaut (autokorrektur):
/*N*/ 	pChartXAxis->GetItemSet()->Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM,bXLogarithm));
/*N*/ 	pChartYAxis->GetItemSet()->Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM,bYLogarithm));
/*N*/ 	pChartZAxis->GetItemSet()->Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM,bZLogarithm));
/*N*/ 
/*N*/ 	String     aNumStr;
/*N*/ 
/*N*/ 	E3dObject* pXGridMainGroup = bShowXGridMain
/*N*/ 											 ? Create3DObject (CHOBJID_DIAGRAM_X_GRID_MAIN_GROUP)
/*N*/ 											 : 0;
/*N*/ 	E3dObject* pYGridMainGroup = bShowYGridMain
/*N*/ 											 ? Create3DObject (CHOBJID_DIAGRAM_Y_GRID_MAIN_GROUP)
/*N*/ 											 : 0;
/*N*/ 	E3dObject* pZGridMainGroup = bShowZGridMain
/*N*/ 											 ? Create3DObject (CHOBJID_DIAGRAM_Z_GRID_MAIN_GROUP)
/*N*/ 											 : 0;
/*N*/ 	E3dObject* pXGridHelpGroup = bShowXGridHelp
/*N*/ 											 ? Create3DObject (CHOBJID_DIAGRAM_X_GRID_HELP_GROUP)
/*N*/ 											 : 0;
/*N*/ 	E3dObject* pYGridHelpGroup = bShowYGridHelp
/*N*/ 											 ? Create3DObject (CHOBJID_DIAGRAM_Y_GRID_HELP_GROUP)
/*N*/ 											 : 0;
/*N*/ 	E3dObject* pZGridHelpGroup = bShowZGridHelp
/*N*/ 											 ? Create3DObject (CHOBJID_DIAGRAM_Z_GRID_HELP_GROUP)
/*N*/ 											 : 0;
/*N*/ 	E3dObject* pXAxisGroup     = pChartXAxis->IsVisible()|| pChartXAxis->HasDescription()  //#47500#
/*N*/ 											 ? Create3DAxisObj( CHOBJID_DIAGRAM_X_AXIS )
/*N*/ 											 : 0;
/*N*/ 	E3dObject* pYAxisGroup     = pChartYAxis->IsVisible() || pChartYAxis->HasDescription() //#47500#
/*N*/ 											 ? Create3DAxisObj( CHOBJID_DIAGRAM_Y_AXIS )
/*N*/ 											 : 0;
/*N*/ 	E3dObject* pZAxisGroup     = pChartZAxis->IsVisible() || pChartZAxis->HasDescription() //#47500#
/*N*/ 											 ? Create3DAxisObj( CHOBJID_DIAGRAM_Z_AXIS )
/*N*/ 											 : 0;
/*N*/ 	Polygon3D    aRect3D;
/*N*/ 	
/*N*/ 	pChartXAxis->Initialise(rRect,bSwitchColRow,eStackMode,bPercent,FALSE);
/*N*/ 	pChartYAxis->Initialise(rRect,bSwitchColRow,eStackMode,bPercent,FALSE);
/*N*/ 	pChartZAxis->Initialise(rRect,bSwitchColRow,eStackMode,bPercent,FALSE);
/*N*/ 
/*N*/ 	pChartXAxis->CalcValueSteps();
/*N*/ 	pChartYAxis->CalcValueSteps();
/*N*/ 	pChartZAxis->CalcValueSteps();
/*N*/ 
/*N*/ //Vorlaeufige Notloesung, entsprechende Stellen sollen noch geaendert werden, daher bitte kein Search&Repleace statt der #defs!
/*N*/ #define	fOriginY		pChartYAxis->GetOrigin()
/*N*/ #define fMinValueY		pChartYAxis->GetMin()
/*N*/ #define fMaxValueY		pChartYAxis->GetMax()
/*N*/ #define fValueStepMainY pChartYAxis->GetStep()
/*N*/ #define fValueStepHelpY pChartYAxis->GetHelpStep()
/*N*/ 
/*N*/ 	//ToDo: vorlaeufig
/*N*/ 	SfxItemSet* pXAxisAttr=&GetAttr(CHOBJID_DIAGRAM_X_AXIS);
/*N*/ 	SfxItemSet* pYAxisAttr=&GetAttr(CHOBJID_DIAGRAM_Y_AXIS);
/*N*/ 	SfxItemSet* pZAxisAttr=&GetAttr(CHOBJID_DIAGRAM_Z_AXIS);
/*N*/ 
/*N*/ 	SvxChartTextOrient eDescrOrient = ((const SvxChartTextOrientItem&)pXAxisAttr->Get(SCHATTR_TEXT_ORIENT)).GetValue();
/*N*/ 	SvxChartTextOrient eValueOrient = ((const SvxChartTextOrientItem&)pYAxisAttr->Get(SCHATTR_TEXT_ORIENT)).GetValue();
/*N*/ 
/*N*/ 	Size aMaxValueSizeY = pChartYAxis->CalcMaxTextSize(eValueOrient);
/*N*/ 	Size aMaxValueSizeX = pChartXAxis->CalcMaxTextSize(eDescrOrient);
/*N*/ 
/*N*/ 	Size aMaxDescrSizeY = CalcMaxDescrSize( TRUE,  eValueOrient, bPercent, CHAXIS_AXIS_Y );
/*N*/ 	Size aMaxDescrSizeX = CalcMaxDescrSize( FALSE, eDescrOrient, bPercent, CHAXIS_AXIS_X );
/*N*/ 
/*N*/ 	short   nV, i;
/*N*/ 
/*N*/ 	long   nRowCnt     = bFlatChart
/*N*/ 							  ? 1
/*N*/ 							  : GetRowCount();
/*N*/ 	long   nBackColCnt = (bPartDescr || GetColCount() > 1)
/*N*/ 							  ? GetColCount()
/*N*/ 							  : GetColCount() + 1;
/*N*/ 	long   nColumnCnt  = bPartDescr
/*N*/ 							  ? nBackColCnt
/*N*/ 							  : nBackColCnt - 1;
/*N*/ 
/*N*/ 
/*N*/ 	// FG: Diese Variable ist ein reiner Zwischenspeicher damit in der Chartscene bei FitInSnapRect
/*N*/ 	//     die Achsentitel gemaess dieses Parameters gesetzt werden koennen.
/*N*/ 	bSwitch3DColRow = bSwitchColRow;
/*N*/ 
/*N*/ 	long nMaxTextWidth = 0;
/*N*/ 
/*N*/ 	aXDescrList.Clear();
/*N*/ 	aYDescrList.Clear();
/*N*/ 	aZDescrList.Clear();
/*N*/ 
/*N*/ 	E3dDefaultAttributes aDefltAttr3D;
/*N*/ 
/*N*/ 	Matrix4D aShift;
/*N*/ 	aShift.Translate(-(aSizeVec/500.0));//Wände etwas verschieben...
/*N*/ 
/*N*/ 	const double fFloorWidth = 100.0;
/*N*/ 	const double fWallWith	 = 50.0;	// BM: unused for now
/*N*/ 	
/*N*/ 	for (nV = 0; nV < 3; nV++)
/*N*/ 	{
/*N*/ 		switch (nV)
/*N*/ 		{
/*N*/ 			case 0:         // X-Achse , XY-Ebene (WALL hinten)
/*N*/ 			case 1:         // Y-Achse , YZ-Ebene (WALL seitlich)
/*N*/ 			{
/*N*/ 				aRect3D[0]    = aPos;
/*N*/ 				aPos[nV]     += aSizeVec[nV];     //nV=0: X(), sonst Y()
/*N*/ 				aRect3D[1]    = aPos;
/*N*/ 				aPos[nV + 1] += aSizeVec[nV + 1]; //nV=0: Y(), sonst Z()
/*N*/ 				aRect3D[2]    = aPos;
/*N*/ 				aPos[nV]     -= aSizeVec[nV];
/*N*/ 				aRect3D[3]    = aPos;
/*N*/ 				aPos[nV + 1] -= aSizeVec[nV + 1];
/*N*/ 
/*N*/ 				E3dPolygonObj *pWallObj = new SchE3dPolygonObj(aDefltAttr3D, aRect3D);
/*N*/ 
/*N*/ 				pWallObj->SetModel (this);
/*N*/ 				pWallObj->InsertUserData(new SchObjectId(CHOBJID_DIAGRAM_WALL));
/*N*/ 				rScene.Insert3DObj(pWallObj);
/*N*/ 
/*N*/ //-/				pWallObj->NbcSetAttributes(*pDiagramWallAttr, FALSE);
/*N*/ 				pWallObj->SetItemSet(*pDiagramWallAttr);
/*N*/ 
/*N*/ 				pWallObj->NbcSetTransform(aShift);
/*N*/ 
/*N*/ 				if (!nV)
/*N*/ 				{
/*N*/ 					// X-Achse zeichnen
/*N*/ 					Vector3D aLine3D [2] = { aRect3D [0], aRect3D [1] };
/*N*/ 
/*N*/ 					//#47500#
/*N*/ 					if (pXAxisGroup && pChartXAxis->IsVisible())
/*N*/ 					{
/*N*/ 						Create3DPolyObject (pXAxisAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*N*/ 							CHOBJID_LINE, pXAxisGroup);
/*N*/ 					}
/*N*/ 
/*N*/ 					if( pYGridMainGroup || pYGridHelpGroup )
/*N*/ 						// Y-Anteile
/*N*/ 						if (bSwitchColRow)
/*N*/ 						{
/*N*/ 							Vector3D aLine3D [2] = { aRect3D [0], aRect3D [1] };
/*N*/ 							long     nStepMainY  = (long) aSizeVec.Y () / nColumnCnt;
/*N*/ 							long     nStepHelpY  = nStepMainY / 2;
/*N*/ 
/*N*/ 							// hauptgitter auf der X-Ebene, parallel zur X-Achse
/*N*/ 							for (i = 0; i <= nColumnCnt; i++)
/*N*/ 							{
/*N*/ 								aLine3D[0].Y() =
/*N*/ 								aLine3D[1].Y() = aPos.Y() + nStepMainY * i;
/*N*/ 								if( pYGridMainGroup )
/*N*/ 									Create3DPolyObject (pYGridMainAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*N*/ 														CHOBJID_DIAGRAM_Y_GRID_MAIN, pYGridMainGroup);
/*N*/ 
/*N*/ 								// hilfsgitter auf der X-Ebene, parallel zur X-Achse
/*N*/ 								if (pYGridHelpGroup && (i < nColumnCnt))
/*N*/ 								{
/*?*/ 									aLine3D [0].Y () += nStepHelpY;
/*?*/ 									aLine3D [1].Y () += nStepHelpY;
/*?*/ 									Create3DPolyObject (pYGridHelpAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*?*/ 														CHOBJID_DIAGRAM_Y_GRID_HELP, pYGridHelpGroup);
/*N*/ 								}
/*N*/ 							}
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							aLine3D [0] = aRect3D[0];
/*N*/ 							aLine3D [1] = aRect3D[3];
/*N*/ 
/*N*/ 							long nStepMainX  = (long) aSizeVec.X () / nColumnCnt;
/*N*/ 							long nStepHelpX  = nStepMainX / 2;
/*N*/ 
/*N*/ 							// hauptgitter auf der X-Ebene, parallel zur Y-Achse
/*N*/ 							for (i = 0; i <= nColumnCnt; i++)
/*N*/ 							{
/*N*/ 								aLine3D[0].X() =
/*N*/ 								aLine3D[1].X() = aPos.X() + nStepMainX * i;
/*N*/ 								if( pYGridMainGroup )
/*N*/ 									Create3DPolyObject (pYGridMainAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*N*/ 														CHOBJID_DIAGRAM_Y_GRID_MAIN, pYGridMainGroup);
/*N*/ 
/*N*/ 								// hilfsgitter auf der X-Ebene, parallel zur Y-Achse
/*N*/ 								if (pYGridHelpGroup && (i < nColumnCnt))
/*N*/ 								{
/*?*/ 									aLine3D [0].X () += nStepHelpX;
/*?*/ 									aLine3D [1].X () += nStepHelpX;
/*?*/ 									Create3DPolyObject (pYGridHelpAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*?*/ 														CHOBJID_DIAGRAM_Y_GRID_HELP, pYGridHelpGroup);
/*N*/ 								}
/*N*/ 							}
/*N*/ 						}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// Y-Achse zeichnen
/*N*/ 					Vector3D aLine3D [2] = { aRect3D [0], aRect3D [1] };
/*N*/ 
/*N*/ 					// Z-Anteile
/*N*/ 					//#47500#
/*N*/ 					if (pYAxisGroup && pChartYAxis->IsVisible()) Create3DPolyObject (pYAxisAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*N*/ 														 CHOBJID_LINE, pYAxisGroup);
/*N*/ 
/*N*/ 					// hauptgitter auf der Z-Ebene, parallel zur Z-Achse
/*N*/ 					if( pZGridMainGroup || pZGridHelpGroup )
/*N*/ 					{
/*N*/ 						Vector3D aLine3D [2] = { aRect3D[2], aRect3D[3] };
/*N*/ 						long     nStepMainZ  = (long) aSizeVec.Z () / nRowCnt;
/*N*/ 						long     nStepHelpZ  = nStepMainZ / 2;
/*N*/ 
/*N*/ 						BOOL bCreateGridLine = ( (aLine3D[ 0 ].X() != aLine3D[ 1 ].X()) ||
/*N*/ 												 (aLine3D[ 0 ].Y() != aLine3D[ 1 ].Y()) );
/*N*/ 						// Z() values become equal in the for loop
/*N*/ 						// => start and end points would be equal if !bCreateGridLine
/*N*/ 
/*N*/ 						for (i = 0; i <= nRowCnt; i++)
/*N*/ 						{
/*N*/ 							aLine3D[0].Z() =
/*N*/ 							aLine3D[1].Z() = aPos.Z() + nStepMainZ * i;
/*N*/ 							if( pZGridMainGroup && bCreateGridLine )
/*N*/ 								Create3DPolyObject( pZGridMainAttr, new SchE3dPolygonObj( aDefltAttr3D, aLine3D[0], aLine3D[1] ),
/*N*/ 													CHOBJID_DIAGRAM_Z_GRID_MAIN, pZGridMainGroup );
/*N*/ 
/*N*/ 							// hilfsgitter auf der Z-Ebene, parallel zur Z-Achse
/*N*/ 							if (pZGridHelpGroup && (i < nRowCnt))
/*N*/ 							{
/*?*/ 								aLine3D[0].Z() += nStepHelpZ;
/*?*/ 								aLine3D[1].Z() += nStepHelpZ;
/*?*/ 								if( bCreateGridLine )
/*?*/ 									Create3DPolyObject( pZGridHelpAttr, new SchE3dPolygonObj( aDefltAttr3D, aLine3D[0], aLine3D[1] ),
/*?*/ 														CHOBJID_DIAGRAM_Z_GRID_HELP, pZGridHelpGroup );
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				Vector3D   aLine3D [2] = { aRect3D[0], aRect3D[(nV || !nV && bSwitchColRow)
/*N*/ 																   ? 3
/*N*/ 																   : 1] };
/*N*/ 				double     fAct        = fMinValueY;
/*N*/ 
/*N*/ 				SfxItemSet aYTextAttr ((const SfxItemSet &) *pYAxisAttr);
/*N*/ 				aYTextAttr.Put(XLineStyleItem (XLINE_NONE));
/*N*/ 				aYTextAttr.Put(XLineWidthItem (0));
/*N*/ 
/*N*/ 				// create Y grid. Attention: Some variables are called XGrid...
/*N*/ 				if (fMinValueY != fMaxValueY)
/*N*/ 					while (fAct <= fMaxValueY)
/*N*/ 					{
/*N*/ 						double fFact = pChartYAxis->CalcFact(fAct);
/*N*/ 
/*N*/ 						if (bSwitchColRow)
/*N*/ 						{
/*N*/ 							// create major gridline
/*N*/ 							aLine3D[0].X() = aLine3D[1].X() = fFact * aSizeVec.X();
/*N*/ 
/*N*/ 							if (pXGridMainGroup)
/*N*/ 								Create3DPolyObject(pXGridMainAttr, new SchE3dPolygonObj(aDefltAttr3D,
/*N*/ 								aLine3D[0], aLine3D[1]),CHOBJID_DIAGRAM_X_GRID_MAIN,pXGridMainGroup);
/*N*/ 
/*N*/ 							if (pYAxisGroup && pChartYAxis->HasDescription() && nV)
/*N*/ 							{
/*N*/ 								Color *pTextColor = NULL;
/*N*/ 								SvxColorItem *pOldTextColor = NULL;
/*N*/ 
/*N*/ 								pNumFormatter->GetOutputString((bPercent)?fAct/100.0:fAct,
/*N*/ 									GetNumFmt(CHOBJID_DIAGRAM_Y_AXIS,bPercent), aNumStr, &pTextColor);
/*N*/ 
/*N*/ 								if( pTextColor )			// BM #60999#
/*N*/ 								{
/*?*/ 									pOldTextColor = (SvxColorItem*)(aYTextAttr.GetItem( EE_CHAR_COLOR ));
/*?*/ 									aYTextAttr.Put(SvxColorItem( *pTextColor ));
/*N*/ 								}
/*N*/ 
/*N*/ 								SdrTextObj *pTextObj = CreateTextObj(CHOBJID_TEXT, Point (),aNumStr,
/*N*/ 									aYTextAttr, FALSE,CHADJUST_TOP_RIGHT);
/*N*/ 								
/*N*/ 								if( pOldTextColor )
/*N*/ 								{
/*?*/ 									aYTextAttr.Put( *pOldTextColor );
/*N*/ 								}
/*N*/ 
/*N*/ 								E3dLabelObj *pE3DLabel = new E3dLabelObj(aLine3D[1],pTextObj);
/*N*/ 								aYDescrList.Insert(pE3DLabel,LIST_APPEND);
/*N*/ 							}
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							// create major gridline
/*N*/ 							aLine3D[0].Y() = aLine3D[1].Y() = fFact * aSizeVec.Y();
/*N*/ 
/*N*/ 							if(pXGridMainGroup)
/*N*/ 								Create3DPolyObject(pXGridMainAttr,new SchE3dPolygonObj(aDefltAttr3D,
/*N*/ 								aLine3D[0],aLine3D[1]),CHOBJID_DIAGRAM_X_GRID_MAIN, pXGridMainGroup);
/*N*/ 							// Y-Achsenwerte
/*N*/ 							if (pYAxisGroup && pChartYAxis->HasDescription() && nV)
/*N*/ 							{
/*N*/ 								Color *pTextColor = NULL;
/*N*/ 								SvxColorItem *pOldTextColor = NULL;
/*N*/ 
/*N*/ 								pNumFormatter->GetOutputString((bPercent)?fAct/100.0:fAct,
/*N*/ 									GetNumFmt(CHOBJID_DIAGRAM_Y_AXIS,bPercent), aNumStr, &pTextColor);
/*N*/ 
/*N*/ 								if( pTextColor )			// BM #60999#
/*N*/ 								{
/*?*/ 									pOldTextColor = (SvxColorItem*)(aYTextAttr.GetItem( EE_CHAR_COLOR ));
/*?*/ 									aYTextAttr.Put(SvxColorItem( *pTextColor ));
/*N*/ 								}
/*N*/ 
/*N*/ 								// erzeuge das 3D-textobjekt
/*N*/ 								SdrTextObj *pTextObj = CreateTextObj(CHOBJID_TEXT, Point (),
/*N*/ 									aNumStr,
/*N*/ 									aYTextAttr, FALSE,CHADJUST_CENTER_RIGHT);
/*N*/ 
/*N*/ 								if( pOldTextColor )
/*N*/ 								{
/*?*/ 									aYTextAttr.Put( *pOldTextColor );
/*N*/ 								}
/*N*/ 
/*N*/ 								E3dLabelObj *pE3DLabel = new E3dLabelObj(aLine3D [1],pTextObj);
/*N*/ 								aYDescrList.Insert(pE3DLabel,LIST_APPEND);
/*N*/ 							}
/*N*/ 
/*N*/ 						}
/*N*/ 
/*N*/ 						IncValue(fAct, fValueStepMainY, bYLogarithm);
/*N*/ 					}
/*N*/ 
/*N*/ 				fAct = fMinValueY;
/*N*/ 				double	fActMain = fAct;	//	This is used for calculating the positions of main grid lines.
/*N*/ 				
/*N*/ 				if (pXGridHelpGroup && (fMinValueY != fMaxValueY))
/*N*/ 				{
/*?*/ 					IncValue(fAct, fValueStepHelpY, FALSE);
/*?*/ 					if (bSwitchColRow)
/*?*/ 					{
/*?*/ 						for (;;)
/*?*/ 						{
/*?*/ 							double fPos = pChartYAxis->CalcFact(fAct ) * aSizeVec.X();
/*?*/ 							if (fPos > aSizeVec.X()) break;
/*?*/ 
/*?*/ 							//	If there is a main grid then ommit creating help grid lines coinciding with main grid lines.
/*?*/ 							if (pXGridMainGroup)
/*?*/ 							{
/*?*/ 								//	Advance main raw y position to at least the raw help position.
/*?*/ 								while (fActMain < fAct-EPSILON)
/*?*/ 									IncValue(fActMain, fValueStepMainY, bYLogarithm);
/*?*/ 								//	If both positions fall together, then do not create the help line and try the next position.
/*?*/ 								if (fabs(fAct - fActMain) < EPSILON)
/*?*/ 								{
/*?*/ 									IncValue(fAct, fValueStepHelpY, FALSE);
/*?*/ 									continue;
/*?*/ 								}
/*?*/ 							}
/*?*/ 
/*?*/ 							aLine3D[0].X() =
/*?*/ 							aLine3D[1].X() = fPos;
/*?*/ 							Create3DPolyObject (pXGridHelpAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*?*/ 												CHOBJID_DIAGRAM_X_GRID_HELP, pXGridHelpGroup);
/*?*/ 
/*?*/ 							//	In case of a help grid with a main grid, draw only every other line.
/*?*/ 							IncValue(fAct, fValueStepHelpY, FALSE);
/*?*/ 						}
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						for (;;)
/*?*/ 						{
/*?*/ 							double fPos = pChartYAxis->CalcFact(fAct ) * aSizeVec.Y();
/*?*/ 							if (fPos > aSizeVec.Y()) break;
/*?*/ 
/*?*/ 							//	If there is a main grid then ommit creating help grid lines coinciding with main grid lines.
/*?*/ 							if (pXGridMainGroup)
/*?*/ 							{
/*?*/ 								//	Advance main raw y position to at least the raw help position.
/*?*/ 								while (fActMain < fAct-EPSILON)
/*?*/ 									IncValue(fActMain, fValueStepMainY, bYLogarithm);
/*?*/ 								//	If both positions fall together, then do not create the help line and try the next position.
/*?*/ 								if (fabs(fAct - fActMain) < EPSILON)
/*?*/ 								{
/*?*/ 									IncValue(fAct, fValueStepHelpY, FALSE);
/*?*/ 									continue;
/*?*/ 								}
/*?*/ 							}
/*?*/ 
/*?*/ 							aLine3D[0].Y() =
/*?*/ 							aLine3D[1].Y() = fPos;
/*?*/ 							Create3DPolyObject (pXGridHelpAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*?*/ 												CHOBJID_DIAGRAM_X_GRID_HELP, pXGridHelpGroup);
/*?*/ 
/*?*/ 							//	In case of a help grid with a main grid, draw only every other line.
/*?*/ 							IncValue(fAct, fValueStepHelpY, FALSE);
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 
/*N*/ 			case 2:		// floor
/*N*/ 			{
/*N*/ 				// BM: create 'thick' floor
/*N*/ 				Rectangle aRect( 0, 0, (long)aSizeVec.X(), (long)aSizeVec.Z() );
/*N*/ 				PolyPolygon aPolyPolygon;
/*N*/ 				Polygon aPoly( aRect );
/*N*/ 				aPolyPolygon.Insert( aPoly );
/*N*/ 
/*N*/ 				E3dExtrudeObj* pFloorObj = new SchE3dExtrudeObj( aDefltAttr3D, aPolyPolygon, fFloorWidth );
/*N*/ 				Matrix4D aMatrix;
/*N*/ 				aMatrix.RotateX( 90.0 * F_PI180 );
/*N*/ 				aMatrix.TranslateZ( aSizeVec.Z() );
/*N*/ 				aMatrix.Translate( aPos );
/*N*/ 
/*N*/ 				pFloorObj->SetModel( this );
/*N*/ 				pFloorObj->InsertUserData( new SchObjectId( CHOBJID_DIAGRAM_FLOOR ) );
/*N*/ 				rScene.Insert3DObj( pFloorObj );
/*N*/ 
/*N*/ //-/				pFloorObj->NbcSetAttributes( *pDiagramFloorAttr, FALSE );
/*N*/ 				pFloorObj->SetItemSet(*pDiagramFloorAttr);
/*N*/ 
/*N*/ 				pFloorObj->NbcSetTransform( aMatrix * aShift );
/*N*/ 
/*N*/ 				Vector3D aEndPos( aPos );
/*N*/ 				aEndPos.Z() += aSizeVec.Z();
/*N*/ 
/*N*/ 				Vector3D aLine3D [2] = { aPos, aEndPos };
/*N*/ 
/*N*/ 				SfxItemSet aXTextAttr ((const SfxItemSet &) *pXAxisAttr);
/*N*/ 				aXTextAttr.Put(XLineStyleItem (XLINE_NONE));
/*N*/ 				aXTextAttr.Put(XLineWidthItem (0));
/*N*/ 
/*N*/ 				//#47500#
/*N*/ 				if (pZAxisGroup && pChartZAxis->IsVisible()) Create3DPolyObject( pZAxisAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*N*/ 																				 CHOBJID_LINE, pZAxisGroup );
/*N*/ 
/*N*/ 				if (bSwitchColRow)
/*N*/ 				{
/*N*/ 					long nStepMainY  = (long) aSizeVec.Y () / nColumnCnt;
/*N*/ 					long nStepHelpY  = nStepMainY / 2;
/*N*/ 
/*N*/ 					for (i = 0; i <= nColumnCnt; i++)
/*N*/ 					{
/*N*/ 						// erzeuge hilfslinie
/*N*/ 						aLine3D[0].Y() =
/*N*/ 						aLine3D[1].Y() = aPos.Y() + nStepMainY * i;
/*N*/ 
/*N*/ 						if (pYGridMainGroup) Create3DPolyObject (pYGridMainAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*N*/ 																 CHOBJID_DIAGRAM_Y_GRID_MAIN, pYGridMainGroup);
/*N*/ 
/*N*/ 						if ((i < nColumnCnt) && pXAxisGroup && pChartXAxis->HasDescription())
/*N*/ 						{
/*N*/ 							// bei tiefen diagrammtypen muessen die beschriftungen auf halbem abstand
/*N*/ 							// untergebracht sein
/*N*/ 							Vector3D aTextPos = aLine3D[1];
/*N*/ 
/*N*/ 							if (bPartDescr) aTextPos.Y() += nStepHelpY / 2;
/*N*/ 
/*N*/ 
/*N*/ 							SdrTextObj *pTextObj = CreateTextObj(CHOBJID_TEXT,
/*N*/ 								Point (),ColText(i),
/*N*/ 								aXTextAttr, FALSE,CHADJUST_CENTER_RIGHT);
/*N*/ 							E3dLabelObj *pE3DLabel = new E3dLabelObj
/*N*/ 								(aTextPos,pTextObj );
/*N*/ 							pE3DLabel->SetMarkProtect(TRUE);
/*N*/ 							aXDescrList.Insert(pE3DLabel,LIST_APPEND);
/*N*/ 						}
/*N*/ 
/*N*/ 						// hilfslinien koennen mit erzeugt werden
/*N*/ 						if (pYGridHelpGroup && (i < nColumnCnt))
/*N*/ 						{
/*?*/ 							aLine3D[0].Y() += nStepHelpY;
/*?*/ 							aLine3D[1].Y() += nStepHelpY;
/*?*/ 							Create3DPolyObject (pYGridHelpAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*?*/ 												CHOBJID_DIAGRAM_Y_GRID_HELP, pYGridHelpGroup);
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					long nStepMainX  = (long) aSizeVec.X () / nColumnCnt;
/*N*/ 					long nStepHelpX  = nStepMainX / 2;
/*N*/ 
/*N*/ 					for (i = 0; i <= nColumnCnt; i++)
/*N*/ 					{
/*N*/ 						// erzeuge hilfslinie
/*N*/ 						aLine3D[0].X() =
/*N*/ 						aLine3D[1].X() = aPos.X() + nStepMainX * i;
/*N*/ 
/*N*/ 						if (pYGridMainGroup) Create3DPolyObject (pYGridMainAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*N*/ 																 CHOBJID_DIAGRAM_Y_GRID_MAIN, pYGridMainGroup);
/*N*/ 
/*N*/ 						if ((i < nColumnCnt) && pXAxisGroup && pChartXAxis->HasDescription())
/*N*/ 						{
/*N*/ 							// bei tiefen diagrammtypen muessen die beschriftungen auf halbem abstand
/*N*/ 							// untergebracht sein
/*N*/ 							Vector3D aTextPos = aLine3D[1];
/*N*/ 
/*N*/ 							if (bPartDescr) aTextPos.X() += nStepHelpX;
/*N*/ 
/*N*/ 
/*N*/ 							SdrTextObj *pTextObj = CreateTextObj(CHOBJID_TEXT,
/*N*/ 								Point (),ColText(i),
/*N*/ 								aXTextAttr, FALSE,CHADJUST_TOP_RIGHT);
/*N*/ 							E3dLabelObj *pE3DLabel = new E3dLabelObj (aTextPos,pTextObj );
/*N*/ 							pE3DLabel->SetMarkProtect(TRUE);
/*N*/ 							aXDescrList.Insert(pE3DLabel,LIST_APPEND);
/*N*/ 						}
/*N*/ 
/*N*/ 						// hilfslinien koennen mit erzeugt werden
/*N*/ 						if (pYGridHelpGroup && (i < nColumnCnt))
/*N*/ 						{
/*?*/ 							aLine3D[0].X() += nStepHelpX;
/*?*/ 							aLine3D[1].X() += nStepHelpX;
/*?*/ 							Create3DPolyObject (pYGridHelpAttr, new SchE3dPolygonObj (aDefltAttr3D, aLine3D[0], aLine3D[1]),
/*?*/ 												CHOBJID_DIAGRAM_Y_GRID_HELP, pYGridHelpGroup);
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				aLine3D[ 0 ] =
/*N*/ 				aLine3D[ 1 ] = aEndPos;
/*N*/ 				aLine3D[ 1 ].X() += aSizeVec.X();
/*N*/ 				
/*N*/ 				SfxItemSet aZTextAttr ((const SfxItemSet &) *pZAxisAttr);
/*N*/ 				aZTextAttr.Put(XLineStyleItem (XLINE_NONE));
/*N*/ 				aZTextAttr.Put(XLineWidthItem (0));
/*N*/ 
/*N*/ 				long nStepMainZ = (long) aSizeVec.Z () / nRowCnt;
/*N*/ 				long nStepHelpZ = nStepMainZ / 2;
/*N*/ 
/*N*/ 				BOOL bCreateGridLine = ( (aLine3D[ 0 ].X() != aLine3D[ 1 ].X()) ||
/*N*/ 										 (aLine3D[ 0 ].Y() != aLine3D[ 1 ].Y()) );
/*N*/ 				// Z() values become equal in the for loop
/*N*/ 				// => start and end points would be equal
/*N*/ 
/*N*/ 				for (i = 0; i <= nRowCnt; i++)
/*N*/ 				{
/*N*/ 					// create main gridline
/*N*/ 					aLine3D[0].Z() =
/*N*/ 					aLine3D[1].Z() = aPos.Z() + nStepMainZ * i;
/*N*/ 
/*N*/ 					if( pZGridMainGroup && bCreateGridLine )
/*N*/ 						Create3DPolyObject( pZGridMainAttr, new SchE3dPolygonObj( aDefltAttr3D, aLine3D[ 0 ], aLine3D[ 1 ] ),
/*N*/ 											CHOBJID_DIAGRAM_Z_GRID_MAIN, pZGridMainGroup );
/*N*/ 
/*N*/ 					// bei tiefen diagrammtypen muessen die beschriftungen auf halbem abstand
/*N*/ 					// untergebracht sein
/*N*/ 					Vector3D aTextPos = aLine3D[1];
/*N*/ 
/*N*/ 					if (bPartDescr) aTextPos.Z () += nStepHelpZ;
/*N*/ 
/*N*/ 					if ((i < nRowCnt) && (nRowCnt > 1) && pZAxisGroup && pChartZAxis->HasDescription())
/*N*/ 					{
/*N*/ 
/*N*/ 						SdrTextObj *pTextObj=CreateTextObj(CHOBJID_TEXT, Point (),
/*N*/ 							RowText(nRowCnt - 1 - i), aZTextAttr,
/*N*/ 							FALSE, CHADJUST_TOP_LEFT) ;
/*N*/ 
/*N*/ 						E3dLabelObj *pE3DLabel = new E3dLabelObj(aTextPos,pTextObj);
/*N*/ 						pE3DLabel->SetMarkProtect(TRUE);
/*N*/ 						aZDescrList.Insert(pE3DLabel,(ULONG)0);//ZListe umgekehrt füllen
/*N*/ 					}
/*N*/ 
/*N*/ 					if (pZGridHelpGroup && (i < nRowCnt))
/*N*/ 					{
/*?*/ 						aLine3D[0].Z() += nStepHelpZ;
/*?*/ 						aLine3D[1].Z() += nStepHelpZ;
/*?*/ 						if( pZGridHelpGroup && bCreateGridLine )
/*?*/ 							Create3DPolyObject( pZGridHelpAttr, new SchE3dPolygonObj( aDefltAttr3D, aLine3D[0], aLine3D[1] ),
/*?*/ 												CHOBJID_DIAGRAM_Z_GRID_HELP, pZGridHelpGroup );
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pScene->InsertAllTitleText (aXDescrList, pXAxisGroup,SCH_AXIS_ID_X);
/*N*/ 	pScene->InsertAllTitleText (aYDescrList, pYAxisGroup,SCH_AXIS_ID_Y);
/*N*/ 	pScene->InsertAllTitleText (aZDescrList, pZAxisGroup,SCH_AXIS_ID_Z);
/*N*/ 
/*N*/ 	if (pXGridHelpGroup) rScene.Insert3DObj(pXGridHelpGroup);
/*N*/ 	if (pYGridHelpGroup) rScene.Insert3DObj(pYGridHelpGroup);
/*N*/ 	if (pZGridHelpGroup) rScene.Insert3DObj(pZGridHelpGroup);
/*N*/ 	if (pXGridMainGroup) rScene.Insert3DObj(pXGridMainGroup);
/*N*/ 	if (pYGridMainGroup) rScene.Insert3DObj(pYGridMainGroup);
/*N*/ 	if (pZGridMainGroup) rScene.Insert3DObj(pZGridMainGroup);
/*N*/ 	if (pXAxisGroup) rScene.Insert3DObj(pXAxisGroup);
/*N*/ 	if (pYAxisGroup) rScene.Insert3DObj(pYAxisGroup);
/*N*/ 	if (pZAxisGroup) rScene.Insert3DObj(pZAxisGroup);
/*N*/ 
/*N*/ 	//TVM: aus FitInSnapRect
/*N*/ 	//TVM: Bound statt Logic
/*N*/ 	Position3DAxisTitles(rScene.GetBoundRect());
/*N*/ }

/*************************************************************************
|*
|* tiefes 3D-Diagramme erzeugen
|*
\************************************************************************/

/*N*/ SdrObjGroup* ChartModel::Create3DDeepChart(Rectangle &rRect)
/*N*/ {
/*N*/ 	const long nGapWidth = 10;
/*N*/ 
/*N*/ 	pScene = CreateScene (rRect, *aLightVec, fSpotIntensity, aSpotColor,
/*N*/ 						  fAmbientIntensity, aAmbientColor);
/*N*/ 
/*N*/ 	Vector3D	aTextPos3D;
/*N*/ 	Polygon3D   aRect3D(4);
/*N*/ 	Polygon3D   aTriangle3D(3);
/*N*/ 	Polygon3D	aFrontSide;
/*N*/ 	Polygon3D	aBackSide;
/*N*/ 	BOOL        bSwitchColRow  = IsBar();
/*N*/ 	E3dDefaultAttributes aDefltAttr3D;
/*N*/ 
/*N*/ 	CreateAndInsert3DAxesTitles (rRect, bSwitchColRow);
/*N*/ 
/*N*/ 	long	nX      = 0;
/*N*/ 	long	nY      = 0;
/*N*/ 	long	nW      = FIXED_SIZE_FOR_3D_CHART_VOLUME;
/*N*/ 	long	nH      = FIXED_SIZE_FOR_3D_CHART_VOLUME;
/*N*/ 	long    nZ      = (FIXED_SIZE_FOR_3D_CHART_VOLUME * 4) / 6;
/*N*/ 	long	nColCnt = GetColCount();
/*N*/ 	long	nRowCnt = GetRowCount();
/*N*/ 	short	nCol, nRow;
/*N*/ 
/*N*/ 	Polygon     aFrontExtrude(1+nColCnt*2);
/*N*/ 
/*N*/ 	long nGapX      = nW * nGapWidth / 1000;
/*N*/ 	long nGapY      = nH * nGapWidth / 1000;
/*N*/ 	long nGapZ      = nZ * nGapWidth / 1000;
/*N*/ 	long nPartDepth = nZ / nRowCnt;
/*N*/ 	long nPartWidth;
/*N*/ 	long nBarWidthX;
/*N*/ 	long nBarWidthZ = nPartDepth - nGapZ * 2;
/*N*/ 
/*N*/ 	BOOL bLogarithm = ((const SfxBoolItem&) pChartYAxis->GetItemSet()->Get(SCHATTR_AXIS_LOGARITHM)).GetValue();
/*N*/ 
/*N*/ 	BOOL bPartDescr;
/*N*/ 	long nDepth = ((eChartStyle == CHSTYLE_3D_BAR) ||
/*N*/ 				   (eChartStyle == CHSTYLE_3D_COLUMN) ||
/*N*/ 				   (eChartStyle == CHSTYLE_3D_STRIPE) ||
/*N*/ 				   (eChartStyle == CHSTYLE_3D_AREA))
/*N*/ 					  ? -nZ
/*N*/ 					  : nPartDepth;
/*N*/ 
/*N*/ 	nPartWidth	= (eChartStyle == CHSTYLE_3D_BAR)
/*N*/ 					  ? nH / nColCnt
/*N*/ 					  : ((eChartStyle == CHSTYLE_3D_COLUMN)
/*N*/ 							 ? nW / nColCnt
/*N*/ 							 : ((nColCnt > 1)
/*N*/ 									? nW / (nColCnt - 1)
/*N*/ 									: nW / nColCnt));
/*N*/ 	nBarWidthX	= (eChartStyle == CHSTYLE_3D_BAR)
/*N*/ 					  ? nPartWidth - nGapY * 2
/*N*/ 					  : nPartWidth - nGapX * 2;
/*N*/ 	bPartDescr	= TRUE;
/*N*/ 
/*N*/ 	Camera3D aCam(pScene->GetCamera());
/*N*/ 	Vector3D aCamPos(nX + nW/2, nH/2, nW/2);
/*N*/ 	Vector3D aLookAt(nX + nW/2, nH/2, nDepth/2);
/*N*/ 	aCam.SetViewWindow(-nW/2, -nH/2, nW, nH);
/*N*/ 	aCam.SetDefaults(aCamPos, aLookAt, 80, DEG2RAD(-(double)nZAngle / 10.0));
/*N*/ 	aCam.Reset();
/*N*/ 	aCam.SetProjection(eProjection);
/*N*/ 	aCam.RotateAroundLookAt(DEG2RAD((double)nYAngle / 10.0), DEG2RAD((double)nXAngle / 10.0));
/*N*/ 	aCam.SetAspectMapping(AS_HOLD_SIZE);
/*N*/ 	pScene->SetCamera(aCam);
/*N*/ 
/*N*/ 	nY = 0;
/*N*/ 
/*N*/ 	Vector3D a3DPos(nX, nY, nDepth);
/*N*/ 	Vector3D a3DSize(nW, nH, -nDepth);
/*N*/ 
/*N*/ 	Create3DBackplanes(rRect, a3DPos, a3DSize, *pScene,
/*N*/ 					   bPartDescr, FALSE, bLogarithm, FALSE, CHSTACK_NONE, FALSE, FALSE,
/*N*/ 					   bSwitchColRow);
/*N*/ 
/*N*/ 	nY     = (long)(pChartYAxis->CalcFactOrigin() * nH);
/*N*/ 	a3DPos = Vector3D(0, nY, nDepth - nBarWidthZ + nPartDepth - nGapZ);
/*N*/ 
/*N*/ 	DataDescription aDescription;
/*N*/ 
/*N*/ 	for (nRow = nRowCnt-1; nRow >= 0; nRow--)
/*N*/ 	{
/*N*/ 		E3dScene* pStripe   = NULL;
/*N*/ 
/*N*/         // create row-groups. Insertion into main scene is done at the end of
/*N*/         // the for loop (#109628#)
/*N*/ 		E3dScene* pRowGroup = Create3DScene (CHOBJID_DIAGRAM_ROWGROUP);
/*N*/         // #102789# some unset attributes may be propagated to the
/*N*/         // top-level scene
/*N*/         pRowGroup->SetItemSet( pScene->GetItemSet() );
/*N*/         pRowGroup->InsertUserData(new SchDataRow(nRow));
/*N*/ 
/*N*/ 		if ( eChartStyle == CHSTYLE_3D_STRIPE )
/*N*/ 			pStripe = pRowGroup;
/*N*/ 
/*N*/ 		a3DPos.X() = nX;
/*N*/ 		a3DPos.Y() = nY;
/*N*/ 
/*N*/ 		switch(eChartStyle)
/*N*/ 		{
/*N*/ 			case  CHSTYLE_3D_COLUMN:
/*N*/ 
/*N*/ 				a3DPos.X() += nGapX;
/*N*/ 				break;
/*N*/ 
/*N*/ 			case CHSTYLE_3D_BAR:
/*N*/ 
/*N*/ 				// #67333# the y axis of bar charts is not interrested in negative values
/*N*/ 				a3DPos.Y() = nGapY; // += nGapY;
/*N*/ 				break;
/*N*/ 
/*N*/ 			default:
/*N*/ 
/*?*/ 				aBackSide[0]       = a3DPos;
/*?*/ 				aFrontSide[0]      = a3DPos;
/*?*/ 				aFrontSide[0].Z() += nBarWidthZ;
/*?*/ 
/*?*/ 				aFrontExtrude[0]   =Point((long)a3DPos.X(),(long)-a3DPos.Y());
/*N*/ 				break;
/*N*/ 		}
/*N*/ 
/*N*/ 		Vector3D aLastValue;
/*N*/ 		short     nPoints = 0;
/*N*/ 
/*N*/ 		for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 		{
/*N*/ 			double fData = GetData(nCol, nRow);
/*N*/ 
/*N*/ 			SfxItemSet aDataPointAttr(GetFullDataPointAttr(nCol, nRow));
/*N*/ 
/*N*/ 			SvxChartDataDescr eDescr = ((const SvxChartDataDescrItem&)aDataPointAttr.
/*N*/ 										  Get(SCHATTR_DATADESCR_DESCR)).GetValue();
/*N*/ 
/*N*/ 			BOOL bShowDataDescrLocal = (eDescr != CHDESCR_NONE) && bShowDataDescr && // bShowDataDescr is class member
/*N*/                 fData != DBL_MIN;
/*N*/ 			if( bShowDataDescrLocal )
/*N*/ 			{
                /**************************************************************
                * DataDescription erforderlich
                **************************************************************/
/*N*/ 				
/*?*/ 				aDescription.eDescr = eDescr;
/*?*/ 				aDescription.bSymbol = ((const SfxBoolItem&)aDataPointAttr.
/*?*/ 											  Get(SCHATTR_DATADESCR_SHOW_SYM)).GetValue();
/*N*/ 			}
/*N*/ 
/*N*/ 			if (eChartStyle == CHSTYLE_3D_COLUMN)
/*N*/ 			{
/*N*/ 				double fTop, fBottom;
/*N*/ 				if (fData < fOriginY)
/*N*/ 				{
/*?*/ 					fTop	= fOriginY;
/*?*/ 					fBottom = fData;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					fTop	= fData;
/*N*/ 					fBottom	= fOriginY;
/*N*/ 				}
/*N*/ 
/*N*/ 				long nTop =	Min((long)(pChartYAxis->CalcFact(fTop) * nH), nH);
/*N*/ 				long nBottom = Max((long)(pChartYAxis->CalcFact(fBottom) * nH), 0L);
/*N*/ 
/*N*/ 				if( fData != DBL_MIN )
/*N*/ 				{
/*N*/ 					long nBarHeight = nTop - nBottom + 1;
/*N*/ 					a3DPos.Y() = nBottom;
/*N*/ 
/*N*/                     if( nTop > nBottom )
/*N*/                         pRowGroup->Insert3DObj(Create3DBar(a3DPos, Vector3D(nBarWidthX, nBarHeight, nBarWidthZ), nCol, nRow,
/*N*/                                                            aDataPointAttr, FALSE,0,pChartYAxis->CalcFactOrigin()*(double)nH,0));
/*N*/ 
/*N*/ 					if( bShowDataDescrLocal )
/*N*/ 					{
/*?*/ 						aDescription.aTextPos3D      = a3DPos;
/*?*/ 						aDescription.aTextPos3D.X() += nBarWidthX / 2;
/*?*/ 						aDescription.aTextPos3D.Y() += (fData<0)? 0: nBarHeight;
/*?*/ 						aDescription.aTextPos3D.Z() += nBarWidthZ / 2;
/*?*/ 						aDescription.fValue          = GetData(nCol, nRow,		// #67378#
/*?*/ 							((aDescription.eDescr == CHDESCR_PERCENT) ||
/*?*/ 							 (aDescription.eDescr == CHDESCR_TEXTANDPERCENT)) );
/*?*/ 						aDescription.eAdjust         = CHADJUST_CENTER_CENTER;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if (bShowDataDescrLocal)
/*N*/ 				{
/*N*/ 					aDescription.fValue = DBL_MIN;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if (eChartStyle == CHSTYLE_3D_BAR)
/*N*/ 			{
/*N*/ 				double fRight, fLeft;
/*N*/ 				if (fData < fOriginY)
/*N*/ 				{
/*?*/ 					fRight	= fOriginY;
/*?*/ 					fLeft  = fData;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					fRight = fData;
/*N*/ 					fLeft	= fOriginY;
/*N*/ 				}
/*N*/ 
/*N*/ 				long nRight = Min((long)(pChartYAxis->CalcFact(fRight) * nW), nW);
/*N*/ 				long nLeft  = Max((long)(pChartYAxis->CalcFact(fLeft) * nW), 0L);
/*N*/ 
/*N*/                 if( fData != DBL_MIN )
/*N*/                 {
/*N*/ 					long nBarWidth = nRight - nLeft + 1;
/*N*/ 					a3DPos.X() = nLeft;
/*N*/ 
/*N*/                     if( nRight > nLeft )
/*N*/                         pRowGroup->Insert3DObj(Create3DBar(a3DPos, Vector3D(nBarWidth, nBarWidthX, nBarWidthZ), nCol, nRow,
/*N*/                                                            aDataPointAttr,FALSE,0,pChartYAxis->CalcFactOrigin()*(double)nW,0));
/*N*/ 
/*N*/ 					if (bShowDataDescrLocal)
/*N*/ 					{
/*?*/ 						aDescription.aTextPos3D      = a3DPos;
/*?*/ 
/*?*/ 						// #67379# corrected orientation of description
/*?*/ 						aDescription.aTextPos3D.X() += (fData<0)? 0: nBarWidth;
/*?*/ 						aDescription.aTextPos3D.Y() += nBarWidthX / 2;
/*?*/ 						aDescription.aTextPos3D.Z() += nBarWidthZ / 2;
/*?*/ 	
/*?*/ 						aDescription.fValue          = GetData(nCol, nRow,		// #67378#
/*?*/ 															   ((aDescription.eDescr == CHDESCR_PERCENT) ||
/*?*/ 																(aDescription.eDescr == CHDESCR_TEXTANDPERCENT)) );
/*?*/ 						aDescription.eAdjust         = CHADJUST_CENTER_CENTER;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if (bShowDataDescrLocal)
/*N*/ 				{
/*?*/ 					aDescription.fValue = DBL_MIN;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				long nPos = (long)(pChartYAxis->CalcFact(fData ) * nH);
/*?*/ 				if (nPos < 0L) nPos = 0L;
/*?*/ 				else if (nPos > nH) nPos = nH;
/*?*/ 
/*?*/ 				const SfxItemSet& rDataRowAttr = GetDataRowAttr(nRow);
/*?*/ 
/*?*/ 				a3DPos.Y() = nPos;
/*?*/ 
/*?*/ 				switch (eChartStyle)
/*?*/ 				{
/*?*/ 					case CHSTYLE_3D_STRIPE:
/*?*/ 					case CHSTYLE_3D_AREA:
/*?*/ 						{
/*?*/ 							UINT16 nRev       = nColCnt + 1 - nPoints;
/*?*/ 							BOOL  bValidData = TRUE;
/*?*/ 
/*?*/ 							if ((eChartStyle == CHSTYLE_3D_AREA) && (fData == DBL_MIN))
/*?*/ 							{
/*?*/ 								fData      = 0.0;
/*?*/ 								bValidData = FALSE;
/*?*/ 							}
/*?*/ 
/*?*/ 							if (fData != DBL_MIN)
/*?*/ 							{
/*?*/ 								//Stripes:
/*?*/ 								aFrontSide[nRev]      = a3DPos;
/*?*/ 								aFrontSide[nRev].Z() += nBarWidthZ;
/*?*/ 								aBackSide[nPoints]    = a3DPos;
/*?*/ 								//Area:
/*?*/ 								aFrontExtrude[nRev]  = Point((long)a3DPos.X(),(long)-a3DPos.Y());
/*?*/ 
/*?*/ 								if (!nPoints)
/*?*/ 								{
/*?*/ 									//Stripes:
/*?*/ 									aFrontSide[0]          = a3DPos;
/*?*/ 									aFrontSide[0].Y()      = 0;
/*?*/ 									aFrontSide[0].Z()     += nBarWidthZ;
/*?*/ 									aBackSide[nColCnt + 1] = aFrontSide[0];
/*?*/ 									//Area:
/*?*/ 									aFrontExtrude[0].X() =  (long)a3DPos.X();
/*?*/ 									aFrontExtrude[0].Y() =  0;
/*?*/ 								}
/*?*/ 								else if(eChartStyle == CHSTYLE_3D_STRIPE)
/*?*/ 								{
/*?*/ 									if (nColCnt > 1)
/*?*/ 									{
/*?*/ 										// deckel
/*?*/ 										aRect3D[0] = aBackSide[nPoints - 1];
/*?*/ 										aRect3D[1] = aFrontSide[nRev + 1];
/*?*/ 										aRect3D[2] = aFrontSide[nRev];
/*?*/ 										aRect3D[3] = aBackSide[nPoints];
/*?*/ 										E3dPolygonObj *pPolyObj=new SchE3dPolygonObj (aDefltAttr3D, aRect3D);
/*?*/ 
/*?*/ //-/										pPolyObj->SetDoubleSided(TRUE); //Neu 18.5.98
/*?*/ 										pPolyObj->SetItem(Svx3DDoubleSidedItem(TRUE)); //Neu 18.5.98
/*?*/ 										
/*?*/ 										Create3DPolyObject (&rDataRowAttr,pPolyObj,CHOBJID_AREA, pStripe);
/*?*/ 									}
/*?*/ 								}
/*?*/ 								nPoints ++;
/*?*/ 							}
/*?*/ 							else bValidData = FALSE;
/*?*/ 
/*?*/ 							if (nPoints && (eChartStyle==CHSTYLE_3D_AREA) && (nCol==nColCnt-1))
/*?*/ 							{
/*?*/ 								Vector3D aFill = (fData == DBL_MIN) ? aLastValue : a3DPos;
/*?*/ 								for (short nFill = nPoints;nFill <= nColCnt;nFill ++)
/*?*/ 								{
/*?*/ 									//aFrontSide wird noch für die Beschriftung gebraucht
/*?*/ 									aFrontSide[nColCnt+1-nFill]=aFill;
/*?*/ 									aFrontSide[nColCnt+1-nFill].Y()=0;
/*?*/ 									aFrontSide[nColCnt+1-nFill].Z() += nBarWidthZ;
/*?*/ 									aFrontExtrude[nColCnt+1-nFill]=Point((long)aFill.X(),0);
/*?*/ 								}
/*?*/ 
/*?*/ 								if (nColCnt > 1)
/*?*/ 								{
/*?*/ 									PolyPolygon aPolyPoly;
/*?*/ 									aFrontExtrude[nColCnt*2]=aFrontExtrude[0];
/*?*/ 									aPolyPoly.Insert(aFrontExtrude);
/*?*/ 									E3dExtrudeObj* pExtrudeObj= new SchE3dExtrudeObj(aDefltAttr3D, aPolyPoly, nBarWidthZ);
/*?*/ 									Matrix4D aMatrix;
/*?*/ 									aMatrix.Translate(Vector3D(0,0,a3DPos.Z() ));   //nDepth - nBarWidthZ + nPartDepth - nGapZ
/*?*/ 									pExtrudeObj->NbcSetTransform(aMatrix);
/*?*/ 									Create3DExtrudePolyObj(&rDataRowAttr,pExtrudeObj,CHOBJID_AREA,pRowGroup);
/*?*/ 								}
/*?*/ 							}    //Area
/*?*/ 
/*?*/ 							if (fData == DBL_MIN) nPoints = 0;
/*?*/ 
/*?*/ 							if (bShowDataDescrLocal)
/*?*/ 							{
/*?*/ 								aDescription.aTextPos3D = aFrontSide[nRev];
/*?*/ 
/*?*/ 								if (bValidData)
/*?*/ 								{
/*?*/ 									if ((aDescription.eDescr == CHDESCR_PERCENT) ||
/*?*/ 										(aDescription.eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 									{
/*?*/ 										aDescription.fValue = GetData (nCol, nRow, TRUE, TRUE);
/*?*/ 									}
/*?*/ 									else
/*?*/ 									{
/*?*/ 										aDescription.fValue = GetData (nCol, nRow);
/*?*/ 									}
/*?*/ 								}
/*?*/ 								else
/*?*/ 								{
/*?*/ 									aDescription.fValue = DBL_MIN;
/*?*/ 								}
/*?*/ 
/*?*/ 								aDescription.eAdjust = CHADJUST_CENTER_CENTER;
/*?*/ 							}
/*?*/ 
/*?*/ 							break;
/*?*/ 						}
/*?*/ 
/*?*/ 					case CHSTYLE_3D_SURFACE:
/*?*/ 						if (nColCnt > 1)
/*?*/ 						{
/*?*/ 							aTriangle3D[0] = a3DPos;
/*?*/ 							aTriangle3D[1] = a3DPos;
/*?*/ 							aTriangle3D[1].Z() += nPartDepth;
/*?*/ 							aTriangle3D[1].Y() =
/*?*/ 								(long)(pChartYAxis->CalcFact(GetData(nCol, nRow-1)) * nH);
/*?*/ 							aTriangle3D[2] = aTriangle3D[1];
/*?*/ 							aTriangle3D[2].X() += nPartWidth;
/*?*/ 							aTriangle3D[2].Y() =
/*?*/ 								(long)(pChartYAxis->CalcFact(GetData(nCol+1, nRow-1)) * nH);
/*?*/ 							aTriangle3D[3] = aTriangle3D[2];
/*?*/ 							aTriangle3D[3].Z() -= nPartDepth;
/*?*/ 							aTriangle3D[3].Y() =
/*?*/ 								(long)(pChartYAxis->CalcFact(GetData(nCol+1, nRow)) * nH);
/*?*/ 
/*?*/ 							Create3DPolyObject (&rDataRowAttr, new SchE3dPolygonObj (aDefltAttr3D, aTriangle3D, TRUE, TRUE),
/*?*/ 												CHOBJID_AREA, pRowGroup);
/*?*/ 
/*?*/ 							aTriangle3D[0] = aTriangle3D[1];
/*?*/ 							aTriangle3D[1] = aTriangle3D[2];
/*?*/ 							aTriangle3D[2] = a3DPos;
/*?*/ 							Create3DPolyObject (&rDataRowAttr, new SchE3dPolygonObj (aDefltAttr3D, aTriangle3D, TRUE, TRUE),
/*?*/ 												CHOBJID_AREA, pRowGroup);
/*?*/ 						}
/*?*/ 						break;
/*?*/ 				}
/*?*/ 			}
/*N*/ 
/*N*/ 			aLastValue = a3DPos;
/*N*/ 			if (eChartStyle == CHSTYLE_3D_BAR) a3DPos.Y() += nPartWidth;
/*N*/ 			else a3DPos.X() += nPartWidth;
/*N*/ 
/*N*/ 			if( pScene && bShowDataDescrLocal )
/*N*/ 			{
/*?*/				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				CreateDataDescr( aDescription, nCol, nRow, NULL, FALSE, TRUE );
/*N*/ 			}
/*N*/ 
/*N*/ 		}// end for nCol
/*N*/ 
/*N*/         // #109628# insert row groups into main scene only if they contain data
/*N*/         // points
/*N*/         if( pRowGroup->GetSubList()->GetObjCount() > 0 )
/*N*/         {
/*?*/             pScene->Insert3DObj( pRowGroup );
/*N*/         }

        /*Dirty3D (nColCnt, nRow, TRUE, pDescription, (eDataDescr != CHDESCR_NONE) && bShowDataDescr
                                                                             ? pScene
                                                                             : NULL);
        */
/*N*/ 		a3DPos.Z() += nPartDepth;
/*N*/ 	}//end for nRow
/*N*/ 
/*N*/ 	return (SdrObjGroup*) pScene;
/*N*/ }

/*************************************************************************
|*
|* flache 3D-Diagramme erzeugen
|*
\************************************************************************/

/*N*/ SdrObjGroup* ChartModel::Create3DFlatChart(Rectangle &rRect)
/*N*/ {
/*N*/ 	const long nGapWidth = 10;
/*N*/ 
/*N*/ 	pScene = CreateScene (rRect, *aLightVec, fSpotIntensity, aSpotColor,
/*N*/ 						  fAmbientIntensity, aAmbientColor);
/*N*/ 
/*N*/ 	Vector3D	aTextPos3D;
/*N*/ 
/*N*/ 	Polygon3D	aFrontSide;
/*N*/ 
/*N*/ 	BOOL       bSwitchColRow   = IsBar();
/*N*/ 	E3dDefaultAttributes aDefltAttr3D;
/*N*/ 
/*N*/ 	CreateAndInsert3DAxesTitles (rRect, bSwitchColRow);
/*N*/ 
/*N*/ 	BOOL bPercent = IsPercent();
/*N*/ 
/*N*/ 
/*N*/ 	USHORT eStackMode;
/*N*/ 	switch (eChartStyle)
/*N*/ 	{
/*N*/ 		case CHSTYLE_3D_STACKEDFLATCOLUMN:
/*N*/ 		case CHSTYLE_3D_PERCENTFLATCOLUMN:
/*N*/ 		case CHSTYLE_3D_STACKEDFLATBAR:
/*N*/ 		case CHSTYLE_3D_PERCENTFLATBAR:
/*N*/ 			eStackMode = CHSTACK_MINMAX;
/*N*/ 			break;
/*N*/ 
/*N*/ 		case CHSTYLE_3D_STACKEDAREA:
/*N*/ 		case CHSTYLE_3D_PERCENTAREA:
/*N*/ 			eStackMode = CHSTACK_OVERLAP;
/*N*/ 			break;
/*N*/ 
/*N*/ 		default:
/*N*/ 			eStackMode = CHSTACK_NONE;
/*N*/ 			break;
/*N*/ 	}
/*N*/ 
/*N*/ 	long	nX = 0;
/*N*/ 	long	nY = 0;
/*N*/ 	long	nW = FIXED_SIZE_FOR_3D_CHART_VOLUME;
/*N*/ 	long	nH = FIXED_SIZE_FOR_3D_CHART_VOLUME;
/*N*/ 	long	nColCnt = GetColCount();
/*N*/ 	long	nRowCnt = GetRowCount();
/*N*/ 	long	nCol, nRow;
/*N*/ 
/*N*/ 	long nGapX      = nW * nGapWidth / 1000;
/*N*/ 	long nGapY      = nH * nGapWidth / 1000;
/*N*/ 	long nGapZ      = nW * nGapWidth / 1000;
/*N*/ 	long nPartDepth = nW / nRowCnt;
/*N*/ 	long nPartWidth;
/*N*/ 	long nColWidthX;
/*N*/ 	long nBarWidthX;
/*N*/ 	long nBarWidthZ = nPartDepth - nGapZ * 2;
/*N*/ 	long nDepth     = -nPartDepth;
/*N*/ 
/*N*/ 	SfxItemSet* pYAxisAttr = pChartYAxis->GetItemSet();
/*N*/ //	BOOL bLogarithm = ((const SfxBoolItem&) pYAxisAttr->Get(SCHATTR_Y_AXIS_LOGARITHM)).GetValue();
/*N*/ 	BOOL bLogarithm = ((const SfxBoolItem&) pYAxisAttr->Get(SCHATTR_AXIS_LOGARITHM)).GetValue();
/*N*/ 	BOOL bPartDescr;
/*N*/ 
/*N*/ 	Polygon     aFrontExtrude(1+nColCnt*2);
/*N*/ 
/*N*/ 	switch (eChartStyle)
/*N*/ 	{
/*N*/ 		case CHSTYLE_3D_STACKEDAREA:
/*N*/ 		case CHSTYLE_3D_PERCENTAREA:
/*N*/ 		{
/*N*/ 			aFrontSide.SetPointCount(nColCnt*2);
/*N*/ 			nPartWidth	= nW / ((nColCnt > 1) ? nColCnt - 1 : nColCnt);
/*N*/ 			nColWidthX	= 0;
/*N*/ 			nBarWidthX	= 0;
/*N*/ 			bPartDescr	= FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		default:
/*N*/ 		{
/*N*/ 			nPartWidth	= bSwitchColRow
/*N*/ 							  ? nH / nColCnt
/*N*/ 							  : nW / nColCnt;
/*N*/ 			nColWidthX	= bSwitchColRow
/*N*/ 							  ? nPartWidth - nGapY * 2
/*N*/ 							  : nPartWidth - nGapX * 2;
/*N*/ 			nBarWidthX	= ((eChartStyle == CHSTYLE_3D_FLATCOLUMN) || (eChartStyle == CHSTYLE_3D_FLATBAR))
/*N*/ 							  ? nColWidthX / nRowCnt
/*N*/ 							  : 0;
/*N*/ 			bPartDescr	= TRUE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	Camera3D aCam(pScene->GetCamera());
/*N*/ 	Vector3D aCamPos(nX + nW/2, nH/2, nW/2);
/*N*/ 	Vector3D aLookAt(nX + nW/2, nH/2, nDepth/2);
/*N*/ 	aCam.SetViewWindow(-nW/2, -nH/2, nW, nH);
/*N*/ 	aCam.SetDefaults(aCamPos, aLookAt, 80, DEG2RAD(-(double)nZAngle / 10.0));
/*N*/ 	aCam.Reset();
/*N*/ 	aCam.SetProjection(eProjection);
/*N*/ 	aCam.RotateAroundLookAt(DEG2RAD((double)nYAngle / 10.0), DEG2RAD((double)nXAngle / 10.0));
/*N*/ 	aCam.SetAspectMapping(AS_HOLD_SIZE);
/*N*/ 	pScene->SetCamera(aCam);
/*N*/ 
/*N*/ 	nY = 0;
/*N*/ 	Vector3D a3DPos(nX, nY, nDepth);
/*N*/ 	Vector3D a3DSize(nW, nH, -nDepth);
/*N*/ 
/*N*/ 	Create3DBackplanes(rRect, a3DPos, a3DSize, *pScene,
/*N*/ 					   bPartDescr, FALSE, bLogarithm, FALSE, eStackMode, bPercent, TRUE,
/*N*/ 					   bSwitchColRow);
/*N*/ 
/*N*/ 	a3DPos = Vector3D(nX, nY, nDepth - nBarWidthZ + nPartDepth - nGapZ);
/*N*/ 
/*N*/ 	switch (eChartStyle)
/*N*/ 	{
/*N*/ 		case CHSTYLE_3D_STACKEDAREA:
/*N*/ 		case CHSTYLE_3D_PERCENTAREA:
/*N*/ 		{
/*N*/ 			DataDescription* pDescription = NULL;
/*N*/ 
/*N*/ 			double* fOldData = new double[nColCnt];
/*N*/ 			a3DPos.Z() += nBarWidthZ;
/*N*/ 
/*N*/ 			for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 			{
/*N*/ 				E3dScene* pRowGroup = Create3DScene (CHOBJID_DIAGRAM_ROWGROUP);
/*N*/ 				pScene->Insert3DObj(pRowGroup);
/*N*/ 
/*N*/ 				pRowGroup->InsertUserData(new SchDataRow((short)nRow));
/*N*/ 
/*N*/ 				const SfxItemSet& rDataRowAttr = GetDataRowAttr(nRow);
/*N*/ 
/*N*/ 				for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 				{
/*N*/ 					double     fData          = fabs(GetData(nCol, nRow, bPercent));
/*N*/ 					SfxItemSet aDataPointAttr (GetFullDataPointAttr(nCol, nRow));
/*N*/ 					BOOL       bValidData;
/*N*/ 
/*N*/ 					if (fData == DBL_MIN)
/*N*/ 					{
/*?*/ 						fData      = 0.0;
/*?*/ 						bValidData = FALSE;
/*N*/ 					}
/*N*/ 					else bValidData = TRUE;
/*N*/ 
/*N*/ 					SvxChartDataDescr eDescr = ((const SvxChartDataDescrItem&)aDataPointAttr.
/*N*/ 												  Get(SCHATTR_DATADESCR_DESCR)).GetValue();
/*N*/ 
/*N*/ 					if( (eDescr != CHDESCR_NONE) && bShowDataDescr)
/*N*/ 					{
                        /******************************************************
                        * DataDescription erforderlich
                        ******************************************************/
/*?*/ 						if (!pDescription)
/*?*/ 						{
/*?*/ 							// DataDescription noch nicht vorhanden -> erzeugen
/*?*/ 							DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pDescription = new DataDescription [nColCnt];
/*?*/ 						//STRIP001 	ClearDataDescription(pDescription,nColCnt);
/*?*/ 						}
/*?*/ 
/*?*/ 						pDescription [nCol].eDescr = eDescr;
/*?*/ 						pDescription [nCol].bSymbol = ((const SfxBoolItem&)aDataPointAttr.
/*?*/ 													   Get(SCHATTR_DATADESCR_SHOW_SYM)).GetValue();
/*N*/ 					}
/*N*/ 
/*N*/ 					long nRev = nColCnt * 2 - 1 - nCol;
/*N*/ 
/*N*/ 					if (!nRow)
/*N*/ 					{
/*N*/ 						a3DPos.Y()           = Min((long)(pChartYAxis->CalcFact(fData) * nH), nH);
/*N*/ 						aFrontSide[(UINT16)nRev]     = a3DPos;
/*N*/ 						aFrontSide[(UINT16)nCol]     = a3DPos;
/*N*/ 						aFrontSide[(UINT16)nCol].Y() = 0;
/*N*/ 						aFrontExtrude[(UINT16)nRev]  = Point((long)a3DPos.X(),(long)-a3DPos.Y());
/*N*/ 						aFrontExtrude[(UINT16)nCol]  = Point((long)a3DPos.X(),(long)-a3DPos.Y());
/*N*/ 						aFrontExtrude[(UINT16)nCol].Y()= 0;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						fData = fOldData[nCol] + fData;
/*N*/ 						a3DPos.Y() = Min((long)(pChartYAxis->CalcFact(fData) * nH), nH);
/*N*/ 						aFrontSide[(UINT16)nCol] = aFrontSide[(UINT16)nRev];
/*N*/ 						aFrontSide[(UINT16)nRev] = a3DPos;
/*N*/ 						aFrontExtrude[(UINT16)nCol] = aFrontExtrude[(UINT16)nRev];
/*N*/ 						aFrontExtrude[(UINT16)nRev]=Point((long)a3DPos.X(),(long)-a3DPos.Y());
/*N*/ 					}
/*N*/ 
/*N*/ 					fOldData[nCol] = fData;
/*N*/ 
/*N*/ 					if (pDescription)
/*N*/ 					{
/*?*/ 						pDescription [nCol].aTextPos3D      = aFrontSide[(UINT16)nRev];
/*?*/ 						pDescription [nCol].aTextPos3D.Y() -= (aFrontSide[(UINT16)nRev].Y() - aFrontSide[(UINT16)nCol].Y()) / 2;
/*?*/ 
/*?*/ 						if (bValidData)
/*?*/ 						{
/*?*/ 							if ((pDescription [nCol].eDescr == CHDESCR_PERCENT) ||
/*?*/ 								(pDescription [nCol].eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 							{
/*?*/ 								pDescription [nCol].fValue = GetData (nCol, nRow, TRUE, TRUE);
/*?*/ 							}
/*?*/ 							else
/*?*/ 							{
/*?*/ 								pDescription [nCol].fValue = GetData(nCol,nRow,FALSE);//#55586# fData;
/*?*/ 							}
/*?*/ 						}
/*?*/ 						else
/*?*/ 						{
/*?*/ 							pDescription [nCol].fValue = DBL_MIN;
/*?*/ 						}
/*?*/ 
/*?*/ 						pDescription [nCol].eAdjust = CHADJUST_CENTER_CENTER;
/*?*/ 
/*?*/ 						if( eDescr != CHDESCR_NONE && bValidData )
/*?*/ 						{
/*?*/ 							DBG_BF_ASSERT(0, "STRIP"); //STRIP001 CreateDataDescr( pDescription[ nCol ], nCol, nRow, NULL, FALSE, TRUE );
/*?*/ 						}
/*N*/ 					}
/*N*/ 
/*N*/ 					a3DPos.X() += nPartWidth;
/*N*/ 				} //end for nCol
/*N*/ 
/*N*/ 				if (nColCnt > 1)
/*N*/ 				{
/*N*/ 
/*N*/ 					PolyPolygon aPolyPoly;
/*N*/ 					aFrontExtrude[nColCnt*2]=aFrontExtrude[0];//.SetClosed(TRUE);
/*N*/ 					aPolyPoly.Insert(aFrontExtrude);
/*N*/ 
/*N*/ 					E3dExtrudeObj* pExtrudeObj= new SchE3dExtrudeObj(aDefltAttr3D, aPolyPoly,nBarWidthZ);
/*N*/ 					Matrix4D aMatrix;
/*N*/ 					aMatrix.Translate(Vector3D(0,0,nDepth - nBarWidthZ + nPartDepth - nGapZ ));   //-(double)nBarWidthZ
/*N*/ 					pExtrudeObj->NbcSetTransform(aMatrix);
/*N*/ 					//#54870# falsche ID:CHOBJID_DIAGRAM_AREA
/*N*/ 					Create3DExtrudePolyObj(&rDataRowAttr,pExtrudeObj,CHOBJID_AREA,pRowGroup);
/*N*/ 				}
/*N*/ 				a3DPos.X() = nX;
/*N*/ 			}//end for nRow
/*N*/ 
/*N*/ 			delete[] fOldData;
/*N*/ 			delete[] pDescription;
/*N*/ 			break;
/*N*/ 		}  // end case Area's
/*N*/ 
/*N*/ 		default:
/*N*/ 		{
/*N*/ 			DataDescription* pDescription = NULL;
/*N*/ 
/*N*/ 			if (bSwitchColRow) a3DPos.Y() += nGapY;
/*N*/ 			else a3DPos.X() += nGapX;
/*N*/ 
/*N*/             // #100288# same structure as all other charts (no stacked and special groups)
/*N*/             // create groups for all series
/*N*/             E3dScene ** pDataGroup = new E3dScene * [ nRowCnt ];
/*N*/ 
/*N*/             // create 3d sub-scenes for each data series. Insertion into the
/*N*/             // main scene is done at the end of the for loop (#109628#)
/*N*/             for( nRow = 0; nRow < nRowCnt; nRow++ )
/*N*/             {
/*N*/                 pDataGroup[ nRow ] = Create3DScene( CHOBJID_DIAGRAM_ROWGROUP );
/*N*/                 pDataGroup[ nRow ]->InsertUserData( new SchDataRow( static_cast< short >( nRow ) ));
/*N*/             }
/*N*/ 
/*N*/             for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 			{
/*N*/ 				double fDataTop		= fOriginY;
/*N*/ 				double fDataBottom	= fOriginY;
/*N*/ 
/*N*/ 				//Vor-Berechnung des Maximalen/Minimalen Zeichenpunktes
/*N*/ 				double fPreBottom=fOriginY;
/*N*/ 				double fPreTop   =fOriginY;
/*N*/ 				double fPreTopPos,fPreBottomPos,fPreOriPos;
/*N*/ 				double fTop,fBottom,fMin,fMax,fMin2,fMax2;
/*N*/ 				fMin2=fMax2=fOriginY;
/*N*/ 
/*N*/                 for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 				{
/*N*/ 					double fData=GetData(nCol, nRow, bPercent);
/*N*/ 					if (fData != DBL_MIN)
/*N*/ 					{
/*N*/ 						if (fData < fOriginY)
/*N*/ 						{
/*?*/ 							fTop = fPreBottom;
/*?*/ 							if (fTop == fOriginY)
/*?*/ 								fPreBottom = fData;
/*?*/ 							else
/*?*/ 								fPreBottom += fData;
/*?*/ 							fBottom = fPreBottom;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							fBottom = fPreTop;
/*N*/ 							if (fBottom == fOriginY)
/*N*/ 								fPreTop = fData;
/*N*/ 							else
/*N*/ 								fPreTop += fData;
/*N*/ 							fTop = fPreTop;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					if(nRow)
/*N*/ 					{
/*N*/ 						if(fTop<fMin)
/*N*/ 							fMin=fTop;
/*N*/ 						if(fBottom>fMax)
/*N*/ 							fMax=fBottom;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						fMin=fTop;
/*N*/ 						fMax=fBottom;
/*N*/ 					}
/*N*/ 					if(fData<fOriginY)
/*N*/ 						fMin2-=fData;  //top,left
/*N*/ 					else
/*N*/ 						fMax2+=fData; //right,bottom
/*N*/ 				}
/*N*/ 
/*N*/ 				double fR = (double)( IsBar()? nW: nH );
/*N*/ 				fPreTopPos = pChartYAxis->CalcFact( fMin2 ) * fR;
/*N*/ 				if( fR < fPreTopPos )
/*N*/ 					fPreTopPos = fR;
/*N*/ 				fPreBottomPos = pChartYAxis->CalcFact( fMax2 ) * fR;
/*N*/ 				if( fPreBottomPos < 0.0 )
/*N*/ 					fPreBottomPos = 0.0;
/*N*/ 				fPreOriPos = pChartYAxis->CalcFactOrigin() * fR;
/*N*/ 
/*N*/ 				for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 				{
/*N*/ 					double     fData     = GetData(nCol, nRow, bPercent);
/*N*/ 					SfxItemSet aDataPointAttr(GetFullDataPointAttr(nCol, nRow));
/*N*/ 
/*N*/ 					SvxChartDataDescr eDescr = ((const SvxChartDataDescrItem&)aDataPointAttr.
/*N*/ 												  Get(SCHATTR_DATADESCR_DESCR)).GetValue();
/*N*/ 
/*N*/ 					if( (eDescr != CHDESCR_NONE) && bShowDataDescr)
/*N*/ 					{
                        /******************************************************
                        * DataDescription erforderlich
                        ******************************************************/
/*?*/ 						if (!pDescription)
/*?*/ 						{
/*?*/ 							// DataDescription noch nicht vorhanden -> erzeugen
/*?*/ 						DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	pDescription = new DataDescription [nRowCnt];
/*?*/ 						}
/*?*/ 
/*?*/ 						pDescription [nRow].eDescr = eDescr;
/*?*/ 						pDescription [nRow].bSymbol = ((const SfxBoolItem&)aDataPointAttr.
/*N*/ 													   Get(SCHATTR_DATADESCR_SHOW_SYM)).GetValue();
/*N*/ 					}
/*N*/ 
/*N*/ 					switch (eChartStyle)
/*N*/ 					{
/*N*/ 						case CHSTYLE_3D_FLATCOLUMN:
/*N*/ 							if (fData != DBL_MIN)
/*N*/ 							{
/*N*/ 								double fTop;
/*N*/ 								double fBottom;
/*N*/ 
/*N*/ 								if (fData < fOriginY)
/*N*/ 								{
/*?*/ 									fTop	= fOriginY;
/*?*/ 									fBottom = fData;
/*N*/ 								}
/*N*/ 								else
/*N*/ 								{
/*N*/ 									fTop	= fData;
/*N*/ 									fBottom	= fOriginY;
/*N*/ 								}
/*N*/ 
/*N*/ 								long nBottom = Max ((long) (pChartYAxis->CalcFact(fBottom) * nH),0L);
/*N*/ 								long nTop =	Min ((long) (pChartYAxis->CalcFact(fTop) * nH),nH);
/*N*/ 
/*N*/ 								{
/*N*/ 									long nBarHeight = nTop - nBottom + 1;
/*N*/ 									a3DPos.Y() = nBottom;
/*N*/ 
/*N*/                                     if (nTop > nBottom)
/*N*/                                         pDataGroup[ nRow ]->Insert3DObj(Create3DBar(a3DPos, Vector3D(nBarWidthX, nBarHeight, nBarWidthZ), nCol, nRow,
/*N*/                                                                             aDataPointAttr,TRUE,0,fPreOriPos,0));
/*N*/ 
/*N*/ 									if (pDescription)
/*N*/ 									{
/*?*/ 										pDescription [nRow].aTextPos3D      = a3DPos;
/*?*/ 										pDescription [nRow].aTextPos3D.X() += nBarWidthX / 2;
/*?*/ 										pDescription [nRow].aTextPos3D.Y() += (fData<0)? 0: nBarHeight;
/*?*/ 										pDescription [nRow].aTextPos3D.Z() += nBarWidthZ;
/*?*/ 
/*?*/ 										if ((pDescription [nRow].eDescr == CHDESCR_PERCENT) ||
/*?*/ 											(pDescription [nRow].eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 										{
/*?*/ 											pDescription [nRow].fValue = GetData (nCol, nRow, TRUE, TRUE);
/*?*/ 										}
/*?*/ 										else
/*?*/ 										{
/*?*/ 											pDescription [nRow].fValue = GetData(nCol,nRow,FALSE);//#55586# fData;
/*?*/ 										}
/*?*/ 
/*?*/ 										pDescription [nRow].eAdjust = CHADJUST_CENTER_CENTER;
/*N*/ 									}
/*N*/ 								}
/*N*/ 							}
/*N*/ 							else if (pDescription)
/*N*/ 							{
/*?*/ 								pDescription [nRow].fValue = DBL_MIN;
/*N*/ 							}
/*N*/ 
/*N*/ 							a3DPos.X() += nBarWidthX;
/*N*/ 							break;
/*N*/ 
/*N*/ 						case CHSTYLE_3D_FLATBAR:
/*N*/ 							if (fData != DBL_MIN)
/*N*/ 							{
/*N*/ 								double fRight;
/*N*/ 								double fLeft;
/*N*/ 
/*N*/ 								if (fData < fOriginY)
/*N*/ 								{
/*?*/ 									fRight	= fOriginY;
/*?*/ 									fLeft   = fData;
/*N*/ 								}
/*N*/ 								else
/*N*/ 								{
/*N*/ 									fLeft	= fOriginY;
/*N*/ 									fRight	= fData;
/*N*/ 								}
/*N*/ 
/*N*/ 								long nLeft = Max ((long) (pChartYAxis->CalcFact(fLeft) * nW),0L);
/*N*/ 								long nRight = Min ((long)(pChartYAxis->CalcFact(fRight) * nW),nW);
/*N*/ 
/*N*/ 								{
/*N*/ 									long nBarWidth = nRight - nLeft + 1;
/*N*/ 									a3DPos.X() = nLeft;
/*N*/ 
/*N*/                                     if (nRight > nLeft)
/*N*/                                         pDataGroup[ nRow ]->Insert3DObj(Create3DBar(a3DPos, Vector3D(nBarWidth, nBarWidthX, nBarWidthZ), nCol, nRow,
/*N*/                                                                             aDataPointAttr,TRUE,0,fPreOriPos,0));
/*N*/ 
/*N*/ 									if (pDescription)
/*N*/ 									{
/*?*/ 										pDescription [nRow].aTextPos3D      = a3DPos;
/*?*/ 										pDescription [nRow].aTextPos3D.X() += (fData<0)? 0: nBarWidth;
/*?*/ 										pDescription [nRow].aTextPos3D.Y() += nBarWidthX / 2;
/*?*/ 										pDescription [nRow].aTextPos3D.Z() += nBarWidthZ;
/*?*/ 
/*?*/ 										if ((pDescription [nRow].eDescr == CHDESCR_PERCENT) ||
/*?*/ 											(pDescription [nRow].eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 										{
/*?*/ 											pDescription [nRow].fValue = GetData (nCol, nRow, TRUE, TRUE);
/*?*/ 										}
/*?*/ 										else
/*?*/ 										{
/*?*/ 											pDescription [nRow].fValue = GetData(nCol,nRow,FALSE);//#55586# fData;
/*?*/ 										}
/*?*/ 
/*?*/ 										pDescription [nRow].eAdjust = CHADJUST_CENTER_CENTER;
/*N*/ 									}
/*N*/ 								}
/*N*/ 							}
/*N*/ 							else if (pDescription)
/*N*/ 							{
/*?*/ 								pDescription [nRow].fValue = DBL_MIN;
/*N*/ 							}
/*N*/ 
/*N*/ 							a3DPos.Y() += nBarWidthX;
/*N*/ 							break;
/*N*/ 
/*N*/ 						case CHSTYLE_3D_STACKEDFLATCOLUMN:
/*N*/ 						case CHSTYLE_3D_PERCENTFLATCOLUMN:
/*N*/ 							if (fData != DBL_MIN)
/*N*/ 							{
/*?*/ 								double fTop;
/*?*/ 								double fBottom;
/*?*/ 
/*?*/ 								if (fData < fOriginY)
/*?*/ 								{
/*?*/ 									fTop = fDataBottom;
/*?*/ 									if (fTop == fOriginY) fDataBottom = fData;
/*?*/ 									else fDataBottom += fData;
/*?*/ 									fBottom = fDataBottom;
/*N*/ 								}
/*N*/ 								else
/*N*/ 								{
/*N*/ 									fBottom = fDataTop;
/*N*/ 									if (fBottom == fOriginY) fDataTop = fData;
/*N*/ 									else fDataTop += fData;
/*N*/ 									fTop = fDataTop;
/*N*/ 								}
/*N*/ 
/*N*/ 								long nTop =	Min((long)(pChartYAxis->CalcFact(fTop) * nH), nH);
/*N*/ 								long nBottom = Max((long)(pChartYAxis->CalcFact(fBottom) * nH),0L);
/*N*/ 
/*N*/ 								{
/*N*/ 									long nBarHeight = nTop - nBottom + 1;
/*N*/ 									a3DPos.Y() = nBottom;
/*N*/ 
/*N*/                                     if (nTop > nBottom)
/*N*/                                         pDataGroup[ nRow ]->Insert3DObj(Create3DBar(a3DPos, Vector3D(nColWidthX, nBarHeight, nBarWidthZ), nCol, nRow,
/*N*/                                                                             aDataPointAttr,TRUE,fPreBottomPos,fPreOriPos,fPreTopPos));
/*N*/ 
/*N*/ 									if (pDescription)
/*?*/ 									{
/*?*/ 										pDescription [nRow].aTextPos3D      = a3DPos;
/*?*/ 										pDescription [nRow].aTextPos3D.X() += nColWidthX / 2;
/*?*/ 										pDescription [nRow].aTextPos3D.Y() += nBarHeight / 2;
/*?*/ 										pDescription [nRow].aTextPos3D.Z() += nBarWidthZ;
/*?*/ 
/*?*/ 										if ((pDescription [nRow].eDescr == CHDESCR_PERCENT) ||
/*?*/ 											(pDescription [nRow].eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 										{
/*?*/ 											pDescription [nRow].fValue = GetData (nCol, nRow, TRUE, TRUE);
/*?*/ 										}
/*?*/ 										else
/*?*/ 										{
/*?*/ 											pDescription [nRow].fValue = GetData(nCol,nRow,FALSE);//#55586# fData;
/*?*/ 										}
/*?*/ 
/*?*/ 										pDescription [nRow].eAdjust = CHADJUST_CENTER_CENTER;
/*N*/ 									}
/*N*/ 								}
/*N*/ 							}
/*N*/ 							else if (pDescription)
/*N*/ 							{
/*?*/ 								pDescription [nRow].fValue = DBL_MIN;
/*N*/ 							}
/*N*/ 							break;
/*N*/ 
/*N*/ 						case CHSTYLE_3D_STACKEDFLATBAR:
/*N*/ 						case CHSTYLE_3D_PERCENTFLATBAR:
/*N*/ 							if (fData != DBL_MIN)
/*N*/ 							{
/*N*/ 								double fRight;
/*N*/ 								double fLeft;
/*N*/ 
/*N*/ 								if (fData < fOriginY)
/*N*/ 								{
/*?*/ 									fRight = fDataBottom;
/*?*/ 									if (fRight == fOriginY)	fDataBottom = fData;
/*?*/ 									else fDataBottom += fData;
/*?*/ 									fLeft = fDataBottom;
/*N*/ 								}
/*N*/ 								else
/*N*/ 								{
/*N*/ 									fLeft = fDataTop;
/*N*/ 									if (fLeft == fOriginY) fDataTop = fData;
/*N*/ 									else fDataTop += fData;
/*N*/ 									fRight = fDataTop;
/*N*/ 								}
/*N*/ 
/*N*/ 								long nRight = Min((long)(pChartYAxis->CalcFact(fRight) * nW), nW);
/*N*/ 								long nLeft = Max((long) (pChartYAxis->CalcFact(fLeft ) * nW), 0L);
/*N*/ 
/*N*/ 								{
/*N*/ 									long nBarWidth = nRight - nLeft + 1;
/*N*/ 									a3DPos.X() = nLeft;
/*N*/ 
/*N*/                                     if (nRight > nLeft)
/*N*/                                         pDataGroup[ nRow ]->Insert3DObj(Create3DBar(a3DPos, Vector3D(nBarWidth, nColWidthX, nBarWidthZ), nCol, nRow,
/*N*/                                                                             aDataPointAttr,TRUE,fPreBottomPos,fPreOriPos,fPreTopPos));
/*N*/ 
/*N*/ 									if (pDescription)
/*N*/ 									{
/*?*/ 										pDescription [nRow].aTextPos3D      = a3DPos;
/*?*/ 										pDescription [nRow].aTextPos3D.X() += nBarWidth / 2;
/*?*/ 										pDescription [nRow].aTextPos3D.Y() += nColWidthX  / 2;
/*?*/ 										pDescription [nRow].aTextPos3D.Z() += nBarWidthZ;
/*?*/ 
/*?*/ 										if ((pDescription [nRow].eDescr == CHDESCR_PERCENT) ||
/*?*/ 											(pDescription [nRow].eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 										{
/*?*/ 											pDescription [nRow].fValue = GetData (nCol, nRow, TRUE, TRUE);
/*?*/ 										}
/*?*/ 										else
/*?*/ 										{
/*?*/ 											pDescription [nRow].fValue = GetData(nCol,nRow,FALSE);//#55586# fData;
/*?*/ 										}
/*?*/ 
/*?*/ 										pDescription [nRow].eAdjust = CHADJUST_CENTER_CENTER;
/*N*/ 									}
/*N*/ 								}
/*N*/ 							}
/*N*/ 							else if (pDescription)
/*N*/ 							{
/*?*/ 								pDescription [nRow].fValue = DBL_MIN;
/*N*/ 							}
/*N*/ 							break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				//Dirty3D (nRowCnt, nCol, TRUE, pDescription, (eDataDescr != CHDESCR_NONE)&& bShowDataDescr
/*N*/ 				//																	 ? pScene
/*N*/ 				//																	 : NULL);
/*N*/ 
/*N*/ 				// BM: moved here from Dirty3D.
/*N*/ 				if( pDescription )
/*N*/ 				{
/*?*/ 					for (nRow = 0; nRow < nRowCnt; nRow ++)
/*?*/ 					{
/*?*/ 						if (pScene && pDescription[nRow].fValue != DBL_MIN)
/*?*/ 						{
/*?*/ 						DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	CreateDataDescr(pDescription[nRow], nCol, nRow, NULL, TRUE, TRUE);
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				if (bSwitchColRow)
/*N*/ 				{
/*N*/ 					a3DPos.Y() += nBarWidthX ? nGapY * 2 : nPartWidth;
/*N*/ 					a3DPos.X() = 0;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					a3DPos.X() += nBarWidthX ? nGapX * 2 : nPartWidth;
/*N*/ 					a3DPos.Y() = 0;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/             // insert the data series sub-scenes if they contain any data points
/*N*/             // (inserting empty scenes may corrupt the camera parameters of the
/*N*/             // main scene.  See #109628#)
/*N*/             for( nRow = 0; nRow < nRowCnt; nRow++ )
/*N*/             {
/*?*/                 // sublist always exists
/*?*/                 if( pDataGroup[ nRow ]->GetSubList()->GetObjCount() > 0 )
/*?*/                     pScene->Insert3DObj( pDataGroup[ nRow ] );
/*N*/             }
/*N*/ 
/*N*/ 			delete[] pDescription;
/*N*/             delete[] pDataGroup;
/*N*/ 
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return (SdrObjGroup*) pScene;
/*N*/ }

/*************************************************************************
|*
|* 3D-Kreisdiagramm erzeugen
|*
\************************************************************************/

/*N*/ SdrObjGroup* ChartModel::Create3DNewPieChart(Rectangle &rRect)
/*N*/ {
/*N*/ 	SdrObjList  *pList=NULL;
/*N*/ 
/*N*/ 	pChartYAxis->SetPercentMode( FALSE );	// percent is calculated using the row rather than the column
/*N*/ 
/*N*/ 	pScene = CreateScene (rRect, *aLightVec, fSpotIntensity, aSpotColor,
/*N*/ 						  fAmbientIntensity, aAmbientColor);
/*N*/ 	const long nSize=FIXED_SIZE_FOR_3D_CHART_VOLUME;
/*N*/ 
/*N*/ 	long	nW      = nSize;
/*N*/ 	long	nH      = nSize;
/*N*/ 	long    nZ      = nSize;//(nSize * 4) / 6;
/*N*/ 
/*N*/ 	//erweiterung auf Donuts  emöglichen!
/*N*/ 	long nColCnt = GetColCount();
/*N*/ 	const short nRowCnt=1;//	long	nRowCnt = GetRowCount();
/*N*/ 	const short nRow=0;
/*N*/ 	short	nCol;//, nRow;
/*N*/ 
/*N*/ 	long nZExtrude=nZ/3;
/*N*/ 
/*N*/ 	Camera3D aCam(pScene->GetCamera());
/*N*/ 	const long nDepth=-nZ;
/*N*/ 
/*N*/ 	Vector3D aCamPos(0,0,nW/2);
/*N*/ 	Vector3D aLookAt(0,0,nDepth/2);
/*N*/ 	aCam.SetViewWindow(-nW/2, -nH/2, nW, nH);
/*N*/ 	aCam.SetDefaults(aCamPos, aLookAt, 80, DEG2RAD(-(double)nZAngle / 10.0));
/*N*/ 	aCam.Reset();
/*N*/ 	aCam.SetProjection(eProjection);
/*N*/ 
/*N*/ 	aCam.SetAspectMapping(AS_HOLD_SIZE);
/*N*/ 	pScene->SetCamera(aCam);
/*N*/ 	pScene->SetTransform(aSceneMatrix);
/*N*/ 
/*N*/ 	//	Pie charts may not have titles of axes.
/*N*/ 	bShowXAxisTitle = FALSE;
/*N*/ 	bShowYAxisTitle = FALSE;
/*N*/ 	bShowZAxisTitle = FALSE;
/*N*/ 
/*N*/ 	// Max. bestimmen
/*N*/ 	double fTotal=0.0;
/*N*/ 	long nSegments=0;
/*N*/ 	for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 	{
/*N*/ 		double fTemp = fabs(GetData(nCol,nRow));
/*N*/ 		if (fTemp != DBL_MIN && fTemp>0)
/*N*/ 		{
/*N*/ 			fTotal += fTemp;
/*N*/ 			nSegments++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if(!nSegments || fTotal == 0.0)
/*N*/ 		return (SdrObjGroup*) pScene;
/*N*/ 
/*N*/ 	Rectangle aPieRect;
/*N*/ 	aPieRect=Rectangle(Point(-nW/2,-nH/2),Size(nW,nH)); // rect in which the pie is drawn
/*N*/ 
/*N*/ 	const long nMaxAngle=36000;
/*N*/ 	long nPos=0;
/*N*/ 	E3dDefaultAttributes aDefltAttr3D;
/*N*/ 
/*N*/ 	long nEndAngle=0;
/*N*/ 	long nStartAngle=0;
/*N*/ 
/*N*/ 	ChartDataDescription aDescr(nColCnt,nRowCnt,pList,this,bShowDataDescr);
/*N*/ 
/*N*/ 	Point aCenter = aPieRect.Center();
/*N*/ 	Size aPieRectSize = aPieRect.GetSize();
/*N*/ 
/*N*/ 	for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 	{
/*N*/ 		double fValue = fabs(GetData( nCol,nRow));
/*N*/ 		if(fValue!=DBL_MIN && fValue>0.0)
/*N*/ 		{
/*N*/ 			nPos++;
/*N*/ 			SfxItemSet aDataPointAttr(GetFullDataPointAttr(nCol,nRow));
/*N*/ 			nStartAngle=nEndAngle;
/*N*/ 			nEndAngle+=(long)((fValue/fTotal)*nMaxAngle);
/*N*/ 
/*N*/ 			// for last segment use exact value to avoid accumulated errors
/*N*/ 			if( nPos == nSegments )
/*N*/ 				nEndAngle = 36000;
/*N*/ 
/*N*/ 			if(nEndAngle>nMaxAngle)
/*N*/ 				nEndAngle-=nMaxAngle;
/*N*/ 
/*N*/ 			XPolyPolygon aPolyPolygon;
/*N*/ 
/*N*/ 			// if the angle of the sector is too small the conversion method produces an error
/*N*/ 			// especially as angles are rounded to integers / 10
/*N*/ 			if( nEndAngle - nStartAngle < 10 )	// approximate as triangle
/*N*/ 			{
/*?*/ 				XPolygon aPoly( 4 );
/*?*/ 				double fAngleStart = (double)(nStartAngle) * F_PI / 18000.0,
/*?*/ 					fAngleEnd = (double)(nEndAngle) * F_PI / 18000.0,
/*?*/ 					fRadiusX = (double)(aPieRectSize.Width()) / 2.0,
/*?*/ 					fRadiusY = (double)(aPieRectSize.Height()) / 2.0;
/*?*/ 
/*?*/ 				aPoly[ 0 ] = aCenter;
/*?*/ 				aPoly[ 1 ] = Point( (long)(aCenter.X() + fRadiusX * cos( fAngleStart )), (long)(aCenter.Y() - fRadiusY * sin( fAngleStart )) );
/*?*/ 				aPoly[ 2 ] = Point( (long)(aCenter.X() + fRadiusX * cos( fAngleEnd )), (long)(aCenter.Y() - fRadiusY * sin( fAngleEnd )) );
/*?*/ 				aPoly[ 3 ] = aCenter;
/*?*/ 
/*?*/ 				aPolyPolygon.Insert( aPoly );
/*N*/ 			}
/*N*/ 			else		// create a polygon
/*N*/ 			{
/*N*/ 				SdrCircObj aSegment( OBJ_SECT, aPieRect, nStartAngle, nEndAngle);
/*N*/ 				GetPage(0)->NbcInsertObject( &aSegment, 0 );
/*N*/ 				SdrPathObj* pTmp = (SdrPathObj*)aSegment.ConvertToPolyObj( FALSE, FALSE );
/*N*/ 
/*N*/ 				// Add two extra points near the end of the arc so that
/*N*/ 				// the lighting of the 3d object is smoothed
/*N*/ 				XPolygon aPoly( pTmp->GetPathPoly().GetObject(0) );
/*N*/ 
/*N*/ 				long nEnd = aPoly.GetPointCount();
/*N*/ 				if( nEnd > 3 )
/*N*/ 				{
/*N*/ 					Point aP2b( aPoly[ nEnd-3 ] );
/*N*/ 					Point aP1b( aPoly[ 2 ] );
/*N*/ 					Point aP2(  aPoly[ nEnd-2] ) ;
/*N*/ 					Point aP1(  aPoly[ 1 ] );
/*N*/ 					aP1 += (aP1b-aP1) / 100;
/*N*/ 					aP2 += (aP2b-aP2) / 100;
/*N*/ 					aPoly.Insert( nEnd - 2, aP2, XPOLY_NORMAL);
/*N*/ 					aPoly.Insert( 2, aP1, XPOLY_NORMAL );
/*N*/ 				}
/*N*/ 				aPolyPolygon.Insert( aPoly );
/*N*/ 
/*N*/ 				GetPage( 0 )->RemoveObject( 0 );
/*N*/ 			}
/*N*/ 
/*N*/ 			SchE3dExtrudeObj* pObj=new SchE3dExtrudeObj(aDefltAttr3D,
/*N*/ 				aPolyPolygon,nZExtrude);
/*N*/ 
/*N*/ 			DBG_ASSERT( pObj, "couldn't create extrude object" );
/*N*/ 
/*N*/ 			// default attributes reset the texture projection items so set them explicitly
/*N*/ 			// use object specific projection in y direction
/*N*/ //-/			pObj->SetUseStdTextureY( FALSE );
/*N*/ 			pObj->SetItem( Svx3DTextureProjectionYItem( 0 ));
/*N*/ 			pObj->SetItem( Svx3DDoubleSidedItem( TRUE ));
/*N*/  
/*N*/ 			pScene->Insert3DObj(pObj);
/*N*/ 			pObj->InsertUserData(new SchDataPoint(nCol, nRow));
/*N*/ 			pObj->InsertUserData (new SchObjectId (CHOBJID_DIAGRAM_DATA));
/*N*/ 			pObj->SetMoveProtect(TRUE);
/*N*/ 			pObj->SetResizeProtect(TRUE);
/*N*/ 			pObj->SetModel(this);
/*N*/ 
/*N*/ //-/			pObj->NbcSetAttributes(aDataPointAttr,FALSE);
/*N*/ 			pObj->SetItemSet(aDataPointAttr);
/*N*/ 
/*N*/ 
/*N*/ 			Matrix4D aMatrix;
/*N*/ 			aMatrix.TranslateZ(-nZExtrude/2);
/*N*/ 			pObj->NbcSetTransform(aMatrix);
/*N*/ 
/*N*/ 
/*N*/ 			if(aDescr.Enabled())
/*N*/ 			{
/*N*/ 				double fZPos = (double)nZExtrude / 2.0;
/*N*/ 				DataDescription* pDescr=aDescr.Insert(nCol,nRow,aDataPointAttr,Point(0,0),FALSE,CHADJUST_BOTTOM_CENTER,pChartYAxis);
/*N*/ 				if(pDescr)
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 	Segment3DDescr(*pDescr,aPieRect,nStartAngle,nEndAngle,0,aPieRect.GetWidth()/2,aPieRect.GetWidth()/2,fZPos);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aDescr.Build3D(pScene);
/*N*/ 
/*N*/ 	return (SdrObjGroup*) pScene;
/*N*/ }

/*************************************************************************
|*
|* Positioniere die Achsentitel um das Rectangle der Scene
|*
\************************************************************************/

/*N*/ void ChartModel::Position3DAxisTitles(const Rectangle& rSceneRect)
/*N*/ {
/*N*/ 	SdrPage *pPage = GetPage (0);
/*N*/ 	Size aPageSize = pPage->GetSize();
/*N*/ 
/*N*/ 	if (bShowXAxisTitle)
/*N*/ 	{
/*N*/ 		SdrObject *pXAxisTitleObj = GetObjWithId(CHOBJID_DIAGRAM_TITLE_X_AXIS, *pPage);
/*N*/ 		if (pXAxisTitleObj != NULL)
/*N*/ 		{
/*N*/ 			Rectangle aXAxisOutRect = pXAxisTitleObj->GetBoundRect();
/*N*/ 
/*N*/ 			Point aXAxesTitlePosition (rSceneRect.Left () + (int) (rSceneRect.GetWidth () / 2),
/*N*/ 									   Min ((long) aChartRect.Bottom(),
/*N*/ 											(long) (rSceneRect.Bottom () + aXAxisOutRect.GetHeight()) ));
/*N*/ 
/*N*/ 			if (GetXAxisTitleHasBeenMoved() && GetUseRelativePositions() &&
/*N*/ 				(aXAxesTitlePosition.X() > 0) && (aXAxesTitlePosition.Y() > 0))
/*N*/ 			{
/*N*/ 				// FG: Das ist eine Variable die in BuildChart gesetzt wird, kutz bevor
/*N*/ 				//     das Objekt zerstoert wird.
/*?*/ 				double fRelativeXPosition = ((double) aTitleXAxisPosition.X()) / aInitialSizefor3d.Width();
/*?*/ 				double fRelativeYPosition = ((double) aTitleXAxisPosition.Y()) / aInitialSizefor3d.Height();
/*?*/ 				aXAxesTitlePosition.X() = (long)(aPageSize.Width() * fRelativeXPosition);
/*?*/ 				aXAxesTitlePosition.Y() = (long)(aPageSize.Height() * fRelativeYPosition);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if (bSwitch3DColRow)
/*N*/ 				{
/*N*/ 					aXAxesTitlePosition.X() = Max (0l, (long) (rSceneRect.Left () - 2 * aXAxisOutRect.GetWidth ()));
/*N*/ 					aXAxesTitlePosition.Y() = Max (0l, (long) (rSceneRect.Top () + rSceneRect.GetHeight () / 2));
/*N*/ 				}
/*N*/ 			}
/*N*/ 			SetTextPos((SdrTextObj &) *pXAxisTitleObj, aXAxesTitlePosition,pXAxisTitleAttr);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bShowYAxisTitle)
/*N*/ 	{
/*N*/ 		SdrObject *pYAxisTitleObj = GetObjWithId(CHOBJID_DIAGRAM_TITLE_Y_AXIS, *pPage);
/*N*/ 		if (pYAxisTitleObj != NULL)
/*N*/ 		{
/*N*/ 			Rectangle aYAxisOutRect = pYAxisTitleObj->GetBoundRect();
/*N*/ 
/*N*/ 			Point aYAxesTitlePosition (Max (1l, (long) (rSceneRect.Left () - 2 * aYAxisOutRect.GetWidth ())),
/*N*/ 									   Max (1l, (long) (rSceneRect.Top () + rSceneRect.GetHeight () / 2)));
/*N*/ 
/*N*/ 			if (GetYAxisTitleHasBeenMoved() && GetUseRelativePositions() &&
/*N*/ 				(aYAxesTitlePosition.X() >= 0) && (aYAxesTitlePosition.Y() > 0))
/*N*/ 			{
/*N*/ 				// FG: Das ist eine Variable die in BuildChart gesetzt wird, kutz bevor
/*N*/ 				//     das Objekt zerstoert wird.
/*N*/ 				double fRelativeXPosition = ((double) aTitleYAxisPosition.X()) / aInitialSizefor3d.Width();
/*N*/ 				double fRelativeYPosition = ((double) aTitleYAxisPosition.Y()) / aInitialSizefor3d.Height();
/*N*/ 				aYAxesTitlePosition.X() = (long)(aPageSize.Width() * fRelativeXPosition);
/*N*/ 				aYAxesTitlePosition.Y() = (long)(aPageSize.Height() * fRelativeYPosition);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if (bSwitch3DColRow)
/*N*/ 				{
/*N*/ 					aYAxesTitlePosition.X() = rSceneRect.Left () + (int) (rSceneRect.GetWidth () / 2 + 0.5);
/*N*/ 					aYAxesTitlePosition.Y() = Min ((long) aChartRect.Bottom(),
/*N*/ 												   (long) (rSceneRect.Bottom () + aYAxisOutRect.GetHeight()) );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			SetTextPos((SdrTextObj &) *pYAxisTitleObj, aYAxesTitlePosition,pYAxisTitleAttr);
/*N*/ 		}
/*N*/ 
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bShowZAxisTitle)
/*N*/ 	{
/*N*/ 		SdrObject *pZAxisTitleObj = GetObjWithId(CHOBJID_DIAGRAM_TITLE_Z_AXIS, *pPage);
/*N*/ 		if (pZAxisTitleObj != NULL)
/*N*/ 		{
/*N*/ 			Rectangle aZAxisOutRect = pZAxisTitleObj->GetBoundRect();
/*N*/ 
/*N*/ 
/*N*/ 			Point aZAxesTitlePosition( (long)(0.95 * rSceneRect.Right()),
/*N*/                                        (long)Min ((long) aChartRect.Bottom(),
/*N*/                                                   (long) (rSceneRect.Bottom () - aZAxisOutRect.GetHeight())));
/*N*/ 
/*N*/ 
/*N*/ 			if (GetZAxisTitleHasBeenMoved() && GetUseRelativePositions() &&
/*N*/ 				(aZAxesTitlePosition.X() > 0) && (aZAxesTitlePosition.Y() > 0))
/*N*/ 			{
/*N*/ 				// FG: Das ist eine Variable die in BuildChart gesetzt wird, kutz bevor
/*N*/ 				//     das Objekt zerstoert wird.
/*N*/ 				double fRelativeXPosition = ((double) aTitleZAxisPosition.X()) / aInitialSizefor3d.Width();
/*N*/ 				double fRelativeYPosition = ((double) aTitleZAxisPosition.Y()) / aInitialSizefor3d.Height();
/*N*/ 				aZAxesTitlePosition.X() = (long)(aPageSize.Width() * fRelativeXPosition);
/*N*/ 				aZAxesTitlePosition.Y() = (long)(aPageSize.Height() * fRelativeYPosition);
/*N*/ 			}
/*N*/ 			else if(aZAxesTitlePosition.Y()<aZAxisOutRect.GetHeight()/2)
/*N*/ 			{
/*?*/ 				aZAxesTitlePosition.Y()=aZAxisOutRect.GetHeight();
/*N*/ 			}
/*N*/ 			SetTextPos((SdrTextObj &) *pZAxisTitleObj, aZAxesTitlePosition,pZAxisTitleAttr);
/*N*/ 		}
/*N*/ 
/*N*/ 	}
/*N*/ 
/*N*/ 	aInitialSizefor3d = aInitialSize;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
