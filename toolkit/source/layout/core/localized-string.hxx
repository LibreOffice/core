/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, 5th Floor, Boston,
 *    MA  02110-1301  USA
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
