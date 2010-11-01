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
#ifndef _CHARTFRAMELOADER_HXX
#define _CHARTFRAMELOADER_HXX

#include "ServiceMacros.hxx"
#include <osl/conditn.hxx>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

class ChartFrameLoader : public ::cppu::WeakImplHelper2<
         ::com::sun::star::frame::XSynchronousFrameLoader
         , ::com::sun::star::lang::XServiceInfo
            //comprehends XComponent (required interface)
    //  ,public ::com::sun::star::uno::XWeak            // implemented by WeakImplHelper(optional interface)
    //  ,public ::com::sun::star::uno::XInterface       // implemented by WeakImplHelper(optional interface)
    //  ,public ::com::sun::star::lang::XTypeProvider   // implemented by WeakImplHelper
        >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>        m_xCC;
    sal_Bool            m_bCancelRequired;
    ::osl::Condition    m_oCancelFinished;

private:
        sal_Bool impl_checkCancel();
    //no default constructor
    ChartFrameLoader(){}
public:
    ChartFrameLoader(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext);
    virtual ~ChartFrameLoader();

    //-----------------------------------------------------------------
    // ::com::sun::star::lang::XServiceInfo
    //-----------------------------------------------------------------

    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ChartFrameLoader)

    //-----------------------------------------------------------------
    // ::com::sun::star::frame::XFrameLoader
    //-----------------------------------------------------------------

    virtual sal_Bool SAL_CALL
        load( const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue >& rMediaDescriptor
                ,const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XFrame >& xFrame )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        cancel()            throw (::com::sun::star::uno::RuntimeException);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
