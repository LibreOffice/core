/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/opengl/OpenGLHelper.hxx>
#include <config_vclplug.h>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/digest.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <config_folders.h>
#include <memory>
#include <vcl/pngwrite.hxx>
#include <vcl/svapp.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>

#include <stdarg.h>
#include <string_view>
#include <vector>
#include <unordered_map>

#include <opengl/zone.hxx>
#include <vcl/opengl/OpenGLWrapper.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <desktop/crashreport.hxx>
#include <bitmap/BitmapWriteAccess.hxx>
#include <watchdog.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <vcl/glxtestprocess.hxx>
#include <salinst.hxx>
#include <svdata.hxx>

#if USING_X11
#include <opengl/x11/X11DeviceInfo.hxx>
#elif defined (_WIN32)
#include <opengl/win/WinDeviceInfo.hxx>
#endif

#include "GLMHelper.hxx"

static bool volatile gbInShaderCompile = false;

namespace {

using namespace rtl;

OUString getShaderFolder()
{
    OUString aUrl("$BRAND_BASE_DIR/" LIBO_ETC_FOLDER);
    rtl::Bootstrap::expandMacros(aUrl);

    return aUrl + "/opengl/";
}

OString loadShader(std::u16string_view rFilename)
{
    OUString aFileURL = getShaderFolder() + rFilename +".glsl";
    osl::File aFile(aFileURL);
    if(aFile.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None)
    {
        sal_uInt64 nSize = 0;
        aFile.getSize(nSize);
        std::unique_ptr<char[]> content(new char[nSize+1]);
        sal_uInt64 nBytesRead = 0;
        aFile.read(content.get(), nSize, nBytesRead);
        assert(nSize == nBytesRead);
        content.get()[nBytesRead] = 0;
        SAL_INFO("vcl.opengl", "Read " << nBytesRead << " bytes from " << aFileURL);
        return content.get();
    }
    else
    {
        SAL_WARN("vcl.opengl", "Could not open " << aFileURL);
    }

    return OString();
}

OString& getShaderSource(const OUString& rFilename)
{
    static std::unordered_map<OUString, OString> aMap;

    if (aMap.find(rFilename) == aMap.end())
    {
        aMap[rFilename] = loadShader(rFilename);
    }

    return aMap[rFilename];
}

}

namespace {
    int LogCompilerError(GLuint nId, const OUString &rDetail,
                         const OUString &rName, bool bShaderNotProgram)
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
                glGetShaderInfoLog (nId, InfoLogLength, nullptr, ErrorMessage.data());
            else
                glGetProgramInfoLog(nId, InfoLogLength, nullptr, ErrorMessage.data());
            CHECK_GL_ERROR();

            ErrorMessage.push_back('\0');
            SAL_WARN("vcl.opengl", rDetail << " shader " << nId << " compile for " << rName << " failed : " << ErrorMessage.data());
        }
        else
            SAL_WARN("vcl.opengl", rDetail << " shader: " << rName << " compile " << nId << " failed without error log");

#ifdef DBG_UTIL
        abort();
#endif
        return 0;
    }
}

static void addPreamble(OString& rShaderSource, std::string_view rPreamble)
{
    if (rPreamble.empty())
        return;

    int nVersionStrStartPos = rShaderSource.indexOf("#version");

    if (nVersionStrStartPos == -1)
    {
        rShaderSource = OString::Concat(rPreamble) + "\n" + rShaderSource;
    }
    else
    {
        int nVersionStrEndPos = rShaderSource.indexOf('\n', nVersionStrStartPos);

        SAL_WARN_IF(nVersionStrEndPos == -1, "vcl.opengl", "syntax error in shader");

        if (nVersionStrEndPos == -1)
            nVersionStrEndPos = nVersionStrStartPos + 8;

        OString aVersionLine = rShaderSource.copy(0, nVersionStrEndPos);
        OString aShaderBody = rShaderSource.copy(nVersionStrEndPos + 1);

        rShaderSource = aVersionLine + "\n" + rPreamble + "\n" + aShaderBody;
    }
}

namespace
{
    const sal_uInt32 GLenumSize = sizeof(GLenum);

    OString getHexString(const sal_uInt8* pData, sal_uInt32 nLength)
    {
        static const char* const pHexData = "0123456789ABCDEF";

        bool bIsZero = true;
        OStringBuffer aHexStr;
        for(size_t i = 0; i < nLength; ++i)
        {
            sal_uInt8 val = pData[i];
            if( val != 0 )
                bIsZero = false;
            aHexStr.append( pHexData[ val & 0xf ] );
            aHexStr.append( pHexData[ val >> 4 ] );
        }
        if( bIsZero )
            return OString();
        else
            return aHexStr.makeStringAndClear();
    }

    OString generateMD5(const void* pData, size_t length)
    {
        sal_uInt8 pBuffer[RTL_DIGEST_LENGTH_MD5];
        rtlDigestError aError = rtl_digest_MD5(pData, length,
                pBuffer, RTL_DIGEST_LENGTH_MD5);
        SAL_WARN_IF(aError != rtl_Digest_E_None, "vcl.opengl", "md5 generation failed");

        return getHexString(pBuffer, RTL_DIGEST_LENGTH_MD5);
    }

    OString getDeviceInfoString()
    {
#if USING_X11
        const X11OpenGLDeviceInfo aInfo;
        return aInfo.GetOS() +
            aInfo.GetOSRelease() +
            aInfo.GetRenderer() +
            aInfo.GetVendor() +
            aInfo.GetVersion();
#elif defined( _WIN32 )
        const WinOpenGLDeviceInfo aInfo;
        return OUStringToOString(aInfo.GetAdapterVendorID(), RTL_TEXTENCODING_UTF8) +
            OUStringToOString(aInfo.GetAdapterDeviceID(), RTL_TEXTENCODING_UTF8) +
            OUStringToOString(aInfo.GetDriverVersion(), RTL_TEXTENCODING_UTF8) +
            OString::number(DriverBlocklist::GetWindowsVersion());
#else
        return OString::Concat(reinterpret_cast<const char*>(glGetString(GL_VENDOR))) +
            reinterpret_cast<const char*>(glGetString(GL_RENDERER)) +
            reinterpret_cast<const char*>(glGetString(GL_VERSION));
#endif
    }

    OString getStringDigest( const OUString& rVertexShaderName,
                             const OUString& rFragmentShaderName,
                             std::string_view rPreamble )
    {
        // read shaders source
        OString aVertexShaderSource = getShaderSource( rVertexShaderName );
        OString aFragmentShaderSource = getShaderSource( rFragmentShaderName );

        // get info about the graphic device
        static const OString aDeviceInfo (getDeviceInfoString());

        OString aMessage = rPreamble +
            aVertexShaderSource +
            aFragmentShaderSource +
            aDeviceInfo;

        return generateMD5(aMessage.getStr(), aMessage.getLength());
    }

    OString getCacheFolder()
    {
        OUString url("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
        rtl::Bootstrap::expandMacros(url);

        osl::Directory::create(url);

        return OUStringToOString(url, RTL_TEXTENCODING_UTF8);
    }


    bool writeProgramBinary( const OString& rBinaryFileName,
                             const std::vector<sal_uInt8>& rBinary )
    {
        osl::File aFile(OStringToOUString(rBinaryFileName, RTL_TEXTENCODING_UTF8));
        osl::FileBase::RC eStatus = aFile.open(
                osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );

        if( eStatus != osl::FileBase::E_None )
        {
            // when file already exists we do not have to save it:
            // we can be sure that the binary to save is exactly equal
            // to the already saved binary, since they have the same hash value
            if( eStatus == osl::FileBase::E_EXIST )
            {
                SAL_INFO( "vcl.opengl",
                        "No binary program saved. A file with the same hash already exists: '" << rBinaryFileName << "'" );
                return true;
            }
            return false;
        }

        sal_uInt64 nBytesWritten = 0;
        aFile.write( rBinary.data(), rBinary.size(), nBytesWritten );

        assert( rBinary.size() == nBytesWritten );

        return true;
    }

    bool readProgramBinary( const OString& rBinaryFileName,
                            std::vector<sal_uInt8>& rBinary )
    {
        osl::File aFile( OStringToOUString( rBinaryFileName, RTL_TEXTENCODING_UTF8 ) );
        if(aFile.open( osl_File_OpenFlag_Read ) == osl::FileBase::E_None)
        {
            sal_uInt64 nSize = 0;
            aFile.getSize( nSize );
            rBinary.resize( nSize );
            sal_uInt64 nBytesRead = 0;
            aFile.read( rBinary.data(), nSize, nBytesRead );
            assert( nSize == nBytesRead );
            VCL_GL_INFO("Loading file: '" << rBinaryFileName << "': success" );
            return true;
        }
        else
        {
            VCL_GL_INFO("Loading file: '" << rBinaryFileName << "': FAIL");
        }

        return false;
    }

    OString createFileName( std::u16string_view rVertexShaderName,
                            std::u16string_view rFragmentShaderName,
                            std::u16string_view rGeometryShaderName,
                            std::string_view rDigest )
    {
        OString aFileName = getCacheFolder() +
            OUStringToOString( rVertexShaderName, RTL_TEXTENCODING_UTF8 ) + "-" +
            OUStringToOString( rFragmentShaderName, RTL_TEXTENCODING_UTF8 ) + "-";
        if (!rGeometryShaderName.empty())
            aFileName += OUStringToOString( rGeometryShaderName, RTL_TEXTENCODING_UTF8 ) + "-";
        aFileName += OString::Concat(rDigest) + ".bin";
        return aFileName;
    }

    GLint loadProgramBinary( GLuint nProgramID, const OString& rBinaryFileName )
    {
        GLint nResult = GL_FALSE;
        GLenum nBinaryFormat;
        std::vector<sal_uInt8> aBinary;
        if( readProgramBinary( rBinaryFileName, aBinary ) && aBinary.size() > GLenumSize )
        {
            GLint nBinaryLength = aBinary.size() - GLenumSize;

            // Extract binary format
            sal_uInt8* pBF = reinterpret_cast<sal_uInt8*>(&nBinaryFormat);
            for( size_t i = 0; i < GLenumSize; ++i )
            {
                pBF[i] = aBinary[nBinaryLength + i];
            }

            // Load the program
            glProgramBinary( nProgramID, nBinaryFormat, aBinary.data(), nBinaryLength );

            // Check the program
            glGetProgramiv(nProgramID, GL_LINK_STATUS, &nResult);
        }
        return nResult;
    }

    void saveProgramBinary( GLint nProgramID, const OString& rBinaryFileName )
    {
        GLint nBinaryLength = 0;
        GLenum nBinaryFormat = GL_NONE;

        glGetProgramiv( nProgramID, GL_PROGRAM_BINARY_LENGTH, &nBinaryLength );
        if( nBinaryLength <= 0 )
        {
            SAL_WARN( "vcl.opengl", "Binary size is zero" );
            return;
        }

        std::vector<sal_uInt8> aBinary( nBinaryLength + GLenumSize );

        glGetProgramBinary( nProgramID, nBinaryLength, nullptr, &nBinaryFormat, aBinary.data() );

        const sal_uInt8* pBF = reinterpret_cast<const sal_uInt8*>(&nBinaryFormat);
        aBinary.insert( aBinary.end(), pBF, pBF + GLenumSize );

        SAL_INFO("vcl.opengl", "Program id: " << nProgramID );
        SAL_INFO("vcl.opengl", "Binary length: " << nBinaryLength );
        SAL_INFO("vcl.opengl", "Binary format: " << nBinaryFormat );

        if( !writeProgramBinary( rBinaryFileName, aBinary ) )
            SAL_WARN("vcl.opengl", "Writing binary file '" << rBinaryFileName << "': FAIL");
        else
            SAL_INFO("vcl.opengl", "Writing binary file '" << rBinaryFileName << "': success");
    }
}

OString OpenGLHelper::GetDigest( const OUString& rVertexShaderName,
                                      const OUString& rFragmentShaderName,
                                      std::string_view rPreamble )
{
    return getStringDigest(rVertexShaderName, rFragmentShaderName, rPreamble);
}

GLint OpenGLHelper::LoadShaders(const OUString& rVertexShaderName,
                                const OUString& rFragmentShaderName,
                                const OUString& rGeometryShaderName,
                                std::string_view preamble,
                                std::string_view rDigest)
{
    OpenGLZone aZone;

    gbInShaderCompile = true;

    bool bHasGeometryShader = !rGeometryShaderName.isEmpty();

    // create the program object
    GLint ProgramID = glCreateProgram();

    // read shaders from file
    OString aVertexShaderSource = getShaderSource(rVertexShaderName);
    OString aFragmentShaderSource = getShaderSource(rFragmentShaderName);
    OString aGeometryShaderSource;
    if (bHasGeometryShader)
        aGeometryShaderSource = getShaderSource(rGeometryShaderName);

    GLint bBinaryResult = GL_FALSE;
    if (epoxy_has_gl_extension("GL_ARB_get_program_binary") && !rDigest.empty())
    {
        OString aFileName =
                createFileName(rVertexShaderName, rFragmentShaderName, rGeometryShaderName, rDigest);
        bBinaryResult = loadProgramBinary(ProgramID, aFileName);
        CHECK_GL_ERROR();
    }

    if( bBinaryResult != GL_FALSE )
        return ProgramID;

    if (bHasGeometryShader)
        VCL_GL_INFO("Load shader: vertex " << rVertexShaderName << " fragment " << rFragmentShaderName << " geometry " << rGeometryShaderName);
    else
        VCL_GL_INFO("Load shader: vertex " << rVertexShaderName << " fragment " << rFragmentShaderName);
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint GeometryShaderID = 0;
    if (bHasGeometryShader)
        GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

    GLint Result = GL_FALSE;

    // Compile Vertex Shader
    if( !preamble.empty())
        addPreamble( aVertexShaderSource, preamble );
    char const * VertexSourcePointer = aVertexShaderSource.getStr();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    if (!Result)
        return LogCompilerError(VertexShaderID, "vertex",
                                rVertexShaderName, true);

    // Compile Fragment Shader
    if( !preamble.empty())
        addPreamble( aFragmentShaderSource, preamble );
    char const * FragmentSourcePointer = aFragmentShaderSource.getStr();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    if (!Result)
        return LogCompilerError(FragmentShaderID, "fragment",
                                rFragmentShaderName, true);

    if (bHasGeometryShader)
    {
        // Compile Geometry Shader
        if( !preamble.empty())
            addPreamble( aGeometryShaderSource, preamble );
        char const * GeometrySourcePointer = aGeometryShaderSource.getStr();
        glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , nullptr);
        glCompileShader(GeometryShaderID);

        // Check Geometry Shader
        glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
        if (!Result)
            return LogCompilerError(GeometryShaderID, "geometry",
                                    rGeometryShaderName, true);
    }

