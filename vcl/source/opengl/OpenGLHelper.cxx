/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/opengl/OpenGLHelper.hxx>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>
#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <boost/scoped_array.hpp>
#include <vcl/pngwrite.hxx>
#include <vcl/graph.hxx>

#include <vector>

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
        char* content = new char[nSize+1];
        sal_uInt64 nBytesRead = 0;
        aFile.read(content, nSize, nBytesRead);
        if(nSize != nBytesRead)
            assert(false);

        content[nSize] = 0;
        return OString(content);
    }
    else
    {
        SAL_WARN("vcl.opengl", "could not load the file: " << aFileURL);
    }

    return OString();
}

}

GLint OpenGLHelper::LoadShaders(const OUString& rVertexShaderName,const OUString& rFragmentShaderName)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    OString aVertexShaderSource = loadShader(rVertexShaderName);
    char const * VertexSourcePointer = aVertexShaderSource.getStr();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    if ( !Result )
    {
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 )
        {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            VertexShaderErrorMessage.push_back('\0');
            SAL_WARN("vcl.opengl", "vertex shader compile failed : " << &VertexShaderErrorMessage[0]);
        }
        else
            SAL_WARN("vcl.opengl", "vertex shader compile failed without error log");

        return 0;
    }

    // Compile Fragment Shader
    OString aFragmentShaderSource = loadShader(rFragmentShaderName);
    char const * FragmentSourcePointer = aFragmentShaderSource.getStr();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    if ( !Result )
    {
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 )
        {
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            FragmentShaderErrorMessage.push_back('\0');
            SAL_WARN("vcl.opengl", "fragment shader compile failed : " << &FragmentShaderErrorMessage[0]);
        }
        else
            SAL_WARN("vcl.opengl", "fragment shader compile failed without error log");


        return 0;
    }

    // Link the program
    GLint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if ( !Result )
    {
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 )
        {
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            ProgramErrorMessage.push_back('\0');
            SAL_WARN("vcl.opengl", "Shader Program failed : " << &ProgramErrorMessage[0]);
        }
        else
            SAL_WARN("vcl.opengl", "shader program link failed without error log");

        return 0;
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

sal_uInt8* OpenGLHelper::ConvertBitmapExToRGBABuffer(const BitmapEx& rBitmapEx)
{
    long nBmpWidth = rBitmapEx.GetSizePixel().Width();
    long nBmpHeight = rBitmapEx.GetSizePixel().Height();

    Bitmap aBitmap (rBitmapEx.GetBitmap());
    AlphaMask aAlpha (rBitmapEx.GetAlpha());
    sal_uInt8* pBitmapBuf(new sal_uInt8[4* nBmpWidth * nBmpHeight ]);
    Bitmap::ScopedReadAccess pReadAccces( aBitmap );
    AlphaMask::ScopedReadAccess pAlphaReadAccess( aAlpha );
    size_t i = 0;
    for (long ny = 0; ny < nBmpHeight; ny++)
    {
        Scanline pAScan = pAlphaReadAccess ? pAlphaReadAccess->GetScanline(ny) : 0;
        for(long nx = 0; nx < nBmpWidth; nx++)
        {
            BitmapColor aCol = pReadAccces->GetColor( ny, nx );
            pBitmapBuf[i++] = aCol.GetRed();
            pBitmapBuf[i++] = aCol.GetGreen();
            pBitmapBuf[i++] = aCol.GetBlue();
            pBitmapBuf[i++] = pAScan ? 255 - *pAScan++ : 255;
        }
    }
    return pBitmapBuf;
}

void OpenGLHelper::renderToFile(long nWidth, long nHeight, const OUString& rFileName)
{
    boost::scoped_array<sal_uInt8> pBuffer(new sal_uInt8[nWidth*nHeight*4]);
    glReadPixels(0, 0, nWidth, nHeight, GL_BGRA, GL_UNSIGNED_BYTE, pBuffer.get());
    BitmapEx aBitmap = ConvertBGRABufferToBitmapEx(pBuffer.get(), nWidth, nHeight);
    try {
        vcl::PNGWriter aWriter( aBitmap );
        SvFileStream sOutput( rFileName, STREAM_WRITE );
        aWriter.Write( sOutput );
        sOutput.Close();
    } catch (...) {
        SAL_WARN("vcl.opengl", "Error writing png to " << rFileName);
    }
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

void OpenGLHelper::createFramebuffer(long nWidth, long nHeight,
        GLuint& nFramebufferId, GLuint& nRenderbufferId, GLuint& nTexturebufferId)
{
    // create a renderbuffer
    glGenRenderbuffers(1, &nRenderbufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, nRenderbufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nWidth, nHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // create a texture
    glGenTextures(1, &nTexturebufferId);
    glBindTexture(GL_TEXTURE_2D, nTexturebufferId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // create a framebuffer object and attach renderbuffer and texture
    glGenFramebuffers(1, &nFramebufferId);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, nFramebufferId);
    glBindTexture(GL_TEXTURE_2D, nTexturebufferId);
    // attach a texture to FBO color attachement point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nTexturebufferId, 0);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindTexture(GL_TEXTURE_2D, 0);
    // attach a renderbuffer to depth attachment point
    glBindRenderbuffer(GL_RENDERBUFFER, nRenderbufferId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, nRenderbufferId);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
