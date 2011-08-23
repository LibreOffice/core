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

#ifdef _MSC_VER
#pragma optimize("",off)
#endif
#ifdef _MSC_VER
#pragma optimize("q",off) // p-code off
#endif

// INCLUDE ---------------------------------------------------------------

#include <bf_sfx2/objsh.hxx>
#include <bf_svx/svditer.hxx>
#include <bf_svx/svdoole2.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_sch/schdll.hxx>
#include <bf_sch/memchrt.hxx>
#include <bf_sch/schdll0.hxx>

#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "refupdat.hxx"
namespace binfilter {

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

// -----------------------------------------------------------------------

        // Charts aus altem Dokument updaten

/*N*/ void ScDocument::UpdateAllCharts(BOOL bDoUpdate)
/*N*/ {
/*N*/ 	if (!pDrawLayer)
/*N*/ 		return;
/*N*/ 
/*N*/ 	USHORT nDataCount = pChartCollection->GetCount();
/*N*/ 	if ( !nDataCount )
/*N*/ 		return ;		// nothing to do
/*?*/ 
/*?*/ 	USHORT nPos;
/*?*/ 
/*?*/ 	for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
/*?*/ 	{
/*?*/ 		if (pTab[nTab])
/*?*/ 		{
/*?*/ 			SdrPage* pPage = pDrawLayer->GetPage(nTab);
/*?*/ 			DBG_ASSERT(pPage,"Page ?");
/*?*/ 
/*?*/ 			ScRange aRange;
/*?*/ 			SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
/*?*/ 			SdrObject* pObject = aIter.Next();
/*?*/ 			while (pObject)
/*?*/ 			{
/*?*/ 				if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
/*?*/ 				{
/*?*/ 					SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
/*?*/ 					if (aIPObj.Is())
/*?*/ 					{
/*?*/ 						// String aIPName = aIPObj->GetName()->GetName();
/*?*/ 
/*?*/ 						SvInfoObject* pInfoObj = pShell->Find( aIPObj );
/*?*/ 						String aIPName;
/*?*/ 
/*?*/ 						if ( pInfoObj )
/*?*/ 							aIPName = pInfoObj->GetObjName();
/*?*/ 
/*?*/ 						for (nPos=0; nPos<nDataCount; nPos++)
/*?*/ 						{
/*?*/ 							ScChartArray* pChartObj = (*pChartCollection)[nPos];
/*?*/ 							if (pChartObj->GetName() == aIPName)
/*?*/ 							{
/*?*/ 								if (bDoUpdate)
/*?*/ 								{
/*?*/ 									SchMemChart* pMemChart = pChartObj->CreateMemChart();
/*?*/ 									SchDLL::Update( aIPObj, pMemChart );
/*?*/ 									delete pMemChart;
/*?*/ 								}
/*?*/ 								else		// nur Position uebernehmen
/*?*/ 								{
/*?*/ 									SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
/*?*/ 									if (pChartData)
/*?*/ 									{
/*?*/ 										pChartObj->SetExtraStrings(*pChartData);
/*?*/ //										aIPObj->SetModified( TRUE );
/*?*/ 									}
/*?*/ 								}
/*?*/ 								ScChartListener* pCL = new ScChartListener(
/*?*/ 									aIPName, this, pChartObj->GetRangeList() );
/*?*/ 								pChartListenerCollection->Insert( pCL );
/*?*/ 								pCL->StartListeningTo();
/*?*/ 							}
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 				pObject = aIter.Next();
/*?*/ 			}
/*?*/ 		}
/*?*/ 	}
/*?*/ 
/*?*/ 	pChartCollection->FreeAll();
/*N*/ }


/*N*/ void ScDocument::UpdateChartArea( const String& rChartName,
/*N*/ 			const ScRangeListRef& rNewList, BOOL bColHeaders, BOOL bRowHeaders,
/*N*/ 			BOOL bAdd, Window* pWindow )
/*N*/ {
/*N*/ 	if (!pDrawLayer)
/*N*/ 		return;
/*N*/ 
/*N*/ 	for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
/*N*/ 	{
/*N*/ 		SdrPage* pPage = pDrawLayer->GetPage(nTab);
/*N*/ 		DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 		SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
/*N*/ 		SdrObject* pObject = aIter.Next();
/*N*/ 		while (pObject)
/*N*/ 		{
/*N*/ 			if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
/*N*/ 					((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
/*N*/ 			{
/*N*/ 				SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
/*N*/ 				if (aIPObj.Is())
/*N*/ 				{
/*N*/ 					const SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
/*N*/ 					if ( pChartData )
/*N*/ 					{
/*N*/ 						ScChartArray aArray( this, *pChartData );
/*N*/ 						if ( bAdd )
/*N*/ 						{
/*N*/ 							// bei bAdd werden Header-Angaben ignoriert
/*N*/ 							aArray.AddToRangeList( rNewList );
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							aArray.SetRangeList( rNewList );
/*N*/ 							aArray.SetHeaders( bColHeaders, bRowHeaders );
/*N*/ 						}
/*N*/ 						pChartListenerCollection->ChangeListening(
/*N*/ 							rChartName, aArray.GetRangeList() );
/*N*/ 
/*N*/ 
/*N*/ 						SchMemChart* pMemChart = aArray.CreateMemChart();
/*N*/ 						ScChartArray::CopySettings( *pMemChart, *pChartData );
/*N*/ 
/*N*/ 						SchDLL::Update( aIPObj, pMemChart, pWindow );
/*N*/ 						delete pMemChart;
/*N*/ 
/*N*/ 						// Dies veranlaesst Chart zum sofortigen Update
/*N*/ 						//SvData aEmpty;
/*N*/ 						//aIPObj->SendDataChanged( aEmpty );
/*N*/ 						aIPObj->SendViewChanged();
/*N*/ 						pObject->SendRepaintBroadcast();
/*N*/ 
/*N*/ 						return;			// nicht weitersuchen
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			pObject = aIter.Next();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::UpdateChart( const String& rChartName, Window* pWindow )
/*N*/ {
/*N*/ 	if (!pDrawLayer || bInDtorClear)
/*N*/ 		return;
/*N*/ 
/*N*/ 	for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
/*N*/ 	{
/*N*/ 		SdrPage* pPage = pDrawLayer->GetPage(nTab);
/*N*/ 		DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 		SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
/*N*/ 		SdrObject* pObject = aIter.Next();
/*N*/ 		while (pObject)
/*N*/ 		{
/*N*/ 			if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
/*N*/ 					((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
/*N*/ 			{
/*N*/ 				SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
/*N*/ 				if (aIPObj.Is())
/*N*/ 				{
/*N*/ 					const SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
/*N*/ 					if ( pChartData )
/*N*/ 					{
/*N*/ 						ScChartArray aArray( this, *pChartData );
/*N*/ 
/*N*/ 						SchMemChart* pMemChart = aArray.CreateMemChart();
/*N*/ 						ScChartArray::CopySettings( *pMemChart, *pChartData );
/*N*/ 
/*N*/ 						//	#57655# Chart-Update ohne geaenderte Einstellungen (MemChart)
/*N*/ 						//	soll das Dokument nicht auf modified setzen (z.B. in frisch
/*N*/ 						//	geladenem Dokument durch initiales Recalc)
/*N*/ 
/*N*/ 						//	#72576# disable SetModified for readonly documents only
/*N*/ 
/*N*/ 						BOOL bEnabled = ( ((pShell && pShell->IsReadOnly()) ||
/*N*/ 											IsImportingXML()) &&
/*N*/ 											aIPObj->IsEnableSetModified() );
/*N*/ 						if (bEnabled)
/*N*/ 							aIPObj->EnableSetModified(FALSE);
/*N*/ 
/*N*/ 						SchDLL::Update( aIPObj, pMemChart, pWindow );
/*N*/ 						delete pMemChart;
/*N*/ 
/*N*/ 						// Dies veranlaesst Chart zum sofortigen Update
/*N*/ 						//SvData aEmpty;
/*N*/ 						//aIPObj->SendDataChanged( aEmpty );
/*N*/ 						aIPObj->SendViewChanged();
/*N*/ 						pObject->SendRepaintBroadcast();
/*N*/ 
/*N*/ 						if (bEnabled)
/*N*/ 							aIPObj->EnableSetModified(TRUE);
/*N*/ 
/*N*/ 						return;			// nicht weitersuchen
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			pObject = aIter.Next();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::UpdateChartRef( UpdateRefMode eUpdateRefMode,
/*N*/ 									USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 									USHORT nCol2, USHORT nRow2, USHORT nTab2,
/*N*/ 									short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	if (!pDrawLayer)
/*N*/ 		return;
/*N*/ 
/*N*/ 	USHORT nChartCount = pChartListenerCollection->GetCount();
/*N*/ 	for ( USHORT nIndex = 0; nIndex < nChartCount; nIndex++ )
/*N*/ 	{
/*?*/ 		ScChartListener* pChartListener =
/*?*/ 			(ScChartListener*) (pChartListenerCollection->At(nIndex));
/*?*/ 		ScRangeListRef aRLR( pChartListener->GetRangeList() );
/*?*/ 		ScRangeListRef aNewRLR( new ScRangeList );
/*?*/ 		BOOL bChanged = FALSE;
/*?*/ 		BOOL bDataChanged = FALSE;
/*?*/ 		for ( ScRangePtr pR = aRLR->First(); pR; pR = aRLR->Next() )
/*?*/ 		{
/*?*/ 			USHORT theCol1 = pR->aStart.Col();
/*?*/ 			USHORT theRow1 = pR->aStart.Row();
/*?*/ 			USHORT theTab1 = pR->aStart.Tab();
/*?*/ 			USHORT theCol2 = pR->aEnd.Col();
/*?*/ 			USHORT theRow2 = pR->aEnd.Row();
/*?*/ 			USHORT theTab2 = pR->aEnd.Tab();
/*?*/ 			ScRefUpdateRes eRes = ScRefUpdate::Update(
/*?*/ 				this, eUpdateRefMode,
/*?*/ 				nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
/*?*/ 				nDx,nDy,nDz,
/*?*/ 				theCol1,theRow1,theTab1,
/*?*/ 				theCol2,theRow2,theTab2 );
/*?*/ 			if ( eRes != UR_NOTHING )
/*?*/ 			{
/*?*/ 				bChanged = TRUE;
/*?*/ 				aNewRLR->Append( ScRange(
/*?*/ 					theCol1, theRow1, theTab1,
/*?*/ 					theCol2, theRow2, theTab2 ));
/*?*/ 				if ( eUpdateRefMode == URM_INSDEL
/*?*/ 					&& !bDataChanged
/*?*/ 					&& (eRes == UR_INVALID ||
/*?*/ 						((pR->aEnd.Col() - pR->aStart.Col()
/*?*/ 						!= theCol2 - theCol1)
/*?*/ 					|| (pR->aEnd.Row() - pR->aStart.Row()
/*?*/ 						!= theRow2 - theRow1)
/*?*/ 					|| (pR->aEnd.Tab() - pR->aStart.Tab()
/*?*/ 						!= theTab2 - theTab1))) )
/*?*/ 				{
/*?*/ 					bDataChanged = TRUE;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 				aNewRLR->Append( *pR );
/*?*/ 		}
/*?*/ 		if ( bChanged )
/*?*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if ( nDz != 0 )
/*?*/ 		}
/*N*/ 	}
/*N*/ }




/*N*/ BOOL ScDocument::HasData( USHORT nCol, USHORT nRow, USHORT nTab )
/*N*/ {
/*N*/ 	if (pTab[nTab])
/*N*/ 		return pTab[nTab]->HasData( nCol, nRow );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }

/*N*/ SchMemChart* ScDocument::FindChartData(const String& rName, BOOL bForModify)
/*N*/ {
/*N*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if (!pDrawLayer)
/*N*/ 	return NULL;							// nix
/*N*/ }


/*N*/ BOOL lcl_StringInCollection( const StrCollection* pColl, const String& rStr )
/*N*/ {
/*N*/ 	if ( !pColl )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	StrData aData( rStr );
/*N*/ 	USHORT nDummy;
/*N*/ 	return pColl->Search( &aData, nDummy );
/*N*/ }

/*N*/ void ScDocument::UpdateChartListenerCollection()
/*N*/ {
/*N*/ 	bChartListenerCollectionNeedsUpdate = FALSE;
/*N*/ 	if (!pDrawLayer)
/*N*/ 		return;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ScRange aRange;
/*N*/ 		// Range fuer Suche unwichtig
/*N*/ 		ScChartListener aCLSearcher( EMPTY_STRING, this, aRange );
/*N*/ 		for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
/*N*/ 		{
/*N*/ 			if (pTab[nTab])
/*N*/ 			{
/*N*/ 				SdrPage* pPage = pDrawLayer->GetPage(nTab);
/*N*/ 				DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 				SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
/*N*/ 				SdrObject* pObject = aIter.Next();
/*N*/ 				while (pObject)
/*N*/ 				{
/*N*/ 					if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
/*N*/ 					{
/*N*/ 						String aObjName = ((SdrOle2Obj*)pObject)->GetPersistName();
/*N*/ 						aCLSearcher.SetString( aObjName );
/*N*/ 						USHORT nIndex;
/*N*/ 						if ( pChartListenerCollection->Search( &aCLSearcher, nIndex ) )
/*N*/ 						{
/*N*/ 							((ScChartListener*) (pChartListenerCollection->
/*N*/ 								At( nIndex )))->SetUsed( TRUE );
/*N*/ 						}
/*N*/ 						else if ( lcl_StringInCollection( pOtherObjects, aObjName ) )
/*N*/ 						{
/*N*/ 							// non-chart OLE object -> don't touch
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 						    //	SchDLL::GetChartData always loads the chart dll,
/*N*/ 						    //	so SchModuleDummy::HasID must be tested before
/*N*/ 
/*N*/ 						    BOOL bIsChart = FALSE;						        
/*N*/ 						    USHORT nId;
/*N*/ 
/*N*/                             //  Ask the SvPersist for the InfoObject to find out
/*N*/                             //  whether it is a Chart. The old way with GetObjRef
/*N*/                             //  loads the object which takes too much unnecessary
/*N*/                             //  time
/*N*/                             SvInfoObject* pInfoObj = pShell->Find(aObjName);
/*N*/                             DBG_ASSERT(pInfoObj, "Why isn't here a SvInfoObject?");
/*N*/ 						    if ( pInfoObj &&
/*N*/ 							     ((nId = SchModuleDummy::HasID(pInfoObj->GetClassName()) ) != 0) )
/*N*/ 						    {
/*N*/                                 SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
/*N*/                                 DBG_ASSERT(aIPObj.Is(), "no SvInPlaceObject given");
/*N*/                                 if (aIPObj.Is())
/*N*/                                 {
/*N*/ 							        BOOL bSO6 = (nId >= SOFFICE_FILEFORMAT_60);
/*N*/ 							        SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
/*N*/ 							        // #84359# manually inserted OLE object
/*N*/ 							        // => no listener at ScAddress(0,0,0)
/*N*/ 							        // >=SO6: if no series set
/*N*/ 							        // < SO6: if no SomeData set
/*N*/ 							        if ( pChartData &&
/*N*/ 								        ((!bSO6 && pChartData->SomeData1().Len()) ||
/*N*/                                         (bSO6 && pChartData->GetChartRange().maRanges.size())) )
/*N*/ 							        {
/*N*/                                         if ( PastingDrawFromOtherDoc() )
/*N*/                                         {
/*?*/                                             // #89247# Remove series ranges from
/*?*/                                             // charts not originating from the
/*?*/                                             // same document, they become true OLE
/*?*/                                             // objects.
/*?*/                                             pChartData->SomeData1().Erase();
/*?*/                                             pChartData->SomeData2().Erase();
/*?*/                                             pChartData->SomeData3().Erase();
/*?*/                                             pChartData->SomeData4().Erase();
/*?*/                                             SchChartRange aChartRange;
/*?*/                                             pChartData->SetChartRange( aChartRange );
/*?*/                                             pChartData->SetReadOnly( FALSE );
/*?*/                                             SchDLL::Update( aIPObj, pChartData );
/*N*/                                         }
/*N*/                                         else
/*N*/                                         {
/*N*/                                             bIsChart = TRUE;
/*N*/ 
/*N*/                                             ScChartArray aArray( this, *pChartData );
/*N*/                                             ScChartListener* pCL = new ScChartListener(
/*N*/                                                 aObjName,
/*N*/                                                 this, aArray.GetRangeList() );
/*N*/                                             pChartListenerCollection->Insert( pCL );
/*N*/                                             pCL->StartListeningTo();
/*N*/                                             pCL->SetUsed( TRUE );
/*N*/ 
/*N*/                                             BOOL bForceSave = FALSE;
/*N*/ 
/*N*/                                             //  Set ReadOnly flag at MemChart, so Chart knows
/*N*/                                             //  about the external data in a freshly loaded document.
/*N*/                                             //  #73642# only if the chart really has external data
/*N*/                                             if ( aArray.IsValid() )
/*N*/                                             {
/*N*/                                                 pChartData->SetReadOnly( TRUE );
/*N*/ 
/*N*/                                                 //  #81525# re-create series ranges from old extra string
/*N*/                                                 //  if not set (after loading)
/*N*/                                                 if ( !bSO6 )
/*N*/                                                 {
/*N*/                                                     String aOldData3 = pChartData->SomeData3();
/*N*/                                                     aArray.SetExtraStrings( *pChartData );
/*N*/                                                     if ( aOldData3 != pChartData->SomeData3() )
/*N*/                                                     {
/*N*/                                                         //  #96148# ChartRange isn't saved in binary format anyway,
/*N*/                                                         //  but SomeData3 (sheet names) has to survive swapping out,
/*N*/                                                         //  or the chart can't be saved to 6.0 format.
/*N*/ 
/*N*/                                                         bForceSave = TRUE;
/*N*/                                                     }
/*N*/                                                 }
/*N*/                                             }
/*N*/ 
/*N*/     #if 1
/*N*/     // #74046# initially loaded charts need the number formatter standard precision
/*N*/                                             BOOL bEnabled = aIPObj->IsEnableSetModified();
/*N*/                                             if (bEnabled)
/*N*/                                                 aIPObj->EnableSetModified(FALSE);
/*N*/                                             pChartData->SetNumberFormatter( GetFormatTable() );
/*N*/                                             SchDLL::Update( aIPObj, pChartData );
/*N*/                                             //! pChartData got deleted, don't use it anymore
/*N*/                                             if (bEnabled)
/*N*/                                                 aIPObj->EnableSetModified(TRUE);
/*N*/     #ifdef DBG_UTIL
/*N*/     //                                          static BOOL bShown74046 = 0;
/*N*/     //                                          if ( !bShown74046 && SOFFICE_FILEFORMAT_NOW > SOFFICE_FILEFORMAT_50 )
/*N*/     //                                          {
/*N*/     //                                              bShown74046 = 1;
/*N*/     //                                              DBG_ERRORFILE( "on incompatible file format save number formatter standard precision in chart" );
/*N*/     //                                          }
/*N*/     #endif
/*N*/     #endif
/*N*/                                             if ( bForceSave )
/*N*/                                             {
/*N*/                                                 //  #96148# after adjusting the data that wasn't in the MemChart
/*N*/                                                 //  in a binary file (ChartRange etc.), the chart object has to be
/*N*/                                                 //  saved (within the open document, in transacted mode, so the
/*N*/                                                 //  original file isn't changed yet), so the changes are still
/*N*/                                                 //  there after the chart is swapped out and loaded again.
/*N*/                                                 //  The chart can't get the modified flag set, because then it
/*N*/                                                 //  wouldn't be swapped out at all. So it has to be saved manually
/*N*/                                                 //  here (which is unnecessary if the chart is modified before it
/*N*/                                                 //  it swapped out). At this point, we don't have to care about
/*N*/                                                 //  contents being lost when saving in old binary format, because
/*N*/                                                 //  the chart was just loaded from that format.
/*N*/ 
/*N*/                                                 aIPObj->DoSave();
/*N*/                                                 aIPObj->DoSaveCompleted();
/*N*/                                             }
/*N*/                                         }
/*N*/ 							        }
/*N*/                                 }
/*N*/ 						    }
/*N*/ 						    if (!bIsChart)
/*N*/ 						    {
/*N*/ 							    //	put into list of other ole objects, so the object doesn't have to
/*N*/ 							    //	be swapped in the next time UpdateChartListenerCollection is called
/*N*/ 							    //!	remove names when objects are no longer there?
/*N*/ 							    //	(object names aren't used again before reloading the document)
/*N*/ 
/*N*/ 							    if (!pOtherObjects)
/*N*/ 								    pOtherObjects = new StrCollection;
/*N*/ 							    pOtherObjects->Insert( new StrData( aObjName ) );
/*N*/ 						    }
/*N*/ 						}
/*N*/ 					}
/*N*/ 					pObject = aIter.Next();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		// alle nicht auf SetUsed gesetzten loeschen
/*N*/ 		pChartListenerCollection->FreeUnused();
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::AddOLEObjectToCollection(const String& rName)
/*N*/ {
/*N*/ 	if (!pOtherObjects)
/*N*/ 		pOtherObjects = new StrCollection;
/*N*/ 	pOtherObjects->Insert( new StrData( rName ) );
/*N*/ }



}
