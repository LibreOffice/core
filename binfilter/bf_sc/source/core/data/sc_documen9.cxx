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

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <bf_svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_svx/fontitem.hxx>
#include <bf_svx/forbiddencharacterstable.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/svditer.hxx>
#include <bf_svx/svdocapt.hxx>
#include <bf_svx/svdoole2.hxx>
#include <bf_svx/svdouno.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_svx/xtable.hxx>
#include <bf_sfx2/objsh.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svtools/saveopt.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_sch/schdll.hxx>
#include <bf_sch/schdll0.hxx>

#include "document.hxx"
#include "docoptio.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "poolhelp.hxx"
#include "docpool.hxx"
#include "detfunc.hxx"		// for UpdateAllComments
#include "editutil.hxx"
namespace binfilter {


// -----------------------------------------------------------------------

/*N*/ XColorTable* ScDocument::GetColorTable()
/*N*/ {
/*N*/ 	if (pDrawLayer)
/*N*/ 		return pDrawLayer->GetColorTable();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (!pColorTable)
/*N*/ 		{
/*N*/ 			SvtPathOptions aPathOpt;
/*N*/ 			pColorTable = new XColorTable( aPathOpt.GetPalettePath() );
/*N*/ 		}
/*N*/
/*N*/ 		return pColorTable;
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::TransferDrawPage(ScDocument* pSrcDoc, USHORT nSrcPos, USHORT nDestPos)
/*N*/ {
/*N*/ 	if (pDrawLayer && pSrcDoc->pDrawLayer)
/*N*/ 	{
/*?*/ 		SdrPage* pOldPage = pSrcDoc->pDrawLayer->GetPage(nSrcPos);
/*?*/ 		SdrPage* pNewPage = pDrawLayer->GetPage(nDestPos);
/*?*/
/*?*/ 		if (pOldPage && pNewPage)
/*?*/ 		{
/*?*/ 			SdrObjListIter aIter( *pOldPage, IM_FLAT );
/*?*/ 			SdrObject* pOldObject = aIter.Next();
/*?*/ 			while (pOldObject)
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScDocument::InitDrawLayer( SfxObjectShell* pDocShell )
/*N*/ {
/*N*/ 	if (pDocShell && !pShell)
/*N*/ 		pShell = pDocShell;
/*N*/
/*N*/ //	DBG_ASSERT(pShell,"InitDrawLayer ohne Shell");
/*N*/
/*N*/ 	if (!pDrawLayer)
/*N*/ 	{
/*N*/ 		String aName;
/*N*/ 		if ( pShell && !pShell->IsLoading() )		// #88438# don't call GetTitle while loading
/*N*/ 			aName = pShell->GetTitle();
/*N*/ 		pDrawLayer = new ScDrawLayer( this, aName );
/*N*/ 		if (pLinkManager)
/*N*/ 			pDrawLayer->SetLinkManager( pLinkManager );
/*N*/
/*N*/ 		//	Drawing pages are accessed by table number, so they must also be present
/*N*/ 		//	for preceding table numbers, even if the tables aren't allocated
/*N*/ 		//	(important for clipboard documents).
/*N*/
/*N*/ 		USHORT nDrawPages = 0;
/*N*/ 		USHORT nTab;
/*N*/ 		for (nTab=0; nTab<=MAXTAB; nTab++)
/*N*/ 			if (pTab[nTab])
/*N*/ 				nDrawPages = nTab + 1;			// needed number of pages
/*N*/
/*N*/ 		for (nTab=0; nTab<nDrawPages; nTab++)
/*N*/ 		{
/*N*/ 			pDrawLayer->ScAddPage( nTab );		// always add page, with or without the table
/*N*/ 			if (pTab[nTab])
/*N*/ 			{
/*N*/ 				String aName;
/*N*/ 				pTab[nTab]->GetName(aName);
/*N*/ 				pDrawLayer->ScRenamePage( nTab, aName );
/*N*/
/*N*/ 				pTab[nTab]->SetDrawPageSize();	// #54782# sofort die richtige Groesse
/*N*/ #if 0
/*N*/ 				ULONG nx = (ULONG) ((double) (MAXCOL+1) * STD_COL_WIDTH			  * HMM_PER_TWIPS );
/*N*/ 				ULONG ny = (ULONG) ((double) (MAXROW+1) * ScGlobal::nStdRowHeight * HMM_PER_TWIPS );
/*N*/ 				pDrawLayer->SetPageSize( nTab, Size( nx, ny ) );
/*N*/ #endif
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		pDrawLayer->SetDefaultTabulator( GetDocOptions().GetTabDistance() );
/*N*/
/*N*/ 		UpdateDrawPrinter();
/*N*/ 		UpdateDrawLanguages();
/*N*/ 		if (bImportingXML)
/*?*/ 			pDrawLayer->EnableAdjust(FALSE);
/*N*/
/*N*/ 		pDrawLayer->SetForbiddenCharsTable( xForbiddenCharacters );
/*N*/ 		pDrawLayer->SetCharCompressType( GetAsianCompression() );
/*N*/ 		pDrawLayer->SetKernAsianPunctuation( GetAsianKerning() );
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::UpdateDrawLanguages()
/*N*/ {
/*N*/ 	if (pDrawLayer)
/*N*/ 	{
/*N*/ 		SfxItemPool& rDrawPool = pDrawLayer->GetItemPool();
/*N*/ 		rDrawPool.SetPoolDefaultItem( SvxLanguageItem( eLanguage, EE_CHAR_LANGUAGE ) );
/*N*/ 		rDrawPool.SetPoolDefaultItem( SvxLanguageItem( eCjkLanguage, EE_CHAR_LANGUAGE_CJK ) );
/*N*/ 		rDrawPool.SetPoolDefaultItem( SvxLanguageItem( eCtlLanguage, EE_CHAR_LANGUAGE_CTL ) );
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::UpdateDrawPrinter()
/*N*/ {
/*N*/ 	if (pDrawLayer)
/*N*/ 	{
/*N*/ 		// use the printer even if IsValid is false
/*N*/ 		// Application::GetDefaultDevice causes trouble with changing MapModes
/*N*/
/*N*/ 		OutputDevice* pRefDev = GetPrinter();
/*N*/ 		pRefDev->SetMapMode( MAP_100TH_MM );
/*N*/ 		pDrawLayer->SetRefDevice(pRefDev);
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScDocument::IsChart( SdrObject* pObject )
/*N*/ {
/*N*/ 	if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
/*N*/ 	{
/*N*/ 		SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
/*N*/ 		if (aIPObj.Is())
/*N*/ 		{
/*N*/ 			SvGlobalName aObjClsId = *aIPObj->GetSvFactory();
/*N*/ 			if (SchModuleDummy::HasID( aObjClsId ))
/*N*/ 				return TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ IMPL_LINK_INLINE_START( ScDocument, GetUserDefinedColor, USHORT *, pColorIndex )
/*N*/ {
/*N*/ 	return (long) &((GetColorTable()->Get(*pColorIndex))->GetColor());
/*N*/ }
/*N*/ IMPL_LINK_INLINE_END( ScDocument, GetUserDefinedColor, USHORT *, pColorIndex )

/*N*/ void ScDocument::DeleteDrawLayer()
/*N*/ {
/*N*/ 	delete pDrawLayer;
/*N*/ }

/*N*/ void ScDocument::DeleteColorTable()
/*N*/ {
/*N*/ 	delete pColorTable;
/*N*/ }

/*N*/ void ScDocument::LoadDrawLayer(SvStream& rStream)
/*N*/ {
/*N*/ 	InitDrawLayer();						// anlegen
/*N*/ 	pDrawLayer->Load(rStream);
/*N*/
/*N*/ 	//	nMaxTableNumber ist noch nicht initialisiert
/*N*/
/*N*/ 	USHORT nTableCount = 0;
/*N*/ 	while ( nTableCount <= MAXTAB && pTab[nTableCount] )
/*N*/ 		++nTableCount;
/*N*/
/*N*/ 	USHORT nPageCount = pDrawLayer->GetPageCount();
/*N*/ 	if ( nPageCount > nTableCount && nTableCount != 0 )
/*N*/ 	{
/*?*/ 		//	Manchmal sind beim Kopieren/Verschieben/Undo von Tabellen zuviele
/*?*/ 		//	(leere) Pages in der Tabelle stehengeblieben. Weg damit!
/*?*/
/*?*/ 		DBG_ERROR("zuviele Draw-Pages in der Datei");
/*?*/
/*?*/ 		for (USHORT i=nTableCount; i<nPageCount; i++)
/*?*/ 			pDrawLayer->DeletePage(nTableCount);
/*N*/ 	}
/*N*/
/*N*/ 	//	Controls auf richtigen Layer setzen
/*N*/ 	//	(zumindest in Dateien aus der 502 koennen sie falsch sein,
/*N*/ 	//	 wegen des fehlenden Layers in alten Dateien)
/*N*/
/*N*/ 	nPageCount = pDrawLayer->GetPageCount();
/*N*/ 	for (USHORT i=0; i<nPageCount; i++)
/*N*/ 	{
/*N*/ 		SdrPage* pPage = pDrawLayer->GetPage(i);
/*N*/ 		SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
/*N*/ 		SdrObject* pObject = aIter.Next();
/*N*/ 		while (pObject)
/*N*/ 		{
/*N*/ 			if ( pObject->ISA(SdrUnoObj) && pObject->GetLayer() != SC_LAYER_CONTROLS )
/*N*/ 			{
/*?*/ 				pObject->NbcSetLayer(SC_LAYER_CONTROLS);
/*?*/ 				DBG_ERROR("Control war auf falschem Layer");
/*N*/ 			}
/*N*/ 			pObject = aIter.Next();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::StoreDrawLayer(SvStream& rStream) const
/*N*/ {
/*N*/ 	if (pDrawLayer)
/*N*/ 	{
/*N*/ 		//	SetSavePortable wird mit VCL nicht mehr gebraucht
/*N*/ 		//BOOL bIndep = SFX_APP()->GetOptions().IsIndepGrfFmt();
/*N*/ 		//pDrawLayer->SetSavePortable( bIndep );
/*N*/
/*N*/       pDrawLayer->SetSaveCompressed( FALSE );
/*N*/       pDrawLayer->SetSaveNative( FALSE );
/*N*/
/*N*/ 		pDrawLayer->GetItemPool().SetFileFormatVersion( (USHORT)rStream.GetVersion() );
/*N*/ 		pDrawLayer->Store(rStream);
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScDocument::DrawGetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const
/*N*/ {
/*N*/ 	return pDrawLayer->GetPrintArea( rRange, bSetHor, bSetVer );
/*N*/ }










/*N*/ BOOL ScDocument::HasNoteObject( USHORT nCol, USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	if (!pDrawLayer)
/*N*/ 		return FALSE;
/*N*/ 	SdrPage* pPage = pDrawLayer->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 	if (!pPage)
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	BOOL bFound = FALSE;
/*N*/
/*N*/ 	SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 	SdrObject* pObject = aIter.Next();
/*N*/ 	while (pObject && !bFound)
/*N*/ 	{
/*N*/ 		if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA( SdrCaptionObj ) )
/*N*/ 		{
/*N*/ 			ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
/*N*/ 			if ( pData && nCol == pData->aStt.nCol && nRow == pData->aStt.nRow )
/*N*/ 				bFound = TRUE;
/*N*/ 		}
/*N*/ 		pObject = aIter.Next();
/*N*/ 	}
/*N*/
/*N*/ 	return bFound;
/*N*/ }

/*N*/ void ScDocument::RefreshNoteFlags()
/*N*/ {
/*N*/ 	if (!pDrawLayer)
/*N*/ 		return;
/*N*/
/*N*/ 	BOOL bAnyIntObj = FALSE;
/*N*/ 	USHORT nTab;
/*N*/ 	ScPostIt aNote;
/*N*/ 	for (nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
/*N*/ 	{
/*N*/ 		SdrPage* pPage = pDrawLayer->GetPage(nTab);
/*N*/ 		DBG_ASSERT(pPage,"Page ?");
/*N*/ 		if (pPage)
/*N*/ 		{
/*N*/ 			SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 			SdrObject* pObject = aIter.Next();
/*N*/ 			while (pObject)
/*N*/ 			{
/*N*/ 				if ( pObject->GetLayer() == SC_LAYER_INTERN )
/*N*/ 				{
/*?*/ 					bAnyIntObj = TRUE;	// for all internal objects, including detective
/*?*/
/*?*/ 					if ( pObject->ISA( SdrCaptionObj ) )
/*?*/ 					{
/*?*/ 						ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
/*?*/ 						if ( pData )
/*?*/ 						{
/*?*/ 							if ( GetNote( pData->aStt.nCol, pData->aStt.nRow, nTab, aNote ) )
/*?*/ 								if ( !aNote.IsShown() )
/*?*/ 								{
/*?*/ 									aNote.SetShown(TRUE);
/*?*/ 									SetNote( pData->aStt.nCol, pData->aStt.nRow, nTab, aNote );
/*?*/ 								}
/*?*/ 						}
/*?*/ 					}
/*N*/ 				}
/*N*/ 				pObject = aIter.Next();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if (bAnyIntObj)
/*N*/ 	{
/*?*/ 		//	update attributes for all note objects and the colors of detective objects
/*?*/ 		//	(we don't know with which settings the file was created)
/*?*/
/*N*/ 	ScDetectiveFunc aFunc( this, 0 ); // detective.sdc
/*N*/ /*?*/ 		aFunc.UpdateAllComments();
/*N*/ /*?*/ 		aFunc.UpdateAllArrowColors();
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScDocument::IsPrintEmpty( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
/*N*/ 								USHORT nEndCol, USHORT nEndRow, BOOL bLeftIsEmpty,
/*N*/ 								ScRange* pLastRange, Rectangle* pLastMM ) const
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	if (!IsBlockEmpty( nTab, nStartCol, nStartRow, nEndCol, nEndRow ))
/*N*/  	return TRUE;
/*N*/ }

/*N*/ void ScDocument::Clear()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 		if (pTab[i])
/*N*/ 		{
/*N*/ 			delete pTab[i];
/*N*/ 			pTab[i]=NULL;
/*N*/ 		}
/*N*/ 	delete pSelectionAttr;
/*N*/ 	pSelectionAttr = NULL;
/*N*/
/*N*/ 	if (pDrawLayer)
/*N*/ 		pDrawLayer->Clear();
/*N*/ }

/*N*/ BOOL ScDocument::HasDetectiveObjects(USHORT nTab) const
/*N*/ {
/*N*/ 	//	looks for detective objects, annotations don't count
/*N*/ 	//	(used to adjust scale so detective objects hit their cells better)
/*N*/
/*N*/ 	BOOL bFound = FALSE;
/*N*/
/*N*/ 	if (pDrawLayer)
/*N*/ 	{
/*N*/ 		SdrPage* pPage = pDrawLayer->GetPage(nTab);
/*N*/ 		DBG_ASSERT(pPage,"Page ?");
/*N*/ 		if (pPage)
/*N*/ 		{
/*N*/ 			SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
/*N*/ 			SdrObject* pObject = aIter.Next();
/*N*/ 			while (pObject && !bFound)
/*N*/ 			{
/*N*/ 				// anything on the internal layer except captions (annotations)
/*N*/ 				if ( pObject->GetLayer() == SC_LAYER_INTERN && !pObject->ISA( SdrCaptionObj ) )
/*N*/ 					bFound = TRUE;
/*N*/
/*N*/ 				pObject = aIter.Next();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return bFound;
/*N*/ }

/*N*/ void ScDocument::UpdateFontCharSet()
/*N*/ {
/*N*/ 	//	In alten Versionen (bis incl. 4.0 ohne SP) wurden beim Austausch zwischen
/*N*/ 	//	Systemen die CharSets in den Font-Attributen nicht angepasst.
/*N*/ 	//	Das muss fuer Dokumente bis incl SP2 nun nachgeholt werden:
/*N*/ 	//	Alles, was nicht SYMBOL ist, wird auf den System-CharSet umgesetzt.
/*N*/ 	//	Bei neuen Dokumenten (Version SC_FONTCHARSET) sollte der CharSet stimmen.
/*N*/
/*N*/ 	BOOL bUpdateOld = ( nSrcVer < SC_FONTCHARSET );
/*N*/
/*N*/ 	CharSet eSysSet = gsl_getSystemTextEncoding();
/*N*/ 	if ( eSrcSet != eSysSet || bUpdateOld )
/*N*/ 	{
/*N*/ 		USHORT nCount,i;
/*N*/ 		SvxFontItem* pItem;
/*N*/
/*N*/ 		ScDocumentPool* pPool = xPoolHelper->GetDocPool();
/*N*/ 		nCount = pPool->GetItemCount(ATTR_FONT);
/*N*/ 		for (i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			pItem = (SvxFontItem*)pPool->GetItem(ATTR_FONT, i);
/*N*/ 			if ( pItem && ( pItem->GetCharSet() == eSrcSet ||
/*N*/ 							( bUpdateOld && pItem->GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
/*N*/ 				pItem->GetCharSet() = eSysSet;
/*N*/ 		}
/*N*/
/*N*/ 		if ( pDrawLayer )
/*N*/ 		{
/*N*/ 			SfxItemPool& rDrawPool = pDrawLayer->GetItemPool();
/*N*/ 			nCount = rDrawPool.GetItemCount(EE_CHAR_FONTINFO);
/*N*/ 			for (i=0; i<nCount; i++)
/*N*/ 			{
/*N*/ 				pItem = (SvxFontItem*)rDrawPool.GetItem(EE_CHAR_FONTINFO, i);
/*N*/ 				if ( pItem && ( pItem->GetCharSet() == eSrcSet ||
/*N*/ 								( bUpdateOld && pItem->GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
/*?*/ 					pItem->GetCharSet() = eSysSet;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::SetImportingXML( BOOL bVal )
/*N*/ {
/*N*/ 	bImportingXML = bVal;
/*N*/ 	if (pDrawLayer)
/*N*/ 		pDrawLayer->EnableAdjust(!bImportingXML);
/*N*/ }

/*N*/ rtl::Reference<SvxForbiddenCharactersTable> ScDocument::GetForbiddenCharacters()
/*N*/ {
/*N*/ 	return xForbiddenCharacters;
/*N*/ }

/*N*/ void ScDocument::SetForbiddenCharacters( const rtl::Reference<SvxForbiddenCharactersTable> xNew )
/*N*/ {
/*N*/ 	xForbiddenCharacters = xNew;
/*N*/ 	if ( pEditEngine )
/*?*/ 		pEditEngine->SetForbiddenCharsTable( xForbiddenCharacters );
/*N*/ 	if ( pDrawLayer )
/*N*/ 		pDrawLayer->SetForbiddenCharsTable( xForbiddenCharacters );
/*N*/ }

/*N*/ BOOL ScDocument::IsValidAsianCompression() const
/*N*/ {
/*N*/ 	return ( nAsianCompression != SC_ASIANCOMPRESSION_INVALID );
/*N*/ }

/*N*/ BYTE ScDocument::GetAsianCompression() const
/*N*/ {
/*N*/ 	if ( nAsianCompression == SC_ASIANCOMPRESSION_INVALID )
/*N*/ 		return 0;
/*N*/ 	else
/*N*/ 		return nAsianCompression;
/*N*/ }

/*N*/ void ScDocument::SetAsianCompression(BYTE nNew)
/*N*/ {
/*N*/ 	nAsianCompression = nNew;
/*N*/ 	if ( pEditEngine )
/*?*/ 		pEditEngine->SetAsianCompressionMode( nAsianCompression );
/*N*/ 	if ( pDrawLayer )
/*N*/ 		pDrawLayer->SetCharCompressType( nAsianCompression );
/*N*/ }

/*N*/ BOOL ScDocument::IsValidAsianKerning() const
/*N*/ {
/*N*/ 	return ( nAsianKerning != SC_ASIANKERNING_INVALID );
/*N*/ }

/*N*/ BOOL ScDocument::GetAsianKerning() const
/*N*/ {
/*N*/ 	if ( nAsianKerning == SC_ASIANKERNING_INVALID )
/*N*/ 		return FALSE;
/*N*/ 	else
/*N*/ 		return (BOOL)nAsianKerning;
/*N*/ }

/*N*/ void ScDocument::SetAsianKerning(BOOL bNew)
/*N*/ {
/*N*/ 	nAsianKerning = (BYTE)bNew;
/*N*/ 	if ( pEditEngine )
/*?*/ 		pEditEngine->SetKernAsianPunctuation( (BOOL)nAsianKerning );
/*N*/ 	if ( pDrawLayer )
/*N*/ 		pDrawLayer->SetKernAsianPunctuation( (BOOL)nAsianKerning );
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
