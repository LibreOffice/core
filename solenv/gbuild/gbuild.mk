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
# JAVA_HOME
# LIBXML_CFLAGS
# OS
# SOLARINC
# UPD

# PTHREAD_CFLAGS (Linux)
# SYSTEM_ICU (Linux)
# SYSTEM_LIBJPEG (Linux)
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

true := T
false :=
gb_not = $(if $(1),$(false),$(true))

define NEWLINE


endef

define WHITESPACE
 
endef

COMMA :=,

OPEN_PAREN :=(
CLOSE_PAREN :=)

gb_SPACE:=$(gb_SPACE) $(gb_SPACE)

gb_VERBOSE := $(verbose)

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

ifneq ($(ENABLE_DBGUTIL),)
gb_ENABLE_DBGUTIL := $(true)
else
gb_ENABLE_DBGUTIL := $(false)
endif

gb_ENABLE_SYMBOLS_FOR := $(ENABLE_SYMBOLS_FOR)

# ENABLE_SYMBOLS (presumably from the command line)
ifneq ($(strip $(ENABLE_SYMBOLS)),)
gb_ENABLE_SYMBOLS_FOR := $(ENABLE_SYMBOLS)
endif
ifneq ($(strip $(enable_symbols)),)
gb_ENABLE_SYMBOLS_FOR := $(enable_symbols)
endif

# note: ENABLE_BREAKPAD turns on symbols
ifneq ($(strip $(ENABLE_BREAKPAD)),)
gb_ENABLE_SYMBOLS_FOR := all
endif

gb_DEBUGLEVEL := 0
ifneq ($(strip $(DEBUG)),)
gb_DEBUGLEVEL := 1
# make DEBUG=true should force -g
ifeq ($(origin DEBUG),command line)
gb_ENABLE_SYMBOLS_FOR := all
endif
endif
ifneq ($(strip $(debug)),)
gb_DEBUGLEVEL := 1
ifeq ($(origin debug),command line)
gb_ENABLE_SYMBOLS_FOR := all
endif
endif
ifeq ($(gb_ENABLE_DBGUTIL),$(true))
gb_DEBUGLEVEL := 1
endif

ifneq ($(strip $(DBGLEVEL)),)
gb_DEBUGLEVEL := $(strip $(DBGLEVEL))
ifeq ($(origin DBGLEVEL),command line)
gb_ENABLE_SYMBOLS_FOR := all
endif
endif
ifneq ($(strip $(dbglevel)),)
gb_DEBUGLEVEL := $(strip $(dbglevel))
ifeq ($(origin dbglevel),command line)
gb_ENABLE_SYMBOLS_FOR := all
endif
endif

# handle special cases
ifeq ($(gb_ENABLE_SYMBOLS_FOR),1)
gb_ENABLE_SYMBOLS_FOR := all
endif
ifeq ($(gb_ENABLE_SYMBOLS_FOR),0)
gb_ENABLE_SYMBOLS_FOR :=
endif
ifeq ($(gb_ENABLE_SYMBOLS_FOR),yes)
gb_ENABLE_SYMBOLS_FOR := all
endif
ifeq ($(gb_ENABLE_SYMBOLS_FOR),no)
gb_ENABLE_SYMBOLS_FOR :=
endif

ifeq ($(BLOCK_PCH),)
gb_ENABLE_PCH := $(ENABLE_PCH)
else
# Setting BLOCK_PCH effectively disables PCH, but the extra object file will be still linked in.
# This is useful for rebuilding only some files with PCH disabled, e.g. to check #include's,
# disabling the whole ENABLE_PCH would lead to unresolved symbols at link time.
gb_ENABLE_PCH :=
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
gb_RUN_CONFIGURE :=

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
	$(gb_OSDEFS) \
	$(gb_COMPILERDEFS) \
	$(gb_CPUDEFS) \

ifeq ($(ENABLE_WASM_STRIP),TRUE)
gb_GLOBALDEFS += -DENABLE_WASM_STRIP
endif

ifeq ($(gb_ENABLE_DBGUTIL),$(true))
gb_GLOBALDEFS += -DDBG_UTIL

ifneq ($(COM)-$(MSVC_USE_DEBUG_RUNTIME),MSC-)
gb_GLOBALDEFS += -D_DEBUG
endif
endif

ifeq ($(gb_TIMELOG),1)
gb_GLOBALDEFS += -DTIMELOG \

