/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GL/glew.h>

#include "GL3DRenderer.hxx"

#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/font.hxx>
#include <vcl/virdev.hxx>

#include <com/sun/star/awt/Size.hpp>

#include <StaticGeometry.h>
#include "glm/gtc/matrix_inverse.hpp"

#define GL_PI 3.14159f
#define OPENGL_SCALE_VALUE 20
#define RGB_WHITE (0xFF | (0xFF << 8) | (0xFF << 16))

using namespace std;
using namespace com::sun::star;

namespace chart {

namespace opengl3D {

namespace {

struct TextInfo
{
    GLuint texture;
    double rotation;
    float vertex[12];
    float nDx;
    float nDy;
};

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

#define CHECK_GL_ERROR() checkGLError(__FILE__, __LINE__)

}

OpenGL3DRenderer::OpenGL3DRenderer():
    m_TranslationMatrix(glm::translate(m_Model, glm::vec3(0.0f, 0.0f, 0.0f)))
{
    m_Polygon3DInfo.lineOnly = false;
    m_Polygon3DInfo.twoSidesLighting = false;
    m_Polygon3DInfo.vertices = NULL;
    m_Polygon3DInfo.uvs = NULL;
    m_Polygon3DInfo.normals = NULL;
    m_Polygon3DInfo.lineWidth = 0.001f;


    m_Extrude3DInfo.lineOnly = false;
    m_Extrude3DInfo.twoSidesLighting = false;
    m_Extrude3DInfo.lineWidth = 0.001f;

    m_bCameraUpdated = false;
    GetFreq();

    for (int i = 0; i < 5; i++)
    {
        m_Extrude3DInfo.startIndex[i] = m_RoundBarMesh.iElementStartIndices[i];
        m_Extrude3DInfo.size[i] = m_RoundBarMesh.iElementSizes[i];
    }

    m_uiSelectFrameCounter = 0;

}

void OpenGL3DRenderer::LoadShaders()
{
    m_3DProID = OpenGLHelper::LoadShaders("Shape3DVertexShader", "Shape3DFragmentShader");
    m_3DProjectionID = glGetUniformLocation(m_3DProID, "P");
    m_3DViewID = glGetUniformLocation(m_3DProID, "V");
    m_3DModelID = glGetUniformLocation(m_3DProID, "M");
    m_3DNormalMatrixID = glGetUniformLocation(m_3DProID, "normalMatrix");
    m_3DVertexID = glGetAttribLocation(m_3DProID, "vertexPositionModelspace");
    m_3DNormalID = glGetAttribLocation(m_3DProID, "vertexNormalModelspace");

    Init3DUniformBlock();
}

void OpenGL3DRenderer::init()
{
    glGenBuffers(1, &m_CubeVertexBuf);
    AddVertexData(m_CubeVertexBuf);

    glGenBuffers(1, &m_CubeNormalBuf);
    AddNormalData(m_CubeNormalBuf);

    glGenBuffers(1, &m_CubeElementBuf);
    AddIndexData(m_CubeElementBuf);

    for (int i = 0; i < 5; i++)
    {
        m_Extrude3DInfo.startIndex[i] = m_RoundBarMesh.iElementStartIndices[i];
        m_Extrude3DInfo.size[i] = m_RoundBarMesh.iElementSizes[i];
    }

    glGenBuffers(1, &m_BoundBox);
    glBindBuffer(GL_ARRAY_BUFFER, m_BoundBox);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boundBox), boundBox, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_CoordinateBuf);
    glBindBuffer(GL_ARRAY_BUFFER, m_CoordinateBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordinateAxis), coordinateAxis, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    m_3DProjection = glm::perspective(30.0f, (float)m_iWidth / (float)m_iHeight, 0.01f, 2000.0f);

}

int OpenGL3DRenderer::AddVertexData(GLuint vertexBuf)
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3), &m_Vertices[0], GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;
}

int OpenGL3DRenderer::AddNormalData(GLuint normalBuf)
{
    glBindBuffer(GL_ARRAY_BUFFER, normalBuf);
    glBufferData(GL_ARRAY_BUFFER, m_Normals.size() * sizeof(glm::vec3), &m_Normals[0], GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;
}

int OpenGL3DRenderer::AddIndexData(GLuint indexBuf)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indeices.size() * sizeof(unsigned short), &m_Indeices[0], GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return 0;
}

bool OpenGL3DRenderer::GetSimilarVertexIndex(PackedVertex & packed,
    std::map<PackedVertex,unsigned short> & VertexToOutIndex,
    unsigned short & result
    ){
        std::map<PackedVertex,unsigned short>::iterator it = VertexToOutIndex.find(packed);
        if ( it == VertexToOutIndex.end() ){
            return false;
        }else{
            result = it->second;
            return true;
        }
}

void OpenGL3DRenderer::SetVertex(PackedVertex &packed,
    std::map<PackedVertex,unsigned short> &VertexToOutIndex,
    vector<glm::vec3> &vertex,
    vector<glm::vec3> &normal,
    vector<unsigned short> &indeices)
{
    unsigned short index;
    bool found = GetSimilarVertexIndex(packed, VertexToOutIndex, index);
    if ( found )
    { // A similar vertex is already in the VBO, use it instead !
        indeices.push_back( index );
    }
    else
    { // If not, it needs to be added in the output data.
        vertex.push_back(packed.position);
        normal.push_back(packed.normal);
        unsigned short newindex = (unsigned short)vertex.size() - 1;
        indeices .push_back( newindex );
        VertexToOutIndex[ packed ] = newindex;
    }
}

void OpenGL3DRenderer::CreateActualRoundedCube(float fRadius, int iSubDivY, int iSubDivZ, float width, float height, float depth)
{
    if ((fRadius > (width / 2)) || (fRadius > (height / 2)) || (fRadius > (depth / 2)))
    {
        return;
    }
    float topThreshold = height - 2 * fRadius;
    float bottomThreshold = fRadius;

    vector<glm::vec3> vertices;
    vector<glm::vec3> normals;
    GenerateRoundCornerBar(vertices, normals, fRadius, iSubDivY, iSubDivZ, width, height, depth);
    std::map<PackedVertex,unsigned short> VertexToOutIndex;
    glm::vec3 actualVerteices[3];
    glm::vec3 actualNormals[3];
    vector<unsigned short> indeices[5];
    glm::vec3 externSurNormal;
    glm::mat4 corrctCoord = glm::translate(glm::vec3(width / 2.0f, height / 2.0f  - fRadius, depth / 2.0f));
    m_RoundBarMesh.topThreshold = topThreshold;
    m_RoundBarMesh.bottomThreshold = bottomThreshold;
    m_RoundBarMesh.iMeshStartIndices = m_Vertices.size();
    for (int k = 0; k < 5; k++)
    {
        m_RoundBarMesh.iElementStartIndices[k] = indeices[k].size();
    }
    for (unsigned int i = 0; i < vertices.size(); i += 3)
    {
        for (int k = 0; k < 3; k++)
        {
            actualVerteices[k] = glm::vec3(corrctCoord * glm::vec4(vertices[i + k], 1.0));
            actualNormals[k] = normals[i + k];
        }
        float maxY = max(max(actualVerteices[0].y, actualVerteices[1].y), actualVerteices[2].y);
        float minY = min(min(actualVerteices[0].y, actualVerteices[1].y), actualVerteices[2].y);
        int surfaceIndex = (minY >= topThreshold - 0.001) ? TOP_SURFACE : ((maxY <= bottomThreshold + 0.001) ? BOTTOM_SURFACE : MIDDLE_SURFACE);
        for (int k = 0; k < 3; k++)
        {
            {
                PackedVertex packed = {actualVerteices[k], actualNormals[k]};
                SetVertex(packed, VertexToOutIndex, m_Vertices, m_Normals, indeices[surfaceIndex]);
            }

            //add extern
            if ((surfaceIndex == TOP_SURFACE) || (surfaceIndex == BOTTOM_SURFACE))
            {
                actualVerteices[k].y = 0.0f;
                externSurNormal = (surfaceIndex == TOP_SURFACE) ? glm::vec3(0.0, 1.0, 0.0) : glm::vec3(0.0, -1.0, 0.0);
                int tmpSurfaceIndex = (surfaceIndex == TOP_SURFACE) ? FLAT_TOP_SURFACE : FLAT_BOTTOM_SURFACE;
                PackedVertex packed = {actualVerteices[k], externSurNormal};
                SetVertex(packed, VertexToOutIndex, m_Vertices, m_Normals, indeices[tmpSurfaceIndex]);
            }
        }

    }
    //create index below
    m_RoundBarMesh.iMeshSizes = m_Vertices.size() - m_RoundBarMesh.iMeshStartIndices;
    for (int k = 0; k < 5; k++)
    {
        m_RoundBarMesh.iElementSizes[k] = indeices[k].size() - m_RoundBarMesh.iElementStartIndices[k];
        m_RoundBarMesh.iElementStartIndices[k] = m_Indeices.size() * sizeof(unsigned short);
        for (unsigned int IdxCnt = 0; IdxCnt < indeices[k].size(); IdxCnt++)
        {
            m_Indeices.push_back(indeices[k][IdxCnt]);
        }
        indeices[k].clear();
    }
    VertexToOutIndex.clear();
}

int OpenGL3DRenderer::GenerateRoundCornerBar(vector<glm::vec3> &vertices, vector<glm::vec3> &normals, float fRadius, int iSubDivY, int iSubDivZ, float width, float height, float depth)
{
    //float fAddAngleY = 360.0f/float(iSubDivY), fAddAngleZ = 180.0f/float(iSubDivZ);
    float fAddAngleY = 360.0f/float(iSubDivY), fAddAngleZ = 180.0f/float(iSubDivZ);
    float fCurAngleY = 0.0f;
    int iStepsY = 1;

    const float PI = float(atan(1.0)*4.0);
    int iFacesAdded = 0;
    float xOffset[] = {(width / 2 - fRadius), -(width / 2 - fRadius), -(width / 2 - fRadius), (width / 2 - fRadius), (width / 2 - fRadius)};
    float yOffset[] = {(height / 2 - fRadius), -(height / 2 - fRadius), (height / 2 - fRadius)};
    float zOffset[] = {-(depth / 2 - fRadius), -(depth / 2 - fRadius), (depth / 2 - fRadius), (depth / 2 - fRadius), -(depth / 2 - fRadius)};
    int iIndices[] = {0, 1, 2, 2, 3, 0};
    while(iStepsY <= iSubDivY)
    {
        float fNextAngleY = fCurAngleY+fAddAngleY;
        if ((fCurAngleY < 90.0f) && (fNextAngleY >= 90.0f))
        {
            fNextAngleY = 90.0f;
        }
        else if ((fCurAngleY < 180.0f) && (fNextAngleY >= 180.0f))
        {
            fNextAngleY = 180.0f;
        }
        else if ((fCurAngleY < 270.0f) && (fNextAngleY >= 270.0f))
        {
            fNextAngleY = 270.0f;
        }
        else if ((fCurAngleY < 360.0f) && (fNextAngleY >= 360.0f))
        {
            fNextAngleY = 360.0f;
        }
        float fSineY = sin(fCurAngleY/180.0f*PI), fCosY = cos(fCurAngleY/180.0f*PI);
        float fNextSineY = sin(fNextAngleY/180.0f*PI), fNextCosY = cos(fNextAngleY/180.0f*PI);
        glm::vec3 vDirY(fCosY, 0.0f, -fSineY), vNextDirY(fNextCosY, 0.0f, -fNextSineY);
        float fCurAngleZ = 0.0f;
        int iStepsZ = 1;
        int xzIndex = 0;
        if ((fCurAngleY >= 0) && (fCurAngleY < 90.0))
        {
            xzIndex = 0;
        }
        else if ((fCurAngleY >= 90) && (fCurAngleY < 180.0))
        {
            xzIndex = 1;
        }
        else if ((fCurAngleY >= 180) && (fCurAngleY < 270.0))
        {
            xzIndex = 2;
        }
        else if ((fCurAngleY >= 270) && (fCurAngleY < 360.0))
        {
            xzIndex = 3;
        }
        while(iStepsZ <= iSubDivZ)
        {
            int yIndex = 0;
            float fNextAngleZ = fCurAngleZ+fAddAngleZ;

            float fSineZ = sin(fCurAngleZ/180.0f*PI), fCosZ = cos(fCurAngleZ/180.0f*PI);
            float fNextSineZ = sin(fNextAngleZ/180.0f*PI), fNextCosZ = cos(fNextAngleZ/180.0f*PI);

            if ((fCurAngleZ < 90.0f) && (fNextAngleZ >= 90.0f))
            {
                fNextAngleZ = 90.0f;
            }

            if ((fCurAngleZ >= 0.0f) && (fCurAngleZ < 90.0f))
            {
                yIndex = 0;
            }
            else
            {
                yIndex = 1;
            }

            glm::vec3 vQuadPoints[] =
            {
                glm::vec3(vDirY.x*fSineZ*fRadius, fCosZ*fRadius, vDirY.z*fSineZ*fRadius),
                glm::vec3(vDirY.x*fNextSineZ*fRadius, fNextCosZ*fRadius, vDirY.z*fNextSineZ*fRadius),
                glm::vec3(vNextDirY.x*fNextSineZ*fRadius, fNextCosZ*fRadius, vNextDirY.z*fNextSineZ*fRadius),
                glm::vec3(vNextDirY.x*fSineZ*fRadius, fCosZ*fRadius, vNextDirY.z*fSineZ*fRadius)
            };

            glm::vec3 vActualQuadPoints[] =
            {

                glm::vec3(vQuadPoints[0].x + xOffset[xzIndex], vQuadPoints[0].y + yOffset[yIndex], vQuadPoints[0].z + zOffset[xzIndex]),
                glm::vec3(vQuadPoints[1].x + xOffset[xzIndex], vQuadPoints[1].y + yOffset[yIndex], vQuadPoints[1].z + zOffset[xzIndex]),
                glm::vec3(vQuadPoints[2].x + xOffset[xzIndex], vQuadPoints[2].y + yOffset[yIndex], vQuadPoints[2].z + zOffset[xzIndex]),
                glm::vec3(vQuadPoints[3].x + xOffset[xzIndex], vQuadPoints[3].y + yOffset[yIndex], vQuadPoints[3].z + zOffset[xzIndex])
            };

            glm::vec3 vNormals[] =
            {
                glm::normalize(vQuadPoints[0]),
                glm::normalize(vQuadPoints[1]),
                glm::normalize(vQuadPoints[2]),
                glm::normalize(vQuadPoints[3])
            };
            for (int i = 0; i < 6; i++)
            {
                int index = iIndices[i];
                vertices.push_back(vActualQuadPoints[index]);
                normals.push_back(vNormals[index]);
            }
            iFacesAdded += 2; // Keep count of added faces
            if (((fCurAngleY < 90.0) && (fNextAngleY >= 90.0)) ||
                ((fCurAngleY < 180.0) && (fNextAngleY >= 180.0)) ||
                ((fCurAngleY < 270.0) && (fNextAngleY >= 270.0)) ||
                ((fCurAngleY < 360.0) && (fNextAngleY >= 360.0)))
            {
                glm::vec3 vXZQuadNextPoints[] =
                {
                    glm::vec3(vQuadPoints[3].x + xOffset[xzIndex], vQuadPoints[3].y + yOffset[yIndex], vQuadPoints[3].z + zOffset[xzIndex]),
                    glm::vec3(vQuadPoints[2].x + xOffset[xzIndex], vQuadPoints[2].y + yOffset[yIndex], vQuadPoints[2].z + zOffset[xzIndex]),
                    glm::vec3(vQuadPoints[2].x + xOffset[xzIndex + 1], vQuadPoints[2].y + yOffset[yIndex], vQuadPoints[2].z + zOffset[xzIndex + 1]),
                    glm::vec3(vQuadPoints[3].x + xOffset[xzIndex + 1], vQuadPoints[3].y + yOffset[yIndex], vQuadPoints[3].z + zOffset[xzIndex + 1])
                };
                glm::vec3 vXZNextNormals[] =
                {
                    glm::normalize(vQuadPoints[3]),
                    glm::normalize(vQuadPoints[2]),
                    glm::normalize(vQuadPoints[2]),
                    glm::normalize(vQuadPoints[3])
                };
                for (int i = 0; i < 6; i++)
                {
                    int index = iIndices[i];
                    vertices.push_back(vXZQuadNextPoints[index]);
                    normals.push_back(vXZNextNormals[index]);
                }
                iFacesAdded += 2;
            }
            if ((fCurAngleZ < 90.0) && (fNextAngleZ >= 90.0))
            {
                glm::vec3 vYQuadNextPoints[] =
                {
                    glm::vec3(vQuadPoints[1].x + xOffset[xzIndex], vQuadPoints[1].y + yOffset[yIndex], vQuadPoints[1].z + zOffset[xzIndex]),
                    glm::vec3(vQuadPoints[1].x + xOffset[xzIndex], vQuadPoints[1].y + yOffset[yIndex + 1], vQuadPoints[1].z + zOffset[xzIndex]),
                    glm::vec3(vQuadPoints[2].x + xOffset[xzIndex], vQuadPoints[2].y + yOffset[yIndex + 1], vQuadPoints[2].z + zOffset[xzIndex]),
                    glm::vec3(vQuadPoints[2].x + xOffset[xzIndex], vQuadPoints[2].y + yOffset[yIndex], vQuadPoints[2].z + zOffset[xzIndex])
                };
                glm::vec3 vYNextNormals[] =
                {
                    glm::normalize(vQuadPoints[1]),
                    glm::normalize(vQuadPoints[1]),
                    glm::normalize(vQuadPoints[2]),
                    glm::normalize(vQuadPoints[2])
                };
                for (int i = 0; i < 6; i++)
                {
                    int index = iIndices[i];
                    vertices.push_back(vYQuadNextPoints[index]);
                    normals.push_back(vYNextNormals[index]);
                }
                iFacesAdded += 2;
            }
            iStepsZ++;
            if ((iStepsZ > iSubDivZ) && (fCurAngleZ < 180.0f))
            {
                iStepsZ--;
            }
            fCurAngleZ = fNextAngleZ;
        }
        iStepsY++;
        if ((iStepsY > iSubDivY) && (fCurAngleY < 360.0f))
        {
            iStepsY--;
        }
        fCurAngleY = fNextAngleY;
    }
    //draw the rectangle face
    //top surface
    glm::vec3 vTopPoints[] =
    {
        glm::vec3(xOffset[0], height / 2, zOffset[0]),
        glm::vec3(xOffset[1], height / 2, zOffset[1]),
        glm::vec3(xOffset[2], height / 2, zOffset[2]),
        glm::vec3(xOffset[3], height / 2, zOffset[3])
    };
    glm::vec3 vTopNormal = glm::vec3(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 6; i++)
    {
        int index = iIndices[i];
        vertices.push_back(vTopPoints[index]);
        normals.push_back(vTopNormal);
    }
    iFacesAdded += 2;
    //bottom surface
    glm::vec3 vBottomPoints[] =
    {
        glm::vec3(xOffset[3], -height / 2, zOffset[3]),
        glm::vec3(xOffset[2], -height / 2, zOffset[2]),
        glm::vec3(xOffset[1], -height / 2, zOffset[1]),
        glm::vec3(xOffset[0], -height / 2, zOffset[0])
    };
    glm::vec3 vBottomNormal = glm::vec3(0.0f, -1.0f, 0.0f);
    for (int i = 0; i < 6; i++)
    {
        int index = iIndices[i];
        vertices.push_back(vBottomPoints[index]);
        normals.push_back(vBottomNormal);
    }
    iFacesAdded += 2;
    //left surface
    glm::vec3 vLeftPoints[] =
    {
        glm::vec3(-width / 2, yOffset[0], zOffset[0]),
        glm::vec3(-width / 2, yOffset[1], zOffset[1]),
        glm::vec3(-width / 2, yOffset[1], zOffset[2]),
        glm::vec3(-width / 2, yOffset[0], zOffset[3])
    };
    glm::vec3 vLeftNormal = glm::vec3(-1.0f, 0.0f, 0.0f);
    for (int i = 0; i < 6; i++)
    {
        int index = iIndices[i];
        vertices.push_back(vLeftPoints[index]);
        normals.push_back(vLeftNormal);
    }
    //right surface
    glm::vec3 vRightPoints[] =
    {
        glm::vec3(width / 2, yOffset[0], zOffset[3]),
        glm::vec3(width / 2, yOffset[1], zOffset[2]),
        glm::vec3(width / 2, yOffset[1], zOffset[1]),
        glm::vec3(width / 2, yOffset[0], zOffset[0])
    };
    glm::vec3 vRightNormal = glm::vec3(1.0f, 0.0f, 0.0f);
    for (int i = 0; i < 6; i++)
    {
        int index = iIndices[i];
        vertices.push_back(vRightPoints[index]);
        normals.push_back(vRightNormal);
    }
    iFacesAdded += 2;
    //front surface
    glm::vec3 vFrontPoints[] =
    {
        glm::vec3(xOffset[0], yOffset[0], depth / 2),
        glm::vec3(xOffset[1], yOffset[0], depth / 2),
        glm::vec3(xOffset[2], yOffset[1], depth / 2),
        glm::vec3(xOffset[3], yOffset[1], depth / 2)
    };
    glm::vec3 vFrontNormal = glm::vec3(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < 6; i++)
    {
        int index = iIndices[i];
        vertices.push_back(vFrontPoints[index]);
        normals.push_back(vFrontNormal);
    }
    //back surface
    glm::vec3 vBackPoints[] =
    {
        glm::vec3(xOffset[0], yOffset[1], -depth / 2),
        glm::vec3(xOffset[1], yOffset[1], -depth / 2),
        glm::vec3(xOffset[2], yOffset[0], -depth / 2),
        glm::vec3(xOffset[3], yOffset[0], -depth / 2)
    };
    glm::vec3 vBackNormal = glm::vec3(0.0f, 0.0f, -1.0f);
    for (int i = 0; i < 6; i++)
    {
        int index = iIndices[i];
        vertices.push_back(vBackPoints[index]);
        normals.push_back(vBackNormal);
    }
    iFacesAdded += 2;
    return iFacesAdded;
}


void OpenGL3DRenderer::GetFreq()
{
#if 0
    LARGE_INTEGER litmpold;
    QueryPerformanceFrequency(&litmpold);
    m_dFreq= litmpold.QuadPart;
#endif
}

double OpenGL3DRenderer::GetTime()
{
#if 0
    LARGE_INTEGER litmpold;
    QueryPerformanceCounter(&litmpold);
    return litmpold.QuadPart*1000000 / m_dFreq;
#endif
    return 0;
}

int OpenGL3DRenderer::RenderLine3D(Polygon3DInfo &polygon)
{
    int listNum;
    listNum = polygon.verticesList.size();
    glUseProgram(m_CommonProID);
    for (int i = 0; i < listNum; i++)
    {
        //move the circle to the pos, and scale using the xScale and Y scale
        Vertices3D *pointList = polygon.verticesList.front();
        PosVecf3 trans = {0.0f, 0, 0.0};
        PosVecf3 angle = {0.0f, 0.0f, 0.0f};
        PosVecf3 scale = {1.0f, 1.0f, 1.0f};
        MoveModelf(trans, angle, scale);

        m_3DMVP = m_3DProjection * m_3DView * m_D3DTrasform;
        m_3DMVP = m_3DProjection * m_3DView * m_Model;
        //if line only, using the common shader to render

        //render to fbo
        //fill vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, pointList->size() * sizeof(glm::vec3), &pointList[0][0], GL_STATIC_DRAW);
        glUniform4fv(m_2DColorID, 1, &polygon.polygonColor[0]);
        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_3DMVP[0][0]);
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(m_2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(m_2DVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                                3,                  // size
                                GL_FLOAT,           // type
                                GL_FALSE,           // normalized?
                                0,                  // stride
                                (void*)0            // array buffer offset
                                );

        glLineWidth(polygon.lineWidth);
        glDrawArrays(GL_LINE_STRIP, 0, pointList->size());
        glDisableVertexAttribArray(m_2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        pointList->clear();
        delete pointList;
        polygon.verticesList.pop_front();
    }
    glUseProgram(0);
    return 0;
}

int OpenGL3DRenderer::RenderPolygon3D(Polygon3DInfo &polygon)
{
    int verticesNum, normalsNum;
    verticesNum = polygon.verticesList.size();
    normalsNum = polygon.normalsList.size();
    //check whether the number of vertices and normals are equal
    if (verticesNum != normalsNum)
    {
        return -1;
    }
    //update ubo
    Update3DUniformBlock();
    glBindBuffer(GL_UNIFORM_BUFFER, m_3DUBOBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, m_3DActualSizeLight, sizeof(Material), &polygon.material);
    CHECK_GL_ERROR();
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glUseProgram(m_3DProID);
    glUniformMatrix4fv(m_3DViewID, 1, GL_FALSE, &m_3DView[0][0]);
    glUniformMatrix4fv(m_3DProjectionID, 1, GL_FALSE, &m_3DProjection[0][0]);
    for (int i = 0; i < verticesNum; i++)
    {
        //move the circle to the pos, and scale using the xScale and Y scale
        Vertices3D *pointList = polygon.verticesList.front();
        Normals3D *normalList = polygon.normalsList.front();
        PosVecf3 trans = {0.0f, 0.0f, 0.0};
        PosVecf3 angle = {0.0f, 0.0f, 0.0f};
        PosVecf3 scale = {1.0f, 1.0f, 1.0f};
        MoveModelf(trans, angle, scale);
        glm::mat3 normalMatrix(m_Model);
        glm::mat3 normalInverseTranspos = glm::inverseTranspose(normalMatrix);

        //render to fbo
        //fill vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, pointList->size() * sizeof(glm::vec3), &pointList[0][0], GL_STATIC_DRAW);
        //fill normal buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_NormalBuffer);
        glBufferData(GL_ARRAY_BUFFER, normalList->size() * sizeof(glm::vec3), &normalList[0][0], GL_STATIC_DRAW);
        glUniformMatrix4fv(m_3DModelID, 1, GL_FALSE, &m_Model[0][0]);
        glUniformMatrix3fv(m_3DNormalMatrixID, 1, GL_FALSE, &normalInverseTranspos[0][0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(m_3DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(m_3DVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                                3,                  // size
                                GL_FLOAT,           // type
                                GL_FALSE,           // normalized?
                                0,                  // stride
                                (void*)0            // array buffer offset
                                );
        // 2nd attribute buffer : normals
        glEnableVertexAttribArray(m_3DNormalID);
        glBindBuffer(GL_ARRAY_BUFFER, m_NormalBuffer);
        glVertexAttribPointer(m_3DNormalID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                                3,                  // size
                                GL_FLOAT,           // type
                                GL_FALSE,           // normalized?
                                0,                  // stride
                                (void*)0            // array buffer offset
                                );

        glDrawArrays(GL_POLYGON, 0, pointList->size());
        glDisableVertexAttribArray(m_3DVertexID);
        glDisableVertexAttribArray(m_3DNormalID);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        pointList->clear();
        delete pointList;
        normalList->clear();
        delete normalList;
        polygon.verticesList.pop_front();
        polygon.normalsList.pop_front();
    }
    glUseProgram(0);
    return 0;
}

int OpenGL3DRenderer::RenderPolygon3DObject()
{
    if (!m_bCameraUpdated)
    {
        return 0;
    }
    glDepthMask(GL_FALSE);
    int polygonNum = m_Polygon3DInfoList.size();
    for (int i = 0; i < polygonNum; i++)
    {
        Polygon3DInfo &polygon = m_Polygon3DInfoList.front();
        if (polygon.lineOnly || (!polygon.fillStyle))
        {
            //just use the common shader is ok for lines
            RenderLine3D(polygon);
        }
        else
        {
            RenderPolygon3D(polygon);
        }
        m_Polygon3DInfoList.pop_front();
    }
    glDepthMask(GL_TRUE);
    return 0;
}

void OpenGL3DRenderer::Set3DSenceInfo(glm::vec3 cameraUp,glm::mat4 D3DTrasform,bool twoSidesLighting,sal_Int32 color)
{

    m_CameraInfo.cameraUp = cameraUp;

    m_D3DTrasform = D3DTrasform;

    m_Polygon3DInfo.material.twoSidesLighting = twoSidesLighting;

    m_LightsInfo.ambient = glm::vec4((float)(((color) & 0x00FF0000) >> 16) / 255.0f,
                                                   (float)(((color) & 0x0000FF00) >> 8) / 255.0f,
                                                   (float)(((color) & 0x000000FF)) / 255.0f,
                                                   1.0);

    m_LightsInfo.lightNum = 0;
}

void OpenGL3DRenderer::SetLightInfo(bool lightOn,sal_Int32 color,glm::vec4 direction)
{
    if (lightOn)
    {
        m_LightsInfo.light[m_LightsInfo.lightNum].lightColor = glm::vec4((float)(((color) & 0x00FF0000) >> 16) / 255.0f,
                                                                                                     (float)(((color) & 0x0000FF00) >> 8) / 255.0f,
                                                                                                     (float)(((color) & 0x000000FF)) / 255.0f,
                                                                                                      1.0);
        m_LightsInfo.light[m_LightsInfo.lightNum].positionWorldspace = direction;
        m_LightsInfo.light[m_LightsInfo.lightNum].lightPower = 1.0;
        m_LightsInfo.lightNum++;
    }

}

void OpenGL3DRenderer::AddShapePolygon3DObject(sal_Int32 color,bool lineOnly,sal_Int32 lineColor,long fillStyle,sal_Int32 specular)
{
    m_Polygon3DInfo.polygonColor = glm::vec4((float)(((color) & 0x00FF0000) >> 16) / 255.0f,
                                             (float)(((color) & 0x0000FF00) >> 8) / 255.0f,
                                             (float)(((color) & 0x000000FF)) / 255.0f,
                                             1.0);
    m_Polygon3DInfo.material.materialColor = m_Polygon3DInfo.polygonColor;//material color seems to be the same for all parts, so we use the polygon color
    //line or Polygon
    m_Polygon3DInfo.lineOnly = lineOnly;


    // if line only, use line color
    if (m_Polygon3DInfo.lineOnly)
    {
        m_Polygon3DInfo.polygonColor = glm::vec4((float)(((lineColor) & 0x00FF0000) >> 16) / 255.0f,
                                                     (float)(((lineColor) & 0x0000FF00) >> 8) / 255.0f,
                                                     (float)(((lineColor) & 0x000000FF)) / 255.0f,
                                                     1.0);
    }

    //fillStyle
    m_Polygon3DInfo.fillStyle= fillStyle;


    //material specular;
     m_Polygon3DInfo.material.specular = glm::vec4((float)(((specular) & 0x00FF0000) >> 16) / 255.0f,
                                        (float)(((specular) & 0x0000FF00) >> 8) / 255.0f,
                                        (float)(((specular) & 0x000000FF)) / 255.0f,
                                        1.0);

    m_Polygon3DInfo.material.diffuse = glm::vec4(1.0, 1.0, 1.0, 1.0);

    m_Polygon3DInfo.material.ambient = glm::vec4(0.2, 0.2, 0.2, 1.0);

    m_Polygon3DInfo.material.shininess = 1.0f;
}

void OpenGL3DRenderer::EndAddShapePolygon3DObject()
{
    m_Polygon3DInfoList.push_back(m_Polygon3DInfo);
    m_Polygon3DInfo.normals = NULL;
    m_Polygon3DInfo.vertices = NULL;
    m_Polygon3DInfo.verticesList.clear();
    m_Polygon3DInfo.normalsList.clear();
}

void OpenGL3DRenderer::AddPolygon3DObjectNormalPoint(float x, float y, float z)
 {
    if (m_Polygon3DInfo.fillStyle)
    {
        if (!m_Polygon3DInfo.normals)
        {
            m_Polygon3DInfo.normals = new Normals3D;
        }
         m_Polygon3DInfo.normals->push_back(glm::vec3(x, -y, z));
    }
 }

void OpenGL3DRenderer::EndAddPolygon3DObjectNormalPoint()
{
    m_Polygon3DInfo.normalsList.push_back(m_Polygon3DInfo.normals);
    m_Polygon3DInfo.normals = NULL;
}

void OpenGL3DRenderer::AddPolygon3DObjectPoint(float x, float y, float z)
{
    if (!m_Polygon3DInfo.vertices)
    {
        m_Polygon3DInfo.vertices = new Vertices3D;
    }
    float actualX = (x / OPENGL_SCALE_VALUE) - ((float)m_iWidth / 2);
    float actualY = (y / OPENGL_SCALE_VALUE) - ((float)m_iHeight / 2);
    float actualZ = z / OPENGL_SCALE_VALUE;
    float maxCoord = max(actualX, max(actualY, actualZ));
    m_fZmax = max(maxCoord, m_fZmax);
    m_Polygon3DInfo.vertices->push_back(glm::vec3(actualX, -actualY, actualZ));
}

void OpenGL3DRenderer::EndAddPolygon3DObjectPoint()
{
    m_Polygon3DInfo.verticesList.push_back(m_Polygon3DInfo.vertices);
    //get the buttom surface to calc the camera org, just for the demo
    if ((m_Polygon3DInfo.vertices->size()) && (!m_bCameraUpdated))
    {
        float minX = m_Polygon3DInfo.vertices->at(0).x;
        float maxX = m_Polygon3DInfo.vertices->at(0).x;
        float minZ = m_Polygon3DInfo.vertices->at(0).z;
        float maxZ = m_Polygon3DInfo.vertices->at(0).z;
        float maxY = m_Polygon3DInfo.vertices->at(0).y;
        float minY = m_Polygon3DInfo.vertices->at(0).y;
        for (unsigned int i = 1; i < m_Polygon3DInfo.vertices->size(); i++)
        {
            minX = min(minX, m_Polygon3DInfo.vertices->at(i).x);
            maxX = max(maxX, m_Polygon3DInfo.vertices->at(i).x);
            minZ = min(minZ, m_Polygon3DInfo.vertices->at(i).z);
            maxZ = max(maxZ, m_Polygon3DInfo.vertices->at(i).z);
            minY = min(minY, m_Polygon3DInfo.vertices->at(i).y);
            maxY = max(maxY, m_Polygon3DInfo.vertices->at(i).y);
        }

        if (maxY == minY)
        {
            float distance = maxZ + 300;
            float veriticalAngle = GL_PI / 6.0f;
            float horizontalAngle = 0;
            m_CameraInfo.cameraOrg = glm::vec3(minX + (maxX - minX) / 2,
                                               minY + (maxY - minY) / 2,
                                               minZ + (maxZ - minZ) / 2);
            //update the camera position and org
            m_CameraInfo.cameraPos.x = m_CameraInfo.cameraOrg.x + distance * cos(veriticalAngle) * sin(horizontalAngle);
            m_CameraInfo.cameraPos.z = m_CameraInfo.cameraOrg.z + distance * cos(veriticalAngle) * cos(horizontalAngle);
            m_CameraInfo.cameraPos.y = m_CameraInfo.cameraOrg.y - distance * sin(veriticalAngle);

            m_3DView = glm::lookAt(m_CameraInfo.cameraPos, // Camera is at (0,0,3), in World Space
                       m_CameraInfo.cameraOrg, // and looks at the origin
                       m_CameraInfo.cameraUp  // Head is up (set to 0,-1,0 to look upside-down)
                       );
            m_3DViewBack = m_3DView;
            #if 0
            cout << "update position" << endl;
            cout << "m_CameraInfo.cameraPos.x = " << m_CameraInfo.cameraPos.x << endl;
            cout << "m_CameraInfo.cameraPos.y = " << m_CameraInfo.cameraPos.y << endl;
            cout << "m_CameraInfo.cameraPos.z = " << m_CameraInfo.cameraPos.z << endl;

            cout << "m_CameraInfo.cameraOrg.x = " << m_CameraInfo.cameraOrg.x << endl;
            cout << "m_CameraInfo.cameraOrg.y = " << m_CameraInfo.cameraOrg.y << endl;
            cout << "m_CameraInfo.cameraOrg.z = " << m_CameraInfo.cameraOrg.z << endl;
            #endif
            m_bCameraUpdated = true;
        }

    }
    m_Polygon3DInfo.vertices = NULL;
}

void OpenGL3DRenderer::AddShape3DExtrudeObject(sal_Int32 color,sal_Int32 specular,float xTransform,float yTransform,float zTransform)
{

    //color
    m_Extrude3DInfo.extrudeColor = glm::vec4((float)(((color) & 0x00FF0000) >> 16) / 255.0f,
                                             (float)(((color) & 0x0000FF00) >> 8) / 255.0f,
                                             (float)(((color) & 0x000000FF)) / 255.0f,
                                                 1.0);
    m_Extrude3DInfo.material.materialColor = m_Extrude3DInfo.extrudeColor;//material color seems to be the same for all parts, so we use the polygon color

    //material specular;
    m_Extrude3DInfo.material.specular = glm::vec4((float)(((specular) & 0x00FF0000) >> 16) / 255.0f,
                                        (float)(((specular) & 0x0000FF00) >> 8) / 255.0f,
                                        (float)(((specular) & 0x000000FF)) / 255.0f,
                                        1.0);

    m_Extrude3DInfo.material.diffuse = glm::vec4(1.0, 1.0, 1.0, 1.0);

    m_Extrude3DInfo.material.ambient = glm::vec4(0.2, 0.2, 0.2, 1.0);

    m_Extrude3DInfo.material.shininess = 1.0f;

    m_Extrude3DInfo.xTransform = xTransform / OPENGL_SCALE_VALUE - ((float)m_iWidth / 2);
    m_Extrude3DInfo.yTransform = yTransform / OPENGL_SCALE_VALUE - ((float)m_iHeight / 2);
    m_Extrude3DInfo.zTransform = zTransform / OPENGL_SCALE_VALUE;
}

void OpenGL3DRenderer::EndAddShape3DExtrudeObject()
{
    m_Extrude3DInfo.pickingFlg = 0;
    m_Extrude3DList.push_back(m_Extrude3DInfo);
}

void OpenGL3DRenderer::AddExtrude3DObjectPoint(float x, float y, float z)
{
    float actualX = (x / OPENGL_SCALE_VALUE);
    float actualY = (y / OPENGL_SCALE_VALUE);
    float actualZ = z / OPENGL_SCALE_VALUE;
    float maxCoord = max(actualX, max(actualY, actualZ));
    m_fZmax = max(maxCoord, m_fZmax);
    if (m_iPointNum == 0)
    {
        m_Extrude3DInfo.xRange[0] = actualX;
        m_Extrude3DInfo.xRange[1] = actualX;
        m_Extrude3DInfo.yRange[0] = actualY;
        m_Extrude3DInfo.yRange[1] = actualY;
    }
    m_Extrude3DInfo.xRange[0] = min(m_Extrude3DInfo.xRange[0], actualX);
    m_Extrude3DInfo.xRange[1] = max(m_Extrude3DInfo.xRange[1], actualX);
    m_Extrude3DInfo.yRange[0] = min(m_Extrude3DInfo.yRange[0], actualY);
    m_Extrude3DInfo.yRange[1] = max(m_Extrude3DInfo.yRange[1], actualY);
    m_iPointNum++;
}

void OpenGL3DRenderer::EndAddExtrude3DObjectPoint()
{
    m_iPointNum = 0;
}

int OpenGL3DRenderer::Init3DUniformBlock()
{
    GLuint a3DLightBlockIndex = glGetUniformBlockIndex(m_3DProID, "GlobalLights");
    GLuint a3DMaterialBlockIndex = glGetUniformBlockIndex(m_3DProID, "GlobalMaterialParameters");

    if ((GL_INVALID_INDEX == a3DLightBlockIndex) || (GL_INVALID_INDEX == a3DMaterialBlockIndex))
    {
        return -1;
    }
    int nUniformBufferAlignSize = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &nUniformBufferAlignSize);
    GLint nBlockDataSizeLight = 0, nBlockDataSizeMertrial = 0;
    glGetActiveUniformBlockiv(m_3DProID, a3DLightBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &nBlockDataSizeLight);
    glGetActiveUniformBlockiv(m_3DProID, a3DMaterialBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &nBlockDataSizeMertrial);
    CHECK_GL_ERROR();
    glGenBuffers(1, &m_3DUBOBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_3DUBOBuffer);
    CHECK_GL_ERROR();
    m_3DActualSizeLight = ((nBlockDataSizeLight / nUniformBufferAlignSize) + min(nBlockDataSizeLight % nUniformBufferAlignSize, 1)) * nUniformBufferAlignSize;
//    cout << "nBlockDataSizeMertrial = " << nBlockDataSizeMertrial << ", nBlockDataSizeLight = " << nBlockDataSizeLight << ", m_3DActualSizeLight = " << m_3DActualSizeLight << endl;
    int dataSize = m_3DActualSizeLight + nBlockDataSizeMertrial;
    glBufferData(GL_UNIFORM_BUFFER, dataSize, NULL, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_3DUBOBuffer, 0, nBlockDataSizeLight);
    CHECK_GL_ERROR();
    glUniformBlockBinding(m_3DProID, a3DLightBlockIndex, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_3DUBOBuffer, ((nBlockDataSizeLight / nUniformBufferAlignSize) + min(nBlockDataSizeLight % nUniformBufferAlignSize, 1)) * nUniformBufferAlignSize, nBlockDataSizeMertrial);
    glUniformBlockBinding(m_3DProID, a3DMaterialBlockIndex, 1);
    //for the light source uniform, we must calc the offset of each element
    CHECK_GL_ERROR();
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return 0;
}

int OpenGL3DRenderer::Update3DUniformBlock()
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_3DUBOBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLint), &m_LightsInfo.lightNum);
    CHECK_GL_ERROR();
    //current std140 alignment: 16
    glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(glm::vec4), &m_LightsInfo.ambient[0]);
    CHECK_GL_ERROR();
    //current std140 alignment: 16
    glBufferSubData(GL_UNIFORM_BUFFER, 32, sizeof(LightSource) * MAX_LIGHT_NUM, &m_LightsInfo.light);
    CHECK_GL_ERROR();
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return 0;
}
int OpenGL3DRenderer::RenderExtrudeFlatSurface(Extrude3DInfo extrude3D, int surIndex)
{
    float xzScale = extrude3D.xRange[1] - extrude3D.xRange[0];
    PosVecf3 trans = {extrude3D.xTransform,//m_Extrude3DInfo.xTransform + 140,
                      -extrude3D.yTransform,
                      extrude3D.zTransform};
    m_TranslationMatrix = glm::translate(glm::vec3(trans.x, trans.y, trans.z));
    glm::mat4 flatScale = glm::scale(xzScale, xzScale, xzScale);
    m_Model = m_TranslationMatrix * flatScale;
    glm::mat3 normalMatrix(m_Model);
    glm::mat3 normalInverseTranspos = glm::inverseTranspose(normalMatrix);
    glUniformMatrix4fv(m_3DModelID, 1, GL_FALSE, &m_Model[0][0]);
    glUniformMatrix3fv(m_3DNormalMatrixID, 1, GL_FALSE, &normalInverseTranspos[0][0]);
    glDrawElements(GL_TRIANGLES, extrude3D.size[surIndex], GL_UNSIGNED_SHORT, &extrude3D.startIndex[surIndex]);
    return 0;
}

int OpenGL3DRenderer::RenderExtrudeBottomSurface(Extrude3DInfo extrude3D)
{
    float xzScale = extrude3D.xRange[1] - extrude3D.xRange[0];
    float yScale = extrude3D.yRange[1] - extrude3D.yRange[0];
    float actualYTrans = yScale - m_RoundBarMesh.bottomThreshold * xzScale;
    PosVecf3 trans = {extrude3D.xTransform,//m_Extrude3DInfo.xTransform + 140,
                      -extrude3D.yTransform,
                      extrude3D.zTransform};
    PosVecf3 angle = {0.0f, 0.0f, 0.0f};
    if (actualYTrans < 0.0f)
    {
        // the height of rounded corner is higher than the cube than use the org scale matrix
   //     yScale /= (float)(1 + BOTTOM_THRESHOLD);
        yScale /= (float)(m_RoundBarMesh.bottomThreshold);
        PosVecf3 scale = {xzScale, yScale, xzScale};
        MoveModelf(trans, angle, scale);
    }
    else
    {
        glm::mat4 topTrans = glm::translate(glm::vec3(0.0, -actualYTrans, 0.0));
        glm::mat4 topScale = glm::scale(xzScale, xzScale, xzScale);
        m_TranslationMatrix = glm::translate(glm::vec3(trans.x, trans.y, trans.z));
        m_Model = m_TranslationMatrix * topTrans * topScale;
    }
    glm::mat3 normalMatrix(m_Model);
    glm::mat3 normalInverseTranspos = glm::inverseTranspose(normalMatrix);
    glUniformMatrix4fv(m_3DModelID, 1, GL_FALSE, &m_Model[0][0]);
    glUniformMatrix3fv(m_3DNormalMatrixID, 1, GL_FALSE, &normalInverseTranspos[0][0]);
    glDrawElements(GL_TRIANGLES, extrude3D.size[BOTTOM_SURFACE], GL_UNSIGNED_SHORT, &extrude3D.startIndex[BOTTOM_SURFACE]);
    return 0;
}

int OpenGL3DRenderer::RenderExtrudeMiddleSurface(Extrude3DInfo extrude3D)
{
    float xzScale = extrude3D.xRange[1] - extrude3D.xRange[0];
    float yScale = extrude3D.yRange[1] - extrude3D.yRange[0];
    float actualYScale = yScale - m_RoundBarMesh.bottomThreshold * xzScale;
    PosVecf3 trans = {extrude3D.xTransform,//m_Extrude3DInfo.xTransform + 140,
                      -extrude3D.yTransform,
                      extrude3D.zTransform};
    if (actualYScale < 0.0f)
    {
        // the height of rounded corner is higher than the cube than use the org scale matrix
 //       yScale /= (float)(1 + BOTTOM_THRESHOLD);
 //       PosVecf3 scale = {xzScale, yScale, xzScale};
 //       MoveModelf(trans, angle, scale);
          return 0;
    }
    else
    {
        glm::mat4 scale = glm::scale(xzScale, actualYScale, xzScale);
        m_TranslationMatrix = glm::translate(glm::vec3(trans.x, trans.y, trans.z));
        m_Model = m_TranslationMatrix * scale;
    }
    if (extrude3D.reverse)
    {
        glm::mat4 reverseMatrix = glm::translate(glm::vec3(0.0, -1.0, 0.0));
        m_Model = m_Model * reverseMatrix;
    }
    glm::mat3 normalMatrix(m_Model);
    glm::mat3 normalInverseTranspos = glm::inverseTranspose(normalMatrix);
    glUniformMatrix4fv(m_3DModelID, 1, GL_FALSE, &m_Model[0][0]);
    glUniformMatrix3fv(m_3DNormalMatrixID, 1, GL_FALSE, &normalInverseTranspos[0][0]);
    glDrawElements(GL_TRIANGLES, extrude3D.size[MIDDLE_SURFACE], GL_UNSIGNED_SHORT, &extrude3D.startIndex[MIDDLE_SURFACE]);
    return 0;
}

int OpenGL3DRenderer::RenderExtrudeTopSurface(Extrude3DInfo extrude3D)
{
    float xzScale = extrude3D.xRange[1] - extrude3D.xRange[0];
    float yScale = extrude3D.yRange[1] - extrude3D.yRange[0];
    float actualYTrans = yScale - m_RoundBarMesh.bottomThreshold * xzScale;
    PosVecf3 trans = {extrude3D.xTransform,//m_Extrude3DInfo.xTransform + 140,
                      -extrude3D.yTransform,
                      extrude3D.zTransform};
    if (actualYTrans < 0.0f)
    {
        // the height of rounded corner is higher than the cube than use the org scale matrix
        //yScale /= (float)(1 + BOTTOM_THRESHOLD);
        yScale /= (float)(m_RoundBarMesh.bottomThreshold);
        glm::mat4 orgTrans = glm::translate(glm::vec3(0.0, -1.0, 0.0));
        glm::mat4 scale = glm::scale(xzScale, yScale, xzScale);
        //MoveModelf(trans, angle, scale);
        m_TranslationMatrix = glm::translate(glm::vec3(trans.x, trans.y, trans.z));
        m_Model = m_TranslationMatrix * scale * orgTrans;
    }
    else
    {
        // use different matrices for different parts
        glm::mat4 orgTrans = glm::translate(glm::vec3(0.0, -1.0, 0.0));
        glm::mat4 topTrans = glm::translate(glm::vec3(0.0, actualYTrans, 0.0));
        glm::mat4 topScale = glm::scale(xzScale, xzScale, xzScale);
        m_TranslationMatrix = glm::translate(glm::vec3(trans.x, trans.y, trans.z));
        m_Model = m_TranslationMatrix * topTrans * topScale * orgTrans;
    }
    glm::mat3 normalMatrix(m_Model);
    glm::mat3 normalInverseTranspos = glm::inverseTranspose(normalMatrix);
    glUniformMatrix4fv(m_3DModelID, 1, GL_FALSE, &m_Model[0][0]);
    glUniformMatrix3fv(m_3DNormalMatrixID, 1, GL_FALSE, &normalInverseTranspos[0][0]);
    glDrawElements(GL_TRIANGLES, extrude3D.size[TOP_SURFACE], GL_UNSIGNED_SHORT, &extrude3D.startIndex[TOP_SURFACE]);
    RenderExtrudeFlatSurface(extrude3D, FLAT_BOTTOM_SURFACE);
    return 0;
}

int OpenGL3DRenderer::RenderExtrudeSurface(Extrude3DInfo extrude3D)
{
    glUniformMatrix4fv(m_3DViewID, 1, GL_FALSE, &m_3DView[0][0]);
    glUniformMatrix4fv(m_3DProjectionID, 1, GL_FALSE, &m_3DProjection[0][0]);
    RenderExtrudeMiddleSurface(extrude3D);
    // check reverse flag to decide whether to render the top middle
    if (extrude3D.reverse)
    {
        RenderExtrudeBottomSurface(extrude3D);
        RenderExtrudeFlatSurface(extrude3D, FLAT_TOP_SURFACE);
    }
    else
    {
        RenderExtrudeTopSurface(extrude3D);
        RenderExtrudeFlatSurface(extrude3D, FLAT_BOTTOM_SURFACE);
    }
    return 0;
}

int OpenGL3DRenderer::RenderExtrude3DObject()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    Update3DUniformBlock();
    //render to fbo
    glUseProgram(m_3DProID);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(m_3DVertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_CubeVertexBuf);
    glVertexAttribPointer(m_3DVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                            3,                  // size
                            GL_FLOAT,           // type
                            GL_FALSE,           // normalized?
                            0,                  // stride
                            (void*)0            // array buffer offset
                            );
    // 2nd attribute buffer : normals
    glEnableVertexAttribArray(m_3DNormalID);
    glBindBuffer(GL_ARRAY_BUFFER, m_CubeNormalBuf);
    glVertexAttribPointer(m_3DNormalID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                            3,                  // size
                            GL_FLOAT,           // type
                            GL_FALSE,           // normalized?
                            0,                  // stride
                            (void*)0            // array buffer offset
                            );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_CubeElementBuf);
    int extrude3DNum = m_Extrude3DList.size();
    for (int i = 0; i < extrude3DNum; i++)
    {
        Extrude3DInfo extrude3DInfo = m_Extrude3DList[i];
        glBindBuffer(GL_UNIFORM_BUFFER, m_3DUBOBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, m_3DActualSizeLight, sizeof(Material), &extrude3DInfo.material);
        CHECK_GL_ERROR();
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        extrude3DInfo.reverse = -extrude3DInfo.yRange[0] > extrude3DInfo.yRange[1] ? 0 : 1;
        RenderExtrudeSurface(extrude3DInfo);
    }
    m_Extrude3DList.clear();
    glDisableVertexAttribArray(m_3DVertexID);
    glDisableVertexAttribArray(m_3DNormalID);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_CULL_FACE);
    return 0;
}

void OpenGL3DRenderer::SetFPS(float fps)
{
    m_fFPS = fps;
}

void OpenGL3DRenderer::SetClickPos(Point aMPos)
{
    m_aMPos = aMPos;
}

int OpenGL3DRenderer::RenderText(::rtl::OUString , awt::Point )
{
    //TODO: moggi: disabled for now
    /*
    Font aFont("Arial", Size(0, 100));
    Rectangle aRect;
    VirtualDevice aDevice;
    aDevice.GetTextBoundRect(aRect, string);
    int screenWidth = (aRect.BottomRight().X() + 3) & ~3;
    int screenHeight = (aRect.BottomRight().Y() + 3) & ~3;
    int textHeight = OPENGL_SCALE_VALUE * m_iHeight / 20;
    int textWidth = (float)textHeight * ((float)screenWidth / (float)screenHeight);
    textWidth = (textWidth + 3) & ~3;
    awt::Size aSize(textWidth, textHeight);
    //clear text info
    size_t listNum = m_TextInfoList.size();
    for (size_t i = 0; i < listNum; i++)
    {
        TextInfo &textInfo = m_TextInfoList.front();
        glDeleteTextures(1, &textInfo.texture);
        m_TextInfoList.pop_front();
    }
    //create text texture
    CreateTextTexture(string, aFont, 0xFF0000, aPos, aSize, 0);
    RenderTextShape();
    */
    return 0;
}

int OpenGL3DRenderer::RenderFPS(float fps)
{
    //use width and height to get the position
    ::rtl::OUString stringFps = ::rtl::OUString("fps ") + ::rtl::OUString::number(fps);
    awt::Point aPos(0,0);
    RenderText(stringFps, aPos);
    return 0;
}

int OpenGL3DRenderer::RenderClickPos(Point aMPos)
{
    //use width and height to get the position
    ::rtl::OUString stringPos = ::rtl::OUString("(") +
                                ::rtl::OUString::number(aMPos.X()) +
                                ::rtl::OUString(",") +
                                ::rtl::OUString::number(aMPos.Y()) +
                                ::rtl::OUString(")");
    awt::Point aPos(aMPos.X(), aMPos.Y());
    RenderText(stringPos, aPos);
    return 0;
}

int OpenGL3DRenderer::ProcessUnrenderedShape()
{
    //Polygon
    RenderPolygon3DObject();
    //Shape3DExtrudeObject
    RenderExtrude3DObject();
    //render the axis
    RenderCoordinateAxis();
    return 0;
}

glm::vec4 OpenGL3DRenderer::GetColorByIndex(int index)
{
    int r = index & 0xFF;
    int g = (index >> 8) & 0xFF;
    int b = (index >> 16) & 0xFF;
    return glm::vec4(((float)r) / 255.0, ((float)g) / 255.0, ((float)b) / 255.0, 1.0);
}

int OpenGL3DRenderer::GetIndexByColor(int r, int g, int b)
{
    return r | (g << 8) | (b << 16);
}

int OpenGL3DRenderer::ProcessPickingBox()
{
    glViewport(0, 0, m_iWidth, m_iHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboID[2]);
    glClearDepth(1.0f);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    if(ProcessExtrude3DPickingBox() == 1)
    {
        //the picked object has been processed, return
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

int OpenGL3DRenderer::ProcessExtrude3DPickingBox()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //render the bounding box
    Extrude3DInfo extrude3DInfo;
    glUseProgram(m_CommonProID);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(m_2DVertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_BoundBox);
    glVertexAttribPointer(m_2DVertexID,       // attribute. No particular reason for 0, but must match the layout in the shader.
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    for (unsigned int i = 0; i < m_Extrude3DList.size(); i++)
    {
        extrude3DInfo = m_Extrude3DList[i];
        PosVecf3 trans = {extrude3DInfo.xTransform,//m_Extrude3DInfo.xTransform + 140,
                          -extrude3DInfo.yTransform,
                          extrude3DInfo.zTransform};
        PosVecf3 angle = {0.0f, 0.0f, 0.0f};
        PosVecf3 scale = {extrude3DInfo.xRange[1] - extrude3DInfo.xRange[0],
                          extrude3DInfo.yRange[1] - extrude3DInfo.yRange[0],
                          extrude3DInfo.xRange[1] - extrude3DInfo.xRange[0]};
        MoveModelf(trans, angle, scale);
        glm::mat4 boundMVP = m_3DProjection * m_3DView * m_Model;
        glm::vec4 boundColor = GetColorByIndex(i);
        int reverse = extrude3DInfo.yRange[0] > extrude3DInfo.yRange[1] ? 1 : -1;
        if (reverse < 0)
        {
            glm::mat4 reverseMatrix = glm::translate(glm::vec3(0.0, -1.0, 0.0));
            boundMVP = boundMVP * reverseMatrix;
        }
        glUniform4fv(m_2DColorID, 1, &boundColor[0]);
        glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &boundMVP[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, sizeof(boundBox) / sizeof(GLfloat) / 3);
    }
    glDisableVertexAttribArray(m_2DVertexID);
    glUseProgram(0);
    //read pixel to get the index
    Point select = Point(m_aMPos.X() / OPENGL_SCALE_VALUE, m_aMPos.Y() / OPENGL_SCALE_VALUE);
    sal_uInt8 selectColor[4] = {0};
#if 0
    int picWidth = m_iWidth - select.X();
    int picHeight = m_iHeight - select.Y();
    picWidth = (picWidth - 1) & ~3;
    picHeight = (picHeight - 1) & ~3;
    sal_uInt8 *outBuf = (sal_uInt8 *)malloc(picWidth * picHeight * 3 + BMP_HEADER_LEN);
    CreateBMPHeader(outBuf, picWidth, picHeight);
    cout << "picWidth = " << picWidth << ", picHeight = " << picHeight << endl;
    glReadPixels(select.X(), select.Y(), picWidth, picHeight, GL_RGB, GL_UNSIGNED_BYTE, outBuf + BMP_HEADER_LEN);
    char fileName[256] = {0};
    sprintf(fileName, "D:\\boundbox.bmp");
    FILE *pfile = fopen(fileName,"wb");
    fwrite(outBuf, picWidth * picHeight * 3 + BMP_HEADER_LEN, 1, pfile);
    fclose(pfile);
    cout << "select.x = " << select.X() << ", select.y = " << select.Y() << ", width = " << m_iWidth << ", height = " << m_iHeight << endl;
#endif
    glReadPixels(select.X(), select.Y(), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, selectColor);
    int selectID = GetIndexByColor(selectColor[0], selectColor[1], selectColor[2]);
//    printf("selectColor[0] = %d, selectColor[1] = %d, selectColor[2] = %d, selectID = %d\n", selectColor[0], selectColor[1], selectColor[2], selectID);

    if (selectID == RGB_WHITE)
    {
        if (m_uiSelectFrameCounter <= 0)
        {
            m_3DView = m_3DViewBack;
            m_uiSelectFrameCounter = 0;
            m_coordinateAxisinfo.pickingFlg = 0;
        }
        else
        {
            m_uiSelectFrameCounter--;
        }
        return 0;
    }
    //reset the camera by index
    //update the camera position and org
    float distance = 500;
    float veriticalAngle = GL_PI / 3.25f;
    float horizontalAngle = GL_PI / 6.0f;
    extrude3DInfo = m_Extrude3DList[selectID];
    int reverse = extrude3DInfo.yRange[0] > extrude3DInfo.yRange[1] ? 1 : -1;
    m_CameraInfo.cameraOrg = glm::vec3(extrude3DInfo.xTransform + (extrude3DInfo.xRange[1] - extrude3DInfo.xRange[0]) / 2,
                                       -extrude3DInfo.yTransform + (extrude3DInfo.yRange[1] - extrude3DInfo.yRange[0]) *reverse,
                                       extrude3DInfo.zTransform + (extrude3DInfo.xRange[1] - extrude3DInfo.xRange[0]) / 2);

    m_CameraInfo.cameraPos.x = m_CameraInfo.cameraOrg.x - distance * cos(veriticalAngle) * sin(horizontalAngle);
    m_CameraInfo.cameraPos.z = m_CameraInfo.cameraOrg.z + distance * cos(veriticalAngle) * cos(horizontalAngle);
    m_CameraInfo.cameraPos.y = m_CameraInfo.cameraOrg.y + distance * sin(horizontalAngle) * reverse;

    m_3DView = glm::lookAt(m_CameraInfo.cameraPos, // Camera is at (0,0,3), in World Space
               m_CameraInfo.cameraOrg, // and looks at the origin
               m_CameraInfo.cameraUp  // Head is up (set to 0,-1,0 to look upside-down)
               );
    m_uiSelectFrameCounter = 5;

    m_coordinateAxisinfo.pickingFlg = 1;
    m_coordinateAxisinfo.reverse = reverse;
    m_coordinateAxisinfo.trans.x = extrude3DInfo.xTransform + (extrude3DInfo.xRange[1] - extrude3DInfo.xRange[0]) / 2;
    m_coordinateAxisinfo.trans.y = -extrude3DInfo.yTransform + (extrude3DInfo.yRange[1] - extrude3DInfo.yRange[0] + 1.5) * reverse;
    m_coordinateAxisinfo.trans.z = extrude3DInfo.zTransform + (extrude3DInfo.xRange[1] - extrude3DInfo.xRange[0]) / 2;

    m_coordinateAxisinfo.scale.x = 4 * (extrude3DInfo.xRange[1] - extrude3DInfo.xRange[0]);
    m_coordinateAxisinfo.scale.y = 4 * (extrude3DInfo.xRange[1] - extrude3DInfo.xRange[0]);
    m_coordinateAxisinfo.scale.z = 4 * (extrude3DInfo.xRange[1] - extrude3DInfo.xRange[0]);
    m_coordinateAxisinfo.color = glm::vec4(0.5, 1.0, 0.8, 1.0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 1;
}

void OpenGL3DRenderer::RenderCoordinateAxis()
{
    if (!m_coordinateAxisinfo.pickingFlg)
    {
        return;
    }

    PosVecf3 angle = {0.0f, 0.0f, 0.0f};
    MoveModelf(m_coordinateAxisinfo.trans, angle, m_coordinateAxisinfo.scale);
    glm::mat4 reverseMatrix = glm::scale(glm::vec3(1.0, m_coordinateAxisinfo.reverse, 1.0));
    glm::mat4 axisMVP = m_3DProjection * m_3DView * m_Model * reverseMatrix;
    glUseProgram(m_CommonProID);
    glLineWidth(3.0);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0, 2.0);
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(m_2DVertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_CoordinateBuf);
    glVertexAttribPointer(m_2DVertexID,       // attribute. No particular reason for 0, but must match the layout in the shader.
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    glUniform4fv(m_2DColorID, 1, &m_coordinateAxisinfo.color[0]);
    glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &axisMVP[0][0]);
    glDrawArrays(GL_LINES, 0, sizeof(coordinateAxis) / sizeof(GLfloat) / 3);
    glDisableVertexAttribArray(m_2DVertexID);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glUseProgram(0);
    return;
}

int OpenGL3DRenderer::MoveModelf(PosVecf3 trans, PosVecf3 angle, PosVecf3 scale)
{
    glm::mat4 aTranslationMatrix = glm::translate(glm::vec3(trans.x, trans.y, trans.z));
    glm::mat4 aScaleMatrix = glm::scale(glm::vec3(scale.x, scale.y, scale.z));
    glm::mat4 aRotationMatrix = glm::eulerAngleYXZ(angle.y, angle.x, angle.z);
    m_Model = aTranslationMatrix * aRotationMatrix * aScaleMatrix;
    return 0;
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
