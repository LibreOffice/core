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

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <swdllapi.h>
#include <swrect.hxx>

#include <optional>
#include <string_view>
#include <vector>

class SdrObject;
class SwAnchoredObject;
class SwFrame;
class SwPageFrame;
class SwTextFrame;

/// The content of a Writer page as a client that draws vector primitives receives it.
namespace sw::vector
{
/// What an entry of a page is: a layout frame, the border lines of a table or of a frame, or a
/// drawing object.
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
    Row,
    Cell,
    TextFrame,
    NoTextFrame,
    Fly,
    Shape
};

/// The kind of entry a frame makes.
SW_DLLPUBLIC EntryKind kindOf(const SwFrame& rFrame);

/// The id of the entry of the page itself. The frame counter gives 0 to the root frame of the
/// first layout, which never becomes an entry, so no entry of a frame takes this id.
constexpr sal_uInt64 constPageEntryId = 0;

/// The id of the entry of a drawing object. Frame ids are 32 bits, so the ids of drawing objects
/// start above them.
SW_DLLPUBLIC sal_uInt64 entryIdOf(const SdrObject& rObject);

/// The id of the entry holding the border lines a table draws over its cells.
SW_DLLPUBLIC sal_uInt64 tableBordersEntryIdOf(const SwFrame& rTable);

/// The id of the entry holding the border lines of rFrame.
SW_DLLPUBLIC sal_uInt64 bordersEntryIdOf(const SwFrame& rFrame);

/** One entry of a page: a frame or a drawing object, with what it draws itself.

    The primitives are in twips relative to the top left of the box, so an entry that only moves
    keeps them. The lowers of a frame are entries of their own and come after it.
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

    /// For a text frame: the paragraph started in an earlier frame and goes on here.
    bool mbIsFollow = false;

    /// For a text frame: the paragraph goes on in a later frame.
    bool mbHasFollow = false;

    /// For a fly or a drawing object: the id of the frame it is anchored in. None for an object
    /// anchored to the page.
    std::optional<sal_uInt64> moAnchor;
};

/** Gathers the entries of one page in paint order.

    A frame opens its entry, adds what it draws itself, lets its lowers open theirs, and closes
    its entry. Each entry comes after the entry it sits in.
 */
class SW_DLLPUBLIC Collector
{
public:
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

    /// Starts a layer of border lines. The border lines of the frames in a page or in a fly lie
    /// over all its content.
    void openBorderLayer();

    /// Keeps rBorders, the border lines of rFrame relative to origin(), as an entry of their own
    /// in the entry of rFrame. It is added when the innermost border layer closes.
    void deferBorders(const SwFrame& rFrame,
                      drawinglayer::primitive2d::Primitive2DContainer&& rBorders);

    /// Adds the entries of the border lines kept since the layer opened last, and closes it.
    void closeBorderLayer();

private:
    std::vector<Entry> maEntries;
    /// Indexes into maEntries of the entries open now, the innermost last.
    std::vector<size_t> maOpen;
    /// The entries of border lines whose layer is still open, in paint order.
    std::vector<Entry> maBorders;
    /// Indexes into maBorders where the layers open now start, the innermost last.
    std::vector<size_t> maBorderLayers;
};

/// The frame the object is anchored in and moves with, or none when that frame is the page.
std::optional<sal_uInt64> anchorOf(const SwAnchoredObject& rObject);

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
/// is visible, on a visible layer and placed where the paint draws it. When the collector is not
/// for a view, it also has to be set to print.
void addPageObjects(Collector& rCollector, const SwPageFrame& rPage, ObjectPlace ePlace);

/// Adds the entries of the objects anchored as a character in rFrame, in the order of their
/// places: behind the text, behind the body text, then in front of the text. The page holds
/// every other object.
void addCharacterObjects(Collector& rCollector, const SwTextFrame& rFrame);

/// The entries of rPage in paint order, the page first.
SW_DLLPUBLIC std::vector<Entry> decomposePage(const SwPageFrame& rPage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
