/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uielementfactorymanager.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <uifactory/uielementfactorymanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include "services.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/frame/XFrame.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <tools/urlobj.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
//

namespace framework
{

// global function needed by both implementations
rtl::OUString getHashKeyFromStrings( const rtl::OUString& aType, const rtl::OUString& aName, const rtl::OUString& aModuleName )
{
    rtl::OUStringBuffer aKey( aType );
    aKey.appendAscii( "^" );
    aKey.append( aName );
    aKey.appendAscii( "^" );
    aKey.append( aModuleName );
    return aKey.makeStringAndClear();
}


//*****************************************************************************************************************
//  Configuration access class for UIElementFactoryManager implementation
//*****************************************************************************************************************

class ConfigurationAccess_UIElementFactoryManager : // interfaces
                                                    public  XTypeProvider                            ,
                                                    public  XContainerListener                       ,
                                                    // baseclasses
                                                    // Order is neccessary for right initialization!
                                                    private ThreadHelpBase                           ,
                                                    public  ::cppu::OWeakObject
{
    public:
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

                      ConfigurationAccess_UIElementFactoryManager( Reference< XMultiServiceFactory >& rServiceManager );
        virtual       ~ConfigurationAccess_UIElementFactoryManager();

        void          readConfigurationData();

        rtl::OUString                           getFactorySpecifierFromTypeNameModule( const rtl::OUString& rType, const rtl::OUString& rName, const rtl::OUString& rModule ) const;
        void                                    addFactorySpecifierToTypeNameModule( const rtl::OUString& rType, const rtl::OUString& rName, const rtl::OUString& rModule, const rtl::OUString& aServiceSpecifier );
        void                                    removeFactorySpecifierFromTypeNameModule( const rtl::OUString& rType, const rtl::OUString& rName, const rtl::OUString& rModule );
        Sequence< rtl::OUString >               getAllFactories() const;
        Sequence< Sequence< PropertyValue > >   getFactoriesDescription() const;

        // container.XContainerListener
        virtual void SAL_CALL elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException);
        virtual void SAL_CALL elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException);
        virtual void SAL_CALL elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException);

        // lang.XEventListener
        virtual void SAL_CALL disposing( const EventObject& aEvent ) throw(RuntimeException);

    private:
        class UIElementFactoryManagerMap : public std::hash_map< rtl::OUString,
                                                                 rtl::OUString,
                                                                 OUStringHashCode,
                                                                 ::std::equal_to< ::rtl::OUString > >
        {
            inline void free()
            {
                UIElementFactoryManagerMap().swap( *this );
            }
        };

        sal_Bool impl_getElementProps( const Any& rElement, rtl::OUString& rType, rtl::OUString& rName, rtl::OUString& rModule, rtl::OUString& rServiceSpecifier ) const;

        rtl::OUString                     m_aPropType;
        rtl::OUString                     m_aPropName;
        rtl::OUString                     m_aPropModule;
        rtl::OUString                     m_aPropFactory;
        UIElementFactoryManagerMap        m_aUIElementFactoryManagerMap;
        Reference< XMultiServiceFactory > m_xServiceManager;
        Reference< XMultiServiceFactory > m_xConfigProvider;
        Reference< XNameAccess >          m_xConfigAccess;
        sal_Bool                          m_bConfigAccessInitialized;
};

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_3     (   ConfigurationAccess_UIElementFactoryManager                         ,
                            OWeakObject                                                         ,
                            DIRECT_INTERFACE ( XTypeProvider                                    ),
                            DIRECT_INTERFACE ( XContainerListener                               ),
                            DERIVED_INTERFACE( css::lang::XEventListener, XContainerListener    )
                        )

DEFINE_XTYPEPROVIDER_3  (   ConfigurationAccess_UIElementFactoryManager ,
                            XTypeProvider                               ,
                            XContainerListener                          ,
                            css::lang::XEventListener
                        )

ConfigurationAccess_UIElementFactoryManager::ConfigurationAccess_UIElementFactoryManager( Reference< XMultiServiceFactory >& rServiceManager ) :
    ThreadHelpBase(),
    m_aPropType( RTL_CONSTASCII_USTRINGPARAM( "Type" )),
    m_aPropName( RTL_CONSTASCII_USTRINGPARAM( "Name" )),
    m_aPropModule( RTL_CONSTASCII_USTRINGPARAM( "Module" )),
    m_aPropFactory( RTL_CONSTASCII_USTRINGPARAM( "FactoryImplementation" )),
    m_xServiceManager( rServiceManager ),
    m_bConfigAccessInitialized( sal_False )
{
    m_xConfigProvider = Reference< XMultiServiceFactory >( rServiceManager->createInstance(
                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                    "com.sun.star.configuration.ConfigurationProvider" ))),
                                                           UNO_QUERY );
}

