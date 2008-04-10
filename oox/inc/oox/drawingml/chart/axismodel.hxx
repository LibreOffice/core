/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: axismodel.hxx,v $
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

#ifndef OOX_DRAWINGML_CHART_AXISMODEL_HXX
#define OOX_DRAWINGML_CHART_AXISMODEL_HXX

#include "oox/drawingml/chart/modelbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct AxisData
{
    ::rtl::OUString     maFormatCode;       /// Number format code for tick mark labels.
    double              mfCrossesAt;        /// Position on this axis where another axis crosses.
    double              mfCustomUnit;       /// Custom unit size on value axis.
    double              mfMajorUnit;        /// Unit for major tick marks on date/value axis.
    double              mfMinorUnit;        /// Unit for minor tick marks on date/value axis.
    double              mfLogBase;          /// Logarithmic base for logarithmic axes.
    double              mfMax;              /// Maximum axis value.
    double              mfMin;              /// Minimum axis value.
    sal_Int32           mnAxisId;           /// Unique axis identifier.
    sal_Int32           mnAxisPos;          /// Position of the axis (top/bottom/left/right).
    sal_Int32           mnBaseTimeUnit;     /// Base time unit shown on a date axis.
    sal_Int32           mnBuiltInUnit;      /// Built-in unit on value axis.
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
    bool                mbAuto;             /// Automatic axis settings.
    bool                mbDeleted;          /// True = axis has been deleted manually.
    bool                mbNoMultiLevel;     /// True = no multi-level categories supported.
    bool                mbSourceLinked;     /// True = number format linked to source data.

    explicit            AxisData( sal_Int32 nTypeId );
};

// ----------------------------------------------------------------------------

class LayoutModel;

class AxisModel : public ModelData< AxisData >
{
public:
    explicit            AxisModel( sal_Int32 nTypeId );
    virtual             ~AxisModel();

    /** Returns true, if this chart model contains a layout object for unit labels. */
    inline bool         hasUnitLabelsLayout() const { return mxLayout.get() != 0; }
    /** Returns a pointer to an existing unit labels layout object, or null if not present. */
    inline LayoutModel* getUnitLabelsLayout() const { return mxLayout.get(); }
    /** Creates and returns a new layout model object for unit labels. */
    LayoutModel&        createUnitLabelsLayout();

private:
    typedef ::boost::shared_ptr< LayoutModel > LayoutRef;

    LayoutRef           mxLayout;
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

