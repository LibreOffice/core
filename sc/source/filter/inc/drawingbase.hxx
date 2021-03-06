/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <oox/drawingml/drawingmltypes.hxx>
#include "worksheethelper.hxx"

namespace oox { class AttributeList; }

namespace oox::xls {

/** Absolute position in a spreadsheet (in EMUs) independent from cells. */
struct AnchorPointModel : public ::oox::drawingml::EmuPoint
{
    explicit     AnchorPointModel() : ::oox::drawingml::EmuPoint( -1, -1 ) {}
    bool         isValid() const { return (X >= 0) && (Y >= 0); }
};

/** Absolute size in a spreadsheet (in EMUs). */
struct AnchorSizeModel : public ::oox::drawingml::EmuSize
{
    explicit     AnchorSizeModel() : ::oox::drawingml::EmuSize( -1, -1 ) {}
    bool         isValid() const { return (Width >= 0) && (Height >= 0); }
};

/** Position in spreadsheet (cell position and offset inside cell). */
struct CellAnchorModel
{
    sal_Int32           mnCol;              /// Column index.
    sal_Int32           mnRow;              /// Row index.
    sal_Int64           mnColOffset;        /// X offset inside the column.
    sal_Int64           mnRowOffset;        /// Y offset inside the row.

    explicit            CellAnchorModel();
    bool         isValid() const { return (mnCol >= 0) && (mnRow >= 0); }
};

/** Application-specific client data of a shape. */
struct AnchorClientDataModel
{
    bool                mbLocksWithSheet;
    bool                mbPrintsWithSheet;

    explicit            AnchorClientDataModel();
};

/** Contains the position of a shape in the spreadsheet. Supports different
    shape anchor modes (absolute, one-cell, two-cell). */
class ShapeAnchor : public WorksheetHelper
{
public:
    enum AnchorType
    {
        ANCHOR_INVALID,         /// Anchor type is unknown.
        ANCHOR_ABSOLUTE,        /// Absolute anchor (top-left corner and size in absolute units).
                                /// Matches our "Page" anchor -> ScAnchorType::SCA_PAGE
        ANCHOR_ONECELL,         /// One-cell anchor (top-left corner at cell, size in absolute units).
                                /// Matches our "Cell" anchor -> ScAnchorType::SCA_CELL
        ANCHOR_TWOCELL,         /// Two-cell anchor (top-left and bottom-right corner at cell).
                                /// Matches our "Cell (resize with cell)" anchor -> ScAnchorType::SCA_CELL_RESIZE
        ANCHOR_VML
    };
    explicit            ShapeAnchor( const WorksheetHelper& rHelper );

    /** Imports the shape anchor (one of the elements xdr:absoluteAnchor, xdr:oneCellAnchor, xdr:twoCellAnchor). */
    void                importAnchor( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Imports the absolute anchor position from the xdr:pos element. */
    void                importPos( const AttributeList& rAttribs );
    /** Imports the absolute anchor size from the xdr:ext element. */
    void                importExt( const AttributeList& rAttribs );
    /** Imports the shape client data from the xdr:clientData element. */
    void                importClientData( const AttributeList& rAttribs );
    /** Sets an attribute of the cell-dependent anchor position from xdr:from and xdr:to elements. */
    void                setCellPos( sal_Int32 nElement, sal_Int32 nParentContext, const OUString& rValue );
    /** Imports the client anchor settings from a VML element. */
    void                importVmlAnchor( const OUString& rAnchor );

    /** Checks whether the shape is visible based on the anchor

        If From and To anchor has the same attribute values, the shape
        will not have width and height and thus we can assume that
        such kind of shape will be not be visible
    */
    bool isAnchorValid() const;

    /** Calculates the resulting shape anchor in EMUs. */
    ::oox::drawingml::EmuRectangle calcAnchorRectEmu( const css::awt::Size& rPageSizeHmm ) const;
    /** Calculates the resulting shape anchor in 1/100 mm. */
    css::awt::Rectangle calcAnchorRectHmm( const css::awt::Size& rPageSizeHmm ) const;
    AnchorType          getEditAs() const { return meEditAs; }
private:
    /** Converts the passed anchor to an absolute position in EMUs. */
    ::oox::drawingml::EmuPoint calcCellAnchorEmu( const CellAnchorModel& rModel ) const;

private:

    /** Specifies how cell positions from CellAnchorModel have to be processed. */
    enum class CellAnchorType
    {
        Emu,             /// Offsets are given in EMUs.
        Pixel,           /// Offsets are given in screen pixels.
    };

    AnchorType          meAnchorType;       /// Type of this shape anchor.
    CellAnchorType      meCellAnchorType;   /// Type of the cell anchor models.
    AnchorPointModel    maPos;              /// Top-left position, if anchor is of type absolute.
    AnchorSizeModel     maSize;             /// Anchor size, if anchor is not of type two-cell.
    CellAnchorModel     maFrom;             /// Top-left position, if anchor is not of type absolute.
    CellAnchorModel     maTo;               /// Bottom-right position, if anchor is of type two-cell.
    AnchorClientDataModel maClientData;     /// Shape client data.
    AnchorType          meEditAs;           /// Anchor mode as shown in the UI.
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
