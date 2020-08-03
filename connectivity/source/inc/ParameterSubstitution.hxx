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

#pragma once

#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

namespace connectivity
{
    typedef ::cppu::WeakImplHelper< css::util::XStringSubstitution
                                   ,css::lang::XServiceInfo
                                   ,css::lang::XInitialization > ParameterSubstitution_BASE;
    class ParameterSubstitution final : public ParameterSubstitution_BASE
    {
        ::osl::Mutex                                          m_aMutex;
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        css::uno::WeakReference< css::sdbc::XConnection >     m_xConnection;

        ParameterSubstitution( const ParameterSubstitution& ) = delete;
        ParameterSubstitution& operator=( const ParameterSubstitution& ) = delete;
    public:
        ParameterSubstitution(const css::uno::Reference< css::uno::XComponentContext >& _rxContext );
    private:

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XStringSubstitution
        virtual OUString SAL_CALL substituteVariables( const OUString& aText, sal_Bool bSubstRequired ) override;
        virtual OUString SAL_CALL reSubstituteVariables( const OUString& aText ) override;
        virtual OUString SAL_CALL getSubstituteVariableValue( const OUString& variable ) override;
    };

} // connectivity


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
