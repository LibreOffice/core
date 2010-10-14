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

#ifndef LAYOUT_CORE_LOCALIZED_STRING_HXX
#define LAYOUT_CORE_LOCALIZED_STRING_HXX

#include <com/sun/star/awt/XFixedText.hpp>
#include <toolkit/awt/vclxwindow.hxx>

namespace layoutimpl
{
namespace css = ::com::sun::star;

// FIXME: misuse XFixedText interface for simple string
class LocalizedString : public css::awt::XFixedText
                      , public VCLXWindow
{
public:
    LocalizedString();

    // css::uno::XInterface
    css::uno::Any SAL_CALL queryInterface( css::uno::Type const& rType )
        throw(css::uno::RuntimeException);
    void SAL_CALL acquire() throw() { OWeakObject::acquire(); }
    void SAL_CALL release() throw() { OWeakObject::release(); }

    // css::awt::XFixedText
    void SAL_CALL setText( ::rtl::OUString const& s )
        throw(css::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getText()
        throw(css::uno::RuntimeException);
    void SAL_CALL setAlignment( sal_Int16 )
        throw(css::uno::RuntimeException) { }
    sal_Int16 SAL_CALL getAlignment()
        throw(css::uno::RuntimeException) { return 0; }

    // css::awt::XLayoutConstrains
    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException) { return css::awt::Size( 0, 0 ); }
    css::awt::Size SAL_CALL getPreferredSize()
        throw(css::uno::RuntimeException) { return getMinimumSize(); }
    css::awt::Size SAL_CALL calcAdjustedSize( css::awt::Size const& size )
        throw(css::uno::RuntimeException) { return size; }

    static void ImplGetPropertyIds( std::list< sal_uInt16 > &ids );
    virtual void GetPropertyIds( std::list< sal_uInt16 > &ids )
    { return ImplGetPropertyIds( ids ); }
};

} // namespace layoutimpl

#endif /* LAYOUT_CORE_LOCALIZED_STRING_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
