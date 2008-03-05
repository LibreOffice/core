/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typegroupmodel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:51:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_DRAWINGML_CHART_TYPEGROUPMODEL_HXX
#define OOX_DRAWINGML_CHART_TYPEGROUPMODEL_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/drawingml/chart/modelbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct TypeGroupData
{
    typedef ::std::vector< sal_Int32 > AxisIdVector;

    AxisIdVector        maAxisIds;          /// List of axis identifiers used by this chart type.
    double              mfSplitPos;         /// Threshold value in pie-to charts.
    sal_Int32           mnBarDir;           /// Bar direction in bar charts (vertical/horizontal).
    sal_Int32           mnBubbleScale;      /// Relative scaling of bubble size (percent).
    sal_Int32           mnFirstAngle;       /// Rotation angle of first slice in pie charts.
    sal_Int32           mnGapDepth;         /// Space between series in deep 3D charts.
    sal_Int32           mnGapWidth;         /// Space between bars in bar charts, or space in pie-to charts.
    sal_Int32           mnGrouping;         /// Series grouping mode.
    sal_Int32           mnHoleSize;         /// Hole size in doughnut charts.
    sal_Int32           mnOfPieType;        /// Pie-to-pie or pie-to-bar chart.
    sal_Int32           mnOverlap;          /// Bar overlap per category (2D bar charts only).
    sal_Int32           mnRadarStyle;       /// Type of radar chart (lines, markers, filled).
    sal_Int32           mnScatterStyle;     /// Type of scatter chart (
    sal_Int32           mnSecondPieSize;    /// relative size of second pie/bar in pie-to charts (percent).
    sal_Int32           mnShape;            /// 3D bar shape.
    sal_Int32           mnSizeRepresents;   /// Bubble size represents area or width.
    sal_Int32           mnSplitType;        /// Split type in pie-to charts.
    sal_Int32           mnTypeId;           /// Chart type identifier.
    bool                mbBubble3d;         /// True = show bubbles with 3D shade.
    bool                mbShowMarker;       /// True = show point markers in line charts.
    bool                mbShowNegBubbles;   /// True = show absolute value of negative bubbles.
    bool                mbSmooth;           /// True = smooth lines in line charts.
    bool                mbVaryColors;       /// True = different automatic colors for each point.
    bool                mbWireframe;        /// True = wireframe surface chart, false = filled surface chart.

    explicit            TypeGroupData( sal_Int32 nTypeId );
};

// ----------------------------------------------------------------------------

class SeriesModel;

class TypeGroupModel : public ModelData< TypeGroupData >
{
public:
    explicit            TypeGroupModel( sal_Int32 nTypeId );
    virtual             ~TypeGroupModel();

    /** Creates a new data series attached to this type group. */
    SeriesModel&        createSeries();

private:
    typedef RefVector< SeriesModel > SeriesVector;

    SeriesVector        maSeries;           /// Series attached to this chart type group.
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

