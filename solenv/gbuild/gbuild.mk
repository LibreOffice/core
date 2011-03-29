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
# UPD

# GXX_INCLUDE_PATH (Linux)
# PTHREAD_CFLAGS (Linux)
# SYSTEM_ICU (Linux)
# SYSTEM_JPEG (Linux)
# SYSTEM_LIBXML (Linux)

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

# BuildDirs uses the Output functions already
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

ifneq ($(strip $(ENABLE_SYMBOLS)$(enable_symbols)),)
gb_SYMBOL := $(true)
else
gb_SYMBOL := $(false)
endif


ifneq ($(strip $(DEBUG)$(debug)),)
gb_DEBUGLEVEL := 2
gb_SYMBOL := $(true)
else
ifeq ($(gb_PRODUCT),$(true))
gb_DEBUGLEVEL := 0
else
gb_DEBUGLEVEL := 1
endif
endif

ifneq ($(strip $(DBGLEVEL)$(dbglevel)),)
# FIXME: problem if both set
gb_DEBUGLEVEL := $(strip $(DBGLEVEL)$(dbglevel))
gb_SYMBOL := $(true)
endif

ifneq ($(strip $(ENABLE_PCH)),)
gb_ENABLE_PCH := $(true)
else
gb_ENABLE_PCH := $(false)
endif

# for clean, setuplocal and removelocal goals we switch off dependencies
ifneq ($(filter cleanpackmodule clean setuplocal removelocal showdeliverables help,$(MAKECMDGOALS)),)
gb_FULLDEPS := $(false)
else
gb_FULLDEPS := $(true)
endif

# save user-supplied flags for latter use
ifneq ($(strip $(CFLAGS)),)
gb__ENV_CFLAGS := $(CFLAGS)
else
# TODO remove after the old build system is abolished
ifneq ($(strip $(ENVCFLAGS)),)
gb__ENV_CFLAGS := $(ENVCFLAGS)
endif
endif
ifneq ($(strip $(CXXFLAGS)),)
gb__ENV_CXXFLAGS := $(CFLAGS)
else
# TODO remove after the old build system is abolished
ifneq ($(strip $(ENVCFLAGSCXX)),)
gb__ENV_CXXFLAGS := $(ENVCFLAGSCXX)
endif
endif

include $(GBUILDDIR)/Helper.mk
include $(GBUILDDIR)/TargetLocations.mk

$(eval $(call gb_Helper_init_registries))
$(eval $(call gb_Helper_add_repositories,$(gb_REPOS)))
$(eval $(call gb_Helper_collect_libtargets))

ifeq ($(OS),LINUX)
include $(GBUILDDIR)/platform/linux-$(CPUNAME).mk
else ifeq ($(OS),MACOSX)
include $(GBUILDDIR)/platform/macosx.mk
else ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
include $(GBUILDDIR)/platform/winmingw.mk
else
include $(GBUILDDIR)/platform/windows.mk
endif
else ifeq ($(OS),OPENBSD)
include $(GBUILDDIR)/platform/openbsd-$(CPUNAME).mk
else ifeq ($(OS),FREEBSD)
include $(GBUILDDIR)/platform/freebsd-$(CPUNAME).mk
else ifeq ($(OS),NETBSD)
include $(GBUILDDIR)/platform/netbsd-$(CPUNAME).mk
else ifeq ($(OS),DRAGONFLY)
include $(GBUILDDIR)/platform/dragonfly.mk
else ifeq ($(OS),SOLARIS)
include $(GBUILDDIR)/platform/solaris.mk
else
$(eval $(call gb_Output_error,Unsupported OS: $(OS)))
endif

include $(GBUILDDIR)/Tempfile.mk

include $(foreach repo,$(gb_REPOS),$(repo)/RepositoryFixes.mk)

$(eval $(call gb_Helper_collect_knownlibs))

