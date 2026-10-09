/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages2.hpp>
#include <com/sun/star/drawing/XDrawPageDuplicator.hpp>
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <svx/sdrhandlerails.hxx>
#include <svx/svdobjkind.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <tools/gen.hxx>
#include <tools/Guid.hxx>
#include <rtl/ref.hxx>
#include <unotools/weakref.hxx>

#include <memory>
#include <unordered_map>
#include <optional>
#include <unordered_set>
#include <vector>

#include <vcl/BinaryDataContainer.hxx>
#include <vcl/graph.hxx>

#include <sfx2/sfxbasemodel.hxx>
#include <svx/fmdmod.hxx>

#include <vcl/ITiledRenderable.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include "sddllapi.h"

namespace com::sun::star::i18n { class XForbiddenCharacters; }
namespace avmedia { struct MediaTempFile; }

class SdrGrafObj;
class SdrObject;
class SdrPage;
class SdDrawDocument;
class SdPage;
class SvxItemPropertySet;
class SdUnoForbiddenCharsTable;
class SdDrawPagesAccess;
class SdMasterPagesAccess;
class SdLayerManager;
class SdXCustomPresentationAccess;
class SdDocLinkTargets;
class SdGenericDrawPage;
class SvxDrawPage;

namespace sd {
class DrawDocShell;
class DrawViewShell;
class SlideshowLayerRenderer;
class SlideShow;
}

extern OUString getPageApiName( SdPage const * pPage );
extern OUString getPageApiNameFromUiName( const OUString& rUIName );

