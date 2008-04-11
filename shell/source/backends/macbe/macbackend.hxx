/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: macbackend.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _MACBACKEND_HXX_
#define _MACBACKEND_HXX_

#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#include <cppuhelper/implbase2.hxx>
#include <rtl/string.hxx>

// FIXME: stdio.h only for debugging...
#include <stdio.h>

namespace css = com::sun::star;
namespace uno = css::uno;
namespace lang = css::lang;
namespace backend = css::configuration::backend;


/**
  Implements the SingleLayerStratum service.
  */
class MacOSXBackend : public ::cppu::WeakImplHelper2 <backend::XSingleLayerStratum, lang::XServiceInfo >
{

public:

    static MacOSXBackend* createInstance(const uno::Reference<uno::XComponentContext>& xContext);

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& aServiceName)
        throw (uno::RuntimeException);

    virtual uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException);

    /**
       Provides the implementation name.

       @return   implementation name
    */
    static rtl::OUString SAL_CALL getBackendName(void);

    /**
       Provides the supported services names

       @return   service names
    */
    static uno::Sequence<rtl::OUString> SAL_CALL getBackendServiceNames(void);

    /**
       Provides the supported component nodes

       @return supported component nodes
    */
    static uno::Sequence<rtl::OUString> SAL_CALL getSupportedComponents(void);

    // XSingleLayerStratum
    virtual uno::Reference<backend::XLayer> SAL_CALL getLayer(const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp)
        throw (backend::BackendAccessException, lang::IllegalArgumentException);

    virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL getUpdatableLayer(const rtl::OUString& aLayerId)
        throw (backend::BackendAccessException, lang::NoSupportException, lang::IllegalArgumentException);

protected:

    /**
       Service constructor from a service factory.

       @param xContext   component context
    */
    MacOSXBackend(const uno::Reference<uno::XComponentContext>& xContext)
        throw (backend::BackendAccessException);

    /** Destructor */
    ~MacOSXBackend(void);

private:

    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<backend::XLayer> m_xSystemLayer;
    uno::Reference<backend::XLayer> m_xPathLayer;

};

#endif // _MACBACKEND_HXX_
