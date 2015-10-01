/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_UNX_PIXMAP_HXX
#define INCLUDED_VCL_INC_UNX_PIXMAP_HXX

#include <prex.h>
#include <postx.h>
#include <tools/gen.hxx>
#include <unx/saltype.h>
#include <vclpluginapi.h>

class VCLPLUG_GEN_PUBLIC X11Pixmap
{
public:
    X11Pixmap( int nWidth, int nHeight )
    {
        mnWidth = nWidth;
        mnHeight = nHeight;
    }

    virtual ~X11Pixmap() {};

    virtual int GetDepth() const = 0;
    virtual SalX11Screen GetScreen() const = 0;
    virtual Pixmap GetPixmap() const = 0;
    Drawable GetDrawable() const { return GetPixmap(); };
    int GetWidth() const { return mnWidth; };
    int GetHeight() const { return mnHeight; };

protected:
    int             mnWidth;
    int             mnHeight;
};

#endif // INCLUDED_VCL_INC_UNX_PIXMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
