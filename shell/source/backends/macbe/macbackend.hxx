/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macbackend.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:56:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _MACBACKEND_HXX_
#define _MACBACKEND_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSINGLELAYERSTRATUM_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDCHANGESNOTIFIER_HPP_
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

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
