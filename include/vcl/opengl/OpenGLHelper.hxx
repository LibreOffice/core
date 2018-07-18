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

#include <epoxy/gl.h>
#include <sal/log.hxx>
#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>

#include <rtl/ustring.hxx>

/// Helper to do a SAL_INFO as well as a GL log.
#define VCL_GL_INFO(stream) \
    do { \
        if (SAL_DETAIL_ENABLE_LOG_INFO && OpenGLHelper::isVCLOpenGLEnabled()) \
        { \
            ::std::ostringstream detail_stream; \
            detail_stream << stream;            \
            OpenGLHelper::debugMsgStream(detail_stream); \
        } \
    } while (false)

/// Helper to do a SAL_WARN as well as a GL log.
#define VCL_GL_WARN(stream) \
    do { \
        if (SAL_DETAIL_ENABLE_LOG_INFO && OpenGLHelper::isVCLOpenGLEnabled()) \
        { \
            ::std::ostringstream detail_stream; \
            detail_stream << stream;            \
            OpenGLHelper::debugMsgStreamWarn(detail_stream); \
        } \
    } while (false)

// All member functions static and VCL_DLLPUBLIC. Basically a glorified namespace.
struct VCL_DLLPUBLIC OpenGLHelper
{
    OpenGLHelper() = delete; // Should not be instantiated

public:

    static OString GetDigest(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OString& preamble );

    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OUString& rGeometryShaderName, const OString& preamble, const OString& rDigest );
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OString& preamble, const OString& rDigest );
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OUString& rGeometryShaderName);
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName);

    /**
     * The caller is responsible for allocate the memory for the RGBA buffer, before call
     * this method. RGBA buffer size is assumed to be 4*width*height.
    **/
    static void ConvertBitmapExToRGBATextureBuffer(const BitmapEx& rBitmapEx, sal_uInt8* o_pRGBABuffer);
    static BitmapEx ConvertBGRABufferToBitmapEx(const sal_uInt8* const pBuffer, long nWidth, long nHeight);
    static void renderToFile(long nWidth, long nHeight, const OUString& rFileName);

    static const char* GLErrorString(GLenum errorCode);

    /**
     * The caller is responsible for deleting the buffer objects identified by
     * nFramebufferId, nRenderbufferDepthId and nRenderbufferColorId.
     * This create a buffer for rendering to texture and should be freed with
     * glDeleteTextures.
     *
     * @param nWidth                Width of frame
     * @param nHeight               Height of frame
     * @param nFramebufferId        FrameBuffer ID
     * @param nRenderbufferDepthId  RenderBuffer's depth ID
     * @param nRenderbufferColorId  RenderBuffer's color ID
     */
    static void createFramebuffer(long nWidth, long nHeight, GLuint& nFramebufferId,
            GLuint& nRenderbufferDepthId, GLuint& nRenderbufferColorId);

    /// Get OpenGL version (needs a context)
    static float getGLVersion();

    static void checkGLError(const char* aFile, size_t nLine);

    /**
     * Insert a glDebugMessage into the queue - helpful for debugging
     * with apitrace to annotate the output and correlate it with code.
     */
    static void debugMsgPrint(const int nType, const char *pFormat, ...);
    static void debugMsgStream(std::ostringstream const &pStream);
    static void debugMsgStreamWarn(std::ostringstream const &pStream);

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
};

#ifdef SAL_LOG_WARN
#define CHECK_GL_ERROR() OpenGLHelper::checkGLError(__FILE__, __LINE__)
#else
#define CHECK_GL_ERROR() do { } while (false)
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
