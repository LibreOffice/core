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

#include <unx/salgdi.h>
#include <unx/x11/x11gdiimpl.h>
#include <openglgdiimpl.hxx>
#include <ControlCacheKey.hxx>

struct TextureCombo;

class VCL_PLUGIN_PUBLIC X11OpenGLSalGraphicsImpl : public OpenGLSalGraphicsImpl, public X11GraphicsImpl
{
private:
    X11SalGraphics&     mrX11Parent;

public:
    X11OpenGLSalGraphicsImpl( X11SalGraphics& rParent );
    virtual ~X11OpenGLSalGraphicsImpl() override;

protected:
    virtual rtl::Reference<OpenGLContext> CreateWinContext() override;

    bool RenderPixmap(X11Pixmap const * pPixmap, X11Pixmap const * pMask, int nX, int nY, TextureCombo& rCombo);

public:
    // implementation of X11GraphicsImpl

    virtual void copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) override;

    virtual void Init() override;

    void FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY ) override;
    bool RenderPixmapToScreen(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY) override;

    bool RenderAndCacheNativeControl(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY,
                                     ControlCacheKey& aControlCacheKey) override;
    bool TryRenderCachedNativeControl(ControlCacheKey& rControlCacheKey,
                                      int nX, int nY) override;
};

#endif // INCLUDED_VCL_INC_OPENGL_X11_GDIIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
