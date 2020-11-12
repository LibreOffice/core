/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_TEXTURE_STATE_H
#define INCLUDED_VCL_INC_OPENGL_TEXTURE_STATE_H

#include <vcl/opengl/OpenGLHelper.hxx>

class TextureState
{
private:
    GLuint mnCurrentTextureUnit;
    std::vector<GLuint> maBoundTextures;

public:
    TextureState()
        : mnCurrentTextureUnit(0)
        , maBoundTextures(4, 0)
    {
    }

    static void generate(GLuint& nTexture)
    {
        glGenTextures(1, &nTexture);
        CHECK_GL_ERROR();
    }

    void active(GLuint nTextureUnit)
    {
        if (mnCurrentTextureUnit != nTextureUnit)
        {
            glActiveTexture(GL_TEXTURE0 + nTextureUnit);
            CHECK_GL_ERROR();
            mnCurrentTextureUnit = nTextureUnit;
        }
    }

    void bind(GLuint nTexture)
    {
        if (maBoundTextures[mnCurrentTextureUnit] != nTexture)
        {
            glBindTexture(GL_TEXTURE_2D, nTexture);
            CHECK_GL_ERROR();
            maBoundTextures[mnCurrentTextureUnit] = nTexture;
        }
    }

    void unbindAndDelete(GLuint nTexture)
    {
        unbind(nTexture);
        glDeleteTextures(1, &nTexture);
    }

    void unbind(GLuint nTexture)
    {
        for (size_t i = 0; i < maBoundTextures.size(); i++)
        {
            if (nTexture == maBoundTextures[i])
                maBoundTextures[i] = 0;
        }
    }
};

#endif // INCLUDED_VCL_INC_OPENGL_TEXTURE_STATE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
