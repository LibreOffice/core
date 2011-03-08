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

#ifndef LAYOUT_CORE_PROPLIST_HXX
#define LAYOUT_CORE_PROPLIST_HXX

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/xml/input/XAttributes.hpp>
#include <list>
#include <rtl/ustring.hxx>
#include <toolkit/dllapi.h>
\
namespace layoutimpl
{

namespace css = ::com::sun::star;

typedef std::list< std::pair< rtl::OUString, rtl::OUString > > PropList;

void propsFromAttributes( const css::uno::Reference<css::xml::input::XAttributes> & xAttributes,
                          PropList &rProps, sal_Int32 nNamespace );

void setProperties( css::uno::Reference< css::uno::XInterface > const& xPeer,
                    PropList const& rProps);

void setProperty( css::uno::Reference< css::uno::XInterface > const& xPeer,
                  rtl::OUString const& attr, rtl::OUString const& value );

long getAttributeProps( PropList &rProps );
bool findAndRemove( const char *pAttr, PropList &rProps, rtl::OUString &rValue);

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


#if !OUSTRING_CSTR_PARANOIA
#define OUSTRING_CSTR( str ) \
    rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US ).getStr()
#else

inline char const* OUSTRING_CSTR( rtl::OUString const& str )
{
    rtl::OString *leak
        = new rtl::OString (rtl::OUStringToOString (str, RTL_TEXTENCODING_ASCII_US));
    return leak->getStr();
}

#endif

#endif /* LAYOUT_CORE_PROPLIST_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
