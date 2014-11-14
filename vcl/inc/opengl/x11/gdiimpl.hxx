/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_X11_GDIIMPL_HXX
#define INCLUDED_VCL_INC_OPENGL_X11_GDIIMPL_HXX

#include <vcl/dllapi.h>

#include "unx/salgdi.h"
#include "unx/x11/x11gdiimpl.h"
#include "openglgdiimpl.hxx"

class VCL_PLUGIN_PUBLIC X11OpenGLSalGraphicsImpl : public OpenGLSalGraphicsImpl, public X11GraphicsImpl
{
private:
    X11SalGraphics&     mrParent;

public:
    X11OpenGLSalGraphicsImpl( X11SalGraphics& rParent );
    virtual ~X11OpenGLSalGraphicsImpl();

protected:
    GLfloat GetWidth() const SAL_OVERRIDE;
    GLfloat GetHeight() const SAL_OVERRIDE;

public:
    // implementation of X11GraphicsImpl

    void Init() SAL_OVERRIDE;
    bool FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY ) SAL_OVERRIDE;
    bool RenderPixmapToScreen( X11Pixmap* pPixmap, int nX, int nY ) SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_INC_OPENGL_X11_GDIIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
