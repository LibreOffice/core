/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "uiconfiguration/uicategorydescription.hxx"
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include "properties.h"

#include "helper/mischelper.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/implbase2.hxx>
#include <unotools/configmgr.hxx>

#include <vcl/mnemonic.hxx>
#include <comphelper/sequence.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::configuration;
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

class ConfigurationAccess_UICategory : // Order is necessary for right initialization!
                                        private ThreadHelpBase                           ,
                                        public  ::cppu::WeakImplHelper2<XNameAccess,XContainerListener>
{
    public:
                                  ConfigurationAccess_UICategory( const OUString& aModuleName, const Reference< XNameAccess >& xGenericUICommands, const Reference< XComponentContext >& rxContext );
        virtual                   ~ConfigurationAccess_UICategory();

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
            throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
            throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
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
        Any                       getUINameFromID( const OUString& rId );
        Any                       getUINameFromCache( const OUString& rId );
        Sequence< OUString > getAllIds();
        sal_Bool                  fillCache();

    private:
        typedef ::boost::unordered_map< OUString,
                                 OUString,
                                 OUStringHash,
                                 ::std::equal_to< OUString > > IdToInfoCache;

        sal_Bool initializeConfigAccess();

        OUString                     m_aConfigCategoryAccess;
        OUString                     m_aPropUIName;
        Reference< XNameAccess >          m_xGenericUICategories;
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

ConfigurationAccess_UICategory::ConfigurationAccess_UICategory( const OUString& aModuleName, const Reference< XNameAccess >& rGenericUICategories, const Reference< XComponentContext >& rxContext ) :
    ThreadHelpBase(),
    m_aConfigCategoryAccess( CONFIGURATION_ROOT_ACCESS ),
    m_aPropUIName( CONFIGURATION_PROPERTY_NAME ),
    m_xGenericUICategories( rGenericUICategories ),
    m_bConfigAccessInitialized( sal_False ),
    m_bCacheFilled( sal_False )
{
    // Create configuration hierachical access name
    m_aConfigCategoryAccess += aModuleName;
    m_aConfigCategoryAccess += OUString( CONFIGURATION_CATEGORY_ELEMENT_ACCESS );

    m_xConfigProvider = theDefaultProvider::get( rxContext );
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
Any SAL_CALL ConfigurationAccess_UICategory::getByName( const OUString& rId )
throw ( NoSuchElementException, WrappedTargetException, RuntimeException)
{
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

Sequence< OUString > SAL_CALL ConfigurationAccess_UICategory::getElementNames()
throw ( RuntimeException )
{
    return getAllIds();
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasByName( const OUString& rId )
throw (::com::sun::star::uno::RuntimeException)
{
    return getByName( rId ).hasValue();
}

// XElementAccess
Type SAL_CALL ConfigurationAccess_UICategory::getElementType()
throw ( RuntimeException )
{
    return( ::getCppuType( (const OUString*)NULL ) );
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasElements()
throw ( RuntimeException )
{
    // There must be global categories!
    return sal_True;
}

sal_Bool ConfigurationAccess_UICategory::fillCache()
{
    SAL_INFO( "fwk", "framework (cd100003) ::ConfigurationAccess_UICategory::fillCache" );

    if ( m_bCacheFilled )
        return sal_True;

    sal_Int32            i( 0 );
    OUString        aUIName;
    Sequence< OUString > aNameSeq = m_xConfigAccess->getElementNames();

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

Any ConfigurationAccess_UICategory::getUINameFromID( const OUString& rId )
{
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

Any ConfigurationAccess_UICategory::getUINameFromCache( const OUString& rId )
{
    Any a;

    IdToInfoCache::const_iterator pIter = m_aIdCache.find( rId );
    if ( pIter != m_aIdCache.end() )
        a <<= pIter->second;

    return a;
}

Sequence< OUString > ConfigurationAccess_UICategory::getAllIds()
{
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
            Sequence< OUString > aNameSeq = m_xConfigAccess->getElementNames();

            if ( m_xGenericUICategories.is() )
            {
                // Create concat list of supported user interface commands of the module
                Sequence< OUString > aGenericNameSeq = m_xGenericUICategories->getElementNames();
                sal_uInt32 nCount1 = aNameSeq.getLength();
                sal_uInt32 nCount2 = aGenericNameSeq.getLength();

                aNameSeq.realloc( nCount1 + nCount2 );
                OUString* pNameSeq = aNameSeq.getArray();
                const OUString* pGenericSeq = aGenericNameSeq.getConstArray();
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

    return Sequence< OUString >();
}

sal_Bool ConfigurationAccess_UICategory::initializeConfigAccess()
{
    Sequence< Any > aArgs( 1 );
    PropertyValue   aPropValue;

    try
    {
        aPropValue.Name  = OUString( "nodepath" );
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
}

void SAL_CALL ConfigurationAccess_UICategory::elementRemoved ( const ContainerEvent& ) throw(RuntimeException)
{
}

void SAL_CALL ConfigurationAccess_UICategory::elementReplaced( const ContainerEvent& ) throw(RuntimeException)
{
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_UICategory::disposing( const EventObject& aEvent ) throw(RuntimeException)
{
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

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2  ( UICategoryDescription                  ,
                                            ::cppu::OWeakObject                    ,
                                            "com.sun.star.ui.UICategoryDescription",
                                            OUString("com.sun.star.comp.framework.UICategoryDescription")
                                        )

DEFINE_INIT_SERVICE                     (   UICategoryDescription, {} )

UICategoryDescription::UICategoryDescription( const Reference< XComponentContext >& rxContext ) :
    UICommandDescription(rxContext,true)
{
    Reference< XNameAccess > xEmpty;
    OUString aGenericCategories( "GenericCategories" );
    m_xGenericUICommands = new ConfigurationAccess_UICategory( aGenericCategories, xEmpty, rxContext );

    // insert generic categories mappings
    m_aModuleToCommandFileMap.insert( ModuleToCommandFileMap::value_type(
        OUString(GENERIC_MODULE_NAME ), aGenericCategories ));

    UICommandsHashMap::iterator pCatIter = m_aUICommandsHashMap.find( aGenericCategories );
    if ( pCatIter != m_aUICommandsHashMap.end() )
        pCatIter->second = m_xGenericUICommands;

    impl_fillElements("ooSetupFactoryCmdCategoryConfigRef");
}

UICategoryDescription::~UICategoryDescription()
{
}
Reference< XNameAccess > UICategoryDescription::impl_createConfigAccess(const OUString& _sName)
{
    return new ConfigurationAccess_UICategory( _sName, m_xGenericUICommands, m_xContext );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
