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

#include <vcl/dllapi.h>

#include <opengl/texture.hxx>

class VCL_DLLPUBLIC OpenGLFramebuffer final
{
private:
    GLuint      mnId;
    int         mnWidth;
    int         mnHeight;
    GLuint      mnAttachedTexture;

public:
    OpenGLFramebuffer();
    ~OpenGLFramebuffer();

    int     GetWidth() const { return mnWidth; };
    int     GetHeight() const { return mnHeight; };

    void    Bind(GLenum eTarget = GL_FRAMEBUFFER);

    static void Unbind(GLenum eTarget = GL_FRAMEBUFFER);

    bool    IsFree() const;
    bool    IsAttached( GLuint nTexture ) const;
    bool    IsAttached( const OpenGLTexture& rTexture ) const;
    void    AttachTexture( const OpenGLTexture& rTexture );
    void    DetachTexture();

public:
    OpenGLFramebuffer* mpPrevFramebuffer;
};

#endif // INCLUDED_VCL_INC_OPENGL_FRAMEBUFFER_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
