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

#pragma once

#include <sal/config.h>
#include <vcl/cairo.hxx>
#include <vcl/salgtype.hxx>

struct BitmapSystemData;
class SalFrame;
struct SystemEnvData;
struct SystemGraphicsData;

namespace cairo {

    /// Holds all X11-output relevant data
    struct X11SysData
    {
        X11SysData();
        explicit X11SysData( const SystemGraphicsData& );
        explicit X11SysData( const SystemEnvData&, const SalFrame* pReference );

        _XDisplay* pDisplay;       // the relevant display connection
        Drawable hDrawable;     // a drawable
        Visual*  pVisual;        // the visual in use
        int nScreen;        // the current screen of the drawable
    };

    /// RAII wrapper for a pixmap
    struct X11Pixmap
    {
        _XDisplay* mpDisplay;  // the relevant display connection
        Pixmap  mhDrawable; // a drawable

        X11Pixmap( Pixmap hDrawable, _XDisplay* pDisplay ) :
            mpDisplay(pDisplay),
            mhDrawable(hDrawable)
        {}

        ~X11Pixmap();
    };

    typedef std::shared_ptr<X11Pixmap>       X11PixmapSharedPtr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
