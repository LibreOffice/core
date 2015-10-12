/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CANVAS_SOURCE_VCL_WINDOWOUTDEVHOLDER_HXX
#define INCLUDED_CANVAS_SOURCE_VCL_WINDOWOUTDEVHOLDER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/window.hxx>

#include "outdevprovider.hxx"

#include <boost/noncopyable.hpp>

namespace vclcanvas
{
    class WindowOutDevHolder : public OutDevProvider,
                               private ::boost::noncopyable
    {
    public:
        explicit WindowOutDevHolder( const css::uno::Reference< css::awt::XWindow>& xWin );

    private:
        virtual OutputDevice&       getOutDev() override { return mrOutputWindow; }
        virtual const OutputDevice& getOutDev() const override { return mrOutputWindow; }

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
        vcl::Window& mrOutputWindow;
    };
}

#endif // INCLUDED_CANVAS_SOURCE_VCL_WINDOWOUTDEVHOLDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
