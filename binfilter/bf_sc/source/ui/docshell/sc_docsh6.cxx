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

// System - Includes -----------------------------------------------------

//#define _BASEDLGS_HXX ***
#define _BIGINT_HXX
#define _CACHESTR_HXX
//#define _CLIP_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
//#define _COLOR_HXX
//#define _CTRLBOX_HXX
//#define _CTRLTOOL_HXX
#define _DIALOGS_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
//#define _FIELD_HXX
#define _FILDLG_HXX
//#define _FILTER_HXX
#define _FONTDLG_HXX
#define _FRM3D_HXX
//#define _GRAPH_HXX
//#define _GDIMTF_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
//#define _MDIFRM_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
//#define _MAPMOD_HXX
#define _OUTLINER_HXX
//#define _PAL_HXX
#define _PASSWD_HXX
//#define _PRNDLG_HXX	//
#define _POLY_HXX
#define _PVRWIN_HXX
#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
//#define _SELENG_HXX
#define _SETBRW_HXX
//#define _SOUND_HXX
#define _STACK_HXX
//#define _STATUS_HXX ***
#define _STDMENU_HXX
//#define _STDCTRL_HXX
//#define _SYSDLG_HXX
//#define _TAB_HXX
#define _TABBAR_HXX
//#define _TREELIST_HXX
//#define _VALUESET_HXX
#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCATTR_HXX
#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX
//#define _VIEWFAC_HXX


#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
//#define _SFX_SAVEOPT_HXX
#define _SFX_TEMPLDLG_HXX
//#define _SFXAPP_HXX
#define _SFXBASIC_HXX
//#define _SFXCTRLITEM_HXX
#define _SFXDISPATCH_HXX
#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXMNUMGR_HXX
//#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
//#define _SFXOBJFACE_HXX
//#define _SFXREQUEST_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX
#define _SFX_TEMPLDLG_HXX

#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NODRW
#define _SI_NOCONTROL

#define _SV_NOXSOUND
#define _SVDATTR_HXX
#define _SVDETC_HXX
#define _SVDIO_HXX
#define _SVDRAG_HXX
#define _SVDLAYER_HXX
#define _SVDXOUT_HXX

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef PCH
#include <bf_so3/ipenv.hxx>
#include <bf_svx/linkmgr.hxx>
#endif

#include "docsh.hxx"

#include "stlpool.hxx"
#include "tablink.hxx"
#include "collect.hxx"

#include <bf_svtools/itemset.hxx>

namespace binfilter {

/*N*/ struct ScStylePair
/*N*/ {
/*N*/ 	SfxStyleSheetBase *pSource;
/*N*/ 	SfxStyleSheetBase *pDest;
/*N*/ };


// STATIC DATA -----------------------------------------------------------

//----------------------------------------------------------------------

//
//	Ole
//


/*N*/ void __EXPORT ScDocShell::SetVisArea( const Rectangle & rVisArea )
/*N*/ {
/*N*/ 	//	with the SnapVisArea call in SetVisAreaOrSize, it's safe to always
/*N*/ 	//	use both the size and position of the VisArea
/*N*/ 	SetVisAreaOrSize( rVisArea, TRUE );
/*N*/ }

/*N*/ void ScDocShell::SetVisAreaOrSize( const Rectangle& rVisArea, BOOL bModifyStart )
/*N*/ {
/*N*/ 	Rectangle aArea = rVisArea;
/*N*/ 	if (bModifyStart)
/*N*/ 	{
/*N*/ 		if ( aArea.Left() < 0 || aArea.Top() < 0 )
/*N*/ 		{
/*?*/ 			//	VisArea start position can't be negative.
/*?*/ 			//	Move the VisArea, otherwise only the upper left position would
/*?*/ 			//	be changed in SnapVisArea, and the size would be wrong.
/*?*/ 
/*?*/ 			Point aNewPos( Max( aArea.Left(), (long) 0 ),
/*?*/ 						   Max( aArea.Top(), (long) 0 ) );
/*?*/ 			aArea.SetPos( aNewPos );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*?*/ 		aArea.SetPos( SfxInPlaceObject::GetVisArea().TopLeft() );
/*N*/ 
/*N*/ 	//		hier Position anpassen!
/*N*/ 
/*N*/ 	//	#92248# when loading an ole object, the VisArea is set from the document's
/*N*/ 	//	view settings and must be used as-is (document content may not be complete yet).
/*N*/ 	if ( !aDocument.IsImportingXML() )
/*N*/ 		aDocument.SnapVisArea( aArea );
/*N*/ 
/*N*/ 	SvInPlaceObject::SetVisArea( aArea );
/*N*/ 
/*N*/ 	if (aDocument.IsEmbedded())
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScTripel aOldStart,aOldEnd;
/*N*/ 	}
/*N*/ }

//
//	Style-Krempel fuer Organizer etc.
//

/*N*/ SfxStyleSheetBasePool* __EXPORT ScDocShell::GetStyleSheetPool()
/*N*/ {
/*N*/ 	return (SfxStyleSheetBasePool*)aDocument.GetStyleSheetPool();
/*N*/ }


//	nach dem Laden von Vorlagen aus einem anderen Dokment (LoadStyles, Insert)
//	muessen die SetItems (ATTR_PAGE_HEADERSET, ATTR_PAGE_FOOTERSET) auf den richtigen
//	Pool umgesetzt werden, bevor der Quell-Pool geloescht wird.

/*N*/ void lcl_AdjustPool( SfxStyleSheetBasePool* pStylePool )
/*N*/ {
/*N*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pStylePool->SetSearchMask(SFX_STYLE_FAMILY_PAGE, 0xffff);
/*N*/ }


/*N*/ void ScDocShell::LoadStylesArgs( ScDocShell& rSource, BOOL bReplace, BOOL bCellStyles, BOOL bPageStyles )
/*N*/ {
/*N*/ 	//	similar to LoadStyles, but with selectable behavior for XStyleLoader::loadStylesFromURL call
/*N*/ 
/*N*/ 	if ( !bCellStyles && !bPageStyles )		// nothing to do
/*N*/ 		return;
/*N*/ 
/*N*/ 	ScStyleSheetPool* pSourcePool = rSource.GetDocument()->GetStyleSheetPool();
/*N*/ 	ScStyleSheetPool* pDestPool = aDocument.GetStyleSheetPool();
/*N*/ 
/*N*/ 	SfxStyleFamily eFamily = bCellStyles ?
/*N*/ 			( bPageStyles ? SFX_STYLE_FAMILY_ALL : SFX_STYLE_FAMILY_PARA ) :
/*N*/ 			SFX_STYLE_FAMILY_PAGE;
/*N*/ 	SfxStyleSheetIterator aIter( pSourcePool, eFamily );
/*N*/ 	USHORT nSourceCount = aIter.Count();
/*N*/ 	if ( nSourceCount == 0 )
/*N*/ 		return;								// no source styles
/*N*/ 
/*N*/ 	ScStylePair* pStyles = new ScStylePair[ nSourceCount ];
/*N*/ 	USHORT nFound = 0;
/*N*/ 
/*N*/ 	//	first create all new styles
/*N*/ 
/*N*/ 	SfxStyleSheetBase* pSourceStyle = aIter.First();
/*N*/ 	while (pSourceStyle)
/*N*/ 	{
/*N*/ 		String aName = pSourceStyle->GetName();
/*N*/ 		SfxStyleSheetBase* pDestStyle = pDestPool->Find( pSourceStyle->GetName(), pSourceStyle->GetFamily() );
/*N*/ 		if ( pDestStyle )
/*N*/ 		{
/*N*/ 			// touch existing styles only if replace flag is set
/*N*/ 			if ( bReplace )
/*N*/ 			{
/*N*/ 				pStyles[nFound].pSource = pSourceStyle;
/*N*/ 				pStyles[nFound].pDest = pDestStyle;
/*N*/ 				++nFound;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pStyles[nFound].pSource = pSourceStyle;
/*N*/ 			pStyles[nFound].pDest = &pDestPool->Make( aName, pSourceStyle->GetFamily(), pSourceStyle->GetMask() );
/*N*/ 			++nFound;
/*N*/ 		}
/*N*/ 
/*N*/ 		pSourceStyle = aIter.Next();
/*N*/ 	}
/*N*/ 
/*N*/ 	//	then copy contents (after inserting all styles, for parent etc.)
/*N*/ 
/*N*/ 	for ( USHORT i = 0; i < nFound; ++i )
/*N*/ 	{
/*N*/ 		pStyles[i].pDest->GetItemSet().PutExtended(
/*N*/ 			pStyles[i].pSource->GetItemSet(), SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT);
/*N*/ 		if(pStyles[i].pSource->HasParentSupport())
/*N*/ 			pStyles[i].pDest->SetParent(pStyles[i].pSource->GetParent());
/*N*/ 		// follow is never used
/*N*/ 	}
/*N*/ 
/*N*/ 	lcl_AdjustPool( GetStyleSheetPool() );		// adjust SetItems
/*N*/ 	UpdateAllRowHeights();
/*N*/ 	PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID | PAINT_LEFT );		// Paint
/*N*/ 
/*N*/ 	delete[] pStyles;
/*N*/ }



/*N*/ void ScDocShell::UpdateLinks()
/*N*/ {
/*N*/ 	SvxLinkManager* pLinkManager = aDocument.GetLinkManager();
/*N*/ 	USHORT nCount;
/*N*/ 	USHORT i;
/*N*/ 	StrCollection aNames;
/*N*/ 
/*N*/ 	// nicht mehr benutzte Links raus
/*N*/ 
/*N*/ 	nCount = pLinkManager->GetLinks().Count();
/*N*/ 	for (i=nCount; i>0; )
/*N*/ 	{
/*?*/ 		--i;
/*?*/ 		::binfilter::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
/*?*/ 		if (pBase->ISA(ScTableLink))
/*?*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScTableLink* pTabLink = (ScTableLink*)pBase;
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	// neue Links eintragen
/*N*/ 
/*N*/ 	nCount = aDocument.GetTableCount();
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 		if (aDocument.IsLinked(i))
/*N*/ 		{
/*N*/ 			String aDocName = aDocument.GetLinkDoc(i);
/*N*/ 			String aFltName = aDocument.GetLinkFlt(i);
/*N*/ 			String aOptions = aDocument.GetLinkOpt(i);
/*N*/ 			ULONG nRefresh	= aDocument.GetLinkRefreshDelay(i);
/*N*/ 			BOOL bThere = FALSE;
/*N*/ 			for (USHORT j=0; j<i && !bThere; j++)				// im Dokument mehrfach?
/*N*/ 				if (aDocument.IsLinked(j)
/*N*/ 						&& aDocument.GetLinkDoc(j) == aDocName
/*N*/ 						&& aDocument.GetLinkFlt(j) == aFltName
/*N*/ 						&& aDocument.GetLinkOpt(j) == aOptions)
/*N*/ 						// Ignore refresh delay in compare, it should be the
/*N*/ 						// same for identical links and we don't want dupes
/*N*/ 						// if it ain't.
/*N*/ 					bThere = TRUE;
/*N*/ 
/*N*/ 			if (!bThere)										// schon als Filter eingetragen?
/*N*/ 			{
/*N*/ 				StrData* pData = new StrData(aDocName);
/*N*/ 				if (!aNames.Insert(pData))
/*N*/ 				{
/*?*/ 					delete pData;
/*?*/ 					bThere = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if (!bThere)
/*N*/ 			{
/*N*/ 				ScTableLink* pLink = new ScTableLink( this, aDocName, aFltName, aOptions, nRefresh );
/*N*/ 				pLink->SetInCreate( TRUE );
/*N*/ 				pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName );
/*N*/ 				pLink->Update();
/*N*/ 				pLink->SetInCreate( FALSE );
/*N*/ 			}
/*N*/ 		}
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
