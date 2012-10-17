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

#ifndef __FRAMEWORK_LAYOUTMANAGER_HELPERS_HXX_
#define __FRAMEWORK_LAYOUTMANAGER_HELPERS_HXX_

#include <macros/generic.hxx>
#include <stdtypes.h>
#include <properties.h>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XToolkit2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/Point.hpp>

#include <vcl/window.hxx>
#include <vcl/toolbox.hxx>

#define UIRESOURCE_URL                  RTL_CONSTASCII_USTRINGPARAM( "private:resource" )
#define UIRESOURCETYPE_TOOLBAR          "toolbar"
#define UIRESOURCETYPE_STATUSBAR        "statusbar"
#define UIRESOURCETYPE_MENUBAR          "menubar"

namespace framework
{

bool hasEmptySize( const ::com::sun::star::awt::Size& rSize );
bool hasDefaultPosValue( const ::com::sun::star::awt::Point& rPos );
bool isDefaultPos( const ::com::sun::star::awt::Point& rPos );
bool isToolboxHorizontalAligned( ToolBox* pToolBox );
bool isReverseOrderDockingArea( const sal_Int32 nDockArea );
bool isHorizontalDockingArea( const sal_Int32 nDockArea );
bool isHorizontalDockingArea( const ::com::sun::star::ui::DockingArea& nDockArea );
::rtl::OUString retrieveToolbarNameFromHelpURL( Window* pWindow );
ToolBox* getToolboxPtr( Window* pWindow );
Window* getWindowFromXUIElement( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& xUIElement );
SystemWindow* getTopSystemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& xWindow );
bool equalRectangles( const css::awt::Rectangle& rRect1, const css::awt::Rectangle& rRect2 );
void setZeroRectangle( ::Rectangle& rRect );
bool lcl_checkUIElement(const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& xUIElement,::com::sun::star::awt::Rectangle& _rPosSize, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xWindow);
::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > createToolkitWindow( const css::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext, const css::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParent, const char* pService );
WindowAlign ImplConvertAlignment( sal_Int16 aAlignment );
::rtl::OUString getElementTypeFromResourceURL( const ::rtl::OUString& aResourceURL );
void parseResourceURL( const rtl::OUString& aResourceURL, rtl::OUString& aElementType, rtl::OUString& aElementName );
::Rectangle putAWTToRectangle( const ::com::sun::star::awt::Rectangle& rRect );
::com::sun::star::awt::Rectangle putRectangleValueToAWT( const ::Rectangle& rRect );
::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > impl_getModelFromFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
sal_Bool implts_isPreviewModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
sal_Bool implts_isFrameOrWindowTop( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame );
void impl_setDockingWindowVisibility( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& rxContext, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& rDockingWindowName, bool bVisible );
void impl_addWindowListeners( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xThis, const ::com::sun::star::uno::Reference< css::ui::XUIElement >& xUIElement );
::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > implts_createToolkitWindow( const css::uno::Reference< ::com::sun::star::awt::XToolkit2 >& rToolkit, const css::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParent );

}

#endif // __FRAMEWORK_LAYOUTMANAGER_HELPERS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
