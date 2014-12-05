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
    mpPrevFramebuffer( NULL ),
    mpNextFramebuffer( NULL )
{
    glGenFramebuffers( 1, &mnId );
    SAL_INFO( "vcl.opengl", "Created framebuffer " << (int)mnId );
}

OpenGLFramebuffer::~OpenGLFramebuffer()
{
    glDeleteFramebuffers( 1, &mnId );
}

void OpenGLFramebuffer::Bind()
{
    glBindFramebuffer( GL_FRAMEBUFFER, mnId );
    SAL_INFO( "vcl.opengl", "Binding framebuffer " << (int)mnId );
    CHECK_GL_ERROR();
}

void OpenGLFramebuffer::Unbind()
{
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    SAL_INFO( "vcl.opengl", "Binding default framebuffer" );
    CHECK_GL_ERROR();
}

bool OpenGLFramebuffer::IsFree() const
{
    return (!maAttachedTexture);
}

bool OpenGLFramebuffer::IsAttached( const OpenGLTexture& rTexture ) const
{
    return ( maAttachedTexture == rTexture );
}

void OpenGLFramebuffer::AttachTexture( const OpenGLTexture& rTexture )
{
    SAL_INFO( "vcl.opengl", "Attaching texture " << rTexture.Id() << " to framebuffer " << (int)mnId );
    maAttachedTexture = rTexture;
    mnWidth = rTexture.GetWidth();
    mnHeight = rTexture.GetHeight();
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                            maAttachedTexture.Id(), 0 );
    CHECK_GL_ERROR();
}

void OpenGLFramebuffer::DetachTexture()
{
    maAttachedTexture = OpenGLTexture();
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0 );
    CHECK_GL_ERROR();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
