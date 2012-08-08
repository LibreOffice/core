/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#include "helpers.hxx"
#include <threadhelp/resetableguard.hxx>
#include <services.h>

#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/awt/XDockableWindowListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/ui/XUIElement.hpp>

#include <comphelper/mediadescriptor.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/unohlp.hxx>

// namespace
using namespace com::sun::star;

namespace framework
{

bool hasEmptySize( const ::com::sun::star::awt::Size& rSize )
{
    return ( rSize.Width == 0 ) && ( rSize.Height == 0 );
}

bool hasDefaultPosValue( const ::com::sun::star::awt::Point& rPos )
{
    return (( rPos.X == SAL_MAX_INT32 ) || ( rPos.Y == SAL_MAX_INT32 ));
}

bool isDefaultPos( const ::com::sun::star::awt::Point& rPos )
{
    return (( rPos.X == SAL_MAX_INT32 ) && ( rPos.Y == SAL_MAX_INT32 ));
}

bool isReverseOrderDockingArea( const sal_Int32 nDockArea )
{
    ui::DockingArea eDockArea = static_cast< ui::DockingArea >( nDockArea );
    return (( eDockArea == ui::DockingArea_DOCKINGAREA_BOTTOM ) ||
            ( eDockArea == ui::DockingArea_DOCKINGAREA_RIGHT ));
}

bool isToolboxHorizontalAligned( ToolBox* pToolBox )
{
    if ( pToolBox )
        return (( pToolBox->GetAlign() == WINDOWALIGN_TOP ) || ( pToolBox->GetAlign() == WINDOWALIGN_BOTTOM ));
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

::rtl::OUString retrieveToolbarNameFromHelpURL( Window* pWindow )
{
    ::rtl::OUString aToolbarName;

    if ( pWindow->GetType() == WINDOW_TOOLBOX )
    {
        ToolBox* pToolBox = dynamic_cast<ToolBox *>( pWindow );
        if ( pToolBox )
        {
            aToolbarName = rtl::OStringToOUString( pToolBox->GetHelpId(), RTL_TEXTENCODING_UTF8 );
            sal_Int32 i = aToolbarName.lastIndexOf( ':' );
            if ( !aToolbarName.isEmpty() && ( i > 0 ) && (( i + 1 ) < aToolbarName.getLength() ))
                aToolbarName = aToolbarName.copy( i+1 ); // Remove ".HelpId:" protocol from toolbar name
            else
              aToolbarName = ::rtl::OUString();
        }
    }
    return aToolbarName;
}

ToolBox* getToolboxPtr( Window* pWindow )
{
    ToolBox* pToolbox(NULL);
    if ( pWindow->GetType() == WINDOW_TOOLBOX )
        pToolbox = dynamic_cast<ToolBox*>( pWindow );
    return pToolbox;
}

Window* getWindowFromXUIElement( const uno::Reference< ui::XUIElement >& xUIElement )
{
    SolarMutexGuard aGuard;
    uno::Reference< awt::XWindow > xWindow;
    if ( xUIElement.is() )
        xWindow = uno::Reference< awt::XWindow >( xUIElement->getRealInterface(), uno::UNO_QUERY );
    return VCLUnoHelper::GetWindow( xWindow );
}

SystemWindow* getTopSystemWindow( const uno::Reference< awt::XWindow >& xWindow )
{
    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    while ( pWindow && !pWindow->IsSystemWindow() )
        pWindow = pWindow->GetParent();

    if ( pWindow )
        return (SystemWindow *)pWindow;
    else
        return 0;
}

void setZeroRectangle( ::Rectangle& rRect )
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

        Window* pWindow = VCLUnoHelper::GetWindow( _xWindow );
        if ( pWindow->GetType() == WINDOW_TOOLBOX )
        {
            ::Size aSize = ((ToolBox*)pWindow)->CalcWindowSizePixel( 1 );
            _rPosSize.Width = aSize.Width();
            _rPosSize.Height = aSize.Height();
        }
    } // if ( xUIElement.is() )
    return bRet;
}

uno::Reference< awt::XWindowPeer > createToolkitWindow( const uno::Reference< lang::XMultiServiceFactory >& rFactory, const uno::Reference< awt::XWindowPeer >& rParent, const char* pService )
{
    const rtl::OUString aAWTToolkit( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.Toolkit" ));

    uno::Reference< awt::XWindowPeer > xPeer;
    if ( rFactory.is() )
    {
        uno::Reference< awt::XToolkit > xToolkit( rFactory->createInstance( aAWTToolkit ), uno::UNO_QUERY_THROW );
        if ( xToolkit.is() )
        {
            // describe window properties.
            css::awt::WindowDescriptor aDescriptor;
            aDescriptor.Type                =   awt::WindowClass_SIMPLE;
            aDescriptor.WindowServiceName   =   ::rtl::OUString::createFromAscii( pService );
            aDescriptor.ParentIndex         =   -1;
            aDescriptor.Parent              =   uno::Reference< awt::XWindowPeer >( rParent, uno::UNO_QUERY );
            aDescriptor.Bounds              =   awt::Rectangle(0,0,0,0);
            aDescriptor.WindowAttributes    =   0;

            // create a awt window
            xPeer = xToolkit->createWindow( aDescriptor );
        }
    }

    return xPeer;
}

// convert alignment constant to vcl's WindowAlign type
WindowAlign ImplConvertAlignment( sal_Int16 aAlignment )
{
    if ( aAlignment == ui::DockingArea_DOCKINGAREA_LEFT )
        return WINDOWALIGN_LEFT;
    else if ( aAlignment == ui::DockingArea_DOCKINGAREA_RIGHT )
        return WINDOWALIGN_RIGHT;
    else if ( aAlignment == ui::DockingArea_DOCKINGAREA_TOP )
        return WINDOWALIGN_TOP;
    else
        return WINDOWALIGN_BOTTOM;
}

::rtl::OUString getElementTypeFromResourceURL( const ::rtl::OUString& aResourceURL )
{
    ::rtl::OUString aType;

    ::rtl::OUString aUIResourceURL( UIRESOURCE_URL );
    if ( aResourceURL.indexOf( aUIResourceURL ) == 0 )
    {
        sal_Int32       nIndex = 0;
        ::rtl::OUString aPathPart   = aResourceURL.copy( aUIResourceURL.getLength() );
        ::rtl::OUString aUIResource = aPathPart.getToken( 0, (sal_Unicode)'/', nIndex );

        return aPathPart.getToken( 0, (sal_Unicode)'/', nIndex );
    }

    return aType;
}

void parseResourceURL( const rtl::OUString& aResourceURL, rtl::OUString& aElementType, rtl::OUString& aElementName )
{
    ::rtl::OUString aUIResourceURL( UIRESOURCE_URL );
    if ( aResourceURL.indexOf( aUIResourceURL ) == 0 )
    {
        sal_Int32       nIndex = 0;
        ::rtl::OUString aPathPart   = aResourceURL.copy( aUIResourceURL.getLength() );
        ::rtl::OUString aUIResource = aPathPart.getToken( 0, (sal_Unicode)'/', nIndex );

        aElementType = aPathPart.getToken( 0, (sal_Unicode)'/', nIndex );
        aElementName = aPathPart.getToken( 0, (sal_Unicode)'/', nIndex );
    }
}

::com::sun::star::awt::Rectangle putRectangleValueToAWT( const ::Rectangle& rRect )
{
    css::awt::Rectangle aRect;
    aRect.X = rRect.Left();
    aRect.Y = rRect.Top();
    aRect.Width = rRect.Right();
    aRect.Height = rRect.Bottom();

    return aRect;
}

::Rectangle putAWTToRectangle( const ::com::sun::star::awt::Rectangle& rRect )
{
    ::Rectangle aRect;
    aRect.Left() = rRect.X;
    aRect.Top() = rRect.Y;
    aRect.Right() = rRect.Width;
    aRect.Bottom() = rRect.Height;

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

sal_Bool implts_isPreviewModel( const uno::Reference< frame::XModel >& xModel )
{
    if ( xModel.is() )
    {
        ::comphelper::MediaDescriptor aDesc( xModel->getArgs() );
        return aDesc.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_PREVIEW(), (sal_Bool)sal_False);
    }
    else
        return sal_False;
}

sal_Bool implts_isFrameOrWindowTop( const uno::Reference< frame::XFrame >& xFrame )
{
    if (xFrame->isTop())
        return sal_True;

    uno::Reference< awt::XTopWindow > xWindowCheck(xFrame->getContainerWindow(), uno::UNO_QUERY); // dont use _THROW here ... its a check only
    if (xWindowCheck.is())
    {
        // #i76867# top and system window is required.
        SolarMutexGuard aGuard;
        uno::Reference< awt::XWindow > xWindow( xWindowCheck, uno::UNO_QUERY );
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        return ( pWindow && pWindow->IsSystemWindow() );
    }

    return sal_False;
}

void impl_setDockingWindowVisibility( const css::uno::Reference< css::lang::XMultiServiceFactory>& rSMGR, const css::uno::Reference< css::frame::XFrame >& rFrame, const ::rtl::OUString& rDockingWindowName, bool bVisible )
{
    const ::rtl::OUString aDockWinPrefixCommand( RTL_CONSTASCII_USTRINGPARAM( "DockingWindow" ));
    css::uno::WeakReference< css::frame::XDispatchHelper > xDispatchHelper;

    sal_Int32 nID    = rDockingWindowName.toInt32();
    sal_Int32 nIndex = nID - DOCKWIN_ID_BASE;

    css::uno::Reference< css::frame::XDispatchProvider > xProvider(rFrame, css::uno::UNO_QUERY);
    if ( nIndex >= 0 && xProvider.is() )
    {
        ::rtl::OUString aDockWinCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));
        ::rtl::OUString aDockWinArgName( aDockWinPrefixCommand );

        aDockWinArgName += ::rtl::OUString::valueOf( nIndex );

        css::uno::Sequence< css::beans::PropertyValue > aArgs(1);
        aArgs[0].Name  = aDockWinArgName;
        aArgs[0].Value = css::uno::makeAny( bVisible );

        css::uno::Reference< css::frame::XDispatchHelper > xDispatcher( xDispatchHelper );
        if ( !xDispatcher.is())
        {
            xDispatcher = css::uno::Reference< css::frame::XDispatchHelper >(
                rSMGR->createInstance(SERVICENAME_DISPATCHHELPER), css::uno::UNO_QUERY_THROW);
        }

        aDockWinCommand = aDockWinCommand + aDockWinArgName;
        xDispatcher->executeDispatch(
            xProvider,
            aDockWinCommand,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")),
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
            xDockWindow->enableDocking( sal_True );
        }
        catch ( const css::uno::Exception& )
        {
        }
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
