/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GL/glew.h>
#include <vector>
#include "OpenGLRender.hxx"
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
#include <vcl/svapp.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>

#include <boost/scoped_array.hpp>
#include "CommonConverters.hxx"

using namespace com::sun::star;

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

static GLfloat squareVertices[] = {
    -1.0f, -1.0f, -1.0,
    1.0f, -1.0f, -1.0,
    1.0f,  1.0f, -1.0,
    -1.0f,  1.0f, -1.0
};

static GLfloat coordReverseVertices[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
};

int static checkGLError(const char *file, int line)
{
    GLenum glErr;
    int retCode = 0;
    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        const char* sError = OpenGLHelper::GLErrorString(glErr);

        if (sError)
            SAL_WARN("chart2.opengl", "GL Error #" << glErr << "(" << sError << ") " << " in File " << file << " at line: " << line);
        else
            SAL_WARN("chart2.opengl", "GL Error #" << glErr << " (no message available)" << " in File " << file << " at line: " << line);

        retCode = -1;
        return retCode;
    }
    return retCode;
}

#define CHECK_GL_ERROR() checkGLError(__FILE__, __LINE__)

#define CHECK_GL_FRAME_BUFFER_STATUS() \
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);\
    if( status != GL_FRAMEBUFFER_COMPLETE ) {\
        SAL_WARN("chart2.opengl", "OpenGL error: " << status );\
        return -1;\
    }

namespace {

GLfloat texCoords[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

}

int OpenGLRender::InitOpenGL()
{
    //TODO: moggi: get the information from the context
    mbArbMultisampleSupported = true;

    aContext.init();

    if (glewIsSupported("framebuffer_object") != GLEW_OK)
    {
        SAL_WARN("chart2.opengl", "GL stack has no framebuffer support");
        return -1;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
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

    //Init the Projection matrix
    m_Projection = glm::ortho(0.f, float(m_iWidth), 0.f, float(m_iHeight), -1.f, 1.f);
    m_View       = glm::lookAt(glm::vec3(0,0,1), // Camera is at (4,3,-3), in World Space
                               glm::vec3(0,0,0), // and looks at the origin
                               glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                               );
    m_MVP = m_Projection * m_View * m_Model;
    glGenBuffers(1, &m_VertexBuffer);
    glGenBuffers(1, &m_ColorBuffer);

    CHECK_GL_ERROR();

    m_CommonProID = OpenGLHelper::LoadShaders("commonVertexShader", "commonFragmentShader");
    m_MatrixID = glGetUniformLocation(m_CommonProID, "MVP");
    m_2DVertexID = glGetAttribLocation(m_CommonProID, "vPosition");
    m_2DColorID = glGetUniformLocation(m_CommonProID, "vColor");
    CHECK_GL_ERROR();

#if DEBUG_POSITIONING
    m_DebugProID = OpenGLHelper::LoadShaders("debugVertexShader", "debugFragmentShader");
    m_DebugVertexID = glGetAttribLocation(m_DebugProID, "vPosition");
    CHECK_GL_ERROR();
#endif

    m_BackgroundProID = OpenGLHelper::LoadShaders("backgroundVertexShader", "backgroundFragmentShader");
    m_BackgroundMatrixID = glGetUniformLocation(m_BackgroundProID, "MVP");
    m_BackgroundVertexID = glGetAttribLocation(m_BackgroundProID, "vPosition");
    m_BackgroundColorID = glGetAttribLocation(m_BackgroundProID, "vColor");

    CHECK_GL_ERROR();

    m_SymbolProID = OpenGLHelper::LoadShaders("symbolVertexShader", "symbolFragmentShader");
    m_SymbolVertexID = glGetAttribLocation(m_SymbolProID, "vPosition");
    m_SymbolMatrixID = glGetUniformLocation(m_SymbolProID, "MVP");
    m_SymbolColorID = glGetUniformLocation(m_SymbolProID, "vColor");
    m_SymbolShapeID = glGetUniformLocation(m_SymbolProID, "shape");

    CHECK_GL_ERROR();

    m_TextProID = OpenGLHelper::LoadShaders("textVertexShader", "textFragmentShader");
    m_TextMatrixID = glGetUniformLocation(m_TextProID, "MVP");
    m_TextVertexID = glGetAttribLocation(m_TextProID, "vPosition");
    m_TextTexCoordID = glGetAttribLocation(m_TextProID, "texCoord");
    m_TextTexID = glGetUniformLocation(m_TextProID, "TextTex");
    CHECK_GL_ERROR();

    glGenBuffers(1, &m_RenderVertexBuf);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderVertexBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_RenderTexCoordBuf);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderTexCoordBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordReverseVertices), coordReverseVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_TextTexCoordBuf);
    glBindBuffer(GL_ARRAY_BUFFER, m_TextTexCoordBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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

    BitmapEx aBmp = OpenGLHelper::ConvertBGRABufferToBitmapEx(buf.get(), m_iWidth, m_iHeight);

#if DEBUG_PNG // debug PNG writing
    static int nIdx = 0;
    OUString aName = OUString( "file:///home/moggi/Documents/work/" ) + OUString::number( nIdx++ ) + ".png";
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
        //fill vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        CHECK_GL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, pointList.size() * sizeof(float), &pointList[0], GL_STATIC_DRAW);
        CHECK_GL_ERROR();
        // Use our shader
        glUseProgram(m_CommonProID);
        CHECK_GL_ERROR();

        glUniform4fv(m_2DColorID, 1, &m_2DColor[0]);
        CHECK_GL_ERROR();
        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);
        //CHECK_GL_ERROR();

        // 1rst attribute buffer : vertices
        CHECK_GL_ERROR();
        glVertexAttribPointer(
            m_2DVertexID,
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
        glEnableVertexAttribArray(m_2DVertexID);
        glDrawArrays(GL_LINE_STRIP, 0, pointList.size()/3); // 12*3 indices starting at 0 -> 12 triangles
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
    aContext.makeCurrent();
    aContext.setWinSize(Size(m_iWidth, m_iHeight));
    glViewport(0, 0, m_iWidth, m_iHeight);
    if (!m_FboID[0])
    {
        // create a texture object
        CreateTextureObj(m_iWidth, m_iHeight);
        //create render buffer object
        CreateRenderObj(m_iWidth, m_iHeight);
        //create fbo
        CreateFrameBufferObj();
        if (mbArbMultisampleSupported)
        {
            CreateMultiSampleFrameBufObj();
        }
    }
    //bind fbo
    if (mbArbMultisampleSupported)
    {
        glBindFramebuffer(GL_FRAMEBUFFER,m_frameBufferMS);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboID[0]);
    }

    // Clear the screen
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
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FboID[0]);
        status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            SAL_INFO("chart2.opengl", "The frame buffer status is not complete!");
        }
        glBlitFramebuffer(0, 0 ,m_iWidth, m_iHeight, 0, 0,m_iWidth ,m_iHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboID[0]);

    BitmapEx aBitmap = GetAsBitmap();
#if RENDER_TO_FILE
    static int nIdx = 0;
    OUString aName = OUString( "file:///home/moggi/Documents/work/text" ) + OUString::number( nIdx++ ) + ".png";
    try {
        vcl::PNGWriter aWriter( aBitmap );
        SvFileStream sOutput( aName, STREAM_WRITE );
        aWriter.Write( sOutput );
        sOutput.Close();
    } catch (...) {
        SAL_WARN("chart2.opengl", "Error writing png to " << aName);
    }
#else
    Graphic aGraphic(aBitmap);
    uno::Reference< awt::XBitmap> xBmp( aGraphic.GetXGraphic(), uno::UNO_QUERY );
    uno::Reference < beans::XPropertySet > xPropSet ( mxTarget, uno::UNO_QUERY );
    xPropSet->setPropertyValue("Graphic", uno::makeAny(aGraphic.GetXGraphic()));
    mxTarget->setSize(awt::Size(m_iWidth*OPENGL_SCALE_VALUE, m_iHeight*OPENGL_SCALE_VALUE));
    mxTarget->setPosition(awt::Point(0,0));
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int OpenGLRender::CreateTextureObj(int width, int height)
{
    glGenTextures(2, m_TextureObj);
    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_TextureObj[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        CHECK_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    return 0;
}

int OpenGLRender::CreateRenderObj(int width, int height)
{
    glGenRenderbuffers(2, m_RboID);
    for (int i = 0; i < 2; i++)
    {
        CHECK_GL_ERROR();
        glBindRenderbuffer(GL_RENDERBUFFER, m_RboID[i]);
        CHECK_GL_ERROR();
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        CHECK_GL_ERROR();
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        CHECK_GL_ERROR();
    }
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
    // create a framebuffer object, you need to delete them when program exits.
    glGenFramebuffers(2, m_FboID);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);
    for (int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboID[i]);
        glBindTexture(GL_TEXTURE_2D, m_TextureObj[i]);
        // attach a texture to FBO color attachement point
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureObj[i], 0);
        glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindTexture(GL_TEXTURE_2D, 0);
        // attach a renderbuffer to depth attachment point
        glBindRenderbuffer(GL_RENDERBUFFER, m_RboID[i]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RboID[i]);
        glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
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
    glDeleteFramebuffers(2, m_FboID);
    glDeleteFramebuffers(1, &m_frameBufferMS);
    glDeleteTextures(2, m_TextureObj);
    glDeleteRenderbuffers(2, m_RboID);
    glDeleteRenderbuffers(1, &m_renderBufferColorMS);
    glDeleteRenderbuffers(1, &m_renderBufferDepthMS);
}

OpenGLRender::OpenGLRender(uno::Reference< drawing::XShape > xTarget)
    : mxTarget(xTarget)
    , m_iWidth(1600)
    , m_iHeight(900)
    , m_Model(glm::mat4(1.0f))
    , m_VertexBuffer(0)
    , m_ColorBuffer(0)
    , m_MatrixID(0)
    , m_RenderVertexBuf(0)
    , m_RenderTexCoordBuf(0)
#if 0
    , m_iPointNum(0)
#endif
    , m_fLineWidth(0.001f)
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
    //TODO: moggi: use STL
    memset(&m_Line2DPointList, 0, sizeof(Line2DPointList));
    m_FboID[0] = 0;
    m_FboID[1] = 0;
    m_TextureObj[0] = 0;
    m_TextureObj[1] = 0;
    m_RboID[0] = 0;
    m_RboID[1] = 0;

    memset(&m_Bubble2DCircle, 0, sizeof(m_Bubble2DCircle));

    //TODO: moggi: use STL
    for (size_t i = 0; i < sizeof(m_BackgroundColor) / sizeof(float); i++)
    {
        m_BackgroundColor[i] = 1.0;
    }
    memset(&m_Area2DPointList, 0, sizeof(m_Area2DPointList));
}

OpenGLRender::~OpenGLRender()
{
    Release();
}

