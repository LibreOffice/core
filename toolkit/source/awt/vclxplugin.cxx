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

#include "vclxplugin.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/property.hxx>
#include <vcl/ctrl.hxx>

#include "forward.hxx"

namespace layoutimpl
{

using namespace ::com::sun::star;

VCLXPlugin::VCLXPlugin( Window *p, WinBits b )
    : VCLXWindow()
    , mpWindow( p )
    , mpPlugin( 0 )
    , mStyle( b )
{
}

VCLXPlugin::~VCLXPlugin()
{
}

void SAL_CALL VCLXPlugin::dispose() throw(uno::RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        lang::EventObject aDisposeEvent;
        aDisposeEvent.Source = W3K_EXPLICIT_CAST (*this);
    }

    VCLXWindow::dispose();
}

void VCLXPlugin::SetPlugin( ::Control *p )
{
    mpPlugin = p;
}

awt::Size SAL_CALL VCLXPlugin::getMinimumSize()
    throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OClearableGuard aGuard( GetMutex() );
    if ( mpPlugin )
        return AWTSize( mpPlugin->GetSizePixel() );
    return awt::Size();
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
