/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_PACKEDTEXTUREATLAS_HXX
#define INCLUDED_VCL_INC_OPENGL_PACKEDTEXTUREATLAS_HXX

#include <memory>
#include <opengl/texture.hxx>

struct PackedTexture;

/**
 * Pack texutres into one texutre atlas.
 *
 * This is based on algorithm described in [1] and is an
 * adaptation of "texture atlas generator" from [2].
 *
 * [1]: http://www.blackpawn.com/texts/lightmaps/
 * [2]: https://github.com/lukaszdk/texture-atlas-generator
 *
 */
class VCL_DLLPUBLIC PackedTextureAtlasManager final
{
    std::vector<std::unique_ptr<PackedTexture>> maPackedTextures;

    int const mnTextureWidth;
    int const mnTextureHeight;

    void CreateNewTexture();

    PackedTextureAtlasManager( const PackedTextureAtlasManager& ) = delete;
    PackedTextureAtlasManager& operator=( const PackedTextureAtlasManager& ) = delete;

public:

    /**
     * nTextureWidth and nTextureHeight are the dimensions of the common texture(s)
     * nTextureLimit is the maximum limit of that a texture atlas can have (0 or less - unlimited)
     */
    PackedTextureAtlasManager(int nTextureWidth, int nTextureHeight);
    ~PackedTextureAtlasManager();

    OpenGLTexture InsertBuffer(int nWidth, int nHeight, int nFormat, int nType, sal_uInt8 const * pData);
    OpenGLTexture Reserve(int nWidth, int nHeight);
    std::vector<GLuint> ReduceTextureNumber(int nMaxNumberOfTextures);
};

#endif // INCLUDED_VCL_INC_OPENGL_PACKEDTEXTUREATLAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
