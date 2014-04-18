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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
