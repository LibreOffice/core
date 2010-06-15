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

#ifndef OOX_DRAWINGML_CHART_CHARTDRAWINGFRAGMENT_HXX
#define OOX_DRAWINGML_CHART_CHARTDRAWINGFRAGMENT_HXX

#include "oox/core/fragmenthandler2.hxx"
#include "oox/drawingml/shape.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

/** Relative shape position in a chart object. */
struct AnchorPosModel
{
    double              mfX;                /// X coordinate relative to chart object (0.0 to 1.0).
    double              mfY;                /// Y coordinate relative to chart object (0.0 to 1.0).

    inline explicit     AnchorPosModel() : mfX( -1.0 ), mfY( -1.0 ) {}
    inline bool         isValid() const { return (0.0 <= mfX) && (mfX <= 1.0) && (0.0 <= mfY) && (mfY <= 1.0); }
};

// ----------------------------------------------------------------------------

/** Absolute shape size in a chart object (in EMUs). */
struct AnchorSizeModel : public EmuSize
{
    inline explicit     AnchorSizeModel() : EmuSize( -1, -1 ) {}
    inline bool         isValid() const { return (Width >= 0) && (Height >= 0); }
};

// ============================================================================

/** Contains the position of a shape in the chart object. Supports different
    shape anchor modes (absolute, relative).
 */
class ShapeAnchor
{
public:
    explicit            ShapeAnchor( bool bRelSize );

    /** Imports the absolute anchor size from the cdr:ext element. */
    void                importExt( const AttributeList& rAttribs );
    /** Sets an the relative anchor position from the cdr:from or cdr:to element. */
    void                setPos( sal_Int32 nElement, sal_Int32 nParentContext, const ::rtl::OUString& rValue );

    /** Calculates the resulting shape anchor in EMUs. */
    ::com::sun::star::awt::Rectangle
                        calcEmuLocation( const EmuRectangle& rEmuChartRect ) const;

private:
    AnchorPosModel      maFrom;             /// Top-left position relative to chart object.
    AnchorPosModel      maTo;               /// Bottom-right position relative to chart object.
    AnchorSizeModel     maSize;             /// Shape size, if anchor has absolute size.
    bool                mbRelSize;          /// True = relative size, false = absolute size.
};

typedef ::boost::shared_ptr< ShapeAnchor > ShapeAnchorRef;

// ============================================================================

/** Handler for a chart drawing fragment (c:userShapes root element).
 */
class ChartDrawingFragment : public ::oox::core::FragmentHandler2
{
public:
    explicit            ChartDrawingFragment(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::rtl::OUString& rFragmentPath,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxDrawPage,
                            const ::com::sun::star::awt::Size& rChartSize,
                            const ::com::sun::star::awt::Point& rShapesOffset,
                            bool bOleSupport );
    virtual             ~ChartDrawingFragment();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                        mxDrawPage;             /// Drawing page of this sheet.
    ::oox::drawingml::ShapePtr mxShape;         /// Current top-level shape.
    ShapeAnchorRef      mxAnchor;               /// Current anchor of top-level shape.
    EmuRectangle        maEmuChartRect;         /// Position and size of the chart object for embedded shapes (in EMUs).
    bool                mbOleSupport;           /// True = allow to insert OLE objects into the drawing page.
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

