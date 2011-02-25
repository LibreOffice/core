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

$(eval $(call gb_Package_Package,canvas_inc,$(SRCDIR)/canvas/inc))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/basemutexhelper.hxx,canvas/base/basemutexhelper.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/bitmapcanvasbase.hxx,canvas/base/bitmapcanvasbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/bufferedgraphicdevicebase.hxx,canvas/base/bufferedgraphicdevicebase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/cachedprimitivebase.hxx,canvas/base/cachedprimitivebase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/canvasbase.hxx,canvas/base/canvasbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/canvascustomspritebase.hxx,canvas/base/canvascustomspritebase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/canvascustomspritehelper.hxx,canvas/base/canvascustomspritehelper.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/doublebitmapbase.hxx,canvas/base/doublebitmapbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/floatbitmapbase.hxx,canvas/base/floatbitmapbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/graphicdevicebase.hxx,canvas/base/graphicdevicebase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/integerbitmapbase.hxx,canvas/base/integerbitmapbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/sprite.hxx,canvas/base/sprite.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/spritecanvasbase.hxx,canvas/base/spritecanvasbase.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/base/spritesurface.hxx,canvas/base/spritesurface.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/canvastools.hxx,canvas/canvastools.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/canvastoolsdllapi.h,canvas/canvastoolsdllapi.h))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/debug.hxx,canvas/debug.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/elapsedtime.hxx,canvas/elapsedtime.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/parametricpolypolygon.hxx,canvas/parametricpolypolygon.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/prioritybooster.hxx,canvas/prioritybooster.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/propertysethelper.hxx,canvas/propertysethelper.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/bitmap.hxx,canvas/rendering/bitmap.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/icachedprimitive.hxx,canvas/rendering/icachedprimitive.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/icolorbuffer.hxx,canvas/rendering/icolorbuffer.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/irendermodule.hxx,canvas/rendering/irendermodule.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/isurface.hxx,canvas/rendering/isurface.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/isurfaceproxy.hxx,canvas/rendering/isurfaceproxy.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/rendering/isurfaceproxymanager.hxx,canvas/rendering/isurfaceproxymanager.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/spriteredrawmanager.hxx,canvas/spriteredrawmanager.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/vclwrapper.hxx,canvas/vclwrapper.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/verbosetrace.hxx,canvas/verbosetrace.hxx))
$(eval $(call gb_Package_add_file,canvas_inc,inc/canvas/verifyinput.hxx,canvas/verifyinput.hxx))


# vim: set noet sw=4 ts=4:
