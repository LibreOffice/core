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

#define UIRESOURCE_URL                  "private:resource"
#define UIRESOURCETYPE_TOOLBAR          "toolbar"
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
OUString retrieveToolbarNameFromHelpURL( vcl::Window* pWindow );
ToolBox* getToolboxPtr( vcl::Window* pWindow );
vcl::Window* getWindowFromXUIElement( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& xUIElement );
SystemWindow* getTopSystemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& xWindow );
bool equalRectangles( const css::awt::Rectangle& rRect1, const css::awt::Rectangle& rRect2 );
void setZeroRectangle( ::Rectangle& rRect );
bool lcl_checkUIElement(const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& xUIElement,::com::sun::star::awt::Rectangle& _rPosSize, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xWindow);
::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > createToolkitWindow( const css::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext, const css::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParent, const char* pService );
WindowAlign ImplConvertAlignment( sal_Int16 aAlignment );
OUString getElementTypeFromResourceURL( const OUString& aResourceURL );
void parseResourceURL( const OUString& aResourceURL, OUString& aElementType, OUString& aElementName );
::Rectangle putAWTToRectangle( const ::com::sun::star::awt::Rectangle& rRect );
::com::sun::star::awt::Rectangle putRectangleValueToAWT( const ::Rectangle& rRect );
::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > impl_getModelFromFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
bool implts_isPreviewModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
bool implts_isFrameOrWindowTop( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame );
void impl_setDockingWindowVisibility( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& rxContext, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const OUString& rDockingWindowName, bool bVisible );
void impl_addWindowListeners( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xThis, const ::com::sun::star::uno::Reference< css::ui::XUIElement >& xUIElement );

}

#endif // INCLUDED_FRAMEWORK_SOURCE_LAYOUTMANAGER_HELPERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
