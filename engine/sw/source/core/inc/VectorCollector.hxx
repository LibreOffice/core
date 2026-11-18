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
#include <basegfx/range/b2drange.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <swdllapi.h>
#include <swrect.hxx>

#include "VectorContext.hxx"

#include <optional>
#include <string_view>
#include <vector>

class Color;
class SdrObject;
class SwAnchoredObject;
class SwFrame;
class SwPageFrame;
class SwRootFrame;
class SwTextFrame;

/// The content of a Writer page as a client that draws vector primitives receives it.
namespace sw::vector
{
/// What an entry of a page is: a layout frame, the background of a frame, the border lines of a
/// table or of a frame, or a drawing object. What a frame draws over its content, such as the
/// cross over a deleted fly, is an entry of the kind of border lines.
enum class EntryKind
{
    Page,
    Header,
    Footer,
    Body,
    Column,
    Section,
    FootnoteContainer,
    Footnote,
    Table,
    TableBorders,
    Borders,
    Background,
    Row,
    Cell,
    TextFrame,
    NoTextFrame,
    Fly,
    Shape
};

/// The name of the kind in lower case, such as "textframe" or "tableborders".
SW_DLLPUBLIC std::string_view nameOf(EntryKind eKind);

/// The kind of entry a frame makes.
SW_DLLPUBLIC EntryKind kindOf(const SwFrame& rFrame);

/// The id of the entry of the page itself. The frame counter gives 0 to the root frame of the
/// first layout, which never becomes an entry, so no entry of a frame takes this id.
constexpr sal_uInt64 constPageEntryId = 0;

/// The id of the entry of a drawing object. Frame ids are 32 bits, so the ids of drawing objects
/// start above them.
SW_DLLPUBLIC sal_uInt64 entryIdOf(const SdrObject& rObject);

/// The step between the id ranges of the entries a frame makes besides its own. Such an id is the
/// frame id plus a multiple of the step, so in decimal the frame id reads in its last digits. The
/// step is above every frame id and every id of a drawing object.
constexpr sal_uInt64 constEntryIdStep = 1'000'000'000'000;

/// The id of the entry holding the border lines a table draws over its cells.
SW_DLLPUBLIC sal_uInt64 tableBordersEntryIdOf(const SwFrame& rTable);

/// The id of the entry holding the border lines of rFrame.
SW_DLLPUBLIC sal_uInt64 bordersEntryIdOf(const SwFrame& rFrame);

/// The id of the entry holding the background of rFrame.
SW_DLLPUBLIC sal_uInt64 backgroundEntryIdOf(const SwFrame& rFrame);

/// The id of the entry holding what rFrame draws over its content and over the border lines of
/// that content.
SW_DLLPUBLIC sal_uInt64 overContentEntryIdOf(const SwFrame& rFrame);

/** One entry of a page: a frame or a drawing object, with what it draws itself.

    The primitives are in twips relative to the top left of the box, so an entry that only moves
    keeps them. The lowers of a frame are entries of their own and come after it. The background
    of a frame inside a page or a fly is an entry of its own too, and comes before the frame.
 */
struct SW_DLLPUBLIC Entry
{
    /// The frame id, or entryIdOf() for a drawing object, or constPageEntryId for the page itself.
    /// Unique within a page.
    sal_uInt64 mnId = constPageEntryId;

    /// The id of the entry this one sits in. None for the page entry.
    std::optional<sal_uInt64> moParent;

    EntryKind meKind = EntryKind::Page;

    /// The frame area, or the bounds of a drawing object, in document twips.
    SwRect maBox;

    drawinglayer::primitive2d::Primitive2DContainer maPrimitives;

    /// For a drawing object: what its primitives decompose to, in document twips, with the
    /// coordinates and the content of its model at the time of the decomposition. Empty for a
    /// frame.
    drawinglayer::primitive2d::Primitive2DContainer maDrawn;

    /// For a text frame: the paragraph started in an earlier frame and goes on here.
    bool mbIsFollow = false;

    /// For a text frame: the paragraph goes on in a later frame.
    bool mbHasFollow = false;

    /// For a fly or a drawing object: the id of the frame it is anchored in. None for an object
    /// anchored to the page.
    std::optional<sal_uInt64> moAnchor;

    /// False for what printing leaves out: a fly or a drawing object that is not set to print, an
    /// object anchored in such a fly, and every entry inside one of them.
    bool mbPrinted = true;
};

/** Gathers the entries of one page in paint order.

    A frame opens its entry, adds what it draws itself, lets its lowers open theirs, and closes
    its entry. Each entry comes after the entry it sits in, except the entry of a background.

    A page is drawn in layers, and so is a fly: the backgrounds of all its frames, the objects
    behind its text, the content of the frames, the border lines, the objects in front.
 */
class SW_DLLPUBLIC Collector
{
public:
    /// rContext says what the entries are for: any client, or one view. An area set on it limits
    /// the entries to what reaches into that area. A frame whose paint area misses it is left out
    /// with its lowers, as in the frame paint, and so is a drawing object whose bounds miss it.
    explicit Collector(const Context& rContext = Context())
        : maContext(rContext)
    {
    }

    /// What the entries are for.
    const Context& context() const { return maContext; }

    /// Whether what is drawn within rBounds, in document twips, belongs to the entries.
    bool wants(const SwRect& rBounds) const { return maContext.wants(rBounds); }

    /// Opens the entry of rFrame inside the entry that is open now.
    void open(const SwFrame& rFrame);

    /// Opens an entry that is not a frame of its own, inside the entry that is open now.
    void open(sal_uInt64 nId, EntryKind eKind, const SwRect& rBox);

