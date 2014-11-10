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
    X11Pixmap();
    X11Pixmap( Display *pDisplay, SalX11Screen nScreen, int nWidth, int nHeight, int nDepth );
    X11Pixmap( X11Pixmap& rOther );
    virtual ~X11Pixmap();

    Pixmap GetPixmap() const { return mpPixmap; };
    Drawable GetDrawable() const { return mpPixmap; };
    int GetWidth() const { return mnWidth; };
    int GetHeight() const { return mnHeight; };
    Size GetSize() const { return Size( mnWidth, mnHeight ); };
    int GetDepth() const { return mnDepth; };
    SalX11Screen GetScreen() const { return mnScreen; }

protected:
    Display*        mpDisplay;
    SalX11Screen    mnScreen;
    Pixmap          mpPixmap;
    bool            mbDeletePixmap;
    int             mnWidth;
    int             mnHeight;
    int             mnDepth;
};

#endif // INCLUDED_VCL_INC_UNX_PIXMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