endif

ifeq ($(strip $(ASSERT_ALWAYS_ABORT)),FALSE)
gb_GLOBALDEFS += -DNDEBUG \

endif

ifeq ($(ENABLE_SAL_LOG),TRUE)
gb_GLOBALDEFS += -DSAL_LOG_INFO \
				 -DSAL_LOG_WARN \

endif

ifneq ($(gb_DEBUGLEVEL),0)
ifneq ($(gb_DEBUGLEVEL),1) # 2 or more
gb_GLOBALDEFS += -DDEBUG \

endif
endif

gb_GLOBALDEFS += \
	$(call gb_Helper_define_if_set,\
		DISABLE_DYNLOADING \
	)

gb_GLOBALDEFS := $(sort $(gb_GLOBALDEFS))

# Common environment variables passed into all gb_*Test classes:
# * Cap the number of threads unittests use:
gb_TEST_ENV_VARS := MAX_CONCURRENCY=4
# * Disable searching for certificates by default:
#
#  see xmlsecurity/source/xmlsec/nss/nssinitializer.cxx for use
#
#  a) If MOZILLA_CERTIFICATE_FOLDER is empty then LibreOffice autodetects
#  the user's mozilla-descended application profile. To disable that we
#  use a non-empty string here.
#
#  b) Using dbm: appears to nss as equivalent to an empty path so the
#  initial NSS_InitReadWrite will fail. In response to that failure
#  LibreOffice will create a temp fallback cert database which is removed
#  on process exit
gb_TEST_ENV_VARS += MOZILLA_CERTIFICATE_FOLDER=dbm:
# Avoid hanging if the cups daemon requests a password:
gb_TEST_ENV_VARS += SAL_DISABLE_SYNCHRONOUS_PRINTER_DETECTION=1
ifeq (,$(SAL_USE_VCLPLUGIN))
gb_TEST_ENV_VARS += SAL_USE_VCLPLUGIN=svp
endif
gb_TEST_ENV_VARS += STATIC_UNO_HOME=file://$$I/program

# This is used to detect whether LibreOffice is being built (as opposed to building
# 3rd-party code). Used for tag deprecation for API we want to
# ensure is not used at all externally while we clean
# out our internal usage, for code in sal/ that should be used only internally, etc.
gb_DEFS_INTERNAL := \
	-DLIBO_INTERNAL_ONLY \

include $(GBUILDDIR)/Deliver.mk

$(eval $(call gb_Deliver_init))

include $(GBUILDDIR)/Trace.mk

# We are using a set of scopes that we might as well call classes.

# TODO: to what extent is the following still true?
# It is important to include them in the right order as that is
# -- at least in part -- defining precedence. This is not an issue in the
# WORKDIR as there are no naming collisions there, but INSTDIR is a mess
# and precedence is important there. This is also platform dependent.
#
# This is less of an issue with GNU Make versions > 3.82 which matches for
# shortest stem instead of first match. However, upon introduction this version
# is not available everywhere by default.

include $(foreach class, \
	ComponentTarget \
	Postprocess \
	AllLangMoTarget \
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
	UITest \
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
	ExtensionPackage \
	Dictionary \
	InstallModuleTarget \
	InstallModule \
	InstallScript \
	AutoInstall \
	PackageSet \
	GeneratedPackage \
	CompilerTest \
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

# Setup for ccache.
ifneq ($(gb_ENABLE_PCH),)
# CCACHE_SLOPPINESS should contain pch_defines,time_macros for PCHs.
gb_CCACHE_SLOPPINESS :=
ifeq ($(shell test -z "$$CCACHE_SLOPPINESS" && echo 1),1)
gb_CCACHE_SLOPPINESS := CCACHE_SLOPPINESS=pch_defines,time_macros
else
ifeq ($(shell echo "$$CCACHE_SLOPPINESS" | grep -q pch_defines | grep -q time_macros && echo 1),1)
gb_CCACHE_SLOPPINESS := CCACHE_SLOPPINESS=$CCACHE_SLOPPINESS:pch_defines,time_macros
endif
endif
gb_COMPILER_SETUP += $(gb_CCACHE_SLOPPINESS)
endif

ifneq ($(CCACHE_DEPEND_MODE),)
gb_COMPILER_SETUP += CCACHE_DEPEND=1
endif

# vim: set noet sw=4:
