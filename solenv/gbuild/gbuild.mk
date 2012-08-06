# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

GBUILDDIR:=$(SRCDIR)/solenv/gbuild

# vars needed from the env/calling makefile

# DEBUG
# INPATH
# JAVA_HOME
# LIBXML_CFLAGS
# OS
# PRODUCT
# SOLARINC
# SOLARLIB
# UPD

# GXX_INCLUDE_PATH (Linux)
# PTHREAD_CFLAGS (Linux)
# SYSTEM_ICU (Linux)
# SYSTEM_JPEG (Linux)
# SYSTEM_LIBXML (Linux)

.DELETE_ON_ERROR:

SHELL := /bin/sh
true := T
false :=
define NEWLINE


endef

define WHITESPACE
 
endef

COMMA :=,

# optional extensions that should never be essential
ifneq ($(wildcard $(GBUILDDIR)/extensions/pre_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/pre_*.mk)
endif

include $(GBUILDDIR)/Output.mk


ifneq ($(strip $(PRODUCT)$(product)),)
gb_PRODUCT := $(true)
else
gb_PRODUCT := $(false)
endif

ifneq ($(strip $(ENABLE_SYMBOLS)$(enable_symbols)),)
gb_SYMBOL := $(true)
else
gb_SYMBOL := $(false)
endif

gb_TIMELOG := 0
ifneq ($(strip $(TIMELOG)$(timelog)),)
gb_TIMELOG := 1
endif

gb_DEBUGLEVEL := 0
ifneq ($(strip $(DEBUG)),)
gb_DEBUGLEVEL := 1
ifeq ($(origin DEBUG),command line)
ENABLE_DEBUG_FOR := all
endif
endif
ifneq ($(strip $(debug)),)
gb_DEBUGLEVEL := 1
ifeq ($(origin debug),command line)
ENABLE_DEBUG_FOR := all
endif
endif
ifeq ($(gb_PRODUCT),$(false))
gb_DEBUGLEVEL := 1
endif

ifneq ($(strip $(DBGLEVEL)),)
gb_DEBUGLEVEL := $(strip $(DBGLEVEL))
ifeq ($(origin DBGLEVEL),command line)
ENABLE_DEBUG_FOR := all
endif
endif
ifneq ($(strip $(dbglevel)),)
gb_DEBUGLEVEL := $(strip $(dbglevel))
ifeq ($(origin dbglevel),command line)
ENABLE_DEBUG_FOR := all
endif
endif

ifneq ($(gb_DEBUGLEVEL),0)
gb_SYMBOL := $(true)
endif

ifneq ($(nodep),)
gb_FULLDEPS := $(false)
else
# for clean, setuplocal and removelocal goals we switch off dependencies
ifneq ($(filter clean setuplocal removelocal showdeliverables help debugrun,$(MAKECMDGOALS)),)
gb_FULLDEPS := $(false)
else
gb_FULLDEPS := $(true)
endif
endif

# save user-supplied flags for latter use
# TODO remove after the old build system is abolished
ifneq ($(strip $(ENVCFLAGS)),)
gb__ENV_CFLAGS := $(ENVCFLAGS)
endif
# TODO remove after the old build system is abolished
ifneq ($(strip $(ENVCFLAGSCXX)),)
gb__ENV_CXXFLAGS := $(ENVCFLAGSCXX)
endif

include $(GBUILDDIR)/Helper.mk
include $(GBUILDDIR)/TargetLocations.mk

$(eval $(call gb_Helper_init_registries))
$(eval $(call gb_Helper_add_repositories,$(SRCDIR)))
$(eval $(call gb_Helper_collect_libtargets))

gb_Library_DLLPOSTFIX := lo

# Include platform/cpu/compiler specific config/definitions
include $(GBUILDDIR)/platform/$(OS)_$(CPUNAME)_$(COM).mk

ifeq ($(CROSS_COMPILING),YES)
# We can safely Assume all cross-compilation is from Unix systems.
gb_Executable_EXT_for_build :=
else
gb_Executable_EXT_for_build := $(gb_Executable_EXT)
endif

include $(GBUILDDIR)/Tempfile.mk

include $(SRCDIR)/RepositoryFixes.mk

$(eval $(call gb_Helper_collect_knownlibs))

# add user-supplied flags
ifneq ($(strip gb__ENV_CFLAGS),)
gb_LinkTarget_CFLAGS += $(gb__ENV_CFLAGS)
endif
ifneq ($(strip gb__ENV_CXXFLAGS),)
gb_LinkTarget_CXXFLAGS += $(gb__ENV_CXXFLAGS)
endif

gb_CPUDEFS += -D$(CPUNAME)

gb_GLOBALDEFS := \
	-D_REENTRANT \
	-DOSL_DEBUG_LEVEL=$(gb_DEBUGLEVEL) \
	-DSUPD=$(UPD) \
	$(gb_OSDEFS) \
	$(gb_COMPILERDEFS) \
	$(gb_CPUDEFS) \


ifeq ($(gb_PRODUCT),$(false))
gb_GLOBALDEFS += \
	-DDBG_UTIL \

ifneq ($(COM),MSC)
gb_GLOBALDEFS += \
	-D_DEBUG \

endif
endif

ifneq ($(strip $(SOLAR_JAVA)),)
gb_GLOBALDEFS += -DSOLAR_JAVA
endif

ifeq ($(gb_TIMELOG),1)
gb_GLOBALDEFS += \
	-DTIMELOG \

endif

ifeq ($(gb_DEBUGLEVEL),0)
gb_GLOBALDEFS += \
	-DOPTIMIZE \
	-DNDEBUG \

else
gb_GLOBALDEFS += \
    -DSAL_LOG_INFO \
    -DSAL_LOG_WARN \

ifneq ($(gb_DEBUGLEVEL),1) # 2 or more
gb_GLOBALDEFS += \
	-DDEBUG \

endif
endif

ifneq ($(strip $(ENABLE_GTK)),)
gb_GLOBALDEFS += -DENABLE_GTK
endif

ifneq ($(strip $(ENABLE_TDE)),)
gb_GLOBALDEFS += -DENABLE_TDE
endif

ifneq ($(strip $(ENABLE_KDE)),)
gb_GLOBALDEFS += -DENABLE_KDE
endif

ifneq ($(strip $(ENABLE_KDE4)),)
gb_GLOBALDEFS += -DENABLE_KDE4
endif

ifeq ($(strip $(ENABLE_GRAPHITE)),TRUE)
gb_GLOBALDEFS += -DENABLE_GRAPHITE
endif

ifeq ($(strip $(ENABLE_HEADLESS)),TRUE)
gb_GLOBALDEFS += -DLIBO_HEADLESS
endif

ifeq ($(strip $(DISABLE_DBCONNECTIVITY)),TRUE)
gb_GLOBALDEFS += -DDISABLE_DBCONNECTIVITY
endif

ifeq ($(strip $(DISABLE_EXTENSIONS)),TRUE)
gb_GLOBALDEFS += -DDISABLE_EXTENSIONS
endif

ifeq ($(strip $(DISABLE_SCRIPTING)),TRUE)
gb_GLOBALDEFS += -DDISABLE_SCRIPTING
endif

ifeq ($(strip $(DISABLE_DYNLOADING)),TRUE)
gb_GLOBALDEFS += -DDISABLE_DYNLOADING
endif

ifeq ($(HAVE_THREADSAFE_STATICS),TRUE)
gb_GLOBALDEFS += -DHAVE_THREADSAFE_STATICS
endif

gb_GLOBALDEFS := $(sort $(gb_GLOBALDEFS))

include $(GBUILDDIR)/Deliver.mk

$(eval $(call gb_Deliver_init))

include $(SOLARENV)/inc/minor.mk

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
	ComponentsTarget \
	AllLangResTarget \
	WinResTarget \
	LinkTarget \
	Library \
	StaticLibrary \
	Executable \
	SdiTarget \
	Package \
	CustomTarget \
	ExternalLib \
	Pagein \
	Pyuno \
	Rdb \
	CppunitTest \
	Jar \
	JavaClassSet \
	JunitTest \
	Module \
	UnoApiTarget \
	UnoApi \
	UnoApiMerge \
	InternalUnoApi \
	Zip \
	Configuration \
	Extension \
	InstallModuleTarget \
	InstallModule \
	InstallScript \
,$(GBUILDDIR)/$(class).mk)

# optional extensions that should never be essential
ifneq ($(wildcard $(GBUILDDIR)/extensions/post_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/post_*.mk)
endif

define gb_Extensions_final_hook
ifneq ($(wildcard $(GBUILDDIR)/extensions/final_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/final_*.mk)
endif

endef


ifeq ($(SYSTEM_LIBXSLT_FOR_BUILD),YES)
gb_XSLTPROCTARGET :=
gb_XSLTPROC := xsltproc
else
gb_XSLTPROCTARGET := $(call gb_Executable_get_target_for_build,xsltproc)
gb_XSLTPROC := $(gb_Helper_set_ld_path) $(gb_XSLTPROCTARGET)
endif

ifeq ($(SYSTEM_LIBXML_FOR_BUILD),YES)
gb_XMLLINTTARGET :=
gb_XMLLINT := xsltproc
else
gb_XMLLINTTARGET := $(call gb_Executable_get_target_for_build,xsltproc)
gb_XMLLINT := $(gb_Helper_set_ld_path) $(gb_XMLLINTTARGET)
endif

ifeq ($(SYSTEM_PYTHON),YES)
gb_PYTHONTARGET :=
gb_PYTHON := $(PYTHON)
else ifeq ($(OS),MACOSX)
#fixme: remove this MACOSX ifeq branch by filling in gb_PYTHON_PRECOMMAND in
#gbuild/platform/macosx.mk correctly for mac, e.g. PYTHONPATH and PYTHONHOME
#dirs for in-tree internal python
gb_PYTHONTARGET :=
gb_PYTHON := $(PYTHON)
else ifeq ($(DISABLE_PYTHON),TRUE)
# Build-time python
gb_PYTHON := python
else
gb_PYTHONTARGET := $(call gb_Executable_get_target_for_build,python)
gb_PYTHON := $(gb_PYTHON_PRECOMMAND) $(gb_PYTHONTARGET)
endif

.PHONY: help
help:
	@cat $(SRCDIR)/solenv/gbuild/gbuild.help.txt
	@true

# vim: set noet sw=4:
