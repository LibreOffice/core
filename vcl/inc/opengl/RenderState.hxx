/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_RENDER_STATE_H
#define INCLUDED_VCL_INC_OPENGL_RENDER_STATE_H

#include "opengl/TextureState.hxx"

class ScissorState
{
    bool mbTest;
    int mX;
    int mY;
    int mWidth;
    int mHeight;

public:

    ScissorState()
        : mbTest(false)
        , mX(0)
        , mY(0)
        , mWidth(0)
        , mHeight(0)
    {
        glDisable(GL_SCISSOR_TEST);
        CHECK_GL_ERROR();
    }

    void set(int x, int y, int width, int height)
    {
        if (x != mX || y != mY || width != mWidth || height != mHeight)
        {
            glScissor(x, y, width, height);
            CHECK_GL_ERROR();

            mX = x;
            mY = y;
            mWidth = width;
            mHeight = height;
        }
    }

    void enable()
    {
        if (!mbTest)
        {
            glEnable(GL_SCISSOR_TEST);
            CHECK_GL_ERROR();
            mbTest = true;
        }
    }

    void disable()
    {
        if (mbTest)
        {
            glDisable(GL_SCISSOR_TEST);
            CHECK_GL_ERROR();
            mbTest = false;
        }
    }
};

class RenderState
{
    TextureState maTexture;
    ScissorState maScissor;

public:
    RenderState()
    {}

    TextureState& texture() { return maTexture; }
    ScissorState& scissor() { return maScissor; }
};

#endif // INCLUDED_VCL_INC_OPENGL_RENDER_STATE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
