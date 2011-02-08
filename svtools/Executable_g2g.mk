#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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

$(eval $(call gb_Executable_Executable,g2g))

$(eval $(call gb_Executable_set_include,g2g,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/ \
    -I$(OUTDIR)/inc/offuh/ \
    -I$(SRCDIR)/svtools/inc/ \
    -I$(SRCDIR)/svtools/inc/pch/ \
    -I$(SRCDIR)/svtools/inc/svtools/ \
))

$(eval $(call gb_Executable_add_linked_libs,g2g,\
    jvmfwk \
    sal \
    stl \
    svt \
    tl \
    vcl \
    vos3 \
))

$(eval $(call gb_Executable_add_exception_objects,g2g,\
    svtools/bmpmaker/g2g \
))
ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
$(eval $(call gb_Executable_add_linked_libs,g2g,\
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
$(eval $(call gb_Executable_add_linked_libs,g2g,\
    kernel32 \
    msvcrt \
    oldnames \
    user32 \
    uwinapi \
))
endif
endif
ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_linked_libs,g2g,\
   pthread \
   dl \
   X11 \
))
endif
# vim: set noet sw=4 ts=4:
