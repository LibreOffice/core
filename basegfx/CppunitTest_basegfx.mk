# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,basegfx))

$(eval $(call gb_CppunitTest_use_sdk_api,basegfx))

$(eval $(call gb_CppunitTest_use_libraries,basegfx,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_CppunitTest_add_exception_objects,basegfx,\
    basegfx/test/B2DRangeTest \
    basegfx/test/B2IRangeTest \
    basegfx/test/B2DPolyRangeTest \
    basegfx/test/B2DHomMatrixTest \
    basegfx/test/B2DPointTest \
    basegfx/test/B2DPolygonTest \
    basegfx/test/B2DPolygonToolsTest \
    basegfx/test/B2DPolyPolygonTest \
    basegfx/test/B2DPolyPolygonCutterTest \
    basegfx/test/B2DSizeTest \
    basegfx/test/B1DRangeTest \
    basegfx/test/B2XRangeTest \
    basegfx/test/B2IBoxTest \
    basegfx/test/B2DTupleTest \
    basegfx/test/B3DTupleTest \
    basegfx/test/BColorModifierTest \
    basegfx/test/BColorTest \
    basegfx/test/SvgPathImportExport \
    basegfx/test/basegfx3d \
    basegfx/test/boxclipper \
    basegfx/test/basegfxtools \
    basegfx/test/clipstate \
    basegfx/test/genericclipper \
    basegfx/test/VectorTest \
))

# vim: set noet sw=4 ts=4:
