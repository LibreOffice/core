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

#include <bf_svx/svdpage.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif


#include "scitems.hxx"
#include <bf_svtools/ctrltool.hxx>
#include <bf_svx/flstitem.hxx>
#include <bf_svx/drawitem.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svx/svditer.hxx>
#include <bf_svx/svdoole2.hxx>
#include <bf_offmgr/app.hxx>
#include <bf_svx/asiancfg.hxx>
#include <bf_svx/forbiddencharacterstable.hxx>
#include <rtl/logfile.hxx>



// INCLUDE ---------------------------------------------------------------
/*
*/
#include "drwlayer.hxx"
#include "stlpool.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "bf_sc.hrc"
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------

/*N*/ BOOL __EXPORT ScDocShell::InitNew( SvStorage * pStor )
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::InitNew" );
/*N*/ 
/*N*/ 	BOOL bRet = SfxInPlaceObject::InitNew( pStor );
/*N*/ 
/*N*/ 	aDocument.MakeTable(0);
/*N*/ 	//	zusaetzliche Tabellen werden von der ersten View angelegt,
/*N*/ 	//	wenn bIsEmpty dann noch TRUE ist
/*N*/ 
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		Size aSize( (long) ( STD_COL_WIDTH			 * HMM_PER_TWIPS * OLE_STD_CELLS_X ),
/*N*/ 					(long) ( ScGlobal::nStdRowHeight * HMM_PER_TWIPS * OLE_STD_CELLS_Y ) );
/*N*/ 		// hier muss auch der Start angepasst werden
/*N*/ 		SetVisAreaOrSize( Rectangle( Point(), aSize ), TRUE );
/*N*/ 	}
/*N*/ 
/*N*/ 	// InitOptions sets the document languages, must be called before CreateStandardStyles
/*N*/ 	InitOptions();
/*N*/ 
/*N*/ 	aDocument.GetStyleSheetPool()->CreateStandardStyles();
/*N*/ 	aDocument.UpdStlShtPtrsFrmNms();
/*N*/ 
/*N*/ 	//	SetDocumentModified ist in Load/InitNew nicht mehr erlaubt!
/*N*/ 
/*N*/ 	InitItems();
/*N*/ 	CalcOutputFactor();
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

//------------------------------------------------------------------

