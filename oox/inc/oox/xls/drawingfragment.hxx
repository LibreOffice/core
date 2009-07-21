/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: drawingfragment.hxx,v $
 * $Revision: 1.3 $
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

#ifndef OOX_XLS_DRAWINGFRAGMENT_HXX
#define OOX_XLS_DRAWINGFRAGMENT_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include "oox/drawingml/shape.hxx"
#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

// ============================================================================

/** Absolute position in spreadsheet (in EMUs) independent from cells. */
struct AnchorPosModel : public ::oox::drawingml::EmuPoint
{
    inline explicit     AnchorPosModel() : ::oox::drawingml::EmuPoint( -1, -1 ) {}
    inline bool         isValid() const { return (X >= 0) && (Y >= 0); }
};

// ----------------------------------------------------------------------------

/** Absolute size in spreadsheet (in EMUs). */
struct AnchorSizeModel : public ::oox::drawingml::EmuSize
{
    inline explicit     AnchorSizeModel() : ::oox::drawingml::EmuSize( -1, -1 ) {}
    inline bool         isValid() const { return (Width >= 0) && (Height >= 0); }
};

// ----------------------------------------------------------------------------

/** Position in spreadsheet (cell position and offset inside cell in EMUs). */
struct AnchorCellModel
{
    sal_Int32           mnCol;              /// Column index.
    sal_Int32           mnRow;              /// Row index.
    sal_Int64           mnColOffset;        /// X offset in column mnCol (EMUs).
    sal_Int64           mnRowOffset;        /// Y offset in row mnRow (EMUs).

    explicit            AnchorCellModel();
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

    /** Returns true, if the anchor contains valid position and size settings. */
    bool                isValidAnchor() const;

#if 0 // unused code
    /** Calculates the resulting shape anchor in 1/100 mm. */
    ::com::sun::star::awt::Rectangle
                        calcApiLocation(
                            const ::com::sun::star::awt::Size& rApiSheetSize,
                            const AnchorSizeModel& rEmuSheetSize ) const;
#endif

    /** Calculates the resulting shape anchor in EMUs. */
    ::com::sun::star::awt::Rectangle
                        calcEmuLocation( const AnchorSizeModel& rEmuSheetSize ) const;

private:
    enum AnchorType { ANCHOR_ABSOLUTE, ANCHOR_ONECELL, ANCHOR_TWOCELL, ANCHOR_INVALID };

    AnchorType          meType;             /// Type of this shape anchor.
    AnchorPosModel      maPos;              /// Top-left position, if anchor is of type absolute.
    AnchorSizeModel     maSize;             /// Anchor size, if anchor is not of type two-cell.
    AnchorCellModel     maFrom;             /// Top-left position, if anchor is not of type absolute.
    AnchorCellModel     maTo;               /// Bottom-right position, if anchor is of type two-cell.
    AnchorClientDataModel maClientData;     /// Shape client data.
    sal_Int32           mnEditAs;           /// Anchor mode as shown in the UI.
};

typedef ::boost::shared_ptr< ShapeAnchor > ShapeAnchorRef;

// ============================================================================

/** Fragment handler for a complete sheet drawing. */
class OoxDrawingFragment : public OoxWorksheetFragmentBase
{
public:
    explicit            OoxDrawingFragment(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                        mxDrawPage;             /// Drawing page of this sheet.
    ::com::sun::star::awt::Size maApiSheetSize; /// Sheet size in 1/100 mm.
    AnchorSizeModel     maEmuSheetSize;         /// Sheet size in EMU.
    ::oox::drawingml::ShapePtr mxShape;         /// Current top-level shape.
    ShapeAnchorRef      mxAnchor;               /// Current anchor of top-level shape.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

