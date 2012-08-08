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

#include <helper/dockingareadefaultacceptor.hxx>
#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>

#include <vcl/svapp.hxx>

namespace framework{

using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::uno           ;
using namespace ::cppu                          ;
using namespace ::osl                           ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
DockingAreaDefaultAcceptor::DockingAreaDefaultAcceptor( const   css::uno::Reference< XFrame >&      xOwner  )
        //  Init baseclasses first
        :   ThreadHelpBase  ( &Application::GetSolarMutex() )
        // Init member
        ,   m_xOwner        ( xOwner    )
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
DockingAreaDefaultAcceptor::~DockingAreaDefaultAcceptor()
{
}

//*****************************************************************************************************************
//  XDockingAreaAcceptor
//*****************************************************************************************************************
css::uno::Reference< css::awt::XWindow > SAL_CALL DockingAreaDefaultAcceptor::getContainerWindow() throw (css::uno::RuntimeException)
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Try to "lock" the frame for access to taskscontainer.
    css::uno::Reference< XFrame > xFrame( m_xOwner.get(), UNO_QUERY );
    css::uno::Reference< css::awt::XWindow > xContainerWindow( xFrame->getContainerWindow() );

    return xContainerWindow;
}

sal_Bool SAL_CALL DockingAreaDefaultAcceptor::requestDockingAreaSpace( const css::awt::Rectangle& RequestedSpace ) throw (css::uno::RuntimeException)
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Try to "lock" the frame for access to taskscontainer.
    css::uno::Reference< XFrame > xFrame( m_xOwner.get(), UNO_QUERY );
    aGuard.unlock();

    if ( xFrame.is() == sal_True )
    {
        css::uno::Reference< css::awt::XWindow > xContainerWindow( xFrame->getContainerWindow() );
        css::uno::Reference< css::awt::XWindow > xComponentWindow( xFrame->getComponentWindow() );

        if (( xContainerWindow.is() == sal_True ) &&
            ( xComponentWindow.is() == sal_True )       )
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
                return sal_False;

            return sal_True;
        }
    }

    return sal_False;
}

void SAL_CALL DockingAreaDefaultAcceptor::setDockingAreaSpace( const css::awt::Rectangle& BorderSpace ) throw (css::uno::RuntimeException)
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Try to "lock" the frame for access to taskscontainer.
    css::uno::Reference< XFrame > xFrame( m_xOwner.get(), UNO_QUERY );
    if ( xFrame.is() == sal_True )
    {
        css::uno::Reference< css::awt::XWindow > xContainerWindow( xFrame->getContainerWindow() );
        css::uno::Reference< css::awt::XWindow > xComponentWindow( xFrame->getComponentWindow() );

        if (( xContainerWindow.is() == sal_True ) &&
            ( xComponentWindow.is() == sal_True )       )
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
