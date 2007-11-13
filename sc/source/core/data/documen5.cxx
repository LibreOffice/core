/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documen5.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-13 15:26:14 $
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

#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>


#ifdef _MSC_VER
#pragma optimize("",off)
#endif

// INCLUDE ---------------------------------------------------------------

#include <sfx2/objsh.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>

//REMOVE    #ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
//REMOVE    #define SO2_DECL_SVINPLACEOBJECT_DEFINED
//REMOVE    SO2_DECL_REF(SvInPlaceObject)
//REMOVE    #endif

#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "chartlock.hxx"
#include "refupdat.hxx"
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#include <sot/exchange.hxx>

#include "miscuno.hxx"
#include "chart2uno.hxx"

using namespace ::com::sun::star;

// -----------------------------------------------------------------------

void lcl_GetChartParameters( const uno::Reference< chart2::XChartDocument >& xChartDoc,
            rtl::OUString& rRanges, chart::ChartDataRowSource& rDataRowSource,
            bool& rHasCategories, bool& rFirstCellAsLabel )
{
    rHasCategories = rFirstCellAsLabel = false;     // default if not in sequence

    uno::Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );

    uno::Reference< chart2::data::XDataSource > xDataSource = xReceiver->getUsedData();
    uno::Reference< chart2::data::XDataProvider > xProvider = xChartDoc->getDataProvider();

    if ( xProvider.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs( xProvider->detectArguments( xDataSource ) );

        const beans::PropertyValue* pPropArray = aArgs.getConstArray();
        long nPropCount = aArgs.getLength();
        for (long i = 0; i < nPropCount; i++)
        {
            const beans::PropertyValue& rProp = pPropArray[i];
            String aPropName(rProp.Name);

            if (aPropName.EqualsAscii( "CellRangeRepresentation" ))
                rProp.Value >>= rRanges;
            else if (aPropName.EqualsAscii( "DataRowSource" ))
                rDataRowSource = (chart::ChartDataRowSource)ScUnoHelpFunctions::GetEnumFromAny( rProp.Value );
            else if (aPropName.EqualsAscii( "HasCategories" ))
                rHasCategories = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if (aPropName.EqualsAscii( "FirstCellAsLabel" ))
                rFirstCellAsLabel = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        }
    }
}

void lcl_SetChartParameters( const uno::Reference< chart2::data::XDataReceiver >& xReceiver,
            const rtl::OUString& rRanges, chart::ChartDataRowSource eDataRowSource,
            bool bHasCategories, bool bFirstCellAsLabel )
{
    if ( xReceiver.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs( 4 );
        aArgs[0] = beans::PropertyValue(
            ::rtl::OUString::createFromAscii("CellRangeRepresentation"), -1,
            uno::makeAny( rRanges ), beans::PropertyState_DIRECT_VALUE );
        aArgs[1] = beans::PropertyValue(
            ::rtl::OUString::createFromAscii("HasCategories"), -1,
            uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE );
        aArgs[2] = beans::PropertyValue(
            ::rtl::OUString::createFromAscii("FirstCellAsLabel"), -1,
            uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE );
        aArgs[3] = beans::PropertyValue(
            ::rtl::OUString::createFromAscii("DataRowSource"), -1,
            uno::makeAny( eDataRowSource ), beans::PropertyState_DIRECT_VALUE );
        xReceiver->setArguments( aArgs );
    }
}

// update charts after loading old document

void ScDocument::UpdateAllCharts()
{
    if ( !pDrawLayer || !pShell )
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
                                ScRangeListRef aRanges = pChartObj->GetRangeList();
                                String sRangeStr;
                                aRanges->Format( sRangeStr, SCR_ABS_3D, this );

                                chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                                bool bHasCategories = pChartObj->HasRowHeaders();
                                bool bFirstCellAsLabel = pChartObj->HasColHeaders();

                                // Calc -> DataProvider
                                uno::Reference< chart2::data::XDataProvider > xDataProvider =
                                        new ScChart2DataProvider( this );
                                // Chart -> DataReceiver
                                uno::Reference< chart2::data::XDataReceiver > xReceiver;
                                uno::Reference< embed::XComponentSupplier > xCompSupp( xIPObj, uno::UNO_QUERY );
                                if( xCompSupp.is())
                                    xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
                                if( xReceiver.is())
                                {
                                    // connect
                                    xReceiver->attachDataProvider( xDataProvider );
                                    uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier(
                                            pShell->GetModel(), uno::UNO_QUERY );
                                    xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );

                                    lcl_SetChartParameters( xReceiver, sRangeStr, eDataRowSource,
                                                            bHasCategories, bFirstCellAsLabel );
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
            BOOL bAdd )
{
    ScRangeListRef aRLR( new ScRangeList );
    aRLR->Append( rNewArea );
    UpdateChartArea( rChartName, aRLR, bColHeaders, bRowHeaders, bAdd );
}

void ScDocument::GetOldChartParameters( const String& rName,
            ScRangeList& rRanges, BOOL& rColHeaders, BOOL& rRowHeaders )
{
    // used for undo of changing chart source area

    if (!pDrawLayer)
        return;

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
                    svt::EmbeddedObjectRef::TryRunningState( xIPObj );

                    uno::Reference< util::XCloseable > xComponent = xIPObj->getComponent();
                    uno::Reference< chart2::XChartDocument > xChartDoc( xComponent, uno::UNO_QUERY );
                    if ( xChartDoc.is() )
                    {
                        chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                        bool bHasCategories = false;
                        bool bFirstCellAsLabel = false;
                        rtl::OUString aRangesStr;
                        lcl_GetChartParameters( xChartDoc, aRangesStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                        rRanges.Parse( aRangesStr, this );
                        if ( eDataRowSource == chart::ChartDataRowSource_COLUMNS )
                        {
                            rRowHeaders = bHasCategories;
                            rColHeaders = bFirstCellAsLabel;
                        }
                        else
                        {
                            rColHeaders = bHasCategories;
                            rRowHeaders = bFirstCellAsLabel;
                        }
                    }
                }
                return;
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChartArea( const String& rChartName,
            const ScRangeListRef& rNewList, BOOL bColHeaders, BOOL bRowHeaders,
            BOOL bAdd )
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
                    svt::EmbeddedObjectRef::TryRunningState( xIPObj );

                    uno::Reference< util::XCloseable > xComponent = xIPObj->getComponent();
                    uno::Reference< chart2::XChartDocument > xChartDoc( xComponent, uno::UNO_QUERY );
                    uno::Reference< chart2::data::XDataReceiver > xReceiver( xComponent, uno::UNO_QUERY );
                    if ( xChartDoc.is() && xReceiver.is() )
                    {
                        ScRangeListRef aNewRanges;
                        chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                        bool bHasCategories = false;
                        bool bFirstCellAsLabel = false;
                        rtl::OUString aRangesStr;
                        lcl_GetChartParameters( xChartDoc, aRangesStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                        sal_Bool bInternalData = xChartDoc->hasInternalDataProvider();

                        if ( bAdd && !bInternalData )
                        {
                            // append to old ranges, keep other settings

                            aNewRanges = new ScRangeList;
                            aNewRanges->Parse( aRangesStr, this );

                            ULONG nAddCount = rNewList->Count();
                            for ( ULONG nAdd=0; nAdd<nAddCount; nAdd++ )
                                aNewRanges->Append( *rNewList->GetObject(nAdd) );
                        }
                        else
                        {
                            // directly use new ranges (only eDataRowSource is used from old settings)

                            if ( eDataRowSource == chart::ChartDataRowSource_COLUMNS )
                            {
                                bHasCategories = bRowHeaders;
                                bFirstCellAsLabel = bColHeaders;
                            }
                            else
                            {
                                bHasCategories = bColHeaders;
                                bFirstCellAsLabel = bRowHeaders;
                            }
                            aNewRanges = rNewList;
                        }

                        if ( bInternalData && pShell )
                        {
                            // Calc -> DataProvider
                            uno::Reference< chart2::data::XDataProvider > xDataProvider = new ScChart2DataProvider( this );
                            xReceiver->attachDataProvider( xDataProvider );
                            uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier(
                                    pShell->GetModel(), uno::UNO_QUERY );
                            xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );
                        }

                        String sRangeStr;
                        aNewRanges->Format( sRangeStr, SCR_ABS_3D, this );

                        lcl_SetChartParameters( xReceiver, sRangeStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                        pChartListenerCollection->ChangeListening( rChartName, aNewRanges );

                        // ((SdrOle2Obj*)pObject)->GetNewReplacement();
                        // pObject->ActionChanged();

                        return;         // nicht weitersuchen
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChart( const String& rChartName )
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
                //@todo?: maybe we need a notification
                //from the calc to the chart in future
                //that calc content has changed
                // ((SdrOle2Obj*)pObject)->GetNewReplacement();

                // Load the object and set modified

                uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if ( xIPObj.is() )
                {
                    svt::EmbeddedObjectRef::TryRunningState( xIPObj );

                    try
                    {
                        uno::Reference< util::XModifiable > xModif( xIPObj->getComponent(), uno::UNO_QUERY_THROW );
                        if( apTemporaryChartLock.get() )
                            apTemporaryChartLock->AlsoLockThisChart( uno::Reference< frame::XModel >( xModif, uno::UNO_QUERY ) );
                        xModif->setModified( sal_True );
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }

                // repaint

                pObject->ActionChanged();

                // After the update, chart keeps track of its own data source ranges,
                // the listener doesn't need to listen anymore.

                pChartListenerCollection->ChangeListening( rChartName, new ScRangeList );

                return;

                /* old chart:
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
                */
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::RestoreChartListener( const String& rName )
{
    // Read the data ranges from the chart object, and start listening to those ranges again
    // (called when a chart is saved, because then it might be swapped out and stop listening itself).

    uno::Reference< embed::XEmbeddedObject > xObject = FindOleObjectByName( rName );
    if ( xObject.is() )
    {
        uno::Reference< util::XCloseable > xComponent = xObject->getComponent();
        uno::Reference< chart2::XChartDocument > xChartDoc( xComponent, uno::UNO_QUERY );
        uno::Reference< chart2::data::XDataReceiver > xReceiver( xComponent, uno::UNO_QUERY );
        if ( xChartDoc.is() && xReceiver.is() && !xChartDoc->hasInternalDataProvider())
        {
            uno::Sequence<rtl::OUString> aRepresentations( xReceiver->getUsedRangeRepresentations() );
            ScRangeListRef aRanges = new ScRangeList;
            sal_Int32 nRangeCount = aRepresentations.getLength();
            for ( sal_Int32 i=0; i<nRangeCount; i++ )
            {
                ScRange aRange;
                if ( aRange.ParseAny( aRepresentations[i], this ) & SCA_VALID )
                    aRanges->Append( aRange );
            }

            pChartListenerCollection->ChangeListening( rName, aRanges );
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
#if 0
            if ( nDz != 0 )
            {   // #81844# sheet to be deleted or inserted or moved
                // => no valid sheet names for references right now
                pChartListener->ChangeListening( aNewRLR, bDataChanged );
                pChartListener->ScheduleSeriesRanges();
            }
            else
#endif
            {
//              SetChartRangeList( pChartListener->GetString(), aNewRLR );
//              pChartListener->ChangeListening( aNewRLR, bDataChanged );

                // Force the chart to be loaded now, so it registers itself for UNO events.
                // UNO broadcasts are done after UpdateChartRef, so the chart will get this
                // reference change.

                uno::Reference< embed::XEmbeddedObject > xIPObj = FindOleObjectByName( pChartListener->GetString() );
                svt::EmbeddedObjectRef::TryRunningState( xIPObj );

                // After the change, chart keeps track of its own data source ranges,
                // the listener doesn't need to listen anymore.

                pChartListener->ChangeListening( new ScRangeList, bDataChanged );
            }
        }
    }
}


void ScDocument::SetChartRangeList( const String& rChartName,
            const ScRangeListRef& rNewRangeListRef )
{
    // called from ChartListener

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
                    svt::EmbeddedObjectRef::TryRunningState( xIPObj );

                    uno::Reference< util::XCloseable > xComponent = xIPObj->getComponent();
                    uno::Reference< chart2::XChartDocument > xChartDoc( xComponent, uno::UNO_QUERY );
                    uno::Reference< chart2::data::XDataReceiver > xReceiver( xComponent, uno::UNO_QUERY );
                    if ( xChartDoc.is() && xReceiver.is() )
                    {
                        ScRangeListRef aNewRanges;
                        chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                        bool bHasCategories = false;
                        bool bFirstCellAsLabel = false;
                        rtl::OUString aRangesStr;
                        lcl_GetChartParameters( xChartDoc, aRangesStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                        String sRangeStr;
                        rNewRangeListRef->Format( sRangeStr, SCR_ABS_3D, this );

                        lcl_SetChartParameters( xReceiver, sRangeStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                        // don't modify pChartListenerCollection here, called from there
                        return;
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}


BOOL ScDocument::HasData( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    if (pTab[nTab])
        return pTab[nTab]->HasData( nCol, nRow );
    else
        return FALSE;
}

uno::Reference< embed::XEmbeddedObject >
    ScDocument::FindOleObjectByName( const String& rName )
{
    if (!pDrawLayer)
        return uno::Reference< embed::XEmbeddedObject >();

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
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                SdrOle2Obj * pOleObject ( dynamic_cast< SdrOle2Obj * >( pObject ));
                if( pOleObject &&
                    pOleObject->GetPersistName() == rName )
                {
                    return pOleObject->GetObjRef();
                }
            }
            pObject = aIter.Next();
        }
    }

    return uno::Reference< embed::XEmbeddedObject >();
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
                            bool bIsChart = false;

                            uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                            DBG_ASSERT( xIPObj.is(), "No embedded object is given!");
                            uno::Reference< ::com::sun::star::chart2::data::XDataReceiver > xReceiver;
                            uno::Reference< embed::XComponentSupplier > xCompSupp( xIPObj, uno::UNO_QUERY );
                            if( xCompSupp.is())
                                xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );

                            // if the object is a chart2::XDataReceiver, we must attach as XDataProvider
                            if( xReceiver.is() &&
                                !PastingDrawFromOtherDoc())
                            {
                                // NOTE: this currently does not work as we are
                                // unable to set the data. So a chart from the
                                // same document is treated like a chart with
                                // own data for the time being.
#if 0
                                // data provider
                                uno::Reference< chart2::data::XDataProvider > xDataProvider = new
                                    ScChart2DataProvider( this );
                                xReceiver->attachDataProvider( xDataProvider );
                                // number formats supplier
                                uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( pShell->GetModel(), uno::UNO_QUERY );
                                xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );
                                // data ?
                                // how to set?? Defined in XML-file, which is already loaded!!!
                                // => we have to do this stuff here, BEFORE the chart is actually loaded

                                bIsChart = true;
#endif
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



