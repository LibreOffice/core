/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uicategorydescription.cxx,v $
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UICONFIGURATION_UICATEGORYDESCRPTION_HXX_
#include "uiconfiguration/uicategorydescription.hxx"
#endif
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include "properties.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <unotools/configmgr.hxx>
#include <tools/string.hxx>

#ifndef _VCL_MNEMONIC_HXX_
#include <vcl/mnemonic.hxx>
#endif
#include <comphelper/sequence.hxx>
#include <rtl/logfile.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
//

struct ModuleToCategory
{
    const char* pModuleId;
    const char* pCommands;
};

static const char CATEGORY[]                                = "Category";
static const char GENERIC_MODULE_NAME[]                     = "generic";
static const char CONFIGURATION_ROOT_ACCESS[]               = "/org.openoffice.Office.UI.";
static const char CONFIGURATION_CATEGORY_ELEMENT_ACCESS[]   = "/Commands/Categories";
static const char CONFIGURATION_PROPERTY_NAME[]             = "Name";

namespace framework
{

//*****************************************************************************************************************
//  Configuration access class for PopupMenuControllerFactory implementation
//*****************************************************************************************************************

class ConfigurationAccess_UICategory : // interfaces
                                        public  XTypeProvider                            ,
                                        public  XNameAccess                              ,
                                        public  XContainerListener                       ,
                                        // baseclasses
                                        // Order is neccessary for right initialization!
                                        private ThreadHelpBase                           ,
                                        public  ::cppu::OWeakObject
{
    public:
                                  ConfigurationAccess_UICategory( const ::rtl::OUString& aModuleName, const Reference< XNameAccess >& xGenericUICommands, const Reference< XMultiServiceFactory >& rServiceManager );
        virtual                   ~ConfigurationAccess_UICategory();

        //  XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

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
        void                      resetCache();
        sal_Bool                  fillCache();

    private:
        typedef ::std::hash_map< ::rtl::OUString,
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
        sal_Bool                          m_bConfigAccessInitialized;
        sal_Bool                          m_bCacheFilled;
        IdToInfoCache                     m_aIdCache;
};

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_5     (   ConfigurationAccess_UICategory                                                   ,
                            OWeakObject                                                                     ,
                            DIRECT_INTERFACE ( css::container::XNameAccess                                  ),
                            DIRECT_INTERFACE ( css::container::XContainerListener                           ),
                            DIRECT_INTERFACE ( css::lang::XTypeProvider                                     ),
                            DERIVED_INTERFACE( css::container::XElementAccess, css::container::XNameAccess  ),
                            DERIVED_INTERFACE( css::lang::XEventListener, XContainerListener                )
                        )

DEFINE_XTYPEPROVIDER_5  (   ConfigurationAccess_UICategory      ,
                            css::container::XNameAccess         ,
                            css::container::XElementAccess      ,
                            css::container::XContainerListener  ,
                            css::lang::XTypeProvider            ,
                            css::lang::XEventListener
                        )

ConfigurationAccess_UICategory::ConfigurationAccess_UICategory( const rtl::OUString& aModuleName, const Reference< XNameAccess >& rGenericUICategories, const Reference< XMultiServiceFactory >& rServiceManager ) :
    ThreadHelpBase(),
    m_aConfigCategoryAccess( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_ROOT_ACCESS )),
    m_aPropUIName( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_PROPERTY_NAME )),
    m_xGenericUICategories( rGenericUICategories ),
    m_xServiceManager( rServiceManager ),
    m_bConfigAccessInitialized( sal_False ),
    m_bCacheFilled( sal_False )
{
    // Create configuration hierachical access name
    m_aConfigCategoryAccess += aModuleName;
    m_aConfigCategoryAccess += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CONFIGURATION_CATEGORY_ELEMENT_ACCESS ));

    m_xConfigProvider = Reference< XMultiServiceFactory >( rServiceManager->createInstance(
                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                    "com.sun.star.configuration.ConfigurationProvider" ))),
                                                           UNO_QUERY );
}

ConfigurationAccess_UICategory::~ConfigurationAccess_UICategory()
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener( this );
}

// XNameAccess
Any SAL_CALL ConfigurationAccess_UICategory::getByName( const ::rtl::OUString& rId )
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

