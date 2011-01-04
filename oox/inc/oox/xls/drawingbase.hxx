/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_XLS_DRAWINGBASE_HXX
#define OOX_XLS_DRAWINGBASE_HXX

#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

/** Absolute position in a spreadsheet (in EMUs) independent from cells. */
struct AnchorPointModel : public ::oox::drawingml::EmuPoint
{
    inline explicit     AnchorPointModel() : ::oox::drawingml::EmuPoint( -1, -1 ) {}
    inline bool         isValid() const { return (X >= 0) && (Y >= 0); }
};

// ----------------------------------------------------------------------------

/** Absolute size in a spreadsheet (in EMUs). */
struct AnchorSizeModel : public ::oox::drawingml::EmuSize
{
    inline explicit     AnchorSizeModel() : ::oox::drawingml::EmuSize( -1, -1 ) {}
    inline bool         isValid() const { return (Width >= 0) && (Height >= 0); }
};

// ----------------------------------------------------------------------------

/** Position in spreadsheet (cell position and offset inside cell). */
struct CellAnchorModel
{
    sal_Int32           mnCol;              /// Column index.
    sal_Int32           mnRow;              /// Row index.
    sal_Int64           mnColOffset;        /// X offset inside the column.
    sal_Int64           mnRowOffset;        /// Y offset inside the row.

    explicit            CellAnchorModel();
    inline bool         isValid() const { return (mnCol >= 0) && (mnRow >= 0); }
};

// ----------------------------------------------------------------------------

/** Application-specific client data of a shape. */
struct AnchorClientDataModel
{
    bool                mbLocksWithSheet;
    bool                mbPrintsWithSheet;

    explicit            AnchorClientDataModel();
};

// ============================================================================

/** Contains the position of a shape in the spreadsheet. Supports different
    shape anchor modes (absolute, one-cell, two-cell). */
class ShapeAnchor : public WorksheetHelper
{
public:
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
    void                setCellPos( sal_Int32 nElement, sal_Int32 nParentContext, const ::rtl::OUString& rValue );
    /** Imports the client anchor settings from a VML element. */
    void                importVmlAnchor( const ::rtl::OUString& rAnchor );
    /** Imports the client anchor settings from a BIFF or DFF stream. */
    void                importBiffAnchor( BinaryInputStream& rStrm );

    /** Calculates the resulting shape anchor in EMUs. */
    ::oox::drawingml::EmuRectangle calcAnchorRectEmu(
                            const ::com::sun::star::awt::Size& rPageSizeHmm ) const;
    /** Calculates the resulting shape anchor in 1/100 mm. */
    ::com::sun::star::awt::Rectangle calcAnchorRectHmm(
                            const ::com::sun::star::awt::Size& rPageSizeHmm ) const;

private:
    /** Converts the passed anchor to an absolute position in EMUs. */
    ::oox::drawingml::EmuPoint calcCellAnchorEmu( const CellAnchorModel& rModel ) const;

private:
    enum AnchorType
    {
        ANCHOR_INVALID,         /// Anchor type is unknown.
        ANCHOR_ABSOLUTE,        /// Absolute anchor (top-left corner and size in absolute units).
        ANCHOR_ONECELL,         /// One-cell anchor (top-left corner at cell, size in absolute units).
        ANCHOR_TWOCELL          /// Two-cell anchor (top-left and bottom-right corner at cell).
    };

    /** Specifies how cell positions from CellAnchorModel have to be processed. */
    enum CellAnchorType
    {
        CELLANCHOR_EMU,             /// Offsets are given in EMUs.
        CELLANCHOR_PIXEL,           /// Offsets are given in screen pixels.
        CELLANCHOR_COLROW           /// Offsets are given in fractions of column width or row height.
    };

    AnchorType          meAnchorType;       /// Type of this shape anchor.
    CellAnchorType      meCellAnchorType;   /// Type of the cell anchor models.
    AnchorPointModel    maPos;              /// Top-left position, if anchor is of type absolute.
    AnchorSizeModel     maSize;             /// Anchor size, if anchor is not of type two-cell.
    CellAnchorModel     maFrom;             /// Top-left position, if anchor is not of type absolute.
    CellAnchorModel     maTo;               /// Bottom-right position, if anchor is of type two-cell.
    AnchorClientDataModel maClientData;     /// Shape client data.
    sal_Int32           mnEditAs;           /// Anchor mode as shown in the UI.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
