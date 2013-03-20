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


#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/PopupMenuControllerFactory.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <rtl/ref.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include "uielement/popupmenucontroller.hxx"
#include "services.h"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

// --------------------------------------------------------------------


namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE_2        (   PopupMenuController                         ,
                                            OWeakObject                      ,
                                            OUString( "com.sun.star.frame.ToolbarController" )            ,
                                            OUString( "com.sun.star.comp.framework.PopupMenuController" )
                                        )

DEFINE_INIT_SERVICE                     (   PopupMenuController, {} )

class PopupMenuControllerImpl
{
};

//========================================================================
// class PopupMenuController
//========================================================================

PopupMenuController::PopupMenuController( const Reference< uno::XComponentContext >& rxContext )
: svt::ToolboxController( rxContext, Reference< frame::XFrame >(), OUString() )
{
}

// --------------------------------------------------------------------

PopupMenuController::~PopupMenuController()
{
}

// --------------------------------------------------------------------
// XInterface
// --------------------------------------------------------------------

Any SAL_CALL PopupMenuController::queryInterface( const Type& aType )  throw (RuntimeException)
{
    Any a( ToolboxController::queryInterface( aType ) );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< lang::XServiceInfo* >( this ));
}

// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::acquire() throw ()
{
    ToolboxController::acquire();
}

// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::release() throw ()
{
    ToolboxController::release();
}

// --------------------------------------------------------------------
// XComponent
// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::dispose() throw (RuntimeException)
{
    if( mxPopupMenuController.is() )
    {
        Reference< XComponent > xComponent( mxPopupMenuController, UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();
        mxPopupMenuController.clear();
    }

    mxPopupMenu.clear();

    svt::ToolboxController::dispose();
}

// --------------------------------------------------------------------
// XStatusListener
// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::statusChanged( const frame::FeatureStateEvent& rEvent ) throw ( RuntimeException )
{
    svt::ToolboxController::statusChanged(rEvent);
    enable( rEvent.IsEnabled );
}

// --------------------------------------------------------------------
// XToolbarController
// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::execute( sal_Int16 KeyModifier ) throw (RuntimeException)
{
    svt::ToolboxController::execute( KeyModifier );
}

// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::click() throw (RuntimeException)
{
    svt::ToolboxController::click();
}

// --------------------------------------------------------------------

void SAL_CALL PopupMenuController::doubleClick() throw (RuntimeException)
{
    svt::ToolboxController::doubleClick();
}

// --------------------------------------------------------------------

bool PopupMenuController::CreatePopupMenuController() throw (Exception)
{
    Reference< XToolbarControllerFactory > xPopupMenuControllerRegistration = PopupMenuControllerFactory::create( m_xContext );

    Sequence< Any > aSeq( 2 );
    PropertyValue aPropValue;

    aPropValue.Name = OUString( "ModuleName" );
    aPropValue.Value <<= getModuleName();
    aSeq[0] <<= aPropValue;
    aPropValue.Name = OUString( "Frame" );
    aPropValue.Value <<= m_xFrame;
    aSeq[1] <<= aPropValue;

    Reference< XPopupMenuController > xPopupMenuController( xPopupMenuControllerRegistration->createInstanceWithArgumentsAndContext( getCommandURL(), aSeq, m_xContext ), UNO_QUERY );
    if ( xPopupMenuController.is() )
    {
        mxPopupMenuController = xPopupMenuController;
        return true;
    }
    return false;
}

Reference< awt::XWindow > SAL_CALL PopupMenuController::createPopupWindow() throw (RuntimeException)
{
    SolarMutexGuard aSolarLock;

    Reference< awt::XWindow > xRet;

    try
    {
        ToolBox* pToolBox = dynamic_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ) );
        if( !pToolBox )
            return xRet;

        // get selected button
        sal_uInt16 nItemId = pToolBox->GetDownItemId();
        if( !nItemId )
            return xRet;

        ::Rectangle aRect( pToolBox->GetItemRect( nItemId ) );

        if( !mxPopupMenuController.is() && !CreatePopupMenuController() )
            return xRet;

        if( !mxPopupMenu.is() )
        {
            mxPopupMenu = Reference< awt::XPopupMenu >(
                m_xContext->getServiceManager()->createInstanceWithContext("stardiv.Toolkit.VCLXPopupMenu", m_xContext), UNO_QUERY_THROW );
            mxPopupMenuController->setPopupMenu( mxPopupMenu );
        }
        else
        {
            mxPopupMenuController->updatePopupMenu();

        }
        pToolBox->SetItemDown( nItemId, sal_True );
        Reference< awt::XWindowPeer > xPeer( getParent(), UNO_QUERY_THROW );
        mxPopupMenu->execute( xPeer, VCLUnoHelper::ConvertToAWTRect( aRect ), 0 );
        pToolBox->SetItemDown( nItemId, sal_False );
    }
    catch( const Exception& )
    {
    }

    return xRet;
}

// --------------------------------------------------------------------

Reference< awt::XWindow > SAL_CALL PopupMenuController::createItemWindow( const Reference< awt::XWindow >& /*Parent*/ )  throw (RuntimeException)
{
    return Reference< awt::XWindow >();
}

// --------------------------------------------------------------------

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
