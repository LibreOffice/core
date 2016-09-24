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

#ifndef INCLUDED_SHELL_SOURCE_WIN32_SIMPLEMAIL_SMPLMAILSUPPL_HXX
#define INCLUDED_SHELL_SOURCE_WIN32_SIMPLEMAIL_SMPLMAILSUPPL_HXX

#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>

#include "simplemapi.hxx"


class CSmplMailSupplBase
{
protected:
    osl::Mutex  m_aMutex;
};

class CSmplMailSuppl :
    public CSmplMailSupplBase,
    public  cppu::WeakComponentImplHelper<
            css::system::XSimpleMailClientSupplier,
            css::lang::XServiceInfo >
{
public:
    CSmplMailSuppl( );
    ~CSmplMailSuppl( );

    // XSimpleMailClientSupplier
    virtual css::uno::Reference< css::system::XSimpleMailClient > SAL_CALL querySimpleMailClient(  )
        throw (css::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(css::uno::RuntimeException);

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(css::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
