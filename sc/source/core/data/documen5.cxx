/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <sfx2/objsh.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svtools/embedhlp.hxx>

#include "document.hxx"
#include "table.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "chartlock.hxx"
#include "refupdat.hxx"
#include <tools/globname.hxx>
#include <sot/exchange.hxx>

#include "miscuno.hxx"
#include "chart2uno.hxx"
#include "charthelper.hxx"

using namespace ::com::sun::star;

static void lcl_GetChartParameters( const uno::Reference< chart2::XChartDocument >& xChartDoc,
            OUString& rRanges, chart::ChartDataRowSource& rDataRowSource,
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
            OUString aPropName(rProp.Name);

            if ( aPropName == "CellRangeRepresentation" )
                rProp.Value >>= rRanges;
            else if ( aPropName == "DataRowSource" )
                rDataRowSource = (chart::ChartDataRowSource)ScUnoHelpFunctions::GetEnumFromAny( rProp.Value );
            else if ( aPropName == "HasCategories" )
                rHasCategories = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if ( aPropName == "FirstCellAsLabel" )
                rFirstCellAsLabel = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        }
    }
}

static void lcl_SetChartParameters( const uno::Reference< chart2::data::XDataReceiver >& xReceiver,
            const OUString& rRanges, chart::ChartDataRowSource eDataRowSource,
            bool bHasCategories, bool bFirstCellAsLabel )
{
    if ( xReceiver.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs( 4 );
        aArgs[0] = beans::PropertyValue(
            OUString("CellRangeRepresentation"), -1,
            uno::makeAny( rRanges ), beans::PropertyState_DIRECT_VALUE );
        aArgs[1] = beans::PropertyValue(
            OUString("HasCategories"), -1,
            uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE );
        aArgs[2] = beans::PropertyValue(
            OUString("FirstCellAsLabel"), -1,
            uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE );
        aArgs[3] = beans::PropertyValue(
            OUString("DataRowSource"), -1,
            uno::makeAny( eDataRowSource ), beans::PropertyState_DIRECT_VALUE );
        xReceiver->setArguments( aArgs );
    }
}

// update charts after loading old document

void ScDocument::UpdateAllCharts()
{
    if ( !pDrawLayer || !pShell )
        return;

    if (pChartCollection->empty())
        return ;        // nothing to do

    size_t nDataCount = pChartCollection->size();

    SCTAB nSize = static_cast<SCTAB>(maTabs.size());
    for (SCTAB nTab=0; nTab< nSize; nTab++)
    {
        if (maTabs[nTab])
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage,"Page ?");

            ScRange aRange;
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
                {
                    uno::Reference< embed::XEmbeddedObject > xIPObj = static_cast<SdrOle2Obj*>(pObject)->GetObjRef();
                    if ( xIPObj.is() )
                    {
                        OUString aIPName = static_cast<SdrOle2Obj*>(pObject)->GetPersistName();

                        for (size_t nPos = 0; nPos < nDataCount; ++nPos)
                        {
                            ScChartArray* pChartObj = (*pChartCollection)[nPos];
                            if (pChartObj->GetName() == aIPName)
                            {
                                ScRangeListRef aRanges = pChartObj->GetRangeList();
                                OUString sRangeStr;
                                aRanges->Format( sRangeStr, ScRefFlags::RANGE_ABS_3D, this, GetAddressConvention() );

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
                                pChartListenerCollection->insert( pCL );
                                pCL->StartListeningTo();
                            }
                        }
                    }
                }
                pObject = aIter.Next();
            }
        }
    }

    pChartCollection->clear();
}

bool ScDocument::HasChartAtPoint( SCTAB nTab, const Point& rPos, OUString& rName )
{
    if (pDrawLayer && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        OSL_ENSURE(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                 pObject->GetCurrentBoundRect().IsInside(rPos) )
            {
                // also Chart-Objects that are not in the Collection

                if (IsChart(pObject))
                {
                    rName = (static_cast<SdrOle2Obj*>(pObject))->GetPersistName();
                    return true;
                }
            }
            pObject = aIter.Next();
        }
    }

    rName.clear();
    return false;                   // nothing found
}

void ScDocument::UpdateChartArea( const OUString& rChartName,
            const ScRange& rNewArea, bool bColHeaders, bool bRowHeaders,
            bool bAdd )
{
    ScRangeListRef aRLR( new ScRangeList );
    aRLR->Append( rNewArea );
    UpdateChartArea( rChartName, aRLR, bColHeaders, bRowHeaders, bAdd );
}

uno::Reference< chart2::XChartDocument > ScDocument::GetChartByName( const OUString& rChartName )
{
    uno::Reference< chart2::XChartDocument > xReturn;

    if (pDrawLayer)
    {
        sal_uInt16 nCount = pDrawLayer->GetPageCount();
        SCTAB nSize = static_cast<SCTAB>(maTabs.size());
        for (sal_uInt16 nTab=0; nTab<nCount && nTab < nSize; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            OSL_ENSURE(pPage,"Page ?");

            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                        OUString(static_cast<SdrOle2Obj*>(pObject)->GetPersistName()) == rChartName )
                {
                    xReturn.set( ScChartHelper::GetChartFromSdrObject( pObject ) );
                    return xReturn;
                }
                pObject = aIter.Next();
            }
        }
    }
    return xReturn;
}
void ScDocument::GetChartRanges( const OUString& rChartName, ::std::vector< ScRangeList >& rRangesVector, ScDocument* pSheetNameDoc )
{
    rRangesVector.clear();
    uno::Reference< chart2::XChartDocument > xChartDoc( GetChartByName( rChartName ) );
    if ( xChartDoc.is() )
    {
        std::vector< OUString > aRangeStrings;
        ScChartHelper::GetChartRanges( xChartDoc, aRangeStrings );
        for(const OUString & aRangeString : aRangeStrings)
        {
            ScRangeList aRanges;
            aRanges.Parse( aRangeString, pSheetNameDoc, ScRefFlags::VALID, pSheetNameDoc->GetAddressConvention() );
            rRangesVector.push_back(aRanges);
        }
    }
}

void ScDocument::SetChartRanges( const OUString& rChartName, const ::std::vector< ScRangeList >& rRangesVector )
{
    uno::Reference< chart2::XChartDocument > xChartDoc( GetChartByName( rChartName ) );
    if ( xChartDoc.is() )
    {
        sal_Int32 nCount = static_cast<sal_Int32>( rRangesVector.size() );
        uno::Sequence< OUString > aRangeStrings(nCount);
        for( sal_Int32 nN=0; nN<nCount; nN++ )
        {
            ScRangeList aScRangeList( rRangesVector[nN] );
            OUString sRangeStr;
            aScRangeList.Format( sRangeStr, ScRefFlags::RANGE_ABS_3D, this, GetAddressConvention() );
            aRangeStrings[nN]=sRangeStr;
        }
        ScChartHelper::SetChartRanges( xChartDoc, aRangeStrings );
    }
}

