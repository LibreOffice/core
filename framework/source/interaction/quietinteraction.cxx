/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include "interaction/quietinteraction.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/document/XInteractionFilterSelect.hpp>
#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#include <com/sun/star/document/AmbigousFilterRequest.hpp>
#include <com/sun/star/document/FilterOptionsRequest.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>

#include <com/sun/star/document/LockedDocumentRequest.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <vcl/svapp.hxx>

#ifndef __RSC
#include <tools/errinf.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

DEFINE_XINTERFACE_2( QuietInteraction                                 ,
                     OWeakObject                                      ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider      ) ,
                     DIRECT_INTERFACE(css::task::XInteractionHandler) )

DEFINE_XTYPEPROVIDER_2( QuietInteraction               ,
                        css::lang::XTypeProvider       ,
                        css::task::XInteractionHandler )

//_________________________________________________________________________________________________________________

QuietInteraction::QuietInteraction()
    : ThreadHelpBase     ( &Application::GetSolarMutex() )
    , ::cppu::OWeakObject(                               )
    , m_aRequest         (                               )
{
}

//_________________________________________________________________________________________________________________

void SAL_CALL QuietInteraction::handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest ) throw( css::uno::RuntimeException )
{
    // safe the request for outside analyzing everytime!
    css::uno::Any aRequest = xRequest->getRequest();
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_aRequest = aRequest;
    aWriteLock.unlock();
    /* } SAFE */

    // analyze the request
    // We need XAbort as possible continuation as minimum!
    // An optional filter selection we can handle too.
    css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations = xRequest->getContinuations();
    css::uno::Reference< css::task::XInteractionAbort >                              xAbort     ;
    css::uno::Reference< css::task::XInteractionApprove >                            xApprove   ;
    css::uno::Reference< css::document::XInteractionFilterSelect >                   xFilter    ;
    css::uno::Reference< css::document::XInteractionFilterOptions >                  xFOptions  ;

    sal_Int32 nCount=lContinuations.getLength();
    for (sal_Int32 i=0; i<nCount; ++i)
    {
        if ( ! xAbort.is() )
            xAbort = css::uno::Reference< css::task::XInteractionAbort >( lContinuations[i], css::uno::UNO_QUERY );

        if( ! xApprove.is() )
            xApprove  = css::uno::Reference< css::task::XInteractionApprove >( lContinuations[i], css::uno::UNO_QUERY );

        if ( ! xFilter.is() )
            xFilter = css::uno::Reference< css::document::XInteractionFilterSelect >( lContinuations[i], css::uno::UNO_QUERY );

        if ( ! xFOptions.is() )
            xFOptions = css::uno::Reference< css::document::XInteractionFilterOptions >( lContinuations[i], css::uno::UNO_QUERY );
    }

    // differ between abortable interactions (error, unknown filter ...)
    // and other ones (ambigous but not unknown filter ...)
    css::task::ErrorCodeRequest          aErrorCodeRequest     ;
    css::document::AmbigousFilterRequest aAmbigousFilterRequest;
    css::document::LockedDocumentRequest aLockedDocumentRequest;
    css::document::FilterOptionsRequest  aFilterOptionsRequest;

    if (aRequest>>=aAmbigousFilterRequest)
    {
        if (xFilter.is())
        {
            // user selected filter wins everytime!
            xFilter->setFilter( aAmbigousFilterRequest.SelectedFilter );
            xFilter->select();
        }
    }
    else
    if( aRequest >>= aErrorCodeRequest )
    {
        // warnings can be ignored   => approve
        // errors must break loading => abort
        sal_Bool bWarning = (aErrorCodeRequest.ErrCode & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK;
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

//_________________________________________________________________________________________________________________

css::uno::Any QuietInteraction::getRequest() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_aRequest;
    /* } SAFE */
}

//_________________________________________________________________________________________________________________

sal_Bool QuietInteraction::wasUsed() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_aRequest.hasValue();
    /* } SAFE */
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
