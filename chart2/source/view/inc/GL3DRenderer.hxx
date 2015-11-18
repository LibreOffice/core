/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_GL3DRENDERER_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_GL3DRENDERER_HXX

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

#include <com/sun/star/awt/Point.hpp>
#include <boost/shared_array.hpp>
#include <tools/gen.hxx>

#include <vcl/bitmapex.hxx>

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
typedef std::vector <glm::vec3> Normals3D;

struct MaterialParameters
{
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 materialColor;

    bool twoSidesLighting;
    float shininess;
    float pad;
    float pad1;
};

struct LightSource
{
    glm::vec4   lightColor;
    glm::vec4   positionWorldspace;
    float  lightPower;
    float  pad1;
    float  pad2;
    float  pad3;
};

struct GlobalLights
{
    int lightNum;
    glm::vec4 ambient;
    LightSource light[MAX_LIGHT_NUM];
};

struct Polygon3DInfo
{
    bool lineOnly;
    float lineWidth;
    bool twoSidesLighting;
    long fillStyle;
    glm::vec4 polygonColor;
    glm::vec4 id;
    Vertices3D *vertices;
    Normals3D *normals;
    std::vector <Vertices3D *> verticesList;
    std::vector <Normals3D *> normalsList;
    MaterialParameters material;
};

struct Extrude3DInfo
{
    bool rounded;
    bool twoSidesLighting;
    glm::vec4 extrudeColor;
    glm::vec4 id;
    sal_uInt32 orgID;
    float xScale;
    float yScale;
    float zScale;
    float xTransform;
    float yTransform;
    float zTransform;
    glm::mat4 rotation;
    MaterialParameters material;
    int startIndex[5];
    int size[5];
    int reverse;
};

struct CameraInfo
{
    glm::vec3 cameraPos;
    glm::vec3 cameraOrg;
    glm::vec3 cameraUp;

    CameraInfo():
        cameraUp(glm::vec3(0, 0, 1)) {}
};

struct RoundBarMesh
{
    float topThreshold;
    float bottomThreshold;
    int iMeshStartIndices;
    int iMeshSizes;
    int iElementStartIndices[5];
    int iElementSizes[5];
};

struct PackedVertex{
    glm::vec3 position;
    glm::vec3 normal;
    bool operator<(const PackedVertex& that) const{
        return memcmp(this, &that, sizeof(PackedVertex))>0;
    };
};

struct TextInfo
{
    glm::vec4 id;
    sal_uInt32 uniqueId;
    GLuint texture;
    float vertex[12];
    glm::vec3 pos;
    glm::vec4 textColor;
};

struct TextureArrayInfo
{
    size_t subTextureNum;
    int textureArrayWidth;
    int textureArrayHeight;
    GLuint textureID;

    TextureArrayInfo();
};

struct TextInfoBatch
{
    size_t batchNum;
    std::vector<glm::vec4> idList;
    std::vector<TextureArrayInfo> texture;
    std::vector<glm::vec3> vertexList;
    std::vector<glm::vec3> textureCoordList;
};

struct BatchBarInfo
{
    std::vector <glm::mat4> modelMatrixList;
    std::vector <glm::mat3> normalMatrixList;
    std::vector <glm::vec4> colorList;
    std::map<sal_uInt32, unsigned int> mapId2Color;
    glm::vec4 selectBarColor;
};

class OpenGL3DRenderer
{
public:
    OpenGL3DRenderer();
    ~OpenGL3DRenderer();

