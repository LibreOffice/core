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
#pragma optimize("e",off)

#pragma hdrstop
#endif

class SbxArray;

#include <bf_svx/svdoutl.hxx>

#include "itempool.hxx"

#include <bf_svx/svdpage.hxx>

#include <bf_svx/svdetc.hxx>

#include <bf_svx/eeitem.hxx>
#include <bf_sfx2/app.hxx>
#ifndef _ZFORLIST_HXX
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <bf_svtools/zforlist.hxx>
#endif
#include <bf_svx/svdorect.hxx>
#include <bf_svx/xlnclit.hxx>
#include <bf_svx/xlnwtit.hxx>
#include <bf_svx/xflclit.hxx>
#include "schattr.hxx"
#include "memchrt.hxx"

#ifndef _SVX_CHRTITEM_HXX
#define ITEMID_DOUBLE	        0
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTTEXTORIENT	SCHATTR_TEXT_ORIENT
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS
#define ITEMID_CHARTDATADESCR   SCHATTR_DATADESCR_DESCR
#define ITEMID_LANGUAGE           EE_CHAR_LANGUAGE

#include <bf_svtools/eitem.hxx>

#include <bf_svx/chrtitem.hxx>
#endif

#define ITEMID_FONT        EE_CHAR_FONTINFO
#define ITEMID_COLOR       EE_CHAR_COLOR
#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#include <bf_svx/fontitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/colritem.hxx>
#include <bf_svx/svxids.hrc>
#include <i18npool/lang.h>

#include <bf_svx/xlineit0.hxx>

// header for LinguMgr
#include <bf_svx/unolingu.hxx>
// header for class SvtLinguConfig
#include <bf_svtools/lingucfg.hxx>
// header for getProcessServiceFactory
#include <comphelper/processfactory.hxx>

#ifndef _CHTMODEL_HXX
#include <chtmodel.hxx>
#include <globfunc.hxx>
#endif
#include "schattr.hxx"
#include "charttyp.hxx"

#include "float.h"


#include "pairs.hxx"
#include "stlpool.hxx"
#include "schresid.hxx"
#include "glob.hrc"

#include "datalog.hxx"
#include "chaxis.hxx"

#include "chtscene.hxx"
#include "bf_svx/def3d.hxx"

