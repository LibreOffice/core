/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Vector3DContent.hxx"

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlightattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightingattribute3d.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrobjectattribute3d.hxx>
#include <drawinglayer/attribute/sdrsceneattribute3d.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/primitive3d/sdrextrudeprimitive3d.hxx>
#include <drawinglayer/primitive3d/sdrlatheprimitive3d.hxx>
#include <drawinglayer/primitive3d/sdrpolypolygonprimitive3d.hxx>
#include <drawinglayer/primitive3d/sdrsphereprimitive3d.hxx>
#include <o3tl/unit_conversion.hxx>
#include <sdr/contact/viewcontactofe3d.hxx>
#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <tools/color.hxx>
#include <tools/json_writer.hxx>

namespace svx::vector
{
namespace
{
/// the drawing layer works in 1/100 mm, the page is told in twips
constexpr double constTwipsPerUnit = o3tl::convert(1.0, o3tl::Length::mm100, o3tl::Length::twip);

ObjectContent::Scene3D sceneOf(const E3dScene& rScene)
{
    const auto& rContact
        = static_cast<const sdr::contact::ViewContactOfE3dScene&>(rScene.GetViewContact());
    const drawinglayer::geometry::ViewInformation3D& rView = rContact.getViewInformation3D();
    const drawinglayer::attribute::SdrSceneAttribute& rLook = rContact.getSdrSceneAttribute();
    const drawinglayer::attribute::SdrLightingAttribute& rLighting
        = rContact.getSdrLightingAttribute();

    ObjectContent::Scene3D aScene;
    aScene.maPlacement
        = basegfx::utils::createScaleB2DHomMatrix(constTwipsPerUnit, constTwipsPerUnit)
          * rContact.getObjectTransformation();
    aScene.maTransformation = rView.getObjectTransformation();
    aScene.maOrientation = rView.getOrientation();
    aScene.maProjection = rView.getProjection();
    aScene.maDeviceToView = rView.getDeviceToView();
    aScene.mbPerspective
        = css::drawing::ProjectionMode_PERSPECTIVE == rLook.getProjectionMode();
    aScene.meShadeMode = rLook.getShadeMode();
    aScene.mbTwoSidedLighting = rLook.getTwoSidedLighting();
    aScene.maAmbientLight = rLighting.getAmbientLightColor();
    // the lights switched off are left out already
    for (const drawinglayer::attribute::Sdr3DLightAttribute& rLight : rLighting.getLightVector())
        aScene.maLights.push_back({ rLight.getColor(), rLight.getDirection(), rLight.getSpecular() });
    return aScene;
}

OString hexOf(const basegfx::BColor& rColor)
{
    return "#" + Color(rColor).AsRGBHexString().toUtf8();
}

void putColor(tools::JsonWriter& rWriter, std::string_view aName, const basegfx::BColor& rColor)
{
    rWriter.put(aName, hexOf(rColor));
}

/// row by row, so that the last row is the one of the perspective
void putMatrix(tools::JsonWriter& rWriter, std::string_view aName,
               const basegfx::B3DHomMatrix& rMatrix)
{
    auto aArray = rWriter.startArray(aName);
    for (sal_uInt16 nRow = 0; nRow < 4; ++nRow)
        for (sal_uInt16 nColumn = 0; nColumn < 4; ++nColumn)
            rWriter.putSimpleValue(rMatrix.get(nRow, nColumn));
}

void putTriple(tools::JsonWriter& rWriter, std::string_view aName, const basegfx::B3DTuple& rTuple)
{
    auto aArray = rWriter.startArray(aName);
    rWriter.putSimpleValue(rTuple.getX());
    rWriter.putSimpleValue(rTuple.getY());
    rWriter.putSimpleValue(rTuple.getZ());
}

void writeScene(tools::JsonWriter& rWriter, const ObjectContent::Scene3D& rScene)
{
    auto aNode = rWriter.startNode("scene3D");
    {
        const basegfx::B2DHomMatrix& rPlacement = rScene.maPlacement;
        auto aArray = rWriter.startArray("placement");
        for (double fValue : { rPlacement.get(0, 0), rPlacement.get(1, 0), rPlacement.get(0, 1),
                               rPlacement.get(1, 1), rPlacement.get(0, 2), rPlacement.get(1, 2) })
            rWriter.putSimpleValue(fValue);
    }
    putMatrix(rWriter, "transformation", rScene.maTransformation);
    putMatrix(rWriter, "orientation", rScene.maOrientation);
    putMatrix(rWriter, "projection", rScene.maProjection);
    putMatrix(rWriter, "deviceToView", rScene.maDeviceToView);
    rWriter.put("perspective", rScene.mbPerspective);
    rWriter.put("shadeMode", static_cast<sal_Int32>(rScene.meShadeMode));
    if (rScene.mbTwoSidedLighting)
        rWriter.put("twoSidedLighting", true);
    putColor(rWriter, "ambientLight", rScene.maAmbientLight);
    auto aLights = rWriter.startArray("lights");
    for (const ObjectContent::Scene3D::Light& rLight : rScene.maLights)
    {
        auto aLight = rWriter.startStruct();
        putColor(rWriter, "color", rLight.maColor);
        putTriple(rWriter, "direction", rLight.maDirection);
        if (rLight.mbSpecular)
            rWriter.put("specular", true);
    }
}

/// the outline as the slices are made from it: curves broken into lines, no double points, the
/// outer polygons running one way and the holes the other, the outermost one first
basegfx::B2DPolyPolygon preparedOutline(const basegfx::B2DPolyPolygon& rOutline)
{
    basegfx::B2DPolyPolygon aPrepared(basegfx::utils::adaptiveSubdivideByAngle(rOutline));
    aPrepared.removeDoublePoints();
    aPrepared = basegfx::utils::correctOrientations(aPrepared);
    return basegfx::utils::correctOutmostPolygon(aPrepared);
}

void putOutline(tools::JsonWriter& rWriter, const basegfx::B2DPolyPolygon& rOutline)
{
    auto aPolygons = rWriter.startArray("outline");
    for (const basegfx::B2DPolygon& rPolygon : rOutline)
    {
        auto aPolygon = rWriter.startStruct();
        rWriter.put("closed", rPolygon.isClosed());
        auto aPoints = rWriter.startArray("points");
        for (sal_uInt32 nPoint = 0; nPoint < rPolygon.count(); ++nPoint)
        {
            rWriter.putSimpleValue(rPolygon.getB2DPoint(nPoint).getX());
            rWriter.putSimpleValue(rPolygon.getB2DPoint(nPoint).getY());
        }
    }
}

void putPolygons3D(tools::JsonWriter& rWriter, const basegfx::B3DPolyPolygon& rPolygons)
{
    auto aPolygons = rWriter.startArray("polygons");
    for (sal_uInt32 nPolygon = 0; nPolygon < rPolygons.count(); ++nPolygon)
    {
        const basegfx::B3DPolygon aPolygon(rPolygons.getB3DPolygon(nPolygon));
        auto aEntry = rWriter.startStruct();
        rWriter.put("closed", aPolygon.isClosed());
        {
            auto aPoints = rWriter.startArray("points");
            for (sal_uInt32 nPoint = 0; nPoint < aPolygon.count(); ++nPoint)
            {
                const basegfx::B3DPoint aPoint(aPolygon.getB3DPoint(nPoint));
                rWriter.putSimpleValue(aPoint.getX());
                rWriter.putSimpleValue(aPoint.getY());
                rWriter.putSimpleValue(aPoint.getZ());
            }
        }
        if (aPolygon.areNormalsUsed())
        {
            auto aNormals = rWriter.startArray("normals");
            for (sal_uInt32 nPoint = 0; nPoint < aPolygon.count(); ++nPoint)
            {
                const basegfx::B3DVector aNormal(aPolygon.getNormal(nPoint));
                rWriter.putSimpleValue(aNormal.getX());
                rWriter.putSimpleValue(aNormal.getY());
                rWriter.putSimpleValue(aNormal.getZ());
            }
        }
        if (aPolygon.areTextureCoordinatesUsed())
        {
            auto aCoordinates = rWriter.startArray("textureCoordinates");
            for (sal_uInt32 nPoint = 0; nPoint < aPolygon.count(); ++nPoint)
            {
                const basegfx::B2DPoint aCoordinate(aPolygon.getTextureCoordinate(nPoint));
                rWriter.putSimpleValue(aCoordinate.getX());
                rWriter.putSimpleValue(aCoordinate.getY());
            }
        }
    }
}

/// what an object looks like: its fill, line and shadow, and how the light falls on it
void putLook(tools::JsonWriter& rWriter, const drawinglayer::primitive3d::SdrPrimitive3D& rPrimitive)
{
    {
        auto aSize = rWriter.startArray("textureSize");
        rWriter.putSimpleValue(rPrimitive.getTextureSize().getX());
        rWriter.putSimpleValue(rPrimitive.getTextureSize().getY());
    }

    const drawinglayer::attribute::SdrLineFillShadowAttribute3D& rLook
        = rPrimitive.getSdrLFSAttribute();
    if (const drawinglayer::attribute::SdrFillAttribute& rFill = rLook.getFill(); !rFill.isDefault())
    {
        auto aFill = rWriter.startNode("fill");
        putColor(rWriter, "color", rFill.getColor());
        if (rFill.getTransparence() > 0.0)
            rWriter.put("transparence", rFill.getTransparence());
        // the kind is told now, a gradient, a hatch or a picture on a 3D object comes later
        if (!rFill.getGradient().isDefault())
            rWriter.put("kind", "gradient");
        else if (!rFill.getHatch().isDefault())
            rWriter.put("kind", "hatch");
        else if (!rFill.getFillGraphic().isDefault())
            rWriter.put("kind", "graphic");
    }
    if (const drawinglayer::attribute::SdrLineAttribute& rLine = rLook.getLine(); !rLine.isDefault())
    {
        auto aLine = rWriter.startNode("line");
        putColor(rWriter, "color", rLine.getColor());
        rWriter.put("width", rLine.getWidth() * constTwipsPerUnit);
        if (rLine.getTransparence() > 0.0)
            rWriter.put("transparence", rLine.getTransparence());
    }
    if (const drawinglayer::attribute::SdrShadowAttribute& rShadow = rLook.getShadow();
        !rShadow.isDefault())
    {
        auto aShadow = rWriter.startNode("shadow");
        putColor(rWriter, "color", rShadow.getColor());
        rWriter.put("offsetX", rShadow.getOffset().getX() * constTwipsPerUnit);
        rWriter.put("offsetY", rShadow.getOffset().getY() * constTwipsPerUnit);
        if (rShadow.getTransparence() > 0.0)
            rWriter.put("transparence", rShadow.getTransparence());
    }

    const drawinglayer::attribute::Sdr3DObjectAttribute& rObject
        = rPrimitive.getSdr3DObjectAttribute();
    auto aSurface = rWriter.startNode("surface");
    rWriter.put("normalsKind", static_cast<sal_Int32>(rObject.getNormalsKind()));
    if (rObject.getNormalsInvert())
        rWriter.put("normalsInvert", true);
    rWriter.put("textureProjectionX", static_cast<sal_Int32>(rObject.getTextureProjectionX()));
    rWriter.put("textureProjectionY", static_cast<sal_Int32>(rObject.getTextureProjectionY()));
    rWriter.put("textureKind", static_cast<sal_Int32>(rObject.getTextureKind()));
    rWriter.put("textureMode", static_cast<sal_Int32>(rObject.getTextureMode()));
    if (rObject.getDoubleSided())
        rWriter.put("doubleSided", true);
    if (rObject.getShadow3D())
        rWriter.put("shadow3D", true);
    if (rObject.getTextureFilter())
        rWriter.put("textureFilter", true);
    if (rObject.getReducedLineGeometry())
        rWriter.put("reducedLineGeometry", true);
    const drawinglayer::attribute::MaterialAttribute3D& rMaterial = rObject.getMaterial();
    auto aMaterial = rWriter.startNode("material");
    putColor(rWriter, "color", rMaterial.getColor());
    putColor(rWriter, "specular", rMaterial.getSpecular());
    putColor(rWriter, "emission", rMaterial.getEmission());
    rWriter.put("specularIntensity", static_cast<sal_Int32>(rMaterial.getSpecularIntensity()));
}

/// the bevel, the scale of the back and the closing of the two ends, which extrusion and turning
/// have alike
template <class Body> void putBody(tools::JsonWriter& rWriter, const Body& rBody)
{
    rWriter.put("diagonal", rBody.getDiagonal());
    rWriter.put("backScale", rBody.getBackScale());
    if (rBody.getSmoothNormals())
        rWriter.put("smoothNormals", true);
    if (rBody.getSmoothLids())
        rWriter.put("smoothLids", true);
    if (rBody.getCharacterMode())
        rWriter.put("characterMode", true);
    if (rBody.getCloseFront())
        rWriter.put("closeFront", true);
    if (rBody.getCloseBack())
        rWriter.put("closeBack", true);
}

void writePrimitive3D(tools::JsonWriter& rWriter,
                      const drawinglayer::primitive3d::BasePrimitive3D& rPrimitive)
{
    namespace primitive3d = drawinglayer::primitive3d;
    auto aEntry = rWriter.startStruct();
    const sal_uInt32 nId = rPrimitive.getPrimitive3DID();
    switch (nId)
    {
        case PRIMITIVE3D_ID_SDRCUBEPRIMITIVE3D:
            rWriter.put("type", "cube");
            break;
        case PRIMITIVE3D_ID_SDRSPHEREPRIMITIVE3D:
        {
            const auto& rSphere = static_cast<const primitive3d::SdrSpherePrimitive3D&>(rPrimitive);
            rWriter.put("type", "sphere");
            rWriter.put("horizontalSegments", sal_Int64(rSphere.getHorizontalSegments()));
            rWriter.put("verticalSegments", sal_Int64(rSphere.getVerticalSegments()));
            break;
        }
        case PRIMITIVE3D_ID_SDREXTRUDEPRIMITIVE3D:
        {
            const auto& rExtrude = static_cast<const primitive3d::SdrExtrudePrimitive3D&>(rPrimitive);
            rWriter.put("type", "extrude");
            putOutline(rWriter, preparedOutline(rExtrude.getPolyPolygon()));
            rWriter.put("depth", rExtrude.getDepth());
            putBody(rWriter, rExtrude);
            break;
        }
        case PRIMITIVE3D_ID_SDRLATHEPRIMITIVE3D:
        {
            const auto& rLathe = static_cast<const primitive3d::SdrLathePrimitive3D&>(rPrimitive);
            rWriter.put("type", "lathe");
            // the turned outline has as many edges as the body has segments along it
            basegfx::B2DPolyPolygon aOutline(preparedOutline(rLathe.getPolyPolygon()));
            if (aOutline.count())
            {
                const basegfx::B2DPolygon aFirst(aOutline.getB2DPolygon(0));
                const sal_uInt32 nEdges(aFirst.isClosed() ? aFirst.count()
                                                          : (aFirst.count() ? aFirst.count() - 1 : 0));
                if (nEdges != rLathe.getVerticalSegments())
                    aOutline = basegfx::utils::reSegmentPolyPolygon(aOutline,
                                                                    rLathe.getVerticalSegments());
            }
            putOutline(rWriter, aOutline);
            rWriter.put("horizontalSegments", sal_Int64(rLathe.getHorizontalSegments()));
            rWriter.put("verticalSegments", sal_Int64(rLathe.getVerticalSegments()));
            rWriter.put("rotation", rLathe.getRotation());
            putBody(rWriter, rLathe);
            break;
        }
        case PRIMITIVE3D_ID_SDRPOLYPOLYGONPRIMITIVE3D:
        {
            const auto& rPolygons
                = static_cast<const primitive3d::SdrPolyPolygonPrimitive3D&>(rPrimitive);
            rWriter.put("type", "polyPolygon");
            putPolygons3D(rWriter, rPolygons.getPolyPolygon3D());
            break;
        }
        default:
            // we tell what it is, a reader takes it as a part it cannot show yet
            rWriter.put("type", rtl::Concat2View("drawinglayer3d:" + OString::number(nId)));
            return;
    }

    const auto& rSdr = static_cast<const primitive3d::SdrPrimitive3D&>(rPrimitive);
    putMatrix(rWriter, "transform", rSdr.getTransform());
    putLook(rWriter, rSdr);
}
}

void describeIn3D(const E3dObject& rObject, ObjectContent& rContent)
{
    rContent.meKind = rObject.GetObjIdentifier();
    if (!rObject.getParentE3dSceneFromE3dObject())
    {
        if (const E3dScene* pScene = DynCastE3dScene(&rObject))
            rContent.moScene3D = sceneOf(*pScene);
        return;
    }

    rContent.mo3DMapping = rObject.GetTransform();
    if (const auto* pContact
        = dynamic_cast<const sdr::contact::ViewContactOfE3d*>(&rObject.GetViewContact()))
        rContent.maPrimitives3D = pContact->getVIP3DSWithoutObjectTransform();
}

void writeIn3D(tools::JsonWriter& rWriter, const ObjectContent& rContent)
{
    if (rContent.moScene3D)
        writeScene(rWriter, *rContent.moScene3D);
    if (rContent.mo3DMapping)
        putMatrix(rWriter, "transform3D", *rContent.mo3DMapping);
    if (!rContent.maPrimitives3D.empty())
    {
        auto aArray = rWriter.startArray("primitives3D");
        for (const drawinglayer::primitive3d::Primitive3DReference& xPrimitive :
             rContent.maPrimitives3D)
            if (const auto* pPrimitive
                = static_cast<const drawinglayer::primitive3d::BasePrimitive3D*>(xPrimitive.get()))
                writePrimitive3D(rWriter, *pPrimitive);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
