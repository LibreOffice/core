/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include "cairo_xlib_cairo.hxx"

#include <utility>
#include <vcl/sysdata.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/virdev.hxx>
#include <sal/log.hxx>

#include <cairo-xlib.h>
#include <cairo-xlib-xrender.h>

namespace cairo
{

    X11SysData::X11SysData() :
        pDisplay(nullptr),
        hDrawable(0),
        pVisual(nullptr),
        nScreen(0)
    {}

    X11SysData::X11SysData( const SystemGraphicsData& pSysDat ) :
        pDisplay(static_cast<_XDisplay*>(pSysDat.pDisplay)),
        hDrawable(pSysDat.hDrawable),
        pVisual(static_cast<Visual*>(pSysDat.pVisual)),
        nScreen(pSysDat.nScreen)
    {}

    X11SysData::X11SysData( const SystemEnvData& pSysDat, const SalFrame* pReference ) :
        pDisplay(static_cast<_XDisplay*>(pSysDat.pDisplay)),
        hDrawable(pSysDat.GetWindowHandle(pReference)),
        pVisual(static_cast<Visual*>(pSysDat.pVisual)),
        nScreen(pSysDat.nScreen)
    {}

    X11Pixmap::~X11Pixmap()
    {
        if( mpDisplay && mhDrawable )
            XFreePixmap( mpDisplay, mhDrawable );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
