/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configregistry.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:29:56 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"


#ifndef _CONFIGMGR_REGISTRY_CONFIGREGISTRY_HXX_
#include "configregistry.hxx"
#endif
#ifndef _CONFIGMGR_REGISTRY_CFGREGISTRYKEY_HXX_
#include "cfgregistrykey.hxx"
#endif
#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETRUNTIMEEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#define THISREF()       static_cast< ::cppu::OWeakObject* >(this)
#define UNISTRING(c)    makeUniString(c)

//..........................................................................
namespace configmgr
{
//..........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::cppu;
using ::rtl::OUString;

namespace beans = ::com::sun::star::beans;

//==========================================================================
//= OConfigurationRegistry
//==========================================================================
    inline
    static
    OUString makeUniString(char const* c)
    {
        return OUString::createFromAscii(c);
    }


    // #99130# Don't export SimpleRegistry service
    static const AsciiServiceName aExportedConfigRegistryServices[] =
    {
        "com.sun.star.configuration.ConfigurationRegistry",
        NULL
    };
    static const AsciiServiceName aAdditionalConfigRegistryServices[] =
    {
        "com.sun.star.registry.SimpleRegistry",
        NULL
    };

    const AsciiServiceName aConfigRegistryImplementationName = "com.sun.star.comp.configuration.OConfigurationRegistry";

    const ServiceImplementationInfo OConfigurationRegistry::s_aServiceInfo =
    {
        aConfigRegistryImplementationName,
        aExportedConfigRegistryServices,
        aAdditionalConfigRegistryServices
    };

    Reference< XInterface > SAL_CALL instantiateConfigRegistry(CreationContext const& xContext )
    {
        OSL_ASSERT( xContext.is() );
        Reference< XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), UNO_QUERY );
        ::cppu::OWeakObject * pNewInstance = new OConfigurationRegistry(xServiceManager);
        return pNewInstance;
    }

    const ServiceRegistrationInfo* getConfigurationRegistryServiceInfo()
    {
        return getRegistrationInfo(& OConfigurationRegistry::s_aServiceInfo);
    }

//--------------------------------------------------------------------------
OConfigurationRegistry::OConfigurationRegistry(const Reference< XMultiServiceFactory >& _rORB) throw(Exception, RuntimeException)
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
        throw ServiceNotRegisteredException(UNISTRING("Failed to instantiate the mandatory service com.sun.star.configuration.ConfigurationProvider."),
            THISREF());
    }
}

//--------------------------------------------------------------------------
Any SAL_CALL OConfigurationRegistry::queryInterface( const Type& _rType ) throw(RuntimeException)
{
    Any aReturn = ServiceComponentImpl::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OConfigurationRegistry_Base::queryInterface(_rType);
    return aReturn;
}

