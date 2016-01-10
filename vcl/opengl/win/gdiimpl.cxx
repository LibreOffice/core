/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "opengl/win/gdiimpl.hxx"
#include <o3tl/lru_map.hxx>
#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salframe.h>

WinOpenGLSalGraphicsImpl::WinOpenGLSalGraphicsImpl(WinSalGraphics& rGraphics,
                                                   SalGeometryProvider *mpProvider):
    OpenGLSalGraphicsImpl(rGraphics,mpProvider),
    mrParent(rGraphics)
{
}

void WinOpenGLSalGraphicsImpl::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    OpenGLSalGraphicsImpl *pImpl = pSrcGraphics ? static_cast< OpenGLSalGraphicsImpl* >(pSrcGraphics->GetImpl()) : static_cast< OpenGLSalGraphicsImpl *>(mrParent.GetImpl());
    OpenGLSalGraphicsImpl::DoCopyBits( rPosAry, *pImpl );
}

rtl::Reference<OpenGLContext> WinOpenGLSalGraphicsImpl::CreateWinContext()
{
    rtl::Reference<OpenGLContext> pContext = OpenGLContext::Create();
    pContext->setVCLOnly();
    pContext->init( mrParent.mhLocalDC, mrParent.mhWnd );
    return pContext;
}

void WinOpenGLSalGraphicsImpl::Init()
{
    if ( !IsOffscreen() && mpContext.is() && mpContext->isInitialized() &&
         ( mpContext->getOpenGLWindow().hWnd != mrParent.mhWnd ||
           mpContext->getOpenGLWindow().hDC == mrParent.mhLocalDC ) )
    {
        // This can legitimately happen, SalFrame keeps 2x
        // SalGraphics which share the same hWnd and hDC.
        // The shape 'Area' dialog does reparenting to trigger this.
        SAL_WARN("vcl.opengl", "Unusual: Windows handle / DC changed without DeInit");
        DeInit();
    }

    OpenGLSalGraphicsImpl::Init();
}

namespace
{

typedef std::pair<ControlCacheKey, std::unique_ptr<TextureCombo>> ControlCachePair;
typedef o3tl::lru_map<ControlCacheKey, std::unique_ptr<TextureCombo>, ControlCacheHashFunction> ControlCacheType;

ControlCacheType gTextureCache(200);

}

bool WinOpenGLSalGraphicsImpl::TryRenderCachedNativeControl(ControlCacheKey& rControlCacheKey, int nX, int nY)
{
    static bool gbCacheEnabled = !getenv("SAL_WITHOUT_WIDGET_CACHE");

    if (!gbCacheEnabled)
        return false;

    ControlCacheType::const_iterator iterator = gTextureCache.find(rControlCacheKey);

    if (iterator == gTextureCache.end())
        return false;

    const std::unique_ptr<TextureCombo>& pCombo = iterator->second;

    bool bRet = false;

    PreDraw();

    bRet = RenderTextureCombo(*pCombo, nX, nY);

    PostDraw();

    return bRet;
}

bool WinOpenGLSalGraphicsImpl::RenderTextureCombo(TextureCombo& rCombo, int nX, int nY)
{
    OpenGLTexture& rTexture = *rCombo.mpTexture;

    SalTwoRect aPosAry(0,   0, rTexture.GetWidth(), rTexture.GetHeight(),
                       nX, nY, rTexture.GetWidth(), rTexture.GetHeight());

    DrawTextureDiff(rTexture, *rCombo.mpMask, aPosAry);

    return true;
}

bool WinOpenGLSalGraphicsImpl::RenderCompatibleDC(OpenGLCompatibleDC& rWhite, OpenGLCompatibleDC& rBlack,
                                                  int nX, int nY, TextureCombo& rCombo)
{
    bool bRet = false;

    PreDraw();

    rCombo.mpTexture.reset(rWhite.getTexture());
    rCombo.mpMask.reset(rBlack.getTexture());

    bRet = RenderTextureCombo(rCombo, nX, nY);

    PostDraw();
    return bRet;
}

bool WinOpenGLSalGraphicsImpl::RenderAndCacheNativeControl(OpenGLCompatibleDC& rWhite, OpenGLCompatibleDC& rBlack,
                                                           int nX, int nY , ControlCacheKey& aControlCacheKey)
{
    std::unique_ptr<TextureCombo> pCombo(new TextureCombo);

    bool bResult = RenderCompatibleDC(rWhite, rBlack, nX, nY, *pCombo);
    if (!bResult)
        return false;

    if (!aControlCacheKey.canCacheControl())
        return true;

    ControlCachePair pair(aControlCacheKey, std::move(pCombo));
    gTextureCache.insert(std::move(pair));

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
