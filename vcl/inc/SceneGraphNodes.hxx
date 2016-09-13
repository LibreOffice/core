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

namespace vcl
{
namespace sg
{

class VCL_DLLPUBLIC ClippingNode : public Node
{
public:
    ClippingNode()
        : Node("ClippingNode", NodeType::CLIPPING)
    {}

    basegfx::B2DRange maClipBox;
};

class VCL_DLLPUBLIC TransformNode : public Node
{
public:
    glm::mat4 maMatrix;

    TransformNode(glm::mat4 aMatrix)
        : Node("TransformNode", NodeType::TRANSFORM)
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

    GeometryNode(GeometryNodeType eGeometryType)
        : Node("GeometryNode", NodeType::GEOMETRY)
        , meGeometryType(eGeometryType)
    {}
};

class VCL_DLLPUBLIC RectangleNode : public GeometryNode
{
public:
    basegfx::B2DRange maRectangle;
    SalColor maLineColor;
    SalColor maFillColor;
    bool mbNormalized;

    RectangleNode(basegfx::B2DRange aRectangle, SalColor aLineColor = SALCOLOR_NONE, SalColor aFillColor = SALCOLOR_NONE)
        : GeometryNode(GeometryNodeType::RECTANGLE)
        , maRectangle(aRectangle)
        , maLineColor(aLineColor)
        , maFillColor(aFillColor)
        , mbNormalized(false)
    {}
};

class VCL_DLLPUBLIC PolyPolygonNode : public GeometryNode
{
public:
    basegfx::B2DPolyPolygon maPolyPolygon;
    SalColor maLineColor;
    SalColor maFillColor;
    bool mbNormalized;

    PolyPolygonNode(basegfx::B2DPolyPolygon aPolyPolygon, SalColor aLineColor = SALCOLOR_NONE, SalColor aFillColor = SALCOLOR_NONE)
        : GeometryNode(GeometryNodeType::POLYPOLYGON)
        , maPolyPolygon(aPolyPolygon)
        , maLineColor(aLineColor)
        , maFillColor(aFillColor)
        , mbNormalized(false)
    {}
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

}} // end vcl::sg namespace

#endif // INCLUDED_VCL_INC_SCENEGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
