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
#define MAX_LIGHT_NUM 8


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

typedef vector <glm::vec3> Vertices3D;
typedef vector <glm::vec2> UVs3D;
typedef vector <glm::vec3> Normals3D;

typedef struct MaterialParameters
{
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 materialColor;

    int twoSidesLighting;
    float shininess;
    float pad;
    float pad1;
}Material;

typedef struct LightSource
{
    glm::vec4   lightColor;
    glm::vec4   positionWorldspace;
    float  lightPower;
    float  pad1;
    float  pad2;
    float  pad3;
}LightSource;

typedef struct GlobalLights
{
    int lightNum;
    glm::vec4 ambient;
    LightSource light[MAX_LIGHT_NUM];
}Lights;

typedef struct Polygon3DInfo
{
    short lineOnly;
    float lineWidth;
    int twoSidesLighting;
    long fillStyle;
    glm::vec4 polygonColor;
    Vertices3D *vertices;
    UVs3D *uvs;
    Normals3D *normals;
    list <Vertices3D *> verticesList;
    list <UVs3D *> uvsList;
    list <Normals3D *> normalsList;
    Material material;
}Polygon3DInfo;

typedef struct Extrude3DInfo
{
    int pickingFlg;
    short lineOnly;
    float lineWidth;
    int twoSidesLighting;
    glm::vec4 extrudeColor;
    long fillStyle;
    float xRange[2];
    float yRange[2];
    float xTransform;
    float yTransform;
    float zTransform;
    Material material;
    int startIndex[5];
    int size[5];
    int reverse;
}Extrude3DInfo;


typedef struct CameraInfo
{
    glm::vec3 cameraPos;
    glm::vec3 cameraOrg;
    glm::vec3 cameraUp;
}CameraInfo;

typedef struct CoordinateAxisInfo
{
    int pickingFlg;
    int reverse;
    glm::vec4 color;
    PosVecf3 trans;
    PosVecf3 scale;
}CoordinateAxisInfo;

typedef struct RoundBarMesh
{
    float topThreshold;
    float bottomThreshold;
    int iMeshStartIndices;
    int iMeshSizes;
    int iElementStartIndices[5];
    int iElementSizes[5];
}RoundBarMesh;

struct PackedVertex{
    glm::vec3 position;
    glm::vec3 normal;
    bool operator<(const PackedVertex that) const{
        return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
    };
};

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
    int CreateTextTexture(::rtl::OUString textValue, Font aFont, long fontColor, awt::Point aPos, awt::Size aSize, long rotation);
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

    void Set3DSenceInfo(glm::vec3 cameraUp,glm::mat4 D3DTrasform,sal_Bool twoSidesLighting,sal_Int32 color);
    void SetLightInfo(sal_Bool lightOn,sal_Int32 color,glm::vec4 direction);
    void AddShapePolygon3DObject(sal_Int32 color,sal_Bool lineOnly,sal_Int32 lineColor,long fillStyle,sal_Int32 specular);
    void EndAddShapePolygon3DObject();
    void AddPolygon3DObjectNormalPoint(float x, float y, float z);
    void EndAddPolygon3DObjectNormalPoint();
    void AddPolygon3DObjectPoint(float x, float y, float z);
    void EndAddPolygon3DObjectPoint();
    void AddShape3DExtrudeObject(sal_Int32 color,sal_Int32 specular,float xTransform,float yTransform,float zTransform);
    void EndAddShape3DExtrudeObject();
    void AddExtrude3DObjectPoint(float x, float y, float z);
    void EndAddExtrude3DObjectPoint();
    double GetTime();
    void SetFPS(float fps);
    void SetClickPos(Point aMPos);
    int RenderClickPos(Point aMPos);

private:

    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxTarget;
        //for performance

    double m_dFreq;

    int m_iWidth;

    int m_iHeight;

private:
    int CreateTextureObj(int width, int height);
    int CreateRenderObj(int width, int height);
    int CreateFrameBufferObj();
    int CreateMultiSampleFrameBufObj();
    int Create2DCircle(int detail);

    void GetFreq();
    int RenderPolygon3DObject();
    int RenderLine3D(Polygon3DInfo &polygon);
    int RenderPolygon3D(Polygon3DInfo &polygon);
    int Init3DUniformBlock();
    int Update3DUniformBlock();
    int RenderExtrude3DObject();
    int RenderFPS(float fps);
    int RenderText(::rtl::OUString string, awt::Point aPos);
    int RenderExtrudeSurface(Extrude3DInfo extrude3D);
    int RenderExtrudeTopSurface(Extrude3DInfo extrude3D);
    int RenderExtrudeMiddleSurface(Extrude3DInfo extrude3D);
    int RenderExtrudeBottomSurface(Extrude3DInfo extrude3D);
    int RenderExtrudeFlatSurface(Extrude3DInfo extrude3D, int surIndex);
    int ProcessUnrenderedShape();
    glm::vec4 GetColorByIndex(int index);
    int GetIndexByColor(int r, int g, int b);
    int ProcessPickingBox();
    int ProcessExtrude3DPickingBox();
    void RenderCoordinateAxis();
    int AddVertexData(GLuint vertexBuf);
    int AddNormalData(GLuint normalBuf);
    int AddIndexData(GLuint indexBuf);
    bool GetSimilarVertexIndex(PackedVertex & packed,
        std::map<PackedVertex,unsigned short> & VertexToOutIndex,
        unsigned short & result
        );
    void SetVertex(PackedVertex &packed,
        std::map<PackedVertex,unsigned short> &VertexToOutIndex,
        vector<glm::vec3> &vertex,
        vector<glm::vec3> &normal,
        vector<unsigned short> &indeices);
    void CreateActualRoundedCube(float fRadius, int iSubDivY, int iSubDivZ, float width, float height, float depth);
    int GenerateRoundCornerBar(vector<glm::vec3> &vertices, vector<glm::vec3> &normals, float fRadius, int iSubDivY,
                               int iSubDivZ, float width, float height, float depth);
private:
    // Projection matrix : default 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 m_Projection;
    // Camera matrix
    glm::mat4 m_View;
    // Model matrix : an identity matrix (model will be at the origin
    glm::mat4 m_Model;
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 m_MVP;

    glm::mat4 m_TranslationMatrix;

    GLint m_RenderProID;

    glm::vec4 m_ClearColor;

    GLuint m_VertexBuffer;

    GLuint m_ColorBuffer;

    GLint m_MatrixID;

    GLint m_RenderVertexID;

    GLint m_RenderTexCoordID;

    GLint m_RenderTexID;

    GLuint m_RenderVertexBuf;

    GLuint m_RenderTexCoordBuf;

    GLuint m_TextureObj[2];

    GLuint m_FboID[2];

    GLuint m_RboID[2];

    int m_iPointNum;

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

    Polygon3DInfo m_Polygon3DInfo;

    list <Polygon3DInfo> m_Polygon3DInfoList;

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
    float m_fZmax;

    Lights m_LightsInfo;

    CameraInfo m_CameraInfo;

    glm::mat4 m_D3DTrasform;

    glm::mat4 m_3DProjection;

    glm::mat4 m_3DView;

    glm::mat4 m_3DViewBack;

    glm::mat4 m_3DMVP;

    GLint m_3DProID;

    GLuint m_3DProjectionID;

    GLuint m_3DViewID;

    GLuint m_3DModelID;

    GLuint m_3DVertexID;

    GLuint m_3DNormalID;

    GLuint m_3DNormalMatrixID;

    GLuint m_3DUBOBuffer;

    GLint m_3DLightBlockIndex;

    GLint m_3DMaterialBlockIndex;

    GLint m_3DActualSizeLight;

    GLuint m_NormalBuffer;

    Extrude3DInfo m_Extrude3DInfo;

    vector <Extrude3DInfo> m_Extrude3DList;

    GLuint m_CubeVertexBuf;

    GLuint m_CubeElementBuf;

    GLuint m_CubeNormalBuf;

    sal_Bool m_bCameraUpdated;

    float m_fFPS;

    Point m_aMPos;

    GLuint m_BoundBox;

    GLuint m_CoordinateBuf;

    int m_uiSelectFrameCounter;

    CoordinateAxisInfo m_coordinateAxisinfo;

    vector<glm::vec3> m_Vertices;

    vector<glm::vec3> m_Normals;

    vector<unsigned short> m_Indeices;

    RoundBarMesh m_RoundBarMesh;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
