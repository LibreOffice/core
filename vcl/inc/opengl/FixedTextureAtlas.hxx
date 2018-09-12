/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_FIXEDTEXTUREATLAS_HXX
#define INCLUDED_VCL_INC_OPENGL_FIXEDTEXTUREATLAS_HXX

#include <memory>
#include <opengl/texture.hxx>

struct FixedTexture;

class FixedTextureAtlasManager final
{
    std::vector<std::unique_ptr<FixedTexture>> maFixedTextures;

    int const mWidthFactor;
    int const mHeightFactor;
    int const mSubTextureSize;

    void CreateNewTexture();

    FixedTextureAtlasManager( const FixedTextureAtlasManager& ) = delete;
    FixedTextureAtlasManager& operator=( const FixedTextureAtlasManager& ) = delete;

public:
    FixedTextureAtlasManager(int nWidthFactor, int nHeightFactor, int nTextureSize);
    ~FixedTextureAtlasManager();

    OpenGLTexture InsertBuffer(int nWidth, int nHeight, int nFormat, int nType, sal_uInt8 const * pData);
    OpenGLTexture Reserve(int nWidth, int nHeight);

    int GetSubtextureSize()
    {
        return mSubTextureSize;
    }
};

#endif // INCLUDED_VCL_INC_OPENGL_FIXEDTEXTUREATLAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
