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

$(eval $(call gb_Library_Library,chartmodel))

$(eval $(call gb_Library_add_precompiled_header,chartmodel,$(SRCDIR)/chart2/inc/pch/precompiled_chart2))

$(eval $(call gb_Library_set_include,chartmodel,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/chart2/inc/pch) \
    -I$(realpath $(SRCDIR)/chart2/source/model/inc) \
    -I$(realpath $(SRCDIR)/chart2/source/inc) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

# TODO: is this still necessary?
# (from chart2/source/model/template/makefile.mk)
# # i26518 the gcc-3.0.4 requires to enhance the template-depth
# # this seems to be a compiler issue, so we recommend not to use 3.0.x anymore
# .IF "$(COM)"=="GCC" 
#     CFLAGS+=-ftemplate-depth-128
# .ENDIF

$(eval $(call gb_Library_add_linked_libs,chartmodel,\
    charttools \
    comphelper \
    cppu \
    cppuhelper \
    fwe \
    sal \
    svl \
    svt \
    ucbhelper \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,chartmodel,chart2/source/model/chartmodel))

$(eval $(call gb_Library_add_exception_objects,chartmodel,\
    chart2/source/model/filter/XMLFilter \
    chart2/source/model/main/Axis \
    chart2/source/model/main/BaseCoordinateSystem \
    chart2/source/model/main/CartesianCoordinateSystem \
    chart2/source/model/main/ChartModel \
    chart2/source/model/main/ChartModel_Persistence \
    chart2/source/model/main/DataPoint \
    chart2/source/model/main/DataPointProperties \
    chart2/source/model/main/DataSeries \
    chart2/source/model/main/DataSeriesProperties \
    chart2/source/model/main/Diagram \
    chart2/source/model/main/FormattedString \
    chart2/source/model/main/GridProperties \
    chart2/source/model/main/Legend \
    chart2/source/model/main/PageBackground \
    chart2/source/model/main/PolarCoordinateSystem \
    chart2/source/model/main/_serviceregistration_model \
    chart2/source/model/main/StockBar \
    chart2/source/model/main/Title \
    chart2/source/model/main/UndoManager \
    chart2/source/model/main/Wall \
    chart2/source/model/template/AreaChartType \
    chart2/source/model/template/AreaChartTypeTemplate \
    chart2/source/model/template/BarChartType \
    chart2/source/model/template/BarChartTypeTemplate \
    chart2/source/model/template/BubbleChartType \
    chart2/source/model/template/BubbleChartTypeTemplate \
    chart2/source/model/template/BubbleDataInterpreter \
    chart2/source/model/template/CandleStickChartType \
    chart2/source/model/template/ChartType \
    chart2/source/model/template/ChartTypeManager \
    chart2/source/model/template/ChartTypeTemplate \
    chart2/source/model/template/ColumnChartType \
    chart2/source/model/template/ColumnLineChartTypeTemplate \
    chart2/source/model/template/ColumnLineDataInterpreter \
    chart2/source/model/template/DataInterpreter \
    chart2/source/model/template/FilledNetChartType \
    chart2/source/model/template/LineChartType \
    chart2/source/model/template/LineChartTypeTemplate \
    chart2/source/model/template/NetChartType \
    chart2/source/model/template/NetChartTypeTemplate \
    chart2/source/model/template/PieChartType \
    chart2/source/model/template/PieChartTypeTemplate \
    chart2/source/model/template/ScatterChartType \
    chart2/source/model/template/ScatterChartTypeTemplate \
    chart2/source/model/template/_serviceregistration_charttypes \
    chart2/source/model/template/StockChartTypeTemplate \
    chart2/source/model/template/StockDataInterpreter \
    chart2/source/model/template/XYDataInterpreter \
))

# vim: set noet ts=4 sw=4:
