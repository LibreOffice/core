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

#ifndef _DBAUI_INTERACTION_HXX_
#define _DBAUI_INTERACTION_HXX_

#include <cppuhelper/implbase2.hxx>

#include "moduledbu.hxx"
#include "apitools.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#include <com/sun/star/sdb/ParametersRequest.hpp>
#include <com/sun/star/sdb/DocumentSaveRequest.hpp>

namespace dbtools
{
    class SQLExceptionInfo;
}

namespace dbaui
{

    // BasicInteractionHandler
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::task::XInteractionHandler2
                                    >   BasicInteractionHandler_Base;
    /** implements an <type scope="com.sun.star.task">XInteractionHandler</type> for
        database related interaction requests.
        <p/>
        Supported interaction requests by now (specified by an exception: The appropriate exception
        has to be returned by the getRequest method of the object implementing the
        <type scope="com.sun.star.task">XInteractionRequest</type> interface.
            <ul>
                <li><b><type scope="com.sun.star.sdbc">SQLException</type></b>: requests to display a
                    standard error dialog for the (maybe chained) exception given</li>
            </ul>
    */
    class BasicInteractionHandler
                :public BasicInteractionHandler_Base
    {
        const OModuleClient m_aModuleClient;
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                            m_xContext;
        const bool          m_bFallbackToGeneric;

    public:
        BasicInteractionHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
            const bool i_bFallbackToGeneric
        );

        // XInitialization
        void SAL_CALL initialize( com::sun::star::uno::Sequence< com::sun::star::uno::Any > const & /*rArguments*/) throw (com::sun::star::uno::Exception) {}

        // XInteractionHandler2
        virtual ::sal_Bool SAL_CALL handleInteractionRequest( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& Request ) throw (::com::sun::star::uno::RuntimeException);

        // XInteractionHandler
        virtual void SAL_CALL handle( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& Request ) throw(::com::sun::star::uno::RuntimeException);

    protected:
        sal_Bool
                impl_handle_throw( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& i_Request );

        /// handle SQLExceptions (and derived classes)
        void    implHandle(
                    const ::dbtools::SQLExceptionInfo& _rSqlInfo,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >& _rContinuations);

        /// handle parameter requests
        void    implHandle(
                    const ::com::sun::star::sdb::ParametersRequest& _rParamRequest,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >& _rContinuations);

        /// handle document save requests
        void    implHandle(
                    const ::com::sun::star::sdb::DocumentSaveRequest& _rParamRequest,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >& _rContinuations);

        /// handles requests which are not SDB-specific
        bool    implHandleUnknown(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& _rxRequest );

        /// known continuation types
        enum Continuation
        {
            APPROVE,
            DISAPPROVE,
            RETRY,
            ABORT,
            SUPPLY_PARAMETERS,
            SUPPLY_DOCUMENTSAVE
        };
        /** check if a given continuation sequence contains a given continuation type<p/>
            @return     the index within <arg>_rContinuations</arg> of the first occurrence of a continuation
                        of the requested type, -1 of no such continuation exists
        */
        sal_Int32 getContinuation(
            Continuation _eCont,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >& _rContinuations);
    };

    // SQLExceptionInteractionHandler
    class SQLExceptionInteractionHandler : public BasicInteractionHandler
    {
    public:
        SQLExceptionInteractionHandler(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
            )
            :BasicInteractionHandler( rxContext, false )
        {
        }

        // XServiceInfo
        DECLARE_SERVICE_INFO_STATIC();
    };

    // SQLExceptionInteractionHandler
    /** an implementation for the legacy css.sdb.InteractionHandler

        css.sdb.InteractionHandler is deprecated, as it does not only handle database related interactions,
        but also delegates all kind of unknown requests to a css.task.InteractionHandler.

        In today's architecture, there's only one central css.task.InteractionHandler, which is to be used
        for all requests. Depending on configuration information, it decides which handler implementation
        to delegate a request to.

        SQLExceptionInteractionHandler is the delegatee which handles only database related interactions.
        LegacyInteractionHandler is the version which first checks for a database related interaction, and
        forwards everything else to the css.task.InteractionHandler.
    */
    class LegacyInteractionHandler : public BasicInteractionHandler
    {
    public:
        LegacyInteractionHandler(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
            )
            :BasicInteractionHandler( rxContext, true )
        {
        }

        // XServiceInfo
        DECLARE_SERVICE_INFO_STATIC();
    };

}   // namespace dbaui

#endif // _DBAUI_INTERACTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
