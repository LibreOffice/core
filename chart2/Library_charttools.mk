# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,charttools))

$(eval $(call gb_Library_set_include,charttools,\
    $$(INCLUDE) \
    -I$(SRCDIR)/chart2/source/inc \
))

$(eval $(call gb_Library_add_defs,charttools,\
    -DOOO_DLLIMPLEMENTATION_CHARTTOOLS \
))

$(eval $(call gb_Library_use_sdk_api,charttools))

$(eval $(call gb_Library_use_libraries,charttools,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nisolang1 \
    sal \
    svl \
    tl \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,charttools,chart2/source/tools/charttools))

$(eval $(call gb_Library_add_exception_objects,charttools,\
    chart2/source/tools/AxisHelper \
    chart2/source/tools/BaseGFXHelper \
    chart2/source/tools/CachedDataSequence \
    chart2/source/tools/CharacterProperties \
    chart2/source/tools/ChartDebugTrace \
    chart2/source/tools/ChartModelHelper \
    chart2/source/tools/ChartTypeHelper \
    chart2/source/tools/ChartViewHelper \
    chart2/source/tools/ColorPerPointHelper \
    chart2/source/tools/CommonConverters \
    chart2/source/tools/ConfigColorScheme \
    chart2/source/tools/ControllerLockGuard \
    chart2/source/tools/DataSeriesHelper \
    chart2/source/tools/DataSource \
    chart2/source/tools/DataSourceHelper \
    chart2/source/tools/DiagramHelper \
    chart2/source/tools/ErrorBar \
    chart2/source/tools/ExplicitCategoriesProvider \
    chart2/source/tools/ExponentialRegressionCurveCalculator \
    chart2/source/tools/FillProperties \
    chart2/source/tools/FormattedStringHelper \
    chart2/source/tools/ImplOPropertySet \
    chart2/source/tools/InternalData \
    chart2/source/tools/InternalDataProvider \
    chart2/source/tools/LabeledDataSequence \
    chart2/source/tools/LegendHelper \
    chart2/source/tools/LifeTime \
    chart2/source/tools/LinearRegressionCurveCalculator \
    chart2/source/tools/LineProperties \
    chart2/source/tools/LogarithmicRegressionCurveCalculator \
    chart2/source/tools/MeanValueRegressionCurveCalculator \
    chart2/source/tools/MediaDescriptorHelper \
    chart2/source/tools/ModifyListenerCallBack \
    chart2/source/tools/ModifyListenerHelper \
    chart2/source/tools/MutexContainer \
    chart2/source/tools/NameContainer \
    chart2/source/tools/NumberFormatterWrapper \
    chart2/source/tools/ObjectIdentifier \
    chart2/source/tools/OPropertySet \
    chart2/source/tools/PotentialRegressionCurveCalculator \
    chart2/source/tools/PropertyHelper \
    chart2/source/tools/RangeHighlighter \
    chart2/source/tools/ReferenceSizeProvider \
    chart2/source/tools/RegressionCurveCalculator \
    chart2/source/tools/RegressionCurveHelper \
    chart2/source/tools/RegressionCurveModel \
    chart2/source/tools/RegressionEquation \
    chart2/source/tools/RelativePositionHelper \
    chart2/source/tools/RelativeSizeHelper \
    chart2/source/tools/ResId \
    chart2/source/tools/ResourceManager \
    chart2/source/tools/Scaling \
    chart2/source/tools/SceneProperties \
    chart2/source/tools/_serviceregistration_tools \
    chart2/source/tools/StatisticsHelper \
    chart2/source/tools/ThreeDHelper \
    chart2/source/tools/TitleHelper \
    chart2/source/tools/TrueGuard \
    chart2/source/tools/UncachedDataSequence \
    chart2/source/tools/UserDefinedProperties \
    chart2/source/tools/WeakListenerAdapter \
    chart2/source/tools/WrappedDefaultProperty \
    chart2/source/tools/WrappedDirectStateProperty \
    chart2/source/tools/WrappedIgnoreProperty \
    chart2/source/tools/WrappedProperty \
    chart2/source/tools/WrappedPropertySet \
    chart2/source/tools/XMLRangeHelper \
))

# vim: set noet sw=4 ts=4:
