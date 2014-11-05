/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "texture.hxx"

OpenGLTexture::OpenGLTexture()
: mnTexture( 0 )
, mnWidth( -1 )
, mnHeight( -1 )
{
}

OpenGLTexture::OpenGLTexture( int nWidth, int nHeight )
: mnTexture( 0 )
, mnWidth( nWidth )
, mnHeight( nHeight )
{
    glGenTextures( 1, &mnTexture );
    glBindTexture( GL_TEXTURE_2D, mnTexture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glBindTexture( GL_TEXTURE_2D, 0 );
}

OpenGLTexture::~OpenGLTexture()
{
    if( mnTexture != 0 )
        glDeleteTextures( 1, &mnTexture );
}

GLuint OpenGLTexture::Id() const
{
    return mnTexture;
}

void OpenGLTexture::Bind()
{
    glBindTexture( GL_TEXTURE_2D, mnTexture );
}

void OpenGLTexture::Unbind()
{
    glBindTexture( GL_TEXTURE_2D, 0 );
}

bool OpenGLTexture::Draw()
{
    const GLfloat aPosition[8] = { -1, -1, -1, 1, 1, 1, 1, -1 };
    const GLfloat aTexCoord[8] = {  0,  0,  0, 1, 1, 1, 1,  0 };

    if( mnTexture == 0 )
        return false;

    glBindTexture( GL_TEXTURE_2D, mnTexture );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, aPosition );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, aTexCoord );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glDisableVertexAttribArray( 0 );
    glDisableVertexAttribArray( 1 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
