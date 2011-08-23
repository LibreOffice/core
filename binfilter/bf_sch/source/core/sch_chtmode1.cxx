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

#include <bf_svx/svxids.hrc>

#include <bf_svx/xlntrit.hxx>
#include <bf_svx/svdviter.hxx>
#include <bf_svx/svdview.hxx>
#ifndef _ZFORLIST_HXX //autogen
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <bf_svtools/zforlist.hxx>
#endif

#include "schattr.hxx"
#define ITEMID_ADJUST EE_PARA_JUST
#include <bf_svx/adjitem.hxx>

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE	        0
#define ITEMID_CHARTDATADESCR	SCHATTR_DATADESCR_DESCR
#define ITEMID_CHARTTEXTORIENT	SCHATTR_TEXT_ORIENT


#endif

#define ITEMID_FONT        EE_CHAR_FONTINFO
#define ITEMID_COLOR       EE_CHAR_COLOR
#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#include "globfunc.hxx"
#include <bf_svx/svxids.hrc>
#include "schresid.hxx"

#include "glob.hrc"
#include <bf_svx/dialogs.hrc>
#include "math.h"


#include <bf_svx/xlnedcit.hxx>

#include <bf_svx/xlnstcit.hxx>

#include <bf_svx/xlnedwit.hxx>

#include <bf_svx/xlnstwit.hxx>

#include <bf_svx/xlnedit.hxx>

#include <bf_svx/xlnstit.hxx>

#include <bf_svx/xlndsit.hxx>


#include <bf_svx/xlnclit.hxx>

#include <bf_svx/xlnwtit.hxx>

#include "pairs.hxx"
#include "datalog.hxx"

#include "chaxis.hxx"

#include "chmod3d.hxx"
#include "schmod.hxx"
namespace binfilter {

/************************************************************************/

/*N*/ const double fDefaultArr[DEFAULT_ROWCNT][DEFAULT_COLCNT] =
/*N*/ {
/*N*/ 	{ 9.1,	3.2,	4.54 },
/*N*/ 	{ 2.4,	8.8,	9.65 },
/*N*/ 	{ 3.1,	1.5,	3.7 },
/*N*/ 	{ 4.3,	9.02,	6.2 }
/*N*/ };

/*************************************************************************
|*
|* delete top-level object (called by BuildChart to create shape newly)
|*
\************************************************************************/

/*N*/ void ChartModel::DeleteObject( SdrObject* pObj )
/*N*/ {
/*N*/ 	if( pObj )
/*N*/ 	{
/*N*/ 		// unmark object in all views
/*N*/ 		SdrViewIter aIter( pObj );
/*N*/ 		SdrView* pView = aIter.FirstView();
/*N*/ 
/*N*/ 		while( pView )
/*N*/ 		{
/*N*/ 			// important: leave group so that SdrPageView's object list is always
/*N*/ 			// the same as the object list of the page
/*N*/ 			pView->LeaveAllGroup();
/*N*/ 			pView->UnmarkAll();
/*N*/ 
/*N*/ 			pView = aIter.NextView();
/*N*/ 		}
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/   		SdrObject* pDelObj = pObj->GetObjList()->RemoveObject( pObj->GetOrdNum());
/*N*/   		DBG_ASSERT( pDelObj == pObj , "Wrong object removed!" );
/*N*/ #else
/*N*/   		pObj->GetObjList()->RemoveObject( pObj->GetOrdNum());
/*N*/ #endif
/*N*/ 
/*N*/   		delete pObj;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR( "DeleteObject: Invalid object" );
/*N*/ }

/*N*/ const USHORT nExchangeTitleWhichPairs[] =
/*N*/ {                                                    //ca.:
/*N*/ 	SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,       // 1
/*N*/ 	SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //
/*N*/ 	XATTR_LINE_FIRST, XATTR_LINE_LAST,              // 1000
/*N*/ 	XATTR_FILL_FIRST, XATTR_FILL_LAST,              // 1020
/*N*/ 	SDRATTR_START, SDRATTR_END, //hier geändert auf alle SdrAttr
/*N*/ 	EE_ITEMS_START, EE_ITEMS_END,                   // 4000
/*N*/ 	0
/*N*/ };

/*************************************************************************
|*
|* Textobjekt fuer Diagramme erzeugen
|* FG: Der Parameter MaximumWidth wird beachtet, falls er groesser als 0 ist.
|*     Dann wird zur Not der Text umgebrochen. Diese Groesse muss auch
|*     CalcMaxDescrSize uebergeben werden, sonst stimmt die Formatierung nicht.
|*     Es werden maximal 2 Zeilen dieser Breite erzeugt der Rest wird abgeschnitten.
|*
|*      Das sollte umgeschrieben werden, denn:
|*      1)  Es wird der Text in ein Outline-Objekt gepackt, damit man mit Stacked-Text
|*          zurecht kommt.
|*      2)  daraus wird die Groesse und die Breite berechnet.
|*      3)  dann wird diese Groesse genommen um ein SdrTextObj mit den gleichen
|*          Attributen zu erzeugen, wie das Outline Objekt.
|*
\************************************************************************/

/*N*/ SdrRectObj* ChartModel::CreateTextObj(UINT16           nId,
/*N*/ 									  const Point      &rPos,
/*N*/ 									  const String     &rText,
/*N*/ 									  const SfxItemSet &rAttr,
/*N*/ 									  BOOL             bIsTitle,
/*N*/ 									  ChartAdjust      eAdjust,
/*N*/ 									  const long       nMaximumWidth)
/*N*/ {
/*N*/ 	const SfxPoolItem* pPoolItem = NULL;
/*N*/ 	SvxChartTextOrient eOrient;
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(SCHATTR_TEXT_ORIENT, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 		eOrient = ((const SvxChartTextOrientItem*)pPoolItem)->GetValue();
/*N*/ 	else
/*N*/ 		eOrient = CHTXTORIENT_STANDARD;
/*N*/ 
/*N*/ 	SfxItemSet aTextAttr(*pItemPool, nTextWhichPairs);
/*N*/ 	aTextAttr.Put(rAttr);
/*N*/ 
/*N*/ 	if (bIsTitle)
/*N*/ 		aTextAttr.Put(SvxAdjustItem(SVX_ADJUST_CENTER));
/*N*/ 	else
/*N*/ 		aTextAttr.Put(SvxAdjustItem((eOrient == CHTXTORIENT_BOTTOMTOP)
/*N*/ 										 ? SVX_ADJUST_RIGHT
/*N*/ 										 : SVX_ADJUST_LEFT));
/*N*/ 
/*N*/     // clear old outliner content
/*N*/ 	pOutliner->Clear();
/*N*/ 
/*N*/ 	if (eOrient == CHTXTORIENT_STACKED)
/*?*/ 		pOutliner->SetText(StackString(rText), pOutliner->GetParagraph( 0 ));
/*N*/ 	else
/*N*/ 		pOutliner->SetText(rText, pOutliner->GetParagraph( 0 ));
/*N*/ 
/*N*/ 		// FG: Diese Routine berechnet nun wirklich ob der Text umgebrochen werden soll oder nicht.
/*N*/ 	Size aSize = CalcTextSizeOfOneText (eOrient, aTextAttr, pOutliner, nMaximumWidth,FALSE);
/*N*/ 
/*N*/ 		// FG: Was macht das?
/*N*/ 	OutlinerParaObject* pPara =	pOutliner->CreateParaObject();
/*N*/ 
/*N*/ 		// FG: Hier wird der Text der oben muehsam erzeugt und formatiert wurde, wieder weggeworfen.
/*N*/ 	pOutliner->Clear();
/*N*/ 
/*N*/ 	//rPos=Position im ChartRect, wird als arg uebergeben,
/*N*/ 	//size ergibt sich aus CalcTextOf... (s.o.)
/*N*/ 	Rectangle aRect(rPos, aSize);
/*N*/ 	SdrRectObj* pObj;
/*N*/ 
/*N*/ 	AdjustRect(aRect, eAdjust);
/*N*/ 
/*N*/ 	switch(nId)
/*N*/ 	{
/*N*/ 		//Alle Titel sorgen selbst für ihre Attributierung:
/*N*/ 		case CHOBJID_DIAGRAM_TITLE_X_AXIS:
/*N*/ 		case CHOBJID_DIAGRAM_TITLE_Y_AXIS:
/*N*/ 		case CHOBJID_DIAGRAM_TITLE_Z_AXIS:
/*N*/ 		case CHOBJID_TITLE_MAIN:
/*N*/ 		case CHOBJID_TITLE_SUB:
/*N*/ 			pObj = new SchRectObj(OBJ_TEXT, aRect);
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			pObj = new SdrRectObj(OBJ_TEXT, aRect);
/*N*/ 			break;
/*N*/ 
/*N*/ 	}
/*N*/ 
/*N*/ 	//Seit 4/1998 koennen Texte frei gedreht werden: SCHATTR_TEXT_DEGREES
/*N*/ 	long nDegrees=GetTextRotation((SfxItemSet&)rAttr,eOrient);
/*N*/ 	if(nDegrees)
/*N*/ 	{
/*N*/ 	   Rectangle aOldBoundRect=pObj->GetBoundRect();
/*N*/ 	   double fVal=nDegrees * nPi180;
/*N*/ 	   pObj->Rotate(pObj->GetSnapRect().Center(), nDegrees, sin(fVal), cos(fVal));
/*N*/ 	   pObj->NbcMove( AdjustRotatedRect(aOldBoundRect, eAdjust,pObj->GetBoundRect()));
/*N*/ 	}
/*N*/ 
/*N*/ 	//ToDo: anhängen der ,*(pItemPool->GetFrozenIdRanges()) ???, erstmal mit diesen Whichpairs
/*N*/ 	SfxItemSet aAreaAttr(*pItemPool,nExchangeTitleWhichPairs);
/*N*/ 
/*N*/ 	//Neu: #52009#
/*N*/ 	aAreaAttr.Put(SdrTextAutoGrowHeightItem( bIsTitle ));
/*N*/ 	aAreaAttr.Put(SdrTextAutoGrowWidthItem( bIsTitle ));
/*N*/ 	aAreaAttr.Put(rAttr);
/*N*/ 
/*N*/ 
/*N*/ //-/	pObj->NbcSetAttributes(aAreaAttr, FALSE);//#63904# 10%
/*N*/ 	pObj->SetItemSet(aAreaAttr);//#63904# 10%
/*N*/ 
/*N*/ 	pObj->InsertUserData(new SchObjectId(nId));
/*N*/ 	pObj->InsertUserData(new SchObjectAdjust(eAdjust, eOrient));
/*N*/ 	pObj->NbcSetOutlinerParaObject(pPara);
/*N*/ 
/*N*/ 	return pObj;
/*N*/ }

/*************************************************************************
|*
|* Text von Diagramm-Textobjekten setzen
|*
\************************************************************************/


/*************************************************************************
|*
|* Attribute von Diagramm-Textobjekten setzen
|*
\************************************************************************/

/*N*/ void ChartModel::SetTextAttr(SdrTextObj& rTextObj,
/*N*/ 							 const SfxItemSet& rAttr,
/*N*/ 							 const long nMaximumWidth)
/*N*/ {
/*N*/     // #97992# calling SetItemSet results in changing the p...Attr in the ChartModel
/*N*/     // because of the implementation of SchRectObj.  Maybe this is wrong, but it seemed
/*N*/     // to risky, therefore I set here all the correct items to prevent loss
/*N*/ //	SfxItemSet aObjAttr( *pItemPool, nAreaWhichPairs );
/*N*/ //	aObjAttr.Put(rAttr);
/*N*/ //	rTextObj.SetItemSetAndBroadcast(aAreaAttr);
/*N*/     rTextObj.SetItemSetAndBroadcast( rAttr );
/*N*/ 
/*N*/ 	if ( rTextObj.GetOutlinerParaObject())
/*N*/ 	{
/*N*/ 		SfxItemSet aTextAttr(*pItemPool, nTextWhichPairs);
/*N*/ 		aTextAttr.Put(rAttr);
/*N*/ 		aTextAttr.Put(SvxAdjustItem());
/*N*/ 
/*N*/ 		pOutliner->SetText(*rTextObj.GetOutlinerParaObject());
/*N*/ 
/*N*/ 		SetTextAttributes (aTextAttr);
/*N*/ 
/*N*/ 		if(IsAttrChangeNeedsBuildChart(rAttr))
/*N*/ 		{
/*N*/ 			//in diesem Fall koennte ein Textresize/reorg noetig sein
/*N*/ 
/*N*/ 			Size aSize = pOutliner->CalcTextSize();
/*N*/ 			aSize.Height() += TEXTHEIGHT_OFS;
/*N*/ 			aSize.Width () = (aSize.Width () * 6) / 5;
/*N*/ 
/*N*/ 			OutlinerParaObject* pPara =	pOutliner->CreateParaObject();
/*N*/ 
/*N*/ 			pOutliner->Clear();
/*N*/ 
/*N*/ 			rTextObj.SetOutlinerParaObject(pPara);
/*N*/ 			AdjustTextSize(rTextObj, aSize);
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|* Liefert die maximale Beschriftungs-Text-Groesse
|* FG: Hier wird der groesste Text berechnet und die Groesse des ersten
|*     und letzten (je nach Chart-Typ braucht man einen oder alle 3 Werte
|*     nNumberFormat ist der Index fuer ein Zahlenformat, dies kann noetig
|*       sin um die wahre Textbreite und Hoehe auszurechnen
|*     nMaximumWidth ist die Breite ab der der Text umgebrochen wird
|*
\************************************************************************/

/*N*/ Size ChartModel::CalcMaxDescrSize(BOOL               bRowDescr,
/*N*/ 								  SvxChartTextOrient eOrient,
/*N*/ 								  const UINT32       nNumberFormat,
/*N*/ 								  long				 nAxisUId,
/*N*/ 								  const long         MaximumWidth,
/*N*/ 								  Pair*				 pFirstAndLast )
/*N*/ {
/*N*/ 	ChartAxis *pCurrentXAxis = (nAxisUId == CHAXIS_AXIS_A)? pChartAAxis: pChartXAxis;
/*N*/ 
/*N*/ 	BOOL bLogarithm = pCurrentXAxis->IsLogarithm();
/*N*/ 	short nCnt;
/*N*/ 	SfxItemSet* pAxisAttr;
/*N*/ 	if (bRowDescr)
/*N*/ 	{
/*N*/ 		nCnt			= GetRowCount();
/*N*/ 		pAxisAttr		= &GetAttr(CHOBJID_DIAGRAM_Z_AXIS);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nCnt			= GetColCount();
/*N*/ 		pAxisAttr		= &GetAttr(CHOBJID_DIAGRAM_X_AXIS);
/*N*/ 	}
/*N*/ 
/*N*/ 	pOutliner->SetUpdateMode (FALSE);
/*N*/ 
/*N*/ 	Size aMaxSize(0, 0);
/*N*/ 
/*N*/ 	if (IsXYChart())
/*N*/ 	{
        /**********************************************************************
        * XY-Chart oder Zeilen muessen zusammengefasst werden
        **********************************************************************/
/*N*/ 		if (/*IsXYChart() && */pCurrentXAxis->GetMin() == pCurrentXAxis->GetMax())//#55400#
/*N*/ 		{
/*?*/ 			return Size (0, 0);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 
/*N*/ 			// #55400#
/*N*/ 			// #67961#
/*N*/ 			double fMinX  = pCurrentXAxis->GetMin();
/*N*/ 			double fMaxX  = pCurrentXAxis->GetMax();
/*N*/ 			double fStepX = pCurrentXAxis->GetStep();
/*N*/ 
/*N*/ 			//	Collect the items which control the text appearence and set them to
/*N*/ 			//	the outliner.
/*N*/ 			SfxItemSet aTextAttr(*pItemPool, nTextOrientWhichPairs);
/*N*/ 			aTextAttr.Put(GetAttr(CHOBJID_DIAGRAM_X_AXIS));
/*N*/ //			SetTextAttributes (aTextAttr);
/*N*/ 
/*N*/ 			double fAct = fMinX;
/*N*/ 
/*N*/ 			while (fAct <= fMaxX)
/*N*/ 			{
/*N*/ 				String aNumStr;
/*N*/ 				Color* pDummy = NULL;
/*N*/ 
/*N*/                 pNumFormatter->GetOutputString(fAct, nNumberFormat, aNumStr, &pDummy);
/*N*/ 
/*N*/ 				if (eOrient == CHTXTORIENT_STACKED)
/*?*/ 					pOutliner->SetText(StackString(aNumStr), pOutliner->GetParagraph( 0 ));
/*N*/ 				else
/*N*/ 					pOutliner->SetText(aNumStr, pOutliner->GetParagraph( 0 ));
/*N*/ 
/*N*/ 				// FG: Hier wird wirklich berechnet wie groß der Textbereich werden soll. Insbesondere
/*N*/ 				//     wird hier entschieden, ob der Text umgebrochen werden soll oder nicht!
/*N*/ 				SetTextAttributes (aTextAttr);
/*N*/ 				Size aSize = CalcTextSizeOfOneText (eOrient, aTextAttr, pOutliner, MaximumWidth,
/*N*/ 					TRUE, FALSE);
/*N*/ 				pOutliner->SetUpdateMode (FALSE);
/*N*/ 
/*N*/ 				pOutliner->Clear();
/*N*/ 
/*N*/ 				if (aSize.Width() > aMaxSize.Width())
/*N*/ 					aMaxSize.Width() = aSize.Width();
/*N*/ 				if (aSize.Height() > aMaxSize.Height())
/*N*/ 					aMaxSize.Height() = aSize.Height();
/*N*/ 
/*N*/ 					// FG: Die Berechnung erfolgt hier, damit die Raender in Create2DBackplane
/*N*/ 				if (fAct <= fMinX)
/*N*/ 				{
/*N*/ 					nWidthOfFirstXAxisText = aSize.Width();
/*N*/ 					if(pFirstAndLast)
/*N*/ 						pFirstAndLast->A()=nWidthOfFirstXAxisText;
/*N*/ 
/*N*/ 				}
/*N*/ 				IncValue(fAct, fStepX, bLogarithm);
/*N*/ 					// FG: Die Berechnung erfolgt hier, damit die Raender in Create2DBackplane
/*N*/ 				if (fAct >= fMaxX)
/*N*/ 				{
/*N*/ 					nWidthOfLastXAxisText = aSize.Width();
/*N*/ 					if(pFirstAndLast)
/*N*/ 						pFirstAndLast->B()=nWidthOfLastXAxisText;
/*N*/ 				}
/*N*/ 
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//	Collect the items which control the text appearence and set them to
/*N*/ 		//	the outliner.
/*N*/ 		SfxItemSet aTextAttr(*pItemPool, nTextOrientWhichPairs);
/*N*/ 		aTextAttr.Put(*pAxisAttr);
/*N*/ //		SetTextAttributes (aTextAttr);
/*N*/ 
/*N*/ 		for (short i = 0; i < nCnt; i++)
/*N*/ 		{
/*N*/ 			String aDescrStr = bRowDescr ? RowText(i) : ColText(i);
/*N*/ 
/*N*/ 			if (eOrient == CHTXTORIENT_STACKED)
/*?*/ 				pOutliner->SetText(StackString(aDescrStr), pOutliner->GetParagraph( 0 ));
/*N*/ 			else
/*N*/ 				pOutliner->SetText(aDescrStr, pOutliner->GetParagraph( 0 ));
/*N*/ 
/*N*/ 			// FG: Hier wird wirklich berechnet wie groß der Textbereich werden soll. Insbesondere wird
/*N*/ 			//     hier entschieden, ob der Text umgebrochen werden soll oder nicht!
/*N*/ 			SetTextAttributes (aTextAttr);
/*N*/ 			Size aSize = CalcTextSizeOfOneText (eOrient, aTextAttr, pOutliner, MaximumWidth,TRUE,
/*N*/ 				FALSE);
/*N*/ 			pOutliner->SetUpdateMode (FALSE);
/*N*/ 			pOutliner->Clear();
/*N*/ 
/*N*/ 			if (aSize.Width() > aMaxSize.Width())
/*N*/ 				aMaxSize.Width() = aSize.Width();
/*N*/ 			if (aSize.Height() > aMaxSize.Height())
/*N*/ 				aMaxSize.Height() = aSize.Height();
/*N*/ 			// FG: Die Berechnung erfolgt hier, damit die Raender in Create2DBackplane
/*N*/ 			//     richtig berechnet werden koennen.
/*N*/ 			if (i == 0)
/*N*/ 			{
/*N*/ 				nWidthOfFirstXAxisText = aSize.Width();
/*N*/ 				if(pFirstAndLast)
/*N*/ 					pFirstAndLast->A()=nWidthOfFirstXAxisText;
/*N*/ 			}
/*N*/ 			if (i == nCnt-1)
/*N*/ 			{
/*N*/ 				nWidthOfLastXAxisText = aSize.Width();
/*N*/ 				if(pFirstAndLast)
/*N*/ 					pFirstAndLast->B()=nWidthOfLastXAxisText;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pOutliner->SetUpdateMode (TRUE);
/*N*/ 
/*N*/ 	return aMaxSize;
/*N*/ }

/*************************************************************************
|*
|* Text-H”he ermitteln
|*
\************************************************************************/


/*************************************************************************
|*
|* Datenzeilen-Attributsets initialisieren
|*
\************************************************************************/

//
// schneller machen, es muß nicht alles neu gebaut werden
//

/*N*/ void ChartModel::InitDataAttrs()
/*N*/ {
/*N*/ 	CHART_TRACE( "ChartModel::InitDataAttrs" );
/*N*/ 
/*N*/ 	DBG_ASSERT( pChartData, "ChartModel::InitDataAttrs: No ChartData-Object available!" );
/*N*/ 
/*N*/ 	short nDataColCnt	= pChartData->GetColCount();
/*N*/ 	short nDataRowCnt	= pChartData->GetRowCount();

    /* For pie charts we need the maximum of both dimensions, because the pie interprets data a bit
     * odd: only the first series (data-row) is used for the pie.  Each segment is a data point of
     * this single series.  However to get the correct colors the array containing series attributes
     * is used instead of the data point attributes.  Thus for a 1x10 data we need 10 series
     * attributes although we have only one series.  However for 20x10 data we need 20 series
     * attributes, because we actually have 20 series although just the first one is visible.  But
     * we need those attributes in case the chart type is switched.
     */
/*N*/     short nCnt          = IsPieChart()
/*N*/         ? ::std::max( GetColCount(), GetRowCount())
/*N*/         : GetRowCount();
/*N*/ 
/*N*/     short i;
/*N*/ 
/*N*/ 	if (nCnt != nPieSegCount)
/*N*/ 	{
/*N*/ 		long *pOfs = new long[nCnt];
/*N*/ 
/*N*/ 		if (nPieSegCount > nCnt)
/*N*/ 			for (i = 0; i < nCnt; i++)
/*N*/ 				pOfs[i] = pPieSegOfs[i];
/*N*/ 		else
/*N*/ 		{
/*N*/ 			for (i = 0; i < nPieSegCount; i++)
/*N*/ 				pOfs[i] = pPieSegOfs[i];
/*N*/ 			for (; i < nCnt; i++)
/*N*/ 				pOfs[i] = 0;
/*N*/ 		}
/*N*/ 
/*N*/ 		delete[] pPieSegOfs;
/*N*/ 		pPieSegOfs = pOfs;
/*N*/ 		nPieSegCount = nCnt;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	long nRowListCnt ;
/*N*/ 	//regressattr
/*N*/ 	nRowListCnt = (short)aRegressAttrList.Count();
/*N*/ 
/*N*/ 	DBG_ASSERT( pDefaultColors, "invalid default colors" );
/*N*/ 	sal_Int32 nNumDefCol = pDefaultColors->Count();
/*N*/ 	DBG_ASSERT( nNumDefCol, "Empty Default Color List" );
/*N*/ 
/*N*/ 	if (nCnt != nRowListCnt)
/*N*/ 	{
/*N*/ 		if (nRowListCnt > nCnt)
/*N*/ 		{
/*N*/ 			aRegressAttrList.Seek((ULONG)nCnt);
/*N*/ 			for (i = nCnt; i < nRowListCnt; i++)
/*N*/ 				delete aRegressAttrList.Remove();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			for (i = nRowListCnt; i < nCnt; i++)
/*N*/ 			{
/*N*/ 				SfxItemSet* pRegressAttr = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 				aRegressAttrList.Insert(pRegressAttr, LIST_APPEND);
/*N*/ 
/*N*/ 				pRegressAttr->Put(XLineStyleItem(XLINE_SOLID));
/*N*/ 				pRegressAttr->Put(XLineWidthItem(100));
/*N*/ 				if( nNumDefCol != 0 )
/*N*/ 				{
/*N*/ 					XColorEntry* pEntry = (XColorEntry*)pDefaultColors->
/*N*/ 						GetObject(i % nNumDefCol);
/*N*/ 					pRegressAttr->Put(XLineColorItem(pEntry->GetName(),
/*N*/ 													 pEntry->GetColor()));
/*N*/ 				}
/*N*/ 				pRegressAttr->Put(XLineDashItem());
/*N*/ 				pRegressAttr->Put(XLineStartItem());
/*N*/ 				pRegressAttr->Put(XLineEndItem());
/*N*/ 				pRegressAttr->Put(XLineStartWidthItem());
/*N*/ 				pRegressAttr->Put(XLineEndWidthItem());
/*N*/ 				pRegressAttr->Put(XLineStartCenterItem());
/*N*/ 				pRegressAttr->Put(XLineEndCenterItem());
/*N*/ 				pRegressAttr->Put(XLineTransparenceItem());
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//average attr
/*N*/ 	nRowListCnt = (short)aAverageAttrList.Count();
/*N*/ 	if (nCnt != nRowListCnt)
/*N*/ 		if (nRowListCnt > nCnt)
/*N*/ 		{
/*N*/ 			aAverageAttrList.Seek((ULONG)nCnt);
/*N*/ 			for (i = nCnt; i < nRowListCnt; i++)
/*N*/ 				delete aAverageAttrList.Remove();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			for (i = nRowListCnt; i < nCnt; i++)
/*N*/ 			{
/*N*/ 				SfxItemSet* pAverageAttr = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 				aAverageAttrList.Insert(pAverageAttr, LIST_APPEND);
/*N*/ 
/*N*/ 				pAverageAttr->Put(XLineStyleItem(XLINE_SOLID));
/*N*/ 				pAverageAttr->Put(XLineWidthItem(0));
/*N*/ 				if( nNumDefCol != 0 )
/*N*/ 				{
/*N*/ 					XColorEntry* pEntry = (XColorEntry*)pDefaultColors->
/*N*/ 						GetObject(i % nNumDefCol);
/*N*/ 					pAverageAttr->Put(XLineColorItem(pEntry->GetName(),
/*N*/ 													 pEntry->GetColor()));
/*N*/ 				}
/*N*/ 				pAverageAttr->Put(XLineDashItem());
/*N*/ 				pAverageAttr->Put(XLineStartItem());
/*N*/ 				pAverageAttr->Put(XLineEndItem());
/*N*/ 				pAverageAttr->Put(XLineStartWidthItem());
/*N*/ 				pAverageAttr->Put(XLineEndWidthItem());
/*N*/ 				pAverageAttr->Put(XLineStartCenterItem());
/*N*/ 				pAverageAttr->Put(XLineEndCenterItem());
/*N*/ 				pAverageAttr->Put(XLineTransparenceItem());
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 	//error attr
/*N*/ 	nRowListCnt = (short)aErrorAttrList.Count();
/*N*/ 	if (nCnt != nRowListCnt)
/*N*/ 		if (nRowListCnt > nCnt)
/*N*/ 		{
/*N*/ 			aErrorAttrList.Seek((ULONG)nCnt);
/*N*/ 			for (i = nCnt; i < nRowListCnt; i++)
/*N*/ 				delete aErrorAttrList.Remove();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			for (i = nRowListCnt; i < nCnt; i++)
/*N*/ 			{
/*N*/ 				SfxItemSet* pErrorAttr = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 				aErrorAttrList.Insert(pErrorAttr, LIST_APPEND);
/*N*/ 
/*N*/ 				pErrorAttr->Put(XLineStyleItem(XLINE_SOLID));
/*N*/ 				pErrorAttr->Put(XLineWidthItem(0));
/*N*/ 				pErrorAttr->Put(XLineColorItem(String(), RGBColor(COL_BLACK)));
/*N*/ 				pErrorAttr->Put(XLineDashItem());
/*N*/ 				pErrorAttr->Put(XLineStartItem());
/*N*/ 				pErrorAttr->Put(XLineEndItem());
/*N*/ 				pErrorAttr->Put(XLineStartWidthItem());
/*N*/ 				pErrorAttr->Put(XLineEndWidthItem());
/*N*/ 				pErrorAttr->Put(XLineStartCenterItem());
/*N*/ 				pErrorAttr->Put(XLineEndCenterItem());
/*N*/ 				pErrorAttr->Put(XLineTransparenceItem());
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 	// Point-Attr
/*N*/ 	long nPointCnt		= nDataColCnt * nDataRowCnt;
/*N*/ 	long nPointListCnt	= aDataPointAttrList.Count();
/*N*/ 	if (nPointCnt != nPointListCnt)
/*N*/ 		if (nPointListCnt > nPointCnt)
/*N*/ 		{
/*N*/ //			aDataPointAttrList.Seek((ULONG)nPointCnt);
/*N*/ //			for (long i = nPointCnt; i < nPointListCnt; i++)
/*N*/ //				delete aDataPointAttrList.Remove();
/*N*/ 			while (nPointListCnt-- > nPointCnt)
/*N*/ 			{
/*N*/ 				aDataPointAttrList.Seek((ULONG)nPointCnt);
/*N*/ 				delete aDataPointAttrList.Remove();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else for (long ii = nPointListCnt; ii < nPointCnt; ii++)
/*N*/ 			 aDataPointAttrList.Insert(NULL, LIST_APPEND);
/*N*/ 
/*N*/ 	// Switch-Point-Attr
/*N*/ 	nPointListCnt = aSwitchDataPointAttrList.Count();
/*N*/ 	if (nPointCnt != nPointListCnt)
/*N*/ 	{
/*N*/ 		if (nPointListCnt > nPointCnt)
/*N*/ 		{
/*N*/ //			aSwitchDataPointAttrList.Seek((ULONG)nPointCnt);
/*N*/ //			for (long i = nPointCnt; i < nPointListCnt; i++)
/*N*/ //				delete aSwitchDataPointAttrList.Remove();
/*N*/ 			while (nPointListCnt-- > nPointCnt)
/*N*/ 			{
/*N*/ 				aSwitchDataPointAttrList.Seek((ULONG)nPointCnt);
/*N*/ 				delete aSwitchDataPointAttrList.Remove();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else for (long iii = nPointListCnt; iii < nPointCnt; iii++)
/*N*/ 			aSwitchDataPointAttrList.Insert(NULL, LIST_APPEND);
/*N*/ 			// Insert (new SfxItemSet(*pItemPool, nRowWhichPairs),...)
/*N*/ 	}
/*N*/ 
/*N*/ 	//row attr
/*N*/ 	nRowListCnt = (short)aDataRowAttrList.Count();
/*N*/ 	if (nCnt != nRowListCnt)
/*N*/ 	{
/*N*/ 		if (nRowListCnt > nCnt)
/*N*/ 		{
/*N*/ 			//bevor attribute geloescht werden, wird reorganisiert
/*N*/ 			LogBookAttrData();
/*N*/ 
/*N*/ 			//Jetzt darf erst der Ueberhang geloescht werden:
/*N*/ 			aDataRowAttrList.Seek((ULONG)nCnt);
/*N*/ 			for (i = nCnt; i < nRowListCnt; i++)
/*N*/ 				delete aDataRowAttrList.Remove();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             bool bIsCombiChart =
/*N*/                 ( CHSTYLE_2D_LINE_COLUMN         ==  eChartStyle ) ||
/*N*/                 ( CHSTYLE_2D_LINE_STACKEDCOLUMN  ==  eChartStyle );
/*N*/ 
/*N*/             for (i = nRowListCnt; i < nCnt; i++)
/*N*/ 			{
/*N*/ 				SfxItemSet* pDataRowAttr = new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/ 				aDataRowAttrList.Insert(pDataRowAttr, LIST_APPEND);
/*N*/ 				SetDefAttrRow(pDataRowAttr,i);
/*N*/ 
/*N*/  				//	Change the defaults for lines in mixed line-column charts.
/*N*/  				if( bIsCombiChart && IsLine( i ) )
/*N*/                 {
/*N*/                     pDataRowAttr->ClearItem (SCHATTR_STYLE_SYMBOL);
/*N*/                     pDataRowAttr->Put (XLineStyleItem (XLINE_SOLID));
/*N*/                     // #101164# as more than one line is possible via GUI, those
/*N*/                     // should not all be black
/*N*/ //                     pDataRowAttr->Put (XLineColorItem (String(), RGBColor (COL_BLACK)));
/*N*/                     pDataRowAttr->Put (XLineWidthItem (0));
/*N*/                 }
/*N*/ 			}
/*N*/             if( ! bIsCombiChart )
/*N*/                 SetupLineColors( SETLINES_FILLCOLOR, nRowListCnt );
/*N*/ 		}
/*N*/ 	}
/*N*/     LogBookAttrData();
/*N*/ }


/*************************************************************************
|*
|* ggf. Attribute neu Organisieren erzeugen
|*
\************************************************************************/
/*N*/ void ChartModel::LogBookAttrData()
/*N*/ {
/*N*/ 	CHART_TRACE( "ChartModel::LogBookAttrData" );
/*N*/ 
/*N*/ 	if(pLogBook)
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if(pLogBook->IsValid())
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|* RowAttr neu setzen:
|*
\************************************************************************/
/*N*/ void ChartModel::SetDefAttrRow(SfxItemSet* pDataRowAttr,const long i)
/*N*/ {
/*N*/ 	DBG_ASSERT( pDefaultColors, "Invalid default color list" );
/*N*/ 	if( ! pDefaultColors )
/*N*/ 		return;
/*N*/ 
/*N*/ 	CHART_TRACE1( "ChartModel::SetDefAttrRow - Row #%ld", i );
/*N*/ 
/*N*/ 	sal_Int32 nNumDefCol = pDefaultColors->Count();
/*N*/ 	DBG_ASSERT( nNumDefCol, "invalid default colors" );
/*N*/ 
/*N*/ 	pDataRowAttr->Put(*pDummyAttr);
/*N*/ 	if( nNumDefCol != 0 )
/*N*/ 	{
/*N*/ 		XColorEntry* pEntry = (XColorEntry*)pDefaultColors->
/*N*/ 			GetObject(i % nNumDefCol);
/*N*/ 		pDataRowAttr->Put(XFillColorItem(pEntry->GetName(),
/*N*/ 										 pEntry->GetColor()));
/*N*/ 
/*N*/ 		if(IsLine(i)) //#54870# bei Linien defaultfarbe der Linie=FillColor
/*N*/ 		{
/*N*/ 			pDataRowAttr->Put(XLineColorItem(pEntry->GetName(),
/*N*/ 											 pEntry->GetColor()));
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		pDataRowAttr->Put(XLineColorItem(String(), RGBColor(COL_BLACK)));
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|* ggf. Demo-Chart erzeugen
|*
\************************************************************************/

/*N*/ void ChartModel::InitChartData(BOOL bNewTitles)
/*N*/ {
/*N*/ 	if (!pChartData)
/*N*/ 	{
/*N*/ 		SchMemChart* pMemChart = new SchMemChart(DEFAULT_COLCNT, DEFAULT_ROWCNT);
/*N*/ 
/*N*/ 		pMemChart->SetMainTitle(String(SchResId(STR_TITLE_MAIN)));
/*N*/ 		pMemChart->SetSubTitle(String(SchResId(STR_TITLE_SUB)));
/*N*/ 		pMemChart->SetXAxisTitle(String(SchResId(STR_DIAGRAM_TITLE_X_AXIS)));
/*N*/ 		pMemChart->SetYAxisTitle(String(SchResId(STR_DIAGRAM_TITLE_Y_AXIS)));
/*N*/ 		pMemChart->SetZAxisTitle(String(SchResId(STR_DIAGRAM_TITLE_Z_AXIS)));
/*N*/ 
/*N*/ 		for( short nCol = 0; nCol < DEFAULT_COLCNT; nCol++ )
/*N*/ 		{
/*N*/ 			pMemChart->SetColText( nCol, pMemChart->GetDefaultColumnText( nCol ));
/*N*/ 
/*N*/ 			for( short nRow = 0; nRow < DEFAULT_ROWCNT; nRow++ )
/*N*/ 			{
/*N*/  				pMemChart->SetData( nCol, nRow, fDefaultArr[ nRow ][ nCol ] );
/*N*/ 				pMemChart->SetRowText( nRow, pMemChart->GetDefaultRowText( nRow ));
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		SetChartData(*pMemChart, bNewTitles);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Defaultfarben erstellen
|*
\************************************************************************/

/*N*/ void ChartModel::CreateDefaultColors ()
/*N*/ {
/*N*/ 	SchOptions* pOptions = SCH_MOD1()->GetSchOptions();
/*N*/ 	long nCount;
/*N*/ 	ColorData* pDefaultCol = NULL;
/*N*/ 
/*N*/ 	if( pOptions )
/*N*/ 	{
/*N*/ 		const SchColorTable& aDefCols = pOptions->GetDefaultColors();
/*N*/ 		nCount = aDefCols.Count();
/*N*/ 		pDefaultCol = new ColorData[ nCount ];
/*N*/  		DBG_ASSERT( nCount == ROW_COLOR_COUNT, "Chart: dynamic default color array size not supported yet" );
/*N*/ 
/*N*/ 		for( int i=0; i<nCount; i++ )
/*N*/ 		{
/*N*/ 			pDefaultCol[ i ] = aDefCols.GetColorData( i );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		nCount = ROW_COLOR_COUNT;
/*?*/ 		pDefaultCol = new ColorData[ nCount ];
/*?*/ 
/*?*/ 		pDefaultCol[  0 ]  = RGB_COLORDATA( 0x99,  0x99, 0xff );
/*?*/ 		pDefaultCol[  1 ]  = RGB_COLORDATA( 0x99,  0x33, 0x66 );
/*?*/ 		pDefaultCol[  2 ]  = RGB_COLORDATA( 0xff,  0xff, 0xcc );
/*?*/ 		pDefaultCol[  3 ]  = RGB_COLORDATA( 0xcc,  0xff, 0xff );
/*?*/ 		pDefaultCol[  4 ]  = RGB_COLORDATA( 0x66,  0x00, 0x66 );
/*?*/ 		pDefaultCol[  5 ]  = RGB_COLORDATA( 0xff,  0x80, 0x80 );
/*?*/ 		pDefaultCol[  6 ]  = RGB_COLORDATA( 0x00,  0x66, 0xcc );
/*?*/ 		pDefaultCol[  7 ]  = RGB_COLORDATA( 0xcc,  0xcc, 0xff );
/*?*/ 		pDefaultCol[  8 ]  = RGB_COLORDATA( 0x00,  0x00, 0x80 );
/*?*/ 		pDefaultCol[  9 ]  = RGB_COLORDATA( 0xff,  0x00, 0xff );
/*?*/ 		pDefaultCol[ 10 ]  = RGB_COLORDATA( 0x00,  0xff, 0xff );
/*?*/ 		pDefaultCol[ 11 ]  = RGB_COLORDATA( 0xff,  0xff, 0x00 );
/*N*/ 	}
/*N*/ 
/*N*/ 	// create colors from table if they exist otherwise copy default colors
/*N*/  	pDefaultColors = new List;
/*N*/ 	Color aCol;
/*N*/ 
/*N*/ 	for( int i=0; i<nCount; i++ )
/*N*/ 	{
/*N*/ 		aCol.SetColor( pDefaultCol[ i ] );
/*N*/ 		pDefaultColors->Insert( new XColorEntry( aCol, String() ), LIST_APPEND );
/*N*/ 	}
/*N*/ 
/*N*/ 	delete[] pDefaultCol;
/*N*/ }

/*************************************************************************
|*
|* Defaultfarben kopieren
|*
\************************************************************************/


/*************************************************************************
|*
|* Defaultfarben loeschen
|*
\************************************************************************/

/*N*/ void ChartModel::DestroyDefaultColors ()
/*N*/ {
/*N*/ 	if (pDefaultColors)
/*N*/ 	{
/*N*/ 		while (pDefaultColors->Count())
/*N*/ 			delete (XColorEntry*)pDefaultColors->Remove(pDefaultColors->Count() - 1);
/*N*/ 		delete pDefaultColors;
/*N*/ 	}
/*N*/ 
/*N*/ 	pDefaultColors = 0;
/*N*/ }

/*************************************************************************
|*
|* Berechnung der Breite und der Hoehe eines Textes
|*
|*    Derzeit ist diese funktion totaler Muell: Im wesentlichen werden die Werte direkt gesetzt
|*    Hoehe = 2 * Texthoehe, Breite hoechstens MaximumWidth
|*
|* FG: Damit dies Funktioniert muss per SetText im Outliner (pOutliner)
|*     der Text schon gesetzt sein!
|*
\************************************************************************/
#define MAXLEGENDLINES 3  //#49908# #NACHTRAG#
#define CDEG2RAD(fAngle) (  (double)(fAngle)*F_PI/18000.0 )


/*N*/ void	ChartModel::SetTextAttributes	(SfxItemSet & rTextAttributes)
/*N*/ {
/*	static	SfxItemSet *	pLastItemSet = NULL;
    if (&rTextAttributes == pLastItemSet)
        return;
    else
        pLastItemSet = &rTextAttributes;
*/
/*N*/ 	ULONG nParagraphCount = pOutliner->GetParagraphCount();
/*N*/ 	for (ULONG i=0; i<nParagraphCount; i++)
/*N*/ 		pOutliner->SetParaAttribs(i, rTextAttributes);
/*N*/ }


// bGetRotated sollte TRUE sein, wenn der verbrauchte Platz des Textes gemessen werden soll,
// und False, falls das TextRect berechnet und dann gedreht wird (sonst doppelte Drehung)
//	The flag bSetTextAttributes tells the method wether to set the given attributes to the
//	outliner.
/*N*/ Size ChartModel::CalcTextSizeOfOneText (SvxChartTextOrient eOrient,
/*N*/ 										SfxItemSet         &rTextAttr,
/*N*/ 										SdrOutliner        *pOutliner,
/*N*/ 										long	MaxW,
/*N*/ 										BOOL	bGetRotated/*=FALSE*/,
/*N*/ 										BOOL	bUseTextAttributes)
/*N*/ {
/*N*/ 	long MaximumWidth=MaxW;
/*N*/ 
/*N*/ 	pOutliner->SetUpdateMode (FALSE);
/*N*/ 	ULONG nParaCnt = pOutliner->GetParagraphCount();
/*N*/ 
/*N*/ 		// FG: Jeder Absatz muss die Text-Attribute einzeln zugewiesen bekommen. (jedenfalls scheint es so)
/*N*/ 		//     Besser waere es dass fuer alle Absaetze auf einmal setzen zu koennen.
/*N*/ 	if (bUseTextAttributes)
/*N*/ 		SetTextAttributes (rTextAttr);
/*N*/ 
/*N*/ 	Size OldPaperSize = pOutliner->GetPaperSize();
/*N*/ 
/*N*/ 	long nDegrees=GetTextRotation((SfxItemSet&)rTextAttr,eOrient);//#62531#
/*N*/ 	double fDeg, fSin,
/*N*/ 		fCos = 1;				// BM: Initialize Cos for if statement after if(nDegrees)-Block
/*N*/ 	BOOL bBreakOK=TRUE; //s.u.
/*N*/ 	if(nDegrees)
/*N*/ 	{
/*N*/ 		//TVM: In einigen Bereichen macht Umbruch keinen Sinn, da die Breite steigt (90 bzw.
/*N*/ 		//270 Grad ist der triviale Fall!) Der genaue Bereich ist hier nicht festgelegt,
/*N*/ 		//denn jeder andere Fall ist leider nicht trivial! (Abhängig davon, wieviele Zeichen
/*N*/ 		//und welche umgebrochen werden, 2 Beispiele:
/*N*/ 		// 1. nur ein 'i' wird umgebrochen    => Text wird fast immer breiter (ausser bei ~ 0 Grad)
/*N*/ 		// 2. der halbe Text wird umgebrochen => fast immer sinnvoll
/*N*/ 		//Die genaue Berechnung erfordert einen Test und ein Undo, wenn es breiter wird
/*N*/ 		//Im folgenden wird einfach die Mitte genommen -> 90% - Lösung
/*N*/ 		bBreakOK =!(   ((nDegrees > 4500) && (nDegrees < 13500))
/*N*/ 					|| ((nDegrees >22500) && (nDegrees < 31500)));
/*N*/ 
/*N*/ 		fDeg=CDEG2RAD(nDegrees);
/*N*/ 		fSin=fabs(sin(fDeg));
/*N*/ 		fCos=fabs(cos(fDeg));
/*N*/ 
/*
  Hat man gedrehten Text, wird ein sinnvoller Umbruch schwierig, da

  1. bei bestimmten Winkeln und bestimmter Textlänge der Text breiter und nicht schmaler wird
  2. Diese Funktion bei Winkeln != 0 mit MaximumWidth u.U. die Höhe vorgegeben bekommt
  (Create2DBackplane tut dies bei gedrehten Texten an der X-Achse)
  untenstehender Code berechnet die vorzugebene MaxBreite, wenn der Text gedreht ist, dies
  wäre leider nur ein Teil der notwendigen Lösung, die so schon recht viel Performance schluckt:

        if( MaximumWidth > 0 && fCos!=0)
        {
            Size aFullSize(pOutliner->CalcTextSize()); //Textgröße ohne Umbruch
            double dW = aFullSize.Height()*fSin;	   //delta width je neue Zeile
            double Wf = aFullSize.Width();			   //Ist-Textbreite durch Soll-Textbreite des Textes ergibt die Anzahl der Zeilen

            double p_halbe			= (dW-(double)MaximumWidth)/(2*fCos);
            double p_halbe_quadrat	= p_halbe * p_halbe;
            double q				= Wf*dW/fCos;
            if(q > p_halbe_quadrat)
            {
                MaximumWidth=-1; //keine Lösung, Abbruch, nix umbrechen!
            }
            else
            {
                long nNewMax1 =(long) (- p_halbe + sqrt(p_halbe_quadrat-q) + 0.5);
                long nNewMax2 =(long) (- p_halbe - sqrt(p_halbe_quadrat-q) + 0.5);
                //Die größere Breite ist immer die bessere
                MaximumWidth=Max(nNewMax1,nNewMax2);
            }
        }
*/
/*N*/ 	}
/*N*/ 
/*N*/ 	if( MaximumWidth > 0 && fCos!=0) //Kein Umbruch bei 90 und 270 Grad oder Max<=0
/*N*/ 		pOutliner->SetPaperSize( Size( MaximumWidth, 0 ) );
/*N*/ 
/*N*/ 
/*N*/ 	pOutliner->SetUpdateMode (TRUE);
/*N*/ 	Size aSize = pOutliner->CalcTextSize();
/*N*/ 	pOutliner->SetUpdateMode (FALSE);
/*N*/ 	Size aRot(aSize);
/*N*/ 	if(nDegrees)
/*N*/ 	{
/*N*/ 		aRot.Width() = (long)( (double)aSize.Width()*fCos + (double)aSize.Height()*fSin );
/*N*/ 		aRot.Height()= (long)( (double)aSize.Width()*fSin + (double)aSize.Height()*fCos );
/*N*/ 	}
/*N*/ 
/*N*/ 		// FG: Diese Groesse wird nun veraendert, falls MaximumWidth > 0 und
/*N*/ 		//     aSize.Width() > MaximumWidth, dann wird umgebrochen, genau einmal.
/*N*/ 		//     Es kommen also hoechstens 2 Zeilen raus, der Rest wird dann abgeschnitten.
/*N*/ 		//     An dieser Stelle werden aber nur die Attribute berechnet.
/*N*/ 
/*N*/ 	if ((MaximumWidth > 0) && (eOrient != CHTXTORIENT_STACKED))
/*N*/ 	{
/*N*/ #ifdef DBG_UTIL
/*N*/ 		if(!(aRot.Width() <= MaximumWidth))
/*N*/ 			DBG_WARNING("ChartModel::CalcTextSizeOfOneText:Doch breiter?" );
/*N*/ #endif
/*N*/ 		ULONG nLines = 0;
/*N*/ 		for( USHORT n = 0; n < pOutliner->GetParagraphCount(); n++ )
/*N*/ 		{
/*N*/ 			nLines += pOutliner->GetLineCount( n );
/*N*/ 		}
/*N*/ 
/*N*/ 		// Silbentrennung nur bei >MAXLEGENDLINES Zeilen oder einem zu langen wort...
/*N*/ 		if ( bBreakOK
/*N*/ 			&& (    ( nLines > MAXLEGENDLINES )
/*N*/ 				 || (    ( nLines >= 2 )
/*N*/ 					  && ( nParaCnt == 1 )
/*N*/ 					  && ( pOutliner->GetText( pOutliner->GetParagraph( 0 ) ).Search( ' ' )
/*N*/ 															 == STRING_NOTFOUND )
/*N*/ 					 )
/*N*/ 			  )
/*N*/ 		   )
/*N*/ 		{
/*?*/ 			if ( nLines > MAXLEGENDLINES )
/*?*/ 			{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	long nHeightOfRows = GetHeightOfnRows (rTextAttr, MAXLEGENDLINES);//war mal 2 statt MAX...#50395#
/*?*/ 			}
/*?*/ 
/*?*/ 			ULONG nParaCnt = pOutliner->GetParagraphCount();
/*?*/ 
/*?*/ 			for (ULONG i = 0; i < nParaCnt; i++)
/*?*/ 			{
/*?*/ 				// Stets Silbentrennung
/*?*/ 				SfxItemSet aAttr(pOutliner->GetParaAttribs(i));
/*?*/ 				aAttr.Put( SfxBoolItem(EE_PARA_HYPHENATE, TRUE) );
/*?*/ 				pOutliner->SetParaAttribs(i, aAttr);
/*?*/ 			}
/*?*/ 
/*?*/ 			//#50395# durch Bindestriche vergrößert worden->
/*?*/ 			//statt 2 werden jetzt 3 Zeilen benötigt
/*?*/ 			ULONG nActLines = 0;
/*?*/ 			for( USHORT n = 0; n < pOutliner->GetParagraphCount(); n++ )
/*?*/ 			{
/*?*/ 				nActLines += pOutliner->GetLineCount( n );
/*?*/ 			}
/*?*/ 			if(nActLines>nLines)
/*?*/ 			{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	nActLines=Min((ULONG)MAXLEGENDLINES,nActLines);
/*N*/ 			}
/*N*/ 
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pOutliner->SetPaperSize(OldPaperSize);
/*N*/ 	pOutliner->SetUpdateMode (TRUE);
/*N*/ 
/*N*/ 	return (bGetRotated && nDegrees) ? aRot : aSize;
/*N*/ }

/*************************************************************************
|*
|*  Liefert die Hoehe von n Textzeilen, mit den uebergebenen Attributen
|* FG: 13.2.97 Hier wird einfach n mal die Texthoehe einer Zeile zurueckgegeben
|*
\************************************************************************/



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
