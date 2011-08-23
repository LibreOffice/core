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
#define ITEMID_DOUBLE           0
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTTEXTORIENT	SCHATTR_TEXT_ORIENT
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS
#define ITEMID_CHARTDATADESCR   SCHATTR_DATADESCR_DESCR


#include <bf_svx/svdopath.hxx>




#include <bf_svx/eeitem.hxx>
#ifndef _ZFORLIST_HXX //autogen
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <bf_svtools/zforlist.hxx>
#endif
#include <rtl/math.hxx>
#include <bf_svx/xlnclit.hxx>

#include <bf_svx/xlnwtit.hxx>
#include "schattr.hxx"

#ifndef _SVX_CHRTITEM_HXX //autogen


#endif

#define ITEMID_FONT        EE_CHAR_FONTINFO
#define ITEMID_COLOR       EE_CHAR_COLOR
#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT

#include <bf_svx/fontitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/svxids.hrc>

#include "float.h"
#include "chaxis.hxx"
#include "pairs.hxx"
#include "glob.hrc"
#include "axisobj.hxx"
#include "globfunc.hxx"
#include	<algorithm>
#include	<limits>
namespace binfilter {

/*N*/ void ChartAxis::SetDefaults()
/*N*/ {
/*N*/ 	Font aLatinFont( OutputDevice::GetDefaultFont( DEFAULTFONT_LATIN_SPREADSHEET, mpModel->GetLanguage( EE_CHAR_LANGUAGE ), DEFAULTFONT_FLAGS_ONLYONE ) );
/*N*/ 	SvxFontItem aSvxFontItem( aLatinFont.GetFamily(), aLatinFont.GetName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
/*N*/ 		                      aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );
/*N*/
/*N*/ 	Font aCJKFont( OutputDevice::GetDefaultFont( DEFAULTFONT_CJK_SPREADSHEET, mpModel->GetLanguage( EE_CHAR_LANGUAGE_CJK ), DEFAULTFONT_FLAGS_ONLYONE ) );
/*N*/ 	SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamily(), aCJKFont.GetName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
/*N*/ 		                         aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );
/*N*/
/*N*/ 	Font aCTLFont( OutputDevice::GetDefaultFont( DEFAULTFONT_CTL_SPREADSHEET, mpModel->GetLanguage( EE_CHAR_LANGUAGE_CTL ), DEFAULTFONT_FLAGS_ONLYONE ) );
/*N*/ 	SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamily(), aCTLFont.GetName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
/*N*/ 		                         aCTLFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );
/*N*/
/*N*/ 	//	Attention!  If the size of the three fonts below is changed from 7pt to
/*N*/ 	//	some other value, change also the font scaling in globfunc.cxx:ItemsToFont.
/*N*/ 	mpAxisAttr->Put(aSvxFontItem);
/*N*/ 	mpAxisAttr->Put(SvxFontHeightItem( 247, 100, EE_CHAR_FONTHEIGHT	));	// 7pt
/*N*/ 	mpAxisAttr->Put(aSvxFontItemCJK);
/*N*/ 	mpAxisAttr->Put(SvxFontHeightItem( 247, 100, EE_CHAR_FONTHEIGHT_CJK	));	// 7pt
/*N*/ 	mpAxisAttr->Put(aSvxFontItemCTL);
/*N*/ 	mpAxisAttr->Put(SvxFontHeightItem( 247, 100, EE_CHAR_FONTHEIGHT_CTL	));	// 7pt
/*N*/
/*N*/     // the font color has 'automatic' as default that should not be overwritten
/*N*/ // 	mpAxisAttr->Put(SvxColorItem(RGBColor(COL_BLACK)));
/*N*/  	mpAxisAttr->ClearItem( ITEMID_COLOR );
/*N*/ 	mpAxisAttr->Put(XLineStyleItem(XLINE_SOLID));
/*N*/ 	mpAxisAttr->Put(XLineWidthItem(0));
/*N*/ 	mpAxisAttr->Put(XLineColorItem( String(), RGBColor(COL_BLACK)));
/*N*/ 	mpAxisAttr->Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*N*/ 	mpAxisAttr->Put(SfxBoolItem(SCHATTR_TEXT_OVERLAP,FALSE));
/*N*/
/*N*/ 	mpAxisAttr->Put(SvxChartTextOrderItem(CHTXTORDER_SIDEBYSIDE));
/*N*/ 	mpAxisAttr->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MIN, TRUE));
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(0.0, SCHATTR_AXIS_MIN));
/*N*/ 	mpAxisAttr->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_MAX, TRUE));
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(0.0, SCHATTR_AXIS_MAX));
/*N*/ 	mpAxisAttr->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN, TRUE));
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(0.0, SCHATTR_AXIS_STEP_MAIN));
/*N*/ 	mpAxisAttr->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP, TRUE));
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(0.0, SCHATTR_AXIS_STEP_HELP));
/*N*/ 	mpAxisAttr->Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM, FALSE));
/*N*/ 	mpAxisAttr->Put(SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN, (mnId==CHART_AXIS_Y)?FALSE:TRUE));
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(0.0, SCHATTR_AXIS_ORIGIN));
/*N*/ 	mpAxisAttr->Put(SfxInt32Item(SCHATTR_AXISTYPE, mnId));
/*N*/ 	mpAxisAttr->Put(SfxInt32Item(SCHATTR_AXIS_TICKS, CHAXIS_MARK_OUTER));
/*N*/ 	ShowAxis(TRUE);
/*N*/ 	ShowDescr(TRUE);
/*N*/
/*N*/ }
/*N*/ void ChartAxis::SetAttributes(const SfxItemSet &rAttr)
/*N*/ {
/*N*/ 	mpAxisAttr->Put(rAttr);
/*N*/
/*N*/ 	//Erst immer Auto-attr holen, damit evtl. berechnete Werte nicht überschrieben werden.
/*N*/ 	ReadAutoAttr();
/*N*/ 	ReadAttr();
/*N*/ };
/*N*/ void ChartAxis::SetAttributes(SfxItemSet *pSet)
/*N*/ {
/*N*/ 	if(pSet)
/*N*/ 		SetAttributes(*pSet);
/*N*/ };
/*N*/ ChartAxis::~ChartAxis()
/*N*/ {
/*N*/ 	if(mpTotal)
/*N*/ 		delete [] mpTotal;
/*N*/
/*N*/ 	if(mpColStack)
/*N*/ 		delete [] mpColStack;
/*N*/
/*N*/ 	delete mpAxisAttr;
/*N*/ 	mpAxisAttr=NULL;
/*N*/ 	mpModel=NULL;
/*N*/ 	if(mpTextAttr)
/*N*/ 		delete mpTextAttr;
/*N*/ }

/*N*/ ChartAxis::ChartAxis(ChartModel* pModel,long nId,long nUId) :
/*N*/ 	mnInnerPos(-1),
/*N*/ 	mbInnerPos(FALSE),
/*N*/ 	mbCenterText(FALSE),
/*N*/ 	mbColText(FALSE),
/*N*/ 	mnUId(nUId),
/*N*/ 	mbAlternativIdUsed(FALSE),
/*N*/ 	mnAlternateId(0),
/*N*/ 	mnTickLen(150),
/*N*/ 	mnHelpTickLen(100),
/*N*/ 	mpAxisList(NULL),
/*N*/ 	mpAxisObj(NULL),
/*N*/ 	mbSecondary(FALSE),
/*N*/ 	mpGridAttr(NULL),
/*N*/ 	mpTextAttr(NULL),
/*N*/ 	mpNumFormatter(NULL),
/*N*/ 	mpModel(pModel),
/*N*/ 	mbPercent(FALSE),
/*N*/ 	mbRadial(FALSE),
/*N*/ 	mpAxisAttr(NULL),
/*N*/ 	mnId(nId),
/*N*/ 	mbFlippedXY(FALSE),
/*N*/ 	mnPosition(0),
/*N*/ 	maArea(0,0,0,0),
/*N*/ 	mfMin(0.0),
/*N*/ 	mfMax(0.0),
/*N*/ 	mfStep(0.0),
/*N*/ 	mfStepHelp(0.0),
/*N*/ 	mfOrigin(0.0),
/*N*/ 	mbAutoMin(FALSE),
/*N*/ 	mbAutoMax(FALSE),
/*N*/ 	mbAutoOrigin(FALSE),
/*N*/ 	mbAutoStep(FALSE),
/*N*/ 	mbAutoStepHelp(FALSE),
/*N*/ 	mbLogarithm(FALSE),
/*N*/     meTextOrient( CHTXTORIENT_AUTOMATIC ),
/*N*/ 	mnTotalSize(0),
/*N*/ 	mpTotal(NULL),
/*N*/ 	mbTotalActual(FALSE),
/*N*/ 	mbTotalAlloc(FALSE),
/*N*/ 	mbColStackOK(FALSE),
/*N*/ 	mpColStack(NULL),
/*N*/ 	mbShowDescr(FALSE),
/*N*/ 	mnTicks(CHAXIS_MARK_OUTER),
/*N*/ 	mnHelpTicks(0),
/*N*/ 	mbPercentCol(TRUE),
/*N*/ 	mnMaxTextWidth(-1)
/*N*/ {
/*N*/ 	mpAxisAttr=new SfxItemSet(mpModel->GetPool(),nAxisWhichPairs);
/*N*/ 	SetDefaults();  //attribute fuellen
/*N*/ 	ReadAutoAttr(); //automatik? aus ItemSet holen
/*N*/ 	ReadAttr();		//evtl. defaults aus Itemset uebernehmen (etwas Overhead, aber sicher)
/*N*/ }

/*N*/ long ChartAxis::GetUpper(double fData,BOOL bConstrained)
/*N*/ {
/*N*/ 	long nData;
/*N*/ 	if(fData < mfOrigin)
/*?*/ 		nData = GetPos(mfOrigin);
/*N*/ 	else
/*N*/ 		nData = GetPos(fData);
/*N*/
/*N*/ 	if(!bConstrained)
/*?*/ 		return nData;
/*N*/
/*N*/ 	if(IsVertical())
/*N*/ 		return Max(maRefArea.Top(),nData);
/*N*/ 	else
/*N*/ 		return Min(maRefArea.Right(),nData);
/*N*/ }
/*N*/ long ChartAxis::GetLower(double fData,BOOL bConstrained)
/*N*/ {
/*N*/ 	long nData;
/*N*/
/*N*/ 	if(fData > mfOrigin)
/*N*/ 		nData = GetPos(mfOrigin);
/*N*/ 	else
/*N*/ 		nData = GetPos(fData);
/*N*/
/*N*/ 	if(!bConstrained)
/*?*/ 		return nData;
/*N*/
/*N*/ 	if(IsVertical())
/*N*/ 		return Min(maRefArea.Bottom(),nData);
/*N*/ 	else
/*N*/ 		return Max(maRefArea.Left(),nData);
/*N*/ }
/*N*/ Pair ChartAxis::Stack(double fData,BOOL bConstrained)
/*N*/ {
/*N*/ 	double fTop,fBottom;
/*N*/
/*N*/ 	if (fData < 0.0)//mfOrigin
/*N*/ 	{
/*?*/ 		fTop = mfDataBottom;
/*?*/ 			mfDataBottom += fData;
/*?*/ 		fBottom = mfDataBottom;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         if( fData == DBL_MIN )
/*N*/             fData = 0;
/*N*/ 		fBottom = mfDataTop;
/*N*/ 			mfDataTop += fData;
/*N*/ 		fTop = mfDataTop;
/*N*/ 	}
/*N*/
/*N*/ 	if(bConstrained)
/*N*/ 	{
/*N*/ 		if(IsVertical())
/*N*/ 			return Pair(Max(maRefArea.Top(),GetPos(fTop))
/*N*/ 					  , Min(maRefArea.Bottom(),GetPos(fBottom)) );
/*N*/ 		else
/*N*/ 			return Pair(Max(maRefArea.Left(),GetPos(fBottom))
/*N*/ 					   ,Min(maRefArea.Right(),GetPos(fTop))     );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		return Pair(GetPos(fTop),GetPos(fBottom));
/*N*/ 	}
/*N*/ }
/*N*/ BOOL ChartAxis::IsVertical()
/*N*/ {
/*N*/ 	return( (mnId==CHART_AXIS_Y && !mbFlippedXY)
/*N*/ 		  ||(mnId==CHART_AXIS_X && mbFlippedXY));
/*N*/ }


/*N*/ void ChartAxis::ResizeTotal(long nSize)
/*N*/ {
/*N*/ 	if(nSize && !mbTotalAlloc)
/*N*/ 	{
/*N*/ 		if(nSize!=mnTotalSize)
/*N*/ 		{
/*N*/ 			if(mpTotal)
/*N*/ 				delete [] mpTotal;
/*N*/ 			mnTotalSize = nSize;
/*N*/ 			mpTotal = new double[mnTotalSize];
/*N*/ 		}
/*N*/
/*N*/ 		while(nSize--)
/*N*/ 			mpTotal[nSize]=0.0;
/*N*/ 	}
/*N*/ 	mbTotalAlloc=TRUE;
/*N*/ };
/*N*/ double ChartAxis::Data2Percent(double fData,long nCol,long nRow)
/*N*/ {
/*N*/ 	double fTotal=GetTotal( mbPercentCol ? nCol : nRow);
/*N*/
/*N*/ 	return fTotal ? ( (fabs(fData) / fTotal) * 100.0 ) : DBL_MIN;
/*N*/ }
/*N*/ double ChartAxis::GetTotal(long n)
/*N*/ {
/*N*/ 	if(!mbTotalActual)
/*N*/ 		CreateTotal();
/*N*/
/*N*/ 	return mpTotal[n];
/*N*/ }
/*N*/ void ChartAxis::CreateTotal()
/*N*/ {
/*N*/ 	if(mbTotalActual)
/*?*/ 		return ;
/*N*/
/*N*/ 	double fData;
/*N*/
/*N*/ 	long nColCnt=mpModel->GetColCount();
/*N*/ 	long nRowCnt=mpModel->GetRowCount();
/*N*/
/*N*/
/*N*/ 	if(mbPercentCol) //Prozent auf Datenreihe beziehen!
/*N*/ 	{
/*N*/ 		ResizeTotal(nColCnt); //Array anlegen und auf 0.0 setzen
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		ResizeTotal(nRowCnt); //Array anlegen und auf 0.0 setzen
/*N*/ 	}
/*N*/
/*N*/
/*N*/ 	long nId,nCol;
/*N*/ 	for(long nRow=0;nRow<nRowCnt;nRow++)
/*N*/ 	{
/*N*/ 		nId=((const SfxInt32Item &)(mpModel->GetDataRowAttr(nRow).Get(SCHATTR_AXIS))).GetValue();
/*N*/ 		if(nId==mnUId)
/*N*/ 		{
/*N*/ 			for(nCol=0;nCol<nColCnt;nCol++)
/*N*/ 			{
/*N*/ 				fData=mpModel->GetData(nCol,nRow);
/*N*/ 				mpTotal[ mbPercentCol ? nCol : nRow] += fabs(fData);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	mbTotalActual=TRUE; //einmal und nie wieder (während eines BuildChart zumindest)
/*N*/ };

/*N*/ BOOL ChartAxis::IsOriginInRange() const
/*N*/ {
/*N*/ 	return  ((mfMin <= mfOrigin) && (mfOrigin <= mfMax));
/*N*/ }

// Stapeln der Werte je Col (für Liniencharts)
/*N*/ void ChartAxis::InitColStacking(long nColCnt)
/*N*/ {
/*N*/
/*N*/
/*N*/ 	if(mpColStack)
/*N*/ 		delete [] mpColStack;
/*N*/ 	mpColStack=new double[nColCnt];
/*N*/ 	while(nColCnt--)
/*N*/ 		mpColStack[nColCnt]=0.0;//mfOrigin ????;
/*N*/ }
//Stapeln der Werte je Column (Stapeln bei LinienCharts)
/*N*/ double ChartAxis::StackColData(double fData,long nCol,long nColCnt)
/*N*/ {
/*N*/ 	if(!mbColStackOK)
/*N*/ 		InitColStacking(nColCnt);//falls noch nicht geschehen
/*N*/ 	mbColStackOK=TRUE;//wird bei Initialise auf FALSE gesetzt
/*N*/
/*N*/ 	mpColStack[nCol]+=fData;
/*N*/ 	return mpColStack[nCol];
/*N*/ }
//Dies Funktion wird u.A. von Initialise gerufen (sehr früh im Buildvorgang)
//Attribute innerer Schleifen (CretaeMarks, etc.) können hier in Variablen
//gewandelt werden
/*N*/ void ChartAxis::ReadAttr()
/*N*/ {
/*N*/
/*N*/ 	//Art der Markierungen (innen/aussen)
/*N*/ 	mnTicks     = ((const SfxInt32Item&)mpAxisAttr->Get(SCHATTR_AXIS_TICKS)).GetValue();
/*N*/ 	mnHelpTicks = ((const SfxInt32Item&)mpAxisAttr->Get(SCHATTR_AXIS_HELPTICKS)).GetValue();
/*N*/
/*N*/ 	long nFak = IsVertical() ? -1 : 1;
/*N*/ 	if(mbSecondary)
/*N*/ 		nFak*=-1;
/*N*/
/*N*/ 	long nHelpFak=nFak;
/*N*/
/*N*/ 	mnTickLen=nFak*150;
/*N*/ 	mnHelpTickLen=nHelpFak*100;
/*N*/
/*N*/ 	// WICHTIG! Diese Methode darf keine Member, die Automatisch sind, ueberschreiben!
/*N*/ 	if (!mbAutoMin)
/*?*/ 		if ((mbLogarithm && (((const SvxDoubleItem&) mpAxisAttr->Get(SCHATTR_AXIS_MIN)).GetValue() != 0.0)) || !mbLogarithm)
/*?*/ 		mfMin   =((const SvxDoubleItem&) mpAxisAttr->Get(SCHATTR_AXIS_MIN)).GetValue();
/*N*/ 	if (!mbAutoMax)
/*?*/ 		mfMax   =((const SvxDoubleItem&) mpAxisAttr->Get(SCHATTR_AXIS_MAX)).GetValue();
/*N*/ 	if (!mbAutoOrigin)
/*N*/ 		mfOrigin=((const SvxDoubleItem&) mpAxisAttr->Get(SCHATTR_AXIS_ORIGIN)).GetValue();
/*N*/ 	if(!mbAutoStep)
/*?*/ 		mfStep  =((const SvxDoubleItem&) mpAxisAttr->Get(SCHATTR_AXIS_STEP_MAIN)).GetValue();
/*N*/ 	if(!mbAutoStepHelp)
/*?*/ 		mfStepHelp=((const SvxDoubleItem&) mpAxisAttr->Get(SCHATTR_AXIS_STEP_HELP)).GetValue();
/*N*/
/*N*/ }

/*N*/ void ChartAxis::ReadAutoAttr() //OK
/*N*/ {
/*N*/ 	mbLogarithm    = ((const SfxBoolItem&) mpAxisAttr->Get(SCHATTR_AXIS_LOGARITHM)).GetValue();
/*N*/ 	mbAutoStep     = ((const SfxBoolItem&) mpAxisAttr->Get(SCHATTR_AXIS_AUTO_STEP_MAIN)).GetValue();
/*N*/ 	mbAutoStepHelp = ((const SfxBoolItem&) mpAxisAttr->Get(SCHATTR_AXIS_AUTO_STEP_HELP)).GetValue();
/*N*/ 	mbAutoMin      = ((const SfxBoolItem&) mpAxisAttr->Get(SCHATTR_AXIS_AUTO_MIN)).GetValue();
/*N*/ 	mbAutoMax      = ((const SfxBoolItem&) mpAxisAttr->Get(SCHATTR_AXIS_AUTO_MAX)).GetValue();
/*N*/ 	mbAutoOrigin   = ((const SfxBoolItem&) mpAxisAttr->Get(SCHATTR_AXIS_AUTO_ORIGIN)).GetValue();
/*N*/ }

//ToDo: dies Fkt. ueberfluessig machen

/* ************************************************************************
|*
|* Minimum und Maximum berechnen, Wertebereich
|*
|* ToDo: Dies Methode muss komplett auf Verbundcharts umgestellt werden,
    Damit faellt meStackMode weg, es muss fuer jede Reihe geprueft werden,
    wie diese gezeichnet werden soll!
\*********************************************************************** */
/*N*/ inline double ChartAxis::GetRowError(long nRow)
/*N*/ {
/*N*/    switch((SvxChartKindError)((const SfxInt32Item &)mpModel->GetDataRowAttr(nRow).Get(SCHATTR_STAT_KIND_ERROR)).GetValue())
/*N*/ 	{
/*N*/ 		case CHERROR_VARIANT :
/*N*/ 			return mpModel->GetVariantY(nRow);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHERROR_SIGMA :
/*N*/ 			return mpModel->GetSigmaY(nRow);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHERROR_BIGERROR :
/*N*/ 			return mpModel->GetBigErrorY(nRow,
/*N*/ 				((const SvxDoubleItem &)mpModel->GetDataRowAttr(nRow).Get(SCHATTR_STAT_BIGERROR)).GetValue());
/*N*/ 			break;
/*N*/
/*N*/ 		default :
/*N*/ 		  break;
/*N*/ 	}
/*N*/    return 0.0;
/*N*/ }
/*N*/ inline BOOL ChartAxis::IsDataOnAxis(long nRow)
/*N*/ {
/*N*/ 	return(mnUId == ((const SfxInt32Item &)mpModel->GetDataRowAttr(nRow)
/*N*/ 		.Get(SCHATTR_AXIS)).GetValue()) ;
/*N*/ }

/*N*/ BOOL ChartAxis::GetMinMaxFromData()
/*N*/ {
/*N*/ 	BOOL bOK=FALSE;
/*N*/
/*N*/ 	long   nColCnt = mpModel->GetColCount();
/*N*/ 	long   nRowCnt = mpModel->GetRowCount();
/*N*/ 	long   nCol,nRow;
/*N*/
/*N*/ 	BOOL bInitialise=TRUE; //statt dem nCol=0&&nRow=0-Unsinn, damit ungültige Daten berücksichtigt werden
/*N*/
/*N*/ 	double fMin = 0.0;
/*N*/ 	double fMax = 0.0;
/*N*/
/*N*/ 	//Verbundchart Typ 2, letzte Reihe ist Linie, nach switch(meStackMode)
/*N*/ 	//folgt dann die beruecksichtigung dieser Linie
/*N*/ 	long nLines = Min((long)mpModel->GetNumLinesColChart(),(long)(nRowCnt-1));;//#50212#
/*N*/
/*N*/ 	if(mpModel->ChartStyle() == CHSTYLE_2D_LINE_STACKEDCOLUMN)
/*N*/ 		nRowCnt-=nLines;
/*N*/ 	else
/*N*/ 		nLines=0;
/*N*/
/*N*/ 	switch (meStackMode)
/*N*/ 	{
/*N*/ 		//Min = Min(0,Summe aller negativen Daten)
/*N*/ 		//Max = Max(0,Summe aller positiven Daten)
/*N*/ 		case CHSTACK_MINMAX:
/*N*/ 			for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 			{
/*N*/ 				double fMinTotal = 0.0;
/*N*/ 				double fMaxTotal = 0.0;
/*N*/
/*N*/ 				for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 				{
/*N*/
/*N*/ 					if(IsDataOnAxis(nRow))
/*N*/ 					{
/*N*/ 						bOK=TRUE;
/*N*/
/*N*/ 						double fData = GetData(nCol, nRow);//mpModel->GetData(nCol, nRow, mbPercent);
/*N*/
/*N*/ 						if (fData != DBL_MIN)
/*N*/ 							if (fData < 0.0)
/*N*/ 								fMinTotal += fData;
/*N*/ 							else if (fData > 0.0)
/*N*/ 								fMaxTotal += fData;
/*N*/ 					}
/*N*/ 				}
/*N*/
/*N*/ 				if (nCol == 0) //nicht gut, aber hier kann man sowas machen, irgendne Reihe hat Daten!
/*N*/ 				{
/*N*/ 					fMin = fMinTotal;
/*N*/ 					fMax = fMaxTotal;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if (fMin > fMinTotal)
/*N*/ 						fMin = fMinTotal;
/*N*/ 					if (fMax < fMaxTotal)
/*N*/ 						fMax = fMaxTotal;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		//Min = Min(0,Min(Daten))
/*N*/ 		//Max = Max(0,Summe aller Daten)
/*N*/ 		case CHSTACK_OVERLAP:
/*N*/ 			for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 			{
/*N*/ 				double fTotal = 0.0;
/*N*/
/*N*/ 				for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 				{
/*N*/ 					if(IsDataOnAxis(nRow))
/*N*/ 					{
/*N*/ 						bOK=TRUE;
/*N*/
/*N*/ 						double rTemp = GetData(nCol, nRow);//mpModel->GetData(nCol, nRow, mbPercent);
/*N*/
/*N*/ 						if (rTemp != DBL_MIN)
/*N*/ 						{
/*N*/ 							if(fMin > rTemp)
/*N*/ 							   fMin = rTemp;
/*N*/ 							fTotal += rTemp;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/
/*N*/ 				if(fMax < fTotal)
/*N*/ 				   fMax = fTotal;
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		default:
/*N*/ 			if (mpModel->IsXYChart ())
/*N*/ 			{
/*N*/ 				if(mnId == CHART_AXIS_X)
/*N*/ 				{
/*N*/ 					// here: x axis values are in row 0, so just iterate over col
/*N*/ 					for( nCol = 0; nCol < nColCnt; nCol++ )
/*N*/ 					{
/*N*/ 						double fData = GetData( nCol, 0 );
/*N*/
/*N*/ 						if (fData != DBL_MIN)
/*N*/ 							if (nCol == 0)    //naja, so eigentlich nicht.... aber die X-Achse sollte eh vollständig sein
/*N*/ 							{
/*N*/ 								fMin = fData;
/*N*/ 								fMax = fData;
/*N*/ 							}
/*N*/ 							else
/*N*/ 							{
/*N*/ 								//if ((fMin > fData) && !mbLogarithm || (fData > 0.0) && mbLogarithm)
/*N*/ 								if( (fMin > fData)
/*N*/ 								&& (!mbLogarithm || fData > 0.0) )
/*N*/ 									fMin = fData;
/*N*/
/*N*/ 								if (fMax < fData)
/*N*/ 									fMax = fData;
/*N*/ 						   }
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else // y axis
/*N*/ 				{
/*N*/ 					// #69912# use x axis range for determining the auto values for y axis
/*N*/ 					ChartAxis* pXAxis = mpModel->GetAxisByUID( CHART_AXIS_PRIMARY_X );
/*N*/ 					double fXMin = pXAxis->GetMin();
/*N*/ 					double fXMax = pXAxis->GetMax();
/*N*/
/*N*/ 					// set min and max to 0/1 resp. in case the x range is empty
/*N*/ 					fMin = 0.0;
/*N*/ 					fMax = 1.0;
/*N*/
/*N*/ 					for (nRow = 1; nRow < nRowCnt; nRow++)
/*N*/ 					{
/*N*/ 						//if(IsDataOnAxis(nRow)) //#63904#: 2,77%
/*N*/ 						const SfxItemSet& rDataRowAttr = mpModel->GetDataRowAttr(nRow);
/*N*/ 						if(mnUId == ((const SfxInt32Item &)rDataRowAttr.Get(SCHATTR_AXIS)).GetValue())
/*N*/ 						{
/*N*/ 							bOK=TRUE;
/*N*/
/*N*/ 							double fDiffUp   = 0.0;
/*N*/ 							double fDiffDown = 0.0;
/*N*/ 							fDiffUp=fDiffDown=GetRowError(nRow); // rm1
/*N*/
/*N*/ 							for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 							{
/*N*/ 								// #69912#
/*N*/ 								double xVal = GetData( nCol, 0 );
/*N*/ 								if( fXMin <= xVal && xVal <= fXMax )
/*N*/ 								{
/*N*/ 									double fData    = GetData( nCol, nRow );
/*N*/
/*N*/ 									if (fData != DBL_MIN)
/*N*/ 									{
/*N*/ 										double fDataMin = fData;
/*N*/ 										double fDataMax = fData;
/*N*/
/*N*/ 										//#63904#neu:	(18.8 ms ->  7.28 ms )
/*N*/ //										SfxItemSet aDataPointAttr(rDataRowAttr);
/*N*/ //										mpModel->MergeDataPointAttr(aDataPointAttr,nCol, nRow);
/*N*/ //									SfxItemSet aDataPointAttr(mpModel->GetFullDataPointAttr(nCol, nRow));//#63904#71% 69,66%
/*N*/ 										const SfxItemSet * pDataPointItemSet = mpModel->GetRawDataPointAttr (nCol,nRow);
/*N*/ 										BOOL bDataPointItemSetValid = (pDataPointItemSet!=NULL);
/*N*/
/*N*/                                         const SfxPoolItem * pItem = sch::GetItem(
/*N*/                                             SCHATTR_STAT_KIND_ERROR,
/*N*/                                             rDataRowAttr,
/*N*/                                             *pDataPointItemSet, bDataPointItemSetValid );
/*N*/
/*N*/                                         DBG_ASSERT( pItem != NULL, "Invalid Itemset" );
/*N*/                                         if( pItem != NULL )
/*N*/                                         {
/*N*/                                             switch (static_cast<SvxChartKindError>(
/*N*/                                                         static_cast<const SfxInt32Item&>( *pItem ).GetValue ()))
/*N*/                                             {
/*?*/                                                 case CHERROR_PERCENT :
/*?*/                                                     fDiffUp   =
/*?*/                                                         fDiffDown = fData * static_cast<const SvxDoubleItem&>(*sch::GetItem(
/*?*/ 														    SCHATTR_STAT_PERCENT,
/*?*/                                                             rDataRowAttr,
/*?*/                                                             *pDataPointItemSet, bDataPointItemSetValid)).GetValue ()
/*?*/ 														/ 100.0;
/*?*/                                                     break;
/*?*/
/*?*/                                                 case CHERROR_CONST :
/*?*/                                                     fDiffUp   = fData + static_cast<const SvxDoubleItem&>(*sch::GetItem(
/*?*/ 														    SCHATTR_STAT_CONSTPLUS,
/*?*/                                                             rDataRowAttr,
/*?*/                                                             *pDataPointItemSet, bDataPointItemSetValid)).GetValue ();
/*?*/                                                     fDiffDown = fData + static_cast<const SvxDoubleItem&>(*sch::GetItem(
/*?*/ 														    SCHATTR_STAT_CONSTMINUS,
/*?*/                                                             rDataRowAttr,
/*?*/                                                             *pDataPointItemSet, bDataPointItemSetValid)).GetValue ();
/*N*/                                                     break;
/*N*/
/*N*/                                                 default :
/*N*/                                                     ;
/*N*/                                             }
/*N*/                                         }
/*N*/
/*N*/                                         pItem = sch::GetItem(
/*N*/                                             SCHATTR_STAT_INDICATE,
/*N*/                                             rDataRowAttr,
/*N*/                                             *pDataPointItemSet, bDataPointItemSetValid );
/*N*/
/*N*/                                         DBG_ASSERT( pItem != NULL, "Invalid Itemset" );
/*N*/                                         if( pItem != NULL )
/*N*/                                         {
/*N*/                                             switch (static_cast<SvxChartIndicate>(
/*N*/                                                         static_cast<const SfxInt32Item&>( *pItem ).GetValue ()))
/*N*/                                             {
/*N*/                                                 case CHINDICATE_BOTH :
/*?*/                                                     fDataMin -= fDiffDown;
/*?*/                                                     fDataMax += fDiffUp;
/*?*/                                                     break;
/*?*/
/*?*/                                                 case CHINDICATE_UP :
/*?*/                                                     fDataMax += fDiffUp;
/*?*/                                                     break;
/*?*/
/*?*/                                                 case CHINDICATE_DOWN :
/*?*/                                                     fDataMin -= fDiffDown;
/*?*/                                                     break;
/*N*/
/*N*/                                                 case CHINDICATE_NONE :
/*N*/                                                 default :
/*N*/                                                     ;
/*N*/                                             }
/*N*/                                         }
/*N*/
/*N*/ 										//if ((nCol == 0) && (nRow == 1)) //und wenn hier mal DBL_MIN steht????
/*N*/ 										if(bInitialise)
/*N*/ 										{
/*N*/ 											bInitialise=FALSE;
/*N*/ 											fMin = fDataMin;
/*N*/ 											fMax = fDataMax;
/*N*/ 										}
/*N*/ 										else
/*N*/ 										{
/*N*/ 											if ( (fMin > fDataMin)
/*N*/ 												 && (!mbLogarithm || fDataMin > 0.0) )
/*N*/ 												fMin = fDataMin;
/*N*/
/*N*/ 											if (fMax < fDataMax)
/*N*/ 												fMax = fDataMax;
/*N*/ 										}
/*N*/ 									}
/*N*/ 								}
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else // ! XY-Chart ... Hier landen auch die StockCharts
/*N*/ 			{
/*N*/ 				for (nRow = 0; nRow < nRowCnt; nRow++)
/*N*/ 				{
/*N*/ 					//if(IsDataOnAxis(nRow)) //#63904#neu:
/*N*/ 					const SfxItemSet& rDataRowAttr = mpModel->GetDataRowAttr(nRow);
/*N*/ 					if(mnUId == ((const SfxInt32Item &)rDataRowAttr.Get(SCHATTR_AXIS)).GetValue())
/*N*/ 					{
/*N*/ 						bOK=TRUE;
/*N*/
/*N*/ 						double fDiffUp   = 0.0;
/*N*/ 						double fDiffDown = 0.0;
/*N*/
/*N*/ 						fDiffUp=fDiffDown=GetRowError(nRow);//rm3
/*N*/
/*N*/ 						for (nCol = 0; nCol < nColCnt; nCol++)
/*N*/ 						{
/*N*/ 							double fData    = GetData(nCol, nRow);
/*N*/
/*N*/ 							if (fData != DBL_MIN)
/*N*/ 							{
/*N*/ 								double fDataMin = fData;
/*N*/ 								double fDataMax = fData;
/*N*/
/*N*/ 								//#63904#
/*N*/ //								SfxItemSet aDataPointAttr(rDataRowAttr);
/*N*/ //								mpModel->MergeDataPointAttr(aDataPointAttr,nCol, nRow);
/*N*/ 								//SfxItemSet aDataPointAttr(mpModel->GetFullDataPointAttr(nCol, nRow));
/*N*/ 								const SfxItemSet * pDataPointItemSet = mpModel->GetRawDataPointAttr (nCol,nRow);
/*N*/ 								BOOL bDataPointItemSetValid = (pDataPointItemSet!=NULL);
/*N*/
/*N*/                                 const SfxPoolItem * pItem = sch::GetItem(
/*N*/                                     SCHATTR_STAT_KIND_ERROR,
/*N*/                                     rDataRowAttr,
/*N*/                                     *pDataPointItemSet, bDataPointItemSetValid);
/*N*/
/*N*/                                 DBG_ASSERT( pItem != NULL, "Invalid Itemset" );
/*N*/                                 if( pItem != NULL )
/*N*/                                 {
/*N*/                                     switch (static_cast<SvxChartKindError>(
/*N*/                                                 static_cast<const SfxInt32Item&>( *pItem ).GetValue ()))
/*N*/                                     {
/*?*/                                         case CHERROR_PERCENT :
/*?*/                                             fDiffUp   =
/*?*/                                                 fDiffDown = fData * static_cast<const SvxDoubleItem&>(*sch::GetItem(
/*?*/ 												    SCHATTR_STAT_PERCENT,
/*?*/                                                     rDataRowAttr,
/*?*/                                                     *pDataPointItemSet, bDataPointItemSetValid)).GetValue ()
/*?*/ 												/ 100.0;
/*?*/                                             break;
/*?*/
/*?*/                                         case CHERROR_CONST :
/*?*/                                             fDiffUp   = fData + static_cast<const SvxDoubleItem&>(*sch::GetItem(
/*?*/ 												    SCHATTR_STAT_CONSTPLUS,
/*?*/                                                     rDataRowAttr,
/*?*/                                                     *pDataPointItemSet, bDataPointItemSetValid)).GetValue ();
/*?*/                                             fDiffDown = fData + static_cast<const SvxDoubleItem&>(*sch::GetItem(
/*?*/ 												    SCHATTR_STAT_CONSTMINUS,
/*?*/                                                     rDataRowAttr,
/*?*/                                                     *pDataPointItemSet, bDataPointItemSetValid)).GetValue ();
/*?*/                                             break;
/*N*/
/*N*/                                         default :
/*N*/                                             ;
/*N*/                                     }
/*N*/                                 }
/*N*/
/*N*/                                 DBG_ASSERT( pItem != NULL, "Invalid Itemset" );
/*N*/                                 pItem = sch::GetItem(
/*N*/                                     SCHATTR_STAT_INDICATE,
/*N*/                                     rDataRowAttr,
/*N*/                                     *pDataPointItemSet, bDataPointItemSetValid );
/*N*/
/*N*/                                 if( pItem != NULL )
/*N*/                                 {
/*N*/                                     switch (static_cast<SvxChartIndicate>(
/*N*/                                                 static_cast<const SfxInt32Item&>( *pItem ).GetValue ()))
/*N*/                                     {
/*?*/                                         case CHINDICATE_BOTH :
/*?*/                                             fDataMin -= fDiffDown;
/*?*/                                             fDataMax += fDiffUp;
/*?*/                                             break;
/*?*/
/*?*/                                         case CHINDICATE_UP :
/*?*/                                             fDataMax += fDiffUp;
/*?*/                                             break;
/*?*/
/*?*/                                         case CHINDICATE_DOWN :
/*?*/                                             fDataMin -= fDiffDown;
/*?*/                                             break;
/*N*/
/*N*/                                         case CHINDICATE_NONE :
/*N*/                                         default :
/*N*/                                             ;
/*N*/                                     }
/*N*/                                 }
/*N*/
/*N*/ 								//if ((nCol == 0) && (nRow == 0))//und wenn hier mal DBL_MIN steht????
/*N*/ 								if(bInitialise)
/*N*/ 								{
/*N*/ 									bInitialise=FALSE;
/*N*/ 									fMin = fDataMin;
/*N*/ 									fMax = fDataMax;
/*N*/ 								}
/*N*/ 								else
/*N*/ 								{
/*N*/ 									if ((fMin > fDataMin)
/*N*/ 									&& (!mbLogarithm ||(fDataMin > 0.0)))
/*N*/ 										fMin = fDataMin;
/*N*/ 									if (fMax < fDataMax)
/*N*/ 										fMax = fDataMax;
/*N*/ 								}
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 		break;
/*N*/ 	}//switch meStackMode
/*N*/
/*N*/
/*N*/ 	// bei CHSTYLE_2D_LINE_STACKEDCOLUMN muessen die Linien nochmal extra berechnet
/*N*/ 	// werden Letzte Linie = nRowCnt, da oben nRowCnt-=n   #50212#
/*N*/ 	long nIndex=nRowCnt;//ab hier liegen evtl. linien vor
/*N*/ 	while(nLines)       //nLines ist 0 wenn kein CHSTYLE_2D_LINE_STACKEDCOLUMN
/*N*/ 	{
/*?*/ 		if( IsDataOnAxis( nIndex ))
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP");}//STRIP001 UpdateRowMinMax( nIndex, nColCnt, mbPercent, fMin, fMax );
/*?*/ 		nLines--;
/*?*/ 		nIndex++;
/*N*/ 	}
/*N*/ 	if (mbPercent)
/*N*/ 	{
/*N*/ 		fMin = 0.0;
/*N*/ 		if (fMax > 100.0)
/*N*/ 			fMax = 100.0;
/*N*/ 	}
/*N*/
/*N*/     if( mbAutoMin )
/*N*/         mfMin = fMin;
/*N*/
/*N*/     if( mbAutoMax )
/*N*/         mfMax = fMax;
/*N*/
/*N*/ 	return bOK;
/*N*/ }

//Ab jetzt wird die Achse gebaut, die meisten Werte  können nicht mehr geändert werden,
//da wir uns im BuildChart (genauer: zumeist in Create<n>DBackplanes) befinden.
//allerdings wird die Area noch verändert. Davn unabhängige Werte, z.B. Items können
//in Variablen gebuffert werden
/*N*/ void ChartAxis::Initialise(const Rectangle &rRect,BOOL bFlippedXY
/*N*/ 						   ,long eStackmode,BOOL bPercent
/*N*/ 						   ,BOOL bRadial,BOOL bPercentCol)
/*N*/
/*N*/ {
/*N*/ 	mnMaxTextWidth=-1;
/*N*/ 	mbInnerPos=FALSE;
/*N*/ 	mbAlternativIdUsed=FALSE;
/*N*/ 	mbTextOverlap= ((const SfxBoolItem&)mpAxisAttr->Get(SCHATTR_TEXT_OVERLAP)).GetValue();
/*N*/ 	mbPercentCol=bPercentCol;
/*N*/ 	mbPercent=bPercent;
/*N*/ 	maArea=rRect;
/*N*/ 	mbFlippedXY=bFlippedXY;
/*N*/ 	meStackMode=eStackmode;
/*N*/ 	mbRadial=bRadial;
/*N*/ 	mbShowDescr=HasDescription();
/*N*/
/*N*/ 	mbColStackOK=FALSE;     //noch nix gestapelt, noch nicht initialisiert
/*N*/ 	mbTotalActual=FALSE; //anzahl cols/daten könnten verändert sein!
/*N*/ 	mbTotalAlloc =FALSE; //anzahl cols/daten könnten verändert sein!
/*N*/
/*N*/ 	mpMainGridList=NULL;
/*N*/ 	mpHelpGridList=NULL;
/*N*/ 	mpAxisList=NULL;
/*N*/
/*N*/ 	//Atribute auslesen, etwas Overhead, ToDo: optimieren
/*N*/ 	ReadAutoAttr();
/*N*/ 	ReadAttr();
/*N*/ 	// #84601# set Min/Max etc. at the local item set!
/*N*/ 	if( mpAxisAttr )
/*N*/ 		GetMembersAsAttr( *mpAxisAttr );
/*N*/ };

/*N*/ void ChartAxis::GetMembersAsAttr(SfxItemSet& rSet) const
/*N*/ {
/*N*/ 	rSet.Put(SfxUInt32Item(SID_ATTR_NUMBERFORMAT_VALUE,((ChartAxis*)this)->GetNumFormat()));
/*N*/ 	rSet.Put(SvxDoubleItem(mfMin		, SCHATTR_AXIS_MIN));
/*N*/ 	rSet.Put(SvxDoubleItem(mfMax		, SCHATTR_AXIS_MAX));
/*N*/ 	rSet.Put(SvxDoubleItem(mfStep		, SCHATTR_AXIS_STEP_MAIN));
/*N*/ 	rSet.Put(SvxDoubleItem(mfStepHelp	, SCHATTR_AXIS_STEP_HELP));
/*N*/ 	rSet.Put(SvxDoubleItem(mfOrigin		, SCHATTR_AXIS_ORIGIN));
/*N*/ }

/*N*/ void ChartAxis::CalcMinMaxValue()
/*N*/ {
/*N*/
/*N*/ 	if (mbAutoMin || mbAutoMax) //Dann alle Werte generieren
/*N*/ 	{
/*N*/ 		if(!GetMinMaxFromData() && mnAlternateId && /*#63904#:*/(IsVisible()||HasDescription()))
/*N*/ 		{
/*?*/ 			long tmp=mnUId;
/*?*/ 			mnUId=mnAlternateId;
/*?*/ 			GetMinMaxFromData();
/*?*/ 			mbAlternativIdUsed=TRUE;
/*?*/ 			mnUId=tmp;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if (mfMin > mfMax) //ToDo: Korrektur, sollte aber ueberfluessig sein
/*N*/ 	{
/*?*/ 		double fTemp = mfMin;
/*?*/ 		mfMin=mfMax;
/*?*/ 		mfMax=fTemp;
/*N*/ 	}
/*N*/
/*N*/ 	ReadAttr();    //Alle nicht-automatischen Wertze werden jetzt geholt
/*N*/
/*N*/ 	if ((mfMin != DBL_MIN) || (mfMax != DBL_MIN))
/*N*/ 	{
/*N*/ 		if (mfMin > mfMax) //noch ne Korrektur, diese ist aber noetig!
/*N*/ 		{
/*?*/             if( mbAutoMax )
/*?*/                 mfMax = mfMin;
/*?*/             else if( mbAutoMin )
/*?*/                 mfMin = mfMax;
/*?*/             else
/*?*/             {
/*?*/                 DBG_ERROR( "Minimum is greater than Maximum" );
/*?*/                 double fTemp = mfMin;
/*?*/
/*?*/                 mfMin = mfMax;
/*?*/                 mfMax = fTemp;
/*?*/             }
/*N*/ 		}
/*N*/
/*N*/ 		if (mbLogarithm)
/*N*/ 		{
/*?*/ 			if (mbAutoMin)
/*?*/ 			{
/*?*/ 				if (mfMin > 1.0)
/*?*/ 				{
/*?*/ 					double fVal = 1.0;
/*?*/
/*?*/ 					while (fVal < mfMin)
/*?*/ 						fVal *= 10.0;
/*?*/
/*?*/ 					if (fVal > mfMin) fVal /= 10.0;
/*?*/ 					mfMin = fVal;
/*?*/ 				}
/*?*/ 				else if (mfMin > 0.0)
/*?*/ 					 {
/*?*/ 						 double fVal = 1.0;
/*?*/
/*?*/ 						 while (fVal > mfMin)
/*?*/ 							 fVal /= 10.0;
/*?*/
/*?*/ 						 mfMin = fVal;
/*?*/ 					 }
/*?*/ 					 else mfMin = 1.0;
/*?*/ 			}
/*?*/
/*?*/ 			if (mbAutoMax)
/*?*/ 			{
/*?*/ 				if (mfMax > 1.0)
/*?*/ 				{
/*?*/ 					double fVal = 1.0;
/*?*/
/*?*/ 					while (fVal < mfMax)
/*?*/ 						fVal *= 10.0;
/*?*/
/*?*/ 					mfMax = fVal;
/*?*/ 				}
/*?*/ 				else if (mfMax > 0.0)
/*?*/ 					 {
/*?*/ 						 double fVal = 1.0;
/*?*/
/*?*/ 						 while (fVal > mfMax)
/*?*/ 							 fVal /= 10.0;
/*?*/
/*?*/ 						 if (fVal < mfMax) fVal *= 10.0;
/*?*/ 						 mfMax = fVal;
/*?*/ 					 }
/*?*/ 					 else mfMax = 1.0;
/*?*/ 			}
/*?*/
/*?*/ 			if (mfMax - mfMin == 0.0) mfMax = mfMin * 10.0;
/*?*/ 			if (mfOrigin != mfMin) mfOrigin = mfMin;
/*?*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if (!mpModel->IsNegativeChart() && (mfMin < 0.0) ||
/*N*/ 				mbAutoMin && (mfMin > 0.0)) mfMin = SizeBounds (mfMin, mfMax, FALSE);
/*N*/ 			if (mbAutoMax && (mfMax < 0.0)) mfMax = SizeBounds (mfMin, mfMax, TRUE);
/*N*/
/*N*/ 			if (mfMax - mfMin == 0.0) mfMax = mfMin + 1.0;
/*N*/
/*N*/ 			if (mbAutoOrigin)
/*N*/ 			{
/*N*/ 				mfOrigin = 0.0;
/*N*/ 				if (mfOrigin < mfMin) mfOrigin = mfMin;
/*N*/ 				else if (mfOrigin > mfMax) mfOrigin = mfMax;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if (mfOrigin < mfMin) mfMin = mfOrigin;
/*N*/ 				if (mfOrigin > mfMax) mfMax = mfOrigin;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/* ************************************************************************
|*
|* Faktor fuer Koordinaten-Multiplikation berechnen
|* abart der globalen Funktion mit den ChartAxis-Membervariablen
|*
\*********************************************************************** */
/*N*/ void ChartAxis::LoadMemberCompat(SvStream& rIn)
/*N*/ {
/*N*/ 	rIn >> mfMin;
/*N*/ 	rIn >> mfMax;
/*N*/ 	rIn >> mfStep;
/*N*/ 	rIn >> mfStepHelp;
/*N*/ 	rIn >> mfOrigin;
/*N*/
/*N*/
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(mfMin		, SCHATTR_AXIS_MIN));
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(mfMax		, SCHATTR_AXIS_MAX));
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(mfStep		, SCHATTR_AXIS_STEP_MAIN));
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(mfStepHelp	, SCHATTR_AXIS_STEP_HELP));
/*N*/ 	mpAxisAttr->Put(SvxDoubleItem(mfOrigin		, SCHATTR_AXIS_ORIGIN));
/*N*/
/*N*/ 	CHART_TRACE2( "ChartAxis::LoadMemberCompat min=%lf, max=%lf", mfMin, mfMax );
/*N*/ 	CHART_TRACE3( "... step=%lf, help=%lf, origin=%lf", mfStep, mfStepHelp, mfOrigin );
/*N*/ }

/*N*/ void ChartAxis::StoreMemberCompat(SvStream& rOut) const
/*N*/ {
/*N*/ 	rOut << mfMin;
/*N*/ 	rOut << mfMax;
/*N*/ 	rOut << mfStep;
/*N*/ 	rOut << mfStepHelp;
/*N*/ 	rOut << mfOrigin;
/*N*/ }
//ToDo: nLen ist maArea.Width(),Height, oder das ganze bei NetChart /2 (vermutlich)
/*N*/ long ChartAxis::GetLength() const
/*N*/ {
/*N*/ 	if(mbRadial)//&& Achse=Y (ist aber immer so bei NetCharts)
/*N*/ 		return maArea.GetHeight () / 2;
/*N*/
/*N*/ 	switch(mnId)
/*N*/ 	{
/*N*/ 		case CHART_AXIS_X:
/*N*/ 			return mbFlippedXY ? maArea.GetHeight() : maArea.GetWidth();
/*N*/ 			break;
/*N*/ 		case CHART_AXIS_Y:
/*N*/ 			return mbFlippedXY ? maArea.GetWidth() : maArea.GetHeight();
/*N*/ 			break;
/*N*/ 		case CHART_AXIS_Z:
/*N*/ 			return (maArea.GetWidth() * 4) / 6; //ToDo: Das hier ist doch quark... kein Flipped?:;
/*N*/ 			break;
/*N*/ 	}
/*?*/ 	DBG_ERROR("ChartAxis::GetLenght() implementation error - ZAxis failure");
/*?*/ 	return maArea.GetWidth();
/*N*/ }




/**
    @descr	Calculate the size of the bounding box of a text string.  The size
            is measured with respect to the coordinate axes.

    @param	rTextAttr	Attributes of the font to use.
    @param	aString		The "typical" description.

    @return	Returns the size of the bounding box of the text.
*/
/*N*/ Size	ChartAxis::CalcDescriptionSize	(const SfxItemSet * rTextAttr,
/*N*/ 										const String & aString)
/*N*/ {
/*N*/ 	//	Get output device, store old page size and set paper size to
/*N*/ 	//	maximal size to inhibit line breaks.
/*N*/ 	Outliner	*	pOutliner = mpModel->GetOutliner();
/*N*/ 	pOutliner->SetUpdateMode (FALSE);
/*N*/ 	Size	aOldPaperSize (pOutliner->GetPaperSize());
/*N*/ 	pOutliner->SetPaperSize (Size (100000,100000));	//	Maximal paper size ?
/*N*/
/*N*/ 	//	Save first paragraphs attributes, switch off hyphenation, and
/*N*/ 	//	set the font attributes.
/*N*/ 	SfxItemSet aOldAttr (pOutliner->GetParaAttribs(0));
/*N*/ 	SfxItemSet aNewAttr	(aOldAttr);
/*N*/ 	aNewAttr.Put (SfxBoolItem(EE_PARA_HYPHENATE, FALSE));
/*N*/ 	aNewAttr.Put (*rTextAttr);
/*N*/ 	mpModel->SetTextAttributes (aNewAttr);
/*N*/
/*N*/     // synchronize item with member
/*N*/ 	meTextOrient = ((const SvxChartTextOrientItem&)
/*N*/ 		mpAxisAttr->Get(SCHATTR_TEXT_ORIENT)).GetValue();
/*N*/     //	Determine the size of the texts bounding box.
/*N*/ 	if (meTextOrient == CHTXTORIENT_STACKED)
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pOutliner->SetText (StackString(aString), pOutliner->GetParagraph (0));
/*N*/ 	else
/*N*/ 		pOutliner->SetText(aString, pOutliner->GetParagraph (0));
/*N*/ 	pOutliner->SetUpdateMode (TRUE);
/*N*/ 	Size aSize = pOutliner->CalcTextSize();
/*N*/     // remove content and attributes
/*N*/     pOutliner->Clear();
/*N*/ 	pOutliner->SetUpdateMode (FALSE);
/*N*/
/*N*/ 	//	Restore the old font and old paper size.
/*N*/ 	mpModel->SetTextAttributes (aOldAttr);
/*N*/ 	pOutliner->SetPaperSize (aOldPaperSize);
/*N*/ 	pOutliner->SetUpdateMode (TRUE);
/*N*/
/*N*/ 	return (aSize);
/*N*/ }




/**
    @descr	Calculate the expected size of a "typical" description.  The size
            is measured along the axis.  The axis orientation is taken into
            account.
    @precond CalcMinMaxValue() has to have been called already.

    @param	rAttr	Attributes of the font to use.

    @return	Returns the size of the text projected onto the axis.
*/
/*N*/ long int	ChartAxis::CalcTypicalDescriptionSize	(const SfxItemSet * rAttr)
/*N*/ {
/*N*/ 	String		aString;
/*N*/ 	Color	*	pDummy = NULL;
/*N*/ 	Size		aSize,
/*N*/ 				aMaxSize;
/*N*/
/*N*/ 	//	As typical text we use the minimum and maximum and some randomly
/*N*/ 	//	chose value in between.  This relies on a previous call to
/*N*/ 	//	CalcMinMaxValue().
/*N*/ 	mpNumFormatter->GetOutputString (mfMin, GetNumFormat(), aString, &pDummy);
/*N*/ 	aMaxSize = CalcDescriptionSize (rAttr, aString);
/*N*/
/*N*/ 	mpNumFormatter->GetOutputString (mfMax, GetNumFormat(), aString, &pDummy);
/*N*/ 	aSize = CalcDescriptionSize (rAttr, aString);
/*N*/ 	if (aSize.Width() > aMaxSize.Width())
/*N*/ 		aMaxSize.Width() = aSize.Width();
/*N*/ 	if (aSize.Height() > aMaxSize.Height())
/*?*/ 		aMaxSize.Height() = aSize.Height();
/*N*/
/*N*/ 	double	fPos = 0.53,	//	Arbitrary position between mfMin and mfMax.
/*N*/ 			fValue = (1-fPos) * mfMin + fPos * mfMax;
/*N*/ 	mpNumFormatter->GetOutputString (fValue, GetNumFormat(), aString, &pDummy);
/*N*/ 	aSize = CalcDescriptionSize (rAttr, aString);
/*N*/ 	if (aSize.Width() > aMaxSize.Width())
/*N*/ 		aMaxSize.Width() = aSize.Width();
/*N*/ 	if (aSize.Height() > aMaxSize.Height())
/*?*/ 		aMaxSize.Height() = aSize.Height();
/*N*/
/*N*/ 	//	Select the right coordinate and return it.
/*N*/ 	if (IsVertical())
/*N*/ 		return (aMaxSize.Height());
/*N*/ 	else
/*N*/ 		return (aMaxSize.Width());
/*N*/ }


/**
    @descr	Calculate the intervall delimited by mfMin and mfMax for which
            tick marks are drawn, its origin (mfOrigin), which is either zero
            if inside the intervall or mfMin or mfMax otherwise.  The
            distance between two adjacent tick marks of the main grid is
            stored into mfStep and the distance for the help grid into
            mfStepHelp.
*/
/*N*/ void	ChartAxis::CalcValueSteps	(void)
/*N*/ {
/*N*/ 	long nLen=GetLength();
/*N*/
/*N*/ 	ReadAutoAttr(); //mbAuto* aus mpAxisAttr lesen
/*N*/
/*N*/ 	CalcMinMaxValue(); //Min und Max bestimmen
/*N*/
/*N*/ 	if (mfMax == DBL_MIN)
/*N*/ 	{
/*N*/ 		mfMax		=
/*N*/ 		mfOrigin    =
/*N*/ 		mfMin       =
/*N*/ 		mfStep		=
/*N*/ 		mfStepHelp  =  0.0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		double	fRange = mfMax-mfMin,
/*N*/ 				fApproximateDescriptionSize
/*N*/ 					= 2.5 * CalcTypicalDescriptionSize(mpAxisAttr);
/*N*/ 					//	former value: (mpModel->GetLineHeight(*mpAxisAttr) * 3)
/*N*/ 		long	nMaxSteps = (long int)(nLen / fApproximateDescriptionSize);
/*N*/
/*N*/ 		if (mbLogarithm)
/*N*/ 		{
/*N*/ 			if (mbAutoStep)
/*N*/ 				mfStep = 10.0;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if (mbAutoStep)
/*N*/ 			{
/*N*/ 				//	Find smallest power of 10 lower than or equal to fRange,
/*N*/ 				//	that scales fFactor into the interval [1,10].
/*N*/ 				double	fFactor = pow ((double)10, floor (log10 (fRange)));
/*N*/ 				fRange /= fFactor;
/*N*/
/*N*/ 				//	Calculate mfStep so that the number of divisions (nSteps)
/*N*/ 				//	is 'not much' higher than nMaxSteps.  The value of nSteps
/*N*/ 				//	is increased succesively to multiples of 2*10^k, 4*10^k,
/*N*/ 				//	5*10^k, and 10*10^k, with k>=0.
/*N*/ 				long nSteps = 1;
/*N*/ 				mfStep = 10.0;
/*N*/ 				if (nSteps < nMaxSteps)
/*N*/ 				{
/*N*/ 					short nDepth = 0;
/*N*/
/*N*/ 					do
/*N*/ 					{
/*N*/ 						if (nDepth % 4 == 2) mfStep = mfStep * 4.0 / 5.0;
/*N*/ 						else mfStep /= 2.0;
/*N*/
/*N*/ 						nSteps = (long)(fRange / mfStep);
/*N*/ 						nDepth++;
/*N*/ 					}
/*N*/ 					while (nSteps < nMaxSteps);
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if (mfStep > fRange)
/*N*/ 					{
/*N*/ 						short  nDepth = 0;
/*N*/ 						double fStep  = mfStep;
/*N*/
/*N*/ 						do
/*N*/ 						{
/*N*/ 							if (nDepth % 4 == 2) fStep = fStep * 4.0 / 5.0;
/*N*/ 							else fStep /= 2.0;
/*N*/
/*N*/ 							if (fStep >= fRange) mfStep = fStep;
/*N*/
/*N*/ 							nDepth++;
/*N*/ 						}
/*N*/ 						while (fStep > fRange);
/*N*/ 					}
/*N*/ 				}
/*N*/ 				mfStep *= fFactor;
/*N*/
/*N*/ 				//	Special treatment for dates and times.
/*N*/ 				short int	nNumberFormat = mpNumFormatter->GetType (
/*N*/ 					GetNumFormat());
/*N*/ 				BOOL		bIsDate = (nNumberFormat & NUMBERFORMAT_DATE) != 0;
/*N*/ 				BOOL		bIsTime = (nNumberFormat & NUMBERFORMAT_TIME) != 0;
/*N*/ 				//	At the moment the only special case is this:
/*N*/ 				//	Dates without time are spaced whole days apart.
/*N*/ 				if (bIsDate && ! bIsTime)
/*N*/ 				{
/*N*/ 					mfStep = ceil(mfStep);
/*N*/ 					if (mfStep < 1)
/*N*/ 						mfStep = 1;
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/             //	Make sure that the step width is positive and not equal to
/*N*/             //	zero.
/*N*/ 			if ((mfStep<0) || (mfMin+mfStep==mfMin) || (mfMax+mfStep==mfMax))
/*N*/ 			{
/*N*/ 				DBG_ERROR( "Decrement of <= 0.0 not allowed" );
/*N*/ 				mfStep = 1.0;			  // to prevent loop
/*N*/ 			}
/*N*/
/*N*/ 			if (mbAutoMin)
/*N*/ 			{
/*N*/ 				double fMin = 0.0;
/*N*/
/*N*/ 				//	Adjust mfMin to the largest value that is lower than or
/*N*/ 				//	equal to its current value and is an integer multiple of
/*N*/ 				//	mfSteps away of fMin.
/*N*/ 				if (fMin > mfMin)
/*N*/ 					mfMin = fMin - mfStep * ceil((fMin - mfMin) / mfStep);
/*N*/ 				else if (fMin < mfMin)
/*N*/ 					mfMin = fMin + mfStep * floor((mfMin - fMin) / mfStep);
/*N*/
/*N*/ 				if ((mfMin >= 0.0) && (mfMin < mfOrigin))
/*N*/ 					mfOrigin = mfMin;
/*N*/ 			}
/*N*/
/*N*/ 			if (mbAutoMax)
/*N*/ 			{
/*N*/ 				double fMax = 0.0;
/*N*/
/*N*/ 				//	Adjust mfMax to the smallest value that is higher than or
/*N*/ 				//	equal to its current value and is an integer multiple of
/*N*/ 				//	mfSteps away of fMax.
/*N*/ 				if (fMax < mfMax)
/*N*/ 					mfMax = fMax + mfStep * ceil((mfMax - fMax) / mfStep);
/*N*/ 				else if (fMax > mfMax)
/*N*/ 					mfMax = fMax - mfStep * floor ((fMax - mfMax) / mfStep);
/*N*/
/*N*/ 				if ((mfMax <= 0.0) && (mfMax > mfOrigin))
/*N*/ 					mfOrigin = mfMax;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if(mbAutoStepHelp)
/*N*/ 		{
/*N*/ 			mfStepHelp = mbLogarithm ? (mfStep / 10) : (mfStep / 2);
/*N*/ 		}
/*N*/ 		VerifySteps();
/*N*/ 	}
/*N*/
/*	//NetCharts, nur absolutwerte (was passiert bei gemischt ?)
    //Sollte sowas nicht vor die Berechnung der Steps?
    if (mbRadial)
    {
        mfMin = std::max (mfMin, 0);
        mfMax = std::max (mfMax, 0);
        if (mfMin > mfMax)
        {
            double fTemp = mfMin;

            mfMin = mfMax;
            mfMax = fTemp;
        }
    }
*/
/*N*/ }


/*N*/ void ChartAxis::VerifySteps()
/*N*/ {
/*N*/ 	//#38835# Sicherheitsueberpruefung der Werte, es werden beide Achsen gleichbehandelt
/*N*/ 	//Es sollte nur mit Double gerechnet werden da bei Fehlern der Bereich nicht reicht
/*N*/ 	//(deshalb double nVarName,
/*N*/ 	//ToDo: GetLenght benutzen! Wir kennen ja in dieser Klasse mittlerweile die Laenge
/*N*/
/*N*/ 	double fRange=(mfMax - mfMin);
/*N*/
/*N*/ 	double nLimit=Max(maArea.GetWidth(),maArea.GetHeight());
/*N*/
/*N*/ 	if(nLimit < 0.0)
/*N*/ 	{
/*?*/ 		DBG_ERROR( "negative value for nLimit, check maArea!" );
/*?*/ 		nLimit=1.0;
/*N*/ 	}
/*N*/ 	nLimit=nLimit/100.0; //Minimum 1 mm Abstand
/*N*/
/*N*/ 	if(nLimit && !mbLogarithm)
/*N*/ 	{
/*N*/ 		if(mfStep!=0.0 )
/*N*/ 		{
/*N*/ 			double fSteps=fRange/mfStep;
/*N*/ 			while(fSteps>nLimit)
/*N*/ 			{
/*?*/ 				mfStep*=10;
/*?*/ 				fSteps=fRange / mfStep;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		nLimit*=10; //Min fuer Hilfsticks: 0,1 mm bzw. 10 Hilfsticks immer moeglich
/*N*/
/*N*/ 		if(mfStepHelp!=0.0)
/*N*/ 		{
/*N*/ 			double fSteps=fRange/mfStepHelp;
/*N*/ 			while(fSteps>nLimit)
/*N*/ 			{
/*?*/ 				mfStepHelp*=10;
/*?*/ 				fSteps=fRange/mfStepHelp;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ long ChartAxis::GetPos(double fData)
/*N*/ {
/*N*/ 	switch(mnId)
/*N*/ 	{
/*N*/ 		case CHART_AXIS_X:
/*N*/ 			if(mbFlippedXY)
/*N*/ 				return maRefArea.Bottom() - (long)(CalcFact(fData) * maRefArea.GetHeight());
/*N*/ 			else
/*N*/ 				return maRefArea.Left()   + (long)(CalcFact(fData) * maRefArea.GetWidth());
/*N*/
/*N*/ 		case CHART_AXIS_Y:
/*N*/ 			if(!mbFlippedXY)
/*N*/ 				return maRefArea.Bottom() - (long)(CalcFact(fData) * maRefArea.GetHeight());
/*N*/ 			else
/*N*/ 				return maRefArea.Left() +   (long)(CalcFact(fData) * maRefArea.GetWidth());
/*N*/
/*N*/ 		case CHART_AXIS_Z:
/*?*/ 			DBG_ERROR("ChartAxis - no z pos");
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }
/*N*/ BOOL ChartAxis::SetArea(const Rectangle &rRect)
/*N*/ {
/*N*/ 	BOOL bRet=TRUE;
/*N*/ 	if(rRect==maRefArea)
/*N*/ 		bRet=FALSE;
/*N*/ 	maRefArea=rRect;
/*N*/
/*N*/ 	SetPosition(mbSecondary ? CHAXIS_POS_B :CHAXIS_POS_A);
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if(maArea!=maRefArea)
/*N*/ 		DBG_TRACE("ChartAxis:: maArea!=maRefArea!!!");
/*N*/ #endif
/*N*/ 	return bRet;
/*N*/ }



/* ************************************************************************
|*
|* Liefert die maximale Werte-Text-Gr”áe
|*
\*********************************************************************** */
/*N*/ Size ChartAxis::CalcMaxTextSize()
/*N*/ {
/*N*/ 	return CalcMaxTextSize(((const SvxChartTextOrientItem&)mpAxisAttr->Get(SCHATTR_TEXT_ORIENT)).GetValue());
/*N*/ }

/*N*/ Size ChartAxis::CalcMaxTextSize(SvxChartTextOrient eOrient)
/*N*/ {
/*N*/ 	meTextOrient = eOrient;
/*N*/ 	maMaxTextSize=Size(0,0);
/*N*/
/*N*/ 	if(!mbShowDescr)
/*N*/ 		return maMaxTextSize;
/*N*/
/*N*/ 	BOOL bStepIsValid = mbLogarithm ? (mfStep > 1.0) : (mfStep > 0.0);
/*N*/
/*N*/ 	if( mfMin != mfMax && bStepIsValid )
/*N*/ 	{
/*N*/ 		OutputDevice *pOut = mpModel->GetOutliner()->GetRefDevice();
/*N*/ 		Font aNewFont;
/*N*/ 		ItemsToFont(*mpAxisAttr,aNewFont);
/*N*/ 		Font aOldFont(pOut->GetFont());
/*N*/ 		pOut->SetFont(aNewFont);
/*N*/
/*N*/ 		double fAct = mfMin;
/*N*/
/*N*/         if( ::rtl::math::isInf( mfMax ) || ::rtl::math::isInf( mfMin ))
/*N*/         {
/*N*/             DBG_ERROR( "maximum or minumum is infinity" );
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             while (fAct <= mfMax)
/*N*/             {
/*N*/                 String aNumStr;
/*N*/                 Color* pDummy = NULL;
/*N*/
/*N*/                 if(mbColText)
/*N*/                 {
/*N*/                     long nCol = long(fAct);
/*N*/                     if(nCol<mpModel->GetColCount())
/*N*/                         aNumStr=mpModel->ColText(nCol);
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     mpNumFormatter->GetOutputString(mbPercent ? fAct / 100.0: fAct,
/*N*/                                                     GetNumFormat(), aNumStr, &pDummy);
/*N*/                 }
/*N*/
/*N*/                 Size aSize;
/*N*/                 if (meTextOrient == CHTXTORIENT_STACKED)
/*N*/                 {
/*?*/                     aSize.Width() = pOut->GetTextWidth( ::binfilter::StackString( aNumStr ));
/*?*/                     aSize.Height() = pOut->GetTextHeight();
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     aSize.Width() = pOut->GetTextWidth( aNumStr );
/*N*/                     aSize.Height() = pOut->GetTextHeight();
/*N*/                 }
/*N*/
/*N*/                 aSize.Height() += TEXTHEIGHT_OFS;
/*N*/                 aSize.Width () = (aSize.Width () * 6) / 5;
/*N*/
/*N*/                 if (aSize.Width() > maMaxTextSize.Width())
/*N*/                     maMaxTextSize.Width() = aSize.Width();
/*N*/                 if (aSize.Height() > maMaxTextSize.Height())
/*N*/                     maMaxTextSize.Height() = aSize.Height();
/*N*/
/*N*/                 IncValue(fAct, mfStep, mbLogarithm);
/*N*/             }
/*N*/             maMaxTextSize = GetRotatedTextSize( maMaxTextSize,
/*N*/                                 GetTextRotation( *mpAxisAttr, meTextOrient ));
/*N*/         }
/*N*/
/*N*/ 		//Evtl. muss hier noch ein Umbruch eingeplant werden!
/*N*/
/*N*/ 		pOut->SetFont(aOldFont);
/*N*/ 	}
/*N*/ 	RecalcTextPos();
/*N*/ 	return maMaxTextSize;
/*N*/ }

/*N*/ void ChartAxis::RecalcTextPos()
/*N*/ {
/*N*/ 	//Textabstand:
/*N*/ 	long nExtra = -100; //+1mm
/*N*/ 	if((mbSecondary && IsVertical()) || (!mbSecondary && !IsVertical()))
/*N*/ 		nExtra=100;
/*N*/
/*N*/ 	if(mnTicks&CHAXIS_MARK_OUTER)
/*N*/ 	{
/*N*/ 		mnTextPos = mnPos + nExtra + mnTickLen; //LONGMARKS;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		mnTextPos = mnPos + nExtra;
/*?*/
/*?*/ 		if(mnHelpTicks&CHAXIS_MARK_OUTER)
/*?*/ 			mnTextPos +=mnHelpTickLen;
/*N*/ 	}
/*N*/ }

/*N*/ void ChartAxis::SetPosition(long nPos)
/*N*/ {
/*N*/ 	mbSecondary=(BOOL)(nPos==CHAXIS_POS_B);
/*N*/
/*N*/ 	if(nPos<0)
/*N*/ 	{
/*N*/ 		if(IsVertical())
/*N*/ 		{
/*N*/ 			if(mbSecondary)
/*N*/ 				mnPos=maRefArea.Right();
/*N*/ 			else
/*N*/ 				mnPos=maRefArea.Left();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if(mbSecondary)
/*N*/ 				mnPos=maRefArea.Top();
/*N*/ 			else
/*N*/ 				mnPos=maRefArea.Bottom();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		mnPos=nPos;
/*N*/ 	}
/*N*/
/*N*/ 	RecalcTextPos();
/*N*/ }

//Wenn Ticks nach innen und/oder nach aussen gerichtet sind, wird ein SdrObject returned,
//sollen in keine Richtung Ticks gezeichnet werden, gbts den Nullpointer zurück
/*N*/ SdrObject *ChartAxis::CreateMarks(long nPosition,long nLen,long nWhichTicks)
/*N*/ {
/*N*/ 	XPolygon saLine(2);
/*N*/
/*N*/ 	long nLen2=(nWhichTicks&CHAXIS_MARK_INNER) ? nLen : 0;
/*N*/
/*N*/ 	if(!(nWhichTicks&CHAXIS_MARK_OUTER))
/*N*/ 		nLen=0;
/*N*/
/*N*/ 	if(!nLen && !nLen2)
/*N*/ 		return NULL;
/*N*/
/*N*/ 	if(IsVertical())
/*N*/ 	{
/*N*/ 		saLine[0].X() = mnPos + nLen;
/*N*/ 		saLine[1].X() = mnPos - nLen2;
/*N*/ 		saLine[0].Y() = saLine[1].Y() = nPosition;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		saLine[0].Y() = mnPos + nLen;
/*N*/ 		saLine[1].Y() = mnPos - nLen2;
/*N*/ 		saLine[0].X() = saLine[1].X() = nPosition;
/*N*/ 	}
/*N*/
/*N*/ 	return SetObjectAttr (new SdrPathObj(OBJ_PLIN,saLine),CHOBJID_LINE,TRUE,TRUE,mpAxisAttr);
/*N*/ }
/*N*/ void ChartAxis::CreateMarkDescr(double fData,long nPosition)
/*N*/ {
/*N*/ 	String aNumStr;
/*N*/ 	Color* pTextColor = NULL;
/*N*/
/*N*/ 	if(mbColText)
/*N*/ 	{
/*N*/ 		long nCol = long(fData);
/*N*/ 		long nColCnt=mpModel->GetColCount();
/*N*/ 		if(IsVertical())
/*N*/ 			nCol=nColCnt-nCol; //andere Reihenfolge wegen Bildschirmkoordinaten
/*N*/
/*N*/ 		if(nCol<nColCnt && nCol>=0)
/*N*/ 			CreateMarkDescr(mpModel->ColText(nCol) ,nPosition, pTextColor);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		mpNumFormatter->GetOutputString(mbPercent ? fData/100.0:fData,GetNumFormat()
/*N*/ 			,aNumStr,&pTextColor);
/*N*/ 		CreateMarkDescr(aNumStr, nPosition, pTextColor);
/*N*/ 	}
/*N*/
/*N*/ }

// BM #60999# new parameter pTextColor, if using red text for negative numbers
/*N*/ void ChartAxis::CreateMarkDescr(const String& rString, long nPosition, Color *pTextColor)
/*N*/ {
/*N*/ 	SvxColorItem* pOldTextColor = NULL;
/*N*/ 
/*N*/ 	if(!mpTextAttr)
/*N*/ 	{
/*?*/ 		DBG_ERROR("ChartAxis: description uninitialised!!!!");
/*?*/ 		CreateTextAttr();
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pTextColor )
/*N*/ 	{
/*?*/ 		pOldTextColor = (SvxColorItem*)mpTextAttr->GetItem( EE_CHAR_COLOR );
/*?*/ 		mpTextAttr->Put(SvxColorItem( *pTextColor ));
/*N*/ 	}
/*N*/ 
/*N*/ 	//Bei z.B. Barcharts ist der Text verschoben (sozusagen unter der Hilfsmarkierung)
/*N*/ 	long nTextOffset=0;
/*N*/ 	if(!mbCenterText && mbColText)
/*N*/ 		nTextOffset+=GetDescrWidth()/2;
/*N*/ 
/*N*/ 	ChartAdjust eAdj;
/*N*/ 	SdrRectObj *pText=NULL;
/*N*/ 	if(IsVertical())
/*N*/ 	{
/*N*/ 		Point aPos(mnTextPos,nPosition+nTextOffset);
/*N*/ 		if(mbStepValue)
/*N*/ 		{
/*?*/ 			if(mbValueDown)
/*?*/ 			{
/*?*/ 				if(mbSecondary)
/*?*/ 					aPos.X() += maMaxTextSize.Width();
/*?*/ 				else
/*?*/ 					aPos.X() -= maMaxTextSize.Width();
/*?*/ 			}
/*?*/ 			mbValueDown = !mbValueDown;
/*N*/ 		}
/*N*/ 		eAdj = mbSecondary ? CHADJUST_CENTER_LEFT : CHADJUST_CENTER_RIGHT;
/*N*/ 		pText=mpModel->CreateTextObj(CHOBJID_TEXT,aPos,rString,*mpTextAttr,FALSE,eAdj,mnMaxTextWidth);
/*N*/ 	}
/*N*/ 	else //horizontal
/*N*/ 	{
/*N*/ 		Point aPos(nPosition+nTextOffset,mnTextPos);
/*N*/ 		if (mbStepValue)
/*N*/ 		{
/*?*/ 			if(mbValueDown)
/*?*/ 			{
/*?*/ 				if(mbSecondary)
/*?*/ 					aPos.Y() -= maMaxTextSize.Height();
/*?*/ 				else
/*?*/ 					aPos.Y() += maMaxTextSize.Height();
/*?*/ 			}
/*?*/ 			mbValueDown = !mbValueDown;
/*N*/ 		}
/*N*/ 		eAdj = mbSecondary ? CHADJUST_BOTTOM_CENTER : CHADJUST_TOP_CENTER;
/*N*/ 		pText=mpModel->CreateTextObj(CHOBJID_TEXT,aPos,rString,*mpTextAttr,FALSE,eAdj,mnMaxTextWidth);
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pOldTextColor )
/*N*/ 	{
/*?*/ 		mpTextAttr->Put(*pOldTextColor);
/*N*/ 	}
/*N*/ 
/*N*/ 	CreateMarkDescr(pText,nPosition);
/*N*/ }
/*N*/ void ChartAxis::CreateMarkDescr(SdrRectObj *pTextObj,long nPosition)
/*N*/ {
/*N*/ 	if(pTextObj && mpAxisList )
/*N*/ 	{
/*N*/ 		Rectangle aTextBoundRect(pTextObj->GetBoundRect());
/*N*/ 		Rectangle aIntersect1(maRectA);
/*N*/ 		Rectangle aIntersect2(maRectB);
/*N*/ 
/*N*/ 		if(!mbTextOverlap && ! (aIntersect1.Intersection(aTextBoundRect).IsEmpty() &&
/*N*/ 			  aIntersect2.Intersection(aTextBoundRect).IsEmpty()) )
/*N*/ 		{
/*?*/ 			delete pTextObj;
/*?*/ 			pTextObj=NULL;
/*N*/ 		}
/*N*/ 
/*N*/ 		if (pTextObj)
/*N*/ 		{
/*N*/ 			SdrObject *pObj=CreateMarks(nPosition,mnTickLen,mnTicks);
/*N*/ 			SdrObjList* pList = mpAxisList;
/*N*/ 			if(pObj)
/*N*/             {
/*N*/                 pObj->SetMarkProtect( TRUE );
/*N*/ 				pList->NbcInsertObject(pObj);
/*N*/             }
/*N*/ 
/*N*/             pTextObj->SetMarkProtect( TRUE );
/*N*/ 			pList->NbcInsertObject(pTextObj);
/*N*/ 			maRectA = maRectB;
/*N*/ 			maRectB = aTextBoundRect;
/*N*/ 			return; //fertig -> Abbruch (die Marks muessen zuerst, dann die Texte)
/*N*/ 		}
/*N*/ 	}
/*N*/ 	//kein Textobjekt oder der Text würde überlappen
/*?*/ 	SdrObject *pObj=CreateMarks(nPosition,mnTickLen,mnTicks);
/*?*/ 	if(pObj && mpAxisList)
/*?*/ 		mpAxisList->NbcInsertObject(pObj);
/*N*/ }
/*N*/ void ChartAxis::InsertMark(long nPos,long nLen,long nWhichTicks)
/*N*/ {
/*N*/ 	SdrObject *pObj=CreateMarks(nPos,nLen,nWhichTicks);
/*N*/ 	if(pObj && mpAxisList)
/*N*/ 		mpAxisList->NbcInsertObject(pObj);
/*N*/ }


//Ähnlich zu SubtractDescrSize, jedoch wird der Umbruch sowie Änderungen am Rect bei
//sehr langen spaltenunterschriften berücksichtigt.
/*N*/ void ChartAxis::SubtractDescrSize_X(Rectangle& rRect,const Rectangle& rOldRect)
/*N*/ {
/*N*/ 	long nDescrWidth=GetDescrWidth();
/*N*/ 	mnMaxTextWidth = (long) (nDescrWidth * 0.8);
/*N*/ 
/*N*/ 	//Breite des ersten und letzten Textes der Achse, war mal ChartModel::nWidthOfFirst(Last)XAxisText
/*N*/ 	//A() = erster, B()=letzter
/*N*/ 	Pair aFirstAndLastTextWidth;
/*N*/ 
/*N*/ 	Size aPageSize(mpModel->GetPage(0)->GetSize());
/*N*/ 
/*N*/ 	//unschön, SID_TEXTBREAK sollte ein SCHATTR_... werden!
/*N*/ 	BOOL bTextBreak=mpModel->GetFormatXAxisTextInMultipleLinesIfNecessary();
/*N*/ 	if(!bTextBreak)  // Dann wird kein Umbruch erzeugt.
/*N*/ 	   mnMaxTextWidth = -1;
/*N*/ 
/*N*/ 	meTextOrient = ((const SvxChartTextOrientItem&)
/*N*/ 		mpAxisAttr->Get(SCHATTR_TEXT_ORIENT)).GetValue();
/*N*/ 
/*N*/ 	BOOL bStepDescr;
/*N*/ 
/*N*/ 	// FG: Falls die X-Achsenbeschriftung aus Zahlen besteht, wird dies beruecksichtigt.
/*N*/ 	UINT32  nNumberFormatXAxisDescr  = GetNumFormat();//0;
/*N*/ 
/*N*/ 	// FG: Jetzt wird der Rand des Charts so eingestellt, dass die Beschriftung der X-Achse
/*N*/ 	//     garantiert nicht aus dme Chart heraus oder ins Diagramm hineinlaeuft. Hier
/*N*/ 	//     muessen alle Faelle der Textausrichtung beruecksichtigt werden.
/*N*/ 	//     Hier wird erstmal die maximale Textgroesse und die Textgroesse der ersten und letzten Spalte
/*N*/ 	//     berechnet (letzteres wird in CalcMaxDescrSize versteckt durchgezogen)
/*N*/ 
/*N*/ 
/*N*/ 	if (HasDescription())//#47500#
/*N*/ 	{
/*N*/ 		if (IsVertical())
/*N*/ 		{
/*N*/ 			// FG: hier wird erstmal die urspruengliche Textgroesse berechnet, sie wird abhaengig
/*N*/ 			//     vom Ergebnis nachkorrigiert. Die Textbreite ab der umgebrochen wird darf nichts
/*N*/ 			//     mit der Spaltenbreite zu tun haben falls der Text links neben dem Chart steht
/*N*/ 			//     und von links nach rechts laeuft.
/*N*/ 			maMaxTextSize = mpModel->CalcMaxDescrSize( FALSE, meTextOrient, mbPercent, mnUId, -1, &aFirstAndLastTextWidth );
/*N*/ 			// FG: Falls man den Text umbrechen darf, so muss nur noch beachtet werden, in welche
/*N*/ 			//     Richtung der Text verlaufen soll (senkrecht, waagrecht) und ob die Groesse der
/*N*/ 			//     Beschriftung das restliche Chart zu klein machen wird. Die groesste Beschriftung
/*N*/ 			//     darf nicht mehr als 25% der Breite oder Hoehe einnehmen, je nach Ausrichtung des
/*N*/ 			//     Textes.
/*N*/ 			if  (bTextBreak)
/*N*/ 			{
/*N*/ 				if (   (meTextOrient == CHTXTORIENT_BOTTOMTOP)
/*N*/ 					|| (meTextOrient == CHTXTORIENT_TOPBOTTOM)
/*N*/ 					|| (meTextOrient == CHTXTORIENT_STACKED))
/*N*/ 				{
/*?*/ 					if ((aPageSize.Height() / 4) < maMaxTextSize.Width())
/*?*/ 					{
/*?*/ 						mnMaxTextWidth = aPageSize.Height() / 4;
/*?*/ 						maMaxTextSize = mpModel->CalcMaxDescrSize( FALSE, meTextOrient, nNumberFormatXAxisDescr,
/*?*/ 															mnUId, mnMaxTextWidth,
/*?*/ 															&aFirstAndLastTextWidth );
/*?*/ 					}
/*?*/ 					// FG: sonst kann die erste Berechnung beibehalten werden.
/*?*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if ((aPageSize.Width() / 4) < maMaxTextSize.Width()) // waagrechte Ausrichtung
/*N*/ 					{
/*N*/ 						// FG. Dann ist also der Platz fuer die Beschriftung groesser als 1/4 des Charts
/*N*/ 						mnMaxTextWidth = aPageSize.Width() / 4;
/*N*/ 					}
/*?*/ 					maMaxTextSize = mpModel->CalcMaxDescrSize( FALSE, meTextOrient, nNumberFormatXAxisDescr,
/*N*/ 																mnUId, mnMaxTextWidth,
/*N*/ 																&aFirstAndLastTextWidth );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else  // FG: Falls der Umbruch nicht erlaubt ist, der Text aber breiter als 1/3 des Charts wird,
/*N*/ 			{     //     wird trotzdem umgebrochen. Dann muss dei maximale Hoehe und Breite der groessten
/*N*/ 				  //     uns insbesondere der 1. und letzten Beschriftung neu berechnet werden.
/*N*/ 				  //     Das laesst sich sicher besser machen als nochmal ueber die gesamte Liste von Texten
/*N*/ 				  //     zu iterieren.
/*?*/ 				if (   (   (meTextOrient == CHTXTORIENT_BOTTOMTOP)
/*?*/ 						|| (meTextOrient == CHTXTORIENT_TOPBOTTOM)
/*?*/ 						|| (meTextOrient == CHTXTORIENT_STACKED)  )
/*?*/ 					 && ((aPageSize.Height() / 3) < maMaxTextSize.Width()))
/*?*/ 				{
/*?*/ 					// FG. Dann ist also der Platz fuer die Beschriftung groesser als 1/4 des Charts
/*?*/ 					mnMaxTextWidth = aPageSize.Height() / 3;
/*?*/ 					maMaxTextSize = mpModel->CalcMaxDescrSize( FALSE, meTextOrient, nNumberFormatXAxisDescr,
/*?*/ 													   mnUId, mnMaxTextWidth,
/*?*/ 													   &aFirstAndLastTextWidth );
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else  // FG: Der Text steht also unter dem Chart.
/*N*/ 		{
/*N*/ 			// FG: Fuer den Fall dass die Beschriftung nicht unter jeden Datenpunkt gesetzt
/*N*/ 			//     wird, weil zu viele Daten vorhanden sind, eignet sich eine vertikale
/*N*/ 			//     Textausrichtung besser (im Falle dass die Ausrichtung automatisch
/*N*/ 			//     erfolgen soll.
/*N*/ 			maMaxTextSize = mpModel->CalcMaxDescrSize( FALSE, meTextOrient, nNumberFormatXAxisDescr,
/*N*/ 											   mnUId, mnMaxTextWidth,
/*N*/ 											   &aFirstAndLastTextWidth );
/*N*/ 
/*N*/ 			if (   (meTextOrient == CHTXTORIENT_BOTTOMTOP)
/*N*/ 				|| (meTextOrient == CHTXTORIENT_TOPBOTTOM)
/*N*/ 				|| (meTextOrient == CHTXTORIENT_STACKED))
/*N*/ 			{
/*?*/ 				if ((aPageSize.Width() / 2) < maMaxTextSize.Height())
/*?*/ 				{
/*?*/ 					mnMaxTextWidth = aPageSize.Height() / 2;
/*?*/ 					maMaxTextSize = mpModel->CalcMaxDescrSize( FALSE, meTextOrient, nNumberFormatXAxisDescr,
/*?*/ 													   mnUId, mnMaxTextWidth,
/*?*/ 													   &aFirstAndLastTextWidth );
/*?*/ 				}
/*?*/ 				if (  ( (aPageSize.Width() / 4) < maMaxTextSize.Height()) &&
/*?*/ 				   (bTextBreak))
/*?*/ 				{
/*?*/ 					mnMaxTextWidth = aPageSize.Height() / 4;
/*?*/ 					maMaxTextSize = mpModel->CalcMaxDescrSize( FALSE, meTextOrient, nNumberFormatXAxisDescr,
/*?*/ 													   mnUId, mnMaxTextWidth,
/*?*/ 													   &aFirstAndLastTextWidth);
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 
/*N*/ 		SvxChartTextOrder eDescrOrder;
/*N*/ 
/*N*/ 		// FG: Einfachere Abfrage ob die Ausrichtung vom Programm vorgegeben wird oder vom Benutzer
/*N*/ 		// #65364# (BM) text order is now independent from text orientation
/*        if (meTextOrient != CHTXTORIENT_AUTOMATIC)
        {
*/
/*N*/ 			eDescrOrder = ((const SvxChartTextOrderItem&)
/*N*/ 				mpAxisAttr->Get(SCHATTR_TEXT_ORDER)).GetValue();
/*        }
        else
        {
            eDescrOrder = CHTXTORDER_SIDEBYSIDE;
        }
*/
        // FG: Hier wird nur abgefragt, ob die Beschriftung der X-Achse (der Datenachse)
        //     so erfolgen soll, dass Text abwechselnd hoch-tief gesetzt werden, oder nicht.
/*N*/ 		long nMaxWidth = IsVertical() ? maMaxTextSize.Height() : maMaxTextSize.Width();
/*N*/ 		BOOL bHelp = (nMaxWidth > nDescrWidth * 8.1 / 10);
/*N*/ 		bStepDescr =  ( bHelp &&
/*N*/ 						   (eDescrOrder == CHTXTORDER_AUTO) &&
/*N*/ 						   (meTextOrient == CHTXTORIENT_STANDARD)) ||
/*N*/ 						   (eDescrOrder == CHTXTORDER_UPDOWN) ||
/*N*/ 						   (eDescrOrder == CHTXTORDER_DOWNUP);

        // FG: Hier wird aufgrund der Beschriftung der obere und der untere Rand nachgeregelt.
        //     und zwar nur hier.

//BEGIN_RECT_CHANGES

/*N*/ 		if (IsVertical())
/*N*/ 		{
/*N*/ 			if (! bTextBreak)
/*N*/ 			{
/*N*/ 				if (   (meTextOrient == CHTXTORIENT_BOTTOMTOP)
/*N*/ 					|| (meTextOrient == CHTXTORIENT_TOPBOTTOM)
/*N*/ 					|| (meTextOrient == CHTXTORIENT_STACKED))
/*N*/ 				{
/*N*/ 					// FG: In diesem Fall muss man nachregeln, der Text wird ohne Umruch formatiert
/*?*/ 					if ((rRect.Bottom() - rOldRect.Bottom()) < (aFirstAndLastTextWidth.B() / 2))
/*?*/ 					{
/*?*/ 						rRect.Bottom() -= aFirstAndLastTextWidth.B() / 2;
/*?*/ 					}
/*?*/ 					if ((rRect.Top() - rOldRect.Top()) < (aFirstAndLastTextWidth.A() / 2))
/*?*/ 					{
/*?*/ 						rRect.Top() -= aFirstAndLastTextWidth.A() / 2;
/*?*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// FG: Der Text steht unter dem Chart, egal ob er von links nach rechts oder von oben nach
/*N*/ 			//     unten verlaeuft aMaxDescrSize muss vorher richtig berechnet worden sein.
/*N*/ 			if(mbSecondary)
/*?*/ 			   rRect.Top() += maMaxTextSize.Height() + mnTickLen + 100;
/*?*/ 			else
/*N*/ 			   rRect.Bottom() -= maMaxTextSize.Height() + mnTickLen + 100;
/*N*/ 		}
/*N*/ 
/*N*/ 		if (bStepDescr)  // FG: Dann erfolgt die Beschriftung hoch- und tief abwechselnd (wie in der Dialogbox)
/*N*/ 		{
/*N*/ 				// dann hat man mehr Platz pro Zeile (eigentlich 2 mal soviel) aber der Platz zwischen den
/*N*/ 				// Beschriftungen steht auch noch zur Verfuegung (jeweils 0,2 * nDescrWidth)
/*?*/ 			mnMaxTextWidth = 2 * mnMaxTextWidth + 0,4 * nDescrWidth;
/*?*/ 				// Man braucht auch 2 mal soviel Platz in unten am Chart.
/*?*/ 			if(mbSecondary)
/*?*/ 				rRect.Top() += maMaxTextSize.Height() + mnTickLen;
/*?*/ 			else
/*?*/ 				rRect.Bottom() -= maMaxTextSize.Height() + mnTickLen;
/*N*/ 		}
/*N*/ 	}
/*N*/ 		// FG (14.2.97): Hier wird der linke Rand nachgeregelt, und zwar je nach Diagrammtyp unterschiedlich.
/*N*/ 	if (mpModel->IsDescriptionCenteredUnderDataPoint())
/*N*/ 	{
/*N*/ 			// Nun jetzt weiss ich dass die Beschriftung zentriert unter einem Datenpunkt steht.
/*N*/ 			// jetzt muss der linke Rand nachgeregelt werden: Genau die Haelfte des
/*N*/ 			// Spaltentextes mit Nummer 0 muss nach links noch Platz sein.
/*N*/ 		if (IsVertical())
/*N*/ 		{
/*N*/ 			if ((rRect.Left() - rOldRect.Left()) < maMaxTextSize.Width())
/*N*/ 			{
/*N*/ 				rRect.Left()    += maMaxTextSize.Width();
/*N*/ 			}
/*N*/ 			// FG: Ein Nachregeln des rechten Randes ist nicht noetig, da die Beschriftung
/*N*/ 			//     nie rechts neben dem Chart stehen kann.
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if(! (     (meTextOrient == CHTXTORIENT_BOTTOMTOP)
/*N*/ 					|| (meTextOrient == CHTXTORIENT_TOPBOTTOM)
/*N*/ 					|| (meTextOrient == CHTXTORIENT_STACKED)) )
/*N*/ 			{
/*N*/ 				if ((rRect.Left() - rOldRect.Left()) < (aFirstAndLastTextWidth.A() / 2))
/*N*/ 				{
/*N*/ 					rRect.Left() += aFirstAndLastTextWidth.A() / 2;
/*N*/ 				}
/*N*/ 				if (mpModel->InitialSize().Width() - rRect.Right() < (aFirstAndLastTextWidth.B() / 2))
/*N*/ 				{
/*N*/ 					rRect.Right() -= aFirstAndLastTextWidth.B() / 2;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// FG: Im anderen Fall wird gerade so umgebrochen, dass kein zusaetzlicher Rand gelassen werden muss.
/*N*/ 		//     Es sei denn im Dialog wurde eingestellt, dass man keinen Umbruch wollte.
/*N*/ 		if (!bTextBreak)
/*N*/ 		{
/*?*/ 			if ((rRect.Left() - rOldRect.Left()) < (aFirstAndLastTextWidth.A() / 2))
/*?*/ 			{
/*?*/ 				rRect.Left() += aFirstAndLastTextWidth.A() / 2;
/*?*/ 			}
/*?*/ 			if (mpModel->InitialSize().Width() - rRect.Right() < (aFirstAndLastTextWidth.B() / 2))
/*?*/ 			{
/*?*/ 				rRect.Right() -= aFirstAndLastTextWidth.B() / 2;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
//END RECT_CHANGES ********************************************************************
/*N*/ }




/*N*/ void ChartAxis::SubtractDescrSize(Rectangle& rRect)
/*N*/ {
/*N*/ 	if(!mbShowDescr)
/*N*/ 		return;

/*N*/ 	long nLong = (mnTicks&CHAXIS_MARK_OUTER) ? 150 : (mnHelpTicks&CHAXIS_MARK_OUTER) ? 100 : 0;

/*N*/ 	if(mbSecondary)
/*N*/ 	{
/*?*/ 		if(!IsVertical())
/*?*/ 		{
/*?*/ 			rRect.Top() += mbStepValue
/*?*/ 								  ? maMaxTextSize.Height() * 2
/*?*/ 								  : maMaxTextSize.Height();
/*?*/ 			rRect.Top() += nLong;
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			rRect.Right() -= mbStepValue
/*?*/ 								? maMaxTextSize.Width() * 2
/*?*/ 								: maMaxTextSize.Width();
/*?*/ 			rRect.Right() -= nLong;
/*?*/ 		}
/*?*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if(!IsVertical())
/*N*/ 		{
/*N*/ 			rRect.Bottom() -= mbStepValue
/*N*/ 								  ? maMaxTextSize.Height() * 2
/*N*/ 								  : maMaxTextSize.Height();
/*N*/ 			rRect.Bottom() -= nLong;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			rRect.Left() += mbStepValue
/*N*/ 								? maMaxTextSize.Width() * 2
/*N*/ 								: maMaxTextSize.Width();
/*N*/ 			rRect.Left() += nLong;
/*N*/ 		}
/*N*/ 	}
/*N*/ }
//Bisher nur Y-Achse:
/*N*/ void ChartAxis::InitDescr()
/*N*/ {
/*N*/ 	InitDescr(meTextOrient,mnMaxTextWidth);
/*N*/ }
/*N*/ void ChartAxis::InitDescr(SvxChartTextOrient &rValueOrient,long nMaxTextWidth)
/*N*/ {
/*N*/ 	mnMaxTextWidth=nMaxTextWidth;//-1;
/*N*/ 
/*N*/ 	BOOL bAutoValue = (rValueOrient == CHTXTORIENT_AUTOMATIC);
/*N*/ 
/*N*/ 	if(!mbShowDescr)
/*N*/ 		return;
/*N*/ 
/*N*/ 	if (bAutoValue)
/*N*/ 		rValueOrient = CHTXTORIENT_STANDARD;

    // #65364# (BM) text order is now independent from text orientation
/*N*/ 	SvxChartTextOrder eValueOrder = /*( bAutoValue
                                    ||( mbFlippedXY && (rValueOrient != CHTXTORIENT_STANDARD))
                                    ||(!mbFlippedXY && (rValueOrient == CHTXTORIENT_STANDARD)))
                                         ? CHTXTORDER_SIDEBYSIDE
                                         :*/
/*N*/ 									((const SvxChartTextOrderItem&)mpAxisAttr->Get(SCHATTR_TEXT_ORDER)).GetValue();
/*N*/ 
/*N*/ 	mbStepValue = (eValueOrder != CHTXTORDER_SIDEBYSIDE);
/*N*/ 	mbValueDown = (eValueOrder == CHTXTORDER_DOWNUP);
/*N*/ 
/*N*/ 	meTextOrient = rValueOrient;
/*N*/ 
/*N*/ 	CreateTextAttr();
/*N*/ }
/*N*/ void ChartAxis::InitDescr_X()
/*N*/ {
/*N*/ 	if(!mbShowDescr)
/*N*/ 		return;
/*N*/ 
/*N*/ 	SvxChartTextOrder eValueOrder;

    // FG: Einfachere Abfrage ob die Ausrichtung vom Programm vorgegeben wird oder vom Benutzer
    // #65364# (BM) text order is now independent from text orientation
/*    if (meTextOrient != CHTXTORIENT_AUTOMATIC)
    {
*/
/*N*/ 		eValueOrder = ((const SvxChartTextOrderItem&)
/*N*/ 			mpAxisAttr->Get(SCHATTR_TEXT_ORDER)).GetValue();
/*    }
    else
    {
        eValueOrder = CHTXTORDER_SIDEBYSIDE;
    }
*/
    // FG: Hier wird nur abgefragt, ob die Beschriftung der X-Achse (der Datenachse)
    //     so erfolgen soll, dass Text abwechselnd hoch-tief gesetzt werden, oder nicht.
/*N*/ 	long nMaxWidth = IsVertical() ? maMaxTextSize.Height() : maMaxTextSize.Width();
/*N*/ 	BOOL bHelp=	(nMaxWidth > GetDescrWidth() * 8.1 / 10);
/*N*/ 	mbStepValue = (bHelp &&
/*N*/ 				  (eValueOrder == CHTXTORDER_AUTO) &&
/*N*/ 				  (meTextOrient == CHTXTORIENT_STANDARD)) ||
/*N*/ 				  (eValueOrder == CHTXTORDER_UPDOWN) ||
/*N*/ 				  (eValueOrder == CHTXTORDER_DOWNUP);
/*N*/ 
/*N*/ 
/*N*/ 	mbValueDown = (eValueOrder == CHTXTORDER_DOWNUP);
//	meTextOrient = rValueOrient;

/*N*/ 	CreateTextAttr();
/*N*/ }




/*N*/ void ChartAxis::CreateTextAttr()
/*N*/ {
/*N*/ 	if(!mpTextAttr)
/*N*/ 		mpTextAttr=new SfxItemSet((const SfxItemSet &)*mpAxisAttr);
/*N*/ 	else
/*N*/ 		mpTextAttr->Put(*mpAxisAttr); //Update
/*N*/ 
/*N*/ 	mpTextAttr->Put(SvxChartTextOrientItem(meTextOrient));
/*N*/ 	mpTextAttr->Put(XLineStyleItem(XLINE_NONE));
/*N*/ 	mpTextAttr->Put(XLineWidthItem(0));
/*N*/ }

/*N*/ void ChartAxis::CreateAxis(SdrObjList& rList,long nChObjId)
/*N*/ {
/*N*/ 	if(IsVisible() || HasDescription())
/*N*/ 	{
/*N*/ 		mpAxisObj = new SchAxisObj( mpModel );
/*N*/ 		SetObjectAttr( mpAxisObj, USHORT(nChObjId), TRUE, TRUE, 0 );
/*N*/ 		SetAxisList( mpAxisObj->GetSubList() );
/*N*/ 
/*N*/ 		rList.NbcInsertObject( mpAxisObj, CONTAINER_APPEND );
/*N*/ 
/*N*/ 		if( IsVisible())
/*N*/ 			CreateAxis();
/*N*/ 	}
/*N*/ }

/*N*/ void ChartAxis::CreateAxis(const long nPos,BOOL bBorderAxis,BOOL bInnerAxis)
/*N*/ {
/*N*/ 	//Position der X-Achse.... aber nicht der Ticks
/*N*/ 	//(nicht ganz verständlich bei XY-Diagrammen)
/*N*/ 	//was ist mit bShow?Descr ?
/*N*/ 	mnInnerPos=nPos;
/*N*/ 
/*N*/ 	long nPos1 = 0;
/*N*/ 	if(bBorderAxis)
/*N*/ 		nPos1=CreateAxis();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		mbInnerPos=TRUE; //ungenutzt.... evtl. wegen SetArea(){...SetPosition()...}
/*N*/ 		SetPosition(nPos);
/*N*/ 	}
/*N*/ 	mnInnerPos=nPos;
/*N*/ 
/*N*/ 	if(bInnerAxis &&(!bBorderAxis || nPos1!=nPos)) //nur eine Achse oder 2 Achsen an versch. Positionen
/*N*/ 	{
/*N*/ 
/*N*/ 		XPolygon aLine(2);
/*N*/ 		if (IsVertical())
/*N*/ 		{
/*N*/ 			aLine[0].X() = aLine[1].X() = nPos;
/*N*/ 			aLine[0].Y() = maRefArea.Bottom();
/*N*/ 			aLine[1].Y() = maRefArea.Top();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aLine[0].Y() = aLine[1].Y() = nPos;
/*N*/ 			aLine[0].X() = maRefArea.Left();
/*N*/ 			aLine[1].X() = maRefArea.Right();
/*N*/ 		}
/*N*/ 		if (mpAxisList)
/*N*/ 			mpAxisList->NbcInsertObject(SetObjectAttr (new SdrPathObj(OBJ_PLIN, aLine),
/*N*/ 			CHOBJID_LINE, TRUE, TRUE,mpAxisAttr));
/*N*/ 	}
/* alter Original-Source, siehe ###hier###
    if (bSwitchColRow)
    {
        VERTICAL_LINE;
        if ((nPos != rRect.Left()) && (bShowYDescr ))
        {
            aLine[0].X() = aLine[1].X() = rRect.Left ();

            rObjList.InsertObject(SetObjectAttr(new SdrPathObj(OBJ_PLIN, aLine),
                CHOBJID_DIAGRAM_X_AXIS,TRUE, TRUE, pXGridMainAttr));
        }
        aLine[0].X() = aLine[1].X() = nPos;
    }
    else //!bSwitchColRow
    {
        HORIZONTAL_LINE;
        if ((nPos != rRect.Bottom()) && (bShowXDescr))
        {
            if(pChartYAxis->GetMin() < 0.0)
            {
                //###hier### ist jetzt anders!

                aLine[0].Y()=aLine[1].Y()= (pChartYAxis->GetMax()>0.0)
                            ? rRect.Bottom ()
                            : rRect.Top ();

            }
            else
                aLine[0].Y()=aLine[1].Y()= rRect.Bottom ();

            HORIZONTAL_LINE;
            rObjList.InsertObject(SetObjectAttr (new SdrPathObj(OBJ_PLIN, aLine),
                CHOBJID_DIAGRAM_X_AXIS,TRUE, TRUE, pXGridMainAttr));
        }
        aLine[0].Y() = aLine[1].Y() = nPos;
    }
*/
/*N*/ }

/*N*/ long ChartAxis::CreateAxis()
/*N*/ {
/*N*/ 	XPolygon aLine(2);
/*N*/ 	long nPos;
/*N*/ 
/*N*/ 	if (IsVertical())
/*N*/ 	{
/*N*/ 		if(mbSecondary)
/*N*/ 		{
/*?*/ 			aLine[0].X() = aLine[1].X() = maRefArea.Right();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aLine[0].X() = aLine[1].X() = maRefArea.Left();
/*N*/ 		}
/*N*/ 		aLine[0].Y() = maRefArea.Bottom();
/*N*/ 		aLine[1].Y() = maRefArea.Top();
/*N*/ 		nPos=aLine[0].X();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if(mbSecondary)
/*N*/ 		{
/*?*/ 			aLine[0].Y() = aLine[1].Y() = maRefArea.Top();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aLine[0].Y() = aLine[1].Y() = maRefArea.Bottom();
/*N*/ 		}
/*N*/ 		aLine[0].X() = maRefArea.Left();
/*N*/ 		aLine[1].X() = maRefArea.Right();
/*N*/ 		nPos=aLine[0].Y();
/*N*/ 	}
/*N*/ 
/*N*/ 	if (mpAxisList && IsVisible())
/*N*/ 		mpAxisList->NbcInsertObject(SetObjectAttr (new SdrPathObj(OBJ_PLIN, aLine),
/*N*/ 		CHOBJID_LINE, TRUE, TRUE,mpAxisAttr));
/*N*/ 
/*N*/ 	return nPos;
/*N*/ 
/*N*/ }
/*N*/ void ChartAxis::SetMainGrid(SdrObjList* pList,SfxItemSet*pAttr)
/*N*/ {
/*N*/ 	mpMainGridList=pList;
/*N*/ 	mpMainGridAttr=pAttr;
/*N*/ 	switch(mnId)
/*N*/ 	{
/*N*/ 		case CHART_AXIS_X:
/*N*/ 			mnIdMainGrid=CHOBJID_DIAGRAM_Y_GRID_MAIN;
/*N*/ 			break;
/*N*/ 		case CHART_AXIS_Y:
/*N*/ 			mnIdMainGrid=CHOBJID_DIAGRAM_X_GRID_MAIN;
/*N*/ 			break;
/*N*/ 		case CHART_AXIS_Z:
/*?*/ 			mnIdMainGrid=CHOBJID_DIAGRAM_Z_GRID_MAIN;
/*?*/ 			break;
/*N*/ 	}
/*N*/ }
/*N*/ void ChartAxis::SetHelpGrid(SdrObjList* pList,SfxItemSet*pAttr)
/*N*/ {
/*N*/ 	mpHelpGridList=pList;
/*N*/ 	mpHelpGridAttr=pAttr;
/*N*/ 	switch(mnId)
/*N*/ 	{
/*N*/ 		case CHART_AXIS_X:
/*N*/ 			mnIdHelpGrid=CHOBJID_DIAGRAM_Y_GRID_HELP;
/*N*/ 			break;
/*N*/ 		case CHART_AXIS_Y:
/*N*/ 			mnIdHelpGrid=CHOBJID_DIAGRAM_X_GRID_HELP;
/*N*/ 			break;
/*N*/ 		case CHART_AXIS_Z:
/*?*/ 			mnIdHelpGrid=CHOBJID_DIAGRAM_Z_GRID_HELP;
/*?*/ 			break;
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ void ChartAxis::GridLine(XPolygon& aLine,long nPos)
/*N*/ {
/*N*/ 	if(IsVertical())
/*N*/ 	{
/*N*/ 		aLine[0].Y() = aLine[1].Y() = nPos;
/*N*/ 		aLine[0].X() = maRefArea.Left();
/*N*/ 		aLine[1].X() = maRefArea.Right();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aLine[0].X() = aLine[1].X() = nPos;
/*N*/ 		aLine[0].Y() = maRefArea.Bottom();
/*N*/ 		aLine[1].Y() = maRefArea.Top();
/*N*/ 	}
/*N*/ }

/*N*/ void ChartAxis::ShowDescr(BOOL b)
/*N*/ {
/*N*/     mbShowDescr=b;
/*N*/     mpAxisAttr->Put(SfxBoolItem(SCHATTR_AXIS_SHOWDESCR,b));
/*N*/ }

/*N*/ void ChartAxis::ShowAxis(BOOL b)
/*N*/ {
/*N*/     mpAxisAttr->Put(SfxBoolItem(SCHATTR_AXIS_SHOWAXIS,b));
/*N*/ }

/*N*/ void ChartAxis::DrawGrids()
/*N*/ {
/*N*/ 	XPolygon aLine(2);
/*N*/ 	long nPos;
/*N*/ 
/*N*/ 	if(!mpMainGridList && !mpHelpGridList && !mpAxisList)
/*N*/ 		return ; //nothing to do
/*N*/ 
/*N*/ 
/*N*/ 	// if the x-axis has no numbers as description, the meaning of main and help ticks might
/*N*/ 	// be interpreted the other way round
/*N*/ 	long   nTicksBack		=mnTicks;
/*N*/ 	long   nHelpTicksBack	=mnHelpTicks;
/*N*/ 	long   nTickLenBack		=mnTickLen;
/*N*/ 	long   nHelpTickLenBack	=mnHelpTickLen;
/*N*/ 
/*N*/ 	// ...Back is for storing original values
/*N*/ 	double fMinBack	 = mfMin;
/*N*/ 	double fMaxBack	 = mfMax;
/*N*/ 	double fStepBack	 = mfStep;
/*N*/ 	double fStepHelpBack= mfStepHelp;
/*N*/ 	BOOL   bLogarithmBack = mbLogarithm;
/*N*/ 
/*N*/ 	if(mbColText) //... da hier modifiziert werden kann, aber die Werte nicht
/*N*/ 	{			  //in den Attrset sollen (also nicht persistent). In wie weit das nötig ist, ist ne andere Frage...
/*N*/ 		mfMin=0;
/*N*/ 		mfStep=1;
/*N*/ 		mbLogarithm = FALSE;
/*N*/ 		mfMax=mpModel->GetColCount();
/*N*/ 		if(mbCenterText && mfMax>1.0)
/*N*/ 		{
/*?*/ 			mfMax-=1;
/*?*/ 
/*?*/ 			//Die Markierungen sind bei Linien und Area-Charts verschoben
/*?*/ 			//=> dies entspricht gewissermassem dem Swap von Help und MainTicks
/*?*/ 			mnTicks=0;//nHelpTicksBack; //HelpTicks bzw. innen/aussen noch nicht per GUI zugänglich, daher fixe Werte
/*?*/ 			mnHelpTicks=CHAXIS_MARK_OUTER;//nTicksBack;
/*?*/ 			mnHelpTickLen=nTickLenBack;
/*?*/ 			mnTickLen=nHelpTickLenBack;
/*?*/ 
/*?*/ 			//	Now we have to copy with the case of to many tick marks.
/*?*/ 			//	The problem lies in the fact, that creating to many tick
/*?*/ 			//	marks results in an overrun of pool item reference counters.
/*?*/ 			//	Creating them without these items leaves them unchangeable.
/*?*/ 			//	The best solution is to generalize the data set, i.e.
/*?*/ 			//	to draw only a part of the tick marks (and draw only a part
/*?*/ 			//	of the data).  The number of tick marks to draw should ideally 
/*?*/ 			//	depend on the real window size measured in pixel.  This is (at
/*?*/ 			//	the moment) not possible because we don't have access to the
/*?*/ 			//	view and therefore no access to it's output device.
/*?*/ 			//	Leaving a clean solution for the new chart for now we
/*?*/ 			//	set a fixed number as the maximal count of tick marks.
/*?*/ 			//	This solution has one advantage: output to the screen and to 
/*?*/ 			//	the printer has the same number of tick marks.
/*?*/ 			const double maximum_number_of_tick_marks = 1000.0;
/*?*/ 			mfStep =(mfMax - mfMin) / ::std::min< double > (maximum_number_of_tick_marks, (mfMax - mfMin));
/*?*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			mnHelpTicks=0; //nicht per Gui möglich, daher fixe Werte
/*N*/ 			mnTicks=CHAXIS_MARK_OUTER;
/*N*/ 		}
/*N*/ 
/*N*/ 		mfStepHelp = mfStep / 2;
/*N*/ 	}
/*N*/ 
/*N*/ 	// #69810# prevent infinite loop if step wouldn't change the value of fAct
/*N*/ 	BOOL bStepIsOK = mbLogarithm? (fabs(mfStep) > 1.0): (mfStep != 0.0);
/*N*/ 
/*N*/ //	DBG_ASSERT( bStepIsOK, "Axis increment is ineffective and would cause loop" );
/*N*/ 
/*N*/ 	if( mfMax!=mfMin && ( bStepIsOK ) )
/*N*/ 	{
/*N*/ 		double fAct     = mfMin;
/*N*/ 		double fActHelp = mfMin;
/*N*/ 
/*N*/         // in the worst case the threshhold is 1/1000th of the height of the
/*N*/         // diagram this is just done because of minor inaccuracies that may
/*N*/         // occur, because in the following loop the current value is achieved by
/*N*/         // repeatedly adding the main step value where small errors might
/*N*/         // accumulate
/*N*/         double fThreshold = ::std::min( mfMax, ::std::min( mfStep, mfStepHelp )) / 1000.0;
/*N*/ 
/*N*/         if( ::rtl::math::isInf( mfMax ) || ::rtl::math::isInf( mfMin ))
/*N*/         {
/*N*/             DBG_ERROR( "maximum or minumum is infinity" );
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             // #102339# iterate one major tick further and inhibit creating
/*N*/             // ticks in case they are above the maximum (+threshold)
/*N*/             double fUpperLimit = mfMax + fThreshold;
/*N*/             double fIterationEnd = fUpperLimit + mfStep;
/*N*/             while( fAct <= fIterationEnd )
/*N*/             {
/*N*/                 long nMainPos = GetPos(fAct);
/*N*/ 
/*N*/                 if( fAct <= fUpperLimit )
/*N*/                 {
/*N*/                     GridLine(aLine,nMainPos);
/*N*/ 
/*N*/                     if (mpMainGridList)
/*N*/                         mpMainGridList->NbcInsertObject(SetObjectAttr(new SdrPathObj(OBJ_PLIN,aLine),
/*N*/                                                                       USHORT(mnIdMainGrid),TRUE,TRUE,mpMainGridAttr));
/*N*/                 }
/*N*/ 
/*N*/                 if(mpHelpGridList || (mpAxisList && mnHelpTicks) )
/*N*/                 {
/*?*/                     if(mbLogarithm)
/*?*/                     {
/*?*/                         fActHelp = mfStep / mfStepHelp * fAct;
/*?*/ 
/*?*/                         double fNext = fAct;
/*?*/                         IncValue(fNext,mfStep,mbLogarithm);
/*?*/ 
/*?*/                         while (fActHelp < fNext)
/*?*/                         {
/*?*/                             if(CalcFact(fActHelp) > 1.0)
/*?*/                                 break;
/*?*/ 
/*?*/                             if( fActHelp <= fUpperLimit )
/*?*/                             {
/*?*/                                 GridLine(aLine,nPos=GetPos(fActHelp));
/*?*/ 
/*?*/                                 if(mpHelpGridList)
/*?*/                                     mpHelpGridList->NbcInsertObject(SetObjectAttr(new SdrPathObj(OBJ_PLIN,aLine),
/*?*/                                                                                   USHORT(mnIdHelpGrid),TRUE,TRUE,mpHelpGridAttr));
/*?*/ 
/*?*/                                 InsertMark(nPos,mnHelpTickLen,mnHelpTicks);
/*?*/                             }
/*?*/ 
/*?*/                             fActHelp+=mfStep/mfStepHelp*fAct;
/*?*/                         }
/*?*/                     }
/*?*/                     else
/*?*/                     {
/*?*/                         while(fActHelp <= fAct)
/*?*/                         {
/*?*/                             if(CalcFact(fActHelp) > 1.0)
/*?*/                                 break;
/*?*/ 
/*?*/                             if( fActHelp <= fUpperLimit )
/*?*/                             {
/*?*/                                 GridLine(aLine,nPos=GetPos(fActHelp));
/*?*/ 
/*?*/                                 if(mpHelpGridList)
/*?*/                                     mpHelpGridList->NbcInsertObject(SetObjectAttr(new SdrPathObj(OBJ_PLIN, aLine),
/*?*/                                                                                   USHORT(mnIdHelpGrid),TRUE,TRUE,mpHelpGridAttr));
/*?*/ 
/*?*/                                 InsertMark(nPos,mnHelpTickLen,mnHelpTicks);
/*?*/                             }
/*?*/ 
/*?*/                             fActHelp+=mfStepHelp;
/*?*/                         }
/*?*/                         //	If the main grid is drawn then skip the coinciding help grid lines.
/*?*/                         if ((fActHelp == fAct) && mpMainGridList || mbColText)
/*?*/                             fActHelp+=mfStepHelp;
/*?*/                     }
/*?*/                 }
/*N*/ 
/*N*/                 if( mpAxisList && fAct < fUpperLimit )
/*N*/                 {
/*N*/                     if(mbShowDescr)
/*N*/                         CreateMarkDescr(fAct,nMainPos);
/*N*/                     else
/*?*/                         InsertMark(nMainPos,mnTickLen,mnTicks);
/*N*/                 }
/*N*/                 IncValue( fAct, mfStep, mbLogarithm );
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/ 	// restore original values. Might be obsolete
/*N*/ 	mfMin	   = fMinBack;
/*N*/ 	mfMax      = fMaxBack;
/*N*/ 	mfStep     = fStepBack;
/*N*/ 	mfStepHelp = fStepHelpBack;
/*N*/ 
/*N*/ 	mnTicks			=nTicksBack;
/*N*/ 	mnHelpTicks		=nHelpTicksBack;
/*N*/ 	mnTickLen		=nTickLenBack;
/*N*/ 	mnHelpTickLen	=nHelpTickLenBack;
/*N*/ 	mbLogarithm		=bLogarithmBack;
/*N*/ }

/*?*/ inline double ChartAxis::GetData(long nCol,long nRow)
/*?*/ {
/*?*/ 	double fData=mpModel->GetData(nCol,nRow);
/*?*/ 	if(mbPercent)
/*?*/ 		fData=Data2Percent(fData,nCol,nRow);
/*?*/ 	return fData;
/*?*/ };
// Falls es sich um eine gespiegelte Achse handelt, ist es nicht möglich,
// einmal Logarithmus zu setzen, das andere mal nicht.
// Die anderen Werte müssen evtl. (bei automatik) angepasst werden.
// allerdings darf hier geändert werden, um z.B. links 0-10 Volt
// und rechts 0-1000 mV an der Achse zu setzen. Das die Werte dann trotzdem
// identisch sind, liegt in diesem Fall dann in der Verantwortung des Users.
/*N*/ BOOL ChartAxis::AttachIfNoOwnData(const ChartAxis* pAxis)
/*N*/ {
/*N*/ 	if(!mbAlternativIdUsed) //Hat eigene Daten!
/*N*/ 		return FALSE;

    //es macht keinen Sinn, unsichtbare Achsen zu kopieren:
/*?*/ 	if(!pAxis->IsVisible() && !pAxis->HasDescription())
/*?*/ 		return FALSE;
/*?*/ 
/*?*/ 	ReadAutoAttr();
/*?*/ 
/*?*/ 	mbLogarithm    = pAxis->mbLogarithm;
/*?*/ 	mpAxisAttr->Put(SfxBoolItem(SCHATTR_AXIS_LOGARITHM,mbLogarithm));
/*?*/ 
/*?*/ 	if(mbAutoStep)
/*?*/ 		mfStep=pAxis->mfStep;
/*?*/ 	if(mbAutoStepHelp)
/*?*/ 		mfStepHelp=pAxis->mfStepHelp;
/*?*/ 	if(mbAutoMin)
/*?*/ 		mfMin=pAxis->mfMin;
/*?*/ 	if(mbAutoMax)
/*?*/ 		mfMax=pAxis->mfMax;
/*?*/ 	if(mbAutoOrigin)
/*?*/ 		mfOrigin=pAxis->mfOrigin;
/*?*/ 
/*?*/ 	FillItemSet();
/*?*/ 
/*?*/ 	return TRUE;
/*?*/ 
/*N*/ }
/*N*/ double ChartAxis::NumStepsMain()
/*N*/ {
/*N*/ 	if( mbLogarithm )
/*N*/ 	{
/*?*/ 		DBG_ASSERT( mfStep > 1.0, "ChartAxis: argument of log must be greater than 1" );
/*?*/ 		return log( mfMax - mfMin ) / log( mfStep );
/*N*/ 	}
/*N*/ 
/*N*/ 	return (mfMax - mfMin) / mfStep;
/*N*/ };

/*N*/ long ChartAxis::GetDescrWidth()
/*N*/ {
/*N*/ 	double fAllTextLen = IsVertical() ? (double)maRefArea.GetHeight()
/*N*/ 									  : (double)maRefArea.GetWidth();
/*N*/ 	long   nDescrWidth=0;
/*N*/ 
/*N*/ 	if(!mbColText) //z.B. bei XY-Charts gelangt man hier hin!
/*N*/ 	{
/*N*/ 		if(mfStep!=0.0)
/*N*/ 		{
/*N*/ 			double fCnt = NumStepsMain();
/*N*/ 			if (mbCenterText)
/*N*/ 				fCnt -= 1.0;
/*N*/ 
/*N*/ 			nDescrWidth  = (long) (fAllTextLen / fCnt);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else //Area, Bar etc.
/*N*/ 	{
/*N*/ 		long  nColCnt=mpModel->GetColCount();
/*N*/ 
/*N*/ 		if (mbCenterText && nColCnt > 1)
/*N*/ 				nColCnt--;
/*N*/ 
/*N*/ 		nDescrWidth = (long) (fAllTextLen / (double)nColCnt);
/*N*/ 	}
/*N*/ 	return nDescrWidth;
/*N*/ }

/*N*/ UINT32 ChartAxis::GetNumFormat( BOOL bPercent )
/*N*/ {
/*N*/ 	return ((const SfxUInt32Item&)mpAxisAttr->Get(bPercent
/*N*/ 		? SCHATTR_AXIS_NUMFMTPERCENT : SCHATTR_AXIS_NUMFMT)).GetValue();
/*N*/ }



/*N*/ BOOL ChartAxis::TranslateMergedNumFormat( SvNumberFormatterIndexTable* pTransTable )
/*N*/ {
/*N*/ 	ULONG nFmt, nMrgFmt;
/*N*/ 	BOOL bRet =FALSE;
/*N*/ 
/*N*/ 	nFmt = GetNumFormat( mbPercent );
/*N*/ 	nMrgFmt = mpNumFormatter->GetMergeFmtIndex( nFmt );
/*N*/ 	if( nFmt != nMrgFmt )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SetNumFormat( mbPercent, nMrgFmt );
/*N*/ 	}
/*N*/ 
/*N*/ 	nFmt = GetNumFormat( ! mbPercent );
/*N*/ 	nMrgFmt = mpNumFormatter->GetMergeFmtIndex( nFmt );
/*N*/ 	if( nFmt != nMrgFmt )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	SetNumFormat( ! mbPercent, nMrgFmt );
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void ChartAxis::SetAxisList( SdrObjList *pList )
/*N*/ {
/*N*/ 	mpAxisList = pList;
/*N*/ 	maRectA = maRectB = Rectangle();
/*N*/ }

/*N*/ long ChartAxis::GetUniqueIdByObjectId( long nObjectId )
/*N*/ {
/*N*/     long nResult = CHAXIS_AXIS_UNKNOWN;
/*N*/ 
/*N*/     switch( nObjectId )
/*N*/     {
/*N*/         case CHOBJID_DIAGRAM_X_AXIS:
/*N*/             nResult = CHAXIS_AXIS_X;
/*N*/             break;
/*N*/         case CHOBJID_DIAGRAM_Y_AXIS:
/*N*/             nResult = CHAXIS_AXIS_Y;
/*N*/             break;
/*N*/         case CHOBJID_DIAGRAM_Z_AXIS:
/*N*/             nResult = CHAXIS_AXIS_Z;
/*N*/             break;
/*N*/         case CHOBJID_DIAGRAM_A_AXIS:
/*N*/             nResult = CHAXIS_AXIS_A;
/*N*/             break;
/*N*/         case CHOBJID_DIAGRAM_B_AXIS:
/*N*/             nResult = CHAXIS_AXIS_B;
/*N*/             break;
/*N*/     }
/*N*/ 
/*N*/     return nResult;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
