/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/bitmapex.hxx>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <vcl/font.hxx>
#include <vcl/opengl/OpenGLContext.hxx>

// Include GLM
#include <list>
#include <map>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

#define BMP_HEADER_LEN 54

#define OPENGL_SCALE_VALUE 20
#define DEBUG_POSITIONING 0
#define RENDER_TO_FILE 0

using namespace std;
using namespace ::com::sun::star;

struct PosVecf3
{
    float x;
    float y;
    float z;
};

typedef std::vector<GLfloat> Line2DPointList;

struct Bubble2DPointList
{
    float x;
    float y;
    float xScale;
    float yScale;
};

typedef std::vector<GLfloat> Bubble2DCircle;

struct RectanglePointList
{
    float points[12];
};

struct TextInfo
{
    GLuint texture;
    double rotation;
    float vertex[12];
    float nDx;
    float nDy;
};

typedef vector<GLfloat> Area2DPointList;
typedef vector<GLfloat> PieSegment2DPointList;
typedef vector<GLfloat> PointList;

// for 3D, use vector to save the points

class OpenGLRender
{
public:
    OpenGLRender(com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xTarget);
    ~OpenGLRender();
    int InitOpenGL();
    int MoveModelf(PosVecf3 trans, PosVecf3 angle, PosVecf3 scale);
    void SetSize(int width, int height);
    void Release();
#if RENDER_TO_FILE
    int CreateBMPHeader(sal_uInt8 *bmpHeader, int xsize, int ysize);
#endif
    int RenderLine2FBO(int wholeFlag);
    int SetLine2DShapePoint(float x, float y, int listLength);
    void SetLine2DColor(sal_uInt8 r, sal_uInt8 g, sal_uInt8 b, sal_uInt8 nAlpha);
    void SetLine2DWidth(int width);
    BitmapEx GetAsBitmap();
    void SetColor(sal_uInt32 color, sal_uInt8 nAlpha);
    int Bubble2DShapePoint(float x, float y, float directionX, float directionY);
    int RenderBubble2FBO(int wholeFlag);

    void prepareToRender();
    void renderToBitmap();

    int RenderRectangleShape(bool bBorder, bool bFill);
    int RectangleShapePoint(float x, float y, float directionX, float directionY);

    int CreateTextTexture(const BitmapEx& rBitmapEx,
            const com::sun::star::awt::Point& aPos, const com::sun::star::awt::Size& aSize,
            long rotation, const com::sun::star::drawing::HomogenMatrix3& rTrans);
    int CreateTextTexture(::rtl::OUString const &textValue, Font aFont, long fontColor, awt::Point aPos, awt::Size aSize, long rotation);
    int RenderTextShape();

    int SetArea2DShapePoint(float x, float y, int listLength);
    int RenderArea2DShape();
    void SetChartTransparencyGradient(long transparencyGradient);

    void GeneratePieSegment2D(double, double, double, double);
    int RenderPieSegment2DShape(float, float, float);

    int RenderSymbol2DShape(float, float, float, float, sal_Int32);
#if DEBUG_POSITIONING
    void renderDebug();
#endif

    void SetBackGroundColor(sal_uInt32 color1, sal_uInt32 color2, sal_uInt8 nAlpha);


private:

    OpenGLContext aContext;

    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxTarget;

#if 0
        //for performance
    double m_dFreq;
#endif
    int m_iWidth;

    int m_iHeight;

private:
    int CreateTextureObj(int width, int height);
    int CreateRenderObj(int width, int height);
    int CreateFrameBufferObj();
    int CreateMultiSampleFrameBufObj();
    int Create2DCircle(int detail);
private:
    // Projection matrix : default 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 m_Projection;
    // Camera matrix
    glm::mat4 m_View;
    // Model matrix : an identity matrix (model will be at the origin
    glm::mat4 m_Model;
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 m_MVP;
#if 0
    GLint m_RenderProID;
#endif
    glm::vec4 m_ClearColor;

    GLuint m_VertexBuffer;

    GLuint m_ColorBuffer;

    GLint m_MatrixID;
#if 0
    GLint m_RenderVertexID;

    GLint m_RenderTexCoordID;

    GLint m_RenderTexID;
#endif
    GLuint m_RenderVertexBuf;

    GLuint m_RenderTexCoordBuf;

    GLuint m_TextureObj[2];

    GLuint m_FboID[2];

    GLuint m_RboID[2];
#if 0
    int m_iPointNum;
#endif
    Line2DPointList m_Line2DPointList;

    float m_fLineWidth;

    std::list <Line2DPointList> m_Line2DShapePointList;


    bool mbArbMultisampleSupported;
#if defined( _WIN32 )
    int m_iArbMultisampleFormat;
#endif

    glm::vec4 m_2DColor;
    GLuint m_frameBufferMS;
    GLuint m_renderBufferColorMS;
    GLuint m_renderBufferDepthMS;

    Bubble2DCircle m_Bubble2DCircle;

    std::list <Bubble2DPointList> m_Bubble2DShapePointList;
    GLint m_CommonProID;
    GLint m_2DVertexID;
    GLint m_2DColorID;

    float m_fZStep;

    std::list <RectanglePointList> m_RectangleShapePointList;
    // add for text
    std::list <TextInfo> m_TextInfoList;
    GLint m_TextProID;
    GLint m_TextMatrixID;
    GLint m_TextVertexID;
    GLint m_TextTexCoordID;
    GLuint m_TextTexCoordBuf;
    GLint m_TextTexID;

    Area2DPointList m_Area2DPointList;
    std::list <Area2DPointList> m_Area2DShapePointList;

    GLint m_BackgroundProID;
    GLint m_BackgroundMatrixID;
    GLint m_BackgroundVertexID;
    GLint m_BackgroundColorID;

    float m_BackgroundColor[16];

    std::list <PieSegment2DPointList> m_PieSegment2DShapePointList;
    PointList m_Symbol2DPointList;
    std::list<PointList> m_Symbol2DShapePointList;

    GLuint m_SymbolProID;
    GLuint m_SymbolVertexID;
    GLuint m_SymbolMatrixID;
    GLuint m_SymbolColorID;
    GLuint m_SymbolShapeID;

#if DEBUG_POSITIONING
    GLuint m_DebugProID;
    GLuint m_DebugVertexID;
    GLuint m_DebugColorID;
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
