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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
