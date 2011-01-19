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

$(eval $(call gb_Executable_Executable,rscdep))

$(eval $(call gb_Executable_set_include,rscdep,\
    $$(INCLUDE) \
    -I$(SRCDIR)/tools/inc/ \
    -I$(SRCDIR)/tools/inc/pch \
    -I$(SRCDIR)/tools/bootstrp/ \
))

$(eval $(call gb_Executable_set_cxxflags,rscdep,\
    $$(CXXFLAGS) \
    -D_TOOLS_STRINGLIST \
))

$(eval $(call gb_Executable_add_linked_libs,rscdep,\
    sal \
    stl \
    tl \
    vos3 \
))

$(eval $(call gb_Executable_add_exception_objects,rscdep,\
    tools/bootstrp/appdef \
    tools/bootstrp/cppdep \
    tools/bootstrp/inimgr \
    tools/bootstrp/prj \
    tools/bootstrp/rscdep \
))

ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
ifeq ($(HAVE_GETOPT),YES)
$(eval $(call gb_Executable_set_cxxflags,rscdep,\
    $$(CXXFLAGS) \
    -DHAVE_GETOPT \
))
endif
$(eval $(call gb_Executable_add_linked_libs,rscdep,\
    mingwthrd \
    $(gb_MINGW_LIBSTDCPP) \
    mingw32 \
    $(gb_MINGW_LIBGCC) \
    uwinapi \
    moldname \
    mingwex \
    kernel32 \
    msvcrt \
    user32 \
))
else
$(eval $(call gb_Executable_add_linked_libs,rscdep,\
    gnu_getopt \
))
$(eval $(call gb_Executable_add_linked_libs,rscdep,\
    kernel32 \
    msvcrt \
    oldnames \
    user32 \
    uwinapi \
))
endif
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_linked_libs,rscdep,\
    dl \
    pthread \
))
endif
# vim: set noet sw=4 ts=4:
