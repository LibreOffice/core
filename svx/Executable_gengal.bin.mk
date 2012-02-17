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

$(eval $(call gb_Executable_Executable,gengal.bin))

$(eval $(call gb_Executable_set_include,gengal.bin,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svx/inc/ \
    -I$(SRCDIR)/svx/inc/pch \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc/udkapi \
    -I$(OUTDIR)/inc/offapi \
))

$(eval $(call gb_Executable_add_cxxflags,gengal.bin,\
    $$(CXXFLAGS) \
))

$(eval $(call gb_Executable_add_linked_libs,gengal.bin,\
    sal \
    tl \
    svl \
    comphelper \
    cppu \
    cppuhelper \
    vcl \
    ucbhelper \
    svxcore \
))

$(eval $(call gb_Executable_add_exception_objects,gengal.bin,\
    svx/source/gengal/gengal \
))

$(eval $(call gb_Executable_add_linked_static_libs,gengal.bin,\
    vclmain \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_add_linked_libs,gengal.bin,\
    kernel32 \
    msvcrt \
    oldnames \
    user32 \
    uwinapi \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_linked_libs,gengal.bin,\
    dl \
    pthread \
))
endif

# vim: set noet sw=4 ts=4:
