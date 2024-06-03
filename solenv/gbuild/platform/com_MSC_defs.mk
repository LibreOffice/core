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

# please make generic Windows modifications to windows.mk
include $(GBUILDDIR)/platform/windows.mk

gb_CC := cl
gb_CXX := cl
gb_LINK := link
gb_DUMPBIN := dumpbin
gb_AWK := awk
gb_CLASSPATHSEP := ;
gb_RC := rc

# use CC/CXX if they are nondefaults
ifneq ($(origin CC),default)
gb_CC := $(CC)
gb_GCCP := $(CC)
endif
ifneq ($(origin CXX),default)
gb_CXX := $(CXX)
endif

# _SILENCE_CXX23_DENORM_DEPRECATION_WARNING is needed at least with Boost 1.82.0 using
# std::numeric_limits::has_denorm in
# workdir/UnpackedTarball/boost/boost/spirit/home/classic/core/primitives/impl/numerics.ipp, in turn
# included from boost/spirit/include/classic_core.hpp as included from various of our code files:

# _SCL_SECURE_NO_WARNINGS avoids deprecation warnings for STL algorithms
# like std::copy, std::transform (when MSVC_USE_DEBUG_RUNTIME is enabled)

gb_COMPILERDEFS := \
	-DBOOST_OPTIONAL_USE_OLD_DEFINITION_OF_NONE \
	-DBOOST_SYSTEM_NO_DEPRECATED \
	-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING \
	-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING \
	-D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING \
	-D_SILENCE_CXX23_DENORM_DEPRECATION_WARNING \
	-D_CRT_NON_CONFORMING_SWPRINTFS \
	-D_CRT_NONSTDC_NO_DEPRECATE \
	-D_CRT_SECURE_NO_DEPRECATE \
	-D_SCL_SECURE_NO_WARNINGS \
	-D_MT \
	-D_DLL \
	-DCPPU_ENV=$(CPPU_ENV) \

ifeq ($(CPUNAME),INTEL)
gb_COMPILERDEFS += \
	-DBOOST_MEM_FN_ENABLE_CDECL \

endif

gb_RCDEFS := \
	$(gb_WIN_VERSION_DEFS) \

gb_RCFLAGS :=

gb_AFLAGS := $(AFLAGS)

# Do we really need to disable this many warnings? It seems to me that
# many of these warnings are for constructs that we have been actively
# cleaning away from the code, to avoid warnings when building with
# gcc or Clang and -Wall -Werror.

# C4127: conditional expression is constant

# C4201: nonstandard extension used : nameless struct/union

# C4244: nonstandard extension used : formal parameter 'identifier'
#   was previously defined as a type

# C4250: 'class1' : inherits 'class2::member' via dominance

# C4251: 'identifier' : class 'type' needs to have dll-interface to be
#   used by clients of class 'type2'

# C4267: conversion from 'size_t' to 'type', possible loss of data

# C4275: non-DLL-interface classkey 'identifier' used as base for
#   DLL-interface classkey 'identifier'

# C4505: 'function' : unreferenced local function has been removed

# C4611: interaction between 'function' and C++ object destruction is
#   non-portable

# C4702: unreachable code

# C4706: assignment within conditional expression

# build-time penalty is to high for ci use/disable when JENKINS_HOME is set
MSVC_ANALYZE_FLAGS := $(if $(JENKINS_HOME),,-analyze:ruleset$(SRCDIR)/solenv/vs/LibreOffice.ruleset)

gb_FilterOutClangCFLAGS += $(MSVC_ANALYZE_FLAGS)

gb_CFLAGS := \
	-utf-8 \
	-Gd \
	-GR \
	-Gs \
	-GS \
	$(if $(MSVC_USE_DEBUG_RUNTIME),-MDd,-MD) \
	-nologo \
	-W4 \
	-wd4244 \
	-wd4505 \
	-bigobj \

gb_CXXFLAGS_DISABLE_WARNINGS = -w

gb_CXXFLAGS := \
	-utf-8 \
	$(CXXFLAGS_CXX11) \
	-Gd \
	-GR \
	-Gs \
	-GS \
	-Gy \
	$(if $(MSVC_USE_DEBUG_RUNTIME),-MDd,-MD) \
	-nologo \
	-W4 \
	-wd4127 \
	-wd4201 \
	-wd4244 \
	-wd4250 \
	-wd4251 \
	-wd4267 \
	-wd4275 \
	-wd4505 \
	-wd4611 \
	-wd4706 \
	-bigobj \
	$(if $(ENABLE_DEBUG),$(MSVC_ANALYZE_FLAGS),) \

ifneq ($(COM_IS_CLANG),TRUE)
gb_CXXFLAGS += -Zc:inline
gb_CXXFLAGS_ZCINLINE_OFF := -Zc:inline-
endif

ifeq ($(CPUNAME),INTEL)

gb_CXXFLAGS += \
	-Zm500 \

gb_CFLAGS += \
	-Zm500 \

endif

ifeq ($(HAVE_DLLEXPORTINLINES),TRUE)
gb_CXXFLAGS += -Zc:dllexportInlines-
endif

gb_CXXFLAGS_include := -FI
ifeq ($(COM_IS_CLANG),TRUE)
gb_CXXFLAGS_no_pch_warnings := -Wno-clang-cl-pch
endif

ifneq ($(COM_IS_CLANG),TRUE)

# Work around MSVC 2017 C4702 compiler bug with release builds
# https://lists.freedesktop.org/archives/libreoffice/2018-July/080532.html
# https://lists.freedesktop.org/archives/libreoffice/2018-August/080776.html
gb_CXXFLAGS += \
	$(if $(ENABLE_OPTIMIZED),-wd4702) \

endif

# rc.exe does not support -nologo in 6.1.6723.1 that is in the Windows SDK 6.0A
gb_RCFLAGS += -nologo

# C4005: 'identifier' : macro redefinition

gb_PCHWARNINGS = \
	-we4650 \
	-we4651 \
	-we4652 \
	-we4653 \
	-we4005 \

gb_STDLIBS := \
	advapi32.lib \

gb_CFLAGS_WERROR = $(if $(ENABLE_WERROR),-WX)

# there does not seem to be a way to force C++03 with MSVC, nor with clang-cl against MSVC system
# headers; the oldest version that MSVC supports is C++14, so use that as a rather imperfect
# approximation:
gb_CXX03FLAGS := -std:c++14

gb_LinkTarget_EXCEPTIONFLAGS := \
	-EHs \

gb_PrecompiledHeader_EXCEPTIONFLAGS := $(gb_LinkTarget_EXCEPTIONFLAGS)

ifneq ($(gb_ENABLE_PCH),)
ifeq ($(COM_IS_CLANG),TRUE)
# the same as in com_GCC_defs.mk
gb_NO_PCH_TIMESTAMP := -Xclang -fno-pch-timestamp
endif
endif

gb_LinkTarget_LDFLAGS := \
	$(if $(findstring s,$(filter-out --%,$(MAKEFLAGS))),-nologo,) \
	$(patsubst %,-LIBPATH:%,$(filter-out .,$(subst ;, ,$(subst \,/,$(ILIB))))) \

# Prevent warning spamming
# Happens because of the way we link our unit tests with our libraries.
# LNK4049: locally defined symbol
gb_LinkTarget_LDFLAGS += \
	/ignore:4217 /ignore:4049


ifeq ($(ENABLE_Z7_DEBUG),)
gb_DEBUGINFO_FLAGS := \
	-FS \
	-Zi \

else
# ccache does not work with -Zi
gb_DEBUGINFO_FLAGS := \
	-Z7 \

endif

# See gb_Windows_PE_TARGETTYPEFLAGS_DEBUGINFO
gb_LINKER_DEBUGINFO_FLAGS :=

gb_COMPILEROPTFLAGS := -O2 -Oy-
gb_COMPILERNOOPTFLAGS := -Od
gb_COMPILERDEBUGOPTFLAGS :=

ifeq ($(gb_FULLDEPS),$(true))
gb_COMPILERDEPFLAGS := -showIncludes
define gb_create_deps
| $(GBUILDDIR)/platform/filter-showIncludes.awk -vdepfile=$(1) -vobjectfile=$(2) -vsourcefile=$(3); exit $${PIPESTATUS[0]}
endef
else
gb_COMPILERDEPFLAGS :=
define gb_create_deps
| $(GBUILDDIR)/platform/filter-sourceName.awk; exit $${PIPESTATUS[0]}
endef
endif

gb_LTOFLAGS := $(if $(filter TRUE,$(ENABLE_LTO)),-GL)

# VS2019 produces a warning C4857, that it doesn't support -std:c++20; it can't
# be suppressed by -wd4857, only by -Wv:18. The warning seems incorrect, because
# using -std:c++17 produces errors about undeclared 'char8_t'. VS2022 doesn't
# have the problem, so drop -Wv:18 when bumping baseline.
gb_CXXCLRFLAGS := \
	$(if $(COM_IS_CLANG), \
	    $(patsubst -std=%,-std:c++20 -Zc:__cplusplus,$(gb_CXXFLAGS)), \
	    $(gb_CXXFLAGS)) \
	$(gb_LinkTarget_EXCEPTIONFLAGS) \
	-AI $(INSTDIR)/$(LIBO_URE_LIB_FOLDER) \
	-EHa \
	-clr \
	-Wv:18 \
	-Zc:twoPhase- \

ifeq ($(COM_IS_CLANG),TRUE)

gb_CFLAGS += \
	-Wendif-labels \
	-Wshadow \
	-Wstrict-prototypes \
	-Wundef \
	-Wunused-macros \

gb_CXXFLAGS += \
	-Wendif-labels \
	-Wimplicit-fallthrough \
	-Wno-missing-braces \
	-Woverloaded-virtual \
	-Wshadow \
	-Wundef \
	-Wunused-macros \

endif

ifeq ($(COM_IS_CLANG),TRUE)
gb_COMPILER_TEST_FLAGS := -Xclang -plugin-arg-loplugin -Xclang --unit-test-mode
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/clang/plugin.dll -Xclang -add-plugin -Xclang loplugin
ifneq ($(COMPILER_PLUGIN_WARNINGS_ONLY),)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang \
    --warnings-only='$(COMPILER_PLUGIN_WARNINGS_ONLY)'
endif
ifeq ($(COMPILER_PLUGINS_DEBUG),TRUE)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang --debug
endif
gb_COMPILER_PLUGINS_TOOL := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/clang/plugin.dll -Xclang -plugin -Xclang loplugin $(foreach plugin,$(COMPILER_PLUGIN_TOOL), -Xclang -plugin-arg-loplugin -Xclang $(plugin))
ifneq ($(UPDATE_FILES),)
gb_COMPILER_PLUGINS_TOOL += -Xclang -plugin-arg-loplugin -Xclang --scope=$(UPDATE_FILES)
endif
ifeq ($(COMPILER_PLUGINS_DEBUG),TRUE)
gb_COMPILER_PLUGINS_TOOL += -Xclang -plugin-arg-loplugin -Xclang --debug
endif
gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS := \
    -Xclang -plugin-arg-loplugin -Xclang --warnings-as-errors
else
gb_COMPILER_TEST_FLAGS :=
gb_COMPILER_PLUGINS :=
gb_COMPILER_PLUGINS_TOOL :=
gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS :=
endif

# Helper class

gb_Helper_set_ld_path := $(call gb_Helper_cyg_path,PATH="$(INSTDIR_FOR_BUILD)/$(LIBO_URE_LIB_FOLDER):$(INSTDIR_FOR_BUILD)/$(LIBO_BIN_FOLDER):$$PATH")

define gb_Helper_prepend_ld_path
PATH="$(call gb_Helper_cyg_path,$(INSTDIR_FOR_BUILD)/$(LIBO_URE_LIB_FOLDER):$(INSTDIR_FOR_BUILD)/$(LIBO_BIN_FOLDER):$(1):$$PATH")
endef

# $(1): one directory pathname to append to the ld path
define gb_Helper_extend_ld_path
$(gb_Helper_set_ld_path)':$(call gb_Helper_cyg_path,$(1))'
endef

# common macros to build GPG related libraries
# we explicitly have to replace cygwin with mingw32 for the host, but the build must stay cygwin, or cmd.exe processes will be spawned
gb_WIN_GPG_WINDRES_target := $(if $(filter INTEL,$(CPUNAME)),pe-i386,pe-x86-64)
gb_WIN_GPG_platform_switches := --build=$(BUILD_PLATFORM) --host=$(subst cygwin,mingw32,$(HOST_PLATFORM))
gb_WIN_GPG_cross_setup_exports = export REAL_BUILD_CC="$(filter-out -%,$(CC_FOR_BUILD))" REAL_BUILD_CC_FLAGS="$(filter -%,$(CC_FOR_BUILD))" \
    && export CC_FOR_BUILD="$(call gb_Executable_get_target_for_build,gcc-wrapper) --wrapper-env-prefix=REAL_BUILD_ $(SOLARINC) -L$(subst ;, -L,$(ILIB_FOR_BUILD))" \
    && export RC='windres -O COFF --target=$(gb_WIN_GPG_WINDRES_target) --preprocessor=$(call gb_Executable_get_target_for_build,cpp) --preprocessor-arg=-+ -DRC_INVOKED -DWINAPI_FAMILY=0 $(SOLARINC)'

ifneq ($(gb_ENABLE_PCH),)
# Enable use of .sum files for PCHs.
gb_COMPILER_SETUP += CCACHE_PCH_EXTSUM=1
endif

# vim: set noet sw=4:
