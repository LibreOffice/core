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

#ifndef __FRAMEWORK_DISPATCH_INTERACTION_HXX_
#define __FRAMEWORK_DISPATCH_INTERACTION_HXX_

#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/document/XInteractionFilterSelect.hpp>
#include <com/sun/star/document/NoSuchFilterRequest.hpp>
#include <com/sun/star/document/AmbigousFilterRequest.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <framework/fwedllapi.h>

namespace framework{

/*-************************************************************************************************************//**
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

    @base           WeakImplHelper1

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
class RequestFilterSelect_Impl;
class FWE_DLLPUBLIC RequestFilterSelect
{
    RequestFilterSelect_Impl* pImp;

    public:
        RequestFilterSelect( const ::rtl::OUString& sURL );
        ~RequestFilterSelect();
        sal_Bool        isAbort  () const;
        ::rtl::OUString getFilter() const;
    com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionRequest > GetRequest();
};

/*-************************************************************************************************************//**
    @short          special request for interaction
    @descr          User must decide between a preselected and another detected filter.
                    It capsulate communication with any interaction handler and supports an easy
                    access on interaction results for user of these class.

    @implements     XInteractionRequest

    @base           WeakImplHelper1

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
class FWE_DLLPUBLIC InteractionRequest
{
public:
    static com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionRequest >
        CreateRequest( const ::com::sun::star::uno::Any& aRequest,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > lContinuations );
};

/*-************************************************************************************************************//**
    @short          special request for interaction
    @descr          User must decide between a preselected and another detected filter.
                    It capsulate communication with any interaction handler and supports an easy
                    access on interaction results for user of these class.

    @implements     XInteractionRequest

    @base           WeakImplHelper1

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
/*
class RequestAmbigousFilter_Impl;
class RequestAmbigousFilter             // seems to be unused currently
{
    RequestAmbigousFilter_Impl* pImp;

    // c++ interface
public:
    RequestAmbigousFilter( const ::rtl::OUString& sURL,
                            const ::rtl::OUString& sSelectedFilter ,
                            const ::rtl::OUString& sDetectedFilter );
    ~RequestAmbigousFilter();
    sal_Bool        isAbort  () const;
    ::rtl::OUString getFilter() const;
    com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionRequest > GetRequest();
};
 */

}       //  namespace framework

#endif  // #define __FRAMEWORK_DISPATCH_INTERACTION_HXX_
