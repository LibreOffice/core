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

#include "uiconfiguration/uicategorydescription.hxx"
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include "properties.h"

#include "helper/mischelper.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/implbase2.hxx>
#include <unotools/configmgr.hxx>

#include <vcl/mnemonic.hxx>
#include <comphelper/sequence.hxx>
#include <rtl/logfile.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________

struct ModuleToCategory
{
    const char* pModuleId;
    const char* pCommands;
};

static const char GENERIC_MODULE_NAME[]                     = "generic";
static const char CONFIGURATION_ROOT_ACCESS[]               = "/org.openoffice.Office.UI.";
static const char CONFIGURATION_CATEGORY_ELEMENT_ACCESS[]   = "/Commands/Categories";
static const char CONFIGURATION_PROPERTY_NAME[]             = "Name";

namespace framework
{

//*****************************************************************************************************************
//  Configuration access class for PopupMenuControllerFactory implementation
//*****************************************************************************************************************

class ConfigurationAccess_UICategory : // Order is neccessary for right initialization!
                                        private ThreadHelpBase                           ,
                                        public  ::cppu::WeakImplHelper2<XNameAccess,XContainerListener>
{
    public:
                                  ConfigurationAccess_UICategory( const ::rtl::OUString& aModuleName, const Reference< XNameAccess >& xGenericUICommands, const Reference< XMultiServiceFactory >& rServiceManager );
        virtual                   ~ConfigurationAccess_UICategory();

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
            throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasElements()
            throw (::com::sun::star::uno::RuntimeException);

        // container.XContainerListener
        virtual void SAL_CALL     elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException);
        virtual void SAL_CALL     elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException);
        virtual void SAL_CALL     elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException);

        // lang.XEventListener
        virtual void SAL_CALL disposing( const EventObject& aEvent ) throw(RuntimeException);

    protected:
        Any                       getUINameFromID( const rtl::OUString& rId );
        Any                       getUINameFromCache( const rtl::OUString& rId );
        Sequence< rtl::OUString > getAllIds();
        sal_Bool                  fillCache();

    private:
        typedef ::boost::unordered_map< ::rtl::OUString,
                                 ::rtl::OUString,
                                 OUStringHashCode,
                                 ::std::equal_to< ::rtl::OUString > > IdToInfoCache;

        sal_Bool initializeConfigAccess();

        rtl::OUString                     m_aConfigCategoryAccess;
        rtl::OUString                     m_aPropUIName;
        Reference< XNameAccess >          m_xGenericUICategories;
        Reference< XMultiServiceFactory > m_xServiceManager;
        Reference< XMultiServiceFactory > m_xConfigProvider;
        Reference< XNameAccess >          m_xConfigAccess;
        Reference< XContainerListener >   m_xConfigListener;
        sal_Bool                          m_bConfigAccessInitialized;
        sal_Bool                          m_bCacheFilled;
        IdToInfoCache                     m_aIdCache;
};

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************

ConfigurationAccess_UICategory::ConfigurationAccess_UICategory( const rtl::OUString& aModuleName, const Reference< XNameAccess >& rGenericUICategories, const Reference< XMultiServiceFactory >& rServiceManager ) :
    ThreadHelpBase(),
    m_aConfigCategoryAccess( CONFIGURATION_ROOT_ACCESS ),
    m_aPropUIName( CONFIGURATION_PROPERTY_NAME ),
    m_xGenericUICategories( rGenericUICategories ),
    m_xServiceManager( rServiceManager ),
    m_bConfigAccessInitialized( sal_False ),
    m_bCacheFilled( sal_False )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::ConfigurationAccess_UICategory" );
    // Create configuration hierachical access name
    m_aConfigCategoryAccess += aModuleName;
    m_aConfigCategoryAccess += rtl::OUString( CONFIGURATION_CATEGORY_ELEMENT_ACCESS );

    m_xConfigProvider = Reference< XMultiServiceFactory >( rServiceManager->createInstance(SERVICENAME_CFGPROVIDER),UNO_QUERY );
}

