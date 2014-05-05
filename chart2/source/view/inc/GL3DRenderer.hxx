/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHART2_GL3DRENDERER_HXX
#define CHART2_GL3DRENDERER_HXX

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

#include <com/sun/star/awt/Point.hpp>
#include <tools/gen.hxx>

#include <vcl/opengl/IOpenGLRenderer.hxx>

#include <vector>
#include <list>
#include <map>

#define MAX_LIGHT_NUM 8

namespace chart {

namespace opengl3D {

struct PosVecf3
{
    float x;
    float y;
    float z;
};

typedef std::vector <glm::vec3> Vertices3D;
typedef std::vector <glm::vec2> UVs3D;
typedef std::vector <glm::vec3> Normals3D;

typedef struct MaterialParameters
{
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 materialColor;

    bool twoSidesLighting;
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
    bool lineOnly;
    float lineWidth;
    bool twoSidesLighting;
    long fillStyle;
    glm::vec4 polygonColor;
    Vertices3D *vertices;
    UVs3D *uvs;
    Normals3D *normals;
    std::list <Vertices3D *> verticesList;
    std::list <UVs3D *> uvsList;
    std::list <Normals3D *> normalsList;
    Material material;
}Polygon3DInfo;

typedef struct Extrude3DInfo
{
    int pickingFlg;
    bool lineOnly;
    float lineWidth;
    bool twoSidesLighting;
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

typedef struct SceneBox
{
    float maxXCoord;
    float minXCoord;
    float maxYCoord;
    float minYCoord;
    float maxZCoord;
    float minZCoord;
}SceneBox;


class OpenGL3DRenderer : public IOpenGLInfoProvider
{
public:
    OpenGL3DRenderer();

    void LoadShaders();
    void init();
    bool isOpenGLInitialized();
    void Set3DSenceInfo(sal_Int32 color = 255, bool twoSidesLighting = true);
    void SetLightInfo(bool lightOn, sal_Int32 color, const glm::vec4& direction);
    void AddShapePolygon3DObject(sal_Int32 color, bool lineOnly, sal_Int32 lineColor,
            long fillStyle, sal_Int32 specular);
    void EndAddShapePolygon3DObject();
    void AddPolygon3DObjectNormalPoint(float x, float y, float z);
    void EndAddPolygon3DObjectNormalPoint();
    void AddPolygon3DObjectPoint(float x, float y, float z);
    void EndAddPolygon3DObjectPoint();
    void AddShape3DExtrudeObject(sal_Int32 color, sal_Int32 specular, float xTransform,
            float yTransform, float zTransform);
    void EndAddShape3DExtrudeObject();
    void AddExtrude3DObjectPoint(float x, float y, float z);
    void EndAddExtrude3DObjectPoint();
    double GetTime();
    void SetFPS(float fps);
    void SetClickPos(Point aMPos);
    void RenderClickPos(Point aMPos);
    void SetSize(const Size& rSize);

private:
    void MoveModelf(PosVecf3& trans,PosVecf3& angle,PosVecf3& scale);

    void GetFreq();
    void RenderPolygon3DObject();
    void RenderLine3D(Polygon3DInfo &polygon);
    void RenderPolygon3D(Polygon3DInfo &polygon);
    void Init3DUniformBlock();
    void Update3DUniformBlock();
    void RenderExtrude3DObject();
    void RenderFPS(float fps);
    void RenderText(const ::rtl::OUString& string, com::sun::star::awt::Point aPos);
    void RenderExtrudeSurface(const Extrude3DInfo& extrude3D);
    void RenderExtrudeTopSurface(const Extrude3DInfo& extrude3D);
    void RenderExtrudeMiddleSurface(const Extrude3DInfo& extrude3D);
    void RenderExtrudeBottomSurface(const Extrude3DInfo& extrude3D);
    void RenderExtrudeFlatSurface(const Extrude3DInfo& extrude3D, int surIndex);
    void ProcessUnrenderedShape();
    glm::vec4 GetColorByIndex(int index);
    sal_uInt32 GetIndexByColor(sal_uInt32 r, sal_uInt32 g, sal_uInt32 b);
    void ProcessPickingBox();
    int ProcessExtrude3DPickingBox();
    void RenderCoordinateAxis();
    void AddVertexData(GLuint vertexBuf);
    void AddNormalData(GLuint normalBuf);
    void AddIndexData(GLuint indexBuf);
    bool GetSimilarVertexIndex(PackedVertex & packed,
        std::map<PackedVertex,unsigned short> & VertexToOutIndex,
        unsigned short & result
        );
    void SetVertex(PackedVertex &packed,
        std::map<PackedVertex,unsigned short> &VertexToOutIndex,
        std::vector<glm::vec3> &vertex,
        std::vector<glm::vec3> &normal,
        std::vector<unsigned short> &indeices);
    void CreateActualRoundedCube(float fRadius, int iSubDivY, int iSubDivZ, float width, float height, float depth);
    int GenerateRoundCornerBar(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, float fRadius, int iSubDivY,
                               int iSubDivZ, float width, float height, float depth);
    void CreateSceneBoxView();
private:
    // Projection matrix : default 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 m_Projection;
    // Camera matrix
    glm::mat4 m_View;
    // Model matrix : an identity matrix (model will be at the origin
    glm::mat4 m_Model;
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 m_MVP;
    bool m_IsOpenglInit;
#if 0
    double m_dFreq;
#endif

    int m_iWidth;

    int m_iHeight;

    float m_fZmax;

    Lights m_LightsInfo;

    CameraInfo m_CameraInfo;

    Polygon3DInfo m_Polygon3DInfo;

    std::list <Polygon3DInfo> m_Polygon3DInfoList;

    glm::mat4 m_D3DTrasform;

    glm::mat4 m_3DProjection;

    glm::mat4 m_3DView;

    glm::mat4 m_3DViewBack;

    glm::mat4 m_3DMVP;

    glm::mat4 m_TranslationMatrix;

    GLint m_3DProID;

    GLuint m_3DProjectionID;

    GLuint m_3DViewID;

    GLuint m_3DModelID;

    GLuint m_3DVertexID;

    GLuint m_3DNormalID;

    GLuint m_3DNormalMatrixID;

    GLuint m_3DUBOBuffer;
#if 0
    GLint m_3DLightBlockIndex;

    GLint m_3DMaterialBlockIndex;
#endif
    GLint m_3DActualSizeLight;

    GLuint m_NormalBuffer;

    GLuint m_VertexBuffer;

    GLint m_MatrixID;

    GLuint m_FboID[2];

    Extrude3DInfo m_Extrude3DInfo;

    std::vector <Extrude3DInfo> m_Extrude3DList;

    GLuint m_CubeVertexBuf;

    GLuint m_CubeElementBuf;

    GLuint m_CubeNormalBuf;

    bool m_bCameraUpdated;

    float m_fFPS;

    Point m_aMPos;

    GLuint m_BoundBox;

    GLuint m_CoordinateBuf;

    int m_uiSelectFrameCounter;

    CoordinateAxisInfo m_coordinateAxisinfo;

    std::vector<glm::vec3> m_Vertices;

    std::vector<glm::vec3> m_Normals;

    std::vector<unsigned short> m_Indeices;

    RoundBarMesh m_RoundBarMesh;

    GLint m_CommonProID;
    GLint m_2DVertexID;
    GLint m_2DColorID;

    //TODO: moggi: kill the following parts
    // don't add anything below or I will remove it
    size_t m_iPointNum;

    SceneBox m_SenceBox;

    float m_fViewAngle;

    float m_fHeightWeight;
};

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
