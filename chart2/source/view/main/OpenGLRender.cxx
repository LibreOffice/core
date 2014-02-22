/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <GL/glew.h>
#include <vector>
#include "OpenGLRender.hxx"
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <editeng/unoprnms.hxx>
#include <vcl/virdev.hxx>
#include <vcl/dibtools.hxx>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>

#include <boost/scoped_array.hpp>

using namespace com::sun::star;

using namespace std;

#define DEBUG_PNG 0
#if RENDER_TO_FILE
#define BMP_HEADER_LEN 54
#endif

#if DEBUG_PNG
#include <vcl/pngwrite.hxx>
#endif

#define GL_PI 3.14159f

#if defined( _WIN32 )
#define WGL_SAMPLE_BUFFERS_ARB   0x2041
#define WGL_SAMPLES_ARB          0x2042
#endif

#define Z_STEP 0.001f

int static checkGLError(const char *file, int line)
{
    GLenum glErr;
    int retCode = 0;
    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        const GLubyte* sError = gluErrorString(glErr);

        if (sError)
            SAL_WARN("chart2.opengl", "GL Error #" << glErr << "(" << gluErrorString(glErr) << ") " << " in File " << file << " at line: " << line);
        else
            SAL_WARN("chart2.opengl", "GL Error #" << glErr << " (no message available)" << " in File " << file << " at line: " << line);

        retCode = -1;
        return retCode;
    }
    return retCode;
}

static bool bGlewInit = false;

#define CHECK_GL_ERROR() checkGLError(__FILE__, __LINE__)

#define CHECK_GL_FRAME_BUFFER_STATUS() \
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);\
    if( status != GL_FRAMEBUFFER_COMPLETE ) {\
        SAL_WARN("chart2.opengl", "OpenGL error: " << status );\
        return -1;\
    }

namespace {

OUString getShaderFolder()
{
    OUString aUrl("$BRAND_BASE_DIR/" LIBO_ETC_FOLDER);
    rtl::Bootstrap::expandMacros(aUrl);

    return aUrl + "/opengl/";
}

OUString maShaderFolder = getShaderFolder();

OString loadShader(const OUString& rFilename)
{
    OUString aFileURL = maShaderFolder + rFilename +".glsl";
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
        SAL_WARN("chart2.opengl", "could not load the file: " << aFileURL);
    }

    return OString();
}

}

GLint OpenGLRender::LoadShaders(const OUString& rVertexShaderName,const OUString& rFragmentShaderName)
{
    
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint Result = GL_FALSE;
    int InfoLogLength;

    
    OString aVertexShaderSource = loadShader(rVertexShaderName);
    char const * VertexSourcePointer = aVertexShaderSource.getStr();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    if ( !Result )
    {
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 )
        {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            VertexShaderErrorMessage.push_back('\0');
            SAL_WARN("chart2.opengl", "vertex shader compile failed : " << &VertexShaderErrorMessage[0]);
        }
        else
            SAL_WARN("chart2.opengl", "vertex shader compile failed without error log");

        return 0;
    }

    
    OString aFragmentShaderSource = loadShader(rFragmentShaderName);
    char const * FragmentSourcePointer = aFragmentShaderSource.getStr();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    if ( !Result )
    {
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 )
        {
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            FragmentShaderErrorMessage.push_back('\0');
            SAL_WARN("chart2.opengl", "fragment shader compile failed : " << &FragmentShaderErrorMessage[0]);
        }
        else
            SAL_WARN("chart2.opengl", "fragment shader compile failed without error log");


        return 0;
    }

    
    GLint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if ( !Result )
    {
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 )
        {
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            ProgramErrorMessage.push_back('\0');
            SAL_WARN("chart2.opengl", "Shader Program failed : " << &ProgramErrorMessage[0]);
        }
        else
            SAL_WARN("chart2.opengl", "shader program link failed without error log");

        return 0;
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

namespace {

GLfloat texCoords[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

}

int OpenGLRender::InitOpenGL(GLWindow aWindow)
{
    glWin = aWindow;
    if(!bGlewInit)
    {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            SAL_WARN("chart2.opengl", "Failed to initialize GLEW");
            return -1;
        }
        else
            bGlewInit = true;
    }

    
    if (glewIsSupported("framebuffer_object") != GLEW_OK)
    {
        SAL_WARN("chart2.opengl", "GL stack has no framebuffer support");
        return -1;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glEnable(GL_DEPTH_TEST);
    
    glDepthFunc(GL_LESS);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor (1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    m_Projection = glm::ortho(0.f, float(m_iWidth), 0.f, float(m_iHeight), -1.f, 1.f);
    m_View       = glm::lookAt(glm::vec3(0,0,1), 
                               glm::vec3(0,0,0), 
                               glm::vec3(0,1,0)  
                               );
    m_MVP = m_Projection * m_View * m_Model;
    glGenBuffers(1, &m_VertexBuffer);
    glGenBuffers(1, &m_ColorBuffer);

    CHECK_GL_ERROR();

    m_CommonProID = LoadShaders("commonVertexShader", "commonFragmentShader");
    m_MatrixID = glGetUniformLocation(m_CommonProID, "MVP");
    m_2DVertexID = glGetAttribLocation(m_CommonProID, "vPosition");
    m_2DColorID = glGetUniformLocation(m_CommonProID, "vColor");
    CHECK_GL_ERROR();

#if DEBUG_POSITIONING
    m_DebugProID = LoadShaders("debugVertexShader", "debugFragmentShader");
    m_DebugVertexID = glGetAttribLocation(m_DebugProID, "vPosition");
    CHECK_GL_ERROR();
#endif

    m_BackgroundProID = LoadShaders("backgroundVertexShader", "backgroundFragmentShader");
    m_BackgroundMatrixID = glGetUniformLocation(m_BackgroundProID, "MVP");
    m_BackgroundVertexID = glGetAttribLocation(m_BackgroundProID, "vPosition");
    m_BackgroundColorID = glGetAttribLocation(m_BackgroundProID, "vColor");

    CHECK_GL_ERROR();

    m_SymbolProID = LoadShaders("symbolVertexShader", "symbolFragmentShader");
    m_SymbolVertexID = glGetAttribLocation(m_SymbolProID, "vPosition");
    m_SymbolMatrixID = glGetUniformLocation(m_SymbolProID, "MVP");
    m_SymbolColorID = glGetUniformLocation(m_SymbolProID, "vColor");
    m_SymbolShapeID = glGetUniformLocation(m_SymbolProID, "shape");

    CHECK_GL_ERROR();

    m_TextProID = LoadShaders("textVertexShader", "textFragmentShader");
    m_TextMatrixID = glGetUniformLocation(m_TextProID, "MVP");
    m_TextVertexID = glGetAttribLocation(m_TextProID, "vPosition");
    m_TextTexCoordID = glGetAttribLocation(m_TextProID, "texCoord");
    m_TextTexID = glGetUniformLocation(m_TextProID, "TextTex");

    CHECK_GL_ERROR();

    glGenBuffers(1, &m_TextTexCoordBuf);
    glBindBuffer(GL_ARRAY_BUFFER, m_TextTexCoordBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

#if defined( WNT )
    SwapBuffers(glWin.hDC);
#elif defined( MACOSX )

#elif defined( UNX )
    glXSwapBuffers(glWin.dpy, glWin.win);
#endif
    glFlush();
    glEnable(GL_LIGHTING);
    GLfloat light_direction[] = { 0.0 , 0.0 , 1.0 };
    GLfloat materialDiffuse[] = { 1.0 , 1.0 , 1.0 , 1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    return 0;
}

BitmapEx OpenGLRender::GetAsBitmap()
{
    boost::scoped_array<sal_uInt8> buf(new sal_uInt8[m_iWidth * m_iHeight * 4]);
    glReadPixels(0, 0, m_iWidth, m_iHeight, GL_BGRA, GL_UNSIGNED_BYTE, buf.get());

    Bitmap aBitmap( Size(m_iWidth, m_iHeight), 24 );
    AlphaMask aAlpha( Size(m_iWidth, m_iHeight) );

    {
        Bitmap::ScopedWriteAccess pWriteAccess( aBitmap );
        AlphaMask::ScopedWriteAccess pAlphaWriteAccess( aAlpha );

        size_t nCurPos = 0;
        for( int y = 0; y < m_iHeight; ++y)
        {
            Scanline pScan = pWriteAccess->GetScanline(y);
            Scanline pAlphaScan = pAlphaWriteAccess->GetScanline(y);
            for( int x = 0; x < m_iWidth; ++x )
            {
                *pScan++ = buf[nCurPos];
                *pScan++ = buf[nCurPos+1];
                *pScan++ = buf[nCurPos+2];

                nCurPos += 3;
                *pAlphaScan++ = static_cast<sal_uInt8>( 255 - buf[nCurPos++] );
            }
        }
    }

    BitmapEx aBmp(aBitmap, aAlpha);

#if DEBUG_PNG 
    static int nIdx = 0;
    OUString aName = OUString( "file:
    try {
        vcl::PNGWriter aWriter( aBmp );
        SvFileStream sOutput( aName, STREAM_WRITE );
        aWriter.Write( sOutput );
        sOutput.Close();
    } catch (...) {
        SAL_WARN("chart2.opengl", "Error writing png to " << aName);
    }
#endif

    return aBmp;
}

int OpenGLRender::SetLine2DShapePoint(float x, float y, int listLength)
{
    if (m_Line2DPointList.empty())
    {
        m_Line2DPointList.reserve(listLength*3);
    }
    float actualX = (x / OPENGL_SCALE_VALUE);
    float actualY = (y / OPENGL_SCALE_VALUE);
    m_Line2DPointList.push_back(actualX);
    m_Line2DPointList.push_back(actualY);
    m_Line2DPointList.push_back(m_fZStep);

    if (m_Line2DPointList.size() == size_t(listLength * 3))
    {
        m_Line2DShapePointList.push_back(m_Line2DPointList);
        m_Line2DPointList.clear();
    }
    return 0;
}

int OpenGLRender::RenderLine2FBO(int)
{
    CHECK_GL_ERROR();
    glLineWidth(m_fLineWidth);
    size_t listNum = m_Line2DShapePointList.size();
    PosVecf3 trans = {0.0f, 0.0f, 0.0f};
    PosVecf3 angle = {0.0f, 0.0f, 0.0f};
    PosVecf3 scale = {1.0f, 1.0f, 1.0f};
    MoveModelf(trans, angle, scale);
    m_MVP = m_Projection * m_View * m_Model;
    for (size_t i = 0; i < listNum; i++)
    {
        Line2DPointList &pointList = m_Line2DShapePointList.front();
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        CHECK_GL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, pointList.size() * sizeof(float), &pointList[0], GL_STATIC_DRAW);
        CHECK_GL_ERROR();
        
        glUseProgram(m_CommonProID);
        CHECK_GL_ERROR();

        glUniform4fv(m_2DColorID, 1, &m_2DColor[0]);
        CHECK_GL_ERROR();
        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);
        

        
        CHECK_GL_ERROR();
        glVertexAttribPointer(
            m_2DVertexID,
            3,                  
            GL_FLOAT,           
            GL_FALSE,           
            0,                  
            (void*)0            
            );
        glEnableVertexAttribArray(m_2DVertexID);
        glDrawArrays(GL_LINE_STRIP, 0, pointList.size()/3); 
        CHECK_GL_ERROR();
        glUseProgram(0);
        glDisableVertexAttribArray(m_2DVertexID);
        CHECK_GL_ERROR();
        m_Line2DShapePointList.pop_front();
    }
    GLenum status;
    CHECK_GL_ERROR();
    CHECK_GL_FRAME_BUFFER_STATUS();
    m_fZStep += Z_STEP;
    return 0;
}

#if DEBUG_POSITIONING
void OpenGLRender::renderDebug()
{
    CHECK_GL_ERROR();

    GLfloat vertices[4][3] = {
        {-0.9, -0.9, 0 },
        {-0.6, -0.2, 0 },
        {0.3, 0.3, 0 },
        {0.9, 0.9, 0 } };

    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    CHECK_GL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glUseProgram(m_DebugProID);
    CHECK_GL_ERROR();
    glVertexAttribPointer(m_DebugVertexID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    CHECK_GL_ERROR();
    glEnableVertexAttribArray(m_DebugVertexID);

    glDrawArrays(GL_LINE_STRIP, 0, 3);
    CHECK_GL_ERROR();
    glDisableVertexAttribArray(m_DebugVertexID);

    CHECK_GL_ERROR();
}
#endif

void OpenGLRender::prepareToRender()
{
    glViewport(0, 0, m_iWidth, m_iHeight);
    if (!m_FboID)
    {
        
        CreateTextureObj(m_iWidth, m_iHeight);
        
        CreateRenderObj(m_iWidth, m_iHeight);
        
        CreateFrameBufferObj();
        if (mbArbMultisampleSupported)
        {
            CreateMultiSampleFrameBufObj();
        }
    }
    
    if (mbArbMultisampleSupported)
    {
        glBindFramebuffer(GL_FRAMEBUFFER,m_frameBufferMS);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);
    }

    
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_fZStep = 0;
}

void OpenGLRender::renderToBitmap()
{
    if (mbArbMultisampleSupported)
    {
        GLenum status;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBufferMS);
        status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            SAL_INFO("chart2.opengl", "The frame buffer status is not complete!");
        }
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FboID);
        status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            SAL_INFO("chart2.opengl", "The frame buffer status is not complete!");
        }
        glBlitFramebuffer(0, 0 ,m_iWidth, m_iHeight, 0, 0,m_iWidth ,m_iHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);

#if RENDER_TO_FILE
    char fileName[256] = {0};
    sprintf(fileName, "D:\\shaderout_%d_%d.bmp", m_iWidth, m_iHeight);
    sal_uInt8 *buf = (sal_uInt8 *)malloc(m_iWidth * m_iHeight * 3 + BMP_HEADER_LEN);
    CreateBMPHeader(buf, m_iWidth, m_iHeight);
    glReadPixels(0, 0, m_iWidth, m_iHeight, GL_BGR, GL_UNSIGNED_BYTE, buf + BMP_HEADER_LEN);
    FILE *pfile = fopen(fileName,"wb");
    fwrite(buf,m_iWidth * m_iHeight * 3 + BMP_HEADER_LEN, 1, pfile);
    free(buf);
    fclose(pfile);
#else
    Graphic aGraphic( GetAsBitmap() );
    uno::Reference< awt::XBitmap> xBmp( aGraphic.GetXGraphic(), uno::UNO_QUERY );
    uno::Reference < beans::XPropertySet > xPropSet ( mxRenderTarget, uno::UNO_QUERY );
    xPropSet->setPropertyValue("Graphic", uno::makeAny(aGraphic.GetXGraphic()));
    mxRenderTarget->setSize(awt::Size(m_iWidth*OPENGL_SCALE_VALUE, m_iHeight*OPENGL_SCALE_VALUE));
    mxRenderTarget->setPosition(awt::Point(0,0));
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if defined( WNT )
    SwapBuffers(glWin.hDC);
#elif defined( MACOSX )

#elif defined( UNX )
    glXSwapBuffers(glWin.dpy, glWin.win);
#endif
    glFlush();
}

int OpenGLRender::CreateTextureObj(int width, int height)
{
    glGenTextures(1, &m_TextureObj);
    glBindTexture(GL_TEXTURE_2D, m_TextureObj);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
}

int OpenGLRender::CreateRenderObj(int width, int height)
{
    glGenRenderbuffers(1, &m_RboID);
    CHECK_GL_ERROR();
    glBindRenderbuffer(GL_RENDERBUFFER, m_RboID);
    CHECK_GL_ERROR();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    CHECK_GL_ERROR();
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK_GL_ERROR();
    return 0;
}

int OpenGLRender::MoveModelf(PosVecf3 trans, PosVecf3 angle, PosVecf3 scale)
{
    glm::mat4 aTranslationMatrix = glm::translate(glm::vec3(trans.x, trans.y, trans.z));
    glm::mat4 aScaleMatrix = glm::scale(glm::vec3(scale.x, scale.y, scale.z));
    glm::mat4 aRotationMatrix = glm::eulerAngleYXZ(angle.y, angle.x, angle.z);
    m_Model = aTranslationMatrix * aRotationMatrix * aScaleMatrix;
    return 0;
}

int OpenGLRender::CreateFrameBufferObj()
{
    GLenum status;
    
    glGenFramebuffers(1, &m_FboID);
    CHECK_GL_FRAME_BUFFER_STATUS();
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);
    glBindTexture(GL_TEXTURE_2D, m_TextureObj);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureObj, 0);
    CHECK_GL_FRAME_BUFFER_STATUS();
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, m_RboID);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RboID);
    CHECK_GL_FRAME_BUFFER_STATUS();
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}

void OpenGLRender::Release()
{
    glDeleteBuffers(1, &m_VertexBuffer);
    glDeleteBuffers(1, &m_ColorBuffer);
    glDeleteBuffers(1, &m_TextTexCoordBuf);
    glDeleteProgram(m_CommonProID);
    glDeleteProgram(m_TextProID);
    glDeleteProgram(m_BackgroundProID);
    glDeleteProgram(m_SymbolProID);
    glDeleteFramebuffers(1, &m_FboID);
    glDeleteFramebuffers(1, &m_frameBufferMS);
    glDeleteTextures(1, &m_TextureObj);
    glDeleteRenderbuffers(1, &m_RboID);
    glDeleteRenderbuffers(1, &m_renderBufferColorMS);
    glDeleteRenderbuffers(1, &m_renderBufferDepthMS);
#if defined( WNT )
    wglMakeCurrent(NULL, NULL);
#elif defined( MACOSX )

#elif defined( UNX )

    glXMakeCurrent(glWin.dpy, None, NULL);
    if( glGetError() != GL_NO_ERROR ) {
        SAL_INFO("chart2.opengl", "glError: " << (char *)gluErrorString(glGetError()));
    }
    glXDestroyContext(glWin.dpy, glWin.ctx);
    glWin.ctx = NULL;
    glWin.win = 0;

#endif
}

OpenGLRender::OpenGLRender(uno::Reference< drawing::XShape > xTarget)
    : m_Model(glm::mat4(1.0f))
    , m_TextureObj(0)
    , m_FboID(0)
    , m_RboID(0)
    , m_iWidth(0)
    , m_iHeight(0)
    , m_fLineWidth(0)
    , mxRenderTarget(xTarget)
    , mbArbMultisampleSupported(false)
#if defined( _WIN32 )
    , m_iArbMultisampleFormat(0)
#endif
    , m_2DColor(glm::vec4(1.0, 0.0, 0.0, 1.0))
    , m_frameBufferMS(0)
    , m_renderBufferColorMS(0)
    , m_renderBufferDepthMS(0)
    , m_CommonProID(0)
    , m_2DVertexID(0)
    , m_2DColorID(0)
    , m_fZStep(0)
    , m_TextProID(0)
    , m_TextMatrixID(0)
    , m_TextVertexID(0)
    , m_TextTexCoordID(0)
    , m_TextTexCoordBuf(0)
    , m_TextTexID(0)
    , m_BackgroundProID(0)
    , m_BackgroundMatrixID(0)
    , m_BackgroundVertexID(0)
    , m_BackgroundColorID(0)
    , m_SymbolProID(0)
    , m_SymbolVertexID(0)
    , m_SymbolMatrixID(0)
    , m_SymbolColorID(0)
    , m_SymbolShapeID(0)
{
    
    memset(&m_Bubble2DCircle, 0, sizeof(m_Bubble2DCircle));

    
    for (size_t i = 0; i < sizeof(m_BackgroundColor) / sizeof(float); i++)
    {
        m_BackgroundColor[i] = 1.0;
    }

    mxRenderTarget->setPosition(awt::Point(0,0));
}

OpenGLRender::~OpenGLRender()
{
    Release();
}


void OpenGLRender::SetSize(int width, int height)
{
    m_iWidth = width;
    m_iHeight = height;
    m_Projection = glm::ortho(0.f, float(m_iWidth), 0.f, float(m_iHeight), -4.f, 3.f);
}

#if RENDER_TO_FILE
int OpenGLRender::CreateBMPHeader(sal_uInt8 *bmpHeader, int xsize, int ysize)
{
    unsigned char header[BMP_HEADER_LEN] = {
        0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
        54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0
    };

    long file_size = (long)xsize * (long)ysize * 3 + 54;
    header[2] = (unsigned char)(file_size &0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    long width = xsize;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) &0x000000ff;
    header[20] = (width >> 16) &0x000000ff;
    header[21] = (width >> 24) &0x000000ff;

    long height = -ysize;
    header[22] = height &0x000000ff;
    header[23] = (height >> 8) &0x000000ff;
    header[24] = (height >> 16) &0x000000ff;
    header[25] = (height >> 24) &0x000000ff;
    memcpy(bmpHeader, header, BMP_HEADER_LEN);
    return 0;
}
#endif

void OpenGLRender::SetLine2DColor(sal_uInt8 r, sal_uInt8 g, sal_uInt8 b, sal_uInt8 nAlpha)
{
    m_2DColor = glm::vec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, nAlpha/255.f);
}

void OpenGLRender::SetLine2DWidth(int width)
{
    m_fLineWidth = std::max((float)width / OPENGL_SCALE_VALUE, 0.001f);
}

#if defined( _WIN32 )
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        return 0;
    case WM_KEYDOWN:
        switch(wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;

        case VK_SPACE:
            break;
        }
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

bool OpenGLRender::InitMultisample(PIXELFORMATDESCRIPTOR pfd)
{
    HWND hWnd = NULL;
    
    if (InitTempWindow(&hWnd, m_iWidth, m_iHeight, pfd) < 0)
    {
        SAL_WARN("chart2.opengl", "Can't create temp window to test");
        return false;
    }

    
    if (!WGLisExtensionSupported("WGL_ARB_multisample"))
    {
        mbArbMultisampleSupported = false;
        SAL_WARN("chart2.opengl", "Device doesn't support multi sample");
        return false;
    }
    
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormatARB)
    {
        mbArbMultisampleSupported = false;
        return false;
    }
    
    HDC hDC = GetDC(hWnd);

    int pixelFormat;
    int valid;
    UINT    numFormats;
    float   fAttributes[] = {0,0};
    
    
    
    
    
    int iAttributes[] =
    {
        WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
        WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB,24,
        WGL_ALPHA_BITS_ARB,8,
        WGL_DEPTH_BITS_ARB,16,
        WGL_STENCIL_BITS_ARB,0,
        WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
        WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
        WGL_SAMPLES_ARB,8,
        0,0
    };
    
    valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    
    if (valid && numFormats >= 1)
    {
        mbArbMultisampleSupported = true;
        m_iArbMultisampleFormat = pixelFormat;
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return mbArbMultisampleSupported;
    }
    
    iAttributes[19] = 2;
    valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    if (valid && numFormats >= 1)
    {
        mbArbMultisampleSupported = true;
        m_iArbMultisampleFormat = pixelFormat;
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return mbArbMultisampleSupported;
    }
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(glWin.hRC);
    ReleaseDC(hWnd, glWin.hDC);
    DestroyWindow(hWnd);
    return  mbArbMultisampleSupported;
}

bool OpenGLRender::GetMSAASupport()
{
    return mbArbMultisampleSupported;
}

int OpenGLRender::GetMSAAFormat()
{
    return m_iArbMultisampleFormat;
}


int OpenGLRender::InitTempWindow(HWND *hwnd, int width, int height, PIXELFORMATDESCRIPTOR inPfd)
{
    PIXELFORMATDESCRIPTOR  pfd = inPfd;
    int  pfmt;
    int ret;
    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = wc.cbWndExtra = 0;
    wc.hInstance = NULL;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = (LPCSTR)"GLRenderer";
    RegisterClass(&wc);
    *hwnd = CreateWindow(wc.lpszClassName, NULL, WS_DISABLED, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL);
    glWin.hDC = GetDC(*hwnd);
    pfmt = ChoosePixelFormat(glWin.hDC, &pfd);
    if (!pfmt)
    {
        return -1;
    }
    ret = SetPixelFormat(glWin.hDC, pfmt, &pfd);
    if(!ret)
    {
        return -1;
    }
    glWin.hRC = wglCreateContext(glWin.hDC);
    if(!(glWin.hRC))
    {
        return -1;
    }
    ret = wglMakeCurrent(glWin.hDC, glWin.hRC);
    if(!ret)
    {
        return -1;
    }
    return 0;
}

