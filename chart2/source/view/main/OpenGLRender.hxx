/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_VIEW_MAIN_OPENGLRENDER_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_MAIN_OPENGLRENDER_HXX

#include <vcl/bitmapex.hxx>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <vcl/font.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <boost/shared_array.hpp>

// Include GLM
#include <list>
#include <map>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

#define DEBUG_POSITIONING 0

struct PosVecf3
{
    float x;
    float y;
    float z;
};

struct Bubble2DPointList
{
    float x;
    float y;
    float xScale;
    float yScale;
};

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

typedef std::vector<GLfloat> PointList;

// for 3D, use vector to save the points

class OpenGLRender
{
public:
    OpenGLRender();
    ~OpenGLRender();
    int InitOpenGL();
    int MoveModelf(const PosVecf3& trans, const PosVecf3& angle, const PosVecf3& scale);
    void SetSize(int width, int height);
    void SetSizePixel(int width, int height);
    void Release();
    int RenderLine2FBO(int wholeFlag);
    int SetLine2DShapePoint(float x, float y, int listLength);
    void SetLine2DColor(sal_uInt8 r, sal_uInt8 g, sal_uInt8 b, sal_uInt8 nAlpha);
    void SetLine2DWidth(int width);
    void SetColor(sal_uInt32 color, sal_uInt8 nAlpha);
    int Bubble2DShapePoint(float x, float y, float directionX, float directionY);
    int RenderBubble2FBO(int wholeFlag);

    void prepareToRender();

    int RenderRectangleShape(bool bBorder, bool bFill);
    int RectangleShapePoint(float x, float y, float directionX, float directionY);

    int CreateTextTexture(const boost::shared_array<sal_uInt8> &rPixels,
                          const ::Size &aPixelSize,
                          const css::awt::Point&,
                          const css::awt::Size& aSize,
                          long rotation,
                          const css::drawing::HomogenMatrix3& rTrans);
    int CreateTextTexture(const BitmapEx& rBitmapEx,
            const css::awt::Point& aPos, const css::awt::Size& aSize,
            long rotation, const css::drawing::HomogenMatrix3& rTrans);
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
#if 0
        //for performance
    double m_dFreq;
#endif
    int m_iWidth;

    int m_iHeight;

private:
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

    PointList m_Line2DPointList;

    float m_fLineWidth;

    std::list <PointList> m_Line2DShapePointList;

    glm::vec4 m_2DColor;

    std::vector<GLfloat> m_Bubble2DCircle;

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

    PointList m_Area2DPointList;
    std::list <PointList> m_Area2DShapePointList;

    GLint m_BackgroundProID;
    GLint m_BackgroundMatrixID;
    GLint m_BackgroundVertexID;
    GLint m_BackgroundColorID;

    float m_BackgroundColor[16];

    std::list <PointList> m_PieSegment2DShapePointList;

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

#endif // INCLUDED_CHART2_SOURCE_VIEW_MAIN_OPENGLRENDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
