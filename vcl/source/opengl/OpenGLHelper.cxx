/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/opengl/GLMHelper.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>
#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <boost/scoped_array.hpp>
#include <vcl/pngwrite.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>

#include <vector>

#include "opengl/zone.hxx"

#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID
#include "opengl/x11/X11DeviceInfo.hxx"
#elif defined (_WIN32)
#include "opengl/win/WinDeviceInfo.hxx"
#endif

namespace {

OUString getShaderFolder()
{
    OUString aUrl("$BRAND_BASE_DIR/" LIBO_ETC_FOLDER);
    rtl::Bootstrap::expandMacros(aUrl);

    return aUrl + "/opengl/";
}

OString loadShader(const OUString& rFilename)
{
    OUString aFileURL = getShaderFolder() + rFilename +".glsl";
    osl::File aFile(aFileURL);
    if(aFile.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None)
    {
        sal_uInt64 nSize = 0;
        aFile.getSize(nSize);
        boost::scoped_array<char> content(new char[nSize+1]);
        sal_uInt64 nBytesRead = 0;
        aFile.read(content.get(), nSize, nBytesRead);
        assert(nSize == nBytesRead);
        content.get()[nBytesRead] = 0;
        return OString(content.get());
    }
    else
    {
        SAL_WARN("vcl.opengl", "could not load the file: " << aFileURL);
    }

    return OString();
}

}

namespace {
    int LogCompilerError(GLuint nId, const rtl::OUString &rDetail,
                         const rtl::OUString &rName, bool bShaderNotProgram)
    {
        OpenGLZone aZone;

        int InfoLogLength = 0;

        CHECK_GL_ERROR();

        if (bShaderNotProgram)
            glGetShaderiv (nId, GL_INFO_LOG_LENGTH, &InfoLogLength);
        else
            glGetProgramiv(nId, GL_INFO_LOG_LENGTH, &InfoLogLength);

        CHECK_GL_ERROR();

        if ( InfoLogLength > 0 )
        {
            std::vector<char> ErrorMessage(InfoLogLength+1);
            if (bShaderNotProgram)
                glGetShaderInfoLog (nId, InfoLogLength, NULL, &ErrorMessage[0]);
            else
                glGetProgramInfoLog(nId, InfoLogLength, NULL, &ErrorMessage[0]);
            CHECK_GL_ERROR();

            ErrorMessage.push_back('\0');
            SAL_WARN("vcl.opengl", rDetail << " shader " << nId << " compile for " << rName << " failed : " << &ErrorMessage[0]);
        }
        else
            SAL_WARN("vcl.opengl", rDetail << " shader: " << rName << " compile " << nId << " failed without error log");
        return 0;
    }
}

static void addPreamble(OString& rShaderSource, const OString& rPreamble)
{
    if (rPreamble.isEmpty())
        return;

    OString aVersionStr("#version");
    int nVersionStrStartPos = rShaderSource.indexOf(aVersionStr, 0);

    if (nVersionStrStartPos == -1)
    {
        rShaderSource = rPreamble + "\n" + rShaderSource;
    }
    else
    {
        int nVersionStrEndPos = rShaderSource.indexOf('\n', nVersionStrStartPos);

        SAL_WARN_IF(nVersionStrEndPos == -1, "vcl.opengl", "syntax error in shader");

        if (nVersionStrEndPos == -1)
            nVersionStrEndPos = nVersionStrStartPos + 8;

        OString aVersionLine = rShaderSource.copy(0, nVersionStrEndPos - nVersionStrStartPos);
        OString aShaderBody = rShaderSource.copy(nVersionStrEndPos - nVersionStrStartPos);

        rShaderSource = aVersionLine + "\n" + rPreamble + "\n" + aShaderBody;
    }
}

GLint OpenGLHelper::LoadShaders(const OUString& rVertexShaderName,const OUString& rFragmentShaderName, const OString& preamble)
{
    OpenGLZone aZone;

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint Result = GL_FALSE;

    // Compile Vertex Shader
    OString aVertexShaderSource = loadShader(rVertexShaderName);
    if( !preamble.isEmpty())
        addPreamble( aVertexShaderSource, preamble );
    char const * VertexSourcePointer = aVertexShaderSource.getStr();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    if (!Result)
        return LogCompilerError(VertexShaderID, "vertex",
                                rVertexShaderName, true);

    // Compile Fragment Shader
    OString aFragmentShaderSource = loadShader(rFragmentShaderName);
    if( !preamble.isEmpty())
        addPreamble( aFragmentShaderSource, preamble );
    char const * FragmentSourcePointer = aFragmentShaderSource.getStr();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    if (!Result)
        return LogCompilerError(FragmentShaderID, "fragment",
                                rFragmentShaderName, true);

    // Link the program
    GLint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if (!Result)
        return LogCompilerError(ProgramID, "program", "<both>", false);

    CHECK_GL_ERROR();
    return ProgramID;
}

void OpenGLHelper::ConvertBitmapExToRGBATextureBuffer(const BitmapEx& rBitmapEx, sal_uInt8* o_pRGBABuffer, const bool bFlip)
{
    long nBmpWidth = rBitmapEx.GetSizePixel().Width();
    long nBmpHeight = rBitmapEx.GetSizePixel().Height();

    Bitmap aBitmap (rBitmapEx.GetBitmap());
    AlphaMask aAlpha (rBitmapEx.GetAlpha());
    Bitmap::ScopedReadAccess pReadAccces( aBitmap );
    AlphaMask::ScopedReadAccess pAlphaReadAccess( aAlpha );
    size_t i = 0;
    for (long ny = (bFlip ? nBmpHeight - 1 : 0); (bFlip ? ny >= 0 : ny < nBmpHeight); (bFlip ? ny-- : ny++))
    {
        Scanline pAScan = pAlphaReadAccess ? pAlphaReadAccess->GetScanline(ny) : 0;
        for(long nx = 0; nx < nBmpWidth; nx++)
        {
            BitmapColor aCol = pReadAccces->GetColor( ny, nx );
            o_pRGBABuffer[i++] = aCol.GetRed();
            o_pRGBABuffer[i++] = aCol.GetGreen();
            o_pRGBABuffer[i++] = aCol.GetBlue();
            o_pRGBABuffer[i++] = pAScan ? 255 - *pAScan++ : 255;
        }
    }
}

void OpenGLHelper::renderToFile(long nWidth, long nHeight, const OUString& rFileName)
{
    OpenGLZone aZone;

    boost::scoped_array<sal_uInt8> pBuffer(new sal_uInt8[nWidth*nHeight*4]);
    glReadPixels(0, 0, nWidth, nHeight, GL_BGRA, GL_UNSIGNED_BYTE, pBuffer.get());
    BitmapEx aBitmap = ConvertBGRABufferToBitmapEx(pBuffer.get(), nWidth, nHeight);
    try {
        vcl::PNGWriter aWriter( aBitmap );
        SvFileStream sOutput( rFileName, StreamMode::WRITE );
        aWriter.Write( sOutput );
        sOutput.Close();
    } catch (...) {
        SAL_WARN("vcl.opengl", "Error writing png to " << rFileName);
    }

    CHECK_GL_ERROR();
}

BitmapEx OpenGLHelper::ConvertBGRABufferToBitmapEx(const sal_uInt8* const pBuffer, long nWidth, long nHeight)
{
    assert(pBuffer);
    Bitmap aBitmap( Size(nWidth, nHeight), 24 );
    AlphaMask aAlpha( Size(nWidth, nHeight) );

    {
        Bitmap::ScopedWriteAccess pWriteAccess( aBitmap );
        AlphaMask::ScopedWriteAccess pAlphaWriteAccess( aAlpha );

        size_t nCurPos = 0;
        for( int y = 0; y < nHeight; ++y)
        {
            Scanline pScan = pWriteAccess->GetScanline(y);
            Scanline pAlphaScan = pAlphaWriteAccess->GetScanline(y);
            for( int x = 0; x < nWidth; ++x )
            {
                *pScan++ = pBuffer[nCurPos];
                *pScan++ = pBuffer[nCurPos+1];
                *pScan++ = pBuffer[nCurPos+2];

                nCurPos += 3;
                *pAlphaScan++ = static_cast<sal_uInt8>( 255 - pBuffer[nCurPos++] );
            }
        }
    }
    return BitmapEx(aBitmap, aAlpha);
}

const char* OpenGLHelper::GLErrorString(GLenum errorCode)
{
    static const struct {
        GLenum code;
        const char *string;
    } errors[]=
    {
        /* GL */
        {GL_NO_ERROR, "no error"},
        {GL_INVALID_ENUM, "invalid enumerant"},
        {GL_INVALID_VALUE, "invalid value"},
        {GL_INVALID_OPERATION, "invalid operation"},
        {GL_STACK_OVERFLOW, "stack overflow"},
        {GL_STACK_UNDERFLOW, "stack underflow"},
        {GL_OUT_OF_MEMORY, "out of memory"},
        {GL_INVALID_FRAMEBUFFER_OPERATION, "invalid framebuffer operation"},

        {0, NULL }
    };

    int i;

    for (i=0; errors[i].string; i++)
    {
        if (errors[i].code == errorCode)
        {
            return errors[i].string;
        }
     }

    return NULL;
}

std::ostream& operator<<(std::ostream& rStrm, const glm::vec4& rPos)
{
    rStrm << "( " << rPos[0] << ", " << rPos[1] << ", " << rPos[2] << ", " << rPos[3] << ")";
    return rStrm;
}

std::ostream& operator<<(std::ostream& rStrm, const glm::vec3& rPos)
{
    rStrm << "( " << rPos[0] << ", " << rPos[1] << ", " << rPos[2] << ")";
    return rStrm;
}

std::ostream& operator<<(std::ostream& rStrm, const glm::mat4& rMatrix)
{
    for(int i = 0; i < 4; ++i)
    {
        rStrm << "\n( ";
        for(int j = 0; j < 4; ++j)
        {
            rStrm << rMatrix[j][i];
            rStrm << " ";
        }
        rStrm << ")\n";
    }
    return rStrm;
}

void OpenGLHelper::createFramebuffer(long nWidth, long nHeight, GLuint& nFramebufferId,
        GLuint& nRenderbufferDepthId, GLuint& nRenderbufferColorId, bool bRenderbuffer)
{
    OpenGLZone aZone;

    // create a renderbuffer for depth attachment
    glGenRenderbuffers(1, &nRenderbufferDepthId);
    glBindRenderbuffer(GL_RENDERBUFFER, nRenderbufferDepthId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nWidth, nHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if(bRenderbuffer)
    {
        // create a renderbuffer for color attachment
        glGenRenderbuffers(1, &nRenderbufferColorId);
        glBindRenderbuffer(GL_RENDERBUFFER, nRenderbufferColorId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, nWidth, nHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    else
    {
        glGenTextures(1, &nRenderbufferColorId);
        glBindTexture(GL_TEXTURE_2D, nRenderbufferColorId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, nRenderbufferColorId, 0);
    }

    // create a framebuffer object and attach renderbuffer
    glGenFramebuffers(1, &nFramebufferId);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, nFramebufferId);
    // attach a renderbuffer to FBO color attachment point
    glBindRenderbuffer(GL_RENDERBUFFER, nRenderbufferColorId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, nRenderbufferColorId);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // attach a renderbuffer to depth attachment point
    glBindRenderbuffer(GL_RENDERBUFFER, nRenderbufferDepthId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, nRenderbufferDepthId);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        SAL_WARN("vcl.opengl", "invalid framebuffer status");
    }
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    CHECK_GL_ERROR();
}

float OpenGLHelper::getGLVersion()
{
    float fVersion = 1.0;
    const GLubyte* aVersion = glGetString( GL_VERSION );
    if( aVersion && aVersion[0] )
    {
        fVersion = aVersion[0] - '0';
        if( aVersion[1] == '.' && aVersion[2] )
        {
            fVersion += (aVersion[2] - '0')/10.0;
        }
    }

    CHECK_GL_ERROR();
    return fVersion;
}

void OpenGLHelper::checkGLError(const char* pFile, size_t nLine)
{
    OpenGLZone aZone;

    GLenum glErr = glGetError();
    if (glErr != GL_NO_ERROR)
    {
        const char* sError = OpenGLHelper::GLErrorString(glErr);

        if (sError)
            SAL_WARN("vcl.opengl", "GL Error #" << glErr << "(" << sError << ") in File " << pFile << " at line: " << nLine);
        else
            SAL_WARN("vcl.opengl", "GL Error #" << glErr << " (no message available) in File " << pFile << " at line: " << nLine);

        glErr = glGetError();
    }
}

bool OpenGLHelper::isDeviceBlacklisted()
{
    static bool bSet = false;
    static bool bBlacklisted = true; // assume the worst
    if (!bSet)
    {
        OpenGLZone aZone;

#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID
        X11OpenGLDeviceInfo aInfo;
        bBlacklisted = aInfo.isDeviceBlocked();
        SAL_INFO("vcl.opengl", "blacklisted: " << bBlacklisted);
#elif defined( _WIN32 )
        WinOpenGLDeviceInfo aInfo;
        bBlacklisted = aInfo.isDeviceBlocked();
#else
        bBlacklisted = false;
#endif
        bSet = true;
    }

    return bBlacklisted;
}

bool OpenGLHelper::supportsVCLOpenGL()
{
    static bool bDisableGL = !!getenv("SAL_DISABLEGL");
    bool bBlacklisted = isDeviceBlacklisted();

    if (bDisableGL || bBlacklisted)
        return false;
    else
        return true;
}

/// How many nested OpenGL code-paths are we inside ?
int OpenGLZone::gnInOpenGLZone = 0;

/**
 * Called from a signal handler if we get a crash in some GL code
 */
void OpenGLZone::hardDisable()
{
    // protect ourselves from double calling etc.
    static bool bDisabled = false;
    if (!bDisabled)
    {
        bDisabled = true;

        // Disable the OpenGL support
        std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::VCL::UseOpenGL::set(false,  xChanges);
        xChanges->commit();

        // Force synchronous config write
        css::uno::Reference< css::util::XFlushable >(
            css::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext()),
            css::uno::UNO_QUERY_THROW)->flush();
    }
}

bool OpenGLHelper::isVCLOpenGLEnabled()
{
    /**
     * The !bSet part should only be called once! Changing the results in the same
     * run will mix OpenGL and normal rendering.
     */
    static bool bSet = false;
    static bool bEnable = false;
    static bool bForceOpenGL = false;

    // If we are a console app, then we don't use OpenGL
    if ( Application::IsConsoleOnly() )
        return false;

    if (bSet)
    {
        return bForceOpenGL || bEnable;
    }
    /*
     * There are a number of cases that these environment variables cover:
     *  * SAL_FORCEGL forces OpenGL independent of any other option
     *  * SAL_DISABLEGL or a blacklisted driver avoid the use of OpenGL if SAL_FORCEGL is not set
     *  * SAL_ENABLEGL overrides VCL_HIDE_WINDOWS and the configuration variable
     *  * the configuration variable is checked if no environment variable is set
     */

    bSet = true;
    bForceOpenGL = !!getenv("SAL_FORCEGL") || officecfg::Office::Common::VCL::ForceOpenGL::get();
    if (bForceOpenGL)
        return true;

    if (!supportsVCLOpenGL())
    {
        return false;
    }

    static bool bEnableGLEnv = !!getenv("SAL_ENABLEGL");

    bEnable = bEnableGLEnv;

    static bool bDuringBuild = getenv("VCL_HIDE_WINDOWS");
    if (bDuringBuild && !bEnable /* env. enable overrides */)
        bEnable = false;
    else if (officecfg::Office::Common::VCL::UseOpenGL::get())
        bEnable = true;

    return bEnable;
}

#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)

bool OpenGLHelper::GetVisualInfo(Display* pDisplay, int nScreen, XVisualInfo& rVI)
{
    OpenGLZone aZone;

    XVisualInfo* pVI;
    int aAttrib[] = { GLX_RGBA,
                      GLX_RED_SIZE, 8,
                      GLX_GREEN_SIZE, 8,
                      GLX_BLUE_SIZE, 8,
                      GLX_DEPTH_SIZE, 24,
                      GLX_STENCIL_SIZE, 8,
                      None };

    pVI = glXChooseVisual( pDisplay, nScreen, aAttrib );
    if( !pVI )
        return false;

    rVI = *pVI;
    XFree( pVI );

    CHECK_GL_ERROR();
    return true;
}

GLXFBConfig OpenGLHelper::GetPixmapFBConfig( Display* pDisplay, bool& bInverted )
{
    OpenGLZone aZone;

    int nScreen = DefaultScreen( pDisplay );
    GLXFBConfig *aFbConfigs;
    int i, nFbConfigs, nValue;

    aFbConfigs = glXGetFBConfigs( pDisplay, nScreen, &nFbConfigs );
    for( i = 0; i < nFbConfigs; i++ )
    {
        glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_DRAWABLE_TYPE, &nValue );
        if( !(nValue & GLX_PIXMAP_BIT) )
            continue;

        glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_BIND_TO_TEXTURE_TARGETS_EXT, &nValue );
        if( !(nValue & GLX_TEXTURE_2D_BIT_EXT) )
            continue;

        glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_DEPTH_SIZE, &nValue );
        if( nValue != 24 )
            continue;

        glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_RED_SIZE, &nValue );
        if( nValue != 8 )
            continue;
        SAL_INFO( "vcl.opengl", "Red is " << nValue );

        // TODO: lfrb: Make it configurable wrt RGB/RGBA
        glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_BIND_TO_TEXTURE_RGB_EXT, &nValue );
        if( nValue == False )
        {
            glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_BIND_TO_TEXTURE_RGBA_EXT, &nValue );
            if( nValue == False )
                continue;
        }

        glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_Y_INVERTED_EXT, &nValue );

        // Looks like that X sends GLX_DONT_CARE but this usually means "true" for most
        // of the X implementations. Investigation on internet pointed that this could be
        // safely "true" all the time (for example gnome-shell always assumes "true").
        bInverted = nValue == True || nValue == int(GLX_DONT_CARE);

        break;
    }

    if( i == nFbConfigs )
    {
        SAL_WARN( "vcl.opengl", "Unable to find FBconfig for pixmap texturing" );
        return 0;
    }

    CHECK_GL_ERROR();
    return aFbConfigs[i];
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
