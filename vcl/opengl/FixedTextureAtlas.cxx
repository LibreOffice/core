/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>
#include <sal/config.h>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include <opengl/framebuffer.hxx>
#include <opengl/texture.hxx>

#include <opengl/FixedTextureAtlas.hxx>

struct FixedTexture
{
    std::shared_ptr<ImplOpenGLTexture> mpTexture;
    int mnFreeSlots;
    std::vector<bool> maAllocatedSlots;

    FixedTexture(int nTextureWidth, int nTextureHeight, int nNumberOfSlots)
        : mpTexture(new ImplOpenGLTexture(nTextureWidth, nTextureHeight, true))
        , mnFreeSlots(nNumberOfSlots)
        , maAllocatedSlots(nNumberOfSlots, false)
    {
        auto aDeallocateFunction = [this] (int nSlotNumber)
        {
            deallocateSlot(nSlotNumber);
        };

        mpTexture->SetSlotDeallocateCallback(aDeallocateFunction);
        mpTexture->InitializeSlotMechanism(nNumberOfSlots);
    }

    ~FixedTexture()
    {
        mpTexture->ResetSlotDeallocateCallback();
    }

    void allocateSlot(int nSlot)
    {
        maAllocatedSlots[nSlot] = true;
        mnFreeSlots--;
    }

    void deallocateSlot(int nSlot)
    {
        maAllocatedSlots[nSlot] = false;
        mnFreeSlots++;
    }

    int findAndAllocateFreeSlot()
    {
        for (size_t i = 0; i < maAllocatedSlots.size(); ++i)
        {
            if (!maAllocatedSlots[i])
            {
                allocateSlot(i);
                return i;
            }
        }
        return -1;
    }

private:
    FixedTexture(const FixedTexture&) = delete;
    FixedTexture& operator=(const FixedTexture&) = delete;
};

FixedTextureAtlasManager::FixedTextureAtlasManager(int nWidthFactor, int nHeightFactor, int nSubTextureSize)
    : mWidthFactor(nWidthFactor)
    , mHeightFactor(nHeightFactor)
    , mSubTextureSize(nSubTextureSize)
{
}

FixedTextureAtlasManager::~FixedTextureAtlasManager()
{
}

void FixedTextureAtlasManager::CreateNewTexture()
{
    int nTextureWidth = mWidthFactor  * mSubTextureSize;
    int nTextureHeight = mHeightFactor * mSubTextureSize;
    maFixedTextures.push_back(std::make_unique<FixedTexture>(nTextureWidth, nTextureHeight, mWidthFactor * mHeightFactor));
}

OpenGLTexture FixedTextureAtlasManager::Reserve(int nWidth, int nHeight)
{
    FixedTexture* pFixedTexture = nullptr;

    auto funFreeSlot = [] (std::unique_ptr<FixedTexture>& inFixedTexture)
    {
        return inFixedTexture->mnFreeSlots > 0;
    };

    auto it = std::find_if(maFixedTextures.begin(), maFixedTextures.end(), funFreeSlot);

    if (it != maFixedTextures.end())
    {
        pFixedTexture = (*it).get();
    }
    else
    {
        CreateNewTexture();
        pFixedTexture = maFixedTextures.back().get();
    }

    int nSlot = pFixedTexture->findAndAllocateFreeSlot();

    // Calculate coordinates in texture
    int nX = (nSlot % mWidthFactor) * mSubTextureSize;
    int nY = (nSlot / mWidthFactor) * mSubTextureSize;

    tools::Rectangle aRectangle(Point(nX, nY), Size(nWidth, nHeight));

    return OpenGLTexture(pFixedTexture->mpTexture, aRectangle, nSlot);
}

OpenGLTexture FixedTextureAtlasManager::InsertBuffer(int nWidth, int nHeight, int nFormat, int nType, sal_uInt8 const * pData)
{
    OpenGLTexture aTexture = Reserve(nWidth, nHeight);
    if (pData == nullptr)
        return aTexture;

    aTexture.CopyData(nWidth, nHeight, nFormat, nType, pData);

    return aTexture;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