#include "docshell.hxx"
#include <com/sun/star/chart/ChartDataChangeEvent.hpp>
#include <bf_svx/unolingu.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <bf_svx/langitem.hxx>
#include <bf_svtools/undo.hxx>

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
namespace binfilter {

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

#define LINGUPROP_DEFLOCALE			"DefaultLocale"
#define LINGUPROP_CJKLOCALE			"DefaultLocale_CJK"
#define LINGUPROP_CTLLOCALE			"DefaultLocale_CTL"


/************************************************************************/

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

using namespace ::com::sun::star;

/*N*/ ChartModel::ChartModel( const String& rPalettePath, SfxObjectShell* pDocSh ) :
/*N*/ 	pChartDataBuffered(NULL),
/*N*/ 	pAutoPilot(NULL),//#46895#
/*N*/ 	bClearDepth(FALSE),
/*N*/ 	bNewOrLoadCompleted(FALSE),//aus SchChartDocument::
/*N*/ 	nChartStatus( CHS_USER_QUERY ),
/*N*/ 	SdrModel( rPalettePath, NULL, SAL_STATIC_CAST( SvPersist*, pDocSh )),
/*N*/ 	aChartRect (Rectangle ()),
/*N*/ 	aInitialSize (Size ()),
/*N*/ 	bTextScalable (TRUE),
/*N*/ 	bIsCopied (FALSE),
/*N*/ 	bLegendVisible (TRUE),
/*N*/ 	bShowAverage (FALSE),
/*N*/ 	eErrorKind (CHERROR_NONE),
/*N*/ 	eIndicate (CHINDICATE_NONE),
/*N*/ 	eRegression (CHREGRESS_NONE),
/*N*/ 	fIndicatePercent (0.0),
/*N*/ 	fIndicateBigError (0.0),
/*N*/ 	fIndicatePlus (0.0),
/*N*/ 	fIndicateMinus (0.0),
/*N*/ 	nSplineDepth (3),
/*N*/ 	nGranularity (20),
/*N*/ 	bSwitch3DColRow (FALSE), // FG: reiner Zwischenspeicher, damit die ChartScene das nicht als Parameter bekommt
/*N*/ 							 //     ist immer gleich bSwitchRowCol, das aber wird durchs Chart als Parameter
/*N*/ 							 //     durchgereicht.
/*N*/ 	pChItemPool (new SchItemPool),
/*N*/ 	pScene (0),
/*N*/ 	aLightVec (new Vector3D (1, 1, 1)), // old: aLightVec (new Vector3D (0, 0, 1)),
/*N*/ 	pChartData (0),
/*N*/ 	fMinData (0.0),
/*N*/ 	fMaxData (0.0),
/*N*/ 	fAmbientIntensity(0.6),
/*N*/ 	aAmbientColor(RGBColor(COL_WHITE)),
/*N*/ 	fSpotIntensity (0.6),
/*N*/ 	aSpotColor(RGBColor(COL_WHITE)),
/*N*/ 	eChartStyle (CHSTYLE_2D_COLUMN),
/*N*/ 	eOldChartStyle (CHSTYLE_3D_XYZSYMBOLS),
/*N*/ 	pDefaultColors (0),
/*N*/ 	nMarkLen (100),
/*N*/ 	nPieHeight (20),
/*N*/ 	pPieSegOfs (0),
/*N*/ 	nPieSegCount (0),
/*N*/ 	nXAngle (100), // old: nXAngle (200),
/*N*/ 	nYAngle (250), // old: nYAngle (350),
/*N*/ 	nZAngle (0),
/*N*/ 	bCanRebuild (TRUE),
/*N*/ 	bShowMainTitle (TRUE),
/*N*/ 	aMainTitle (String ()),
/*N*/ 	bShowSubTitle (FALSE),
/*N*/ 	aSubTitle (String ()),
/*N*/ 	bShowXAxisTitle (FALSE),
/*N*/ 	aXAxisTitle (String ()),
/*N*/ 	bShowYAxisTitle (FALSE),
/*N*/ 	aYAxisTitle (String ()),
/*N*/ 	bShowZAxisTitle (FALSE),
/*N*/ 	aZAxisTitle (String ()),
/*N*/ 	bShowXGridMain (TRUE),
/*N*/ 	bShowXGridHelp (FALSE),
/*N*/ 	bShowYGridMain (FALSE),
/*N*/ 	bShowYGridHelp (FALSE),
/*N*/ 	bShowZGridMain (FALSE),
/*N*/ 	bShowZGridHelp (FALSE),
/*N*/ 	eDataDescr (CHDESCR_NONE),
/*N*/ 	bShowSym (FALSE),
/*N*/ 	bSwitchData (TRUE),
/*N*/ 	bNoBuildChart( FALSE ),
/*N*/ 	bShouldBuildChart( TRUE ),
/*N*/ 	bReadError (FALSE),
/*N*/ 	mbIsInitialized(FALSE),
/*N*/ 	pOwnNumFormatter(FALSE),
/*N*/ 	pOutliner(NULL),
/*N*/ 		// FG: nMoreData >=12
/*N*/ 	bFormatXAxisTextInMultipleLinesIfNecessary (TRUE),
/*N*/ 	bFormatYAxisTextInMultipleLinesIfNecessary (FALSE),
/*N*/ 	bFormatZAxisTextInMultipleLinesIfNecessary (FALSE),
/*N*/ 	bFormatLegendTextInMultipleLinesIfNecessary (TRUE),
/*N*/ 		// FG: nMoreData >=13
/*N*/ 	nXAxisTextMaximumNumberOfLines(2),
/*N*/ 	nYAxisTextMaximumNumberOfLines(1),
/*N*/ 	nZAxisTextMaximumNumberOfLines(1),
/*N*/ 	nWidthOfFirstXAxisText (0),
/*N*/ 	nWidthOfLastXAxisText (0),
/*N*/ 	aTitleTopCenter(-1,-1),
/*N*/ 	aSubTitleTopCenter(-1,-1),
/*N*/ 	aDiagramRectangle(-1,-1,-1,-1),
/*N*/ 	aLastDiagramRectangle(-1,-1,-1,-1),
/*N*/ 	aLegendTopLeft(-1,-1),
/*N*/ 	aTitleXAxisPosition (-1,-1),
/*N*/ 	eAdjustXAxesTitle(CHADJUST_TOP_CENTER),
/*N*/ 	aTitleYAxisPosition(-1,-1),
/*N*/ 	eAdjustYAxesTitle(CHADJUST_TOP_CENTER),
/*N*/ 	aTitleZAxisPosition (-1,-1),
/*N*/ 	eAdjustZAxesTitle(CHADJUST_TOP_CENTER),
/*N*/ 	bUseRelativePositionsForChartGroups(FALSE),
/*N*/ 	bAdjustMarginsForLegend(TRUE),
/*N*/ 	bAdjustMarginsForMainTitle(TRUE),
/*N*/ 	bAdjustMarginsForSubTitle(TRUE),
/*N*/ 	bAdjustMarginsForXAxisTitle(TRUE),
/*N*/ 	bAdjustMarginsForYAxisTitle(TRUE),
/*N*/ 	bAdjustMarginsForZAxisTitle(TRUE),
/*N*/ 	bDiagramHasBeenMovedOrResized(FALSE),
/*N*/ 	bMainTitleHasBeenMoved(FALSE),
/*N*/ 	bSubTitleHasBeenMoved(FALSE),
/*N*/ 	bLegendHasBeenMoved(FALSE),
/*N*/ 	bXAxisTitleHasBeenMoved(FALSE),
/*N*/ 	bYAxisTitleHasBeenMoved(FALSE),
/*N*/ 	bZAxisTitleHasBeenMoved(FALSE),
/*N*/ 	aInitialSizefor3d (-1,-1),    // FG: Zwischenspeicher fuer InitalSize (siehe chtmod3d.cxx, Position3DAxisTitles
/*N*/ 	pTestTextObj(NULL),  //  FG: fuer GetHeightOfnRows, ein Dummy-Textpointer
/*N*/ 	pLogBook(NULL),
/*N*/ 	bShowDataDescr(TRUE),
/*N*/ 
/*N*/ 	pDocShell(pDocSh), //aus SchChartDocument::
/*N*/ //  	bFreshLoaded (FALSE),
/*N*/ 	m_nDefaultColorSet(0),  //#50037#
/*N*/ 	nBarPercentWidth(100),   //#50116#
/*N*/ 	nNumLinesInColChart(0),   //#50212#
/*N*/ 	bAttrAutoStorage(FALSE),
/*N*/ 	pChartRefOutDev(NULL),
/*N*/ 	pChartXAxis(NULL),
/*N*/ 	pChartYAxis(NULL),
/*N*/ 	pChartZAxis(NULL),
/*N*/ 	pChartBAxis(NULL),
/*N*/ 	pChartAAxis(NULL),
/*N*/ 	pTmpXItems(NULL),
/*N*/ 	pTmpYItems(NULL),
/*N*/ 	pTmpZItems(NULL),
/*N*/ 	pSdrObjList(NULL),
/*N*/ 	eProjection(PR_PERSPECTIVE),
/*N*/ 	bResizePie(TRUE),
/*N*/ 	nPieRadius(0),
/*N*/ 	pNumFormatter(NULL),
/*N*/ 	nXLastNumFmt(-1),
/*N*/ 	nYLastNumFmt(-1),
/*N*/ 	nBLastNumFmt(-1),
/*N*/ 	eLanguage( LANGUAGE_SYSTEM ),
/*N*/ 	eLanguageCJK( LANGUAGE_SYSTEM ),
/*N*/ 	eLanguageCTL( LANGUAGE_SYSTEM ),
/*N*/     mpDocStor( NULL ),
/*N*/ 	m_pUndoActionFromDraw(NULL),
/*N*/ 	m_bDeleteUndoActionNotificationFromDraw(TRUE)
/*N*/ {
/*N*/ 	SdrModel::SetNotifyUndoActionHdl(LINK( this, ChartModel, NotifyUndoActionHdl ));
/*N*/ 
/*N*/     if( pDocSh != NULL )
/*N*/     {
/*N*/         // enable swapping of metafiles and bitmaps that
/*N*/         // might be in a chart as additional objects
/*N*/         SetSwapGraphics( TRUE );
/*N*/     }
/*N*/ 
/*N*/ 	aLightVec->Normalize ();
/*N*/ 
/*N*/ 	SetScaleUnit(MAP_100TH_MM);
/*N*/ 	SetScaleFraction(Fraction(1, 1));
/*N*/ 	SetDefaultFontHeight(847);     // 24pt
/*N*/ 
/*N*/ 	SfxItemPool* pPool = &GetItemPool();
/*N*/ 	pPool->SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
/*N*/ 	pPool->SetPoolDefaultItem( SfxBoolItem(EE_PARA_HYPHENATE, TRUE) );
/*N*/ 	pPool->SetPoolDefaultItem(Svx3DPercentDiagonalItem (5));
/*N*/ 
/*N*/ 	pOwnNumFormatter = new SvNumberFormatter( ::legacy_binfilters::getLegacyProcessServiceFactory(),
/*N*/ 											  LANGUAGE_SYSTEM );
/*N*/ 	pOwnNumFormatter->ChangeStandardPrec( 15 );
/*N*/ 
/*N*/ 	// append pool to end of pool chain
/*N*/ 	for (;;)
/*N*/ 	{
/*N*/ 		SfxItemPool* pSecondary = pPool->GetSecondaryPool();
/*N*/ 		if (!pSecondary)
/*N*/ 			break;
/*N*/ 
/*N*/ 		pPool = pSecondary;
/*N*/ 	}
/*N*/ 
/*N*/ 	pPool->SetSecondaryPool(pChItemPool);
/*N*/ 
/*N*/ 	SfxItemPool* pMasterPool = &GetItemPool();
/*N*/ 	pMasterPool->FreezeIdRanges();
/*N*/ 
/*N*/ 	// get current language
/*N*/ 	pOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, this);
/*N*/ 	SdrOutliner& rDrawOutliner = GetDrawOutliner();
/*N*/ 
/*N*/ 	try
/*N*/ 	{
/*N*/         // set language properties
/*N*/         SvtLinguConfig aLinguConfig;
/*N*/         SvtLinguOptions aLinguOptions;
/*N*/ 
/*N*/         if( aLinguConfig.GetOptions( aLinguOptions ) )
/*N*/         {
/*N*/             SetLanguage( aLinguOptions.nDefaultLanguage,     EE_CHAR_LANGUAGE );
/*N*/             SetLanguage( aLinguOptions.nDefaultLanguage_CJK, EE_CHAR_LANGUAGE_CJK );
/*N*/             SetLanguage( aLinguOptions.nDefaultLanguage_CTL, EE_CHAR_LANGUAGE_CTL );
/*N*/         }
/*N*/     }
/*N*/ 	catch( uno::Exception aEx )
/*N*/ 	{
/*N*/ #ifdef DBG_UTIL
/*N*/         // convert ::rtl::OUString => tools String => ByteString
/*N*/         String aStr( aEx.Message );
/*N*/         ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
/*N*/         DBG_ERROR1( "LinguProperties threw exception: %s", aBStr.GetBuffer());
/*N*/ #endif
/*N*/ 	}
/*N*/ 
/*N*/ 	// init item sets for chart objects
/*N*/ 	pTitleAttr          = new SfxItemSet(*pItemPool, nTitleWhichPairs);
/*N*/ 	pMainTitleAttr      = new SfxItemSet(*pItemPool, nTitleWhichPairs);
/*N*/ 	pSubTitleAttr       = new SfxItemSet(*pItemPool, nTitleWhichPairs);
/*N*/ 	pXAxisTitleAttr     = new SfxItemSet(*pItemPool, nTitleWhichPairs);
/*N*/ 	pYAxisTitleAttr     = new SfxItemSet(*pItemPool, nTitleWhichPairs);
/*N*/ 	pZAxisTitleAttr     = new SfxItemSet(*pItemPool, nTitleWhichPairs);
/*N*/ 	pAxisAttr           = new SfxItemSet(*pItemPool, nAllAxisWhichPairs);
/*N*/ 	pGridAttr           = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 	pXGridMainAttr      = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 	pYGridMainAttr      = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 	pZGridMainAttr      = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 	pXGridHelpAttr      = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 	pYGridHelpAttr      = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 	pZGridHelpAttr      = new SfxItemSet(*pItemPool, nGridWhichPairs);
/*N*/ 	pDiagramAreaAttr    = new SfxItemSet(*pItemPool, nDiagramAreaWhichPairs);
/*N*/ 	pDiagramWallAttr    = new SfxItemSet(*pItemPool, nDiagramAreaWhichPairs);
/*N*/ 	pDiagramFloorAttr   = new SfxItemSet(*pItemPool, nDiagramAreaWhichPairs);
/*N*/ 	pLegendAttr         = new SfxItemSet(*pItemPool, nLegendWhichPairs);
/*N*/ 	pChartAttr          = new SfxItemSet(*pItemPool, nChartWhichPairs);
/*N*/ 	pDummyAttr          = new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/ 
/*N*/ 	pStockLineAttr		= new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/ 	pStockLossAttr		= new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/ 	pStockPlusAttr		= new SfxItemSet(*pItemPool, nRowWhichPairs);
/*N*/ 
/*N*/ 	SdrPage* pPage = GetPage(0);
/*N*/ 	if (pPage) aInitialSize = pPage->GetSize();
/*N*/ 
/*N*/ 	CreateDefaultColors ();
/*N*/ 	eChartLinePoints[0] =
/*N*/ 	eChartLinePoints[1] =
/*N*/ 	eChartLinePoints[2] =
/*N*/ 	eChartLinePoints[3] =
/*N*/ 	eChartLinePoints[4] =
/*N*/ 	eChartLinePoints[5] =
/*N*/ 	eChartLinePoints[6] =
/*N*/ 	eChartLinePoints[7] =
/*N*/ 	eChartLinePoints[8] = 0;
/*N*/ 
/*N*/ 	Font aLatinFont( OutputDevice::GetDefaultFont( DEFAULTFONT_LATIN_SPREADSHEET, GetLanguage( EE_CHAR_LANGUAGE ), DEFAULTFONT_FLAGS_ONLYONE ) );
/*N*/ 	SvxFontItem aSvxFontItem( aLatinFont.GetFamily(), aLatinFont.GetName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
/*N*/ 		                      aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );
/*N*/ 
/*N*/ 	Font aCJKFont( OutputDevice::GetDefaultFont( DEFAULTFONT_CJK_SPREADSHEET, GetLanguage( EE_CHAR_LANGUAGE_CJK ), DEFAULTFONT_FLAGS_ONLYONE ) );
/*N*/ 	SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamily(), aCJKFont.GetName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
/*N*/ 		                         aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );
/*N*/ 
/*N*/ 	Font aCTLFont( OutputDevice::GetDefaultFont( DEFAULTFONT_CTL_SPREADSHEET, GetLanguage( EE_CHAR_LANGUAGE_CTL ), DEFAULTFONT_FLAGS_ONLYONE ) );
/*N*/ 	SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamily(), aCTLFont.GetName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
/*N*/ 		                         aCTLFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );
/*N*/ 
/*N*/ 	// main title
/*N*/ 	pTitleAttr->Put(aSvxFontItem);
/*N*/ 	pTitleAttr->Put(SvxFontHeightItem( 459, 100, EE_CHAR_FONTHEIGHT ));	// 13pt
/*N*/ 	pTitleAttr->Put(aSvxFontItemCJK);
/*N*/ 	pTitleAttr->Put(SvxFontHeightItem( 459, 100, EE_CHAR_FONTHEIGHT_CJK ));	// 13pt
/*N*/ 	pTitleAttr->Put(aSvxFontItemCTL);
/*N*/ 	pTitleAttr->Put(SvxFontHeightItem( 459, 100, EE_CHAR_FONTHEIGHT_CTL ));	// 13pt
/*N*/     // the font color has 'automatic' as default that should not be overwritten
/*N*/ // 	pTitleAttr->Put(SvxColorItem(RGBColor(COL_BLACK)));
/*N*/ 	pTitleAttr->Put(XLineStyleItem(XLINE_NONE));
/*N*/ 	pTitleAttr->Put(XFillStyleItem(XFILL_NONE));
/*N*/ 	pTitleAttr->Put(SvxChartTextOrientItem(CHTXTORIENT_AUTOMATIC));
/*N*/ 	pMainTitleAttr->Put(*pTitleAttr);
/*N*/ 
/*N*/ 	// sub title
/*N*/ 	pTitleAttr->Put(SvxFontHeightItem( 388, 100, EE_CHAR_FONTHEIGHT ));	// 11pt
/*N*/ 	pTitleAttr->Put(SvxFontHeightItem( 388, 100, EE_CHAR_FONTHEIGHT_CJK ));	// 11pt
/*N*/ 	pTitleAttr->Put(SvxFontHeightItem( 388, 100, EE_CHAR_FONTHEIGHT_CTL ));	// 11pt
/*N*/ 	pSubTitleAttr->Put(*pTitleAttr);
/*N*/ 
/*N*/ 	// axis titles
/*N*/ 	pTitleAttr->Put(SvxFontHeightItem( 318, 100, EE_CHAR_FONTHEIGHT ));	// 9pt
/*N*/ 	pTitleAttr->Put(SvxFontHeightItem( 318, 100, EE_CHAR_FONTHEIGHT_CJK ));	// 9pt
/*N*/ 	pTitleAttr->Put(SvxFontHeightItem( 318, 100, EE_CHAR_FONTHEIGHT_CTL ));	// 9pt
/*N*/ 	pXAxisTitleAttr->Put(*pTitleAttr);
/*N*/ 	pYAxisTitleAttr->Put(*pTitleAttr);
/*N*/ 	pZAxisTitleAttr->Put(*pTitleAttr);
/*N*/ 
/*N*/ 	// general axis attributies: are also set in class ChartAxis()
/*N*/ 	// (BM) => deprecated and therefore removed here
/*N*/ 
/*N*/     Color aAreaBackColor( RGBColor( COL_WHITE ));
/*N*/ 	pDiagramAreaAttr->Put(XLineStyleItem(XLINE_NONE));
/*N*/ 	pDiagramAreaAttr->Put(XFillColorItem(String(), aAreaBackColor));
/*N*/ 
/*N*/     // set the page color (in a chart this is the color of the area)
/*N*/     // at the outliner
/*N*/     if( pOutliner )
/*N*/         pOutliner->SetBackgroundColor( aAreaBackColor );
/*N*/ 
/*N*/ 	pDiagramWallAttr->Put(XLineStyleItem(XLINE_NONE));
/*N*/ 	pDiagramWallAttr->Put(XFillStyleItem(XFILL_NONE));
/*N*/ 
/*N*/ 	pDiagramFloorAttr->Put(XLineStyleItem(XLINE_NONE));
/*N*/ 	pDiagramFloorAttr->Put(XFillColorItem(String(), Color(153, 153, 153)));
/*N*/ 
/*N*/ 	pLegendAttr->Put(aSvxFontItem);
/*N*/ 	pLegendAttr->Put(aSvxFontItemCJK);
/*N*/ 	pLegendAttr->Put(aSvxFontItemCTL);
/*N*/ 	pLegendAttr->Put(SvxFontHeightItem( 212, 100, EE_CHAR_FONTHEIGHT ));	// 6pt #72012#
/*N*/ 	pLegendAttr->Put(SvxFontHeightItem( 212, 100, EE_CHAR_FONTHEIGHT_CJK ));	// 6pt #72012#
/*N*/ 	pLegendAttr->Put(SvxFontHeightItem( 212, 100, EE_CHAR_FONTHEIGHT_CTL ));	// 6pt #72012#
/*N*/ 	pLegendAttr->Put(XFillStyleItem(XFILL_NONE));
/*N*/ 
/*N*/ 	//Todo: ueberpruefen, ob noch korekkt, es wird der default-ChartTyp erzeugt und der
/*N*/ 	//AttrSet in pChartAttr gesetzt
/*N*/ 	ChartType aTyp((SvxChartStyle) CHSTYLE_2D_COLUMN);
/*N*/ 	aTyp.GetAttrSet(pChartAttr);
/*N*/ 
/*N*/ 	// the dummy attribute is (mainly!) used for data row defaults
/*N*/ 	// so avoid setting (pool) default values here as fixed
/*N*/ 	pDummyAttr->Put( aSvxFontItem );
/*N*/ 	pDummyAttr->Put( aSvxFontItemCJK );
/*N*/ 	pDummyAttr->Put( aSvxFontItemCTL );
/*N*/ 	pDummyAttr->Put(SvxFontHeightItem( 212, 100, EE_CHAR_FONTHEIGHT ));	// 6pt #72012#
/*N*/ 	pDummyAttr->Put(SvxFontHeightItem( 212, 100, EE_CHAR_FONTHEIGHT_CJK ));	// 6pt #72012#
/*N*/ 	pDummyAttr->Put(SvxFontHeightItem( 212, 100, EE_CHAR_FONTHEIGHT_CTL ));	// 6pt #72012#
/*N*/ 
/*N*/ 	// what are these needed for?
/*N*/ 	pDummyAttr->Put (SfxInt32Item (SCHATTR_DUMMY0, (INT32) eChartStyle));
/*N*/ 	pDummyAttr->Put (SfxInt32Item (SCHATTR_DUMMY1, 0));
/*N*/ 
/*N*/ 	pStockLossAttr->Put(XFillColorItem(String(), RGBColor(COL_BLACK)));
/*N*/ 	pStockPlusAttr->Put(XFillColorItem(String(), RGBColor(COL_WHITE)));
/*N*/ 
/*N*/ 	// this test object is for calculating the text height in GetHeightOfnRows
/*N*/ 	// (chtmode1.cxx) without creating a new object for each call.
/*N*/ 	pTestTextObj = new SdrRectObj (OBJ_TEXT, Rectangle(0, 0, 10, 10) /*, aTestStr*/);
/*N*/ 
/*N*/ 	//Aus SchChartDocument::
/*N*/ 	SetStyleSheetPool(new SchStyleSheetPool(*pItemPool));
/*N*/ 	((SdrOutliner*)pDrawOutliner)->SetStyleSheetPool((SfxStyleSheetPool*)pStyleSheetPool);
/*N*/ 	pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)pStyleSheetPool);
/*N*/ 
/*N*/ 	// Layer anlegen
/*N*/ 
/*N*/ 	SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
/*N*/ 	rLayerAdmin.NewLayer( String( SchResId( STR_LAYOUT )));
/*N*/ 	rLayerAdmin.NewLayer( String( SchResId( STR_CONTROLS )));
/*N*/ 
/*N*/ 
/*N*/ 	//Wichtig! die Achsen muessen jetzt erstellt werden, dies darf erst nach dem
/*N*/ 	//anlegen des ItemPools und der Defaults passieren!
/*N*/ 	pChartXAxis = new ChartAxis( this, CHART_AXIS_X, CHAXIS_AXIS_X );
/*N*/ 	pChartYAxis = new ChartAxis( this, CHART_AXIS_Y, CHAXIS_AXIS_Y );
/*N*/ 	pChartZAxis = new ChartAxis( this, CHART_AXIS_Z, CHAXIS_AXIS_Z );
/*N*/ 	pChartBAxis = new ChartAxis( this, CHART_AXIS_Y, CHAXIS_AXIS_B );
/*N*/ 	pChartAAxis = new ChartAxis( this, CHART_AXIS_X, CHAXIS_AXIS_A );
/*N*/ 
/*N*/ 	pChartYAxis->IfNoDataLookAt( CHAXIS_AXIS_B ); //Falls Achse nur ein Spiegelbild ist....
/*N*/ 	pChartBAxis->IfNoDataLookAt( CHAXIS_AXIS_Y ); //d.h. keine eigenen Datenreihen besitzt
/*N*/ 
/*N*/ 	aBarY1.Assign(this,pChartYAxis);
/*N*/ 	aBarY2.Assign(this,pChartBAxis);
/*N*/ 
/*N*/ 	pChartXAxis->SetAllAxisAttr(pAxisAttr);
/*N*/ 	pChartYAxis->SetAllAxisAttr(pAxisAttr);
/*N*/ 	pChartZAxis->SetAllAxisAttr(pAxisAttr);
/*N*/ 	pChartAAxis->SetAllAxisAttr(pAxisAttr);
/*N*/ 	pChartBAxis->SetAllAxisAttr(pAxisAttr);
/*N*/ 
/*N*/ 	pChartXAxis->SetGridAttrList(pYGridMainAttr);//XGrid->YAchse :(  (u.U.)
/*N*/ 	pChartYAxis->SetGridAttrList(pXGridMainAttr);
/*N*/ 
/*N*/ 	SetNumberFormatter( pOwnNumFormatter );				// sets number formatter also for all axes
/*N*/ 
/*N*/ 	long nTmp=pNumFormatter->GetStandardFormat(NUMBERFORMAT_NUMBER, LANGUAGE_SYSTEM);
/*N*/ 	SetNumFmt(CHOBJID_DIAGRAM_X_AXIS,nTmp,FALSE);
/*N*/ 	SetNumFmt(CHOBJID_DIAGRAM_Y_AXIS,nTmp,FALSE);
/*N*/ 	SetNumFmt(CHOBJID_DIAGRAM_Z_AXIS,nTmp,FALSE);
/*N*/ 	SetNumFmt(CHOBJID_DIAGRAM_A_AXIS,nTmp,FALSE);
/*N*/ 	SetNumFmt(CHOBJID_DIAGRAM_B_AXIS,nTmp,FALSE);
/*N*/ 
/*N*/ 	pChartBAxis->ShowAxis(FALSE);
/*N*/ 	pChartBAxis->ShowDescr(FALSE);
/*N*/ 	pChartAAxis->ShowAxis(FALSE);
/*N*/ 	pChartAAxis->ShowDescr(FALSE);
/*N*/ 
/*N*/ 	pSdrObjList=new SdrObjList(this,NULL);
/*N*/ 
/*N*/     // #99528# change auto-font color according to default diagram area
/*N*/     PageColorChanged( *pDiagramAreaAttr );
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/
/*N*/ void ChartModel::ClearItemSetLists()
/*N*/ {
/*N*/ 	long i, nCount;
/*N*/ 
/*N*/ 	nCount = aDataRowAttrList.Count();
/*N*/ 	for (i = 0 ; i < nCount; i++)
/*N*/ 		delete aDataRowAttrList.GetObject(i);
/*N*/ 	aDataRowAttrList.Clear();
/*N*/ 
/*N*/ 	nCount = aRegressAttrList.Count();
/*N*/ 	for (i = 0 ; i < nCount; i++)
/*N*/ 		delete aRegressAttrList.GetObject(i);
/*N*/ 	aRegressAttrList.Clear();
/*N*/ 
/*N*/ 	nCount = aDataPointAttrList.Count();
/*N*/ 	for (i = 0 ; i < nCount; i++)
/*N*/ 		delete aDataPointAttrList.GetObject(i);
/*N*/ 	aDataPointAttrList.Clear();
/*N*/ 
/*N*/ 	nCount = aSwitchDataPointAttrList.Count();
/*N*/ 	for (i = 0 ; i < nCount; i++)
/*N*/ 		delete aSwitchDataPointAttrList.GetObject(i);
/*N*/ 	aSwitchDataPointAttrList.Clear();
/*N*/ 
/*N*/ 	nCount =  aAverageAttrList.Count();
/*N*/ 	for (i = 0 ; i < nCount; i++)
/*N*/ 		delete aAverageAttrList.GetObject(i);
/*N*/ 	aAverageAttrList.Clear();
/*N*/ 
/*N*/ 	nCount =  aErrorAttrList.Count();
/*N*/ 	for (i = 0 ; i < nCount; i++)
/*N*/ 		delete aErrorAttrList.GetObject(i);
/*N*/ 	aErrorAttrList.Clear();
/*N*/ }
/*N*/ ChartModel::~ChartModel()
/*N*/ {
/*N*/ 	if(pTmpXItems)
/*N*/ 		delete pTmpXItems;
/*N*/ 	if(pTmpYItems)
/*N*/ 		delete pTmpYItems;
/*N*/ 	if(pTmpZItems)
/*N*/ 		delete pTmpZItems;
/*N*/ 
/*N*/ 	if(pChartRefOutDev)
/*?*/ 		delete pChartRefOutDev;
/*N*/ 
/*N*/ 	Clear();//aus SchChartDocument::
/*N*/ 
/*N*/ 	delete pTitleAttr;
/*N*/ 	delete pMainTitleAttr;
/*N*/ 	delete pSubTitleAttr;
/*N*/ 	delete pXAxisTitleAttr;
/*N*/ 	delete pYAxisTitleAttr;
/*N*/ 	delete pZAxisTitleAttr;
/*N*/ 	delete pAxisAttr;
/*N*/ 
/*N*/ 	delete pChartXAxis;
/*N*/ 	delete pChartYAxis;
/*N*/ 	delete pChartZAxis;
/*N*/ 	delete pChartAAxis;
/*N*/ 	delete pChartBAxis;
/*N*/ 
/*N*/ 	delete pGridAttr;
/*N*/ 	delete pXGridMainAttr;
/*N*/ 	delete pYGridMainAttr;
/*N*/ 	delete pZGridMainAttr;
/*N*/ 	delete pXGridHelpAttr;
/*N*/ 	delete pYGridHelpAttr;
/*N*/ 	delete pZGridHelpAttr;
/*N*/ 	delete pDiagramAreaAttr;
/*N*/ 	delete pDiagramWallAttr;
/*N*/ 	delete pDiagramFloorAttr;
/*N*/ 	delete pLegendAttr;
/*N*/ 	delete pChartAttr;
/*N*/ 	delete pDummyAttr;
/*N*/ 	delete aLightVec;
/*N*/ 
/*N*/ 	delete pStockLineAttr;
/*N*/ 	delete pStockLossAttr;
/*N*/ 	delete pStockPlusAttr;
/*N*/ 
/*N*/ 	DestroyDefaultColors();
/*N*/ 	delete pOutliner;
/*N*/ 	ClearItemSetLists();
/*N*/ 
/*N*/ 	// suche nach dem ChartPool in der Poolkette, loesche alle verbindungen
/*N*/ 	// und vernichte den pool. vorgehen ist unabhaengig von der anzahl
/*N*/ 	// der vorangehendem pools. [RB]
/*N*/ 	SfxItemPool* pPool = &GetItemPool();
/*N*/ 
/*N*/ 	for (;;)
/*N*/ 	{
/*N*/ 		SfxItemPool* pSecondary = pPool->GetSecondaryPool();
/*N*/ 		if (pSecondary == pChItemPool)
/*N*/ 		{
/*N*/ 			pPool->SetSecondaryPool (NULL);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 		pPool = pSecondary;
/*N*/ 	}
/*N*/ 
/*N*/ 	delete pChItemPool;
/*N*/ 
/*N*/ 	delete pOwnNumFormatter;
/*N*/ 
/*N*/ 	delete[] pPieSegOfs;
/*N*/ 
/*N*/ 	if(pChartData)
/*N*/ 	{
/*N*/ 		if(pChartData->DecreaseRefCount())
/*N*/ 			delete pChartData;
/*N*/ 		pChartData=NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (pTestTextObj != NULL) delete pTestTextObj;
/*N*/ 	if (pLogBook) delete pLogBook;
/*N*/ 
/*N*/ 
/*N*/ 
/*N*/ 	SdrObject *pObj;
/*N*/ 	while(pSdrObjList->GetObjCount())
/*?*/ 		if(pObj=pSdrObjList->RemoveObject(0))
/*?*/ 			delete	pObj;
/*N*/ 	delete pSdrObjList;
/*N*/ 
/*N*/ 	if(m_pUndoActionFromDraw) delete m_pUndoActionFromDraw;
/*N*/ }
/*************************************************************************
|*
|* Das Logbuch dient zur ReAttributierung, nachdem die DataBrowseBox
|* aufgerufen und Daten hinzugefuegt wurden. Die BrowseBox setzt
|* eine
|*
\************************************************************************/


/*************************************************************************
|*
|* Chart-Datenstruktur auswechseln; bisherige wird ggf. geloescht;
|* Liefert TRUE, wenn die Chart-Datenstruktur gewechselt wurde.
|*
\************************************************************************/

/*N*/ BOOL ChartModel::ChangeChartData(SchMemChart& rData, BOOL bNewTitles,BOOL bDontBuild)
/*N*/ {
/*N*/ 	if (pChartData == &rData) return FALSE;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SetChartData(rData, bNewTitles);
/*N*/ 
/*N*/ 		if( pDocShell &&
/*N*/ 			pDocShell->ISA( SchChartDocShell ))
/*N*/ 		{
/*N*/ 			chart::ChartDataChangeEvent aEvent;
/*N*/ 			aEvent.Type = chart::ChartDataChangeType_ALL;
/*N*/ 			aEvent.StartColumn = 0;
/*N*/ 			aEvent.EndColumn = 0;
/*N*/ 			aEvent.StartRow = 0;
/*N*/ 			aEvent.EndRow = 0;
/*N*/ 			SAL_STATIC_CAST( SchChartDocShell*, pDocShell )->DataModified( aEvent );
/*N*/ 		}
/*N*/ 
/*N*/ 		if(!bDontBuild)
/*N*/ 			BuildChart(FALSE);
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Datenzeilen in Reihen/Spalten umschalten;
|* Liefert bei Umschaltung TRUE
|*
\************************************************************************/

/*N*/ BOOL ChartModel::ChangeSwitchData(BOOL bSwitch)
/*N*/ {
/*N*/ 	if (bSwitchData == bSwitch) return FALSE;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		bSwitchData = bSwitch;
/*N*/ 
/*N*/ #ifndef NO_56798_FIX //#56798# QuickFix 5.0-Final
/*N*/ 		if( IsReal3D() )
/*N*/ 		{
/*?*/ 			bClearDepth=TRUE;
/*?*/ 			long i,nORow=aDataRowAttrList.Count();
/*?*/ 			for(i=0;i<nORow;i++)
/*?*/ //-/				aDataRowAttrList.GetObject(i)->ClearItem(SID_ATTR_3D_DEPTH);
/*?*/ 				aDataRowAttrList.GetObject(i)->ClearItem(SDRATTR_3DOBJ_DEPTH);
/*?*/ 			SfxItemSet	*	pAttributes;
/*?*/ 			nORow=aDataPointAttrList.Count();
/*?*/ 			for(i=0;i<nORow;i++)
/*?*/ 			{
/*?*/ 				pAttributes = aDataPointAttrList.GetObject(i);
/*?*/ 				if (pAttributes != NULL)
/*?*/ 					pAttributes->ClearItem(SDRATTR_3DOBJ_DEPTH);
/*?*/ 			}
/*?*/ 			nORow=aSwitchDataPointAttrList.Count();
/*?*/ 			for(i=0;i<nORow;i++)
/*?*/ 			{
/*?*/ 				pAttributes = aSwitchDataPointAttrList.GetObject(i);
/*?*/ 				if (pAttributes != NULL)
/*?*/ 					pAttributes->ClearItem(SDRATTR_3DOBJ_DEPTH);
/*?*/ 			}
/*N*/ 		}
/*N*/ #endif
/*N*/ 
/*N*/         // the number of series may change, so the attribute list
/*N*/         // must be adapted
/*N*/         InitDataAttrs();
/*N*/ 
/*N*/ 		// BM: #68764#
/*N*/ 		// if global data descriptions are on transfer them to new rows
/*N*/ 		if( eDataDescr != CHDESCR_NONE )
/*N*/ 		{
/*?*/ 			ChangeDataDescr( eDataDescr, bShowSym, -1, FALSE );
/*N*/ 		}
/*N*/ 
/*N*/ 		BuildChart(FALSE);
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ }




/**	@descr	Set up the line attributes of every data row.  This includes
        the line style, width, and color.  Style is set to solid, width to
        the minimal width (if the given mode is not SETLINES_COMPAT).
        The color depends on the given mode.
        If the chart is a stock chart then all the lines are simply switched
        off (by setting line style XLINE_NONE).  No other attributes are 
        modified.
    @param	nMode	Specifies the line color of each data row.
            SETLINES_COMPAT	and SETLINES_FILLCOLOR set the line color
                to the fill color.  SETLINES_COMPAT does not change line
                style nor width.
            SETLINES_REVERSE sets the fill color to the line color.
            SETLINES_BLACK sets the line color to black.
*/
/*N*/ void ChartModel::SetupLineColors( const long nMode, long nStartIndex ) //#54870#
/*N*/ {
/*N*/ 	long	nRow,
/*N*/ 			nRowCnt;
/*N*/ 			
/*N*/ 	//	The Row count depends (again) on wether the data set is transposed
/*N*/ 	//	(switched) or not.  Because this returns the wrong value in case of 
/*N*/ 	//	a pie chart, then we access the raw row count without regarding the
/*N*/ 	//	transpose switch.
/*N*/ 	if (IsPieChart())
/*N*/ 		nRowCnt = GetColCount(); // ChartData()->GetRowCount();
/*N*/ 	else
/*N*/ 		nRowCnt = GetRowCount();
/*N*/ 
/*N*/     long nUpperIndex = ::std::min( static_cast< ULONG >( nRowCnt ), aDataRowAttrList.Count() );
/*N*/ 
/*N*/     if( nStartIndex < nUpperIndex )
/*N*/     {
/*N*/         if (HasStockLines())
/*N*/         {
/*N*/             //	Switch off lines for stock charts.
/*N*/             for( nRow = nStartIndex; nRow < nUpperIndex; nRow++ )
/*N*/                 aDataRowAttrList.GetObject(nRow)->Put(XLineStyleItem(XLINE_NONE));
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             //	Initialize the item set that will be set to all data rows.
/*N*/             //	If nMode==SETLINES_COMPAT initialization is skipped and only
/*N*/             //	the line colors are set to the fill colors.
/*N*/             SfxItemSet rAttr(GetItemPool(),XATTR_START,XATTR_END);
/*N*/             if(nMode != SETLINES_COMPAT)
/*N*/             {
/*N*/                 //	Default values represent a solid black line of minimal width.
/*N*/                 rAttr.Put(XLineStyleItem(XLINE_SOLID));
/*N*/                 rAttr.Put(XLineColorItem(String(),RGBColor(COL_BLACK)));
/*N*/                 rAttr.Put(XLineWidthItem (0));
/*N*/             }
/*N*/ 
/*N*/             //	Set the itemset rAttr to all data rows.  Depending on nMode it is first
/*N*/             //	modified so that the line color is set to the fill color or the other
/*N*/             //	way round.
/*N*/             //	The for loop and switch statement changed places in order to not
/*N*/             //	having to execute the switch statement in every iteration.
/*N*/             switch(nMode)
/*N*/             {
/*N*/                 case SETLINES_COMPAT:
/*N*/                 case SETLINES_FILLCOLOR:
/*N*/                     //	Set the line colors to the former fill colors.
/*N*/                     for( nRow = nStartIndex; nRow < nUpperIndex; nRow++ )
/*N*/                         if(IsLine(nRow))
/*N*/                         {
/*N*/                             rAttr.Put(XLineColorItem(String(),
/*N*/                                                      ((XFillColorItem &)GetDataRowAttr(nRow).Get(XATTR_FILLCOLOR)).
/*N*/                                                      GetValue()));
/*N*/                             aDataRowAttrList.GetObject(nRow)->Put(rAttr);
/*N*/                         }
/*N*/                     break;
/*N*/ 
/*N*/                 case SETLINES_REVERSE:
/*N*/                     //	Set the fill colors to the former line colors.
/*?*/                     for( nRow = nStartIndex; nRow < nUpperIndex; nRow++ )
/*?*/                         if(IsLine(nRow))
/*?*/                         {
/*?*/                             rAttr.Put(XFillColorItem(String(),
/*?*/                                                      ((XLineColorItem &)GetDataRowAttr(nRow).Get(XATTR_LINECOLOR)).
/*?*/                                                      GetValue()));
/*?*/                             aDataRowAttrList.GetObject(nRow)->Put(rAttr);
/*?*/                         }
/*?*/                     break;
/*N*/ 
/*N*/                 case SETLINES_BLACK:
/*N*/                     //	Set the default values to all data rows.
/*N*/                     for( nRow = nStartIndex; nRow < nUpperIndex; nRow++ )
/*N*/                     {
/*N*/                         aDataRowAttrList.GetObject(nRow)->Put(rAttr);
/*N*/                     }
/*N*/                     break;
/*N*/             }
/*N*/         }
/*N*/ 	}	
/*N*/ }

/*************************************************************************
|*
|* Charttyp aendern;
|* Liefert bei neuem Charttyp TRUE.
|*
\************************************************************************/
// BM: src566b: ChangeChart doesn't execute BuildChart any more!
/*N*/ BOOL ChartModel::ChangeChart( SvxChartStyle eStyle, bool bSetDefaultAttr /* = true */ )
/*N*/ {
/*N*/ 	if( eStyle == CHSTYLE_ADDIN )
/*N*/ 	{
/*?*/ 		eChartStyle = eStyle;
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	else if( eStyle == eChartStyle )
/*N*/ 	{
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// if chart style is reset disable addin
/*N*/ 		if( ! GetChartStatusFlag( CHS_KEEP_ADDIN ))
/*N*/ 		{
/*N*/ 			mxChartAddIn = NULL;
/*N*/ 		}
/*N*/ 
/*N*/ 		bResizePie=TRUE;
/*N*/ 
/*N*/ 		// OldChartStyle merken
/*N*/ 		eOldChartStyle = eChartStyle;
/*N*/ 
/*N*/ 		//########### Ab hier werden defaultwerte umgesetzt:######### :
/*N*/ 
/*N*/ 		//Wenn Linien, aber nicht 3D, dann muss evtl. bei
/*N*/ 		//Typwechsel die Linienfarbe neu defaultet werden!
/*N*/ 
/*N*/ 		long nRefLine=0;//Bei StockChart 3,4 ist die erste Zeile keine Linie
/*N*/ 		if(GetRowCount()>1)
/*N*/ 			nRefLine=1;
/*N*/ 
/*N*/         // #101164# a combi chart may contain lines for all but the first series
/*N*/         if( eOldChartStyle == CHSTYLE_2D_LINE_COLUMN ||
/*N*/             eOldChartStyle == CHSTYLE_2D_LINE_STACKEDCOLUMN )
/*N*/             nRefLine = 0;
/*N*/ 
/*N*/ 		BOOL bOldIsLine=IsLine(nRefLine)&& !Is3DChart();//#54870#//Verbund-Charts OK???
/*N*/ 		BOOL bOldIsStock=HasStockLines();
/*N*/ 		BOOL bOldIs3D=IsReal3D();
/*N*/ 		BOOL bOldHadStockBars=HasStockBars();
/*N*/ 		BOOL bOldXY = IsXYChart();
/*N*/ 		BOOL bOldNet = IsNetChart();
/*N*/         BOOL bOldPie = IsPieChart();
/*N*/         BOOL bOldDonut = IsDonutChart();
/*N*/ 
/*N*/ 		eChartStyle = eStyle;
/*N*/ 
/*N*/ 		BOOL bNewIsLine=IsLine(nRefLine) && !Is3DChart();//#54870#
/*N*/ 		BOOL bNewIs3D=IsReal3D();
/*N*/ 
/*N*/         // data row attributes are used for data points in a piechart
/*N*/         // therefore these need to be initialized correctly
/*N*/         BOOL bMustInitDataAttrs = (bOldPie || IsPieChart() || IsDonutChart() || bOldDonut);
/*N*/ 
/*N*/         if( eStyle == CHSTYLE_3D_PIE )
/*N*/ 		{
/*?*/ 			for( short i = 0; i < nPieSegCount; i++ )
/*?*/ 				SetPieSegOfs( i, 0 );
/*N*/ 		}

/*N*/         if( bSetDefaultAttr )
/*?*/         {
/*?*/             // BM: use gray (15%) background (wall) for some charts
/*?*/             if( HasDefaultGrayWall() != HasDefaultGrayWall( &eOldChartStyle ) )
/*?*/             {
/*?*/                 if( HasDefaultGrayWall() )
/*?*/                 {
/*?*/                     pDiagramWallAttr->Put( XFillStyleItem( XFILL_SOLID ));
/*?*/                     pDiagramWallAttr->Put( XFillColorItem( String(), RGBColor( RGB_COLORDATA( 0xd9, 0xd9, 0xd9 ) )) );
/*?*/ 
/*?*/                     pLegendAttr->Put( XFillStyleItem( XFILL_SOLID ));
/*?*/                     pLegendAttr->Put( XFillColorItem( String(), RGBColor( RGB_COLORDATA( 0xd9, 0xd9, 0xd9 ) )) );
/*?*/                 }
/*?*/                 else
/*?*/                 {
/*?*/                     pDiagramWallAttr->Put( XFillStyleItem( XFILL_NONE ));
/*?*/                     pDiagramWallAttr->Put( XFillColorItem( String(), RGBColor( COL_WHITE )) );
/*?*/ 
/*?*/                     pLegendAttr->Put( XFillStyleItem( XFILL_NONE ));
/*?*/                     pLegendAttr->Put( XFillColorItem( String(), RGBColor( COL_WHITE )) );
/*?*/                 }
/*?*/             }
/*?*/ 
/*?*/             // BM: use gray (15%) background (area) for some charts
/*?*/             if( HasDefaultGrayArea() != HasDefaultGrayArea( &eOldChartStyle ) )
/*?*/             {
/*?*/                 if( HasDefaultGrayArea() )
/*?*/                 {
/*?*/                     pDiagramAreaAttr->Put( XFillStyleItem( XFILL_SOLID ));
/*?*/                     pDiagramAreaAttr->Put( XFillColorItem( String(), RGBColor( RGB_COLORDATA( 0xd9, 0xd9, 0xd9 ) )) );
/*?*/ 
/*?*/                     pLegendAttr->Put( XFillStyleItem( XFILL_SOLID ));
/*?*/                     pLegendAttr->Put( XFillColorItem( String(), RGBColor( RGB_COLORDATA( 0xd9, 0xd9, 0xd9 ) )) );
/*?*/                 }
/*?*/                 else
/*?*/                 {
/*?*/                     pDiagramAreaAttr->Put( XFillColorItem( String(), RGBColor( COL_WHITE )) );
/*?*/ 
/*?*/                     pLegendAttr->Put( XFillStyleItem( XFILL_NONE ));
/*?*/                     pLegendAttr->Put( XFillColorItem( String(), RGBColor( COL_WHITE )) );
/*?*/                 }
/*?*/             }
/*?*/         }
/*?*/ 
/*N*/         if(bNewIsLine != bOldIsLine)
/*N*/ 		{
/*?*/             // map old line colors to new fill colors
/*?*/             if( bNewIsLine )
/*?*/             {
/*?*/                 if( bOldPie )
/*?*/                     // && GetRowCount() < GetColCount())
/*?*/                 {
/*?*/                     InitDataAttrs();
/*?*/                     bMustInitDataAttrs = FALSE;
/*?*/                 }
/*?*/                 SetupLineColors( SETLINES_COMPAT );
/*?*/             }
/*?*/             else
/*?*/             {
/*?*/                 if( IsPieChart() )
/*?*/                     // && GetRowCount() < GetColCount())
/*?*/                 {
/*?*/                     InitDataAttrs();
/*?*/                     bMustInitDataAttrs = FALSE;
/*?*/                 }
/*?*/                 // the chart style must be swapped to the old one
/*?*/                 // temporarily, so that IsLine() works correctly
/*?*/ //                  eChartStyle = eOldChartStyle;
/*?*/                 SetupLineColors( SETLINES_BLACK );
/*?*/ //                  eChartStyle = eStyle;
/*?*/             }
/*?*/ 
/*?*/ 		}

/*N*/         if( bMustInitDataAttrs )
/*N*/         {
/*?*/             InitDataAttrs();
/*?*/             bMustInitDataAttrs = FALSE;
/*N*/         }
/*N*/ 
/*N*/ 		if(bOldXY!=IsXYChart())
/*N*/ 		{
/*?*/ 			if(bOldXY)
/*?*/ 				aDataRowAttrList.GetObject(0)->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_PRIMARY_Y));//wird evtl. unten geändert, s.u. StockCharts
/*?*/ 			else
/*?*/ 				aDataRowAttrList.GetObject(0)->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_PRIMARY_X));
/*?*/ 
/*?*/ 			CheckForNewAxisNumFormat();			// BM #59532#
/*N*/ 		}
/*N*/ 
/*N*/ 
/*N*/         long nRowCnt=aDataRowAttrList.Count();//=GetRowCount();
/*N*/ 		if( (bOldIsStock && !HasStockLines()) || (bOldIs3D && !bNewIs3D) )
/*N*/ 		{
                long n=0;
/*?*/ 			for(n=0;n<nRowCnt;n++)
/*?*/ 				aDataRowAttrList.GetObject(n)->Put(XLineStyleItem(XLINE_SOLID));
/*?*/ 			long nColCnt=aDataPointAttrList.Count();
/*?*/ 			SfxItemSet	*	pAttributes;
/*?*/ 			for(n=0;n<nColCnt;n++)
/*?*/ 			{
/*?*/ 				pAttributes = aDataPointAttrList.GetObject(n);
/*?*/ 				if (pAttributes != NULL)
/*?*/ 					pAttributes->ClearItem(XATTR_LINESTYLE);
/*?*/ 			}
/*?*/ 			nColCnt=aSwitchDataPointAttrList.Count();
/*?*/ 			for(n=0;n<nColCnt;n++)
/*?*/ 			{
/*?*/ 				pAttributes = aSwitchDataPointAttrList.GetObject(n);
/*?*/ 				if (pAttributes != NULL)
/*?*/ 					pAttributes->ClearItem(XATTR_LINESTYLE);
/*?*/ 			}
/*?*/ 		}
/*N*/ 		if(!bOldIsStock && HasStockLines() || (!bOldIs3D && bNewIs3D) )
/*N*/ 		{
                long n=0;
/*N*/ 			for(n=0;n<nRowCnt;n++)
/*N*/ 				aDataRowAttrList.GetObject(n)->Put(XLineStyleItem(XLINE_NONE));
/*N*/ 			long nColCnt=aDataPointAttrList.Count();
/*N*/ 			SfxItemSet	*	pAttributes;
/*N*/ 			for(n=0;n<nColCnt;n++)
/*N*/ 			{
/*N*/ 				pAttributes = aDataPointAttrList.GetObject(n);
/*N*/ 				if (pAttributes != NULL)
/*?*/ 					pAttributes->ClearItem(XATTR_LINESTYLE);
/*N*/ 			}
/*N*/ 			nColCnt=aSwitchDataPointAttrList.Count();
/*N*/ 			for(n=0;n<nColCnt;n++)
/*N*/ 			{
/*N*/ 				pAttributes = aSwitchDataPointAttrList.GetObject(n);
/*N*/ 				if (pAttributes != NULL)
/*?*/ 					pAttributes->ClearItem(XATTR_LINESTYLE);
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if(/*!bOldHadStockBars && */HasStockBars())//Hat Balken im Hintergrund (ab jetzt oder Typ 3 <-> 4, #65070#)
/*N*/ 		{
/*?*/ 			if(nRowCnt)
/*?*/ 			{
/*?*/ 				aDataRowAttrList.GetObject(0)->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_PRIMARY_Y));
/*?*/ 				aDataRowAttrList.GetObject(0)->Put(XLineStyleItem(XLINE_SOLID));
/*?*/ 			}
/*?*/ 			for(long n=1;n<nRowCnt;n++)
/*?*/ 				aDataRowAttrList.GetObject(n)->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_SECONDARY_Y));
/*?*/ 			pChartBAxis->ShowAxis(TRUE);
/*?*/ 			pChartBAxis->ShowDescr(TRUE);
/*?*/ 			SfxItemSet aSet(*pItemPool,SCHATTR_AXIS_AUTO_ORIGIN,SCHATTR_AXIS_AUTO_ORIGIN);
/*?*/ 			aSet.Put(SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN,TRUE));
/*?*/ 			pChartBAxis->SetAttributes(aSet);
/*?*/ 
/*?*/             // #100923#
/*?*/             SfxItemSet aSet2( *pItemPool, SCHATTR_AXIS_AUTO_ORIGIN, SCHATTR_AXIS_ORIGIN );
/*?*/             aSet2.Put( SfxBoolItem( SCHATTR_AXIS_AUTO_ORIGIN, FALSE ));
/*?*/ 			aSet2.Put( SvxDoubleItem( 0.0, SCHATTR_Y_AXIS_ORIGIN ));
/*?*/             pChartYAxis->SetAttributes( aSet2 );
/*?*/         }
/*?*/ 
/*N*/ 		if(bOldHadStockBars && !HasStockBars())//hat jetzt keine Balken mehr
/*?*/ 		{
/*?*/ 			for(long n=0;n<nRowCnt;n++)
/*?*/ 				aDataRowAttrList.GetObject(n)->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_PRIMARY_Y));
/*?*/ 			if(IsXYChart())
/*?*/ 				aDataRowAttrList.GetObject(0)->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_PRIMARY_X));
/*?*/ 			pChartBAxis->ShowAxis(FALSE);
/*?*/ 			pChartBAxis->ShowDescr(FALSE);
/*?*/ 		}

        // use default position if base type changed
/*N*/ 		ChartType aOldType( eOldChartStyle );
/*N*/ 		ChartType aNewType( eChartStyle );
/*N*/ 
/*N*/ 		if( aOldType.GetBaseType() !=
/*N*/ 			aNewType.GetBaseType() )
/*N*/ 		{
/*?*/ 			SetUseRelativePositions( FALSE );
/*N*/ 		}
/*N*/ 
/*N*/ 		Matrix4D aTmp;
/*N*/ 		aSceneMatrix = aTmp;
/*N*/ 		if(IsPieChart() && IsReal3D() )
/*N*/ 		{
/*?*/ 			aSceneMatrix.RotateX(-F_PI/3);
/*?*/ 			if(pScene)
/*?*/ 				pScene->NbcSetTransform(aSceneMatrix);
/*N*/ 		}
/*N*/ 		else if(pScene)
/*?*/ 			pScene->NbcSetTransform(aSceneMatrix);
/*N*/ 
/*N*/ 
/*N*/ 		if( IsReal3D()) //#56798# QuickFix 5.0-Final
/*N*/ 		{
/*N*/ 			bClearDepth=TRUE;
/*N*/ 			ULONG	i, nORow=aDataRowAttrList.Count();
/*N*/ 			for(i=0;i<nORow;i++)
/*N*/ 			{
/*N*/ //-/				aDataRowAttrList.GetObject(i)->ClearItem(SID_ATTR_3D_DEPTH);
/*N*/ 				aDataRowAttrList.GetObject(i)->ClearItem( SDRATTR_3DOBJ_DEPTH );
/*N*/ 				aDataRowAttrList.GetObject(i)->Put( Svx3DDoubleSidedItem( TRUE ));
/*N*/ 
/*N*/   				//if(eChartStyle == CHSTYLE_3D_STRIPE || eChartStyle==CHSTYLE_3D_PIE)
/*N*/ //-/					aDataRowAttrList.GetObject(i)->Put(SfxBoolItem(SID_ATTR_3D_DOUBLE_SIDED,TRUE));
/*N*/ //  				else
/*N*/ //-/					aDataRowAttrList.GetObject(i)->Put(SfxBoolItem(SID_ATTR_3D_DOUBLE_SIDED,FALSE));
/*N*/ //  					aDataRowAttrList.GetObject(i)->Put(Svx3DDoubleSidedItem(FALSE));
/*N*/ 			}
/*N*/ 			nORow=aDataPointAttrList.Count();
/*N*/ 			SfxItemSet	*	pAttributes;
/*N*/ 			for(i=0;i<nORow;i++)
/*N*/ 			{
/*N*/ 				pAttributes = aDataPointAttrList.GetObject(i);
/*N*/ 				if (pAttributes != NULL)
/*N*/ 				{
/*?*/ 					pAttributes->ClearItem( SDRATTR_3DOBJ_DEPTH );
/*?*/ 					pAttributes->ClearItem( SDRATTR_3DOBJ_DOUBLE_SIDED );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			nORow=aSwitchDataPointAttrList.Count();
/*N*/ 			for(i=0;i<nORow;i++)
/*N*/ 			{
/*N*/ 				pAttributes = aSwitchDataPointAttrList.GetObject(i);
/*N*/ 				if (pAttributes != NULL)
/*N*/ 				{
/*?*/ 					pAttributes->ClearItem( SDRATTR_3DOBJ_DEPTH );
/*?*/ 					pAttributes->ClearItem( SDRATTR_3DOBJ_DOUBLE_SIDED );
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			// switch off rounded edges for:
/*N*/ 			// area charts
/*N*/ 			Svx3DPercentDiagonalItem aItem(
/*N*/   				(eStyle == CHSTYLE_3D_AREA || eStyle == CHSTYLE_3D_STACKEDAREA || eStyle == CHSTYLE_3D_PERCENTAREA ||
/*N*/                     eStyle == CHSTYLE_3D_PIE )
/*N*/   				? 0 : 5 );
/*N*/ 
/*N*/ 			// item-set for whole chart used if item in series is not set
/*N*/ 			pDummyAttr->Put( aItem );
/*N*/ 
/*N*/   			for( i = 0; i < aDataRowAttrList.Count(); i++ )
/*N*/   			{
/*N*/ 				aDataRowAttrList.GetObject( i )->Put( aItem );
/*N*/   			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//	If set to xy-chart or certain stock chart variants then turn on
/*N*/ 		//	automatic calculation of the origin for all y-axes as default.
/*N*/ 		//	This affects the second y-axis even if it is not (yet) visible.
/*N*/ 		//	This was previously done in the autopilot but belongs here
/*N*/ 		//	because XML loading calls this method but not the autopilot.
/*N*/ 		if (	IsXYChart()
/*N*/ 			||	(eChartStyle == CHSTYLE_2D_STOCK_1)
/*N*/ 			||	(eChartStyle == CHSTYLE_2D_STOCK_2))
/*N*/ 		{
/*?*/ 			SfxItemSet aAutoOrigin (*pItemPool, SCHATTR_AXIS_AUTO_ORIGIN, SCHATTR_AXIS_AUTO_ORIGIN);
/*?*/ 			aAutoOrigin.Put (SfxBoolItem (SCHATTR_AXIS_AUTO_ORIGIN, TRUE));
/*?*/ 			pChartYAxis->SetAttributes (aAutoOrigin);
/*?*/ 			//	The second y-axis exists (pChartBAxis!=NULL) even if it is not
/*?*/ 			//	visible.
/*?*/ 			pChartBAxis->SetAttributes (aAutoOrigin);
/*N*/ 		}
/*N*/ 			
/*N*/ 		SetUseRelativePositions(TRUE);// New arrangement (see SID_NEW_ARRANGEMENT)
/*N*/ 		eOldChartStyle = eChartStyle;
/*N*/ 
/*N*/ 		//	Set the number of data series that are displayed as lines to a fixed value.
/*N*/ 		//	This is one for the combined chart types of columns/stacked columns and lines
/*N*/ 		//	and zero for all other chart types.
/*N*/ 
/*N*/         // #103682# this seems not to be necessary.  This method is called on
/*N*/         // XML-import very early, when the data is 1x1 in size.  The number of
/*N*/         // lines must be preserved until in the end the original size-data is
/*N*/         // set.  In all other cases when this method is called, the chart-type
/*N*/         // itself should handle a value that is too big or small. (Advantage: if
/*N*/         // you change to bar with no lines and then back, you get the old
/*N*/         // value).
/*N*/ // 		switch (eStyle)
/*N*/ // 		{
/*N*/ // 			case	CHSTYLE_2D_LINE_COLUMN:
/*N*/ // 			case	CHSTYLE_2D_LINE_STACKEDCOLUMN:
/*N*/ //                 {
/*N*/ //                     long nNumLines = GetNumLinesColChart();
/*N*/ //                     if( nNumLines < 1 ||
/*N*/ //                         nNumLines >= GetRowCount() )
/*N*/ //                         SetNumLinesColChart (1);
/*N*/ //                 }
/*N*/ // 				break;
/*N*/ 				
/*N*/ // 			default:
/*N*/ // 				SetNumLinesColChart (0);
/*N*/ // 		}
/*N*/ 
/*N*/         // #104525# however the default for a combi-chart is one line.  So if
/*N*/         // the setting is on 0, we have to change it to 1.
/*N*/         if( ( eStyle == CHSTYLE_2D_LINE_COLUMN
/*N*/               || eStyle == CHSTYLE_2D_LINE_STACKEDCOLUMN )
/*N*/             && GetNumLinesColChart() == 0 )
/*N*/         {
/*?*/             SetNumLinesColChart( 1 );
/*N*/         }
/*N*/ 
/*N*/         // broadcast UIFeature chang
/*N*/         // #85069# introduced because the 3d effect flyer has to be disabled for 2d charts
/*N*/         Broadcast( SfxSimpleHint( SFX_HINT_MODECHANGED ));
/*N*/ 
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*
\************************************************************************/



/*************************************************************************
|*
|* Language setzen
|*
\************************************************************************/

/*N*/ void ChartModel::SetLanguage( const LanguageType eLang, const USHORT nId )
/*N*/ {
/*N*/ 	BOOL bChanged = FALSE;
/*N*/ 
/*N*/ 	if( nId == EE_CHAR_LANGUAGE && eLanguage != eLang )
/*N*/ 	{
/*N*/ 		eLanguage = eLang;
/*N*/ 		bChanged = TRUE;
/*N*/ 	}
/*N*/ 	else if( nId == EE_CHAR_LANGUAGE_CJK && eLanguageCJK != eLang )
/*N*/ 	{
/*N*/ 		eLanguageCJK = eLang;
/*N*/ 		bChanged = TRUE;
/*N*/ 	}
/*N*/ 	else if( nId == EE_CHAR_LANGUAGE_CTL && eLanguageCTL != eLang )
/*N*/ 	{
/*N*/ 		eLanguageCTL = eLang;
/*N*/ 		bChanged = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bChanged )
/*N*/ 	{
/*N*/ 		GetDrawOutliner().SetDefaultLanguage( eLang );
/*N*/ 		pOutliner->SetDefaultLanguage( eLang );
/*N*/ 		pItemPool->SetPoolDefaultItem( SvxLanguageItem( eLang, nId ) );
/*N*/ 		SetChanged( bChanged );
/*N*/ 	}
/*N*/ }


/*************************************************************************
|*
|* Return language
|*
\************************************************************************/

/*N*/ LanguageType ChartModel::GetLanguage( const USHORT nId ) const
/*N*/ {
/*N*/ 	LanguageType eLangType = eLanguage;
/*N*/ 
/*N*/ 	if( nId == EE_CHAR_LANGUAGE_CJK )
/*N*/ 		eLangType = eLanguageCJK;
/*N*/ 	else if( nId == EE_CHAR_LANGUAGE_CTL )
/*N*/ 		eLangType = eLanguageCTL;
/*N*/ 
/*N*/ 	return eLangType;
/*N*/ }

IMPL_LINK( ChartModel, NotifyUndoActionHdl, SfxUndoAction*, pUndo )
{
    DBG_ASSERT(!m_pUndoActionFromDraw, "New UndoAction from Draw received while elder is not handled yet");
    if(m_bDeleteUndoActionNotificationFromDraw)
    {
        if(pUndo)
            delete pUndo;
    }
    else
    {
        m_pUndoActionFromDraw = pUndo;
    }
    return 1;
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
