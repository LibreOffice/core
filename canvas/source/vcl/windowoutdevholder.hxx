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

#ifndef _VCLCANVAS_WINDOWOUTDEVHOLDER_HXX
#define _VCLCANVAS_WINDOWOUTDEVHOLDER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/window.hxx>

#include "outdevprovider.hxx"

#include <boost/utility.hpp>

namespace vclcanvas
{
    class WindowOutDevHolder : public OutDevProvider,
                               private ::boost::noncopyable
    {
    public:
        explicit WindowOutDevHolder( const ::com::sun::star::uno::Reference<
                                           ::com::sun::star::awt::XWindow>& xWin );

    private:
        virtual OutputDevice&       getOutDev() { return mrOutputWindow; }
        virtual const OutputDevice& getOutDev() const { return mrOutputWindow; }

        // TODO(Q2): Lifetime issue. Though WindowGraphicDeviceBase
        // now listenes to the window component, I still consider
        // holding a naked reference unsafe here (especially as we
        // pass it around via getOutDev). This _only_ works reliably,
        // if disposing the SpriteCanvas correctly disposes all
        // entities which hold this pointer.
        // So: as soon as the protocol inside
        // vcl/source/window/window.cxx is broken, that disposes the
        // canvas during window deletion, we're riding a dead horse
        // here
        Window& mrOutputWindow;
    };
}

#endif /* _VCLCANVAS_WINDOWOUTDEVHOLDER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
