/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef VCL_INC_SCENEGRAPHNODES_HXX
#define VCL_INC_SCENEGRAPHNODES_HXX

#include <vcl/SceneGraph.hxx>

#include <glm/glm.hpp>
#include <basegfx/range/b2drange.hxx>
#include <o3tl/make_unique.hxx>

namespace vcl
{
namespace sg
{

class VCL_DLLPUBLIC ClippingNode : public Node
{
public:
    ClippingNode()
        : Node(NodeType::CLIPPING)
    {}

    basegfx::B2DRange maClipBox;
};

class VCL_DLLPUBLIC TransformNode : public Node
{
public:
    glm::mat4 maMatrix;

    TransformNode(glm::mat4 aMatrix)
        : Node(NodeType::TRANSFORM)
        , maMatrix(aMatrix)
    {}
};

enum class GeometryNodeType
{
    RECTANGLE,
    POLYPOLYGON,
    BITMAP
};

class VCL_DLLPUBLIC GeometryNode : public Node
{
public:
    GeometryNodeType meGeometryType;
    bool mbNormalized;

    GeometryNode(GeometryNodeType eGeometryType)
        : Node(NodeType::GEOMETRY)
        , meGeometryType(eGeometryType)
        , mbNormalized(false)
    {}


protected:
    void markAsModified()
    {}
};

struct GeometryAttributes
{
    SalColor maLineColor;
    SalColor maFillColor;

    GeometryAttributes(SalColor aLineColor = SALCOLOR_NONE, SalColor aFillColor = SALCOLOR_NONE)
        : maLineColor(aLineColor)
        , maFillColor(aFillColor)
    {}
};

class VCL_DLLPUBLIC RectangleNode : public GeometryNode
{
private:
    basegfx::B2DRange maRectangle;
    GeometryAttributes maAttributes;

public:
    RectangleNode(const basegfx::B2DRange& rRectangle, const GeometryAttributes& rAttributes)
        : GeometryNode(GeometryNodeType::RECTANGLE)
        , maRectangle(rRectangle)
        , maAttributes(rAttributes)
    {}

    const GeometryAttributes& getAttributes()
    {
        return maAttributes;
    }

    const basegfx::B2DRange& getRectangle()
    {
        return maRectangle;
    }

    void modifyAttributes(GeometryAttributes& rAttributes)
    {
        maAttributes = rAttributes;
        markAsModified();
    }
};

class VCL_DLLPUBLIC PolyPolygonNode : public GeometryNode
{
private:
    basegfx::B2DPolyPolygon maPolyPolygon;
    GeometryAttributes maAttributes;

public:
    PolyPolygonNode(const basegfx::B2DPolyPolygon& rPolyPolygon, const GeometryAttributes& rAttributes)
        : GeometryNode(GeometryNodeType::POLYPOLYGON)
        , maPolyPolygon(rPolyPolygon)
        , maAttributes(rAttributes)
    {}

    const GeometryAttributes& getAttributes()
    {
        return maAttributes;
    }

    const basegfx::B2DPolyPolygon& getPolyPolygon()
    {
        return maPolyPolygon;
    }

    void modifyAttributes(GeometryAttributes& rAttributes)
    {
        maAttributes = rAttributes;
        markAsModified();
    }
};

class VCL_DLLPUBLIC BitmapNode : public GeometryNode
{
public:
    Bitmap maBitmap;
    basegfx::B2DRange maRectangle;

    BitmapNode(Bitmap aBitmap, basegfx::B2DRange aRectangle)
        : GeometryNode(GeometryNodeType::BITMAP)
        , maBitmap(aBitmap)
        , maRectangle(aRectangle)
    {}
};

class SceneGraphFactory
{
private:
    std::vector<std::shared_ptr<Node>> maNodeStack;

public:
    SceneGraphFactory(const std::shared_ptr<Node>& pRootNode)
        : maNodeStack({pRootNode})
    {}

    Node& getCurrent()
    {
        return *maNodeStack.back().get();
    }

    void pushTransform(glm::mat4 aMatrix, const OUString& rName)
    {
        auto pTransformNode = std::make_shared<vcl::sg::TransformNode>(aMatrix);
        pTransformNode->setName(rName);
        getCurrent().mChildren.push_back(pTransformNode);
        maNodeStack.push_back(pTransformNode);
    }

    void pop()
    {
        maNodeStack.pop_back();
    }

    void addRectangle(const basegfx::B2DRange& rRange, SalColor nLineColor, SalColor nFillColor)
    {
        GeometryAttributes aAttributes(nLineColor, nFillColor);
        getCurrent().mChildren.push_back(std::make_shared<RectangleNode>(rRange, aAttributes));
    }

    void addNormalizedRectangle(const basegfx::B2DRange& rRange, SalColor nLineColor, SalColor nFillColor)
    {
        addRectangle(rRange, nLineColor, nFillColor);
        std::static_pointer_cast<RectangleNode>(getCurrent().mChildren.back())->mbNormalized = true;
    }

    void addBitmap(Bitmap& rBitmap, const basegfx::B2DRange& rRange)
    {
        getCurrent().mChildren.push_back(std::make_shared<BitmapNode>(rBitmap, rRange));
    }

    void addPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, SalColor nLineColor, SalColor nFillColor)
    {
        GeometryAttributes aAttributes(nLineColor, nFillColor);
        getCurrent().mChildren.push_back(std::make_shared<PolyPolygonNode>(rPolyPolygon, aAttributes));
    }
};

}} // end vcl::sg namespace

#endif // INCLUDED_VCL_INC_SCENEGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
