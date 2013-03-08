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

#include <uifactory/addonstoolboxfactory.hxx>

#include <uielement/addonstoolbarwrapper.hxx>
#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>

#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

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
    , m_xModuleManager( ModuleManager::create( comphelper::getComponentContext(xServiceManager) ) )
{
}

AddonsToolBoxFactory::~AddonsToolBoxFactory()
{
}

static sal_Bool IsCorrectContext( const OUString& rModuleIdentifier, const OUString& aContextList )
{
    if ( aContextList.isEmpty() )
        return sal_True;

    if ( !rModuleIdentifier.isEmpty() )
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
    OUString aModuleIdentifier;
    try
    {
        aModuleIdentifier = m_xModuleManager->identify( rFrame );
    }
    catch ( const RuntimeException& )
    {
        throw;
    }
    catch ( const Exception& )
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
    const OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::uno::RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Sequence< Sequence< PropertyValue > >   aConfigData;
    Reference< XFrame >                     xFrame;
    OUString                           aResourceURL( ResourceURL );

    for ( sal_Int32 n = 0; n < Args.getLength(); n++ )
    {
        if ( Args[n].Name == "ConfigurationData" )
            Args[n].Value >>= aConfigData;
        else if ( Args[n].Name == "Frame" )
            Args[n].Value >>= xFrame;
        else if ( Args[n].Name == "ResourceURL" )
            Args[n].Value >>= aResourceURL;
    }

    if ( aResourceURL.indexOf( "private:resource/toolbar/addon_" ) != 0 )
        throw IllegalArgumentException();

    // Identify frame and determine module identifier to look for context based buttons
    Reference< ::com::sun::star::ui::XUIElement > xToolBar;
    if ( xFrame.is() &&
         ( aConfigData.getLength()> 0 ) &&
         hasButtonsInContext( aConfigData, xFrame ))
    {
        PropertyValue aPropValue;
        Sequence< Any > aPropSeq( 3 );
        aPropValue.Name = OUString( "Frame" );
        aPropValue.Value <<= xFrame;
        aPropSeq[0] <<= aPropValue;
        aPropValue.Name = OUString( "ConfigurationData" );
        aPropValue.Value <<= aConfigData;
        aPropSeq[1] <<= aPropValue;
        aPropValue.Name = OUString( "ResourceURL" );
        aPropValue.Value <<= aResourceURL;
        aPropSeq[2] <<= aPropValue;

        SolarMutexGuard aGuard;
        AddonsToolBarWrapper* pToolBarWrapper = new AddonsToolBarWrapper( m_xServiceManager );
        xToolBar = Reference< ::com::sun::star::ui::XUIElement >( (OWeakObject *)pToolBarWrapper, UNO_QUERY );
        Reference< XInitialization > xInit( xToolBar, UNO_QUERY );
        xInit->initialize( aPropSeq );
    }

    return xToolBar;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
