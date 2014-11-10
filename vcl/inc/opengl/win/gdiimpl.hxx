/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_WIN_GDIIMPL_HXX
#define INCLUDED_VCL_INC_OPENGL_WIN_GDIIMPL_HXX

#include <vcl/dllapi.h>

#include "openglgdiimpl.hxx"
#include "win/salgdi.h"

class WinOpenGLSalGraphicsImpl : public OpenGLSalGraphicsImpl
{
private:
    WinSalGraphics& mrParent;

public:
    WinOpenGLSalGraphicsImpl(WinSalGraphics& rGraphics);

protected:
    virtual GLfloat GetWidth() const SAL_OVERRIDE;
    virtual GLfloat GetHeight() const SAL_OVERRIDE;

public:

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
