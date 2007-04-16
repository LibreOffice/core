/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: menubarfactory.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-04-16 16:47:07 $
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
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_UIFACTORY_MENUBARFACTORY_HXX_
#include <uifactory/menubarfactory.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include "services.h"
#endif

#ifndef __FRAMEWORK_UIELEMENT_MENUBARWRAPPER_HXX_
#include <uielement/menubarwrapper.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPLLIER_HPP_
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

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
DEFINE_XINTERFACE_3                    (    MenuBarFactory                                                  ,
                                            OWeakObject                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                       ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIElementFactory )
                                        )

DEFINE_XTYPEPROVIDER_3                  (   MenuBarFactory                                  ,
                                            css::lang::XTypeProvider                        ,
                                            css::lang::XServiceInfo                         ,
                                            ::com::sun::star::ui::XUIElementFactory
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   MenuBarFactory                                  ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_MENUBARFACTORY                      ,
                                            IMPLEMENTATIONNAME_MENUBARFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   MenuBarFactory, {} )

MenuBarFactory::MenuBarFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase()
    , m_xServiceManager( xServiceManager )
    , m_xModuleManager( xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY )
{
}

MenuBarFactory::~MenuBarFactory()
{
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL MenuBarFactory::createUIElement(
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
    sal_Bool                             bMenuOnly( sal_False );

    for ( sal_Int32 n = 0; n < Args.getLength(); n++ )
    {
        if ( Args[n].Name.equalsAscii( "ConfigurationSource" ))
            Args[n].Value >>= xConfigSource;
        else if ( Args[n].Name.equalsAscii( "Frame" ))
            Args[n].Value >>= xFrame;
        else if ( Args[n].Name.equalsAscii( "ResourceURL" ))
            Args[n].Value >>= aResourceURL;
        else if ( Args[n].Name.equalsAscii( "Persistent" ))
            Args[n].Value >>= bPersistent;
        else if ( Args[n].Name.equalsAscii( "MenuOnly" ))
            Args[n].Value >>= bMenuOnly;
    }

    Reference< XUIConfigurationManager > xCfgMgr;
    if ( aResourceURL.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/menubar/" ))) != 0 )
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
                        m_xServiceManager->createInstance( SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ), UNO_QUERY );
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
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MenuOnly" ));
    aPropValue.Value <<= bMenuOnly;
    aPropSeq[4] <<= aPropValue;

    vos::OGuard aGuard( Application::GetSolarMutex() );
    MenuBarWrapper* pMenuBarWrapper = new MenuBarWrapper( m_xServiceManager );
    Reference< ::com::sun::star::ui::XUIElement > xMenuBar( (OWeakObject *)pMenuBarWrapper, UNO_QUERY );
    Reference< XInitialization > xInit( xMenuBar, UNO_QUERY );
    xInit->initialize( aPropSeq );
    return xMenuBar;
}

} // namespace framework
