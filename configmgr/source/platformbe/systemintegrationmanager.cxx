/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: systemintegrationmanager.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_configmgr.hxx"
#include "systemintegrationmanager.hxx"
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

namespace configmgr { namespace backend {


//==============================================================================
#define OU2A( oustr ) (rtl::OUStringToOString( oustr, RTL_TEXTENCODING_ASCII_US ).getStr())
//==============================================================================
uno::Reference<backenduno::XSingleLayerStratum> BackendRef::getBackend(uno::Reference<uno::XComponentContext> const & xContext)
{
    if (!mBackend.is() && mFactory.is())
    try
    {
        mBackend.set( mFactory->createInstanceWithContext(xContext), uno::UNO_QUERY_THROW );
    }
    catch(uno::Exception& e)
    {
        OSL_TRACE("SystemIntegration::getSupportingBackend - could not create platform Backend: %s",
                    OU2A(e.Message) );
    }
    return mBackend;
}
//------------------------------------------------------------------------------
void BackendRef::disposeBackend()
{
    uno::Reference< lang::XComponent> xComp( mBackend, uno::UNO_QUERY );
    if (xComp.is())
    try
    {
        xComp->dispose();
    }
    catch(uno::Exception &)
    {}
    mBackend.clear();
}
//==============================================================================
SystemIntegrationManager::SystemIntegrationManager(const uno::Reference<uno::XComponentContext>& xContext)
: cppu::WeakComponentImplHelper4< backenduno::XBackend, backenduno::XBackendChangesNotifier, lang::XInitialization, lang::XServiceInfo>(mMutex)
, mMutex()
, mContext(xContext)
, mPlatformBackends()
{
}
//------------------------------------------------------------------------------
SystemIntegrationManager::~SystemIntegrationManager()
{
}
//------------------------------------------------------------------------------
void SAL_CALL SystemIntegrationManager::initialize(
    const uno::Sequence<uno::Any>& /*aParameters*/)
    throw (uno::RuntimeException, uno::Exception,
           lang::IllegalArgumentException,
           backenduno::BackendSetupException)
{
    buildLookupTable();
}
//------------------------------------------------------------------------------
static const rtl::OUString getAllComponentsName()
{
    sal_Unicode const kStar = '*';
    return rtl::OUString(&kStar,1);
}

//------------------------------------------------------------------------------

void SystemIntegrationManager::buildLookupTable()
{
    static const rtl::OUString kPlatformServiceName(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.PlatformBackend")) ;

    //Build platform backend map componentName -> servicefactory
    uno::Reference<css::container::XContentEnumerationAccess> xEnumAccess
        (mContext->getServiceManager(),uno::UNO_QUERY_THROW);

    uno::Reference<css::container::XEnumeration> xEnum =
        xEnumAccess->createContentEnumeration(kPlatformServiceName);
    if (xEnum.is())
    {
        osl::MutexGuard lock(mMutex);
        while (xEnum->hasMoreElements())
        {
            uno::Reference<lang::XSingleComponentFactory> xServiceFactory( xEnum->nextElement(),uno::UNO_QUERY);
            if (xServiceFactory.is())
            {
                uno::Sequence<rtl::OUString> aKeys = getSupportedComponents(xServiceFactory);

                for (sal_Int32 i = 0 ; i < aKeys.getLength() ; ++i)
                {
                    BackendRef aBackendRef(xServiceFactory);
                    //OSL_TRACE("SystemInteg -Adding Factory Backend to map for key %s",
                    //rtl::OUStringToOString(aKeys[i], RTL_TEXTENCODING_ASCII_US).getStr() );
                    mPlatformBackends.insert( std::multimap<rtl::OUString, BackendRef>::value_type(aKeys[i],aBackendRef));
                }
            }
        }
    }
}
//---------------------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SystemIntegrationManager::getSupportedComponents(const uno::Reference<lang::XSingleComponentFactory>& xFactory)
{
    static const rtl::OUString kProperSubkeyName( RTL_CONSTASCII_USTRINGPARAM("/DATA/SupportedComponents")) ;
    static const rtl::OUString kImplKeyPropertyName( RTL_CONSTASCII_USTRINGPARAM("ImplementationKey")) ;

    uno::Reference<css::beans::XPropertySet> xSMProp(xFactory,uno::UNO_QUERY);
    if (xSMProp.is())
    {
        try
        {
            uno::Reference< css::registry::XRegistryKey > xImplKey(
                xSMProp->getPropertyValue(kImplKeyPropertyName), uno::UNO_QUERY);

            if (xImplKey.is())
            {
                uno::Reference< css::registry::XRegistryKey > xKey(
                    xImplKey->openKey(kProperSubkeyName));
                if(xKey.is())
                    return xKey->getAsciiListValue();
            }
        }
        catch(css::beans::UnknownPropertyException&){}
        catch(css::registry::InvalidValueException&){}
        catch(css::registry::InvalidRegistryException&){}
    }
    static const rtl::OUString kAllComponentsName = getAllComponentsName();
    return uno::Sequence<rtl::OUString>(&kAllComponentsName, 1);
}
//---------------------------------------------------------------------------------------------

uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
    SystemIntegrationManager::listOwnLayers(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
               lang::IllegalArgumentException,
               uno::RuntimeException)
{
    return listLayers(aComponent, rtl::OUString() ) ;
}
//------------------------------------------------------------------------------

uno::Reference<backenduno::XUpdateHandler> SAL_CALL
    SystemIntegrationManager::getOwnUpdateHandler(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
                lang::NoSupportException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{

    return getUpdateHandler(aComponent, rtl::OUString()) ;
}
//------------------------------------------------------------------------------

std::vector< uno::Reference<backenduno::XSingleLayerStratum> > SystemIntegrationManager::getSupportingBackends(const rtl::OUString& aComponent)
{
    std::vector< uno::Reference<backenduno::XSingleLayerStratum> > backends;

    osl::MutexGuard lock(mMutex);
    std::pair<std::multimap<rtl::OUString, BackendRef>::iterator, std::multimap<rtl::OUString, BackendRef>::iterator> aRange = mPlatformBackends.equal_range(aComponent);
    for (std::multimap<rtl::OUString, BackendRef>::iterator it=aRange.first; it != aRange.second; )
    {
        std::multimap<rtl::OUString, BackendRef>::iterator cur = it++;  // increment here, as erase() may invalidate cur
        uno::Reference<backenduno::XSingleLayerStratum> xBackend = cur->second.getBackend(mContext);
        if (xBackend.is())
            backends.push_back(xBackend);

        else  // prevent repeated attempts to create
            mPlatformBackends.erase(cur);
    }
    return backends;
}
//------------------------------------------------------------------------------
uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
    SystemIntegrationManager::listLayers(const rtl::OUString& aComponent,
                                         const rtl::OUString& /*aEntity*/)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    std::vector< uno::Reference<backenduno::XSingleLayerStratum> > const aUniversalBackends = getSupportingBackends(getAllComponentsName());
    std::vector< uno::Reference<backenduno::XSingleLayerStratum> > const aSpecialBackends  = getSupportingBackends(aComponent);

    uno::Sequence< uno::Reference<backenduno::XLayer> > aLayers(aUniversalBackends.size() + aSpecialBackends.size());

    uno::Reference<backenduno::XLayer> * pLayer = aLayers.getArray();

    for (std::vector< uno::Reference<backenduno::XSingleLayerStratum> >::size_type i=0 ; i< aUniversalBackends.size(); ++i, ++pLayer)
        *pLayer = aUniversalBackends[i]->getLayer(aComponent, rtl::OUString());

    for (std::vector< uno::Reference<backenduno::XSingleLayerStratum> >::size_type j=0 ; j< aSpecialBackends.size(); ++j, ++pLayer)
        *pLayer = aSpecialBackends[j]->getLayer(aComponent, rtl::OUString());

    OSL_ASSERT( aLayers.getConstArray()+aLayers.getLength() == pLayer );
    return aLayers;
}
//------------------------------------------------------------------------------

uno::Reference<backenduno::XUpdateHandler> SAL_CALL
    SystemIntegrationManager::getUpdateHandler(const rtl::OUString& /*aComponent*/,
                                               const rtl::OUString& /*aEntity*/)
        throw (backenduno::BackendAccessException,
                lang::NoSupportException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{

    throw lang::NoSupportException(
                rtl::OUString::createFromAscii(
                "SystemIntegrationManager: No Update Operation allowed, Read Only access"),
                *this) ;
}
// ---------------------------------------------------------------------------
// ComponentHelper
void SAL_CALL SystemIntegrationManager::disposing()
{
    osl::MutexGuard lock(mMutex);
    for (std::multimap<rtl::OUString, BackendRef>::iterator it =  mPlatformBackends.begin(); it != mPlatformBackends.end(); ++it)
        it->second.disposeBackend();

    mPlatformBackends.clear();
    mContext.clear();
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL SystemIntegrationManager::
    getSystemIntegrationManagerName(void)
{
    static const rtl::OUString kImplementationName(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.comp.configuration.backend.SystemIntegration")) ;

    return kImplementationName ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL SystemIntegrationManager::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getSystemIntegrationManagerName() ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL SystemIntegrationManager::
    getServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServices(2) ;
    aServices[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.SystemIntegration")) ;
    aServices[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.Backend")) ;

    return aServices ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL SystemIntegrationManager::supportsService(
    const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;
    return false;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL SystemIntegrationManager::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getServiceNames() ;
}
//------------------------------------------------------------------------------

void SAL_CALL SystemIntegrationManager::addChangesListener(
    const uno::Reference<backenduno::XBackendChangesListener>& xListener,
    const rtl::OUString& aComponent)
    throw (::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);

    // FIXME: we really need our own InterfaceContainer plus a helper object
    // that listens on the backends and forwards notifications

    //Simply forward listener to platform backend that support listening
    {
        std::vector< uno::Reference<backenduno::XSingleLayerStratum> > aUniversalBackends = getSupportingBackends(getAllComponentsName());
        for (sal_uInt32 i=0; i< aUniversalBackends.size(); i++)
        {
            uno::Reference<backenduno::XBackendChangesNotifier> xBackend( aUniversalBackends[i], uno::UNO_QUERY) ;
            if (xBackend.is())
                xBackend->addChangesListener(xListener, aComponent);
        }
    }
    {
        std::vector< uno::Reference<backenduno::XSingleLayerStratum> > aSpecialBackends   = getSupportingBackends(aComponent);
        for (sal_uInt32 i=0; i< aSpecialBackends.size(); i++)
        {
            uno::Reference<backenduno::XBackendChangesNotifier> xBackend( aSpecialBackends[i], uno::UNO_QUERY) ;
            if (xBackend.is())
                xBackend->addChangesListener(xListener, aComponent);
        }
    }
}
//------------------------------------------------------------------------------
void SAL_CALL SystemIntegrationManager::removeChangesListener(
    const uno::Reference<backenduno::XBackendChangesListener>& xListener,
    const rtl::OUString& aComponent)
    throw (::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);
    {
        std::vector< uno::Reference<backenduno::XSingleLayerStratum> > aUniversalBackends = getSupportingBackends(getAllComponentsName());
        for (sal_uInt32 i=0; i< aUniversalBackends.size(); i++)
        {
            uno::Reference<backenduno::XBackendChangesNotifier> xBackend( aUniversalBackends[i], uno::UNO_QUERY) ;
            if (xBackend.is())
                xBackend->removeChangesListener(xListener, aComponent);
        }
    }
    {
        std::vector< uno::Reference<backenduno::XSingleLayerStratum> > aSpecialBackends   = getSupportingBackends(aComponent);
        for (sal_uInt32 i=0; i< aSpecialBackends.size(); i++)
        {
            uno::Reference<backenduno::XBackendChangesNotifier> xBackend( aSpecialBackends[i], uno::UNO_QUERY) ;
            if (xBackend.is())
                xBackend->removeChangesListener(xListener, aComponent);
        }
    }
}
//------------------------------------------------------------------------------


} } // configmgr.backend
