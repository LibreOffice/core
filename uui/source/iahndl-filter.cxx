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

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/FilterOptionsRequest.hpp>
#include <com/sun/star/document/NoSuchFilterRequest.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#include <com/sun/star/document/XInteractionFilterSelect.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <vcl/svapp.hxx>

#include "getcontinuations.hxx"
#include "fltdlg.hxx"

#include "iahndl.hxx"

using namespace com::sun::star;

namespace {

void
executeFilterDialog(
    weld::Window* pParent ,
    OUString       const & rURL    ,
    uui::FilterNameList const & rFilters,
    OUString             & rFilter )
{
    SolarMutexGuard aGuard;

    uui::FilterDialog aDialog(pParent);

    aDialog.SetURL(rURL);
    aDialog.ChangeFilters(&rFilters);

    uui::FilterNameListPtr pSelected = rFilters.end();
    if (aDialog.AskForFilter(pSelected))
    {
        rFilter = pSelected->sInternal;
    }
}

void
handleNoSuchFilterRequest_(
    weld::Window* pParent,
    uno::Reference< uno::XComponentContext > const & xContext,
    document::NoSuchFilterRequest const & rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
            rContinuations )
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
                                  u"com.sun.star.document.FilterFactory"_ustr, xContext ),
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
            u"_query_all:sort_prop=uiname:iflags=1:eflags=143360"_ustr);
    while (xFilters->hasMoreElements())
    {
        try
        {
            ::comphelper::SequenceAsHashMap lProps(xFilters->nextElement());
            uui::FilterNamePair             aPair;

            aPair.sInternal = lProps.getUnpackedValueOrDefault(
                u"Name"_ustr, OUString());
            aPair.sUI       = lProps.getUnpackedValueOrDefault(
                 u"UIName"_ustr, OUString());
            if ( aPair.sInternal.isEmpty() || aPair.sUI.isEmpty() )
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
    if (lNames.empty())
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
handleFilterOptionsRequest_(
    uno::Reference<awt::XWindow> const & rWindow,
    uno::Reference< uno::XComponentContext > const & xContext,
    document::FilterOptionsRequest const & rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations)
{
    uno::Reference< task::XInteractionAbort > xAbort;
    uno::Reference< document::XInteractionFilterOptions > xFilterOptions;
    getContinuations(rContinuations, &xAbort, &xFilterOptions);

    uno::Reference< container::XNameAccess > xFilterCFG;
    try
    {
        xFilterCFG.set( xContext->getServiceManager()->createInstanceWithContext(
                            u"com.sun.star.document.FilterFactory"_ustr, xContext ),
                        uno::UNO_QUERY );
    }
    catch ( uno::Exception const & )
    {
    }

    if( xFilterCFG.is() && rRequest.rProperties.hasElements() )
    {
        try
        {
            OUString aFilterName;
            auto pProperty = std::find_if(rRequest.rProperties.begin(), rRequest.rProperties.end(),
                [](const beans::PropertyValue& rProp) { return rProp.Name == "FilterName"; });
            if (pProperty != rRequest.rProperties.end())
            {
                pProperty->Value >>= aFilterName;
            }

            uno::Sequence < beans::PropertyValue > aProps;
            if ( xFilterCFG->getByName( aFilterName ) >>= aProps )
            {
                auto pProp = std::find_if(std::cbegin(aProps), std::cend(aProps),
                    [](const beans::PropertyValue& rProp) { return rProp.Name == "UIComponent"; });
                if (pProp != std::cend(aProps))
                {
                    OUString aServiceName;
                    pProp->Value >>= aServiceName;
                    if( !aServiceName.isEmpty() )
                    {
                        uno::Sequence<uno::Any> aDialogArgs(comphelper::InitAnyPropertySequence(
                        {
                            {"ParentWindow", uno::Any(rWindow)},
                        }));

                        uno::Reference<
                            ui::dialogs::XExecutableDialog > xFilterDialog(
                                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                    aServiceName, aDialogArgs, xContext ),
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
                                xImporter->setTargetDocument( rRequest.rModel );

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
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::NoSuchFilterRequest aNoSuchFilterRequest;
    if (aAnyRequest >>= aNoSuchFilterRequest)
    {
        uno::Reference<awt::XWindow> xParent = getParentXWindow();
        handleNoSuchFilterRequest_(Application::GetFrameWeld(xParent),
                                   m_xContext,
                                   aNoSuchFilterRequest,
                                   rRequest->getContinuations());
        return true;
    }
    return false;
}

bool
UUIInteractionHelper::handleFilterOptionsRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    uno::Any aAnyRequest(rRequest->getRequest());

    document::FilterOptionsRequest aFilterOptionsRequest;
    if (aAnyRequest >>= aFilterOptionsRequest)
    {
        handleFilterOptionsRequest_(getParentXWindow(),
                                    m_xContext,
                                    aFilterOptionsRequest,
                                    rRequest->getContinuations());
        return true;
    }
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
