/*************************************************************************
 *
 *  $RCSfile: systemintegrationmanager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 14:36:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_
#include "systemintegrationmanager.hxx"
#endif // CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

namespace configmgr { namespace backend {


//==============================================================================
//------------------------------------------------------------------------------
SystemIntegrationManager::SystemIntegrationManager(
        const uno::Reference<uno::XComponentContext>& xContext)
        :BackendBase(mMutex),mContext(xContext)

{
}
//------------------------------------------------------------------------------
SystemIntegrationManager::~SystemIntegrationManager()
{
}
//------------------------------------------------------------------------------
void SAL_CALL SystemIntegrationManager::initialize(
        const uno::Sequence<uno::Any>& aParameters)
    throw (uno::RuntimeException, uno::Exception,
           lang::IllegalArgumentException,
           backenduno::BackendSetupException)
{
    buildPlatformBeLookupTable();
}
//------------------------------------------------------------------------------
static const rtl::OUString kPlatformServiceName(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.PlatformBackend")) ;

static const rtl::OUString kAllComponentNames(
            RTL_CONSTASCII_USTRINGPARAM("*")) ;



void SystemIntegrationManager::buildPlatformBeLookupTable()
{
    //Build platform backend map componentName -> servicefactory
    uno::Reference<css::container::XContentEnumerationAccess> xEnumAccess
        (mContext->getServiceManager(),uno::UNO_QUERY_THROW);

    uno::Reference<css::container::XEnumeration> xEnum =
        xEnumAccess->createContentEnumeration(kPlatformServiceName);
    if (xEnum.is())
    {
        while (xEnum->hasMoreElements())
        {
            BackendFactory xServiceFactory( xEnum->nextElement(),uno::UNO_QUERY);
            if (xServiceFactory.is())
            {

                uno::Sequence<rtl::OUString> aKeys = getSupportedComponents
                    (xServiceFactory);

                for (sal_Int32 i = 0 ; i < aKeys.getLength() ; ++i)
                {
                    BackendRefs aBackendRef(xServiceFactory,NULL);
                    rtl::OUString aName = aKeys[i];
                    //OSL_TRACE("SystemInteg -Adding Factory Backend to map for key %s",
                    //rtl::OUStringToOString(aKeys[i], RTL_TEXTENCODING_ASCII_US).getStr() );
                    mPlatformBackends.insert
                        ( BackendFactoryList::value_type(aKeys[i],aBackendRef));

                }
            }
        }
    }
}
//---------------------------------------------------------------------------------------------
static const rtl::OUString kProperSubkeyName(
            RTL_CONSTASCII_USTRINGPARAM("/DATA/SupportedComponents")) ;


uno::Sequence<rtl::OUString>
SystemIntegrationManager::getSupportedComponents(const BackendFactory& xFactory)
{
    uno::Reference<css::beans::XPropertySet> xSMProp(xFactory,uno::UNO_QUERY);
    if (xSMProp.is())
    {
        try
        {
            uno::Reference< css::registry::XRegistryKey > xImplKey(
                xSMProp->getPropertyValue(rtl::OUString::createFromAscii
                ("ImplementationKey")), uno::UNO_QUERY);

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
    return uno::Sequence<rtl::OUString>(&kAllComponentNames, 1);
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

std::vector<XSingleLayerStratum> SystemIntegrationManager::getSupportingBackends(
    const rtl::OUString& aComponent)
{
    typedef BackendFactoryList::iterator BFIter;
    typedef std::pair<BFIter, BFIter> BFRange;
    std::vector<XSingleLayerStratum> backends;
    BFRange aRange = mPlatformBackends.equal_range(aComponent);

    while (aRange.first != aRange.second)
    {
        BFIter cur = aRange.first++;
        //Check if we have valid XSingleLayerStratum Reference first
        XSingleLayerStratum xBackend (cur->second.mBackend, uno::UNO_QUERY);
        if (xBackend.is())
        {
            backends.push_back(xBackend);
        }
        else
        {
            BackendFactory xSingleServiceFactory (cur->second.mFactory, uno::UNO_QUERY);

            if (xSingleServiceFactory.is())
            {

                try{
                    xBackend = XSingleLayerStratum::query(
                        xSingleServiceFactory->createInstanceWithContext(mContext));
                     if (xBackend.is())
                     {

                         cur->second.mBackend = xBackend;
                         backends.push_back(xBackend);
                     }
                }
                catch(uno::Exception& e)
                {
                    OSL_TRACE("SystemIntegration::getSupportingBackend - could not create platform Backend");

                }

            }
        }
    }
    return backends;
}
//------------------------------------------------------------------------------
uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
    SystemIntegrationManager::listLayers(const rtl::OUString& aComponent,
                                         const rtl::OUString& aEntity)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{

    std::vector<XSingleLayerStratum> backends = getSupportingBackends(aComponent);

    std::vector< uno::Reference<backenduno::XLayer> > xLayers;

    sal_Int32 nLoopCount = 0;
    do
    {
        if (!backends.empty())
        {
            for (sal_uInt32 i=0; i< backends.size(); i++)
            {
                xLayers.push_back( backends[i]->getLayer(aComponent, rtl::OUString()));

            }
        }
        //Get backend that support ALL(*) components
        if(nLoopCount != 1)
        {
            backends = getSupportingBackends(kAllComponentNames);
        }
        nLoopCount++;
     }while (nLoopCount <= 1);


    uno::Sequence<uno::Reference<backenduno::XLayer> > xRetLayers(xLayers.size());
    for (sal_uInt32 j=0; j < xLayers.size(); ++j)
    {
        xRetLayers[j] = xLayers[j];
    }

    return xRetLayers;
}
//------------------------------------------------------------------------------

uno::Reference<backenduno::XUpdateHandler> SAL_CALL
    SystemIntegrationManager::getUpdateHandler(const rtl::OUString& aComponent,
                                          const rtl::OUString& aEntity)
        throw (backenduno::BackendAccessException,
                lang::NoSupportException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{

    throw lang::NoSupportException(
                rtl::OUString::createFromAscii(
                "SystemIntegrationManager: No Update Operation allowed, Read Only access"),
                *this) ;
    return NULL;

}
// ---------------------------------------------------------------------------
// ComponentHelper
void SAL_CALL SystemIntegrationManager::disposing()
{
     if (!mPlatformBackends.empty())
    {
        for (BackendFactoryList::const_iterator it =  mPlatformBackends.begin(); it != mPlatformBackends.end(); ++it)
        {
            XSingleLayerStratum xBackend (it->second.mBackend, uno::UNO_QUERY);
            if (xBackend.is())
            {
                uno::Reference<  lang::XComponent> xComp( xBackend, uno::UNO_QUERY );
                if (xComp.is())
                {
                    try
                    {
                        xComp->dispose();
                    }
                    catch(uno::Exception &){}
                }
            }
        }

    }

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
    //Simply forward listener to platform backend that support listening
    std::vector<XSingleLayerStratum> backends = getSupportingBackends(aComponent);
    sal_Int32 nLoopCount = 0;

    do
    {
        for (sal_uInt32 i=0; i< backends.size(); i++)
        {
            uno::Reference<backenduno::XBackendChangesNotifier> xBackend( backends[i],
                uno::UNO_QUERY) ;
            if (xBackend.is())
            {
                xBackend->addChangesListener(xListener, aComponent);
            }
        }
        //Get backends that support ALL(*) components
        backends = getSupportingBackends(kAllComponentNames);
        nLoopCount++;
    }while (nLoopCount <= 1);
}
//------------------------------------------------------------------------------
void SAL_CALL SystemIntegrationManager::removeChangesListener(
    const uno::Reference<backenduno::XBackendChangesListener>& xListener,
    const rtl::OUString& aComponent)
    throw (::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);
    std::vector<XSingleLayerStratum> backends = getSupportingBackends(aComponent);
    sal_Int32 nLoopCount = 0;

    do
    {
        for (sal_uInt32 i=0; i< backends.size(); i++)
        {
            uno::Reference<backenduno::XBackendChangesNotifier> xBackend( backends[i],
                uno::UNO_QUERY) ;
            if (xBackend.is())
            {
                xBackend->removeChangesListener(xListener, aComponent);
            }
        }
        //Get backends that support ALL(*) components
        backends = getSupportingBackends(kAllComponentNames);
        nLoopCount++;
    }while (nLoopCount <= 1);
}
//------------------------------------------------------------------------------


} } // configmgr.backend
