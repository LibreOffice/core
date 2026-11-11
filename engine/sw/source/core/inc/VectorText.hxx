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

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <rtl/ustring.hxx>
#include <swdllapi.h>
#include <swrect.hxx>
#include <swtypes.hxx>
#include <tools/color.hxx>
#include <vcl/kernarray.hxx>

#include <optional>
#include <span>

class GraphicObject;
class OutputDevice;
class SwDrawTextInfo;
class SwViewOption;

namespace sw::vector
{
/** Receives what the text of one text frame draws, as primitives relative to an origin, and
    groups them as one block holding one paragraph holding its lines.

    Each text portion is tagged with the line it sits on: where the line starts in the text area
    before alignment, how wide it wraps, its ascent and height, the size of the text area and the
    alignment of the paragraph.
 */
class SW_DLLPUBLIC TextSink
{
public:
    /// rOrigin is the top left of the entry the text belongs to and rTextArea the area the lines
    /// wrap in, both in document twips. With bForView set, the view being painted decides the
    /// text colors. Otherwise the automatic text color stays automatic.
    TextSink(const Point& rOrigin, const SwRect& rTextArea,
             drawinglayer::primitive2d::TextPortionAlign eAlign, bool bForView);

    /// A sink for text drawn beside the paragraph, like line numbers. Its text is not tagged with
    /// a line, and take() gives the primitives as they were added. rOrigin and bForView are as in
    /// the other constructor.
    TextSink(const Point& rOrigin, bool bForView);

    /// Starts the next line. nLeft is where the line starts before alignment and nWidth how far
    /// it reaches, both in document twips.
    void startLine(SwTwips nLeft, SwTwips nWidth, SwTwips nAscent, SwTwips nHeight);

    /// Adds text that rDevice would draw in its current font, colors and layout mode at
    /// rPosition, a point on the baseline in document twips. That point is the left end of the
    /// run, or the right end of a right-to-left run when the layout mode puts the origin there.
    /// aAdvances holds the distance from the start to the end of each character. Empty advances
    /// are measured on rDevice.
    void addText(const OutputDevice& rDevice, const Point& rPosition, const OUString& rText,
                 sal_Int32 nIndex, sal_Int32 nLength, KernArraySpan aAdvances,
                 std::span<const bool> aKashida);

    /// Fills rArea with rColor, for a highlight or a character background.
    void addFill(const SwRect& rArea, const Color& rColor);

    /// Adds hairlines in rColor along rLines, which are in document twips.
    void addLines(const basegfx::B2DPolyPolygon& rLines, const Color& rColor);

    /// Adds a graphic drawn into the area, for a bullet that is a picture.
    void addGraphic(const SwRect& rArea, const GraphicObject& rGraphic);

    /// Adds primitives that are already relative to origin().
    void add(drawinglayer::primitive2d::Primitive2DContainer&& rPrimitives);

    /// Adds hairlines in rColor along rLines, in document twips, that lie over the objects
    /// anchored as a character in the frame, like the cross over a deleted picture.
    void addLinesOverObjects(const basegfx::B2DPolyPolygon& rLines, const Color& rColor);

    /// Cuts what the line draws from now on to rArea, in document twips, until endClip() or the
    /// end of the line.
    void startClip(const SwRect& rArea);

    /// Stops the cut that startClip() started. What the line drew since then stays inside the area
    /// given there.
    void endClip();

    /// What the primitives of one portion of a line are, apart from text of the paragraph.
    enum class PortionGroup
    {
        /// The bullet or the numbering label in front of the paragraph text, and a tab after it.
        Label,
        /// The result of a field, a content control or a form field.
        Field
    };

    /// Starts the primitives of a portion that endGroup() groups.
    void startGroup();

    /// Groups the primitives added on the line since startGroup() as one label or one field.
    void endGroup(PortionGroup eGroup);

    /// Moves the start of the line nDistance to the right, and its end stays. The text portions
    /// added after this are tagged with the moved start, where the paragraph text starts after
    /// a label.
    void shiftLineStart(SwTwips nDistance);

    const Point& origin() const { return maOrigin; }

    bool isForView() const { return mbForView; }

    /// The block with the paragraph and its lines, empty when nothing was drawn. The sink is
    /// empty afterwards.
    drawinglayer::primitive2d::Primitive2DContainer take();

    /// The primitives added by addLinesOverObjects(), relative to origin(). The sink holds none
    /// of them afterwards.
    drawinglayer::primitive2d::Primitive2DContainer takeOverObjects();

private:
    void finishLine();

    Point maOrigin;
    SwRect maTextArea;
    bool mbForView;
    /// Whether the text is the one of the paragraph, tagged with its lines and grouped in a block.
    bool mbParagraph;

    /// The paragraph, the line being drawn and the text area, which every text portion of the
    /// line is tagged with.
    drawinglayer::primitive2d::TextPortionLayout maLayout;

    /// What the line being drawn holds so far.
    drawinglayer::primitive2d::Primitive2DContainer maLine;
    /// The number of primitives maLine held when the group being drawn started.
    size_t mnGroupStart = 0;
    /// The lines drawn before it, each a line primitive.
    drawinglayer::primitive2d::Primitive2DContainer maLines;

    /// The area, in document twips, that the primitives of maLine from mnClipStart on are cut
    /// to. None when the line is not cut.
    std::optional<SwRect> moClipArea;
    /// The index in maLine of the first primitive drawn since startClip().
    size_t mnClipStart = 0;

    /// What lies over the objects anchored as a character, relative to the origin.
    drawinglayer::primitive2d::Primitive2DContainer maOverObjects;
};

/// rPrimitives cut to rArea, which is in document twips. The primitives and the result are
/// relative to rOrigin.
drawinglayer::primitive2d::Primitive2DReference
createMask(const SwRect& rArea, const Point& rOrigin,
           drawinglayer::primitive2d::Primitive2DContainer&& rPrimitives);

/// The view options of text drawn for any client. They are the same for every document and every
/// view, and no setting of a view changes them.
const SwViewOption& optionsForAnyView();

/// Draws a text with its advances into the primitive sink of rInfo, or onto its device without one.
void drawTextArray(SwDrawTextInfo& rInfo, const Point& rPosition, const OUString& rText,
                   KernArraySpan aAdvances, std::span<const bool> aKashida, sal_Int32 nIndex,
                   sal_Int32 nLength);

/// Draws a text at its natural advances into the primitive sink of rInfo, or onto its device
/// without one.
void drawText(SwDrawTextInfo& rInfo, const Point& rPosition, const OUString& rText,
              sal_Int32 nIndex, sal_Int32 nLength);

/// Draws a text stretched to nWidth into the primitive sink of rInfo, or onto its device without
/// one.
void drawStretchText(SwDrawTextInfo& rInfo, const Point& rPosition, tools::Long nWidth,
                     const OUString& rText, sal_Int32 nIndex, sal_Int32 nLength);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
