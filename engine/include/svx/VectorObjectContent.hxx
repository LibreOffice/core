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

#pragma once

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <svx/sdrhandlerails.hxx>
#include <svx/svdobjkind.hxx>
#include <svx/svxdllapi.h>
#include <tools/gen.hxx>

#include <optional>
#include <vector>

class SdrObject;
class SdrObjList;

namespace basegfx
{
class B2DRange;
}

namespace drawinglayer
{
class Primitive2dJsonProcessor;
}

namespace drawinglayer::geometry
{
class ViewInformation2D;
}

namespace tools
{
class JsonWriter;
}

/// The content of a drawing object as a client that draws vector primitives receives it.
namespace svx::vector
{
/// What was last written for one object. A change that leaves none of this different is a
/// change nobody can see, so it need not travel. The box and the transformation are held
/// alongside the primitives because an object can move without its primitives changing.
struct ObjectContent
{
    /// What is written for the object.
    drawinglayer::primitive2d::Primitive2DContainer maPrimitives;

    /// What those primitives decompose to, which is what the writing walks and so what a
    /// client receives. The two do not move together: an object shows none of its own text
    /// while an edit runs on it, so its model text can change while what it draws does not.
    /// The decomposition is what is compared, so the object travels when what it draws
    /// changes.
    drawinglayer::primitive2d::Primitive2DContainer maDrawn;

    /// The aids that mark out a placeholder: a dashed boundary around the area it occupies
    /// and, on a master page, the name of the area. Empty for an object that is not a
    /// placeholder. They are written apart from the primitives.
    drawinglayer::primitive2d::Primitive2DContainer maAids;

    /** One handle of the object that a reader cannot work out from what the object draws:
        the corner radius of a rectangle and the points a custom shape is shaped by.

        What names the handle is what it means, not where it sits in a list: the kind, and
        with it the polygon and the point it belongs to. A handle keeps that name however
        many handles the object has and whatever else is selected, where a place in a list
        holds only for one object on its own.

        The position is in twips.
     */
    struct Handle
    {
        sal_Int32 mnKind = 0;
        sal_uInt32 mnPolygon = 0;
        sal_uInt32 mnPoint = 0;

        /// The number the object itself gives the handle, which is what tells apart the
        /// handles that are alike in every other way.
        sal_uInt32 mnObjHandle = 0;
        Point maPosition;

        /** How far the handle may be moved, for a point a custom shape is shaped by, on the
            shape's own square: 0 and 0 is its upper left corner and 1 and 1 its lower right
            one. Says nothing for a handle of another kind.
         */
        bool mbHasRails = false;
        SdrHandleRails maRails;

        /// True where the handle is drawn larger than the others, so that it can still be
        /// taken hold of where a handle of the usual size lies over it: the help line ends of
        /// a measurement, which stand on the ends of the measured line until moved.
        bool mbLarger = false;

        bool operator==(const Handle& rOther) const
        {
            return mnKind == rOther.mnKind && mnPolygon == rOther.mnPolygon
                   && mnPoint == rOther.mnPoint && mnObjHandle == rOther.mnObjHandle
                   && maPosition == rOther.maPosition && mbLarger == rOther.mbLarger
                   && mbHasRails == rOther.mbHasRails
                   && (!mbHasRails || sameRails(maRails, rOther.maRails));
        }

        static bool sameRails(const SdrHandleRails& rOne, const SdrHandleRails& rOther)
        {
            return rOne.meKind == rOther.meKind && rOne.mbMovesAcross == rOther.mbMovesAcross
                   && rOne.mbMovesDown == rOther.mbMovesDown
                   && rOne.mbHasLeastAcross == rOther.mbHasLeastAcross
                   && rOne.mbHasMostAcross == rOther.mbHasMostAcross
                   && rOne.mbHasLeastDown == rOther.mbHasLeastDown
                   && rOne.mbHasMostDown == rOther.mbHasMostDown
                   && rOne.maLeastAcross == rOther.maLeastAcross
                   && rOne.maMostAcross == rOther.maMostAcross
                   && rOne.maLeastDown == rOther.maLeastDown && rOne.maMostDown == rOther.maMostDown
                   && rOne.maAround == rOther.maAround && rOne.mbHasNearest == rOther.mbHasNearest
                   && rOne.mbHasFurthest == rOther.mbHasFurthest
                   && rOne.maNearest == rOther.maNearest && rOne.maFurthest == rOther.maFurthest
                   && rOne.mbHasFrom == rOther.mbHasFrom && rOne.mbHasTo == rOther.mbHasTo
                   && rOne.maFrom == rOther.maFrom && rOne.maTo == rOther.maTo
                   && rOne.mbKeepsItsLength == rOther.mbKeepsItsLength;
        }
    };

    /// The handles above, in the order the object adds them, empty for an object that has
    /// none of that kind.
    std::vector<Handle> maHandles;

    /** The path of an object that is drawn from one, in twips: the polygons and the points
        the model holds, with the control points of a curve as they stand there. They keep
        the order the model has them in, which is the order a handle is named by, so that a
        point spoken about here is the point the engine will look up. Empty for an object
        that has no path.
     */
    basegfx::B2DPolyPolygon maPath;

    /// What kind of object it is, as the drawing layer numbers the kinds.
    SdrObjKind meKind = SdrObjKind::NONE;

    /** What may be done to the object, which is the object's own business and not the
        selection's: whether it may be moved, made larger or smaller, and turned. A reader
        that offers a drag asks every object it would change and offers it only where they
        all allow it.
     */
    bool mbCanBeMoved = true;
    bool mbCanBeResized = true;
    bool mbCanBeTurned = true;
    /// False where the object allows a resize in ratio only: a text frame that is turned
    /// off the right angles, a shape that is turned at all. A resize of a selection holding
    /// such an object keeps the ratio for the whole selection, as the office does.
    bool mbCanBeResizedFreely = true;
    /// False where the object may not be turned over, a media object for one.
    bool mbCanBeMirrored = true;
    /// False where the object cannot hold a shear at all, a text frame for one. A resize of
    /// such an object with two different factors is held to the ratio while it stands off
    /// the right angles, since the shear it would bring in has nowhere to go.
    bool mbCanBeSheared = true;
    /// False where the object may not be turned over across or down as a whole, a text
    /// frame or a caption for one.
    bool mbCanBeFlipped = true;
    /// False where the object cannot be changed into curves, a measurement for one.
    bool mbCanBeChangedToCurves = true;
    /// True where the object, selected on its own, is shaped by its own handles alone and
    /// has no frame of eight around it, a measurement for one.
    bool mbHandlesOfItsOwnOnly = false;

    /** True where the object is a diagram, which is a set of shapes the office lays out
        from data it holds beside them, and the file name ending of the picture an object
        holds, empty where it holds none. Both belong to the object and are asked of it.
     */
    bool mbIsDiagram = false;
    OUString maGraphicExtension;

    /** True while the document is being signed and a certificate has been chosen. The
        document is read-only then, and the object may still be moved and resized: it is
        how the signature line is placed.
     */
    bool mbSigning = false;

    /** For a picture, the size the graphic has by its own resolution and the four amounts
        cropped away from it, left, top, right and bottom, all in twips of that size. The
        object's rectangle is the window onto what is left, so the picture as a whole would
        stretch from the window by these amounts. Zero throughout for anything else, with
        mbIsPicture false.
     */
    bool mbIsPicture = false;
    sal_Int64 mnGraphicChecksum = 0;
    /// How the graphic is turned over as it is drawn: bit 0 across, bit 1 down.
    sal_Int32 mnGraphicMirror = 0;
    double mfGraphicWidth = 0.0;
    double mfGraphicHeight = 0.0;
    double mfCropLeft = 0.0;
    double mfCropTop = 0.0;
    double mfCropRight = 0.0;
    double mfCropBottom = 0.0;

    /// True for a connector whose first end is tied to an object, and the same for its last
    /// end. A connector runs from where it is tied, so a tied end stays with the object it
    /// holds on to. False for anything that is not a connector.
    bool mbTiedAtStart = false;
    bool mbTiedAtEnd = false;
    /// The unique id of the object a tied end holds on to, zero for a free end and for
    /// anything that is not a connector. The connector names what it is tied to. Which
    /// connectors hang on an object follows from that.
    sal_uInt64 mnTiedToAtStart = 0;
    sal_uInt64 mnTiedToAtEnd = 0;

    /** How one end of a connector holds on to its object, or where it lies when it holds on
        to none. A tied end has the number of the glue point it is tied to, or none while the
        connector picks the best point itself, and the three flags say how that pick goes:
        whether the best of the four sides is searched for, whether the pick landed on a
        side or on a corner. A free end has its place on the page instead, in twips. */
    struct ConnectorTie
    {
        std::optional<sal_uInt16> moPoint;
        bool mbBestVertex = false;
        bool mbAutoVertex = false;
        bool mbAutoCorner = false;
        std::optional<basegfx::B2DPoint> moAt;

        bool operator==(const ConnectorTie&) const = default;
    };

    /** The numbers a connector's run between its two ends is worked out from, all in twips
        where they are lengths. Only a connector has them.

        The kind names the shape of the run: "ortho" for lines meeting at right angles,
        "three" for three straight lines, "one" for one straight line, "bezier" for a curve
        and "arc" for an arc. The curve follows the OOXML way of routing when the flag says
        so, and the office's own way otherwise.

        The node distances are how far the run keeps away from each object it is tied to:
        across and down for the first object, then for the second.

        The line deltas are how far the user moved the lines of an orthogonal run from where
        the routing would have put them: up to three of them, and the count says how many
        are set.

        The run is laid out against the snap rectangle of each object when the document asks
        for it, against the object's bound rectangle otherwise. */
    struct ConnectorRouting
    {
        OString maKind;
        bool mbCurveFollowsOOXML = false;
        double mfNode1Across = 0.0;
        double mfNode1Down = 0.0;
        double mfNode2Across = 0.0;
        double mfNode2Down = 0.0;
        sal_Int32 mnLineDeltaCount = 0;
        double mfLineDelta1 = 0.0;
        double mfLineDelta2 = 0.0;
        double mfLineDelta3 = 0.0;
        bool mbAgainstSnapRectangles = false;
        /// True for a run that was set from outside, by the file it was read from or by
        /// hand, and is kept as it is rather than laid out from the numbers above.
        bool mbTrackUserDefined = false;
        ConnectorTie maTieAtStart;
        ConnectorTie maTieAtEnd;

        bool operator==(const ConnectorRouting&) const = default;
    };

    /// The routing numbers of a connector. Nothing for anything else.
    std::optional<ConnectorRouting> moRouting;

    /** One of the points someone added to the object for a connector to tie itself to. The
        place is given on the object rather than on the page: 0 and 0 is the upper left corner
        of the object and 1 and 1 the lower right one, so the point follows the object
        wherever it goes and however it is turned.

        A point that is held as a share of the object's size keeps that place when the object
        is made larger or smaller. One that is held as a distance of its own keeps the
        distance instead, so its place on the object changes with the size.
     */
    struct GluePoint
    {
        double mfX = 0.0;
        double mfY = 0.0;
        bool mbShareOfTheSize = true;

        /** The way a connector leaves the object at this point: "left", "right", "top",
            "bottom", or "horizontal" and "vertical" for the two ways that face each other.
            Empty when the point names none and the way is worked out from where it lies.

            These are the ways a point can hold. The model itself can hold any mix of the
            four sides, but neither the interface a document is read through nor the file it
            is read from can carry one, so a mix never arrives here.
         */
        OString maWayOut;

        /** The side the place of the point is measured from, "left" or "right" across the
            object and "top" or "bottom" down it. Empty where it is measured from the middle,
            which is what a point is given when nothing else is said.
         */
        OString maFromAcross;
        OString maFromDown;

        /// The number the point is known by on the object, which is how a connector tied
        /// to it names it.
        sal_uInt16 mnId = 0;

        /// True for a point a shape makes from its own geometry, which it lays out again
        /// with the shape whenever that changes; false for a point given to the object.
        bool mbFromTheShape = false;

        bool operator==(const GluePoint& rOther) const
        {
            return mfX == rOther.mfX && mfY == rOther.mfY && mnId == rOther.mnId
                   && mbShareOfTheSize == rOther.mbShareOfTheSize && maWayOut == rOther.maWayOut
                   && maFromAcross == rOther.maFromAcross && maFromDown == rOther.maFromDown
                   && mbFromTheShape == rOther.mbFromTheShape;
        }
    };

    /// The points the object was given, in the order it holds them. Empty for an object that
    /// was given none, and then the four middles of the sides of its box are what a connector
    /// can reach for.
    std::vector<GluePoint> maOwnGluePoints;
    /// The four glue points every object has at the middles of its sides - the upper, the
    /// right, the lower and the left one - where they lie on the object, as the points in
    /// maOwnGluePoints are given. Empty for a connector.
    std::vector<basegfx::B2DPoint> maSideGluePoints;

    tools::Rectangle maPaintedBox;
    basegfx::B2DHomMatrix maTransformation;
    /** What shapes a measurement beyond its two points, for a reader that draws it while one
        of its handles is dragged. The lengths are in the units of the model, which are the
        units across the measurement's square: its second side is one unit long.
     */
    struct Measurement
    {
        /// How far the dimension line stands from the measured line.
        double mfLineDistance = 0.0;
        /// True where it stands below the measured line, across the square's way down.
        bool mbBelow = false;
        /// How far the help lines reach beyond the dimension line.
        double mfOverhang = 0.0;
        /// How far from the measured line the help lines begin, before their own lengths.
        double mfHelpLineDistance = 0.0;
        /// How much longer each help line is towards the measured line, at the first point
        /// and at the second.
        double mfHelpLineLength1 = 0.0;
        double mfHelpLineLength2 = 0.0;
        /// The value as the text writes it, the unit after it, how many places the value
        /// is written with, and the character between whole and part.
        OUString maValue;
        OUString maUnit;
        sal_Int16 mnDecimals = 0;
        OUString maDecimalSeparator;
        /// The arrows at the start and the end: how wide each is and how far it reaches
        /// along the line, nought where there is none. The width of the line.
        double mfStartArrowWidth = 0.0;
        double mfStartArrowLength = 0.0;
        double mfEndArrowWidth = 0.0;
        double mfEndArrowLength = 0.0;
        double mfLineWidth = 0.0;
        /// The size of the text as laid out, before any turn, and its distances to the four
        /// sides of its frame: left, upper, right, lower.
        double mfTextWidth = 0.0;
        double mfTextHeight = 0.0;
        double mfTextLeftDistance = 0.0;
        double mfTextUpperDistance = 0.0;
        double mfTextRightDistance = 0.0;
        double mfTextLowerDistance = 0.0;
        /// Where the text stands along the line: -1 before the first point, 0 between the
        /// two, 1 beyond the second, 2 where the measurement decides. Across the line: -1
        /// on the side away from the measured line, 0 on the dimension line, 1 on the side
        /// towards it, 2 where the measurement decides.
        sal_Int32 mnTextAlong = 2;
        sal_Int32 mnTextAcross = 2;
        /// Whether the text is turned a right angle, and whether it turns itself upside down
        /// where it would otherwise be read upside down.
        bool mbTextTurnedRightAngle = false;
        bool mbTextTurnsItself = false;

        bool operator==(const Measurement&) const = default;
    };

    /// Nothing for any object that is not a measurement.
    std::optional<Measurement> moMeasurement;
    /** What lays out the tail of a caption from its end and its box, as the caption's own
        settings say: the lengths are in twips, the box is the one of the mapping.
     */
    struct Caption
    {
        /// The kind of tail, 1 to 4: 1 a line bent once, 2 a straight line, 3 and 4 a line
        /// with a piece of its own length coming out of the box.
        sal_Int32 mnType = 3;
        /// How far from the box the tail begins.
        double mfGap = 0.0;
        /// Which sides the tail may leave the box at: 0 left or right, 1 top or bottom, 2
        /// whichever is nearest to the end.
        sal_Int32 mnEscapeDirection = 0;
        /// Where along the side the tail leaves, in 1/10000 of the side where relative, or
        /// as a length from the upper left corner.
        bool mbEscapeRelative = true;
        sal_Int32 mnEscapeRelative = 5000;
        double mfEscapeAbsolute = 0.0;
        /// How long the piece coming out of the box is, unless it is fitted to half the way.
        double mfLineLength = 0.0;
        bool mbFitLineLength = true;

