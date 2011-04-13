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

$(eval $(call gb_Library_Library,basebmp))

$(eval $(call gb_Library_add_package_headers,basebmp,basebmp_inc))

$(eval $(call gb_Library_set_include,basebmp,\
	-I$(SRCDIR)/basebmp/inc/ \
	-I$(SRCDIR)/basebmp/inc/pch \
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_defs,basebmp,\
	-DBASEBMP_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,basebmp,\
    sal \
    basegfx \
    stl \
    $(gb_STDLIBS) \
))

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_cxxobjects,basebmp, \
	basebmp/source/bitmapdevice \
	basebmp/source/debug \
	basebmp/source/polypolygonrenderer \
    , $(gb_COMPILEROPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) -xalias_level=compatible \
))
else
$(eval $(call gb_Library_add_exception_objects,basebmp,\
	basebmp/source/bitmapdevice \
	basebmp/source/debug \
	basebmp/source/polypolygonrenderer \
))
endif

# vim: set noet sw=4 ts=4:
