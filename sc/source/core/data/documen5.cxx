/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documen5.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 10:52:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#ifndef _COM_SUN_STAR_EMBED_XCLASSIFIEDOBJECT_HPP_
#include <com/sun/star/embed/XClassifiedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif




#pragma optimize("",off)
#pragma optimize("q",off) // p-code off

// INCLUDE ---------------------------------------------------------------

#include <sfx2/objsh.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
//REMOVE    #include <so3/ipobj.hxx>

//REMOVE    #ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
//REMOVE    #define SO2_DECL_SVINPLACEOBJECT_DEFINED
//REMOVE    SO2_DECL_REF(SvInPlaceObject)
//REMOVE    #endif

#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "refupdat.hxx"
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#include <sot/exchange.hxx>

using namespace ::com::sun::star;

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

    for (SCTAB nTab=0; nTab<=MAXTAB; nTab++)
    {
        if (pTab[nTab])
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            DBG_ASSERT(pPage,"Page ?");

            ScRange aRange;
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
                {
                    uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                    if ( xIPObj.is() )
                    {
                        String aIPName = ((SdrOle2Obj*)pObject)->GetPersistName();

                        for (nPos=0; nPos<nDataCount; nPos++)
                        {
                            ScChartArray* pChartObj = (*pChartCollection)[nPos];
                            if (pChartObj->GetName() == aIPName)
                            {
                                if (bDoUpdate)
                                {
                                    SchMemChart* pMemChart = pChartObj->CreateMemChart();
                                    SchDLL::Update( xIPObj, pMemChart );
                                    ((SdrOle2Obj*)pObject)->GetNewReplacement();
                                    delete pMemChart;
                                }
                                else        // nur Position uebernehmen
                                {
                                    SchMemChart* pChartData = SchDLL::GetChartData(xIPObj);
                                    if (pChartData)
                                    {
                                        pChartObj->SetExtraStrings(*pChartData);
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

BOOL ScDocument::HasChartAtPoint( SCTAB nTab, const Point& rPos, String* pName )
{
    if (pDrawLayer && pTab[nTab])
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                 pObject->GetCurrentBoundRect().IsInside(rPos) )
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

    for (SCTAB nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
            {
                uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if ( xIPObj.is() )
                {
                    const SchMemChart* pChartData = SchDLL::GetChartData(xIPObj);
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

                        SchDLL::Update( xIPObj, pMemChart, pWindow );
                        ((SdrOle2Obj*)pObject)->GetNewReplacement();
                        delete pMemChart;

                        // Dies veranlaesst Chart zum sofortigen Update
                        //SvData aEmpty;
                        //aIPObj->SendDataChanged( aEmpty );

                        // the method below did nothing in SO7
//REMOVE                            aIPObj->SendViewChanged();

                        // repaint only
                        pObject->ActionChanged();
                        // pObject->SendRepaintBroadcast();

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

    for (SCTAB nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
            {
                uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if ( xIPObj.is() )
                {
                    const SchMemChart* pChartData = SchDLL::GetChartData(xIPObj);
                    if ( pChartData )
                    {
                        ScChartArray aArray( this, *pChartData );

                        SchMemChart* pMemChart = aArray.CreateMemChart();
                        ScChartArray::CopySettings( *pMemChart, *pChartData );

                        //  #57655# Chart-Update ohne geaenderte Einstellungen (MemChart)
                        //  soll das Dokument nicht auf modified setzen (z.B. in frisch
                        //  geladenem Dokument durch initiales Recalc)

                        //  #72576# disable SetModified for readonly documents only

                        sal_Bool bEnabled = ( (pShell && pShell->IsReadOnly()) || IsImportingXML() );
                        sal_Bool bModified = sal_False;
                        uno::Reference< util::XModifiable > xModif;

                        if ( bEnabled )
                        {
                            try
                            {
                                xModif =
                                    uno::Reference< util::XModifiable >( xIPObj->getComponent(), uno::UNO_QUERY_THROW );
                                bModified = xModif->isModified();
                            }
                            catch( uno::Exception& )
                            {
                                bEnabled = sal_False;
                            }
                        }

                        SchDLL::Update( xIPObj, pMemChart, pWindow );
                        ((SdrOle2Obj*)pObject)->GetNewReplacement();
                        delete pMemChart;

                        // Dies veranlaesst Chart zum sofortigen Update
                        //SvData aEmpty;
                        //aIPObj->SendDataChanged( aEmpty );

                        // the method below did nothing in SO7
//REMOVE                            aIPObj->SendViewChanged();

                        // redraw only
                        pObject->ActionChanged();
                        // pObject->SendRepaintBroadcast();

                        if ( bEnabled && xModif.is() )
                        {
                            try
                            {
                                if ( xModif->isModified() != bModified )
                                    xModif->setModified( bModified );
                            }
                            catch ( uno::Exception& )
                            {}
                        }

                        return;         // nicht weitersuchen
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
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
            SCCOL theCol1 = pR->aStart.Col();
            SCROW theRow1 = pR->aStart.Row();
            SCTAB theTab1 = pR->aStart.Tab();
            SCCOL theCol2 = pR->aEnd.Col();
            SCROW theRow2 = pR->aEnd.Row();
            SCTAB theTab2 = pR->aEnd.Tab();
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


BOOL ScDocument::HasData( SCCOL nCol, SCROW nRow, SCTAB nTab )
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

    sal_uInt16 nCount = pDrawLayer->GetPageCount();
    for (sal_uInt16 nTab=0; nTab<nCount; nTab++)
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
                uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if ( xIPObj.is() )
                {
                    // load object before setting modified
                    SchMemChart* pMemChart = SchDLL::GetChartData( xIPObj );
                    if (bForModify)
                    {
                        try
                        {
                            uno::Reference< util::XModifiable> xModif =
                                uno::Reference< util::XModifiable>(
                                        xIPObj->getComponent(),
                                        uno::UNO_QUERY_THROW);
                            xModif->setModified( sal_True);
                        }
                        catch( uno::Exception& )
                        {
                        }
                    }
                    return pMemChart;
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
        for (SCTAB nTab=0; nTab<=MAXTAB; nTab++)
        {
            if (pTab[nTab])
            {
                SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
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

//REMOVE                                //  Ask the SvPersist for the InfoObject to find out
//REMOVE                                //  whether it is a Chart. The old way with GetObjRef
//REMOVE                                //  loads the object which takes too much unnecessary
//REMOVE                                //  time
//REMOVE                                SvInfoObject* pInfoObj = pShell->Find(aObjName);
//REMOVE                                DBG_ASSERT(pInfoObj, "Why isn't here a SvInfoObject?");
//REMOVE                                if ( pInfoObj && ( nId = SotExchange::IsChart( pInfoObj->GetClassName() ) ) )
                            uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                            DBG_ASSERT( xIPObj.is(), "No embedded object is given!");
                            uno::Reference< embed::XClassifiedObject > xClassified( xIPObj, uno::UNO_QUERY );
                            DBG_ASSERT( xClassified.is(), "The object must implement XClassifiedObject!" );
                            if ( xClassified.is() )
                            {
                                SvGlobalName aObjectClassName;
                                try {
                                    aObjectClassName = SvGlobalName( xClassified->getClassID() );
                                } catch( uno::Exception& )
                                {
                                    // TODO: handle error
                                }

                                if ( nId = SotExchange::IsChart( aObjectClassName ) )
                                {
//REMOVE                                        BOOL bSO6 = (nId >= SOFFICE_FILEFORMAT_60);
                                    SchMemChart* pChartData = SchDLL::GetChartData(xIPObj);
                                    // #84359# manually inserted OLE object
                                    // => no listener at ScAddress(0,0,0)
                                    // >=SO6: if no series set
                                    // < SO6: if no SomeData set
                                    if ( pChartData &&
                                         pChartData->GetChartRange().maRanges.size() )
//REMOVE                                            ((!bSO6 && pChartData->SomeData1().Len()) ||
//REMOVE                                            (bSO6 && pChartData->GetChartRange().maRanges.size())) )
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
                                            SchDLL::Update( xIPObj, pChartData );
                                            ((SdrOle2Obj*)pObject)->GetNewReplacement();
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
//REMOVE                                                    if ( !bSO6 )
//REMOVE                                                    {
//REMOVE                                                        String aOldData3 = pChartData->SomeData3();
//REMOVE                                                        aArray.SetExtraStrings( *pChartData );
//REMOVE                                                        if ( aOldData3 != pChartData->SomeData3() )
//REMOVE                                                        {
//REMOVE                                                            //  #96148# ChartRange isn't saved in binary format anyway,
//REMOVE                                                            //  but SomeData3 (sheet names) has to survive swapping out,
//REMOVE                                                            //  or the chart can't be saved to 6.0 format.
//REMOVE
//REMOVE                                                            bForceSave = TRUE;
//REMOVE                                                        }
//REMOVE                                                    }
                                            }

    #if 1
    // #74046# initially loaded charts need the number formatter standard precision
                                            // TODO/LATER: probably should be handled somehow in the future
//REMOVE                                                BOOL bEnabled = aIPObj->IsEnableSetModified();
//REMOVE                                                if (bEnabled)
//REMOVE                                                    aIPObj->EnableSetModified(FALSE);
                                            pChartData->SetNumberFormatter( GetFormatTable() );
                                            SchDLL::Update( xIPObj, pChartData );
                                            ((SdrOle2Obj*)pObject)->GetNewReplacement();
                                            //! pChartData got deleted, don't use it anymore
//REMOVE                                                if (bEnabled)
//REMOVE                                                    aIPObj->EnableSetModified(TRUE);
    #ifndef PRODUCT
    //                                          static BOOL bShown74046 = 0;
    //                                          if ( !bShown74046 && SOFFICE_FILEFORMAT_NOW > SOFFICE_FILEFORMAT_50 )
    //                                          {
    //                                              bShown74046 = 1;
    //                                              DBG_ERRORFILE( "on incompatible file format save number formatter standard precision in chart" );
    //                                          }
    #endif
    #endif
                                                // the following saving was used only for SO5 formats
//REMOVE                                                if ( bForceSave )
//REMOVE                                                {
//REMOVE                                                    //  #96148# after adjusting the data that wasn't in the MemChart
//REMOVE                                                    //  in a binary file (ChartRange etc.), the chart object has to be
//REMOVE                                                    //  saved (within the open document, in transacted mode, so the
//REMOVE                                                    //  original file isn't changed yet), so the changes are still
//REMOVE                                                    //  there after the chart is swapped out and loaded again.
//REMOVE                                                    //  The chart can't get the modified flag set, because then it
//REMOVE                                                    //  wouldn't be swapped out at all. So it has to be saved manually
//REMOVE                                                    //  here (which is unnecessary if the chart is modified before it
//REMOVE                                                    //  it swapped out). At this point, we don't have to care about
//REMOVE                                                    //  contents being lost when saving in old binary format, because
//REMOVE                                                    //  the chart was just loaded from that format.
//REMOVE
//REMOVE                                                    uno::Reference< embed::XEmbedPersist > xPersist( xIOPbj, uno::UNO_QUERY );
//REMOVE                                                    try
//REMOVE                                                    {
//REMOVE                                                        xPersist->storeOwn();
//REMOVE                                                    }
//REMOVE                                                    catch( uno::Exception& )
//REMOVE                                                    {
//REMOVE                                                        // TODO/LATER: error handling
//REMOVE                                                    }
//REMOVE    //REMOVE                                                    aIPObj->DoSave();
//REMOVE    //REMOVE                                                    aIPObj->DoSaveCompleted();
//REMOVE                                                }
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



