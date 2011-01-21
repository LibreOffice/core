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

$(eval $(call gb_Executable_Executable,so_checksum))

$(eval $(call gb_Executable_set_include,so_checksum,\
    $$(INCLUDE) \
    -I$(SRCDIR)/tools/inc/ \
    -I$(SRCDIR)/tools/inc/pch \
    -I$(SRCDIR)/tools/bootstrp/ \
))

$(eval $(call gb_Executable_set_cxxflags,so_checksum,\
    $$(CXXFLAGS) \
    -D_TOOLS_STRINGLIST \
))

$(eval $(call gb_Executable_add_linked_libs,so_checksum,\
    sal \
    tl \
))
# used to link against basegfxlx comphelp4gcc3 i18nisolang1gcc3 ucbhelper4gcc3 uno_cppu uno_cppuhelpergcc3 uno_salhelpergcc3 vos3gcc3 - seems to be superficial

$(eval $(call gb_Executable_add_exception_objects,so_checksum,\
    tools/bootstrp/md5 \
    tools/bootstrp/so_checksum \
))

ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
$(eval $(call gb_Executable_add_linked_libs,so_checksum,\
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
$(eval $(call gb_Executable_add_linked_libs,so_checksum,\
    kernel32 \
    msvcrt \
    oldnames \
    user32 \
    uwinapi \
))
endif
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_linked_libs,so_checksum,\
    dl \
    pthread \
))
endif
# vim: set noet sw=4 ts=4:
