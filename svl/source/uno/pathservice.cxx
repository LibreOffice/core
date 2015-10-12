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
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

class PathService : public ::cppu::WeakImplHelper< css::frame::XConfigManager, css::lang::XServiceInfo >
{
    SvtPathOptions m_aOptions;

public:
    PathService()
        {}

    virtual OUString SAL_CALL getImplementationName()
        throw(css::uno::RuntimeException, std::exception) override
        {
            return OUString("com.sun.star.comp.svl.PathService");
        }

    virtual sal_Bool SAL_CALL supportsService (
        const OUString & rName)
        throw(css::uno::RuntimeException, std::exception) override
        {
            return cppu::supportsService(this, rName);
        }

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(css::uno::RuntimeException, std::exception) override
        {
            css::uno::Sequence< OUString > aRet(1);
            aRet.getArray()[0] = "com.sun.star.config.SpecialConfigManager";
            return aRet;
        }

    virtual OUString SAL_CALL substituteVariables (
        const OUString& sText)
        throw(css::uno::RuntimeException, std::exception) override
        {
            return m_aOptions.SubstituteVariable( sText );
        }

    virtual void SAL_CALL addPropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException, std::exception) override
        {}

    virtual void SAL_CALL removePropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException, std::exception) override
        {}

    virtual void SAL_CALL flush()
        throw(css::uno::RuntimeException, std::exception) override
        {}
};


extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
com_sun_star_comp_svl_PathService_get_implementation(::com::sun::star::uno::XComponentContext*,
                                                     ::com::sun::star::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new PathService());
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
