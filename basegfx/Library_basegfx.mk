# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,basegfx))

$(eval $(call gb_Library_use_packages,basegfx,\
    basegfx_inc \
    comphelper_inc \
    o3tl_inc \
))

$(eval $(call gb_Library_use_sdk_api,basegfx))

$(eval $(call gb_Library_use_libraries,basegfx,\
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,basegfx))

$(eval $(call gb_Library_set_include,basegfx,\
    -I$(SRCDIR)/basegfx/inc \
    -I$(SRCDIR)/basegfx/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,basegfx,\
    -DBASEGFX_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_exception_objects,basegfx,\
    basegfx/source/color/bcolormodifier \
    basegfx/source/color/bcolortools \
    basegfx/source/curve/b2dbeziertools \
    basegfx/source/curve/b2dcubicbezier \
    basegfx/source/matrix/b2dhommatrix \
    basegfx/source/matrix/b2dhommatrixtools \
    basegfx/source/matrix/b3dhommatrix \
    basegfx/source/numeric/ftools \
    basegfx/source/pixel/bpixel \
    basegfx/source/point/b2dpoint \
    basegfx/source/point/b2ipoint \
    basegfx/source/point/b3dpoint \
    basegfx/source/point/b3ipoint \
    basegfx/source/polygon/b2dlinegeometry \
    basegfx/source/polygon/b2dpolygon \
    basegfx/source/polygon/b2dpolygonclipper \
    basegfx/source/polygon/b2dpolygoncutandtouch \
    basegfx/source/polygon/b2dpolygontools \
    basegfx/source/polygon/b2dpolygontriangulator \
    basegfx/source/polygon/b2dpolypolygon \
    basegfx/source/polygon/b2dpolypolygoncutter \
    basegfx/source/polygon/b2dpolypolygontools \
    basegfx/source/polygon/b2dsvgpolypolygon \
    basegfx/source/polygon/b2dtrapezoid \
    basegfx/source/polygon/b3dpolygon \
    basegfx/source/polygon/b3dpolygontools \
    basegfx/source/polygon/b3dpolypolygon \
    basegfx/source/polygon/b3dpolypolygontools \
    basegfx/source/range/b1drange \
    basegfx/source/range/b2dpolyrange \
    basegfx/source/range/b2drange \
    basegfx/source/range/b2drangeclipper \
    basegfx/source/range/b2xrange \
    basegfx/source/range/b3drange \
    basegfx/source/raster/rasterconvert3d \
    basegfx/source/tools/b2dclipstate \
    basegfx/source/tools/canvastools \
    basegfx/source/tools/gradienttools \
    basegfx/source/tools/keystoplerp \
    basegfx/source/tools/tools \
    basegfx/source/tools/unopolypolygon \
    basegfx/source/tools/unotools \
    basegfx/source/tools/zoomtools \
    basegfx/source/tuple/b2dtuple \
    basegfx/source/tuple/b2i64tuple \
    basegfx/source/tuple/b2ituple \
    basegfx/source/tuple/b3dtuple \
    basegfx/source/tuple/b3i64tuple \
    basegfx/source/tuple/b3ituple \
    basegfx/source/vector/b2dvector \
    basegfx/source/vector/b2ivector \
    basegfx/source/vector/b3dvector \
    basegfx/source/vector/b3ivector \
))


# vim: set noet sw=4 ts=4:
