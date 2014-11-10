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

#ifndef INCLUDED_VCL_INC_OPENGL_TEXTURE_H
#define INCLUDED_VCL_INC_OPENGL_TEXTURE_H

#include <boost/shared_ptr.hpp>
#include <GL/glew.h>

class OpenGLTexture
{
private:
    GLuint mnTexture;
    int    mnWidth;
    int    mnHeight;
    GLenum mnFilter;

public:
    OpenGLTexture();
    OpenGLTexture( int nWidth, int nHeight );
    OpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData );
    OpenGLTexture( int nX, int nY, int nWidth, int nHeight );
    virtual ~OpenGLTexture();

    GLuint Id() const;
    void Bind();
    void Unbind();
    bool Draw();

    GLenum GetFilter() const;
    void SetFilter( GLenum nFilter );
};

typedef boost::shared_ptr< OpenGLTexture > OpenGLTextureSharedPtr;

#endif // INCLUDED_VCL_INC_OPENGL_TEXTURE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
