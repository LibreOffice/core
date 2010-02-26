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

#ifndef _DBAUI_INTERACTION_HXX_
#define _DBAUI_INTERACTION_HXX_

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_AUTHENTICATIONREQUEST_HPP_
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_PARAMETERSREQUEST_HPP_
#include <com/sun/star/sdb/ParametersRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_DOCUMENTSAVEREQUEST_HPP_
#include <com/sun/star/sdb/DocumentSaveRequest.hpp>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

namespace dbtools
{
    class SQLExceptionInfo;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

    //=========================================================================
    //= OInteractionHandler
    //=========================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::task::XInteractionHandler
                                    >   OInteractionHandler_Base;
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
    class OInteractionHandler
                :public OInteractionHandler_Base
    {
        OModuleClient m_aModuleClient;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                m_xORB;
    public:
        OInteractionHandler(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

    // XServiceInfo
        DECLARE_SERVICE_INFO_STATIC();

    // XInteractionHandler
        virtual void SAL_CALL handle( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& Request ) throw(::com::sun::star::uno::RuntimeException);

    protected:
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
            @return     the index within <arg>_rContinuations</arg> of the first occurence of a continuation
                        of the requested type, -1 of no such continuation exists
        */
        sal_Int32 getContinuation(
            Continuation _eCont,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >& _rContinuations);
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_INTERACTION_HXX_

