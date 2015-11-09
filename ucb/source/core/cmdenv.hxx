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

#ifndef INCLUDED_UCB_SOURCE_CORE_CMDENV_HXX
#define INCLUDED_UCB_SOURCE_CORE_CMDENV_HXX

#include <cppuhelper/implbase.hxx>

#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"

namespace ucb_cmdenv {

class UcbCommandEnvironment :
        public cppu::WeakImplHelper< css::lang::XInitialization,
                                      css::lang::XServiceInfo,
                                      css::ucb::XCommandEnvironment >
{
    css::uno::Reference< css::task::XInteractionHandler > m_xIH;
    css::uno::Reference< css::ucb::XProgressHandler >     m_xPH;

public:
    explicit UcbCommandEnvironment( const css::uno::Reference< css::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~UcbCommandEnvironment();

    // XInitialization
    virtual void SAL_CALL
    initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw( css::uno::Exception,
               css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    supportsService( const OUString& ServiceName )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XCommandEnvironment
    virtual css::uno::Reference< css::task::XInteractionHandler > SAL_CALL
    getInteractionHandler()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::ucb::XProgressHandler > SAL_CALL
    getProgressHandler()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // Non-UNO interfaces
    static OUString  getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory >
    createServiceFactory( const css::uno::Reference< css::lang::XMultiServiceFactory > & rxServiceMgr );
};

} // namespace ucb_cmdenv

#endif // INCLUDED_UCB_SOURCE_CORE_CMDENV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
