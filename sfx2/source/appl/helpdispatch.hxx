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
#ifndef SFX_HELPDISPATCH_HXX
#define SFX_HELPDISPATCH_HXX

#include <com/sun/star/frame/XDispatch.hpp>
#include <cppuhelper/implbase1.hxx>

#include "helpinterceptor.hxx"

class HelpDispatch_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XDispatch >
{
private:
    HelpInterceptor_Impl&       m_rInterceptor;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
                                m_xRealDispatch;

public:
    HelpDispatch_Impl( HelpInterceptor_Impl& _rInterceptor,
                       const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& _xDisp );
    ~HelpDispatch_Impl();

    // XDispatch
    virtual void SAL_CALL   dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);
};

#endif // #ifndef SFX_HELPDISPATCHER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
