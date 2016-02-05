/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_UNX_X11WINDOWPROVIDER
#define INCLUDED_VCL_UNX_X11WINDOWPROVIDER

#include <X11/Xlib.h>

#include <vcl/dllapi.h>

class VCL_PLUGIN_PUBLIC X11WindowProvider
{
public:
    virtual ~X11WindowProvider();

    virtual Window GetX11Window() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
