/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uielementfactorymanager.cxx,v $
 * $Revision: 1.10.12.1 $
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
#include <uifactory/windowcontentfactorymanager.hxx>
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
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
// 

using namespace ::com::sun::star;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
// 

namespace framework
{

// global function needed by both implementations
static rtl::OUString getHashKeyFromStrings( const rtl::OUString& aType, const rtl::OUString& aName, const rtl::OUString& aModuleName )
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

class ConfigurationAccess_WindowContentFactoryManager : // interfaces
                                                    public  lang::XTypeProvider                      ,
                                                    public  container::XContainerListener            ,
                                                    // baseclasses
                                                    // Order is neccessary for right initialization!
                                                    private ThreadHelpBase                           ,
                                                    public  ::cppu::OWeakObject
{
    public:
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
                      
                      ConfigurationAccess_WindowContentFactoryManager( uno::Reference< lang::XMultiServiceFactory >& rServiceManager );
        virtual       ~ConfigurationAccess_WindowContentFactoryManager();

        void          readConfigurationData();

        rtl::OUString getFactorySpecifierFromTypeNameModule( const rtl::OUString& rType, const rtl::OUString& rName, const rtl::OUString& rModule );

        // container.XContainerListener
        virtual void SAL_CALL elementInserted( const container::ContainerEvent& aEvent ) throw(uno::RuntimeException);
        virtual void SAL_CALL elementRemoved ( const container::ContainerEvent& aEvent ) throw(uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const container::ContainerEvent& aEvent ) throw(uno::RuntimeException);

        // lang.XEventListener
        virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) throw(uno::RuntimeException);
 
    private:
        class FactoryMap : public std::hash_map< rtl::OUString,
                                                 rtl::OUString,
                                                 OUStringHashCode,
                                                 ::std::equal_to< ::rtl::OUString > >
        {
            inline void free()
            {
                FactoryMap().swap( *this );
            }
        };
    
        sal_Bool impl_getElementProps( const uno::Any& rElement, rtl::OUString& rType, rtl::OUString& rName, rtl::OUString& rModule, rtl::OUString& rServiceSpecifier ) const;

        rtl::OUString                                m_aPropType;
        rtl::OUString                                m_aPropName;
        rtl::OUString                                m_aPropModule;
        rtl::OUString                                m_aPropFactory;
        FactoryMap                                   m_aFactoryMap;
        uno::Reference< lang::XMultiServiceFactory > m_xServiceManager;
        uno::Reference< lang::XMultiServiceFactory > m_xConfigProvider;
        uno::Reference< container::XNameAccess >     m_xConfigAccess;
        bool                                         m_bConfigAccessInitialized;
        bool                                         m_bConfigDirty;
};

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_3     (   ConfigurationAccess_WindowContentFactoryManager                     ,
                            OWeakObject                                                         ,
                            DIRECT_INTERFACE ( lang::XTypeProvider                              ),
                            DIRECT_INTERFACE ( container::XContainerListener                    ),
                            DERIVED_INTERFACE( lang::XEventListener, XContainerListener         )
                        )

DEFINE_XTYPEPROVIDER_3  (   ConfigurationAccess_WindowContentFactoryManager ,
                            lang::XTypeProvider                             ,
                            container::XContainerListener                   ,
                            lang::XEventListener
                        )

ConfigurationAccess_WindowContentFactoryManager::ConfigurationAccess_WindowContentFactoryManager( uno::Reference< lang::XMultiServiceFactory >& rServiceManager ) :
    ThreadHelpBase(),
    m_aPropType( RTL_CONSTASCII_USTRINGPARAM( "Type" )),
    m_aPropName( RTL_CONSTASCII_USTRINGPARAM( "Name" )),
    m_aPropModule( RTL_CONSTASCII_USTRINGPARAM( "Module" )),
    m_aPropFactory( RTL_CONSTASCII_USTRINGPARAM( "FactoryImplementation" )),
    m_xServiceManager( rServiceManager ),
    m_bConfigAccessInitialized( false ),
    m_bConfigDirty( true )
{
    m_xConfigProvider = uno::Reference< lang::XMultiServiceFactory >( rServiceManager->createInstance( 
                                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( 
                                                                            "com.sun.star.configuration.ConfigurationProvider" ))),
                                                                      uno::UNO_QUERY );
}

ConfigurationAccess_WindowContentFactoryManager::~ConfigurationAccess_WindowContentFactoryManager()
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    
    uno::Reference< container::XContainer > xContainer( m_xConfigAccess, uno::UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener( this );
}

rtl::OUString ConfigurationAccess_WindowContentFactoryManager::getFactorySpecifierFromTypeNameModule( const rtl::OUString& rType, const rtl::OUString& rName, const rtl::OUString& rModule )
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    
    // Make sure that we read the configuration data at least once.
    // May be more dependent on the dirty flag!
    readConfigurationData();

    FactoryMap::const_iterator pIter = 
        m_aFactoryMap.find( getHashKeyFromStrings( rType, rName, rModule ));
    if ( pIter != m_aFactoryMap.end() )
        return pIter->second;
    else
    {
        pIter = m_aFactoryMap.find( getHashKeyFromStrings( rType, rName, rtl::OUString() ));
        if ( pIter != m_aFactoryMap.end() )
            return pIter->second;
        else
        {
            // Support factories which uses a defined prefix for their element names.
            sal_Int32 nIndex = rName.indexOf( '_' );
            if ( nIndex > 0 )
            {
                rtl::OUString aName = rName.copy( 0, nIndex );
                pIter = m_aFactoryMap.find( getHashKeyFromStrings( rType, aName, rtl::OUString() ));
                if ( pIter != m_aFactoryMap.end() )
                    return pIter->second;
            }

            pIter = m_aFactoryMap.find( getHashKeyFromStrings( rType, rtl::OUString(), rtl::OUString() ));
            if ( pIter != m_aFactoryMap.end() )
                return pIter->second;
        }
    }
    
    return rtl::OUString();
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_WindowContentFactoryManager::elementInserted( const container::ContainerEvent& ) throw(uno::RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    m_bConfigDirty = true;
}

void SAL_CALL ConfigurationAccess_WindowContentFactoryManager::elementRemoved ( const container::ContainerEvent& ) throw(uno::RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    m_bConfigDirty = true;
}

void SAL_CALL ConfigurationAccess_WindowContentFactoryManager::elementReplaced( const container::ContainerEvent& ) throw(uno::RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    m_bConfigDirty = true;
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_WindowContentFactoryManager::disposing( const lang::EventObject& ) throw(uno::RuntimeException)
{
    // SAFE
    // remove our reference to the config access
    ResetableGuard aLock( m_aLock );
    m_xConfigAccess.clear();   
}

void ConfigurationAccess_WindowContentFactoryManager::readConfigurationData()
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    
    bool bConfigAccessInitialized(m_bConfigAccessInitialized);
    if ( !m_bConfigAccessInitialized )
    {
        uno::Sequence< uno::Any > aArgs( 1 );
        beans::PropertyValue     aPropValue;
        
        aPropValue.Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ));
        aPropValue.Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.WindowContentFactories/Registered/ContentFactories" ));
        aArgs[0] <<= aPropValue;
        
        try
        {
            m_xConfigAccess = uno::Reference< container::XNameAccess >( m_xConfigProvider->createInstanceWithArguments( 
                                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( 
                                                                                    "com.sun.star.configuration.ConfigurationAccess" )),
                                                                                aArgs ), 
                                                                                uno::UNO_QUERY );
        }
        catch ( lang::WrappedTargetException& )
        {
        }
        
        m_bConfigAccessInitialized = sal_True;
    }
    
    if ( m_xConfigAccess.is() && m_bConfigDirty )
    {
        uno::Sequence< rtl::OUString > aUIElementFactories = m_xConfigAccess->getElementNames();
        
        uno::Any      a;
        rtl::OUString aType;
        rtl::OUString aName;
        rtl::OUString aModule;
        rtl::OUString aService;
        rtl::OUString aHashKey;
        
        uno::Reference< beans::XPropertySet > xPropertySet;
        
        m_aFactoryMap.clear();
        for ( sal_Int32 i = 0; i < aUIElementFactories.getLength(); i++ )
        {
            if ( impl_getElementProps( m_xConfigAccess->getByName( aUIElementFactories[i] ), aType, aName, aModule, aService ))
            {
                // Create hash key from type, name and module as they are together a primary key to
                // the UNO service that implements the user interface element factory.
                aHashKey = getHashKeyFromStrings( aType, aName, aModule );
                m_aFactoryMap.insert( FactoryMap::value_type( aHashKey, aService ));
            }
        }
    
        uno::Reference< container::XContainer > xContainer( m_xConfigAccess, uno::UNO_QUERY );
        m_bConfigDirty = false;

        aLock.unlock();
        // UNSAFE
        
        if ( xContainer.is() && !bConfigAccessInitialized)
            xContainer->addContainerListener( this );
    }
}