bool OpenGLRender::WGLisExtensionSupported(const char *extension)
{
    const size_t extlen = strlen(extension);
    const char *supported = NULL;

    
    PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

    if (wglGetExtString)
        supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
    
    if (supported == NULL)
        supported = (char*)glGetString(GL_EXTENSIONS);
    
    if (supported == NULL)
        return 0;

    
    for (const char* p = supported; ; p++)
    {
        
        p = strstr(p, extension);

        if (p == NULL)
            return 0; 

        
        
        

        
        
        if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
            return 1; 
    }
}
#endif

void OpenGLRender::SetColor(sal_uInt32 color, sal_uInt8 nAlpha)
{
    sal_uInt8 r = (color & 0x00FF0000) >> 16;
    sal_uInt8 g = (color & 0x0000FF00) >> 8;
    sal_uInt8 b = (color & 0x000000FF);
    m_2DColor = glm::vec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, nAlpha/ 255.f);
}

int OpenGLRender::CreateMultiSampleFrameBufObj()
{
    glGenFramebuffers(1, &m_frameBufferMS);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferMS);

    glGenRenderbuffers(1, &m_renderBufferColorMS);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferColorMS);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8,  GL_RGB, m_iWidth, m_iHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderBufferColorMS);

    glGenRenderbuffers(1, &m_renderBufferDepthMS);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferDepthMS);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT24, m_iWidth, m_iHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferDepthMS);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}

int OpenGLRender::Create2DCircle(int detail)
{
    float angle;
    if (detail <= 0)
    {
        return -1;
    }
    m_Bubble2DCircle.clear();
    m_Bubble2DCircle.reserve(2 * (detail + 3));
    m_Bubble2DCircle.push_back(0);
    m_Bubble2DCircle.push_back(0);
    for(angle = 2.0f * GL_PI; angle > -(2.0f * GL_PI / detail); angle -= (2.0f * GL_PI / detail))
    {
        m_Bubble2DCircle.push_back(sin(angle));
        m_Bubble2DCircle.push_back(cos(angle));
    }
    return 0;
}

int OpenGLRender::Bubble2DShapePoint(float x, float y, float directionX, float directionY)
{
    
    if (m_Bubble2DCircle.empty())
    {
        Create2DCircle(100);
    }

    float actualX = (x / OPENGL_SCALE_VALUE);
    float actualY = (y / OPENGL_SCALE_VALUE);
    Bubble2DPointList aBubble2DPointList;
    aBubble2DPointList.xScale = directionX / OPENGL_SCALE_VALUE;
    aBubble2DPointList.yScale = directionY / OPENGL_SCALE_VALUE;
    aBubble2DPointList.x = actualX + aBubble2DPointList.xScale / 2;
    aBubble2DPointList.y = actualY + aBubble2DPointList.yScale / 2;

    m_Bubble2DShapePointList.push_back(aBubble2DPointList);
    return 0;
}

int OpenGLRender::RenderBubble2FBO(int)
{
    CHECK_GL_ERROR();
    size_t listNum = m_Bubble2DShapePointList.size();
    for (size_t i = 0; i < listNum; i++)
    {
        
        Bubble2DPointList &pointList = m_Bubble2DShapePointList.front();
        PosVecf3 trans = {pointList.x, pointList.y, m_fZStep};
        PosVecf3 angle = {0.0f, 0.0f, 0.0f};
        PosVecf3 scale = {pointList.xScale, pointList.yScale, 1.0f};
        MoveModelf(trans, angle, scale);
        m_MVP = m_Projection * m_View * m_Model;
        
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        if (m_Bubble2DCircle.empty())
        {
            Create2DCircle(100);
        }
        glBufferData(GL_ARRAY_BUFFER, m_Bubble2DCircle.size() * sizeof(GLfloat), &m_Bubble2DCircle[0], GL_STATIC_DRAW);

        glUseProgram(m_CommonProID);

        glUniform4fv(m_2DColorID, 1, &m_2DColor[0]);

        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);
        
        glEnableVertexAttribArray(m_2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_2DVertexID,                  
            2,                  
            GL_FLOAT,           
            GL_FALSE,           
            0,                  
            (void*)0            
            );
        glDrawArrays(GL_TRIANGLE_FAN, 0, m_Bubble2DCircle.size() / 2);
        glDisableVertexAttribArray(m_2DVertexID);
        glUseProgram(0);
        m_Bubble2DShapePointList.pop_front();
    }
    
    GLenum fbResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( fbResult != GL_FRAMEBUFFER_COMPLETE )
    {
        SAL_WARN("chart2.opengl", "error");
        return -1;
    }
    CHECK_GL_ERROR();
    m_fZStep += Z_STEP;
    return 0;
}

int OpenGLRender::RectangleShapePoint(float x, float y, float directionX, float directionY)
{
    
    float actualX = x / OPENGL_SCALE_VALUE;
    float actualY = y / OPENGL_SCALE_VALUE;
    float actualSizeX = directionX / OPENGL_SCALE_VALUE;
    float actualSizeY = directionY / OPENGL_SCALE_VALUE;
    RectanglePointList aRectangle;

    aRectangle.points[0] = actualX;
    aRectangle.points[1] = actualY;
    aRectangle.points[2] = m_fZStep;
    aRectangle.points[3] = actualX + actualSizeX;
    aRectangle.points[4] = actualY;
    aRectangle.points[5] = m_fZStep;
    aRectangle.points[6] = actualX + actualSizeX;
    aRectangle.points[7] = actualY + actualSizeY;
    aRectangle.points[8] = m_fZStep;
    aRectangle.points[9] = actualX;
    aRectangle.points[10] = actualY + actualSizeY;
    aRectangle.points[11] = m_fZStep;

    m_RectangleShapePointList.push_back(aRectangle);
    return 0;
}


int OpenGLRender::RenderRectangleShape(bool bBorder, bool bFill)
{
    size_t listNum = m_RectangleShapePointList.size();
    for (size_t i = 0; i < listNum; i++)
    {
        
        RectanglePointList &pointList = m_RectangleShapePointList.front();
        {
            PosVecf3 trans = {0, 0, 0};
            PosVecf3 angle = {0.0f, 0.0f, 0.0f};
            PosVecf3 scale = {1, 1, 1.0f};
            MoveModelf(trans, angle, scale);
            m_MVP = m_Projection * m_View * m_Model;
        }

        
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pointList.points), pointList.points, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_BackgroundColor), m_BackgroundColor, GL_STATIC_DRAW);
        glUseProgram(m_BackgroundProID);

        glUniformMatrix4fv(m_BackgroundMatrixID, 1, GL_FALSE, &m_MVP[0][0]);
        if(bFill)
        {
            glEnableVertexAttribArray(m_BackgroundVertexID);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glVertexAttribPointer(
                    m_BackgroundVertexID,                  
                    3,                  
                    GL_FLOAT,           
                    GL_FALSE,           
                    0,                  
                    (void*)0            
                    );

            
            glEnableVertexAttribArray(m_BackgroundColorID);
            glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
            glVertexAttribPointer(
                    m_BackgroundColorID,                  
                    4,                  
                    GL_FLOAT,           
                    GL_FALSE,           
                    0,                  
                    (void*)0            
                    );
            
            glDrawArrays(GL_QUADS, 0, 4);
            glDisableVertexAttribArray(m_BackgroundVertexID);
            glDisableVertexAttribArray(m_BackgroundColorID);
        }
        if(bBorder)
        {
            if(bFill)
            {
                PosVecf3 trans = {0.0, 0.0, Z_STEP };
                PosVecf3 angle = {0.0f, 0.0f, 0.0f};
                PosVecf3 scale = {1, 1, 1.0f};
                MoveModelf(trans, angle, scale);
                m_MVP = m_Projection * m_View * m_Model;

                m_fZStep += Z_STEP;
                glUniformMatrix4fv(m_BackgroundMatrixID, 1, GL_FALSE, &m_MVP[0][0]);
            }
            SetBackGroundColor(COL_BLACK, COL_BLACK, 255);

            glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(m_BackgroundColor), m_BackgroundColor, GL_STATIC_DRAW);
            
            glEnableVertexAttribArray(m_BackgroundVertexID);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glVertexAttribPointer(
                    m_BackgroundVertexID,                  
                    3,                  
                    GL_FLOAT,           
                    GL_FALSE,           
                    0,                  
                    (void*)0            
                    );

            
            glEnableVertexAttribArray(m_BackgroundColorID);
            glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
            glVertexAttribPointer(
                    m_BackgroundColorID,                  
                    4,                  
                    GL_FLOAT,           
                    GL_FALSE,           
                    0,                  
                    (void*)0            
                    );
            glDrawArrays(GL_LINE_LOOP, 0, 4);
            glDisableVertexAttribArray(m_BackgroundVertexID);
            glDisableVertexAttribArray(m_BackgroundColorID);
        }
        glDisableVertexAttribArray(m_BackgroundVertexID);
        glDisableVertexAttribArray(m_BackgroundColorID);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_RectangleShapePointList.pop_front();
    }
    CHECK_GL_ERROR();

    m_fZStep += Z_STEP;
    return 0;
}


int OpenGLRender::CreateTextTexture(const BitmapEx& rBitmapEx, const awt::Point&, const awt::Size& aSize, long rotation,
        const drawing::HomogenMatrix3& rTrans)
{
#if DEBUG_PNG 
    static int nIdx = 0;
    OUString aName = OUString( "file:
    try {
        vcl::PNGWriter aWriter( rBitmapEx );
        SvFileStream sOutput( aName, STREAM_WRITE );
        aWriter.Write( sOutput );
        sOutput.Close();
    } catch (...) {
        SAL_WARN("chart2.opengl", "Error writing png to " << aName);
    }
#endif

    long bmpWidth = rBitmapEx.GetSizePixel().Width();
    long bmpHeight = rBitmapEx.GetSizePixel().Height();

    Bitmap aBitmap (rBitmapEx.GetBitmap());
    AlphaMask aAlpha (rBitmapEx.GetAlpha());
    boost::scoped_array<sal_uInt8> bitmapBuf(new sal_uInt8[4* bmpWidth * bmpHeight ]);
    Bitmap::ScopedReadAccess pReadAccces( aBitmap );
    AlphaMask::ScopedReadAccess pAlphaReadAccess( aAlpha );

    size_t i = 0;
    for (long ny = 0; ny < bmpHeight; ny++)
    {
        Scanline pAScan = pAlphaReadAccess->GetScanline(ny);
        for(long nx = 0; nx < bmpWidth; nx++)
        {
            BitmapColor aCol = pReadAccces->GetColor( ny, nx );
            bitmapBuf[i++] = aCol.GetRed();
            bitmapBuf[i++] = aCol.GetGreen();
            bitmapBuf[i++] = aCol.GetBlue();
            bitmapBuf[i++] = 255 - *pAScan++;
        }
    }

    TextInfo aTextInfo;
    aTextInfo.rotation = -(double)rotation * GL_PI / 18000.0f;
    aTextInfo.vertex[0] = rTrans.Line1.Column3 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[1] = rTrans.Line2.Column3 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[2] = m_fZStep;

    aTextInfo.vertex[3] = (rTrans.Line1.Column3 + aSize.Width ) / OPENGL_SCALE_VALUE ;
    aTextInfo.vertex[4] = rTrans.Line2.Column3 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[5] = m_fZStep;

    aTextInfo.vertex[6] = (rTrans.Line1.Column3 + aSize.Width) / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[7] = (rTrans.Line2.Column3 + aSize.Height) / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[8] = m_fZStep;

    aTextInfo.vertex[9] = rTrans.Line1.Column3 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[10] = (rTrans.Line2.Column3 + aSize.Height) / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[11] = m_fZStep;

    aTextInfo.bmpWidth = bmpWidth;
    aTextInfo.bmpHeight = bmpHeight;

    
    if (!rtl::math::approxEqual(0, rotation))
    {
        
    }

    CHECK_GL_ERROR();
    glGenTextures(1, &aTextInfo.texture);
    CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, aTextInfo.texture);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmpWidth, bmpHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmapBuf.get());
    CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERROR();
    m_TextInfoList.push_back(aTextInfo);
    return 0;
}

int OpenGLRender::RenderTextShape()
{
    CHECK_GL_ERROR();
    size_t listNum = m_TextInfoList.size();
    for (size_t i = 0; i < listNum; i++)
    {
        TextInfo &textInfo = m_TextInfoList.front();
        PosVecf3 trans = { (float)-textInfo.bmpWidth/2.0f, (float)-textInfo.bmpHeight/2.0f, 0};
        PosVecf3 angle = {0.0f, 0.0f, float(textInfo.rotation)};
        PosVecf3 scale = {1.0, 1.0, 1.0f};
        MoveModelf(trans, angle, scale);
        m_MVP = m_Projection * m_View * m_Model;
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        CHECK_GL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, sizeof(textInfo.vertex), textInfo.vertex, GL_STATIC_DRAW);
        CHECK_GL_ERROR();
        glUseProgram(m_TextProID);

        CHECK_GL_ERROR();
        glUniformMatrix4fv(m_TextMatrixID, 1, GL_FALSE, &m_MVP[0][0]);
        
        glEnableVertexAttribArray(m_TextVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_TextVertexID,
            3,                  
            GL_FLOAT,           
            GL_FALSE,           
            0,                  
            (void*)0            
            );
        
        CHECK_GL_ERROR();
        glEnableVertexAttribArray(m_TextTexCoordID);
        glBindBuffer(GL_ARRAY_BUFFER, m_TextTexCoordBuf);
        glVertexAttribPointer(
            m_TextTexCoordID,
            2,                  
            GL_FLOAT,           
            GL_FALSE,           
            0,                  
            (void*)0            
            );
        
        CHECK_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, textInfo.texture);
        CHECK_GL_ERROR();
        glUniform1i(m_TextTexID, 0);
        CHECK_GL_ERROR();
        
        glDrawArrays(GL_QUADS, 0, 4);
        CHECK_GL_ERROR();
        glDisableVertexAttribArray(m_TextTexCoordID);
        CHECK_GL_ERROR();
        glDisableVertexAttribArray(m_TextVertexID);
        CHECK_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        glDeleteTextures(1, &textInfo.texture);
        CHECK_GL_ERROR();
        m_TextInfoList.pop_front();
    }
    CHECK_GL_ERROR();
    m_fZStep += Z_STEP;
    return 0;
}

int OpenGLRender::SetArea2DShapePoint(float x, float y, int listLength)
{
    if (m_Area2DPointList.empty())
    {
        m_Area2DPointList.reserve(listLength);
    }
    float actualX = (x / OPENGL_SCALE_VALUE);
    float actualY = (y / OPENGL_SCALE_VALUE);
    m_Area2DPointList.push_back(actualX);
    m_Area2DPointList.push_back(actualY);
    m_Area2DPointList.push_back(m_fZStep);

    if (m_Area2DPointList.size() == size_t(listLength * 3))
    {
        m_Area2DShapePointList.push_back(m_Area2DPointList);
        m_Area2DPointList.clear();
    }
    return 0;
}

namespace {


bool checkCCW(const Area2DPointList& rPoints)
{
    if(rPoints.size() < 3)
        return true;

    GLfloat sum = 0;
    for(size_t i = 1; i < rPoints.size()/3; i += 3)
    {
        GLfloat x1 = rPoints[(i-1)*3];
        GLfloat x2 = rPoints[i*3];
        GLfloat y1 = rPoints[(i-1)*3 + 1];
        GLfloat y2 = rPoints[i*3 + 1];
        GLfloat prod = (x2-x1)*(y2+y1);

        sum += prod;
    }

    return (sum <= 0);
}

}

int OpenGLRender::RenderArea2DShape()
{
    CHECK_GL_ERROR();

    glDisable(GL_MULTISAMPLE);
    size_t listNum = m_Area2DShapePointList.size();
    PosVecf3 trans = {0.0f, 0.0f, 0.0f};
    PosVecf3 angle = {0.0f, 0.0f, 0.0f};
    PosVecf3 scale = {1.0f, 1.0f, 1.0f};
    MoveModelf(trans, angle, scale);
    m_MVP = m_Projection * m_View * m_Model;
    for (size_t i = 0; i < listNum; ++i)
    {
        Area2DPointList &pointList = m_Area2DShapePointList.front();
        bool bIsCCW = checkCCW(pointList); 
        if(!bIsCCW)
            glFrontFace(GL_CW);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, pointList.size() * sizeof(float), &pointList[0], GL_STATIC_DRAW);
        
        glUseProgram(m_CommonProID);

        glUniform4fv(m_2DColorID, 1, &m_2DColor[0]);

        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);

        
        glEnableVertexAttribArray(m_2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_2DVertexID,                  
            3,                  
            GL_FLOAT,           
            GL_FALSE,           
            0,                  
            (void*)0            
            );
        
        glDrawArrays(GL_POLYGON, 0, pointList.size() / 3); 
        glDisableVertexAttribArray(m_2DVertexID);
        glUseProgram(0);
        if(!bIsCCW)
            glFrontFace(GL_CCW);
        m_Area2DShapePointList.pop_front();
    }
    glEnable(GL_MULTISAMPLE);
    m_fZStep += Z_STEP;

    CHECK_GL_ERROR();

    return 0;
}

