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

#include <memory>
#include <vcl/dllapi.h>

#include "openglgdiimpl.hxx"
#include "svdata.hxx"
#include "win/salgdi.h"
#include <o3tl/lru_map.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include "ControlCacheKey.hxx"

class WinOpenGLSalGraphicsImpl : public OpenGLSalGraphicsImpl
{
    friend class WinLayout;
private:
    WinSalGraphics& mrWinParent;

    bool RenderCompatibleDC(OpenGLCompatibleDC& rWhite, OpenGLCompatibleDC& rBlack,
                            int nX, int nY, TextureCombo& rCombo);

public:
    WinOpenGLSalGraphicsImpl(WinSalGraphics& rGraphics,
                             SalGeometryProvider *mpProvider);

protected:
    virtual rtl::Reference<OpenGLContext> CreateWinContext() override;

    bool RenderTextureCombo(TextureCombo const & rCombo, int nX, int nY);

public:
    virtual void Init() override;
    virtual void copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) override;


    bool TryRenderCachedNativeControl(ControlCacheKey const & rControlCacheKey, int nX, int nY);

    bool RenderAndCacheNativeControl(OpenGLCompatibleDC& rWhite, OpenGLCompatibleDC& rBlack,
                                     int nX, int nY , ControlCacheKey& aControlCacheKey);

};

typedef std::pair<ControlCacheKey, std::unique_ptr<TextureCombo>> ControlCachePair;
typedef o3tl::lru_map<ControlCacheKey, std::unique_ptr<TextureCombo>, ControlCacheHashFunction> ControlCacheType;

class TheTextureCache {
    ControlCacheType cache;

    TheTextureCache();

public:
    static ControlCacheType & get();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
