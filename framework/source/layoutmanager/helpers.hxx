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

#ifndef INCLUDED_FRAMEWORK_SOURCE_LAYOUTMANAGER_HELPERS_HXX
#define INCLUDED_FRAMEWORK_SOURCE_LAYOUTMANAGER_HELPERS_HXX

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

#define UIRESOURCE_URL                  "private:resource"
#define UIRESOURCETYPE_TOOLBAR          "toolbar"
#define UIRESOURCETYPE_MENUBAR          "menubar"

namespace framework
{

bool hasEmptySize( const css::awt::Size& rSize );
bool hasDefaultPosValue( const css::awt::Point& rPos );
bool isDefaultPos( const css::awt::Point& rPos );
bool isToolboxHorizontalAligned( ToolBox* pToolBox );
bool isReverseOrderDockingArea( const sal_Int32 nDockArea );
bool isHorizontalDockingArea( const sal_Int32 nDockArea );
bool isHorizontalDockingArea( const css::ui::DockingArea& nDockArea );
OUString retrieveToolbarNameFromHelpURL( vcl::Window* pWindow );
ToolBox* getToolboxPtr( vcl::Window* pWindow );
vcl::Window* getWindowFromXUIElement( const css::uno::Reference< css::ui::XUIElement >& xUIElement );
SystemWindow* getTopSystemWindow( const css::uno::Reference< css::awt::XWindow >& xWindow );
bool equalRectangles( const css::awt::Rectangle& rRect1, const css::awt::Rectangle& rRect2 );
void setZeroRectangle( ::Rectangle& rRect );
bool lcl_checkUIElement(const css::uno::Reference< css::ui::XUIElement >& xUIElement,css::awt::Rectangle& _rPosSize, css::uno::Reference< css::awt::XWindow >& _xWindow);
css::uno::Reference< css::awt::XWindowPeer > createToolkitWindow( const css::uno::Reference< css::uno::XComponentContext >& rxContext, const css::uno::Reference< css::awt::XWindowPeer >& rParent, const char* pService );
WindowAlign ImplConvertAlignment( sal_Int16 aAlignment );
OUString getElementTypeFromResourceURL( const OUString& aResourceURL );
void parseResourceURL( const OUString& aResourceURL, OUString& aElementType, OUString& aElementName );
::Rectangle putAWTToRectangle( const css::awt::Rectangle& rRect );
css::awt::Rectangle putRectangleValueToAWT( const ::Rectangle& rRect );
css::uno::Reference< css::frame::XModel > impl_getModelFromFrame( const css::uno::Reference< css::frame::XFrame >& rFrame );
bool implts_isPreviewModel( const css::uno::Reference< css::frame::XModel >& xModel );
bool implts_isFrameOrWindowTop( const css::uno::Reference< css::frame::XFrame >& xFrame );
void impl_setDockingWindowVisibility( const css::uno::Reference< css::uno::XComponentContext>& rxContext, const css::uno::Reference< css::frame::XFrame >& rFrame, const OUString& rDockingWindowName, bool bVisible );
void impl_addWindowListeners( const css::uno::Reference< css::uno::XInterface >& xThis, const css::uno::Reference< css::ui::XUIElement >& xUIElement );

}

#endif // INCLUDED_FRAMEWORK_SOURCE_LAYOUTMANAGER_HELPERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
