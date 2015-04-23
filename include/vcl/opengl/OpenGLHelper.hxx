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
#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>

#include <rtl/ustring.hxx>

#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
#  include <prex.h>
#  include "GL/glxew.h"
#  include <postx.h>
#endif

class VCL_DLLPUBLIC OpenGLHelper
{
public:
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OString& preamble = "" );

    /**
     * The caller is responsible for allocate the memory for the RGBA buffer, before call
     * this method. RGBA buffer size is assumed to be 4*width*height.
     * Since OpenGL uses textures flipped relative to BitmapEx storage this method
     * also adds the possibility to mirror the bitmap vertically at the same time.
    **/
    static void ConvertBitmapExToRGBATextureBuffer(const BitmapEx& rBitmapEx, sal_uInt8* o_pRGBABuffer, const bool bFlip = false);
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

    static void checkGLError(const char* aFile, size_t nLine);

    /**
     * checks if the device/driver pair is on our OpenGL blacklist
     */
    static bool isDeviceBlacklisted();
    /**
     * checks if the system supports all features that are necessary for the OpenGL VCL support
     */
    static bool supportsVCLOpenGL();

    /**
     * Returns true if VCL has OpenGL rendering enabled
     */
    static bool isVCLOpenGLEnabled();

#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
    static bool GetVisualInfo(Display* pDisplay, int nScreen, XVisualInfo& rVI);
    static GLXFBConfig GetPixmapFBConfig( Display* pDisplay, bool& bInverted );
#endif
};

#define CHECK_GL_ERROR() OpenGLHelper::checkGLError(__FILE__, __LINE__)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
