/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include "opengl/framebuffer.hxx"
#include "opengl/texture.hxx"

#include "opengl/FixedTextureAtlas.hxx"

FixedTextureAtlasManager::FixedTextureAtlasManager(int nWidthFactor, int nHeightFactor, int nSubTextureSize)
    : mWidthFactor(nWidthFactor)
    , mHeightFactor(nHeightFactor)
    , mSubTextureSize(nSubTextureSize)
{
}

FixedTextureAtlasManager::~FixedTextureAtlasManager()
{
    for (auto i = mpTextures.begin(); i != mpTextures.end(); ++i)
    {
        // Free texture early in VCL shutdown while we have a context.
        (*i)->Dispose();
        (*i)->DecreaseRefCount(0);
    }
}

void FixedTextureAtlasManager::CreateNewTexture()
{
    int nTextureWidth = mWidthFactor  * mSubTextureSize;
    int nTextureHeight = mHeightFactor * mSubTextureSize;
    mpTextures.push_back(new ImplOpenGLTexture(nTextureWidth, nTextureHeight, true));
    mpTextures.back()->InitializeSlots(mWidthFactor * mHeightFactor);
}

OpenGLTexture FixedTextureAtlasManager::InsertBuffer(int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData)
{
    ImplOpenGLTexture* pTexture = nullptr;

    auto funFreeSlot = [] (ImplOpenGLTexture *mpTexture)
    {
        return mpTexture->mnFreeSlots > 0;
    };

    auto it = std::find_if(mpTextures.begin(), mpTextures.end(), funFreeSlot);

    if (it != mpTextures.end())
    {
        pTexture = *it;
    }
    else
    {
        CreateNewTexture();
        pTexture = mpTextures.back();
    }

    int nSlot = pTexture->FindFreeSlot();

    // Calculate coordinates in texture
    int nX = (nSlot % mWidthFactor) * mSubTextureSize;
    int nY = (nSlot / mWidthFactor) * mSubTextureSize;

    Rectangle aRectangle(Point(nX, nY), Size(nWidth, nHeight));

    // If available, copy the image data to the texture
    if (pData)
    {
        if (!pTexture->InsertBuffer(nX, nY, nWidth, nHeight, nFormat, nType, pData))
            return OpenGLTexture();
    }

    return OpenGLTexture(pTexture, aRectangle, nSlot);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
