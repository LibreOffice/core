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
#ifndef INCLUDED_SFX2_SOURCE_APPL_HELPDISPATCH_HXX
#define INCLUDED_SFX2_SOURCE_APPL_HELPDISPATCH_HXX

#include <com/sun/star/frame/XDispatch.hpp>
#include <cppuhelper/implbase.hxx>

#include "helpinterceptor.hxx"

class HelpDispatch_Impl : public ::cppu::WeakImplHelper< ::com::sun::star::frame::XDispatch >
{
private:
    HelpInterceptor_Impl&       m_rInterceptor;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
                                m_xRealDispatch;

public:
    HelpDispatch_Impl( HelpInterceptor_Impl& _rInterceptor,
                       const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& _xDisp );
    virtual ~HelpDispatch_Impl();

    // XDispatch
    virtual void SAL_CALL   dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SFX2_SOURCE_APPL_HELPDISPATCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
