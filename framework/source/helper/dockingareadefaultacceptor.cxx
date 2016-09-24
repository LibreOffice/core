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

#include <helper/dockingareadefaultacceptor.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>

#include <vcl/svapp.hxx>

namespace framework{

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::cppu;
using namespace ::osl;

//  constructor

DockingAreaDefaultAcceptor::DockingAreaDefaultAcceptor( const   css::uno::Reference< XFrame >&      xOwner  )
        :   m_xOwner        ( xOwner    )
{
}

//  destructor

DockingAreaDefaultAcceptor::~DockingAreaDefaultAcceptor()
{
}

//  XDockingAreaAcceptor
css::uno::Reference< css::awt::XWindow > SAL_CALL DockingAreaDefaultAcceptor::getContainerWindow() throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    // Try to "lock" the frame for access to taskscontainer.
    css::uno::Reference< XFrame > xFrame( m_xOwner );
    //TODO: check xFrame for null?
    css::uno::Reference< css::awt::XWindow > xContainerWindow( xFrame->getContainerWindow() );

    return xContainerWindow;
}

sal_Bool SAL_CALL DockingAreaDefaultAcceptor::requestDockingAreaSpace( const css::awt::Rectangle& RequestedSpace ) throw (css::uno::RuntimeException, std::exception)
{
    // Try to "lock" the frame for access to taskscontainer.
    css::uno::Reference< XFrame > xFrame( m_xOwner );

    if ( xFrame.is() )
    {
        css::uno::Reference< css::awt::XWindow > xContainerWindow( xFrame->getContainerWindow() );
        css::uno::Reference< css::awt::XWindow > xComponentWindow( xFrame->getComponentWindow() );

        if ( xContainerWindow.is() && xComponentWindow.is() )
        {
            css::uno::Reference< css::awt::XDevice > xDevice( xContainerWindow, css::uno::UNO_QUERY );
            // Convert relativ size to output size.
            css::awt::Rectangle  aRectangle  = xContainerWindow->getPosSize();
            css::awt::DeviceInfo aInfo       = xDevice->getInfo();
            css::awt::Size       aSize       (  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                                aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

            css::awt::Size aMinSize( 0, 0 ); // = xLayoutContrains->getMinimumSize();

            // Check if request border space would decrease component window size below minimum size
            if ((( aSize.Width - RequestedSpace.X - RequestedSpace.Width ) < aMinSize.Width ) ||
                (( aSize.Height - RequestedSpace.Y - RequestedSpace.Height ) < aMinSize.Height  )       )
                return false;

            return true;
        }
    }

    return false;
}

void SAL_CALL DockingAreaDefaultAcceptor::setDockingAreaSpace( const css::awt::Rectangle& BorderSpace ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    // Try to "lock" the frame for access to taskscontainer.
    css::uno::Reference< XFrame > xFrame( m_xOwner );
    if ( xFrame.is() )
    {
        css::uno::Reference< css::awt::XWindow > xContainerWindow( xFrame->getContainerWindow() );
        css::uno::Reference< css::awt::XWindow > xComponentWindow( xFrame->getComponentWindow() );

        if ( xContainerWindow.is() && xComponentWindow.is() )
        {
            css::uno::Reference< css::awt::XDevice > xDevice( xContainerWindow, css::uno::UNO_QUERY );
            // Convert relativ size to output size.
            css::awt::Rectangle  aRectangle  = xContainerWindow->getPosSize();
            css::awt::DeviceInfo aInfo       = xDevice->getInfo();
            css::awt::Size       aSize       (  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                                aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );
            css::awt::Size aMinSize( 0, 0 );// = xLayoutContrains->getMinimumSize();

            // Check if request border space would decrease component window size below minimum size
            sal_Int32 nWidth     = aSize.Width - BorderSpace.X - BorderSpace.Width;
            sal_Int32 nHeight    = aSize.Height - BorderSpace.Y - BorderSpace.Height;

            if (( nWidth > aMinSize.Width ) && ( nHeight > aMinSize.Height ))
            {
                // Resize our component window.
                xComponentWindow->setPosSize( BorderSpace.X, BorderSpace.Y, nWidth, nHeight, css::awt::PosSize::POSSIZE );
            }
        }
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