sal_Bool ConfigurationAccess_WindowContentFactoryManager::impl_getElementProps( const uno::Any& aElement, rtl::OUString& rType, rtl::OUString& rName, rtl::OUString& rModule, rtl::OUString& rServiceSpecifier ) const
{
    uno::Reference< beans::XPropertySet > xPropertySet;
    uno::Reference< container::XNameAccess > xNameAccess;
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
        catch ( beans::UnknownPropertyException& )
        {
            return sal_False;
        }
        catch ( lang::WrappedTargetException& )
        {
            return sal_False;
        }
    }

    return sal_True;
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_3                    (    WindowContentFactoryManager                              ,
                                            OWeakObject                                              ,
                                            DIRECT_INTERFACE( lang::XTypeProvider                    ),
                                            DIRECT_INTERFACE( lang::XServiceInfo                     ),
                                            DIRECT_INTERFACE( lang::XSingleComponentFactory          )
                                        )

DEFINE_XTYPEPROVIDER_3                  (   WindowContentFactoryManager     ,
                                            lang::XTypeProvider             ,
                                            lang::XServiceInfo              ,
                                            lang::XSingleComponentFactory
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   WindowContentFactoryManager                     ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_WINDOWCONTENTFACTORYMANAGER         ,
                                            IMPLEMENTATIONNAME_WINDOWCONTENTFACTORYMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   WindowContentFactoryManager, {} )

WindowContentFactoryManager::WindowContentFactoryManager( const uno::Reference< lang::XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_bConfigRead( sal_False ),
    m_xServiceManager( xServiceManager )
{
    m_pConfigAccess = new ConfigurationAccess_WindowContentFactoryManager( m_xServiceManager );
    m_pConfigAccess->acquire();
    m_xModuleManager = uno::Reference< frame::XModuleManager >( m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), uno::UNO_QUERY );
}

WindowContentFactoryManager::~WindowContentFactoryManager()
{
    ResetableGuard aLock( m_aLock );
    
    // reduce reference count
    m_pConfigAccess->release();
}

void WindowContentFactoryManager::RetrieveTypeNameFromResourceURL( const rtl::OUString& aResourceURL, rtl::OUString& aType, rtl::OUString& aName )
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

// XSingleComponentFactory
uno::Reference< uno::XInterface > SAL_CALL WindowContentFactoryManager::createInstanceWithContext( 
    const uno::Reference< uno::XComponentContext >& /*xContext*/ ) 
throw (uno::Exception, uno::RuntimeException)
{
/*
    // Currently this method cannot be implemented for generic use. There is no way for external
       code to get a handle to the dialog model.

    uno::Reference< lang::XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), uno::UNO_QUERY );
    
    const ::rtl::OUString sToolkitService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.Toolkit"));
    uno::Reference< awt::XToolkit > xToolkit( xServiceManager->createInstance( sToolkitService ), uno::UNO_QUERY_THROW );
    
    const ::rtl::OUString sDialogModelService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlDialogModel"));
    uno::Reference< awt::XControlModel > xDialogModel( xServiceManager->createInstance( sDialogModelService ), uno::UNO_QUERY_THROW );

    const ::rtl::OUString sDecoration(RTL_CONSTASCII_USTRINGPARAM("Decoration"));
    uno::Reference< beans::XPropertySet > xPropSet( xDialogModel, uno::UNO_QUERY_THROW );
    xPropSet->setPropertyValue( sDecoration, uno::makeAny(false));

    const ::rtl::OUString sDialogService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlDialog"));
    uno::Reference< awt::XControl > xDialogControl( xServiceManager->createInstance( sDialogService ), uno::UNO_QUERY_THROW );

    xDialogControl->setModel( xDialogModel );
  
    uno::Reference< awt::XWindowPeer > xWindowParentPeer( xToolkit->getDesktopWindow(), uno::UNO_QUERY );
    xDialogControl->createPeer( xToolkit, xWindowParentPeer );
    uno::Reference< uno::XInterface > xWindow( xDialogControl->getPeer(), uno::UNO_QUERY );
*/
    uno::Reference< uno::XInterface > xWindow;
    return xWindow;
}

uno::Reference< uno::XInterface > SAL_CALL WindowContentFactoryManager::createInstanceWithArgumentsAndContext( 
    const uno::Sequence< uno::Any >& Arguments, const uno::Reference< uno::XComponentContext >& Context ) 
throw (uno::Exception, uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xWindow;
    uno::Reference< frame::XFrame >   xFrame;
    ::rtl::OUString                   aResourceURL;

    for (sal_Int32 i=0; i < Arguments.getLength(); i++ )
    {
        beans::PropertyValue aPropValue;
        if ( Arguments[i] >>= aPropValue )
        {
            if ( aPropValue.Name.equalsAscii( "Frame" ))
                aPropValue.Value >>= xFrame;
            else if ( aPropValue.Name.equalsAscii( "ResourceURL" ))
                aPropValue.Value >>= aResourceURL;
        }
    }

    uno::Reference< frame::XModuleManager > xModuleManager;
    // SAFE    
    {
        ResetableGuard aLock( m_aLock );
        xModuleManager = m_xModuleManager;
    }
    // UNSAFE
    
    // Determine the module identifier
    ::rtl::OUString aType;
    ::rtl::OUString aName;
    ::rtl::OUString aModuleId;
    try
    {
        if ( xFrame.is() && xModuleManager.is() )
            aModuleId = xModuleManager->identify( uno::Reference< uno::XInterface >( xFrame, uno::UNO_QUERY ) );
    }
    catch ( frame::UnknownModuleException& )
    {
    }

    RetrieveTypeNameFromResourceURL( aResourceURL, aType, aName );
    if ( aType.getLength() > 0 && 
         aName.getLength() > 0 && 
         aModuleId.getLength() > 0 )
    {
        ::rtl::OUString                   aImplementationName;
        uno::Reference< uno::XInterface > xHolder( static_cast<cppu::OWeakObject*>(this), uno::UNO_QUERY );
        
        // Detetmine the implementation name of the window content factory dependent on the
        // module identifier, user interface element type and name
        // SAFE    
        ResetableGuard aLock( m_aLock );
        aImplementationName = m_pConfigAccess->getFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
        if ( aImplementationName.getLength() > 0 )
        {
            aLock.unlock();
            // UNSAFE

            uno::Reference< lang::XMultiServiceFactory > xServiceManager( Context->getServiceManager(), uno::UNO_QUERY );
            if ( xServiceManager.is() )
            {
                uno::Reference< lang::XSingleComponentFactory > xFactory( 
                    xServiceManager->createInstance( aImplementationName ), uno::UNO_QUERY );
                if ( xFactory.is() )
                {
                    // Be careful: We call external code. Therefore here we have to catch all exceptions
                    try
                    {
                        xWindow = xFactory->createInstanceWithArgumentsAndContext( Arguments, Context );
                    }
                    catch ( uno::RuntimeException& )
                    {
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }
            }
        }
    }
    
    // UNSAFE
    if ( !xWindow.is())
    {
        // Fallback: Use internal factory code to create a toolkit dialog as a content window
        xWindow = createInstanceWithContext(Context);
    }

    return xWindow;
}

} // namespace framework
