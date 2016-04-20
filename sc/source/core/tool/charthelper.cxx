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

#include "charthelper.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "rangelst.hxx"
#include "chartlis.hxx"
#include "docuno.hxx"

#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svtools/embedhlp.hxx>

#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/util/XModifiable.hpp>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;

namespace
{

sal_uInt16 lcl_DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc, bool bAllCharts )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return 0;

    sal_uInt16 nFound = 0;

    sal_uInt16 nPageCount = pModel->GetPageCount();
    for (sal_uInt16 nPageNo=0; nPageNo<nPageCount; nPageNo++)
    {
        SdrPage* pPage = pModel->GetPage(nPageNo);
        OSL_ENSURE(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 && ScDocument::IsChart( pObject ) )
            {
                OUString aName = static_cast<SdrOle2Obj*>(pObject)->GetPersistName();
                bool bHit = true;
                if ( !bAllCharts )
                {
                    ScRangeList aRanges;
                    bool bColHeaders = false;
                    bool bRowHeaders = false;
                    pDoc->GetOldChartParameters( aName, aRanges, bColHeaders, bRowHeaders );
                    bHit = aRanges.In( rPos );
                }
                if ( bHit )
                {
                    pDoc->UpdateChart( aName );
                    ++nFound;
                }
            }
            pObject = aIter.Next();
        }
    }
    return nFound;
}

bool lcl_AdjustRanges( ScRangeList& rRanges, SCTAB nSourceTab, SCTAB nDestTab, SCTAB nTabCount )
{
    //TODO: if multiple sheets are copied, update references into the other copied sheets?

    bool bChanged = false;

    for ( size_t i=0, nCount = rRanges.size(); i < nCount; i++ )
    {
        ScRange* pRange = rRanges[ i ];
        if ( pRange->aStart.Tab() == nSourceTab && pRange->aEnd.Tab() == nSourceTab )
        {
            pRange->aStart.SetTab( nDestTab );
            pRange->aEnd.SetTab( nDestTab );
            bChanged = true;
        }
        if ( pRange->aStart.Tab() >= nTabCount )
        {
            pRange->aStart.SetTab( nTabCount > 0 ? ( nTabCount - 1 ) : 0 );
            bChanged = true;
        }
        if ( pRange->aEnd.Tab() >= nTabCount )
        {
            pRange->aEnd.SetTab( nTabCount > 0 ? ( nTabCount - 1 ) : 0 );
            bChanged = true;
        }
    }

    return bChanged;
}

}//end anonymous namespace

// ScChartHelper
//static
sal_uInt16 ScChartHelper::DoUpdateAllCharts( ScDocument* pDoc )
{
    return lcl_DoUpdateCharts( ScAddress(), pDoc, true );
}

