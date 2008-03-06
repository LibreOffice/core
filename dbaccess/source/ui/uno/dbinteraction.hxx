/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbinteraction.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:31:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

