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

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/dllapi.h>

#include <rtl/ustring.hxx>

// All member functions static and VCL_DLLPUBLIC. Basically a glorified namespace.
struct VCL_DLLPUBLIC OpenGLHelper
{
    OpenGLHelper() = delete; // Should not be instantiated

public:
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OUString& rGeometryShaderName, const OString& preamble, const OString& rDigest );
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OString& preamble, const OString& rDigest );
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName, const OUString& rGeometryShaderName);
    static GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName);

    /**
     * The caller is responsible for allocate the memory for the RGBA buffer, before call
     * this method. RGBA buffer size is assumed to be 4*width*height.
     * Since OpenGL uses textures flipped relative to BitmapEx storage this method
     * also adds the possibility to mirror the bitmap vertically at the same time.
    **/
    static void ConvertBitmapExToRGBATextureBuffer(const BitmapEx& rBitmapEx, sal_uInt8* o_pRGBABuffer, const bool bFlip = false);
    static BitmapEx ConvertBGRABufferToBitmapEx(const sal_uInt8* const pBuffer, long nWidth, long nHeight);
    static void renderToFile(long nWidth, long nHeight, const OUString& rFileName);

    /**
     * The caller is responsible for deleting the buffer objects identified by
     * nFramebufferId, nRenderbufferDepthId and nRenderbufferColorId
     * @param nWidth                Width of frame
     * @param nHeight               Height of frame
     * @param nFramebufferId        FrameBuffer ID
     * @param nRenderbufferDepthId  RenderBuffer's depth ID
     * @param nRenderbufferColorId  RenderBuffer's color ID
     * @param bRenderbuffer         true => off-screen rendering, false => rendering to texture
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
    static void debugMsgStream(std::ostringstream const &pStream);

    /**
     * Returns true if VCL has OpenGL rendering enabled
     */
    static bool isVCLOpenGLEnabled();

    /**
     * Returns the number of times OpenGL buffers have been swapped.
     */
    static sal_Int64 getBufferSwapCounter();
};

#ifdef SAL_LOG_WARN
#define CHECK_GL_ERROR() OpenGLHelper::checkGLError(__FILE__, __LINE__)
#else
#define CHECK_GL_ERROR() do { } while (false)
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
