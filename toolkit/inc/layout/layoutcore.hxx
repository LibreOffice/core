/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layoutcore.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef _LAYOUT_CORE_HXX_
#define _LAYOUT_CORE_HXX_

#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/XLayoutContainer.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>

#include <layout/layout.hxx>

namespace layoutimpl
{

namespace css = layout::css;

css::uno::Reference< css::awt::XLayoutContainer >
createContainer( const rtl::OUString &rName );

css::uno::Reference< css::awt::XLayoutConstrains >
createWidget( css::uno::Reference< css::awt::XToolkit > xToolkit,
              css::uno::Reference< css::uno::XInterface > xParent,
              const rtl::OUString &rName, long nProps );

css::uno::Reference< css::awt::XLayoutConstrains >
createInternalWidget( css::uno::Reference< css::awt::XToolkit > xToolkit,
                      css::uno::Reference< css::uno::XInterface > xParent,
                      const rtl::OUString &rName, long nProps );

css::uno::Reference< css::graphic::XGraphic > loadGraphic( const char *pName );

// Helpers - unfortunately VCLXWindows don't implement XPropertySet
// but containers do - these helpers help us to hide this
namespace prophlp
{
// can we set properties on this handle ?
bool TOOLKIT_DLLPUBLIC canHandleProps( const css::uno::Reference< css::uno::XInterface > &xRef );
// if so which properties ?
css::uno::Reference< css::beans::XPropertySetInfo > TOOLKIT_DLLPUBLIC queryPropertyInfo(
    const css::uno::Reference< css::uno::XInterface > &xRef );
// set / get ...
void TOOLKIT_DLLPUBLIC setProperty( const css::uno::Reference< css::uno::XInterface > &xRef,
                  const rtl::OUString &rName,
                  css::uno::Any aValue );
css::uno::Any TOOLKIT_DLLPUBLIC getProperty( const css::uno::Reference< css::uno::XInterface > &xRef,
                           const rtl::OUString &rName );
} // namespace prophlp
} // namespace layoutimpl

#endif // _LAYOUT_CORE_HXX_
