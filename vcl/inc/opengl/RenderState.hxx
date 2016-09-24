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

template<GLenum ENUM_TYPE, typename TYPE>
class GenericCapabilityState
{
protected:
    bool mbTest;

    static bool readState()
    {
        return glIsEnabled(ENUM_TYPE);
    }

public:
    void sync()
    {
        mbTest = readState();
    }

    void enable()
    {
        if (!mbTest)
        {
            glEnable(ENUM_TYPE);
            CHECK_GL_ERROR();
            mbTest = true;
        }
        else
        {
            VCL_GL_INFO(TYPE::className() << ": enable called but already enabled");
        }
#ifdef DBG_UTIL
        checkState();
#endif
    }

    void disable()
    {
        if (mbTest)
        {
            glDisable(ENUM_TYPE);
            CHECK_GL_ERROR();
            mbTest = false;
        }
        else
        {
            VCL_GL_INFO(TYPE::className() << ": disable called but already disabled");
        }
#ifdef DBG_UTIL
        checkState();
#endif
    }

#ifdef DBG_UTIL
    void checkState()
    {
        bool bRealState = readState();
        if (mbTest != bRealState)
        {
            VCL_GL_INFO(TYPE::className() << " mismatch! "
                            << "Expected: " << (mbTest ? "enabled" : "disabled")
                            << " but is: "        << (bRealState ? "enabled" : "disabled"));
        }
    }
#endif
};

class ScissorState : public GenericCapabilityState<GL_SCISSOR_TEST, ScissorState>
{
private:
    int mX;
    int mY;
    int mWidth;
    int mHeight;

public:
    static std::string className() { return std::string("ScissorState"); }

    ScissorState()
        : mX(0)
        , mY(0)
        , mWidth(0)
        , mHeight(0)
    {}

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
};

class StencilState : public GenericCapabilityState<GL_STENCIL_TEST, StencilState>
{
public:
    static std::string className() { return std::string("StencilState"); }
};

class BlendState : public GenericCapabilityState<GL_BLEND, BlendState>
{
    GLenum mnSourceMode;
    GLenum mnDestinationMode;
public:
    BlendState()
        : mnSourceMode(GL_ZERO)
        , mnDestinationMode(GL_ZERO)
    {}

    static std::string className() { return std::string("BlendState"); }

    void func(GLenum nSource, GLenum nDestination)
    {
        if (mnSourceMode != nSource || mnDestinationMode != nDestination)
        {
            glBlendFunc(nSource, nDestination);
            CHECK_GL_ERROR();
            mnSourceMode = nSource;
            mnDestinationMode = nDestination;
        }
    }
};

class RenderState
{
    TextureState maTexture;
    ScissorState maScissor;
    StencilState maStencil;
    BlendState   maBlend;

    Rectangle maCurrentViewport;

public:
    RenderState()
    {}

    void viewport(Rectangle aViewPort)
    {
        if (aViewPort != maCurrentViewport)
        {
            glViewport(aViewPort.Left(), aViewPort.Top(), aViewPort.GetWidth(), aViewPort.GetHeight());
            CHECK_GL_ERROR();
            maCurrentViewport = aViewPort;
        }
    }

    TextureState& texture() { return maTexture; }
    ScissorState& scissor() { return maScissor; }
    StencilState& stencil() { return maStencil; }
    BlendState&   blend()   { return maBlend; }

    void sync()
    {
        VCL_GL_INFO("RenderState::sync");
        maScissor.sync();
        maStencil.sync();
        maBlend.sync();
    }
};

#endif // INCLUDED_VCL_INC_OPENGL_RENDER_STATE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
