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

#include <com/sun/star/embed/XHatchWindowController.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include "hatchwindow.hxx"
#include "ipwin.hxx"

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>

using namespace ::com::sun::star;

VCLXHatchWindow::VCLXHatchWindow()
: pHatchWindow(nullptr)
{
}

VCLXHatchWindow::~VCLXHatchWindow()
{
}

void VCLXHatchWindow::initializeWindow( const uno::Reference< awt::XWindowPeer >& xParent,
                const awt::Rectangle& aBounds,
                const awt::Size& aSize )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pParent;
    VCLXWindow* pParentComponent = dynamic_cast<VCLXWindow*>( xParent.get() );

    if ( pParentComponent )
        pParent = pParentComponent->GetWindow();

    OSL_ENSURE( pParent, "No parent window is provided!" );
    if ( !pParent )
        throw lang::IllegalArgumentException(); // TODO

    pHatchWindow = VclPtr<SvResizeWindow>::Create( pParent, this );
    pHatchWindow->setPosSizePixel( aBounds.X, aBounds.Y, aBounds.Width, aBounds.Height );
    aHatchBorderSize = aSize;
    pHatchWindow->SetHatchBorderPixel( Size( aSize.Width, aSize.Height ) );

    SetWindow( pHatchWindow );
    pHatchWindow->SetComponentInterface( this );

    //pHatchWindow->Show();
}

void VCLXHatchWindow::QueryObjAreaPixel( tools::Rectangle & aRect )
{
    if ( !m_xController.is() )
        return;

    awt::Rectangle aUnoRequestRect = vcl::unohelper::ConvertToAWTRect(aRect);

    try {
        awt::Rectangle aUnoResultRect = m_xController->calcAdjustedRectangle( aUnoRequestRect );
        aRect = vcl::unohelper::ConvertToVCLRect(aUnoResultRect);
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( "Can't adjust rectangle size!" );
    }
}

void VCLXHatchWindow::RequestObjAreaPixel( const tools::Rectangle & aRect )
{
    if ( m_xController.is() )
    {
        awt::Rectangle aUnoRequestRect = vcl::unohelper::ConvertToAWTRect(aRect);

        try {
            m_xController->requestPositioning( aUnoRequestRect );
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Can't request resizing!" );
        }
    }
}

void VCLXHatchWindow::InplaceDeactivate()
{
    if ( m_xController.is() )
    {
        // TODO: communicate with controller
    }
}


css::awt::Size SAL_CALL VCLXHatchWindow::getHatchBorderSize()
{
    return aHatchBorderSize;
}

void SAL_CALL VCLXHatchWindow::setHatchBorderSize( const css::awt::Size& _hatchbordersize )
{
    if ( pHatchWindow )
    {
        aHatchBorderSize = _hatchbordersize;
        pHatchWindow->SetHatchBorderPixel( Size( aHatchBorderSize.Width, aHatchBorderSize.Height ) );
    }
}

void SAL_CALL VCLXHatchWindow::setController( const uno::Reference< embed::XHatchWindowController >& xController )
{
    m_xController = xController;
}

void SAL_CALL VCLXHatchWindow::dispose()
{
    pHatchWindow.reset();
    VCLXWindow::dispose();
}

void SAL_CALL VCLXHatchWindow::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    VCLXWindow::addEventListener( xListener );
}

void SAL_CALL VCLXHatchWindow::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    VCLXWindow::removeEventListener( xListener );
}

void VCLXHatchWindow::Activated()
{
    if ( m_xController.is() )
        m_xController->activated();
}

void VCLXHatchWindow::Deactivated()
{
    if ( m_xController.is() )
        m_xController->deactivated();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
