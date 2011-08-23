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

#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_sfx2/app.hxx>
#include <bf_svx/editobj.hxx>
#include <bf_svx/linkmgr.hxx>
#include <bf_svx/svdundo.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <bf_svtools/zforlist.hxx>
#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include <bf_svtools/PasswordHelper.hxx>
#endif

#include <list>

#include "docfunc.hxx"

#include "bf_sc.hrc"

#include "arealink.hxx"
#include "autoform.hxx"
#include "cell.hxx"
#include "detdata.hxx"
#include "detfunc.hxx"
#include "docpool.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "editutil.hxx"
#include "globstr.hrc"
#include "namecrea.hxx"		// NAME_TOP etc.
#include "olinetab.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "scresid.hxx"
#include "stlpool.hxx"
#include "tablink.hxx"
#include "uiitems.hxx"
#include "undocell.hxx"
#include "undodraw.hxx"
#include "undotab.hxx"
#include "sizedev.hxx"
#include "scmod.hxx"
#include "editable.hxx"
namespace binfilter {

#define TABLEID_DOC                0xFFFF

using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

//========================================================================


//	Zeile ueber dem Range painten (fuer Linien nach AdjustRowHeight)

/*N*/ void lcl_PaintAbove( ScDocShell& rDocShell, const ScRange& rRange )
/*N*/ {
/*N*/ 	USHORT nRow = rRange.aStart.Row();
/*N*/ 	if ( nRow > 0 )
/*N*/ 	{
/*N*/ 		USHORT nTab = rRange.aStart.Tab();	//! alle?
/*N*/ 		--nRow;
/*N*/ 		rDocShell.PostPaint( ScRange(0,nRow,nTab, MAXCOL,nRow,nTab), PAINT_GRID );
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::AdjustRowHeight( const ScRange& rRange, BOOL bPaint )
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	if ( pDoc->IsImportingXML() )
/*N*/ 	{
/*N*/ 		//	for XML import, all row heights are updated together after importing
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT nTab      = rRange.aStart.Tab();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nEndRow   = rRange.aEnd.Row();
/*N*/ 
/*N*/ 	ScSizeDeviceProvider aProv( &rDocShell );
/*N*/ 	Fraction aOne(1,1);
/*N*/ 
/*N*/ 	BOOL bChanged = pDoc->SetOptimalHeight( nStartRow, nEndRow, nTab, 0, aProv.GetDevice(),
/*N*/ 											aProv.GetPPTX(), aProv.GetPPTY(), aOne, aOne, FALSE );
/*N*/ 
/*N*/ 	if ( bPaint && bChanged )
/*N*/ 		rDocShell.PostPaint( 0, nStartRow, nTab, MAXCOL, MAXROW, nTab,
/*N*/ 											PAINT_GRID | PAINT_LEFT );
/*N*/ 
/*N*/ 	return bChanged;
/*N*/ }


//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::DetectiveAddPred(const ScAddress& rPos)
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	rDocShell.MakeDrawLayer();
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	USHORT nCol = rPos.Col();
/*N*/ 	USHORT nRow = rPos.Row();
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 
/*N*/ 	BOOL bDone = ScDetectiveFunc( pDoc,nTab ).ShowPred( nCol, nRow );
/*N*/ 	if (bDone)
/*N*/ 	{
/*N*/ 		ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_ADDPRED );
/*N*/ 		pDoc->AddDetectiveOperation( aOperation );
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bDone;
/*N*/ }

/*N*/ BOOL ScDocFunc::DetectiveDelPred(const ScAddress& rPos)
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	USHORT nCol = rPos.Col();
/*N*/ 	USHORT nRow = rPos.Row();
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 
/*N*/ 	BOOL bDone = ScDetectiveFunc( pDoc,nTab ).DeletePred( nCol, nRow );
/*N*/ 	if (bDone)
/*N*/ 	{
/*N*/ 		ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_DELPRED );
/*N*/ 		pDoc->AddDetectiveOperation( aOperation );
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bDone;
/*N*/ }

/*N*/ BOOL ScDocFunc::DetectiveAddSucc(const ScAddress& rPos)
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); return 0; //STRIP001 ScDocShellModificator aModificator( rDocShell );
/*N*/ }

/*N*/ BOOL ScDocFunc::DetectiveDelSucc(const ScAddress& rPos)
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	USHORT nCol = rPos.Col();
/*N*/ 	USHORT nRow = rPos.Row();
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 
/*N*/ 	BOOL bDone = ScDetectiveFunc( pDoc,nTab ).DeleteSucc( nCol, nRow );
/*N*/ 	if (bDone)
/*N*/ 	{
/*N*/ 		ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_DELSUCC );
/*N*/ 		pDoc->AddDetectiveOperation( aOperation );
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bDone;
/*N*/ }

/*N*/ BOOL ScDocFunc::DetectiveAddError(const ScAddress& rPos)
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	rDocShell.MakeDrawLayer();
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	USHORT nCol = rPos.Col();
/*N*/ 	USHORT nRow = rPos.Row();
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 
/*N*/ 	BOOL bDone = ScDetectiveFunc( pDoc,nTab ).ShowError( nCol, nRow );
/*N*/ 	if (bDone)
/*N*/ 	{
/*N*/ 		ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_ADDERROR );
/*N*/ 		pDoc->AddDetectiveOperation( aOperation );
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bDone;
/*N*/ }

/*N*/ BOOL ScDocFunc::DetectiveMarkInvalid(USHORT nTab)
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	rDocShell.MakeDrawLayer();
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 
/*N*/ 	Window* pWaitWin = rDocShell.GetDialogParent();
/*N*/ 	if (pWaitWin)
/*N*/ 		pWaitWin->EnterWait();
/*N*/ 	BOOL bOverflow;
/*N*/ 	BOOL bDone = ScDetectiveFunc( pDoc,nTab ).MarkInvalid( bOverflow );
/*N*/ 	if (pWaitWin)
/*N*/ 		pWaitWin->LeaveWait();
/*N*/ 	if (bDone)
/*N*/ 	{
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 		if ( bOverflow )
/*N*/ 		{
/*N*/ 			InfoBox( NULL,
/*N*/ 					ScGlobal::GetRscString( STR_DETINVALID_OVERFLOW ) ).Execute();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bDone;
/*N*/ }

/*N*/ BOOL ScDocFunc::DetectiveDelAll(USHORT nTab)
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	BOOL bDone = ScDetectiveFunc( pDoc,nTab ).DeleteAll( SC_DET_DETECTIVE );
/*N*/ 	if (bDone)
/*N*/ 	{
/*N*/ 		ScDetOpList* pOldList = pDoc->GetDetOpList();
/*N*/ 
/*N*/ 		pDoc->ClearDetectiveOperations();
/*N*/ 
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bDone;
/*N*/ }

/*N*/ BOOL ScDocFunc::DetectiveRefresh( BOOL bAutomatic )
/*N*/ {
/*N*/ 	BOOL bDone = FALSE;
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 	ScDetOpList* pList = pDoc->GetDetOpList();
/*N*/ 	if ( pList && pList->Count() )
/*N*/ 	{
/*N*/ 		rDocShell.MakeDrawLayer();
/*N*/ 		ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 
/*N*/ 		//	Loeschen auf allen Tabellen
/*N*/ 
/*N*/ 		USHORT nTabCount = pDoc->GetTableCount();
/*N*/ 		for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/ 			ScDetectiveFunc( pDoc,nTab ).DeleteAll( SC_DET_ARROWS );	// don't remove circles
/*N*/ 
/*N*/ 		//	Wiederholen
/*N*/ 
/*N*/ 		USHORT nCount = pList->Count();
/*N*/ 		for (USHORT i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			ScDetOpData* pData = (*pList)[i];
/*N*/ 			if (pData)
/*N*/ 			{
/*N*/ 				ScAddress aPos = pData->GetPos();
/*N*/ 				ScDetectiveFunc aFunc( pDoc, aPos.Tab() );
/*N*/ 				USHORT nCol = aPos.Col();
/*N*/ 				USHORT nRow = aPos.Row();
/*N*/ 				switch (pData->GetOperation())
/*N*/ 				{
/*N*/ 					case SCDETOP_ADDSUCC:
/*N*/ 						aFunc.ShowSucc( nCol, nRow );
/*N*/ 						break;
/*N*/ 					case SCDETOP_DELSUCC:
/*N*/ 						aFunc.DeleteSucc( nCol, nRow );
/*N*/ 						break;
/*N*/ 					case SCDETOP_ADDPRED:
/*N*/ 						aFunc.ShowPred( nCol, nRow );
/*N*/ 						break;
/*N*/ 					case SCDETOP_DELPRED:
/*N*/ 						aFunc.DeletePred( nCol, nRow );
/*N*/ 						break;
/*N*/ 					case SCDETOP_ADDERROR:
/*N*/ 						aFunc.ShowError( nCol, nRow );
/*N*/ 						break;
/*N*/ 					default:
/*N*/ 						DBG_ERROR("falsche Op bei DetectiveRefresh");
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		rDocShell.SetDrawModified();
/*N*/ 		bDone = TRUE;
/*N*/ 	}
/*N*/ 	return bDone;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::DeleteContents( const ScMarkData& rMark, USHORT nFlags,
/*N*/ 									BOOL bRecord, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
/*N*/ 	{
/*N*/ 		DBG_ERROR("ScDocFunc::DeleteContents ohne Markierung");
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 		bRecord = FALSE;
/*N*/ 
/*N*/ 	ScEditableTester aTester( pDoc, rMark );
/*N*/ 	if (!aTester.IsEditable())
/*N*/ 	{
/*N*/ 		if (!bApi)
/*N*/ 			rDocShell.ErrorMessage(aTester.GetMessageId());
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScRange aMarkRange;
/*N*/ 	BOOL bSimple = FALSE;
/*N*/ 
/*N*/ 	ScMarkData aMultiMark = rMark;
/*N*/ 	aMultiMark.SetMarking(FALSE);		// fuer MarkToMulti
/*N*/ 
/*N*/ 	ScDocument* pUndoDoc = NULL;
/*N*/ 	BOOL bMulti = !bSimple && aMultiMark.IsMultiMarked();
/*N*/ 	if (!bSimple)
/*N*/ 	{
/*N*/ 		aMultiMark.MarkToMulti();
/*N*/ 		aMultiMark.GetMultiMarkArea( aMarkRange );
/*N*/ 	}
/*N*/ 	ScRange aExtendedRange(aMarkRange);
/*N*/ 	if (!bSimple)
/*N*/ 	{
/*N*/ 		if ( pDoc->ExtendMerge( aExtendedRange, TRUE ) )
/*N*/ 			bMulti = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	// keine Objekte auf geschuetzten Tabellen
/*N*/ 	BOOL bObjects = FALSE;
/*N*/ 	if ( nFlags & IDF_OBJECTS )
/*N*/ 	{
/*N*/ 		bObjects = TRUE;
/*N*/ 		USHORT nTabCount = pDoc->GetTableCount();
/*N*/ 		for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/ 			if (aMultiMark.GetTableSelect(nTab) && pDoc->IsTabProtected(nTab))
/*N*/ 				bObjects = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT nExtFlags = 0;						// Linien interessieren nur, wenn Attribute
/*N*/ 	if ( nFlags & IDF_ATTRIB )					// geloescht werden
/*N*/ 		if (pDoc->HasAttrib( aMarkRange, HASATTR_PAINTEXT ))
/*N*/ 			nExtFlags |= SC_PF_LINES;
/*N*/ 
/*N*/ 
/*N*/ 	if (bObjects)
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if (bRecord)
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ //!	HideAllCursors();	// falls Zusammenfassung aufgehoben wird
/*N*/ 	if (bSimple)
/*N*/ 		pDoc->DeleteArea( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
/*N*/ 						  aMarkRange.aEnd.Col(),   aMarkRange.aEnd.Row(),
/*N*/ 						  aMultiMark, nFlags );
/*N*/ 	else
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pDoc->DeleteSelection( nFlags, aMultiMark );
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!AdjustRowHeight( aExtendedRange ))
/*N*/ 		rDocShell.PostPaint( aExtendedRange, PAINT_GRID, nExtFlags );
/*N*/ 	else if (nExtFlags & SC_PF_LINES)
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 lcl_PaintAbove( rDocShell, aExtendedRange );	// fuer Linien ueber dem Bereich
/*N*/ 
/*N*/ //	rDocShell.UpdateOle(GetViewData());		//! an der View?
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ //!	CellContentChanged();
/*N*/ //!	ShowAllCursors();
/*N*/ 
/*N*/ #if 0
/*N*/ 	//!	muss an der View bleiben !!!!
/*N*/ 	if ( nFlags & IDF_ATTRIB )
/*N*/ 	{
/*N*/ 		if ( nFlags & IDF_CONTENTS )
/*N*/ 			ForgetFormatArea();
/*N*/ 		else
/*N*/ 			StartFormatArea();				// Attribute loeschen ist auch Attributierung
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::SetNormalString( const ScAddress& rPos, const String& rText, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 	ScEditableTester aTester( pDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
/*N*/ 	if (!aTester.IsEditable())
/*N*/ 	{
/*N*/ 		if (!bApi)
/*N*/ 			rDocShell.ErrorMessage(aTester.GetMessageId());
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScBaseCell* pDocCell = pDoc->GetCell( rPos );
/*N*/ 	BOOL bEditDeleted = (pDocCell && pDocCell->GetCellType() == CELLTYPE_EDIT);
/*N*/ 
/*N*/ 	pDoc->SetString( rPos.Col(), rPos.Row(), rPos.Tab(), rText );
/*N*/ 
/*N*/ 	if ( bEditDeleted || pDoc->HasAttrib( ScRange(rPos), HASATTR_NEEDHEIGHT ) )
/*N*/ 		AdjustRowHeight( ScRange(rPos) );
/*N*/ 
/*N*/ 	rDocShell.PostPaintCell( rPos.Col(), rPos.Row(), rPos.Tab() );
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ 
/*N*/     // #107160# notify input handler here the same way as in PutCell
/*N*/     if (bApi)
/*?*/         NotifyInputHandler( rPos );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScDocFunc::PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, BOOL bApi )
/*N*/ {
/*N*/ 
/*N*/     ScDocShellModificator aModificator( rDocShell );
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/     BOOL bXMLLoading(pDoc->IsImportingXML());
/*N*/ 
/*N*/     // #i925#; it is not neccessary to test whether the cell is editable on loading a XML document
/*N*/     if (!bXMLLoading)
/*N*/     {
/*N*/ 	    ScEditableTester aTester( pDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
/*N*/ 	    if (!aTester.IsEditable())
/*N*/ 	    {
/*N*/ 		    if (!bApi)
/*?*/ 			    rDocShell.ErrorMessage(aTester.GetMessageId());
/*N*/ 		    pNewCell->Delete();
/*N*/ 		    return FALSE;
/*N*/ 	    }
/*N*/     }
/*N*/ 
/*N*/ 	BOOL bEditCell(FALSE);
/*N*/ 	BOOL bEditDeleted(FALSE);
/*N*/ 	BOOL bHeight = FALSE;
/*N*/ 	pDoc->PutCell( rPos, pNewCell );
/*N*/ 
/*N*/ 	//	wegen ChangeTracking darf UndoAction erst nach PutCell angelegt werden
/*N*/ 
/*N*/ 	if (bHeight)
/*N*/ 		AdjustRowHeight( ScRange(rPos) );
/*N*/ 
/*N*/     if (!bXMLLoading)
/*N*/     	rDocShell.PostPaintCell( rPos.Col(), rPos.Row(), rPos.Tab() );
/*N*/ 
/*N*/     aModificator.SetDocumentModified();
/*N*/ 
/*N*/     // #i925#; it is not neccessary to notify on loading a XML document
/*N*/     // #103934#; notify editline and cell in edit mode
/*N*/     if (bApi && !bXMLLoading)
/*N*/         NotifyInputHandler( rPos );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ void ScDocFunc::NotifyInputHandler( const ScAddress& rPos )
/*N*/ {
/*N*/ }

/*N*/ 		struct ScMyRememberItem
/*N*/ 		{
/*N*/ 			USHORT		nIndex;
/*N*/ 			SfxItemSet	aItemSet;
/*N*/ 
/*N*/ 			ScMyRememberItem(const SfxItemSet& rItemSet, USHORT nTempIndex) :
/*N*/ 				aItemSet(rItemSet), nIndex(nTempIndex) {}
/*N*/ 		};

/*N*/ 		typedef ::std::list<ScMyRememberItem*> ScMyRememberItemList;

/*N*/ BOOL ScDocFunc::PutData( const ScAddress& rPos, EditEngine& rEngine, BOOL bInterpret, BOOL bApi )
/*N*/ {
/*N*/ 	//	PutData ruft PutCell oder SetNormalString
/*N*/ 
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	ScEditAttrTester aTester( &rEngine );
/*N*/ 	BOOL bEditCell = aTester.NeedsObject();
/*N*/ 	if ( bEditCell )
/*N*/ 	{
/*N*/ 		sal_Bool bUpdateMode(rEngine.GetUpdateMode());
/*N*/ 		if (bUpdateMode)
/*N*/ 			rEngine.SetUpdateMode(sal_False);
/*N*/ 
/*N*/ 		ScMyRememberItemList aRememberItems;
/*N*/ 		ScMyRememberItem* pRememberItem = NULL;
/*N*/ 
/*N*/ 		//	All paragraph attributes must be removed before calling CreateTextObject,
/*N*/ 		//	not only alignment, so the object doesn't contain the cell attributes as
/*N*/ 		//	paragraph attributes. Before remove the attributes store they in a list to
/*N*/ 		//	set they back to the EditEngine.
/*N*/ 		USHORT nCount = rEngine.GetParagraphCount();
/*N*/ 		for (USHORT i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			const SfxItemSet& rOld = rEngine.GetParaAttribs( i );
/*N*/ 			if ( rOld.Count() )
/*N*/ 			{
/*N*/ 				pRememberItem = new ScMyRememberItem(rEngine.GetParaAttribs(i), i);
/*N*/ 				aRememberItems.push_back(pRememberItem);
/*N*/ 				rEngine.SetParaAttribs( i, SfxItemSet( *rOld.GetPool(), rOld.GetRanges() ) );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		EditTextObject* pNewData = rEngine.CreateTextObject();
/*N*/ 		bRet = PutCell( rPos,
/*N*/ 						new ScEditCell( pNewData, pDoc, rEngine.GetEditTextObjectPool() ),
/*N*/ 						bApi );
/*N*/ 		delete pNewData;
/*N*/ 
/*N*/ 		// Set the paragraph attributes back to the EditEngine.
/*N*/ 		if (!aRememberItems.empty())
/*N*/ 		{
/*N*/ 			ScMyRememberItem* pRememberItem = NULL;
/*N*/ 			ScMyRememberItemList::iterator aItr = aRememberItems.begin();
/*N*/ 			while (aItr != aRememberItems.end())
/*N*/ 			{
/*N*/ 				pRememberItem = *aItr;
/*N*/ 				rEngine.SetParaAttribs(pRememberItem->nIndex, pRememberItem->aItemSet);
/*N*/ 				delete pRememberItem;
/*N*/ 				aItr = aRememberItems.erase(aItr);
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if (bUpdateMode)
/*N*/ 			rEngine.SetUpdateMode(sal_True);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		 String aText = rEngine.GetText();
/*N*/ 		if ( bInterpret || !aText.Len() )
/*N*/ 			bRet = SetNormalString( rPos, aText, bApi );
/*N*/ 		else
/*N*/ 			bRet = PutCell( rPos, new ScStringCell( aText ), bApi );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bRet && aTester.NeedsCellAttr() )
/*N*/ 	{
/*N*/ 		const SfxItemSet& rEditAttr = aTester.GetAttribs();
/*N*/ 		ScPatternAttr aPattern( pDoc->GetPool() );
/*N*/ 		aPattern.GetFromEditItemSet( &rEditAttr );
/*N*/ 		aPattern.DeleteUnchanged( pDoc->GetPattern( rPos.Col(), rPos.Row(), rPos.Tab() ) );
/*N*/ 		aPattern.GetItemSet().ClearItem( ATTR_HOR_JUSTIFY );	// wasn't removed above if no edit object
/*N*/ 		if ( aPattern.GetItemSet().Count() > 0 )
/*N*/ 		{
/*?*/ 			ScMarkData aMark;
/*?*/ 			aMark.SelectTable( rPos.Tab(), TRUE );
/*?*/ 			aMark.SetMarkArea( ScRange( rPos ) );
/*?*/ 			ApplyAttributes( aMark, aPattern, TRUE, bApi );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }


/*N*/  ScTokenArray* lcl_ScDocFunc_CreateTokenArrayXML( const String& rText )
/*N*/  {
/*N*/  	ScTokenArray* pCode = new ScTokenArray;
/*N*/      pCode->AddString( rText );
/*N*/  	return pCode;
/*N*/  }


/*N*/ ScBaseCell* ScDocFunc::InterpretEnglishString( const ScAddress& rPos, const String& rText )
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	ScBaseCell* pNewCell = NULL;
/*N*/ 
/*N*/ 	if ( rText.Len() > 1 && rText.GetChar(0) == '=' )
/*N*/ 	{
/*N*/ 		ScTokenArray* pCode;
/*N*/ 		if ( pDoc->IsImportingXML() )
/*N*/ 		{	// temporary formula string as string tokens
/*N*/ 			pCode = lcl_ScDocFunc_CreateTokenArrayXML( rText );
/*N*/             pDoc->IncXMLImportedFormulaCount( rText.Len() );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			ScCompiler aComp( pDoc, rPos );
/*N*/ 			aComp.SetCompileEnglish( TRUE );
/*N*/ 			pCode = aComp.CompileString( rText );
/*N*/ 		}
/*N*/ 		pNewCell = new ScFormulaCell( pDoc, rPos, pCode, 0 );
/*N*/ 		delete pCode;	// Zell-ctor hat das TokenArray kopiert
/*N*/ 	}
/*N*/ 	else if ( rText.Len() > 1 && rText.GetChar(0) == '\'' )
/*N*/ 	{
/*N*/ 		//	for bEnglish, "'" at the beginning is always interpreted as text
/*N*/ 		//	marker and stripped
/*?*/ 		pNewCell = ScBaseCell::CreateTextCell( rText.Copy( 1 ), pDoc );
/*N*/ 	}
/*N*/ 	else		// (nur) auf englisches Zahlformat testen
/*N*/ 	{
/*?*/ 		SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
/*?*/ 		sal_uInt32 nEnglish = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);
/*?*/ 		double fVal;
/*?*/ 		if ( pFormatter->IsNumberFormat( rText, nEnglish, fVal ) )
/*?*/ 			pNewCell = new ScValueCell( fVal );
/*?*/ 		else if ( rText.Len() )
/*?*/ 			pNewCell = ScBaseCell::CreateTextCell( rText, pDoc );
/*?*/ 
/*?*/ 		//	das (englische) Zahlformat wird nicht gesetzt
/*?*/ 		//!	passendes lokales Format suchen und setzen???
/*N*/ 	}
/*N*/ 
/*N*/ 	return pNewCell;
/*N*/ }


/*N*/ BOOL ScDocFunc::SetCellText( const ScAddress& rPos, const String& rText,
/*N*/ 								BOOL bInterpret, BOOL bEnglish, BOOL bApi )
/*N*/ {
/*N*/ 	//	SetCellText ruft PutCell oder SetNormalString
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	ScBaseCell* pNewCell = NULL;
/*N*/ 	if ( bInterpret )
/*N*/ 	{
/*N*/ 		if ( bEnglish )
/*N*/ 		{
/*N*/ 			//	code moved to own method InterpretEnglishString because it is also used in
/*N*/ 			//	ScCellRangeObj::setFormulaArray
/*N*/ 
/*N*/ 			pNewCell = InterpretEnglishString( rPos, rText );
/*N*/ 		}
/*N*/ 		// sonst Null behalten -> SetString mit lokalen Formeln/Zahlformat
/*N*/ 	}
/*N*/ 	else if ( rText.Len() )
/*N*/ 		pNewCell = ScBaseCell::CreateTextCell( rText, pDoc );	// immer Text
/*N*/ 
/*N*/ 	if (pNewCell)
/*N*/ 		return PutCell( rPos, pNewCell, bApi );
/*N*/ 	else
/*?*/ 		return SetNormalString( rPos, rText, bApi );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::SetNoteText( const ScAddress& rPos, const String& rText, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	ScEditableTester aTester( pDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
/*N*/ 	if (!aTester.IsEditable())
/*N*/ 	{
/*?*/ 		if (!bApi)
/*?*/ 			rDocShell.ErrorMessage(aTester.GetMessageId());
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	String aNewText = rText;
/*N*/ 	aNewText.ConvertLineEnd();		//! ist das noetig ???
/*N*/ 
/*N*/ 	ScPostIt aNote;
/*N*/ 	pDoc->GetNote( rPos.Col(), rPos.Row(), rPos.Tab(), aNote );
/*N*/ 	aNote.AutoSetText( aNewText );		// setzt auch Author und Date
/*N*/ 	pDoc->SetNote( rPos.Col(), rPos.Row(), rPos.Tab(), aNote );
/*N*/ 
/*N*/ 	if ( aNote.IsShown() )
/*N*/ 	{
/*N*/ 		//	Zeichenobjekt updaten
/*N*/ 		//!	bei gelocktem Paint auch erst spaeter !!!
/*N*/ 
/*N*/ 		ScDetectiveFunc aDetFunc( pDoc, rPos.Tab() );
/*N*/ 		aDetFunc.HideComment( rPos.Col(), rPos.Row() );
/*N*/ 		aDetFunc.ShowComment( rPos.Col(), rPos.Row(), FALSE );	// FALSE: nur wenn gefunden
/*N*/ 	}
/*N*/ 
/*N*/ 	//!	Undo !!!
/*N*/ 
/*N*/ 	rDocShell.PostPaintCell( rPos.Col(), rPos.Row(), rPos.Tab() );
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
/*N*/ 									BOOL bRecord, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 		bRecord = FALSE;
/*N*/ 
/*N*/ 	// nur wegen Matrix nicht editierbar? Attribute trotzdem ok
/*N*/ 	BOOL bOnlyNotBecauseOfMatrix;
/*N*/ 	if ( !pDoc->IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
/*N*/ 			&& !bOnlyNotBecauseOfMatrix )
/*N*/ 	{
/*?*/ 		if (!bApi)
/*?*/ 			rDocShell.ErrorMessage(STR_PROTECTIONERR);
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	//!	Umrandung
/*N*/ 
/*N*/ 	ScRange aMultiRange;
/*N*/ 	BOOL bMulti = rMark.IsMultiMarked();
/*N*/ 	if ( bMulti )
/*N*/ 		rMark.GetMultiMarkArea( aMultiRange );
/*N*/ 	else
/*N*/ 		rMark.GetMarkArea( aMultiRange );
/*N*/ 
/*N*/ 
/*N*/ 	// While loading XML it is not neccessary to ask HasAttrib. It needs too much time.
/*N*/ 	BOOL bPaintExt = pDoc->IsImportingXML() || pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
/*N*/ 	pDoc->ApplySelectionPattern( rPattern, rMark );
/*N*/ 
/*N*/ 	if (!bPaintExt)
/*N*/ 		bPaintExt = pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
/*N*/ 	USHORT nExtFlags = bPaintExt ? SC_PF_LINES : 0;
/*N*/ 	if (!AdjustRowHeight( aMultiRange ))
/*N*/ 		rDocShell.PostPaint( aMultiRange, PAINT_GRID, nExtFlags );
/*N*/ 	else if (nExtFlags & SC_PF_LINES)
/*N*/ 	lcl_PaintAbove( rDocShell, aMultiRange );	// fuer Linien ueber dem Bereich
/*N*/ 
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ BOOL ScDocFunc::ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
/*N*/ 									BOOL bRecord, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 		bRecord = FALSE;
/*N*/ 
/*N*/ 	// nur wegen Matrix nicht editierbar? Attribute trotzdem ok
/*N*/ 	BOOL bOnlyNotBecauseOfMatrix;
/*N*/ 	if ( !pDoc->IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
/*N*/ 			&& !bOnlyNotBecauseOfMatrix )
/*N*/ 	{
/*?*/ 		if (!bApi)
/*?*/ 			rDocShell.ErrorMessage(STR_PROTECTIONERR);
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScStyleSheet* pStyleSheet = (ScStyleSheet*) pDoc->GetStyleSheetPool()->Find(
/*N*/ 												rStyleName, SFX_STYLE_FAMILY_PARA );
/*N*/ 	if (!pStyleSheet)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScRange aMultiRange;
/*N*/ 	BOOL bMulti = rMark.IsMultiMarked();
/*N*/ 	if ( bMulti )
/*N*/ 		rMark.GetMultiMarkArea( aMultiRange );
/*N*/ 	else
/*N*/ 		rMark.GetMarkArea( aMultiRange );
/*N*/ 
/*N*/ 
/*N*/ //	BOOL bPaintExt = pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
/*N*/ //	pDoc->ApplySelectionPattern( rPattern, rMark );
/*N*/ 
/*N*/ 	pDoc->ApplySelectionStyle( (ScStyleSheet&)*pStyleSheet, rMark );
/*N*/ 
/*N*/ //	if (!bPaintExt)
/*N*/ //		bPaintExt = pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
/*N*/ //	USHORT nExtFlags = bPaintExt ? SC_PF_LINES : 0;
/*N*/ 	USHORT nExtFlags = 0;
/*N*/ 	if (!AdjustRowHeight( aMultiRange ))
/*N*/ 		rDocShell.PostPaint( aMultiRange, PAINT_GRID, nExtFlags );
/*N*/ 	else if (nExtFlags & SC_PF_LINES)
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 lcl_PaintAbove( rDocShell, aMultiRange );	// fuer Linien ueber dem Bereich
/*N*/ 
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------

/*M*/ BOOL ScDocFunc::InsertCells( const ScRange& rRange, InsCellCmd eCmd,
/*N*/ 								BOOL bRecord, BOOL bApi, BOOL bPartOfPaste )
/*M*/ {
/*M*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 ScDocShellModificator aModificator( rDocShell );
/*M*/ }

/*N*/ BOOL ScDocFunc::DeleteCells( const ScRange& rRange, DelCellCmd eCmd, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScDocShellModificator aModificator( rDocShell );
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScDocFunc::MoveBlock( const ScRange& rSource, const ScAddress& rDestPos,
/*N*/ 								BOOL bCut, BOOL bRecord, BOOL bPaint, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScDocShellModificator aModificator( rDocShell );
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::InsertTable( USHORT nTab, const String& rName, BOOL bRecord, BOOL bApi )
/*N*/ {
/*N*/ 	BOOL bSuccess = FALSE;
/*N*/ 	WaitObject aWait( rDocShell.GetDialogParent() );
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 		bRecord = FALSE;
/*N*/ 
/*N*/ 	USHORT nTabCount = pDoc->GetTableCount();
/*N*/ 	BOOL bAppend = ( nTab >= nTabCount );
/*N*/ 	if ( bAppend )
/*N*/ 		nTab = nTabCount;		// wichtig fuer Undo
/*N*/ 
/*N*/ 	if (pDoc->InsertTab( nTab, rName ))
/*N*/ 	{
/*N*/ 		//	Views updaten:
/*N*/ 		rDocShell.Broadcast( ScTablesHint( SC_TAB_INSERTED, nTab ) );
/*N*/ 
/*N*/ 		rDocShell.PostPaintExtras();
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 		SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
/*N*/ 		bSuccess = TRUE;
/*N*/ 	}
/*N*/ 	else if (!bApi)
/*?*/ 		rDocShell.ErrorMessage(STR_TABINSERT_ERROR);
/*N*/ 
/*N*/ 	return bSuccess;
/*N*/ }

/*N*/ BOOL ScDocFunc::DeleteTable( USHORT nTab, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 WaitObject aWait( rDocShell.GetDialogParent() );
/*N*/ }

/*N*/ BOOL ScDocFunc::SetTableVisible( USHORT nTab, BOOL bVisible, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	if ( pDoc->IsVisible( nTab ) == bVisible )
/*N*/ 		return TRUE;								// nichts zu tun - ok
/*N*/ 
/*N*/ 	if ( !pDoc->IsDocEditable() )
/*N*/ 	{
/*N*/ 		if (!bApi)
/*N*/ 			rDocShell.ErrorMessage(STR_PROTECTIONERR);
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	if ( !bVisible )
/*N*/ 	{
/*N*/ 		//	nicht alle Tabellen ausblenden
/*N*/ 
/*N*/ 		USHORT nVisCount = 0;
/*N*/ 		USHORT nCount = pDoc->GetTableCount();
/*N*/ 		for (USHORT i=0; i<nCount; i++)
/*N*/ 			if (pDoc->IsVisible(i))
/*N*/ 				++nVisCount;
/*N*/ 
/*N*/ 		if (nVisCount <= 1)
/*N*/ 		{
/*N*/ 			if (!bApi)
/*N*/ 				rDocShell.ErrorMessage(STR_PROTECTIONERR);	//!	eigene Meldung?
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pDoc->SetVisible( nTab, bVisible );
/*N*/ 
/*N*/ 	//	Views updaten:
/*N*/ 	if (!bVisible)
/*N*/ 		rDocShell.Broadcast( ScTablesHint( SC_TAB_HIDDEN, nTab ) );
/*N*/ 
/*N*/ 	SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
/*N*/ 	rDocShell.PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_EXTRAS);
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScDocFunc::RenameTable( USHORT nTab, const String& rName, BOOL bRecord, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 		bRecord = FALSE;
/*N*/ 	if ( !pDoc->IsDocEditable() )
/*N*/ 	{
/*?*/ 		if (!bApi)
/*?*/ 			rDocShell.ErrorMessage(STR_PROTECTIONERR);
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	BOOL bSuccess = FALSE;
/*N*/ 	String sOldName;
/*N*/ 	pDoc->GetName(nTab, sOldName);
/*N*/ 	if (pDoc->RenameTab( nTab, rName ))
/*N*/ 	{
/*N*/ 		rDocShell.PostPaintExtras();
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 		SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
/*N*/ 
/*N*/ 		bSuccess = TRUE;
/*N*/ 	}
/*N*/ 	return bSuccess;
/*N*/ }

//------------------------------------------------------------------------

//!	SetWidthOrHeight - noch doppelt zu ViewFunc !!!!!!
//!	Probleme:
//!	- Optimale Hoehe fuer Edit-Zellen ist unterschiedlich zwischen Drucker und Bildschirm
//!	- Optimale Breite braucht Selektion, um evtl. nur selektierte Zellen zu beruecksichtigen

/*N*/ USHORT lcl_GetOptimalColWidth( ScDocShell& rDocShell, USHORT nCol, USHORT nTab, BOOL bFormula )
/*N*/ {
/*N*/ 	USHORT nTwips = 0;
/*N*/ 
/*N*/ 	ScSizeDeviceProvider aProv(&rDocShell);
/*N*/ 	OutputDevice* pDev = aProv.GetDevice();			// has pixel MapMode
/*N*/ 	double nPPTX = aProv.GetPPTX();
/*N*/ 	double nPPTY = aProv.GetPPTY();
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	Fraction aOne(1,1);
/*N*/ 	nTwips = pDoc->GetOptimalColWidth( nCol, nTab, pDev, nPPTX, nPPTY, aOne, aOne,
/*N*/ 										bFormula, NULL );
/*N*/ 
/*N*/ 	return nTwips;
/*N*/ }

/*N*/ BOOL ScDocFunc::SetWidthOrHeight( BOOL bWidth, USHORT nRangeCnt, USHORT* pRanges, USHORT nTab,
/*N*/ 										ScSizeMode eMode, USHORT nSizeTwips,
/*N*/ 										BOOL bRecord, BOOL bApi )
/*N*/ {
/*N*/ 	if (!nRangeCnt)
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 		bRecord = FALSE;
/*N*/ 
/*N*/ 	if ( !rDocShell.IsEditable() )
/*N*/ 	{
/*?*/ 		if (!bApi)
/*?*/ 			rDocShell.ErrorMessage(STR_PROTECTIONERR);		//! eigene Meldung?
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bSuccess = FALSE;
/*N*/ 	USHORT nStart = pRanges[0];
/*N*/ 	USHORT nEnd = pRanges[2*nRangeCnt-1];
/*N*/ 
/*N*/ 	BOOL bFormula = FALSE;
/*N*/ 	if ( eMode == SC_SIZE_OPTIMAL )
/*N*/ 	{
/*N*/ 		//!	Option "Formeln anzeigen" - woher nehmen?
/*N*/ 	}
/*N*/ 
/*N*/ 	ScDocument* 	pUndoDoc = NULL;
/*N*/ 	ScOutlineTable* pUndoTab = NULL;
/*N*/ 	USHORT*			pUndoRanges = NULL;
/*N*/ 
/*N*/ 	BOOL bShow = nSizeTwips > 0 || eMode != SC_SIZE_DIRECT;
/*N*/ 	BOOL bOutline = FALSE;
/*N*/ 
/*N*/ 	pDoc->IncSizeRecalcLevel( nTab );		// nicht fuer jede Spalte einzeln
/*N*/ 	for (USHORT nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
/*N*/ 	{
/*N*/ 		USHORT nStartNo = *(pRanges++);
/*N*/ 		USHORT nEndNo = *(pRanges++);
/*N*/ 
/*N*/ 		if ( !bWidth )						// Hoehen immer blockweise
/*N*/ 		{
/*N*/ 			if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
/*N*/ 			{
/*?*/ 				BOOL bAll = ( eMode==SC_SIZE_OPTIMAL );
/*?*/ 				if (!bAll)
/*?*/ 				{
/*?*/ 					//	fuer alle eingeblendeten CR_MANUALSIZE loeschen,
/*?*/ 					//	dann SetOptimalHeight mit bShrink = FALSE
/*?*/ 					for (USHORT nRow=nStartNo; nRow<=nEndNo; nRow++)
/*?*/ 					{
/*?*/ 						BYTE nOld = pDoc->GetRowFlags(nRow,nTab);
/*?*/ 						if ( (nOld & CR_HIDDEN) == 0 && ( nOld & CR_MANUALSIZE ) )
/*?*/ 							pDoc->SetRowFlags( nRow, nTab, nOld & ~CR_MANUALSIZE );
/*?*/ 					}
/*?*/ 				}
/*?*/ 
/*?*/ 				ScSizeDeviceProvider aProv( &rDocShell );
/*?*/ 				Fraction aOne(1,1);
/*?*/ 				pDoc->SetOptimalHeight( nStartNo, nEndNo, nTab, 0, aProv.GetDevice(),
/*?*/ 										aProv.GetPPTX(), aProv.GetPPTY(), aOne, aOne, bAll );
/*?*/ 
/*?*/ 				if (bAll)
/*?*/ 					pDoc->ShowRows( nStartNo, nEndNo, nTab, TRUE );
/*?*/ 
/*?*/ 				//	Manual-Flag wird bei bAll=TRUE schon in SetOptimalHeight gesetzt
/*?*/ 				//	(an bei Extra-Height, sonst aus).
/*N*/ 			}
/*N*/ 			else if ( eMode==SC_SIZE_DIRECT || eMode==SC_SIZE_ORIGINAL )
/*N*/ 			{
/*N*/ 				if (nSizeTwips)
/*N*/ 				{
/*N*/ 					pDoc->SetRowHeightRange( nStartNo, nEndNo, nTab, nSizeTwips );
/*N*/ 					pDoc->SetManualHeight( nStartNo, nEndNo, nTab, TRUE );			// height was set manually
/*N*/ 				}
/*N*/ 				if ( eMode != SC_SIZE_ORIGINAL )
/*N*/ 					pDoc->ShowRows( nStartNo, nEndNo, nTab, nSizeTwips != 0 );
/*N*/ 			}
/*N*/ 			else if ( eMode==SC_SIZE_SHOW )
/*N*/ 			{
/*?*/ 				pDoc->ShowRows( nStartNo, nEndNo, nTab, TRUE );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else								// Spaltenbreiten
/*N*/ 		{
/*N*/ 			for (USHORT nCol=nStartNo; nCol<=nEndNo; nCol++)
/*N*/ 			{
/*N*/ 				if ( eMode != SC_SIZE_VISOPT ||
/*N*/ 					 (pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN) == 0 )
/*N*/ 				{
/*N*/ 					USHORT nThisSize = nSizeTwips;
/*N*/ 
/*N*/ 					if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
/*N*/ 					 nThisSize = nSizeTwips +
/*N*/ 									lcl_GetOptimalColWidth( rDocShell, nCol, nTab, bFormula );
/*N*/ 					if ( nThisSize )
/*N*/ 						pDoc->SetColWidth( nCol, nTab, nThisSize );
/*N*/ 
/*N*/ 					if ( eMode != SC_SIZE_ORIGINAL )
/*N*/ 						pDoc->ShowCol( nCol, nTab, bShow );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 							//	adjust outlines
/*N*/ 
/*N*/ 		if ( eMode != SC_SIZE_ORIGINAL )
/*N*/ 		{
/*N*/ 			if (bWidth)
/*N*/ 				bOutline = bOutline || pDoc->UpdateOutlineCol( nStartNo, nEndNo, nTab, bShow );
/*N*/ 			else
/*N*/ 				bOutline = bOutline || pDoc->UpdateOutlineRow( nStartNo, nEndNo, nTab, bShow );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	pDoc->DecSizeRecalcLevel( nTab );		// nicht fuer jede Spalte einzeln
/*N*/ 
/*N*/ 	if (!bOutline)
/*N*/ 		DELETEZ(pUndoTab);
/*N*/ 
/*N*/ 	pDoc->UpdatePageBreaks( nTab );
/*N*/ 
/*N*/ 	rDocShell.PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_ALL);
/*N*/ 
/*N*/ 	return bSuccess;
/*N*/ }


/*N*/ BOOL ScDocFunc::InsertPageBreak( BOOL bColumn, const ScAddress& rPos,
/*N*/ 								BOOL bRecord, BOOL bSetModified, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScDocShellModificator aModificator( rDocShell );
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScDocFunc::RemovePageBreak( BOOL bColumn, const ScAddress& rPos,
/*N*/ 								BOOL bRecord, BOOL bSetModified, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 		bRecord = FALSE;
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 
/*N*/ 	USHORT nPos = bColumn ? rPos.Col() : rPos.Row();
/*N*/ 	BYTE nFlags = bColumn ? pDoc->GetColFlags( nPos, nTab ) : pDoc->GetRowFlags( nPos, nTab );
/*N*/ 	if ((nFlags & CR_MANUALBREAK)==0)
/*N*/ 		return FALSE;							// kein Umbruch gesetzt
/*N*/ 
/*?*/ 	nFlags &= ~CR_MANUALBREAK;
/*?*/ 	if (bColumn)
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pDoc->SetColFlags( nPos, nTab, nFlags );
/*?*/ 	else
/*?*/ 		pDoc->SetRowFlags( nPos, nTab, nFlags );
/*?*/ 	pDoc->UpdatePageBreaks( nTab );
/*?*/ 
/*?*/ 	if (bColumn)
/*?*/ 	{
/*?*/ 		rDocShell.PostPaint( nPos-1, 0, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID );
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		rDocShell.PostPaint( 0, nPos-1, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID );
/*?*/ 	}
/*?*/ 
/*?*/ 	if (bSetModified)
/*?*/ 		aModificator.SetDocumentModified();
/*?*/ 
/*?*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL lcl_ValidPassword( ScDocument* pDoc, USHORT nTab,
/*N*/ 						const String& rPassword,
/*N*/ 						uno::Sequence<sal_Int8>* pReturnOld = NULL )
/*N*/ {
/*N*/ 	uno::Sequence<sal_Int8> aOldPassword;
/*N*/ 	if ( nTab == TABLEID_DOC )
/*N*/ 	{
/*N*/ 		if (pDoc->IsDocProtected())
/*N*/ 			aOldPassword = pDoc->GetDocPassword();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (pDoc->IsTabProtected(nTab))
/*N*/ 			aOldPassword = pDoc->GetTabPassword(nTab);
/*N*/ 	}
/*N*/ 
/*N*/ 	if (pReturnOld)
/*N*/ 		*pReturnOld = aOldPassword;
/*N*/ 
/*N*/ 	return ((aOldPassword.getLength() == 0) || SvPasswordHelper::CompareHashPassword(aOldPassword, rPassword));
/*N*/ }

/*N*/ BOOL ScDocFunc::Protect( USHORT nTab, const String& rPassword, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	BOOL bOk = lcl_ValidPassword( pDoc, nTab, rPassword);
/*N*/ 	if ( bOk )
/*N*/ 	{
/*N*/ 	    uno::Sequence<sal_Int8> aPass;
/*N*/ 	    if (rPassword.Len())
/*N*/ 	        SvPasswordHelper::GetHashPassword(aPass, rPassword);
/*N*/ 
/*N*/ 		if ( nTab == TABLEID_DOC )
/*N*/ 			pDoc->SetDocProtection( TRUE, aPass );
/*N*/ 		else
/*N*/ 			pDoc->SetTabProtection( nTab, TRUE, aPass );
/*N*/ 
/*N*/ 		rDocShell.PostPaintGridAll();
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 	else if (!bApi)
/*N*/ 	{
/*N*/ 		//	different password was set before
/*N*/ 
/*N*/ //!		rDocShell.ErrorMessage(...);
/*N*/ 
/*N*/ 		InfoBox aBox( rDocShell.GetDialogParent(), String( ScResId( SCSTR_WRONGPASSWORD ) ) );
/*N*/ 		aBox.Execute();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }

/*N*/ BOOL ScDocFunc::Unprotect( USHORT nTab, const String& rPassword, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	uno::Sequence<sal_Int8> aOldPassword;
/*N*/ 	uno::Sequence<sal_Int8> aPass;
/*N*/ 	BOOL bOk = lcl_ValidPassword( pDoc, nTab, rPassword, &aOldPassword );
/*N*/ 	if ( bOk )
/*N*/ 	{
/*N*/ 		uno::Sequence<sal_Int8> aEmptyPass;
/*N*/ 		if ( nTab == TABLEID_DOC )
/*N*/ 			pDoc->SetDocProtection( FALSE, aEmptyPass );
/*N*/ 		else
/*N*/ 			pDoc->SetTabProtection( nTab, FALSE, aEmptyPass );
/*N*/ 
/*N*/ 		rDocShell.PostPaintGridAll();
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 	else if (!bApi)
/*N*/ 	{
/*N*/ //!		rDocShell.ErrorMessage(...);
/*N*/ 
/*N*/ 		InfoBox aBox( rDocShell.GetDialogParent(), String( ScResId( SCSTR_WRONGPASSWORD ) ) );
/*N*/ 		aBox.Execute();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::ClearItems( const ScMarkData& rMark, const USHORT* pWhich, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	ScEditableTester aTester( pDoc, rMark );
/*N*/ 	if (!aTester.IsEditable())
/*N*/ 	{
/*?*/ 		if (!bApi)
/*?*/ 			rDocShell.ErrorMessage(aTester.GetMessageId());
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	//	#i12940# ClearItems is called (from setPropertyToDefault) directly with uno object's cached
/*N*/ 	//	MarkData (GetMarkData), so rMark must be changed to multi selection for ClearSelectionItems
/*N*/ 	//	here.
/*N*/ 
/*N*/ 	ScRange aMarkRange;
/*N*/ 	ScMarkData aMultiMark = rMark;
/*N*/ 	aMultiMark.SetMarking(FALSE);		// for MarkToMulti
/*N*/ 	aMultiMark.MarkToMulti();
/*N*/ 	aMultiMark.GetMultiMarkArea( aMarkRange );
/*N*/ 
/*N*/ 	pDoc->ClearSelectionItems( pWhich, aMultiMark );
/*N*/ 
/*N*/ 	rDocShell.PostPaint( aMarkRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScDocFunc::ChangeIndent( const ScMarkData& rMark, BOOL bIncrement, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	ScEditableTester aTester( pDoc, rMark );
/*N*/ 	if (!aTester.IsEditable())
/*N*/ 	{
/*N*/ 		if (!bApi)
/*N*/ 			rDocShell.ErrorMessage(aTester.GetMessageId());
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScRange aMarkRange;
/*N*/ 	rMark.GetMultiMarkArea( aMarkRange );
/*N*/ 
/*N*/ 	pDoc->ChangeSelectionIndent( bIncrement, rMark );
/*N*/ 
/*N*/ 	rDocShell.PostPaint( aMarkRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScDocFunc::AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
/*N*/ 							USHORT nFormatNo, BOOL bRecord, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	BOOL bSuccess = FALSE;
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nStartTab = rRange.aStart.Tab();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 	USHORT nEndTab = rRange.aEnd.Tab();
/*N*/ 
/*N*/ 		bRecord = FALSE;
/*N*/ 	ScMarkData aMark;
/*N*/ 	if (pTabMark)
/*N*/ 		aMark = *pTabMark;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
/*N*/ 			aMark.SelectTable( nTab, TRUE );
/*N*/ 	}
/*N*/ 
/*N*/ 	ScAutoFormat* pAutoFormat = ScGlobal::GetAutoFormat();
/*N*/ 	ScEditableTester aTester( pDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
/*N*/ 	if ( pAutoFormat && nFormatNo < pAutoFormat->GetCount() && aTester.IsEditable() )
/*N*/ 	{
/*N*/ 		WaitObject aWait( rDocShell.GetDialogParent() );
/*N*/ 
/*N*/ 		BOOL bSize = (*pAutoFormat)[nFormatNo]->GetIncludeWidthHeight();
/*N*/ 
/*N*/ 		USHORT nTabCount = pDoc->GetTableCount();
/*N*/ 		ScDocument* pUndoDoc = NULL;
/*N*/ 
/*N*/ 		pDoc->AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo, aMark );
/*N*/ 
/*N*/ 		if (bSize)
/*N*/ 		{
/*			USHORT nCols[2];
            nCols[0] = nStartCol;
            nCols[1] = nEndCol;
            USHORT nRows[2];
            nRows[0] = nStartRow;
            nRows[1] = nEndRow;
*/
/*N*/ 			USHORT nCols[2] = { nStartCol, nEndCol };
/*N*/ 			USHORT nRows[2] = { nStartRow, nEndRow };
/*N*/ 
/*N*/ 			for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/ 				if (aMark.GetTableSelect(nTab))
/*N*/ 				{
/*N*/ 					SetWidthOrHeight( TRUE, 1,nCols, nTab, SC_SIZE_VISOPT, STD_EXTRA_WIDTH, FALSE, TRUE );
/*N*/ 					SetWidthOrHeight( FALSE,1,nRows, nTab, SC_SIZE_VISOPT, 0, FALSE, FALSE );
/*N*/ 					rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
/*N*/ 									PAINT_GRID | PAINT_LEFT | PAINT_TOP );
/*N*/ 				}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/ 				if (aMark.GetTableSelect(nTab))
/*N*/ 				{
/*N*/ 					BOOL bAdj = AdjustRowHeight( ScRange(nStartCol, nStartRow, nTab,
/*N*/ 														nEndCol, nEndRow, nTab), FALSE );
/*N*/ 					if (bAdj)
/*N*/ 						rDocShell.PostPaint( 0,nStartRow,nTab, MAXCOL,MAXROW,nTab,
/*N*/ 											PAINT_GRID | PAINT_LEFT );
/*N*/ 					else
/*N*/ 						rDocShell.PostPaint( nStartCol, nStartRow, nTab,
/*N*/ 											nEndCol, nEndRow, nTab, PAINT_GRID );
/*N*/ 				}
/*N*/ 		}
/*N*/ 
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 	else if (!bApi)
/*N*/ 		rDocShell.ErrorMessage(aTester.GetMessageId());
/*N*/ 
/*N*/ 	return bSuccess;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::EnterMatrix( const ScRange& rRange, const ScMarkData* pTabMark,
/*N*/ 								const String& rString, BOOL bApi, BOOL bEnglish )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	BOOL bSuccess = FALSE;
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nStartTab = rRange.aStart.Tab();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 	USHORT nEndTab = rRange.aEnd.Tab();
/*N*/ 
/*N*/ 	ScMarkData aMark;
/*N*/ 	if (pTabMark)
/*N*/ 		aMark = *pTabMark;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
/*N*/ 			aMark.SelectTable( nTab, TRUE );
/*N*/ 	}
/*N*/ 
/*N*/ 	ScEditableTester aTester( pDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
/*N*/ 	if ( aTester.IsEditable() )
/*N*/ 	{
/*N*/ 		WaitObject aWait( rDocShell.GetDialogParent() );
/*N*/ 
/*N*/ 		if ( pDoc->IsImportingXML() )
/*N*/ 		{
/*N*/ 			ScTokenArray* pCode = lcl_ScDocFunc_CreateTokenArrayXML( rString );
/*N*/ 			pDoc->InsertMatrixFormula(nStartCol,nStartRow,nEndCol,nEndRow,aMark,EMPTY_STRING,pCode);
/*N*/ 			delete pCode;
/*N*/             pDoc->IncXMLImportedFormulaCount( rString.Len() );
/*N*/ 		}
/*N*/ 		else if (bEnglish)
/*N*/         {
/*N*/ 			ScCompiler aComp( pDoc, rRange.aStart );
/*N*/ 			aComp.SetCompileEnglish( TRUE );
/*N*/ 			ScTokenArray* pCode = aComp.CompileString( rString );
/*N*/ 			pDoc->InsertMatrixFormula(nStartCol,nStartRow,nEndCol,nEndRow,aMark,EMPTY_STRING,pCode);
/*N*/ 			delete pCode;
/*N*/         }
/*N*/         else
/*N*/ 			pDoc->InsertMatrixFormula(nStartCol,nStartRow,nEndCol,nEndRow,aMark,rString);
/*N*/ 
/*N*/ 		//	Err522 beim Paint von DDE-Formeln werden jetzt beim Interpretieren abgefangen
/*N*/ 		rDocShell.PostPaint( nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab, PAINT_GRID );
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 
/*N*/ 		bSuccess = TRUE;
/*N*/ 	}
/*N*/ 	else if (!bApi)
/*N*/ 		rDocShell.ErrorMessage(aTester.GetMessageId());
/*N*/ 
/*N*/ 	return bSuccess;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::TabOp( const ScRange& rRange, const ScMarkData* pTabMark,
/*N*/ 							const ScTabOpParam& rParam, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 ScDocShellModificator aModificator( rDocShell );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ inline ScDirection DirFromFillDir( FillDir eDir )
/*N*/ {
/*N*/ 	if (eDir==FILL_TO_BOTTOM)
/*N*/ 		return DIR_BOTTOM;
/*N*/ 	else if (eDir==FILL_TO_RIGHT)
/*N*/ 		return DIR_RIGHT;
/*N*/ 	else if (eDir==FILL_TO_TOP)
/*N*/ 		return DIR_TOP;
/*N*/ 	else // if (eDir==FILL_TO_LEFT)
/*N*/ 		return DIR_LEFT;
/*N*/ }


/*N*/ BOOL ScDocFunc::FillSeries( const ScRange& rRange, const ScMarkData* pTabMark,
/*N*/ 							FillDir	eDir, FillCmd eCmd, FillDateCmd	eDateCmd,
/*N*/ 							double fStart, double fStep, double fMax,
/*N*/ 							BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 ScDocShellModificator aModificator( rDocShell );
/*N*/ }

/*N*/ BOOL ScDocFunc::FillAuto( ScRange& rRange, const ScMarkData* pTabMark,
/*N*/ 							FillDir eDir, USHORT nCount, BOOL bRecord, BOOL bApi )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScDocShellModificator aModificator( rDocShell );
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::MergeCells( const ScRange& rRange, BOOL bContents, BOOL bRecord, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 	USHORT nTab = rRange.aStart.Tab();
/*N*/ 
/*N*/ 		bRecord = FALSE;
/*N*/ 
/*N*/ 	ScEditableTester aTester( pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
/*N*/ 	if (!aTester.IsEditable())
/*N*/ 	{
/*?*/ 		if (!bApi)
/*?*/ 			rDocShell.ErrorMessage(aTester.GetMessageId());
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( nStartCol == nEndCol && nStartRow == nEndRow )
/*N*/ 	{
/*N*/ 		// nichts zu tun
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pDoc->HasAttrib( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
/*N*/ 							HASATTR_MERGED | HASATTR_OVERLAPPED ) )
/*N*/ 	{
/*?*/ 		// "Zusammenfassen nicht verschachteln !"
/*?*/ 		if (!bApi)
/*?*/ 			rDocShell.ErrorMessage(STR_MSSG_MERGECELLS_0);
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bNeedContents = bContents && 
/*N*/ 			( !pDoc->IsBlockEmpty( nTab, nStartCol,nStartRow+1, nStartCol,nEndRow ) ||
/*N*/ 			  !pDoc->IsBlockEmpty( nTab, nStartCol+1,nStartRow, nEndCol,nEndRow ) );
/*N*/ 
/*N*/ 	if (bNeedContents && bContents)
/*?*/ 	{	DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pDoc->DoMergeContents( nTab, nStartCol,nStartRow, nEndCol,nEndRow );
/*N*/ 	pDoc->DoMerge( nTab, nStartCol,nStartRow, nEndCol,nEndRow );
/*N*/ 
/*N*/ 	if ( !AdjustRowHeight( ScRange( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab ) ) )
/*N*/ 		rDocShell.PostPaint( nStartCol, nStartRow, nTab,
/*N*/ 											nEndCol, nEndRow, nTab, PAINT_GRID );
/*N*/ 	if (bNeedContents && bContents)
/*?*/ 		pDoc->SetDirty( rRange );
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScDocFunc::UnmergeCells( const ScRange& rRange, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScDocShellModificator aModificator( rDocShell );
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::ModifyRangeNames( const ScRangeName& rNewRanges, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 	pDoc->CompileNameFormula( TRUE );	// CreateFormulaString
/*N*/ 	pDoc->SetRangeName( new ScRangeName( rNewRanges ) );
/*N*/ 	pDoc->CompileNameFormula( FALSE );	// CompileFormulaString
/*N*/ 
/*N*/ 	aModificator.SetDocumentModified();
/*N*/ 	SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocFunc::CreateOneName( ScRangeName& rList,
/*N*/ 								USHORT nPosX, USHORT nPosY, USHORT nTab,
/*N*/ 								USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
/*N*/ 								BOOL& rCancel, BOOL bApi )
/*N*/ {
/*N*/ 	if (rCancel)
/*N*/ 		return;
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	if (!pDoc->HasValueData( nPosX, nPosY, nTab ))
/*N*/ 	{
/*N*/ 		String aName;
/*N*/ 		pDoc->GetString( nPosX, nPosY, nTab, aName );
/*N*/ 		ScRangeData::MakeValidName(aName);
/*N*/ 		if (aName.Len())
/*N*/ 		{
/*N*/ 			String aContent;
/*N*/             ScRange( nX1, nY1, nTab, nX2, nY2, nTab ).Format( aContent, SCR_ABS_3D, pDoc );
/*N*/ 
/*N*/ 			ScRangeName* pList = pDoc->GetRangeName();
/*N*/ 
/*N*/ 			BOOL bInsert = FALSE;
/*N*/ 			USHORT nOldPos;
/*N*/ 			if (rList.SearchName( aName, nOldPos ))			// vorhanden ?
/*N*/ 			{
/*N*/ 				ScRangeData* pOld = rList[nOldPos];
/*N*/ 				String aOldStr;
/*N*/ 				pOld->GetSymbol( aOldStr );
/*N*/ 				if (aOldStr != aContent)
/*N*/ 				{
/*N*/ 					if (bApi)
/*N*/ 						bInsert = TRUE;		// per API nicht nachfragen
/*N*/ 					else
/*N*/ 					{
/*N*/ 						String aTemplate = ScGlobal::GetRscString( STR_CREATENAME_REPLACE );
/*N*/ 
/*N*/ 						String aMessage = aTemplate.GetToken( 0, '#' );
/*N*/ 						aMessage += aName;
/*N*/ 						aMessage += aTemplate.GetToken( 1, '#' );
/*N*/ 
/*N*/ 						short nResult = QueryBox( rDocShell.GetDialogParent(),
/*N*/ 													WinBits(WB_YES_NO_CANCEL | WB_DEF_YES),
/*N*/ 													aMessage ).Execute();
/*N*/ 						if ( nResult == RET_YES )
/*N*/ 						{
/*N*/ 							rList.AtFree(nOldPos);
/*N*/ 							bInsert = TRUE;
/*N*/ 						}
/*N*/ 						else if ( nResult == RET_CANCEL )
/*N*/ 							rCancel = TRUE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bInsert = TRUE;
/*N*/ 
/*N*/ 			if (bInsert)
/*N*/ 			{
/*N*/ 				ScRangeData* pData = new ScRangeData( pDoc, aName, aContent, nPosX, nPosY, nTab );
/*N*/ 				if (!rList.Insert(pData))
/*N*/ 				{
/*N*/ 					DBG_ERROR("nanu?");
/*N*/ 					delete pData;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScDocFunc::CreateNames( const ScRange& rRange, USHORT nFlags, BOOL bApi )
/*N*/ {
/*N*/ 	if (!nFlags)
/*N*/ 		return FALSE;		// war nix
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 	BOOL bDone = FALSE;
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 	USHORT nTab = rRange.aStart.Tab();
/*N*/ 	DBG_ASSERT(rRange.aEnd.Tab() == nTab, "CreateNames: mehrere Tabellen geht nicht");
/*N*/ 
/*N*/ 	BOOL bValid = TRUE;
/*N*/ 	if ( nFlags & ( NAME_TOP | NAME_BOTTOM ) )
/*N*/ 		if ( nStartRow == nEndRow )
/*N*/ 			bValid = FALSE;
/*N*/ 	if ( nFlags & ( NAME_LEFT | NAME_RIGHT ) )
/*N*/ 		if ( nStartCol == nEndCol )
/*N*/ 			bValid = FALSE;
/*N*/ 
/*N*/ 	if (bValid)
/*N*/ 	{
/*N*/ 		ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 		ScRangeName* pNames = pDoc->GetRangeName();
/*N*/ 		if (!pNames)
/*N*/ 			return FALSE;	// soll nicht sein
/*N*/ 		ScRangeName aNewRanges( *pNames );
/*N*/ 
/*N*/ 		BOOL bTop    = ( ( nFlags & NAME_TOP ) != 0 );
/*N*/ 		BOOL bLeft   = ( ( nFlags & NAME_LEFT ) != 0 );
/*N*/ 		BOOL bBottom = ( ( nFlags & NAME_BOTTOM ) != 0 );
/*N*/ 		BOOL bRight  = ( ( nFlags & NAME_RIGHT ) != 0 );
/*N*/ 
/*N*/ 		USHORT nContX1 = nStartCol;
/*N*/ 		USHORT nContY1 = nStartRow;
/*N*/ 		USHORT nContX2 = nEndCol;
/*N*/ 		USHORT nContY2 = nEndRow;
/*N*/ 
/*N*/ 		if ( bTop )
/*N*/ 			++nContY1;
/*N*/ 		if ( bLeft )
/*N*/ 			++nContX1;
/*N*/ 		if ( bBottom )
/*N*/ 			--nContY2;
/*N*/ 		if ( bRight )
/*N*/ 			--nContX2;
/*N*/ 
/*N*/ 		BOOL bCancel = FALSE;
/*N*/ 		USHORT i;
/*N*/ 
/*N*/ 		if ( bTop )
/*N*/ 			for (i=nContX1; i<=nContX2; i++)
/*N*/ 				CreateOneName( aNewRanges, i,nStartRow,nTab, i,nContY1,i,nContY2, bCancel, bApi );
/*N*/ 		if ( bLeft )
/*N*/ 			for (i=nContY1; i<=nContY2; i++)
/*N*/ 				CreateOneName( aNewRanges, nStartCol,i,nTab, nContX1,i,nContX2,i, bCancel, bApi );
/*N*/ 		if ( bBottom )
/*N*/ 			for (i=nContX1; i<=nContX2; i++)
/*N*/ 				CreateOneName( aNewRanges, i,nEndRow,nTab, i,nContY1,i,nContY2, bCancel, bApi );
/*N*/ 		if ( bRight )
/*N*/ 			for (i=nContY1; i<=nContY2; i++)
/*N*/ 				CreateOneName( aNewRanges, nEndCol,i,nTab, nContX1,i,nContX2,i, bCancel, bApi );
/*N*/ 
/*N*/ 		if ( bTop && bLeft )
/*N*/ 			CreateOneName( aNewRanges, nStartCol,nStartRow,nTab, nContX1,nContY1,nContX2,nContY2, bCancel, bApi );
/*N*/ 		if ( bTop && bRight )
/*N*/ 			CreateOneName( aNewRanges, nEndCol,nStartRow,nTab, nContX1,nContY1,nContX2,nContY2, bCancel, bApi );
/*N*/ 		if ( bBottom && bLeft )
/*N*/ 			CreateOneName( aNewRanges, nStartCol,nEndRow,nTab, nContX1,nContY1,nContX2,nContY2, bCancel, bApi );
/*N*/ 		if ( bBottom && bRight )
/*N*/ 			CreateOneName( aNewRanges, nEndCol,nEndRow,nTab, nContX1,nContY1,nContX2,nContY2, bCancel, bApi );
/*N*/ 
/*N*/ 		bDone = ModifyRangeNames( aNewRanges, bApi );
/*N*/ 
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 		SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	return bDone;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::InsertNameList( const ScAddress& rStartPos, BOOL bApi )
/*N*/ {
/*N*/ 	ScDocShellModificator aModificator( rDocShell );
/*N*/ 
/*N*/ 
/*N*/ 	BOOL bDone = FALSE;
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	USHORT nTab = rStartPos.Tab();
/*N*/ 	ScDocument* pUndoDoc = NULL;
/*N*/ 
/*N*/ 	ScRangeName* pList = pDoc->GetRangeName();
/*N*/ 	USHORT nCount = pList->GetCount();
/*N*/ 	USHORT nValidCount = 0;
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScRangeData* pData = (*pList)[i];
/*N*/ 		if ( !pData->HasType( RT_DATABASE ) && !pData->HasType( RT_SHARED ) )
/*N*/ 			++nValidCount;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (nValidCount)
/*N*/ 	{
/*N*/ 		USHORT nStartCol = rStartPos.Col();
/*N*/ 		USHORT nStartRow = rStartPos.Row();
/*N*/ 		USHORT nEndCol = nStartCol + 1;
/*N*/ 		USHORT nEndRow = nStartRow + nValidCount - 1;
/*N*/ 
/*N*/ 		ScEditableTester aTester( pDoc, nTab, nStartCol,nStartRow, nEndCol,nEndRow );
/*N*/ 		if (aTester.IsEditable())
/*N*/ 		{
/*N*/ 			ScRangeData** ppSortArray = new ScRangeData* [ nValidCount ];
/*N*/ 			USHORT j = 0;
/*N*/ 			for (i=0; i<nCount; i++)
/*N*/ 			{
/*N*/ 				ScRangeData* pData = (*pList)[i];
/*N*/ 				if ( !pData->HasType( RT_DATABASE ) && !pData->HasType( RT_SHARED ) )
/*N*/ 					ppSortArray[j++] = pData;
/*N*/ 			}
/*N*/ #ifndef ICC
/*N*/ 			qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
/*N*/ 				&ScRangeData::QsortNameCompare );
/*N*/ #else
/*N*/ 			qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
/*N*/ 				ICCQsortNameCompare );
/*N*/ #endif
/*N*/ 			String aName;
/*N*/ 			String aContent;
/*N*/ 			String aFormula;
/*N*/ 			USHORT nOutRow = nStartRow;
/*N*/ 			for (j=0; j<nValidCount; j++)
/*N*/ 			{
/*N*/ 				ScRangeData* pData = ppSortArray[j];
/*N*/ 				pData->GetName(aName);
/*N*/ 				// relative Referenzen Excel-konform auf die linke Spalte anpassen:
/*N*/ 				pData->UpdateSymbol(aContent, ScAddress( nStartCol, nOutRow, nTab ));
/*N*/ 				aFormula = '=';
/*N*/ 				aFormula += aContent;
/*N*/ 				pDoc->PutCell( nStartCol,nOutRow,nTab, new ScStringCell( aName ) );
/*N*/ 				pDoc->PutCell( nEndCol  ,nOutRow,nTab, new ScStringCell( aFormula ) );
/*N*/ 				++nOutRow;
/*N*/ 			}
/*N*/ 
/*N*/ 			delete [] ppSortArray;
/*N*/ 
/*N*/ 			if (!AdjustRowHeight(ScRange(0,nStartRow,nTab,MAXCOL,nEndRow,nTab)))
/*N*/ 				rDocShell.PostPaint( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab, PAINT_GRID );
/*N*/ //!			rDocShell.UpdateOle(GetViewData());
/*N*/ 			aModificator.SetDocumentModified();
/*N*/ 			bDone = TRUE;
/*N*/ 		}
/*N*/ 		else if (!bApi)
/*?*/ 			rDocShell.ErrorMessage(aTester.GetMessageId());
/*N*/ 	}
/*N*/ 	return bDone;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ BOOL ScDocFunc::InsertAreaLink( const String& rFile, const String& rFilter,
/*N*/ 								const String& rOptions, const String& rSource,
/*N*/ 								const ScRange& rDestRange, ULONG nRefresh,
/*N*/ 								BOOL bFitBlock, BOOL bApi )
/*N*/ {
/*N*/ 	//!	auch fuer ScViewFunc::InsertAreaLink benutzen!
/*N*/ 
/*N*/ 	ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 
/*N*/ 	String aFilterName = rFilter;
/*N*/ 	String aNewOptions = rOptions;
/*N*/ 	if (!aFilterName.Len())
/*N*/ 		ScDocumentLoader::GetFilterName( rFile, aFilterName, aNewOptions );
/*N*/ 
/*N*/ 	//	remove application prefix from filter name here, so the filter options
/*N*/ 	//	aren't reset when the filter name is changed in ScAreaLink::DataChanged
/*N*/ 	ScDocumentLoader::RemoveAppPrefix( aFilterName );
/*N*/ 
/*N*/ 	SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
/*N*/ 
/*N*/ 	ScAreaLink* pLink = new ScAreaLink( &rDocShell, rFile, aFilterName,
/*N*/ 										aNewOptions, rSource, rDestRange, nRefresh );
/*N*/ 	pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, rFile, &aFilterName, &rSource );
/*N*/ 
/*N*/ 	pLink->SetDoInsert(bFitBlock);	// beim ersten Update ggf. nichts einfuegen
/*N*/ 	pLink->Update();				// kein SetInCreate -> Update ausfuehren
/*N*/ 	pLink->SetDoInsert(TRUE);		// Default = TRUE
/*N*/ 
/*N*/ 
/*N*/ 	SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );		// Navigator
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }




}
