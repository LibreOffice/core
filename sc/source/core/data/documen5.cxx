/*************************************************************************
 *
 *  $RCSfile: documen5.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:21:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#pragma optimize("",off)
#pragma optimize("q",off) // p-code off

// INCLUDE ---------------------------------------------------------------

#include <sfx2/objsh.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
#include <so3/ipobj.hxx>

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "refupdat.hxx"
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#include <sot/exchange.hxx>

// -----------------------------------------------------------------------

        // Charts aus altem Dokument updaten

void ScDocument::UpdateAllCharts(BOOL bDoUpdate)
{
    if (!pDrawLayer)
        return;

    USHORT nDataCount = pChartCollection->GetCount();
    if ( !nDataCount )
        return ;        // nothing to do

    USHORT nPos;

    for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
    {
        if (pTab[nTab])
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage,"Page ?");

            ScRange aRange;
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
                {
                    SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                    if (aIPObj.Is())
                    {
                        // String aIPName = aIPObj->GetName()->GetName();

                        SvInfoObject* pInfoObj = pShell->Find( aIPObj );
                        String aIPName;

                        if ( pInfoObj )
                            aIPName = pInfoObj->GetObjName();

                        for (nPos=0; nPos<nDataCount; nPos++)
                        {
                            ScChartArray* pChartObj = (*pChartCollection)[nPos];
                            if (pChartObj->GetName() == aIPName)
                            {
                                if (bDoUpdate)
                                {
                                    SchMemChart* pMemChart = pChartObj->CreateMemChart();
                                    SchDLL::Update( aIPObj, pMemChart );
                                    delete pMemChart;
                                }
                                else        // nur Position uebernehmen
                                {
                                    SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                                    if (pChartData)
                                    {
                                        pChartObj->SetExtraStrings(*pChartData);
//                                      aIPObj->SetModified( TRUE );
                                    }
                                }
                                ScChartListener* pCL = new ScChartListener(
                                    aIPName, this, pChartObj->GetRangeList() );
                                pChartListenerCollection->Insert( pCL );
                                pCL->StartListeningTo();
                            }
                        }
                    }
                }
                pObject = aIter.Next();
            }
        }
    }

    pChartCollection->FreeAll();
}

BOOL ScDocument::HasChartAtPoint( USHORT nTab, const Point& rPos, String* pName )
{
    if (pDrawLayer && pTab[nTab])
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                 pObject->GetBoundRect().IsInside(rPos) )
            {
                        // auch Chart-Objekte die nicht in der Collection sind

                if (IsChart(pObject))
                {
                    if (pName)
                        *pName = ((SdrOle2Obj*)pObject)->GetPersistName();
                    return TRUE;
                }
            }
            pObject = aIter.Next();
        }
    }

    if (pName)
        pName->Erase();
    return FALSE;                   // nix gefunden
}

void ScDocument::UpdateChartArea( const String& rChartName,
            const ScRange& rNewArea, BOOL bColHeaders, BOOL bRowHeaders,
            BOOL bAdd, Window* pWindow )
{
    ScRangeListRef aRLR( new ScRangeList );
    aRLR->Append( rNewArea );
    UpdateChartArea( rChartName, aRLR, bColHeaders, bRowHeaders, bAdd, pWindow );
}

void ScDocument::UpdateChartArea( const String& rChartName,
            const ScRangeListRef& rNewList, BOOL bColHeaders, BOOL bRowHeaders,
            BOOL bAdd, Window* pWindow )
{
    if (!pDrawLayer)
        return;

    for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if (aIPObj.Is())
                {
                    const SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                    if ( pChartData )
                    {
                        ScChartArray aArray( this, *pChartData );
                        if ( bAdd )
                        {
                            // bei bAdd werden Header-Angaben ignoriert
                            aArray.AddToRangeList( rNewList );
                        }
                        else
                        {
                            aArray.SetRangeList( rNewList );
                            aArray.SetHeaders( bColHeaders, bRowHeaders );
                        }
                        pChartListenerCollection->ChangeListening(
                            rChartName, aArray.GetRangeList() );


                        SchMemChart* pMemChart = aArray.CreateMemChart();
                        ScChartArray::CopySettings( *pMemChart, *pChartData );

                        SchDLL::Update( aIPObj, pMemChart, pWindow );
                        delete pMemChart;

                        // Dies veranlaesst Chart zum sofortigen Update
                        //SvData aEmpty;
                        //aIPObj->SendDataChanged( aEmpty );
                        aIPObj->SendViewChanged();
                        pObject->SendRepaintBroadcast();

                        return;         // nicht weitersuchen
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChart( const String& rChartName, Window* pWindow )
{
    if (!pDrawLayer || bInDtorClear)
        return;

    for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if (aIPObj.Is())
                {
                    const SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                    if ( pChartData )
                    {
                        ScChartArray aArray( this, *pChartData );

                        SchMemChart* pMemChart = aArray.CreateMemChart();
                        ScChartArray::CopySettings( *pMemChart, *pChartData );

                        //  #57655# Chart-Update ohne geaenderte Einstellungen (MemChart)
                        //  soll das Dokument nicht auf modified setzen (z.B. in frisch
                        //  geladenem Dokument durch initiales Recalc)

                        //  #72576# disable SetModified for readonly documents only

                        BOOL bEnabled = ( ((pShell && pShell->IsReadOnly()) ||
                                            IsImportingXML()) &&
                                            aIPObj->IsEnableSetModified() );
                        if (bEnabled)
                            aIPObj->EnableSetModified(FALSE);

                        SchDLL::Update( aIPObj, pMemChart, pWindow );
                        delete pMemChart;

                        // Dies veranlaesst Chart zum sofortigen Update
                        //SvData aEmpty;
                        //aIPObj->SendDataChanged( aEmpty );
                        aIPObj->SendViewChanged();
                        pObject->SendRepaintBroadcast();

                        if (bEnabled)
                            aIPObj->EnableSetModified(TRUE);

                        return;         // nicht weitersuchen
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz )
{
    if (!pDrawLayer)
        return;

    USHORT nChartCount = pChartListenerCollection->GetCount();
    for ( USHORT nIndex = 0; nIndex < nChartCount; nIndex++ )
    {
        ScChartListener* pChartListener =
            (ScChartListener*) (pChartListenerCollection->At(nIndex));
        ScRangeListRef aRLR( pChartListener->GetRangeList() );
        ScRangeListRef aNewRLR( new ScRangeList );
        BOOL bChanged = FALSE;
        BOOL bDataChanged = FALSE;
        for ( ScRangePtr pR = aRLR->First(); pR; pR = aRLR->Next() )
        {
            USHORT theCol1 = pR->aStart.Col();
            USHORT theRow1 = pR->aStart.Row();
            USHORT theTab1 = pR->aStart.Tab();
            USHORT theCol2 = pR->aEnd.Col();
            USHORT theRow2 = pR->aEnd.Row();
            USHORT theTab2 = pR->aEnd.Tab();
            ScRefUpdateRes eRes = ScRefUpdate::Update(
                this, eUpdateRefMode,
                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
                nDx,nDy,nDz,
                theCol1,theRow1,theTab1,
                theCol2,theRow2,theTab2 );
            if ( eRes != UR_NOTHING )
            {
                bChanged = TRUE;
                aNewRLR->Append( ScRange(
                    theCol1, theRow1, theTab1,
                    theCol2, theRow2, theTab2 ));
                if ( eUpdateRefMode == URM_INSDEL
                    && !bDataChanged
                    && (eRes == UR_INVALID ||
                        ((pR->aEnd.Col() - pR->aStart.Col()
                        != theCol2 - theCol1)
                    || (pR->aEnd.Row() - pR->aStart.Row()
                        != theRow2 - theRow1)
                    || (pR->aEnd.Tab() - pR->aStart.Tab()
                        != theTab2 - theTab1))) )
                {
                    bDataChanged = TRUE;
                }
            }
            else
                aNewRLR->Append( *pR );
        }
        if ( bChanged )
        {
            if ( nDz != 0 )
            {   // #81844# sheet to be deleted or inserted or moved
                // => no valid sheet names for references right now
                pChartListener->ChangeListening( aNewRLR, bDataChanged );
                pChartListener->ScheduleSeriesRanges();
            }
            else
            {
                SetChartRangeList( pChartListener->GetString(), aNewRLR );
                pChartListener->ChangeListening( aNewRLR, bDataChanged );
            }
        }
    }
}


void ScDocument::SetChartRangeList( const String& rChartName,
            const ScRangeListRef& rNewRangeListRef )
{
    SchMemChart* pChartData = FindChartData( rChartName, TRUE );
    if ( pChartData )
    {
        ScChartArray aArray( this, *pChartData );
        aArray.SetRangeList( rNewRangeListRef );
        aArray.SetExtraStrings( *pChartData );
    }
}


BOOL ScDocument::HasData( USHORT nCol, USHORT nRow, USHORT nTab )
{
    if (pTab[nTab])
        return pTab[nTab]->HasData( nCol, nRow );
    else
        return FALSE;
}

SchMemChart* ScDocument::FindChartData(const String& rName, BOOL bForModify)
{
    if (!pDrawLayer)
        return NULL;

    //  die Seiten hier vom Draw-Layer nehmen,
    //  weil sie evtl. nicht mit den Tabellen uebereinstimmen
    //  (z.B. Redo von Tabelle loeschen, Draw-Redo passiert vor DeleteTab).

    USHORT nCount = pDrawLayer->GetPageCount();
    for (USHORT nTab=0; nTab<nCount; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetPersistName() == rName )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if ( aIPObj.Is() )
                {
                    if (bForModify)
                        aIPObj->SetModified( TRUE );
                    return SchDLL::GetChartData( aIPObj );
                }
            }
            pObject = aIter.Next();
        }
    }

    return NULL;                            // nix
}


BOOL lcl_StringInCollection( const StrCollection* pColl, const String& rStr )
{
    if ( !pColl )
        return FALSE;

    StrData aData( rStr );
    USHORT nDummy;
    return pColl->Search( &aData, nDummy );
}

void ScDocument::UpdateChartListenerCollection()
{
    bChartListenerCollectionNeedsUpdate = FALSE;
    if (!pDrawLayer)
        return;
    else
    {
        ScRange aRange;
        // Range fuer Suche unwichtig
        ScChartListener aCLSearcher( EMPTY_STRING, this, aRange );
        for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
        {
            if (pTab[nTab])
            {
                SdrPage* pPage = pDrawLayer->GetPage(nTab);
                DBG_ASSERT(pPage,"Page ?");

                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
                    {
                        String aObjName = ((SdrOle2Obj*)pObject)->GetPersistName();
                        aCLSearcher.SetString( aObjName );
                        USHORT nIndex;
                        if ( pChartListenerCollection->Search( &aCLSearcher, nIndex ) )
                        {
                            ((ScChartListener*) (pChartListenerCollection->
                                At( nIndex )))->SetUsed( TRUE );
                        }
                        else if ( lcl_StringInCollection( pOtherObjects, aObjName ) )
                        {
                            // non-chart OLE object -> don't touch
                        }
                        else
                        {
                            //  SchDLL::GetChartData always loads the chart dll,
                            //  so SchModuleDummy::HasID must be tested before

                            BOOL bIsChart = FALSE;
                            USHORT nId;

                            //  Ask the SvPersist for the InfoObject to find out
                            //  whether it is a Chart. The old way with GetObjRef
                            //  loads the object which takes too much unnecessary
                            //  time
                            SvInfoObject* pInfoObj = pShell->Find(aObjName);
                            DBG_ASSERT(pInfoObj, "Why isn't here a SvInfoObject?");
                            if ( pInfoObj && ( nId = SotExchange::IsChart( pInfoObj->GetClassName() ) ) )
                            {
                                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                                DBG_ASSERT(aIPObj.Is(), "no SvInPlaceObject given");
                                if (aIPObj.Is())
                                {
                                    BOOL bSO6 = (nId >= SOFFICE_FILEFORMAT_60);
                                    SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                                    // #84359# manually inserted OLE object
                                    // => no listener at ScAddress(0,0,0)
                                    // >=SO6: if no series set
                                    // < SO6: if no SomeData set
                                    if ( pChartData &&
                                        ((!bSO6 && pChartData->SomeData1().Len()) ||
                                        (bSO6 && pChartData->GetChartRange().maRanges.size())) )
                                    {
                                        if ( PastingDrawFromOtherDoc() )
                                        {
                                            // #89247# Remove series ranges from
                                            // charts not originating from the
                                            // same document, they become true OLE
                                            // objects.
                                            pChartData->SomeData1().Erase();
                                            pChartData->SomeData2().Erase();
                                            pChartData->SomeData3().Erase();
                                            pChartData->SomeData4().Erase();
                                            SchChartRange aChartRange;
                                            pChartData->SetChartRange( aChartRange );
                                            pChartData->SetReadOnly( FALSE );
                                            SchDLL::Update( aIPObj, pChartData );
                                        }
                                        else
                                        {
                                            bIsChart = TRUE;

                                            ScChartArray aArray( this, *pChartData );
                                            ScChartListener* pCL = new ScChartListener(
                                                aObjName,
                                                this, aArray.GetRangeList() );
                                            pChartListenerCollection->Insert( pCL );
                                            pCL->StartListeningTo();
                                            pCL->SetUsed( TRUE );

                                            BOOL bForceSave = FALSE;

                                            //  Set ReadOnly flag at MemChart, so Chart knows
                                            //  about the external data in a freshly loaded document.
                                            //  #73642# only if the chart really has external data
                                            if ( aArray.IsValid() )
                                            {
                                                pChartData->SetReadOnly( TRUE );

                                                //  #81525# re-create series ranges from old extra string
                                                //  if not set (after loading)
                                                if ( !bSO6 )
                                                {
                                                    String aOldData3 = pChartData->SomeData3();
                                                    aArray.SetExtraStrings( *pChartData );
                                                    if ( aOldData3 != pChartData->SomeData3() )
                                                    {
                                                        //  #96148# ChartRange isn't saved in binary format anyway,
                                                        //  but SomeData3 (sheet names) has to survive swapping out,
                                                        //  or the chart can't be saved to 6.0 format.

                                                        bForceSave = TRUE;
                                                    }
                                                }
                                            }

    #if 1
    // #74046# initially loaded charts need the number formatter standard precision
                                            BOOL bEnabled = aIPObj->IsEnableSetModified();
                                            if (bEnabled)
                                                aIPObj->EnableSetModified(FALSE);
                                            pChartData->SetNumberFormatter( GetFormatTable() );
                                            SchDLL::Update( aIPObj, pChartData );
                                            //! pChartData got deleted, don't use it anymore
                                            if (bEnabled)
                                                aIPObj->EnableSetModified(TRUE);
    #ifndef PRODUCT
    //                                          static BOOL bShown74046 = 0;
    //                                          if ( !bShown74046 && SOFFICE_FILEFORMAT_NOW > SOFFICE_FILEFORMAT_50 )
    //                                          {
    //                                              bShown74046 = 1;
    //                                              DBG_ERRORFILE( "on incompatible file format save number formatter standard precision in chart" );
    //                                          }
    #endif
    #endif
                                            if ( bForceSave )
                                            {
                                                //  #96148# after adjusting the data that wasn't in the MemChart
                                                //  in a binary file (ChartRange etc.), the chart object has to be
                                                //  saved (within the open document, in transacted mode, so the
                                                //  original file isn't changed yet), so the changes are still
                                                //  there after the chart is swapped out and loaded again.
                                                //  The chart can't get the modified flag set, because then it
                                                //  wouldn't be swapped out at all. So it has to be saved manually
                                                //  here (which is unnecessary if the chart is modified before it
                                                //  it swapped out). At this point, we don't have to care about
                                                //  contents being lost when saving in old binary format, because
                                                //  the chart was just loaded from that format.

                                                aIPObj->DoSave();
                                                aIPObj->DoSaveCompleted();
                                            }
                                        }
                                    }
                                }
                            }
                            if (!bIsChart)
                            {
                                //  put into list of other ole objects, so the object doesn't have to
                                //  be swapped in the next time UpdateChartListenerCollection is called
                                //! remove names when objects are no longer there?
                                //  (object names aren't used again before reloading the document)

                                if (!pOtherObjects)
                                    pOtherObjects = new StrCollection;
                                pOtherObjects->Insert( new StrData( aObjName ) );
                            }
                        }
                    }
                    pObject = aIter.Next();
                }
            }
        }
        // alle nicht auf SetUsed gesetzten loeschen
        pChartListenerCollection->FreeUnused();
    }
}

void ScDocument::AddOLEObjectToCollection(const String& rName)
{
    if (!pOtherObjects)
        pOtherObjects = new StrCollection;
    pOtherObjects->Insert( new StrData( rName ) );
}



