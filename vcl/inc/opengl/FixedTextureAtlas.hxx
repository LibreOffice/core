/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_FIXED_TEXTURE_ATLAS_H
#define INCLUDED_VCL_INC_OPENGL_FIXED_TEXTURE_ATLAS_H

#include "opengl/texture.hxx"


class VCL_PLUGIN_PUBLIC FixedTextureAtlasManager
{
    std::vector<std::unique_ptr<ImplOpenGLTexture>> mpTextures;

    int mWidthFactor;
    int mHeightFactor;
    int mSubTextureSize;

    void CreateNewTexture();

public:
    FixedTextureAtlasManager(int nWidthFactor, int nHeightFactor, int nTextureSize);
    OpenGLTexture InsertBuffer(int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData);

    int GetSubtextureSize()
    {
        return mSubTextureSize;
    }
};

#endif // INCLUDED_VCL_INC_OPENGL_FIXED_TEXTURE_ATLAS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
