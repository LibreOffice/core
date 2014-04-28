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

#include <vector>

namespace chart {

namespace opengl3D {

typedef std::vector <glm::vec3> Vertices3D;
typedef std::vector <glm::vec2> UVs3D;
typedef std::vector <glm::vec3> Normals3D;

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

class OpenGL3DRenderer : public IOpenGLInfoProvider
{
public:
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
    float m_fZmax;

    Lights m_LightsInfo;

    CameraInfo m_CameraInfo;

    Polygon3DInfo m_Polygon3DInfo;

    list <Polygon3DInfo> m_Polygon3DInfoList;

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

};

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