Sequence< ::rtl::OUString > SAL_CALL ConfigurationAccess_UICategory::getElementNames()
throw ( RuntimeException )
{
    return getAllIds();
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasByName( const ::rtl::OUString& rId )
throw (::com::sun::star::uno::RuntimeException)
{
    Any a = getByName( rId );
    if ( a != Any() )
        return sal_True;
    else
        return sal_False;
}

// XElementAccess
Type SAL_CALL ConfigurationAccess_UICategory::getElementType()
throw ( RuntimeException )
{
    return( ::getCppuType( (const rtl::OUString*)NULL ) );
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasElements()
throw ( RuntimeException )
{
    // There must be global categories!
    return sal_True;
}

void ConfigurationAccess_UICategory::resetCache()
{
    m_aIdCache.clear();
    m_bCacheFilled = sal_False;
}

sal_Bool ConfigurationAccess_UICategory::fillCache()
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::ConfigurationAccess_UICategory::fillCache" );

    if ( m_bCacheFilled )
        return sal_True;

    sal_Int32            i( 0 );
    Any                  a;
    rtl::OUString        aUIName;
    Sequence< OUString > aNameSeq = m_xConfigAccess->getElementNames();

    for ( i = 0; i < aNameSeq.getLength(); i++ )
    {
        try
        {
            Reference< XNameAccess > xNameAccess;
            a = m_xConfigAccess->getByName( aNameSeq[i] );
            if ( a >>= xNameAccess )
            {
                a = xNameAccess->getByName( m_aPropUIName );
                a >>= aUIName;

                m_aIdCache.insert( IdToInfoCache::value_type( aNameSeq[i], aUIName ));
            }
        }
        catch ( com::sun::star::lang::WrappedTargetException& )
        {
        }
        catch ( com::sun::star::container::NoSuchElementException& )
        {
        }
    }

    m_bCacheFilled = sal_True;

    return sal_True;
}

Any ConfigurationAccess_UICategory::getUINameFromID( const rtl::OUString& rId )
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
                catch ( com::sun::star::lang::WrappedTargetException& )
                {
                }
                catch ( com::sun::star::container::NoSuchElementException& )
                {
                }
            }
        }
    }
    catch( com::sun::star::container::NoSuchElementException& )
    {
    }
    catch ( com::sun::star::lang::WrappedTargetException& )
    {
    }

    return a;
}

Any ConfigurationAccess_UICategory::getUINameFromCache( const rtl::OUString& rId )
{
    Any a;

    IdToInfoCache::const_iterator pIter = m_aIdCache.find( rId );
    if ( pIter != m_aIdCache.end() )
        a <<= pIter->second;

    return a;
}

Sequence< rtl::OUString > ConfigurationAccess_UICategory::getAllIds()
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
        catch( com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( com::sun::star::lang::WrappedTargetException& )
        {
        }
    }

    return Sequence< rtl::OUString >();
}

sal_Bool ConfigurationAccess_UICategory::initializeConfigAccess()
{
    Sequence< Any > aArgs( 1 );
    PropertyValue   aPropValue;

    try
    {
        aPropValue.Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ));
        aPropValue.Value = makeAny( m_aConfigCategoryAccess );
        aArgs[0] <<= aPropValue;

        m_xConfigAccess = Reference< XNameAccess >( m_xConfigProvider->createInstanceWithArguments(
                                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                                "com.sun.star.configuration.ConfigurationAccess" )),
                                                                            aArgs ),
                                                                        UNO_QUERY );
        if ( m_xConfigAccess.is() )
        {
            // Add as container listener
            Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
            if ( xContainer.is() )
                xContainer->addContainerListener( this );
        }

        return sal_True;
    }
    catch ( WrappedTargetException& )
    {
    }
    catch ( Exception& )
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
DEFINE_XINTERFACE_4                    (    UICategoryDescription                                                           ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( css::container::XNameAccess                                   ),
                                            DERIVED_INTERFACE( css::container::XElementAccess, css::container::XNameAccess  )
                                        )

DEFINE_XTYPEPROVIDER_4                  (   UICategoryDescription           ,
                                            css::lang::XTypeProvider        ,
                                            css::lang::XServiceInfo         ,
                                            css::container::XNameAccess     ,
                                            css::container::XElementAccess
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   UICategoryDescription                   ,
                                            ::cppu::OWeakObject                     ,
                                            SERVICENAME_UICATEGORYDESCRIPTION       ,
                                            IMPLEMENTATIONNAME_UICATEGORYDESCRIPTION
                                        )

DEFINE_INIT_SERVICE                     (   UICategoryDescription, {} )

UICategoryDescription::UICategoryDescription( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    m_xServiceManager( xServiceManager )
{
    Reference< XNameAccess > xEmpty;
    rtl::OUString aGenericCategories( OUString::createFromAscii( "GenericCategories" ));
    m_xGenericCategories = new ConfigurationAccess_UICategory( aGenericCategories, xEmpty, xServiceManager );

    m_xModuleManager = Reference< XModuleManager >( m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ),
                                                    UNO_QUERY );
    Reference< XNameAccess > xNameAccess( m_xModuleManager, UNO_QUERY_THROW );
    Sequence< rtl::OUString > aElementNames = xNameAccess->getElementNames();
    Sequence< PropertyValue > aSeq;
    OUString                  aModuleIdentifier;

    // insert generic categories mappings
    m_aModuleToCategoryFileMap.insert( ModuleToCategoryFileMap::value_type(
        rtl::OUString::createFromAscii( GENERIC_MODULE_NAME ), aGenericCategories ));

    CategoryHashMap::iterator pCatIter = m_aCategoryHashMap.find( aGenericCategories );
    if ( pCatIter != m_aCategoryHashMap.end() )
        pCatIter->second = m_xGenericCategories;

    for ( sal_Int32 i = 0; i < aElementNames.getLength(); i++ )
    {
        aModuleIdentifier = aElementNames[i];
        Any a = xNameAccess->getByName( aModuleIdentifier );
        if ( a >>= aSeq )
        {
            OUString aCmdCategoryStr;
            for ( sal_Int32 y = 0; y < aSeq.getLength(); y++ )
            {
                if ( aSeq[y].Name.equalsAscii("ooSetupFactoryCmdCategoryConfigRef") )
                {
                    aSeq[y].Value >>= aCmdCategoryStr;
                    break;
                }
            }

            // Create first mapping module identifier ==> category file
            m_aModuleToCategoryFileMap.insert(
                ModuleToCategoryFileMap::value_type( aModuleIdentifier, aCmdCategoryStr ));

            // Create second mapping category file ==> category instance
            CategoryHashMap::iterator pIter = m_aCategoryHashMap.find( aCmdCategoryStr );
            if ( pIter == m_aCategoryHashMap.end() )
                m_aCategoryHashMap.insert( CategoryHashMap::value_type( aCmdCategoryStr, xEmpty ));
        }
    }
}

UICategoryDescription::~UICategoryDescription()
{
    ResetableGuard aLock( m_aLock );
    m_aModuleToCategoryFileMap.clear();
    m_aCategoryHashMap.clear();
    m_xGenericCategories.clear();
}

Any SAL_CALL UICategoryDescription::getByName( const ::rtl::OUString& aName )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    Any a;

    ResetableGuard aLock( m_aLock );

    ModuleToCategoryFileMap::const_iterator pM2CIter = m_aModuleToCategoryFileMap.find( aName );
    if ( pM2CIter != m_aModuleToCategoryFileMap.end() )
    {
        OUString aCommandFile( pM2CIter->second );
        CategoryHashMap::iterator pIter = m_aCategoryHashMap.find( aCommandFile );
        if ( pIter != m_aCategoryHashMap.end() )
        {
            if ( pIter->second.is() )
                a <<= pIter->second;
            else
            {
                Reference< XNameAccess > xUICategory;
                ConfigurationAccess_UICategory* pUICategory = new ConfigurationAccess_UICategory( aCommandFile,
                                                                                                  m_xGenericCategories,
                                                                                                  m_xServiceManager );
                xUICategory = Reference< XNameAccess >( static_cast< cppu::OWeakObject* >( pUICategory ),UNO_QUERY );
                pIter->second = xUICategory;
                a <<= xUICategory;
            }
        }
    }
    else
    {
        throw NoSuchElementException();
    }

    return a;
}

Sequence< ::rtl::OUString > SAL_CALL UICategoryDescription::getElementNames()
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    Sequence< rtl::OUString > aSeq( m_aModuleToCategoryFileMap.size() );

    sal_Int32 n = 0;
    ModuleToCategoryFileMap::const_iterator pIter = m_aModuleToCategoryFileMap.begin();
    while ( pIter != m_aModuleToCategoryFileMap.end() )
    {
        aSeq[n] = pIter->first;
        ++pIter;
    }

    return aSeq;
}

sal_Bool SAL_CALL UICategoryDescription::hasByName( const ::rtl::OUString& aName )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    ModuleToCategoryFileMap::const_iterator pIter = m_aModuleToCategoryFileMap.find( aName );
    return ( pIter != m_aModuleToCategoryFileMap.end() );
}

// XElementAccess
Type SAL_CALL UICategoryDescription::getElementType()
throw (::com::sun::star::uno::RuntimeException)
{
    return( ::getCppuType( (const Reference< XNameAccess >*)NULL ) );
}

sal_Bool SAL_CALL UICategoryDescription::hasElements()
throw (::com::sun::star::uno::RuntimeException)
{
    // generic UI commands are always available!
    return sal_True;
}

} // namespace framework
