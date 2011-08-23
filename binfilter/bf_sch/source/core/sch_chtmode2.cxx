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

#ifndef _STREAM_HXX
// enable stream operators >>/<< for UniString (8 Bit !)
//#ifndef ENABLE_STRING_STREAM_OPERATORS
//#define ENABLE_STRING_STREAM_OPERATORS
//#endif
#endif

#include "schattr.hxx"
#include "memchrt.hxx"

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE	        0
#define ITEMID_CHARTTEXTORIENT	SCHATTR_TEXT_ORIENT
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS

#include <bf_svtools/eitem.hxx>

#endif

#include <bf_svtools/itempool.hxx>


#include <bf_svx/svdopath.hxx>


#include <bf_svx/xlnclit.hxx>
#include <bf_svx/xlnwtit.hxx>
#ifndef _ZFORLIST_HXX //autogen
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#endif
#include <bf_svx/eeitem.hxx>
#include <bf_svtools/zformat.hxx>
// header for getProcessServiceFactory

#include "chmod3d.hxx" //SchRectObj

#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#define ITEMID_FONTWIDTH   EE_CHAR_FONTWIDTH
#define ITEMID_FONT        EE_CHAR_FONTINFO
#include "globfunc.hxx"
#include <bf_svx/svxids.hrc>
#include "schresid.hxx"
#include "schiocmp.hxx"

#include "strings.hrc"
#include "glob.hrc"

#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/fontitem.hxx>

#include <tools/tenccvt.hxx>

#include "pairs.hxx"
#include "chaxis.hxx"


#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
namespace binfilter {


/************************************************************************/
const USHORT nCompatAxisWhichPairs[] =
{
    SCHATTR_TEXT_START, SCHATTR_TEXT_END,
    SCHATTR_Y_AXIS_START, SCHATTR_Z_AXIS_END, //X-Z!
    SCHATTR_AXISTYPE, SCHATTR_AXISTYPE,
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,
    SCHATTR_TEXT_OVERLAP, SCHATTR_TEXT_OVERLAP,
    SCHATTR_AXIS_START,SCHATTR_AXIS_END,
    XATTR_LINE_FIRST, XATTR_LINE_LAST,
    EE_ITEMS_START, EE_ITEMS_END,
    SID_TEXTBREAK, SID_TEXTBREAK,
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE, //10.585
   0
};
/************************************************************************/

enum ChartStyleV0
{
    CHART2D_LINE,
    CHART2D_STACKEDLINE,
    CHART2D_BAR,
    CHART2D_STACKEDBAR,
    CHART2D_COLUMN,
    CHART2D_STACKEDCOLUMN,
    CHART2D_AREA,
    CHART2D_STACKEDAREA,
    CHART2D_PIE,
    CHART3D_STRIPE,
    CHART3D_BAR,
    CHART3D_FLATBAR,
    CHART3D_STACKEDFLATBAR,
    CHART3D_AREA,
    CHART3D_STACKEDAREA,
    CHART3D_SURFACE,
    CHART3D_PIE
};

/*************************************************************************
|*
|* Datenbeschriftung erzeugen
|*
\************************************************************************/

//SdrObject*

/*N*/ USHORT ChartModel::GetRegressStrId( long nRow )
/*N*/ {
/*N*/
/*N*/ 	const SfxItemSet& aDataRowAttr = GetDataRowAttr( nRow );
/*N*/ 	USHORT nStringID = 0;
/*N*/
/*N*/ 	switch (((const SfxInt32Item &) aDataRowAttr.Get (SCHATTR_STAT_REGRESSTYPE)).GetValue ())
/*N*/ 	{
/*N*/ 		case CHREGRESS_NONE :
/*N*/ 			break;
/*N*/
/*N*/ 		case CHREGRESS_LINEAR :
/*?*/ 			nStringID = STR_REGRESSION_LINEAR;
/*?*/ 			break;
/*N*/
/*?*/ 		case CHREGRESS_LOG :
/*?*/ 			nStringID = STR_REGRESSION_LOG;
/*?*/ 			break;
/*?*/
/*?*/ 		case CHREGRESS_EXP :
/*?*/ 			nStringID = STR_REGRESSION_EXP;
/*?*/ 			break;
/*?*/
/*?*/ 		case CHREGRESS_POWER :
/*?*/ 			nStringID = STR_REGRESSION_POWER;
/*?*/
/*N*/ 	}
/*N*/ 	return nStringID;
/*N*/ }
/*************************************************************************
|*
|* Diagrammlegende erzeugen
|*
\************************************************************************/

/*N*/ SdrObjGroup* ChartModel::CreateLegend(const Rectangle &aRect)
/*N*/ {
/*N*/ 	// Default ist, dass die Legende nicht breiter als 20% der gesamten Seitenbreite
/*N*/ 	// verwenden darf, unter der Bedingung, dass sie links oder rechts vom Chart steht.
/*N*/ 	// wenn sie ueber oder unter dem Chart steht, dann ist die Seitenbreite das Maximum
/*N*/
/*N*/ 	//TVM: obiges ist nicht richtig, CreateLegend wurde stets ohne 2.Argument
/*N*/ 	//aufgerufen, daher galt immer fMax...=0.2, ausserdem bezieht sich die maximale
/*N*/ 	//Breite auf den Text in der Legende, nicht auf die Legende selbst
/*N*/ 	//(Test: 1-Zeiliges Chart mit langem Text!)
/*N*/ 	//Obiges klngt aber durchaus plausibel, also sollte man es mal bei Gelegenheit
/*N*/ 	//einbauen (ToDo:-Liste)
/*N*/ 	const double  fMaximumWidth=0.2;
/*N*/
/*N*/ 	SvxChartLegendPos eLegendPos = ((const SvxChartLegendPosItem&) pLegendAttr->Get(SCHATTR_LEGEND_POS)).
/*N*/ 								GetValue();
/*N*/ 	BOOL bWide = (eLegendPos == CHLEGEND_TOP || eLegendPos == CHLEGEND_BOTTOM);
/*N*/ 	BOOL bRowLegend = !IsPieChart();
/*N*/ 	BOOL bReverse = !bWide && IsStackedChart();
/*N*/ 	BOOL bForceSolidLine = FALSE;
/*N*/
/*N*/ 	SchObjGroup* pGroup = NULL;
/*N*/
/*N*/ 	if (bLegendVisible)
/*N*/ 	{
/*N*/
/*N*/ 		List  aTextList;
/*N*/ 		long i;
/*N*/ 		long nRowCnt     = GetRowCount();
/*N*/ 			// FG: nCnt ist die Anzahl Texte!
/*N*/ 		long nCnt        = bRowLegend ? nRowCnt : GetColCount();
/*N*/ 		long nMaxX        = 0;
/*N*/ 		long nMaxY        = 0;
/*N*/
/*N*/ 		long  nLineMaxY = 0; //#50082#
/*N*/
/*N*/ 		long* pHeightOfEntry = new long[nCnt*2];    // FG: Wirkliche Hoehe der Zeilen
/*N*/ 		long* pWidthOfEntry  = new long[nCnt*2];    // FG: Wirkliche Breite der Zeilen
/*N*/ 		long nLines       = 0;                // Anzahl Zeilen
/*N*/ 		long nActualColumn = 1; // FG: Zaehlt die Anzahl Spalten
/*N*/
/*N*/ 		long* pRegressNr  = new long [nCnt];
/*N*/ 		memset (pRegressNr, 0, sizeof (long) * nCnt);
/*N*/
/*N*/ 		SfxItemSet aTextAttr(*pItemPool, nTextWhichPairs);
/*N*/
/*N*/ 		aTextAttr.Put(*pLegendAttr);
/*N*/
/*N*/ 		// FG: Hier wird einmal die Liste aller Texte durchgegangen um die maximale Hoehe und
/*N*/ 		//     die maximale Breite zu bestimmen. Bei der Gelegenheit merkt man sich auch die
/*N*/ 		//     Ausmasse der einzelnen Eintraege.
/*N*/ 		for (i = 0; i < nCnt; i++)
/*N*/ 		{
/*N*/ 			// FG: Mehr als fMaximumWidth des Charts soll die Legende nie einnehmen
/*N*/ 			SdrObject *pText = CreateTextObj(CHOBJID_TEXT, Point (),
/*N*/ 									 bRowLegend ? RowText(i) : ColText(i),
/*N*/ 									 aTextAttr,
/*N*/ 									 FALSE,
/*N*/ 									 CHADJUST_TOP_LEFT,  //FG: wie der Default
/*N*/ 									 GetPage(0)->GetSize().Width() * fMaximumWidth);
/*N*/
/*N*/ 			// FG: Das hier soll verhindern dass der Text in der Legende markiert werden kann
/*N*/           //     dazu gibt es ja kein gueltiges Kontextmenue
/*N*/ 			pText->SetMarkProtect(TRUE);
/*N*/ 			aTextList.Insert(pText, LIST_APPEND);
/*N*/
/*N*/ 			pWidthOfEntry[i] = pText->GetLogicRect().GetWidth();
/*N*/ 			pHeightOfEntry[i] = pText->GetLogicRect().GetHeight();
/*N*/ 			nMaxX  = Max (nMaxX, pWidthOfEntry[i]);
/*N*/ 			nMaxY  = Max (nMaxY, pHeightOfEntry[i]);
/*N*/ 		}
/*N*/
/*N*/ 		if (IsXYChart())
/*N*/ 		{
/*N*/ 			USHORT nStringID;
/*N*/ 			for( i = 1; i < nCnt; i++ )
/*N*/ 			{
/*N*/ 				nStringID = GetRegressStrId( i );
/*N*/ 				if( nStringID )
/*N*/ 				{
/*?*/ 					SchResId aRegId = SchResId( nStringID );
/*?*/ 					String aRegressStr( aRegId );
/*?*/ 					String aSeriesName( bRowLegend
/*?*/ 										? RowText( i )
/*?*/ 										: ColText( i ));
/*?*/ 					String aLegendText( SchResId( STR_STATISTICS_IN_LEGEND ));
/*?*/
/*?*/ 					aLegendText.SearchAndReplaceAscii( "$(STATTYP)", aRegressStr );
/*?*/ 					aLegendText.SearchAndReplaceAscii( "$(ROWNAME)", aSeriesName );
/*?*/
/*?*/ 					SdrObject *pText = CreateTextObj( CHOBJID_TEXT, Point(), aLegendText,
/*?*/ 													  aTextAttr, FALSE,
/*?*/ 													  CHADJUST_TOP_LEFT,
/*?*/ 													  GetPage( 0 )->GetSize().Width() * fMaximumWidth );
/*?*/
/*?*/ 					pText->SetMarkProtect( TRUE );
/*?*/ 					aTextList.Insert(pText, LIST_APPEND);
/*?*/
/*?*/ 					pWidthOfEntry[nLines+nCnt]  = pText->GetLogicRect().GetWidth();
/*?*/ 					pHeightOfEntry[nLines+nCnt] = pText->GetLogicRect().GetHeight();
/*?*/ 					nMaxX  = Max (nMaxX,  pWidthOfEntry[nLines+nCnt]);
/*?*/ 					nMaxY  = Max (nMaxY, pHeightOfEntry[nLines+nCnt]);
/*?*/
/*?*/ 				   // nMaxX  = Max (nMaxX, pText->GetLogicRect().GetWidth());
/*?*/ 				   // nMaxY  = Max (nMaxY, pText->GetLogicRect().GetHeight());
/*?*/
/*?*/ 					pRegressNr [nLines] = i;
/*?*/ 					nLines ++;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if (IsXYChart ()) nCnt --;
/*N*/ 		nLines += nCnt;
/*N*/
/*N*/ 		long nTextRows    = 0;
/*N*/ 		long nTextCols    = 0;
/*N*/ 		long nShows       = 0;
/*N*/
/*N*/ 		ULONG nLegendHeight = ((SvxFontHeightItem &) pLegendAttr->Get (EE_CHAR_FONTHEIGHT)).GetHeight();
/*N*/ 		long nLittleSpace = nLegendHeight / 3;
/*N*/
/*N*/ 		// FG: Hier wird berechnet wieviele Spalten und Zeilen die Legende haben soll
/*N*/ 		if (!bWide)  // dann ist die Legende mit den Zeilen untereinander links oder rechts
/*N*/ 		{
/*N*/ 			// FG: Die Legende darf maximal 90% der Blatthoehe einnehmen und maximal 50% der Blattbreite
/*N*/ 			if (nLines * (nMaxY + nLittleSpace) < (aRect.GetHeight() - 0.1 * aRect.GetHeight()))
/*N*/ 			{
/*N*/ 				nTextRows = nLines;
/*N*/ 				nTextCols = 1;      // Also eine Spalte
/*N*/ 			}
/*N*/ 			else  // Es gibt also mehrere Spalten
/*N*/ 			{
/*?*/ 				nTextRows = (long) ((aRect.GetHeight() - 0.1 * aRect.GetHeight()) / (nMaxY + nLittleSpace));
/*?*/ 				if(!nTextRows)
/*?*/ 					nTextRows=1;
/*?*/ 				nTextCols = (long) ((nLines % nTextRows) ? nLines / nTextRows + 1 : nLines / nTextRows);
/*?*/ 				// FG: Falls die Legende zu breit (mehr als 50% der Chart-Breite) wird muss man nachregeln
/*?*/ 				if (nTextCols * (nMaxX + nLittleSpace) > aRect.GetWidth())
/*?*/ 				{
/*?*/ 					nTextCols = (long) (aRect.GetWidth() * 0.5 / (nMaxX + nLittleSpace));
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else  // die Legende befindet sich also unter oder ueber dem Chart (die wird noch beliebig gross)
/*N*/ 		{
/*?*/ 			nTextCols = (nLines * (nMaxX + 2*nLittleSpace + nLegendHeight) + nLittleSpace < aRect.GetWidth())
/*?*/ 						 ? nLines : (aRect.GetWidth() - nLittleSpace) / (nMaxX + 2*nLittleSpace + nLegendHeight);
/*?*/ 			nTextRows = nTextCols
/*?*/ 						 ? ((nLines % nTextCols) ? nLines / nTextCols + 1 : nLines / nTextCols) : 0;
/*N*/ 		}

/*N*/ 		if (nTextRows > 0 && nTextCols > 0)
/*N*/ 		{
/*N*/ 			pGroup = new SchObjGroup;
/*N*/ 			pGroup->InsertUserData(new SchObjectId(CHOBJID_LEGEND));
/*N*/ 			// pGroup->SetResizeProtect(TRUE);
/*N*/ 			SdrObject*  pObj;
/*N*/ 			SdrObjList* pObjList = pGroup->GetSubList();
/*N*/
/*N*/ 			Point aTextPos (nLittleSpace, nLegendHeight / 4);
/*N*/
/*N*/ 			for (i = 0, nShows = 0;
/*N*/ 				 i < nLines;
/*N*/ 				 i++, nShows ++)
/*N*/ 			{
/*N*/ 				// FG: bReverse ist fuer gestapelte Diagramme gedacht, damit die Legendensymbole
/*N*/ 				//     optisch der Reihenfolge des stapelns entsprechen.
/*N*/ 			   long nIndex = (IsXYChart())
/*N*/ 							   ?  i+1
/*N*/ 							   : (bReverse)
/*N*/ 								   ? nCnt - 1 - i
/*N*/ 								   : i;
/*N*/
/*N*/ 				if (i < nCnt)
/*N*/ 				{
/*N*/ 					BOOL bIsSymbol = FALSE;
/*N*/ 					BOOL bIsLine   = FALSE;
/*N*/ 					if(HasSymbols(nIndex))
/*N*/ 					{
/*N*/ 						bIsSymbol = TRUE;
/*N*/ 						pObj = CreateSymbol (Point (aTextPos.X () + nLegendHeight / 2,
/*N*/ 															aTextPos.Y () + nLegendHeight / 2 + nLittleSpace / 3),
/*N*/ 											 nIndex, 0, (SfxItemSet &) GetDataRowAttr(nIndex), nLegendHeight, FALSE);
/*N*/ 						if(pObj)
/*N*/ 						{
/*N*/ 							Rectangle aRect(pObj->GetSnapRect());
/*N*/ 							if((aRect.GetHeight() > nLegendHeight) && nLegendHeight)
/*N*/ 							{
/*?*/ 								Fraction aFract(nLegendHeight,aRect.GetHeight());
/*?*/ 								pObj->NbcResize(aRect.Center(),aFract,aFract);
/*N*/ 							}
/*N*/ 						}
/*N*/ 						else //dann Linie als Legendensymbol, sonst geht evtl. garnix mehr
/*N*/ 						{
/*?*/ 							XPolygon aLine(2);
/*?*/ 							aLine [0] =
/*?*/ 							aLine [1] = aTextPos;
/*?*/ 							aLine [1].X() += nLegendHeight;
/*?*/ 							aLine [1].Y() += nLegendHeight;
/*?*/ 							bIsLine = TRUE;
/*?*/ 							pObj = new SdrPathObj(OBJ_PLIN, aLine);
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else if (IsLine(nIndex))
/*N*/ 					{
/*N*/ 						XPolygon aLine(2);
/*N*/
/*N*/ 						aLine [0] =
/*N*/ 						aLine [1] = aTextPos;
/*N*/ 						aLine [1].X() += nLegendHeight;
/*N*/ 						aLine [1].Y() += nLegendHeight;
/*N*/
/*N*/ 						bIsLine = TRUE;
/*N*/ 						pObj = new SdrPathObj(OBJ_PLIN, aLine);
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						// create rectangular shape as legend symbol
/*N*/ 						pObj = new SdrRectObj(Rectangle(Point (aTextPos.X(),
/*N*/ 															   aTextPos.Y() + nLittleSpace / 3),
/*N*/ 														Size(nLegendHeight, nLegendHeight)));
/*N*/ 						bForceSolidLine = TRUE;
/*N*/ 					}
/*N*/ 					// FG: setzen des Symbols neben dem Legendentext
/*N*/
/*N*/ 					SfxItemSet *pSymbolAttr;
/*N*/
/*N*/ 					if( bRowLegend )
/*N*/ 					{
/*N*/ 						pSymbolAttr = new SfxItemSet( GetDataRowAttr( nIndex ) );
/*N*/ 						if( ! bIsLine && pSymbolAttr )
/*N*/ 						  GenerateSymbolAttr( *pSymbolAttr, nIndex, SYMBOLMODE_LEGEND );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						pSymbolAttr = new SfxItemSet( GetFullDataPointAttr( nIndex, 0 ) );
/*N*/ 						if( ! bIsLine && pSymbolAttr )
/*N*/ 						  GenerateSymbolAttr( *pSymbolAttr, 0, SYMBOLMODE_LEGEND );
/*N*/ 					}
/*N*/
/*N*/ 					if( bForceSolidLine )
/*N*/ 					{
/*N*/ 						XLineStyle eLineStyle =
/*N*/ 							SAL_STATIC_CAST( const XLineStyleItem *, &(pSymbolAttr->Get( XATTR_LINESTYLE )) )->GetValue(); // bug in Win-C++ compiler: casting to pointer
/*N*/
/*N*/ 						if( eLineStyle == XLINE_NONE )
/*N*/ 						{
/*N*/ 							pSymbolAttr->ClearItem( XATTR_LINESTYLE );
/*N*/ 							pSymbolAttr->ClearItem( XATTR_LINEWIDTH );
/*N*/ 							pSymbolAttr->ClearItem( XATTR_LINECOLOR );
/*N*/ 						}
/*N*/ 					}
/*N*/
/*N*/ //-/					pObj->NbcSetAttributes( *pSymbolAttr, FALSE );
/*N*/ 					pObj->SetItemSet(*pSymbolAttr);
/*N*/
/*N*/
/*N*/ 					if(bRowLegend)
/*N*/ 					{
/*N*/ 						pObj->InsertUserData(new SchObjectId(CHOBJID_LEGEND_SYMBOL_ROW));
/*N*/ 						pObj->InsertUserData(new SchDataRow(nIndex));
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						pObj->InsertUserData(new SchObjectId(CHOBJID_LEGEND_SYMBOL_COL));
/*N*/ 						pObj->InsertUserData(new SchDataPoint(nIndex, 0));
/*N*/ 					}
/*N*/
/*N*/ 					pObj->SetMoveProtect(TRUE);
/*N*/ 					pObj->SetResizeProtect(TRUE);
/*N*/ 					pObjList->NbcInsertObject(pObj);
/*N*/
/*N*/ 					SdrObject *pText = (SdrObject*)aTextList.GetObject(nIndex);
/*N*/
/*N*/ 					pText->NbcMove(Size(aTextPos.X() + nLittleSpace + nLegendHeight, aTextPos.Y()));
/*N*/ 					pObjList->NbcInsertObject(pText);
/*N*/
/*N*/ 					delete pSymbolAttr;
/*N*/ 				}
/*N*/ 				else //i >= nCnt
/*N*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if (pRegressNr [i - nCnt])
/*N*/ 				}
/*N*/
/*N*/               // FG: Jetzt wird aTextPos fuer den naechsten Legendeneintrag gesetzt
/*N*/ 			   if (bWide)
/*N*/ 				{
/*?*/ 					if (nShows >= nTextCols - 1)
/*?*/ 					{
/*?*/ 						nLineMaxY = Max(nLineMaxY,pHeightOfEntry[i]);//#50082# #NACHTRAG#
/*?*/ 						aTextPos.Y () += nLineMaxY  + nLittleSpace;//#50082#
/*?*/ 						aTextPos.X ()  = nLittleSpace;//nLegendHeight / 6;//SP3: #49906#
/*?*/ 						nLineMaxY = 0; //#50082# #NACHTRAG#
/*?*/ 						nShows = -1;
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						nLineMaxY = Max(nLineMaxY,pHeightOfEntry[i]);//#50082#
/*?*/ 						aTextPos.X () += nMaxX +  2 * nLittleSpace + nLegendHeight;
/*?*/ 					}
/*N*/ 				}
/*N*/ 				else if (nShows >= nTextRows - 1) // FG: Die Legende wird in mehrere Spalten umgebrochen
/*N*/ 				{                                 //     also den Spaltenabstand setzen
/*N*/ 					aTextPos.X () += nMaxX + nLittleSpace + nLegendHeight + 2*nLittleSpace;
/*N*/ 					aTextPos.Y ()  = nLegendHeight / 4;
/*N*/ 					nShows = -1;
/*N*/ 					if (nActualColumn >= nTextCols)  // FG: Dann wird die Legende zu breit!
/*N*/ 					{
/*N*/ 						break;                       // aus der for-Schleife raus,
/*N*/ 													 // es muessen noch Felder deleted werden.
/*N*/ 					}
/*?*/ 					nActualColumn++;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/
/*N*/ 					aTextPos.Y() += pHeightOfEntry[nIndex] + nLittleSpace;
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 				//FG: Jetzt wird das umschliessende Rechteck berechnet. Man fraegt erst das
/*N*/ 				//    BoundingRect des Legenden-Gruppenobjektes ab und addiert an den Raendern ein
/*N*/ 				//    wenig Platz dazu.
/*N*/ 			Rectangle aLogRect = pGroup->GetLogicRect();
/*N*/ 			aLogRect.Left() -= nLittleSpace;
/*N*/ 			aLogRect.Right() += nLittleSpace;
/*N*/ 			aLogRect.Top() -= nLittleSpace;
/*N*/ 			aLogRect.Bottom() += nLittleSpace;
/*N*/ 			SdrRectObj* pRectObj = new SchRectObj(aLogRect);
/*N*/ 			// FG: Das hier soll verhindern dass das Rechteck um die Legende markiert werden kann
/*N*/ 			pRectObj->SetMarkProtect(TRUE);
/*N*/ 			pRectObj->SetModel( this );
/*N*/ 			pObjList->NbcInsertObject(SetObjectAttr(pRectObj, CHOBJID_LEGEND_BACK,
/*N*/ 													TRUE, TRUE, pLegendAttr), 0);
/*N*/ 		}
/*N*/
/*N*/ 		delete[] pRegressNr;
/*N*/ 		delete[] pHeightOfEntry;
/*N*/ 		delete[] pWidthOfEntry;
/*N*/ 	}
/*N*/
/*N*/ 	return pGroup;
/*N*/ }

/*************************************************************************
|*
|* Diagramm erzeugen
|*
\************************************************************************/

/*N*/ SdrObjGroup* ChartModel::CreateChart(const Rectangle &rRect)
/*N*/ {
/*N*/ 	if( pDocShell )
/*N*/ 		pDocShell->SetWaitCursor( TRUE );
/*N*/
/*N*/ 	Rectangle aRect( rRect );
/*N*/ 	SdrObjGroup* pGroup;
/*N*/
/*N*/ 	switch (eChartStyle)
/*N*/ 	{
/*N*/ 		case CHSTYLE_2D_AREA:
/*N*/ 		case CHSTYLE_2D_STACKEDAREA:
/*N*/ 		case CHSTYLE_2D_PERCENTAREA:
/*N*/ 			pGroup = Create2DRowLineChart(aRect);//neu, test!
/*N*/ 			break;
/*N*/
/*N*/ 		case CHSTYLE_2D_LINE:
/*N*/ 		case CHSTYLE_2D_STACKEDLINE:
/*N*/ 		case CHSTYLE_2D_PERCENTLINE:
/*N*/ 		case CHSTYLE_2D_LINESYMBOLS :
/*N*/ 		case CHSTYLE_2D_STACKEDLINESYM :
/*N*/ 		case CHSTYLE_2D_PERCENTLINESYM :
/*N*/ 		case CHSTYLE_2D_CUBIC_SPLINE :
/*N*/ 		case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL :
/*N*/ 		case CHSTYLE_2D_B_SPLINE :
/*N*/ 		case CHSTYLE_2D_B_SPLINE_SYMBOL :
/*N*/ 		case CHSTYLE_2D_STOCK_1:
/*N*/ 		case CHSTYLE_2D_STOCK_2:
/*N*/ 		case CHSTYLE_2D_STOCK_3:
/*N*/ 		case CHSTYLE_2D_STOCK_4:
/*N*/ 			pGroup = Create2DRowLineChart(aRect);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHSTYLE_2D_COLUMN:
/*N*/ 		case CHSTYLE_2D_STACKEDCOLUMN:
/*N*/ 		case CHSTYLE_2D_PERCENTCOLUMN:
/*N*/ 		case CHSTYLE_2D_BAR:
/*N*/ 		case CHSTYLE_2D_STACKEDBAR:
/*N*/ 		case CHSTYLE_2D_PERCENTBAR:
/*N*/ 		case CHSTYLE_2D_LINE_COLUMN:
/*N*/ 		case CHSTYLE_2D_LINE_STACKEDCOLUMN:
/*N*/ 			pGroup = Create2DColChart(aRect);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHSTYLE_2D_PIE_SEGOF1:
/*N*/ 		{
/*N*/ 			for (short i = 1; i < nPieSegCount; i++)
/*N*/ 				SetPieSegOfs(i, 0);
/*N*/             SetPieSegOfs( 0, 10 );
/*N*/ 			pGroup = Create2DPieChart(aRect);
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case CHSTYLE_2D_PIE_SEGOFALL:
/*N*/ 		{
/*N*/ 			for (short i = 0; i < nPieSegCount; i++)
/*N*/                 SetPieSegOfs( i, 10 );
/*N*/ 		}
/*N*/
/*N*/ 		case CHSTYLE_2D_PIE:
/*N*/ 			 pGroup = Create2DPieChart(aRect);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHSTYLE_2D_DONUT1:  //DonutCharts haben jetzt eigenes Create
/*N*/ 		case CHSTYLE_2D_DONUT2:
/*N*/ 			pGroup = Create2DDonutChart(aRect);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHSTYLE_2D_XYSYMBOLS :
/*N*/ 		case CHSTYLE_2D_XY_LINE :
/*N*/ 		case CHSTYLE_2D_XY :
/*N*/ 		case CHSTYLE_2D_CUBIC_SPLINE_XY :
/*N*/ 		case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY :
/*N*/ 		case CHSTYLE_2D_B_SPLINE_XY :
/*N*/ 		case CHSTYLE_2D_B_SPLINE_SYMBOL_XY :
/*N*/ 			pGroup = Create2DXYChart(aRect);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHSTYLE_2D_NET:
/*N*/ 		case CHSTYLE_2D_NET_SYMBOLS:
/*N*/ 		case CHSTYLE_2D_NET_STACK:
/*N*/ 		case CHSTYLE_2D_NET_SYMBOLS_STACK:
/*N*/ 		case CHSTYLE_2D_NET_PERCENT:
/*N*/ 		case CHSTYLE_2D_NET_SYMBOLS_PERCENT:
/*N*/ 			pGroup = Create2DNetChart(aRect);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHSTYLE_3D_COLUMN:
/*N*/ 		case CHSTYLE_3D_BAR:
/*N*/ 		case CHSTYLE_3D_STRIPE:
/*N*/ 		case CHSTYLE_3D_AREA:
/*N*/ 		case CHSTYLE_3D_SURFACE:
/*N*/ 			pGroup = Create3DDeepChart(aRect);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHSTYLE_3D_FLATCOLUMN:
/*N*/ 		case CHSTYLE_3D_STACKEDFLATCOLUMN:
/*N*/ 		case CHSTYLE_3D_PERCENTFLATCOLUMN:
/*N*/ 		case CHSTYLE_3D_STACKEDAREA:
/*N*/ 		case CHSTYLE_3D_PERCENTAREA:
/*N*/ 		case CHSTYLE_3D_FLATBAR:
/*N*/ 		case CHSTYLE_3D_STACKEDFLATBAR:
/*N*/ 		case CHSTYLE_3D_PERCENTFLATBAR:
/*N*/ 			pGroup = Create3DFlatChart(aRect);
/*N*/ 			break;
/*N*/
/*N*/ 		case CHSTYLE_3D_PIE:
/*N*/ 			pGroup = Create3DNewPieChart(aRect);
/*N*/ 			break;
/*N*/
/*N*/ 		default:
/*?*/ 			eChartStyle = CHSTYLE_2D_COLUMN;
/*?*/ 			pGroup = Create2DColChart(aRect);
/*?*/ 			break;
/*N*/ 	}
/*N*/
/*N*/ 	if( pDocShell )
/*N*/ 		pDocShell->SetWaitCursor( FALSE );
/*N*/
/*N*/ 	SdrPage* pPage=GetPage( 0 );
/*N*/ 	SdrObject* pObj = GetObjWithId( CHOBJID_DIAGRAM_AREA, *pPage );
/*N*/ 	if( pObj )
/*N*/ 		pObj->SetMoveProtect( TRUE );
/*N*/
/*N*/ 	return pGroup;
/*N*/ }

/*N*/ void ChartModel::PrepareOld3DStorage()
/*N*/ {
/*N*/ 	SfxItemSet aTmpSet(*pItemPool,nRowWhichPairs);
/*N*/ 	aTmpSet.Put(XLineStyleItem(XLINE_SOLID));
/*N*/ 	aTmpSet.Put(XLineWidthItem(0));
/*N*/ 	aTmpSet.Put(XLineColorItem(String(), RGBColor(COL_BLACK)));
/*N*/
/*N*/
/*N*/ 	long i,nCount=aDataRowAttrList.Count();
/*N*/ 	for (i=0;i<nCount;i++)
/*N*/ 	{
/*N*/ 		SfxItemSet *pDataRowAttr = new SfxItemSet (*pItemPool, nRowWhichPairs);
/*N*/ 		pDataRowAttr->Put(*(aDataRowAttrList.GetObject(i)));
/*N*/ 		pDataRowAttr->Put(aTmpSet);
/*N*/ 		aTmpDataRowAttrList.Insert (pDataRowAttr, LIST_APPEND);
/*N*/ 	}
/*N*/
/*N*/ 	SfxItemSet	*	pItemSet;
/*N*/ 	nCount=aDataPointAttrList.Count();
/*N*/ 	for (i=0;i<nCount;i++)
/*N*/ 	 {
/*N*/ 		SfxItemSet* pSet=new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/ 		pItemSet = aDataPointAttrList.GetObject(i);
/*N*/ 		if (pItemSet != NULL)
/*N*/ 			pSet->Put(*pItemSet);
/*N*/ 		pSet->Put(aTmpSet);
/*N*/ 		aTmpDataPointAttrList.Insert (pSet, LIST_APPEND);
/*N*/ 	 }
/*N*/
/*N*/ 	nCount=aSwitchDataPointAttrList.Count();
/*N*/ 	for (i=0;i<nCount;i++)
/*N*/ 	 {
/*N*/ 		SfxItemSet* pSet=new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/ 		pItemSet = aSwitchDataPointAttrList.GetObject(i);
/*N*/ 		if (pItemSet != NULL)
/*N*/ 			pSet->Put(*pItemSet);
/*N*/ 		pSet->Put(aTmpSet);
/*N*/ 		aTmpSwitchDataPointAttrList.Insert (pSet, LIST_APPEND);
/*N*/ 	 }
/*N*/ }
/*N*/ void ChartModel::CleanupOld3DStorage()
/*N*/ {
/*N*/ 	long i,nCount = aTmpDataRowAttrList.Count();
/*N*/ 	for (i = 0 ; i < nCount; i++)
/*N*/ 		delete aTmpDataRowAttrList.GetObject(i);
/*N*/ 	aTmpDataRowAttrList.Clear();
/*N*/
/*N*/ 	nCount = aTmpDataPointAttrList.Count();
/*N*/ 	for (i = 0 ; i < nCount; i++)
/*N*/ 		delete aTmpDataPointAttrList.GetObject(i);
/*N*/ 	aTmpDataPointAttrList.Clear();
/*N*/
/*N*/ 	nCount = aTmpSwitchDataPointAttrList.Count();
/*N*/ 	for (i = 0 ; i < nCount; i++)
/*N*/ 		delete aTmpSwitchDataPointAttrList.GetObject(i);
/*N*/ 	aTmpSwitchDataPointAttrList.Clear();
/*N*/ }
/*************************************************************************
|*
|* Chart-Attribute speichern

\************************************************************************/
/*N*/ void ChartModel::PrepareAxisStorage()
/*N*/ {
/*N*/ 	if(pTmpXItems)
/*?*/ 		delete pTmpXItems;
/*N*/ 	if(pTmpYItems)
/*?*/ 		delete pTmpYItems;
/*N*/ 	if(pTmpZItems)
/*?*/ 		delete pTmpZItems;
/*N*/ 	//Leider muss das ummappen vorm speichern des pools stattfinden
/*N*/ 	pTmpXItems = new SfxItemSet(*pItemPool,nCompatAxisWhichPairs);
/*N*/ 	pTmpYItems = new SfxItemSet(*pItemPool,nCompatAxisWhichPairs);
/*N*/ 	pTmpZItems = new SfxItemSet(*pItemPool,nCompatAxisWhichPairs);
/*N*/
/*N*/ 	pTmpXItems->Put(GetAttr(CHOBJID_DIAGRAM_X_AXIS));
/*N*/ 	AxisAttrNew2Old(*pTmpXItems,CHOBJID_DIAGRAM_X_AXIS,TRUE);
/*N*/
/*N*/ 	pTmpYItems->Put(GetAttr(CHOBJID_DIAGRAM_Y_AXIS));
/*N*/ 	AxisAttrNew2Old(*pTmpYItems,CHOBJID_DIAGRAM_Y_AXIS,TRUE);
/*N*/
/*N*/ 	pTmpZItems->Put(GetAttr(CHOBJID_DIAGRAM_Z_AXIS));
/*N*/ 	AxisAttrNew2Old(*pTmpZItems,CHOBJID_DIAGRAM_Z_AXIS,TRUE);
/*N*/
/*N*/
/*N*/ 	//Achse wird auf Attr-Basis gestreamt!
/*N*/ 	pChartXAxis->FillItemSet();
/*N*/ 	pChartYAxis->FillItemSet();
/*N*/ 	pChartZAxis->FillItemSet();
/*N*/ 	pChartBAxis->FillItemSet();
/*N*/ 	pChartAAxis->FillItemSet();
/*N*/ }

/*N*/ void ChartModel::StoreAttributes(SvStream& rOut) const
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/ 	ByteString aBStr( aMainTitle, RTL_TEXTENCODING_ASCII_US );
/*N*/ 	CHART_TRACE1( "ChartModel::StoreAttributes (%s)", aBStr.GetBuffer() );
/*N*/ #endif
/*N*/
/*N*/ 	rtl_TextEncoding eSysSet = ::GetSOStoreTextEncoding( gsl_getSystemTextEncoding());
/*N*/ 	rOut.SetStreamCharSet( eSysSet );
/*N*/
/*N*/ 	SchIOCompat aIO(rOut, STREAM_WRITE, 17);
/*N*/ 	long nFileFormat = rOut.GetVersion ();
/*N*/
/*N*/ 	//#50116# 8->9
/*N*/ 	//#54870# ->10
/*N*/ 	//12: ChartAchsen
/*N*/   //13 Nachtrag: X-AchsenAttr-default bei XY-Charts (store unveraendert)
/*N*/ 	//14 Overlap , Gapwith der 2 Y-Achsen
/*N*/ 	//15 Neues 3D-Pie
/*N*/   //16 Items fuer Achse SCHATTR_AXIS_SHOWDESCR,SCHATTR_AXIS_SHOWAXIS aktiviert,
/*N*/ 	//    bisher nur von pChartBAxis genutzt!
/*N*/
/*N*/ 	//FG: 12 - seit 20.02.1997 - Umbruch ja/ein pro Achse hinzu
/*N*/ 	//FG: 13 - seit 09.03.1997 Alle Variablen mit Tag 13:
/*N*/ 	//DL: 14 - seit 11.03.1997 aSpotColor & aAmbientColor
/*N*/ 	//                         FG  + TextausPositionen
/*N*/ 	//FG: 15 - set  14.03.1997 Textausrichtung
/*N*/ 	//SOH:16 - seit 04.05.1997 NumberFormatter
/*N*/
/*N*/ 	//TODO: konvertieren der Attribute pChartAttr in eChartStyle (Fileversion >=10)
/*N*/
/*N*/ 	UINT32 nMoreData = 16;
/*N*/
/*N*/ 	short i, nCount;
/*N*/
/*N*/ 	rOut << aLightVec->X ();
/*N*/ 	rOut << aLightVec->Y ();
/*N*/ 	rOut << aLightVec->Z ();
/*N*/
/*N*/ 	if (pChartData)
/*N*/ 	{
/*N*/ 		INT16 ID = CHDATAID_MEMCHART;    // damit aeltere versionen damit fertig werden
/*N*/ 		rOut << (INT16) ID;
/*N*/ 		rOut << *(SchMemChart*)pChartData;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		INT16 nInt16 = (INT16)CHDATAID_NONE; rOut << nInt16;
/*N*/ 	}
/*N*/
/*N*/ 	BOOL bDummy = TRUE;            // war frueher bOwnChart
/*N*/ 	rOut << bIsCopied;
/*N*/ 	rOut << fMinData;
/*N*/ 	rOut << (INT16)eChartStyle;
/*N*/
/*N*/ 	for (i = 0; i < LINE_POINT_COUNT; i++)
/*N*/ 		rOut << (INT16)eChartLinePoints[i];
/*N*/
/*N*/
/*N*/ 	// Das ehemalige Member aRowColors[]. Es wird hier simuliert, damit
/*N*/ 	// alte Codeversionen mit der hier geschriebenen Datei arbeiten koennen.
/*N*/ 	Color aRowColors[ROW_COLOR_COUNT];
/*N*/
/*N*/ 	aRowColors[0]   = Color(  0, 153, 255);
/*N*/ 	aRowColors[1]   = Color(255,  51, 102);
/*N*/ 	aRowColors[2]   = Color(102, 102, 102);
/*N*/ 	aRowColors[3]   = Color( 51, 204, 102);
/*N*/ 	aRowColors[4]   = Color(153, 102, 204);
/*N*/ 	aRowColors[5]   = Color(255, 153, 102);
/*N*/ 	aRowColors[6]   = Color(255, 204, 153);
/*N*/ 	aRowColors[7]   = Color(153, 204, 255);
/*N*/ 	aRowColors[8]   = Color(  0, 204, 204);
/*N*/ 	aRowColors[9]   = Color(153,   0, 153);
/*N*/ 	aRowColors[10]  = Color(255,  51,  51);
/*N*/
/*N*/ 	aRowColors[11]  = RGBColor(COL_WHITE);
/*N*/ 	for (i = 0; i < ROW_COLOR_COUNT; i++)
/*N*/ 		rOut << aRowColors[i];
/*N*/
/*N*/ 	const long nGapWidth  = 10;
/*N*/ 	const long nOverlap = 0;
/*N*/ 	rOut << (INT32)nGapWidth;
/*N*/ 	rOut << (INT32)nOverlap;
/*N*/   rOut << (INT32)nMarkLen; //entfaellt demnaechst! -> ChartAxis
/*N*/ 	rOut << aChartRect;
/*N*/ 	rOut << (INT32)nPieHeight;
/*N*/
/*N*/ 	rOut << (INT16)nPieSegCount;
/*N*/ 	for (i = 0; i < nPieSegCount; i++)
/*N*/ 		rOut << (INT32)pPieSegOfs[i];
/*N*/
/*N*/ 	INT16 nXA=nXAngle;
/*N*/ 	INT16 nYA=nYAngle;
/*N*/ 	INT16 nZA=nZAngle;
/*N*/ 	if(IsReal3D() && IsPieChart())
/*N*/ 	{
/*N*/ 		Vector3D aVRot,aVTrans,aVShear,aVScale;
/*N*/ 		aSceneMatrix.Decompose(aVScale,aVTrans,aVRot,aVShear);
/*N*/ 		long nAngle= (long)(aVRot.Z() * 1800.0 / F_PI);
/*N*/ 		nAngle -= 900;
/*N*/ 		nAngle += 3600;
/*N*/ 		nAngle %= 3600;
/*N*/ 		nYA = (INT16)nAngle;
/*N*/ 	}
/*N*/
/*N*/ 	rOut << (INT16)nXA;//anle;
/*N*/ 	rOut << (INT16)nYA;//ngle;
/*N*/ 	rOut << (INT16)nZA;//ngle;
/*N*/
/*N*/
/*N*/ 	//neu wegen Achsenrestrukturierung
/*N*/ 	BOOL bShowXAxis =pChartXAxis->IsVisible();
/*N*/ 	BOOL bShowXDescr=pChartXAxis->HasDescription();
/*N*/ 	BOOL bShowYAxis =pChartYAxis->IsVisible();
/*N*/ 	BOOL bShowYDescr=pChartYAxis->HasDescription();
/*N*/ 	BOOL bShowZAxis =pChartZAxis->IsVisible();
/*N*/ 	BOOL bShowZDescr=pChartZAxis->HasDescription();
/*N*/
/*N*/
/*N*/ 	// save unicode strings as ascii-strings for old binary format
/*N*/ 	rOut << (INT16)eSysSet;
/*N*/
/*N*/ 	rOut << bShowMainTitle;
/*N*/ 	rOut.WriteByteString( aMainTitle );
/*N*/ 	rOut << bShowSubTitle;
/*N*/ 	rOut.WriteByteString( aSubTitle );
/*N*/ 	rOut << bShowXAxisTitle;
/*N*/ 	rOut.WriteByteString( aXAxisTitle );
/*N*/ 	rOut << bShowYAxisTitle;
/*N*/ 	rOut.WriteByteString( aYAxisTitle );
/*N*/ 	rOut << bShowZAxisTitle;
/*N*/ 	rOut.WriteByteString( aZAxisTitle );
/*N*/ 	rOut << bShowXAxis;
/*N*/ 	rOut << bShowXGridMain;
/*N*/ 	rOut << bShowXGridHelp;
/*N*/ 	rOut << bShowXDescr;
/*N*/ 	rOut << bShowYAxis;
/*N*/ 	rOut << bShowYGridMain;
/*N*/ 	rOut << bShowYGridHelp;
/*N*/ 	rOut << bShowYDescr;
/*N*/ 	rOut << bShowZAxis;
/*N*/ 	rOut << bShowZGridMain;
/*N*/ 	rOut << bShowZGridHelp;
/*N*/ 	rOut << bShowZDescr;
/*N*/
/*N*/ 	// In diesem Set koennten ungueltige Items vorhanden sein, da es
/*N*/ 	// das Set der allgemeinen TitleAttribute (aller Titel) ist
/*N*/ 	pTitleAttr->ClearInvalidItems();
/*N*/ 	pTitleAttr->Store(rOut);
/*N*/
/*N*/ 	pMainTitleAttr->Store(rOut);
/*N*/ 	pSubTitleAttr->Store(rOut);
/*N*/ 	pXAxisTitleAttr->Store(rOut);
/*N*/ 	pYAxisTitleAttr->Store(rOut);
/*N*/ 	pZAxisTitleAttr->Store(rOut);
/*N*/
/*N*/ 	pAxisAttr->ClearInvalidItems();
/*N*/
/*N*/ 	pAxisAttr->Store(rOut);
/*N*/
/*N*/
/*N*/
/*N*/
/*N*/
/*N*/   //Abwaertskompatibel speichern (this->PrepareAxisStorage())
/*N*/ 	pTmpXItems->Store(rOut);
/*N*/ 	pTmpYItems->Store(rOut);
/*N*/ 	pTmpZItems->Store(rOut);
/*N*/
/*N*/ 	pGridAttr->ClearInvalidItems();
/*N*/ 	pGridAttr->Store(rOut);
/*N*/ 	pXGridMainAttr->Store(rOut);
/*N*/ 	pYGridMainAttr->Store(rOut);
/*N*/ 	pZGridMainAttr->Store(rOut);
/*N*/ 	pXGridHelpAttr->Store(rOut);
/*N*/ 	pYGridHelpAttr->Store(rOut);
/*N*/ 	pZGridHelpAttr->Store(rOut);
/*N*/ 	pDiagramAreaAttr->Store(rOut);
/*N*/ 	pDiagramWallAttr->Store(rOut);
/*N*/ 	pDiagramFloorAttr->Store(rOut);
/*N*/ 	pLegendAttr->Store(rOut);
/*N*/
/*N*/
/*N*/
/*N*/ 	if (nFileFormat == SOFFICE_FILEFORMAT_31)
/*N*/ 	{
/*N*/
/*N*/ 		SfxItemSet aStoreAttr (*pItemPool, XATTR_LINE_FIRST, XATTR_LINE_LAST,
/*N*/ 										   XATTR_FILL_FIRST, XATTR_FILL_LAST,
/*N*/ 										   //EE_ITEMS_START, EE_ITEMS_END,
/*N*/ 										   SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END, 0),
/*N*/ 					aPoolItemSet (*pItemPool, nRowWhichPairs),
/*N*/ 					*pItemSet;
/*N*/
/*N*/ 		// FG: 25.2.97 Das Schreiben im 3.1 Format  geht schief wenn man
/*N*/ 		//     in globfunc.cxx die Routine CopyAttributesFrom40to31 verwendet.
/*N*/ 		//     Dort wurden die items per Invalidate und Set einzeln kopiert. (falsch)
/*N*/ 		nCount = (short)aDataRowAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			SfxItemSet* pSet = aStoreAttr.Clone();
/*N*/ 			pSet->Put( *aDataRowAttrList.GetObject(i), TRUE );
/*N*/ 			pSet->Store( rOut );
/*N*/ 			delete pSet;
/*N*/ 		}
/*N*/
/*N*/ 		nCount = (short)aDataPointAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			SfxItemSet* pSet = aStoreAttr.Clone();
/*N*/ 			pItemSet = aDataPointAttrList.GetObject(i);
/*N*/ 			if (pItemSet != NULL)
/*N*/ 				pSet->Put (*pItemSet, TRUE );
/*N*/ 			else
/*N*/ 				pSet->Put (aPoolItemSet, TRUE);
/*N*/ 			pSet->Store(rOut);
/*N*/ 			delete pSet;
/*N*/ 		}
/*N*/
/*N*/ 		nCount = (short)aSwitchDataPointAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			SfxItemSet* pSet = aStoreAttr.Clone();
/*N*/ 			pItemSet = aSwitchDataPointAttrList.GetObject(i);
/*N*/ 			if (pItemSet != NULL)
/*N*/ 				pSet->Put (*pItemSet, TRUE );
/*N*/ 			else
/*N*/ 				pSet->Put (aPoolItemSet, TRUE);
/*N*/ 			pSet->Store(rOut);
/*N*/ 			delete pSet;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if(nFileFormat <= SOFFICE_FILEFORMAT_40 && IsReal3D()) //siehe auch DocShell, Save,SaveAs!!!!!
/*N*/ 	{
/*N*/
/*N*/ 		nCount = (short)aTmpDataRowAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 			aTmpDataRowAttrList.GetObject(i)->Store(rOut);
/*N*/
/*N*/ 		nCount = (short)aTmpDataPointAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 			aTmpDataPointAttrList.GetObject(i)->Store(rOut);
/*N*/
/*N*/ 		nCount = (short)aTmpSwitchDataPointAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 			aTmpSwitchDataPointAttrList.GetObject(i)->Store(rOut);
/*N*/
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SfxItemSet	aPoolItemSet (*pItemPool, nRowWhichPairs),
/*N*/ 				*	pItemSet;
/*N*/
/*N*/ 		nCount = (short)aDataRowAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 			aDataRowAttrList.GetObject(i)->Store(rOut);
/*N*/
/*N*/ 		nCount = (short)aDataPointAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			pItemSet = aDataPointAttrList.GetObject(i);
/*N*/ 			if (pItemSet != NULL)
/*N*/ 				pItemSet->Store(rOut);
/*N*/ 			else
/*N*/ 				aPoolItemSet.Store(rOut);
/*N*/ 		}
/*N*/
/*N*/ 		nCount = (short)aSwitchDataPointAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			pItemSet = aSwitchDataPointAttrList.GetObject(i);
/*N*/ 			if (pItemSet != NULL)
/*N*/ 				pItemSet->Store(rOut);
/*N*/ 			else
/*N*/ 				aPoolItemSet.Store(rOut);
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	rOut << (INT16)eDataDescr;
/*N*/ 	rOut << bShowSym;
/*N*/ 	rOut << bSwitchData;
/*N*/
/*N*/ 	UINT32 nTmp;
/*N*/
/*N*/ 	nTmp=((ChartModel*)this)->GetNumFmt(CHOBJID_DIAGRAM_Y_AXIS,FALSE);
/*N*/ 	rOut << nTmp;//nValFormat;
/*N*/ 	nTmp=((ChartModel*)this)->GetNumFmt(CHOBJID_DIAGRAM_Y_AXIS,TRUE);
/*N*/ 	rOut << nTmp;//nPercentValFormat;
/*N*/ 	nTmp=((ChartModel*)this)->GetNumFmt(CHOBJID_DIAGRAM_X_AXIS,FALSE);
/*N*/ 	rOut << nTmp;//nDescrFormat;
/*N*/ 	nTmp=((ChartModel*)this)->GetNumFmt(CHOBJID_DIAGRAM_X_AXIS,TRUE);
/*N*/ 	rOut << nTmp;//nPercentDescrFormat;
/*N*/
/*N*/ 	// Achtung : Gravierende Aenderungen an der Datei gegenueber der Auslieferungsversion
/*N*/ 	// es werden mehr Daten geschrieben : Die alte Version muss (!) das aber auch verkraften
/*N*/   // Die Reihenfolge muss (!!!!) eingehalten werden
/*N*/ 	pChartYAxis->StoreMemberCompat(rOut);
/*N*/ 	pChartXAxis->StoreMemberCompat(rOut);
/*N*/ 	pChartZAxis->StoreMemberCompat(rOut);
/*N*/
/*N*/ 	rOut << fMaxData;
/*N*/
/*N*/ 	rOut << nMoreData;
/*N*/
/*N*/ 	if (nMoreData >= 2)
/*N*/ 		if (pChartData)
/*N*/ 		{
/*N*/ 			rOut.WriteByteString( pChartData->SomeData1() );
/*N*/ 			rOut.WriteByteString( pChartData->SomeData2() );
/*N*/ 			rOut.WriteByteString( pChartData->SomeData3() );
/*N*/ 			rOut.WriteByteString( pChartData->SomeData4() );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			String aEmpty;
/*?*/
/*?*/ 			rOut.WriteByteString( aEmpty );
/*?*/ 			rOut.WriteByteString( aEmpty );
/*?*/ 			rOut.WriteByteString( aEmpty );
/*?*/ 			rOut.WriteByteString( aEmpty );
/*N*/ 		}
/*N*/
/*N*/ 	if (nMoreData >= 3) rOut << fSpotIntensity;
/*N*/
/*N*/ 	if (nMoreData >= 4)
/*N*/ 	{
/*N*/ 		BOOL bDummy = TRUE;   // war frueher bIsRegression
/*N*/
/*N*/ 		rOut << bShowAverage;
/*N*/ 		rOut << (INT16)eErrorKind;
/*N*/ 		rOut << bDummy;
/*N*/ 		rOut << (INT16)eIndicate;
/*N*/ 		rOut << fIndicatePercent;
/*N*/ 		rOut << fIndicateBigError;
/*N*/ 		rOut << fIndicatePlus;
/*N*/ 		rOut << fIndicateMinus;
/*N*/ 	}
/*N*/
/*N*/ 	if (nMoreData >= 5) rOut << (INT16) eRegression;
/*N*/
/*N*/ 	if (nMoreData >= 6)
/*N*/ 	{
/*N*/ 		rOut << (INT32) nSplineDepth;
/*N*/ 		rOut << (INT32) nGranularity;
/*N*/ 	}
/*N*/
/*N*/ 	if (nMoreData >= 7)
/*N*/ 	{
/*N*/ 		rOut << bLegendVisible;
/*N*/ 	}
/*N*/
/*N*/ 	if (nMoreData >= 8)
/*N*/ 	{
/*N*/ 		nCount = (short)aRegressAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 			aRegressAttrList.GetObject(i)->Store(rOut);
/*N*/
/*N*/ 		nCount = (short)aAverageAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 			aAverageAttrList.GetObject(i)->Store(rOut);
/*N*/
/*N*/ 		nCount = (short)aErrorAttrList.Count();
/*N*/ 		rOut << (INT16)nCount;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 			aErrorAttrList.GetObject(i)->Store(rOut);
/*N*/
/*N*/ 	}
/*N*/
/*N*/ 	if (nMoreData >= 9) rOut << fAmbientIntensity;
/*N*/ 	if (nMoreData >= 10) rOut << bTextScalable;
/*N*/ 	if (nMoreData >= 11) rOut << aInitialSize;
/*N*/ 	if (nMoreData >= 12) // FG: Zusaetze wegen des Umruchs von Achsentext und Legendentext 20.02.97
/*N*/ 	{
/*N*/ 		// FG: Vielleicht etwas viel Speicher fuer BOOL, aber es macht nur 8 Byte pro Chart
/*N*/ 		rOut << (INT16) bFormatXAxisTextInMultipleLinesIfNecessary;
/*N*/ 		rOut << (INT16) bFormatYAxisTextInMultipleLinesIfNecessary;
/*N*/ 		rOut << (INT16) bFormatZAxisTextInMultipleLinesIfNecessary;
/*N*/ 		rOut << (INT16) bFormatLegendTextInMultipleLinesIfNecessary;
/*N*/ 	}
/*N*/ 	if (nMoreData >= 13) // FG: 9.3.1997 Zusaetze wegen einer freien Positionierung und einem freien
/*N*/ 	{                    //     Resize von Gruppenobjekten im Chart.
/*N*/ 		rOut << (INT16) nXAxisTextMaximumNumberOfLines;
/*N*/ 		rOut << (INT16) nYAxisTextMaximumNumberOfLines;
/*N*/ 		rOut << (INT16) nZAxisTextMaximumNumberOfLines;
/*N*/ 		rOut << (INT32) nWidthOfFirstXAxisText;
/*N*/ 		rOut << (INT32) nWidthOfLastXAxisText;
/*N*/ 		rOut << aTitleTopCenter;
/*N*/ 		rOut << aSubTitleTopCenter;
/*N*/ 		rOut << aDiagramRectangle;
/*N*/ 		rOut << aLegendTopLeft;
/*N*/ 		rOut << aTitleXAxisPosition;
/*N*/ 		rOut << aTitleYAxisPosition;
/*N*/ 		rOut << aTitleZAxisPosition;
/*N*/ 		rOut << (INT16) bUseRelativePositionsForChartGroups;
/*N*/ 		rOut << (INT16) bAdjustMarginsForLegend;
/*N*/ 		rOut << (INT16) bAdjustMarginsForMainTitle;
/*N*/ 		rOut << (INT16) bAdjustMarginsForSubTitle;
/*N*/ 		rOut << (INT16) bAdjustMarginsForXAxisTitle;
/*N*/ 		rOut << (INT16) bAdjustMarginsForYAxisTitle;
/*N*/ 		rOut << (INT16) bAdjustMarginsForZAxisTitle;
/*N*/ 	}
/*N*/ 	if (nMoreData >= 14)
/*N*/ 	{
/*N*/ 		rOut << aSpotColor;     // FG: Von DL, um sich die Farbeinstellungen der 3d-Lichter
/*N*/ 		rOut << aAmbientColor;  //     bei 3d-Charts merken zu koennen.
/*N*/
/*N*/ 		BOOL bTmp=bDiagramHasBeenMovedOrResized; //neu -> alt: dann immer rearrange
/*N*/ 		if(IsReal3D() && IsPieChart())
/*N*/ 			bTmp=FALSE;
/*N*/
/*N*/ 		rOut << bTmp; // FG: 12.3.97, Falls das Chart gespeichert wird
/*N*/ 		rOut << bMainTitleHasBeenMoved;        //     soll es immer noch die relativen Positionen
/*N*/ 		rOut << bSubTitleHasBeenMoved;         //     der Objektgruppen beachten, falls eine
/*N*/ 		rOut << bLegendHasBeenMoved;           //     dieser Gruppen bewegt worden ist.
/*N*/ 		rOut << bXAxisTitleHasBeenMoved;
/*N*/ 		rOut << bYAxisTitleHasBeenMoved;
/*N*/ 		rOut << bZAxisTitleHasBeenMoved;
/*N*/ 	}
/*N*/ 	if (nMoreData >= 15)  // FG: 14.3.97 Damit bei einem angefassten Text die Position sinnvoll gesetzt werden
/*N*/ 	{                     //     auch nach einer Schriftgroessenaenderung muss man sich merken wie die Ausrichtung war
/*N*/ 		rOut << (INT16) eAdjustXAxesTitle;
/*N*/ 		rOut << (INT16) eAdjustYAxesTitle;
/*N*/ 		rOut << (INT16) eAdjustZAxesTitle;
/*N*/ 	}
/*N*/ 	if (nMoreData >= 16) // NumberFormater waren bisher nicht persistent
/*N*/ 	{
/*N*/ 		pNumFormatter->Save( rOut );
/*N*/ 		pNumFormatter->Save( rOut );
/*N*/ 		pNumFormatter->Save( rOut );
/*N*/ 	}
/*N*/
/*N*/ 	rOut << (INT16) nBarPercentWidth; //#50116# ist ein long, aber Range von 0..100
/*N*/ 	rOut << (INT32) m_nDefaultColorSet;//#50149#
/*N*/
/*N*/ 	rOut << (INT32) nNumLinesInColChart;//#50212#
/*N*/
/*N*/  	// #74536# BM use these three former -1 dummies for
/*N*/  	// numberformat last set by CALC
/*N*/  	rOut << (INT32) nXLastNumFmt;
/*N*/  	rOut << (INT32) nYLastNumFmt;
/*N*/  	rOut << (INT32) nBLastNumFmt;
/*N*/
/*N*/ 	rOut << (INT32)(-1);		// dummy
/*N*/
/*N*/ 	//IOVer 11
/*N*/ 	pStockLineAttr->Store(rOut);
/*N*/ 	pStockLossAttr->Store(rOut);
/*N*/ 	pStockPlusAttr->Store(rOut);
/*N*/
/*N*/
/*N*/ 	//IOVer12:
/*N*/ 	//nAxisId ist die zu ladene/speichernde Achse,
/*N*/ 	//festgelegt sind in V12 nur 1=X, 2=Y,3=Z und 4=2.Y-Achse (B)
/*N*/ 	//neu: 5 = 2. X-Achse
/*N*/ 	//-1 bedeutet KEINE WEITEREN ACHSEN!
/*N*/ 	//Es MUSS keine Achse gestreamt werden (siehe Lade-Routine)
/*N*/ 	//Die Reihenfolge ist eigentlich auch egal, jedoch gilt beim Laden:
/*N*/   //erst die primaere und dann die sekundaere (X, bzw. evtl auch Y) Achse
/*N*/   //(um defaults aus der primaeren Achse heraus zu erzeugen)
/*N*/
/*N*/ 	INT32 nAxisId=CHART_AXIS_PRIMARY_X;
/*N*/ 	rOut << nAxisId;
/*N*/ 	GetAttr(CHOBJID_DIAGRAM_X_AXIS).Store(rOut);
/*N*/
/*N*/ 	nAxisId=CHART_AXIS_PRIMARY_Y;
/*N*/ 	rOut << nAxisId;
/*N*/ 	GetAttr(CHOBJID_DIAGRAM_Y_AXIS).Store(rOut);
/*N*/
/*N*/ 	nAxisId=CHART_AXIS_PRIMARY_Z;
/*N*/ 	rOut << nAxisId;
/*N*/ 	GetAttr(CHOBJID_DIAGRAM_Z_AXIS).Store(rOut);
/*N*/
/*N*/ 	nAxisId=CHART_AXIS_SECONDARY_Y;
/*N*/ 	rOut << nAxisId;
/*N*/ 	GetAttr(CHOBJID_DIAGRAM_B_AXIS).Store(rOut);
/*N*/
/*N*/ 	nAxisId=CHART_AXIS_SECONDARY_X;
/*N*/ 	rOut << nAxisId;
/*N*/ 	GetAttr(CHOBJID_DIAGRAM_A_AXIS).Store(rOut);
/*N*/
/*N*/ 	//Elementar wichtig! Diesen Wert zu streamen ist absolutes Minimum
/*N*/ 	//die obigen derzeit 4 Achsen kann man dagegen getrost reduzieren oder aufstocken
/*N*/   //ohne dass die Abwaertskompatiblitaet leidet! Genauso lassen sich beliebige
/*N*/   //Achsen mit einer Id>4 dazwischen haengen. In alten Versionen wird bei
/*N*/ 	//unbekannter id einfach ein Dummy geladen (und weggeschmissen)
/*N*/ 	nAxisId=-1;
/*N*/ 	rOut << nAxisId;
/*N*/ 	//Ende IOVersion 12
/*N*/
/*N*/ 	nTmp = aBarY1.GetGap();
/*N*/ 	rOut << nTmp;
/*N*/ 	nTmp = aBarY1.GetOverlap();
/*N*/ 	rOut << nTmp;
/*N*/ 	nTmp = aBarY2.GetGap();
/*N*/ 	rOut << nTmp;
/*N*/ 	nTmp = aBarY2.GetOverlap();
/*N*/ 	rOut << nTmp;
/*N*/
/*N*/
/*N*/  	// IOVersion 15
/*N*/ 	rOut << bDiagramHasBeenMovedOrResized; // FG: 12.3.97, Falls das Chart gespeichert wird
/*N*/ }

/*************************************************************************
|*
|* Chart-Attribute laden
|*
\************************************************************************/

/*N*/ void ChartModel::LoadAttributes(SvStream& rIn)
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/ 	ByteString aBStr( aMainTitle, RTL_TEXTENCODING_ASCII_US );
/*N*/ 	CHART_TRACE1( "ChartModel::LoadAttributes (%s)", aBStr.GetBuffer() );
/*N*/ #endif
/*N*/
/*N*/     // aInfo is a member of SdrModel
/*N*/     // the SdrModel was read before this, so the value is set
/*N*/     rtl_TextEncoding eSysSet = ::GetSOLoadTextEncoding( aInfo.eLastReadCharSet, rIn.GetVersion());
/*N*/ 	rIn.SetStreamCharSet( eSysSet );
/*N*/     // Note: The CharSet is stored in the MemChart and later set again at the stream !
/*N*/
/*N*/ 	SchIOCompat aIO(rIn, STREAM_READ);
/*N*/
/*N*/ 	short       i, nCount;
/*N*/ 	INT16       nInt16, nCharSet;
/*N*/ 	INT32       nInt32;
/*N*/ 	SfxItemSet* pAttr;
/*N*/ 	BOOL        bNoMore = FALSE;
/*N*/ 	int			nVersion = aIO.GetVersion();
/*N*/
/*N*/ 	rIn >> aLightVec->X ();
/*N*/ 	rIn >> aLightVec->Y ();
/*N*/ 	rIn >> aLightVec->Z ();
/*N*/
/*N*/ 	if (nVersion >= 3)
/*N*/ 	{
/*N*/ 		rIn >> nInt16;
/*N*/
/*N*/ 		// fuer den fall, dass initial das diagrammfenster generiert wurde
/*N*/ 		delete pChartData;
/*N*/
/*N*/         // Note: The CharSet is stored in the MemChart and set there at the stream !
/*N*/ 		switch ((ChartDataId)nInt16)
/*N*/ 		{
/*N*/ 			case CHDATAID_MEMCHART_PLUS :
/*?*/ 				pChartData = new SchMemChart (CHDATAID_MEMCHART_PLUS);
/*?*/ 				rIn >> *(SchMemChart*)pChartData;
/*?*/ 				pChartData->IncreaseRefCount();
/*?*/ 				break;
/*?*/
/*?*/ 			case CHDATAID_DYNCHART:
/*N*/ 			case CHDATAID_MEMCHART:
/*N*/ 				pChartData = new SchMemChart (CHDATAID_MEMCHART);
/*N*/ 				rIn >> *(SchMemChart*)pChartData;
/*N*/ 				pChartData->IncreaseRefCount();
/*N*/ 				break;
/*N*/
/*N*/ 			default :
/*N*/ 				bNoMore = TRUE;
/*N*/ 		}
/*N*/
/*N*/ 		if (nVersion >= 5) rIn >> bIsCopied;
/*N*/
/*N*/ 		if (nVersion >= 8) rIn >> fMinData;
/*N*/ 		else if (pChartData)
/*N*/ 		{
/*?*/ 			long nColCnt = GetColCount();
/*?*/ 			long nRowCnt = GetRowCount();
/*?*/
/*?*/ 			for (short nCol = 0; nCol < nColCnt; nCol++)
/*?*/ 				for (short nRow = 0; nRow < nRowCnt; nRow++)
/*?*/ 				{
/*?*/ 					double fData = GetData(nCol, nRow);
/*?*/
/*?*/ 					if ((nCol == 0) && (nRow == 0))
/*?*/ 					{
/*?*/ 						pChartYAxis->SetMin(fData);
/*?*/ 						pChartYAxis->SetMax(fData);
/*?*/ 					}
/*?*/ 					if (fData < pChartYAxis->GetMin())
/*?*/ 						pChartYAxis->SetMin(fData);
/*?*/ 					if (fData > pChartYAxis->GetMax())
/*?*/ 						pChartYAxis->SetMax(fData);
/*?*/ 				}
/*?*/
/*?*/ 			bNoMore = TRUE;
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	rIn >> nInt16;
/*N*/ 	if (nVersion < 1)
/*N*/ 	{
/*N*/ 		switch ((ChartStyleV0)nInt16)
/*N*/ 		{
/*?*/ 			case CHART2D_LINE:
/*?*/ 				eChartStyle = CHSTYLE_2D_LINE;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART2D_STACKEDLINE:
/*?*/ 				eChartStyle = CHSTYLE_2D_STACKEDLINE;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART2D_BAR:
/*?*/ 				eChartStyle = CHSTYLE_2D_COLUMN;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART2D_STACKEDBAR:
/*?*/ 				eChartStyle = CHSTYLE_2D_STACKEDCOLUMN;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART2D_COLUMN:
/*?*/ 				eChartStyle = CHSTYLE_2D_BAR;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART2D_STACKEDCOLUMN:
/*?*/ 				eChartStyle = CHSTYLE_2D_STACKEDBAR;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART2D_AREA:
/*?*/ 				eChartStyle = CHSTYLE_2D_AREA;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART2D_STACKEDAREA:
/*?*/ 				eChartStyle = CHSTYLE_2D_STACKEDAREA;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART2D_PIE:
/*?*/ 				eChartStyle = CHSTYLE_2D_PIE;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART3D_STRIPE:
/*?*/ 				eChartStyle = CHSTYLE_3D_STRIPE;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART3D_BAR:
/*?*/ 				eChartStyle = CHSTYLE_3D_COLUMN;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART3D_FLATBAR:
/*?*/ 				eChartStyle = CHSTYLE_3D_FLATCOLUMN;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART3D_STACKEDFLATBAR:
/*?*/ 				eChartStyle = CHSTYLE_3D_STACKEDFLATCOLUMN;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART3D_AREA:
/*?*/ 				eChartStyle = CHSTYLE_3D_AREA;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART3D_STACKEDAREA:
/*?*/ 				eChartStyle = CHSTYLE_3D_STACKEDAREA;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART3D_SURFACE:
/*?*/ 				eChartStyle = CHSTYLE_3D_SURFACE;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHART3D_PIE:
/*?*/ 				eChartStyle = CHSTYLE_3D_PIE;
/*?*/ 				break;
/*?*/
/*?*/ 			default :
/*?*/ 				eChartStyle = CHSTYLE_2D_COLUMN;
/*?*/ 				break;
/*?*/ 		}
/*?*/
/*?*/ 		bNoMore = TRUE;
/*?*/ 	}
/*N*/ 	else
/*N*/ 		eChartStyle = (SvxChartStyle)nInt16;
/*N*/
/*N*/ 	for (i = 0; i < LINE_POINT_COUNT; i++)
/*N*/ 	{
/*N*/ 		rIn >> nInt16; eChartLinePoints[i] = nInt16;
/*N*/ 	}
/*N*/
/*N*/ 	// Das ehemalige Member aRowColors[] wird eingelesen.
/*N*/ 	Color aDummyColor;
/*N*/ 	for (i = 0; i < ROW_COLOR_COUNT; i++)
/*N*/ 		rIn >> aDummyColor;
/*N*/
/*N*/ 	long nOverlap;
/*N*/ 	rIn >> nInt32; //nGapWidth = (long)nInt32;
/*N*/ 	rIn >> nInt32; nOverlap = (long)nInt32;
/*N*/ 	rIn >> nInt32;
/*N*/ 	DBG_ASSERT( nInt32 == 100, "LoadAttributes: nMarkLen's value differs from 100 in this stream" );
/*N*/
/*N*/ 	rIn >> aChartRect;
/*N*/ 	rIn >> nInt32; nPieHeight = (long)nInt32;
/*N*/
/*N*/ 	if (nVersion >= 6)
/*N*/ 	{
/*N*/ 		rIn >> nInt16; nPieSegCount = (short)nInt16;
/*N*/ 		pPieSegOfs = new long[nPieSegCount];
/*N*/
/*N*/ 		BOOL bNullify = (nVersion < 17) && eChartStyle == CHSTYLE_2D_PIE;
/*N*/
/*N*/ 		for( i = 0; i < nPieSegCount; i++ )
/*N*/ 		{
/*N*/ 			rIn >> nInt32;
/*N*/ 			pPieSegOfs[ i ] = bNullify? 0: (long)nInt32;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	rIn >> nInt16; nXAngle = (short)nInt16;
/*N*/ 	rIn >> nInt16; nYAngle = (short)nInt16;
/*N*/ 	rIn >> nInt16; nZAngle = (short)nInt16;
/*N*/
/*N*/ 	rIn >> nCharSet;

    /**************************************************************************
    * So machts der Writer, und so muessen es alle machen:
    * Bug 9714: Der CharSet an den Fonts muss geaendert werden, wenn
    * es der globale CharSet ist (MT)
    **************************************************************************/
/*N*/ 	SfxItemPool& rPool = GetItemPool();
/*N*/ 	USHORT nMaxItems = rPool.GetItemCount(EE_CHAR_FONTINFO);
/*N*/ 	SvxFontItem* pItem;
/*N*/
/*N*/ 	for (USHORT n = 0; n < nMaxItems; ++n)
/*N*/ 	{
/*N*/ 		pItem = (SvxFontItem*) rPool.GetItem(EE_CHAR_FONTINFO, n);
/*N*/ 		if (pItem && pItem->GetCharSet() == nCharSet)
/*N*/ 		{
/*N*/ 			pItem->GetCharSet() = eSysSet;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/
/*N*/ 	BOOL bShowXAxis;
/*N*/ 	BOOL bShowXDescr;
/*N*/ 	BOOL bShowYAxis;
/*N*/ 	BOOL bShowYDescr;
/*N*/ 	BOOL bShowZAxis;
/*N*/ 	BOOL bShowZDescr;
/*N*/
/*N*/ 	rIn >> bShowMainTitle;
/*N*/ 	rIn.ReadByteString( aMainTitle );
/*N*/ 	rIn >> bShowSubTitle;
/*N*/ 	rIn.ReadByteString( aSubTitle );
/*N*/ 	rIn >> bShowXAxisTitle;
/*N*/ 	rIn.ReadByteString( aXAxisTitle );
/*N*/ 	rIn >> bShowYAxisTitle;
/*N*/ 	rIn.ReadByteString( aYAxisTitle );
/*N*/ 	rIn >> bShowZAxisTitle;
/*N*/ 	rIn.ReadByteString( aZAxisTitle );
/*N*/ 	rIn >> bShowXAxis;
/*N*/ 	rIn >> bShowXGridMain;
/*N*/ 	rIn >> bShowXGridHelp;
/*N*/ 	rIn >> bShowXDescr;
/*N*/ 	rIn >> bShowYAxis;
/*N*/ 	rIn >> bShowYGridMain;
/*N*/ 	rIn >> bShowYGridHelp;
/*N*/ 	rIn >> bShowYDescr;
/*N*/ 	rIn >> bShowZAxis;
/*N*/ 	rIn >> bShowZGridMain;
/*N*/ 	rIn >> bShowZGridHelp;
/*N*/ 	rIn >> bShowZDescr;
/*N*/
    /**************************************************************************
    * Nun werden die Attribut-ItemSets geladen
    * Die etwas umstaendliche Art und Weise ist notwendig, da fruehere Charts
    * nicht alle Items weggeschrieben haben. Daher wird in ein leeres ItemSet
    * gestreamt und dieses in das Default-ItemSet geputtet (damit keine
    * Pool-Defaults zum Tragen kommen).
    **************************************************************************/

/*N*/ 	SfxItemSet aTitleAttr(*pItemPool, nTitleWhichPairs);
/*N*/ 	aTitleAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pTitleAttr, aTitleAttr );
/*N*/
/*N*/ 	SfxItemSet aMainTitleAttr(*pItemPool, nTitleWhichPairs);
/*N*/ 	aMainTitleAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pMainTitleAttr, aMainTitleAttr );
/*N*/
/*N*/ 	SfxItemSet aSubTitleAttr(*pItemPool, nTitleWhichPairs);
/*N*/ 	aSubTitleAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pSubTitleAttr, aSubTitleAttr );
/*N*/
/*N*/ 	SfxItemSet aXAxisTitleAttr(*pItemPool, nTitleWhichPairs);
/*N*/ 	aXAxisTitleAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pXAxisTitleAttr, aXAxisTitleAttr );
/*N*/
/*N*/ 	SfxItemSet aYAxisTitleAttr(*pItemPool, nTitleWhichPairs);
/*N*/ 	aYAxisTitleAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pYAxisTitleAttr, aYAxisTitleAttr );
/*N*/
/*N*/ 	SfxItemSet aZAxisTitleAttr(*pItemPool, nTitleWhichPairs);
/*N*/ 	aZAxisTitleAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pZAxisTitleAttr, aZAxisTitleAttr );
/*N*/
/*N*/ 	SfxItemSet aAxisAttr(*pItemPool, nAllAxisWhichPairs);
/*N*/ 	aAxisAttr.Load(rIn);
/*N*/ 	pAxisAttr->Put(aAxisAttr);
/*N*/
/*N*/ CHART_TRACE( "LoadAttributes: Debugging load of old axis attr BEFORE conversion" );
/*N*/ 	SfxItemSet aXAxisAttr(*pItemPool, nCompatAxisWhichPairs);//nXAxisWhich....
/*N*/ 	aXAxisAttr.Load(rIn);
/*N*/ DBG_ITEMS(aXAxisAttr,(ChartModel*)this);
/*N*/ 	SfxItemSet aYAxisAttr(*pItemPool, nCompatAxisWhichPairs);
/*N*/ 	aYAxisAttr.Load(rIn);
/*N*/ DBG_ITEMS(aYAxisAttr,(ChartModel*)this);
/*N*/ 	SfxItemSet aZAxisAttr(*pItemPool, nCompatAxisWhichPairs);
/*N*/ 	aZAxisAttr.Load(rIn);
/*N*/ DBG_ITEMS(aZAxisAttr,(ChartModel*)this);
/*N*/
/*N*/   if(nVersion < 12) //sonst wirds eh uebergebuegelt:
/*N*/ 	{
/*N*/ 		//konvertieren:
/*N*/ 		AxisAttrOld2New(aXAxisAttr,TRUE,CHOBJID_DIAGRAM_X_AXIS);
/*N*/ 		AxisAttrOld2New(aYAxisAttr,TRUE,CHOBJID_DIAGRAM_Y_AXIS);
/*N*/ 		AxisAttrOld2New(aZAxisAttr,TRUE,CHOBJID_DIAGRAM_Z_AXIS);
/*N*/ 		pChartXAxis->SetAttributes(aXAxisAttr);
/*N*/ 		pChartYAxis->SetAttributes(aYAxisAttr);
/*N*/ 		pChartZAxis->SetAttributes(aZAxisAttr);
/*N*/ CHART_TRACE( "LoadAttributes: Debugging AFTER conversion" );
/*N*/ DBG_ITEMS(aXAxisAttr,(ChartModel*)this);
/*N*/ DBG_ITEMS(aYAxisAttr,(ChartModel*)this);
/*N*/ DBG_ITEMS(aZAxisAttr,(ChartModel*)this);
/*N*/ 	}
/*N*/
/*N*/ 	SfxItemSet aGridAttr(*pItemPool, nGridWhichPairs);
/*N*/ 	aGridAttr.Load(rIn);
/*N*/ 	pGridAttr->Put(aGridAttr);
/*N*/
/*N*/ 	SfxItemSet aXGridMainAttr(*pItemPool, nGridWhichPairs);
/*N*/ 	aXGridMainAttr.Load(rIn);
/*N*/ 	pXGridMainAttr->Put(aXGridMainAttr);
/*N*/
/*N*/ 	SfxItemSet aYGridMainAttr(*pItemPool, nGridWhichPairs);
/*N*/ 	aYGridMainAttr.Load(rIn);
/*N*/ 	pYGridMainAttr->Put(aYGridMainAttr);
/*N*/
/*N*/ 	SfxItemSet aZGridMainAttr(*pItemPool, nGridWhichPairs);
/*N*/ 	aZGridMainAttr.Load(rIn);
/*N*/ 	pZGridMainAttr->Put(aZGridMainAttr);
/*N*/
/*N*/ 	SfxItemSet aXGridHelpAttr(*pItemPool, nGridWhichPairs);
/*N*/ 	aXGridHelpAttr.Load(rIn);
/*N*/ 	pXGridHelpAttr->Put(aXGridHelpAttr);
/*N*/
/*N*/ 	SfxItemSet aYGridHelpAttr(*pItemPool, nGridWhichPairs);
/*N*/ 	aYGridHelpAttr.Load(rIn);
/*N*/ 	pYGridHelpAttr->Put(aYGridHelpAttr);
/*N*/
/*N*/ 	SfxItemSet aZGridHelpAttr(*pItemPool, nGridWhichPairs);
/*N*/ 	aZGridHelpAttr.Load(rIn);
/*N*/ 	pZGridHelpAttr->Put(aZGridHelpAttr);
/*N*/
/*N*/ 	SfxItemSet aDiagramAreaAttr(*pItemPool, nDiagramAreaWhichPairs);
/*N*/ 	aDiagramAreaAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pDiagramAreaAttr, aDiagramAreaAttr );
/*N*/
/*N*/ 	SfxItemSet aDiagramWallAttr(*pItemPool, nDiagramAreaWhichPairs);
/*N*/ 	aDiagramWallAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pDiagramWallAttr, aDiagramWallAttr );
/*N*/
/*N*/ 	SfxItemSet aDiagramFloorAttr(*pItemPool, nDiagramAreaWhichPairs);
/*N*/ 	aDiagramFloorAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pDiagramFloorAttr, aDiagramFloorAttr );
/*N*/
/*N*/ 	SfxItemSet aLegendAttr(*pItemPool, nLegendWhichPairs);
/*N*/ 	aLegendAttr.Load(rIn);
/*N*/ 	PutItemSetWithNameCreation( *pLegendAttr, aLegendAttr );
/*N*/
/*N*/ 	if (nVersion < 2)
/*N*/ 	{
/*?*/ 		pTitleAttr->Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/ 		pMainTitleAttr->Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/ 		pSubTitleAttr->Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/ 		pXAxisTitleAttr->Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/ 		pYAxisTitleAttr->Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/ 		pZAxisTitleAttr->Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/ 		pAxisAttr->Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/
/*?*/       //Achsen auch fuer V12+ OK, (ToDo: Hier ist recht viel Overhead! )
/*?*/ 		aXAxisAttr.ClearItem();
/*?*/ 		aYAxisAttr.ClearItem();
/*?*/ 		aZAxisAttr.ClearItem();
/*?*/ 		aXAxisAttr.Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/ 		aXAxisAttr.Put(SvxChartTextOrderItem(CHTXTORDER_SIDEBYSIDE));
/*?*/ 		aYAxisAttr.Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/ 		aYAxisAttr.Put(SvxChartTextOrderItem(CHTXTORDER_SIDEBYSIDE));
/*?*/ 		aZAxisAttr.Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*?*/ 		pChartXAxis->SetAttributes(aXAxisAttr);
/*?*/ 		pChartYAxis->SetAttributes(aYAxisAttr);
/*?*/ 		pChartZAxis->SetAttributes(aZAxisAttr);
/*?*/
/*?*/ 		bNoMore = TRUE;
/*?*/ 	}
/*N*/
/*N*/ 	if (nVersion < 7)
/*N*/ 	{
/*?*/ 		GetAttr(CHOBJID_DIAGRAM_Y_AXIS).Put(SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN, TRUE));
/*?*/ 		GetAttr(CHOBJID_DIAGRAM_Y_AXIS).Put(SvxDoubleItem(0.0, SCHATTR_AXIS_ORIGIN));
/*?*/
/*?*/ 		bNoMore = TRUE;
/*N*/ 	}
/*N*/
/*N*/ 	rIn >> nInt16; nCount = (short)nInt16;
/*N*/ 	for (i = 0; i < nCount; i++)
/*N*/ 	{
/*N*/ 		pAttr = new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/
/*N*/ 		pAttr->Load(rIn);
/*N*/ 		aDataRowAttrList.Insert(pAttr, LIST_APPEND);
/*N*/
/*N*/ //		Debug-Code
/*N*/ //
/*N*/ //  		SfxItemState sfx_test_state = aDataRowAttrList.GetObject(i)->GetItemState(XATTR_FILLSTYLE);
/*N*/ //  		if (sfx_test_state >= SFX_ITEM_AVAILABLE)
/*N*/ //  		{
/*N*/ //  			XFillStyleItem& rTest_set = (XFillStyleItem&) aDataRowAttrList.GetObject(i)->Get(XATTR_FILLSTYLE);
/*N*/ //  		}
/*N*/ //  		sfx_test_state = aDataRowAttrList.GetObject(i)->GetItemState(XATTR_FILLCOLOR);
/*N*/ //  		if (sfx_test_state >= SFX_ITEM_AVAILABLE)
/*N*/ //  		{
/*N*/ //  			XColorItem& rTest_set = (XColorItem&) aDataRowAttrList.GetObject(i)->Get(XATTR_FILLCOLOR);
/*N*/ //  		}
/*N*/ 	}
/*N*/
/*N*/ 	rIn >> nInt16; nCount = (short)nInt16;
/*N*/ 	for (i = 0; i < nCount; i++)
/*N*/ 	{
/*N*/ 		pAttr = new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/
/*N*/ 		pAttr->Load(rIn);
/*N*/ 		if (pAttr->Count() == 0)
/*N*/ 		{
/*N*/ 			delete pAttr;
/*N*/ 			pAttr = NULL;
/*N*/ 		}
/*N*/ 		aDataPointAttrList.Insert(pAttr, LIST_APPEND);
/*N*/ 	}
/*N*/
/*N*/ 	if (nVersion < 4)
/*N*/ 	{
/*?*/ 		for (i = 0; i < nCount; i++)
/*?*/ 		{
//			SfxItemSet* pDataPointAttr =
//				new SfxItemSet(*pItemPool, nRowWhichPairs);
/*?*/ 			aSwitchDataPointAttrList.Insert(NULL, LIST_APPEND);
/*?*/ 		}
/*?*/
/*?*/ 		bNoMore = TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rIn >> nInt16; nCount = (short)nInt16;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			pAttr = new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/
/*N*/ 			pAttr->Load(rIn);
/*N*/ 			if (pAttr->Count() == 0)
/*N*/ 			{
/*N*/ 				delete pAttr;
/*N*/ 				pAttr = NULL;
/*N*/ 			}
/*N*/ 			aSwitchDataPointAttrList.Insert(pAttr, LIST_APPEND);
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	rIn >> nInt16; eDataDescr = (SvxChartDataDescr)nInt16;
/*N*/ 	rIn >> bShowSym;
/*N*/ 	rIn >> bSwitchData;
/*N*/
/*N*/ 	if (nVersion < 1)
/*N*/ 	{
/*?*/ 		BOOL bDataPercent;
/*?*/ 		rIn >> bDataPercent;
/*?*/
/*?*/ 		switch (eChartStyle)
/*?*/ 		{
/*?*/ 			case CHSTYLE_2D_LINE:
/*?*/ 				eChartStyle = CHSTYLE_2D_PERCENTLINE;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHSTYLE_2D_COLUMN:
/*?*/ 				eChartStyle = CHSTYLE_2D_PERCENTCOLUMN;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHSTYLE_2D_BAR:
/*?*/ 				eChartStyle = CHSTYLE_2D_PERCENTBAR;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHSTYLE_2D_AREA:
/*?*/ 				eChartStyle = CHSTYLE_2D_PERCENTAREA;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHSTYLE_3D_FLATCOLUMN:
/*?*/ 				eChartStyle = CHSTYLE_3D_PERCENTFLATCOLUMN;
/*?*/ 				break;
/*?*/
/*?*/ 			case CHSTYLE_3D_AREA:
/*?*/ 				eChartStyle = CHSTYLE_3D_PERCENTAREA;
/*?*/ 				break;
/*?*/ 		}
/*?*/
/*?*/ 		bNoMore = TRUE;
/*N*/ 	}
/*N*/
/*N*/ 	UINT32 nTmp,nDescrFormat,nDescrPercentFormat;
/*N*/ 	rIn >> nTmp;//nValFo rmat;
/*N*/ 	SetNumFmt(CHOBJID_DIAGRAM_Y_AXIS,nTmp,FALSE);
/*N*/ 	rIn >> nTmp;//nPerce ntValFormat;
/*N*/ 	SetNumFmt(CHOBJID_DIAGRAM_Y_AXIS,nTmp,TRUE);
/*N*/ 	rIn >> nDescrFormat;
/*N*/ 	SetNumFmt(CHOBJID_DIAGRAM_X_AXIS,nDescrFormat,FALSE);
/*N*/ 	rIn >> nDescrPercentFormat;
/*N*/ 	SetNumFmt(CHOBJID_DIAGRAM_X_AXIS,nDescrPercentFormat,TRUE);
/*N*/
/*N*/ 	// Achtung : Gravierende Aenderungen an der Datei gegenueber der Auslieferungsversion
/*N*/ 	// es werden mehr Daten geschrieben : Die alte Version muss (!) das aber auch verkraften
/*N*/   // Die Reihenfolge muss (!!!!) eingehalten werden
/*N*/ 	pChartYAxis->LoadMemberCompat(rIn);
/*N*/ 	pChartXAxis->LoadMemberCompat(rIn);
/*N*/ 	pChartZAxis->LoadMemberCompat(rIn);
/*N*/
/*N*/ 	rIn >> fMaxData;
/*N*/
/*N*/ 	if (!bNoMore)
/*N*/ 	{
/*N*/ 		UINT32 nMoreData = 0;
/*N*/ 		rIn >> nMoreData;
/*N*/
/*N*/ 		// nMoreData soll als Weiche fuer zukuenftige Versionen dienen,
/*N*/ 		// insbesondere fuer den Fall, dass weitere Daten geschrieben werden
/*N*/ 		// muessen. Die Pools duerfen ab dem 19.01.1996 auf gar keinen Fall
/*N*/ 		// mehr modifiziert oder aufgebohrt werden !!!!
/*N*/ 		// nMoreData = 1     (Version vom 19.01.1996)
/*N*/ 		// nMoreData = 2     (Version vom 13.03.1996)
/*N*/ 		// nMoreData = 3     (Version vom 1.04.1996)
/*N*/ 		// nMoreData = 4     (Version vom 13.05.1996)
/*N*/ 		// nMoreData = 5     (Version vom 20.05.1996)
/*N*/ 		// nMoreData = 6     (Version vom 20.06.1996)
/*N*/ 		// nMoreData = 7     (Version vom 02.07.1996)
/*N*/ 		// nMoreData = 8     (Version vom 09.07.1996)
/*N*/ 		// nMoreData = 9     (Version vom 11.07.1996)
/*N*/ 		// ...
/*N*/ 		// nMoreData = 12    (Version vom 20.02.1997) FG (Vobis-Version 4.0)
/*N*/ 		// nMoreData = 13    (Version vom 9.3.1997) FG Cebit-Version
/*N*/ 		// .... siehe Text unten
/*N*/ 		// nMoreData = 15    (Version vom 14.3.1997)  FG V4.0 nach Cebit >= 358
/*N*/ 		if (nMoreData > 1)
/*N*/ 		{
/*N*/ 			ByteString aReadString;
/*N*/
/*N*/ 			rIn.ReadByteString( pChartData->SomeData1() );
/*N*/ 			rIn.ReadByteString( pChartData->SomeData2() );
/*N*/ 			rIn.ReadByteString( pChartData->SomeData3() );
/*N*/ 			rIn.ReadByteString( pChartData->SomeData4() );
/*N*/
/*N*/ 			if (nMoreData >= 3) rIn >> fSpotIntensity;
/*N*/ 			if (nMoreData <= 8) fAmbientIntensity = fSpotIntensity;
/*N*/
/*N*/ 			if (nMoreData >= 4)
/*N*/ 			{
/*N*/ 				BOOL bDummy;
/*N*/
/*N*/ 				rIn >> bShowAverage;
/*N*/ 				rIn >> nInt16; eErrorKind = (SvxChartKindError) nInt16;
/*N*/ 				rIn >> bDummy;
/*N*/ 				rIn >> nInt16; eIndicate = (SvxChartIndicate) nInt16;
/*N*/ 				rIn >> fIndicatePercent;
/*N*/ 				rIn >> fIndicateBigError;
/*N*/ 				rIn >> fIndicatePlus;
/*N*/ 				rIn >> fIndicateMinus;
/*N*/ 			}
/*N*/
/*N*/ 			if (nMoreData >= 5)
/*N*/ 			{
/*N*/ 				rIn >> nInt16; eRegression = (SvxChartRegress) nInt16;
/*N*/ 			}
/*N*/
/*N*/ 			if (nMoreData >= 6)
/*N*/ 			{
/*N*/ 				INT32 nInt32;
/*N*/ 				rIn >> nInt32; nSplineDepth = nInt32;
/*N*/ 				rIn >> nInt32; nGranularity = nInt32;
/*N*/ 			}
/*N*/
/*N*/ 			if (nMoreData >= 7)
/*N*/ 			{
/*N*/ 				rIn >> bLegendVisible;
/*N*/ 			}
/*N*/
/*N*/ 			if (nMoreData >= 8)
/*N*/ 			{
/*N*/ 				rIn >> nInt16; nCount = (short)nInt16;
/*N*/ 				for (i = 0; i < nCount; i++)
/*N*/ 				{
/*N*/ 					pAttr = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/
/*N*/ 					pAttr->Load(rIn);
/*N*/ 					aRegressAttrList.Insert(pAttr, LIST_APPEND);
/*N*/ 				}
/*N*/
/*N*/ 				rIn >> nInt16; nCount = (short)nInt16;
/*N*/ 				for (i = 0; i < nCount; i++)
/*N*/ 				{
/*N*/ 					pAttr = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/
/*N*/ 					pAttr->Load(rIn);
/*N*/ 					aAverageAttrList.Insert(pAttr, LIST_APPEND);
/*N*/ 				}
/*N*/
/*N*/ 				rIn >> nInt16; nCount = (short)nInt16;
/*N*/ 				for (i = 0; i < nCount; i++)
/*N*/ 				{
/*N*/ 					pAttr = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/
/*N*/ 					pAttr->Load(rIn);
/*N*/ 					aErrorAttrList.Insert(pAttr, LIST_APPEND);
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 			if (nMoreData >= 9) rIn >> fAmbientIntensity;
/*N*/ 			if (nMoreData >= 10) rIn >> bTextScalable;
/*N*/ 			if (nMoreData >= 11) rIn >> aInitialSize;
/*N*/ 			if (nMoreData >= 12)     //FG: 20.2.1997
/*N*/ 			{
/*N*/ 				// FG: Vielleicht etwas viel Speicher fuer BOOL, aber es macht nur 16 Byte pro Chart
/*N*/ 				rIn >> nInt16;  bFormatXAxisTextInMultipleLinesIfNecessary = (BOOL) nInt16;
/*N*/ 				rIn >> nInt16;  bFormatYAxisTextInMultipleLinesIfNecessary = (BOOL) nInt16;
/*N*/ 				rIn >> nInt16;  bFormatZAxisTextInMultipleLinesIfNecessary = (BOOL) nInt16;
/*N*/ 				rIn >> nInt16;  bFormatLegendTextInMultipleLinesIfNecessary = (BOOL) nInt16;
/*N*/ 			}
/*N*/ 			if (nMoreData >= 13) // FG: 9.3.1997 Zusaetze wegen einer freien Positionierung und einem freien
/*N*/ 			{                    //     Resize von Gruppenobjekten im Chart.
/*N*/ 				rIn >> nInt16;  nXAxisTextMaximumNumberOfLines = nInt16;
/*N*/ 				rIn >> nInt16;  nYAxisTextMaximumNumberOfLines = nInt16;
/*N*/ 				rIn >> nInt16;  nZAxisTextMaximumNumberOfLines = nInt16;
/*N*/ 				rIn >> nInt32; nWidthOfFirstXAxisText = nInt32;
/*N*/ 				rIn >> nInt32; nWidthOfLastXAxisText = nInt32;
/*N*/ 				rIn >> aTitleTopCenter;
/*N*/ 				rIn >> aSubTitleTopCenter;
/*N*/ 				rIn >> aDiagramRectangle;
/*N*/ 				rIn >> aLegendTopLeft;
/*N*/ 				rIn >> aTitleXAxisPosition; // FG: welcher Punkt da jeweils gemeint ist findet
/*N*/ 				rIn >> aTitleYAxisPosition; //     bei nMoreData = 15
/*N*/ 				rIn >> aTitleZAxisPosition;
/*N*/ 				rIn >> nInt16;  bUseRelativePositionsForChartGroups = (BOOL) nInt16;
/*N*/ 				rIn >> nInt16;  bAdjustMarginsForLegend = (BOOL) nInt16;
/*N*/ 				rIn >> nInt16;  bAdjustMarginsForMainTitle = (BOOL) nInt16;
/*N*/ 				rIn >> nInt16;  bAdjustMarginsForSubTitle = (BOOL) nInt16;
/*N*/ 				rIn >> nInt16;  bAdjustMarginsForXAxisTitle = (BOOL) nInt16;
/*N*/ 				rIn >> nInt16;  bAdjustMarginsForYAxisTitle = (BOOL) nInt16;
/*N*/ 				rIn >> nInt16;  bAdjustMarginsForZAxisTitle = (BOOL) nInt16;
/*N*/ 			}
/*N*/ 			if (nMoreData >= 14)
/*N*/ 			{
/*N*/ 				rIn >> aSpotColor;       // FG: Aenderung von DL damit sich das 3d-Chart die
/*N*/ 				rIn >> aAmbientColor;    //     Farbfilter-Einstellungen merkt.
/*N*/
/*N*/ 				rIn >> bDiagramHasBeenMovedOrResized; // FG: 12.3.97, Falls das Chart gespeichert wird
/*N*/ 				rIn >> bMainTitleHasBeenMoved;        //     soll es immer noch die relativen Positionen
/*N*/ 				rIn >> bSubTitleHasBeenMoved;         //     der Objektgruppen beachten, falls eine
/*N*/ 				rIn >> bLegendHasBeenMoved;           //     dieser Gruppen bewegt worden ist.
/*N*/ 				rIn >> bXAxisTitleHasBeenMoved;
/*N*/ 				rIn >> bYAxisTitleHasBeenMoved;
/*N*/ 				rIn >> bZAxisTitleHasBeenMoved;
/*N*/ 			}
/*N*/ 			if (nMoreData >= 15) // FG: 14.3.97 Es braucht noch die Information der Ausrichtung
/*N*/ 			{                    //             damit beim Resize die Texte richtig plaziert werden
/*N*/ 				rIn >> nInt16; eAdjustXAxesTitle = (ChartAdjust) nInt16;
/*N*/ 				rIn >> nInt16; eAdjustYAxesTitle = (ChartAdjust) nInt16;
/*N*/ 				rIn >> nInt16; eAdjustZAxesTitle = (ChartAdjust) nInt16;
/*N*/ 			}
/*N*/ 			if (nMoreData >= 16)
/*N*/ 			{
/*N*/ 				BOOL bNeedMerge=FALSE;
/*N*/ 				String aFmtStr;
/*N*/ 				sal_uInt32 nId;
/*N*/ 				if(nVersion <= 11)//ab V 12 X=Y=Z-Numberformatter!
/*N*/ 				{
/*?*/ 					SvNumberFormatter *pTmp = new SvNumberFormatter( ::legacy_binfilters::getLegacyProcessServiceFactory(),
/*?*/ 																	 LANGUAGE_SYSTEM );
/*?*/ 					pTmp->Load(rIn);
/*?*/ 					nId = IsPercentChart() ? nDescrPercentFormat : nDescrFormat;
/*?*/ 					const SvNumberformat* pFmt=pTmp->GetEntry(nId);
/*?*/
/*?*/ 					if(pFmt && (pFmt->GetType() & NUMBERFORMAT_DEFINED))
/*?*/ 					{
/*?*/ 						aFmtStr = pFmt->GetFormatstring();
/*?*/ 						bNeedMerge=TRUE;
/*?*/ 					}
/*?*/ 					delete pTmp;
/*?*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					pNumFormatter->SkipNumberFormatterInStream( rIn ); //Skip x
/*N*/ 				}
/*N*/ 				pOwnNumFormatter->Load( rIn );		// if not skipped load the own formatter ...
/*N*/ 				pNumFormatter = pOwnNumFormatter;	// ... and use it
/*N*/
/*N*/ 				// merge user formats of x axis formatter
/*N*/ 				if(bNeedMerge)
/*N*/ 				{
/*?*/ 					xub_StrLen nChk;
/*?*/ 					short nType;
/*?*/ 					pNumFormatter->PutEntry(aFmtStr,nChk,nType,nId,LANGUAGE_SYSTEM);
/*?*/ 					DBG_ASSERT(nChk==0,"Chart:: Converting X-Axis Numformat failed");
/*?*/ 					SetNumFmt(CHOBJID_DIAGRAM_X_AXIS,nId,IsPercentChart());
/*N*/ 				}
/*N*/
/*N*/ 				pNumFormatter->SkipNumberFormatterInStream(rIn);//aTmp.Load( rIn );//pNumFormatterZ //immer wegschmeissen, hatte nie userdef. formats
/*N*/ 			}
/*N*/ 		}
/*N*/ 		aLastDiagramRectangle = aDiagramRectangle;	// #68131# (#67459#)
/*N*/ 	}
/*N*/
/*N*/ 	// FG: Das ist notwendig damit in SchChartDocShell::Draw das Chart Initialisiert
/*N*/ 	//     wird, es also die wirkliche Groesse mitbekommt. Zum jetzigen Zeitpunkt nach dem
/*N*/ 	//     Laden gibt es ja noch kein Fenster. Ein Aufruf von BuildChart an dieser Stelle
/*N*/ 	//     geht also nicht. Bei einem Repaint wird in der oben genannten Methode abgefragt
/*N*/ 	//     ob das Chart bereits existiert (bNoBuildChart), falls nicht wird es neu angelegt.
/*N*/ 	//
/*N*/ 	//     bei den Versionen 4.0 oder hoeher wird die Initialisierung richtig gemacht
/*N*/ 	//     bei Charts die Inplace aus Versionen 3.1 verwendet werden passiert das schon
/*N*/ 	//     richtig.
/*N*/ 	//
/*N*/ 	//     Dies betrifft nur 3D-Charts, die im Format der Version 3.1 geschrieben wurden.
/*N*/ 	//
/*N*/ 	//     21.4. Aenderung: Das Chart kann ja aus allen gespeicheten Daten erzeugt werden.
/*N*/ 	//     um das File klein zu halten und da die Neuberechnung sehr viel schneller
/*N*/ 	//     geworden ist.
/*N*/ 	if (((rIn.GetVersion() <= SOFFICE_FILEFORMAT_31) && (Is3DChart())) ||
/*N*/ 		bUseRelativePositionsForChartGroups || (rIn.GetVersion() > SOFFICE_FILEFORMAT_31))
/*N*/ 	{
/*N*/ 		bNoBuildChart = TRUE;
/*N*/ 	}
/*N*/
/*N*/
/*N*/
/*N*/ 	if (nVersion >= 9)//#50116# Saeulenbreite
/*N*/ 	{
/*N*/ 		rIn >> nInt16; SetBarPercentWidth((long)nInt16);
/*N*/
/*N*/ 		rIn >> nInt32; m_nDefaultColorSet=nInt32;//#50149#
/*N*/
/*N*/ 		rIn >> nInt32;
/*N*/ 		// #78911# always set to 0 if charttype doesn't support lines
/*N*/ 		SetNumLinesColChart( ( eChartStyle == CHSTYLE_2D_LINE_COLUMN )? nInt32: 0 );
/*N*/
/*N*/ 		// #74536# BM use these three former -1 dummies for
/*N*/ 		// numberformat last set by CALC
/*N*/ 		rIn	>> nXLastNumFmt;
/*N*/ 		rIn	>> nYLastNumFmt;
/*N*/  		rIn	>> nBLastNumFmt;
/*N*/ 		rIn >> nInt32; // still dummy (-1)
/*N*/
/*N*/ 	}
/*N*/
/*N*/ 	if(nVersion >= 11)//10 existiert nicht (pChartAttr)
/*N*/ 	{
/*N*/
/*N*/ 		SfxItemSet aAttr(*pItemPool, nRowWhichPairs);
/*N*/ 		aAttr.Load(rIn);
/*N*/ 		pStockLineAttr->Put(aAttr);
/*N*/
/*N*/ 		aAttr.ClearItem();
/*N*/ 		aAttr.Load(rIn);
/*N*/ 		pStockLossAttr->Put(aAttr);
/*N*/
/*N*/ 		aAttr.ClearItem();
/*N*/ 		aAttr.Load(rIn);
/*N*/ 		pStockPlusAttr->Put(aAttr);
/*N*/ 	}
/*N*/
/*N*/ 	InitChartData(FALSE); //weia, beeinflusst die Achsen....also erst ab hier:
/*N*/
/*N*/
/*N*/ 	if(nVersion >= 12)
/*N*/ 	{
/*N*/ 		//Achse wird nur noch auf Attr-Basis gestreamt!
/*N*/
/*N*/ 		//nAxisId ist die zu ladene/speichernde Achse,
/*N*/ 		//festgelegt sind in V12 nur 1=X, 2=Y,3=Z und 4=B (2.Y-Achse)
/*N*/ 		//-1 bedeutet KEINE WEITEREN ACHSEN !!!!!!!!!!!
/*N*/ 		//Es MUSS keine Achse gestreamt werden (siehe Lade-Routine)
/*N*/ 		INT32 nAxisId=0;
/*N*/ 		SfxItemSet aAxisSet(*pItemPool,nAxisWhichPairs);
/*N*/       while(nAxisId != -1)//hier koennen beliebig viele, auch unbekannte Achsen kommen
/*N*/ 		{
/*N*/ 			aAxisSet.ClearItem();
/*N*/ 			rIn >> nAxisId;
/*N*/
/*N*/ 			switch(nAxisId)
/*N*/ 			{
/*N*/ 				case CHART_AXIS_PRIMARY_X:
/*N*/ 					aAxisSet.Load(rIn);
/*N*/ 					SetAttributes(CHOBJID_DIAGRAM_X_AXIS,aAxisSet,FALSE);
/*N*/                   SetAttributes(CHOBJID_DIAGRAM_A_AXIS,aAxisSet,FALSE);//falls nicht spaeter geladen wird, hier neue defaults setzen
/*N*/ 					pChartAAxis->ShowAxis(FALSE); //default aus
/*N*/ 					pChartAAxis->ShowDescr(FALSE);
/*N*/                   pChartXAxis->Update();  //Ab V12 kann jetzt der ganze LoadMemberCompat...Kram uebergebuegelt werden, in den Attr war alles drin!
/*N*/ 					pChartAAxis->Update();
/*N*/ 				break;
/*N*/ 				case CHART_AXIS_PRIMARY_Y:
/*N*/ 					aAxisSet.Load(rIn);
/*N*/ 					SetAttributes(CHOBJID_DIAGRAM_Y_AXIS,aAxisSet,FALSE);
/*N*/ 					pChartYAxis->Update();
/*N*/ 				break;
/*N*/ 				case CHART_AXIS_PRIMARY_Z:
/*N*/ 					aAxisSet.Load(rIn);
/*N*/ 					SetAttributes(CHOBJID_DIAGRAM_Z_AXIS,aAxisSet,FALSE);
/*N*/ 					pChartZAxis->Update();
/*N*/ 				break;
/*N*/ 				case CHART_AXIS_SECONDARY_Y:
/*N*/ 					aAxisSet.Load(rIn);
/*N*/ 					SetAttributes(CHOBJID_DIAGRAM_B_AXIS,aAxisSet,FALSE);
/*N*/ 					pChartBAxis->Update();
/*N*/ 				break;
/*N*/ 				case CHART_AXIS_SECONDARY_X:
/*N*/ 					aAxisSet.Load(rIn);
/*N*/ 					SetAttributes(CHOBJID_DIAGRAM_A_AXIS,aAxisSet,FALSE);
/*N*/ 					pChartAAxis->Update();
/*N*/ 				break;
/*N*/ 				case -1: //Ende der Achsen!
/*N*/ 					break;
/*N*/ 				default: //Attrset kommt aus der Zukunft! Laden und vernichten:
/*N*/ 					{
/*?*/ 						aAxisSet.Load(rIn);
/*N*/ 					}
/*N*/ 					break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/   else //defaults ergaenzen
/*N*/ 	{
/*?*/ 		SfxItemSet aSet(*pItemPool,nAxisWhichPairs);
/*?*/ 		aSet.Put(SfxInt32Item(SCHATTR_AXIS_TICKS,CHAXIS_MARK_OUTER));
/*?*/ 		SetAttributes(CHOBJID_DIAGRAM_X_AXIS,aSet);
/*?*/ 		SetAttributes(CHOBJID_DIAGRAM_Y_AXIS,aSet);
/*N*/ 	}
/*N*/ 	//Ende IOVersion 12
/*N*/

/*N*/ 	if(nVersion >= 14)
/*N*/ 	{
/*N*/ 		rIn >> nTmp;
/*N*/ 		aBarY1.SetGap(nTmp);
/*N*/ 		rIn >> nTmp;
/*N*/ 		aBarY1.SetOverlap(nTmp);
/*N*/ 		rIn >> nTmp;
/*N*/ 		aBarY2.SetGap(nTmp);
/*N*/ 		rIn >> nTmp;
/*N*/ 		aBarY2.SetOverlap(nTmp);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*		aBarY1.SetGap()
        aBarY1.SetOverlap(nTmp)
        aBarY2.SetGap(nTmp)
        aBarY2.SetOverlap(nTmp)
        */
/*N*/ 	}
/*N*/
/*N*/ 	if(nVersion >= 15)
/*N*/ 	{
/*N*/ 		SfxItemSet	*	pAttributes;
/*N*/ 		bResizePie=FALSE;
/*N*/ 		rIn >> bDiagramHasBeenMovedOrResized; // FG: 12.3.97, Falls das Chart gespeichert wird
/*N*/
/*N*/ 		if(IsReal3D() && (IsPieChart()||IsLine())) //Hack wegen  #62363#, double-sided neu defaulten
/*N*/ 		{
/*N*/ 			long nMax;
/*N*/ 			nMax=aDataRowAttrList.Count();
/*N*/ 			for(i=0;i<nMax;i++)
/*N*/ //-/				aDataRowAttrList.GetObject(i)->Put(SfxBoolItem(SID_ATTR_3D_DOUBLE_SIDED,TRUE));
/*N*/ 				aDataRowAttrList.GetObject(i)->Put(Svx3DDoubleSidedItem(TRUE));
/*N*/ 			nMax=aDataPointAttrList.Count();
/*N*/ 			for(i=0;i<nMax;i++)
/*N*/ 			{
/*N*/ 				pAttributes = aDataPointAttrList.GetObject(i);
/*N*/ 				if (pAttributes != NULL)
/*?*/ 					pAttributes->Put(Svx3DDoubleSidedItem(TRUE));
/*N*/ 			}
/*N*/ 			nMax=aSwitchDataPointAttrList.Count();
/*N*/ 			for(i=0;i<nMax;i++)
/*N*/ 			{
/*N*/ 				pAttributes = aSwitchDataPointAttrList.GetObject(i);
/*N*/ 				if (pAttributes != NULL)
/*N*/ 					pAttributes->Put(Svx3DDoubleSidedItem(TRUE));
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//altes 3D-Pie?
/*?*/ 		bResizePie=TRUE;
/*?*/ 		if(IsReal3D() && IsPieChart())
/*?*/ 		{
/*?*/ 			Matrix4D aTmp;        //Matrix auf default
/*?*/
/*?*/ 			aTmp.RotateZ((900 + nYAngle) * F_PI / 1800); //??? !!!
/*?*/ 			aTmp.RotateX(-F_PI/3);
/*?*/
/*?*/ 			aSceneMatrix=aTmp;
/*?*/ 		}
/*?*/
/*?*/ 		if(IsReal3D()) //#61923#
/*?*/ 		{
/*?*/ 			long nMax;
/*?*/ 			nMax=aDataRowAttrList.Count();
/*?*/ 			SfxItemSet	*	pAttributes;
/*?*/
/*?*/ 			for(i=0;i<nMax;i++)
/*?*/ 			{
/*?*/ 				if(IsPieChart()||IsLine())
/*?*/ //-/					aDataRowAttrList.GetObject(i)->Put(SfxBoolItem(SID_ATTR_3D_DOUBLE_SIDED,TRUE));
/*?*/ 					aDataRowAttrList.GetObject(i)->Put(Svx3DDoubleSidedItem(TRUE));
/*?*/ 				aDataRowAttrList.GetObject(i)->Put(XLineStyleItem(XLINE_NONE));
/*?*/ 			}
/*?*/
/*?*/ 			nMax=aDataPointAttrList.Count();
/*?*/ 			for(i=0;i<nMax;i++)
/*?*/ 			{
/*?*/ 				pAttributes = aDataPointAttrList.GetObject(i);
/*?*/ 				if (pAttributes == NULL)
/*?*/ 					continue;
/*?*/ 				if(IsPieChart()||IsLine())
/*?*/ 					aDataPointAttrList.GetObject(i)->Put(Svx3DDoubleSidedItem(TRUE));
/*?*/ 				aDataPointAttrList.GetObject(i)->Put(XLineStyleItem(XLINE_NONE));
/*?*/ 			}
/*?*/
/*?*/
/*?*/ 			nMax=aSwitchDataPointAttrList.Count();
/*?*/ 			for(i=0;i<nMax;i++)
/*?*/ 			{
/*?*/ 				pAttributes = aSwitchDataPointAttrList.GetObject(i);
/*?*/ 				if (pAttributes == NULL)
/*?*/ 					continue;
/*?*/ 				if(IsPieChart()||IsLine())
/*?*/ 					aSwitchDataPointAttrList.GetObject(i)->Put(Svx3DDoubleSidedItem(TRUE));
/*?*/ 				aSwitchDataPointAttrList.GetObject(i)->Put(XLineStyleItem(XLINE_NONE));
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}

    // ******************** LAST READ ********************


    // ############# defaults erstellen, falls alte Fileversion ####################

/*N*/ 	if(nVersion < 13)
/*N*/ 	{
/*?*/ 		if(IsXYChart())
/*?*/ 			aDataRowAttrList.GetObject(0)->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_PRIMARY_X));
/*N*/ 	}
    //#54870# restaurieren der line-attribute
/*N*/ 	if( nVersion < 10)
/*N*/ 	{
/*?*/ 	   SetupLineColors(SETLINES_COMPAT);
/*N*/ 	}
/*N*/ 	if( nVersion < 16) //Automatische 2.YAchse in Items umsetzten
/*N*/ 	{
/*?*/ 		const SfxPoolItem* pPoolItem=NULL;
/*?*/ 		for(long n=0;n<GetRowCount();n++)
/*?*/ 			if(GetDataRowAttr(n).GetItemState(SCHATTR_AXIS,TRUE, &pPoolItem) == SFX_ITEM_SET)
/*?*/ 			if(((const SfxInt32Item*)pPoolItem)->GetValue()==CHART_AXIS_SECONDARY_Y)
/*?*/ 			{
/*?*/ 				n=GetRowCount();//abbruch
/*?*/ 				pChartBAxis->ShowDescr(TRUE);
/*?*/ 				pChartBAxis->ShowAxis(TRUE);
/*?*/ 			}
/*N*/ 	}
/*N*/
/*N*/   //dies ist immer moeglich (IoVersion<=16), in Zukunft sollten nur noch die Attr unterstuetzt werden, dann kann hier evtl. mit IOVersion geklammert werden!
/*N*/ 	pChartXAxis->ShowDescr(bShowXDescr);
/*N*/ 	pChartXAxis->ShowAxis(bShowXAxis);
/*N*/ 	pChartYAxis->ShowDescr(bShowYDescr);
/*N*/ 	pChartYAxis->ShowAxis(bShowYAxis);
/*N*/ 	pChartZAxis->ShowDescr(bShowZDescr);
/*N*/ 	pChartZAxis->ShowAxis(bShowZAxis);
/*N*/
/*N*/
/*N*/ 	//Bei unsortierten Charts machts keinen Sinn, nachdem Laden nochmal zu fragen
/*N*/ 	if( IsXYChart() )
/*N*/ 		SETFLAG( nChartStatus, CHS_USER_NOQUERY );
/*N*/ 	else
/*N*/ 		RESETFLAG( nChartStatus, CHS_USER_NOQUERY );
/*N*/
/*N*/ 	CHART_TRACE( "ChartModel::LoadAttributes END" );
/*N*/ }

/*************************************************************************
|*
|* Zeigt Legende an bzw. loescht sie.
|*
\************************************************************************/

/*N*/ void ChartModel::SetShowLegend(BOOL bShow)
/*N*/ {
/*N*/ 	pLegendAttr->Put(SvxChartLegendPosItem(bShow
/*N*/ 											   ? CHLEGEND_RIGHT
/*N*/ 											   : CHLEGEND_NONE));
/*N*/ 	bLegendVisible = bShow;
/*N*/ }
/*************************************************************************
|*
|* Legenden-Attribute ermitteln
|*
\************************************************************************/

/*N*/ SfxItemSet ChartModel::GetFullLegendAttr() const
/*N*/ {
/*N*/ 	SfxItemSet aAttr(*pItemPool, nLegendWhichPairs);
/*N*/ 	aAttr.Put(*pLegendAttr);
/*N*/ 	return aAttr;
/*N*/ }

/*************************************************************************
|*
|* Legenden-Attribute aendern;
|* Liefert bei geaenderten Attributen TRUE.
|*
\************************************************************************/

/*************************************************************************
|*
|* Chart-Attribute aendern;
|* Liefert bei geaenderten Attributen TRUE.
|*
\************************************************************************/

/*************************************************************************
|*
|* Erstelle Symbole fuer Diagrammtypen mit Symbolen
|*
\************************************************************************/


/*************************************************************************
|*
|* Aendere die Attribute einer Achse
|*
\************************************************************************/


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
