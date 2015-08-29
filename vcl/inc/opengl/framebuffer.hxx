/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_FRAMEBUFFER_H
#define INCLUDED_VCL_INC_OPENGL_FRAMEBUFFER_H

#include <GL/glew.h>
#include <vcl/dllapi.h>

#include <opengl/texture.hxx>

class VCL_DLLPUBLIC OpenGLFramebuffer
{
private:
    GLuint      mnId;
    int         mnWidth;
    int         mnHeight;
    GLuint      mnAttachedTexture;

public:
    OpenGLFramebuffer();
    virtual ~OpenGLFramebuffer();

    GLuint  Id() const { return mnId; };
    int     GetWidth() const { return mnWidth; };
    int     GetHeight() const { return mnHeight; };

    void    Bind();
    static void Unbind();

    bool    IsFree() const;
    bool    IsAttached( GLuint nTexture ) const;
    bool    IsAttached( const OpenGLTexture& rTexture ) const;
    void    AttachTexture( const OpenGLTexture& rTexture );
    void    DetachTexture();

public:
    OpenGLFramebuffer* mpPrevFramebuffer;
    OpenGLFramebuffer* mpNextFramebuffer;
};

#endif // INCLUDED_VCL_INC_OPENGL_FRAMEBUFFER_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