ConfigurationAccess_UIElementFactoryManager::~ConfigurationAccess_UIElementFactoryManager()
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener( this );
}

rtl::OUString ConfigurationAccess_UIElementFactoryManager::getFactorySpecifierFromTypeNameModule( const rtl::OUString& rType, const rtl::OUString& rName, const rtl::OUString& rModule ) const
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    UIElementFactoryManagerMap::const_iterator pIter =
        m_aUIElementFactoryManagerMap.find( getHashKeyFromStrings( rType, rName, rModule ));
    if ( pIter != m_aUIElementFactoryManagerMap.end() )
        return pIter->second;
    else
    {
        pIter = m_aUIElementFactoryManagerMap.find( getHashKeyFromStrings( rType, rName, rtl::OUString() ));
        if ( pIter != m_aUIElementFactoryManagerMap.end() )
            return pIter->second;
        else
        {
            // Support factories which uses a defined prefix before the ui name.
            sal_Int32 nIndex = rName.indexOf( '_' );
            if ( nIndex > 0 )
            {
                rtl::OUString aName = rName.copy( 0, nIndex+1 );
                pIter = m_aUIElementFactoryManagerMap.find( getHashKeyFromStrings( rType, aName, rtl::OUString() ));
                if ( pIter != m_aUIElementFactoryManagerMap.end() )
                    return pIter->second;
            }

            pIter = m_aUIElementFactoryManagerMap.find( getHashKeyFromStrings( rType, rtl::OUString(), rtl::OUString() ));
            if ( pIter != m_aUIElementFactoryManagerMap.end() )
                return pIter->second;
        }
    }

    return rtl::OUString();
}

void ConfigurationAccess_UIElementFactoryManager::addFactorySpecifierToTypeNameModule( const rtl::OUString& rType, const rtl::OUString& rName, const rtl::OUString& rModule, const rtl::OUString& rServiceSpecifier )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    rtl::OUString aHashKey = getHashKeyFromStrings( rType, rName, rModule );

    UIElementFactoryManagerMap::const_iterator pIter =
        m_aUIElementFactoryManagerMap.find( getHashKeyFromStrings( rType, rName, rModule ));

    if ( pIter != m_aUIElementFactoryManagerMap.end() )
        throw ElementExistException();
    else
        m_aUIElementFactoryManagerMap.insert( UIElementFactoryManagerMap::value_type( aHashKey, rServiceSpecifier ));
}


void ConfigurationAccess_UIElementFactoryManager::removeFactorySpecifierFromTypeNameModule( const rtl::OUString& rType, const rtl::OUString& rName, const rtl::OUString& rModule )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    rtl::OUString aHashKey = getHashKeyFromStrings( rType, rName, rModule );

    UIElementFactoryManagerMap::const_iterator pIter =
        m_aUIElementFactoryManagerMap.find( getHashKeyFromStrings( rType, rName, rModule ));

    if ( pIter == m_aUIElementFactoryManagerMap.end() )
        throw NoSuchElementException();
    else
        m_aUIElementFactoryManagerMap.erase( aHashKey );
}

Sequence< rtl::OUString > ConfigurationAccess_UIElementFactoryManager::getAllFactories() const
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Sequence< rtl::OUString > aSeq( m_aUIElementFactoryManagerMap.size() );

    sal_Int32 nIndex = 0;
    UIElementFactoryManagerMap::const_iterator pIter = m_aUIElementFactoryManagerMap.begin();
    while ( pIter != m_aUIElementFactoryManagerMap.end() );
    {
        aSeq[nIndex++] = pIter->second;
        ++pIter;
    }

    return aSeq;
}

Sequence< Sequence< PropertyValue > > ConfigurationAccess_UIElementFactoryManager::getFactoriesDescription() const
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Sequence< Sequence< PropertyValue > > aSeqSeq;

    sal_Int32 nIndex( 0 );
    UIElementFactoryManagerMap::const_iterator pIter = m_aUIElementFactoryManagerMap.begin();
    while ( pIter != m_aUIElementFactoryManagerMap.end() )
    {
        rtl::OUString aFactory = pIter->first;
        if ( aFactory.getLength() > 0 )
        {
            sal_Int32                 nToken = 0;
            Sequence< PropertyValue > aSeq( 1 );

            aSeqSeq.realloc( aSeqSeq.getLength() + 1 );
            aSeq[0].Name  = m_aPropType;
            aSeq[0].Value = makeAny( aFactory.getToken( 0, '^', nToken ));
            if ( nToken > 0 )
            {
                aSeq.realloc( 2 );
                aSeq[1].Name  = m_aPropName;
                aSeq[1].Value = makeAny( aFactory.getToken( 0, '^', nToken ));
                if ( nToken > 0 )
                {
                    aSeq.realloc( 3 );
                    aSeq[2].Name  = m_aPropModule;
                    aSeq[2].Value = makeAny( aFactory.getToken( 0, '^', nToken ));
                }
            }

            aSeqSeq[nIndex++] = aSeq;
        }

        ++pIter;
    }

    return aSeqSeq;
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_UIElementFactoryManager::elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    rtl::OUString   aType;
    rtl::OUString   aName;
    rtl::OUString   aModule;
    rtl::OUString   aService;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        // Create hash key from type, name and module as they are together a primary key to
        // the UNO service that implements a user interface factory.
        rtl::OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aUIElementFactoryManagerMap.insert( UIElementFactoryManagerMap::value_type( aHashKey, aService ));
    }
}

void SAL_CALL ConfigurationAccess_UIElementFactoryManager::elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    rtl::OUString   aType;
    rtl::OUString   aName;
    rtl::OUString   aModule;
    rtl::OUString   aService;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        // Create hash key from command and model as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        rtl::OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aUIElementFactoryManagerMap.erase( aHashKey );
    }
}

void SAL_CALL ConfigurationAccess_UIElementFactoryManager::elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    rtl::OUString   aType;
    rtl::OUString   aName;
    rtl::OUString   aModule;
    rtl::OUString   aService;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        // Create hash key from command and model as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        rtl::OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aUIElementFactoryManagerMap.erase( aHashKey );
        m_aUIElementFactoryManagerMap.insert( UIElementFactoryManagerMap::value_type( aHashKey, aService ));
    }
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_UIElementFactoryManager::disposing( const EventObject& ) throw(RuntimeException)
{
    // SAFE
    // remove our reference to the config access
    ResetableGuard aLock( m_aLock );
    m_xConfigAccess.clear();
}

void ConfigurationAccess_UIElementFactoryManager::readConfigurationData()
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigAccessInitialized )
    {
        Sequence< Any > aArgs( 1 );
        PropertyValue   aPropValue;

        aPropValue.Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ));
        aPropValue.Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Factories/Registered/UIElementFactories" ));
        aArgs[0] <<= aPropValue;

        try
        {
            m_xConfigAccess = Reference< XNameAccess >( m_xConfigProvider->createInstanceWithArguments(
                                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                                    "com.sun.star.configuration.ConfigurationAccess" )),
                                                                                aArgs ),
                                                                            UNO_QUERY );
        }
        catch ( WrappedTargetException& )
        {
        }

        m_bConfigAccessInitialized = sal_True;
    }

    if ( m_xConfigAccess.is() )
    {
        Sequence< rtl::OUString >   aUIElementFactories = m_xConfigAccess->getElementNames();

        Any a;
        rtl::OUString             aType;
        rtl::OUString             aName;
        rtl::OUString             aModule;
        rtl::OUString             aService;
        rtl::OUString             aHashKey;
        Reference< XPropertySet > xPropertySet;
        for ( sal_Int32 i = 0; i < aUIElementFactories.getLength(); i++ )
        {
            if ( impl_getElementProps( m_xConfigAccess->getByName( aUIElementFactories[i] ), aType, aName, aModule, aService ))
            {
                // Create hash key from type, name and module as they are together a primary key to
                // the UNO service that implements the user interface element factory.
                aHashKey = getHashKeyFromStrings( aType, aName, aModule );
                m_aUIElementFactoryManagerMap.insert( UIElementFactoryManagerMap::value_type( aHashKey, aService ));
            }
        }

        // UNSAFE
        aLock.unlock();

        Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
        if ( xContainer.is() )
            xContainer->addContainerListener( this );
    }
}

sal_Bool ConfigurationAccess_UIElementFactoryManager::impl_getElementProps( const Any& aElement, rtl::OUString& rType, rtl::OUString& rName, rtl::OUString& rModule, rtl::OUString& rServiceSpecifier ) const
{
    Reference< XPropertySet > xPropertySet;
    Reference< XNameAccess > xNameAccess;
    aElement >>= xPropertySet;
    aElement >>= xNameAccess;

    if ( xPropertySet.is() )
    {
        try
        {
            xPropertySet->getPropertyValue( m_aPropType ) >>= rType;
            xPropertySet->getPropertyValue( m_aPropName ) >>= rName;
            xPropertySet->getPropertyValue( m_aPropModule ) >>= rModule;
            xPropertySet->getPropertyValue( m_aPropFactory ) >>= rServiceSpecifier;
        }
        catch ( com::sun::star::beans::UnknownPropertyException& )
        {
            return sal_False;
        }
        catch ( com::sun::star::lang::WrappedTargetException& )
        {
            return sal_False;
        }
    }

    return sal_True;
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_4                    (    UIElementFactoryManager                                                         ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIElementFactory                 ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIElementFactoryRegistration     )
                                        )

