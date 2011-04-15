# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,chartview))

$(eval $(call gb_Library_add_precompiled_header,chartview,$(SRCDIR)/chart2/inc/pch/precompiled_chart2))

$(eval $(call gb_Library_set_include,chartview,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/chart2/inc/pch) \
    -I$(realpath $(SRCDIR)/chart2/source/view/inc) \
    -I$(realpath $(SRCDIR)/chart2/source/inc) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,chartview,\
    $$(DEFS) \
    -DOOO_DLLIMPLEMENTATION_CHARTVIEW \
))

$(eval $(call gb_Library_add_linked_libs,chartview,\
    basegfx \
    charttools \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    sal \
    sfx \
    svl \
    svt \
    svxcore \
    tl \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,chartview,chart2/source/view/chartview))

$(eval $(call gb_Library_add_exception_objects,chartview,\
    chart2/source/view/axes/DateHelper \
    chart2/source/view/axes/DateScaling \
    chart2/source/view/axes/MinimumAndMaximumSupplier \
    chart2/source/view/axes/ScaleAutomatism \
    chart2/source/view/axes/Tickmarks \
    chart2/source/view/axes/Tickmarks_Dates \
    chart2/source/view/axes/Tickmarks_Equidistant \
    chart2/source/view/axes/VAxisBase \
    chart2/source/view/axes/VAxisOrGridBase \
    chart2/source/view/axes/VAxisProperties \
    chart2/source/view/axes/VCartesianAxis \
    chart2/source/view/axes/VCartesianCoordinateSystem \
    chart2/source/view/axes/VCartesianGrid \
    chart2/source/view/axes/VCoordinateSystem \
    chart2/source/view/axes/VPolarAngleAxis \
    chart2/source/view/axes/VPolarAxis \
    chart2/source/view/axes/VPolarCoordinateSystem \
    chart2/source/view/axes/VPolarGrid \
    chart2/source/view/axes/VPolarRadiusAxis \
    chart2/source/view/charttypes/AreaChart \
    chart2/source/view/charttypes/BarChart \
    chart2/source/view/charttypes/BarPositionHelper \
    chart2/source/view/charttypes/BubbleChart \
    chart2/source/view/charttypes/CandleStickChart \
    chart2/source/view/charttypes/CategoryPositionHelper \
    chart2/source/view/charttypes/PieChart \
    chart2/source/view/charttypes/Splines \
    chart2/source/view/charttypes/VSeriesPlotter \
    chart2/source/view/diagram/VDiagram \
    chart2/source/view/main/ChartItemPool \
    chart2/source/view/main/ChartView \
    chart2/source/view/main/Clipping \
    chart2/source/view/main/DataPointSymbolSupplier \
    chart2/source/view/main/DrawModelWrapper \
    chart2/source/view/main/LabelPositionHelper \
    chart2/source/view/main/Linear3DTransformation \
    chart2/source/view/main/PlotterBase \
    chart2/source/view/main/PlottingPositionHelper \
    chart2/source/view/main/PolarLabelPositionHelper \
    chart2/source/view/main/PropertyMapper \
    chart2/source/view/main/_serviceregistration_view \
    chart2/source/view/main/ShapeFactory \
    chart2/source/view/main/Stripe \
    chart2/source/view/main/VDataSeries \
    chart2/source/view/main/VLegend \
    chart2/source/view/main/VLegendSymbolFactory \
    chart2/source/view/main/VLineProperties \
    chart2/source/view/main/VPolarTransformation \
    chart2/source/view/main/VTitle \
))

# vim: set noet ts=4 sw=4:
