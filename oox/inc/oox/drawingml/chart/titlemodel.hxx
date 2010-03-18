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

#ifndef OOX_DRAWINGML_CHART_TITLEMODEL_HXX
#define OOX_DRAWINGML_CHART_TITLEMODEL_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/chart/datasourcemodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct LayoutModel
{
    double              mfX;                /// Left position of this object.
    double              mfY;                /// Top position of this object.
    double              mfW;                /// Width of this object.
    double              mfH;                /// Height of this object.
    sal_Int32           mnXMode;            /// Mode for left position.
    sal_Int32           mnYMode;            /// Mode for top position.
    sal_Int32           mnWMode;            /// Mode for width.
    sal_Int32           mnHMode;            /// Mode for height.
    sal_Int32           mnTarget;           /// Layout target for plot area.

    explicit            LayoutModel();
                        ~LayoutModel();
};

// ============================================================================

struct TextModel
{
    typedef ModelRef< DataSequenceModel >   DataSequenceRef;
    typedef ModelRef< TextBody >            TextBodyRef;

    DataSequenceRef     mxDataSeq;          /// The string data or formula link of this text.
    TextBodyRef         mxTextBody;         /// Rich-formatted literal text (for title objects only).

    explicit            TextModel();
                        ~TextModel();
};

// ============================================================================

struct TitleModel
{
    typedef ModelRef< Shape >       ShapeRef;
    typedef ModelRef< TextBody >    TextBodyRef;
    typedef ModelRef< LayoutModel > LayoutRef;
    typedef ModelRef< TextModel >   TextRef;

    ShapeRef            mxShapeProp;        /// Title shape formatting.
    TextBodyRef         mxTextProp;         /// Title text formatting.
    LayoutRef           mxLayout;           /// Layout/position of the frame.
    TextRef             mxText;             /// Text source of the title.
    bool                mbOverlay;          /// True = title may overlay other objects.

    explicit            TitleModel();
                        ~TitleModel();
};

// ============================================================================

struct LegendModel
{
    typedef ModelRef< Shape >       ShapeRef;
    typedef ModelRef< TextBody >    TextBodyRef;
    typedef ModelRef< LayoutModel > LayoutRef;

    ShapeRef            mxShapeProp;        /// Legend shape formatting.
    TextBodyRef         mxTextProp;         /// Legend text formatting.
    LayoutRef           mxLayout;           /// Layout/position of the legend.
    sal_Int32           mnPosition;         /// Legend position.
    bool                mbOverlay;          /// True = legend may overlay other objects.

    explicit            LegendModel();
                        ~LegendModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

