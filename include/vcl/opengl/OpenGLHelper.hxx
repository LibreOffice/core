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

#include <config_options.h>
#include <epoxy/gl.h>
#include <sal/detail/log.h>
#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>

#include <rtl/ustring.hxx>
#include <sstream>
#include <string_view>

/// Helper to do a SAL_INFO as well as a GL log.
#define VCL_GL_INFO(stream) \
    do { \
        if (SAL_DETAIL_ENABLE_LOG_INFO) \
        { \
            ::std::ostringstream detail_stream; \
            detail_stream << stream;            \
            OpenGLHelper::debugMsgStream(detail_stream); \
        } \
    } while (false)

/// Helper to do a SAL_WARN as well as a GL log.
#define VCL_GL_WARN(stream) \
    do { \
        if (SAL_DETAIL_ENABLE_LOG_INFO) \
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

#if defined _WIN32
    static OString GetDigest(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, std::string_view preamble );
#endif

    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OUString& rGeometryShaderName, std::string_view preamble, std::string_view rDigest );
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, std::string_view preamble, std::string_view rDigest );
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OUString& rGeometryShaderName);
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName);

    /**
     * The caller is responsible for allocating the memory for the buffer before calling
     * this method. The buffer size is assumed to be 4*width*height and the format
     * to be OptimalBufferFormat().
    **/
    static BitmapEx ConvertBufferToBitmapEx(const sal_uInt8* const pBuffer, tools::Long nWidth, tools::Long nHeight);
    /**
     * Returns the optimal buffer format for OpenGL (GL_BGRA or GL_RGBA).
    **/
    static GLenum OptimalBufferFormat();
    static void renderToFile(tools::Long nWidth, tools::Long nHeight, const OUString& rFileName);

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
    static void createFramebuffer(tools::Long nWidth, tools::Long nHeight, GLuint& nFramebufferId,
            GLuint& nRenderbufferDepthId, GLuint& nRenderbufferColorId);

    /// Get OpenGL version (needs a context)
    static float getGLVersion();

    static void checkGLError(const char* aFile, size_t nLine);

    /**
     * Insert a glDebugMessage into the queue - helpful for debugging
     * with apitrace to annotate the output and correlate it with code.
     */
#if defined __GNUC__
    __attribute__ ((format (printf, 2, 3)))
#endif
    static void debugMsgPrint(const int nType, const char *pFormat, ...);
    static void debugMsgStream(std::ostringstream const &pStream);
    static void debugMsgStreamWarn(std::ostringstream const &pStream);

    /**
     * checks if the device/driver pair is on our OpenGL denylist
     */
    static bool isDeviceDenylisted();

    /**
     * checks if the system supports all features that are necessary for the OpenGL support
     */
    static bool supportsOpenGL();
};

#ifdef SAL_LOG_WARN
#define CHECK_GL_ERROR() OpenGLHelper::checkGLError(__FILE__, __LINE__)
#else
#define CHECK_GL_ERROR() do { } while (false)
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
