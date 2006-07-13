/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kdebackend.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 12:31:31 $
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

#include "kdebackend.hxx"

#ifndef KDECOMMONLAYER_HXX_
#include "kdecommonlayer.hxx"
#endif
#ifndef KDEINETLAYER_HXX_
#include "kdeinetlayer.hxx"
#endif
#ifndef KDEVCLLAYER_HXX_
#include "kdevcllayer.hxx"
#endif
#ifndef KDEPATHSLAYER_HXX_
#include "kdepathslayer.hxx"
#endif

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
    uno::Sequence<rtl::OUString> aServices(2) ;
    aServices[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.KDEBackend")) ;
    aServices[1] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.PlatformBackend")) ;

    return aServices ;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL KDEBackend::getSupportedComponents(void)
{
    uno::Sequence<rtl::OUString> aSupportedComponentsList(4) ;
    aSupportedComponentsList[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.VCL")) ;
    aSupportedComponentsList[1] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Inet")) ;
    aSupportedComponentsList[2] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Common")) ;
    aSupportedComponentsList[3] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Paths")) ;

    return aSupportedComponentsList ;
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
