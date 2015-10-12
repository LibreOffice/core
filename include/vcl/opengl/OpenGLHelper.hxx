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
#include <sal/log.hxx>
#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>

#include <rtl/ustring.hxx>

#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
#  include <prex.h>
#  include "GL/glxew.h"
#  include <postx.h>
#endif

/// Helper to do a SAL_INFO as well as a GL log.
#define VCL_GL_INFO(area,stream) \
    do { \
        if (SAL_DETAIL_ENABLE_LOG_INFO && OpenGLHelper::isVCLOpenGLEnabled()) \
        { \
            ::std::ostringstream detail_stream; \
            detail_stream << stream;            \
            OpenGLHelper::debugMsgStream((area),detail_stream); \
        } \
    } while (false)

// All member functions static and VCL_DLLPUBLIC. Basically a glorified namespace.
struct VCL_DLLPUBLIC OpenGLHelper
{
    OpenGLHelper() = delete; // Should not be instantiated

public:

    static rtl::OString GetDigest(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const rtl::OString& preamble = "" );

    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const rtl::OString& preamble = "", const rtl::OString& rDigest = "" );

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

    /// Get OpenGL version (needs a context)
    static float getGLVersion();

    static void checkGLError(const char* aFile, size_t nLine);

    /**
     * Insert a glDebugMessage into the queue - helpful for debugging
     * with apitrace to annotate the output and correlate it with code.
     */
    static void debugMsgPrint(const char *pArea, const char *pFormat, ...);
    static void debugMsgStream(const char *pArea, std::ostringstream const &pStream);

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
