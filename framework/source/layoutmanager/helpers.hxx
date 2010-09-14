/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layoutmanager.hxx,v $
 * $Revision: 1.34 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>
#include <stdtypes.h>
#include <properties.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/window.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

#define UIRESOURCE_PROTOCO_ASCII        "private:"
#define UIRESOURCE_RESOURCE_ASCII       "resource"
#define UIRESOURCE_URL_ASCII            "private:resource"
#define UIRESOURCE_URL                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UIRESOURCE_URL_ASCII ))

namespace framework
{

bool lcl_checkUIElement(const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& xUIElement,::com::sun::star::awt::Rectangle& _rPosSize, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xWindow);
::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > createToolkitWindow( const css::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rFactory, const css::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParent, const char* pService );
WindowAlign ImplConvertAlignment( sal_Int16 aAlignment );
bool impl_parseResourceURL( const rtl::OUString aResourceURL, rtl::OUString& aElementType, rtl::OUString& aElementName );
::com::sun::star::awt::Rectangle impl_convertRectangleToAWT( const ::Rectangle& rRect );
::Rectangle impl_convertAWTToRectangle( const ::com::sun::star::awt::Rectangle& rRect );
::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > impl_getModelFromFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
sal_Bool implts_isPreviewModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
sal_Bool implts_isFrameOrWindowTop( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame );
void impl_setDockingWindowVisibility( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& rSMGR, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& rDockingWindowName, bool bVisible );
void impl_addWindowListeners( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xThis, const ::com::sun::star::uno::Reference< css::ui::XUIElement >& xUIElement );
::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > implts_createToolkitWindow( const css::uno::Reference< ::com::sun::star::awt::XToolkit >& rToolkit, const css::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParent );

}

#endif // __FRAMEWORK_LAYOUTMANAGER_HELPERS_HXX_
