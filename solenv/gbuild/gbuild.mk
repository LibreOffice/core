# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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

# PTHREAD_CFLAGS (Linux)
# SYSTEM_ICU (Linux)
# SYSTEM_JPEG (Linux)
# SYSTEM_LIBXML (Linux)

.DELETE_ON_ERROR:

# do not use built-in rules
# DO NOT TOUCH THIS LINE UNLESS YOU REALLY KNOW WHAT YOU ARE DOING
# REMOVING THIS MAKES e.g. MODULE SW ALONE SLOWER BY SOME 300%
# FOR TAIL_BUILD THE IMPACT IS HUGE!
# (unless you are doing make -r, which we should explicitly NOT require from
# users)
MAKEFLAGS += r
.SUFFIXES:

# by default gbuild use /bin/sh
# if you want to use a particular shell
# you can export gb_SHELL=<path_to_shell>
#

ifdef gb_SHELL
SHELL := $(gb_SHELL)
else
SHELL := /bin/sh
endif

true := T
false :=
define NEWLINE


endef

define WHITESPACE
 
endef

COMMA :=,

CLOSE_PAREN :=)

include $(GBUILDDIR)/Helper.mk

# optional extensions that should never be essential
ifneq ($(wildcard $(GBUILDDIR)/extensions/pre_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/pre_*.mk)
endif

include $(GBUILDDIR)/Output.mk

gb_TIMELOG := 0
ifneq ($(strip $(TIMELOG)$(timelog)),)
gb_TIMELOG := 1
endif

# This used to be PRODUCT="" (for the same meaning as ENABLE_DBGUTIL="TRUE"),
# but the product meaning is now only confusing.
ifneq ($(ENABLE_DBGUTIL),)
gb_ENABLE_DBGUTIL := $(true)
else
gb_ENABLE_DBGUTIL := $(false)
endif

gb_DEBUGLEVEL := 0
ifneq ($(strip $(DEBUG)),)
gb_DEBUGLEVEL := 1
# make DEBUG=true should force -g
ifeq ($(origin DEBUG),command line)
ENABLE_DEBUGINFO_FOR := all
endif
endif
ifneq ($(strip $(debug)),)
gb_DEBUGLEVEL := 1
ifeq ($(origin debug),command line)
ENABLE_DEBUGINFO_FOR := all
endif
endif
ifeq ($(gb_ENABLE_DBGUTIL),$(true))
gb_DEBUGLEVEL := 1
endif

ifneq ($(strip $(DBGLEVEL)),)
gb_DEBUGLEVEL := $(strip $(DBGLEVEL))
ifeq ($(origin DBGLEVEL),command line)
ENABLE_DEBUGINFO_FOR := all
endif
endif
ifneq ($(strip $(dbglevel)),)
gb_DEBUGLEVEL := $(strip $(dbglevel))
ifeq ($(origin dbglevel),command line)
ENABLE_DEBUGINFO_FOR := all
endif
endif

ifeq ($(HARDLINKDELIVER),TRUE)
gb_Deliver_HARDLINK := $(true)
endif

ifeq ($(or $(ENABLE_SYMBOLS),$(enable_symbols)),FALSE)
gb_SYMBOL := $(false)
else
ifneq ($(strip $(ENABLE_SYMBOLS)$(enable_symbols)),)
gb_SYMBOL := $(true)
endif
endif

ifneq ($(strip $(ENABLE_PCH)),)
gb_ENABLE_PCH := $(true)
else
gb_ENABLE_PCH := $(false)
endif

ifneq ($(nodep)$(ENABLE_PRINT_DEPS),)
gb_FULLDEPS := $(false)
else
gb_FULLDEPS := $(true)
endif

ifneq ($(strip $(patches)$(PATCHES)),)
gb_KEEP_PRISTINE := $(true)
else
gb_KEEP_PRISTINE := $(false)
endif

ifeq ($(findstring s,$(MAKEFLAGS)),)
gb_QUIET_EXTERNAL := $(false)
else
gb_QUIET_EXTERNAL := $(true)
endif

# save user-supplied flags for latter use
ifneq ($(strip $(ENVCFLAGS)),)
gb__ENV_CFLAGS := $(ENVCFLAGS)
endif
ifneq ($(strip $(ENVCFLAGSCXX)),)
gb__ENV_CXXFLAGS := $(ENVCFLAGSCXX)
endif

include $(GBUILDDIR)/ExternalExecutable.mk
include $(GBUILDDIR)/TargetLocations.mk
include $(GBUILDDIR)/Tempfile.mk

$(eval $(call gb_Helper_init_registries))
include $(SRCDIR)/Repository.mk
include $(SRCDIR)/RepositoryExternal.mk
$(eval $(call gb_Helper_collect_knownlibs))

gb_Library_DLLPOSTFIX := lo

# Include platform/cpu/compiler specific config/definitions
include $(GBUILDDIR)/platform/$(OS)_$(CPUNAME)_$(COM).mk

# this is optional
include $(SRCDIR)/RepositoryFixes.mk

# after platform; at least currently python depends on variable set in platform
$(eval $(call gb_ExternalExecutable_collect_registrations))

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

# This is used to detect whether LibreOffice is being built (as opposed to building
# 3rd-party code). Used for tag deprecation for API we want to
# ensure is not used at all externally while we clean
# out our internal usage, for code in sal/ that should be used only internally, etc.
gb_GLOBALDEFS += \
	-DLIBO_INTERNAL_ONLY \

ifeq ($(gb_ENABLE_DBGUTIL),$(true))
gb_GLOBALDEFS += -DDBG_UTIL

ifneq ($(COM)-$(MSVC_USE_DEBUG_RUNTIME),MSC-)
gb_GLOBALDEFS += -D_DEBUG
endif
endif

ifneq ($(strip $(SOLAR_JAVA)),)
gb_GLOBALDEFS += -DSOLAR_JAVA
endif

ifeq ($(gb_TIMELOG),1)
gb_GLOBALDEFS += -DTIMELOG \

endif

ifeq ($(gb_DEBUGLEVEL),0)
gb_GLOBALDEFS += -DOPTIMIZE \

ifeq ($(strip $(ASSERT_ALWAYS_ABORT)),FALSE)
gb_GLOBALDEFS += -DNDEBUG \

endif

else
gb_GLOBALDEFS += -DSAL_LOG_INFO \
				 -DSAL_LOG_WARN \

ifneq ($(gb_DEBUGLEVEL),1) # 2 or more
gb_GLOBALDEFS += -DDEBUG \

endif
endif

ifeq ($(ENABLE_HEADLESS),TRUE)
gb_GLOBALDEFS += -DLIBO_HEADLESS \

endif

gb_GLOBALDEFS += \
	$(call gb_Helper_define_if_set,\
		DISABLE_DBCONNECTIVITY \
		DISABLE_DYNLOADING \
		DISABLE_EXPORT \
		DISABLE_SCRIPTING \
		ENABLE_READONLY_INSTALLSET \
	)

gb_GLOBALDEFS := $(sort $(gb_GLOBALDEFS))

ifeq ($(origin gb_RUNNABLE_INSTDIR),undefined)
gb_RUNNABLE_INSTDIR :=
endif

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
	Postprocess \
	AllLangResTarget \
	WinResTarget \
	LinkTarget \
	Library \
	StaticLibrary \
	Executable \
	SdiTarget \
	Package \
	ExternalPackage \
	CustomTarget \
	ExternalProject \
	Gallery \
	Pagein \
	PrecompiledHeaders \
	Pyuno \
	PythonTest \
	Rdb \
	CppunitTest \
	Jar \
	JavaClassSet \
	JunitTest \
	Module \
	UIConfig \
	UnoApiTarget \
	UnoApi \
	UnpackedTarball \
	InternalUnoApi \
	CliAssembly \
	CliLibrary \
	CliNativeLibrary \
	CliUnoApi \
	Zip \
	AllLangPackage \
	Configuration \
	HelpTarget \
	AllLangHelp \
	Extension \
	Dictionary \
	InstallModuleTarget \
	InstallModule \
	InstallScript \
	AutoInstall \
	PackageSet \
	GeneratedPackage \
,$(GBUILDDIR)/$(class).mk)

$(eval $(call gb_Helper_process_executable_registrations))
$(eval $(call gb_Postprocess_make_targets))

# optional extensions that should never be essential
ifneq ($(wildcard $(GBUILDDIR)/extensions/post_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/post_*.mk)
endif

define gb_Extensions_final_hook
ifneq ($(wildcard $(GBUILDDIR)/extensions/final_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/final_*.mk)
endif

endef

# vim: set noet sw=4:
