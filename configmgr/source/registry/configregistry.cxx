/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configregistry.cxx,v $
 * $Revision: 1.13 $
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


#include "configregistry.hxx"
#include "cfgregistrykey.hxx"
#include "confapifactory.hxx"
#include "datalock.hxx"
#include "utility.hxx"
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#define THISREF()       static_cast< ::cppu::OWeakObject* >(this)
#define UNISTRING(c)    makeUniString(c)

//..........................................................................
namespace configmgr
{
//..........................................................................

namespace beans = ::com::sun::star::beans;

//==========================================================================
//= OConfigurationRegistry
//==========================================================================
    inline
    static
    rtl::OUString makeUniString(char const* c)
    {
        return rtl::OUString::createFromAscii(c);
    }


    // #99130# Don't export SimpleRegistry service
    static sal_Char const * const aExportedConfigRegistryServices[] =
    {
        "com.sun.star.configuration.ConfigurationRegistry",
        NULL
    };
    static sal_Char const * const aAdditionalConfigRegistryServices[] =
    {
        "com.sun.star.registry.SimpleRegistry",
        NULL
    };

    sal_Char const * const aConfigRegistryImplementationName = "com.sun.star.comp.configuration.OConfigurationRegistry";

    const ServiceImplementationInfo OConfigurationRegistry::s_aServiceInfo =
    {
        aConfigRegistryImplementationName,
        aExportedConfigRegistryServices,
        aAdditionalConfigRegistryServices
    };

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL instantiateConfigRegistry(uno::Reference< uno::XComponentContext > const& xContext )
    {
        OSL_ASSERT( xContext.is() );
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), com::sun::star::uno::UNO_QUERY );
        ::cppu::OWeakObject * pNewInstance = new OConfigurationRegistry(xServiceManager);
        return pNewInstance;
    }

    const ServiceRegistrationInfo* getConfigurationRegistryServiceInfo()
    {
        return getRegistrationInfo(& OConfigurationRegistry::s_aServiceInfo);
    }

//--------------------------------------------------------------------------
OConfigurationRegistry::OConfigurationRegistry(const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& _rORB) throw(com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
    :ServiceComponentImpl(&s_aServiceInfo)
    ,m_xORB(_rORB)
{
    // create the configuration provider used for accessing the configuration
    OSL_ENSURE(m_xORB.is(), "OConfigurationRegistry::OConfigurationRegistry : invalid service factory !");
    if (m_xORB.is())
    {
        m_xConfigurationProvider =
            m_xConfigurationProvider.query(
                m_xORB->createInstance(UNISTRING("com.sun.star.configuration.ConfigurationProvider"))
            );
    }

    if (!m_xConfigurationProvider.is())
    {
        // it's heavily needed ...
        throw com::sun::star::lang::ServiceNotRegisteredException(UNISTRING("Failed to instantiate the mandatory service com.sun.star.configuration.ConfigurationProvider."),
            THISREF());
    }
}

//--------------------------------------------------------------------------
com::sun::star::uno::Any SAL_CALL OConfigurationRegistry::queryInterface( const com::sun::star::uno::Type& _rType ) throw(com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aReturn = ServiceComponentImpl::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = cppu::ImplHelper2< com::sun::star::registry::XSimpleRegistry, com::sun::star::util::XFlushable >::queryInterface(_rType);
    return aReturn;
}

//--------------------------------------------------------------------------
com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL OConfigurationRegistry::getTypes(  ) throw(com::sun::star::uno::RuntimeException)
{
    return ::comphelper::concatSequences(
        ServiceComponentImpl::getTypes(),
        cppu::ImplHelper2< com::sun::star::registry::XSimpleRegistry,   com::sun::star::util::XFlushable >::getTypes());
}

//--------------------------------------------------------------------------
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL OConfigurationRegistry::getImplementationId(  ) throw(com::sun::star::uno::RuntimeException)
{
    static cppu::OImplementationId aId;
    return aId.getImplementationId();
}

//--------------------------------------------------------------------------
::rtl::OUString OConfigurationRegistry::getNodePathFromURL(const ::rtl::OUString& _rURL)
{
    // TODO
    return _rURL;
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistry::getURL() throw(com::sun::star::uno::RuntimeException)
{
    UnoApiLock aLock;
    return m_sLocation;
}

//--------------------------------------------------------------------------

// Not guarded !
void OConfigurationRegistry::implCheckOpen() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    if (!implIsOpen())
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The registry is not bound to a configuration node."), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::open( const ::rtl::OUString& _rURL, sal_Bool _bReadOnly, sal_Bool /*_bCreate*/ ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    UnoApiLock aLock;

    if (implIsOpen())
        close();

    ::rtl::OUString sNodePath = getNodePathFromURL(_rURL);

    if (!m_xConfigurationProvider.is())
        throw com::sun::star::lang::DisposedException(UNISTRING("invalid object. configuration provider is already disposed."), THISREF());

    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xNodeAccess;
    try
    {
        char const * const sAccessType = _bReadOnly ?
            "com.sun.star.configuration.ConfigurationAccess" :
            "com.sun.star.configuration.ConfigurationUpdateAccess";

        // prepare parameters for creating the config access : the node path
        beans::PropertyValue aArgValue;
        aArgValue.Handle = -1;

        // currently theres is one parameter: the node path
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > aArguments(1);

        aArgValue.Name = UNISTRING("nodepath");
        aArgValue.Value <<= sNodePath;

        aArguments[0] <<= aArgValue;


        xNodeAccess = m_xConfigurationProvider->createInstanceWithArguments(UNISTRING(sAccessType), aArguments);
    }
    catch (com::sun::star::uno::RuntimeException&)
    {   // allowed to leave this method
        throw;
    }
    catch (com::sun::star::uno::Exception& e)
    {   // not allowed to leave this method
        ::rtl::OUString sMessage = UNISTRING("The configuration provider does not supply a registry access for the requested Node.");
        sMessage += UNISTRING(" original error message of the provider : ");
        sMessage += e.Message;
        throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
    }

    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xReadRoot(xNodeAccess, com::sun::star::uno::UNO_QUERY);
    if (!_bReadOnly)
        m_xUpdateRoot = m_xUpdateRoot.query(xReadRoot);

    if (!xReadRoot.is() || (!_bReadOnly && !m_xUpdateRoot.is()))
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The object supplied the by configuration provider is invalid."), THISREF());

    m_xRootKey = new OConfigurationRegistryKey(xReadRoot, !_bReadOnly, OConfigurationRegistryKey::SubtreeRoot());
    m_xSubtreeRoot = xNodeAccess;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistry::isValid(  ) throw(com::sun::star::uno::RuntimeException)
{
    UnoApiLock aLock;
    return implIsOpen();
}

//--------------------------------------------------------------------------
sal_Bool OConfigurationRegistry::implIsOpen(  ) throw(com::sun::star::uno::RuntimeException)
{
    return m_xRootKey.is();
}

//--------------------------------------------------------------------------
void SAL_CALL  OConfigurationRegistry::close(  ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    UnoApiLock aLock;

    com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > xRootKey(m_xRootKey);
    m_xRootKey = NULL;

    com::sun::star::uno::Reference< XComponent > xRootComponent(m_xSubtreeRoot, com::sun::star::uno::UNO_QUERY);
    m_xSubtreeRoot = NULL;
    m_xUpdateRoot = NULL;

    m_sLocation = ::rtl::OUString();

    if (xRootKey.is())
        xRootKey->closeKey();

    if (xRootComponent.is())
        xRootComponent->dispose();
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::disposing()
{
    close();

    {
        UnoApiLock aLock;

        m_xConfigurationProvider.clear();
        m_xORB.clear();
    }

    ServiceComponentImpl::disposing();
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::destroy(  ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    UnoApiLock aLock;
    implCheckOpen();

    throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This registry is a wrapper for a configuration access. It can not be destroyed."), THISREF());
}

//--------------------------------------------------------------------------
com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > SAL_CALL OConfigurationRegistry::getRootKey(  ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    UnoApiLock aLock;
    implCheckOpen();

    return m_xRootKey;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistry::isReadOnly(  ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    UnoApiLock aLock;
    implCheckOpen();

    return !m_xUpdateRoot.is();
        // if we don't have the update root, we're readonly
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::mergeKey( const ::rtl::OUString& /*aKeyName*/, const ::rtl::OUString& /*aUrl*/ ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::registry::MergeConflictException, com::sun::star::uno::RuntimeException)
{
    UnoApiLock aLock;
    implCheckOpen();

    // not supported. but we can't throw an NoSupportException here ...
    throw com::sun::star::registry::InvalidRegistryException(UNISTRING("You can't merge into this registry. It's just a wrapper for a configuration node, which has a fixed structure which can not be modified"), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::flush(  ) throw(com::sun::star::uno::RuntimeException)
{
    {
        UnoApiLock aLock;
        if (m_xUpdateRoot.is())
        {
            try
            {
                m_xUpdateRoot->commitChanges();
            }
            catch (com::sun::star::lang::WrappedTargetException& e)
            {   // not allowed to leave this method

                ::rtl::OUString sMessage;
                sMessage = UNISTRING("The changes made could not be committed. Orginal exception message : ");
                sMessage += e.Message;

                // TODO : the specification of XFlushable has to be changed !!!!!
                OSL_ENSURE(sal_False, "OConfigurationRegistry::flush : caught an exception, could not flush the data !");
    //          return;

                throw com::sun::star::lang::WrappedTargetRuntimeException(sMessage, THISREF(), e.TargetException);
            }
        }
    }

    com::sun::star::uno::Reference< com::sun::star::util::XFlushListener > const * const pSelector = 0;
    if (cppu::OInterfaceContainerHelper* pContainer = this->rBHelper.getContainer(::getCppuType(pSelector)) )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        com::sun::star::lang::EventObject aFlushed(THISREF());
        while (aIter.hasMoreElements())
        try
        {
            static_cast< com::sun::star::util::XFlushListener* >(aIter.next())->flushed(aFlushed);
        }
        catch (uno::Exception & )
        {}
    }
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::addFlushListener( const com::sun::star::uno::Reference< com::sun::star::util::XFlushListener >& _rxListener ) throw(com::sun::star::uno::RuntimeException)
{
    this->rBHelper.addListener(::getCppuType(&_rxListener),_rxListener);
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::removeFlushListener( const com::sun::star::uno::Reference< com::sun::star::util::XFlushListener >& _rxListener ) throw(com::sun::star::uno::RuntimeException)
{
    this->rBHelper.removeListener(::getCppuType(&_rxListener),_rxListener);
}

//..........................................................................
}   // namespace configmgr
//..........................................................................