void ScChartHelper::AdjustRangesOfChartsOnDestinationPage( ScDocument* pSrcDoc, ScDocument* pDestDoc, const SCTAB nSrcTab, const SCTAB nDestTab )
{
    if( !pSrcDoc || !pDestDoc )
        return;
    ScDrawLayer* pDrawLayer = pDestDoc->GetDrawLayer();
    if( !pDrawLayer )
        return;

    SdrPage* pDestPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nDestTab));
    if( pDestPage )
    {
        SdrObjListIter aIter( *pDestPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while( pObject )
        {
            if( pObject->GetObjIdentifier() == OBJ_OLE2 && static_cast<SdrOle2Obj*>(pObject)->IsChart() )
            {
                OUString aChartName = static_cast<SdrOle2Obj*>(pObject)->GetPersistName();

                Reference< chart2::XChartDocument > xChartDoc( pDestDoc->GetChartByName( aChartName ) );
                Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );
                if( xChartDoc.is() && xReceiver.is() && !xChartDoc->hasInternalDataProvider() )
                {
                    ::std::vector< ScRangeList > aRangesVector;
                    pDestDoc->GetChartRanges( aChartName, aRangesVector, pSrcDoc );

                    ::std::vector< ScRangeList >::iterator aIt( aRangesVector.begin() );
                    for( ; aIt!=aRangesVector.end(); ++aIt )
                    {
                        ScRangeList& rScRangeList( *aIt );
                        lcl_AdjustRanges( rScRangeList, nSrcTab, nDestTab, pDestDoc->GetTableCount() );
                    }
                    pDestDoc->SetChartRanges( aChartName, aRangesVector );
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScChartHelper::UpdateChartsOnDestinationPage( ScDocument* pDestDoc, const SCTAB nDestTab )
{
    if( !pDestDoc )
        return;
    ScDrawLayer* pDrawLayer = pDestDoc->GetDrawLayer();
    if( !pDrawLayer )
        return;

    SdrPage* pDestPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nDestTab));
    if( pDestPage )
    {
        SdrObjListIter aIter( *pDestPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while( pObject )
        {
            if( pObject->GetObjIdentifier() == OBJ_OLE2 && static_cast<SdrOle2Obj*>(pObject)->IsChart() )
            {
                OUString aChartName = static_cast<SdrOle2Obj*>(pObject)->GetPersistName();
                Reference< chart2::XChartDocument > xChartDoc( pDestDoc->GetChartByName( aChartName ) );
                Reference< util::XModifiable > xModif(xChartDoc, uno::UNO_QUERY_THROW);
                xModif->setModified( true);
            }
            pObject = aIter.Next();
        }
    }
}

uno::Reference< chart2::XChartDocument > ScChartHelper::GetChartFromSdrObject( SdrObject* pObject )
{
    uno::Reference< chart2::XChartDocument > xReturn;
    if( pObject )
    {
        if( pObject->GetObjIdentifier() == OBJ_OLE2 && static_cast<SdrOle2Obj*>(pObject)->IsChart() )
        {
            uno::Reference< embed::XEmbeddedObject > xIPObj = static_cast<SdrOle2Obj*>(pObject)->GetObjRef();
            if( xIPObj.is() )
            {
                svt::EmbeddedObjectRef::TryRunningState( xIPObj );
                uno::Reference< util::XCloseable > xComponent = xIPObj->getComponent();
                xReturn.set( uno::Reference< chart2::XChartDocument >( xComponent, uno::UNO_QUERY ) );
            }
        }
    }
    return xReturn;
}

void ScChartHelper::GetChartRanges( const uno::Reference< chart2::XChartDocument >& xChartDoc,
            uno::Sequence< OUString >& rRanges )
{
    rRanges.realloc(0);
    uno::Reference< chart2::data::XDataSource > xDataSource( xChartDoc, uno::UNO_QUERY );
    if( !xDataSource.is() )
        return;

    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aLabeledDataSequences( xDataSource->getDataSequences() );
    rRanges.realloc(2*aLabeledDataSequences.getLength());
    sal_Int32 nRealCount=0;
    for( sal_Int32 nN=0;nN<aLabeledDataSequences.getLength();nN++)
    {
        uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSequence( aLabeledDataSequences[nN] );
        if(!xLabeledSequence.is())
            continue;
        uno::Reference< chart2::data::XDataSequence > xLabel( xLabeledSequence->getLabel());
        uno::Reference< chart2::data::XDataSequence > xValues( xLabeledSequence->getValues());

        if( xLabel.is())
            rRanges[nRealCount++] = xLabel->getSourceRangeRepresentation();
        if( xValues.is())
            rRanges[nRealCount++] = xValues->getSourceRangeRepresentation();
    }
    rRanges.realloc(nRealCount);
}

void ScChartHelper::SetChartRanges( const uno::Reference< chart2::XChartDocument >& xChartDoc,
            const uno::Sequence< OUString >& rRanges )
{
    uno::Reference< chart2::data::XDataSource > xDataSource( xChartDoc, uno::UNO_QUERY );
    if( !xDataSource.is() )
        return;
    uno::Reference< chart2::data::XDataProvider > xDataProvider = xChartDoc->getDataProvider();
    if( !xDataProvider.is() )
        return;

    uno::Reference< frame::XModel > xModel( xChartDoc, uno::UNO_QUERY );
    if( xModel.is() )
        xModel->lockControllers();

    try
    {
        OUString aPropertyNameRole( "Role" );

        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aLabeledDataSequences( xDataSource->getDataSequences() );
        sal_Int32 nRange=0;
        for( sal_Int32 nN=0; (nN<aLabeledDataSequences.getLength()) && (nRange<rRanges.getLength()); nN++ )
        {
            uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSequence( aLabeledDataSequences[nN] );
            if(!xLabeledSequence.is())
                continue;
            uno::Reference< beans::XPropertySet > xLabel( xLabeledSequence->getLabel(), uno::UNO_QUERY );
            uno::Reference< beans::XPropertySet > xValues( xLabeledSequence->getValues(), uno::UNO_QUERY );

            if( xLabel.is())
            {
                uno::Reference< chart2::data::XDataSequence > xNewSeq(
                    xDataProvider->createDataSequenceByRangeRepresentation( rRanges[nRange++] ));

                uno::Reference< beans::XPropertySet > xNewProps( xNewSeq, uno::UNO_QUERY );
                if( xNewProps.is() )
                    xNewProps->setPropertyValue( aPropertyNameRole, xLabel->getPropertyValue( aPropertyNameRole ) );

                xLabeledSequence->setLabel( xNewSeq );
            }

            if( !(nRange<rRanges.getLength()) )
                break;

            if( xValues.is())
            {
                uno::Reference< chart2::data::XDataSequence > xNewSeq(
                    xDataProvider->createDataSequenceByRangeRepresentation( rRanges[nRange++] ));

                uno::Reference< beans::XPropertySet > xNewProps( xNewSeq, uno::UNO_QUERY );
                if( xNewProps.is() )
                    xNewProps->setPropertyValue( aPropertyNameRole, xValues->getPropertyValue( aPropertyNameRole ) );

                xLabeledSequence->setValues( xNewSeq );
            }
        }
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL("Exception in ScChartHelper::SetChartRanges - invalid range string?");
    }

    if( xModel.is() )
        xModel->unlockControllers();
}

void ScChartHelper::AddRangesIfProtectedChart( ScRangeListVector& rRangesVector, ScDocument* pDocument, SdrObject* pObject )
{
    if ( pDocument && pObject && ( pObject->GetObjIdentifier() == OBJ_OLE2 ) )
    {
        SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >( pObject );
        if ( pSdrOle2Obj && pSdrOle2Obj->IsChart() )
        {
            uno::Reference< embed::XEmbeddedObject > xEmbeddedObj = pSdrOle2Obj->GetObjRef();
            if ( xEmbeddedObj.is() )
            {
                bool bDisableDataTableDialog = false;
                sal_Int32 nOldState = xEmbeddedObj->getCurrentState();
                svt::EmbeddedObjectRef::TryRunningState( xEmbeddedObj );
                uno::Reference< beans::XPropertySet > xProps( xEmbeddedObj->getComponent(), uno::UNO_QUERY );
                if ( xProps.is() &&
                     ( xProps->getPropertyValue("DisableDataTableDialog") >>= bDisableDataTableDialog ) &&
                     bDisableDataTableDialog )
                {
                    ScChartListenerCollection* pCollection = pDocument->GetChartListenerCollection();
                    if (pCollection)
                    {
                        OUString aChartName = pSdrOle2Obj->GetPersistName();
                        const ScChartListener* pListener = pCollection->findByName(aChartName);
                        if (pListener)
                        {
                            const ScRangeListRef& rRangeList = pListener->GetRangeList();
                            if ( rRangeList.Is() )
                            {
                                rRangesVector.push_back( *rRangeList );
                            }
                        }
                    }
                }
                if ( xEmbeddedObj->getCurrentState() != nOldState )
                {
                    xEmbeddedObj->changeState( nOldState );
                }
            }
        }
    }
}

void ScChartHelper::FillProtectedChartRangesVector( ScRangeListVector& rRangesVector, ScDocument* pDocument, SdrPage* pPage )
{
    if ( pDocument && pPage )
    {
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while ( pObject )
        {
            AddRangesIfProtectedChart( rRangesVector, pDocument, pObject );
            pObject = aIter.Next();
        }
    }
}

void ScChartHelper::GetChartNames( ::std::vector< OUString >& rChartNames, SdrPage* pPage )
{
    if ( pPage )
    {
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while ( pObject )
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >( pObject );
                if ( pSdrOle2Obj && pSdrOle2Obj->IsChart() )
                {
                    rChartNames.push_back( pSdrOle2Obj->GetPersistName() );
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScChartHelper::CreateProtectedChartListenersAndNotify( ScDocument* pDoc, SdrPage* pPage, ScModelObj* pModelObj, SCTAB nTab,
    const ScRangeListVector& rRangesVector, const ::std::vector< OUString >& rExcludedChartNames, bool bSameDoc )
{
    if ( pDoc && pPage && pModelObj )
    {
        size_t nRangeListCount = rRangesVector.size();
        size_t nRangeList = 0;
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while ( pObject )
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >( pObject );
                if ( pSdrOle2Obj && pSdrOle2Obj->IsChart() )
                {
                    OUString aChartName = pSdrOle2Obj->GetPersistName();
                    ::std::vector< OUString >::const_iterator aEnd = rExcludedChartNames.end();
                    ::std::vector< OUString >::const_iterator aFound = ::std::find( rExcludedChartNames.begin(), aEnd, aChartName );
                    if ( aFound == aEnd )
                    {
                        uno::Reference< embed::XEmbeddedObject > xEmbeddedObj = pSdrOle2Obj->GetObjRef();
                        if ( xEmbeddedObj.is() && ( nRangeList < nRangeListCount ) )
                        {
                            bool bDisableDataTableDialog = false;
                            svt::EmbeddedObjectRef::TryRunningState( xEmbeddedObj );
                            uno::Reference< beans::XPropertySet > xProps( xEmbeddedObj->getComponent(), uno::UNO_QUERY );
                            if ( xProps.is() &&
                                 ( xProps->getPropertyValue("DisableDataTableDialog") >>= bDisableDataTableDialog ) &&
                                 bDisableDataTableDialog )
                            {
                                if ( bSameDoc )
                                {
                                    ScChartListenerCollection* pCollection = pDoc->GetChartListenerCollection();
                                    if (pCollection && !pCollection->findByName(aChartName))
                                    {
                                        ScRangeList aRangeList( rRangesVector[ nRangeList++ ] );
                                        ScRangeListRef rRangeList( new ScRangeList( aRangeList ) );
                                        ScChartListener* pChartListener = new ScChartListener( aChartName, pDoc, rRangeList );
                                        pCollection->insert( pChartListener );
                                        pChartListener->StartListeningTo();
                                    }
                                }
                                else
                                {
                                    xProps->setPropertyValue("DisableDataTableDialog",
                                        uno::makeAny( false ) );
                                    xProps->setPropertyValue("DisableComplexChartTypes",
                                        uno::makeAny( false ) );
                                }
                            }
                        }

                        if ( pModelObj && pModelObj->HasChangesListeners() )
                        {
                            Rectangle aRectangle = pSdrOle2Obj->GetSnapRect();
                            ScRange aRange( pDoc->GetRange( nTab, aRectangle ) );
                            ScRangeList aChangeRanges;
                            aChangeRanges.Append( aRange );

                            uno::Sequence< beans::PropertyValue > aProperties( 1 );
                            aProperties[ 0 ].Name = "Name";
                            aProperties[ 0 ].Value <<= aChartName;

                            pModelObj->NotifyChanges( "insert-chart", aChangeRanges, aProperties );
                        }
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
