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

#ifeq ($(OS),UNX)
$(eval $(call gb_Library_Library,qstart))

$(eval $(call gb_Library_set_include,qstart,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sfx2/inc \
    -I$(SRCDIR)/sfx2/inc/sfx2 \
    -I$(SRCDIR)/sfx2/inc/pch \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
))

#WTF: this nice code is currently found in the makefile.mk (unix part only)
#	PKGCONFIG_MODULES=gtk+-2.0
#    .IF "$(PKGCONFIG_ROOT)"!=""
#    PKG_CONFIG=$(PKGCONFIG_ROOT)/bin/pkg-config
#    PKG_CONFIG_PATH:=$(PKGCONFIG_ROOT)/lib/pkgconfig
#    .EXPORT : PKG_CONFIG_PATH
#    PKGCONFIG_PREFIX=--define-variable=prefix=$(PKGCONFIG_ROOT)
#    .ELSE
#    PKG_CONFIG*=pkg-config
#    .ENDIF
#    PKGCONFIG_CFLAGS:=$(shell @$(PKG_CONFIG) $(PKGCONFIG_PREFIX) --cflags $(PKGCONFIG_MODULES))
#    PKGCONFIG_LIBS:=$(shell @$(PKG_CONFIG) $(PKGCONFIG_PREFIX) --libs $(PKGCONFIG_MODULES))
#    CFLAGS+=$(PKGCONFIG_CFLAGS

ifeq ($(ENABLE_SYSTRAY_GTK),TRUE)
$(eval $(call gb_Library_set_defs,qstart,\
    $$(DEFS) \
    -DDLL_NAME=libsfx$(DLLPOSTFIX)$(DLLPOST) \
    -DENABLE_QUICKSTART_APPLET \
))
endif

#todo: add libs
$(eval $(call gb_Library_add_linked_libs,qstart,\
    sfx \
))

$(eval $(call gb_Library_add_linked_system_libs,qstart,\
    icuuc \
    dl \
    m \
    pthread \
))

ifeq ($(ENABLE_SYSTRAY_GTK),TRUE)
$(eval $(call gb_Library_add_exception_objects,qstart,\
    sfx2/source/appl/shutdowniconunx.ob \
))
endif

endif
