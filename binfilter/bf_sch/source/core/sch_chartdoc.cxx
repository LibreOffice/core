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


#ifndef _EEITEM_HXX //autogen
#include <bf_svx/eeitem.hxx>
#endif

#ifndef _ZFORLIST_HXX
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <bf_svtools/zforlist.hxx>
#endif



#ifndef _SFX_PRINTER_HXX //autogen
#include <bf_sfx2/printer.hxx>
#endif

#ifndef _SCHATTR_HXX
#include "schattr.hxx"
#endif
#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE	        0
#define ITEMID_CHARTDATADESCR   SCHATTR_DATADESCR_DESCR

#ifndef _SFXENUMITEM_HXX
#include <bf_svtools/eitem.hxx>
#endif

#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
#ifndef _SCH_STLPOOL_HXX
#include "stlpool.hxx"
#endif
#ifndef _SCH_SCHIOCMP_HXX
#include "schiocmp.hxx"
#endif
#ifndef _SCH_SCHRESID_HXX
#include "schresid.hxx"
#endif
#ifndef _SCH_DOCSHELL_HXX
#include "docshell.hxx"
#endif

#ifdef ITEMID_FONT
#undef ITEMID_FONT
#endif
#define ITEMID_FONT        EE_CHAR_FONTINFO

#ifdef ITEMID_COLOR
#undef ITEMID_COLOR
#endif
#define ITEMID_COLOR       EE_CHAR_COLOR

#ifdef ITEMID_FONTHEIGHT
#undef ITEMID_FONTHEIGHT
#endif
#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT

#include "glob.hrc"
#include "globfunc.hxx"

#include <bf_svx/fontitem.hxx>
#include <bf_svx/fhgtitem.hxx>


#include "chaxis.hxx"

#include "memchrt.hxx"

#ifndef _SFXITEMPOOL_HXX
#include <bf_svtools/itempool.hxx>
#endif

namespace binfilter {

/*************************************************************************
|*
|* Diese Methode erzeugt ein neues Dokument (ChartModel) und gibt einen
|* Zeiger darauf zurueck. Die Drawing Engine benutzt diese Methode um das
|* Dokument oder Teile davon ins Clipboard/DragServer stellen zu koennen.
|*
|* this method works like a copy constructor. So make sure to copy all
|* members inside this method !!!
|*
\************************************************************************/
/*N*/ SdrModel* ChartModel::AllocModel() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL; //STRIP001 
/*N*/ }

/*************************************************************************
|*
|* Diese Methode erzeugt eine neue Seite (SchPage) und gibt einen Zeiger
|* darauf zurueck. Die Drawing Engine benutzt diese Methode beim Laden
|* zur Erzeugung von Seiten (deren Typ sie ja nicht kennt, da es ABLEITUNGEN
|* der SdrPage sind).
|*
\************************************************************************/

/*N*/ SdrPage* ChartModel::AllocPage(FASTBOOL bMasterPage)
/*N*/ {
/*N*/ 	return new SdrPage(*this, bMasterPage);
/*N*/ }
/*************************************************************************
|*
|* Inserter fuer SvStream zum Speichern
|*
\************************************************************************/
/*N*/ SvStream& operator << (SvStream& rOut, const ChartModel& rDoc)
/*N*/ {
/*N*/ 	// die daemlichen blockklammern muessen erhalten bleiben, damit
/*N*/ 	// im destruktor ~SchIOCompat dei korrekte blockgröße geschrieben
/*N*/ 	// wird. ansonsten gibt es beim einlesen mit der 304 jede menge
/*N*/ 	// aerger
/*N*/ 	{
/*N*/ 		SchIOCompat aIO(rOut, STREAM_WRITE, 1);
/*N*/ 
/*N*/ 		// AF: Don't write VCItemPool any longer (01/2001)
/*N*/ 		rOut << (sal_uInt32)0L;
/*N*/ 
        /**************************************************************************
        * Frueher (StarChart Version 3.0, File-Format Version 1) wurde hier das
        * JobSetup geschrieben, nun der Printer (binaer-kompatibel, daher wurde
        * die Version des File-Formats nicht geaendert)
        **************************************************************************/
/*N*/ 		if (rDoc.pDocShell)
/*N*/ 		{
/*N*/ 			SfxPrinter* pPrinter =
/*N*/ 				 ((SchChartDocShell*)rDoc.pDocShell)->GetPrinter();
/*N*/ 
/*N*/ 
/*N*/ 			if (pPrinter)
/*N*/ 			{
/*N*/ 				pPrinter->Store(rOut);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				// Es ist kein Printer vorhanden -> muss erzeugt werden
/*?*/ 				SfxBoolItem aItem(SID_PRINTER_NOTFOUND_WARN, TRUE);
/*?*/ 				SfxItemSet* pSet = new SfxItemSet(((ChartModel&) rDoc).GetPool(),
/*?*/ 								SID_PRINTER_NOTFOUND_WARN,
/*?*/ 								SID_PRINTER_NOTFOUND_WARN, 0);
/*?*/ 				pSet->Put(aItem);
/*?*/ 				SfxPrinter* pPrinter = new SfxPrinter(pSet);
/*?*/ 
/*?*/ 				MapMode aMapMode = pPrinter->GetMapMode();
/*?*/ 				aMapMode.SetMapUnit(MAP_100TH_MM);
/*?*/ 				pPrinter->SetMapMode(aMapMode);
/*?*/ 
/*?*/ 				pPrinter->Store(rOut);
/*?*/ 				delete pPrinter;
/*?*/ 				//delete pSet; bloss nicht! Killt der Printer gleich mit!
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			// Keine DocShell, daher wird ein JobSetup geschrieben
/*?*/ 			JobSetup aJobSetup;
/*?*/ 			rOut << aJobSetup;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	rOut << (SdrModel&) rDoc;
/*N*/ 
/*N*/ 	SchIOCompat aIO(rOut, STREAM_WRITE, 0);
/*N*/ 
/*N*/ 	rDoc.StoreAttributes(rOut);
/*N*/ 	//<- ehemals Basisklasse
/*N*/ 
/*N*/ 
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

/*************************************************************************
|*
|* Extractor fuer SvStream zum Laden
|*
\************************************************************************/

/*N*/ SvStream& operator >> (SvStream& rIn, ChartModel& rDoc)
/*N*/ {
/*N*/ 	// auch hier muessen die blockklammern erhalten bleiben,
/*N*/ 	// um 304-er dokumente sauber lesen zu koennen.
/*N*/ 	{
/*N*/ 		SchIOCompat aIO(rIn, STREAM_READ);
/*N*/ 
/*N*/ 		sal_uInt32 n;
/*N*/ 		rIn >> n;
/*N*/ 		// must be 0 or 1. Otherwise we have an error
/*N*/ 		// most probably a wrong password
/*N*/ 		if( n == 1L )
/*N*/ 		{
/*N*/ 			// AF: Skip VCItemPool.
/*N*/ 			rIn.SeekRel (74);
/*N*/ 		}
/*N*/ 		else if( n != 0L )
/*N*/ 		{
/*?*/ 			rIn.SetError( ERRCODE_IO_GENERAL );
/*?*/ 			return rIn;
/*N*/ 		}
/*N*/ 
/*N*/ 		if (aIO.GetVersion() >= 1)
/*N*/ 		{

            /******************************************************************
            * Frueher (StarChart Version 3.0, File-Format Version 1) wurde hier
            * das JobSetup eingelesen, nun wird der Printer erzeugt
            * (binaer-kompatibel)
            *******************************************************************/
            // ItemSet mit speziellem Poolbereich anlegen
/*N*/ 			SfxItemSet* pSet =
/*N*/ 				new SfxItemSet(rDoc.GetPool(),
/*N*/ 							   SID_PRINTER_NOTFOUND_WARN,
/*N*/ 							   SID_PRINTER_NOTFOUND_WARN, 0);
/*N*/ 			pSet->Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN, TRUE));
/*N*/ 
/*N*/ 			SfxPrinter* pPrinter = SfxPrinter::Create(rIn, pSet);
/*N*/ 
/*N*/ 			MapMode aMM(pPrinter->GetMapMode());
/*N*/ 			aMM.SetMapUnit(MAP_100TH_MM);
/*N*/ 			pPrinter->SetMapMode(aMM);
/*N*/ 
/*N*/ 			if (rDoc.pDocShell->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED)
/*N*/ 			{
/*N*/ 				((SchChartDocShell*)rDoc.pDocShell)->SetPrinter( pPrinter, TRUE );	// will be deleted by DocShell
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				delete pPrinter;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// statt Basisklassenaufruf [    rIn >> (ChartModel&)rDoc;] direkt:
/*N*/ 	rIn >> (SdrModel&)rDoc;
/*N*/ 
/*N*/ 	if (rIn.GetError() != SVSTREAM_FILEFORMAT_ERROR)
/*N*/ 	{
/*N*/ 		SchIOCompat aIO(rIn, STREAM_READ);
/*N*/ 
/*N*/ 		rDoc.LoadAttributes(rIn);
/*N*/ 	}
/*N*/ 	// <- Basisaufruf
/*N*/ 
/*N*/ 	rDoc.GetItemPool().LoadCompleted();
/*N*/ 
/*N*/ 	rDoc.ReadError () = FALSE;
/*N*/ 
/*N*/ 	// alle 3D-Objekte werden bei einem Paint erstmailg sortiert, damit wird auch
/*N*/ 	// das Dokument als geaendert gekennzeichnet. Diese Variable wird in der
/*N*/ 	// CHVIEWSH-Paint-Methode abgefragt und bei Bedarf wird der Modified-Status
/*N*/ 	// des Dokuments zurueckgesetzt.
/*N*/ 
/*N*/ 	// no longer needed
/*N*/ //-  	rDoc.FreshLoaded () = TRUE;
/*N*/ 
/*N*/ 	return rIn;
/*N*/ }

/*************************************************************************
|*
|* SetChanged(), das Model wurde geaendert
|*
\************************************************************************/

/*N*/ void ChartModel::SetChanged(FASTBOOL bFlag)
/*N*/ {
/*N*/ 	if (pDocShell)
/*N*/ 	{
/*N*/ 		if (bNewOrLoadCompleted && pDocShell->IsEnableSetModified())
/*N*/ 		{
/*N*/ 			// weitergeben an Basisklasse
/*N*/ 			//ChartModel
/*N*/ 			SdrModel::SetChanged(bFlag);
/*N*/ 
/*N*/ 			// an ObjectShell weiterleiten
/*N*/ 			pDocShell->SetModified(bFlag);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		// weitergeben an Basisklasse
/*?*/ 		//ChartModel
/*?*/ 		SdrModel::SetChanged(bFlag);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* NewOrLoadCompleted
|*
|* Wird gerufen, wenn das Dokument geladen wurde bzw. feststeht, dass es
|* nicht mehr geladen wird.
|*
\************************************************************************/

/*N*/ void ChartModel::NewOrLoadCompleted(USHORT eMode)
/*N*/ {
/*N*/ 	if (eMode == NEW_DOC)
/*N*/ 	{
/*N*/ 		// StyleSheets generieren
/*N*/ 		Font aFont( OutputDevice::GetDefaultFont( DEFAULTFONT_SANS, GetLanguage( EE_CHAR_LANGUAGE ), DEFAULTFONT_FLAGS_ONLYONE ) );
/*N*/ 		SvxFontItem aFontItem( aFont.GetFamily(), aFont.GetName(), aFont.GetStyleName(), aFont.GetPitch(),
/*N*/ 			                   aFont.GetCharSet(), EE_CHAR_FONTINFO );
/*N*/ 
/*N*/ 		SfxStyleSheetBase* pSheet =
/*N*/ 			&pStyleSheetPool->Make(String(SchResId(STR_STLSHEET_TITLE_MAIN)),
/*N*/ 										  SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO ) ));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(493));
/*N*/ 
/*N*/ 		pSheet = &pStyleSheetPool->
/*N*/ 					Make(String(SchResId(STR_STLSHEET_TITLE_SUB)),
/*N*/ 						 SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO ) ));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(423));
/*N*/ 
/*N*/ 		pSheet = &pStyleSheetPool->
/*N*/ 					Make(String(SchResId(STR_STLSHEET_TITLE_X_AXIS)),
/*N*/ 						 SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO ) ));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(352));
/*N*/ 
/*N*/ 		pSheet = &pStyleSheetPool->
/*N*/ 					Make(String(SchResId(STR_STLSHEET_TITLE_Y_AXIS)),
/*N*/ 						 SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO ) ));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(352));
/*N*/ 
/*N*/ 		pSheet = &pStyleSheetPool->
/*N*/ 					Make(String(SchResId(STR_STLSHEET_TITLE_Z_AXIS)),
/*N*/ 						 SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO ) ));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(352));
/*N*/ 
/*N*/ 		pSheet = &pStyleSheetPool->
/*N*/ 					Make(String(SchResId(STR_STLSHEET_DATAROWS)),
/*N*/ 						 SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO ) ));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(282));
/*N*/ 
/*N*/ 		pSheet = &pStyleSheetPool->
/*N*/ 					Make(String(SchResId(STR_STLSHEET_DATACOLS)),
/*N*/ 						 SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO ) ));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(282));
/*N*/ 
/*N*/ 		pSheet = &pStyleSheetPool->
/*N*/ 					Make(String(SchResId(STR_STLSHEET_DATAVALUES)),
/*N*/ 						 SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO ) ));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(282));
/*N*/ 
/*N*/ 		pSheet = &pStyleSheetPool->
/*N*/ 					Make(String(SchResId(STR_STLSHEET_DATADESCR)),
/*N*/ 						 SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO ) ));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(282));
/*N*/ 
/*N*/ 		pSheet = &pStyleSheetPool->
/*N*/ 					Make(String(SchResId(STR_STLSHEET_LEGEND)),
/*N*/ 						 SFX_STYLE_FAMILY_PARA);
/*N*/ 		pSheet->GetItemSet().Put(aFontItem);
/*N*/ 		pSheet->GetItemSet().Put(SvxColorItem( Color( COL_AUTO )));
/*N*/ 		pSheet->GetItemSet().Put(SvxFontHeightItem(282));
/*N*/ 
/*N*/         pAxisAttr->ClearItem( SCHATTR_AXIS_SHOWDESCR );
/*N*/ 	}
/*N*/ 	else if (eMode == DOC_LOADED)
/*N*/ 	{
/*N*/         // set intersection of all axis attributes used as axis
/*N*/         // 'style' (that is an itemset for the 'all axes' dialog)
/*N*/         GetFullAxisAttr( NULL, true ).ClearInvalidItems();
/*N*/         pAxisAttr->ClearItem( SCHATTR_AXIS_SHOWDESCR );
/*N*/ 
/*N*/         // set 'all axis' attributes on all axes that are currently
/*N*/         // disabled
/*N*/         if( ! HasAxis( CHOBJID_DIAGRAM_X_AXIS ))
/*?*/             pChartXAxis->SetAttributes( pAxisAttr );
/*N*/         if( ! HasAxis( CHOBJID_DIAGRAM_Y_AXIS ))
/*?*/             pChartYAxis->SetAttributes( pAxisAttr );
/*N*/         if( ! HasAxis( CHOBJID_DIAGRAM_Z_AXIS ))
/*N*/             pChartZAxis->SetAttributes( pAxisAttr );
/*N*/         if( ! HasAxis( CHOBJID_DIAGRAM_A_AXIS ))
/*N*/             pChartAAxis->SetAttributes( pAxisAttr );
/*N*/         if( ! HasAxis( CHOBJID_DIAGRAM_B_AXIS ))
/*N*/             pChartBAxis->SetAttributes( pAxisAttr );
/*N*/         // #99528# change auto-font color according to diagram area
/*N*/         PageColorChanged( *pDiagramAreaAttr );
/*N*/ 
/*N*/         // #101591# set precision of own number-formatter to 2 if we reside in a
/*N*/         // calc or writer, which is assumed by checking the cell-range in the
/*N*/         // MemChart.
/*N*/         OSL_ASSERT( pChartData );
/*N*/         const SchChartRange & rRange = pChartData->GetChartRange();
/*N*/         if( rRange.maRanges.size() > 0 )
/*N*/         {
/*?*/             pOwnNumFormatter->ChangeStandardPrec( 2 );
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/ 	bNewOrLoadCompleted = TRUE;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if(pChartData && GetRowCount() && GetColCount())
/*N*/ 	{
/*N*/ 		CHART_TRACE1( "ChartModel::NewOrLoadCompleted debugging ChartItems Model %s ",
/*N*/ 					  (eMode==DOC_LOADED)? "was loaded": ((eMode==NEW_DOC)? "is new" : "don't know") );
/*N*/ 		DBG_ITEMS((SfxItemSet&)GetDataRowAttr(0),this);
/*N*/ 		DBG_ITEMS((SfxItemSet&)GetDataPointAttr(0,0),this);
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Setze den Modified-Status zurueck, wenn ein Dokument mit
|* 3D-Objekten geladen und ausgegeben wurde
|*
\************************************************************************/

//  void ChartModel::ResetFreshLoaded ()
//  {
//  	bFreshLoaded = FALSE;
//  	SetChanged (FALSE);
//  }


}
