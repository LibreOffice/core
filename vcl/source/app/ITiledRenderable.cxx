/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#pragma warning( disable : 4592)
#endif
    const std::map <PointerStyle, OString> gaLOKPointerMap {
    { PointerStyle::Arrow, "default" },
    // PointerStyle::Null ?
    { PointerStyle::Wait, "wait" },
    { PointerStyle::Text, "text" },
    { PointerStyle::Help, "help" },
    { PointerStyle::Cross, "crosshair" },
    { PointerStyle::Fill, "fill" },
    { PointerStyle::Move, "move" },
    { PointerStyle::NSize, "n-resize" },
    { PointerStyle::SSize, "s-resize" },
    { PointerStyle::WSize, "w-resize" },
    { PointerStyle::ESize, "e-resize" },
    { PointerStyle::NWSize, "ne-resize" },
    { PointerStyle::NESize, "ne-resize" },
    { PointerStyle::SWSize, "sw-resize" },
    { PointerStyle::SESize, "se-resize" },
    // WindowNSize through WindowSESize
    { PointerStyle::HSplit, "col-resize" },
    { PointerStyle::VSplit, "row-resize" },
    { PointerStyle::HSizeBar, "col-resize" },
    { PointerStyle::VSizeBar, "row-resize" },
    { PointerStyle::Hand, "grab" },
    { PointerStyle::RefHand, "pointer" },
    // Pen, Magnify, Fill, Rotate
    // HShear, VShear
    // Mirror, Crook, Crop, MovePoint, MoveBezierWeight
    // MoveData
    { PointerStyle::CopyData, "copy" },
    { PointerStyle::LinkData, "alias" },
    // MoveDataLink, CopyDataLink
    //MoveFile, CopyFile, LinkFile
    // MoveFileLink, CopyFileLink, MoveFiless, CopyFiles
    { PointerStyle::NotAllowed, "not-allowed" },
    // DrawLine through DrawCaption
    // Chart, Detective, PivotCol, PivotRow, PivotField, Chain, ChainNotAllowed
    // TimeEventMove, TimeEventSize
    // AutoScrollN through AutoScrollNSWE
    // Airbrush
    { PointerStyle::TextVertical, "vertical-text" }
    // Pivot Delete, TabSelectS through TabSelectSW
    // PaintBrush, HideWhiteSpace, ShowWhiteSpace
    };
#ifdef _MSC_VER
#pragma warning(pop)
#endif

ITiledRenderable::~ITiledRenderable()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
