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

//------------------------------------------------------------------------
#include "document.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"

#include "scitems.hxx"
#include <bf_svx/boxitem.hxx>
#include <bf_svx/frmdiritem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/pageitem.hxx>
#include <bf_svx/paperinf.hxx>
#include <bf_svx/sizeitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svtools/itempool.hxx>
#include <bf_svtools/itemset.hxx>

#include "globstr.hrc"
namespace binfilter {

//------------------------------------------------------------------------

/*N*/ TYPEINIT1(ScStyleSheet, SfxStyleSheet);

#define TWO_CM		1134
#define HFDIST_CM	142

//========================================================================

/*N*/ ScStyleSheet::ScStyleSheet( const String&		rName,
/*N*/ 							ScStyleSheetPool&	rPool,
/*N*/ 							SfxStyleFamily		eFamily,
/*N*/ 							USHORT				nMask )
/*N*/ 
/*N*/ 	:	SfxStyleSheet	( rName, rPool, eFamily, nMask )
/*N*/     , eUsage( UNKNOWN )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScStyleSheet::ScStyleSheet( const ScStyleSheet& rStyle )
/*N*/     : SfxStyleSheet	( rStyle )
/*N*/     , eUsage( UNKNOWN )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ __EXPORT ScStyleSheet::~ScStyleSheet()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ BOOL __EXPORT ScStyleSheet::SetParent( const String& rParentName )
/*N*/ {
/*N*/ 	BOOL bResult = FALSE;
/*N*/ 	String aEffName = rParentName;
/*N*/ 	SfxStyleSheetBase* pStyle = rPool.Find( aEffName, nFamily );
/*N*/ 	if (!pStyle)
/*N*/ 	{
/*N*/ 		SfxStyleSheetIterator* pIter = rPool.CreateIterator( nFamily, SFXSTYLEBIT_ALL );
/*N*/ 		pStyle = pIter->First();
/*N*/ 		if (pStyle)
/*N*/ 			aEffName = pStyle->GetName();
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pStyle && aEffName != GetName() )
/*N*/ 	{
/*N*/ 		bResult = SfxStyleSheet::SetParent( aEffName );
/*N*/ 		if (bResult)
/*N*/ 		{
/*N*/ 			SfxItemSet& rParentSet = pStyle->GetItemSet();
/*N*/ 			GetItemSet().SetParent( &rParentSet );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bResult;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SfxItemSet& __EXPORT ScStyleSheet::GetItemSet()
/*N*/ {
/*N*/ 	if ( !pSet )
/*N*/ 	{
/*N*/ 		switch ( GetFamily() )
/*N*/ 		{
/*N*/ 			case SFX_STYLE_FAMILY_PAGE:
/*N*/ 				{
/*N*/ 					// Seitenvorlagen sollen nicht ableitbar sein,
/*N*/ 					// deshalb werden an dieser Stelle geeignete
/*N*/ 					// Werte eingestellt. (==Standard-Seitenvorlage)
/*N*/ 
/*N*/ 					SfxItemPool& rPool = GetPool().GetPool();
/*N*/ 					pSet = new SfxItemSet( rPool,
/*N*/ 										   ATTR_BACKGROUND, ATTR_BACKGROUND,
/*N*/ 										   ATTR_BORDER, ATTR_SHADOW,
/*N*/ 										   ATTR_LRSPACE, ATTR_PAGE_NULLVALS,
/*M*/ 										   ATTR_WRITINGDIR, ATTR_WRITINGDIR,
/*N*/ 										   ATTR_USERDEF, ATTR_USERDEF,
/*N*/ 										   0 );
/*N*/ 
/*N*/ 					//	Wenn gerade geladen wird, wird auch der Set hinterher aus der Datei
/*N*/ 					//	gefuellt, es brauchen also keine Defaults gesetzt zu werden.
/*N*/ 					//	GetPrinter wuerde dann auch einen neuen Printer anlegen, weil der
/*N*/ 					//	gespeicherte Printer noch nicht geladen ist!
/*N*/ 
/*N*/ 					ScDocument* pDoc = ((ScStyleSheetPool&)GetPool()).GetDocument();
/*N*/ 					if ( pDoc && pDoc->IsLoadingDone() )
/*N*/ 					{
/*N*/ 						// Setzen von sinnvollen Default-Werten:
/*N*/ 						//!!! const-Document wegcasten (im Ctor mal bei Gelegenheit aendern)
/*N*/ 						SfxPrinter*		pPrinter = pDoc->GetPrinter();
/*N*/ 						USHORT			nBinCount = pPrinter->GetPaperBinCount();
/*N*/ 						SvxPageItem		aPageItem( ATTR_PAGE );
/*N*/ 						// #50536# PaperBin auf Default lassen,
/*N*/ 						// nicht auf aktuelle Drucker-Einstellung umsetzen
/*N*/ 						SvxSizeItem		aPaperSizeItem(
/*N*/ 											ATTR_PAGE_SIZE,
/*N*/ 											SvxPaperInfo::GetPaperSize(pPrinter) );
/*N*/ 
/*N*/ 						SvxSetItem		aHFSetItem(
/*N*/ 											(const SvxSetItem&)
/*N*/ 											rPool.GetDefaultItem(ATTR_PAGE_HEADERSET) );
/*N*/ 
/*N*/ 						SfxItemSet&		rHFSet = aHFSetItem.GetItemSet();
/*N*/ 						SvxSizeItem		aHFSizeItem( // 0,5 cm + Abstand
/*N*/ 											ATTR_PAGE_SIZE,
/*N*/ 											Size( 0, (long)( 500 / HMM_PER_TWIPS ) + HFDIST_CM ) );
/*N*/ 
/*N*/ 						SvxULSpaceItem	aHFDistItem	( HFDIST_CM,// nUp
/*N*/ 													  HFDIST_CM,// nLow
/*N*/ 													  ATTR_ULSPACE );
/*N*/ 
/*N*/ 						SvxLRSpaceItem	aLRSpaceItem( TWO_CM,	// nLeft
/*N*/ 													  TWO_CM,	// nRight
/*N*/ 													  TWO_CM,	// nTLeft
/*N*/ 													  0,		// nFirstLineOffset
/*N*/ 													  ATTR_LRSPACE );
/*N*/ 						SvxULSpaceItem	aULSpaceItem( TWO_CM,	// nUp
/*N*/ 													  TWO_CM,	// nLow
/*N*/ 													  ATTR_ULSPACE );
/*N*/ 						SvxBoxInfoItem	aBoxInfoItem( ATTR_BORDER_INNER );
/*N*/ 
/*N*/ 						aBoxInfoItem.SetTable( FALSE );
/*N*/ 						aBoxInfoItem.SetDist( TRUE );
/*N*/ 						aBoxInfoItem.SetValid( VALID_DISTANCE, TRUE );
/*N*/ 
/*N*/ 						aPageItem.SetLandscape( ORIENTATION_LANDSCAPE
/*N*/ 												== pPrinter->GetOrientation() );
/*N*/ 
/*N*/ 						rHFSet.Put( aBoxInfoItem );
/*N*/ 						rHFSet.Put( aHFSizeItem );
/*N*/ 						rHFSet.Put( aHFDistItem );
/*N*/ 						rHFSet.Put( SvxLRSpaceItem( 0,0,0,0, ATTR_LRSPACE ) ); // Rand auf Null setzen
/*N*/ 
/*N*/ 						pSet->Put( aHFSetItem, ATTR_PAGE_HEADERSET );
/*N*/ 						pSet->Put( aHFSetItem, ATTR_PAGE_FOOTERSET );
/*N*/ 						pSet->Put( aBoxInfoItem ); // PoolDefault wg. Formatvorlagen
/*N*/ 												   // nicht ueberschreiben!
/*N*/ 
/*M*/ 						//	Writing direction: not as pool default because the default for cells
/*M*/ 						//	must remain FRMDIR_ENVIRONMENT, and each page style's setting is
/*M*/ 						//	supposed to be saved in the file format.
/*M*/ 						//	The page default may be read from a configuration item later.
/*M*/ 						SvxFrameDirection eDirection = FRMDIR_HORI_LEFT_TOP;
/*M*/ 						pSet->Put( SvxFrameDirectionItem( eDirection ), ATTR_WRITINGDIR );
/*M*/ 
/*N*/ 						rPool.SetPoolDefaultItem( aPageItem );
/*N*/ 						rPool.SetPoolDefaultItem( aPaperSizeItem );
/*N*/ 						rPool.SetPoolDefaultItem( aLRSpaceItem );
/*N*/ 						rPool.SetPoolDefaultItem( aULSpaceItem );
/*N*/ 						rPool.SetPoolDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALE, 100 ) );
/*N*/ 						rPool.SetPoolDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 0 ) );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 
/*N*/ 			case SFX_STYLE_FAMILY_PARA:
/*N*/ 			default:
/*N*/ 				pSet = new SfxItemSet( GetPool().GetPool(),
/*N*/ 									   ATTR_PATTERN_START, ATTR_PATTERN_END,
/*N*/ 									   0 );
/*N*/ 				break;
/*N*/ 		}
/*N*/ 		bMySet = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return *pSet;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL __EXPORT ScStyleSheet::IsUsed() const
/*N*/ {
/*N*/ 	if ( GetFamily() == SFX_STYLE_FAMILY_PARA )
/*N*/ 	{
/*?*/         // Always query the document to let it decide if a rescan is necessary,
/*?*/         // and store the state.
/*?*/         ScDocument* pDoc = ((ScStyleSheetPool&)rPool).GetDocument();
/*?*/         if ( pDoc && pDoc->IsStyleSheetUsed( *this, TRUE ) )
/*?*/             eUsage = USED;
/*?*/         else
/*?*/             eUsage = NOTUSED;
/*?*/         return eUsage == USED;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return TRUE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void __EXPORT ScStyleSheet::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
/*N*/ 						   const SfxHint& rHint, const TypeId& rHintType )
/*N*/ {
/*N*/ 	if ( rHint.ISA(SfxSimpleHint) )
/*N*/ 		if ( ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
/*N*/ 			GetItemSet().SetParent( NULL );
/*N*/ }

//------------------------------------------------------------------------

//	#66123# schmutzige Tricks, um die Standard-Vorlage immer als "Standard" zu speichern,
//	obwohl der fuer den Benutzer sichtbare Name uebersetzt ist:

/*N*/ const String& ScStyleSheet::GetName() const
/*N*/ {
/*N*/ 	const String& rBase = SfxStyleSheet::GetName();
/*N*/ 	const String* pForceStdName = ((ScStyleSheetPool&)rPool).GetForceStdName();
/*N*/ 	if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
/*N*/ 		return *pForceStdName;
/*N*/ 	else
/*N*/ 		return rBase;
/*N*/ }

/*N*/ const String& ScStyleSheet::GetParent() const
/*N*/ {
/*N*/ 	const String& rBase = SfxStyleSheet::GetParent();
/*N*/ 	const String* pForceStdName = ((ScStyleSheetPool&)rPool).GetForceStdName();
/*N*/ 	if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
/*N*/ 		return *pForceStdName;
/*N*/ 	else
/*N*/ 		return rBase;
/*N*/ }

/*N*/ const String& ScStyleSheet::GetFollow() const
/*N*/ {
/*N*/ 	const String& rBase = SfxStyleSheet::GetFollow();
/*N*/ 	const String* pForceStdName = ((ScStyleSheetPool&)rPool).GetForceStdName();
/*N*/ 	if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
/*N*/ 		return *pForceStdName;
/*N*/ 	else
/*N*/ 		return rBase;
/*N*/ }

//	Verhindern, dass ein Style "Standard" angelegt wird, wenn das nicht der
//	Standard-Name ist, weil sonst beim Speichern zwei Styles denselben Namen haetten
//	(Beim Laden wird der Style direkt per Make mit dem Namen erzeugt, so dass diese
//	Abfrage dann nicht gilt)
//!	Wenn irgendwann aus dem Laden SetName aufgerufen wird, muss fuer das Laden ein
//!	Flag gesetzt und abgefragt werden.
//!	Die ganze Abfrage muss raus, wenn fuer eine neue Datei-Version die Namens-Umsetzung wegfaellt.

/*N*/ BOOL ScStyleSheet::SetName( const String& rNew )
/*N*/ {
/*N*/ 	String aFileStdName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_STANDARD));
/*N*/ 	if ( rNew == aFileStdName && aFileStdName != ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
/*N*/ 		return FALSE;
/*N*/ 	else
/*N*/ 		return SfxStyleSheet::SetName( rNew );
/*N*/ }



}
