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

#include "precompiled_vcl.hxx"

#include "vcl/window.hxx"
#include "vcl/window.h"
#include "vcl/arrange.hxx"

namespace vcl
{
    struct ExtWindowImpl
    {
        ExtWindowImpl()
        {}
        ~ExtWindowImpl()
        {}

        boost::shared_ptr< WindowArranger > mxLayout;
    };
}


void Window::ImplFreeExtWindowImpl()
{
    if( mpWindowImpl )
    {
        delete mpWindowImpl->mpExtImpl;
        mpWindowImpl->mpExtImpl = NULL;
    }
}

vcl::ExtWindowImpl* Window::ImplGetExtWindowImpl() const
{
    vcl::ExtWindowImpl* pImpl = NULL;
    if( mpWindowImpl )
    {
        if( ! mpWindowImpl->mpExtImpl )
            mpWindowImpl->mpExtImpl = new vcl::ExtWindowImpl();
        pImpl = mpWindowImpl->mpExtImpl;
    }
    return pImpl;
}

void Window::ImplExtResize()
{
    if( mpWindowImpl && mpWindowImpl->mpExtImpl )
    {
        if( mpWindowImpl->mpExtImpl->mxLayout.get() )
            mpWindowImpl->mpExtImpl->mxLayout->setManagedArea( Rectangle( Point( 0, 0 ), GetSizePixel() ) );
    }
}

boost::shared_ptr< vcl::WindowArranger > Window::getLayout()
{
    boost::shared_ptr< vcl::WindowArranger > xRet;
    vcl::ExtWindowImpl* pImpl = ImplGetExtWindowImpl();
    if( pImpl )
    {
        if( ! pImpl->mxLayout.get() )
        {
            pImpl->mxLayout.reset( new vcl::LabelColumn() );
            pImpl->mxLayout->setParentWindow( this );
            pImpl->mxLayout->setOuterBorder( -1 );
        }
        xRet = pImpl->mxLayout;
    }

    return xRet;
}
