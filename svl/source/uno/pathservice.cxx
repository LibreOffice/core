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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <unotools/pathoptions.hxx>
#include "sal/types.h"
#include "rtl/ustring.hxx"
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace css = com::sun::star;
using rtl::OUString;

// -----------------------------------------------------------------------

class PathService : public ::cppu::WeakImplHelper2< css::frame::XConfigManager, css::lang::XServiceInfo >
{
    SvtPathOptions m_aOptions;

public:
    PathService()
        {}

    virtual OUString SAL_CALL getImplementationName()
        throw(css::uno::RuntimeException)
        {
            return OUString::createFromAscii("com.sun.star.comp.svl.PathService");
        }

    virtual sal_Bool SAL_CALL supportsService (
        const OUString & rName)
        throw(css::uno::RuntimeException)
        {
            return (rName.compareToAscii("com.sun.star.config.SpecialConfigManager") == 0);
        }

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(css::uno::RuntimeException)
        {
            css::uno::Sequence< OUString > aRet(1);
            aRet.getArray()[0] = OUString::createFromAscii("com.sun.star.config.SpecialConfigManager");
            return aRet;
        }

    virtual OUString SAL_CALL substituteVariables (
        const OUString& sText)
        throw(css::uno::RuntimeException)
        {
            return m_aOptions.SubstituteVariable( sText );
        }

    virtual void SAL_CALL addPropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException)
        {}

    virtual void SAL_CALL removePropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException)
        {}

    virtual void SAL_CALL flush()
        throw(css::uno::RuntimeException)
        {}
};

// -----------------------------------------------------------------------

css::uno::Reference< css::uno::XInterface > PathService_CreateInstance (
    const css::uno::Reference< css::lang::XMultiServiceFactory > &)
{
    return css::uno::Reference< css::uno::XInterface >(
        static_cast< cppu::OWeakObject* >(new PathService()));
}

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
