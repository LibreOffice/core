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

#include <vcl/metric.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#include <com/sun/star/i18n/WordType.hdl>
#include <breakit.hxx>
#include <viewsh.hxx>		// Bildschirmabgleich
#include <viewopt.hxx>		// Bildschirmabgleich abschalten, ViewOption
#include <fntcache.hxx>

#include <horiornt.hxx>

#include <swfont.hxx>       // CH_BLANK + CH_BULLET
#include <txtfrm.hxx>       // SwTxtFrm
#include <pagefrm.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>

// Enable this to use the helpclass SwRVPMark
#if OSL_DEBUG_LEVEL > 1
#endif
namespace binfilter {

// globale Variablen, werden in FntCache.Hxx bekanntgegeben
// Der FontCache wird in TxtInit.Cxx _TXTINIT erzeugt und in _TXTEXIT geloescht
SwFntCache *pFntCache = NULL;
// Letzter Font, der durch ChgFntCache eingestellt wurde.
SwFntObj *pLastFont = NULL;
// Die "MagicNumber", die den Fonts zur Identifizierung verpasst wird
BYTE* pMagicNo = NULL;

Color *pWaveCol = 0;

long SwFntObj::nPixWidth;
MapMode* SwFntObj::pPixMap = NULL;
OutputDevice* SwFntObj::pPixOut = NULL;

extern USHORT UnMapDirection( USHORT nDir, const BOOL bVertFormat );

#ifdef _RVP_MARK_HXX


#endif

/*************************************************************************
|*
|*	SwFntCache::Flush()
|*
|*	Ersterstellung		AMA 16. Dez. 94
|*	Letzte Aenderung	AMA 16. Dez. 94
|*
|*************************************************************************/

/*N*/ void SwFntCache::Flush( )
/*N*/ {
/*N*/ 	if ( pLastFont )
/*N*/ 	{
/*N*/ 		pLastFont->Unlock();
/*N*/ 		pLastFont = NULL;
/*N*/ 	}
/*N*/ 	SwCache::Flush( );
/*N*/ }

/*************************************************************************
|*
|*	SwFntObj::SwFntObj(), ~SwFntObj()
|*
|*	Ersterstellung		AMA 7. Nov. 94
|*	Letzte Aenderung	AMA 7. Nov. 94
|*
|*************************************************************************/

/*N*/ SwFntObj::SwFntObj( const SwSubFont &rFont, const void *pOwner, ViewShell *pSh ) :
/*N*/ 	SwCacheObj( (void*)pOwner ),
/*N*/ 	aFont( rFont ),
/*N*/ 	pScrFont( NULL ),
/*N*/ 	pPrtFont( &aFont ),
/*N*/ 	pPrinter( NULL ),
/*N*/ 	nPropWidth( rFont.GetPropWidth() )
/*N*/ {
/*N*/ 	nZoom = pSh ? pSh->GetViewOptions()->GetZoom() : USHRT_MAX;
/*N*/ 	nLeading = USHRT_MAX;
/*N*/ 	nPrtAscent = USHRT_MAX;
/*N*/ 	nPrtHeight = USHRT_MAX;
/*N*/ 	bPaintBlank = ( UNDERLINE_NONE != aFont.GetUnderline()
/*N*/ 				  || STRIKEOUT_NONE != aFont.GetStrikeout() )
/*N*/ 				  && !aFont.IsWordLineMode();
/*N*/ }

/*N*/ SwFntObj::~SwFntObj()
/*N*/ {
/*N*/ 	if ( pScrFont != pPrtFont )
/*N*/ 		delete pScrFont;
/*N*/ 	if ( pPrtFont != &aFont )
/*?*/ 		delete pPrtFont;
/*N*/ }

/*N*/ void SwFntObj::CreatePrtFont( const OutputDevice& rPrt )
/*N*/ {
/*N*/     if ( nPropWidth != 100 && pPrinter != &rPrt )
/*N*/     {
/*?*/         if( pScrFont != pPrtFont )
/*?*/             delete pScrFont;
/*?*/         if( pPrtFont != &aFont )
/*?*/             delete pPrtFont;
/*?*/ 
/*?*/         const Font aOldFnt( rPrt.GetFont() );
/*?*/         ((OutputDevice&)rPrt).SetFont( aFont );
/*?*/         const FontMetric aWinMet( rPrt.GetFontMetric() );
/*?*/         ((OutputDevice&)rPrt).SetFont( aOldFnt );
/*?*/         long nWidth = ( aWinMet.GetSize().Width() * nPropWidth ) / 100;
/*?*/ 
/*?*/         if( !nWidth )
/*?*/             ++nWidth;
/*?*/         pPrtFont = new Font( aFont );
/*?*/         pPrtFont->SetSize( Size( nWidth, aFont.GetSize().Height() ) );
/*?*/         pScrFont = NULL;
/*N*/     }
/*N*/ }

/*************************************************************************
 *
 *  USHORT SwFntObj::GetAscent( const OutputDevice *pOut )
 *
 *	Ersterstellung		AMA 7. Nov. 94
 *	Letzte Aenderung	AMA 7. Nov. 94
 *
 *  Beschreibung: liefern den Ascent des Fonts auf dem
 * 	gewuenschten Outputdevice zurueck, ggf. muss der Bildschirmfont erst
 *  erzeugt werden.
 *************************************************************************/

/*N*/ USHORT SwFntObj::GetAscent( const ViewShell *pSh, const OutputDevice *pOut )
/*N*/ {
/*N*/     // Condition for output font / refdev font adjustment
/*N*/     // 1. RefDef == OutDev (common printing, online layout)
/*N*/     // 2. Prospect/PagePreview pringing
/*N*/     // 3. PDF export from online layout
/*N*/     const OutputDevice* pRefDev = pOut;
/*N*/     if ( !pSh || ( pRefDev = &pSh->GetRefDev() ) == pOut ||
/*N*/          ( OUTDEV_PRINTER == pRefDev->GetOutDevType() &&
/*N*/            OUTDEV_PRINTER == pOut->GetOutDevType() ) ||
/*N*/          OUTDEV_WINDOW == pRefDev->GetOutDevType() )
/*N*/ 	{
/*N*/ 		if ( nPrtAscent == USHRT_MAX ) // DruckerAscent noch nicht bekannt?
/*N*/ 		{
/*N*/             CreatePrtFont( *pOut );
/*N*/ 			const Font aOldFnt( pRefDev->GetFont() );
/*N*/ 			((OutputDevice*)pRefDev)->SetFont( *pPrtFont );
/*N*/ 			const FontMetric aOutMet( pRefDev->GetFontMetric() );
/*N*/ 			nPrtAscent = (USHORT) aOutMet.GetAscent();
/*N*/ 			( (OutputDevice *)pRefDev )->SetFont( aOldFnt );
/*N*/ 		}
/*N*/ 		return nPrtAscent + nLeading;
/*N*/ 	}
/*N*/ 
/*N*/     CreateScrFont( pSh, *pOut );  // eventuell Bildschirmanpassung durchfuehren
/*N*/ 	return nScrAscent;
/*N*/ }

/*N*/ USHORT SwFntObj::GetHeight( const ViewShell *pSh, const OutputDevice *pOut )
/*N*/ {
/*N*/     // Condition for output font / refdev font adjustment
/*N*/     // 1. RefDef == OutDev (common printing, online layout)
/*N*/     // 2. Prospect/PagePreview pringing
/*N*/     // 3. PDF export from online layout
/*N*/     const OutputDevice* pRefDev = pOut;
/*N*/     if ( ! pSh || ( pRefDev = &pSh->GetRefDev() ) == pOut ||
/*N*/          ( OUTDEV_PRINTER == pRefDev->GetOutDevType() &&
/*N*/            OUTDEV_PRINTER == pOut->GetOutDevType() ) ||
/*N*/          OUTDEV_WINDOW == pRefDev->GetOutDevType()  )
/*N*/ 	{
/*N*/ 		if ( nPrtHeight == USHRT_MAX ) // PrinterHeight noch nicht bekannt?
/*N*/ 		{
/*N*/             CreatePrtFont( *pOut );
/*N*/ 			const Font aOldFnt( pRefDev->GetFont() );
/*N*/ 			((OutputDevice*)pRefDev)->SetFont( *pPrtFont );
/*N*/ 			nPrtHeight = (USHORT)pRefDev->GetTextHeight();
/*N*/ 			((OutputDevice *)pRefDev)->SetFont( aOldFnt );
/*N*/ 		}
/*N*/ 		return nPrtHeight + nLeading;
/*N*/ 	}
/*N*/ 
/*N*/     CreateScrFont( pSh, *pOut );  // eventuell Bildschirmanpassung durchfuehren
/*N*/ 	if ( nScrHeight == USHRT_MAX ) // ScreenHeight noch nicht bekannt?
/*N*/ 	{
/*N*/ 		const Font aOldFnt( pOut->GetFont() );
/*N*/ 		((OutputDevice*)pOut)->SetFont( *pPrtFont );
/*N*/ 		nScrHeight = (USHORT)pOut->GetTextHeight();
/*N*/ 		((OutputDevice*)pOut)->SetFont( aOldFnt );
/*N*/ 	}
/*N*/ 	return nScrHeight;
/*N*/ }

/*************************************************************************
 *
 *  SwFntObj::CreateScrFont( const ViewShell *pSh, const OutputDevice& rOut )
 *
 *	Ersterstellung		AMA 7. Nov. 94
 *	Letzte Aenderung	AMA 7. Nov. 94
 *
 *  pOut is the output device, not the reference device
 *
 *************************************************************************/

/*N*/ void SwFntObj::CreateScrFont( const ViewShell *pSh, const OutputDevice& rOut )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }


/*N*/ void SwFntObj::GuessLeading( const ViewShell *pSh, const FontMetric& rMet )
/*N*/ {
/*N*/ //  Wie waere es mit 50% des Descents (StarMath??):
/*N*/ //	nLeading = USHORT( aMet.GetDescent() / 2 );
/*N*/ 
/*N*/ #if defined(WNT) || defined(WIN) || defined(PM2)
/*N*/ 	OutputDevice *pWin = ( pSh && pSh->GetWin() ) ? pSh->GetWin() :
/*N*/ 						 GetpApp()->GetDefaultDevice();
/*N*/ 	if ( pWin )
/*N*/ 	{
/*N*/ 		MapMode aTmpMap( MAP_TWIP );
/*N*/ 		MapMode aOldMap = pWin->GetMapMode( );
/*N*/ 		pWin->SetMapMode( aTmpMap );
/*N*/ 		const Font aOldFnt( pWin->GetFont() );
/*N*/ 		pWin->SetFont( *pPrtFont );
/*N*/ 		const FontMetric aWinMet( pWin->GetFontMetric() );
/*N*/ 		const USHORT nWinHeight = USHORT( aWinMet.GetSize().Height() );
/*N*/ 		if( pPrtFont->GetName().Search( aWinMet.GetName() ) < USHRT_MAX )
/*N*/ 		{
/*N*/ 			// Wenn das Leading auf dem Window auch 0 ist, dann
/*N*/ 			// muss es auch so bleiben (vgl. StarMath!).
/*N*/ 			long nTmpLeading = (long)aWinMet.GetIntLeading();
/*N*/ 			 // einen Versuch haben wir noch wg. 31003:
/*N*/ 			if( nTmpLeading <= 0 )
/*N*/ 			{
/*N*/ 				pWin->SetFont( rMet );
/*N*/ 				nTmpLeading = (long)pWin->GetFontMetric().GetIntLeading();
/*N*/ 				if( nTmpLeading < 0 )
/*N*/ 					nLeading = 0;
/*N*/ 				else
/*N*/ 					nLeading = USHORT(nTmpLeading);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				nLeading = USHORT(nTmpLeading);
/*?*/ 				// Manta-Hack #50153#:
/*?*/ 				// Wer beim Leading luegt, luegt moeglicherweise auch beim
/*?*/ 				// Ascent/Descent, deshalb wird hier ggf. der Font ein wenig
/*?*/ 				// tiefergelegt, ohne dabei seine Hoehe zu aendern.
/*?*/ 				long nDiff = Min( rMet.GetDescent() - aWinMet.GetDescent(),
/*?*/ 					aWinMet.GetAscent() - rMet.GetAscent() - nTmpLeading );
/*?*/ 				if( nDiff > 0 )
/*?*/ 				{
/*?*/ 					ASSERT( nPrtAscent < USHRT_MAX, "GuessLeading: PrtAscent-Fault" );
/*?*/                   nPrtAscent += (USHORT)(( 2 * nDiff ) / 5);
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// Wenn alle Stricke reissen, nehmen wir 15% der
/*N*/ 			// Hoehe, ein von CL empirisch ermittelter Wert.
/*?*/ 			nLeading = (nWinHeight * 15) / 100;
/*N*/ 		}
/*N*/ 		pWin->SetFont( aOldFnt );
/*N*/ 		pWin->SetMapMode( aOldMap );
/*N*/ 	}
/*N*/ 	else
/*N*/ #endif
/*N*/ #ifdef MAC
/*N*/ 		nLeading = (pPrtFont->GetSize().Height() * 15) / 100;
/*N*/ #else
/*N*/ 		nLeading = 0;
/*N*/ #endif
/*N*/ }

/*************************************************************************
 *
 *	void SwFntObj::SetDeviceFont( const OutputDevice *pOut ),
 *
 *	Ersterstellung		AMA 7. Nov. 94
 *	Letzte Aenderung	AMA 7. Nov. 94
 *
 *  Beschreibung: stellt den Font am gewuenschten OutputDevice ein,
 *  am Bildschirm muss eventuell erst den Abgleich durchgefuehrt werden.
 *
 *************************************************************************/

/*N*/ void SwFntObj::SetDevFont( const ViewShell *pSh, OutputDevice *pOut )
/*N*/ {
/*N*/     // Condition for output font / refdev font adjustment
    // 1. RefDef == OutDev (common printing, online layout)
    // 2. Prospect/PagePreview pringing
    // 3. PDF export from online layout
/*N*/     const OutputDevice* pRefDev = pOut;
/*N*/     if ( pSh && ( pRefDev = &pSh->GetRefDev() ) != pOut &&
/*N*/          ( OUTDEV_PRINTER != pRefDev->GetOutDevType() ||
/*N*/            OUTDEV_PRINTER != pOut->GetOutDevType() ) &&
/*N*/          OUTDEV_WINDOW != pRefDev->GetOutDevType() )
/*N*/     {
/*?*/         CreateScrFont( pSh, *pOut );
/*?*/         if( !GetScrFont()->IsSameInstance( pOut->GetFont() ) )
/*?*/             pOut->SetFont( *pScrFont );
/*?*/         if( pPrinter && ( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) ) )
/*?*/             pPrinter->SetFont( *pPrtFont );
/*N*/     }
/*N*/     else
/*N*/ 	{
/*N*/         CreatePrtFont( *pOut );
/*N*/ 		if( !pPrtFont->IsSameInstance( pOut->GetFont() ) )
/*N*/ 			pOut->SetFont( *pPrtFont );
/*N*/ 		if ( nLeading == USHRT_MAX )
/*N*/ 		{
/*N*/ 			FontMetric aMet( pOut->GetFontMetric() );
/*N*/ 			bSymbol = RTL_TEXTENCODING_SYMBOL == aMet.GetCharSet();
/*N*/ 			long nTmpLead = (long)aMet.GetIntLeading();
/*N*/ 			if ( nTmpLead < 5 )
/*N*/ 			{
/*N*/ 				GetAscent( pSh, pOut );
/*N*/ 				GuessLeading( pSh, aMet );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nLeading = 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

#define WRONG_SHOW_MIN 5
#define WRONG_SHOW_SMALL 11
#define WRONG_SHOW_MEDIUM 15

/*************************************************************************
 *
 * void SwFntObj::DrawText( ... )
 *
 *	Ersterstellung		AMA 16. Dez. 94
 *	Letzte Aenderung	AMA 16. Dez. 94
 *
 *  Beschreibung: Textausgabe
 * 					auf dem Bildschirm 			=> DrawTextArray
 * 					auf dem Drucker, !Kerning 	=> DrawText
 * 					auf dem Drucker + Kerning	=> DrawStretchText
 *
 *************************************************************************/


/*N*/ sal_Bool lcl_IsMonoSpaceFont( const OutputDevice* pOut )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return sal_False; //STRIP001 
/*N*/ }

// ER 09.07.95 20:34
// mit -Ox Optimierung stuerzt's unter win95 ab
// JP 12.07.95: unter WNT auch (i386);       Alpha ??
// global optimization off
#ifdef _MSC_VER
#pragma optimize("g",off)
#endif



// Optimierung war fuer DrawText() ausgeschaltet
#ifdef _MSC_VER
#pragma optimize("",on)
#endif


/*************************************************************************
 *
 *	Size SwFntObj::GetTextSize( const OutputDevice *pOut, const String &rTxt,
 *			 const USHORT nIdx, const USHORT nLen, const short nKern = 0 );
 *
 *	Ersterstellung		AMA 16. Dez. 94
 *	Letzte Aenderung	AMA 16. Dez. 94
 *
 *  Beschreibung: ermittelt die TextSize (des Druckers)
 *
 *************************************************************************/

/*N*/ Size SwFntObj::GetTextSize( SwDrawTextInfo& rInf )
/*N*/ {
/*N*/ 	Size aTxtSize;
/*N*/ 	const xub_StrLen nLn = ( STRING_LEN != rInf.GetLen() ) ? rInf.GetLen() :
/*N*/ 						   rInf.GetText().Len();
/*N*/ 
/*N*/     // be sure to have the correct layout mode at the printer
/*N*/     if ( pPrinter )
/*N*/     {
/*N*/         pPrinter->SetLayoutMode( rInf.GetpOut()->GetLayoutMode() );
/*N*/         pPrinter->SetDigitLanguage( rInf.GetpOut()->GetDigitLanguage() );
/*N*/     }
/*N*/ 
/*N*/     if ( rInf.GetFrm() && nLn && rInf.SnapToGrid() && rInf.GetFont() &&
/*N*/          SW_CJK == rInf.GetFont()->GetActual() )
/*N*/     {
/*?*/         GETGRID( rInf.GetFrm()->FindPageFrm() )
/*?*/         if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() )
/*?*/         {
/*?*/             const USHORT nGridWidth = pGrid->GetBaseHeight();
/*?*/ 
/*?*/             OutputDevice* pOutDev;
/*?*/ 
/*?*/             if ( pPrinter )
/*?*/             {
/*?*/                 if( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) )
/*?*/                     pPrinter->SetFont(*pPrtFont);
/*?*/                 pOutDev = pPrinter;
/*?*/             }
/*?*/             else
/*?*/                 pOutDev = rInf.GetpOut();
/*?*/ 
/*?*/             aTxtSize.Width() =
/*?*/                     pOutDev->GetTextWidth( rInf.GetText(), rInf.GetIdx(), nLn );
/*?*/             aTxtSize.Height() = pOutDev->GetTextHeight() + nLeading;
/*?*/ 
/*?*/             long nWidthPerChar = aTxtSize.Width() / nLn;
/*?*/ 
/*?*/             const ULONG i = nWidthPerChar ?
/*?*/                                 ( nWidthPerChar - 1 ) / nGridWidth + 1:
/*?*/                                 1;
/*?*/ 
/*?*/             aTxtSize.Width() = i * nGridWidth * nLn;
/*?*/ 
/*?*/             rInf.SetKanaDiff( 0 );
/*?*/             return aTxtSize;
/*?*/         }
/*N*/     }
/*N*/ 
/*N*/     const BOOL bCompress = rInf.GetKanaComp() && nLn &&
/*N*/                            rInf.GetFont() &&
/*N*/                            SW_CJK == rInf.GetFont()->GetActual() &&
/*N*/                            rInf.GetScriptInfo() &&
/*N*/                            rInf.GetScriptInfo()->CountCompChg() &&
/*N*/                      lcl_IsMonoSpaceFont( rInf.GetpOut() );
/*N*/ 
/*N*/ 	ASSERT(	!bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
/*N*/ 			CountCompChg()), "Compression without info" );
/*N*/ 
/*N*/     // This is the part used e.g., for cursor travelling
/*N*/     // See condition for DrawText or DrawTextArray (bDirectPrint)
/*N*/     if ( pPrinter && pPrinter != rInf.GetpOut() )
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         if( !pPrtFont->IsSameInstance( rInf.GetpOut()->GetFont() ) )
/*N*/             rInf.GetpOut()->SetFont( *pPrtFont );
/*N*/ 		if( bCompress )
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             aTxtSize.Width() = rInf.GetpOut()->GetTextWidth( rInf.GetText(),
/*N*/                                                              rInf.GetIdx(), nLn );
/*N*/ 			rInf.SetKanaDiff( 0 );
/*N*/ 		}
/*N*/ 
/*N*/         aTxtSize.Height() = rInf.GetpOut()->GetTextHeight();
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( rInf.GetKern() && nLn )
/*N*/ 		aTxtSize.Width() += ( nLn - 1 ) * long( rInf.GetKern() );
/*N*/ 
/*N*/ 	aTxtSize.Height() += nLeading;
/*N*/ 	return aTxtSize;
/*N*/ }




/*************************************************************************
|*
|*	SwFntAccess::SwFntAccess()
|*
|*	Ersterstellung		AMA 9. Nov. 94
|*	Letzte Aenderung	AMA 9. Nov. 94
|*
|*************************************************************************/

/*N*/ SwFntAccess::SwFntAccess( const void* &rMagic,
/*N*/ 				USHORT &rIndex, const void *pOwner, ViewShell *pSh,
/*N*/ 				BOOL bCheck ) :
/*N*/   SwCacheAccess( *pFntCache, rMagic, rIndex ),
/*N*/   pShell( pSh )
/*N*/ {
/*N*/ 	// Der benutzte CTor von SwCacheAccess sucht anhand rMagic+rIndex im Cache
/*N*/ 	if ( IsAvail() )
/*N*/ 	{
/*N*/ 		// Der schnellste Fall: ein bekannter Font ( rMagic ),
/*N*/ 		// bei dem Drucker und Zoom nicht ueberprueft werden brauchen.
/*N*/ 		if ( !bCheck )
/*N*/ 			return;
/*N*/ 
/*N*/ 		// Hier ist zwar der Font bekannt, muss aber noch ueberprueft werden.
/*N*/ 
/*N*/ 	}
/*N*/ 	else
/*N*/ 		// Hier ist der Font nicht bekannt, muss also gesucht werden.
/*N*/ 		bCheck = FALSE;
/*N*/ 
/*N*/ 
/*N*/     {
/*N*/         OutputDevice* pOut = 0;
/*N*/ 		USHORT nZoom = USHRT_MAX;
/*N*/ 
/*N*/         // Get the reference device
/*N*/         if ( pSh )
/*N*/         {
/*N*/             pOut = &pSh->GetRefDev();
/*N*/             nZoom = pSh->GetViewOptions()->GetZoom();
/*N*/         }
/*N*/ 
/*N*/ 		SwFntObj *pFntObj;
/*N*/ 		if ( bCheck )
/*N*/ 		{
/*N*/             pFntObj = Get();
/*N*/ 			if ( ( pFntObj->GetZoom( ) == nZoom ) &&
/*N*/ 				 ( pFntObj->pPrinter == pOut ) &&
/*N*/ 				   pFntObj->GetPropWidth() ==
/*N*/ 				   		((SwSubFont*)pOwner)->GetPropWidth() )
/*N*/ 				return; // Die Ueberpruefung ergab: Drucker+Zoom okay.
/*N*/ 			pFntObj->Unlock( ); // Vergiss dies Objekt, es wurde leider
/*N*/ 			pObj = NULL;	 	// eine Drucker/Zoomaenderung festgestellt.
/*N*/ 		}
/*N*/ 
/*N*/         // Search by font comparison, quite expensive!
/*N*/         // Look for same font and same printer
/*N*/         pFntObj = pFntCache->First();
/*N*/ 		while ( pFntObj && !( pFntObj->aFont == *(Font *)pOwner &&
/*N*/                               pFntObj->GetZoom() == nZoom &&
/*N*/                               pFntObj->GetPropWidth() ==
/*N*/                               ((SwSubFont*)pOwner)->GetPropWidth() &&
/*N*/                               ( !pFntObj->pPrinter || pFntObj->pPrinter == pOut ) ) )
/*N*/ 			pFntObj = pFntCache->Next( pFntObj );
/*N*/ 
/*N*/ 		if( pFntObj && pFntObj->pPrinter != pOut )
/*N*/ 		{
/*N*/ 			// Wir haben zwar einen ohne Drucker gefunden, mal sehen, ob es
/*N*/ 			// auch noch einen mit identischem Drucker gibt.
/*N*/ 			SwFntObj *pTmpObj = pFntObj;
/*N*/ 			while( pTmpObj && !( pTmpObj->aFont == *(Font *)pOwner &&
/*N*/ 				   pTmpObj->GetZoom()==nZoom && pTmpObj->pPrinter==pOut &&
/*N*/ 				   pTmpObj->GetPropWidth() ==
/*N*/ 				   		((SwSubFont*)pOwner)->GetPropWidth() ) )
/*N*/ 				pTmpObj = pFntCache->Next( pTmpObj );
/*N*/ 			if( pTmpObj )
/*N*/ 				pFntObj = pTmpObj;
/*N*/ 		}
/*N*/ 
/*N*/         if ( !pFntObj ) // Font has not been found, create one
/*N*/ 		{
/*N*/ 			// Das Objekt muss neu angelegt werden, deshalb muss der Owner ein
/*N*/ 			// SwFont sein, spaeter wird als Owner die "MagicNumber" gehalten.
/*N*/ 			SwCacheAccess::pOwner = pOwner;
/*N*/ 			pFntObj = Get(); // hier wird via NewObj() angelegt und gelockt.
/*N*/ 			ASSERT(pFntObj, "No Font, no Fun.");
/*N*/ 		}
/*N*/         else  // Font has been found, so we lock it.
/*N*/ 		{
/*N*/ 			pFntObj->Lock();
/*N*/ 			if( pFntObj->pPrinter != pOut ) // Falls bis dato kein Drucker bekannt
/*N*/ 			{
/*N*/ 				ASSERT( !pFntObj->pPrinter, "SwFntAccess: Printer Changed" );
/*N*/                 pFntObj->CreatePrtFont( *pOut );
/*N*/ 				pFntObj->pPrinter = pOut;
/*N*/ 				pFntObj->pScrFont = NULL;
/*N*/ 				pFntObj->nLeading = USHRT_MAX;
/*N*/ 				pFntObj->nPrtAscent = USHRT_MAX;
/*N*/ 				pFntObj->nPrtHeight = USHRT_MAX;
/*N*/ 			}
/*N*/ 			pObj = pFntObj;
/*N*/ 		}
/*N*/ 
/*N*/ 		// egal, ob neu oder gefunden, ab jetzt ist der Owner vom Objekt eine
/*N*/ 		// MagicNumber und wird auch dem aufrufenden SwFont bekanntgegeben,
/*N*/ 		// ebenso der Index fuer spaetere direkte Zugriffe
/*N*/ 		rMagic = pFntObj->GetOwner();
/*N*/ 		SwCacheAccess::pOwner = rMagic;
/*N*/ 		rIndex = pFntObj->GetCachePos();
/*N*/ 	}
/*N*/ }

/*N*/ SwCacheObj *SwFntAccess::NewObj( )
/*N*/ {
/*N*/ 	// Ein neuer Font, eine neue "MagicNumber".
/*N*/ 	return new SwFntObj( *(SwSubFont *)pOwner, ++pMagicNo, pShell );
/*N*/ }

/*N*/ extern xub_StrLen lcl_CalcCaseMap( const SwFont& rFnt,
/*N*/                                    const XubString& rOrigString,
/*N*/                                    xub_StrLen nOfst,
/*N*/                                    xub_StrLen nLen,
/*N*/                                    xub_StrLen nIdx );

/*N*/ xub_StrLen SwFont::GetTxtBreak( SwDrawTextInfo& rInf, long nTextWidth )
/*N*/ {
/*N*/  	ChgFnt( rInf.GetShell(), rInf.GetpOut() );
/*N*/ 
/*N*/     const BOOL bCompress = rInf.GetKanaComp() && rInf.GetLen() &&
/*N*/                            SW_CJK == GetActual() &&
/*N*/                            rInf.GetScriptInfo() &&
/*N*/                            rInf.GetScriptInfo()->CountCompChg() &&
/*N*/                      lcl_IsMonoSpaceFont( rInf.GetpOut() );
/*N*/ 
/*N*/     ASSERT( !bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
/*N*/             CountCompChg()), "Compression without info" );
/*N*/ 
/*N*/ 	USHORT nTxtBreak = 0;
/*N*/ 	long nKern = 0;
/*N*/ 
/*N*/ 	USHORT nLn = ( rInf.GetLen() == STRING_LEN ? rInf.GetText().Len()
/*N*/ 											   : rInf.GetLen() );
/*N*/ 
/*N*/     if ( rInf.GetFrm() && nLn && rInf.SnapToGrid() &&
/*N*/          rInf.GetFont() && SW_CJK == rInf.GetFont()->GetActual() )
/*N*/     {
/*?*/         GETGRID( rInf.GetFrm()->FindPageFrm() )
/*?*/         if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() )
/*?*/         {
/*?*/             const USHORT nGridWidth = pGrid->GetBaseHeight();
/*?*/ 
/*?*/             sal_Int32* pKernArray = new sal_Int32[rInf.GetLen()];
/*?*/             rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
/*?*/                                         rInf.GetIdx(), rInf.GetLen() );
/*?*/ 
/*?*/             long nWidthPerChar = pKernArray[ rInf.GetLen() - 1 ] / rInf.GetLen();
/*?*/ 
/*?*/             const ULONG i = nWidthPerChar ?
/*?*/                                 ( nWidthPerChar - 1 ) / nGridWidth + 1:
/*?*/                                 1;
/*?*/ 
/*?*/             nWidthPerChar = i * nGridWidth;
/*?*/             long nCurrPos = nWidthPerChar;
/*?*/ 
/*?*/             while( nTxtBreak < rInf.GetLen() && nTextWidth >= nCurrPos )
/*?*/             {
/*?*/                 nCurrPos += nWidthPerChar;
/*?*/                 ++nTxtBreak;
/*?*/             }
/*?*/ 
/*?*/             delete[] pKernArray;
/*?*/             return nTxtBreak + rInf.GetIdx();
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     if( aSub[nActual].IsCapital() && nLn )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 nTxtBreak = GetCapitalBreak( rInf.GetShell(), rInf.GetpOut(),
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nKern = CheckKerning();
/*N*/ 
/*N*/         const XubString* pTmpText;
/*N*/         XubString aTmpText;
/*N*/         xub_StrLen nTmpIdx;
/*N*/         xub_StrLen nTmpLen;
/*N*/         bool bTextReplaced = false;
/*N*/ 
/*N*/         if ( !aSub[nActual].IsCaseMap() )
/*N*/         {
/*N*/             pTmpText = &rInf.GetText();
/*N*/             nTmpIdx = rInf.GetIdx();
/*N*/             nTmpLen = nLn;
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/ 			const XubString aSnippet( rInf.GetText(), rInf.GetIdx(), nLn );
/*N*/             aTmpText = aSub[nActual].CalcCaseMap( aSnippet );
/*N*/             const bool bTitle = SVX_CASEMAP_TITEL == aSub[nActual].GetCaseMap() &&
/*N*/                                 pBreakIt->xBreak.is();
/*N*/ 
/*N*/             // Uaaaaahhhh!!! In title case mode, we would get wrong results
/*N*/             if ( bTitle && nLn )
/*N*/             {
/*N*/                 // check if rInf.GetIdx() is begin of word
/*N*/                 if ( !pBreakIt->xBreak->isBeginWord(
/*N*/                      rInf.GetText(), rInf.GetIdx(),
/*N*/                      pBreakIt->GetLocale( aSub[nActual].GetLanguage() ),
/*N*/                      ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) )
/*N*/                 {
/*N*/                     // In this case, the beginning of aTmpText is wrong.
/*N*/                     XubString aSnippetTmp( aSnippet, 0, 1 );
/*N*/                     aSnippetTmp = aSub[nActual].CalcCaseMap( aSnippetTmp );
/*N*/                     aTmpText.Erase( 0, aSnippetTmp.Len() );
/*N*/                     aTmpText.Insert( aSnippet.GetChar( 0 ), 0 );
/*N*/                 }
/*N*/             }
/*N*/ 
/*N*/             pTmpText = &aTmpText;
/*N*/             nTmpIdx = 0;
/*N*/             nTmpLen = aTmpText.Len();
/*N*/             bTextReplaced = true;
/*N*/         }
/*N*/ 
/*N*/        	if( rInf.GetHyphPos() )
/*N*/     		nTxtBreak = rInf.GetpOut()->GetTextBreak( *pTmpText, nTextWidth,
/*N*/                                                       '-', *rInf.GetHyphPos(),
/*N*/                                                       nTmpIdx, nTmpLen, nKern );
/*N*/         else
/*N*/        		nTxtBreak = rInf.GetpOut()->GetTextBreak( *pTmpText, nTextWidth,
/*N*/                                                       nTmpIdx, nTmpLen, nKern );
/*N*/ 
/*N*/         if ( bTextReplaced && STRING_LEN != nTxtBreak )
/*N*/         {
/*N*/             if ( nTmpLen != nLn )
/*N*/                 nTxtBreak = lcl_CalcCaseMap( *this, rInf.GetText(),
/*N*/                                              rInf.GetIdx(), nLn, nTxtBreak );
/*N*/             else
/*N*/                 nTxtBreak += rInf.GetIdx();
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/     if ( ! bCompress )
/*N*/         return nTxtBreak;
/*N*/ 
/*N*/     nTxtBreak -= rInf.GetIdx();
/*N*/ 
/*N*/     if( nTxtBreak < nLn )
/*N*/ 	{
/*N*/         if( !nTxtBreak && nLn )
/*N*/ 			nLn = 1;
/*N*/ 		else if( nLn > 2 * nTxtBreak )
/*N*/ 			nLn = 2 * nTxtBreak;
/*N*/ 		sal_Int32 *pKernArray = new sal_Int32[ nLn ];
/*N*/ 		rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
/*N*/ 									rInf.GetIdx(), nLn );
/*N*/         if( rInf.GetScriptInfo()->Compress( pKernArray, rInf.GetIdx(), nLn,
/*N*/         					rInf.GetKanaComp(), (USHORT)GetHeight( nActual ) ) )
/*N*/ 		{
/*N*/ 			long nKernAdd = nKern;
/*N*/ 			xub_StrLen nTmpBreak = nTxtBreak;
/*N*/ 			if( nKern && nTxtBreak )
/*N*/ 				nKern *= nTxtBreak - 1;
/*N*/ 			while( nTxtBreak<nLn && nTextWidth >= pKernArray[nTxtBreak] +nKern )
/*N*/ 			{
/*N*/ 				nKern += nKernAdd;
/*N*/ 				++nTxtBreak;
/*N*/ 			}
/*N*/ 			if( rInf.GetHyphPos() )
/*N*/ 				*rInf.GetHyphPos() += nTxtBreak - nTmpBreak; // It's not perfect
/*N*/ 		}
/*N*/ 		delete[] pKernArray;
/*N*/     }
/*N*/     nTxtBreak += rInf.GetIdx();
/*N*/ 
/*N*/     return nTxtBreak;
/*N*/ }

extern Color aGlobalRetoucheColor;


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
