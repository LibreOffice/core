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

                 AnchorPosModel() : mfX( -1.0 ), mfY( -1.0 ) {}
    bool         isValid() const { return (0.0 <= mfX) && (mfX <= 1.0) && (0.0 <= mfY) && (mfY <= 1.0); }
};

// ----------------------------------------------------------------------------

/** Absolute shape size in a chart object (in EMUs). */
struct AnchorSizeModel : public EmuSize
{
                 AnchorSizeModel() : EmuSize( -1, -1 ) {}
    bool         isValid() const { return (Width >= 0) && (Height >= 0); }
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
    void                setPos( sal_Int32 nElement, sal_Int32 nParentContext, const OUString& rValue );

    /** Calculates the resulting shape anchor in EMUs. */
    EmuRectangle        calcAnchorRectEmu( const EmuRectangle& rChartRect ) const;

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
                            const OUString& rFragmentPath,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxDrawPage,
                            const ::com::sun::star::awt::Size& rChartSize,
                            const ::com::sun::star::awt::Point& rShapesOffset,
                            bool bOleSupport );
    virtual             ~ChartDrawingFragment();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const OUString& rChars );
    virtual void        onEndElement();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                        mxDrawPage;             /// Drawing page of this sheet.
    ::oox::drawingml::ShapePtr mxShape;         /// Current top-level shape.
    ShapeAnchorRef      mxAnchor;               /// Current anchor of top-level shape.
    EmuRectangle        maChartRectEmu;         /// Position and size of the chart object for embedded shapes (in EMUs).
    bool                mbOleSupport;           /// True = allow to insert OLE objects into the drawing page.
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
