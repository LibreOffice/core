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

#include "interaction/quietinteraction.hxx"

#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/document/XInteractionFilterSelect.hpp>
#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#include <com/sun/star/document/FilterOptionsRequest.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>

#include <com/sun/star/document/LockedDocumentRequest.hpp>

#include <vcl/svapp.hxx>

#ifndef __RSC
#include <tools/errinf.hxx>
#endif

namespace framework{

QuietInteraction::QuietInteraction()
    : m_aRequest         (                               )
{
}

void SAL_CALL QuietInteraction::handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest ) throw( css::uno::RuntimeException, std::exception )
{
    // safe the request for outside analyzing every time!
    css::uno::Any aRequest = xRequest->getRequest();
    {
        SolarMutexGuard g;
        m_aRequest = aRequest;
    }

    // analyze the request
    // We need XAbort as possible continuation as minimum!
    // An optional filter selection we can handle too.
    css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations = xRequest->getContinuations();
    css::uno::Reference< css::task::XInteractionAbort >                              xAbort;
    css::uno::Reference< css::task::XInteractionApprove >                            xApprove;
    css::uno::Reference< css::document::XInteractionFilterSelect >                   xFilter;
    css::uno::Reference< css::document::XInteractionFilterOptions >                  xFOptions;

    sal_Int32 nCount=lContinuations.getLength();
    for (sal_Int32 i=0; i<nCount; ++i)
    {
        if ( ! xAbort.is() )
            xAbort.set( lContinuations[i], css::uno::UNO_QUERY );

        if( ! xApprove.is() )
            xApprove.set( lContinuations[i], css::uno::UNO_QUERY );

        if ( ! xFilter.is() )
            xFilter.set( lContinuations[i], css::uno::UNO_QUERY );

        if ( ! xFOptions.is() )
            xFOptions.set( lContinuations[i], css::uno::UNO_QUERY );
    }

    // differ between abortable interactions (error, unknown filter ...)
    // and other ones (ambigous but not unknown filter ...)
    css::task::ErrorCodeRequest          aErrorCodeRequest;
    css::document::LockedDocumentRequest aLockedDocumentRequest;
    css::document::FilterOptionsRequest  aFilterOptionsRequest;

    if( aRequest >>= aErrorCodeRequest )
    {
        // warnings can be ignored   => approve
        // errors must break loading => abort
        bool bWarning = (aErrorCodeRequest.ErrCode & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK;
        if (xApprove.is() && bWarning)
            xApprove->select();
        else
        if (xAbort.is())
            xAbort->select();
    }
    else
    if( aRequest >>= aLockedDocumentRequest )
    {
        // the locked document should be opened readonly by default
        if (xApprove.is())
            xApprove->select();
        else
        if (xAbort.is())
            xAbort->select();
    }
    else
    if (aRequest>>=aFilterOptionsRequest)
    {
        if (xFOptions.is())
        {
            // let the default filter options be used
            xFOptions->select();
        }
    }
    else
    if (xAbort.is())
        xAbort->select();
}

css::uno::Any QuietInteraction::getRequest() const
{
    SolarMutexGuard g;
    return m_aRequest;
}

bool QuietInteraction::wasUsed() const
{
    SolarMutexGuard g;
    return m_aRequest.hasValue();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