class SAL_DLLPUBLIC_RTTI SdXImpressDocument final : public SfxBaseModel, // implements SfxListener, OWEAKOBJECT & other
                           public SvxFmMSFactory,
                           public css::drawing::XDrawPageDuplicator,
                           public css::drawing::XLayerSupplier,
                           public css::drawing::XMasterPagesSupplier,
                           public css::drawing::XDrawPagesSupplier,
                           public css::presentation::XPresentationSupplier,
                           public css::presentation::XCustomPresentationSupplier,
                           public css::document::XLinkTargetSupplier,
                           public css::beans::XPropertySet,
                           public css::style::XStyleFamiliesSupplier,
                           public css::lang::XServiceInfo,
                           public css::ucb::XAnyCompareFactory,
                           public css::presentation::XHandoutMasterSupplier,
                           public css::view::XRenderable,
                           public vcl::ITiledRenderable
{
    friend class SdDrawPagesAccess;
    friend class SdMasterPagesAccess;
    friend class SdLayerManager;

public:
    /// Cache of graphics from vector rendering, keyed by checksum.
    std::unordered_map<sal_Int64, Graphic>& getBitmapCache() { return maBitmapCache; }

    /// Cache of font files used by vector-rendering text, keyed by a
    /// content hash of the font bytes.
    std::unordered_map<sal_uInt64, BinaryDataContainer>& getVectorFontCache()
    {
        return maVectorFontCache;
    }

    /// What each font key resolved to, once per document.
    std::unordered_map<OUString, drawinglayer::Primitive2dJsonProcessor::ResolvedFace>&
    getVectorFontFaceByKey()
    {
        return maVectorFontFaceByKey;
    }

    /// What was last written for one object. A change that leaves none of this different is a
    /// change nobody can see, so it need not travel. The box and the transformation are held
    /// alongside the primitives because an object can move without its primitives changing.
    struct VectorObjectContent
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

            bool operator==(const Handle& rOther) const
            {
                return mnKind == rOther.mnKind && mnPolygon == rOther.mnPolygon
                       && mnPoint == rOther.mnPoint && mnObjHandle == rOther.mnObjHandle
                       && maPosition == rOther.maPosition
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
                       && rOne.maLeastDown == rOther.maLeastDown
                       && rOne.maMostDown == rOther.maMostDown
                       && rOne.maAround == rOther.maAround
                       && rOne.mbHasNearest == rOther.mbHasNearest
                       && rOne.mbHasFurthest == rOther.mbHasFurthest
                       && rOne.maNearest == rOther.maNearest
                       && rOne.maFurthest == rOther.maFurthest
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

            bool operator==(const GluePoint& rOther) const
            {
                return mfX == rOther.mfX && mfY == rOther.mfY && mnId == rOther.mnId
                       && mbShareOfTheSize == rOther.mbShareOfTheSize
                       && maWayOut == rOther.maWayOut
                       && maFromAcross == rOther.maFromAcross
                       && maFromDown == rOther.maFromDown;
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
        /// The page background the automatic color of the object's text resolved against when
        /// it was written, COL_TRANSPARENT when it has no text. The decomposition stops above
        /// the text and resolves the color only when drawn, so the background is compared too.
        Color maAutoColor = COL_TRANSPARENT;

        bool operator==(const VectorObjectContent& rOther) const
        {
            return maPaintedBox == rOther.maPaintedBox
                   && maTransformation == rOther.maTransformation && maName == rOther.maName
                   && moLayer == rOther.moLayer && mbEmptyPlaceholder == rOther.mbEmptyPlaceholder
                   && mbTextEdit == rOther.mbTextEdit && mnParentId == rOther.mnParentId
                   && maAutoColor == rOther.maAutoColor && maDrawn == rOther.maDrawn
                   && maAids == rOther.maAids && maHandles == rOther.maHandles
                   && maPath == rOther.maPath && meKind == rOther.meKind
                   && mbIsDiagram == rOther.mbIsDiagram
                   && maGraphicExtension == rOther.maGraphicExtension
                   && mbSigning == rOther.mbSigning
                   && mbIsPicture == rOther.mbIsPicture
                   && mnGraphicChecksum == rOther.mnGraphicChecksum
                   && mnGraphicMirror == rOther.mnGraphicMirror
                   && mfGraphicWidth == rOther.mfGraphicWidth
                   && mfGraphicHeight == rOther.mfGraphicHeight
                   && mfCropLeft == rOther.mfCropLeft && mfCropTop == rOther.mfCropTop
                   && mfCropRight == rOther.mfCropRight
                   && mfCropBottom == rOther.mfCropBottom
                   && mbCanBeMoved == rOther.mbCanBeMoved
                   && mbCanBeResized == rOther.mbCanBeResized
                   && mbCanBeTurned == rOther.mbCanBeTurned
                   && mbCanBeResizedFreely == rOther.mbCanBeResizedFreely
                   && mbCanBeMirrored == rOther.mbCanBeMirrored
                   && mbCanBeSheared == rOther.mbCanBeSheared
                   && mbTiedAtStart == rOther.mbTiedAtStart
                   && mbTiedAtEnd == rOther.mbTiedAtEnd
                   && mnTiedToAtStart == rOther.mnTiedToAtStart
                   && mnTiedToAtEnd == rOther.mnTiedToAtEnd && moRouting == rOther.moRouting
                   && maOwnGluePoints == rOther.maOwnGluePoints
                   && maSideGluePoints == rOther.maSideGluePoints;
        }
    };

    /// Names the version space the part versions count in. Two versions stand for the content
    /// of the same model only when they carry the same epoch.
    SD_DLLPUBLIC sal_Int32 getVectorEpoch() const;

    /// Content version of the part that stands for the page, counted up on each object
    /// change. 0 when nothing changed since the document was opened.
    sal_uInt64 getVectorPartVersion(const SdrPage& rPage) const;

    /// True when the object with the given unique id last changed on the
    /// part at a version later than nSince.
    bool isVectorObjectChangedSince(const SdrPage& rPage, sal_uInt64 nObjectId,
                                    sal_uInt64 nSince) const;

    /// True when the set of objects on the part, or the order they paint in, changed after the
    /// given version.
    bool isVectorOrderChangedSince(const SdrPage& rPage, sal_uInt64 nSince) const;

    /// The objects a change asked for a fresh look at, taken out of the part's state.
    std::unordered_set<sal_uInt64> takeVectorDirtyObjects(const SdrPage& rPage);

    /// Records what is being written for one object. Counts the part's version up and returns
    /// true when the content differs from what was recorded before, false when the object looks
    /// the same and sits where it did.
    bool recordVectorObjectContent(const SdrPage& rPage, sal_uInt64 nObjectId,
                                   const VectorObjectContent& rContent);

    /// What was last written for the object on the part, or nullptr when nothing was.
    const VectorObjectContent* findVectorObjectContent(const SdrPage& rPage,
                                                       sal_uInt64 nObjectId) const;

    /// Records what is being written for one object without touching any version. Writing an
    /// object is what makes it the content the client holds, whether the write was a full
    /// response or a delta.
    void noteVectorObjectWritten(const SdrPage& rPage, sal_uInt64 nObjectId,
                                 const VectorObjectContent& rContent);

    /// Drops what was recorded for an object that is no longer on the part.
    void forgetVectorObject(const SdrPage& rPage, sal_uInt64 nObjectId);

    /// The ids everything the part has recorded is keyed by, in no particular order.
    std::vector<sal_uInt64> getVectorRecordedIds(const SdrPage& rPage) const;

    /// Records the order the objects of the part paint in. When it differs from the order
    /// recorded before, counts the part's version up, remembers that version as the one the
    /// order last moved at, and returns true. The first order recorded moves nothing.
    bool recordVectorPaintOrder(const SdrPage& rPage, const std::vector<sal_uInt64>& rOrder);

    /// The text edit running on rEdited shows something other than it did. Tells the views of
    /// the part it sits on, and the write that follows counts the version up if the text really
    /// moved. Only the entry carrying the edit is affected, so no object is recorded against.
    void notifyTextEditChanged(const SdrObject& rEdited);

    /// True when the part's master page last changed at a version later
    /// than nSince.
    bool isVectorMasterChangedSince(const SdrPage& rPage, sal_uInt64 nSince) const;

    /// True when a change marked the page entry, or nothing was recorded for it yet. Clears
    /// the mark.
    bool takeVectorPageDirty(const SdrPage& rPage);

    /// Records what is written for the page entry: the content behind the objects and the id
    /// of the master part the page names. When that differs from the last record, counts the
    /// part's version up, remembers it as the version the entry last changed at, and returns
    /// true. The first record for a part moves nothing.
    bool recordVectorPageContent(const SdrPage& rPage, const VectorObjectContent& rContent,
                                 const OString& rMasterPartId,
                                 const std::vector<sal_Int32>& rMasterHiddenLayers);

    /// Content state of one vector-rendering part: its current version, the version the last
    /// delta written for it was computed up to, the version at which its master page last
    /// changed, and, per object unique id, the version at which that object last changed.
    ///
    /// maObjectContent holds what was last written per object, and maDirtyObjects the objects a
    /// change asked for a fresh look at. An object stays in maDirtyObjects until a write
    /// compares it against maObjectContent.
    struct VectorPartState
    {
        sal_uInt64 mnVersion = 0;
        /// Where the readers of the part stand: a delta written for the part steps from here
        /// and ends at the version it carries. The first response to serve the part sets it,
        /// since nothing holds the part before that. One delta is written per part rather than
        /// one per reader, so the mark counts for the part.
        sal_uInt64 mnLastSentVersion = 0;
        /// True once a response has served the part, so a reader may hold it. The first full
        /// response leaves the version where it found it, which can be zero, so the version
        /// alone does not say whether anyone holds the part.
        bool mbServed = false;
        /// The version at which the page entry last changed: the page's own properties, its
        /// background, the master it names or the master content it carries inline.
        sal_uInt64 mnMasterChangeVersion = 0;
        /// The version at which the set of objects on the part, or the order they paint in,
        /// last changed. A client whose content is newer than this already holds the order.
        sal_uInt64 mnOrderChangeVersion = 0;
        /// True while a change marked the page entry for comparison.
        bool mbPageDirty = false;
        /// What was last written for the page entry, and the id of the master part it named.
        /// Nothing before the part was first written.
        std::optional<VectorObjectContent> moPageContent;
        OString maPageMasterPartId;
        /// The layers of the master the page leaves out, as last written.
        std::vector<sal_Int32> maPageMasterHiddenLayers;
        std::unordered_map<sal_uInt64, sal_uInt64> maObjectChangeVersions;
        std::unordered_map<sal_uInt64, VectorObjectContent> maObjectContent;
        std::unordered_set<sal_uInt64> maDirtyObjects;
        /// The ids of the painted objects in the order they were last written, without the page
        /// entry. Nothing before the part was first written.
        std::optional<std::vector<sal_uInt64>> maPaintOrder;
    };

private:
    ::sd::DrawDocShell* mpDocShell;
    SdDrawDocument* mpDoc;
    bool mbDisposed;

    std::unique_ptr<sd::SlideshowLayerRenderer> mpSlideshowLayerRenderer;
    std::unordered_map<sal_Int64, Graphic> maBitmapCache;
    std::unordered_map<sal_uInt64, BinaryDataContainer> maVectorFontCache;
    std::unordered_map<OUString, drawinglayer::Primitive2dJsonProcessor::ResolvedFace>
        maVectorFontFaceByKey;

    struct AnimatedGifTempFile
    {
        sal_uInt64 mnChecksum = 0;
        OUString maUrl;
        std::shared_ptr<avmedia::MediaTempFile> mpTempFile;
    };

    /// Extracted animated GIF files, keyed by the graphic object's unique id.
    mutable std::unordered_map<sal_uInt64, AnimatedGifTempFile> maAnimatedGifCache;

    /// Vector content state, keyed by the GUID of the page it belongs to, which is also the
    /// page's part id. The state stays with the page when pages are inserted, removed or moved
    /// and its index in the page list changes.
    std::unordered_map<tools::Guid, VectorPartState> maVectorParts;

    /// The version space the part versions count in, drawn once for this model. 0 until it is
    /// first asked for.
    mutable sal_Int32 mnVectorEpoch = 0;

    /// A number that names one model's version space, different from the number every other
    /// model draws.
    static sal_Int32 newVectorEpoch();

    /// Writes the primitives of the page rPartId names in the page list nMode. A non-negative
    /// nSinceVersion asks for the delta against that version instead of the whole page. A push
    /// steps from the version the part was last pushed at and moves that mark afterwards. A
    /// pull leaves the mark alone unless nothing has served the part yet.
    void writeVectorPrimitives(tools::JsonWriter& rJsonWriter, const OString& rPartId,
                               sal_Int32 nMode, sal_Int64 nSinceVersion, bool bPush);

    cpo::uno::Reference<cpo::uno::XInterface> create(
        OUString const & aServiceSpecifier, OUString const & referer);

    /// @throws cpo::uno::RuntimeException
    SdPage* InsertSdPage( sal_uInt16 nPage, bool bDuplicate );

    const bool mbImpressDoc;
    bool mbClipBoard;

    unotools::WeakReference< SdDrawPagesAccess > mxDrawPagesAccess;
    unotools::WeakReference< SdMasterPagesAccess > mxMasterPagesAccess;
    unotools::WeakReference< SdLayerManager > mxLayerManager;
    unotools::WeakReference< SdXCustomPresentationAccess > mxCustomPresentationAccess;
    unotools::WeakReference< SdUnoForbiddenCharsTable > mxForbiddenCharacters;
    unotools::WeakReference< SdDocLinkTargets > mxLinks;

    cpo::uno::Reference< cpo::uno::XInterface > mxDashTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxGradientTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxHatchTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxBitmapTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxTransGradientTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxMarkerTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxDrawingPool;

    const SvxItemPropertySet*   mpPropSet;

    cpo::uno::Sequence< cpo::uno::Type > maTypeSequence;

    OUString   maBuildId;

    bool mbPaintTextEdit;

    void initializeDocument();

    /// Creates file URL of the animated GIF extracted from rGraphicObject
    OUString getOrCreateAnimatedGifUrl(const SdrGrafObj& rGraphicObject) const;

    SAL_RET_MAYBENULL sd::DrawViewShell* GetViewShell();

    /** abstract SdrModel provider */
    virtual SdrModel& getSdrModelFromUnoModel() const override;

public:
    SdXImpressDocument(::sd::DrawDocShell* pShell, bool bClipBoard);
    SdXImpressDocument(SdDrawDocument* pDoc, bool bClipBoard);
    virtual ~SdXImpressDocument() noexcept override;

    static rtl::Reference< SdXImpressDocument > GetModel( SdDrawDocument const & rDoc );

    // internal
    bool operator==( const SdXImpressDocument& rModel ) const { return mpDoc == rModel.mpDoc; }
    bool operator!=( const SdXImpressDocument& rModel ) const { return mpDoc != rModel.mpDoc; }

    ::sd::DrawDocShell* GetDocShell() const { return mpDocShell; }
    SdDrawDocument* GetDoc() const { return mpDoc; }
    bool IsImpressDocument() const { return mbImpressDoc; }

    void SetModified() noexcept;

    cpo::uno::Reference< css::i18n::XForbiddenCharacters > getForbiddenCharsTable();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    UNO3_GETIMPLEMENTATION_DECL(SdXImpressDocument)

    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    SD_DLLPUBLIC virtual void acquire() noexcept override;
    SD_DLLPUBLIC virtual void release() noexcept override;

    // XModel
    virtual void lockControllers(  ) override;
    virtual void unlockControllers(  ) override;
    virtual bool hasControllersLocked(  ) override;
    virtual cpo::uno::Reference < css::container::XIndexAccess > getViewData() override;
    virtual void setViewData( const cpo::uno::Reference < css::container::XIndexAccess >& aData ) override;

    // XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId(  ) override;

    // XDrawPageDuplicator
    virtual cpo::uno::Reference< css::drawing::XDrawPage > duplicate( const cpo::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XDrawPagesSupplier
    SD_DLLPUBLIC virtual cpo::uno::Reference< css::drawing::XDrawPages > getDrawPages(  ) override;

    // XMasterPagesSupplier
    virtual cpo::uno::Reference< css::drawing::XDrawPages > getMasterPages(  ) override;

    // XLayerManagerSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess > getLayerManager(  ) override;

    // XCustomPresentationSupplier
    virtual cpo::uno::Reference< css::container::XNameContainer > getCustomPresentations(  ) override;

    // XHandoutMasterSupplier
    virtual cpo::uno::Reference< css::drawing::XDrawPage > getHandoutMasterPage(  ) override;

    // XPresentationSupplier
    virtual cpo::uno::Reference< css::presentation::XPresentation > getPresentation(  ) override;

    // XMultiServiceFactory ( SvxFmMSFactory )
    SD_DLLPUBLIC virtual cpo::uno::Reference< cpo::uno::XInterface > createInstance( const OUString& aServiceSpecifier ) override;
    virtual cpo::uno::Reference<cpo::uno::XInterface>
    createInstanceWithArguments(
        OUString const & ServiceSpecifier,
        cpo::uno::Sequence<cpo::uno::Any> const & Arguments) override;
    virtual cpo::uno::Sequence< OUString > getAvailableServiceNames(  ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void addVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XLinkTargetSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess > getLinks(  ) override;

    // XStyleFamiliesSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess > getStyleFamilies(  ) override;

    // XAnyCompareFactory
    virtual cpo::uno::Reference< css::ucb::XAnyCompare > createAnyCompareByName( const OUString& PropertyName ) override;

    // XRenderable
    virtual sal_Int32 getRendererCount( const cpo::uno::Any& aSelection, const cpo::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getRenderer( sal_Int32 nRenderer, const cpo::uno::Any& aSelection, const cpo::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual void render( sal_Int32 nRenderer, const cpo::uno::Any& aSelection, const cpo::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;

    rtl::Reference< sd::SlideShow > getSlideShow();

    // ITiledRenderable
    SD_DLLPUBLIC virtual void paintTile( VirtualDevice& rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            tools::Long nTileWidth,
                            tools::Long nTileHeight ) override;
    SD_DLLPUBLIC virtual Size getDocumentSize() override;
    virtual Size getPartSize(int part) override;
    virtual void getAllPartSize(::tools::JsonWriter& rJsonWriter) override;
    SD_DLLPUBLIC virtual void setPart(   int nPart, bool bAllowChangeFocus = true ) override;
    SD_DLLPUBLIC virtual int  getPart() override;
    SD_DLLPUBLIC virtual int  getParts() override;
    SD_DLLPUBLIC virtual OUString getPartName( int nPart ) override;
    SD_DLLPUBLIC virtual OUString getPartHash( int nPart ) override;
    SD_DLLPUBLIC virtual VclPtr<vcl::Window> getDocWindow() override;
    bool isMasterViewMode();

    /// @see vcl::ITiledRenderable::setPartMode().
    virtual void setPartMode( COKitPartMode ePartMode ) override;
    /// @see vcl::ITiledRenderable::getEditMode().
    SD_DLLPUBLIC virtual int getEditMode() override;
    /// @see vcl::ITiledRenderable::setEditMode().
    SD_DLLPUBLIC virtual void setEditMode(int) override;

    /// @see vcl::ITiledRenderable::setDrawnFromModel().
    virtual void setDrawnFromModel(bool bDrawnFromModel) override;
    /// @see vcl::ITiledRenderable::initializeForTiledRendering().
    SD_DLLPUBLIC virtual void initializeForTiledRendering(const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments) override;
    /// @see vcl::ITiledRenderable::postKeyEvent().
    SD_DLLPUBLIC virtual void postKeyEvent(COKitKeyEventType eType, int nCharCode,
                                           int nKeyCode) override;
    /// @see vcl::ITiledRenderable::postMouseEvent().
    SD_DLLPUBLIC virtual void postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount,
                                             int nButtons, int nModifier) override;
    /// @see vcl::ITiledRenderable::setTextSelection().
    SD_DLLPUBLIC virtual void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY) override;
    /// @see vcl::ITiledRenderable::getSelection().
    SD_DLLPUBLIC virtual cpo::uno::Reference<css::datatransfer::XTransferable> getSelection() override;
    /// @see vcl::ITiledRenderable::setGraphicSelection().
    SD_DLLPUBLIC virtual void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY) override;
    /// @see COKitDocument::resetSelection().
    SD_DLLPUBLIC virtual void resetSelection() override;
    /// @see vcl::ITiledRenderable::setClientVisibleArea().
    SD_DLLPUBLIC virtual void setClientVisibleArea(const tools::Rectangle& rRectangle) override;
    /// @see vcl::ITiledRenderable::setPageZoom().
    virtual void setPageZoom(int nPageZoom) override;
    /// @see vcl::ITiledRenderable::setClipboard().
    virtual void setClipboard(const cpo::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard) override;
    /// @see vcl::ITiledRenderable::isMimeTypeSupported().
    virtual bool isMimeTypeSupported() override;
    /// @see vcl::ITiledRenderable::getPointer().
    virtual PointerStyle getPointer() override;
    /// @see vcl::ITiledRenderable::getPostIts().
    SD_DLLPUBLIC virtual void getPostIts(tools::JsonWriter& /*rJsonWriter*/) override;
    /// @see vcl::ITiledRenderable::selectPart().
    virtual void selectPart(int nPart, int nSelect) override;
    /// @see vcl::ITiledRenderable::moveSelectedParts().
    virtual void moveSelectedParts(int nPosition, bool bDuplicate, int nIntoSection) override;
    /// @see vcl::ITiledRenderable::getPartInfo().
    virtual std::string getPartInfo(int nPart) override;
    /// @see vcl::ITiledRenderable::getPartId().
    virtual OString getPartId(int nPart, int nMode) override;
    /// @see vcl::ITiledRenderable::getPartIndex().
    virtual int getPartIndex(std::string_view rPartId, int nMode) override;
    /// @see vcl::ITiledRenderable::isDisposed().
    virtual bool isDisposed() const override
    {
        return mbDisposed;
    }
    /// @see vcl::ITiledRenderable::setPaintTextEdit().
    virtual void setPaintTextEdit(bool bPaint) override { mbPaintTextEdit = bPaint; }
    /// @see vcl::ITiledRenderable::getViewRenderState().
    SD_DLLPUBLIC OString getViewRenderState(const SfxViewShell* pViewShell = nullptr) override;

    /// @see vcl::ITiledRenderable::supportsCommand().
    SD_DLLPUBLIC virtual bool supportsCommand(std::u16string_view rCommand) override;

    SD_DLLPUBLIC virtual void getCommandValues(tools::JsonWriter& rJsonWriter, std::string_view rCommand) override;

    /// @see vcl::ITiledRenderable::pushVectorPrimitivesDelta().
    SD_DLLPUBLIC void pushVectorPrimitivesDelta(tools::JsonWriter& rJsonWriter,
                                                std::string_view rPartId, int nMode) override;

    /// @see vcl::ITiledRenderable::getPresentationInfo().
    SD_DLLPUBLIC std::string getPresentationInfo(bool bAllyState = false) const override;
    /// @see vcl::ITiledRenderable::createSlideRenderer().
    SD_DLLPUBLIC bool createSlideRenderer(
        const OString& rSlideHash,
        sal_Int32 nSlideNumber, sal_Int32& nViewWidth, sal_Int32& nViewHeight,
        bool bRenderBackground, bool bRenderMasterPage) override;
    /// @see vcl::ITiledRenderable::renderNextSlideLayer().
    SD_DLLPUBLIC bool renderNextSlideLayer(unsigned char* pBuffer, bool& bIsBitmapLayer, double& rScale, std::string& rJsonMsg) override;
    /// @see vcl::ITiledRenderable::insertPagesFromFile().
    SD_DLLPUBLIC bool insertPagesFromFile(const OUString& rFileUrl,
                                          const OString& rJsonOptions) override;
    /// @see vcl::ITiledRenderable::getSlideLinks().
    SD_DLLPUBLIC bool getSlideLinks(tools::JsonWriter& rJsonWriter) override;
    /// @see vcl::ITiledRenderable::refreshSlideLinks().
    SD_DLLPUBLIC sal_Int32 refreshSlideLinks(const OUString& rSourceName,
                                             const OUString& rFileUrl,
                                             const OUString& rLastModifiedTime,
                                             std::vector<OString>* pNotUpdated = nullptr,
                                             sal_Int32 nPageIndex = -1) override;
    /// @see vcl::ITiledRenderable::breakSlideLink().
    SD_DLLPUBLIC bool breakSlideLink(sal_Int32 nIndex) override;

    /// @see vcl::ITiledRenderable::getSlideIndexOfGuid().
    SD_DLLPUBLIC sal_Int32 getSlideIndexOfGuid(const OUString& rGuid) override;
    /// @see vcl::ITiledRenderable::exportPages().
    SD_DLLPUBLIC bool exportPages(const std::vector<sal_Int32>& rPages,
                                  const OUString& rFileUrl) override;

    SD_DLLPUBLIC rtl::Reference< SdDrawPagesAccess > getSdDrawPages();

    // XComponent

    /** This dispose implementation releases the resources held by the
        called object and forwards the call to its base class.
        When close() has not yet been called then this is done first.  As a
        consequence the implementation has to cope with being called twice
        and still has to forward the second call to the base class.
        See also comments of issue 27847.
    */
    virtual void dispose() override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SAL_DLLPUBLIC_RTTI SdDrawPagesAccess final : public ::cppu::WeakImplHelper< css::drawing::XDrawPages, css::container::XNameAccess, css::lang::XServiceInfo, css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdDrawPagesAccess( SdXImpressDocument&  rMyModel ) noexcept;
    virtual ~SdDrawPagesAccess() noexcept override;

    // XDrawPages
    virtual cpo::uno::Reference< css::drawing::XDrawPage > insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void remove( const cpo::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

    // XIndexAccess
    virtual sal_Int32 getCount() override ;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

    SD_DLLPUBLIC SdGenericDrawPage* getDrawPageByIndex( sal_Int32 Index );
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdMasterPagesAccess final : public ::cppu::WeakImplHelper< css::drawing::XDrawPages2, css::lang::XServiceInfo, css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdMasterPagesAccess( SdXImpressDocument& rMyModel ) noexcept;
    virtual ~SdMasterPagesAccess() noexcept override;

    // XDrawPages
    virtual cpo::uno::Reference< css::drawing::XDrawPage > insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void remove( const cpo::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XDrawPages2
    virtual cpo::uno::Reference< ::css::drawing::XDrawPage > insertNamedNewByIndex( sal_Int32 nIndex, const OUString& sName ) override;

    // XIndexAccess
    virtual sal_Int32 getCount() override ;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

private:
    rtl::Reference< SvxDrawPage > insertNewImpl( sal_Int32 nIndex, std::optional<OUString> oName );
};

/***********************************************************************
*                                                                      *
***********************************************************************/

enum SdLinkTargetType
{
    Page = 0,
    Notes,
    Handout,
    MasterPage,
    Count
};

class SdDocLinkTargets final : public ::cppu::WeakImplHelper< css::container::XNameAccess,
                                                         css::lang::XServiceInfo , css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;
    OUString aNames[SdLinkTargetType::Count];

public:
    SdDocLinkTargets(SdXImpressDocument& rMyModel);
    virtual ~SdDocLinkTargets() noexcept override;

    // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;
};

class SdDocLinkTargetType final : public ::cppu::WeakImplHelper< css::document::XLinkTargetSupplier,
                                                             css::beans::XPropertySet,
                                                             css::lang::XServiceInfo >
{
    SdXImpressDocument* mpModel;
    sal_uInt16 mnType;
    OUString maName;

public:
    SdDocLinkTargetType(SdXImpressDocument* pModel, sal_uInt16 nT);

    // css::document::XLinkTargetSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess > getLinks() override;

    // css::lang::XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString> getSupportedServiceNames() override;

    // css::beans::XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;
    virtual void setPropertyValue(const OUString& aPropertyName,
                                           const cpo::uno::Any& aValue) override;
    virtual cpo::uno::Any getPropertyValue(const OUString& PropertyName) override;
    virtual void addPropertyChangeListener(const OUString& aPropertyName,
                          const cpo::uno::Reference< css::beans::XPropertyChangeListener > & xListener) override;
    virtual void removePropertyChangeListener(const OUString& aPropertyName,
                          const cpo::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void addVetoableChangeListener(const OUString& PropertyName,
                          const cpo::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void removeVetoableChangeListener(const OUString& PropertyName,
                          const cpo::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
};

class SdDocLinkTarget final : public ::cppu::WeakImplHelper< css::container::XNameAccess,
                                                             css::lang::XServiceInfo >
{
private:
    SdXImpressDocument* mpModel;
    sal_uInt16 mnType;

public:
    SdDocLinkTarget( SdXImpressDocument* pModel, sal_uInt16 nT );

    // css::container::XNameAccess
    virtual cpo::uno::Any getByName(const OUString& aName) override;
    virtual cpo::uno::Sequence< OUString> getElementNames() override;
    virtual bool hasByName(const OUString& aName) override;

    // css::container::XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // css::lang::XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString> getSupportedServiceNames() override;

    // internal
    /// @throws std::exception
    SdPage* FindPage( std::u16string_view rName ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
