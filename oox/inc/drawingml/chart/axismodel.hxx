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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_AXISMODEL_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_AXISMODEL_HXX

#include <oox/drawingml/shape.hxx>
#include <oox/token/tokens.hxx>
#include <drawingml/chart/titlemodel.hxx>

namespace oox {
namespace drawingml {
namespace chart {

struct AxisDispUnitsModel
{
    typedef ModelRef< Shape >       ShapeRef;
    typedef ModelRef< TextBody >    TextBodyRef;
    typedef ModelRef< LayoutModel > LayoutRef;
    typedef ModelRef< TextModel >   TextRef;

    ShapeRef            mxShapeProp;        /// Label frame formatting.
    TextBodyRef         mxTextProp;         /// Label text formatting.
    LayoutRef           mxLayout;           /// Layout/position of the axis units label.
    TextRef             mxText;             /// Text source of the axis units label.
    double              mfCustomUnit;       /// Custom unit size on value axis.
    OUString            mnBuiltInUnit;      /// Built-in unit on value axis.

    explicit            AxisDispUnitsModel();
                        ~AxisDispUnitsModel();
};

struct AxisModel
{
    typedef ModelRef< Shape >               ShapeRef;
    typedef ModelRef< TextBody >            TextBodyRef;
    typedef ModelRef< TitleModel >          TitleRef;
    typedef ModelRef< AxisDispUnitsModel >  AxisDispUnitsRef;

    ShapeRef            mxShapeProp;        /// Axis line formatting.
    TextBodyRef         mxTextProp;         /// Axis label text formatting.
    TitleRef            mxTitle;            /// Axis title.
    AxisDispUnitsRef    mxDispUnits;        /// Axis units label.
    ShapeRef            mxMajorGridLines;   /// Major grid lines formatting.
    ShapeRef            mxMinorGridLines;   /// Minor grid lines formatting.
    NumberFormat        maNumberFormat;     /// Number format for axis tick labels.
    OptValue< double >  mofCrossesAt;       /// Position on this axis where another axis crosses.
    OptValue< double >  mofMajorUnit;       /// Unit for major tick marks on date/value axis.
    OptValue< double >  mofMinorUnit;       /// Unit for minor tick marks on date/value axis.
    OptValue< double >  mofLogBase;         /// Logarithmic base for logarithmic axes.
    OptValue< double >  mofMax;             /// Maximum axis value.
    OptValue< double >  mofMin;             /// Minimum axis value.
    OptValue< sal_Int32 > monBaseTimeUnit;  /// Base time unit shown on a date axis.
    sal_Int32           mnAxisId;           /// Unique axis identifier.
    sal_Int32           mnAxisPos;          /// Position of the axis (top/bottom/left/right).
    sal_Int32           mnCrossAxisId;      /// Identifier of a crossing axis.
    sal_Int32           mnCrossBetween;     /// This value axis crosses between or inside category.
    sal_Int32           mnCrossMode;        /// Mode this axis crosses another axis (min, max, auto).
    sal_Int32           mnLabelAlign;       /// Tick mark label alignment.
    sal_Int32           mnLabelOffset;      /// Tick mark label distance from axis.
    sal_Int32           mnMajorTickMark;    /// Major tick mark style.
    sal_Int32           mnMajorTimeUnit;    /// Time unit for major tick marks on date axis.
    sal_Int32           mnMinorTickMark;    /// Mainor tick mark style.
    sal_Int32           mnMinorTimeUnit;    /// Time unit for minor tick marks on date axis.
    sal_Int32           mnOrientation;      /// Axis orientation (value order min to max, or max to min).
    sal_Int32           mnTickLabelPos;     /// Position of tick mark labels relative to the axis.
    sal_Int32           mnTickLabelSkip;    /// Number of tick mark labels to skip.
    sal_Int32           mnTickMarkSkip;     /// Number of tick marks to skip.
    sal_Int32           mnTypeId;           /// Type identifier of this axis.
    bool                mbAuto;             /// True = automatic selection of text/date axis type.
    bool                mbDeleted;          /// True = axis has been deleted manually.
    bool                mbNoMultiLevel;     /// True = no multi-level categories supported.

    explicit            AxisModel( sal_Int32 nTypeId, bool bMSO2007Doc );
                        ~AxisModel();
};

}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