DEFINE_XTYPEPROVIDER_4                  (   UIElementFactoryManager                                     ,
                                            css::lang::XTypeProvider                                    ,
                                            css::lang::XServiceInfo                                     ,
                                            ::com::sun::star::ui::XUIElementFactory               ,
                                            ::com::sun::star::ui::XUIElementFactoryRegistration
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   UIElementFactoryManager                         ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_UIELEMENTFACTORYMANAGER             ,
                                            IMPLEMENTATIONNAME_UIELEMENTFACTORYMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   UIElementFactoryManager, {} )

UIElementFactoryManager::UIElementFactoryManager( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    m_bConfigRead( sal_False ),
    m_xServiceManager( xServiceManager )
{
    m_pConfigAccess = new ConfigurationAccess_UIElementFactoryManager( m_xServiceManager );
    m_pConfigAccess->acquire();
    m_xModuleManager = Reference< XModuleManager >( m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY );
}

UIElementFactoryManager::~UIElementFactoryManager()
{
    ResetableGuard aLock( m_aLock );

    // reduce reference count
    m_pConfigAccess->release();
}

void UIElementFactoryManager::RetrieveTypeNameFromResourceURL( const rtl::OUString& aResourceURL, rtl::OUString& aType, rtl::OUString& aName )
{
    const sal_Int32 RESOURCEURL_PREFIX_SIZE = 17;
    const char      RESOURCEURL_PREFIX[] = "private:resource/";

    if (( aResourceURL.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( RESOURCEURL_PREFIX ))) == 0 ) &&
        ( aResourceURL.getLength() > RESOURCEURL_PREFIX_SIZE ))
    {
        rtl::OUString aTmpStr( aResourceURL.copy( RESOURCEURL_PREFIX_SIZE ));
        sal_Int32 nToken = 0;
        sal_Int32 nPart  = 0;
        do
        {
            ::rtl::OUString sToken = aTmpStr.getToken( 0, '/', nToken);
            if ( sToken.getLength() )
            {
                if ( nPart == 0 )
                    aType = sToken;
                else if ( nPart == 1 )
                    aName = sToken;
                else
                    break;
                nPart++;
            }
        }
        while( nToken >=0 );
    }
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL UIElementFactoryManager::createUIElement(
    const ::rtl::OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    const rtl::OUString aPropFrame( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));

    rtl::OUString   aModuleId;
    PropertyValue   aPropValue;
    Reference< XFrame > xFrame;

    // Retrieve the frame instance from the arguments to determine the module identifier. This must be provided
    // to the search function. An empty module identifier is provided if the frame is missing or the module id cannot
    // retrieve from it.
    for ( int i = 0; i < Args.getLength(); i++ )
    {
        if ( Args[i].Name.equals( aPropFrame ))
            Args[i].Value >>= xFrame;
    }

    // Determine the module identifier
    try
    {
        if ( xFrame.is() && m_xModuleManager.is() )
            aModuleId = m_xModuleManager->identify( Reference< XInterface >( xFrame, UNO_QUERY ) );

        Reference< XUIElementFactory > xUIElementFactory = getFactory( ResourceURL, aModuleId );
        if ( xUIElementFactory.is() )
            return xUIElementFactory->createUIElement( ResourceURL, Args );
    }
    catch ( UnknownModuleException& )
    {
    }

    throw NoSuchElementException();
}

// XUIElementFactoryRegistration
Sequence< Sequence< PropertyValue > > SAL_CALL UIElementFactoryManager::getRegisteredFactories()
throw ( RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    return m_pConfigAccess->getFactoriesDescription();
}

Reference< XUIElementFactory > SAL_CALL UIElementFactoryManager::getFactory( const ::rtl::OUString& aResourceURL, const ::rtl::OUString& aModuleId )
throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    rtl::OUString aType;
    rtl::OUString aName;

    RetrieveTypeNameFromResourceURL( aResourceURL, aType, aName );

    rtl::OUString aServiceSpecifier = m_pConfigAccess->getFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
    if ( aServiceSpecifier.getLength() )
        return Reference< XUIElementFactory >( m_xServiceManager->createInstance( aServiceSpecifier ), UNO_QUERY );
    else
        return Reference< XUIElementFactory >();
}

void SAL_CALL UIElementFactoryManager::registerFactory( const ::rtl::OUString& aType, const ::rtl::OUString& aName, const ::rtl::OUString& aModuleId, const ::rtl::OUString& aFactoryImplementationName )
throw ( ElementExistException, RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->addFactorySpecifierToTypeNameModule( aType, aName, aModuleId, aFactoryImplementationName );
    // SAFE
}

void SAL_CALL UIElementFactoryManager::deregisterFactory( const ::rtl::OUString& aType, const ::rtl::OUString& aName, const ::rtl::OUString& aModuleId )
throw ( NoSuchElementException, RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->removeFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
    // SAFE
}

} // namespace framework
