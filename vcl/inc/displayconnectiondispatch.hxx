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

#ifndef INCLUDED_VCL_INC_DISPLAYCONNECTIONDISPATCH_HXX
#define INCLUDED_VCL_INC_DISPLAYCONNECTIONDISPATCH_HXX

#include <sal/config.h>
#include <com/sun/star/awt/XDisplayConnection.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <list>

namespace vcl {

class DisplayConnectionDispatch :
    public cppu::WeakImplHelper< css::awt::XDisplayConnection >
{
    ::osl::Mutex                    m_aMutex;
    ::std::list< css::uno::Reference< css::awt::XEventHandler > >
                                    m_aHandlers;
    ::std::list< css::uno::Reference< css::awt::XEventHandler > >
                                    m_aErrorHandlers;
    OUString                        m_ConnectionIdentifier;
public:
    DisplayConnectionDispatch();
    ~DisplayConnectionDispatch() override;

    void start();
    void terminate();

    bool dispatchEvent( void* pData, int nBytes );

    // XDisplayConnection
    virtual void SAL_CALL addEventHandler( const css::uno::Any& window, const css::uno::Reference< css::awt::XEventHandler >& handler, sal_Int32 eventMask ) override;
    virtual void SAL_CALL removeEventHandler( const css::uno::Any& window, const css::uno::Reference< css::awt::XEventHandler >& handler ) override;
    virtual void SAL_CALL addErrorHandler( const css::uno::Reference< css::awt::XEventHandler >& handler ) override;
    virtual void SAL_CALL removeErrorHandler( const css::uno::Reference< css::awt::XEventHandler >& handler ) override;
    virtual css::uno::Any SAL_CALL getIdentifier() override;

};

}



#endif // INCLUDED_VCL_INC_DISPLAYCONNECTIONDISPATCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