ConfigurationAccess_UICategory::~ConfigurationAccess_UICategory()
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigListener);
}

// XNameAccess
Any SAL_CALL ConfigurationAccess_UICategory::getByName( const ::rtl::OUString& rId )
throw ( NoSuchElementException, WrappedTargetException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::getByName" );
    ResetableGuard aLock( m_aLock );
    if ( !m_bConfigAccessInitialized )
    {
        initializeConfigAccess();
        m_bConfigAccessInitialized = sal_True;
        fillCache();
    }

    // SAFE
    Any a = getUINameFromID( rId );

    if ( !a.hasValue() )
        throw NoSuchElementException();

    return a;
}

Sequence< ::rtl::OUString > SAL_CALL ConfigurationAccess_UICategory::getElementNames()
throw ( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::getElementNames" );
    return getAllIds();
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasByName( const ::rtl::OUString& rId )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::hasByName" );
    return getByName( rId ).hasValue();
}

// XElementAccess
Type SAL_CALL ConfigurationAccess_UICategory::getElementType()
throw ( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::getElementType" );
    return( ::getCppuType( (const rtl::OUString*)NULL ) );
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasElements()
throw ( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::hasElements" );
    // There must be global categories!
    return sal_True;
}

sal_Bool ConfigurationAccess_UICategory::fillCache()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::fillCache" );
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ConfigurationAccess_UICategory::fillCache" );

    if ( m_bCacheFilled )
        return sal_True;

    sal_Int32            i( 0 );
    rtl::OUString        aUIName;
    Sequence< ::rtl::OUString > aNameSeq = m_xConfigAccess->getElementNames();

    for ( i = 0; i < aNameSeq.getLength(); i++ )
    {
        try
        {
            Reference< XNameAccess > xNameAccess(m_xConfigAccess->getByName( aNameSeq[i] ),UNO_QUERY);
            if ( xNameAccess.is() )
            {
                xNameAccess->getByName( m_aPropUIName ) >>= aUIName;

                m_aIdCache.insert( IdToInfoCache::value_type( aNameSeq[i], aUIName ));
            }
        }
        catch ( const com::sun::star::lang::WrappedTargetException& )
        {
        }
        catch ( const com::sun::star::container::NoSuchElementException& )
        {
        }
    }

    m_bCacheFilled = sal_True;

    return sal_True;
}

Any ConfigurationAccess_UICategory::getUINameFromID( const rtl::OUString& rId )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::getUINameFromID" );
    Any a;

    try
    {
        a = getUINameFromCache( rId );
        if ( !a.hasValue() )
        {
            // Try to ask our global commands configuration access
            if ( m_xGenericUICategories.is() )
            {
                try
                {
                    return m_xGenericUICategories->getByName( rId );
                }
                catch ( const com::sun::star::lang::WrappedTargetException& )
                {
                }
                catch ( const com::sun::star::container::NoSuchElementException& )
                {
                }
            }
        }
    }
    catch( const com::sun::star::container::NoSuchElementException& )
    {
    }
    catch ( const com::sun::star::lang::WrappedTargetException& )
    {
    }

    return a;
}

Any ConfigurationAccess_UICategory::getUINameFromCache( const rtl::OUString& rId )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::getUINameFromCache" );
    Any a;

    IdToInfoCache::const_iterator pIter = m_aIdCache.find( rId );
    if ( pIter != m_aIdCache.end() )
        a <<= pIter->second;

    return a;
}

Sequence< rtl::OUString > ConfigurationAccess_UICategory::getAllIds()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::getAllIds" );
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigAccessInitialized )
    {
        initializeConfigAccess();
        m_bConfigAccessInitialized = sal_True;
        fillCache();
    }

    if ( m_xConfigAccess.is() )
    {
        Any                      a;
        Reference< XNameAccess > xNameAccess;

        try
        {
            Sequence< ::rtl::OUString > aNameSeq = m_xConfigAccess->getElementNames();

            if ( m_xGenericUICategories.is() )
            {
                // Create concat list of supported user interface commands of the module
                Sequence< ::rtl::OUString > aGenericNameSeq = m_xGenericUICategories->getElementNames();
                sal_uInt32 nCount1 = aNameSeq.getLength();
                sal_uInt32 nCount2 = aGenericNameSeq.getLength();

                aNameSeq.realloc( nCount1 + nCount2 );
                ::rtl::OUString* pNameSeq = aNameSeq.getArray();
                const ::rtl::OUString* pGenericSeq = aGenericNameSeq.getConstArray();
                for ( sal_uInt32 i = 0; i < nCount2; i++ )
                    pNameSeq[nCount1+i] = pGenericSeq[i];
            }

            return aNameSeq;
        }
        catch( const com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( const com::sun::star::lang::WrappedTargetException& )
        {
        }
    }

    return Sequence< rtl::OUString >();
}

sal_Bool ConfigurationAccess_UICategory::initializeConfigAccess()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::initializeConfigAccess" );
    Sequence< Any > aArgs( 1 );
    PropertyValue   aPropValue;

    try
    {
        aPropValue.Name  = rtl::OUString( "nodepath" );
        aPropValue.Value <<= m_aConfigCategoryAccess;
        aArgs[0] <<= aPropValue;

        m_xConfigAccess = Reference< XNameAccess >( m_xConfigProvider->createInstanceWithArguments(SERVICENAME_CFGREADACCESS,aArgs ),UNO_QUERY );
        if ( m_xConfigAccess.is() )
        {
            // Add as container listener
            Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
            if ( xContainer.is() )
            {
                m_xConfigListener = new WeakContainerListener(this);
                xContainer->addContainerListener(m_xConfigListener);
            }
        }

        return sal_True;
    }
    catch ( const WrappedTargetException& )
    {
    }
    catch ( const Exception& )
    {
    }

    return sal_False;
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_UICategory::elementInserted( const ContainerEvent& ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::elementInserted" );
}

void SAL_CALL ConfigurationAccess_UICategory::elementRemoved ( const ContainerEvent& ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::elementRemoved " );
}

void SAL_CALL ConfigurationAccess_UICategory::elementReplaced( const ContainerEvent& ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::elementReplaced" );
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_UICategory::disposing( const EventObject& aEvent ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_UICategory::disposing" );
    // SAFE
    // remove our reference to the config access
    ResetableGuard aLock( m_aLock );

    Reference< XInterface > xIfac1( aEvent.Source, UNO_QUERY );
    Reference< XInterface > xIfac2( m_xConfigAccess, UNO_QUERY );
    if ( xIfac1 == xIfac2 )
        m_xConfigAccess.clear();
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   UICategoryDescription                   ,
                                            ::cppu::OWeakObject                     ,
                                            SERVICENAME_UICATEGORYDESCRIPTION       ,
                                            IMPLEMENTATIONNAME_UICATEGORYDESCRIPTION
                                        )

DEFINE_INIT_SERVICE                     (   UICategoryDescription, {} )

UICategoryDescription::UICategoryDescription( const Reference< XMultiServiceFactory >& xServiceManager ) :
    UICommandDescription(xServiceManager,true)
{
    Reference< XNameAccess > xEmpty;
    rtl::OUString aGenericCategories( "GenericCategories" );
    m_xGenericUICommands = new ConfigurationAccess_UICategory( aGenericCategories, xEmpty, xServiceManager );

    // insert generic categories mappings
    m_aModuleToCommandFileMap.insert( ModuleToCommandFileMap::value_type(
        rtl::OUString(GENERIC_MODULE_NAME ), aGenericCategories ));

    UICommandsHashMap::iterator pCatIter = m_aUICommandsHashMap.find( aGenericCategories );
    if ( pCatIter != m_aUICommandsHashMap.end() )
        pCatIter->second = m_xGenericUICommands;

    impl_fillElements("ooSetupFactoryCmdCategoryConfigRef");
}

UICategoryDescription::~UICategoryDescription()
{
}
Reference< XNameAccess > UICategoryDescription::impl_createConfigAccess(const ::rtl::OUString& _sName)
{
    return new ConfigurationAccess_UICategory( _sName,m_xGenericUICommands,m_xServiceManager );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