# add user-supplied flags
ifneq ($(strip gb__ENV_CFLAGS),)
gb_LinkTarget_CFLAGS += $(gb__ENV_CFLAGS)
endif
ifneq ($(strip gb__ENV_CXXFLAGS),)
gb_LinkTarget_CXXFLAGS += $(gb__ENV_CXXFLAGS)
endif

gb_GLOBALDEFS := \
	-D_REENTRANT \
	-DCUI \
	-DENABLE_LAYOUT_EXPERIMENTAL=0 \
	-DENABLE_LAYOUT=0 \
	-DOSL_DEBUG_LEVEL=$(gb_DEBUGLEVEL) \
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
	-D_DEBUG \

endif

ifneq ($(strip $(SOLAR_JAVA)),)
gb_GLOBALDEFS += -DSOLAR_JAVA
endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_GLOBALDEFS += \
	-DDEBUG \

else
gb_GLOBALDEFS += \
	-DOPTIMIZE \
	-DNDEBUG \

endif

ifneq ($(strip $(ENABLE_GTK)),)
gb_GLOBALDEFS += -DENABLE_GTK
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

gb_GLOBALDEFS := $(sort $(gb_GLOBALDEFS))

include $(GBUILDDIR)/Deliver.mk

$(eval $(call gb_Deliver_init))

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
	CustomTarget \
	PrecompiledHeaders \
	CppunitTest \
	JavaClassSet \
	JunitTest \
	Module \
,$(GBUILDDIR)/$(class).mk)

# optional extensions that should never be essential
ifneq ($(wildcard $(GBUILDDIR)/extensions/post_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/post_*.mk)
endif

ifeq ($(SYSTEM_LIBXSLT),YES)
gb_XSLTPROCTARGET :=
gb_XSLTPROC := xsltproc
else
gb_XSLTPROCTARGET := $(call gb_Executable_get_target,xsltproc)
gb_XSLTPROC := $(gb_XSLTPROCPRECOMMAND) $(gb_XSLTPROCTARGET)
endif

export gb_AWK
export gb_XSLTPROC

define gb_HelpMessage
NAME
       gbuild - GNU make based build system for LibreOffice

SYNOPSIS
       make [ -f makefile ] [ options ] [ variable=value ... ] [ targets ] ...

IMPORTANT OPTIONS
       -r Eliminate use of the built-in implicit rules. Improves performance, please use always.
       -s Silent operation; do not print the commands as they are executed.

       -n Print the commands that would be executed, but do not execute them.
       -k Continue as much as possible after an error.

       -j Specifies the number of jobs (commands) to run simultaneously.
       -l Specifies that no new jobs (commands) should be started if there are others jobs running and the load average is at least load.

       -t Touch files (mark them up to date without really changing them) instead of running their commands.
       -W Pretend that the target file has just been modified.
       -o Do not remake the file file even if it is older than its dependencies, and do not remake anything on account of changes in file.

       -p Print the data base (rules and variable values) that results from reading the makefiles.
       --debug=b debug make run, see GNU make man page for details
       (descriptions from GNU make man page)

AVAILABLE TARGETS
       allandcheck     build product and run unit tests (default goal)
       all             build product
       check           run unit tests
       subsequentcheck run system tests (requires full installation)
       clean           remove all generated files

INTERACTIVE VARIABLES:
       DEBUG           If not empty, build with OSL_DEBUG_LEVEL=2 and symbols.
       debug
       ENABLE_SYMBOLS  If not empty, build with debug symbols. Automatically enabled by DEBUG/debug.
       enable_symbols
       ENABLE_PCH      If not empty, use precompiled headers (Windows only).
       CFLAGS          Add as compiler flags for plain c compilation.
       CXXFLAGSX       Add as compiler flags for c++ compilation.
       gb_FULLDEPS     Generate and use dependencies (on by default, handle with care).
       gb_COLOR        Use ASCII color output.
       gb_TITLES       Show progress in terminal title.

endef

.PHONY: help
help :
	$(info $(gb_HelpMessage))
	@true

# vim: set noet sw=4:
