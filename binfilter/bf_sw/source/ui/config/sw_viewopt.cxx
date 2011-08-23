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

#ifndef _OFF_APP_HXX //autogen
#include <bf_offmgr/app.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <bf_svx/svxids.hrc>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <bf_svtools/colorcfg.hxx>
#endif
#ifndef _AUTHRATR_HXX
#include "authratr.hxx"
#endif
namespace binfilter {


/*N*/ #ifdef DBG_UTIL
/*N*/ BOOL   SwViewOption::bTest9 = FALSE;		//DrawingLayerNotLoading
/*N*/ #endif
/*N*/ Color SwViewOption::aDocBoundColor(COL_LIGHTGRAY);
/*N*/ Color SwViewOption::aObjectBoundColor(COL_LIGHTGRAY);
/*N*/ Color SwViewOption::aDocColor(COL_LIGHTGRAY);
/*N*/ Color SwViewOption::aAppBackgroundColor(COL_LIGHTGRAY);
/*N*/ Color SwViewOption::aTableBoundColor(COL_LIGHTGRAY);
/*N*/ Color SwViewOption::aIndexShadingsColor(COL_LIGHTGRAY);
/*N*/ Color SwViewOption::aLinksColor(COL_BLUE);
/*N*/ Color SwViewOption::aVisitedLinksColor(COL_RED);
/*N*/ Color SwViewOption::aDirectCursorColor(COL_BLUE);
/*N*/ Color SwViewOption::aTextGridColor(COL_LIGHTGRAY);
/*N*/ Color SwViewOption::aSpellColor(COL_LIGHTRED);
/*N*/ Color SwViewOption::aFontColor(COL_BLACK);
/*N*/ Color SwViewOption::aFieldShadingsColor(COL_LIGHTGRAY);
/*N*/ Color SwViewOption::aSectionBoundColor(COL_LIGHTGRAY);
/*M*/ Color SwViewOption::aPageBreakColor(COL_BLUE);
/*N*/ Color SwViewOption::aScriptIndicatorColor(COL_GREEN);
/*N*/
/*N*/ sal_Int32 SwViewOption::nAppearanceFlags = VIEWOPT_DOC_BOUNDARIES|VIEWOPT_OBJECT_BOUNDARIES;
/*N*/ USHORT SwViewOption::nPixelTwips = 0;   //ein Pixel auf dem Bildschirm
/*N*/

#define LINEBREAK_SIZE 12, 8
#define TAB_SIZE 12, 6

#define MIN_BLANKWIDTH		 40
#define MIN_BLANKHEIGHT 	 40
#define MIN_TABWIDTH		120
#define MIN_TABHEIGHT		200

static const char __FAR_DATA aPostItStr[] = "  ";

/*************************************************************************
 *					  SwViewOption::IsEqualFlags()
 *************************************************************************/


/*************************************************************************
 *					  class SwPxlToTwips
 *************************************************************************/


/*************************************************************************
 *					  SwPxlToTwips::CTOR
 *************************************************************************/


/*************************************************************************
 *					  SwPxlToTwips::DTOR
 *************************************************************************/


/*************************************************************************
 *					  SwPxlToTwips::DrawLine
 *************************************************************************/


/*************************************************************************
 *					  SwViewOption::DrawRect()
 *************************************************************************/


/*************************************************************************
 *					  SwViewOption::DrawRectPrinter()
 *************************************************************************/


/*************************************************************************
 *					  SwViewOption::PaintTab()
 *************************************************************************/
/*************************************************************************
 *					  SwViewOption::GetLineBreakWidth()
 *************************************************************************/


/*************************************************************************
 *					  SwViewOption::PaintLineBreak()
 *************************************************************************/
/*************************************************************************
 *					  SwViewOption::GetPostItsWidth()
 *************************************************************************/


/*************************************************************************
 *					  SwViewOption::PaintPostIts()
 *************************************************************************/



/*************************************************************************
|*
|*	ViewOption::ViewOption()
|*
|*	Letzte Aenderung	MA 04. Aug. 93
|*
|*************************************************************************/

/*M*/ SwViewOption::SwViewOption() :
/*M*/ 	nZoom( 100 ),
/*M*/ 	nPagePrevRow( 1 ),
/*M*/ 	nPagePrevCol( 2 ),
/*M*/ 	eZoom( 0 ),
/*M*/ 	nTblDest(TBL_DEST_CELL),
/*M*/ 	bReadonly(FALSE),
/*M*/     bSelectionInReadonly(FALSE),
/*M*/ 	aRetoucheColor( COL_TRANSPARENT ),
/*M*/ 	nShdwCrsrFillMode( FILL_TAB ),
/*M*/     bStarOneSetting(FALSE),
/*M*/     bIsPagePreview(FALSE)
/*M*/ {
/*M*/ 	// Initialisierung ist jetzt etwas einfacher
/*M*/ 	// alle Bits auf 0
/*M*/ 	nCoreOptions = 	VIEWOPT_1_IDLE | VIEWOPT_1_HARDBLANK | VIEWOPT_1_SOFTHYPH |
/*M*/                     VIEWOPT_1_REF |
/*M*/                     VIEWOPT_1_GRAPHIC |
/*M*/ 					VIEWOPT_1_TABLE	   | VIEWOPT_1_DRAW | VIEWOPT_1_CONTROL	|
/*M*/                     VIEWOPT_1_PAGEBACK |
/*M*/ 					VIEWOPT_1_SOLIDMARKHDL | VIEWOPT_1_POSTITS;
/*M*/     nCore2Options = VIEWOPT_CORE2_BLACKFONT | VIEWOPT_CORE2_HIDDENPARA;
/*M*/     nUIOptions    = VIEWOPT_2_MODIFIED | VIEWOPT_2_EXECHYPERLINKS | VIEWOPT_2_GRFKEEPZOOM |VIEWOPT_2_ANY_RULER;
/*M*/
/*M*/ 	if(MEASURE_METRIC != GetAppLocaleData().getMeasurementSystemEnum())
/*M*/ 		aSnapSize.Width() = aSnapSize.Height() = 720;	// 1/2"
/*M*/ 	else
/*M*/ 		aSnapSize.Width() = aSnapSize.Height() = 567;	// 1 cm
/*M*/ 	nDivisionX = nDivisionY = 1;
/*M*/
/*M*/     bSelectionInReadonly = true;
/*M*/
/*M*/ #ifdef DBG_UTIL
/*M*/ 	// korrespondieren zu den Angaben in ui/config/cfgvw.src
/*M*/ 	bTest1 = bTest2 = bTest3 = bTest4 =
/*M*/ 			 bTest5 = bTest6 = bTest7 = bTest8 = bTest10 = FALSE;
/*M*/ #endif
/*M*/ }


/*N*/ SwViewOption::SwViewOption(const SwViewOption& rVOpt)
/*N*/ {
/*N*/ 	bReadonly = FALSE;
/*M*/     bSelectionInReadonly = FALSE;
/*N*/ 	nZoom   		= rVOpt.nZoom   	;
/*N*/ 	aSnapSize   	= rVOpt.aSnapSize   ;
/*N*/ 	nDivisionX  	= rVOpt.nDivisionX  ;
/*N*/ 	nDivisionY  	= rVOpt.nDivisionY  ;
/*N*/ 	nPagePrevRow	= rVOpt.nPagePrevRow;
/*N*/ 	nPagePrevCol	= rVOpt.nPagePrevCol;
/*N*/     bIsPagePreview  = rVOpt.bIsPagePreview;
/*N*/ 	eZoom      		= rVOpt.eZoom       ;
/*N*/ 	nTblDest    	= rVOpt.nTblDest    ;
/*N*/ 	nUIOptions		= rVOpt.nUIOptions  ;
/*N*/ 	nCoreOptions	= rVOpt.nCoreOptions  ;
/*N*/ 	nCore2Options	= rVOpt.nCore2Options  ;
/*N*/ 	aRetoucheColor	= rVOpt.GetRetoucheColor();
/*N*/ 	sSymbolFont 	= rVOpt.sSymbolFont;
/*N*/ 	nShdwCrsrFillMode = rVOpt.nShdwCrsrFillMode;
/*N*/ 	bStarOneSetting = rVOpt.bStarOneSetting;
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/ 	bTest1          = rVOpt.bTest1      ;
/*N*/ 	bTest2          = rVOpt.bTest2      ;
/*N*/ 	bTest3          = rVOpt.bTest3      ;
/*N*/ 	bTest4          = rVOpt.bTest4      ;
/*N*/ 	bTest5          = rVOpt.bTest5      ;
/*N*/ 	bTest6          = rVOpt.bTest6      ;
/*N*/ 	bTest7          = rVOpt.bTest7      ;
/*N*/ 	bTest8          = rVOpt.bTest8      ;
/*N*/ 	bTest10         = rVOpt.bTest10     ;
/*N*/ #endif
/*N*/ }


/*N*/ SwViewOption& SwViewOption::operator=( const SwViewOption &rVOpt )
/*N*/ {
/*N*/ 	nZoom   		= rVOpt.nZoom   	;
/*N*/ 	aSnapSize   	= rVOpt.aSnapSize   ;
/*N*/ 	nDivisionX  	= rVOpt.nDivisionX  ;
/*N*/ 	nDivisionY  	= rVOpt.nDivisionY  ;
/*N*/ 	nPagePrevRow	= rVOpt.nPagePrevRow;
/*N*/ 	nPagePrevCol	= rVOpt.nPagePrevCol;
/*N*/     bIsPagePreview  = rVOpt.bIsPagePreview;
/*N*/ 	eZoom      		= rVOpt.eZoom       ;
/*N*/ 	nTblDest    	= rVOpt.nTblDest    ;
/*N*/ 	nUIOptions		= rVOpt.nUIOptions  ;
/*N*/ 	nCoreOptions	= rVOpt.nCoreOptions;
/*N*/ 	nCore2Options	= rVOpt.nCore2Options;
/*N*/ 	aRetoucheColor	= rVOpt.GetRetoucheColor();
/*N*/ 	sSymbolFont 	= rVOpt.sSymbolFont;
/*N*/ 	nShdwCrsrFillMode = rVOpt.nShdwCrsrFillMode;
/*N*/ 	bStarOneSetting = rVOpt.bStarOneSetting;
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/ 	bTest1          = rVOpt.bTest1      ;
/*N*/ 	bTest2          = rVOpt.bTest2      ;
/*N*/ 	bTest3          = rVOpt.bTest3      ;
/*N*/ 	bTest4          = rVOpt.bTest4      ;
/*N*/ 	bTest5          = rVOpt.bTest5      ;
/*N*/ 	bTest6          = rVOpt.bTest6      ;
/*N*/ 	bTest7          = rVOpt.bTest7      ;
/*N*/ 	bTest8          = rVOpt.bTest8      ;
/*N*/ 	bTest10         = rVOpt.bTest10     ;
/*N*/ #endif
/*N*/ 	return *this;
/*N*/ }


/*N*/ SwViewOption::~SwViewOption()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	ViewOption::Init()
|*
|*	Letzte Aenderung	MA 04. Aug. 93
|*
|*************************************************************************/

/*N*/ void SwViewOption::Init( Window *pWin )
/*N*/ {
/*N*/ 	if( !nPixelTwips && pWin )
/*N*/ 	{
/*N*/ 		nPixelTwips = (USHORT)pWin->PixelToLogic( Size(1,1) ).Height();
/*N*/ 	}
/*N*/ }

/*************************************************************************/
/*
/*************************************************************************/

/*N*/ AuthorCharAttr::AuthorCharAttr() :
/*N*/ 	nItemId	(SID_ATTR_CHAR_UNDERLINE),
/*N*/ 	nAttr	(UNDERLINE_SINGLE),
/*N*/ 	nColor	(COL_TRANSPARENT)
/*N*/ {
/*N*/ }

/*-----------------07.01.97 13.50-------------------

--------------------------------------------------*/

/*N*/ USHORT		GetHtmlMode(const SwDocShell* pShell)
/*N*/ {
/*N*/ 	USHORT nRet = 0;
/*N*/ 	if(!pShell || PTR_CAST(SwWebDocShell, pShell))
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/* -----------------------------23.04.2002 17:41------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwViewOption::ApplyColorConfigValues(const ColorConfig& rConfig )
/*N*/ {
/*N*/     aDocColor.SetColor(rConfig.GetColorValue(DOCCOLOR).nColor);
/*N*/
/*N*/     ColorConfigValue aValue = rConfig.GetColorValue(DOCBOUNDARIES);
/*N*/     aDocBoundColor.SetColor(aValue.nColor);
/*N*/     nAppearanceFlags = 0;
/*N*/     if(aValue.bIsVisible)
/*N*/         nAppearanceFlags |= VIEWOPT_DOC_BOUNDARIES;
/*N*/
/*N*/     aAppBackgroundColor.SetColor(rConfig.GetColorValue(APPBACKGROUND).nColor);
/*N*/
/*N*/     aValue = rConfig.GetColorValue(OBJECTBOUNDARIES);
/*N*/     aObjectBoundColor.SetColor(aValue.nColor);
/*N*/     if(aValue.bIsVisible)
/*N*/         nAppearanceFlags |= VIEWOPT_OBJECT_BOUNDARIES;
/*N*/
/*N*/     aValue = rConfig.GetColorValue(TABLEBOUNDARIES);
/*N*/     aTableBoundColor.SetColor(aValue.nColor);
/*N*/     if(aValue.bIsVisible)
/*N*/         nAppearanceFlags |= VIEWOPT_TABLE_BOUNDARIES;
/*N*/
/*N*/     aValue = rConfig.GetColorValue(WRITERIDXSHADINGS);
/*N*/     aIndexShadingsColor.SetColor(aValue.nColor);
/*N*/     if(aValue.bIsVisible)
/*N*/         nAppearanceFlags |= VIEWOPT_INDEX_SHADINGS;
/*N*/
/*N*/     aValue = rConfig.GetColorValue(LINKS);
/*N*/     aLinksColor.SetColor(aValue.nColor);
/*N*/     if(aValue.bIsVisible)
/*N*/         nAppearanceFlags |= VIEWOPT_LINKS;
/*N*/
/*N*/     aValue = rConfig.GetColorValue(LINKSVISITED);
/*N*/     aVisitedLinksColor.SetColor(aValue.nColor);
/*N*/     if(aValue.bIsVisible)
/*N*/         nAppearanceFlags |= VIEWOPT_VISITED_LINKS;
/*N*/
/*N*/     aDirectCursorColor.SetColor(rConfig.GetColorValue(WRITERDIRECTCURSOR).nColor);
/*N*/
/*N*/     aTextGridColor.SetColor(rConfig.GetColorValue(WRITERTEXTGRID).nColor);
/*N*/
/*N*/     aSpellColor.SetColor(rConfig.GetColorValue(SPELL).nColor);
/*N*/
/*N*/     aFontColor.SetColor(rConfig.GetColorValue(FONTCOLOR).nColor);
/*N*/
/*N*/     aValue = rConfig.GetColorValue(WRITERFIELDSHADINGS);
/*N*/     aFieldShadingsColor.SetColor(aValue.nColor);
/*N*/     if(aValue.bIsVisible)
/*N*/         nAppearanceFlags |= VIEWOPT_FIELD_SHADINGS;
/*N*/
/*N*/     aValue = rConfig.GetColorValue(WRITERSECTIONBOUNDARIES);
/*N*/     aSectionBoundColor.SetColor(aValue.nColor);
/*N*/     if(aValue.bIsVisible)
/*N*/         nAppearanceFlags |= VIEWOPT_SECTION_BOUNDARIES;
/*N*/
/*N*/     aValue = rConfig.GetColorValue(WRITERPAGEBREAKS);
/*N*/     aPageBreakColor.SetColor(aValue.nColor);
/*N*/
/*N*/     aScriptIndicatorColor.SetColor(rConfig.GetColorValue(WRITERSCRIPTINDICATOR).nColor);
/*N*/ }
/* -----------------------------23.04.2002 17:48------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwViewOption::SetAppearanceFlag(sal_Int32 nFlag, BOOL bSet, BOOL bSaveInConfig )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }
/* -----------------------------24.04.2002 10:42------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ BOOL SwViewOption::IsAppearanceFlag(sal_Int32 nFlag)
/*N*/ {
/*N*/     return 0 != (nAppearanceFlags & nFlag);
/*N*/ }

}
