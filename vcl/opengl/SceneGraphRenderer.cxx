/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "opengl/SceneGraphRenderer.hxx"
#include <vcl/SceneGraph.hxx>

#include "opengl/LineRenderUtils.hxx"

#include <glm/gtx/string_cast.hpp>

#include "impbmp.hxx"
#include "opengl/salbmp.hxx"

#include <basegfx/polygon/b2dtrapezoid.hxx>

namespace {

void appendRectangle(std::vector<Vertex>& rVertices, std::vector<GLuint>& rIndices,
                     GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
                     SalColor nColor, GLfloat fTransparency)
{
    GLubyte nR, nG, nB, nA;
    vcl::vertex::createColor(nColor, fTransparency, nR, nG, nB, nA);

    GLuint zero = rVertices.size();

    rVertices.insert(rVertices.end(), {
        Vertex{glm::vec2{x1, y1}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x2, y1}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x1, y2}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x2, y2}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
    });

    rIndices.insert(rIndices.end(), {
        zero + 0, zero + 1, zero + 2,
        zero + 2, zero + 1, zero + 3
    });
}

void addTextureRectangle(std::vector<TextureVertex>& rVertices, std::vector<GLuint>& rIndices,
                     GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
                     GLfloat tx1, GLfloat ty1, GLfloat tx2, GLfloat ty2,
                     SalColor nColor, GLfloat fTransparency)
{
    GLubyte nR, nG, nB, nA;
    vcl::vertex::createColor(nColor, fTransparency, nR, nG, nB, nA);

    GLuint zero = rVertices.size();

    rVertices.insert(rVertices.end(), {
        TextureVertex{glm::vec2{x1, y1}, glm::vec2{tx1, ty1}, glm::vec2{tx1, ty1}, glm::vec2{tx1, ty1}, glm::vec4{nR, nG, nB, nA}},
        TextureVertex{glm::vec2{x2, y1}, glm::vec2{tx2, ty1}, glm::vec2{tx2, ty1}, glm::vec2{tx2, ty1}, glm::vec4{nR, nG, nB, nA}},
        TextureVertex{glm::vec2{x1, y2}, glm::vec2{tx1, ty2}, glm::vec2{tx1, ty2}, glm::vec2{tx1, ty2}, glm::vec4{nR, nG, nB, nA}},
        TextureVertex{glm::vec2{x2, y2}, glm::vec2{tx2, ty2}, glm::vec2{tx2, ty2}, glm::vec2{tx2, ty2}, glm::vec4{nR, nG, nB, nA}},
    });

    rIndices.insert(rIndices.end(), {
        zero + 0, zero + 1, zero + 2,
        zero + 2, zero + 1, zero + 3
    });
}

void appendRectangle(RenderEntry& rRenderEntry,
                     GLfloat fX1, GLfloat fY1, GLfloat fX2, GLfloat fY2,
                     SalColor nLineColor, SalColor nFillColor, GLfloat fTransparency)
{
    if (nLineColor == SALCOLOR_NONE && nFillColor == SALCOLOR_NONE)
        return;
    if (fTransparency == 1.0f)
        return;

    RenderParameters& rRenderParameter = rRenderEntry.maTriangleParameters;

    // Draw rectangle stroke with line color
    if (nLineColor != SALCOLOR_NONE)
    {
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                    fX1 - 0.5f, fY1 - 0.5f, fX1 + 0.5f, fY2 + 0.5f, nLineColor, fTransparency);
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                    fX1 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY1 + 0.5f, nLineColor, fTransparency);
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                    fX2 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY2 + 0.5f, nLineColor, fTransparency);
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                    fX1 - 0.5f, fY2 - 0.5f, fX2 + 0.5f, fY2 + 0.5f, nLineColor, fTransparency);
    }

    if (nFillColor != SALCOLOR_NONE)
    {
        if (nLineColor == SALCOLOR_NONE)
        {
            appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX1 - 0.5f, fY1 - 0.5f, fX1 + 0.5f, fY2 + 0.5f, nFillColor, fTransparency);
            appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX1 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY1 + 0.5f, nFillColor, fTransparency);
            appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX2 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY2 + 0.5f, nFillColor, fTransparency);
            appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX1 - 0.5f, fY2 - 0.5f, fX2 + 0.5f, fY2 + 0.5f, nFillColor, fTransparency);
        }
        // Draw rectangle fill with fill color
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX1 + 0.5f, fY1 + 0.5f, fX2 - 0.5f, fY2 - 0.5f, nFillColor, fTransparency);
    }
}

bool addTextureDraw(RenderEntry& rRenderEntry, OpenGLTexture& rTexture, const SalTwoRect& r2Rect)
{
    if (!rTexture)
        return false;

    GLfloat x1 = r2Rect.mnDestX;
    GLfloat y1 = r2Rect.mnDestY;
    GLfloat x2 = x1 + r2Rect.mnDestWidth;
    GLfloat y2 = y1 + r2Rect.mnDestHeight;

    GLuint nTextureId = rTexture.Id();

    TextureParameters& rTextureParameter = rRenderEntry.maTextureMap[nTextureId];
    rTextureParameter.maTexture = rTexture;

    float tx1 = 0.0f;
    float ty1 = 0.0f;
    float tx2 = 0.0f;
    float ty2 = 0.0f;

    rTextureParameter.maTexture.GetTextureRect(r2Rect, false, tx1, tx2, ty1, ty2);

    addTextureRectangle(rTextureParameter.maVertices, rTextureParameter.maIndices,
                        x1, y1, x2, y2, tx1, ty1, tx2, ty2,
                        SALCOLOR_NONE, 0.0f);

    return true;
}

inline void appendTrapezoid(std::vector<Vertex>& rVertices, std::vector<GLuint>& rIndices,
                       GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
                       GLfloat x3, GLfloat y3, GLfloat x4, GLfloat y4,
                       SalColor nColor, GLfloat fTransparency)
{
    GLubyte nR, nG, nB, nA;
    vcl::vertex::createColor(nColor, fTransparency, nR, nG, nB, nA);

    GLuint zero = rVertices.size();

    rVertices.insert(rVertices.end(), {
        Vertex{glm::vec2{x1, y1}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x2, y2}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x3, y3}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x4, y4}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
    });

    rIndices.insert(rIndices.end(), {
        zero + 0, zero + 1, zero + 2,
        zero + 2, zero + 1, zero + 3
    });
}

void addDrawPolyPolygon(RenderEntry& rRenderEntry,const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency,
                        SalColor nLineColor, SalColor nFillColor, bool bUseAA)
{
    if (rPolyPolygon.count() <= 0)
        return;
    if (nLineColor == SALCOLOR_NONE && nFillColor == SALCOLOR_NONE)
        return;
    if (fTransparency == 1.0)
        return;

    RenderParameters& rLineRenderParameter = rRenderEntry.maLineParameters;
    RenderParameters& rTriangleRenderParameter = rRenderEntry.maTriangleParameters;

    if (nFillColor != SALCOLOR_NONE)
    {
        basegfx::B2DTrapezoidVector aTrapezoidVector;
        basegfx::tools::trapezoidSubdivide(aTrapezoidVector, rPolyPolygon);

        if (!aTrapezoidVector.empty())
        {
            for (basegfx::B2DTrapezoid & rTrapezoid : aTrapezoidVector)
            {
                GLfloat topX1 = rTrapezoid.getTopXLeft();
                GLfloat topX2 = rTrapezoid.getTopXRight();
                GLfloat topY  = rTrapezoid.getTopY();

                GLfloat bottomX1 = rTrapezoid.getBottomXLeft();
                GLfloat bottomX2 = rTrapezoid.getBottomXRight();
                GLfloat bottomY  = rTrapezoid.getBottomY();

                appendTrapezoid(rTriangleRenderParameter.maVertices, rTriangleRenderParameter.maIndices,
                                topX1,    topY, topX2,    topY,
                                bottomX1, bottomY, bottomX2, bottomY,
                                nFillColor, fTransparency);
            }
        }
    }

    if (nLineColor != SALCOLOR_NONE || bUseAA)
    {
        SalColor nColor = (nLineColor == SALCOLOR_NONE) ? nFillColor : nLineColor;

        vcl::LineBuilder aBuilder(rLineRenderParameter.maVertices, rLineRenderParameter.maIndices,
                                  nColor, fTransparency, 1.0f, bUseAA);

        for (const basegfx::B2DPolygon& rPolygon : rPolyPolygon)
        {
            basegfx::B2DPolygon aPolygon(rPolygon);
            if (rPolygon.areControlPointsUsed())
                aPolygon = rPolygon.getDefaultAdaptiveSubdivision();

            sal_uInt32 nPoints = aPolygon.count();

            GLfloat x1, y1, x2, y2;
            sal_uInt32 index1, index2;

            for (sal_uInt32 i = 0; i <= nPoints; ++i)
            {
                index1 = (i)     % nPoints;
                index2 = (i + 1) % nPoints;

                x1 = aPolygon.getB2DPoint(index1).getX();
                y1 = aPolygon.getB2DPoint(index1).getY();
                x2 = aPolygon.getB2DPoint(index2).getX();
                y2 = aPolygon.getB2DPoint(index2).getY();

                aBuilder.appendLine(glm::vec2(x1, y1), glm::vec2(x2, y2));
            }
        }
    }
}

} // end anonymous namespace

SceneGraphRenderer::SceneGraphRenderer(vcl::sg::Node& rRootNode, RenderList& rRenderList)
    : mrRootNode(rRootNode)
    , mrRenderList(rRenderList)
{}

bool SceneGraphRenderer::doesOverlap(const basegfx::B2DRange& rDrawRectangle)
{
    return mrRenderList.getEntries().back().maOverlapTrackingRectangle.overlaps(rDrawRectangle);
}

void SceneGraphRenderer::checkOverlapping(float x1, float y1, float x2, float y2)
{
    auto& rRenderEntries = mrRenderList.getEntries();

    basegfx::B2DRange aRectangle(x1, y1, x2, y2);

    if (rRenderEntries.empty() || doesOverlap(aRectangle))
    {
        rRenderEntries.resize(rRenderEntries.size() + 1);
        rRenderEntries.back().maOverlapTrackingRectangle = aRectangle;
        rRenderEntries.back().maMatrix = maMatrix;
    }
    else
    {
        rRenderEntries.back().maOverlapTrackingRectangle.expand(aRectangle);
    }
}

void SceneGraphRenderer::processTransformNode(vcl::sg::TransformNode* pTransformNode)
{
    glm::mat4 oldMatrix = maMatrix;
    maMatrix = maMatrix * pTransformNode->maMatrix;

    auto& rRenderEntries = mrRenderList.getEntries();
    rRenderEntries.resize(rRenderEntries.size() + 1);
    rRenderEntries.back().maMatrix = maMatrix;

    runChildren(*pTransformNode);

    maMatrix = oldMatrix;
    rRenderEntries.resize(rRenderEntries.size() + 1);
    rRenderEntries.back().maMatrix = maMatrix;
}

void SceneGraphRenderer::processBitmapNode(vcl::sg::BitmapNode* pBitmapNode)
{
    SalBitmap* pSalBitmap = pBitmapNode->maBitmap.ImplGetImpBitmap()->ImplGetSalBitmap();
    OpenGLSalBitmap* pGLBitmap = static_cast<OpenGLSalBitmap*>(pSalBitmap);

    float fDestX = pBitmapNode->maRectangle.getMinX();
    float fDestY = pBitmapNode->maRectangle.getMinY();
    float fDestW = pBitmapNode->maRectangle.getWidth();
    float fDestH = pBitmapNode->maRectangle.getHeight();

    float fSrcX = 0.0f;
    float fSrcY = 0.0f;
    float fSrcW = pGLBitmap->GetSize().Width();
    float fSrcH = pGLBitmap->GetSize().Height();

    SalTwoRect a2Rect(fSrcX, fSrcY, fSrcH, fSrcW, fDestX, fDestY, fDestH, fDestW);

    RenderEntry& rRenderEntry = mrRenderList.getEntries().back();

    addTextureDraw(rRenderEntry, pGLBitmap->GetTexture(), a2Rect);
}

void SceneGraphRenderer::processRectangleNode(vcl::sg::RectangleNode* pRectangleNode)
{
    const vcl::sg::GeometryAttributes& rAttributes = pRectangleNode->getAttributes();
    const basegfx::B2DRange& rRectangle = pRectangleNode->getRectangle();

    float x1 = rRectangle.getMinX();
    float y1 = rRectangle.getMinY();
    float x2 = rRectangle.getMaxX();
    float y2 = rRectangle.getMaxY();

    if (pRectangleNode->mbNormalized)
    {
        x1 *= mfWidth;
        x2 *= mfWidth;
        y1 *= mfHeight;
        y2 *= mfHeight;
    }

    checkOverlapping(x1, y1, x2, y2);

    RenderEntry& rRenderEntry = mrRenderList.getEntries().back();

    appendRectangle(rRenderEntry, x1, y1, x2, y2, rAttributes.maLineColor, rAttributes.maFillColor, 0.0f);
}

void SceneGraphRenderer::processPolyPolygonNode(vcl::sg::PolyPolygonNode* pPolyPolygonNode)
{
    const vcl::sg::GeometryAttributes& rAttributes = pPolyPolygonNode->getAttributes();

    const basegfx::B2DPolyPolygon& rPolyPolygon(pPolyPolygonNode->getPolyPolygon());
    basegfx::B2DRange aBoundingRect(rPolyPolygon.getB2DRange());

    float x1 = aBoundingRect.getMinX();
    float y1 = aBoundingRect.getMinY();
    float x2 = aBoundingRect.getMaxX();
    float y2 = aBoundingRect.getMaxY();

    checkOverlapping(x1, y1, x2, y2);

    RenderEntry& rRenderEntry = mrRenderList.getEntries().back();

    addDrawPolyPolygon(rRenderEntry, rPolyPolygon, 0.0, rAttributes.maLineColor, rAttributes.maFillColor, true);
}

void SceneGraphRenderer::runChildren(vcl::sg::Node& rNode)
{
    for (auto& pChild : rNode.mChildren)
    {
        switch (pChild->meType)
        {
            case vcl::sg::NodeType::TRANSFORM:
            {
                processTransformNode(static_cast<vcl::sg::TransformNode*>(pChild.get()));
            }
            break;

            case vcl::sg::NodeType::GEOMETRY:
            {
                vcl::sg::GeometryNode* pGeometryNode = static_cast<vcl::sg::GeometryNode*>(pChild.get());
                switch (pGeometryNode->meGeometryType)
                {
                    case vcl::sg::GeometryNodeType::BITMAP:
                    {
                        processBitmapNode(static_cast<vcl::sg::BitmapNode*>(pGeometryNode));
                    }
                    break;

                    case vcl::sg::GeometryNodeType::RECTANGLE:
                    {
                        processRectangleNode(static_cast<vcl::sg::RectangleNode*>(pGeometryNode));
                    }
                    break;

                    case vcl::sg::GeometryNodeType::POLYPOLYGON:
                    {
                        processPolyPolygonNode(static_cast<vcl::sg::PolyPolygonNode*>(pGeometryNode));
                    }
                    break;

                    default:
                        break;
                }
            }
            break;

            default:
                break;
        }
    }
}

bool SceneGraphRenderer::render(float fWidth, float fHeight)
{
    mfWidth = fWidth;
    mfHeight = fHeight;

    runChildren(mrRootNode);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
