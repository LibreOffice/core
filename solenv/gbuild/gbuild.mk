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

# vars needed from the env/calling makefile

# CVER
# DEBUG
# GBUILDDIR
# INPATH
# JAVA_HOME
# JDKINCS
# LIBXML_CFLAGS
# OS
# PRODUCT
# SOLARINC
# SOLARLIB
# STLPORT_VER
# UPD

# GXX_INCLUDE_PATH (Linux)
# PTHREAD_CFLAGS (Linux)
# SYSTEM_ICU (Linux)
# SYSTEM_JPEG (Linux)
# SYSTEM_LIBXML (Linux)
# USE_SYSTEM_STL (Linux)

SHELL := /bin/sh
true := T
false :=

include $(GBUILDDIR)/BuildDirs.mk

ifneq ($(strip $(PRODUCT)$(product)),)
gb_PRODUCT := $(true)
else
ifneq ($(strip $(product)),)
gb_PRODUCT := $(true)
else
gb_PRODUCT := $(false)
endif
endif

ifneq ($(strip $(DEBUG)),)
gb_DEBUGLEVEL := 2
else
ifeq ($(gb_PRODUCT),$(true))
gb_DEBUGLEVEL := 0
else
gb_DEBUGLEVEL := 1
endif
endif

ifneq ($(strip $(ENABLE_PCH)),)
gb_ENABLE_PCH := $(true)
else
gb_ENABLE_PCH := $(false)
endif

# for clean, uninstall and setuplocal goals we switch of dependencies
ifneq ($(filter clean uninstall setuplocal,$(MAKECMDGOALS)),)
gb_FULLDEPS := $(false)
else
gb_FULLDEPS := $(true)
endif

include $(GBUILDDIR)/Helper.mk
gb_Library_NAMESCHEMES := OOO PLAIN RT RTVER STL UNO UNOVER
gb_StaticLibrary_NAMESCHEMES := PLAIN
include $(foreach repo,$(gb_REPOS),$(repo)/Targetnames.mk)
gb_Library_PLAINLIBS := \
    $(gb_Library_PLAINLIBS_URE) \
    $(gb_Library_PLAINLIBS_OOO) \

gb_Library_UNOLIBS := \
    $(gb_Library_UNOLIBS_URE) \
    $(gb_Library_UNOLIBS_OOO) \

gb_Library_TARGETS := $(foreach namescheme,$(gb_Library_NAMESCHEMES),$(gb_Library_$(namescheme)LIBS))
gb_StaticLibrary_TARGETS := $(foreach namescheme,$(gb_StaticLibrary_NAMESCHEMES),$(gb_StaticLibrary_$(namescheme)LIBS))

ifeq ($(OS),LINUX)
include $(GBUILDDIR)/platform/linux.mk
else
ifeq ($(OS),WNT)
include $(GBUILDDIR)/platform/windows.mk
else
ifeq ($(OS),SOLARIS)
include $(GBUILDDIR)/platform/solaris.mk
else
ifeq ($(OS),MACOSX)
include $(GBUILDDIR)/platform/macosx.mk
else
$(error unsupported OS: $(OS))
endif
endif
endif
endif

include $(foreach repo,$(gb_REPOS),$(repo)/Platformfixes.mk)
gb_Library_KNOWNLIBS := $(foreach namescheme,$(gb_Library_NAMESCHEMES),$(gb_Library_$(namescheme)LIBS))
gb_StaticLibrary_KNOWNLIBS := $(foreach namescheme,$(gb_StaticLibrary_NAMESCHEMES),$(gb_StaticLibrary_$(namescheme)LIBS))

ifeq ($(USE_SYSTEM_STL),YES)
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))
endif

ifeq ($(SYSTEM_LIBXML),YES)
gb_Library_TARGETS := $(filter-out xml2,$(gb_Library_TARGETS))
endif

ifeq ($(SYSTEM_ICU),YES)
gb_Library_TARGETS := $(filter-out icuuc,$(gb_Library_TARGETS))
endif

gb_GLOBALDEFS := \
    -D__DMAKE \
    -D_REENTRANT \
    -D_USE_NAMESPACE=1 \
    -DCUI \
    -DENABLE_LAYOUT_EXPERIMENTAL=0 \
    -DENABLE_LAYOUT=0 \
    -DNEW_SOLAR \
    -DOSL_DEBUG_LEVEL=$(gb_DEBUGLEVEL) \
    -DSOLAR_JAVA \
    -DSTLPORT_VERSION=$(STLPORT_VER) \
    -DSUPD=$(UPD) \
    -DVCL \
    $(gb_OSDEFS) \
    $(gb_COMPILERDEFS) \
    $(gb_CPUDEFS) \

ifeq ($(gb_PRODUCT),$(true))
gb_GLOBALDEFS += \
    -DPRODUCT \
    -DPRODUCT_FULL \

else
gb_GLOBALDEFS += \
    -DDBG_UTIL \
    -D_STLP_DEBUG \
    -D_DEBUG \

endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_GLOBALDEFS += \
    -DDEBUG \

else
gb_GLOBALDEFS += \
    -DOPTIMIZE \
    -DNDEBUG \

endif

gb_GLOBALDEFS := $(sort $(gb_GLOBALDEFS))

include $(GBUILDDIR)/TargetLocations.mk

# We are using a set of scopes that we might as well call classes.

# It is important to include them in the right order as that is
# -- at least in part -- defining precedence. This is not an issue in the
# WORKDIR as there are no nameing collisions there, but OUTDIR is a mess
# and precedence is important there. This is also platform dependant.
# For example:
# $(OUTDIR)/bin/% for executables collides
#	with $(OUTDIR)/bin/%.res for resources on unix
# $(OUTDIR)/lib/%.lib collides
#	on windows (static and dynamic libs)
# $(OUTDIR)/xml/% for packageparts collides
#	with $(OUTDIR)/xml/component/%.component for components
# This is less of an issue with GNU Make versions > 3.82 which matches for
# shortest stem instead of first match. However, upon intoduction this version
# is not available everywhere by default.

include $(foreach class, \
    ComponentTarget \
    AllLangResTarget \
    LinkTarget \
    Library \
    StaticLibrary \
    Executable \
    SdiTarget \
    Package \
    PrecompiledHeaders \
    Module \
,$(GBUILDDIR)/$(class).mk)

# vim: set noet sw=4 ts=4:
