/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_OPENGL_SCENEGRAPHRENDERER_HXX
#define INCLUDED_VCL_OPENGL_SCENEGRAPHRENDERER_HXX

#include <vcl/dllapi.h>
#include <vcl/SceneGraph.hxx>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include "opengl/VertexUtils.hxx"
#include "opengl/RenderList.hxx"

#include "SceneGraphNodes.hxx"

class SceneGraphRenderer
{
private:
    vcl::sg::Node& mrRootNode;
    float mfWidth;
    float mfHeight;
    RenderList& mrRenderList;
    glm::mat4 maMatrix;

    bool doesOverlap(const basegfx::B2DRange& rDrawRectangle);
    void checkOverlapping(float x1, float y1, float x2, float y2);

    void processTransformNode(vcl::sg::TransformNode* pTransformNode);

    void processBitmapNode(vcl::sg::BitmapNode* pBitmapNode);
    void processRectangleNode(vcl::sg::RectangleNode* pRectangleNode);
    void processPolyPolygonNode(vcl::sg::PolyPolygonNode* pPolyPolygonNode);

    void runChildren(vcl::sg::Node& rNode);
public:
    SceneGraphRenderer(vcl::sg::Node& rRootNode, RenderList& rRenderList);
    bool render(float fWidth, float fHeight);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
