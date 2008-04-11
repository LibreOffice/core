/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: toolboxfactory.cxx,v $
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
#include "precompiled_framework.hxx"
#include <uifactory/toolboxfactory.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <uielement/toolbarwrapper.hxx>
#include <threadhelp/resetableguard.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPLLIER_HPP_
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace ::com::sun::star::ui;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_3                    (    ToolBoxFactory                                                  ,
                                            OWeakObject                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                       ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIElementFactory )
                                        )

DEFINE_XTYPEPROVIDER_3                  (   ToolBoxFactory                                  ,
                                            css::lang::XTypeProvider                        ,
                                            css::lang::XServiceInfo                         ,
                                            ::com::sun::star::ui::XUIElementFactory
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   ToolBoxFactory                                  ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_TOOLBARFACTORY                      ,
                                            IMPLEMENTATIONNAME_TOOLBARFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   ToolBoxFactory, {} )

ToolBoxFactory::ToolBoxFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_xServiceManager( xServiceManager )
    , m_xModuleManager( xServiceManager->createInstance(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ModuleManager" ))),
                        UNO_QUERY )
{
}

ToolBoxFactory::~ToolBoxFactory()
{
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL ToolBoxFactory::createUIElement(
    const ::rtl::OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Reference< XUIConfigurationManager > xConfigSource;
    Reference< XFrame >                  xFrame;
    rtl::OUString                        aResourceURL( ResourceURL );
    sal_Bool                             bPersistent( sal_True );
    sal_Bool                             bPopupMode( sal_False );

    for ( sal_Int32 n = 0; n < Args.getLength(); n++ )
    {
        if ( Args[n].Name.equalsAscii( "ConfigurationSource" ))
            Args[n].Value >>= xConfigSource;
        else if ( Args[n].Name.equalsAsciiL( "Frame", 5 ))
            Args[n].Value >>= xFrame;
        else if ( Args[n].Name.equalsAsciiL( "ResourceURL", 11 ))
            Args[n].Value >>= aResourceURL;
        else if ( Args[n].Name.equalsAsciiL( "Persistent", 10 ))
            Args[n].Value >>= bPersistent;
        else if ( Args[n].Name.equalsAsciiL( "PopupMode", 9 ))
            Args[n].Value >>= bPopupMode;
    }

    Reference< XUIConfigurationManager > xCfgMgr;
    if ( aResourceURL.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/" ))) != 0 )
        throw IllegalArgumentException();
    else
    {
        // Identify frame and determine document based ui configuration manager/module ui configuration manager
        if ( xFrame.is() && !xConfigSource.is() )
        {
            bool bHasSettings( false );
            Reference< XModel > xModel;

            Reference< XController > xController = xFrame->getController();
            if ( xController.is() )
                xModel = xController->getModel();

            if ( xModel.is() )
            {
                Reference< XUIConfigurationManagerSupplier > xUIConfigurationManagerSupplier( xModel, UNO_QUERY );
                if ( xUIConfigurationManagerSupplier.is() )
                {
                    xCfgMgr = xUIConfigurationManagerSupplier->getUIConfigurationManager();
                    bHasSettings = xCfgMgr->hasSettings( aResourceURL );
                }
            }

            if ( !bHasSettings )
            {
                rtl::OUString aModuleIdentifier = m_xModuleManager->identify( Reference< XInterface >( xFrame, UNO_QUERY ));
                if ( aModuleIdentifier.getLength() )
                {
                    Reference< ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier(
                        m_xServiceManager->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.ui.ModuleUIConfigurationManagerSupplier" ))),
                        UNO_QUERY );
                    xCfgMgr = xModuleCfgSupplier->getUIConfigurationManager( aModuleIdentifier );
                    bHasSettings = xCfgMgr->hasSettings( aResourceURL );
                }
            }
        }
    }

    PropertyValue aPropValue;
    Sequence< Any > aPropSeq( 5 );
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropValue.Value <<= xFrame;
    aPropSeq[0] <<= aPropValue;
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
    aPropValue.Value <<= xCfgMgr;
    aPropSeq[1] <<= aPropValue;
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceURL" ));
    aPropValue.Value <<= aResourceURL;
    aPropSeq[2] <<= aPropValue;
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
    aPropValue.Value <<= bPersistent;
    aPropSeq[3] <<= aPropValue;
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PopupMode" ));
    aPropValue.Value <<= bPopupMode;
    aPropSeq[4] <<= aPropValue;

    vos::OGuard aGuard( Application::GetSolarMutex() );
    ToolBarWrapper* pToolBarWrapper = new ToolBarWrapper( m_xServiceManager );
    Reference< ::com::sun::star::ui::XUIElement > xToolBar( (OWeakObject *)pToolBarWrapper, UNO_QUERY );
    Reference< XInitialization > xInit( xToolBar, UNO_QUERY );
    xInit->initialize( aPropSeq );
    return xToolBar;
}

}