    // Link the program
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    if (bHasGeometryShader)
        glAttachShader(ProgramID, GeometryShaderID);

    if (epoxy_has_gl_extension("GL_ARB_get_program_binary") && !rDigest.empty())
    {
        glProgramParameteri(ProgramID, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
        glLinkProgram(ProgramID);
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        if (!Result)
        {
            SAL_WARN("vcl.opengl", "linking failed: " << Result );
            return LogCompilerError(ProgramID, "program", "<both>", false);
        }
        OString aFileName =
                createFileName(rVertexShaderName, rFragmentShaderName, rGeometryShaderName, rDigest);
        saveProgramBinary(ProgramID, aFileName);
    }
    else
    {
        glLinkProgram(ProgramID);
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    if (bHasGeometryShader)
        glDeleteShader(GeometryShaderID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if (!Result)
        return LogCompilerError(ProgramID, "program", "<both>", false);

    CHECK_GL_ERROR();

    // Ensure we bump our counts before we leave the shader zone.
    { OpenGLZone aMakeProgress; }
    gbInShaderCompile = false;

    return ProgramID;
}

GLint OpenGLHelper::LoadShaders(const OUString& rVertexShaderName,
                                const OUString& rFragmentShaderName,
                                std::string_view preamble,
                                std::string_view rDigest)
{
    return LoadShaders(rVertexShaderName, rFragmentShaderName, OUString(), preamble, rDigest);
}

GLint OpenGLHelper::LoadShaders(const OUString& rVertexShaderName,
                                const OUString& rFragmentShaderName,
                                const OUString& rGeometryShaderName)
{
    return LoadShaders(rVertexShaderName, rFragmentShaderName, rGeometryShaderName, std::string_view(), std::string_view());
}

GLint OpenGLHelper::LoadShaders(const OUString& rVertexShaderName,
                                const OUString& rFragmentShaderName)
{
    return LoadShaders(rVertexShaderName, rFragmentShaderName, OUString(), "", "");
}

void OpenGLHelper::renderToFile(tools::Long nWidth, tools::Long nHeight, const OUString& rFileName)
{
    OpenGLZone aZone;

    std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nWidth*nHeight*4]);
    glReadPixels(0, 0, nWidth, nHeight, OptimalBufferFormat(), GL_UNSIGNED_BYTE, pBuffer.get());
    BitmapEx aBitmap = ConvertBufferToBitmapEx(pBuffer.get(), nWidth, nHeight);
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

GLenum OpenGLHelper::OptimalBufferFormat()
{
#ifdef _WIN32
    return GL_BGRA; // OpenGLSalBitmap is internally ScanlineFormat::N24BitTcBgr
#else
    return GL_RGBA; // OpenGLSalBitmap is internally ScanlineFormat::N24BitTcRgb
#endif
}

BitmapEx OpenGLHelper::ConvertBufferToBitmapEx(const sal_uInt8* const pBuffer, tools::Long nWidth, tools::Long nHeight)
{
    assert(pBuffer);
    Bitmap aBitmap(Size(nWidth, nHeight), vcl::PixelFormat::N24_BPP);
    AlphaMask aAlpha(Size(nWidth, nHeight));

    {
        BitmapScopedWriteAccess pWriteAccess( aBitmap );
        AlphaScopedWriteAccess pAlphaWriteAccess( aAlpha );
#ifdef _WIN32
        assert(pWriteAccess->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr);
        assert(pWriteAccess->IsTopDown());
        assert(pAlphaWriteAccess->IsTopDown());
#else
        assert(pWriteAccess->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb);
        assert(!pWriteAccess->IsTopDown());
        assert(!pAlphaWriteAccess->IsTopDown());
#endif
        assert(pAlphaWriteAccess->GetScanlineFormat() == ScanlineFormat::N8BitPal);

        size_t nCurPos = 0;
        for( tools::Long y = 0; y < nHeight; ++y)
        {
#ifdef _WIN32
            Scanline pScan = pWriteAccess->GetScanline(y);
            Scanline pAlphaScan = pAlphaWriteAccess->GetScanline(y);
#else
            Scanline pScan = pWriteAccess->GetScanline(nHeight-1-y);
            Scanline pAlphaScan = pAlphaWriteAccess->GetScanline(nHeight-1-y);
#endif
            for( tools::Long x = 0; x < nWidth; ++x )
            {
                *pScan++ = pBuffer[nCurPos];
                *pScan++ = pBuffer[nCurPos+1];
                *pScan++ = pBuffer[nCurPos+2];

                nCurPos += 3;
                *pAlphaScan++ = pBuffer[nCurPos++];
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

        {0, nullptr }
    };

    int i;

    for (i=0; errors[i].string; i++)
    {
        if (errors[i].code == errorCode)
        {
            return errors[i].string;
        }
    }

    return nullptr;
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

void OpenGLHelper::createFramebuffer(tools::Long nWidth, tools::Long nHeight, GLuint& nFramebufferId,
        GLuint& nRenderbufferDepthId, GLuint& nRenderbufferColorId)
{
    OpenGLZone aZone;

    // create a renderbuffer for depth attachment
    glGenRenderbuffers(1, &nRenderbufferDepthId);
    glBindRenderbuffer(GL_RENDERBUFFER, nRenderbufferDepthId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nWidth, nHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenTextures(1, &nRenderbufferColorId);
    glBindTexture(GL_TEXTURE_2D, nRenderbufferColorId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, nRenderbufferColorId, 0);

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

    int nErrors = 0;
    for (;;)
    {
        GLenum glErr = glGetError();
        if (glErr == GL_NO_ERROR)
        {
            break;
        }
        const char* sError = OpenGLHelper::GLErrorString(glErr);
        if (!sError)
            sError = "no message available";

        SAL_WARN("vcl.opengl", "GL Error " << std::hex << std::setw(4) << std::setfill('0') << glErr << std::dec << std::setw(0) << std::setfill(' ') << " (" << sError << ") in file " << pFile << " at line " << nLine);

        // tdf#93798 - apitrace appears to sometimes cause issues with an infinite loop here.
        if (++nErrors >= 8)
        {
            SAL_WARN("vcl.opengl", "Breaking potentially recursive glGetError loop");
            break;
        }
    }
}

bool OpenGLHelper::isDeviceDenylisted()
{
    static bool bSet = false;
    static bool bDenylisted = true; // assume the worst
    if (!bSet)
    {
        OpenGLZone aZone;

#if USING_X11
        X11OpenGLDeviceInfo aInfo;
        bDenylisted = aInfo.isDeviceBlocked();
        SAL_INFO("vcl.opengl", "denylisted: " << bDenylisted);
#elif defined( _WIN32 )
        WinOpenGLDeviceInfo aInfo;
        bDenylisted = aInfo.isDeviceBlocked();

        if (DriverBlocklist::GetWindowsVersion() == 0x00060001 && /* Windows 7 */
            (aInfo.GetAdapterVendorID() == "0x1002" || aInfo.GetAdapterVendorID() == "0x1022")) /* AMD */
        {
            SAL_INFO("vcl.opengl", "Relaxing watchdog timings.");
            OpenGLZone::relaxWatchdogTimings();
        }
#else
        bDenylisted = false;
#endif
        bSet = true;
    }

    return bDenylisted;
}

bool OpenGLHelper::supportsOpenGL()
{
    if( getenv("SAL_DISABLEGL") != nullptr )
        return false;
    if (!ImplGetSVData()->mpDefInst->supportsOpenGL())
        return false;
    if( isDeviceDenylisted())
        return false;
    if( officecfg::Office::Common::VCL::DisableOpenGL::get())
        return false;
    WatchdogThread::start();
    return true;
}

namespace
{

enum class CrashWatchdogTimingMode
{
    NORMAL,
    SHADER_COMPILE
};

class CrashWatchdogTimings
{
private:
    std::vector<CrashWatchdogTimingsValues> maTimingValues;
    std::atomic<bool> mbRelaxed;

public:
    CrashWatchdogTimings();

    void setRelax(bool bRelaxed)
    {
        mbRelaxed = bRelaxed;
    }

    CrashWatchdogTimingsValues const & getWatchdogTimingsValues(CrashWatchdogTimingMode eMode)
    {
        size_t index = (eMode == CrashWatchdogTimingMode::SHADER_COMPILE) ? 1 : 0;
        index = mbRelaxed ? index + 2 : index;

        return maTimingValues[index];
    }
};

CrashWatchdogTimings gWatchdogTimings;

CrashWatchdogTimings::CrashWatchdogTimings()
    : maTimingValues{
                     {{6,   20} /* 1.5s,  5s */, {20, 120} /*  5s, 30s */,
                      {60, 240} /*  15s, 60s */, {60, 240} /* 15s, 60s */}
                    }
    , mbRelaxed(false)
{
}

} // namespace

/**
 * Called from a signal handler or watchdog thread if we get
 * a crash or hang in some GL code.
 */
void OpenGLZone::hardDisable()
{
    // protect ourselves from double calling etc.
    static bool bDisabled = false;
    if (bDisabled)
        return;

    bDisabled = true;

    // Disable the OpenGL support
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::VCL::DisableOpenGL::set(true, xChanges);
    xChanges->commit();

    // Force synchronous config write
    css::uno::Reference< css::util::XFlushable >(
        css::configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext()),
        css::uno::UNO_QUERY_THROW)->flush();
}

void OpenGLZone::relaxWatchdogTimings()
{
    gWatchdogTimings.setRelax(true);
}

void OpenGLZone::checkDebug( int nUnchanged, const CrashWatchdogTimingsValues& aTimingValues )
{
    SAL_INFO("vcl.watchdog", "GL watchdog - unchanged "
                                 << nUnchanged << " enter count " << enterCount() << " type "
                                 << (gbInShaderCompile ? "in shader" : "normal gl")
                                 << " breakpoints mid: " << aTimingValues.mnDisableEntries
                                 << " max " << aTimingValues.mnAbortAfter);
}

const CrashWatchdogTimingsValues& OpenGLZone::getCrashWatchdogTimingsValues()
{
    // The shader compiler can take a long time, first time.
    CrashWatchdogTimingMode eMode = gbInShaderCompile ? CrashWatchdogTimingMode::SHADER_COMPILE : CrashWatchdogTimingMode::NORMAL;
    return gWatchdogTimings.getWatchdogTimingsValues(eMode);
}

void OpenGLHelper::debugMsgStream(std::ostringstream const &pStream)
{
    debugMsgPrint(
        0, "%" SAL_PRIxUINT32 ": %s", osl_getThreadIdentifier(nullptr), pStream.str().c_str());
}

void OpenGLHelper::debugMsgStreamWarn(std::ostringstream const &pStream)
{
    debugMsgPrint(
        1, "%" SAL_PRIxUINT32 ": %s", osl_getThreadIdentifier(nullptr), pStream.str().c_str());
}

void OpenGLHelper::debugMsgPrint(const int nType, const char *pFormat, ...)
{
    va_list aArgs;
    va_start (aArgs, pFormat);

    char pStr[1044];
#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif
    vsnprintf(pStr, sizeof(pStr), pFormat, aArgs);
    pStr[sizeof(pStr)-20] = '\0';

    bool bHasContext = OpenGLContext::hasCurrent();
    if (!bHasContext)
        strcat(pStr, " (no GL context)");

    if (nType == 0)
    {
        SAL_INFO("vcl.opengl", pStr);
    }
    else if (nType == 1)
    {
        SAL_WARN("vcl.opengl", pStr);
    }

    if (bHasContext)
    {
        OpenGLZone aZone;

        if (epoxy_has_gl_extension("GL_KHR_debug"))
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION,
                                 GL_DEBUG_TYPE_OTHER,
                                 1, // one[sic] id is as good as another ?
                                 // GL_DEBUG_SEVERITY_NOTIFICATION for >= GL4.3 ?
                                 GL_DEBUG_SEVERITY_LOW,
                                 strlen(pStr), pStr);
        else if (epoxy_has_gl_extension("GL_AMD_debug_output"))
            glDebugMessageInsertAMD(GL_DEBUG_CATEGORY_APPLICATION_AMD,
                                    GL_DEBUG_SEVERITY_LOW_AMD,
                                    1, // one[sic] id is as good as another ?
                                    strlen(pStr), pStr);
    }

    va_end (aArgs);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