void OpenGLRender::SetBackGroundColor(sal_uInt32 color1, sal_uInt32 color2, sal_uInt8 nAlpha)
{
    sal_uInt8 r = (color1 & 0x00FF0000) >> 16;
    sal_uInt8 g = (color1 & 0x0000FF00) >> 8;
    sal_uInt8 b = (color1 & 0x000000FF);

    m_BackgroundColor[0] = (float)r / 255.0f;
    m_BackgroundColor[1] = (float)g / 255.0f;
    m_BackgroundColor[2] = (float)b / 255.0f;
    m_BackgroundColor[3] = nAlpha / 255.0f;

    m_BackgroundColor[4] = (float)r / 255.0f;
    m_BackgroundColor[5] = (float)g / 255.0f;
    m_BackgroundColor[6] = (float)b / 255.0f;
    m_BackgroundColor[7] = nAlpha / 255.0f;

    r = (color2 & 0x00FF0000) >> 16;
    g = (color2 & 0x0000FF00) >> 8;
    b = (color2 & 0x000000FF);

    m_BackgroundColor[8] = (float)r / 255.0f;
    m_BackgroundColor[9] = (float)g / 255.0f;
    m_BackgroundColor[10] = (float)b / 255.0f;
    m_BackgroundColor[11] = nAlpha / 255.0f;

    m_BackgroundColor[12] = (float)r / 255.0f;
    m_BackgroundColor[13] = (float)g / 255.0f;
    m_BackgroundColor[14] = (float)b / 255.0f;
    m_BackgroundColor[15] = nAlpha / 255.0f;
    SAL_INFO("chart2.opengl", "color1 = " << color1 << ", color2 = " << color2);

}

void OpenGLRender::SetChartTransparencyGradient(long transparencyGradient)
{
    if (transparencyGradient == 1)
    {
        m_BackgroundColor[11] = 0.0;
        m_BackgroundColor[15] = 0.0;
    }
}

void OpenGLRender::GeneratePieSegment2D(double fInnerRadius, double fOutterRadius, double nAngleStart, double nAngleWidth)
{
    double nAngleStep = 1;
    PieSegment2DPointList aPointList;
    
    bool bInnerRadiusNotZero = true; 
    size_t nVectorSize = 3*(nAngleWidth/nAngleStep);
    if(bInnerRadiusNotZero)
        nVectorSize *= 2;

    nAngleStart += 90;
    aPointList.reserve(nVectorSize);
    
    
    if(!bInnerRadiusNotZero)
    {
        aPointList.push_back(0);
        aPointList.push_back(0);
        aPointList.push_back(m_fZStep);
    }
    for(double nAngle = nAngleStart; nAngle <= nAngleStart + nAngleWidth; nAngle += nAngleStep)
    {
        float xVal = sin(nAngle/360*2*GL_PI);
        float yVal = cos(nAngle/360*2*GL_PI);
        aPointList.push_back(fOutterRadius * xVal);
        aPointList.push_back(fOutterRadius * yVal);
        aPointList.push_back(m_fZStep);

        if(bInnerRadiusNotZero)
        {
            aPointList.push_back(fInnerRadius * xVal);
            aPointList.push_back(fInnerRadius * yVal);
            aPointList.push_back(m_fZStep);
        }
    }

    m_PieSegment2DShapePointList.push_back(aPointList);
}

int OpenGLRender::RenderPieSegment2DShape(float fSize, float fPosX, float fPosY)
{
    int listNum = m_PieSegment2DShapePointList.size();
    PosVecf3 trans = {fPosX/OPENGL_SCALE_VALUE, fPosY/OPENGL_SCALE_VALUE, 0.0f};
    PosVecf3 angle = {0.0f, 0.0f, 0.0f};
    PosVecf3 scale = {fSize/OPENGL_SCALE_VALUE, fSize/OPENGL_SCALE_VALUE, 1.0f};
    MoveModelf(trans, angle, scale);
    m_MVP = m_Projection * m_View * m_Model;

    for (int i = 0; i < listNum; i++)
    {
        PieSegment2DPointList &pointList = m_PieSegment2DShapePointList.back();
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, pointList.size() * sizeof(float), &pointList[0] , GL_STATIC_DRAW);
        
        glUseProgram(m_CommonProID);

        glUniform4fv(m_2DColorID, 1, &m_2DColor[0]);

        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);

        
        glEnableVertexAttribArray(m_2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_2DVertexID,                  
            3,                  
            GL_FLOAT,           
            GL_FALSE,           
            0,                  
            (void*)0            
            );
        glDrawArrays(GL_TRIANGLE_STRIP, 0, pointList.size() / 3); 
        glDisableVertexAttribArray(m_2DVertexID);
        glUseProgram(0);
        m_PieSegment2DShapePointList.pop_back();
        CHECK_GL_ERROR();

    }
    m_fZStep += Z_STEP;

    CHECK_GL_ERROR();
    return 0;
}

int OpenGLRender::RenderSymbol2DShape(float x, float y, float , float , sal_Int32 nSymbol)
{
    CHECK_GL_ERROR();

    glPointSize(20.f);
    CHECK_GL_ERROR();
    PosVecf3 trans = {0.0, 0.0, 0.0};
    PosVecf3 angle = {0.0f, 0.0f, 0.0f};
    PosVecf3 scale = {1.0, 1.0, 1.0f};
    MoveModelf(trans, angle, scale);
    m_MVP = m_Projection * m_View * m_Model;

    float aPos[3] = { x/OPENGL_SCALE_VALUE, y/OPENGL_SCALE_VALUE, m_fZStep };
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    CHECK_GL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), aPos, GL_STATIC_DRAW);
    CHECK_GL_ERROR();

    
    glUseProgram(m_SymbolProID);
    CHECK_GL_ERROR();

    glUniform4fv(m_SymbolColorID, 1, &m_2DColor[0]);
    glUniform1i(m_SymbolShapeID, nSymbol);
    CHECK_GL_ERROR();

    glUniformMatrix4fv(m_SymbolMatrixID, 1, GL_FALSE, &m_MVP[0][0]);

    CHECK_GL_ERROR();
    
    glEnableVertexAttribArray(m_SymbolVertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glVertexAttribPointer(
            m_SymbolVertexID,                  
            3,                  
            GL_FLOAT,           
            GL_FALSE,           
            0,                  
            (void*)0            
            );

    glDrawArrays(GL_POINTS, 0, 1);

    glDisableVertexAttribArray(m_SymbolVertexID);
    CHECK_GL_ERROR();
    glUseProgram(0);
    m_fZStep += Z_STEP;

    CHECK_GL_ERROR();
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
