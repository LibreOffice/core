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
#include <bf_svx/editobj.hxx>
#include <bf_svx/scripttypeitem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_sfx2/app.hxx>
#include <bf_svtools/zformat.hxx>
#include <vcl/image.hxx>
#include <tools/rcid.h>
#include <vcl/virdev.hxx>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>


#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#ifndef _UNOTOOLS_INTLWRAPPER_HXX
#include <unotools/intlwrapper.hxx>
#endif

#include "autoform.hxx"
#include "patattr.hxx"
#include "addincol.hxx"
#include "adiasync.hxx"
#include "userlist.hxx"
#include "interpre.hxx"
#include "strload.hxx"
#include "docpool.hxx"
#include "unitconv.hxx"
#include "globstr.hrc"
#include "bf_sc.hrc"
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif

namespace binfilter {
// -----------------------------------------------------------------------

#define CLIPST_AVAILABLE	0
#define CLIPST_CAPTURED		1
#define CLIPST_DELETE		2
#define CLIPST_DRAW			3

ScDocShellRef*	ScGlobal::pDrawClipDocShellRef = NULL;
ScAutoFormat*	ScGlobal::pAutoFormat = NULL;
FuncCollection* ScGlobal::pFuncCollection = NULL;
ScUnoAddInCollection* ScGlobal::pAddInCollection = NULL;
ScUserList*		ScGlobal::pUserList = NULL;
String**		ScGlobal::ppRscString = NULL;
LanguageType    ScGlobal::eLnge = LANGUAGE_SYSTEM;
::com::sun::star::lang::Locale*		ScGlobal::pLocale = NULL;
SvtSysLocale*   ScGlobal::pSysLocale = NULL;
const CharClass*  ScGlobal::pCharClass = NULL;
const LocaleDataWrapper*  ScGlobal::pLocaleData = NULL;
CalendarWrapper* ScGlobal::pCalendar = NULL;
CollatorWrapper* ScGlobal::pCollator = NULL;
CollatorWrapper* ScGlobal::pCaseCollator = NULL;
::utl::TransliterationWrapper* ScGlobal::pTransliteration = NULL;
::utl::TransliterationWrapper* ScGlobal::pCaseTransliteration = NULL;
IntlWrapper*    ScGlobal::pScIntlWrapper = NULL;
sal_Unicode		ScGlobal::cListDelimiter = ',';
String*			ScGlobal::pEmptyString = NULL;
String*			ScGlobal::pStrClipDocName = NULL;

SvxBrushItem*	ScGlobal::pEmptyBrushItem = NULL;
SvxBrushItem*	ScGlobal::pButtonBrushItem = NULL;
SvxBrushItem*	ScGlobal::pEmbeddedBrushItem = NULL;
SvxBrushItem*	ScGlobal::pProtectedBrushItem = NULL;

ImageList*      ScGlobal::pOutlineBitmaps = NULL;
ImageList*      ScGlobal::pOutlineBitmapsHC = NULL;

ScFunctionList* ScGlobal::pStarCalcFunctionList = NULL;
ScFunctionMgr*	ScGlobal::pStarCalcFunctionMgr	= NULL;

ScUnitConverter* ScGlobal::pUnitConverter = NULL;
SvNumberFormatter* ScGlobal::pEnglishFormatter = NULL;

double			ScGlobal::nScreenPPTX			= 96.0;
double			ScGlobal::nScreenPPTY			= 96.0;

USHORT			ScGlobal::nDefFontHeight		= 240;
USHORT			ScGlobal::nStdRowHeight			= 257;

long			ScGlobal::nLastRowHeightExtra	= 0;
long			ScGlobal::nLastColWidthExtra	= STD_EXTRA_WIDTH;

static USHORT nPPTZoom = 0;		// ScreenZoom used to determine nScreenPPTX/Y


// ... oder so?

BOOL bOderSo;

class SfxViewShell;
SfxViewShell* pScActiveViewShell = NULL;			//! als Member !!!!!
USHORT nScClickMouseModifier = 0;					//! dito
USHORT nScFillModeMouseModifier = 0;				//! dito

// Hack: ScGlobal::GetUserList() muss InitAppOptions in der UI aufrufen,
//		 damit UserList aus Cfg geladen wird

void global_InitAppOptions();

// -----------------------------------------------------------------------

/*N*/ String ScTripel::GetColRowString( BOOL bAbsolute ) const
/*N*/ {
/*N*/ 	String aString;
/*N*/ 	if (bAbsolute)
/*N*/ 		aString.Append( '$' );
/*N*/ 
/*N*/ 	if ( nCol < 26 )
/*N*/ 		aString.Append( (sal_Unicode) ( 'A' + nCol ) );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aString.Append( (sal_Unicode) ( 'A' + ( nCol / 26 ) - 1 ) );
/*N*/ 		aString.Append( (sal_Unicode) ( 'A' + ( nCol % 26 ) ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bAbsolute )
/*N*/ 		aString.Append( '$' );
/*N*/ 
/*N*/ 	aString += String::CreateFromInt32(nRow+1);
/*N*/ 
/*N*/ 	return aString;
/*N*/ }

/*N*/ String ScRefTripel::GetRefString(ScDocument* pDoc, USHORT nActTab) const
/*N*/ {
/*N*/ 	if ( !pDoc )
/*N*/ 		return EMPTY_STRING;
/*N*/ 	if ( nTab+1 > pDoc->GetTableCount() )
/*N*/ 		return ScGlobal::GetRscString( STR_NOREF_STR );
/*N*/ 
/*N*/     String aString;
/*N*/     USHORT nFlags = SCA_VALID;
/*N*/     if ( nActTab != nTab )
/*N*/     {
/*N*/         nFlags |= SCA_TAB_3D;
/*N*/         if ( !bRelTab )
/*N*/             nFlags |= SCA_TAB_ABSOLUTE;
/*N*/     }
/*N*/     if ( !bRelCol )
/*N*/         nFlags |= SCA_COL_ABSOLUTE;
/*N*/     if ( !bRelRow )
/*N*/         nFlags |= SCA_ROW_ABSOLUTE;
/*N*/     
/*N*/     ScAddress( nCol, nRow, nTab ).Format( aString, nFlags, pDoc );
/*N*/ 
/*N*/     return aString;
/*N*/ }


//========================================================================
//
//		statische Funktionen
//
//========================================================================

/*N*/ BOOL ScGlobal::HasAttrChanged( const SfxItemSet&  rNewAttrs,
/*N*/ 							   const SfxItemSet&  rOldAttrs,
/*N*/ 							   const USHORT		  nWhich )
/*N*/ {
/*N*/ 	BOOL				bInvalidate = FALSE;
/*N*/ 	const SfxItemState	eNewState	= rNewAttrs.GetItemState( nWhich );
/*N*/ 	const SfxItemState	eOldState	= rOldAttrs.GetItemState( nWhich );
/*N*/ 
/*N*/ 	//----------------------------------------------------------
/*N*/ 
/*N*/ 	if ( eNewState == eOldState )
/*N*/ 	{
/*N*/ 		// beide Items gesetzt
/*N*/ 		// PoolItems, d.h. Pointer-Vergleich zulaessig
/*N*/ 		if ( SFX_ITEM_SET == eOldState )
/*N*/ 			bInvalidate = (&rNewAttrs.Get( nWhich ) != &rOldAttrs.Get( nWhich ));
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// ein Default-Item dabei
/*N*/ 		// PoolItems, d.h. Item-Vergleich noetig
/*N*/ 
/*N*/ 		const SfxPoolItem& rOldItem = ( SFX_ITEM_SET == eOldState )
/*N*/ 					? rOldAttrs.Get( nWhich )
/*N*/ 					: rOldAttrs.GetPool()->GetDefaultItem( nWhich );
/*N*/ 
/*N*/ 		const SfxPoolItem& rNewItem = ( SFX_ITEM_SET == eNewState )
/*N*/ 					? rNewAttrs.Get( nWhich )
/*N*/ 					: rNewAttrs.GetPool()->GetDefaultItem( nWhich );
/*N*/ 
/*N*/ 		bInvalidate = (rNewItem != rOldItem);
/*N*/ 	}
/*N*/ 
/*N*/ 	return bInvalidate;
/*N*/ }

/*N*/ ULONG ScGlobal::GetStandardFormat( SvNumberFormatter& rFormatter,
/*N*/ 		ULONG nFormat, short nType )
/*N*/ {
/*N*/ 	const SvNumberformat* pFormat = rFormatter.GetEntry( nFormat );
/*N*/ 	if ( pFormat )
/*N*/ 		return rFormatter.GetStandardFormat( nFormat, nType, pFormat->GetLanguage() );
/*N*/ 	return rFormatter.GetStandardFormat( nType, eLnge );
/*N*/ }

/*N*/ ULONG ScGlobal::GetStandardFormat( double fNumber, SvNumberFormatter& rFormatter,
/*N*/ 		ULONG nFormat, short nType )
/*N*/ {
/*N*/ 	const SvNumberformat* pFormat = rFormatter.GetEntry( nFormat );
/*N*/ 	if ( pFormat )
/*N*/ 		return rFormatter.GetStandardFormat( fNumber, nFormat, nType,
/*N*/ 			pFormat->GetLanguage() );
/*?*/ 	return rFormatter.GetStandardFormat( nType, eLnge );
/*N*/ }


// static
/*N*/ SvNumberFormatter* ScGlobal::GetEnglishFormatter()
/*N*/ {
/*N*/     if ( !pEnglishFormatter )
/*N*/     {
/*N*/         pEnglishFormatter = new SvNumberFormatter(
/*N*/             ::legacy_binfilters::getLegacyProcessServiceFactory(), LANGUAGE_ENGLISH_US );
/*N*/         pEnglishFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );
/*N*/     }
/*N*/     return pEnglishFormatter;
/*N*/ }


//------------------------------------------------------------------------

/*N*/ BOOL ScGlobal::CheckWidthInvalidate( BOOL& bNumFormatChanged,
/*N*/ 									 const SfxItemSet& rNewAttrs,
/*N*/ 									 const SfxItemSet& rOldAttrs )
/*N*/ {
/*N*/ 	// Ueberpruefen, ob Attributaenderungen in rNewAttrs gegnueber
/*N*/ 	// rOldAttrs die Textbreite an einer Zelle ungueltig machen
/*N*/ 
/*N*/ 	bNumFormatChanged =
/*N*/ 			HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_VALUE_FORMAT );
/*N*/ 	return ( bNumFormatChanged
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_LANGUAGE_FORMAT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CJK_FONT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CTL_FONT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_HEIGHT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CJK_FONT_HEIGHT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CTL_FONT_HEIGHT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_WEIGHT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CJK_FONT_WEIGHT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CTL_FONT_WEIGHT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_POSTURE )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CJK_FONT_POSTURE )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_CTL_FONT_POSTURE )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_UNDERLINE )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_CROSSEDOUT )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_CONTOUR )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_FONT_SHADOWED )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_ORIENTATION )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_ROTATE_VALUE )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_ROTATE_MODE )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_LINEBREAK )
/*N*/ 		|| HasAttrChanged( rNewAttrs, rOldAttrs, ATTR_MARGIN )
/*N*/ 		);
/*N*/ }




/*N*/ ScAutoFormat* ScGlobal::GetAutoFormat()
/*N*/ {
/*N*/ 	if ( !pAutoFormat )
/*N*/ 	{
/*N*/ 		pAutoFormat = new ScAutoFormat;
/*N*/ 		pAutoFormat->Load();
/*N*/ 	}
/*N*/ 
/*N*/ 	return pAutoFormat;
/*N*/ }

/*N*/ FuncCollection* ScGlobal::GetFuncCollection()
/*N*/ {
/*N*/ 	if (!pFuncCollection)
/*N*/ 		pFuncCollection = new FuncCollection();
/*N*/ 	return pFuncCollection;
/*N*/ }

/*N*/ ScUnoAddInCollection* ScGlobal::GetAddInCollection()
/*N*/ {
/*N*/ 	if (!pAddInCollection)
/*N*/ 		pAddInCollection = new ScUnoAddInCollection();
/*N*/ 	return pAddInCollection;
/*N*/ }

/*N*/ ScUserList* ScGlobal::GetUserList()
/*N*/ {
/*N*/ 	// Hack: Cfg-Item an der App ggF. laden
/*N*/ 
/*N*/ 	global_InitAppOptions();
/*N*/ 
/*N*/ 	if (!pUserList)
/*N*/ 		pUserList = new ScUserList();
/*N*/ 	return pUserList;
/*N*/ }

/*N*/ void ScGlobal::SetUserList( const ScUserList* pNewList )
/*N*/ {
/*N*/ 	if ( pNewList )
/*N*/ 	{
/*N*/ 		if ( !pUserList )
/*N*/ 			pUserList = new ScUserList( *pNewList );
/*N*/ 		else
/*?*/ 			*pUserList = *pNewList;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		delete pUserList;
/*?*/ 		pUserList = NULL;
/*N*/ 	}
/*N*/ }

/*N*/ const String& ScGlobal::GetRscString( USHORT nIndex )
/*N*/ {
/*N*/ 	DBG_ASSERT( nIndex <= STR_COUNT,
/*N*/ 		"-ScGlobal::GetRscString(): Index zu gross!");
/*N*/ 	if( !ppRscString[ nIndex ] )
/*N*/ 	{
/*N*/ 		ppRscString[ nIndex ] =
/*N*/ 			new String( ScRscStrLoader( RID_GLOBSTR, nIndex ).GetString() );
/*N*/ 	}
/*N*/ 	return *ppRscString[ nIndex ];
/*N*/ }

/*N*/ String ScGlobal::GetErrorString(USHORT nErrNumber)
/*N*/ {
/*N*/ 	String sResStr;
/*N*/ 	switch (nErrNumber)
/*N*/ 	{
/*N*/ 		case NOVALUE     : nErrNumber = STR_NV_STR; break;
/*N*/ 		case errNoRef    : nErrNumber = STR_NO_REF_TABLE; break;
/*N*/ 		case errNoName   : nErrNumber = STR_NO_NAME_REF; break;
/*N*/         case errNoAddin  : nErrNumber = STR_NO_ADDIN; break;
/*N*/         case errNoMacro  : nErrNumber = STR_NO_MACRO; break;
/*N*/ 		case errDoubleRef:
/*N*/ 		case errNoValue  : nErrNumber = STR_NO_VALUE; break;
/*N*/ 
/*N*/ 		default		     : sResStr = GetRscString(STR_ERROR_STR);
/*N*/ 						   sResStr += String::CreateFromInt32( nErrNumber );
/*N*/ 						   nErrNumber = 0;
/*N*/ 						   break;
/*N*/ 	}
/*N*/ 	if( nErrNumber )
/*N*/ 		sResStr = GetRscString( nErrNumber );
/*N*/ 	return sResStr;
/*N*/ }

/*N*/ const String& ScGlobal::GetEmptyString()
/*N*/ {
/*N*/ 	return *pEmptyString;
/*N*/ }


/*N*/ void ScGlobal::Init()
/*N*/ {
/*N*/ 	pEmptyString = new String;
/*N*/ 
/*N*/ 	//	Die Default-Sprache fuer Zahlenformate (ScGlobal::eLnge)
/*N*/ 	//	muss immer LANGUAGE_SYSTEM sein
/*N*/ 	//!	Dann kann auch die Variable raus
/*N*/ 	eLnge = LANGUAGE_SYSTEM;
/*N*/ 
/*N*/ 	//!	Wenn Sortierung etc. von der Sprache der installierten Offfice-Version
/*N*/     //! abhaengen sollen, hier "Application::GetSettings().GetUILanguage()"
/*N*/ 	String aLanguage, aCountry;
/*N*/     LanguageType eOfficeLanguage = Application::GetSettings().GetLanguage();
/*N*/ 	pLocale = new ::com::sun::star::lang::Locale( MsLangId::convertLanguageToLocale( eOfficeLanguage ));
/*N*/     pSysLocale = new SvtSysLocale;
/*N*/     pCharClass = pSysLocale->GetCharClassPtr();
/*N*/     pLocaleData = pSysLocale->GetLocaleDataPtr();
/*N*/ 	pCalendar = new CalendarWrapper( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*N*/ 	pCalendar->loadDefaultCalendar( *pLocale );
/*N*/ 	pCollator = new CollatorWrapper( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*N*/ 	pCollator->loadDefaultCollator( *pLocale, SC_COLLATOR_IGNORES );
/*N*/ 	pCaseCollator = new CollatorWrapper( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*N*/ 	pCaseCollator->loadDefaultCollator( *pLocale, 0 );
/*N*/     pTransliteration = new ::utl::TransliterationWrapper(
/*N*/         ::legacy_binfilters::getLegacyProcessServiceFactory(), SC_TRANSLITERATION_IGNORECASE );
/*N*/     pTransliteration->loadModuleIfNeeded( eOfficeLanguage );
/*N*/     pCaseTransliteration = new ::utl::TransliterationWrapper(
/*N*/         ::legacy_binfilters::getLegacyProcessServiceFactory(), SC_TRANSLITERATION_CASESENSE );
/*N*/     pCaseTransliteration->loadModuleIfNeeded( eOfficeLanguage );
/*N*/     pScIntlWrapper = new IntlWrapper( ::legacy_binfilters::getLegacyProcessServiceFactory(), *pLocale );
/*N*/ 
/*N*/ 	ppRscString = new String *[ STR_COUNT+1 ];
/*N*/ 	for( USHORT nC = 0 ; nC <= STR_COUNT ; nC++ ) ppRscString[ nC ] = NULL;
/*N*/ 
/*N*/ 	pEmptyBrushItem = new SvxBrushItem( Color( COL_TRANSPARENT ) );
/*N*/     pButtonBrushItem = new SvxBrushItem( Color() );
/*N*/ 	pEmbeddedBrushItem = new SvxBrushItem( Color( COL_LIGHTCYAN ) );
/*N*/ 	pProtectedBrushItem = new SvxBrushItem( Color( COL_LIGHTGRAY ) );
/*N*/ 
/*N*/ 	UpdatePPT(NULL);
/*N*/ 	ScCompiler::Init();
/*N*/ 	srand( (unsigned) time( NULL ) );		// Random Seed Init fuer Interpreter
/*N*/ 
/*N*/ 	InitAddIns();
/*N*/ 
/*N*/ 	pStrClipDocName = new String( ScResId( SCSTR_NONAME ) );
/*N*/ 	*pStrClipDocName += '1';
/*N*/ 
/*N*/ 	//	ScDocumentPool::InitVersionMaps() ist schon vorher gerufen worden
/*N*/ }

/*N*/ void ScGlobal::UpdatePPT( OutputDevice* pDev )
/*N*/ {
/*N*/ 	USHORT nCurrentZoom = Application::GetSettings().GetStyleSettings().GetScreenZoom();
/*N*/ 	if ( nCurrentZoom != nPPTZoom )
/*N*/ 	{
/*N*/ 		//	Screen PPT values must be updated when ScreenZoom has changed.
/*N*/ 		//	If called from Window::DataChanged, the window is passed as pDev,
/*N*/ 		//	to make sure LogicToPixel uses a device which already uses the new zoom.
/*N*/ 		//	For the initial settings, NULL is passed and GetDefaultDevice used.
/*N*/ 
/*N*/ 		if ( !pDev )
/*N*/ 			pDev = Application::GetDefaultDevice();
/*N*/ 		Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
/*N*/ 		nScreenPPTX = aPix1000.X() / 1000.0;
/*N*/ 		nScreenPPTY = aPix1000.Y() / 1000.0;
/*N*/ 		nPPTZoom = nCurrentZoom;
/*N*/ 	}
/*N*/ }




/*N*/ void ScGlobal::InitTextHeight(SfxItemPool* pPool)
/*N*/ {
/*N*/ 	if (!pPool)
/*N*/ 	{
/*?*/ 		DBG_ERROR("kein Pool bei ScGlobal::InitTextHeight");
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	const ScPatternAttr* pPattern = (const ScPatternAttr*)&pPool->GetDefaultItem(ATTR_PATTERN);
/*N*/ 	if (!pPattern)
/*N*/ 	{
/*?*/ 		DBG_ERROR("kein Default-Pattern bei ScGlobal::InitTextHeight");
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ //	String aTestString('X');
/*N*/ 	OutputDevice* pDefaultDev = Application::GetDefaultDevice();
/*N*/ 	VirtualDevice aVirtWindow( *pDefaultDev );
/*N*/ 	aVirtWindow.SetMapMode(MAP_PIXEL);
/*N*/ 	Font aDefFont;
/*N*/ 	pPattern->GetFont(aDefFont, SC_AUTOCOL_BLACK, &aVirtWindow);		// font color doesn't matter here
/*N*/ 	aVirtWindow.SetFont(aDefFont);
/*N*/ 	nDefFontHeight = (USHORT) aVirtWindow.PixelToLogic(Size(0, aVirtWindow.GetTextHeight()),
/*N*/ 								MAP_TWIP).Height();
/*N*/ 
/*N*/ 	const SvxMarginItem* pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);
/*N*/ 
/*N*/ 	nStdRowHeight = (USHORT) ( nDefFontHeight +
/*N*/ 								pMargin->GetTopMargin() + pMargin->GetBottomMargin()
/*N*/ 								- STD_ROWHEIGHT_DIFF );
/*N*/ }

/*N*/ void ScGlobal::Clear()
/*N*/ {
/*N*/ 	// asyncs _vor_ ExitExternalFunc zerstoeren!
/*N*/ 	theAddInAsyncTbl.DeleteAndDestroy( 0, theAddInAsyncTbl.Count() );
/*N*/ 	ExitExternalFunc();
/*N*/ 	DELETEZ(pAutoFormat);
/*N*/ 	DELETEZ(pFuncCollection);
/*N*/ 	DELETEZ(pAddInCollection);
/*N*/ 	DELETEZ(pUserList);
/*N*/ 
/*N*/ 	for( USHORT nC = 0 ; nC < STR_COUNT ; nC++ )
/*N*/ 		if( ppRscString ) delete ppRscString[ nC ];
/*N*/ 	delete[] ppRscString;
/*N*/ 	ppRscString = NULL;
/*N*/ 
/*N*/ 	DELETEZ(pStarCalcFunctionList);		// vor ResMgr zerstoeren!
/*N*/ 	DELETEZ(pStarCalcFunctionMgr);
/*N*/ 	ScCompiler::DeInit();
/*N*/ 	ScInterpreter::GlobalExit();			// statischen Stack loeschen
/*N*/ 
/*N*/ 	DELETEZ(pEmptyBrushItem);
/*N*/ 	DELETEZ(pButtonBrushItem);
/*N*/ 	DELETEZ(pEmbeddedBrushItem);
/*N*/ 	DELETEZ(pProtectedBrushItem);
/*N*/     DELETEZ(pOutlineBitmaps);
/*N*/     DELETEZ(pOutlineBitmapsHC);
/*N*/ //	DELETEZ(pAnchorBitmap);
/*N*/ //	DELETEZ(pGrayAnchorBitmap);
/*N*/     DELETEZ(pEnglishFormatter);
/*N*/     DELETEZ(pCaseTransliteration);
/*N*/     DELETEZ(pTransliteration);
/*N*/ 	DELETEZ(pCaseCollator);
/*N*/ 	DELETEZ(pCollator);
/*N*/ 	DELETEZ(pCalendar);
/*N*/     //! do NOT delete pCharClass since it is a pointer to the single SvtSysLocale instance
/*N*/     pCharClass = NULL;
/*N*/     //! do NOT delete pLocaleData since it is a pointer to the single SvtSysLocale instance
/*N*/     pLocaleData = NULL;
/*N*/     DELETEZ(pSysLocale);
/*N*/ 	DELETEZ(pLocale);
/*N*/     DELETEZ(pScIntlWrapper);
/*N*/ 	DELETEZ(pStrClipDocName);
/*N*/ 
/*N*/ 	DELETEZ(pUnitConverter);
/*N*/ 
/*N*/ 	ScDocumentPool::DeleteVersionMaps();
/*N*/ 
/*N*/ 	DELETEZ(pEmptyString);
/*N*/ }

//------------------------------------------------------------------------

// static
/*N*/ CharSet ScGlobal::GetCharsetValue( const String& rCharSet )
/*N*/ {
/*N*/ 	// new TextEncoding values
/*N*/ 	if ( CharClass::isAsciiNumeric( rCharSet ) )
/*N*/ 	{
/*?*/ 		sal_Int32 nVal = rCharSet.ToInt32();
/*?*/ 		if ( !nVal || nVal == RTL_TEXTENCODING_DONTKNOW )
/*?*/ 			return gsl_getSystemTextEncoding();
/*?*/ 		return (CharSet) nVal;
/*N*/ 	}
/*N*/ 	// old CharSet values for compatibility
/*N*/ 	else if	(rCharSet.EqualsIgnoreCaseAscii("ANSI")		) return RTL_TEXTENCODING_MS_1252;
/*N*/ 	else if (rCharSet.EqualsIgnoreCaseAscii("MAC")		) return RTL_TEXTENCODING_APPLE_ROMAN;
/*N*/ 	else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC")	) return RTL_TEXTENCODING_IBM_850;
/*N*/ 	else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_437")) return RTL_TEXTENCODING_IBM_437;
/*N*/ 	else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_850")) return RTL_TEXTENCODING_IBM_850;
/*N*/ 	else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_860")) return RTL_TEXTENCODING_IBM_860;
/*N*/ 	else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_861")) return RTL_TEXTENCODING_IBM_861;
/*N*/ 	else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_863")) return RTL_TEXTENCODING_IBM_863;
/*N*/ 	else if (rCharSet.EqualsIgnoreCaseAscii("IBMPC_865")) return RTL_TEXTENCODING_IBM_865;
/*N*/ //	else if (rCharSet.EqualsIgnoreCaseAscii("SYSTEM")	) return gsl_getSystemTextEncoding();
/*N*/ 	else return gsl_getSystemTextEncoding();
/*N*/ }

//------------------------------------------------------------------------

// static
/*N*/ String ScGlobal::GetCharsetString( CharSet eVal )
/*N*/ {
/*N*/ 	const sal_Char* pChar;
/*N*/  	switch ( eVal )
/*N*/  	{
/*N*/  		// old CharSet strings for compatibility
/*N*/  		case RTL_TEXTENCODING_MS_1252:		pChar = "ANSI";			break;
/*N*/  		case RTL_TEXTENCODING_APPLE_ROMAN:	pChar = "MAC";			break;
/*N*/  		// IBMPC == IBMPC_850
/*N*/  		case RTL_TEXTENCODING_IBM_437:		pChar = "IBMPC_437";	break;
/*N*/  		case RTL_TEXTENCODING_IBM_850:		pChar = "IBMPC_850";	break;
/*N*/  		case RTL_TEXTENCODING_IBM_860:		pChar = "IBMPC_860";	break;
/*N*/  		case RTL_TEXTENCODING_IBM_861:		pChar = "IBMPC_861";	break;
/*N*/  		case RTL_TEXTENCODING_IBM_863:		pChar = "IBMPC_863";	break;
/*N*/  		case RTL_TEXTENCODING_IBM_865:		pChar = "IBMPC_865";	break;
/*N*/  		case RTL_TEXTENCODING_DONTKNOW:		pChar = "SYSTEM";		break;
/*N*/  		// new string of TextEncoding value
/*N*/  		default:
/*N*/  			return String::CreateFromInt32( eVal );
/*N*/  	}
/*N*/  	return String::CreateFromAscii(pChar);
/*N*/  }

//------------------------------------------------------------------------

/*N*/ ScFunctionList* ScGlobal::GetStarCalcFunctionList()
/*N*/ {
/*N*/ 	if ( !pStarCalcFunctionList )
/*N*/ 		pStarCalcFunctionList = new	ScFunctionList;
/*N*/ 
/*N*/ 	return pStarCalcFunctionList;
/*N*/ }

// static
/*N*/ ScUnitConverter* ScGlobal::GetUnitConverter()
/*N*/ {
/*N*/ 	if ( !pUnitConverter )
/*N*/ 		pUnitConverter = new ScUnitConverter;
/*N*/ 
/*N*/ 	return pUnitConverter;
/*N*/ }


//------------------------------------------------------------------------

// static
/*N*/ const sal_Unicode* ScGlobal::UnicodeStrChr( const sal_Unicode* pStr,
/*N*/ 			sal_Unicode c )
/*N*/ {
/*N*/ 	if ( !pStr )
/*N*/ 		return NULL;
/*N*/ 	while ( *pStr )
/*N*/ 	{
/*N*/ 		if ( *pStr == c )
/*N*/ 			return pStr;
/*N*/ 		pStr++;
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }


//------------------------------------------------------------------------

/*N*/ BOOL ScGlobal::EETextObjEqual( const EditTextObject* pObj1,
/*N*/ 							   const EditTextObject* pObj2 )
/*N*/ {
/*N*/ 	if ( pObj1 == pObj2 )				// both empty or the same object
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	if ( pObj1 && pObj2 )
/*N*/ 	{
/*N*/ 		//	first test for equal text content
/*N*/ 		USHORT nParCount = pObj1->GetParagraphCount();
/*N*/ 		if ( nParCount != pObj2->GetParagraphCount() )
/*N*/ 			return FALSE;
/*N*/ 		for (USHORT nPar=0; nPar<nParCount; nPar++)
/*N*/ 			if ( pObj1->GetText(nPar) != pObj2->GetText(nPar) )
/*N*/ 				return FALSE;
/*N*/ 
/*N*/ 		SvMemoryStream	aStream1;
/*N*/ 		SvMemoryStream	aStream2;
/*N*/ 		pObj1->Store( aStream1 );
/*N*/ 		pObj2->Store( aStream2 );
/*N*/ 		ULONG nSize = aStream1.Tell();
/*N*/ 		if ( aStream2.Tell() == nSize )
/*N*/ 			if ( !memcmp( aStream1.GetData(), aStream2.GetData(), (USHORT) nSize ) )
/*N*/ 				return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }


//------------------------------------------------------------------------

/*N*/ BYTE ScGlobal::GetDefaultScriptType()
/*N*/ {
/*N*/ 	//	Used when text contains only WEAK characters.
/*N*/ 	//	Script type of office language is used then (same as GetEditDefaultLanguage,
/*N*/ 	//	to get consistent behavior of text in simple cells and EditEngine,
/*N*/ 	//	also same as GetAppLanguage() in Writer)
/*N*/ 
/*N*/ 	return (BYTE) SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguage() );
/*N*/ }

/*N*/ LanguageType ScGlobal::GetEditDefaultLanguage()
/*N*/ {
/*N*/ 	//	used for EditEngine::SetDefaultLanguage
/*N*/ 
/*N*/ 	return Application::GetSettings().GetLanguage();
/*N*/ }


//------------------------------------------------------------------------

/*N*/ void ScGlobal::AddLanguage( SfxItemSet& rSet, SvNumberFormatter& rFormatter )
/*N*/ {
/*N*/     DBG_ASSERT( rSet.GetItemState( ATTR_LANGUAGE_FORMAT, FALSE ) == SFX_ITEM_DEFAULT,
/*N*/         "ScGlobal::AddLanguage - language already added");
/*N*/ 
/*N*/     const SfxPoolItem* pHardItem;
/*N*/     if ( rSet.GetItemState( ATTR_VALUE_FORMAT, FALSE, &pHardItem ) == SFX_ITEM_SET )
/*N*/     {
/*N*/         const SvNumberformat* pHardFormat = rFormatter.GetEntry(
/*N*/             ((const SfxUInt32Item*)pHardItem)->GetValue() );
/*N*/ 
/*N*/         ULONG nParentFmt = 0;   // pool default
/*N*/         const SfxItemSet* pParent = rSet.GetParent();
/*N*/         if ( pParent )
/*N*/             nParentFmt = ((const SfxUInt32Item&)pParent->Get( ATTR_VALUE_FORMAT )).GetValue();
/*N*/         const SvNumberformat* pParFormat = rFormatter.GetEntry( nParentFmt );
/*N*/ 
/*N*/         if ( pHardFormat && pParFormat &&
/*N*/                 (pHardFormat->GetLanguage() != pParFormat->GetLanguage()) )
/*N*/             rSet.Put( SvxLanguageItem( pHardFormat->GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
/*N*/     }
/*N*/ }


/*N*/ ScFunctionList::ScFunctionList() :
/*N*/ 		nMaxFuncNameLen	( 0 )
/*N*/ {
/*N*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScFuncDesc*		pDesc	= NULL;
}

//------------------------------------------------------------------------

/*N*/ ScFunctionList::~ScFunctionList()
/*N*/ {
/*N*/ 	ScFuncDesc* pDesc = (ScFuncDesc*)aFunctionList.First();
/*N*/ 	while (pDesc)
/*N*/ 	{
/*N*/ 		delete pDesc;
/*N*/ 		pDesc = (ScFuncDesc*)aFunctionList.Next();
/*N*/ 	}
/*N*/ }


//========================================================================
// class ScFuncDesc:

/*N*/ ScFuncDesc::~ScFuncDesc()
/*N*/ {
/*N*/ 	USHORT		nArgs;
/*N*/ 
/*N*/ 	nArgs = nArgCount;
/*N*/ 	if (nArgs >= VAR_ARGS) nArgs -= VAR_ARGS-1;
/*N*/ 	if (nArgs)
/*N*/ 	{
/*?*/ 		for (USHORT i=0; i<nArgs; i++ )
/*?*/ 		{
/*?*/ 			delete aDefArgNames[i];
/*?*/ 			delete aDefArgDescs[i];
/*?*/ 		}
/*?*/ 		delete [] aDefArgNames;
/*?*/ 		delete [] aDefArgDescs;
/*?*/ 		delete [] aDefArgOpt;
/*N*/ 	}
/*N*/ 	if(pFuncName)
/*?*/ 		delete pFuncName;
/*N*/ 
/*N*/ 	if(pFuncDesc)
/*?*/ 		delete pFuncDesc;
/*N*/ }

//========================================================================
// class ScFunctionMgr:

/*N*/ ScFunctionMgr::~ScFunctionMgr()
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 for (USHORT i = 0; i < MAX_FUNCCAT; i++)
/*N*/ }

/*N*/ String ColToAlpha( const USHORT nCol )
/*N*/ {
/*N*/ 	String aStr;
/*N*/ 
/*N*/ 	if ( nCol < 26 )
/*N*/ 		aStr = (sal_Unicode) ( 'A' + nCol );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aStr  = (sal_Unicode) ( 'A' + ( nCol / 26 ) - 1 );
/*N*/ 		aStr += (sal_Unicode) ( 'A' + ( nCol % 26 ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	return aStr;
/*N*/ }

}
