/*************************************************************************
 *
 *  $RCSfile: toolboxfactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 17:03:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UIFACTORY_TOOLBOXFACTORY_HXX_
#include <uifactory/toolboxfactory.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARWRAPPER_HXX_
#include <uielement/toolbarwrapper.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
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

#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPLLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

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

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace drafts::com::sun::star::ui;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_3                    (    ToolBoxFactory                                                  ,
                                            OWeakObject                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                       ),
                                            DIRECT_INTERFACE( drafts::com::sun::star::ui::XUIElementFactory )
                                        )

DEFINE_XTYPEPROVIDER_3                  (   ToolBoxFactory                                  ,
                                            css::lang::XTypeProvider                        ,
                                            css::lang::XServiceInfo                         ,
                                            drafts::com::sun::star::ui::XUIElementFactory
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
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.frame.ModuleManager" ))),
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
                    Reference< ::drafts::com::sun::star::ui::XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier(
                        m_xServiceManager->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "drafts.com.sun.star.ui.ModuleUIConfigurationManagerSupplier" ))),
                        UNO_QUERY );
                    xCfgMgr = xModuleCfgSupplier->getUIConfigurationManager( aModuleIdentifier );
                    bHasSettings = xCfgMgr->hasSettings( aResourceURL );
                }
            }
        }
    }

    PropertyValue aPropValue;
    Sequence< Any > aPropSeq( 4 );
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

    vos::OGuard aGuard( Application::GetSolarMutex() );
    ToolBarWrapper* pToolBarWrapper = new ToolBarWrapper( m_xServiceManager );
    Reference< drafts::com::sun::star::ui::XUIElement > xToolBar( (OWeakObject *)pToolBarWrapper, UNO_QUERY );
    Reference< XInitialization > xInit( xToolBar, UNO_QUERY );
    xInit->initialize( aPropSeq );
    return xToolBar;
}

}
