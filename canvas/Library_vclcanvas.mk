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

$(eval $(call gb_Library_Library,vclcanvas))

$(eval $(call gb_Library_set_componentfile,vclcanvas,canvas/source/vcl/vclcanvas))

$(eval $(call gb_Library_set_include,vclcanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
	-I$(OUTDIR)/inc/offuh \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,vclcanvas,\
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_add_linked_libs,vclcanvas,\
	sal \
	stl \
	cppu \
	basegfx \
	cppuhelper \
	comphelper \
	vcl \
	tk \
	tl \
	svt \
	i18nisolang1 \
	canvastools \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,vclcanvas,\
	canvas/source/vcl/backbuffer \
	canvas/source/vcl/bitmapbackbuffer \
	canvas/source/vcl/cachedbitmap \
	canvas/source/vcl/canvas \
	canvas/source/vcl/canvasbitmap \
	canvas/source/vcl/canvasbitmaphelper \
	canvas/source/vcl/canvascustomsprite \
	canvas/source/vcl/canvasfont \
	canvas/source/vcl/canvashelper \
	canvas/source/vcl/devicehelper \
	canvas/source/vcl/impltools \
	canvas/source/vcl/services \
	canvas/source/vcl/spritecanvas \
	canvas/source/vcl/spritecanvashelper \
	canvas/source/vcl/spritedevicehelper \
	canvas/source/vcl/spritehelper \
	canvas/source/vcl/textlayout \
	canvas/source/vcl/windowoutdevholder \
))

# Solaris Sparc with Sun Compiler: noopt
ifneq ($(strip($OS)),SOLARIS)
$(eval $(call gb_Library_add_exception_objects,vclcanvas,\
	canvas/source/vcl/canvashelper_texturefill \
))
else
$(eval $(call gb_Library_add_cxxobjects,vclcanvas,\
	canvas/source/vcl/canvashelper_texturefill \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:
