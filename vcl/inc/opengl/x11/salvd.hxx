/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_X11_SALVD_H
#define INCLUDED_VCL_INC_OPENGL_X11_SALVD_H

#include <prex.h>
#include <postx.h>

#include <unx/saltype.h>
#include <salvd.hxx>

class SalDisplay;
class X11OpenGLSalGraphics;
class X11SalGraphics;

class X11OpenGLSalVirtualDevice : public SalVirtualDevice
{
    SalDisplay       *mpDisplay;
    X11SalGraphics   *mpGraphics;
    bool              mbGraphics;         // is Graphics used
    SalX11Screen      mnXScreen;
    int               mnWidth;
    int               mnHeight;
    sal_uInt16        mnDepth;

public:
    X11OpenGLSalVirtualDevice( SalGraphics *pGraphics,
                               long &nDX, long &nDY,
                               sal_uInt16 nBitCount,
                               const SystemGraphicsData *pData,
                               X11SalGraphics* pNewGraphics);
    virtual ~X11OpenGLSalVirtualDevice();

    // SalGeometryProvider
    virtual long GetWidth() const override { return mnWidth; }
    virtual long GetHeight() const override { return mnHeight; }

    SalDisplay *            GetDisplay() const { return mpDisplay; }
    SalX11Screen            GetXScreenNumber() const { return mnXScreen; }

    virtual SalGraphics*    AcquireGraphics() override;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) override;

                            // Set new size, without saving the old contents
    virtual bool            SetSize( long nNewDX, long nNewDY ) override;
};

#endif // INCLUDED_VCL_INC_OPENGL_X11_SALVD_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
