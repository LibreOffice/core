# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

ifeq ($(OS),WNT)
svx_GENGALBIN := gengal
else
svx_GENGALBIN := gengal.bin
endif


$(eval $(call gb_Executable_Executable,$(svx_GENGALBIN)))

$(eval $(call gb_Executable_set_include,$(svx_GENGALBIN),\
    $$(INCLUDE) \
    -I$(SRCDIR)/svx/inc/ \
    -I$(SRCDIR)/svx/inc/pch \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc/udkapi \
    -I$(OUTDIR)/inc/offapi \
))

$(eval $(call gb_Executable_add_cxxflags,$(svx_GENGALBIN),\
    $$(CXXFLAGS) \
))

$(eval $(call gb_Executable_use_libraries,$(svx_GENGALBIN),\
    sal \
    tl \
    svl \
    comphelper \
    cppu \
    cppuhelper \
    vcl \
    svxcore \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_standard_system_libs,$(svx_GENGALBIN)))

$(eval $(call gb_Executable_add_exception_objects,$(svx_GENGALBIN),\
    svx/source/gengal/gengal \
))

$(eval $(call gb_Executable_use_static_libraries,$(svx_GENGALBIN),\
    vclmain \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_use_libraries,$(svx_GENGALBIN),\
    kernel32 \
    msvcrt \
    $(gb_Library_win32_OLDNAMES) \
    user32 \
    uwinapi \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_libs,$(svx_GENGALBIN),\
	-ldl \
	-lpthread \
))
endif

# vim: set noet sw=4 ts=4:
