/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_LAYOUTMANAGER_HELPERS_HXX_
#define __FRAMEWORK_LAYOUTMANAGER_HELPERS_HXX_

// my own includes
#include <macros/generic.hxx>
#include <stdtypes.h>
#include <properties.h>

// interface includes
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/Point.hpp>

// other includes
#include <vcl/window.hxx>
#include <vcl/toolbox.hxx>

#define UIRESOURCE_PROTOCO_ASCII        "private:"
#define UIRESOURCE_RESOURCE_ASCII       "resource"
#define UIRESOURCE_URL_ASCII            "private:resource"
#define UIRESOURCE_URL                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UIRESOURCE_URL_ASCII ))
#define UIRESOURCETYPE_TOOLBAR          "toolbar"
#define UIRESOURCETYPE_STATUSBAR        "statusbar"
#define UIRESOURCETYPE_MENUBAR          "menubar"

namespace framework
{

bool hasEmptySize( const:: Size& aSize );
bool hasDefaultPosValue( const ::Point& aPos );
bool isDefaultPos( const ::com::sun::star::awt::Point& aPos );
bool isDefaultPos( const ::Point& aPos );
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
::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > createToolkitWindow( const css::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rFactory, const css::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParent, const char* pService );
WindowAlign ImplConvertAlignment( sal_Int16 aAlignment );
::rtl::OUString getElementTypeFromResourceURL( const ::rtl::OUString& aResourceURL );
void parseResourceURL( const rtl::OUString& aResourceURL, rtl::OUString& aElementType, rtl::OUString& aElementName );
::Rectangle putAWTToRectangle( const ::com::sun::star::awt::Rectangle& rRect );
::com::sun::star::awt::Rectangle putRectangleValueToAWT( const ::Rectangle& rRect );
::com::sun::star::awt::Rectangle convertRectangleToAWT( const ::Rectangle& rRect );
::Rectangle convertAWTToRectangle( const ::com::sun::star::awt::Rectangle& rRect );
::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > impl_getModelFromFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
sal_Bool implts_isPreviewModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
sal_Bool implts_isFrameOrWindowTop( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame );
void impl_setDockingWindowVisibility( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& rSMGR, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& rDockingWindowName, bool bVisible );
void impl_addWindowListeners( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xThis, const ::com::sun::star::uno::Reference< css::ui::XUIElement >& xUIElement );
::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > implts_createToolkitWindow( const css::uno::Reference< ::com::sun::star::awt::XToolkit >& rToolkit, const css::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParent );

}

#endif // __FRAMEWORK_LAYOUTMANAGER_HELPERS_HXX_
