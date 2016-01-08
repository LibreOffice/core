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

#include <comphelper/interaction.hxx>
#include <framework/interaction.hxx>
#include <general.h>

using namespace ::com::sun::star;

namespace framework{

/*-************************************************************************************************************
    @short          declaration of special continuation for filter selection
    @descr          Sometimes filter detection during loading document failed. Then we need a possibility
                    to ask user for his decision. These continuation transport selected filter by user to
                    code user of interaction.

    @attention      This implementation could be used one times only. We don't support a resettable continuation yet!
                    Why? Normally interaction should show a filter selection dialog and ask user for his decision.
                    He can select any filter - then instances of these class will be called by handler... or user
                    close dialog without any selection. Then another continuation should be selected by handler to
                    abort continuations... Retrying isn't very useful here... I think.

    @implements     XInteractionFilterSelect

    @base           ImplInheritanceHelper
                    ContinuationBase

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
class ContinuationFilterSelect : public comphelper::OInteraction< css::document::XInteractionFilterSelect >
{
    // c++ interface
    public:
        ContinuationFilterSelect();

    // uno interface
    public:
        virtual void            SAL_CALL setFilter( const OUString& sFilter ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual OUString SAL_CALL getFilter(                                ) throw( css::uno::RuntimeException, std::exception ) override;

    // member
    private:
        OUString m_sFilter;

};  // class ContinuationFilterSelect

// initialize continuation with right start values

ContinuationFilterSelect::ContinuationFilterSelect()
    : m_sFilter( OUString() )
{
}

// handler should use it after selection to set user specified filter for transport

void SAL_CALL ContinuationFilterSelect::setFilter( const OUString& sFilter ) throw( css::uno::RuntimeException, std::exception )
{
    m_sFilter = sFilter;
}

// read access to transported filter

OUString SAL_CALL ContinuationFilterSelect::getFilter() throw( css::uno::RuntimeException, std::exception )
{
    return m_sFilter;
}

class RequestFilterSelect_Impl : public ::cppu::WeakImplHelper< css::task::XInteractionRequest >
{
public:
    explicit RequestFilterSelect_Impl(const OUString& rURL);
    bool     isAbort  () const;
    OUString getFilter() const;

public:
    virtual css::uno::Any SAL_CALL getRequest() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( css::uno::RuntimeException, std::exception ) override;

private:
    css::uno::Any                                                                       m_aRequest;
    css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >    m_lContinuations;
    comphelper::OInteractionAbort*                                                      m_pAbort;
    ContinuationFilterSelect*                                                           m_pFilter;
};

// initialize instance with all necessary information
// We use it without any further checks on our member then ...!

RequestFilterSelect_Impl::RequestFilterSelect_Impl( const OUString& sURL )
{
    OUString temp;
    css::uno::Reference< css::uno::XInterface > temp2;
    css::document::NoSuchFilterRequest aFilterRequest( temp                             ,
                                                       temp2                            ,
                                                       sURL                                          );
    m_aRequest <<= aFilterRequest;

    m_pAbort  = new comphelper::OInteractionAbort;
    m_pFilter = new ContinuationFilterSelect;

    m_lContinuations.realloc( 2 );
    m_lContinuations[0].set( m_pAbort  );
    m_lContinuations[1].set( m_pFilter );
}

// return abort state of interaction
// If it is true, return value of method "getFilter()" will be unspecified then!

bool RequestFilterSelect_Impl::isAbort() const
{
    return m_pAbort->wasSelected();
}

// return user selected filter
// Return value valid for non aborted interaction only. Please check "isAbort()" before you call these ony!

OUString RequestFilterSelect_Impl::getFilter() const
{
    return m_pFilter->getFilter();
}

// handler call it to get type of request
// Is hard coded to "please select filter" here. see ctor for further information.

css::uno::Any SAL_CALL RequestFilterSelect_Impl::getRequest() throw( css::uno::RuntimeException, std::exception )
{
    return m_aRequest;
}

// handler call it to get possible continuations
// We support "abort/select_filter" only here.
// After interaction we support read access on these continuations on our c++ interface to
// return user decision.

css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL RequestFilterSelect_Impl::getContinuations() throw( css::uno::RuntimeException, std::exception )
{
    return m_lContinuations;
}

RequestFilterSelect::RequestFilterSelect( const OUString& sURL )
{
    pImp = new RequestFilterSelect_Impl( sURL );
    pImp->acquire();
}

RequestFilterSelect::~RequestFilterSelect()
{
    pImp->release();
}

// return abort state of interaction
// If it is true, return value of method "getFilter()" will be unspecified then!

bool RequestFilterSelect::isAbort() const
{
    return pImp->isAbort();
}

// return user selected filter
// Return value valid for non aborted interaction only. Please check "isAbort()" before you call these ony!

OUString RequestFilterSelect::getFilter() const
{
    return pImp->getFilter();
}

uno::Reference < task::XInteractionRequest > RequestFilterSelect::GetRequest()
{
    return uno::Reference < task::XInteractionRequest > (pImp);
}

class InteractionRequest_Impl : public ::cppu::WeakImplHelper< css::task::XInteractionRequest >
{
    uno::Any m_aRequest;
    uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > m_lContinuations;

public:
    InteractionRequest_Impl( const css::uno::Any& aRequest,
        const css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >& lContinuations )
    {
        m_aRequest = aRequest;
        m_lContinuations = lContinuations;
    }

    virtual uno::Any SAL_CALL getRequest() throw( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL getContinuations()
            throw( uno::RuntimeException, std::exception ) override;
};

uno::Any SAL_CALL InteractionRequest_Impl::getRequest() throw( uno::RuntimeException, std::exception )
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL InteractionRequest_Impl::getContinuations()
    throw( uno::RuntimeException, std::exception )
{
    return m_lContinuations;
}

uno::Reference < task::XInteractionRequest > InteractionRequest::CreateRequest(
    const uno::Any& aRequest, const uno::Sequence< uno::Reference< task::XInteractionContinuation > >& lContinuations )
{
    return new InteractionRequest_Impl( aRequest, lContinuations );
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
