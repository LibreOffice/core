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

$(eval $(call gb_Package_Package,basebmp_inc,$(SRCDIR)/basebmp/inc))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/accessor.hxx,basebmp/accessor.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/accessoradapters.hxx,basebmp/accessoradapters.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/accessorfunctors.hxx,basebmp/accessorfunctors.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/accessortraits.hxx,basebmp/accessortraits.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/bitmapdevice.hxx,basebmp/bitmapdevice.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/clippedlinerenderer.hxx,basebmp/clippedlinerenderer.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/color.hxx,basebmp/color.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/colorblendaccessoradapter.hxx,basebmp/colorblendaccessoradapter.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/colormisc.hxx,basebmp/colormisc.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/colortraits.hxx,basebmp/colortraits.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/compositeiterator.hxx,basebmp/compositeiterator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/debug.hxx,basebmp/debug.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/drawmodes.hxx,basebmp/drawmodes.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/endian.hxx,basebmp/endian.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/fillimage.hxx,basebmp/fillimage.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/genericcolorimageaccessor.hxx,basebmp/genericcolorimageaccessor.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/greylevelformats.hxx,basebmp/greylevelformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/iteratortraits.hxx,basebmp/iteratortraits.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/linerenderer.hxx,basebmp/linerenderer.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/metafunctions.hxx,basebmp/metafunctions.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/nonstandarditerator.hxx,basebmp/nonstandarditerator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/packedpixeliterator.hxx,basebmp/packedpixeliterator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/paletteformats.hxx,basebmp/paletteformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/paletteimageaccessor.hxx,basebmp/paletteimageaccessor.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/pixelformatadapters.hxx,basebmp/pixelformatadapters.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/pixeliterator.hxx,basebmp/pixeliterator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/polypolygonrenderer.hxx,basebmp/polypolygonrenderer.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/rgb24pixelformats.hxx,basebmp/rgb24pixelformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/rgbmaskpixelformats.hxx,basebmp/rgbmaskpixelformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/scaleimage.hxx,basebmp/scaleimage.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/scanlineformats.hxx,basebmp/scanlineformats.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/stridedarrayiterator.hxx,basebmp/stridedarrayiterator.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/tools.hxx,basebmp/tools.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/truecolormaskaccessor.hxx,basebmp/truecolormaskaccessor.hxx))

# vim: set noet sw=4 ts=4:
