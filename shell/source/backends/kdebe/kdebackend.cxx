/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: kdebackend.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_shell.hxx"

#include "kdebackend.hxx"
#include "kdecommonlayer.hxx"
#include "kdeinetlayer.hxx"
#include "kdevcllayer.hxx"
#include "kdepathslayer.hxx"

//------------------------------------------------------------------------------

KDEBackend* KDEBackend::mInstance= 0;

KDEBackend* KDEBackend::createInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    if (mInstance == 0)
    {
        mInstance = new KDEBackend (xContext);
    }

    return mInstance;
}

//------------------------------------------------------------------------------

KDEBackend::KDEBackend(const uno::Reference<uno::XComponentContext>& xContext)
        throw (backend::BackendAccessException)
    : BackendBase(mMutex), m_xContext(xContext)
{
}

//------------------------------------------------------------------------------

KDEBackend::~KDEBackend(void)
{
}

//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL KDEBackend::getLayer(
    const rtl::OUString& aComponent, const rtl::OUString& /* aTimestamp */)
    throw (backend::BackendAccessException, lang::IllegalArgumentException)
{
    uno::Reference<backend::XLayer> xLayer;

    if( aComponent.equalsAscii("org.openoffice.Office.Common" ) )
    {
        xLayer = new KDECommonLayer(m_xContext);
    }
    else if( aComponent.equalsAscii("org.openoffice.Inet" ) )
    {
        xLayer = new KDEInetLayer(m_xContext);
    }
    else if( aComponent.equalsAscii("org.openoffice.VCL" ) )
    {
        xLayer = new KDEVCLLayer(m_xContext);
    }
    else if( aComponent.equalsAscii("org.openoffice.Office.Paths" ) )
    {
        xLayer = new KDEPathsLayer(m_xContext);
    }

    return xLayer;
}

//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> SAL_CALL
KDEBackend::getUpdatableLayer(const rtl::OUString& /* aComponent */)
    throw (backend::BackendAccessException,lang::NoSupportException,
           lang::IllegalArgumentException)
{
    throw lang::NoSupportException( rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("KDEBackend: No Update Operation allowed, Read Only access") ),
        *this) ;
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL KDEBackend::getBackendName(void)
{
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.configuration.backend.KDEBackend") );
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL KDEBackend::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getBackendName() ;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL KDEBackend::getBackendServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServices(1) ;
    aServices[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.KDEBackend")) ;

    return aServices ;
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL KDEBackend::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getBackendServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;

    return false;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL KDEBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getBackendServiceNames() ;
}

// ---------------------------------------------------------------------------------------