    /// Closes the entry opened last.
    void close();

    /// Adds primitives to the open entry. They are relative to origin().
    void add(const drawinglayer::primitive2d::Primitive2DReference& rPrimitive);
    void add(drawinglayer::primitive2d::Primitive2DContainer&& rPrimitives);

    /// The top left of the open entry's box, in document twips.
    Point origin() const;

    /// The entry opened last and not closed yet.
    Entry& current();

    /// The entries gathered so far, in paint order. The collector is empty afterwards.
    std::vector<Entry> take();

    /// Starts a layer of backgrounds right after the entries gathered so far. The backgrounds of
    /// the frames in a page or in a fly lie under all its content and under the objects behind
    /// its text.
    void openBackgroundLayer();

    /// Adds rBackground, the background of rFrame relative to origin(), with rFrame's entry open.
    /// It stays in that entry for a page, a fly, no open layer, and a paragraph or table cell
    /// whose background the document draws over the objects behind the text. Otherwise it
    /// becomes an entry of its own at the start of the innermost background layer.
    void addBackground(const SwFrame& rFrame,
                       drawinglayer::primitive2d::Primitive2DContainer&& rBackground);

    /// Moves the background entries kept since the layer opened last to where the layer started,
    /// and closes it.
    void closeBackgroundLayer();

    /// Starts a layer of border lines. The border lines of the frames in a page or in a fly lie
    /// over all its content.
    void openBorderLayer();

    /// Keeps rBorders, the border lines of rFrame relative to origin(), as an entry of their own
    /// in the entry of rFrame. It is added when the innermost border layer closes.
    void deferBorders(const SwFrame& rFrame,
                      drawinglayer::primitive2d::Primitive2DContainer&& rBorders);

    /// Keeps rLines, what rFrame draws over its content and over the border lines of that content,
    /// relative to origin(), as an entry of the kind of border lines in the entry of rFrame. It is
    /// added when the innermost border layer closes, after the entries kept before it.
    void deferOverContent(const SwFrame& rFrame,
                          drawinglayer::primitive2d::Primitive2DContainer&& rLines);

    /// Adds the entries of the border lines kept since the layer opened last, and closes it.
    void closeBorderLayer();

private:
    /// Keeps rLines, relative to origin(), as an entry of the kind of border lines with the id nId
    /// in the open entry. It is added when the innermost border layer closes.
    void deferLines(sal_uInt64 nId, drawinglayer::primitive2d::Primitive2DContainer&& rLines);

    /// A layer of backgrounds that is open.
    struct BackgroundLayer
    {
        /// The index into maEntries where the backgrounds of the layer go.
        size_t mnPosition = 0;
        /// The index into maBackgrounds of the first background of the layer.
        size_t mnFirst = 0;
    };

    Context maContext;
    std::vector<Entry> maEntries;
    /// Indexes into maEntries of the entries open now, the innermost last.
    std::vector<size_t> maOpen;
    /// The entries of border lines whose layer is still open, in paint order.
    std::vector<Entry> maBorders;
    /// Indexes into maBorders where the layers open now start, the innermost last.
    std::vector<size_t> maBorderLayers;
    /// The entries of backgrounds whose layer is still open, in paint order.
    std::vector<Entry> maBackgrounds;
    /// The layers of backgrounds open now, the innermost last.
    std::vector<BackgroundLayer> maBackgroundLayers;
};

/// The range of rArea, which is in document twips, relative to rOrigin.
basegfx::B2DRange rangeRelativeTo(const SwRect& rArea, const Point& rOrigin);

/// Adds a fill of rArea in rColor to rTarget, with the transparency of the color. An empty area
/// and a fully transparent color add nothing.
void addColorFill(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                  basegfx::B2DPolyPolygon&& rArea, const Color& rColor);

/// The frame the object is anchored in and moves with, or none when that frame is the page.
std::optional<sal_uInt64> anchorOf(const SwAnchoredObject& rObject);

/// Whether printing shows rObject: it is set to print, and so is every fly it is anchored in.
bool isPrinted(const SwAnchoredObject& rObject);

/// Where the objects on a layer of the drawing are drawn, against the text of the page.
enum class ObjectPlace
{
    /// Behind all the text: the hell layer.
    BehindText,
    /// Over the header and the footer and behind the body text: the layer of the objects behind
    /// the text when the document paints them over the header and the footer.
    BehindBodyText,
    /// In front of all the text: the heaven layer and the controls.
    InFrontOfText
};

/// Adds the entries of the objects anchored on rPage that are drawn at ePlace, in paint order.
/// The page holds every object but the ones anchored as a character. An object is added when it
/// is visible, on a visible layer and placed where the paint draws it.
void addPageObjects(Collector& rCollector, const SwPageFrame& rPage, ObjectPlace ePlace);

/// Adds the entries of the objects anchored as a character in rFrame, in the order of their
/// places: behind the text, behind the body text, then in front of the text. The page holds
/// every other object.
void addCharacterObjects(Collector& rCollector, const SwTextFrame& rFrame);

/// The frame of rLayout with the frame id nId, in the flow of the pages or in a fly, or nullptr
/// when there is none. With pNear, a page of rLayout, the pages are searched outward from pNear,
/// the nearest first.
SW_DLLPUBLIC const SwFrame* frameOf(const SwRootFrame& rLayout, sal_uInt64 nId,
                                    const SwPageFrame* pNear = nullptr);

/// The entries of rPage in paint order, the page first.
SW_DLLPUBLIC std::vector<Entry> decomposePage(const SwPageFrame& rPage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