        bool operator==(const Caption&) const = default;
    };

    /// Nothing for any object that is not a caption.
    std::optional<Caption> moCaption;
    /** What a part of a chart active in place allows, for a part the chart can select on its
        own: whether a first click selects the part holding it and only the next one this part,
        as a data point in its series, and whether it can be dragged, resized and turned.
     */
    struct ChartPart
    {
        bool mbMultiClick = false;
        bool mbDraggable = false;
        bool mbResizable = false;
        bool mbRotatable = false;

        bool operator==(const ChartPart&) const = default;
    };

    /// Nothing for any object that is no part of a chart active in place with a selection of
    /// its own.
    std::optional<ChartPart> moChartPart;
    /** Where the chart's own keys go from a part of a chart active in place, by the ids the
        vector view knows the parts by, 0 for none: the next and the previous part of its level,
        going round, the first part a level down and the part a level up. The first part of the
        top level is marked, the chart starts there.
     */
    struct ChartKeys
    {
        sal_uInt64 mnNext = 0;
        sal_uInt64 mnPrevious = 0;
        sal_uInt64 mnInside = 0;
        sal_uInt64 mnOutside = 0;
        bool mbFirst = false;

        bool operator==(const ChartKeys&) const = default;
    };

    /// Nothing for any object the chart's keys do not go to.
    std::optional<ChartKeys> moChartKeys;
    /// The name the object carries, empty for an entry that stands for no object.
    OUString maName;
    /// The layer the entry draws with, nothing for an entry that belongs to no object. For
    /// the entry of a running text edit it is the layer of the object the edit runs on.
    std::optional<sal_Int32> moLayer;
    /// True for a placeholder that holds none of its own content yet.
    bool mbEmptyPlaceholder = false;
    /// True while a text edit runs on the object. It is compared along with the rest so
    /// that an edit which ends without changing the text still reaches the client.
    bool mbTextEdit = false;
    /// The id of the group the object sits in, 0 for an object directly on the page. For
    /// the entry of a running text edit, the id of the object the edit runs on.
    sal_uInt64 mnParentId = 0;

    bool operator==(const ObjectContent& rOther) const
    {
        return maPaintedBox == rOther.maPaintedBox && maTransformation == rOther.maTransformation
               && moMeasurement == rOther.moMeasurement && moCaption == rOther.moCaption
               && moChartPart == rOther.moChartPart && moChartKeys == rOther.moChartKeys
               && maName == rOther.maName && moLayer == rOther.moLayer
               && mbEmptyPlaceholder == rOther.mbEmptyPlaceholder && mbTextEdit == rOther.mbTextEdit
               && mnParentId == rOther.mnParentId && maDrawn == rOther.maDrawn
               && maAids == rOther.maAids && maHandles == rOther.maHandles
               && maPath == rOther.maPath && meKind == rOther.meKind
               && mbIsDiagram == rOther.mbIsDiagram
               && maGraphicExtension == rOther.maGraphicExtension && mbSigning == rOther.mbSigning
               && mbIsPicture == rOther.mbIsPicture && mnGraphicChecksum == rOther.mnGraphicChecksum
               && mnGraphicMirror == rOther.mnGraphicMirror
               && mfGraphicWidth == rOther.mfGraphicWidth
               && mfGraphicHeight == rOther.mfGraphicHeight && mfCropLeft == rOther.mfCropLeft
               && mfCropTop == rOther.mfCropTop && mfCropRight == rOther.mfCropRight
               && mfCropBottom == rOther.mfCropBottom && mbCanBeMoved == rOther.mbCanBeMoved
               && mbCanBeResized == rOther.mbCanBeResized && mbCanBeTurned == rOther.mbCanBeTurned
               && mbCanBeResizedFreely == rOther.mbCanBeResizedFreely
               && mbCanBeMirrored == rOther.mbCanBeMirrored
               && mbCanBeSheared == rOther.mbCanBeSheared && mbCanBeFlipped == rOther.mbCanBeFlipped
               && mbCanBeChangedToCurves == rOther.mbCanBeChangedToCurves
               && mbHandlesOfItsOwnOnly == rOther.mbHandlesOfItsOwnOnly
               && mbTiedAtStart == rOther.mbTiedAtStart && mbTiedAtEnd == rOther.mbTiedAtEnd
               && mnTiedToAtStart == rOther.mnTiedToAtStart && mnTiedToAtEnd == rOther.mnTiedToAtEnd
               && moRouting == rOther.moRouting && maOwnGluePoints == rOther.maOwnGluePoints
               && maSideGluePoints == rOther.maSideGluePoints;
    }
};

/** The members of an object as the vector view takes them: those of a group, and nothing for any
    other object. A 3D scene holds its 3D objects as members but draws them as one picture of its
    own, so it is taken whole. A chart active in place has the objects of the page it draws on as
    members, as long as it is active.
 */
SVXCORE_DLLPUBLIC const SdrObjList* membersOf(const SdrObject& rObject);

/** The id the vector view knows the object by: its own, or for a part of a chart active in place
    that the chart selects on its own, one kept for that part. The chart makes its shapes anew on
    every change, so their own ids change, while the part stays the same.
 */
SVXCORE_DLLPUBLIC sal_uInt64 idOf(const SdrObject& rObject);

/** The name a chart knows that part of it by, for a part of a chart active in place that the
    chart selects on its own, without what a drag of it changes in the name. Empty for anything
    else.
 */
SVXCORE_DLLPUBLIC OUString chartPartNameOf(const SdrObject& rObject);

/// The same name for any object of the part, inner objects of the same name included.
SVXCORE_DLLPUBLIC OUString chartPartNameAround(const SdrObject& rObject);

/** The object of the chart active in place that the vector view knows by that id: a part the
    chart selects on its own by the id kept for it, any other object of the chart's page by its
    own id, a shape someone added to the chart for one. Nothing where the chart holds no such
    object.
 */
SVXCORE_DLLPUBLIC SdrObject* chartPartById(sal_uInt64 nId);

/** The mapping from the page of a chart active in place onto the chart's window on the slide,
    in the units of the model, for an object on that page. Nothing for any other object.
 */
SVXCORE_DLLPUBLIC std::optional<basegfx::B2DHomMatrix> chartMappingOf(const SdrObject& rObject);

/** The object holding that one as the vector view takes it: its group, or for an object on the
    page of a chart active in place, that chart. Nothing for an object directly on the page.
 */
SVXCORE_DLLPUBLIC const SdrObject* holderOf(const SdrObject& rObject);

/** Fills in what the object says about itself: its handles, path, connector ties, glue points,
    what may be done to it, the box it paints and the mapping of its square onto the page, all in
    twips. The objects inside a group draw the group's content, so a group with members has no
    primitives of its own. A leaf object with no primitives set yet takes the ones the object
    draws on its own. The aids and whether the document is being signed are left as they are.
 */
SVXCORE_DLLPUBLIC void
describeObject(SdrObject& rObject, ObjectContent& rContent,
               const drawinglayer::geometry::ViewInformation2D& rViewInformation);

/// Fills in what the primitives of the content decompose to, which is what is compared. A
/// primitive that decomposes to nothing is a leaf, a fill or a background color, and is drawn
/// itself, so it is compared itself.
SVXCORE_DLLPUBLIC void
decomposeForComparison(ObjectContent& rContent,
                       const drawinglayer::geometry::ViewInformation2D& rViewInformation);

/// Writes where an entry paints, how the unit rectangle maps onto it, and how it looks, into the
/// open entry.
SVXCORE_DLLPUBLIC void writeEntryGeometry(tools::JsonWriter& rWriter, const ObjectContent& rContent,
                                          drawinglayer::Primitive2dJsonProcessor& rProcessor);

/// The range, in 1/100 mm, as an upright box in twips, empty for an empty range.
SVXCORE_DLLPUBLIC tools::Rectangle rangeInTwips(const basegfx::B2DRange& rRange);

/// The mapping of the unit rectangle onto an upright box, for an entry that has no
/// transformation of its own.
SVXCORE_DLLPUBLIC basegfx::B2DHomMatrix boxTransformation(const tools::Rectangle& rBox);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
