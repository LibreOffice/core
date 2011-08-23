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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <bf_svx/flstitem.hxx>
#include <bf_svx/paperinf.hxx>

#ifndef _SFXSTRITEM_HXX
#include <bf_svtools/stritem.hxx>
#endif

//#include <bf_svx/postdlg.hxx>
#include <bf_svx/sizeitem.hxx>
#include <bf_offmgr/app.hxx>

#include <bf_sfx2/misccfg.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svtools/ctrltool.hxx>
#include <vcl/virdev.hxx>

#include "viewopti.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "patattr.hxx"
#include "hints.hxx"
#include "docoptio.hxx"
#include "pntlock.hxx"
#include "bf_sc.hrc"
#include "inputopt.hxx"
#include "drwlayer.hxx"

namespace binfilter {

//------------------------------------------------------------------

//
//			Redraw - Benachrichtigungen
//



/*N*/ void ScDocShell::PostDataChanged()
/*N*/ {
/*N*/ 	Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
/*N*/ 	aDocument.ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );
/*N*/ 
/*N*/ 	SFX_APP()->Broadcast(SfxSimpleHint( FID_ANYDATACHANGED ));		// Navigator
/*N*/ 	//!	Navigator direkt benachrichtigen!
/*N*/ }

/*N*/ void ScDocShell::PostPaint( USHORT nStartCol, USHORT nStartRow, USHORT nStartTab,
/*N*/ 							USHORT nEndCol, USHORT nEndRow, USHORT nEndTab, USHORT nPart,
/*N*/ 							USHORT nExtFlags )
/*N*/ {
/*N*/ 	if (nStartCol > MAXCOL) nStartCol = MAXCOL;
/*N*/ 	if (nStartRow > MAXROW) nStartRow = MAXROW;
/*N*/ 	if (nEndCol > MAXCOL) nEndCol = MAXCOL;
/*N*/ 	if (nEndRow > MAXROW) nEndRow = MAXROW;
/*N*/ 
/*N*/ 	if ( pPaintLockData )
/*N*/ 	{
/*N*/ 		//!	nExtFlags ???
/*N*/ 		pPaintLockData->AddRange( ScRange( nStartCol, nStartRow, nStartTab,
/*N*/ 											nEndCol, nEndRow, nEndTab ), nPart );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	if (nExtFlags & SC_PF_LINES)			// Platz fuer Linien berücksichtigen
/*N*/ 	{
/*N*/ 											//! Abfrage auf versteckte Spalten/Zeilen!
/*N*/ 		if (nStartCol>0) --nStartCol;
/*N*/ 		if (nEndCol<MAXCOL) ++nEndCol;
/*N*/ 		if (nStartRow>0) --nStartRow;
/*N*/ 		if (nEndRow<MAXROW) ++nEndRow;
/*N*/ 	}
/*N*/ 
/*N*/ 											// um zusammengefasste erweitern
/*N*/ 	if (nExtFlags & SC_PF_TESTMERGE)
/*?*/ 		aDocument.ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nStartTab );
/*N*/ 
/*N*/ 	if ( nStartCol != 0 || nEndCol != MAXCOL )
/*N*/ 	{
/*?*/ 		//	If rotated text is involved, repaint the entire rows.
/*?*/ 		//	#i9731# If there's right-to-left text to the left of the area,
/*?*/ 		//	the displacement for clipping of that text may change.
/*?*/ 		if ( aDocument.HasAttrib( 0,nStartRow,nStartTab,
/*?*/ 									MAXCOL,nEndRow,nEndTab, HASATTR_ROTATE | HASATTR_RTL ) )
/*?*/ 		{
/*?*/ 			nStartCol = 0;
/*?*/ 			nEndCol = MAXCOL;
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	Broadcast( ScPaintHint( ScRange( nStartCol, nStartRow, nStartTab,
/*N*/ 									 nEndCol, nEndRow, nEndTab ), nPart ) );
/*N*/ 
/*N*/ 	if ( nPart & PAINT_GRID )
/*N*/ 		aDocument.ResetChanged( ScRange(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab) );
/*N*/ }

/*N*/ void ScDocShell::PostPaint( const ScRange& rRange, USHORT nPart, USHORT nExtFlags )
/*N*/ {
/*N*/ 	PostPaint( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
/*N*/ 			   rRange.aEnd.Col(),   rRange.aEnd.Row(),   rRange.aEnd.Tab(),
/*N*/ 			   nPart, nExtFlags );
/*N*/ }

/*N*/ void ScDocShell::PostPaintGridAll()
/*N*/ {
/*N*/ 	PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
/*N*/ }

/*N*/ void ScDocShell::PostPaintCell( USHORT nCol, USHORT nRow, USHORT nTab )
/*N*/ {
/*N*/ 	PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PAINT_GRID, SC_PF_TESTMERGE );
/*N*/ }

/*N*/ void ScDocShell::PostPaintExtras()
/*N*/ {
/*N*/ 	PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_EXTRAS );
/*N*/ }

//------------------------------------------------------------------

/*N*/ void ScDocShell::LockPaint_Impl(BOOL bDoc)
/*N*/ {
/*N*/ 	if ( pPaintLockData )
/*N*/ 		pPaintLockData->IncLevel(bDoc);
/*N*/ 	else
/*N*/ 		pPaintLockData = new ScPaintLockData(0);	//! Modus...
/*N*/ }

/*N*/ void ScDocShell::UnlockPaint_Impl(BOOL bDoc)
/*N*/ {
/*N*/ 	if ( pPaintLockData )
/*N*/ 	{
/*N*/ 		if ( pPaintLockData->GetLevel(bDoc) )
/*N*/ 			pPaintLockData->DecLevel(bDoc);
/*N*/ 		else if (!pPaintLockData->GetLevel(!bDoc))
/*N*/ 		{
/*N*/ 			//		Paint jetzt ausfuehren
/*N*/ 
/*N*/ 			ScPaintLockData* pPaint = pPaintLockData;
/*N*/ 			pPaintLockData = NULL;						// nicht weitersammeln
/*N*/ 
/*N*/ 			ScRangeListRef xRangeList = pPaint->GetRangeList();
/*N*/ 			if (xRangeList)
/*N*/ 			{
/*N*/ 				USHORT nParts = pPaint->GetParts();
/*N*/ 				ULONG nCount = xRangeList->Count();
/*N*/ 				for ( ULONG i=0; i<nCount; i++ )
/*N*/ 				{
/*N*/ 					//!	nExtFlags ???
/*N*/ 					ScRange aRange = *xRangeList->GetObject(i);
/*N*/ 					PostPaint( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab(),
/*N*/ 								aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aEnd.Tab(),
/*N*/ 								nParts );
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( pPaint->GetModified() )
/*N*/ 				SetDocumentModified();
/*N*/ 
/*N*/ 			delete pPaint;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("UnlockPaint ohne LockPaint");
/*N*/ }

/*N*/ void ScDocShell::LockDocument_Impl(USHORT nNew)
/*N*/ {
/*N*/ 	if (!nDocumentLock)
/*N*/ 	{
/*N*/ 		ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
/*N*/ 		if (pDrawLayer)
/*N*/ 			pDrawLayer->setLock(TRUE);
/*N*/ 	}
/*N*/ 	nDocumentLock = nNew;
/*N*/ }

/*N*/ void ScDocShell::UnlockDocument_Impl(USHORT nNew)
/*N*/ {
/*N*/ 	nDocumentLock = nNew;
/*N*/ 	if (!nDocumentLock)
/*N*/ 	{
/*N*/ 		ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
/*N*/ 		if (pDrawLayer)
/*N*/ 			pDrawLayer->setLock(FALSE);
/*N*/ 	}
/*N*/ }

/*N*/ USHORT ScDocShell::GetLockCount() const
/*N*/ {
/*N*/ 	return nDocumentLock;
/*N*/ }

/*N*/ void ScDocShell::SetLockCount(USHORT nNew)
/*N*/ {
/*N*/ 	if (nNew)					// setzen
/*N*/ 	{
/*N*/ 		if ( !pPaintLockData )
/*N*/ 			pPaintLockData = new ScPaintLockData(0);	//! Modus...
/*N*/ 		pPaintLockData->SetLevel(nNew-1, TRUE);
/*N*/ 		LockDocument_Impl(nNew);
/*N*/ 	}
/*N*/ 	else if (pPaintLockData)	// loeschen
/*N*/ 	{
/*N*/ 		pPaintLockData->SetLevel(0, TRUE);	// bei Unlock sofort ausfuehren
/*N*/ 		UnlockPaint_Impl(TRUE);					// jetzt
/*N*/ 		UnlockDocument_Impl(0);
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocShell::LockPaint()
/*N*/ {
/*N*/ 	LockPaint_Impl(FALSE);
/*N*/ }

/*N*/ void ScDocShell::UnlockPaint()
/*N*/ {
/*N*/ 	UnlockPaint_Impl(FALSE);
/*N*/ }

/*N*/ void ScDocShell::LockDocument()
/*N*/ {
/*N*/ 	LockPaint_Impl(TRUE);
/*N*/ 	LockDocument_Impl(nDocumentLock + 1);
/*N*/ }

/*N*/ void ScDocShell::UnlockDocument()
/*N*/ {
/*N*/ 	if (nDocumentLock)
/*N*/ 	{
/*N*/ 		UnlockPaint_Impl(TRUE);
/*N*/ 		UnlockDocument_Impl(nDocumentLock - 1);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("UnlockDocument without LockDocument");
/*N*/ }

//------------------------------------------------------------------


/*N*/ void ScDocShell::CalcOutputFactor()
/*N*/ {
/*N*/ 	if (bIsInplace)
/*N*/ 	{
/*N*/ 		nPrtToScreenFactor = 1.0;			// passt sonst nicht zur inaktiven Darstellung
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bTextWysiwyg = SC_MOD()->GetInputOptions().GetTextWysiwyg();
/*N*/ 	if (bTextWysiwyg)
/*N*/ 	{
/*N*/ 		nPrtToScreenFactor = 1.0;
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	String aTestString = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
/*N*/ 			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890123456789" ));
/*N*/ 	long nPrinterWidth = 0;
/*N*/ 	long nWindowWidth = 0;
/*N*/ 	const ScPatternAttr* pPattern = (const ScPatternAttr*)&aDocument.GetPool()->
/*N*/ 											GetDefaultItem(ATTR_PATTERN);
/*N*/ 
/*N*/ 	Font aDefFont;
/*N*/ 	Printer* pPrinter = GetPrinter();
/*N*/ 	if (pPrinter)
/*N*/ 	{
/*N*/ 		MapMode aOldMode = pPrinter->GetMapMode();
/*N*/ 		Font	aOldFont = pPrinter->GetFont();
/*N*/ 
/*N*/ 		pPrinter->SetMapMode(MAP_PIXEL);
/*N*/ 		pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, pPrinter);	// font color doesn't matter here
/*N*/ 		pPrinter->SetFont(aDefFont);
/*N*/ 		nPrinterWidth = pPrinter->PixelToLogic( Size( pPrinter->GetTextWidth(aTestString), 0 ),
/*N*/ 													MAP_100TH_MM ).Width();
/*N*/ 		pPrinter->SetFont(aOldFont);
/*N*/ 		pPrinter->SetMapMode(aOldMode);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("kein Drucker ?!?!?");
/*N*/ 
/*N*/ 	VirtualDevice aVirtWindow( *Application::GetDefaultDevice() );
/*N*/ 	aVirtWindow.SetMapMode(MAP_PIXEL);
/*N*/ 	pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, &aVirtWindow);	// font color doesn't matter here
/*N*/ 	aVirtWindow.SetFont(aDefFont);
/*N*/ 	nWindowWidth = aVirtWindow.GetTextWidth(aTestString);
/*N*/ 	nWindowWidth = (long) ( nWindowWidth / ScGlobal::nScreenPPTX * HMM_PER_TWIPS );
/*N*/ 
/*N*/ 	if (nPrinterWidth && nWindowWidth)
/*N*/ 		nPrtToScreenFactor = nPrinterWidth / (double) nWindowWidth;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR("GetTextSize gibt 0 ??");
/*N*/ 		nPrtToScreenFactor = 1.0;
/*N*/ 	}
/*N*/ }

/*N*/ double ScDocShell::GetOutputFactor() const
/*N*/ {
/*N*/ 	return nPrtToScreenFactor;
/*N*/ }

//---------------------------------------------------------------------

/*N*/ void ScDocShell::InitOptions()			// Fortsetzung von InitNew (CLOOKs)
/*N*/ {
/*N*/ 	//	Einstellungen aus dem SpellCheckCfg kommen in Doc- und ViewOptions
/*N*/ 
/*N*/ 	USHORT nDefLang, nCjkLang, nCtlLang;
/*N*/ 	BOOL bAutoSpell, bHideAuto;
/*N*/ 	ScModule::GetSpellSettings( nDefLang, nCjkLang, nCtlLang, bAutoSpell, bHideAuto );
/*N*/ 	ScModule* pScMod = SC_MOD();
/*N*/ 
/*N*/ 	ScDocOptions  aDocOpt  = pScMod->GetDocOptions();
/*N*/ 	ScViewOptions aViewOpt = pScMod->GetViewOptions();
/*N*/ 	aDocOpt.SetAutoSpell( bAutoSpell );
/*N*/ 	aViewOpt.SetHideAutoSpell( bHideAuto );
/*N*/ 
/*N*/ 	// zweistellige Jahreszahleneingabe aus Extras->Optionen->Allgemein->Sonstiges
/*N*/ 	aDocOpt.SetYear2000( SFX_APP()->GetMiscConfig()->GetYear2000() );
/*N*/ 
/*N*/ 	aDocument.SetDocOptions( aDocOpt );
/*N*/ 	aDocument.SetViewOptions( aViewOpt );
/*N*/ 
/*N*/ 	//	Druck-Optionen werden jetzt direkt vor dem Drucken gesetzt
/*N*/ 
/*N*/ 	aDocument.SetLanguage( (LanguageType) nDefLang, (LanguageType) nCjkLang, (LanguageType) nCtlLang );
/*N*/ }

//---------------------------------------------------------------------

/*N*/ Printer* ScDocShell::GetDocumentPrinter()		// fuer OLE
/*N*/ {
/*N*/ 	return aDocument.GetPrinter();
/*N*/ }

/*N*/ SfxPrinter* ScDocShell::GetPrinter()
/*N*/ {
/*N*/ 	return aDocument.GetPrinter();
/*N*/ }


/*N*/ USHORT ScDocShell::SetPrinter( SfxPrinter* pNewPrinter, USHORT nDiffFlags )
/*N*/ {
/*N*/ 	if (nDiffFlags & SFX_PRINTER_PRINTER)
/*N*/ 	{
/*N*/ 		if ( aDocument.GetPrinter() != pNewPrinter )
/*N*/ 		{
/*N*/ 			aDocument.SetPrinter( pNewPrinter );
/*N*/ 			aDocument.SetPrintOptions();
/*N*/ 
/*N*/ 			delete pFontList;
/*N*/ 			pFontList = new FontList( pNewPrinter, Application::GetDefaultDevice() );
/*N*/ 			SvxFontListItem aFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST );
/*N*/ 			PutItem( aFontListItem );
/*N*/ 
/*N*/ 			CalcOutputFactor();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if (nDiffFlags & SFX_PRINTER_JOBSETUP)
/*N*/ 	{
/*?*/ 		SfxPrinter* pOldPrinter = aDocument.GetPrinter();
/*?*/ 		if (pOldPrinter)
/*?*/ 		{
/*?*/ 			pOldPrinter->SetJobSetup( pNewPrinter->GetJobSetup() );
/*?*/ 
/*?*/ 			//	#i6706# Call SetPrinter with the old printer again, so the drawing layer
/*?*/ 			//	RefDevice is set (calling ReformatAllTextObjects and rebuilding charts),
/*?*/ 			//	because the JobSetup (printer device settings) may affect text layout.
/*?*/ 			aDocument.SetPrinter( pOldPrinter );
/*?*/ 			CalcOutputFactor();							// also with the new settings
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (nDiffFlags & SFX_PRINTER_OPTIONS)
/*N*/ 	{
/*N*/ 		aDocument.SetPrintOptions();		//! aus neuem Printer ???
/*N*/ 	}
/*N*/ 
/*N*/ 	if (nDiffFlags & (SFX_PRINTER_CHG_ORIENTATION | SFX_PRINTER_CHG_SIZE))
/*N*/ 	{
/*N*/ 		String aStyle = aDocument.GetPageStyle( GetCurTab() );
/*N*/ 		ScStyleSheetPool* pStPl = aDocument.GetStyleSheetPool();
/*N*/ 		SfxStyleSheet* pStyleSheet = (SfxStyleSheet*)pStPl->Find(aStyle, SFX_STYLE_FAMILY_PAGE);
/*N*/ 		if (pStyleSheet)
/*N*/ 		{
/*N*/ 			SfxItemSet& rSet = pStyleSheet->GetItemSet();
/*N*/ 
/*N*/ 			if (nDiffFlags & SFX_PRINTER_CHG_ORIENTATION)
/*N*/ 			{
/*N*/ 				const SvxPageItem& rOldItem = (const SvxPageItem&)rSet.Get(ATTR_PAGE);
/*N*/ 				BOOL bWasLand = rOldItem.IsLandscape();
/*N*/ 				BOOL bNewLand = ( pNewPrinter->GetOrientation() == ORIENTATION_LANDSCAPE );
/*N*/ 				if (bNewLand != bWasLand)
/*N*/ 				{
/*?*/ 					SvxPageItem aNewItem( rOldItem );
/*?*/ 					aNewItem.SetLandscape( bNewLand );
/*?*/ 					rSet.Put( aNewItem );
/*?*/ 
/*?*/ 					//	Groesse umdrehen
/*?*/ 					Size aOldSize = ((const SvxSizeItem&)rSet.Get(ATTR_PAGE_SIZE)).GetSize();
/*?*/ 					Size aNewSize(aOldSize.Height(),aOldSize.Width());
/*?*/ 					SvxSizeItem aNewSItem(ATTR_PAGE_SIZE,aNewSize);
/*?*/ 					rSet.Put( aNewSItem );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if (nDiffFlags & SFX_PRINTER_CHG_SIZE)
/*N*/ 			{
/*N*/ 				SvxSizeItem	aPaperSizeItem( ATTR_PAGE_SIZE, SvxPaperInfo::GetPaperSize(pNewPrinter) );
/*N*/ 				rSet.Put( aPaperSizeItem );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB,PAINT_ALL);
/*N*/ 
/*N*/ 	return 0;
/*N*/ }

//---------------------------------------------------------------------




//---------------------------------------------------------------------


//---------------------------------------------------------------------
//
//				Merge (Aenderungen zusammenfuehren)
//
//---------------------------------------------------------------------







}