// TODO: moggi: that screws up FBO if called after buffers have been created!!!!
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
    //check whether to create the circle data
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
    glm::vec4 edgeColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
    size_t listNum = m_Bubble2DShapePointList.size();
    for (size_t i = 0; i < listNum; i++)
    {
        //move the circle to the pos, and scale using the xScale and Y scale
        Bubble2DPointList &pointList = m_Bubble2DShapePointList.front();
        PosVecf3 trans = {pointList.x, pointList.y, m_fZStep};
        PosVecf3 angle = {0.0f, 0.0f, 0.0f};
        PosVecf3 scale = {pointList.xScale / 2, pointList.yScale / 2 , 1.0f};
        MoveModelf(trans, angle, scale);
        m_MVP = m_Projection * m_View * m_Model;
        //render to fbo
        //fill vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        if (m_Bubble2DCircle.empty())
        {
            Create2DCircle(100);
        }
        glBufferData(GL_ARRAY_BUFFER, m_Bubble2DCircle.size() * sizeof(GLfloat), &m_Bubble2DCircle[0], GL_STATIC_DRAW);

        glUseProgram(m_CommonProID);

        glUniform4fv(m_2DColorID, 1, &m_2DColor[0]);

        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(m_2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_2DVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
        glDrawArrays(GL_TRIANGLE_FAN, 0, m_Bubble2DCircle.size() / 2);
        glDisableVertexAttribArray(m_2DVertexID);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //add black edge
        glLineWidth(3.0);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_Bubble2DCircle.size() * sizeof(GLfloat) -2 , &m_Bubble2DCircle[2], GL_STATIC_DRAW);
        glUseProgram(m_CommonProID);
        glUniform4fv(m_2DColorID, 1, &edgeColor[0]);
        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);
        glEnableVertexAttribArray(m_2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_2DVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
        glDrawArrays(GL_LINE_STRIP, 0, (m_Bubble2DCircle.size() * sizeof(GLfloat) -2) / sizeof(float) / 2);
        glDisableVertexAttribArray(m_2DVertexID);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_Bubble2DShapePointList.pop_front();
        glLineWidth(m_fLineWidth);
    }
    //if use MSAA, we should copy the data to the FBO texture
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
    //check whether to create the circle data
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
        //move the circle to the pos, and scale using the xScale and Y scale
        RectanglePointList &pointList = m_RectangleShapePointList.front();
        {
            PosVecf3 trans = {0, 0, 0};
            PosVecf3 angle = {0.0f, 0.0f, 0.0f};
            PosVecf3 scale = {1, 1, 1.0f};
            MoveModelf(trans, angle, scale);
            m_MVP = m_Projection * m_View * m_Model;
        }

        //render to fbo
        //fill vertex buffer
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
                    m_BackgroundVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );

            // 2nd attribute buffer : color
            glEnableVertexAttribArray(m_BackgroundColorID);
            glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
            glVertexAttribPointer(
                    m_BackgroundColorID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                    4,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );
            //TODO: moggi: get rid of GL_QUADS
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
            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(m_BackgroundVertexID);
            glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
            glVertexAttribPointer(
                    m_BackgroundVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );

            // 2nd attribute buffer : color
            glEnableVertexAttribArray(m_BackgroundColorID);
            glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
            glVertexAttribPointer(
                    m_BackgroundColorID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                    4,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
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

int OpenGLRender::CreateTextTexture(::rtl::OUString const &textValue, Font aFont, long , awt::Point aPos, awt::Size aSize, long rotation)
{
    VirtualDevice aDevice(*Application::GetDefaultDevice(), 0, 0);
    aDevice.Erase();
    Rectangle aRect;
    aDevice.SetFont(aFont);
    aDevice.GetTextBoundRect(aRect, textValue);
    int screenWidth = (aRect.BottomRight().X() + 3) & ~3;
    int screenHeight = (aRect.BottomRight().Y() + 3) & ~3;
    aDevice.SetOutputSizePixel(Size(screenWidth * 3, screenHeight));
    aDevice.SetBackground(Wallpaper(COL_TRANSPARENT));
    aDevice.DrawText(Point(0, 0), textValue);
    int bmpWidth = (aRect.Right() - aRect.Left() + 3) & ~3;
    int bmpHeight = (aRect.Bottom() - aRect.Top() + 3) & ~3;
    BitmapEx aBitmap = BitmapEx(aDevice.GetBitmapEx(aRect.TopLeft(), Size(bmpWidth, bmpHeight)));

    sal_Int32 nXPos = aPos.X;
    sal_Int32 nYPos = aPos.Y;
    ::basegfx::B2DHomMatrix aM;
    aM.rotate( -rotation*F_PI/180.0 );//#i78696#->#i80521#
    aM.translate( nXPos, nYPos );
    drawing::HomogenMatrix3 aTrans = chart::B2DHomMatrixToHomogenMatrix3(aM);
    aTrans.Line1.Column1 = 20 * bmpWidth;
    aTrans.Line2.Column2 = 20 * bmpHeight;
    return CreateTextTexture(aBitmap,aPos,aSize,rotation,aTrans);
}


int OpenGLRender::CreateTextTexture(const BitmapEx& rBitmapEx, const awt::Point&, const awt::Size& aSize, long rotation,
        const drawing::HomogenMatrix3& rTrans)
{
#if DEBUG_PNG // debug PNG writing
    static int nIdx = 0;
    OUString aName = OUString( "file:///home/moggi/Documents/work/text" ) + OUString::number( nIdx++ ) + ".png";
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
    boost::scoped_array<sal_uInt8> bitmapBuf(OpenGLHelper::ConvertBitmapExToRGBABuffer(rBitmapEx));

    TextInfo aTextInfo;
    aTextInfo.rotation = -(double)rotation / 360.0 * 2* GL_PI;
    aTextInfo.vertex[0] = -aSize.Width / 2 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[1] = -aSize.Height / 2 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[2] = m_fZStep;

    aTextInfo.vertex[3] = aSize.Width / 2 / OPENGL_SCALE_VALUE ;
    aTextInfo.vertex[4] = -aSize.Height / 2 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[5] = m_fZStep;

    aTextInfo.vertex[6] = aSize.Width / 2 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[7] = aSize.Height / 2 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[8] = m_fZStep;

    aTextInfo.vertex[9] = -aSize.Width / 2 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[10] = aSize.Height / 2 / OPENGL_SCALE_VALUE;
    aTextInfo.vertex[11] = m_fZStep;
    aTextInfo.nDx = (rTrans.Line1.Column3 + aSize.Width / 2 ) / OPENGL_SCALE_VALUE - bmpWidth/2;
    aTextInfo.nDy = (rTrans.Line2.Column3 + aSize.Height / 2 ) / OPENGL_SCALE_VALUE - bmpHeight/2;

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
        PosVecf3 trans = { textInfo.nDx, textInfo.nDy, 0};
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
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(m_TextVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_TextVertexID,
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
        //tex coord
        CHECK_GL_ERROR();
        glEnableVertexAttribArray(m_TextTexCoordID);
        glBindBuffer(GL_ARRAY_BUFFER, m_TextTexCoordBuf);
        glVertexAttribPointer(
            m_TextTexCoordID,
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
        //texture
        CHECK_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, textInfo.texture);
        CHECK_GL_ERROR();
        glUniform1i(m_TextTexID, 0);
        CHECK_GL_ERROR();
        //TODO: moggi: get rid fo GL_QUADS
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

// only 2D
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
        bool bIsCCW = checkCCW(pointList); // is it counter clockwise (CCW) or clockwise (CW)
        if(!bIsCCW)
            glFrontFace(GL_CW);
        //fill vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, pointList.size() * sizeof(float), &pointList[0], GL_STATIC_DRAW);
        // Use our shader
        glUseProgram(m_CommonProID);

        glUniform4fv(m_2DColorID, 1, &m_2DColor[0]);

        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(m_2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_2DVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
        // TODO: moggi: remove deprecated GL_POLYGON
        glDrawArrays(GL_POLYGON, 0, pointList.size() / 3); // 12*3 indices starting at 0 -> 12 triangles
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

void OpenGLRender::SetBackGroundColor(sal_uInt32 color1, sal_uInt32 color2, sal_uInt8 fillStyle)
{
    sal_uInt8 r = (color1 & 0x00FF0000) >> 16;
    sal_uInt8 g = (color1 & 0x0000FF00) >> 8;
    sal_uInt8 b = (color1 & 0x000000FF);

    m_BackgroundColor[0] = (float)r / 255.0f;
    m_BackgroundColor[1] = (float)g / 255.0f;
    m_BackgroundColor[2] = (float)b / 255.0f;
    m_BackgroundColor[3] = fillStyle ? 1.0 : 0.0;

    m_BackgroundColor[4] = (float)r / 255.0f;
    m_BackgroundColor[5] = (float)g / 255.0f;
    m_BackgroundColor[6] = (float)b / 255.0f;
    m_BackgroundColor[7] = fillStyle ? 1.0 : 0.0;

    r = (color2 & 0x00FF0000) >> 16;
    g = (color2 & 0x0000FF00) >> 8;
    b = (color2 & 0x000000FF);

    m_BackgroundColor[8] = (float)r / 255.0f;
    m_BackgroundColor[9] = (float)g / 255.0f;
    m_BackgroundColor[10] = (float)b / 255.0f;
    m_BackgroundColor[11] = fillStyle ? 1.0 : 0.0;

    m_BackgroundColor[12] = (float)r / 255.0f;
    m_BackgroundColor[13] = (float)g / 255.0f;
    m_BackgroundColor[14] = (float)b / 255.0f;
    m_BackgroundColor[15] = fillStyle ? 1.0 : 0.0;
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
    // TODO: moggi: GL_TRIANGLE_FAN seems not to work
    bool bInnerRadiusNotZero = true; //!rtl::math::approxEqual(0.0, fInnerRadius);
    size_t nVectorSize = 3*(nAngleWidth/nAngleStep);
    if(bInnerRadiusNotZero)
        nVectorSize *= 2;

    nAngleStart += 90;
    aPointList.reserve(nVectorSize);
    // if inner radius = 0 generate a normal pie segment (triangle fan)
    // if inner radius != 0 generate a pie segment - inner pie (triangle strip)
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
        //fill vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, pointList.size() * sizeof(float), &pointList[0] , GL_STATIC_DRAW);
        // Use our shader
        glUseProgram(m_CommonProID);

        glUniform4fv(m_2DColorID, 1, &m_2DColor[0]);

        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(m_2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_2DVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
        glDrawArrays(GL_TRIANGLE_STRIP, 0, pointList.size() / 3); // 12*3 indices starting at 0 -> 12 triangles
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
    //fill vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    CHECK_GL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), aPos, GL_STATIC_DRAW);
    CHECK_GL_ERROR();

    // Use our shader
    glUseProgram(m_SymbolProID);
    CHECK_GL_ERROR();

    glUniform4fv(m_SymbolColorID, 1, &m_2DColor[0]);
    glUniform1i(m_SymbolShapeID, nSymbol);
    CHECK_GL_ERROR();

    glUniformMatrix4fv(m_SymbolMatrixID, 1, GL_FALSE, &m_MVP[0][0]);

    CHECK_GL_ERROR();
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(m_SymbolVertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glVertexAttribPointer(
            m_SymbolVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
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
