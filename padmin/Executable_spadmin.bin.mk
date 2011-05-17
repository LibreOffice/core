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

$(eval $(call gb_Executable_Executable,spadmin.bin))

$(eval $(call gb_Executable_set_include,spadmin.bin,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Executable_set_cxxflags,spadmin.bin,\
    $$(CXXFLAGS) \
))

$(eval $(call gb_Executable_add_linked_static_libs,spadmin.bin,\
    vclmain \
))

$(eval $(call gb_Executable_add_linked_libs,spadmin.bin,\
    spa \
    svt \
    vcl \
    utl \
    ucbhelper \
    comphelper \
    tl \
    cppuhelper \
    cppu \
    sal \
))

$(eval $(call gb_Executable_add_exception_objects,spadmin.bin,\
    padmin/source/desktopcontext \
    padmin/source/pamain \
))

ifneq (,$(filter LINUX DRAGONFLY OPENBSD FREEBSD NETBSD, $(OS)))
$(eval $(call gb_Executable_add_linked_libs,spadmin.bin,\
    pthread \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_linked_libs,spadmin.bin,\
    dl \
))
endif

# vim: set noet sw=4 ts=4:
