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


#include <unotools/pathoptions.hxx>
#include "sal/types.h"
#include "rtl/ustring.hxx"
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>




class PathService : public ::cppu::WeakImplHelper2< css::frame::XConfigManager, css::lang::XServiceInfo >
{
    SvtPathOptions m_aOptions;

public:
    PathService()
        {}

    virtual OUString SAL_CALL getImplementationName()
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            return OUString("com.sun.star.comp.svl.PathService");
        }

    virtual sal_Bool SAL_CALL supportsService (
        const OUString & rName)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            return rName.equalsAscii("com.sun.star.config.SpecialConfigManager");
        }

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            css::uno::Sequence< OUString > aRet(1);
            aRet.getArray()[0] = "com.sun.star.config.SpecialConfigManager";
            return aRet;
        }

    virtual OUString SAL_CALL substituteVariables (
        const OUString& sText)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            return m_aOptions.SubstituteVariable( sText );
        }

    virtual void SAL_CALL addPropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {}

    virtual void SAL_CALL removePropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {}

    virtual void SAL_CALL flush()
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {}
};



css::uno::Reference< css::uno::XInterface > PathService_CreateInstance (
    SAL_UNUSED_PARAMETER const css::uno::Reference<
        css::lang::XMultiServiceFactory > &)
{
    return css::uno::Reference< css::uno::XInterface >(
        static_cast< cppu::OWeakObject* >(new PathService()));
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
