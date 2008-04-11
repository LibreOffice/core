/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: addonstoolboxfactory.cxx,v $
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
#include <uifactory/addonstoolboxfactory.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <uielement/addonstoolbarwrapper.hxx>
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
DEFINE_XINTERFACE_3                    (    AddonsToolBoxFactory                                            ,
                                            OWeakObject                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                       ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIElementFactory )
                                        )

DEFINE_XTYPEPROVIDER_3                  (   AddonsToolBoxFactory                            ,
                                            css::lang::XTypeProvider                        ,
                                            css::lang::XServiceInfo                         ,
                                            ::com::sun::star::ui::XUIElementFactory
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   AddonsToolBoxFactory                            ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_TOOLBARFACTORY                      ,
                                            IMPLEMENTATIONNAME_ADDONSTOOLBARFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   AddonsToolBoxFactory, {} )

AddonsToolBoxFactory::AddonsToolBoxFactory(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_xServiceManager( xServiceManager )
    , m_xModuleManager( xServiceManager->createInstance(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ModuleManager" ))),
                        UNO_QUERY )
{
}

AddonsToolBoxFactory::~AddonsToolBoxFactory()
{
}

static sal_Bool IsCorrectContext( const ::rtl::OUString& rModuleIdentifier, const rtl::OUString& aContextList )
{
    if ( aContextList.getLength() == 0 )
        return sal_True;

    if ( rModuleIdentifier.getLength() > 0 )
    {
        sal_Int32 nIndex = aContextList.indexOf( rModuleIdentifier );
        return ( nIndex >= 0 );
    }

    return sal_False;
}

sal_Bool AddonsToolBoxFactory::hasButtonsInContext(
    const Sequence< Sequence< PropertyValue > >& rPropSeqSeq,
    const Reference< XFrame >& rFrame )
{
    ::rtl::OUString aModuleIdentifier;
    try
    {
        aModuleIdentifier = m_xModuleManager->identify( rFrame );
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( Exception& )
    {
    }

    // Check before we create a toolbar that we have at least one button in
    // the current frame context.
    for ( sal_uInt32 i = 0; i < (sal_uInt32)rPropSeqSeq.getLength(); i++ )
    {
        sal_Bool    bIsButton( sal_True );
        sal_Bool    bIsCorrectContext( sal_False );
        sal_uInt32  nPropChecked( 0 );

        const Sequence< PropertyValue >& rPropSeq = rPropSeqSeq[i];
        for ( sal_uInt32 j = 0; j < (sal_uInt32)rPropSeq.getLength(); j++ )
        {
            if ( rPropSeq[j].Name.equalsAsciiL( "Context", 7 ))
            {
                OUString aContextList;
                if ( rPropSeq[j].Value >>= aContextList )
                    bIsCorrectContext = IsCorrectContext( aModuleIdentifier, aContextList );
                nPropChecked++;
            }
            else if ( rPropSeq[j].Name.equalsAsciiL( "URL", 3 ))
            {
                OUString aURL;
                rPropSeq[j].Value >>= aURL;
                bIsButton = !aURL.equalsAsciiL( "private:separator", 17 );
                nPropChecked++;
            }

            if ( nPropChecked == 2 )
                break;
        }

        if ( bIsButton && bIsCorrectContext )
            return sal_True;
    }

    return sal_False;
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL AddonsToolBoxFactory::createUIElement(
    const ::rtl::OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::uno::RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Sequence< Sequence< PropertyValue > >   aConfigData;
    Reference< XFrame >                     xFrame;
    rtl::OUString                           aResourceURL( ResourceURL );

    for ( sal_Int32 n = 0; n < Args.getLength(); n++ )
    {
        if ( Args[n].Name.equalsAscii( "ConfigurationData" ))
            Args[n].Value >>= aConfigData;
        else if ( Args[n].Name.equalsAscii( "Frame" ))
            Args[n].Value >>= xFrame;
        else if ( Args[n].Name.equalsAscii( "ResourceURL" ))
            Args[n].Value >>= aResourceURL;
    }

    if ( aResourceURL.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/addon_" ))) != 0 )
        throw IllegalArgumentException();

    // Identify frame and determine module identifier to look for context based buttons
    Reference< ::com::sun::star::ui::XUIElement > xToolBar;
    if ( xFrame.is() &&
         ( aConfigData.getLength()> 0 ) &&
         hasButtonsInContext( aConfigData, xFrame ))
    {
        PropertyValue aPropValue;
        Sequence< Any > aPropSeq( 3 );
        aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
        aPropValue.Value <<= xFrame;
        aPropSeq[0] <<= aPropValue;
        aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationData" ));
        aPropValue.Value <<= aConfigData;
        aPropSeq[1] <<= aPropValue;
        aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceURL" ));
        aPropValue.Value <<= aResourceURL;
        aPropSeq[2] <<= aPropValue;

        vos::OGuard aGuard( Application::GetSolarMutex() );
        AddonsToolBarWrapper* pToolBarWrapper = new AddonsToolBarWrapper( m_xServiceManager );
        xToolBar = Reference< ::com::sun::star::ui::XUIElement >( (OWeakObject *)pToolBarWrapper, UNO_QUERY );
        Reference< XInitialization > xInit( xToolBar, UNO_QUERY );
        xInit->initialize( aPropSeq );
    }

    return xToolBar;
}

}
