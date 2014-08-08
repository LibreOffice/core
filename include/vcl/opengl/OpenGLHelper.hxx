/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_OPENGL_OPENGLHELPER_HXX
#define INCLUDED_VCL_OPENGL_OPENGLHELPER_HXX

#include <GL/glew.h>
#include <vcl/vclopengl_dllapi.hxx>
#include <vcl/bitmapex.hxx>

#include <rtl/ustring.hxx>

class VCLOPENGL_DLLPUBLIC OpenGLHelper
{
public:
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName);

    static sal_uInt8* ConvertBitmapExToRGBABuffer(const BitmapEx& rBitmapEx);
    static BitmapEx ConvertBGRABufferToBitmapEx(const sal_uInt8* const pBuffer, long nWidth, long nHeight);
    static void renderToFile(long nWidth, long nHeight, const OUString& rFileName);

    static const char* GLErrorString(GLenum errorCode);

    /**
     * The caller is responsible for deleting the buffer objects identified by
     * nFramebufferId, nRenderbufferDepthId and nRenderbufferColorId
     * @param bRenderbuffer true => off-screen rendering, false => rendering to texture
     *          This also affects whether to free with glDeleteRenderbuffers or glDeleteTextures
     */
    static void createFramebuffer(long nWidth, long nHeight, GLuint& nFramebufferId,
            GLuint& nRenderbufferDepthId, GLuint& nRenderbufferColorId, bool bRenderbuffer = true);

    // Get OpenGL version (needs a context)
    static float getGLVersion();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
