/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

#ifndef _COM_SUN_STAR_DOCUMENT_LOCKEDDOCUMENTREQUEST_HPP_
#include <com/sun/star/document/LockedDocumentRequest.hpp>
#endif

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
    // safe the request for outside analyzing every time!
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
            // user selected filter wins every time!
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
