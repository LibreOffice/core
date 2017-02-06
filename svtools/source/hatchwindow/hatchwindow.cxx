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

#include <toolkit/helper/convert.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

VCLXHatchWindow::VCLXHatchWindow()
: VCLXWindow()
, pHatchWindow(nullptr)
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
    VCLXWindow* pParentComponent = VCLXWindow::GetImplementation( xParent );

    if ( pParentComponent )
        pParent = pParentComponent->GetWindow();

    OSL_ENSURE( pParent, "No parent window is provided!\n" );
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

void VCLXHatchWindow::QueryObjAreaPixel( Rectangle & aRect )
{
    if ( m_xController.is() )
    {
        awt::Rectangle aUnoRequestRect = AWTRectangle( aRect );

        try {
            awt::Rectangle aUnoResultRect = m_xController->calcAdjustedRectangle( aUnoRequestRect );
            aRect = VCLRectangle( aUnoResultRect );
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Can't adjust rectangle size!\n" );
        }
    }
}

void VCLXHatchWindow::RequestObjAreaPixel( const Rectangle & aRect )
{
    if ( m_xController.is() )
    {
        awt::Rectangle aUnoRequestRect = AWTRectangle( aRect );

        try {
            m_xController->requestPositioning( aUnoRequestRect );
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Can't request resizing!\n" );
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


uno::Any SAL_CALL VCLXHatchWindow::queryInterface( const uno::Type & rType )
{
    // Attention:
    //    Don't use mutex or guard in this method!!! Is a method of XInterface.

    uno::Any aReturn( ::cppu::queryInterface( rType,
                                           static_cast< embed::XHatchWindow* >( this ) ) );

    if ( aReturn.hasValue() )
    {
        return aReturn ;
    }

    return VCLXWindow::queryInterface( rType ) ;
}

void SAL_CALL VCLXHatchWindow::acquire()
    throw()
{
    VCLXWindow::acquire();
}

void SAL_CALL VCLXHatchWindow::release()
    throw()
{
    VCLXWindow::release();
}

uno::Sequence< uno::Type > SAL_CALL VCLXHatchWindow::getTypes()
{
    static ::cppu::OTypeCollection* pTypeCollection = nullptr ;

    if ( pTypeCollection == nullptr )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pTypeCollection == nullptr )
        {
            static ::cppu::OTypeCollection aTypeCollection(
                    cppu::UnoType<embed::XHatchWindow>::get(),
                    VCLXHatchWindow::getTypes() );

            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

uno::Sequence< sal_Int8 > SAL_CALL VCLXHatchWindow::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
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
    pHatchWindow.clear();
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
