/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/log.hxx>

#include <opengl/framebuffer.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>

OpenGLFramebuffer::OpenGLFramebuffer() :
    mnId( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 ),
    mnAttachedTexture( 0 ),
    mpPrevFramebuffer( nullptr )
{
    glGenFramebuffers( 1, &mnId );
    CHECK_GL_ERROR();
    VCL_GL_INFO( "Created framebuffer " << static_cast<int>(mnId) );
}

OpenGLFramebuffer::~OpenGLFramebuffer()
{
    glDeleteFramebuffers( 1, &mnId );
    VCL_GL_INFO( "Deleted framebuffer " << static_cast<int>(mnId) );
    CHECK_GL_ERROR();
}

void OpenGLFramebuffer::Bind(GLenum eTarget)
{
    VCL_GL_INFO( "Binding framebuffer " << static_cast<int>(mnId) );
    glBindFramebuffer(eTarget, mnId);
    CHECK_GL_ERROR();
}

void OpenGLFramebuffer::Unbind(GLenum eTarget)
{
    glBindFramebuffer(eTarget, 0);
    CHECK_GL_ERROR();
    VCL_GL_INFO( "Binding default framebuffer" );
}

bool OpenGLFramebuffer::IsFree() const
{
    return !mnAttachedTexture;
}

bool OpenGLFramebuffer::IsAttached( GLuint nTexture ) const
{
    return mnAttachedTexture == nTexture;
}

bool OpenGLFramebuffer::IsAttached( const OpenGLTexture& rTexture ) const
{
    return mnAttachedTexture == rTexture.Id();
}

void OpenGLFramebuffer::AttachTexture( const OpenGLTexture& rTexture )
{
    if( rTexture.Id() == mnAttachedTexture )
        return;

    VCL_GL_INFO( "Attaching texture " << rTexture.Id() << " to framebuffer " << static_cast<int>(mnId) );
    mnAttachedTexture = rTexture.Id();
    mnWidth = rTexture.GetWidth();
    mnHeight = rTexture.GetHeight();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mnAttachedTexture, 0);
    CHECK_GL_ERROR();

    GLuint nStencil = rTexture.StencilId();
    if( nStencil )
    {
        VCL_GL_INFO( "Attaching stencil " << nStencil << " to framebuffer " << static_cast<int>(mnId) );
        glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                   GL_RENDERBUFFER, nStencil );
        CHECK_GL_ERROR();
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    CHECK_GL_ERROR();
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        SAL_WARN("vcl.opengl", "Framebuffer incomplete");
    }
}

void OpenGLFramebuffer::DetachTexture()
{
    if( mnAttachedTexture != 0 )
    {
        mnAttachedTexture = 0;
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0 );
        CHECK_GL_ERROR();

        // FIXME: we could make this conditional on having a stencil ?
        glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                   GL_RENDERBUFFER, 0 );
        CHECK_GL_ERROR();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