void ScDocument::GetOldChartParameters( const OUString& rName,
            ScRangeList& rRanges, bool& rColHeaders, bool& rRowHeaders )
{
    // used for undo of changing chart source area

    if (!pDrawLayer)
        return;

    sal_uInt16 nCount = pDrawLayer->GetPageCount();
    for (sal_uInt16 nTab=0; nTab<nCount && nTab < static_cast<SCTAB>(maTabs.size()); nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        OSL_ENSURE(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    OUString(static_cast<SdrOle2Obj*>(pObject)->GetPersistName()) == rName )
            {
                uno::Reference< chart2::XChartDocument > xChartDoc( ScChartHelper::GetChartFromSdrObject( pObject ) );
                if ( xChartDoc.is() )
                {
                    chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                    bool bHasCategories = false;
                    bool bFirstCellAsLabel = false;
                    OUString aRangesStr;
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
                return;
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChartArea( const OUString& rChartName,
            const ScRangeListRef& rNewList, bool bColHeaders, bool bRowHeaders,
            bool bAdd )
{
    if (!pDrawLayer)
        return;

    for (SCTAB nTab=0; nTab< static_cast<SCTAB>(maTabs.size()) && maTabs[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        OSL_ENSURE(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    OUString(static_cast<SdrOle2Obj*>(pObject)->GetPersistName()) == rChartName )
            {
                uno::Reference< chart2::XChartDocument > xChartDoc( ScChartHelper::GetChartFromSdrObject( pObject ) );
                uno::Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );
                if ( xChartDoc.is() && xReceiver.is() )
                {
                    ScRangeListRef aNewRanges;
                    chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                    bool bHasCategories = false;
                    bool bFirstCellAsLabel = false;
                    OUString aRangesStr;
                    lcl_GetChartParameters( xChartDoc, aRangesStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                    bool bInternalData = xChartDoc->hasInternalDataProvider();

                    if ( bAdd && !bInternalData )
                    {
                        // append to old ranges, keep other settings

                        aNewRanges = new ScRangeList;
                        aNewRanges->Parse( aRangesStr, this );

                        for ( size_t nAdd = 0, nAddCount = rNewList->size(); nAdd < nAddCount; ++nAdd )
                            aNewRanges->Append( *(*rNewList)[nAdd] );
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

                    OUString sRangeStr;
                    aNewRanges->Format( sRangeStr, ScRefFlags::RANGE_ABS_3D, this, GetAddressConvention() );

                    lcl_SetChartParameters( xReceiver, sRangeStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                    pChartListenerCollection->ChangeListening( rChartName, aNewRanges );

                    return;         // do not search anymore
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChart( const OUString& rChartName )
{
    if (!pDrawLayer || bInDtorClear)
        return;
    uno::Reference< chart2::XChartDocument > xChartDoc( GetChartByName( rChartName ) );
    if( xChartDoc.is() )
    {
        try
        {
            uno::Reference< util::XModifiable > xModif( xChartDoc, uno::UNO_QUERY_THROW );
            if( apTemporaryChartLock.get() )
                apTemporaryChartLock->AlsoLockThisChart( uno::Reference< frame::XModel >( xModif, uno::UNO_QUERY ) );
            xModif->setModified( true );
        }
        catch ( uno::Exception& )
        {
        }
    }

    // After the update, chart keeps track of its own data source ranges,
    // the listener doesn't need to listen anymore, except the chart has
    // an internal data provider.
    if ( !( xChartDoc.is() && xChartDoc->hasInternalDataProvider() ) && pChartListenerCollection )
    {
        pChartListenerCollection->ChangeListening( rChartName, new ScRangeList );
    }
}

void ScDocument::RestoreChartListener( const OUString& rName )
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
            uno::Sequence<OUString> aRepresentations( xReceiver->getUsedRangeRepresentations() );
            ScRangeListRef aRanges = new ScRangeList;
            sal_Int32 nRangeCount = aRepresentations.getLength();
            for ( sal_Int32 i=0; i<nRangeCount; i++ )
            {
                ScRange aRange;
                ScAddress::Details aDetails(GetAddressConvention(), 0, 0);
                if ( aRange.ParseAny( aRepresentations[i], this, aDetails ) & ScRefFlags::VALID )
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

    ScChartListenerCollection::ListenersType& rListeners = pChartListenerCollection->getListeners();
    for (auto const& it : rListeners)
    {
        ScChartListener *const pChartListener = it.second.get();
        ScRangeListRef aRLR( pChartListener->GetRangeList() );
        ScRangeListRef aNewRLR( new ScRangeList );
        bool bChanged = false;
        bool bDataChanged = false;
        for ( size_t i = 0, nListSize = aRLR->size(); i < nListSize; ++i )
        {
            ScRange* pR = (*aRLR)[i];
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
                bChanged = true;
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
                    bDataChanged = true;
                }
            }
            else
                aNewRLR->Append( *pR );
        }
        if ( bChanged )
        {
            {
                // Force the chart to be loaded now, so it registers itself for UNO events.
                // UNO broadcasts are done after UpdateChartRef, so the chart will get this
                // reference change.

                uno::Reference<embed::XEmbeddedObject> xIPObj =
                    FindOleObjectByName(pChartListener->GetName());

                svt::EmbeddedObjectRef::TryRunningState( xIPObj );

                // After the change, chart keeps track of its own data source ranges,
                // the listener doesn't need to listen anymore, except the chart has
                // an internal data provider.
                bool bInternalDataProvider = false;
                if ( xIPObj.is() )
                {
                    try
                    {
                        uno::Reference< chart2::XChartDocument > xChartDoc( xIPObj->getComponent(), uno::UNO_QUERY_THROW );
                        bInternalDataProvider = xChartDoc->hasInternalDataProvider();
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }
                if ( bInternalDataProvider )
                {
                    pChartListener->ChangeListening( aNewRLR, bDataChanged );
                }
                else
                {
                    pChartListener->ChangeListening( new ScRangeList, bDataChanged );
                }
            }
        }
    }
}

void ScDocument::SetChartRangeList( const OUString& rChartName,
            const ScRangeListRef& rNewRangeListRef )
{
    // called from ChartListener

    if (!pDrawLayer)
        return;

    for (SCTAB nTab=0; nTab< static_cast<SCTAB>(maTabs.size()) && maTabs[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        OSL_ENSURE(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    OUString(static_cast<SdrOle2Obj*>(pObject)->GetPersistName()) == rChartName )
            {
                uno::Reference< chart2::XChartDocument > xChartDoc( ScChartHelper::GetChartFromSdrObject( pObject ) );
                uno::Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );
                if ( xChartDoc.is() && xReceiver.is() )
                {
                    ScRangeListRef aNewRanges;
                    chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                    bool bHasCategories = false;
                    bool bFirstCellAsLabel = false;
                    OUString aRangesStr;
                    lcl_GetChartParameters( xChartDoc, aRangesStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                    OUString sRangeStr;
                    rNewRangeListRef->Format( sRangeStr, ScRefFlags::RANGE_ABS_3D, this, GetAddressConvention() );

                    lcl_SetChartParameters( xReceiver, sRangeStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                    // don't modify pChartListenerCollection here, called from there
                    return;
                }
            }
            pObject = aIter.Next();
        }
    }
}

bool ScDocument::HasData( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->HasData( nCol, nRow );
    else
        return false;
}

uno::Reference< embed::XEmbeddedObject >
    ScDocument::FindOleObjectByName( const OUString& rName )
{
    if (!pDrawLayer)
        return uno::Reference< embed::XEmbeddedObject >();

    //  take the pages here from Draw-Layer, as they might not match with the tables
    //  (e.g. delete Redo of table; Draw-Redo happens before DeleteTab)

    sal_uInt16 nCount = pDrawLayer->GetPageCount();
    for (sal_uInt16 nTab=0; nTab<nCount; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        OSL_ENSURE(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                SdrOle2Obj * pOleObject ( dynamic_cast< SdrOle2Obj * >( pObject ));
                if( pOleObject &&
                    OUString(pOleObject->GetPersistName()) == rName )
                {
                    return pOleObject->GetObjRef();
                }
            }
            pObject = aIter.Next();
        }
    }

    return uno::Reference< embed::XEmbeddedObject >();
}

void ScDocument::UpdateChartListenerCollection()
{
    OSL_ASSERT(pChartListenerCollection);

    bChartListenerCollectionNeedsUpdate = false;
    if (!pDrawLayer)
        return;

    for (SCTAB nTab=0; nTab< static_cast<SCTAB>(maTabs.size()); nTab++)
    {
        if (!maTabs[nTab])
            continue;

        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        OSL_ENSURE(pPage,"Page ?");

        if (!pPage)
            continue;

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        ScChartListenerCollection::StringSetType& rNonOleObjects =
            pChartListenerCollection->getNonOleObjectNames();

        for (SdrObject* pObject = aIter.Next(); pObject; pObject = aIter.Next())
        {
            if ( pObject->GetObjIdentifier() != OBJ_OLE2 )
                continue;

            OUString aObjName = static_cast<SdrOle2Obj*>(pObject)->GetPersistName();
            ScChartListener* pListener = pChartListenerCollection->findByName(aObjName);

            if (pListener)
                pListener->SetUsed(true);
            else if (rNonOleObjects.count(aObjName) > 0)
            {
                // non-chart OLE object -> don't touch
            }
            else
            {
                uno::Reference< embed::XEmbeddedObject > xIPObj = static_cast<SdrOle2Obj*>(pObject)->GetObjRef();
                OSL_ENSURE( xIPObj.is(), "No embedded object is given!");
                uno::Reference< css::chart2::data::XDataReceiver > xReceiver;
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

                    // data provider
                    // number formats supplier

                    // data ?
                    // how to set?? Defined in XML-file, which is already loaded!!!
                    // => we have to do this stuff here, BEFORE the chart is actually loaded
                }

                //  put into list of other ole objects, so the object doesn't have to
                //  be swapped in the next time UpdateChartListenerCollection is called
                //TODO: remove names when objects are no longer there?
                //  (object names aren't used again before reloading the document)

                rNonOleObjects.insert(aObjName);
            }
        }
    }
    // delete all that are not set SetUsed
    pChartListenerCollection->FreeUnused();
}

void ScDocument::AddOLEObjectToCollection(const OUString& rName)
{
    OSL_ASSERT(pChartListenerCollection);
    ScChartListenerCollection::StringSetType& rNonOleObjects =
        pChartListenerCollection->getNonOleObjectNames();

    rNonOleObjects.insert(rName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
