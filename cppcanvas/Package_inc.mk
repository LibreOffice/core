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

$(eval $(call gb_Package_Package,cppcanvas_inc,$(SRCDIR)/cppcanvas/inc))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/canvas.hxx,cppcanvas/canvas.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/sprite.hxx,cppcanvas/sprite.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/vclfactory.hxx,cppcanvas/vclfactory.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/font.hxx,cppcanvas/font.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/customsprite.hxx,cppcanvas/customsprite.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/canvasgraphic.hxx,cppcanvas/canvasgraphic.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/polypolygon.hxx,cppcanvas/polypolygon.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/color.hxx,cppcanvas/color.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/spritecanvas.hxx,cppcanvas/spritecanvas.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/text.hxx,cppcanvas/text.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/renderer.hxx,cppcanvas/renderer.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/basegfxfactory.hxx,cppcanvas/basegfxfactory.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/bitmap.hxx,cppcanvas/bitmap.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/bitmapcanvas.hxx,cppcanvas/bitmapcanvas.hxx))
$(eval $(call gb_Package_add_file,cppcanvas_inc,inc/cppcanvas/cppcanvasdllapi.h,cppcanvas/cppcanvasdllapi.h))

# vim: set noet sw=4 ts=4:
