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

$(eval $(call gb_Library_Library,passwordcontainer))

$(eval $(call gb_Library_set_componentfile,passwordcontainer,svl/source/passwordcontainer/passwordcontainer))

$(eval $(call gb_Library_set_include,passwordcontainer,\
    $$(SOLARINC) \
    -I$(WORKDIR)/inc/svl \
    -I$(WORKDIR)/inc/ \
    -I$(SRCDIR)/svl/inc \
    -I$(SRCDIR)/svl/inc/svl \
    -I$(SRCDIR)/svl/source/inc \
    -I$(SRCDIR)/svl/inc/pch \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_linked_libs,passwordcontainer,\
    cppu \
    cppuhelper \
    sal \
    stl \
    ucbhelper \
    utl \
))

$(eval $(call gb_Library_add_linked_system_libs,passwordcontainer,\
    icuuc \
    dl \
    m \
    pthread \
))

$(eval $(call gb_Library_add_exception_objects,passwordcontainer,\
    svl/source/passwordcontainer/passwordcontainer \
    svl/source/passwordcontainer/syscreds \
))

ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
$(eval $(call gb_Library_add_linked_libs,passwordcontainer,\
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
$(eval $(call gb_Library_add_linked_libs,passwordcontainer,\
    kernel32 \
    msvcrt \
    oldnames \
    user32 \
    uwinapi \
))
endif
endif
# vim: set noet sw=4 ts=4:
