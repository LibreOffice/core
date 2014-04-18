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

#include <StaticGeometry.h>
#include "glm/gtc/matrix_inverse.hpp"
#include "CommonConverters.hxx"

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
#define RGB_WHITE (0xFF | (0xFF << 8) | (0xFF << 16))

static GLfloat squareVertices[] = {
    -1.0f, -1.0f, -1.0,
    1.0f, -1.0f, -1.0,
    1.0f,  1.0f, -1.0,
    -1.0f,  1.0f, -1.0
};

static GLfloat coordVertices[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
};

static GLfloat coordReverseVertices[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
};


static GLfloat square2DVertices[] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    1.0f,  1.0f,
    -1.0f,  1.0f
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


int OpenGLRender::AddVertexData(GLuint vertexBuf)
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3), &m_Vertices[0], GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;
}

int OpenGLRender::AddNormalData(GLuint normalBuf)
{
    glBindBuffer(GL_ARRAY_BUFFER, normalBuf);
    glBufferData(GL_ARRAY_BUFFER, m_Normals.size() * sizeof(glm::vec3), &m_Normals[0], GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;
}

int OpenGLRender::AddIndexData(GLuint indexBuf)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indeices.size() * sizeof(unsigned short), &m_Indeices[0], GL_STATIC_DRAW);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return 0;
}
bool OpenGLRender::GetSimilarVertexIndex(PackedVertex & packed,
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
void OpenGLRender::SetVertex(PackedVertex &packed,
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
void OpenGLRender::CreateActualRoundedCube(float fRadius, int iSubDivY, int iSubDivZ, float width, float height, float depth)
{
    if ((fRadius > (width / 2)) || (fRadius > (height / 2)) || (fRadius > (depth / 2)))
    {
        return;
    }
    float topThreshold = height - 2 * fRadius;
    float bottomThreshold = fRadius;

    vector<glm::vec3> vertices;
    vector<glm::vec3> normals;
    int faceNum = GenerateRoundCornerBar(vertices, normals, fRadius, iSubDivY, iSubDivZ, width, height, depth);
    std::map<PackedVertex,unsigned short> VertexToOutIndex;
    glm::vec3 actualVerteices[3];
    glm::vec3 actualNormals[3];
    vector<unsigned short> indeices[5];
    glm::vec3 externVertex[3];
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
            PackedVertex packed = {actualVerteices[k], actualNormals[k]};
            SetVertex(packed, VertexToOutIndex, m_Vertices, m_Normals, indeices[surfaceIndex]);

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
int OpenGLRender::GenerateRoundCornerBar(vector<glm::vec3> &vertices, vector<glm::vec3> &normals, float fRadius, int iSubDivY, int iSubDivZ, float width, float height, float depth)
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
            if ((fCurAngleY < 90.0) && (fNextAngleY >= 90.0) ||
                (fCurAngleY < 180.0) && (fNextAngleY >= 180.0) ||
                (fCurAngleY < 270.0) && (fNextAngleY >= 270.0) ||
                (fCurAngleY < 360.0) && (fNextAngleY >= 360.0))
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

int OpenGLRender::InitOpenGL()
{
    //TODO: moggi: get the information from the context
    mbArbMultisampleSupported = true;

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
    m_3DProID = OpenGLHelper::LoadShaders("Shape3DVertexShader", "Shape3DFragmentShader");
    m_3DProjectionID = glGetUniformLocation(m_3DProID, "P");
    m_3DViewID = glGetUniformLocation(m_3DProID, "V");
    m_3DModelID = glGetUniformLocation(m_3DProID, "M");
    m_3DNormalMatrixID = glGetUniformLocation(m_3DProID, "normalMatrix");
    m_3DVertexID = glGetAttribLocation(m_3DProID, "vertexPositionModelspace");
    m_3DNormalID = glGetAttribLocation(m_3DProID, "vertexNormalModelspace");

    Init3DUniformBlock();
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

Graphic OpenGLRender::renderToBitmap()
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
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
return aGraphic;
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
    GLenum status;
    // create a framebuffer object, you need to delete them when program exits.
    glGenFramebuffers(2, m_FboID);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    for (int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboID[i]);
        glBindTexture(GL_TEXTURE_2D, m_TextureObj[i]);
        // attach a texture to FBO color attachement point
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureObj[i], 0);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindTexture(GL_TEXTURE_2D, 0);
        // attach a renderbuffer to depth attachment point
        glBindRenderbuffer(GL_RENDERBUFFER, m_RboID[i]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RboID[i]);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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

OpenGLRender::OpenGLRender()
    : m_Model(glm::mat4(1.0f))
    , m_iWidth(0)
    , m_iHeight(0)
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
    m_Model = glm::mat4(1.0f);
    m_TranslationMatrix = glm::translate(m_Model, glm::vec3(0.0f, 0.0f, 0.0f));
    m_iPointNum = 0;
    memset(&m_Line2DPointList, 0, sizeof(Line2DPointList));
    m_FboID[0] = 0;
    m_FboID[1] = 0;
    m_TextureObj[0] = 0;
    m_TextureObj[1] = 0;
    m_RboID[0] = 0;
    m_RboID[1] = 0;
    m_iWidth = 1600;
    m_iHeight = 900;

    m_fZmax = -10000000.0f;


    memset(&m_Bubble2DCircle, 0, sizeof(m_Bubble2DCircle));

    //TODO: moggi: use STL
    for (size_t i = 0; i < sizeof(m_BackgroundColor) / sizeof(float); i++)
    {
        m_BackgroundColor[i] = 1.0;
    }
    memset(&m_Area2DPointList, 0, sizeof(m_Area2DPointList));

    //performance
    m_dFreq = 0.0;

    m_Polygon3DInfo.lineOnly = false;
    m_Polygon3DInfo.twoSidesLighting = false;
    m_Polygon3DInfo.vertices = NULL;
    m_Polygon3DInfo.uvs = NULL;
    m_Polygon3DInfo.normals = NULL;
    m_Polygon3DInfo.lineWidth = 0.001f;


    m_Extrude3DInfo.lineOnly = false;
    m_Extrude3DInfo.twoSidesLighting = false;
    m_Extrude3DInfo.lineWidth = 0.001f;

    m_fFPS = 0.0f;

    m_bCameraUpdated = false;
    GetFreq();

    for (int i = 0; i < 5; i++)
    {
        m_Extrude3DInfo.startIndex[i] = m_RoundBarMesh.iElementStartIndices[i];
        m_Extrude3DInfo.size[i] = m_RoundBarMesh.iElementSizes[i];
    }

    m_uiSelectFrameCounter = 0;
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

int OpenGLRender::CreateTextTexture(::rtl::OUString textValue, Font aFont, long , awt::Point aPos, awt::Size aSize, long rotation)
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
    awt::Size asize = awt::Size(20*bmpWidth, 20*bmpHeight);
    awt::Point aPoint = awt::Point(aTrans.Line1.Column3, aTrans.Line2.Column3);
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

void OpenGLRender::GetFreq()
{
#if _WIN32
    LARGE_INTEGER litmpold;
    QueryPerformanceFrequency(&litmpold);
    m_dFreq= litmpold.QuadPart;
#endif
}

double OpenGLRender::GetTime()
{
#if _WIN32
    LARGE_INTEGER litmpold;
    QueryPerformanceCounter(&litmpold);
    return litmpold.QuadPart*1000000 / m_dFreq;
#endif
    return 0;
}

int OpenGLRender::RenderLine3D(Polygon3DInfo &polygon)
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

int OpenGLRender::RenderPolygon3D(Polygon3DInfo &polygon)
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
    glBindBuffer(GL_UNIFORM_BUFFER, NULL);
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

int OpenGLRender::RenderPolygon3DObject()
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

void OpenGLRender::Set3DSenceInfo(glm::vec3 cameraUp,glm::mat4 D3DTrasform,sal_Bool twoSidesLighting,sal_Int32 color)
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

void OpenGLRender::SetLightInfo(sal_Bool lightOn,sal_Int32 color,glm::vec4 direction)
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

void OpenGLRender::AddShapePolygon3DObject(sal_Int32 color,sal_Bool lineOnly,sal_Int32 lineColor,long fillStyle,sal_Int32 specular)
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

void OpenGLRender::EndAddShapePolygon3DObject()
{
    m_Polygon3DInfoList.push_back(m_Polygon3DInfo);
    m_Polygon3DInfo.normals = NULL;
    m_Polygon3DInfo.vertices = NULL;
    m_Polygon3DInfo.verticesList.clear();
    m_Polygon3DInfo.normalsList.clear();
}

void OpenGLRender::AddPolygon3DObjectNormalPoint(float x, float y, float z)
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

void OpenGLRender::EndAddPolygon3DObjectNormalPoint()
{
    m_Polygon3DInfo.normalsList.push_back(m_Polygon3DInfo.normals);
    m_Polygon3DInfo.normals = NULL;
}

void OpenGLRender::AddPolygon3DObjectPoint(float x, float y, float z)
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

void OpenGLRender::EndAddPolygon3DObjectPoint()
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

void OpenGLRender::AddShape3DExtrudeObject(sal_Int32 color,sal_Int32 specular,float xTransform,float yTransform,float zTransform)
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

void OpenGLRender::EndAddShape3DExtrudeObject()
{
    m_Extrude3DInfo.pickingFlg = 0;
    m_Extrude3DList.push_back(m_Extrude3DInfo);
}

void OpenGLRender::AddExtrude3DObjectPoint(float x, float y, float z)
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

void OpenGLRender::EndAddExtrude3DObjectPoint()
{
    m_iPointNum = 0;
}

int OpenGLRender::Init3DUniformBlock()
{
    m_3DLightBlockIndex = glGetUniformBlockIndex(m_3DProID, "GlobalLights");
    m_3DMaterialBlockIndex = glGetUniformBlockIndex(m_3DProID, "GlobalMaterialParameters");

    if ((GL_INVALID_INDEX == m_3DLightBlockIndex) || (GL_INVALID_INDEX == m_3DMaterialBlockIndex))
    {
        return -1;
    }
    int nUniformBufferAlignSize = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &nUniformBufferAlignSize);
    GLint nBlockDataSizeLight = 0, nBlockDataSizeMertrial = 0;
    glGetActiveUniformBlockiv(m_3DProID, m_3DLightBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &nBlockDataSizeLight);
    glGetActiveUniformBlockiv(m_3DProID, m_3DMaterialBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &nBlockDataSizeMertrial);
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
    glUniformBlockBinding(m_3DProID, m_3DLightBlockIndex, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_3DUBOBuffer, ((nBlockDataSizeLight / nUniformBufferAlignSize) + min(nBlockDataSizeLight % nUniformBufferAlignSize, 1)) * nUniformBufferAlignSize, nBlockDataSizeMertrial);
    glUniformBlockBinding(m_3DProID, m_3DMaterialBlockIndex, 1);
    //for the light source uniform, we must calc the offset of each element
    CHECK_GL_ERROR();
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return 0;
}

int OpenGLRender::Update3DUniformBlock()
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
    glBindBuffer(GL_UNIFORM_BUFFER, NULL);
    return 0;
}
int OpenGLRender::RenderExtrudeFlatSurface(Extrude3DInfo extrude3D, int surIndex)
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
    glDrawElements(GL_TRIANGLES, extrude3D.size[surIndex], GL_UNSIGNED_SHORT, (void *)extrude3D.startIndex[surIndex]);
    return 0;
}

int OpenGLRender::RenderExtrudeBottomSurface(Extrude3DInfo extrude3D)
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
        PosVecf3 scale = {xzScale, xzScale, xzScale};
        glm::mat4 topTrans = glm::translate(glm::vec3(0.0, -actualYTrans, 0.0));
        glm::mat4 topScale = glm::scale(xzScale, xzScale, xzScale);
        m_TranslationMatrix = glm::translate(glm::vec3(trans.x, trans.y, trans.z));
        m_Model = m_TranslationMatrix * topTrans * topScale;
    }
    glm::mat3 normalMatrix(m_Model);
    glm::mat3 normalInverseTranspos = glm::inverseTranspose(normalMatrix);
    glUniformMatrix4fv(m_3DModelID, 1, GL_FALSE, &m_Model[0][0]);
    glUniformMatrix3fv(m_3DNormalMatrixID, 1, GL_FALSE, &normalInverseTranspos[0][0]);
    glDrawElements(GL_TRIANGLES, extrude3D.size[BOTTOM_SURFACE], GL_UNSIGNED_SHORT, (void *)extrude3D.startIndex[BOTTOM_SURFACE]);
    return 0;
}

int OpenGLRender::RenderExtrudeMiddleSurface(Extrude3DInfo extrude3D)
{
    float xzScale = extrude3D.xRange[1] - extrude3D.xRange[0];
    float yScale = extrude3D.yRange[1] - extrude3D.yRange[0];
    float actualYScale = yScale - m_RoundBarMesh.bottomThreshold * xzScale;
    PosVecf3 trans = {extrude3D.xTransform,//m_Extrude3DInfo.xTransform + 140,
                      -extrude3D.yTransform,
                      extrude3D.zTransform};
    PosVecf3 angle = {0.0f, 0.0f, 0.0f};
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
    glDrawElements(GL_TRIANGLES, extrude3D.size[MIDDLE_SURFACE], GL_UNSIGNED_SHORT, (void *)extrude3D.startIndex[MIDDLE_SURFACE]);
    return 0;
}

int OpenGLRender::RenderExtrudeTopSurface(Extrude3DInfo extrude3D)
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
    glDrawElements(GL_TRIANGLES, extrude3D.size[TOP_SURFACE], GL_UNSIGNED_SHORT, (void *)extrude3D.startIndex[TOP_SURFACE]);
    RenderExtrudeFlatSurface(extrude3D, FLAT_BOTTOM_SURFACE);
    return 0;
}

int OpenGLRender::RenderExtrudeSurface(Extrude3DInfo extrude3D)
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

int OpenGLRender::RenderExtrude3DObject()
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
        glBindBuffer(GL_UNIFORM_BUFFER, NULL);
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

void OpenGLRender::SetFPS(float fps)
{
    m_fFPS = fps;
}

void OpenGLRender::SetClickPos(Point aMPos)
{
    m_aMPos = aMPos;
}


int OpenGLRender::RenderText(::rtl::OUString string, awt::Point aPos)
{
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
    int listNum = m_TextInfoList.size();
    for (int i = 0; i < listNum; i++)
    {
        TextInfo &textInfo = m_TextInfoList.front();
        glDeleteTextures(1, &textInfo.texture);
        m_TextInfoList.pop_front();
    }
    //create text texture
    CreateTextTexture(string, aFont, 0xFF0000, aPos, aSize, 0);
    RenderTextShape();
    return 0;
}

int OpenGLRender::RenderFPS(float fps)
{
    //use width and height to get the position
    ::rtl::OUString stringFps = ::rtl::OUString("fps ") + ::rtl::OUString::number(fps);
    awt::Point aPos(0,0);
    RenderText(stringFps, aPos);
    return 0;
}

int OpenGLRender::RenderClickPos(Point aMPos)
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

int OpenGLRender::ProcessUnrenderedShape()
{
    //Polygon
    RenderPolygon3DObject();
    //Shape3DExtrudeObject
    RenderExtrude3DObject();
    //render the axis
    RenderCoordinateAxis();
    return 0;
}

glm::vec4 OpenGLRender::GetColorByIndex(int index)
{
    int r = index & 0xFF;
    int g = (index >> 8) & 0xFF;
    int b = (index >> 16) & 0xFF;
    return glm::vec4(((float)r) / 255.0, ((float)g) / 255.0, ((float)b) / 255.0, 1.0);
}

int OpenGLRender::GetIndexByColor(int r, int g, int b)
{
    return r | (g << 8) | (b << 16);
}

int OpenGLRender::ProcessPickingBox()
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

int OpenGLRender::ProcessExtrude3DPickingBox()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //render the bounding box
    int extrudeNum = m_Extrude3DList.size();
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
    BYTE selectColor[4] = {0};
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

void OpenGLRender::RenderCoordinateAxis()
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
