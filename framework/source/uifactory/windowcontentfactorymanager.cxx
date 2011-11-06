/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <uifactory/windowcontentfactorymanager.hxx>
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
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
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

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
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
    m_pConfigAccess = new ConfigurationAccess_FactoryManager( m_xServiceManager,rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.WindowContentFactories/Registered/ContentFactories" )) );
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

        if ( !m_bConfigRead )
        {
            m_bConfigRead = sal_True;
            m_pConfigAccess->readConfigurationData();
        }

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
                    catch ( uno::Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
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
