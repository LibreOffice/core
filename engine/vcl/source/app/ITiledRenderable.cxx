/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/ITiledRenderable.hxx>

namespace vcl
{
/*
 * Map directly to css cursor styles to avoid further mapping in the client.
 * Gtk (via gdk_cursor_new_from_name) also supports the same css cursor styles.
 *
 * This was created partially with help of the mappings in gtkdata.cxx.
 * The list is incomplete as some cursor style simply aren't supported
 * by css, it might turn out to be worth mapping some of these missing cursors
 * to available cursors?
 */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4592)
#endif
const std::map<PointerStyle, OString> gaKitPointerMap{
    { PointerStyle::Arrow, "default"_ostr },
    // PointerStyle::Null ?
    { PointerStyle::Wait, "wait"_ostr },
    { PointerStyle::Text, "text"_ostr },
    { PointerStyle::Help, "help"_ostr },
    { PointerStyle::Cross, "crosshair"_ostr },
    { PointerStyle::Fill, "fill"_ostr },
    { PointerStyle::Move, "move"_ostr },
    { PointerStyle::NSize, "n-resize"_ostr },
    { PointerStyle::SSize, "s-resize"_ostr },
    { PointerStyle::WSize, "w-resize"_ostr },
    { PointerStyle::ESize, "e-resize"_ostr },
    { PointerStyle::NWSize, "ne-resize"_ostr },
    { PointerStyle::NESize, "ne-resize"_ostr },
    { PointerStyle::SWSize, "sw-resize"_ostr },
    { PointerStyle::SESize, "se-resize"_ostr },
    // WindowNSize through WindowSESize
    { PointerStyle::HSplit, "col-resize"_ostr },
    { PointerStyle::VSplit, "row-resize"_ostr },
    { PointerStyle::HSizeBar, "col-resize"_ostr },
    { PointerStyle::VSizeBar, "row-resize"_ostr },
    { PointerStyle::Hand, "grab"_ostr },
    { PointerStyle::RefHand, "pointer"_ostr },
    // Pen, Magnify, Fill, Rotate
    // HShear, VShear
    // Mirror, Crook, Crop, MovePoint, MoveBezierWeight
    // MoveData
    { PointerStyle::CopyData, "copy"_ostr },
    { PointerStyle::LinkData, "alias"_ostr },
    // MoveDataLink, CopyDataLink
    //MoveFile, CopyFile, LinkFile
    // MoveFileLink, CopyFileLink, MoveFiless, CopyFiles
    { PointerStyle::NotAllowed, "not-allowed"_ostr },
    // DrawLine through DrawCaption
    // Chart, Detective, PivotCol, PivotRow, PivotField, Chain, ChainNotAllowed
    // TimeEventMove, TimeEventSize
    // AutoScrollN through AutoScrollNSWE
    // Airbrush
    { PointerStyle::TextVertical, "vertical-text"_ostr }
    // Pivot Delete, TabSelectS through TabSelectSW
    // PaintBrush, HideWhiteSpace, ShowWhiteSpace
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

ITiledRenderable::~ITiledRenderable() {}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
