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

$(eval $(call gb_Library_Library,cairocanvas))

$(eval $(call gb_Library_set_componentfile,cairocanvas,canvas/source/cairo/cairocanvas))

$(eval $(call gb_Library_set_include,cairocanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,cairocanvas,\
	$$(DEFS) \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_set_defs,cairocanvas,\
	$$(DEFS) \
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_add_linked_libs,cairocanvas,\
	sal \
	stl \
	cppu \
	basegfx \
	cppuhelper \
	comphelper \
	vcl \
	tk \
	tl \
	i18nisolang1 \
	canvastools \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_cachedbitmap \
	canvas/source/cairo/cairo_cairo \
	canvas/source/cairo/cairo_canvas \
	canvas/source/cairo/cairo_canvasbitmap \
	canvas/source/cairo/cairo_canvascustomsprite \
	canvas/source/cairo/cairo_canvasfont \
	canvas/source/cairo/cairo_canvashelper \
	canvas/source/cairo/cairo_canvashelper_text \
	canvas/source/cairo/cairo_devicehelper \
	canvas/source/cairo/cairo_services \
	canvas/source/cairo/cairo_spritecanvas \
	canvas/source/cairo/cairo_spritecanvashelper \
	canvas/source/cairo/cairo_spritedevicehelper \
	canvas/source/cairo/cairo_spritehelper \
	canvas/source/cairo/cairo_textlayout \
))

$(call gb_Library_use_external,cairocanvas,cairo)

ifeq ($(OS),WNT)

$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_win32_cairo \
))
$(eval $(call gb_Library_add_linked_libs,cairocanvas,\
	gdi32 \
	user32 \
))

else

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_quartz_cairo \
))

#CFLAGSCXX+=$(OBJCXXFLAGS)

else

$(eval $(call gb_Library_add_exception_objects,cairocanvas,\
	canvas/source/cairo/cairo_xlib_cairo \
))

# freetype? fontconfig? -> test on Solaris
$(eval $(call gb_Library_add_linked_libs,cairocanvas,\
	X11 \
	Xrender \
))

endif

endif

# vim: set noet sw=4 ts=4:
