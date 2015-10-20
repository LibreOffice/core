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

#ifndef INCLUDED_FRAMEWORK_INTERACTION_HXX
#define INCLUDED_FRAMEWORK_INTERACTION_HXX

#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/document/XInteractionFilterSelect.hpp>
#include <com/sun/star/document/NoSuchFilterRequest.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <framework/fwedllapi.h>

namespace framework{

/*-************************************************************************************************************
    @short          special request for interaction to ask user for right filter
    @descr          These helper can be used to ask user for right filter, if filter detection failed.
                    It capsulate communication with any interaction handler and supports an easy
                    access on interaction results for user of these class.
                    Use it and forget complex mechanism of interaction ...

    @example        RequestFilterSelect*             pRequest = new RequestFilterSelect;
                    Reference< XInteractionRequest > xRequest ( pRequest );
                    xInteractionHandler->handle( xRequest );
                    if( ! pRequest.isAbort() )
                    {
                        OUString sFilter = pRequest->getFilter();
                    }

    @implements     XInteractionRequest

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
class RequestFilterSelect_Impl;
class FWE_DLLPUBLIC RequestFilterSelect
{
    RequestFilterSelect_Impl* pImp;

public:
    RequestFilterSelect( const OUString& sURL );
    ~RequestFilterSelect();
    bool     isAbort  () const;
    OUString getFilter() const;
    css::uno::Reference < css::task::XInteractionRequest > GetRequest();
};

/*-************************************************************************************************************
    @short          special request for interaction
    @descr          User must decide between a preselected and another detected filter.
                    It capsulate communication with any interaction handler and supports an easy
                    access on interaction results for user of these class.

    @implements     XInteractionRequest

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
class FWE_DLLPUBLIC InteractionRequest
{
public:
    static css::uno::Reference < css::task::XInteractionRequest > CreateRequest(
        const css::uno::Any& aRequest,
        const css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >& lContinuations );
};


}       //  namespace framework

#endif  // #define INCLUDED_FRAMEWORK_INTERACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
