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
#include <bf_svx/eeitem.hxx>
#ifndef _ZFORLIST_HXX //autogen
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <bf_svtools/zforlist.hxx>
#endif

#ifndef _SVDOCIRC_HXX //autogen
#include <bf_svx/svdocirc.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <bf_svx/svdopath.hxx>
#endif

#ifndef _SCHATTR_HXX
#include "schattr.hxx"
#endif
#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE	        0
#define ITEMID_CHARTDATADESCR	SCHATTR_DATADESCR_DESCR


#endif
#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#define ITEMID_FONTWIDTH   EE_CHAR_FONTWIDTH
#include <bf_svx/fwdtitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
#ifndef _CHTMODEL_HXX
#endif

#include "strings.hrc"
#include "glob.hrc"
#include <math.h>
#include <float.h>


#include "pairs.hxx"
#include "globfunc.hxx"


#ifndef _SVX_XLINIIT_HXX //autogen
#include <bf_svx/xlineit.hxx>
#endif
// header for Line

#include "chaxis.hxx"
#include "chdescr.hxx"
#include "calculat.hxx"
namespace binfilter {

#define SCH_SIN(a)		(sin((double)a * F_PI / 18000.0))
#define SCH_COS(a)		(cos((double)a * F_PI / 18000.0))

/*************************************************************************
|*
|* DataDescription Array initialisieren (loeschen)
|*
\************************************************************************/
/*************************************************************************
|*
|* Kreisdiagramm erzeugen
|*
\************************************************************************/
/*N*/ SdrObjGroup* ChartModel::Create2DPieChart(Rectangle aRect)
/*N*/ {
/*N*/ 	SchObjGroup *pGroup;
/*N*/ 	SdrObjList  *pList;
/*N*/ 
/*N*/ 	CreateChartGroup (pGroup, pList);
/*N*/ 
/*N*/ 	// pie charts always have row number 0 for now
/*N*/ 	const long			nRow = 0;
/*N*/ 
/*N*/ 	double				fTotal	= 0.0;
/*N*/ 	long				nColCnt	= GetColCount();
/*N*/ 	long				nSegOfsMax = 0;
/*N*/ 	
/*N*/ 	SchObjGroup*		pDescrGroup  = NULL;
/*N*/ 	SdrObjList			*pDescrList  = NULL;
/*N*/ 	DataDescription*	pDescription = NULL;
/*N*/ 	long				nCol;
/*N*/ 	Size				aDescrOfs;
/*N*/ 	BOOL        		bInserted  = FALSE;
/*N*/ 
/*N*/ 	//	Pie charts may not have titles of axes.
/*N*/ 	bShowXAxisTitle = FALSE;
/*N*/ 	bShowYAxisTitle = FALSE;
/*N*/ 	bShowZAxisTitle = FALSE;
/*N*/ 
/*N*/ 	// determine the space required by the chart. For this chart type
/*N*/ 	// the labelling is also taken into the calculation
/*N*/ 	pDescrList = 0;
/*N*/ 
/*N*/ 	for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 	{
/*N*/ 		double fData = GetData (nCol, nRow);
/*N*/ 		if (fData != DBL_MIN) fTotal += fabs(fData);
/*N*/ 	}
/*N*/ 
/*N*/ 	for( nCol = 0; nCol < nColCnt; nCol++ )
/*N*/ 	{
/*N*/ 		SfxItemSet aDataPointAttr( GetFullDataPointAttr( nCol, nRow ));
/*N*/ 		double     fData	= GetData( nCol, nRow );
/*N*/ 		long       nIndex	= nCol + nRow * nColCnt;
/*N*/ 
/*N*/ 		nSegOfsMax = Max( PieSegOfs( nCol ), nSegOfsMax );
/*N*/ 
/*N*/ 		SvxChartDataDescr eDescr = ((const SvxChartDataDescrItem&)aDataPointAttr.
/*N*/ 									Get(SCHATTR_DATADESCR_DESCR)).GetValue();
/*N*/ 
/*N*/ 		if( (eDescr != CHDESCR_NONE) && bShowDataDescr )
/*N*/ 		{
/*?*/			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			// data description required
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// shrink rectangle if labels are printed beside the pies
/*N*/ 	if (pDescription)
/*N*/ 	{
/*N*/ 		// shrink by size of description text plus a
/*?*/ 		// percentage of the rect size as text offset
/*?*/ 		long nShrinkX = aDescrOfs.Width() + (aRect.GetWidth() / 20);
/*?*/ 		long nShrinkY = aDescrOfs.Height() + (aRect.GetHeight() / 20);
/*?*/ 
/*?*/ 		aRect.Left()   += nShrinkX;
/*?*/ 		aRect.Right()  -= nShrinkX;
/*?*/ 		aRect.Top()    += nShrinkY;
/*?*/ 		aRect.Bottom() -= nShrinkY;
/*N*/ 	}
/*N*/ 
/*N*/ 	// make sure the pie rectangle is a square
/*N*/ 	// shrink rect
/*N*/ 	if (aRect.GetWidth() > aRect.GetHeight())
/*N*/ 	{
/*N*/ 		aRect.Left() += (aRect.GetWidth() - aRect.GetHeight()) / 2;
/*N*/ 		aRect.Right() = aRect.Left() + aRect.GetHeight();
/*N*/ 	}
/*N*/ 	else if (aRect.GetHeight() > aRect.GetWidth())
/*N*/ 	{
/*?*/ 		aRect.Top() += (aRect.GetHeight() - aRect.GetWidth()) / 2;
/*?*/ 		aRect.Bottom() = aRect.Top() + aRect.GetWidth();
/*N*/ 	}
/*N*/ 
/*N*/ 	long nEndAng = 9000;
/*N*/ 	long nSegOfs;
/*N*/ 
/*N*/ 	Rectangle aDefaultCircRect = aRect;
/*N*/ 	Rectangle aCircRect;
/*N*/ 
/*N*/ 	// shrink default circle rectangle (square) to allow exploded pie segments
/*N*/ 	if( nSegOfsMax )
/*N*/ 	{
/*?*/ 		double fRadius = (double)(aDefaultCircRect.GetWidth()) / 2.0;
/*?*/ 		long   nRadiusDiff = (long)(fRadius * (1.0 - (1.0  / (1.0 + ((double)nSegOfsMax / 100.0)))));
/*?*/ 		aDefaultCircRect.Left()		+= nRadiusDiff;
/*?*/ 		aDefaultCircRect.Right()	-= nRadiusDiff;
/*?*/ 
/*?*/ 		aDefaultCircRect.Top()		+= nRadiusDiff;
/*?*/ 		aDefaultCircRect.Bottom()	-= nRadiusDiff;		
/*N*/ 	}
/*N*/ 	nPieRadius = aDefaultCircRect.GetWidth() / 2;	// member set to query radius of pie segments
/*N*/ 
/*N*/ 	// create all pie objects
/*N*/ 	for( nCol = 0; nCol < nColCnt; nCol++ )
/*N*/ 	{
/*N*/ 		long       nStartAng      = nEndAng;
/*N*/ 		SfxItemSet aDataPointAttr( GetFullDataPointAttr( nCol, nRow ));
/*N*/ 		long       nIndex         = nCol + nRow * nColCnt;
/*N*/ 		double     fData          = fabs( GetData( nCol, nRow ));
/*N*/ 
/*N*/ 		if (fData != DBL_MIN)
/*N*/ 		{
/*N*/ 			if( nCol == nColCnt - 1 )
/*N*/ 			{
/*N*/                 // ensure that the circle is closed even if we start at 90 degrees
/*N*/ 				nEndAng = 9000 + 36000;
/*N*/ 			}
/*N*/ 			else if( fTotal != 0.0 )
/*N*/ 			{
/*?*/ 				nEndAng += (long)((fData / fTotal) * 36000);
/*N*/ 			}
/*N*/ 
/*N*/ 			aCircRect = aDefaultCircRect;
/*N*/ 
/*N*/ 			nSegOfs = PieSegOfs(nCol);
/*N*/ 			if( nSegOfs )
/*N*/ 			{
/*?*/ 				double fSegShift = (double)(nPieRadius * nSegOfs) / 100.0;
/*?*/ 
/*?*/ 				Point aCircPos = aCircRect.TopLeft();
/*?*/ 
/*?*/ 				long nAngleDiff;
/*?*/ 				long nAngleHook;
/*?*/ 
/*?*/ 				// determine the bisector angle
/*?*/ 				if (nStartAng > nEndAng)
/*?*/ 				{
/*?*/ 					nAngleDiff = (nEndAng + 36000 - nStartAng) / 2;
/*?*/ 					nAngleHook = (nStartAng + nAngleDiff) % 36000;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					nAngleDiff = (nEndAng - nStartAng) / 2;
/*?*/ 					nAngleHook = nStartAng + nAngleDiff;
/*?*/ 				}
/*?*/ 
/*?*/ 				// set the text position according to the position of the pie
/*?*/ 				if (nStartAng < 9000)
/*?*/ 				{
/*?*/ 					long   nAngle = nAngleDiff + nStartAng;
/*?*/ 
/*?*/ 					aCircPos.X() += (long) (SCH_COS( nAngle ) * fSegShift);
/*?*/ 					aCircPos.Y() -= (long) (SCH_SIN( nAngle ) * fSegShift);
/*?*/ 				}
/*?*/ 				else if (nStartAng < 18000)
/*?*/ 				{
/*?*/ 					long   nAngle = nAngleDiff + (nStartAng - 9000);
/*?*/ 
/*?*/ 					aCircPos.X() -= (long) (SCH_SIN( nAngle ) * fSegShift);
/*?*/ 					aCircPos.Y() -= (long) (SCH_COS( nAngle ) * fSegShift);
/*?*/ 				}
/*?*/ 				else if (nStartAng < 27000)
/*?*/ 				{
/*?*/ 					long   nAngle = nAngleDiff + (nStartAng - 18000);
/*?*/ 
/*?*/ 					aCircPos.X() -= (long) (SCH_COS( nAngle ) * fSegShift);
/*?*/ 					aCircPos.Y() += (long) (SCH_SIN( nAngle ) * fSegShift);
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					long   nAngle = nAngleDiff + (nStartAng - 27000);
/*?*/ 
/*?*/ 					aCircPos.X() += (long) (SCH_SIN( nAngle ) * fSegShift);
/*?*/ 					aCircPos.Y() += (long) (SCH_COS( nAngle ) * fSegShift);
/*?*/ 				}
/*?*/ 
/*?*/ 				aCircRect.SetPos(aCircPos);
/*?*/ 			}
/*N*/ 
/*N*/ 			// pies and description are not grouped
/*N*/ 			if( fData > 0 )
/*N*/ 			{
/*N*/ 				SdrObject* pObj = CreatePieSegment( aDataPointAttr, aCircRect, nCol, nRow,
/*N*/ 													nStartAng, nEndAng, GetColCount() );
/*N*/ 				pObj->SetMoveProtect( FALSE );		// bm allow dragging pies
/*N*/ 				pList->NbcInsertObject( pObj );
/*N*/ 			}
/*N*/ 
/*N*/ 		}
/*N*/ 		else if( pDescription )
/*N*/ 		{
/*?*/ 			pDescription[ nIndex ].fValue = DBL_MIN;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(pDescrGroup)
/*?*/ 		pList->NbcInsertObject(pDescrGroup);
/*N*/ 
/*N*/ 	pGroup->GetSubList ()->SetRectsDirty ();
/*N*/ 
/*N*/ 	// resize of pie charts is allowed proportionally only
/*N*/ 	SdrObjTransformInfoRec aInfo;
/*N*/ 	aInfo.bResizeFreeAllowed    = FALSE;
/*N*/ 	aInfo.bResizePropAllowed    = TRUE;
/*N*/ 	aInfo.bRotateFreeAllowed    = FALSE;
/*N*/ 	aInfo.bRotate90Allowed      = FALSE;
/*N*/ 	aInfo.bMirrorFreeAllowed    = FALSE;
/*N*/ 	aInfo.bMirror45Allowed      = FALSE;
/*N*/ 	aInfo.bMirror90Allowed      = FALSE;
/*N*/ 	aInfo.bShearAllowed         = FALSE;
/*N*/ 	pGroup->SetObjInfo(aInfo);
/*N*/ 
/*N*/ 	Dirty2D (1, nColCnt, &pDescrList, FALSE, pDescription);
/*N*/ 
/*N*/ 	delete[] pDescription;
/*N*/ 
/*N*/ 	return pGroup;
/*N*/ }
/*************************************************************************
|*
|* Donutdiagramm erzeugen
|*
\************************************************************************/
/*N*/ SdrObjGroup* ChartModel::Create2DDonutChart(Rectangle aRect)
/*N*/ {
/*N*/ 	SchObjGroup *pGroup;
/*N*/ 	SdrObjList  *pList;
/*N*/ 
/*N*/ 	CreateChartGroup (pGroup, pList);
/*N*/ 
/*N*/ 	long              nCol;
/*N*/ 	long              nRow;
/*N*/ 
/*N*/ 	long              nColCnt       = GetColCount() ;
/*N*/ 	long              nRowCnt       = GetRowCount() ;
/*N*/ 
/*N*/ 	double	          *pTotal       = new double [nColCnt];
/*N*/ 	SdrObjList        **pRowLists   = new SdrObjList* [nRowCnt];
/*N*/ 	SdrObjList        **pDescrLists = new SdrObjList* [nRowCnt];
/*N*/ 	DataDescription*  pDescription = NULL;
/*N*/ 	Size              aDescrOfs;
/*N*/ 
/*N*/ 	//	Donut charts may not have titles of axes.
/*N*/ 	bShowXAxisTitle = FALSE;
/*N*/ 	bShowYAxisTitle = FALSE;
/*N*/ 	bShowZAxisTitle = FALSE;
/*N*/ 
/*N*/ 	SchObjGroup**    pDescrGroups=new SchObjGroup*[nRowCnt];
/*N*/ 	for(nRow=0;nRow<nRowCnt;nRow++)
/*N*/ 		pDescrGroups[nRow]=NULL;
/*N*/ 
/*N*/ 	//Summiere ueber alle Elemente der gleichen Spalte (nicht Reihe!),
/*N*/ 	for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 	{
/*N*/ 	   pTotal [nCol]     = 0.0;
/*N*/ 	   for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 		{
/*N*/ 			double fData = GetData (nCol, nRow);
/*N*/ 			if (fData != DBL_MIN) pTotal [nCol] += fabs(fData);
/*N*/ 	   }
/*N*/ 	}

    // FG: 22.3.97 Als erstes wird der Platzbedarf der Beschriftung berechnet.
    //     Bei diesem Chart-Typ ist die Beschriftung mit in die Berechnung der
    //     Chartgroesse einbezogen worden.
/*N*/ 	for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 	{
/*N*/ 		SchObjGroup *pRowGroup = (SchObjGroup*) CreateSimpleGroup (CHOBJID_DIAGRAM_ROWGROUP, TRUE, TRUE);
/*N*/ 		BOOL        bInserted  = FALSE;
/*N*/ 
/*N*/ 		pRowGroup->InsertUserData(new SchDataRow((short)nRow));
/*N*/ 		pList->NbcInsertObject(pRowGroup);
/*N*/ 		pRowLists[nRow]   = pRowGroup->GetSubList();
/*N*/ 		pDescrLists[nRow] = 0;
/*N*/ 
/*N*/ 		// ******************** Data Descriptions erstellen ******************************
/*N*/ 		for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 		{
/*N*/ 			SfxItemSet aDataPointAttr(GetFullDataPointAttr(nCol, nRow));
/*N*/ 			double     fData           = GetData (nCol, nRow);
/*N*/ 			long       nIndex          = nCol + nRow * nColCnt;
/*N*/ 
/*N*/ 			SvxChartDataDescr eDescr = ((const SvxChartDataDescrItem&)aDataPointAttr.
/*N*/ 											 Get(SCHATTR_DATADESCR_DESCR)).GetValue();
/*N*/ 
/*N*/ 			if( (eDescr != CHDESCR_NONE) && bShowDataDescr)
/*N*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
                /**************************************************************
                * DataDescription erforderlich
                **************************************************************/
/*N*/ 			}
/*N*/ 		} //*************************** END DESCRIPTIONS ***********************************
/*N*/ 	}//End for nRow

    // Das Rechteck wird dann etwas kleiner, da Text ausgegeben wird
/*N*/ 	if (pDescription)
/*N*/ 	{
/*?*/ 		aRect.Left()   += aDescrOfs.Width();
/*?*/ 		aRect.Top()    += aDescrOfs.Height();
/*?*/ 		aRect.Right()  -= aDescrOfs.Width();
/*?*/ 		aRect.Bottom() -= aDescrOfs.Height();
/*?*/ 
/*?*/ 		// prozentual verkleinern wir es noch etwas, um einen Offset zum Text zu bekommen
/*?*/ 		long nWidth  = (aRect.GetWidth () * 1) / 20;
/*?*/ 		long nHeight = (aRect.GetHeight () * 1) / 20;
/*?*/ 
/*?*/ 		aRect.Left ()   += nWidth;
/*?*/ 		aRect.Right ()  -= nWidth;
/*?*/ 		aRect.Top ()    += nHeight;
/*?*/ 		aRect.Bottom () -= nHeight;
/*N*/ 	}

/*N*/ 	SdrPage* pPage=GetPage(0);
/*N*/ 	Size aPageSize = pPage->GetSize();
/*N*/ 	if (GetDiagramHasBeenMovedOrResized() && (aInitialSize != aPageSize))
/*N*/ 	{                                           //  in dem Fall hat ein Resize stattgefunden
/*?*/ 		if (aInitialSize.Width() > aPageSize.Width())
/*?*/ 		{
/*?*/ 			if (aRect.GetWidth() > aRect.GetHeight())
/*?*/ 			{
/*?*/ 				aRect.Left() += (aRect.GetWidth() - aRect.GetHeight()) / 2;
/*?*/ 				aRect.Right() = aRect.Left() + aRect.GetHeight();
/*?*/ 			}
/*?*/ 			else if (aRect.GetHeight() > aRect.GetWidth())
/*?*/ 			{
/*?*/ 				aRect.Top() += (aRect.GetHeight() - aRect.GetWidth()) / 2;
/*?*/ 				aRect.Bottom() = aRect.Top() + aRect.GetWidth();
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			if (aInitialSize.Height() > aPageSize.Height())
/*?*/ 			{
/*?*/ 				if (aRect.GetWidth() > aRect.GetHeight())
/*?*/ 				{
/*?*/ 					aRect.Left() += (aRect.GetWidth() - aRect.GetHeight()) / 2;
/*?*/ 					aRect.Right() = aRect.Left() + aRect.GetHeight();
/*?*/ 				}
/*?*/ 				else if (aRect.GetHeight() > aRect.GetWidth())
/*?*/ 				{
/*?*/ 					aRect.Top() += (aRect.GetHeight() - aRect.GetWidth()) / 2;
/*?*/ 					aRect.Bottom() = aRect.Top() + aRect.GetWidth();
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				if (aRect.GetWidth() > aRect.GetHeight())
/*?*/ 				{
/*?*/ 					aRect.Top() += (aRect.GetHeight() - aRect.GetWidth()) / 2;
/*?*/ 					aRect.Bottom() = aRect.Top() + aRect.GetWidth();
/*?*/ 				}
/*?*/ 				else if (aRect.GetHeight() > aRect.GetWidth())
/*?*/ 				{
/*?*/ 					aRect.Left() += (aRect.GetWidth() - aRect.GetHeight()) / 2;
/*?*/ 					aRect.Right() = aRect.Left() + aRect.GetHeight();
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else  // *************  nicht gemoved oder resized *****************************
/*N*/ 	{
/*N*/ 		// FG: Hier wird dafuer gesorgt, dass die Chart-Groesse immer ein Rechteck ist.
/*N*/ 		if (aRect.GetWidth() > aRect.GetHeight())
/*N*/ 		{
/*N*/ 			aRect.Left() += (aRect.GetWidth() - aRect.GetHeight()) / 2;
/*N*/ 			aRect.Right() = aRect.Left() + aRect.GetHeight();
/*N*/ 		}
/*N*/ 		else if (aRect.GetHeight() > aRect.GetWidth())
/*N*/ 		{
/*?*/ 			aRect.Top() += (aRect.GetHeight() - aRect.GetWidth()) / 2;
/*?*/ 			aRect.Bottom() = aRect.Top() + aRect.GetWidth();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	ULONG nSegWidth = (aRect.Right() - aRect.Left()) / (2 * (nColCnt + 1));
/*N*/ 
/*N*/ 	for (nCol = 0; nCol < nColCnt; nCol++) //alle Ringe des Donuts
/*N*/ 	{
/*N*/ 		long nStartAng, nEndAng      = 9000; //90 Grad
/*N*/ 
/*N*/ 		double fTotal = pTotal[nCol];
/*N*/ 
/*N*/ 		for (nRow = 0; nRow < nRowCnt; nRow++) //Alle Datenreihen
/*N*/ 		{
/*N*/ 			nStartAng      = nEndAng;
/*N*/ 			SfxItemSet aDataPointAttr(GetFullDataPointAttr(nCol, nRow));
/*N*/ 			long      nIndex          = nCol + nRow * nColCnt;//            long       nIndex         = nRow + nRowCnt * nCol ;
/*N*/ 
/*N*/ 
/*N*/ 			double fData = fabs(GetData(nCol, nRow));
/*N*/ 
/*N*/ 			if (fData != DBL_MIN)
/*N*/ 			{
/*N*/ 				//letzte Datenreihe ?? (eigentlich Unsinn, nur wegen evtl. Rundungsfehler  notwendig?
/*N*/ 				if (nRow == nRowCnt - 1)
/*N*/ 				{
/*N*/ 					nEndAng = 9000;
/*N*/ 				}
/*N*/ 				else if (fTotal != 0.0)
/*N*/ 				{
/*N*/ 					nEndAng += (long)((fData / fTotal) * 36000);
/*N*/ 				}
/*N*/ 
/*N*/ 				Rectangle aCircRect = aRect;
/*N*/ 
/*N*/ 				if( fData > 0.0 )
/*N*/ 					// Die Segmente einer Reihe werden nicht gruppiert!
/*N*/ 					// Diese Ausnahme besteht, da sonst eine Reihe Segmente anderer Reihen
/*N*/ 					// verdeckt (Gruppenobjekte koennen nicht ineinnander greifen (Problem
/*N*/ 					// des Drawinglayer), vorher: pRowLists[nRow]-> ...
/*N*/ 					pList->NbcInsertObject( CreateDonutSegment( aDataPointAttr,
/*N*/ 															 aCircRect, nSegWidth,
/*N*/ 															 nCol, nRow,
/*N*/ 															 nStartAng, nEndAng, nRowCnt));
/*N*/ 				
/*N*/ 			}
/*N*/ 			else if (pDescription)
/*N*/ 			{
/*N*/ 				pDescription [nIndex].fValue = DBL_MIN;
/*N*/ 			}
/*N*/ 
/*N*/ 		}//Alle Datenreihen
/*N*/ 
/*N*/ 		//Jetzt wird die Flaeche verkleinert, um den naechst-inneren Pie zu zeichnen,
/*N*/ 		//Ein Donut ist nichts anderes als ein Tuerme-von-Hanoi artiger Pie-Stapel
/*N*/ 		aRect.Left()   += nSegWidth;
/*N*/ 		aRect.Top()    += nSegWidth;
/*N*/ 		aRect.Right()  -= nSegWidth;
/*N*/ 		aRect.Bottom() -= nSegWidth;
/*N*/ 	}//Alle Ringe
/*N*/ 
/*N*/ 	const XFillStyleItem &rFillStyleItem = (const XFillStyleItem &) pDiagramAreaAttr->Get (XATTR_FILLSTYLE);
/*N*/ 	SfxItemSet           aAreaAttr (*pDiagramAreaAttr);
/*N*/ 
/*N*/ 	if (rFillStyleItem.GetValue () == XFILL_NONE)
/*N*/ 	{
/*N*/ 		aAreaAttr.Put(XFillStyleItem(XFILL_SOLID));
/*N*/ 		aAreaAttr.Put(XFillColorItem(String(), RGBColor(COL_WHITE)));
/*N*/ 	}
/*N*/ 
/*N*/ 	aAreaAttr.Put(XLineStyleItem(XLINE_SOLID));
/*N*/ 	aAreaAttr.Put(XLineWidthItem(0));
/*N*/ 	aAreaAttr.Put(XLineColorItem(String(), RGBColor(COL_BLACK)));
/*N*/ 
/*N*/ 	SdrCircObj* pCirc = new SdrCircObj(OBJ_CIRC, aRect);
/*N*/ 	pList->NbcInsertObject(SetObjectAttr( pCirc, CHOBJID_DIAGRAM_AREA, TRUE, TRUE, &aAreaAttr));
/*N*/ 
/*N*/ 	//Descriptions zuletzt, da sie sonst verdeckt sind!
/*N*/ 	for(nRow=0;nRow<nRowCnt;nRow++)
/*N*/ 	{
/*N*/ 		if(pDescrGroups[nRow])
/*N*/ 		{
/*?*/ 			 pList->NbcInsertObject(pDescrGroups[nRow]);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pGroup->GetSubList()->SetRectsDirty();
/*N*/ 
/*N*/ 	// Ein Kreisdiagramm soll man nur proportional Resizen koennen (vorerst)
/*N*/ 	SdrObjTransformInfoRec aInfo;
/*N*/ 	aInfo.bResizeFreeAllowed    = FALSE;
/*N*/ 	aInfo.bResizePropAllowed    = TRUE;
/*N*/ 	aInfo.bRotateFreeAllowed    = FALSE;
/*N*/ 	aInfo.bRotate90Allowed      = FALSE;
/*N*/ 	aInfo.bMirrorFreeAllowed    = FALSE;
/*N*/ 	aInfo.bMirror45Allowed      = FALSE;
/*N*/ 	aInfo.bMirror90Allowed      = FALSE;
/*N*/ 	aInfo.bShearAllowed         = FALSE;
/*N*/ 	pGroup->SetObjInfo(aInfo);
/*N*/ 
/*N*/     Dirty2D (nRowCnt, nColCnt, pDescrLists, TRUE, pDescription);
/*N*/ 
/*N*/ 	delete[] pTotal;
/*N*/ 	delete[] pDescription;
/*N*/ 	delete[] pDescrLists;
/*N*/ 	delete[] pRowLists;
/*N*/ 	delete[] pDescrGroups;
/*N*/ 
/*N*/ 
/*N*/ 	return pGroup;
/*N*/ }
/*************************************************************************
|*
|* XY-Diagramm erzeugen
|*
\************************************************************************/
/*N*/ SdrObjGroup* ChartModel::Create2DXYChart(Rectangle aRect)
/*N*/ {
/*N*/ 	long	nColCnt = GetColCount();
/*N*/     long    nRowCnt = GetRowCount();
/*N*/ 
/*N*/ 	long	nCol, nRow;
/*N*/ 
/*N*/ 	// ask for sorting
/*N*/ 	
/*N*/ 	BOOL	bSortTable = FALSE;
/*N*/ 	BOOL	bRepaint   = FALSE;

/*N*/ 	if( IsXYChart() && ! ISFLAGSET( nChartStatus, CHS_USER_NOQUERY ) )	// in this case ask for sorting
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 
/*N*/ 	SchObjGroup *pGroup;
/*N*/ 	SdrObjList  *pList;
/*N*/ 	SdrObject   *pObj;
/*N*/     SdrPathObj  *pLineObject = NULL;
/*N*/ 
/*N*/ 	CreateChartGroup (pGroup, pList);
/*N*/ 
/*N*/ 	SdrPage* pPage = GetPage(0);
/*N*/ 
/*N*/ 	Create2DBackplane(aRect, *pList, TRUE,CHSTACK_NONE);
/*N*/ 
/*N*/ 	SdrObjList      ** pRowLists   = new SdrObjList*[nRowCnt];
/*N*/ 	SdrObjList      ** pStatLists  = new SdrObjList*[nRowCnt];
/*N*/ 	Size            aLegendSize (((SvxFontWidthItem &) pLegendAttr->Get (EE_CHAR_FONTWIDTH)).GetWidth (),
/*N*/ 								 ((SvxFontHeightItem &) pLegendAttr->Get (EE_CHAR_FONTHEIGHT)).GetHeight ());
/*N*/ 
/*N*/ 
/*N*/ 	ChartDataDescription aDescr(nColCnt,nRowCnt,pList,this,bShowDataDescr);

    //#54884# Was soll das hier? //Create2DXYChart
/*N*/     OutputDevice * pRefDev = GetRefDevice();
/*N*/ 	if(pRefDev)
/*N*/ 		aLegendSize = pRefDev->PixelToLogic (pRefDev->LogicToPixel (aLegendSize));
/*N*/ 	else
/*N*/ 		DBG_ERROR("ChartModel: no RefDevice");
/*N*/ 
/*N*/ 	long nLegendHeight = aLegendSize.Height () * 9 / 10;
/*N*/ 
/*N*/ 	pRowLists [0]  =
/*N*/ 	pStatLists [0] = 0;
/*N*/ 
/*N*/ 	XPolygon aPolygon ((unsigned short)nColCnt);
/*N*/     ::std::vector< ::std::pair< double, double > > aSplinePoints;
/*N*/     bool bIsSplineChart = ( IsSplineChart() != FALSE );
/*N*/ 
/*N*/ 	SfxItemSet aLineAttr(*pItemPool, XATTR_LINE_FIRST, XATTR_LINE_LAST, 0);
/*N*/ 
/*N*/ 	ChartAxis* pAxis=pChartYAxis;
/*N*/ 	BOOL    bLogarithmY   = pAxis->IsLogarithm();
/*N*/ 	BOOL    bLogarithmX   = pChartXAxis->IsLogarithm();
/*N*/ 
/*N*/ 	// for some reason the ChartRect is one pixel too small
/*N*/ 	Rectangle aClipRect( aRect );
/*N*/ 	aClipRect.Right() += 1;
/*N*/ 	aClipRect.Top() -= 1;
/*N*/ 
/*N*/ 	//	Iterate in reverse order over the data series so that the first one is displayed
/*N*/ 	//	in the front and the last one in the back.
/*N*/ 	for( nRow = nRowCnt-1; nRow >= 1; nRow-- )
/*N*/ 	{
/*N*/ 		SchObjGroup* pRowGroup = (SchObjGroup*) CreateSimpleGroup (CHOBJID_DIAGRAM_ROWGROUP, TRUE, TRUE);
/*N*/ 
/*N*/ 		pRowGroup->InsertUserData(new SchDataRow((short)nRow));
/*N*/ 		pList->NbcInsertObject(pRowGroup);
/*N*/ 		pRowLists[nRow] = pRowGroup->GetSubList();
/*N*/ 
/*N*/ 		SchObjGroup* pStatGroup = (SchObjGroup*) CreateSimpleGroup (CHOBJID_DIAGRAM_STATISTICS_GROUP, TRUE, TRUE);
/*N*/ 
/*N*/ 		pStatGroup->InsertUserData(new SchDataRow((short)nRow));
/*N*/ 		pList->NbcInsertObject(pStatGroup);
/*N*/ 		pStatLists [nRow] = pStatGroup->GetSubList ();
/*N*/ 
/*N*/ 		const SfxItemSet& rDataRowAttr = GetDataRowAttr(nRow);
/*N*/ 
/*N*/ 		pAxis=GetAxisByUID(((const SfxInt32Item&)rDataRowAttr.Get(SCHATTR_AXIS)).GetValue());
/*N*/ 		bLogarithmY = pAxis->IsLogarithm();
/*N*/ 
/*N*/ 		long nPoints = 0;
/*N*/         pLineObject  = NULL;
/*N*/ 
        if( ((const SfxBoolItem &) rDataRowAttr.Get( SCHATTR_STAT_AVERAGE )).GetValue() )
// 			pStatLists[ nRow ]->NbcInsertObject( AverageValueY( nRow, FALSE, aRect,
           {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001  pList->NbcInsertObject( AverageValueY( nRow, FALSE, aRect,

/*N*/         aSplinePoints.clear();
/*N*/ 
/*N*/         for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 		{
/*N*/ 			SfxItemSet aDataPointAttr(rDataRowAttr);//#63904#
/*N*/ 			MergeDataPointAttr(aDataPointAttr,nCol,nRow);
/*N*/ 
/*N*/ 			Point  aPoint;
/*N*/ 			double fDataY    = GetData(nCol, nRow, FALSE);
/*N*/ 			double fDataX    = GetData(nCol, 0, FALSE);
/*N*/ 
/*N*/             if (((fDataX != DBL_MIN) && (!bLogarithmX || bLogarithmX && (fDataX > 0.0))) &&
/*N*/ 				((fDataY != DBL_MIN) && (!bLogarithmY || bLogarithmY && (fDataY > 0.0))))
/*N*/ 			{
/*N*/ 				long nXPos = pChartXAxis->GetPos(fDataX);
/*N*/ 				long nYPos = pAxis->GetPos(fDataY);
/*N*/ 
/*N*/                 // expect nPoints to be 0 on first entry of the for loop
/*N*/                 // insert the empty polygon now, so that symbols are painted over the line
/*N*/                 if( nPoints == 0 &&
/*N*/                     IsLine( nRow ))
/*N*/                 {
/*N*/                     XPolyPolygon aSeriesPoly;
/*N*/                     pLineObject = new SdrPathObj( OBJ_PLIN, aSeriesPoly );
/*N*/                     pRowLists[ nRow ]->NbcInsertObject( pLineObject);
/*N*/                 }
/*N*/ 				aPolygon [(USHORT) nPoints].X () = nXPos;
/*N*/ 				aPolygon [(USHORT) nPoints].Y () = nYPos;
/*N*/                 if( bIsSplineChart )
/*N*/                 {
/*N*/                     aSplinePoints.push_back(
/*N*/                         ::std::pair< double, double >(
/*N*/                             static_cast< double >( nXPos ),
/*N*/                             static_cast< double >( nYPos )));
/*N*/                 }
/*N*/ 				nPoints ++;
/*N*/ 
/*N*/ 				aPoint.X () = nXPos;
/*N*/ 				aPoint.Y () = nYPos;
/*N*/ 
/*N*/ 				// draw symbols only if they are contained in the diagrams boundrect
/*N*/ 				if( aClipRect.IsInside( aPoint ) )
/*N*/ 				{
/*N*/ 					if(HasSymbols(nRow))
/*N*/ 					{
/*N*/ 						pObj = CreateSymbol (aPoint, nRow, nCol, aDataPointAttr, nLegendHeight);
/*N*/ 						if(pObj)
/*N*/ 						{
/*N*/ 							pObj->InsertUserData(new SchObjectId(CHOBJID_DIAGRAM_DATA));
/*N*/ 							pRowLists[nRow]->NbcInsertObject(pObj);
/*N*/ 						}
/*N*/ 					}
/*N*/ 
/*N*/ 					if ((SvxChartKindError)
/*?*/ 						((const SfxInt32Item &) aDataPointAttr.Get (SCHATTR_STAT_KIND_ERROR)).GetValue () != CHERROR_NONE)
/*?*/ 						{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 AverageErrorY(nRow,fDataY,aPoint,FALSE,aDataPointAttr,pStatLists[nRow],pAxis);
/*N*/ 
/*N*/ 					if(aDescr.Enabled())
/*N*/ 					{
/*N*/ 						Point aPos(aPolygon[(USHORT)nPoints-1]);
/*N*/ 						aPos.Y () -= 150;
/*N*/ 						aDescr.Insert(nCol,nRow,aDataPointAttr,aPos,FALSE,CHADJUST_BOTTOM_CENTER,pAxis);
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if(nPoints && IsLine(nRow))
/*N*/ 		{
/*N*/             aPolygon.SetPointCount( nPoints );
/*N*/ 
/*N*/             // clear old items from last loop pass
/*N*/             aLineAttr.ClearItem();
/*N*/ 			aLineAttr.Put(rDataRowAttr);
/*N*/ 
/*N*/ 			if( nPoints > 1 )
/*N*/ 			{
/*N*/ 				XPolyPolygon aSeriesPoly;
/*N*/ 
/*N*/ 				if( bIsSplineChart )
/*N*/ 				{
/*?*/ 					if ((eChartStyle == CHSTYLE_2D_CUBIC_SPLINE_XY) || (eChartStyle == CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY))
/*?*/                     {DBG_BF_ASSERT(0, "STRIP");
/*?*/                     }
/*?*/                     else
/*?*/                     {
/*?*/                         XPolygon aMeshPoly;
/*?*/                         approxMesh( nGranularity, aMeshPoly, aPolygon, nPoints - 1, nSplineDepth );
/*?*/                         SchCalculationHelper::IntersectPolygonWithRectangle(
/*?*/                             aMeshPoly, aClipRect, aSeriesPoly );
/*?*/                     }
/*?*/ 
/*?*/                     if( pLineObject )
/*?*/                         pLineObject->NbcSetPathPoly( aSeriesPoly );
/*?*/                     else
/*?*/                     {
/*?*/                         pLineObject = new SdrPathObj( OBJ_PLIN, aSeriesPoly );
/*?*/                         pRowLists[ nRow ]->NbcInsertObject( pLineObject);
/*?*/                     }
/*?*/ 				}
/*N*/ 				else		// series consits of lines
/*N*/ 				{
/*N*/ 					SchCalculationHelper::IntersectPolygonWithRectangle( aPolygon, aClipRect, aSeriesPoly );
/*N*/ 
/*N*/                     if( pLineObject )
/*N*/                         pLineObject->NbcSetPathPoly( aSeriesPoly );
/*N*/                     else
/*N*/                     {
/*?*/                         pLineObject = new SdrPathObj( OBJ_PLIN, aSeriesPoly );
/*?*/                         pRowLists[ nRow ]->NbcInsertObject( pLineObject);
/*N*/                     }
/*N*/ 				}
/*N*/ 
/*N*/ 				pLineObject->InsertUserData( new SchObjectId( CHOBJID_DIAGRAM_ROWSLINE ));
/*N*/ 				pLineObject->InsertUserData( new SchDataRow( (short)nRow ));
/*N*/ 
//-/				pObj->NbcSetAttributes( aLineAttr, FALSE );
/*N*/ 				pLineObject->SetItemSet( aLineAttr);
/*N*/ 
/*N*/ 			}
/*N*/ 		}

        // insert regression curve if necessary
        // ------------------------------------
/*N*/ 		if (((const SfxInt32Item &) rDataRowAttr.Get (SCHATTR_STAT_REGRESSTYPE)).GetValue () != CHREGRESS_NONE)
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 double fConst;
/*N*/ 		}
/*N*/ 	}

/*N*/ 	aDescr.Build(TRUE);
/*N*/ 
/*N*/ 	if( bRepaint )
/*?*/ 		pPage->SendRepaintBroadcast();
/*N*/ 	
/*N*/ 	delete[] pRowLists;
/*N*/ 	delete[] pStatLists;
/*N*/ 	return pGroup;
/*N*/ }


/*************************************************************************
|*
|* Netzdiagramm erzeugen
|*
\************************************************************************/

/*N*/ SdrObjGroup* ChartModel::Create2DNetChart(Rectangle aRect)
/*N*/ {
/*N*/ 	//vorlaeufiger Hack, bis logarithmus-Ueberpruefung eingebaut (autokorrektur):
/*N*/ 	pChartXAxis->GetItemSet()->Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM,FALSE));
/*N*/ 	pChartYAxis->GetItemSet()->Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM,FALSE));
/*N*/ 
/*N*/ 	//ToDo: vorlaeufig
/*N*/ 	SfxItemSet* pYAxisAttr=&GetAttr(CHOBJID_DIAGRAM_Y_AXIS);
/*N*/ 
/*N*/ 	SchObjGroup *pGroup;
/*N*/ 	SdrObjList  *pList;
/*N*/ 	SdrObject   *pObj;
/*N*/ 
/*N*/ 	CreateChartGroup (pGroup, pList);
/*N*/ 
/*N*/ 	long 	nColCnt = GetColCount();
/*N*/ 	long 	nRowCnt = GetRowCount();
/*N*/ 	long 	nCol, nRow;
/*N*/ 
/*N*/ 	BOOL    bPercent   = IsPercent();
/*N*/ 	BOOL    bStacked   = IsStacked();
/*N*/ 
/*N*/ 	// bPercent => bStacked ... ersetzt: (bPercent || bStacked)  durch bStacked
/*N*/ 	USHORT  eStackMode =  bStacked ? CHSTACK_MINMAX : CHSTACK_NONE;
/*N*/ 
/*N*/ 	Size            aYDescrSize;
/*N*/ 	SdrObjList      ** pRowLists    = new SdrObjList*[nRowCnt];
/*N*/ 	SdrObjList      ** pDescrLists  = new SdrObjList*[nRowCnt];
/*N*/ 	DataDescription* pDescription   = NULL;
/*N*/ 	SdrObjList      *pYAxisList     = pChartYAxis->IsVisible()
/*N*/ 										  ? CreateGroup (*pList, CHOBJID_DIAGRAM_Y_AXIS, 0)
/*N*/ 										  : NULL;
/*N*/ 	SdrObjList      *pXGridMainList = bShowXGridMain
/*N*/ 										  ? CreateGroup (*pList, CHOBJID_DIAGRAM_X_GRID_MAIN_GROUP, 0)
/*N*/ 										  : 0;
/*N*/ 	SdrObjList      *pXGridHelpList = bShowXGridHelp
/*N*/ 										  ? CreateGroup (*pList, CHOBJID_DIAGRAM_X_GRID_HELP_GROUP, 0)
/*N*/ 										  : 0;
/*N*/ 
/*N*/ 	XPolygon aDataLine (nColCnt + 1);
/*N*/ 	SfxItemSet aLineAttr(*pItemPool, XATTR_LINE_FIRST, XATTR_LINE_LAST, 0);
/*N*/ 	SfxItemSet aTextAttr(*pItemPool, nTextWhichPairs);
/*N*/ 	aTextAttr.Put(*pYAxisAttr);
/*N*/ 
/*N*/ 	if (aRect.GetWidth() > aRect.GetHeight())
/*N*/ 	{
/*N*/ 		aRect.Left() += (aRect.GetWidth() - aRect.GetHeight()) / 2;
/*N*/ 		aRect.Right() = aRect.Left() + aRect.GetHeight();
/*N*/ 	}
/*N*/ 	else if (aRect.GetHeight() > aRect.GetWidth())
/*N*/ 	{
/*?*/ 		aRect.Top() += (aRect.GetHeight() - aRect.GetWidth()) / 2;
/*?*/ 		aRect.Bottom() = aRect.Top() + aRect.GetWidth();
/*N*/ 	}

/*N*/ 	if (pChartYAxis->HasDescription())
/*N*/ 	{
/*N*/ 		aYDescrSize = Size (((SvxFontWidthItem &) pYAxisTitleAttr->Get (EE_CHAR_FONTWIDTH)).GetWidth (),
/*N*/ 							((SvxFontHeightItem &) pYAxisTitleAttr->Get (EE_CHAR_FONTHEIGHT)).GetHeight ());
/*N*/ 
/*N*/ 		aRect.Bottom () -= (aYDescrSize.Height () * 12) / 5;
/*N*/ 		aRect.Top ()    += (aYDescrSize.Height () * 12) / 5;
/*N*/ 	}

/*N*/ 	long   nLength    = aRect.GetHeight () / 2;
/*N*/ 	long   nOffsetX   = aRect.Left () + aRect.GetWidth () / 2;
/*N*/ 	long   nOffsetY   = aRect.Top () + aRect.GetHeight () / 2;
/*N*/ 	double fAngleStep = F_PI * 2 / nColCnt;
/*N*/ 	long   aOldPos    = 0;


/*N*/ 	BOOL bSwitchColRow=IsDataSwitched();
/*N*/ 	pChartYAxis->Initialise(aRect,bSwitchColRow,eStackMode,bPercent,TRUE);
/*N*/ 	pChartYAxis->CalcValueSteps();
/*N*/ 
/*N*/ 	Size aMaxValueSizeY = pChartYAxis->CalcMaxTextSize(CHTXTORIENT_STANDARD);
/*N*/ 	//nur zu test zwecken
/*N*/ 
/*N*/ 	XPolygon aLine (2);
/*N*/ 	double fSteps = pChartYAxis->GetMax();
/*N*/ 	double* fOldData = new double[nColCnt];
/*N*/ 	double fAngle = F_PI / 2;
/*N*/ 
/*N*/ 	Size aLegendSize (((SvxFontWidthItem &) pLegendAttr->Get (EE_CHAR_FONTWIDTH)).GetWidth (),
/*N*/ 					  ((SvxFontHeightItem &) pLegendAttr->Get (EE_CHAR_FONTHEIGHT)).GetHeight ());
/*N*/ 
/*N*/ 	//#54884# Was soll das hier? Create2DNetChart
/*N*/     OutputDevice * pRefDev = GetRefDevice();
/*N*/ 	if(pRefDev)
/*N*/ 		aLegendSize = pRefDev->PixelToLogic (pRefDev->LogicToPixel (aLegendSize));
/*N*/ 	else
/*?*/ 		DBG_ERROR("ChartModel: no RefDevice");
/*N*/ 
/*N*/ 	long nLegendHeight = aLegendSize.Height () * 9 / 10;
/*N*/ 
/*N*/ 	if (pChartYAxis->HasDescription())
/*N*/ 		while (fSteps >= pChartYAxis->GetMin())
/*N*/ 		{
/*N*/ 			double fLength = nLength * pChartYAxis->CalcFact(fSteps);
/*N*/ 
/*N*/ 			for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 			{
/*N*/ 				double fCos = cos (fAngle);
/*N*/ 				double fSin = sin (fAngle);
/*N*/ 
/*N*/ 				aLine [1].X () = (long) (nOffsetX + fLength * fCos + nMarkLen * fSin);
/*N*/ 				aLine [1].Y () = (long) (nOffsetY - fLength * fSin + nMarkLen * fCos);
/*N*/ 				aLine [0].X () = (long) (nOffsetX + fLength * fCos - nMarkLen * fSin);
/*N*/ 				aLine [0].Y () = (long) (nOffsetY - fLength * fSin - nMarkLen * fCos);
/*N*/ 
/*N*/ 				pYAxisList->NbcInsertObject(SetObjectAttr (new SdrPathObj(OBJ_PLIN, aLine), CHOBJID_LINE,
/*N*/ 														   TRUE, TRUE, pAxisAttr));
/*N*/ 
/*N*/ 				if (fSteps == pChartYAxis->GetMax())
/*N*/ 				{
/*N*/ 					Point aTextPos ((long) (nOffsetX + nLength * fCos),
/*N*/ 									(long) (nOffsetY - nLength * fSin));
/*N*/ 					ChartAdjust eAdjust;
/*N*/ 
/*N*/ 					if (fAngle > 0 && fAngle < F_PI / 4)
/*N*/ 					{
/*?*/ 						eAdjust = CHADJUST_CENTER_LEFT;
/*?*/ 						aTextPos.X () = (long) (nOffsetX + nLength * fCos + (aYDescrSize.Height () * 6) / 5);
/*N*/ 					}
/*N*/ 					else if (fAngle >= F_PI / 4 && fAngle <= 3 * F_PI / 4)
/*N*/ 						 {
/*N*/ 							 eAdjust = CHADJUST_BOTTOM_CENTER;
/*N*/ 							 aTextPos.Y () = (long) (nOffsetY - nLength * fSin - (aYDescrSize.Height () * 6) / 5);
/*N*/ 						 }
/*N*/ 						 else if (fAngle > 3 * F_PI / 4 && fAngle <= 5 * F_PI / 4)
/*N*/ 							  {
/*N*/ 								  eAdjust = CHADJUST_CENTER_RIGHT;
/*N*/ 								  aTextPos.X () = (long) (nOffsetX + nLength * fCos - (aYDescrSize.Height () * 6) / 5);
/*N*/ 							  }
/*N*/ 							  else if (fAngle > 5 * F_PI / 4 && fAngle <= 7 * F_PI / 4)
/*N*/ 								   {
/*N*/ 									   eAdjust = CHADJUST_TOP_CENTER;
/*N*/ 									   aTextPos.Y () = (long) (nOffsetY - nLength * fSin + (aYDescrSize.Height () * 6) / 5);
/*N*/ 								   }
/*N*/ 								   else
/*N*/ 								   {
/*N*/ 									   eAdjust = CHADJUST_CENTER_LEFT;
/*N*/ 									   aTextPos.X () = (long) (nOffsetX + nLength * fCos + (aYDescrSize.Height () * 6) / 5);
/*N*/ 								   }
/*N*/ 
/*N*/ 					pYAxisList->NbcInsertObject(CreateTextObj(CHOBJID_TEXT, aTextPos,
/*N*/ 															  ColText(nCol),
/*N*/ 															  aTextAttr,
/*N*/ 															  FALSE, eAdjust));
/*N*/ 				}
/*N*/ 
/*N*/ 				fAngle += fAngleStep;
/*N*/ 			}
/*N*/ 
/*N*/ 			if (pChartYAxis->HasDescription())
/*N*/ 			{
/*N*/ 				Color* pDummy = NULL;
/*N*/ 				String aNumStr;
/*N*/ 				Point aPos;
/*N*/ 
/*N*/ 				pNumFormatter->GetOutputString((bPercent)?fSteps/100.0:fSteps,
/*N*/ 					GetNumFmt(CHOBJID_DIAGRAM_Y_AXIS,bPercent),aNumStr,&pDummy);
/*N*/ 
/*N*/ 				aPos.X() = (long) (nOffsetX + nMarkLen);
/*N*/ 				aPos.Y() = (long) (nOffsetY - fLength);
/*N*/ 
/*N*/ 					if (aPos.Y () - aMaxValueSizeY.Height () > aOldPos)
/*N*/ 					{
/*N*/ 						pYAxisList->NbcInsertObject(CreateTextObj(CHOBJID_TEXT, aPos, aNumStr,
/*N*/ 																  aTextAttr, FALSE, CHADJUST_CENTER_LEFT));
/*N*/ 						aOldPos = aPos.Y ();
/*N*/ 					}
/*N*/ 			}
/*N*/ 
/*N*/ 			if (pXGridMainList)
/*N*/ 			{
/*N*/ 				Rectangle aCircRect ((long) (nOffsetX - fLength),
/*N*/ 									 (long) (nOffsetY - fLength),
/*N*/ 									 (long) (nOffsetX + fLength),
/*N*/ 									 (long) (nOffsetY + fLength));
/*N*/ 
/*N*/ 				SfxItemSet aCircAttr (*pItemPool,
/*N*/ 									  XATTR_LINE_FIRST, XATTR_LINE_LAST,
/*N*/ 									  XATTR_FILL_FIRST, XATTR_FILL_LAST,
/*N*/ 									  0);
/*N*/ 
/*N*/ 				aCircAttr.Put (XLineColorItem (String (),
/*N*/ 							   ((XLineColorItem &) pXGridMainAttr->Get (XATTR_LINECOLOR)).GetValue ()));
/*N*/ 				aCircAttr.Put (XFillColorItem (String (),
/*N*/ 							   ((XFillColorItem &) pXGridMainAttr->Get (XATTR_FILLCOLOR)).GetValue ()));
/*N*/ 				aCircAttr.Put (XLineStyleItem (((XLineStyleItem &) pXGridMainAttr->Get (XATTR_LINESTYLE)).
/*N*/ 											   GetValue ()));
/*N*/ 				aCircAttr.Put (XLineWidthItem (((XLineWidthItem &) pXGridMainAttr->Get (XATTR_LINEWIDTH)).
/*N*/ 											   GetValue ()));
/*N*/ 				aCircAttr.Put (XFillStyleItem(XFILL_NONE));
/*N*/ 				SdrCircObj* pCirc = new SdrCircObj( OBJ_CIRC, aCircRect );
/*N*/ 				pCirc->SetModel( this );
/*N*/ 				pXGridMainList->NbcInsertObject(SetObjectAttr( pCirc,
/*N*/ 															   CHOBJID_DIAGRAM_NET, TRUE, TRUE,
/*N*/ 															   &aCircAttr));
/*N*/ 			}
/*N*/ 
/*N*/ 			DecValue(fSteps, pChartYAxis->GetStep(), FALSE);
/*N*/ 
/*N*/ 			if (pChartYAxis->GetStep() == 0.0) break;
/*N*/ 		}
/*N*/ 
/*N*/ 	fSteps = pChartYAxis->GetMax();
/*N*/ 
/*N*/ 	if (pXGridHelpList)
/*?*/ 		while (fSteps >= pChartYAxis->GetMin())
/*?*/ 		{
/*?*/ 			double fLength = nLength * pChartYAxis->CalcFact(fSteps);
/*?*/ 
/*?*/ 			Rectangle aCircRect ((long) (nOffsetX - fLength),
/*?*/ 								 (long) (nOffsetY - fLength),
/*?*/ 								 (long) (nOffsetX + fLength),
/*?*/ 								 (long) (nOffsetY + fLength));
/*?*/ 
/*?*/ 			SfxItemSet aCircAttr (*pItemPool,
/*?*/ 								  XATTR_LINE_FIRST, XATTR_LINE_LAST,
/*?*/ 								  XATTR_FILL_FIRST, XATTR_FILL_LAST,
/*?*/ 								  0);
/*?*/ 
/*?*/ 			aCircAttr.Put (XLineColorItem (String (),
/*?*/ 						   ((XLineColorItem &) pXGridHelpAttr->Get (XATTR_LINECOLOR)).GetValue ()));
/*?*/ 			aCircAttr.Put (XFillColorItem (String (),
/*?*/ 						   ((XFillColorItem &) pXGridHelpAttr->Get (XATTR_FILLCOLOR)).GetValue ()));
/*?*/ 			aCircAttr.Put (XLineStyleItem (((XLineStyleItem &) pXGridHelpAttr->Get (XATTR_LINESTYLE)).
/*?*/ 										   GetValue ()));
/*?*/ 			aCircAttr.Put (XLineWidthItem (((XLineWidthItem &) pXGridHelpAttr->Get (XATTR_LINEWIDTH)).
/*?*/ 										   GetValue ()));
/*?*/ 			aCircAttr.Put (XFillStyleItem(XFILL_NONE));
/*?*/ 
/*?*/ 			SdrCircObj* pCirc = new SdrCircObj( OBJ_CIRC, aCircRect );
/*?*/ 			pCirc->SetModel( this );
/*?*/ 			pXGridHelpList->NbcInsertObject(SetObjectAttr( pCirc,
/*?*/ 														   CHOBJID_DIAGRAM_X_GRID_HELP, TRUE, TRUE,
/*?*/ 														   &aCircAttr));
/*?*/ 
/*?*/ 			DecValue(fSteps, pChartYAxis->GetStep(), FALSE);
/*?*/ 
/*?*/ 			if (pChartYAxis->GetStep() == 0.0) break;
/*?*/ 		}
/*N*/ 
/*N*/ 	fSteps = pChartYAxis->GetMin();
/*N*/ 
/*N*/ 	for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 	{
/*N*/ 		const SfxItemSet& rDataRowAttr = GetDataRowAttr(nRow);
/*N*/ 		Point aFirstPoint;
/*N*/ 		fAngle = F_PI / 2;
/*N*/ 		long nPoints = 0;
/*N*/ 
/*N*/ 		SchObjGroup* pRowGroup = (SchObjGroup*) CreateSimpleGroup (CHOBJID_DIAGRAM_ROWGROUP, TRUE, TRUE);
/*N*/ 
/*N*/ 		pRowGroup->InsertUserData(new SchDataRow((short)nRow));
/*N*/ 		pList->NbcInsertObject(pRowGroup);
/*N*/ 		pRowLists[nRow] = pRowGroup->GetSubList();
/*N*/ 		pDescrLists[nRow] = 0;
/*N*/ 
/*N*/ 		for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 		{
/*N*/ 			SfxItemSet aDataPointAttr(rDataRowAttr);//#63904#
/*N*/ 			MergeDataPointAttr(aDataPointAttr,nCol,nRow);
/*N*/ 
/*N*/ 			long nIndex = nCol + nRow * nColCnt;
/*N*/ //			double fData = fabs(GetData(nCol, nRow, bPercent));
/*N*/ 			double fData = GetData(nCol, nRow, bPercent);
/*N*/ 
/*N*/ 			if (pYAxisList)
/*N*/ 			{
/*N*/ 				if (!nRow)
/*N*/ 				{
/*N*/ 					aLine [0].X () = (long) (nOffsetX + nLength * cos (fAngle));
/*N*/ 					aLine [0].Y () = (long) (nOffsetY - nLength * sin (fAngle));
/*N*/ 					aLine [1].X () = nOffsetX;
/*N*/ 					aLine [1].Y () = nOffsetY;
/*N*/ 
/*N*/ 					pYAxisList->NbcInsertObject(SetObjectAttr (new SdrPathObj(OBJ_PLIN, aLine),
/*N*/ 															   CHOBJID_LINE, TRUE, TRUE, pYAxisAttr));
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			SvxChartDataDescr eDescr = ((const SvxChartDataDescrItem&)aDataPointAttr.
/*N*/ 											Get(SCHATTR_DATADESCR_DESCR)).GetValue();
/*N*/ 
/*N*/ 			if( (eDescr != CHDESCR_NONE) && bShowDataDescr)
/*N*/ 			{
                /**************************************************************
                * DataDescription erforderlich
                **************************************************************/
/*?*/ 				if (!pDescription)
/*?*/ 				{
/*?*/ 					// DataDescription noch nicht vorhanden -> erzeugen
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	pDescription = new DataDescription [nRowCnt * nColCnt];
/*?*/ 				}
/*?*/ 
/*?*/ 				pDescription [nIndex].eDescr = eDescr;
/*?*/ 				pDescription [nIndex].bSymbol = ((const SfxBoolItem&)aDataPointAttr.
/*?*/ 												Get(SCHATTR_DATADESCR_SHOW_SYM)).GetValue();
/*?*/ 
/*?*/ 				if (!pDescrLists[nRow])
/*?*/ 				{
/*?*/ 					SchObjGroup* pDescrGroup = (SchObjGroup*) CreateSimpleGroup (CHOBJID_DIAGRAM_DESCRGROUP, TRUE, TRUE);
/*?*/ 
/*?*/ 					pDescrGroup->InsertUserData(new SchDataRow((short)nRow));
/*?*/ 					pList->NbcInsertObject(pDescrGroup);
/*?*/ 					pDescrLists[nRow] = pDescrGroup->GetSubList();
/*?*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if (bStacked && !nRow) fOldData[nCol] = pChartYAxis->GetOrigin();
/*N*/ 
/*N*/ 			if (fData != DBL_MIN)
/*N*/ 			{
/*N*/ 				if (bStacked && nRow) fData += fOldData[nCol];
/*N*/ 
/*N*/ 				double fLength = nLength * pChartYAxis->CalcFact(fData);
/*N*/ 
/*N*/ 				aDataLine [(USHORT) nPoints].X () = (long) (nOffsetX + fLength * cos (fAngle));
/*N*/ 				aDataLine [(USHORT) nPoints].Y () = (long) (nOffsetY - fLength * sin (fAngle));
/*N*/ 
/*N*/ 				if (!nCol)
/*N*/ 				{
/*N*/ 					aFirstPoint.X () = aDataLine [(USHORT) nPoints].X();
/*N*/ 					aFirstPoint.Y () = aDataLine [(USHORT) nPoints].Y();
/*N*/ 				}
/*N*/ 
/*N*/ 				nPoints ++;
/*N*/ 
/*N*/ 				if(HasSymbols(nRow)) // if ((eChartStyle == CHSTYLE_2D_NET_SYMBOLS) ||(eChartStyle == CHSTYLE_2D_NET_SYMBOLS_STACK) ||(eChartStyle == CHSTYLE_2D_NET_SYMBOLS_PERCENT))
/*N*/ 				{
/*N*/ 					SdrObject *pNewObj = CreateSymbol (aDataLine[nPoints - 1], nRow, nCol,
/*N*/ 													   (SfxItemSet &) rDataRowAttr, nLegendHeight);
/*N*/ 					if(pNewObj)
/*N*/ 					{
/*N*/ 						pNewObj->InsertUserData(new SchObjectId(CHOBJID_DIAGRAM_DATA));
/*N*/ 						pRowLists[nRow]->NbcInsertObject(pNewObj);
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				fOldData [nCol] = fData;
/*N*/ 
/*N*/ 				ChartAdjust eAdjust;
/*N*/ 
/*N*/ 				if (fAngle > 0 && fAngle < F_PI / 4)
/*N*/ 				{
/*?*/ 					eAdjust = CHADJUST_CENTER_LEFT;
/*N*/ 				}
/*N*/ 				else if (fAngle >= F_PI / 4 && fAngle <= 3 * F_PI / 4)
/*N*/ 					 {
/*N*/ 						 eAdjust = CHADJUST_BOTTOM_CENTER;
/*N*/ 					 }
/*N*/ 					 else if (fAngle > 3 * F_PI / 4 && fAngle <= 5 * F_PI / 4)
/*N*/ 						  {
/*N*/ 							  eAdjust = CHADJUST_CENTER_RIGHT;
/*N*/ 						  }
/*N*/ 						  else if (fAngle > 5 * F_PI / 4 && fAngle <= 7 * F_PI / 4)
/*N*/ 							   {
/*N*/ 								   eAdjust = CHADJUST_TOP_CENTER;
/*N*/ 							   }
/*N*/ 							   else
/*N*/ 							   {
/*N*/ 								   eAdjust = CHADJUST_CENTER_LEFT;
/*N*/ 							   }
/*N*/ 
/*N*/ 				if (pDescription)
/*N*/ 				{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	pDescription [nIndex].fValue = GetData(nCol,nRow,FALSE);//#55586# fData;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if (nPoints)
/*N*/ 				{
/*?*/ 					for (long nFill = nPoints;nFill < nColCnt;nFill ++)
/*?*/ 						aDataLine [(USHORT) nFill] = aDataLine [(USHORT) nPoints - 1];
/*?*/ 
/*?*/ 					aLineAttr.Put(rDataRowAttr);
/*?*/ 
/*?*/ 					pObj = new SdrPathObj(OBJ_PLIN, aDataLine);
/*?*/ 					pObj->InsertUserData(new SchObjectId (CHOBJID_DIAGRAM_ROWSLINE));
/*?*/ 					pObj->InsertUserData(new SchDataRow((short)nRow));
/*?*/ 					pRowLists[nRow]->NbcInsertObject(pObj,0); //#54870# Linie nach hinten
/*?*/ 
/*?*/ //-/					pObj->NbcSetAttributes(aLineAttr, FALSE);
/*?*/ 					pObj->SetItemSet(aLineAttr);
/*?*/ 
/*?*/ 					nPoints = 0;
/*N*/ 				}
/*N*/ 
/*N*/ 				if (pDescription)
/*N*/ 				{
/*?*/ 					pDescription [nIndex].fValue = DBL_MIN;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			fAngle += fAngleStep;
/*N*/ 			if (fAngle > 2 * F_PI) fAngle -= 2 * F_PI;
/*N*/ 		}
/*N*/ 
/*N*/ 		if (nPoints)
/*N*/ 		{
/*N*/ 			for (long nFill = nPoints;nFill < nColCnt;nFill ++)
/*?*/ 				aDataLine [(USHORT) nFill] = aDataLine [(USHORT) nPoints - 1];
/*N*/ 
/*N*/ 			if (aFirstPoint.X () && aFirstPoint.Y ())
/*N*/ 			{
/*N*/ 				aDataLine [(unsigned short)nColCnt].X () = aFirstPoint.X ();
/*N*/ 				aDataLine [(unsigned short)nColCnt].Y () = aFirstPoint.Y ();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				aDataLine [(unsigned short)nColCnt].X () = aDataLine [nColCnt - 1].X ();
/*?*/ 				aDataLine [(unsigned short)nColCnt].Y () = aDataLine [nColCnt - 1].Y ();
/*N*/ 			}
/*N*/ 
/*N*/ 			aLineAttr.Put(rDataRowAttr);
/*N*/ 
/*N*/ 			pObj = new SdrPathObj(OBJ_PLIN, aDataLine);
/*N*/ 			pObj->InsertUserData(new SchObjectId (CHOBJID_DIAGRAM_ROWSLINE));
/*N*/ 			pObj->InsertUserData(new SchDataRow((short)nRow));
/*N*/ 			pRowLists[nRow]->NbcInsertObject(pObj,0);//#54870# hinter die Symbole mit der Linie
/*N*/ 
/*N*/ //-/			pObj->NbcSetAttributes(aLineAttr, FALSE);
/*N*/ 			pObj->SetItemSet(aLineAttr);
/*N*/ 
/*N*/ 		}
/*N*/ 
/*N*/ 		IncValue(fSteps, pChartYAxis->GetStep(), FALSE);
/*N*/ 
/*N*/ 		if (pChartYAxis->GetStep() == 0.0) break;
/*N*/ 	}
/*N*/ 
/*N*/ 	Dirty2D (nRowCnt, nColCnt, pDescrLists, TRUE/*FALSE*/, pDescription);
/*N*/ 
/*N*/ 	delete[] pDescription;
/*N*/ 	delete[] pDescrLists;
/*N*/ 	delete[] pRowLists;
/*N*/ 	delete[] fOldData;
/*N*/ 	return pGroup;
/*N*/ }

/*************************************************************************
|*
|* Frisiere die % auf 100%
|*
\************************************************************************/

/*N*/ void ChartModel::Dirty2D (long            nRowCnt,
/*N*/ 						  long            nColCnt,
/*N*/ 						  SdrObjList      **pDescrLists,
/*N*/ 						  BOOL            bRowDescr,
/*N*/ 						  DataDescription *pDescription)
/*N*/ {
/*N*/     if (!pDescription)
/*N*/ 		return;

/*?*/ 	long nStart = IsXYChart() ? 1 : 0;
/*?*/ 
/*?*/ 	if (bRowDescr)
/*?*/ 	{
/*?*/ 		for (long nCols = 0;nCols < nColCnt;nCols ++)
/*?*/ 		{
/*?*/ 			double fTotal = 0.0;
/*?*/ 			double fMax   = 0.0;
/*?*/ 			long   nDirty = 0;
                long   nRows  = 0;
/*?*/ 
/*?*/             //	Calculate the total of all segements with percentage value and 
/*?*/ 			//	remember the largest segment's index in nDirty and its value in fMax.
/*?*/ 			for (nRows = nStart;nRows < nRowCnt;nRows ++)
/*?*/ 				if (pDescrLists [nRows])
/*?*/ 				{
/*?*/ 					long   nIndex= nCols + nRows * nColCnt;
/*?*/ 					if ((pDescription [nIndex].eDescr == CHDESCR_PERCENT) ||
/*?*/ 						(pDescription [nIndex].eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 					{
/*?*/ 						double fTemp = pDescription [nIndex].fValue;
/*?*/ 	
/*?*/ 						if (fTemp != DBL_MIN)
/*?*/ 						{
/*?*/ 							fTotal += fTemp;
/*?*/ 							if (fMax < fTemp)
/*?*/ 							{
/*?*/ 								fMax   = fTemp;
/*?*/ 								nDirty = nIndex;
/*?*/ 							}
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 
/*?*/ 			BOOL bIsDirty = FALSE;
/*?*/ 
/*?*/ 			for (nRows = nStart;nRows < nRowCnt;nRows ++)
/*?*/ 				if (pDescrLists[nRows])
/*?*/ 				{
/*?*/ 					long nIndex = nCols + nRows * nColCnt;//                    long nIndex = nRows + nCols * nRowCnt;
/*?*/ 
/*?*/ 					if (!bIsDirty)
/*?*/ 						if ((pDescription [nIndex].eDescr == CHDESCR_PERCENT) ||
/*?*/ 							(pDescription [nIndex].eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 							if (fTotal > 100.0000001)
/*?*/ 							{
/*?*/ 								DBG_BF_ASSERT(0, "STRIP"); //STRIP001 DBG_ERROR2( "Dirty2D: ROW value is being changed total=%lf, Vman=%lf",
/*?*/ 							}
/*?*/ 
/*?*/ 					if (pDescription [nIndex].fValue != DBL_MIN)
/*?*/ 						if( pDescription[nIndex].pLabelObj )
/*?*/ 							pDescrLists[nRows]->NbcInsertObject( pDescription[nIndex].pLabelObj );
/*?*/ 				}
/*?*/ 		}
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		for (long nRows = nStart;nRows < nRowCnt; nRows ++)
/*?*/ 		{
/*?*/ 			if ( ! pDescrLists[nRows])
/*?*/             	continue;
/*?*/                 
/*?*/ 			double fTotal = 0.0;
/*?*/ 			double fMax   = 0.0;
/*?*/ 			long   nDirty = 0;
                long   nCols = 0;
/*?*/ 
/*?*/             //	Calculate the total of all segements with a percentage value and
/*?*/ 			//	remember the largest segment's index in nDirty and its value in fMax.
/*?*/ 			for (nCols = 0;nCols < nColCnt;nCols ++)
/*?*/ 			{
/*?*/ 				long   nIndex = nCols + nRows * nColCnt;
/*?*/ 				if ((pDescription [nIndex].eDescr == CHDESCR_PERCENT) ||
/*?*/ 					(pDescription [nIndex].eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 				{
/*?*/ 					double fTemp = pDescription [nIndex].fValue;
/*?*/ 
/*?*/ 					if (fTemp != DBL_MIN)
/*?*/ 					{
/*?*/ 						fTotal += fTemp;
/*?*/ 						if (fMax < fTemp)
/*?*/ 						{
/*?*/ 							fMax   = fTemp;
/*?*/ 							nDirty = nIndex;
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 
/*?*/ 			BOOL bIsDirty = FALSE;
/*?*/ 
/*?*/ 			for (nCols = 0;nCols < nColCnt;nCols ++)
/*?*/ 			{
/*?*/ 				long nIndex = nCols + nRows * nColCnt;
/*?*/ 
/*?*/ 				if (!bIsDirty)
/*?*/ 					if ((pDescription [nIndex].eDescr == CHDESCR_PERCENT) ||
/*?*/ 						(pDescription [nIndex].eDescr == CHDESCR_TEXTANDPERCENT))
/*?*/ 						if (fTotal > 100.0000001)
/*?*/ 						{
/*?*/ 							DBG_BF_ASSERT(0, "STRIP"); //STRIP001 DBG_ERROR2( "Dirty2D: COL value is being changed total=%lf, Vman=%lf",
/*?*/ 						}
/*?*/                         
/*?*/ 				if (pDescription[ nIndex ].fValue != DBL_MIN)
/*?*/ 					if( pDescription[ nIndex ].pLabelObj )
/*?*/ 						pDescrLists[nRows]->NbcInsertObject( pDescription[ nIndex ].pLabelObj );
/*?*/ 			}
/*?*/ 		}
/*?*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Trage ggf. Mittelwert und Fehlerbalken ein
|*
\************************************************************************/

/*************************************************************************
|*
|* Erstelle Kreissegment
|*
\************************************************************************/
/*N*/ SdrObject *ChartModel::CreatePieSegment(SfxItemSet &rAttr,
/*N*/ 							 Rectangle  &rRect,
/*N*/ 							 long       nCol,
/*N*/ 							 long       nRow,
/*N*/ 							 long       nStartAngle,
/*N*/ 							 long       nEndAngle,
/*N*/ 							 long       nCnt)
/*N*/ {
/*N*/ 	SdrObject  *pObj;
/*N*/ 
/*N*/ 	// BM: Attention: Small angles are treated as equal, if they are equal
/*N*/ 	//     after divided by 10. That is because that is also done in the
/*N*/ 	//	   drawing layer. Giving angles which differ by 1 causes the creation
/*N*/ 	//	   of a full circle.
/*N*/ 
/*N*/ 	if( nCnt == 1 )	 // only one datarow => whole circle
/*N*/ 	{
/*?*/ 		pObj = new SdrCircObj( OBJ_CIRC, rRect );
/*?*/ 		pObj->SetModel( this );
/*?*/ 		SetObjectAttr( pObj, CHOBJID_DIAGRAM_DATA, TRUE, TRUE, &rAttr);
/*N*/ 	}
/*N*/ 	else if( (nStartAngle/10) - (nEndAngle/10) )		// create pie segment
/*N*/ 	{
/*N*/ 		pObj = new SdrCircObj( OBJ_SECT, rRect, nStartAngle, nEndAngle );
/*N*/ 		pObj->SetModel( this );
/*N*/ 		SetObjectAttr( pObj, CHOBJID_DIAGRAM_DATA, TRUE, TRUE, &rAttr );
/*N*/ 	}
/*N*/ 	else // the segment is treated as line
/*N*/ 	{
/*?*/ 		pObj = new SdrCircObj( OBJ_SECT, rRect, nStartAngle, nStartAngle );
/*?*/ 		pObj->SetModel( this );
/*?*/ 		SetObjectAttr( pObj, CHOBJID_DIAGRAM_DATA, TRUE, TRUE, &rAttr );
/*N*/ 	}
/*N*/ 	pObj->InsertUserData(new SchDataPoint((short)nCol, (short)nRow));
/*N*/ 	return pObj;
/*N*/ }

/*************************************************************
|*
|* create donut segment - same as circle segment for now
|*
\*************************************************************/
/*N*/ SdrObject* ChartModel::CreateDonutSegment( SfxItemSet& aAttr,
/*N*/ 										   Rectangle&  aRect, ULONG nWidth,
/*N*/ 										   long nCol,		  long nRow,
/*N*/ 										   long nStartAngle,  long nEndAngle,
/*N*/ 										   long nCount)
/*N*/ {
/*N*/ 	SdrObject* pObj;
/*N*/ 	if( nStartAngle > nEndAngle )
/*N*/ 		nStartAngle += 36000;
/*N*/ 
/*N*/ 	if( nCount==1 || nStartAngle==nEndAngle )	// we have to paint complete ring
/*N*/ 	{
/*?*/ 		pObj = new SdrCircObj( OBJ_CIRC, aRect );
/*?*/ 		pObj->SetModel( this );
/*?*/ 		SetObjectAttr( pObj, CHOBJID_DIAGRAM_DATA, TRUE, TRUE, &aAttr );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pObj = new SdrCircObj( OBJ_SECT, aRect, nStartAngle, nEndAngle );
/*N*/ 		pObj->SetModel( this );
/*N*/ 		SetObjectAttr( pObj, CHOBJID_DIAGRAM_DATA, TRUE, TRUE, &aAttr );
/*N*/ 	}
/*N*/ 	pObj->InsertUserData( new SchDataPoint( (short)nCol, (short)nRow ));
/*N*/ 	return pObj;
/*N*/ }

/*************************************************************************
|*
|* Objektgruppe erzeugen
|*
\************************************************************************/
/*N*/ SdrRectObj *ChartModel::CreateRect (Rectangle  &rRect,long nCol,long nRow,SfxItemSet &rAttr)
/*N*/ {
/*N*/ 	SdrRectObj* pRectObj = new SdrRectObj( rRect );
/*N*/ 	pRectObj->SetModel( this );
/*N*/ 	SetObjectAttr( pRectObj, CHOBJID_DIAGRAM_DATA, TRUE, TRUE, &rAttr );
/*N*/ 	pRectObj->InsertUserData(new SchDataPoint((short) nCol, (short) nRow));
/*N*/ 	return pRectObj;
/*N*/ }
}
