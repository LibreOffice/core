#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Package_Package,basegfx_inc,$(SRCDIR)/basegfx/inc))

$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tools/lerp.hxx,basegfx/tools/lerp.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tools/keystoplerp.hxx,basegfx/tools/keystoplerp.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tools/debugplotter.hxx,basegfx/tools/debugplotter.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tools/canvastools.hxx,basegfx/tools/canvastools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tools/rectcliptools.hxx,basegfx/tools/rectcliptools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tools/b2dclipstate.hxx,basegfx/tools/b2dclipstate.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tools/unopolypolygon.hxx,basegfx/tools/unopolypolygon.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tools/tools.hxx,basegfx/tools/tools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tools/gradienttools.hxx,basegfx/tools/gradienttools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/numeric/ftools.hxx,basegfx/numeric/ftools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tuple/b3i64tuple.hxx,basegfx/tuple/b3i64tuple.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tuple/b3dtuple.hxx,basegfx/tuple/b3dtuple.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tuple/b3ituple.hxx,basegfx/tuple/b3ituple.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tuple/b2i64tuple.hxx,basegfx/tuple/b2i64tuple.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tuple/b2ituple.hxx,basegfx/tuple/b2ituple.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/tuple/b2dtuple.hxx,basegfx/tuple/b2dtuple.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b2drangeclipper.hxx,basegfx/range/b2drangeclipper.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b3irange.hxx,basegfx/range/b3irange.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b3dvolume.hxx,basegfx/range/b3dvolume.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/basicbox.hxx,basegfx/range/basicbox.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b2dconnectedranges.hxx,basegfx/range/b2dconnectedranges.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b3drange.hxx,basegfx/range/b3drange.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b2drange.hxx,basegfx/range/b2drange.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/rangeexpander.hxx,basegfx/range/rangeexpander.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b2irange.hxx,basegfx/range/b2irange.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b1ibox.hxx,basegfx/range/b1ibox.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b2dpolyrange.hxx,basegfx/range/b2dpolyrange.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b3ivolume.hxx,basegfx/range/b3ivolume.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b1irange.hxx,basegfx/range/b1irange.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b2drectangle.hxx,basegfx/range/b2drectangle.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b2irectangle.hxx,basegfx/range/b2irectangle.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b1drange.hxx,basegfx/range/b1drange.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b2ibox.hxx,basegfx/range/b2ibox.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/b3ibox.hxx,basegfx/range/b3ibox.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/range/basicrange.hxx,basegfx/range/basicrange.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/vector/b3dsize.hxx,basegfx/vector/b3dsize.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/vector/b2ivector.hxx,basegfx/vector/b2ivector.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/vector/b2dvector.hxx,basegfx/vector/b2dvector.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/vector/b3dvector.hxx,basegfx/vector/b3dvector.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/vector/b2dsize.hxx,basegfx/vector/b2dsize.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/vector/b2isize.hxx,basegfx/vector/b2isize.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/vector/b2enums.hxx,basegfx/vector/b2enums.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/vector/b3ivector.hxx,basegfx/vector/b3ivector.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/vector/b3isize.hxx,basegfx/vector/b3isize.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dtrapezoid.hxx,basegfx/polygon/b2dtrapezoid.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolypolygoncutter.hxx,basegfx/polygon/b2dpolypolygoncutter.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolypolygon.hxx,basegfx/polygon/b2dpolypolygon.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolypolygontools.hxx,basegfx/polygon/b2dpolypolygontools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolygon.hxx,basegfx/polygon/b2dpolygon.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolygonclipper.hxx,basegfx/polygon/b2dpolygonclipper.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolypolygonfillrule.hxx,basegfx/polygon/b2dpolypolygonfillrule.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolygontriangulator.hxx,basegfx/polygon/b2dpolygontriangulator.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b3dpolygontools.hxx,basegfx/polygon/b3dpolygontools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolygontools.hxx,basegfx/polygon/b2dpolygontools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b3dpolypolygon.hxx,basegfx/polygon/b3dpolypolygon.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolygoncutandtouch.hxx,basegfx/polygon/b2dpolygoncutandtouch.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dlinegeometry.hxx,basegfx/polygon/b2dlinegeometry.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b3dpolypolygontools.hxx,basegfx/polygon/b3dpolypolygontools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b2dpolypolygonrasterconverter.hxx,basegfx/polygon/b2dpolypolygonrasterconverter.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b3dpolygon.hxx,basegfx/polygon/b3dpolygon.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/polygon/b3dpolygonclipper.hxx,basegfx/polygon/b3dpolygonclipper.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/pixel/bpixel.hxx,basegfx/pixel/bpixel.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/matrix/b2dhommatrix.hxx,basegfx/matrix/b2dhommatrix.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/matrix/b3dhommatrix.hxx,basegfx/matrix/b3dhommatrix.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/matrix/b2dhommatrixtools.hxx,basegfx/matrix/b2dhommatrixtools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/point/b3dhompoint.hxx,basegfx/point/b3dhompoint.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/point/b3ipoint.hxx,basegfx/point/b3ipoint.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/point/b3dpoint.hxx,basegfx/point/b3dpoint.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/point/b2dpoint.hxx,basegfx/point/b2dpoint.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/point/b2dhompoint.hxx,basegfx/point/b2dhompoint.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/point/b2ipoint.hxx,basegfx/point/b2ipoint.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/curve/b2dcubicbezier.hxx,basegfx/curve/b2dcubicbezier.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/curve/b2dquadraticbezier.hxx,basegfx/curve/b2dquadraticbezier.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/curve/b2dbeziertools.hxx,basegfx/curve/b2dbeziertools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/color/bcolor.hxx,basegfx/color/bcolor.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/color/bcolortools.hxx,basegfx/color/bcolortools.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/color/bcolormodifier.hxx,basegfx/color/bcolormodifier.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/raster/bpixelraster.hxx,basegfx/raster/bpixelraster.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/raster/rasterconvert3d.hxx,basegfx/raster/rasterconvert3d.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/raster/bzpixelraster.hxx,basegfx/raster/bzpixelraster.hxx))
$(eval $(call gb_Package_add_file,basegfx_inc,inc/basegfx/basegfxdllapi.h,basegfx/basegfxdllapi.h))

# vim: set noet sw=4 ts=4:
