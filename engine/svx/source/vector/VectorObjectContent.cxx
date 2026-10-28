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

#include <svx/VectorObjectContent.hxx>

#include <algorithm>
#include <map>
#include <vector>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <comphelper/servicehelper.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/unit_conversion.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <svx/compatflags.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdglue.hxx>
#include <svx/charthelper.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/sxcgitm.hxx>
#include <svx/sxcllitm.hxx>
#include <svx/sxctitm.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxenditm.hxx>
#include <svx/sxmbritm.hxx>
#include <svx/sxmlhitm.hxx>
#include <svx/sxmtfitm.hxx>
#include <svx/sxmtpitm.hxx>
#include <svx/sxmtritm.hxx>
#include <svx/unopage.hxx>
#include <svx/xlnwtit.hxx>
#include <tools/json_writer.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/weakref.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>

namespace svx::vector
{
namespace
{
/// The drawing layer works in 1/100 mm.
constexpr double constTwipConversionFactor
    = o3tl::convert(1.0, o3tl::Length::mm100, o3tl::Length::twip);

/// How one end of the connector holds on to its object, read from the connection record,
/// or where the end lies when it holds on to none, read from the run.
ObjectContent::ConnectorTie tieOf(const SdrEdgeObj& rConnector, bool bStart)
{
    ObjectContent::ConnectorTie aTie;
    const SdrObjConnection& rConnection = rConnector.GetConnection(bStart);

    if (!rConnector.GetConnectedNode(bStart))
    {
        const basegfx::B2DPolyPolygon aTrack(rConnector.GetEdgeTrackPath());
        if (aTrack.count() && aTrack.getB2DPolygon(0).count())
        {
            const basegfx::B2DPolygon& rRun = aTrack.getB2DPolygon(0);
            const basegfx::B2DPoint aAt(rRun.getB2DPoint(bStart ? 0 : rRun.count() - 1));
            aTie.moAt = basegfx::B2DPoint(aAt.getX() * constTwipConversionFactor,
                                          aAt.getY() * constTwipConversionFactor);
        }
        return aTie;
    }

    if (!rConnection.IsBestConnection())
        aTie.moPoint = rConnection.GetConnectorId();
    aTie.mbBestVertex = rConnection.IsBestVertex();
    aTie.mbAutoVertex = rConnection.IsAutoVertex();
    aTie.mbAutoCorner = rConnection.IsAutoCorner();
    return aTie;
}

/// The numbers the connector's run is worked out from, read from its items and its ties.
ObjectContent::ConnectorRouting routingOf(const SdrEdgeObj& rConnector)
{
    ObjectContent::ConnectorRouting aRouting;
    const SfxItemSet& rSet = rConnector.GetMergedItemSet();

    switch (rSet.Get(SDRATTR_EDGEKIND).GetValue())
    {
        case SdrEdgeKind::OrthoLines:
            aRouting.maKind = "ortho"_ostr;
            break;
        case SdrEdgeKind::ThreeLines:
            aRouting.maKind = "three"_ostr;
            break;
        case SdrEdgeKind::OneLine:
            aRouting.maKind = "one"_ostr;
            break;
        case SdrEdgeKind::Bezier:
            aRouting.maKind = "bezier"_ostr;
            break;
        case SdrEdgeKind::Arc:
            aRouting.maKind = "arc"_ostr;
            break;
    }

    if (rSet.HasItem(SDRATTR_EDGEOOXMLCURVE))
        aRouting.mbCurveFollowsOOXML = rSet.Get(SDRATTR_EDGEOOXMLCURVE).GetValue();

    aRouting.mfNode1Across
        = rSet.Get(SDRATTR_EDGENODE1HORZDIST).GetValue() * constTwipConversionFactor;
    aRouting.mfNode1Down
        = rSet.Get(SDRATTR_EDGENODE1VERTDIST).GetValue() * constTwipConversionFactor;
    aRouting.mfNode2Across
        = rSet.Get(SDRATTR_EDGENODE2HORZDIST).GetValue() * constTwipConversionFactor;
    aRouting.mfNode2Down
        = rSet.Get(SDRATTR_EDGENODE2VERTDIST).GetValue() * constTwipConversionFactor;

    aRouting.mnLineDeltaCount = rSet.Get(SDRATTR_EDGELINEDELTACOUNT).GetValue();
    aRouting.mfLineDelta1 = rSet.Get(SDRATTR_EDGELINE1DELTA).GetValue() * constTwipConversionFactor;
    aRouting.mfLineDelta2 = rSet.Get(SDRATTR_EDGELINE2DELTA).GetValue() * constTwipConversionFactor;
    aRouting.mfLineDelta3 = rSet.Get(SDRATTR_EDGELINE3DELTA).GetValue() * constTwipConversionFactor;

    aRouting.mbAgainstSnapRectangles = rConnector.getSdrModelFromSdrObject().GetCompatibilityFlag(
        SdrCompatibilityFlag::ConnectorUseSnapRect);

    aRouting.mbTrackUserDefined = rConnector.IsEdgeTrackUserDefined();
    aRouting.maTieAtStart = tieOf(rConnector, true);
    aRouting.maTieAtEnd = tieOf(rConnector, false);
    return aRouting;
}

/*
    What a picture says about the graphic it shows: the size the graphic has by its own
    resolution, brought into the unit the document keeps its items in the way the crop drag
    does, and the four amounts cropped away from it, all as twips. A picture that shows no
    graphic yet says nothing.
*/
void takeInThePicture(const SdrGrafObj& rPicture, ObjectContent& rContent)
{
    if (rPicture.GetGraphicType() == GraphicType::NONE
        || rPicture.GetGraphicType() == GraphicType::Default)
        return;

    const GraphicObject& rGraphic(rPicture.GetGraphicObject());
    const MapMode aPoolMapMode(rPicture.getSdrModelFromSdrObject().GetItemPool().GetMetric(0));
    Size aGraphicSize(rGraphic.GetPrefSize());

    if (MapUnit::MapPixel == rGraphic.GetPrefMapMode().GetMapUnit())
        aGraphicSize = Application::GetDefaultDevice()->PixelToLogic(aGraphicSize, aPoolMapMode);
    else
        aGraphicSize
            = OutputDevice::LogicToLogic(aGraphicSize, rGraphic.GetPrefMapMode(), aPoolMapMode);

    if (0 == aGraphicSize.Width() || 0 == aGraphicSize.Height())
        return;

    const SdrGrafCropItem& rCrop = rPicture.GetMergedItem(SDRATTR_GRAFCROP);

    rContent.mbIsPicture = true;
    // The checksum the graphic is named by on the wire, so the picture can be drawn from
    // the same fetched image the primitives draw it from.
    rContent.mnGraphicChecksum = static_cast<sal_Int64>(rGraphic.GetGraphic().GetChecksum());
    // The graphic is turned over as the picture draws it: across where the picture is
    // mirrored, inside the picture's own mapping, which carries any turn. A picture turned
    // upside down is a mirrored one turned by half a circle.
    rContent.mnGraphicMirror = static_cast<sal_Int32>(
        rPicture.IsMirrored() ? BmpMirrorFlags::Horizontal : BmpMirrorFlags::NONE);
    rContent.mfGraphicWidth = aGraphicSize.Width() * constTwipConversionFactor;
    rContent.mfGraphicHeight = aGraphicSize.Height() * constTwipConversionFactor;
    rContent.mfCropLeft = rCrop.GetLeft() * constTwipConversionFactor;
    rContent.mfCropTop = rCrop.GetTop() * constTwipConversionFactor;
    rContent.mfCropRight = rCrop.GetRight() * constTwipConversionFactor;
    rContent.mfCropBottom = rCrop.GetBottom() * constTwipConversionFactor;
}

/// True when the object, or any object inside it when it is a group, has a text edit
/// running on it.
bool hasActiveTextEdit(SdrObject* pObject)
{
    SdrTextObj* pTextObject = DynCastSdrTextObj(pObject);
    if (pTextObject && pTextObject->IsInEditMode())
        return true;

    if (SdrObjList* pChildren = pObject->GetSubList())
    {
        for (size_t i = 0; i < pChildren->GetObjCount(); ++i)
        {
            SdrObject* pChild = pChildren->GetObj(i);
            if (pChild && hasActiveTextEdit(pChild))
                return true;
        }
    }

    return false;
}

/*
    The mapping an object is drawn by. What the object answers describes its base geometry,
    which holds the shear the other way round: the object lays itself out with the shear
    turned over, and the answer keeps the angle as the model holds it, so that the two agree
    when the answer is handed back. A reader that wants to know where the object is drawn
    needs the first of those, so the shear is turned over here.

    An object with no shear is the same either way, and a scale of a turned object is what
    gives an object shear, which is why this only shows there.
*/
basegfx::B2DHomMatrix asItIsDrawn(const basegfx::B2DHomMatrix& rBaseGeometry)
{
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate(0.0);
    double fShearX(0.0);
    rBaseGeometry.decompose(aScale, aTranslate, fRotate, fShearX);

    if (basegfx::fTools::equalZero(fShearX))
        return rBaseGeometry;

    return basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(aScale, -fShearX, fRotate,
                                                                        aTranslate);
}

/** Mirrors the mapping back about the line the drawing layer un-mirrored it about: an
    upright line through the middle of the box around it, or a level one.

    A shape that says it faces the other way is answered for with the mapping it would have
    if it did not, and that is worked out by mirroring it about a line of the page, not about
    a line of its own. For a shape standing straight the two are the same line and nothing
    moves. For a turned one they are not, and the answer lies somewhere else than the shape
    is drawn, which is what this takes back. The box around a shape is not changed by
    mirroring it about its own middle, so the line can be worked out here just as well.
 */
basegfx::B2DHomMatrix mirroredBack(const basegfx::B2DHomMatrix& rTransformation, const bool bAcross)
{
    basegfx::B2DRange aBox;
    for (const auto& rCorner : { basegfx::B2DPoint(0.0, 0.0), basegfx::B2DPoint(1.0, 0.0),
                                 basegfx::B2DPoint(1.0, 1.0), basegfx::B2DPoint(0.0, 1.0) })
        aBox.expand(rTransformation * rCorner);

    const double fMiddle(bAcross ? aBox.getCenter().getX() : aBox.getCenter().getY());

    basegfx::B2DHomMatrix aMirror;
    if (bAcross)
    {
        aMirror.set(0, 0, -1.0);
        aMirror.set(0, 2, 2.0 * fMiddle);
    }
    else
    {
        aMirror.set(1, 1, -1.0);
        aMirror.set(1, 2, 2.0 * fMiddle);
    }

    return aMirror * rTransformation;
}

/** The mapping that takes the object's own square onto the page, in twips, saying the whole
    truth about where the object is and which way round it is drawn.

    The drawing layer holds none of its own mirroring in a mapping: a shape keeps it apart,
    as the way it says it faces, and answers with a mapping that has positive lengths
    whichever way it is drawn. A reader that only has the mapping would then have no way of
    telling the two apart, so the mirroring is laid back into it here. The shear is turned
    over here for the same reason, and both belong at this one place: what leaves the engine
    should be a mapping anybody can work with, and the drawing layer's own way of keeping
    these should not have to be known on the other side.
 */
basegfx::B2DHomMatrix transformationInTwips(const SdrObject& rObject)
{
    /*
        A measurement is described by the mapping it reads when it is given one, the unit line
        onto the measured line, rather than by the upright box TRGetBaseGeometry answers for
        it, which loses which way the line runs and has no width for an upright line.
    */
    if (const auto* pMeasurement = dynamic_cast<const SdrMeasureObj*>(&rObject))
        return basegfx::utils::createScaleB2DHomMatrix(constTwipConversionFactor,
                                                       constTwipConversionFactor)
               * pMeasurement->GetLineMapping();

    basegfx::B2DHomMatrix aTransformation;
    basegfx::B2DPolyPolygon aPolyPolygon;
    rObject.TRGetBaseGeometry(aTransformation, aPolyPolygon);
    aTransformation = asItIsDrawn(aTransformation);

    if (const auto* pShape = dynamic_cast<const SdrObjCustomShape*>(&rObject))
    {
        if (pShape->IsMirroredX())
            aTransformation = mirroredBack(aTransformation, true);
        if (pShape->IsMirroredY())
            aTransformation = mirroredBack(aTransformation, false);
    }

    return basegfx::utils::createScaleB2DHomMatrix(constTwipConversionFactor,
                                                   constTwipConversionFactor)
           * aTransformation;
}

/** Takes the places a handle may be moved between onto the object's own square, where 0 and
    0 is its upper left corner and 1 and 1 its lower right one.

    A place written that way travels with the object: a reader that moves the object itself,
    without waiting to be told what became of it, still has the places right. It also says
    the same thing however the object is turned, sheared or mirrored, since all of that is
    in the mapping the reader already holds.
 */
void onTheObjectsSquare(SdrHandleRails& rRails, const SdrObject& rObject)
{
    basegfx::B2DHomMatrix aOntoTheSquare(transformationInTwips(rObject));
    if (!aOntoTheSquare.invert())
        return;

    auto aOne = [&aOntoTheSquare](basegfx::B2DPoint& rPlace)
    {
        rPlace = aOntoTheSquare
                 * basegfx::B2DPoint(rPlace.getX() * constTwipConversionFactor,
                                     rPlace.getY() * constTwipConversionFactor);
    };

    aOne(rRails.maLeastAcross);
    aOne(rRails.maMostAcross);
    aOne(rRails.maLeastDown);
    aOne(rRails.maMostDown);
    aOne(rRails.maAround);
    aOne(rRails.maNearest);
    aOne(rRails.maFurthest);
    aOne(rRails.maFrom);
    aOne(rRails.maTo);
}

/** The handles that shape an object, in twips: the corner radius of a rectangle and the
    points a custom shape is shaped by.

    A reader works the rest out from what it draws - the points of a polygon and the weights
    of a curve are in the geometry it already has - while these two say nothing about the
    drawing and cannot be found in it. Each handle is named by what it means, the kind with
    the polygon and the point it belongs to, so that a reader can say which handle it moves
    without counting the handles it was never sent.
 */
std::vector<ObjectContent::Handle> shapingHandlesOf(const SdrObject& rObject)
{
    SdrHdlList aHandleList(nullptr);
    rObject.AddToHdlList(aHandleList);

    // a rectangle that takes no corner radius, a chart for one, still adds the handle of it, which
    // the office leaves out; the angles of a piece of a circle come as the same kind
    SdrObjTransformInfoRec aWhatItAllows;
    rObject.TakeObjInfo(aWhatItAllows);
    const bool bRadiusLeftOut(!aWhatItAllows.bEdgeRadiusAllowed
                              && nullptr == dynamic_cast<const SdrCircObj*>(&rObject));

    std::vector<ObjectContent::Handle> aHandles;
    for (size_t nHandle = 0; nHandle < aHandleList.GetHdlCount(); ++nHandle)
    {
        const SdrHdl* pHandle = aHandleList.GetHdl(nHandle);
        if (!pHandle)
            continue;

        /*
            The handles a reader cannot work out for itself. The corner radius of a
            rectangle and the points a custom shape is shaped by say nothing about what is
            drawn. The points of an object that is shaped by them - the ends of a connector
            and the lines between them, the tail of a caption - are another such case,
            unless the object's path travels as well, and then they follow from it.
        */
        const SdrHdlKind eKind = pHandle->GetKind();
        const bool bMeasurement(SdrObjKind::Measure == rObject.GetObjIdentifier());
        // a line sends its two ends as handles and no path, as a measurement does
        const bool bLine(SdrObjKind::Line == rObject.GetObjIdentifier());
        const bool bWanted(eKind == SdrHdlKind::Circle || eKind == SdrHdlKind::CustomShape1
                           || (eKind == SdrHdlKind::Poly
                               && (bLine || nullptr == dynamic_cast<const SdrPathObj*>(&rObject)))
                           || (eKind == SdrHdlKind::User && bMeasurement));
        if (!bWanted || (eKind == SdrHdlKind::Circle && bRadiusLeftOut))
            continue;

        const Point aPosition(pHandle->GetPos());
        ObjectContent::Handle aOne;
        aOne.mnKind = static_cast<sal_Int32>(eKind);
        aOne.mnPolygon = pHandle->GetPolyNum();
        aOne.mnPoint = pHandle->GetPointNum();
        aOne.mnObjHandle = pHandle->GetObjHdlNum();

        // The first two handles of a measurement are the help line ends nearer to the line,
        // which stand on the two ends of the measured line until moved; the office draws
        // them larger, behind the handles of those ends.
        aOne.mbLarger = bMeasurement && aOne.mnObjHandle < 2;

        /*
            A point a custom shape is shaped by may only go where the shape lets it, and what
            it lets it do is worked out from the shape as it stands. It travels so that a
            client can hold a drag to it without knowing anything about the shape itself.
        */
        // How far the handle may be moved is the object's own business, and it answers in
        // its own coordinates, so the places are taken onto its square here.
        aOne.mbHasRails = rObject.GetHandleRails(*pHandle, aOne.maRails);
        if (aOne.mbHasRails)
            onTheObjectsSquare(aOne.maRails, rObject);
        aOne.maPosition
            = Point(basegfx::fround<tools::Long>(aPosition.X() * constTwipConversionFactor),
                    basegfx::fround<tools::Long>(aPosition.Y() * constTwipConversionFactor));
        aHandles.push_back(aOne);
    }

    return aHandles;
}

/** The path an object is drawn from, in twips, taken from the model rather than from what
    the object draws.

    What is drawn is the path with everything the drawing does to it: a stroke becomes the
    outline of a fat line, a dash becomes many polygons, a shadow is another copy of it all,
    and a curve may have been cut into straight pieces. An editor needs the points the model
    holds, in the order it holds them, because that order is what names a point when it is
    moved. An object that has no path of its own answers with an empty one.
 */
basegfx::B2DPolyPolygon pathOf(const SdrObject& rObject)
{
    const auto* pPathObject = dynamic_cast<const SdrPathObj*>(&rObject);
    // a line has its two ends in the handles instead
    if (!pPathObject || SdrObjKind::Line == pPathObject->GetObjIdentifier())
        return basegfx::B2DPolyPolygon();

    basegfx::B2DPolyPolygon aPath(pPathObject->GetPathPoly());
    aPath.transform(basegfx::utils::createScaleB2DHomMatrix(constTwipConversionFactor,
                                                            constTwipConversionFactor));
    return aPath;
}

/// The rectangle the object paints, in twips: the range of its primitives, which takes in
/// the line width and a shadow, or the object's bound rectangle when it paints nothing.
tools::Rectangle
paintedRectangleInTwips(const SdrObject& rObject,
                        const drawinglayer::primitive2d::Primitive2DContainer& rPrimitives,
                        const drawinglayer::geometry::ViewInformation2D& rViewInformation)
{
    basegfx::B2DRange aRange(rPrimitives.getB2DRange(rViewInformation));
    if (aRange.isEmpty())
        aRange = vcl::unotools::b2DRectangleFromRectangle(rObject.GetCurrentBoundRect());

    return rangeInTwips(aRange);
}

/// How one end of a connector holds on to its object, as a node of the given name.
void writeTie(tools::JsonWriter& rWriter, std::string_view aName,
              const ObjectContent::ConnectorTie& rTie)
{
    auto aNode = rWriter.startNode(aName);
    if (rTie.moAt)
    {
        rWriter.put("x", rTie.moAt->getX());
        rWriter.put("y", rTie.moAt->getY());
    }
    if (rTie.moPoint)
        rWriter.put("point", sal_Int32(*rTie.moPoint));
    if (rTie.mbBestVertex)
        rWriter.put("bestVertex", true);
    if (rTie.mbAutoVertex)
        rWriter.put("autoVertex", true);
    if (rTie.mbAutoCorner)
        rWriter.put("autoCorner", true);
}

/// The numbers a connector's run is worked out from, as the "routing" node of its entry.
void writeRouting(tools::JsonWriter& rWriter, const ObjectContent::ConnectorRouting& rRouting)
{
    auto aNode = rWriter.startNode("routing");
    rWriter.put("kind", rRouting.maKind);
    if (rRouting.mbCurveFollowsOOXML)
        rWriter.put("curveFollowsOOXML", true);
    {
        auto aDistances = rWriter.startArray("nodeDistances");
        rWriter.putSimpleValue(rRouting.mfNode1Across);
        rWriter.putSimpleValue(rRouting.mfNode1Down);
        rWriter.putSimpleValue(rRouting.mfNode2Across);
        rWriter.putSimpleValue(rRouting.mfNode2Down);
    }
    rWriter.put("lineDeltaCount", rRouting.mnLineDeltaCount);
    {
        auto aDeltas = rWriter.startArray("lineDeltas");
        rWriter.putSimpleValue(rRouting.mfLineDelta1);
        rWriter.putSimpleValue(rRouting.mfLineDelta2);
        rWriter.putSimpleValue(rRouting.mfLineDelta3);
    }
    if (rRouting.mbAgainstSnapRectangles)
        rWriter.put("againstSnapRectangles", true);
    if (rRouting.mbTrackUserDefined)
        rWriter.put("trackUserDefined", true);
    writeTie(rWriter, "tieAtStart", rRouting.maTieAtStart);
    writeTie(rWriter, "tieAtEnd", rRouting.maTieAtEnd);
}
}

tools::Rectangle rangeInTwips(const basegfx::B2DRange& rRange)
{
    if (rRange.isEmpty())
        return tools::Rectangle();

    return tools::Rectangle(
        basegfx::fround<tools::Long>(rRange.getMinX() * constTwipConversionFactor),
        basegfx::fround<tools::Long>(rRange.getMinY() * constTwipConversionFactor),
        basegfx::fround<tools::Long>(rRange.getMaxX() * constTwipConversionFactor),
        basegfx::fround<tools::Long>(rRange.getMaxY() * constTwipConversionFactor));
}

basegfx::B2DHomMatrix boxTransformation(const tools::Rectangle& rBox)
{
    basegfx::B2DHomMatrix aTransformation;
    aTransformation.set(0, 0, double(rBox.GetWidth()));
    aTransformation.set(1, 1, double(rBox.GetHeight()));
    aTransformation.set(0, 2, double(rBox.Left()));
    aTransformation.set(1, 2, double(rBox.Top()));
    return aTransformation;
}

void decomposeForComparison(ObjectContent& rContent,
                            const drawinglayer::geometry::ViewInformation2D& rViewInformation)
{
    for (const auto& rPrimitive : rContent.maPrimitives)
    {
        // A page preview shows another page, and a field inside it reads against that page
        // rather than against the one being drawn, so the preview names the page it shows
        // as the visualized one.
        drawinglayer::geometry::ViewInformation2D aViewInformation(rViewInformation);
        if (const auto* pPreview
            = dynamic_cast<const drawinglayer::primitive2d::PagePreviewPrimitive2D*>(
                rPrimitive.get()))
            aViewInformation.setVisualizedPage(pPreview->getXDrawPage());

        drawinglayer::primitive2d::Primitive2DContainer aDecomposed;
        rPrimitive->get2DDecomposition(aDecomposed, aViewInformation);
        if (aDecomposed.empty())
            rContent.maDrawn.push_back(rPrimitive);
        else
            rContent.maDrawn.append(std::move(aDecomposed));
    }
}

namespace
{
/// the page of a chart active in place, handed out as its members, with the chart and the
/// mapping from the page onto the chart's upright window, in the units of the model
struct Embedding
{
    unotools::WeakReference<SdrPage> mxPage;
    unotools::WeakReference<SdrObject> mxHolder;
    basegfx::B2DHomMatrix maMapping;
};

/// the pages and charts are held weakly, an entry whose page is gone is dropped
std::vector<Embedding>& embeddings()
{
    static std::vector<Embedding> aEmbeddings;
    std::erase_if(aEmbeddings,
                  [](const Embedding& rEmbedding) { return !rEmbedding.mxPage.get().is(); });
    return aEmbeddings;
}

Embedding* embeddingFor(const SdrPage* pPage)
{
    std::vector<Embedding>& rEmbeddings(embeddings());
    const auto aFound(std::find_if(rEmbeddings.begin(), rEmbeddings.end(),
                                   [pPage](const Embedding& rEmbedding)
                                   { return rEmbedding.mxPage.get().get() == pPage; }));
    return aFound == rEmbeddings.end() ? nullptr : &*aFound;
}

bool activeInPlace(const SdrOle2Obj& rObject)
{
    try
    {
        const cpo::uno::Reference<css::embed::XEmbeddedObject>& xObject(rObject.GetObjRef());
        if (!xObject.is())
            return false;
        const sal_Int32 nState(xObject->getCurrentState());
        return nState == css::embed::EmbedStates::INPLACE_ACTIVE
               || nState == css::embed::EmbedStates::UI_ACTIVE;
    }
    catch (const cpo::uno::Exception&)
    {
        return false;
    }
}

/// the page the chart draws its content on, or nothing for an object that is no chart
SdrPage* chartPageOf(const SdrOle2Obj& rObject)
{
    if (!rObject.IsChart())
        return nullptr;
    const cpo::uno::Reference<css::drawing::XDrawPageSupplier> xSupplier(rObject.getXModel(),
                                                                         cpo::uno::UNO_QUERY);
    if (!xSupplier.is())
        return nullptr;
    SvxDrawPage* pPage(comphelper::getFromUnoTunnel<SvxDrawPage>(xSupplier->getDrawPage()));
    return pPage ? pPage->GetSdrPage() : nullptr;
}

/*
    The mapping of a chart's page onto the upright window of the chart active in place: the
    range its content covers onto the chart's logic rectangle, which is what the window fills.
    The range is the one the chart's content is embedded by when it is not active, so nothing
    moves when it becomes active.
*/
basegfx::B2DHomMatrix mappingOfTheChart(const SdrOle2Obj& rObject)
{
    basegfx::B2DRange aRange;
    ChartHelper::tryToGetChartContentAsPrimitive2DSequence(rObject.getXModel(), aRange);
    const tools::Rectangle& rWindow(rObject.GetLogicRect());
    if (aRange.isEmpty() || aRange.getWidth() <= 0.0 || aRange.getHeight() <= 0.0)
        return basegfx::utils::createTranslateB2DHomMatrix(rWindow.Left(), rWindow.Top());

    basegfx::B2DHomMatrix aMapping(
        basegfx::utils::createTranslateB2DHomMatrix(-aRange.getMinX(), -aRange.getMinY()));
    aMapping = basegfx::utils::createScaleB2DHomMatrix(rWindow.GetWidth() / aRange.getWidth(),
                                                       rWindow.GetHeight() / aRange.getHeight())
               * aMapping;
    aMapping
        = basegfx::utils::createTranslateB2DHomMatrix(rWindow.Left(), rWindow.Top()) * aMapping;
    return aMapping;
}

/// how the object is embedded in a chart active in place, where it is on such a chart's page
const Embedding* embeddingOf(const SdrObject& rObject)
{
    return embeddingFor(rObject.getSdrPageFromSdrObject());
}

/*
    The name the chart gives a part it selects on its own (its CID), without what changes when the
    part is dragged, or empty for any other object. The chart names its parts "CID/...", and a
    dragged part carries "/DragParameter=..." last, which changes with the drag.
*/
OUString partNameOf(const SdrObject& rObject)
{
    if (!embeddingOf(rObject))
        return OUString();
    OUString aName(rObject.GetName());
    if (!aName.startsWith("CID/"))
        return OUString();
    const sal_Int32 nDragged(aName.indexOf("/DragParameter="));
    return nDragged < 0 ? aName : aName.copy(0, nDragged);
}

/// the ids kept for the parts of a chart page, by the part's name
struct PartIds
{
    unotools::WeakReference<SdrPage> mxPage;
    std::map<OUString, UniqueID> maIds;
};

/// the pages are held weakly, an entry whose page is gone is dropped
std::vector<PartIds>& partIds()
{
    static std::vector<PartIds> aPartIds;
    std::erase_if(aPartIds, [](const PartIds& rPartIds) { return !rPartIds.mxPage.get().is(); });
    return aPartIds;
}

std::map<OUString, UniqueID>& partIdsOf(SdrPage* pPage)
{
    std::vector<PartIds>& rPartIds(partIds());
    const auto aFound(std::find_if(rPartIds.begin(), rPartIds.end(), [pPage](const PartIds& rIds)
                                   { return rIds.mxPage.get().get() == pPage; }));
    if (aFound != rPartIds.end())
        return aFound->maIds;
    rPartIds.push_back(PartIds{ unotools::WeakReference<SdrPage>(pPage), {} });
    return rPartIds.back().maIds;
}

/*
    What a part of a chart allows, from its name, as the chart decides it for its own selection
    (ObjectIdentifier::getObjectType, isDragableObject, isRotateableObject and
    Selection::isResizeableObjectSelected in chart2). The kind of the part is the last piece of
    the name, after the last ":" or "/", so "D=0:CS=0:CT=0:Series=0" is a series and a bare
    "D=0" the diagram. A title, a legend, the diagram and a data label can be dragged, the
    diagram, its wall and the legend resized, and a part with a drag method of its own, a pie
    segment for one, dragged along it. A turn is for a diagram in 3D only, which is not told
    apart here yet.
*/
ObjectContent::ChartPart chartPartOf(std::u16string_view aName)
{
    ObjectContent::ChartPart aPart;
    aPart.mbMultiClick = o3tl::starts_with(aName, u"CID/MultiClick/");
    const bool bDragsItsOwnWay(aName.find(u"DragMethod=") != std::u16string_view::npos);

    size_t nLast(aName.rfind(':'));
    if (nLast == std::u16string_view::npos)
        nLast = aName.rfind('/');
    const std::u16string_view aKind(nLast == std::u16string_view::npos ? aName
                                                                         : aName.substr(nLast + 1));

    const bool bTitle(o3tl::starts_with(aKind, u"Title"));
    const bool bLegend(o3tl::starts_with(aKind, u"Legend")
                       && !o3tl::starts_with(aKind, u"LegendEntry"));
    const bool bWall(o3tl::starts_with(aKind, u"DiagramWall"));
    const bool bDiagram(!bWall && !o3tl::starts_with(aKind, u"DiagramFloor")
                        && o3tl::starts_with(aKind, u"D="));
    const bool bDataLabel(o3tl::starts_with(aKind, u"DataLabel")
                          && !o3tl::starts_with(aKind, u"DataLabels"));
    const bool bEquation(o3tl::starts_with(aKind, u"Equation"));

    aPart.mbDraggable = bTitle || bLegend || bDiagram || bDataLabel || bEquation || bDragsItsOwnWay;
    aPart.mbResizable = bDiagram || bWall || bLegend;
    aPart.mbRotatable = false;
    return aPart;
}
}

sal_uInt64 idOf(const SdrObject& rObject)
{
    const OUString aName(partNameOf(rObject));
    if (aName.isEmpty())
        return rObject.GetUniqueID();

    // only the outermost object of a part is the part, one inside it with the same name keeps
    // its own id, so no two entries share one
    if (const SdrObject* pParent = rObject.getParentSdrObjectFromSdrObject();
        pParent && partNameOf(*pParent) == aName)
        return rObject.GetUniqueID();

    // a new UniqueID draws the next id of the one counter, so it is never another object's
    return partIdsOf(rObject.getSdrPageFromSdrObject())[aName].getID();
}

const SdrObjList* membersOf(const SdrObject& rObject)
{
    if (DynCastE3dScene(&rObject))
        return nullptr;

    // a chart active in place has the objects of its page as members, as long as it is active
    if (const auto* pChart = dynamic_cast<const SdrOle2Obj*>(&rObject))
    {
        SdrPage* pPage(chartPageOf(*pChart));
        if (!pPage)
            return nullptr;
        if (!activeInPlace(*pChart))
        {
            std::erase_if(embeddings(), [pPage](const Embedding& rEmbedding)
                          { return rEmbedding.mxPage.get().get() == pPage; });
            std::erase_if(partIds(), [pPage](const PartIds& rPartIds)
                          { return rPartIds.mxPage.get().get() == pPage; });
            return nullptr;
        }
        if (Embedding* pEmbedding = embeddingFor(pPage))
        {
            pEmbedding->mxHolder = const_cast<SdrOle2Obj*>(pChart);
            pEmbedding->maMapping = mappingOfTheChart(*pChart);
        }
        else
            embeddings().push_back(Embedding{ unotools::WeakReference<SdrPage>(pPage),
                                              unotools::WeakReference<SdrObject>(
                                                  const_cast<SdrOle2Obj*>(pChart)),
                                              mappingOfTheChart(*pChart) });
        return pPage;
    }

    return rObject.GetSubList();
}

const SdrObject* holderOf(const SdrObject& rObject)
{
    if (const SdrObject* pParent = rObject.getParentSdrObjectFromSdrObject())
        return pParent;
    const Embedding* pEmbedding(embeddingOf(rObject));
    return pEmbedding ? pEmbedding->mxHolder.get().get() : nullptr;
}

void describeObject(SdrObject& rObject, ObjectContent& rContent,
                    const drawinglayer::geometry::ViewInformation2D& rViewInformation)
{
    const SdrObjList* pChildren = membersOf(rObject);
    if (!pChildren || pChildren->GetObjCount() == 0)
    {
        if (rContent.maPrimitives.empty())
        {
            rObject.GetViewContact().getViewIndependentPrimitive2DContainer(rContent.maPrimitives);

            // an object of a chart active in place is drawn where the chart's window is
            if (const Embedding* pEmbedding = embeddingOf(rObject);
                pEmbedding && !rContent.maPrimitives.empty())
                rContent.maPrimitives = drawinglayer::primitive2d::Primitive2DContainer{
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        pEmbedding->maMapping, std::move(rContent.maPrimitives))
                };
        }

        rContent.maHandles = shapingHandlesOf(rObject);
        rContent.maPath = pathOf(rObject);
        rContent.meKind = rObject.GetObjIdentifier();

        if (const SdrEdgeObj* pConnector = dynamic_cast<const SdrEdgeObj*>(&rObject))
        {
            const SdrObject* pAtStart = pConnector->GetConnectedNode(true);
            const SdrObject* pAtEnd = pConnector->GetConnectedNode(false);
            rContent.mbTiedAtStart = nullptr != pAtStart;
            rContent.mbTiedAtEnd = nullptr != pAtEnd;
            rContent.mnTiedToAtStart = pAtStart ? idOf(*pAtStart) : 0;
            rContent.mnTiedToAtEnd = pAtEnd ? idOf(*pAtEnd) : 0;
            rContent.moRouting = routingOf(*pConnector);
        }

        /*
            The place of a glue point is given on the object, so that the client can move it,
            scale it and turn it with the object without asking anything. The way back from
            the page to the object is the transformation the entry carries, undone.
        */
        basegfx::B2DHomMatrix aFromTheObject(transformationInTwips(rObject));
        const bool bCanBeUndone(aFromTheObject.invert());

        /*
            The four at the middles of the sides are the object's own business: a rectangle
            or a picture turns them and shears them with itself and puts them out by half its
            line, a custom shape has its own, and anything else takes the middles of its box.
            So they are asked for, and given on the object like the others.
        */
        if (bCanBeUndone && !dynamic_cast<const SdrEdgeObj*>(&rObject))
        {
            for (sal_uInt16 nSide = 0; nSide < 4; ++nSide)
            {
                const Point aAt(rObject.GetVertexGluePoint(nSide).GetAbsolutePos(rObject));
                rContent.maSideGluePoints.push_back(
                    aFromTheObject
                    * basegfx::B2DPoint(aAt.X() * constTwipConversionFactor,
                                        aAt.Y() * constTwipConversionFactor));
            }
        }

        if (const SdrGluePointList* pGluePoints = rObject.GetGluePointList())
        {
            for (sal_uInt16 nGluePoint = 0; nGluePoint < pGluePoints->GetCount(); ++nGluePoint)
            {
                const SdrGluePoint& rGluePoint((*pGluePoints)[nGluePoint]);
                const Point aAt(rGluePoint.GetAbsolutePos(rObject));
                basegfx::B2DPoint aOnThePage(aAt.X() * constTwipConversionFactor,
                                             aAt.Y() * constTwipConversionFactor);

                if (bCanBeUndone)
                    aOnThePage = aFromTheObject * aOnThePage;

                ObjectContent::GluePoint aOne;
                aOne.mfX = aOnThePage.getX();
                aOne.mfY = aOnThePage.getY();
                aOne.mbShareOfTheSize = rGluePoint.IsPercent();
                aOne.mbFromTheShape = !rGluePoint.IsUserDefined();
                aOne.mnId = rGluePoint.GetId();

                /*
                    The six a point can hold, which are the six the interface a document is
                    read through knows. A mix of sides the model could hold besides them
                    turns into none of them on the way in, so it never reaches here.
                */
                switch (rGluePoint.GetEscDir())
                {
                    case SdrEscapeDirection::LEFT:
                        aOne.maWayOut = "left"_ostr;
                        break;
                    case SdrEscapeDirection::RIGHT:
                        aOne.maWayOut = "right"_ostr;
                        break;
                    case SdrEscapeDirection::TOP:
                        aOne.maWayOut = "top"_ostr;
                        break;
                    case SdrEscapeDirection::BOTTOM:
                        aOne.maWayOut = "bottom"_ostr;
                        break;
                    case SdrEscapeDirection::HORZ:
                        aOne.maWayOut = "horizontal"_ostr;
                        break;
                    case SdrEscapeDirection::VERT:
                        aOne.maWayOut = "vertical"_ostr;
                        break;
                    default:
                        break;
                }

                const SdrAlign eAcross(rGluePoint.GetHorzAlign());
                if (SdrAlign::HORZ_LEFT == eAcross)
                    aOne.maFromAcross = "left"_ostr;
                else if (SdrAlign::HORZ_RIGHT == eAcross)
                    aOne.maFromAcross = "right"_ostr;

                const SdrAlign eDown(rGluePoint.GetVertAlign());
                if (SdrAlign::VERT_TOP == eDown)
                    aOne.maFromDown = "top"_ostr;
                else if (SdrAlign::VERT_BOTTOM == eDown)
                    aOne.maFromDown = "bottom"_ostr;

                rContent.maOwnGluePoints.push_back(aOne);
            }
        }

        decomposeForComparison(rContent, rViewInformation);
    }

    /*
        What may be done to the object. The place and the size are each held against being
        changed on their own, while whether it may be turned is one of the things the object
        answers about itself - a media object, for one, will not be. A group answers about
        these as much as anything else does, so they are taken outside the part that only an
        object drawing something of its own goes through.
    */
    rContent.mbIsDiagram = rObject.isDiagram();
    rContent.maGraphicExtension = rObject.getGraphicExtension();

    if (const auto* pPicture = dynamic_cast<const SdrGrafObj*>(&rObject))
        takeInThePicture(*pPicture, rContent);

    /*
        The place held fast holds the turn and the mirror fast as well, and the size held fast
        the shear, as the office's own view has it.
    */
    SdrObjTransformInfoRec aWhatItAllows;
    rObject.TakeObjInfo(aWhatItAllows);
    const bool bPlaceHeldFast(rObject.IsMoveProtect());
    const bool bSizeHeldFast(rObject.IsResizeProtect());
    rContent.mbCanBeMoved = !bPlaceHeldFast;
    rContent.mbCanBeResized = !bSizeHeldFast;
    // a 3D scene turns in 3D with a drag of its own, it keeps no turn and no shear in 2D
    const bool bScene(nullptr != DynCastE3dScene(&rObject));
    rContent.mbCanBeTurned = aWhatItAllows.bRotateFreeAllowed && !bPlaceHeldFast && !bScene;
    rContent.mbCanBeResizedFreely = aWhatItAllows.bResizeFreeAllowed;
    rContent.mbCanBeMirrored = aWhatItAllows.bMirrorFreeAllowed && !bPlaceHeldFast;
    // The office lets a measurement be sheared, but the measurement stays at right angles to
    // its line whatever the two points do, so nothing comes of it, and none is offered.
    // a sheared line is just another line, same thing
    const SdrObjKind eKind(rObject.GetObjIdentifier());
    const bool bLineLike(SdrObjKind::Measure == eKind || SdrObjKind::Line == eKind);
    rContent.mbCanBeSheared
        = aWhatItAllows.bShearAllowed && !bSizeHeldFast && !bLineLike && !bScene;
    rContent.mbCanBeFlipped = aWhatItAllows.bMirror90Allowed && !bPlaceHeldFast;
    rContent.mbCanBeChangedToCurves = aWhatItAllows.bCanConvToPath;
    // a line is shaped by its two ends only, as a measurement is
    rContent.mbHandlesOfItsOwnOnly = bLineLike;

    rContent.maPaintedBox
        = paintedRectangleInTwips(rObject, rContent.maPrimitives, rViewInformation);
    rContent.maTransformation = transformationInTwips(rObject);

    // an object of a chart active in place lies on the chart's page, so its mapping, handles and
    // path are taken from there onto the chart's window, the primitives were above
    if (const Embedding* pEmbedding = embeddingOf(rObject))
    {
        const basegfx::B2DHomMatrix aInTwips(
            basegfx::utils::createScaleB2DHomMatrix(constTwipConversionFactor,
                                                    constTwipConversionFactor)
            * pEmbedding->maMapping
            * basegfx::utils::createScaleB2DHomMatrix(1.0 / constTwipConversionFactor,
                                                      1.0 / constTwipConversionFactor));
        rContent.maTransformation = aInTwips * rContent.maTransformation;
        rContent.maPath.transform(aInTwips);

        // the box is the one round the object on the chart's page, mapped: a group has no
        // primitives, and a part drawn inside a clip of the plot area answers that clip both
        // for its primitives and for its bound rectangle, while its logic rectangle is its own
        {
            basegfx::B2DRange aBox(vcl::unotools::b2DRectangleFromRectangle(
                rContent.maPrimitives.empty() ? rObject.GetCurrentBoundRect()
                                              : rObject.GetLogicRect()));
            aBox.transform(pEmbedding->maMapping);
            rContent.maPaintedBox = rangeInTwips(aBox);
        }
        for (ObjectContent::Handle& rHandle : rContent.maHandles)
        {
            const basegfx::B2DPoint aAt(
                aInTwips * basegfx::B2DPoint(rHandle.maPosition.X(), rHandle.maPosition.Y()));
            rHandle.maPosition = Point(basegfx::fround<tools::Long>(aAt.getX()),
                                       basegfx::fround<tools::Long>(aAt.getY()));
        }
    }

    if (const auto* pMeasurement = dynamic_cast<const SdrMeasureObj*>(&rObject))
    {
        const SfxItemSet& rSet = pMeasurement->GetMergedItemSet();
        ObjectContent::Measurement aMeasurement;
        aMeasurement.mfLineDistance = rSet.Get(SDRATTR_MEASURELINEDIST).GetValue();
        aMeasurement.mbBelow = rSet.Get(SDRATTR_MEASUREBELOWREFEDGE).GetValue();
        aMeasurement.mfOverhang = rSet.Get(SDRATTR_MEASUREHELPLINEOVERHANG).GetValue();
        aMeasurement.mfHelpLineDistance = rSet.Get(SDRATTR_MEASUREHELPLINEDIST).GetValue();
        aMeasurement.mfHelpLineLength1 = rSet.Get(SDRATTR_MEASUREHELPLINE1LEN).GetValue();
        aMeasurement.mfHelpLineLength2 = rSet.Get(SDRATTR_MEASUREHELPLINE2LEN).GetValue();
        aMeasurement.maValue = pMeasurement->GetValueText();
        aMeasurement.maUnit = pMeasurement->GetUnitText();
        aMeasurement.mnDecimals = rSet.Get(SDRATTR_MEASUREDECIMALPLACES).GetValue();
        aMeasurement.maDecimalSeparator = SvtSysLocale().GetLocaleData().getNumDecimalSep();
        pMeasurement->GetArrowSizes(aMeasurement.mfStartArrowWidth, aMeasurement.mfStartArrowLength,
                                    aMeasurement.mfEndArrowWidth, aMeasurement.mfEndArrowLength);
        aMeasurement.mfLineWidth = rSet.Get(XATTR_LINEWIDTH).GetValue();

        // A text turned a right angle stands across the line, so its width and height swap.
        aMeasurement.mbTextTurnedRightAngle = rSet.Get(SDRATTR_MEASURETEXTROTA90).GetValue();
        // The class of that item is private to svx; it is a yes or no all the same.
        aMeasurement.mbTextTurnsItself
            = static_cast<const SfxBoolItem&>(rSet.Get(sal_uInt16(SDRATTR_MEASURETEXTAUTOANGLE)))
                  .GetValue();
        const Size& rTextSize = pMeasurement->GetTextSize();
        aMeasurement.mfTextWidth
            = aMeasurement.mbTextTurnedRightAngle ? rTextSize.Height() : rTextSize.Width();
        aMeasurement.mfTextHeight
            = aMeasurement.mbTextTurnedRightAngle ? rTextSize.Width() : rTextSize.Height();
        aMeasurement.mfTextLeftDistance = pMeasurement->GetTextLeftDistance();
        aMeasurement.mfTextUpperDistance = pMeasurement->GetTextUpperDistance();
        aMeasurement.mfTextRightDistance = pMeasurement->GetTextRightDistance();
        aMeasurement.mfTextLowerDistance = pMeasurement->GetTextLowerDistance();

        switch (rSet.Get(SDRATTR_MEASURETEXTHPOS).GetValue())
        {
            case css::drawing::MeasureTextHorzPos_LEFTOUTSIDE:
                aMeasurement.mnTextAlong = -1;
                break;
            case css::drawing::MeasureTextHorzPos_INSIDE:
                aMeasurement.mnTextAlong = 0;
                break;
            case css::drawing::MeasureTextHorzPos_RIGHTOUTSIDE:
                aMeasurement.mnTextAlong = 1;
                break;
            default:
                break;
        }
        switch (rSet.Get(SDRATTR_MEASURETEXTVPOS).GetValue())
        {
            case css::drawing::MeasureTextVertPos_EAST:
                aMeasurement.mnTextAcross = -1;
                break;
            case css::drawing::MeasureTextVertPos_CENTERED:
                aMeasurement.mnTextAcross = 0;
                break;
            case css::drawing::MeasureTextVertPos_WEST:
                aMeasurement.mnTextAcross = 1;
                break;
            default:
                break;
        }

        rContent.moMeasurement = aMeasurement;
    }
    if (dynamic_cast<const SdrCaptionObj*>(&rObject))
    {
        // read as SdrCaptionObj::ImpGetCaptParams reads it
        const SfxItemSet& rSet = rObject.GetMergedItemSet();
        ObjectContent::Caption aCaption;
        aCaption.mnType = sal_Int32(rSet.Get(SDRATTR_CAPTIONTYPE).GetValue()) + 1;
        aCaption.mfGap
            = static_cast<const SdrCaptionGapItem&>(rSet.Get(SDRATTR_CAPTIONGAP)).GetValue()
              * constTwipConversionFactor;
        aCaption.mnEscapeDirection = sal_Int32(rSet.Get(SDRATTR_CAPTIONESCDIR).GetValue());
        aCaption.mbEscapeRelative = rSet.Get(SDRATTR_CAPTIONESCISREL).GetValue();
        aCaption.mnEscapeRelative = rSet.Get(SDRATTR_CAPTIONESCREL).GetValue();
        aCaption.mfEscapeAbsolute
            = rSet.Get(SDRATTR_CAPTIONESCABS).GetValue() * constTwipConversionFactor;
        aCaption.mfLineLength
            = rSet.Get(SDRATTR_CAPTIONLINELEN).GetValue() * constTwipConversionFactor;
        aCaption.mbFitLineLength = rSet.Get(SDRATTR_CAPTIONFITLINELEN).GetValue();
        rContent.moCaption = aCaption;
    }
    // the name of a part of a chart is what the chart knows it by, it stays here
    const OUString aPartName(partNameOf(rObject));
    rContent.maName = embeddingOf(rObject) ? OUString() : rObject.GetName();
    if (!aPartName.isEmpty() && idOf(rObject) != rObject.GetUniqueID())
        rContent.moChartPart = chartPartOf(aPartName);
    rContent.moLayer = sal_Int32(rObject.GetLayer().get());
    rContent.mbEmptyPlaceholder = rObject.IsEmptyPresObj();
    rContent.mbTextEdit = hasActiveTextEdit(&rObject);
    const SdrObject* pParent = holderOf(rObject);
    rContent.mnParentId = pParent ? idOf(*pParent) : 0;
}

void writeEntryGeometry(tools::JsonWriter& rWriter, const ObjectContent& rContent,
                        drawinglayer::Primitive2dJsonProcessor& rProcessor)
{
    const tools::Rectangle& rPainted = rContent.maPaintedBox;
    rWriter.put("x", sal_Int64(rPainted.Left()));
    rWriter.put("y", sal_Int64(rPainted.Top()));
    rWriter.put("width", sal_Int64(rPainted.GetWidth()));
    rWriter.put("height", sal_Int64(rPainted.GetHeight()));

    if (rContent.moMeasurement)
    {
        const auto& rMeasurement = *rContent.moMeasurement;
        auto aNode = rWriter.startNode("measurement");
        rWriter.put("lineDistance", rMeasurement.mfLineDistance);
        if (rMeasurement.mbBelow)
            rWriter.put("below", true);
        rWriter.put("overhang", rMeasurement.mfOverhang);
        rWriter.put("helpLineDistance", rMeasurement.mfHelpLineDistance);
        rWriter.put("helpLineLength1", rMeasurement.mfHelpLineLength1);
        rWriter.put("helpLineLength2", rMeasurement.mfHelpLineLength2);
        rWriter.put("value", rMeasurement.maValue);
        rWriter.put("unit", rMeasurement.maUnit);
        rWriter.put("decimals", sal_Int64(rMeasurement.mnDecimals));
        rWriter.put("decimalSeparator", rMeasurement.maDecimalSeparator);
        rWriter.put("startArrowWidth", rMeasurement.mfStartArrowWidth);
        rWriter.put("startArrowLength", rMeasurement.mfStartArrowLength);
        rWriter.put("endArrowWidth", rMeasurement.mfEndArrowWidth);
        rWriter.put("endArrowLength", rMeasurement.mfEndArrowLength);
        rWriter.put("lineWidth", rMeasurement.mfLineWidth);
        rWriter.put("textWidth", rMeasurement.mfTextWidth);
        rWriter.put("textHeight", rMeasurement.mfTextHeight);
        rWriter.put("textLeftDistance", rMeasurement.mfTextLeftDistance);
        rWriter.put("textUpperDistance", rMeasurement.mfTextUpperDistance);
        rWriter.put("textRightDistance", rMeasurement.mfTextRightDistance);
        rWriter.put("textLowerDistance", rMeasurement.mfTextLowerDistance);
        rWriter.put("textAlong", sal_Int64(rMeasurement.mnTextAlong));
        rWriter.put("textAcross", sal_Int64(rMeasurement.mnTextAcross));
        if (rMeasurement.mbTextTurnedRightAngle)
            rWriter.put("textTurnedRightAngle", true);
        if (rMeasurement.mbTextTurnsItself)
            rWriter.put("textTurnsItself", true);
    }

    if (rContent.moChartPart)
    {
        const auto& rPart = *rContent.moChartPart;
        auto aNode = rWriter.startNode("chartPart");
        if (rPart.mbMultiClick)
            rWriter.put("multiClick", true);
        if (rPart.mbDraggable)
            rWriter.put("draggable", true);
        if (rPart.mbResizable)
            rWriter.put("resizable", true);
        if (rPart.mbRotatable)
            rWriter.put("rotatable", true);
    }

    if (rContent.moCaption)
    {
        const auto& rCaption = *rContent.moCaption;
        auto aNode = rWriter.startNode("caption");
        rWriter.put("type", sal_Int64(rCaption.mnType));
        rWriter.put("gap", rCaption.mfGap);
        rWriter.put("escapeDirection", sal_Int64(rCaption.mnEscapeDirection));
        if (rCaption.mbEscapeRelative)
            rWriter.put("escapeRelative", sal_Int64(rCaption.mnEscapeRelative));
        else
            rWriter.put("escapeAbsolute", rCaption.mfEscapeAbsolute);
        if (rCaption.mbFitLineLength)
            rWriter.put("fitLineLength", true);
        else
            rWriter.put("lineLength", rCaption.mfLineLength);
    }

    // In the order a canvas takes it: x' = a * x + c * y + e and y' = b * x + d * y + f.
    {
        const basegfx::B2DHomMatrix& aTransformation = rContent.maTransformation;
        auto aTransformArray = rWriter.startArray("transform");
        rWriter.putSimpleValue(aTransformation.get(0, 0));
        rWriter.putSimpleValue(aTransformation.get(1, 0));
        rWriter.putSimpleValue(aTransformation.get(0, 1));
        rWriter.putSimpleValue(aTransformation.get(1, 1));
        rWriter.putSimpleValue(aTransformation.get(0, 2));
        rWriter.putSimpleValue(aTransformation.get(1, 2));
    }

    {
        auto aPrimitiveArray = rWriter.startArray("primitives");
        rProcessor.decomposeAndWrite(rContent.maPrimitives);
    }
    if (!rContent.maAids.empty())
    {
        auto aAidArray = rWriter.startArray("aids");
        rProcessor.decomposeAndWrite(rContent.maAids);
    }
    if (rContent.meKind != SdrObjKind::NONE)
        rWriter.put("objectKind", static_cast<sal_Int32>(rContent.meKind));
    if (rContent.mbIsDiagram)
        rWriter.put("isDiagram", true);
    if (!rContent.maGraphicExtension.isEmpty())
        rWriter.put("graphicExtension", rContent.maGraphicExtension);
    if (rContent.mbSigning)
        rWriter.put("signing", true);
    if (rContent.mbIsPicture)
    {
        rWriter.put("graphicChecksum", rContent.mnGraphicChecksum);
        if (rContent.mnGraphicMirror)
            rWriter.put("graphicMirror", rContent.mnGraphicMirror);
        rWriter.put("graphicWidth", rContent.mfGraphicWidth);
        rWriter.put("graphicHeight", rContent.mfGraphicHeight);
        rWriter.put("cropLeft", rContent.mfCropLeft);
        rWriter.put("cropTop", rContent.mfCropTop);
        rWriter.put("cropRight", rContent.mfCropRight);
        rWriter.put("cropBottom", rContent.mfCropBottom);
    }

    // Only what it may not have done travels, since most objects allow everything.
    if (!rContent.mbCanBeMoved)
        rWriter.put("cannotBeMoved", true);
    if (!rContent.mbCanBeResized)
        rWriter.put("cannotBeResized", true);
    if (!rContent.mbCanBeTurned)
        rWriter.put("cannotBeTurned", true);
    if (!rContent.mbCanBeResizedFreely)
        rWriter.put("cannotBeResizedFreely", true);
    if (!rContent.mbCanBeMirrored)
        rWriter.put("cannotBeMirrored", true);
    if (!rContent.mbCanBeSheared)
        rWriter.put("cannotBeSheared", true);
    if (!rContent.mbCanBeFlipped)
        rWriter.put("cannotBeFlipped", true);
    if (!rContent.mbCanBeChangedToCurves)
        rWriter.put("cannotBeChangedToCurves", true);
    if (rContent.mbHandlesOfItsOwnOnly)
        rWriter.put("handlesOfItsOwnOnly", true);

    if (rContent.mbTiedAtStart)
        rWriter.put("tiedAtStart", true);
    if (rContent.mbTiedAtEnd)
        rWriter.put("tiedAtEnd", true);
    if (rContent.mnTiedToAtStart)
        rWriter.put("tiedToAtStart", sal_Int64(rContent.mnTiedToAtStart));
    if (rContent.mnTiedToAtEnd)
        rWriter.put("tiedToAtEnd", sal_Int64(rContent.mnTiedToAtEnd));

    if (rContent.moRouting)
        writeRouting(rWriter, *rContent.moRouting);

    if (!rContent.maOwnGluePoints.empty())
    {
        auto aGlueNode = rWriter.startArray("gluePoints");
        for (const auto& rGluePoint : rContent.maOwnGluePoints)
        {
            auto aOneNode = rWriter.startStruct();
            rWriter.put("x", rGluePoint.mfX);
            rWriter.put("y", rGluePoint.mfY);
            rWriter.put("id", sal_Int32(rGluePoint.mnId));
            if (!rGluePoint.mbShareOfTheSize)
                rWriter.put("keepsItsDistance", true);
            if (rGluePoint.mbFromTheShape)
                rWriter.put("fromTheShape", true);
            if (!rGluePoint.maWayOut.isEmpty())
                rWriter.put("wayOut", rGluePoint.maWayOut);
            if (!rGluePoint.maFromAcross.isEmpty())
                rWriter.put("fromAcross", rGluePoint.maFromAcross);
            if (!rGluePoint.maFromDown.isEmpty())
                rWriter.put("fromDown", rGluePoint.maFromDown);
        }
    }

    if (!rContent.maSideGluePoints.empty())
    {
        auto aSideNode = rWriter.startArray("sideGluePoints");
        for (const auto& rPoint : rContent.maSideGluePoints)
        {
            auto aOneNode = rWriter.startStruct();
            rWriter.put("x", rPoint.getX());
            rWriter.put("y", rPoint.getY());
        }
    }

    if (rContent.maPath.count())
    {
        auto aPathArray = rWriter.startArray("path");
        for (const auto& rPolygon : rContent.maPath)
        {
            auto aPolygonNode = rWriter.startStruct();
            if (rPolygon.isClosed())
                rWriter.put("closed", true);

            auto aPointArray = rWriter.startArray("points");
            for (sal_uInt32 nPoint = 0; nPoint < rPolygon.count(); ++nPoint)
            {
                /*
                    The points travel in whole twips. A client that edits the path says which
                    point moved and where to, so nothing of what it was given comes back, and
                    a path of many thousand points stays short on the way out.
                */
                auto aPointNode = rWriter.startStruct();
                const basegfx::B2DPoint aPoint(rPolygon.getB2DPoint(nPoint));
                rWriter.put("x", basegfx::fround<sal_Int64>(aPoint.getX()));
                rWriter.put("y", basegfx::fround<sal_Int64>(aPoint.getY()));

                // The two weights of a curve, each written only where it is in use. They are
                // named as a handle names them: the one behind the point and the one ahead.
                if (rPolygon.isPrevControlPointUsed(nPoint))
                {
                    const basegfx::B2DPoint aBehind(rPolygon.getPrevControlPoint(nPoint));
                    rWriter.put("behindX", basegfx::fround<sal_Int64>(aBehind.getX()));
                    rWriter.put("behindY", basegfx::fround<sal_Int64>(aBehind.getY()));
                }
                if (rPolygon.isNextControlPointUsed(nPoint))
                {
                    const basegfx::B2DPoint aAhead(rPolygon.getNextControlPoint(nPoint));
                    rWriter.put("aheadX", basegfx::fround<sal_Int64>(aAhead.getX()));
                    rWriter.put("aheadY", basegfx::fround<sal_Int64>(aAhead.getY()));
                }

                /*
                    How the curve carries on through the point: 1 where the two weights lie on
                    one line through it, 2 where they lie on one line and are the same length
                    as well. It is worked out here from the points as the model holds them,
                    which are of a finer grain than the twips that travel, and rounding them
                    first can turn either answer into the other.
                */
                const basegfx::B2VectorContinuity eContinuity(
                    rPolygon.getContinuityInPoint(nPoint));
                if (eContinuity != basegfx::B2VectorContinuity::NONE)
                    rWriter.put("continuity",
                                eContinuity == basegfx::B2VectorContinuity::C2 ? 2 : 1);
            }
        }
    }
    if (!rContent.maHandles.empty())
    {
        auto aHandleArray = rWriter.startArray("handles");
        for (const auto& rHandle : rContent.maHandles)
        {
            auto aHandleNode = rWriter.startStruct();
            rWriter.put("kind", rHandle.mnKind);
            if (rHandle.mnPolygon)
                rWriter.put("polygon", sal_Int64(rHandle.mnPolygon));
            if (rHandle.mnPoint)
                rWriter.put("point", sal_Int64(rHandle.mnPoint));
            if (rHandle.mnObjHandle)
                rWriter.put("at", sal_Int64(rHandle.mnObjHandle));
            rWriter.put("x", sal_Int64(rHandle.maPosition.X()));
            rWriter.put("y", sal_Int64(rHandle.maPosition.Y()));
            if (rHandle.mbLarger)
                rWriter.put("larger", true);

            if (rHandle.mbHasRails)
            {
                const auto& rRails = rHandle.maRails;
                auto aRailNode = rWriter.startNode("rails");

                // A place is written on the object's own square, where 0 and 0 is its
                // upper left corner and 1 and 1 its lower right one.
                auto aPlace =
                    [&rWriter](const char* pName, const bool bHas, const basegfx::B2DPoint& rPlace)
                {
                    if (!bHas)
                        return;

                    auto aNode = rWriter.startNode(pName);
                    rWriter.put("x", rPlace.getX());
                    rWriter.put("y", rPlace.getY());
                };

                if (SdrHandleRailKind::Around == rRails.meKind)
                {
                    rWriter.put("movesAround", true);
                    aPlace("around", true, rRails.maAround);
                    aPlace("nearest", rRails.mbHasNearest, rRails.maNearest);
                    aPlace("furthest", rRails.mbHasFurthest, rRails.maFurthest);
                    aPlace("from", rRails.mbHasFrom, rRails.maFrom);
                    aPlace("to", rRails.mbHasTo, rRails.maTo);
                }
                else
                {
                    if (rRails.mbMovesAcross)
                        rWriter.put("movesAcross", true);
                    if (rRails.mbMovesDown)
                        rWriter.put("movesDown", true);
                    aPlace("leastAcross", rRails.mbHasLeastAcross, rRails.maLeastAcross);
                    aPlace("mostAcross", rRails.mbHasMostAcross, rRails.maMostAcross);
                    aPlace("leastDown", rRails.mbHasLeastDown, rRails.maLeastDown);
                    aPlace("mostDown", rRails.mbHasMostDown, rRails.maMostDown);
                }

                if (rRails.mbKeepsItsLength)
                    rWriter.put("keepsItsLength", true);
            }
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
