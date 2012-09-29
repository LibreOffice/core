/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <rtl/ref.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include "uielement/popupmenucontroller.hxx"
#include "services.h"

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

// --------------------------------------------------------------------


namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   PopupMenuController                         ,
                                            OWeakObject                      ,
                                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ) )            ,
                                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.framework.PopupMenuController" ) )
                                        )

DEFINE_INIT_SERVICE                     (   PopupMenuController, {} )

class PopupMenuControllerImpl
{
};

//========================================================================
// class PopupMenuController
//========================================================================

PopupMenuController::PopupMenuController( const Reference< lang::XMultiServiceFactory >& rServiceManager )
: svt::ToolboxController( rServiceManager, Reference< frame::XFrame >(), OUString() )
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
    Reference< XMultiComponentFactory > xPopupMenuControllerRegistration( getServiceManager()->createInstance( SERVICENAME_POPUPMENUCONTROLLERFACTORY ), UNO_QUERY_THROW );

    Sequence< Any > aSeq( 2 );
    PropertyValue aPropValue;

    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleName" ));
    aPropValue.Value <<= getModuleName();
    aSeq[0] <<= aPropValue;
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropValue.Value <<= m_xFrame;
    aSeq[1] <<= aPropValue;

    Reference< XComponentContext > xComponentContext(
        comphelper::getComponentContext( getServiceManager() ) );

    Reference< XPopupMenuController > xPopupMenuController( xPopupMenuControllerRegistration->createInstanceWithArgumentsAndContext( getCommandURL(), aSeq, xComponentContext ), UNO_QUERY );
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
            mxPopupMenu = Reference< awt::XPopupMenu >( getServiceManager()->createInstance( DECLARE_ASCII( "stardiv.Toolkit.VCLXPopupMenu" ) ), UNO_QUERY_THROW );
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
