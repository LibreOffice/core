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

#ifndef INCLUDED_COMPHELPER_SOURCE_MISC_DOCUMENTIOLOGRING_HXX
#define INCLUDED_COMPHELPER_SOURCE_MISC_DOCUMENTIOLOGRING_HXX

#include <com/sun/star/logging/XSimpleLogRing.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>

#define SIMPLELOGRING_SIZE 256

namespace comphelper
{

class OSimpleLogRing : public ::cppu::WeakImplHelper< css::logging::XSimpleLogRing,
                                                      css::lang::XInitialization,
                                                      css::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;
    css::uno::Sequence< OUString > m_aMessages;

    bool      m_bInitialized;
    bool      m_bFull;
    sal_Int32 m_nPos;

public:
    OSimpleLogRing();
    virtual ~OSimpleLogRing();

    static css::uno::Sequence< OUString > SAL_CALL
            getSupportedServiceNames_static();

    static OUString SAL_CALL getImplementationName_static();

    static OUString SAL_CALL getSingletonName_static();

    static OUString SAL_CALL getServiceName_static();

    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        Create( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

// XSimpleLogRing
    virtual void SAL_CALL logString( const OUString& aMessage ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getCollectedLog() throw (css::uno::RuntimeException, std::exception) override;

// XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

// XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

};

} // namespace comphelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
