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

#include "vclxtabpage.hxx"
#include "forward.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <toolkit/helper/convert.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>

#if !defined (__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif /* !__GNUC__ */

namespace layoutimpl
{

using namespace ::com::sun::star;

// XInterface
IMPLEMENT_FORWARD_XINTERFACE2( VCLXTabPage, VCLXWindow, Bin );

// XTypeProvider
IMPLEMENT_FORWARD_XTYPEPROVIDER1( VCLXTabPage, VCLXWindow );

VCLXTabPage::VCLXTabPage( Window *p )
    : VCLXWindow()
    , Bin()
    , bRealized( false )
{
    /* FIXME: before Window is set, setLabel, setProperty->setImage
     * are silent no-ops.  */
    p->SetComponentInterface( this );
}

VCLXTabPage::~VCLXTabPage()
{
}

void SAL_CALL VCLXTabPage::dispose() throw(uno::RuntimeException)
{
    {
        ::osl::SolarMutexGuard aGuard( GetMutex() );

        lang::EventObject aDisposeEvent;
        aDisposeEvent.Source = W3K_EXPLICIT_CAST (*this);
    }

    VCLXWindow::dispose();
}

void SAL_CALL VCLXTabPage::allocateArea( awt::Rectangle const& area )
    throw (uno::RuntimeException)
{
    awt::Size currentSize = getSize();
    awt::Size requestedSize = getMinimumSize();
    requestedSize.Height = getHeightForWidth( area.Width );

    if ( currentSize.Width > 0 && currentSize.Height > 0
         && requestedSize.Width > currentSize.Width )
        requestedSize.Width = currentSize.Width;
    if ( currentSize.Width > 0 && currentSize.Height > 0
         && requestedSize.Height > currentSize.Height )
        requestedSize.Height = currentSize.Height;

    // FIXME: missing destructor?
    if ( !GetWindow() )
        return;

    Size windowSize = GetWindow()->GetSizePixel();
    Window *parent = GetWindow()->GetParent();
    Size parentSize = parent->GetSizePixel();

    Point pos = GetWindow()->GetPosPixel();
#ifndef __SUNPRO_CC
    OSL_TRACE ("\n%s", __PRETTY_FUNCTION__);
    OSL_TRACE ("%s: curpos: %d ,%d", __FUNCTION__, pos.X(), pos.Y() );

    OSL_TRACE ("%s: cursize: %d ,%d", __FUNCTION__, currentSize.Width, currentSize.Height );
    OSL_TRACE ("%s: area: %d, %d", __FUNCTION__, area.Width, area.Height );
    OSL_TRACE ("%s: requestedSize: %d, %d", __FUNCTION__, requestedSize.Width, requestedSize.Height );
    OSL_TRACE ("%s: parent: %d, %d", __FUNCTION__, parentSize.Width(), parentSize.Height() );
    OSL_TRACE ("%s: window: %d, %d", __FUNCTION__, windowSize.Width(), windowSize.Height() );
#endif

    if ( !bRealized )
    {
        setPosSize( area.X, area.Y, requestedSize.Width, requestedSize.Height, awt::PosSize::SIZE );
        bRealized = true;
    }
    else
    {
        if ( requestedSize.Width > currentSize.Width + 10)
            setPosSize( 0, 0, requestedSize.Width, 0, awt::PosSize::WIDTH );
        if ( requestedSize.Height > currentSize.Height + 10)
            setPosSize( 0, 0, 0, requestedSize.Height, awt::PosSize::HEIGHT );
    }

    awt::Size newSize = getSize();
#ifndef __SUNPRO_CC
    OSL_TRACE ("%s: newSize: %d, %d", __FUNCTION__, newSize.Width, newSize.Height );
#endif
    maAllocation.Width = newSize.Width;
    maAllocation.Height = newSize.Height;

    Bin::allocateArea( maAllocation );
}

awt::Size SAL_CALL VCLXTabPage::getMinimumSize()
    throw(uno::RuntimeException)
{
    ::osl::SolarMutexGuard aGuard( GetMutex() );

    return Bin::getMinimumSize();
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
