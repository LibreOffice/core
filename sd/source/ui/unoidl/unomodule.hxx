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

#ifndef _SD_UNOMODULE_HXX
#define _SD_UNOMODULE_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/implbase3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace lang
            {
                class XMultiServiceFactory;
            }
            namespace beans
            {
                struct PropertyValue;
            }
        }
    }
}

#define REFERENCE               ::com::sun::star::uno::Reference
#define SEQUENCE                ::com::sun::star::uno::Sequence
#define RUNTIMEEXCEPTION        ::com::sun::star::uno::RuntimeException
#define REFERENCE               ::com::sun::star::uno::Reference
#define SEQUENCE                ::com::sun::star::uno::Sequence
#define XDISPATCH               ::com::sun::star::frame::XDispatch
#define XNOTIFYINGDISPATCH      ::com::sun::star::frame::XNotifyingDispatch
#define OUSTRING                ::rtl::OUString
#define UNOURL                  ::com::sun::star::util::URL
#define DISPATCHDESCRIPTOR      ::com::sun::star::frame::DispatchDescriptor

class SdUnoModule : public ::cppu::WeakImplHelper3< ::com::sun::star::frame::XDispatchProvider, ::com::sun::star::frame::XNotifyingDispatch, ::com::sun::star::lang::XServiceInfo >
{
    REFERENCE < ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

public:
                            SdUnoModule( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
                                : m_xFactory( xFactory )
                            {}

    // XnotifyingDispatch
    virtual void SAL_CALL dispatchWithNotification( const ::com::sun::star::util::URL& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // XDispatch
    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );

    // XDispatchProvider
    virtual SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescriptor ) throw( RUNTIMEEXCEPTION ) ;
    virtual REFERENCE< XDISPATCH > SAL_CALL queryDispatch(  const   UNOURL &            aURL            ,
                                                            const   OUSTRING &          sTargetFrameName,
                                                                    sal_Int32   eSearchFlags    ) throw( RUNTIMEEXCEPTION ) ;
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