    void init();
    void Set3DSenceInfo(sal_uInt32 color = 255, bool twoSidesLighting = true);
    void SetLightInfo(bool lightOn, sal_uInt32 color, const glm::vec4& direction);
    void AddShapePolygon3DObject(sal_uInt32 color, bool lineOnly, sal_uInt32 lineColor,
            long fillStyle, sal_uInt32 specular, sal_uInt32 nUniqueId);
    void EndAddShapePolygon3DObject();
    void AddPolygon3DObjectNormalPoint(float x, float y, float z);
    void EndAddPolygon3DObjectNormalPoint();
    void AddPolygon3DObjectPoint(float x, float y, float z);
    void EndAddPolygon3DObjectPoint();
    void AddShape3DExtrudeObject(bool roundedCorner, sal_uInt32 color, sal_uInt32 specular, const glm::mat4& modelMatrix, sal_uInt32 nUniqueId);
    void EndAddShape3DExtrudeObject();
    void SetSize(const Size& rSize);
    void SetCameraInfo(const glm::vec3& pos, const glm::vec3& direction, const glm::vec3& up);
    void CreateTextTexture(const boost::shared_array<sal_uInt8> &bitmapBuf,
                           const ::Size& rSizePixels,
                           const glm::vec3& vTopLeft, const glm::vec3& vTopRight,
                           const glm::vec3& vBottomRight, const glm::vec3& vBottomLeft,
                           sal_uInt32 nUniqueId);
    void CreateScreenTextTexture(const boost::shared_array<sal_uInt8> &bitmapBuf,
                                 const ::Size& rSizePixels, const glm::vec2& vTopLeft,
                                 const glm::vec2& vBottomRight, const glm::vec3& vPos, const glm::vec4& vScreenTextColor,
                                 sal_uInt32 nUniqueId);
    void ProcessUnrenderedShape(bool bNewScene);

    void SetPickingMode(bool bPickingMode);

    sal_uInt32 GetPixelColorFromPoint(long nX, long nY);

    void ReleaseShapes();
    void ReleaseScreenTextShapes();
    void ReleaseTextTexture();
    void ReleaseScreenTextTexture();
    void StartClick(sal_uInt32 &selectID);
    void EndClick();
    void SetScroll();
    void SetScrollSpeed(float scrollSpeed);
    void SetScrollDistance(float scrollDistance);
    void SetSceneEdge(float minCoordX, float maxCoordX);
    glm::mat4 GetProjectionMatrix();
    glm::mat4 GetViewMatrix();
    glm::mat4 GetGlobalScaleMatrix();
    glm::mat4 GetDiffOfTwoCameras(const glm::vec3& rBeginPos, const glm::vec3& rEndPos, const glm::vec3& rBeginDirection, const glm::vec3& rEndDirection);
    glm::mat4 GetDiffOfTwoCameras(const glm::vec3& rEndPos, const glm::vec3& rEndDirection);
    void AddMatrixDiff(const glm::mat4& aMat);
    void ResetMatrixDiff();
private:
    void MoveModelf( const PosVecf3& trans, const PosVecf3& angle, const PosVecf3& scale);

    static void ClearBuffer();
    void RenderPolygon3DObject();
    void RenderLine3D(const Polygon3DInfo &polygon);
    void RenderPolygon3D(const Polygon3DInfo &polygon);
    void Init3DUniformBlock();
    void Update3DUniformBlock();
    void RenderExtrude3DObject();
    //add for text
    void RenderTextShape();
    void RenderScreenTextShape();
    void RenderExtrudeSurface(const Extrude3DInfo& extrude3D);
    void RenderExtrudeTopSurface(const Extrude3DInfo& extrude3D);
    void RenderExtrudeMiddleSurface(const Extrude3DInfo& extrude3D);
    void RenderExtrudeBottomSurface(const Extrude3DInfo& extrude3D);
    void RenderExtrudeFlatSurface(const Extrude3DInfo& extrude3D, int surIndex);
    void AddVertexData(GLuint vertexBuf);
    void AddNormalData(GLuint normalBuf);
    void AddIndexData(GLuint indexBuf);
    void RenderNonRoundedBar(const Extrude3DInfo& extrude3D);
    static bool GetSimilarVertexIndex(PackedVertex & packed,
        std::map<PackedVertex,unsigned short> & VertexToOutIndex,
        unsigned short & result
        );
    static void SetVertex(PackedVertex &packed,
        std::map<PackedVertex,unsigned short> &VertexToOutIndex,
        std::vector<glm::vec3> &vertex,
        std::vector<glm::vec3> &normal,
        std::vector<unsigned short> &indeices);
    void CreateActualRoundedCube(float fRadius, int iSubDivY, int iSubDivZ, float width, float height, float depth);
    static int GenerateRoundCornerBar(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, float fRadius, int iSubDivY,
                               int iSubDivZ, float width, float height, float depth);
    void CreateSceneBoxView();