//--------------------------------------------------------------------------
Sequence< Type > SAL_CALL OConfigurationRegistry::getTypes(  ) throw(RuntimeException)
{
    return ::comphelper::concatSequences(
        ServiceComponentImpl::getTypes(),
        OConfigurationRegistry_Base::getTypes());
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OConfigurationRegistry::getImplementationId(  ) throw(RuntimeException)
{
    static OImplementationId aId;
    return aId.getImplementationId();
}

//--------------------------------------------------------------------------
::rtl::OUString OConfigurationRegistry::getNodePathFromURL(const ::rtl::OUString& _rURL)
{
    // TODO
    return _rURL;
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistry::getURL() throw(RuntimeException)
{
    UnoApiLock aLock;
    return m_sLocation;
}

//--------------------------------------------------------------------------

// Not guarded !
void OConfigurationRegistry::implCheckOpen() throw(InvalidRegistryException, RuntimeException)
{
    if (!implIsOpen())
        throw InvalidRegistryException(UNISTRING("The registry is not bound to a configuration node."), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::open( const ::rtl::OUString& _rURL, sal_Bool _bReadOnly, sal_Bool /*_bCreate*/ ) throw(InvalidRegistryException, RuntimeException)
{
    UnoApiLock aLock;

    if (implIsOpen())
        close();

    ::rtl::OUString sNodePath = getNodePathFromURL(_rURL);

    if (!m_xConfigurationProvider.is())
        throw DisposedException(UNISTRING("invalid object. configuration provider is already disposed."), THISREF());

    Reference< XInterface > xNodeAccess;
    try
    {
        char const * const sAccessType = _bReadOnly ?
            "com.sun.star.configuration.ConfigurationAccess" :
            "com.sun.star.configuration.ConfigurationUpdateAccess";

        // prepare parameters for creating the config access : the node path
        beans::PropertyValue aArgValue;
        aArgValue.Handle = -1;

        // currently theres is one parameter: the node path
        Sequence< Any > aArguments(1);

        // Argumenbt: NodePath
        aArgValue.Name = UNISTRING("nodepath");
        aArgValue.Value <<= sNodePath;

        aArguments[0] <<= aArgValue;


        xNodeAccess = m_xConfigurationProvider->createInstanceWithArguments(UNISTRING(sAccessType), aArguments);
    }
    catch (RuntimeException&)
    {   // allowed to leave this method
        throw;
    }
    catch (Exception& e)
    {   // not allowed to leave this method
        ::rtl::OUString sMessage = UNISTRING("The configuration provider does not supply a registry access for the requested Node.");
        sMessage += UNISTRING(" original error message of the provider : ");
        sMessage += e.Message;
        throw InvalidRegistryException(sMessage, THISREF());
    }

    Reference< XNameAccess > xReadRoot(xNodeAccess, UNO_QUERY);
    if (!_bReadOnly)
        m_xUpdateRoot = m_xUpdateRoot.query(xReadRoot);

    if (!xReadRoot.is() || (!_bReadOnly && !m_xUpdateRoot.is()))
        throw InvalidRegistryException(UNISTRING("The object supplied the by configuration provider is invalid."), THISREF());

    m_xRootKey = new OConfigurationRegistryKey(xReadRoot, !_bReadOnly, OConfigurationRegistryKey::SubtreeRoot());
    m_xSubtreeRoot = xNodeAccess;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistry::isValid(  ) throw(RuntimeException)
{
    UnoApiLock aLock;
    return implIsOpen();
}

//--------------------------------------------------------------------------
sal_Bool OConfigurationRegistry::implIsOpen(  ) throw(RuntimeException)
{
    return m_xRootKey.is();
}

//--------------------------------------------------------------------------
void SAL_CALL  OConfigurationRegistry::close(  ) throw(InvalidRegistryException, RuntimeException)
{
    UnoApiLock aLock;

    Reference< XRegistryKey > xRootKey(m_xRootKey);
    m_xRootKey = NULL;

    Reference< XComponent > xRootComponent(m_xSubtreeRoot, UNO_QUERY);
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
void SAL_CALL OConfigurationRegistry::destroy(  ) throw(InvalidRegistryException, RuntimeException)
{
    UnoApiLock aLock;
    implCheckOpen();

    throw InvalidRegistryException(UNISTRING("This registry is a wrapper for a configuration access. It can not be destroyed."), THISREF());
}

//--------------------------------------------------------------------------
Reference< XRegistryKey > SAL_CALL OConfigurationRegistry::getRootKey(  ) throw(InvalidRegistryException, RuntimeException)
{
    UnoApiLock aLock;
    implCheckOpen();

    return m_xRootKey;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistry::isReadOnly(  ) throw(InvalidRegistryException, RuntimeException)
{
    UnoApiLock aLock;
    implCheckOpen();

    return !m_xUpdateRoot.is();
        // if we don't have the update root, we're readonly
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::mergeKey( const ::rtl::OUString& /*aKeyName*/, const ::rtl::OUString& /*aUrl*/ ) throw(InvalidRegistryException, MergeConflictException, RuntimeException)
{
    UnoApiLock aLock;
    implCheckOpen();

    // not supported. but we can't throw an NoSupportException here ...
    throw InvalidRegistryException(UNISTRING("You can't merge into this registry. It's just a wrapper for a configuration node, which has a fixed structure which can not be modified"), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::flush(  ) throw(RuntimeException)
{
    {
        UnoApiLock aLock;
        if (m_xUpdateRoot.is())
        {
            try
            {
                m_xUpdateRoot->commitChanges();
            }
            catch (WrappedTargetException& e)
            {   // not allowed to leave this method

                ::rtl::OUString sMessage;
                sMessage = UNISTRING("The changes made could not be committed. Orginal exception message : ");
                sMessage += e.Message;

                // TODO : the specification of XFlushable has to be changed !!!!!
                OSL_ENSURE(sal_False, "OConfigurationRegistry::flush : caught an exception, could not flush the data !");
    //          return;

                throw WrappedTargetRuntimeException(sMessage, THISREF(), e.TargetException);
            }
        }
    }

    Reference< XFlushListener > const * const pSelector = 0;
    if (OInterfaceContainerHelper* pContainer = this->rBHelper.getContainer(::getCppuType(pSelector)) )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        EventObject aFlushed(THISREF());
        while (aIter.hasMoreElements())
        try
        {
            static_cast< XFlushListener* >(aIter.next())->flushed(aFlushed);
        }
        catch (uno::Exception & )
        {}
    }
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::addFlushListener( const Reference< XFlushListener >& _rxListener ) throw(RuntimeException)
{
    this->rBHelper.addListener(::getCppuType(&_rxListener),_rxListener);
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistry::removeFlushListener( const Reference< XFlushListener >& _rxListener ) throw(RuntimeException)
{
    this->rBHelper.removeListener(::getCppuType(&_rxListener),_rxListener);
}

//..........................................................................
}   // namespace configmgr
//..........................................................................


