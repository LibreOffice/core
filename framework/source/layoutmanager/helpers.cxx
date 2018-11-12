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

#include "helpers.hxx"
#include <services.h>

#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/awt/XDockableWindowListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/ui/XUIElement.hpp>

#include <unotools/mediadescriptor.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace com::sun::star;

namespace framework
{

bool hasEmptySize( const css::awt::Size& rSize )
{
    return ( rSize.Width == 0 ) && ( rSize.Height == 0 );
}

bool hasDefaultPosValue( const css::awt::Point& rPos )
{
    return (( rPos.X == SAL_MAX_INT32 ) || ( rPos.Y == SAL_MAX_INT32 ));
}

bool isDefaultPos( const css::awt::Point& rPos )
{
    return (( rPos.X == SAL_MAX_INT32 ) && ( rPos.Y == SAL_MAX_INT32 ));
}

bool isReverseOrderDockingArea( const sal_Int32 nDockArea )
{
    ui::DockingArea eDockArea = static_cast< ui::DockingArea >( nDockArea );
    return (( eDockArea == ui::DockingArea_DOCKINGAREA_BOTTOM ) ||
            ( eDockArea == ui::DockingArea_DOCKINGAREA_RIGHT ));
}

bool isToolboxHorizontalAligned( ToolBox const * pToolBox )
{
    if ( pToolBox )
        return (( pToolBox->GetAlign() == WindowAlign::Top ) || ( pToolBox->GetAlign() == WindowAlign::Bottom ));
    return false;
}

bool isHorizontalDockingArea( const ui::DockingArea& nDockingArea )
{
    return (( nDockingArea == ui::DockingArea_DOCKINGAREA_TOP ) ||
            ( nDockingArea == ui::DockingArea_DOCKINGAREA_BOTTOM ));
}

bool isHorizontalDockingArea( const sal_Int32 nDockArea )
{
  return isHorizontalDockingArea(static_cast< ui::DockingArea >( nDockArea ));
}

OUString retrieveToolbarNameFromHelpURL( vcl::Window* pWindow )
{
    OUString aToolbarName;

    if ( pWindow->GetType() == WindowType::TOOLBOX )
    {
        ToolBox* pToolBox = dynamic_cast<ToolBox *>( pWindow );
        if ( pToolBox )
        {
            aToolbarName = OStringToOUString( pToolBox->GetHelpId(), RTL_TEXTENCODING_UTF8 );
            sal_Int32 i = aToolbarName.lastIndexOf( ':' );
            if ( !aToolbarName.isEmpty() && ( i > 0 ) && (( i + 1 ) < aToolbarName.getLength() ))
                aToolbarName = aToolbarName.copy( i+1 ); // Remove ".HelpId:" protocol from toolbar name
            else
              aToolbarName.clear();
        }
    }
    return aToolbarName;
}

ToolBox* getToolboxPtr( vcl::Window* pWindow )
{
    ToolBox* pToolbox(nullptr);
    if ( pWindow->GetType() == WindowType::TOOLBOX )
        pToolbox = dynamic_cast<ToolBox*>( pWindow );
    return pToolbox;
}

vcl::Window* getWindowFromXUIElement( const uno::Reference< ui::XUIElement >& xUIElement )
{
    SolarMutexGuard aGuard;
    uno::Reference< awt::XWindow > xWindow;
    if ( xUIElement.is() )
        xWindow.set( xUIElement->getRealInterface(), uno::UNO_QUERY );
    return VCLUnoHelper::GetWindow( xWindow );
}

SystemWindow* getTopSystemWindow( const uno::Reference< awt::XWindow >& xWindow )
{
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    while ( pWindow && !pWindow->IsSystemWindow() )
        pWindow = pWindow->GetParent();

    if ( pWindow )
        return static_cast<SystemWindow *>(pWindow.get());
    else
        return nullptr;
}

void setZeroRectangle( ::tools::Rectangle& rRect )
{
    rRect.setX(0);
    rRect.setY(0);
    rRect.setWidth(0);
    rRect.setHeight(0);
}

// ATTENTION!
// This value is directly copied from the sfx2 project.
// You have to change BOTH values, see sfx2/inc/sfx2/sfxsids.hrc (SID_DOCKWIN_START)
static const sal_Int32 DOCKWIN_ID_BASE = 9800;

bool lcl_checkUIElement(const uno::Reference< ui::XUIElement >& xUIElement, awt::Rectangle& _rPosSize, uno::Reference< awt::XWindow >& _xWindow)
{
    bool bRet = xUIElement.is();
    if ( bRet )
    {
        SolarMutexGuard aGuard;
        _xWindow.set( xUIElement->getRealInterface(), uno::UNO_QUERY );
        _rPosSize = _xWindow->getPosSize();

        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( _xWindow );
        if ( pWindow->GetType() == WindowType::TOOLBOX )
        {
            ::Size aSize = static_cast<ToolBox*>(pWindow.get())->CalcWindowSizePixel( 1 );
            _rPosSize.Width = aSize.Width();
            _rPosSize.Height = aSize.Height();
        }
    } // if ( xUIElement.is() )
    return bRet;
}

uno::Reference< awt::XWindowPeer > createToolkitWindow( const uno::Reference< uno::XComponentContext >& rxContext, const uno::Reference< awt::XWindowPeer >& rParent, const char* pService )
{
    uno::Reference< awt::XToolkit2 > xToolkit = awt::Toolkit::create( rxContext );

    // describe window properties.
    css::awt::WindowDescriptor aDescriptor;
    aDescriptor.Type                =   awt::WindowClass_SIMPLE;
    aDescriptor.WindowServiceName   =   OUString::createFromAscii( pService );
    aDescriptor.ParentIndex         =   -1;
    aDescriptor.Parent.set( rParent, uno::UNO_QUERY );
    aDescriptor.Bounds              =   awt::Rectangle(0,0,0,0);
    aDescriptor.WindowAttributes    =   0;

    // create a awt window
    uno::Reference< awt::XWindowPeer > xPeer = xToolkit->createWindow( aDescriptor );

    return xPeer;
}

// convert alignment constant to vcl's WindowAlign type
WindowAlign ImplConvertAlignment( ui::DockingArea aAlignment )
{
    if ( aAlignment == ui::DockingArea_DOCKINGAREA_LEFT )
        return WindowAlign::Left;
    else if ( aAlignment == ui::DockingArea_DOCKINGAREA_RIGHT )
        return WindowAlign::Right;
    else if ( aAlignment == ui::DockingArea_DOCKINGAREA_TOP )
        return WindowAlign::Top;
    else
        return WindowAlign::Bottom;
}

OUString getElementTypeFromResourceURL( const OUString& aResourceURL )
{
    OUString aUIResourceURL( UIRESOURCE_URL );
    if ( aResourceURL.startsWith( aUIResourceURL ) )
    {
        sal_Int32       nIndex = 0;
        OUString aPathPart   = aResourceURL.copy( aUIResourceURL.getLength() );
        aPathPart.getToken( 0, '/', nIndex );

        return aPathPart.getToken( 0, '/', nIndex );
    }

    return OUString();
}

void parseResourceURL( const OUString& aResourceURL, OUString& aElementType, OUString& aElementName )
{
    OUString aUIResourceURL( UIRESOURCE_URL );
    if ( aResourceURL.startsWith( aUIResourceURL ) )
    {
        sal_Int32       nIndex = 0;
        OUString aPathPart   = aResourceURL.copy( aUIResourceURL.getLength() );
        aPathPart.getToken( 0, '/', nIndex );

        aElementType = aPathPart.getToken( 0, '/', nIndex );
        aElementName = aPathPart.getToken( 0, '/', nIndex );
    }
}

css::awt::Rectangle putRectangleValueToAWT( const ::tools::Rectangle& rRect )
{
    css::awt::Rectangle aRect;
    aRect.X = rRect.Left();
    aRect.Y = rRect.Top();
    aRect.Width = rRect.Right();
    aRect.Height = rRect.Bottom();

    return aRect;
}

::tools::Rectangle putAWTToRectangle( const css::awt::Rectangle& rRect )
{
    ::tools::Rectangle aRect;
    aRect.SetLeft( rRect.X );
    aRect.SetTop( rRect.Y );
    aRect.SetRight( rRect.Width );
    aRect.SetBottom( rRect.Height );

    return aRect;
}

bool equalRectangles( const css::awt::Rectangle& rRect1,
                      const css::awt::Rectangle& rRect2 )
{
    return (( rRect1.X == rRect2.X ) &&
            ( rRect1.Y == rRect2.Y ) &&
            ( rRect1.Width == rRect2.Width ) &&
            ( rRect1.Height == rRect2.Height ));
}

uno::Reference< frame::XModel > impl_getModelFromFrame( const uno::Reference< frame::XFrame >& rFrame )
{
    // Query for the model to get check the context information
    uno::Reference< frame::XModel > xModel;
    if ( rFrame.is() )
    {
        uno::Reference< frame::XController > xController( rFrame->getController(), uno::UNO_QUERY );
        if ( xController.is() )
            xModel = xController->getModel();
    }

    return xModel;
}

bool implts_isPreviewModel( const uno::Reference< frame::XModel >& xModel )
{
    if ( xModel.is() )
    {
        utl::MediaDescriptor aDesc( xModel->getArgs() );
        return aDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_PREVIEW(), false);
    }
    else
        return false;
}

bool implts_isFrameOrWindowTop( const uno::Reference< frame::XFrame >& xFrame )
{
    if (xFrame->isTop())
        return true;

    uno::Reference< awt::XTopWindow > xWindowCheck(xFrame->getContainerWindow(), uno::UNO_QUERY); // don't use _THROW here ... it's a check only
    if (xWindowCheck.is())
    {
        // #i76867# top and system window is required.
        SolarMutexGuard aGuard;
        uno::Reference< awt::XWindow > xWindow( xWindowCheck, uno::UNO_QUERY );
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        return pWindow && pWindow->IsSystemWindow();
    }

    return false;
}

void impl_setDockingWindowVisibility( const css::uno::Reference< css::uno::XComponentContext>& rxContext, const css::uno::Reference< css::frame::XFrame >& rFrame, const OUString& rDockingWindowName, bool bVisible )
{
    sal_Int32 nID    = rDockingWindowName.toInt32();
    sal_Int32 nIndex = nID - DOCKWIN_ID_BASE;

    css::uno::Reference< css::frame::XDispatchProvider > xProvider(rFrame, css::uno::UNO_QUERY);
    if ( nIndex >= 0 && xProvider.is() )
    {
        OUString aDockWinArgName = "DockingWindow" + OUString::number( nIndex );

        css::uno::Sequence< css::beans::PropertyValue > aArgs(1);
        aArgs[0].Name  = aDockWinArgName;
        aArgs[0].Value <<= bVisible;

        css::uno::Reference< css::frame::XDispatchHelper > xDispatcher = css::frame::DispatchHelper::create( rxContext );

        OUString aDockWinCommand = ".uno:" + aDockWinArgName;
        xDispatcher->executeDispatch(
            xProvider,
            aDockWinCommand,
            "_self",
            0,
            aArgs);
    }
}

void impl_addWindowListeners(
    const css::uno::Reference< css::uno::XInterface >& xThis,
    const css::uno::Reference< css::ui::XUIElement >& xUIElement )
{
    css::uno::Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XDockableWindow > xDockWindow( xUIElement->getRealInterface(), css::uno::UNO_QUERY );
    if ( xDockWindow.is() && xWindow.is() )
    {
        try
        {
            xDockWindow->addDockableWindowListener(
                css::uno::Reference< css::awt::XDockableWindowListener >(
                    xThis, css::uno::UNO_QUERY ));
            xWindow->addWindowListener(
                css::uno::Reference< css::awt::XWindowListener >(
                    xThis, css::uno::UNO_QUERY ));
            xDockWindow->enableDocking( true );
        }
        catch ( const css::uno::Exception& )
        {
        }
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