    void ReleasePolygonShapes();
    void ReleaseExtrude3DShapes();
    void ReleaseTextShapes();
    void ReleaseBatchBarInfo();
    void GetBatchBarsInfo();
    void GetBatchTopAndFlatInfo(const Extrude3DInfo &extrude3D);
    void GetBatchMiddleInfo(const Extrude3DInfo &extrude3D);
    void InitBatch3DUniformBlock();
    void UpdateBatch3DUniformBlock();
    void RenderBatchBars(bool bNewScene);
    void CheckGLSLVersion();
    void RenderTextShapeBatch();
    void ReleaseTextShapesBatch();
    void CreateTextTextureSingle(const boost::shared_array<sal_uInt8> &bitmapBuf,
                           const ::Size& rSizePixels,
                           const glm::vec3& vTopLeft, const glm::vec3& vTopRight,
                           const glm::vec3& vBottomRight, const glm::vec3& vBottomLeft,
                           sal_uInt32 nUniqueId);
    void CreateTextTextureBatch(const boost::shared_array<sal_uInt8> &bitmapBuf,
                       const ::Size& rSizePixels,
                       const glm::vec3& vTopLeft, const glm::vec3& vTopRight,
                       const glm::vec3& vBottomRight, const glm::vec3& vBottomLeft,
                       sal_uInt32 nUniqueId);
    void SetHighLightBar(BatchBarInfo &barInfo);
    void DisableHighLightBar(BatchBarInfo &barInfo);
    void CalcScrollMoveMatrix(bool bNewScene);
private:

    struct ShaderResources
    {
        bool m_b330Support;
        bool m_bScrollFlag;
        // 3DProID
        GLint m_3DProID;
        GLint m_3DProjectionID;
        GLint m_3DViewID;
        GLint m_3DModelID;
        GLint m_3DNormalMatrixID;
        GLint m_3DVertexID;
        GLint m_3DNormalID;
        GLint m_3DMinCoordXID;
        GLint m_3DMaxCoordXID;
        GLint m_3DUndrawID;
        //300 version;
        GLint m_3DMaterialAmbientID;
        GLint m_3DMaterialDiffuseID;
        GLint m_3DMaterialSpecularID;
        GLint m_3DMaterialColorID;
        GLint m_3DMaterialTwoSidesID;
        GLint m_3DMaterialShininessID;
        GLint m_3DLightColorID;
        GLint m_3DLightPosID;
        GLint m_3DLightPowerID;
        GLint m_3DLightNumID;
        GLint m_3DLightAmbientID;

        // TextProID
        GLint m_TextProID;
        GLint m_TextMatrixID;
        GLint m_TextVertexID;
        GLint m_TextTexCoordID;
        GLint m_TextTexID;

        // ScreenTextProID
        GLint m_ScreenTextProID;
        GLint m_ScreenTextVertexID;
        GLint m_ScreenTextTexCoordID;
        GLint m_ScreenTextTexID;
        GLint m_ScreenTextColorID;

        // CommonProID
        GLint m_CommonProID;
        GLint m_2DVertexID;
        GLint m_2DColorID;
        GLint m_MatrixID;

        // Batch render
        GLint m_3DBatchProID;
        GLint m_3DBatchProjectionID;
        GLint m_3DBatchViewID;
        GLint m_3DBatchModelID;
        GLint m_3DBatchNormalMatrixID;
        GLint m_3DBatchVertexID;
        GLint m_3DBatchNormalID;
        GLint m_3DBatchColorID;
        GLint m_3DBatchTransMatrixID;
        GLint m_3DBatchMinCoordXID;
        GLint m_3DBatchMaxCoordXID;
        GLint m_3DBatchUndrawID;

        //Batch render text
        bool mbTexBatchSupport;
        GLint m_BatchTextProID;
        GLint m_BatchTextMatrixID;
        GLint m_BatchTextVertexID;
        GLint m_BatchTextTexCoordID;
        GLint m_BatchTextTexID;

        ShaderResources();
        ~ShaderResources();

        void LoadShaders();
    };

    struct PickingShaderResources
    {
        // CommonProID
        GLint m_CommonProID;
        GLint m_2DVertexID;
        GLint m_2DColorID;
        GLint m_MatrixID;
        GLint m_ModelID;
        GLint m_MinCoordXID;
        GLint m_MaxCoordXID;
        PickingShaderResources();
        ~PickingShaderResources();

        void LoadShaders();
    };

    ShaderResources maResources;
    PickingShaderResources maPickingResources;

    // Model matrix : an identity matrix (model will be at the origin
    glm::mat4 m_Model;

    sal_Int32 m_iWidth;

    sal_Int32 m_iHeight;

    GlobalLights m_LightsInfo;

    CameraInfo m_CameraInfo;

    Polygon3DInfo m_Polygon3DInfo;

    std::vector <Polygon3DInfo> m_Polygon3DInfoList;

    glm::mat4 m_3DProjection;

    glm::mat4 m_3DView;

    glm::mat4 m_3DMVP;

    GLuint m_3DUBOBuffer;
#if 0
    GLint m_3DLightBlockIndex;

    GLint m_3DMaterialBlockIndex;
#endif
    GLint m_3DActualSizeLight;

    GLuint m_NormalBuffer;

    GLuint m_VertexBuffer;

    Extrude3DInfo m_Extrude3DInfo;

    std::vector <Extrude3DInfo> m_Extrude3DList;

    GLuint m_CubeVertexBuf;

    GLuint m_CubeElementBuf;

    GLuint m_CubeNormalBuf;

    GLuint m_BoundBox;
    GLuint m_BoundBoxNormal;
     // add for text
    std::vector <TextInfo> m_TextInfoList;
    std::vector <TextInfo> m_ScreenTextInfoList;
    GLuint m_TextTexCoordBuf;
    GLuint m_TextTexCoordBufBatch;

    std::vector<glm::vec3> m_Vertices;

    std::vector<glm::vec3> m_Normals;

    std::vector<unsigned short> m_Indices;

    RoundBarMesh m_RoundBarMesh;

    GLuint m_RenderVertexBuf;

    GLuint m_RenderTexCoordBuf;

    float m_fViewAngle;

    bool mbPickingMode;

    GLuint mnPickingFbo;
    GLuint mnPickingRboDepth;
    GLuint mnPickingRboColor;

    BatchBarInfo m_BarSurface[3];
    GLuint m_BatchModelMatrixBuf;
    GLuint m_BatchNormalMatrixBuf;
    GLuint m_BatchColorBuf;
    MaterialParameters m_Batchmaterial;
    GLuint m_Batch3DUBOBuffer;
    GLint m_Batch3DActualSizeLight;

    glm::mat4 m_GlobalScaleMatrix;
    TextInfoBatch m_TextInfoBatch;
    //for 3.0 version
    int m_iLightNum;
    glm::vec4 m_Ambient;
    glm::vec4 m_LightColor[MAX_LIGHT_NUM];
    glm::vec4 m_PositionWorldspace[MAX_LIGHT_NUM];
    float m_fLightPower[MAX_LIGHT_NUM];
    //for 3.0 end
    std::vector<GLuint> m_Texturelist;
    std::vector<GLuint> m_ScreenTexturelist;
    bool m_bHighLighting;
    sal_uInt32 m_uiSelectID;
    float m_fScrollSpeed;
    float m_fScrollDistance;
    float m_fMinCoordX;
    float m_fMaxCoordX;
    float m_fCurDistance;
    glm::mat4 m_ScrollMoveMatrix;
    bool m_bUndrawFlag;
    glm::mat4 m_matDiff;
};

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
