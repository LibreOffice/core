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

#include "com/sun/star/beans/XPropertyAccess.hpp"
#include "com/sun/star/container/XContainerQuery.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/document/AmbigousFilterRequest.hpp"
#include "com/sun/star/document/FilterOptionsRequest.hpp"
#include "com/sun/star/document/NoSuchFilterRequest.hpp"
#include "com/sun/star/document/XImporter.hpp"
#include "com/sun/star/document/XInteractionFilterOptions.hpp"
#include "com/sun/star/document/XInteractionFilterSelect.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/ui/dialogs/XExecutableDialog.hpp"

#include "osl/mutex.hxx"
#include "comphelper/sequenceashashmap.hxx"
#include "vcl/svapp.hxx"

#include "getcontinuations.hxx"
#include "fltdlg.hxx"

#include "iahndl.hxx"

using namespace com::sun::star;

namespace {

void
executeFilterDialog(
    Window                    * pParent ,
    OUString       const & rURL    ,
    uui::FilterNameList const & rFilters,
    OUString             & rFilter )
       SAL_THROW((uno::RuntimeException))
{
    try
    {
        SolarMutexGuard aGuard;

        std::auto_ptr< ResMgr > xManager(ResMgr::CreateResMgr("uui"));

        std::auto_ptr< uui::FilterDialog > xDialog(
            new uui::FilterDialog(pParent, xManager.get()));

        xDialog->SetURL(rURL);
        xDialog->ChangeFilters(&rFilters);

        uui::FilterNameListPtr pSelected = rFilters.end();
        if( xDialog->AskForFilter( pSelected ) )
        {
            rFilter = pSelected->sInternal;
        }
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
            OUString("out of memory"),
            uno::Reference< uno::XInterface >());
    }
}

void
handleNoSuchFilterRequest_(
    Window * pParent,
    uno::Reference< uno::XComponentContext > const & xContext,
    document::NoSuchFilterRequest const & rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
            rContinuations )
    SAL_THROW((uno::RuntimeException))
{
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< document::XInteractionFilterSelect > xFilterTransport;
    getContinuations(rContinuations, &xAbort, &xFilterTransport);

    // check necessary resources - if they don't exist - abort or
    // break this operation
    if (!xAbort.is())
        return;

    if (!xFilterTransport.is())
    {
        xAbort->select();
        return;
    }

    uno::Reference< container::XContainerQuery > xFilterContainer;
    try
    {
        xFilterContainer.set( xContext->getServiceManager()->createInstanceWithContext(
                                  OUString( "com.sun.star.document.FilterFactory"), xContext ),
                              uno::UNO_QUERY );
    }
    catch ( uno::Exception const & )
    {
    }

    if (!xFilterContainer.is())
    {
        xAbort->select();
        return;
    }

    uui::FilterNameList lNames;

    // Note: We look for all filters here which match the following criteria:
    //          - they are import filters as minimum (of course they can
    //            support export too)
    //          - we don't show any filter which are flagged as "don't show it
    //            at the UI" or "they are not installed"
    //          - we ignore filters, which have not set any valid
    //            DocumentService (e.g. our pure graphic filters)
    //          - we show it sorted by her UIName's
    //          - We don't use the order flag or prefer default filters.
    //            (Because this list shows all filters and the user should
    //            find his filter very easy by his UIName ...)
    //          - We use "_query_all" here ... but we filter graphic filters
    //            out by using DocumentService property later!
    uno::Reference< container::XEnumeration > xFilters
        = xFilterContainer->createSubSetEnumerationByQuery(
            OUString( "_query_all:sort_prop=uiname:iflags=1:eflags=143360"));
    while (xFilters->hasMoreElements())
    {
        try
        {
            ::comphelper::SequenceAsHashMap lProps(xFilters->nextElement());
            uui::FilterNamePair             aPair;

            aPair.sInternal = lProps.getUnpackedValueOrDefault(
                OUString("Name"), OUString());
            aPair.sUI       = lProps.getUnpackedValueOrDefault(
                 OUString("UIName"), OUString());
            if ( (!aPair.sInternal.Len()) || (!aPair.sUI.Len() ) )
            {
               continue;
            }
            lNames.push_back( aPair );
        }
        catch(const uno::RuntimeException&)
        {
            throw;
        }
        catch(const uno::Exception&)
        {
            continue;
        }
    }

    // no list available for showing
    // -> abort operation
    if (lNames.size()<1)
    {
        xAbort->select();
        return;
    }

    // let the user select the right filter
    OUString sSelectedFilter;
    executeFilterDialog( pParent,
                         rRequest.URL,
                         lNames,
                         sSelectedFilter );

    // If he doesn't select anyone
    // -> abort operation
    if (sSelectedFilter.isEmpty())
    {
        xAbort->select();
        return;
    }

    // otherwise set it for return
    xFilterTransport->setFilter( sSelectedFilter );
    xFilterTransport->select();
}

void
handleAmbigousFilterRequest_(
    Window * pParent,
    uno::Reference< uno::XComponentContext > const & xContext,
    document::AmbigousFilterRequest const & rRequest,
    uno::Sequence<
        uno::Reference<
            task::XInteractionContinuation > > const & rContinuations)
    SAL_THROW((uno::RuntimeException))
{
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< document::XInteractionFilterSelect > xFilterTransport;
    getContinuations(rContinuations, &xAbort, &xFilterTransport);

    uui::FilterNameList lNames;

    uno::Reference< container::XNameContainer > xFilterContainer;
    try
    {
        xFilterContainer.set( xContext->getServiceManager()->createInstanceWithContext(
            OUString( "com.sun.star.document.FilterFactory"), xContext ),
            uno::UNO_QUERY );
    }
    catch ( uno::Exception & )
    {
    }

    if( xFilterContainer.is() )
    {
        uno::Any                              aPackedSet    ;
        uno::Sequence< beans::PropertyValue > lProps        ;
        sal_Int32                             nStep         ;
        uui::FilterNamePair                   aPair         ;

        try
        {
            aPackedSet = xFilterContainer->getByName( rRequest.SelectedFilter );
        }
        catch(const container::NoSuchElementException&)
        {
            aPackedSet.clear();
        }
        aPackedSet >>= lProps;
        for( nStep=0; nStep<lProps.getLength(); ++nStep )
        {
            if( lProps[nStep].Name.compareToAscii("UIName") == 0 )
            {
                OUString sTemp;
                lProps[nStep].Value >>= sTemp;
                aPair.sUI       = sTemp;
                aPair.sInternal = rRequest.SelectedFilter;
                lNames.push_back( aPair );
                break;
            }
        }

        try
        {
            aPackedSet = xFilterContainer->getByName( rRequest.DetectedFilter );
        }
        catch(const container::NoSuchElementException&)
        {
            aPackedSet.clear();
        }
        aPackedSet >>= lProps;
        for( nStep=0; nStep<lProps.getLength(); ++nStep )
        {
            if( lProps[nStep].Name.compareToAscii("UIName") == 0 )
            {
                OUString sTemp;
                lProps[nStep].Value >>= sTemp;
                aPair.sUI       = sTemp;
                aPair.sInternal = rRequest.DetectedFilter;
                lNames.push_back( aPair );
                break;
            }
        }
    }

    if( xAbort.is() && xFilterTransport.is() )
    {
        if( lNames.size() < 1 )
        {
            xAbort->select();
        }
        else
        {
            OUString sFilter;
            executeFilterDialog( pParent,
                                 rRequest.URL,
                                 lNames,
                                 sFilter );

            if( !sFilter.isEmpty() )
            {
                xFilterTransport->setFilter( sFilter );
                xFilterTransport->select();
            }
            else
                xAbort->select();
        }
    }
}

void
handleFilterOptionsRequest_(
    uno::Reference< uno::XComponentContext > const & xContext,
    document::FilterOptionsRequest const & rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((uno::RuntimeException))
{
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< document::XInteractionFilterOptions > xFilterOptions;
    getContinuations(rContinuations, &xAbort, &xFilterOptions);

    uno::Reference< container::XNameAccess > xFilterCFG;
    try
    {
        xFilterCFG.set( xContext->getServiceManager()->createInstanceWithContext(
                            OUString( "com.sun.star.document.FilterFactory" ), xContext ),
                        uno::UNO_QUERY );
    }
    catch ( uno::Exception const & )
    {
    }

    if( xFilterCFG.is() && rRequest.rProperties.getLength() )
    {
        try
        {
            OUString aFilterName;
            sal_Int32 nPropCount = rRequest.rProperties.getLength();
            for( sal_Int32 ind = 0; ind < nPropCount; ++ind )
            {
                if( rRequest.rProperties[ind].Name.equals(
                        OUString("FilterName")) )
                {
                    rRequest.rProperties[ind].Value >>= aFilterName;
                    break;
                }
            }

            uno::Sequence < beans::PropertyValue > aProps;
            if ( xFilterCFG->getByName( aFilterName ) >>= aProps )
            {
                sal_Int32 nPropertyCount = aProps.getLength();
                for( sal_Int32 nProperty=0;
                     nProperty < nPropertyCount;
                     ++nProperty )
                    if( aProps[nProperty].Name.equals(
                            OUString("UIComponent")) )
                    {
                        OUString aServiceName;
                        aProps[nProperty].Value >>= aServiceName;
                        if( !aServiceName.isEmpty() )
                        {
                            uno::Reference<
                                ui::dialogs::XExecutableDialog > xFilterDialog(
                                    xContext->getServiceManager()->createInstanceWithContext(
                                        aServiceName, xContext ),
                                    uno::UNO_QUERY );
                            uno::Reference< beans::XPropertyAccess >
                                xFilterProperties( xFilterDialog,
                                                   uno::UNO_QUERY );

                            if( xFilterDialog.is() && xFilterProperties.is() )
                            {
                                uno::Reference<
                                    document::XImporter > xImporter(
                                        xFilterDialog, uno::UNO_QUERY );
                                if( xImporter.is() )
                                    xImporter->setTargetDocument(
                                        uno::Reference< lang::XComponent >(
                                            rRequest.rModel, uno::UNO_QUERY ) );

                                xFilterProperties->setPropertyValues(
                                    rRequest.rProperties );

                                if( xFilterDialog->execute() )
                                {
                                    xFilterOptions->setFilterOptions(
                                        xFilterProperties->getPropertyValues() );
                                    xFilterOptions->select();
                                    return;
                                }
                            }
                        }
                        break;
                    }
            }
        }
        catch( container::NoSuchElementException& )
        {
            // the filter name is unknown
        }
        catch( uno::Exception& )
        {
        }
    }

    xAbort->select();
}

} // namespace

bool
UUIInteractionHelper::handleNoSuchFilterRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::NoSuchFilterRequest aNoSuchFilterRequest;
    if (aAnyRequest >>= aNoSuchFilterRequest)
    {
        handleNoSuchFilterRequest_(getParentProperty(),
                                   m_xContext,
                                   aNoSuchFilterRequest,
                                   rRequest->getContinuations());
        return true;
    }
    return false;
}

bool
UUIInteractionHelper::handleAmbigousFilterRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::AmbigousFilterRequest aAmbigousFilterRequest;
    if (aAnyRequest >>= aAmbigousFilterRequest)
    {
        handleAmbigousFilterRequest_(getParentProperty(),
                                     m_xContext,
                                     aAmbigousFilterRequest,
                                     rRequest->getContinuations());
        return true;
    }
    return false;
}

bool
UUIInteractionHelper::handleFilterOptionsRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::FilterOptionsRequest aFilterOptionsRequest;
    if (aAnyRequest >>= aFilterOptionsRequest)
    {
        handleFilterOptionsRequest_(m_xContext,
                                    aFilterOptionsRequest,
                                    rRequest->getContinuations());
        return true;
    }
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
