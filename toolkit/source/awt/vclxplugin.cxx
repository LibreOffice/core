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