/*N*/ void ScDocShell::InitItems()
/*N*/ {
/*N*/ 	// AllItemSet fuer Controller mit benoetigten Items fuellen:
/*N*/ 
/*N*/ 	if ( pFontList )
/*N*/ 		delete pFontList;
/*N*/ 
/*N*/ 	//	Druck-Optionen werden beim Drucken und evtl. in GetPrinter gesetzt
/*N*/ 
/*N*/ 	pFontList = new FontList( GetPrinter(), Application::GetDefaultDevice() );
/*N*/ 	PutItem( SvxFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST ) );
/*N*/ 
/*N*/ 	ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
/*N*/ 	if (pDrawLayer)
/*N*/ 	{
/*N*/ 		PutItem( SvxColorTableItem	( pDrawLayer->GetColorTable() ) );
/*N*/ 		PutItem( SvxGradientListItem( pDrawLayer->GetGradientList() ) );
/*N*/ 		PutItem( SvxHatchListItem	( pDrawLayer->GetHatchList() ) );
/*N*/ 		PutItem( SvxBitmapListItem	( pDrawLayer->GetBitmapList() ) );
/*N*/ 		PutItem( SvxDashListItem	( pDrawLayer->GetDashList() ) );
/*N*/ 		PutItem( SvxLineEndListItem ( pDrawLayer->GetLineEndList() ) );
/*N*/ 
/*N*/ 			//	andere Anpassungen nach dem Anlegen des DrawLayers
/*N*/ 
/*N*/ 		//if (SfxObjectShell::HasSbxObject())
/*N*/ 		pDrawLayer->UpdateBasic();			// DocShell-Basic in DrawPages setzen
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//	always use global color table instead of local copy
/*N*/ 
/*N*/ 		PutItem( SvxColorTableItem( OFF_APP()->GetStdColorTable() ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !aDocument.GetForbiddenCharacters().is() ||
/*N*/ 			!aDocument.IsValidAsianCompression() || !aDocument.IsValidAsianKerning() )
/*N*/ 	{
/*N*/ 		//	get settings from SvxAsianConfig
/*N*/ 		SvxAsianConfig aAsian( sal_False );
/*N*/ 
/*N*/ 		if ( !aDocument.GetForbiddenCharacters().is() )
/*N*/ 		{
/*N*/ 			// set forbidden characters if necessary
/*N*/ 			uno::Sequence<lang::Locale> aLocales = aAsian.GetStartEndCharLocales();
/*N*/ 			if (aLocales.getLength())
/*N*/ 			{
/*?*/ 				rtl::Reference<SvxForbiddenCharactersTable> xForbiddenTable =
/*?*/ 						new SvxForbiddenCharactersTable( aDocument.GetServiceManager() );
/*?*/ 
/*?*/ 				const lang::Locale* pLocales = aLocales.getConstArray();
/*?*/ 				for (sal_Int32 i = 0; i < aLocales.getLength(); i++)
/*?*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 i18n::ForbiddenCharacters aForbidden;
/*?*/ 				}
/*?*/ 
/*?*/ 				aDocument.SetForbiddenCharacters( xForbiddenTable );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( !aDocument.IsValidAsianCompression() )
/*N*/ 		{
/*N*/ 			// set compression mode from configuration if not already set (e.g. XML import)
/*N*/ 			aDocument.SetAsianCompression( aAsian.GetCharDistanceCompression() );
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( !aDocument.IsValidAsianKerning() )
/*N*/ 		{
/*N*/ 			// set asian punctuation kerning from configuration if not already set (e.g. XML import)
/*N*/ 			aDocument.SetAsianKerning( !aAsian.IsKerningWesternTextOnly() );	// reversed
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------

/*N*/ void ScDocShell::ResetDrawObjectShell()
/*N*/ {
/*N*/ 	ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
/*N*/ 	if (pDrawLayer)
/*N*/ 		pDrawLayer->SetObjectShell( NULL );
/*N*/ }

//------------------------------------------------------------------




//------------------------------------------------------------------


/*N*/ ScDrawLayer* ScDocShell::MakeDrawLayer()
/*N*/ {
/*N*/ 	ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
/*N*/ 	if (!pDrawLayer)
/*N*/ 	{
/*N*/ 		RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::MakeDrawLayer" );
/*N*/ 
/*N*/ 		aDocument.InitDrawLayer(this);
/*N*/ 		pDrawLayer = aDocument.GetDrawLayer();
/*N*/ 		InitItems();											// incl. Undo und Basic
/*N*/ 		Broadcast( SfxSimpleHint( SC_HINT_DRWLAYER_NEW ) );
/*N*/ 		if (nDocumentLock)
/*N*/ 			pDrawLayer->setLock(TRUE);
/*N*/ 	}
/*N*/ 	return pDrawLayer;
/*N*/ }

//------------------------------------------------------------------


/*N*/ void ScDocShell::RemoveUnknownObjects()
/*N*/ {
/*N*/ 	//	OLE-Objekte loeschen, wenn kein Drawing-Objekt dazu existiert
/*N*/ 	//	Loeschen wie in SvPersist::CleanUp
/*N*/ 
/*N*/ 	ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
/*N*/ 	const SvInfoObjectMemberList* pChildList = GetObjectList();
/*N*/ 
/*N*/ 	if( pChildList && pChildList->Count() )
/*N*/ 	{
/*N*/ 		for( ULONG i=0; i<pChildList->Count(); )
/*N*/ 		{
/*N*/ 			SvInfoObjectRef pEle = pChildList->GetObject(i);
/*N*/ 			String aObjName = pEle->GetObjName();
/*N*/ 			BOOL bFound = FALSE;
/*N*/ 			if ( pDrawLayer )
/*N*/ 			{
/*N*/ 				USHORT nTabCount = pDrawLayer->GetPageCount();
/*N*/ 				for (USHORT nTab=0; nTab<nTabCount && !bFound; nTab++)
/*N*/ 				{
/*N*/ 					SdrPage* pPage = pDrawLayer->GetPage(nTab);
/*N*/ 					DBG_ASSERT(pPage,"Page ?");
/*N*/ 					if (pPage)
/*N*/ 					{
/*N*/ 						SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
/*N*/ 						SdrObject* pObject = aIter.Next();
/*N*/ 						while (pObject && !bFound)
/*N*/ 						{
/*N*/ 							// name from InfoObject is PersistName
/*N*/ 							if ( pObject->ISA(SdrOle2Obj) &&
/*N*/ 									static_cast<SdrOle2Obj*>(pObject)->GetPersistName() == aObjName )
/*N*/ 								bFound = TRUE;
/*N*/ 							pObject = aIter.Next();
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if (!bFound)
/*N*/ 			{
/*?*/ 				DBG_ASSERT(pEle->GetRefCount()==2, "Loeschen von referenziertem Storage");
/*?*/ 				String aStorName(pEle->GetStorageName());
/*?*/ 				SvPersist::Remove(pEle);
/*?*/ 				GetStorage()->Remove(aStorName);
/*N*/ 			}
/*N*/ 			else
/*N*/ 				i++;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
