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

#include "localized-string.hxx"

#include <toolkit/helper/property.hxx>
#include <vcl/window.hxx>

namespace layoutimpl
{

namespace css = ::com::sun::star;
using namespace css;
using rtl::OUString;

LocalizedString::LocalizedString()
    : VCLXWindow()
{
}

void LocalizedString::ImplGetPropertyIds( std::list< sal_uInt16 > &ids )
{
    PushPropertyIds( ids, BASEPROPERTY_TEXT, 0);
    VCLXWindow::ImplGetPropertyIds( ids );
}

// XInterface
uno::Any LocalizedString::queryInterface( uno::Type const& rType )
    throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                            SAL_STATIC_CAST( awt::XFixedText*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

void LocalizedString::setText( OUString const& s )
    throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( Window *w = GetWindow() )
        return w->SetText( s );
}

OUString LocalizedString::getText()
    throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( Window *w = GetWindow() )
        return w->GetText();
    return OUString();
}

} // namespace layoutimpl
