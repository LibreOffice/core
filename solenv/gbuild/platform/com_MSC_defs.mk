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

# set tmpdir to some mixed case path, suitable for native tools
gb_TMPDIR:=$(if $(TMPDIR),$(shell cygpath -m $(TMPDIR)),$(shell cygpath -m /tmp))

# please make generic Windows modifications to windows.mk
include $(GBUILDDIR)/platform/windows.mk

gb_CC := cl
gb_CXX := cl
gb_LINK := link
gb_AWK := awk
gb_CLASSPATHSEP := ;
gb_RC := rc
gb_YACC := bison

# use CC/CXX if they are nondefaults
ifneq ($(origin CC),default)
gb_CC := $(CC)
gb_GCCP := $(CC)
endif
ifneq ($(origin CXX),default)
gb_CXX := $(CXX)
endif

# _SCL_SECURE_NO_WARNINGS avoids deprecation warnings for STL algorithms
# like std::copy, std::transform (when MSVC_USE_DEBUG_RUNTIME is enabled)

gb_COMPILERDEFS := \
	-DBOOST_ERROR_CODE_HEADER_ONLY \
	-DBOOST_OPTIONAL_USE_OLD_DEFINITION_OF_NONE \
	-DBOOST_SYSTEM_NO_DEPRECATED \
	-D_HAS_AUTO_PTR_ETC \
	-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING \
	-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING \
	-D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING \
	-D_CRT_NON_CONFORMING_SWPRINTFS \
	-D_CRT_NONSTDC_NO_DEPRECATE \
	-D_CRT_SECURE_NO_DEPRECATE \
	-D_SCL_SECURE_NO_WARNINGS \
	-D_MT \
	-D_DLL \
	-DCPPU_ENV=$(gb_CPPU_ENV) \

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

# C4091: 'typedef ': ignored on left of '' when no variable is declared

# C4100: 'identifier' : unreferenced formal parameter

# C4127: conditional expression is constant

# C4189: 'identifier' : local variable is initialized but not referenced

# C4200: nonstandard extension used : zero-sized array in struct/union

# C4201: nonstandard extension used : nameless struct/union

# C4244: nonstandard extension used : formal parameter 'identifier'
#   was previously defined as a type

# C4250: 'class1' : inherits 'class2::member' via dominance

# C4251: 'identifier' : class 'type' needs to have dll-interface to be
#   used by clients of class 'type2'

# C4267: conversion from 'size_t' to 'type', possible loss of data

# C4275: non-DLL-interface classkey 'identifier' used as base for
#   DLL-interface classkey 'identifier'

# C4290: C++ exception specification ignored except to indicate a
#   function is not __declspec(nothrow)

# C4351: new behavior: elements of array 'array' will be default
#   initialized
# (an issue with MSVC 2013 that appears to be gone with MSVC 2015)

# C4373: '%$S': virtual function overrides '%$pS', previous versions
#   of the compiler did not override when parameters only differed by
#   const/volatile qualifiers.
# [translation: ancient compilers that don't actually support C++ do
#  stupid things]

# C4505: 'function' : unreferenced local function has been removed

# C4510: 'class' : default constructor could not be generated

# C4512: 'class' : assignment operator could not be generated
# (an issue with MSVC 2013 that appears to be gone with MSVC 2015)

# C4610: 'class' can never be instantiated - user defined constructor required

# C4611: interaction between 'function' and C++ object destruction is
#   non-portable

# C4702: unreachable code

# C4706: assignment within conditional expression

# C4800: 'type' : forcing value to bool 'true' or 'false' (performance
#   warning)

gb_CFLAGS := \
	-utf-8 \
	-Gd \
	-GR \
	-Gs \
	-GS \
	$(if $(MSVC_USE_DEBUG_RUNTIME),-MDd,-MD) \
	-nologo \
	-W4 \
	-wd4091 \
	$(if $(filter 0,$(gb_DEBUGLEVEL)),-wd4100) \
	-wd4127 \
	$(if $(filter 0,$(gb_DEBUGLEVEL)),-wd4189) \
	-wd4200 \
	-wd4244 \
	-wd4251 \
	-wd4505 \
	-wd4512 \
	-wd4706 \
	-wd4800 \
	-wd4267 \

ifneq ($(COM_IS_CLANG),TRUE)

# clang-cl doesn't support -Wv:18 for now
gb_CFLAGS += \
	-Wv:18 \

endif

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
	-wd4091 \
	$(if $(filter 0,$(gb_DEBUGLEVEL)),-wd4100) \
	-wd4127 \
	$(if $(filter 0,$(gb_DEBUGLEVEL)),-wd4189) \
	-wd4201 \
	-wd4244 \
	-wd4250 \
	-wd4251 \
	-wd4267 \
	-wd4275 \
	-wd4290 \
	-wd4351 \
	-wd4373 \
	-wd4505 \
	-wd4510 \
	-wd4512 \
	-wd4610 \
	-wd4611 \
	-wd4706 \
	-wd4800 \

ifeq ($(CPUNAME),INTEL)

gb_CXXFLAGS += \
	-Zm500 \

gb_CFLAGS += \
	-Zm500 \

endif

ifneq ($(COM_IS_CLANG),TRUE)

# clang-cl doesn't support -Wv:18 for now
# Work around MSVC 2017 C4702 compiler bug with release builds
# http://document-foundation-mail-archive.969070.n3.nabble.com/Windows-32-bit-build-failure-unreachable-code-tp4243848.html
# http://document-foundation-mail-archive.969070.n3.nabble.com/64-bit-Windows-build-failure-after-MSVC-Update-tp4246816.html
gb_CXXFLAGS += \
	-Wv:18 \
	$(if $(filter 0,$(gb_DEBUGLEVEL)),-wd4702) \

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

# there does not seem to be a way to force C++03 with MSVC
gb_CXX03FLAGS :=

gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \
	-EHs \

gb_PrecompiledHeader_EXCEPTIONFLAGS := $(gb_LinkTarget_EXCEPTIONFLAGS)

gb_LinkTarget_LDFLAGS := \
	$(if $(findstring s,$(filter-out --%,$(MAKEFLAGS))),-nologo,) \
	$(patsubst %,-LIBPATH:%,$(filter-out .,$(subst ;, ,$(subst \,/,$(ILIB))))) \

# Prevent warning spamming
# Happens because of the way we link our unit tests with our libraries.
# LNK4049: locally defined symbol
gb_LinkTarget_LDFLAGS += \
	/ignore:4217 /ignore:4049


gb_DEBUGINFO_FLAGS := \
	-FS \
	-Zi \

gb_COMPILEROPTFLAGS := -O2 -Oy-
gb_COMPILERNOOPTFLAGS := -Od

ifeq ($(gb_FULLDEPS),$(true))
gb_COMPILERDEPFLAGS := -showIncludes
define gb_create_deps
| $(GBUILDDIR)/platform/filter-showIncludes.awk -vdepfile=$(1) -vobjectfile=$(2) -vsourcefile=$(3); exit $${PIPESTATUS[0]}
endef
else
gb_COMPILERDEPFLAGS :=
define gb_create_deps
endef
endif

gb_LTOFLAGS := $(if $(filter TRUE,$(ENABLE_LTO)),-GL)

# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
gb_CXXCLRFLAGS := $(gb_CXXFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
	-AI $(INSTDIR)/$(LIBO_URE_LIB_FOLDER) \
	-EHa \
	-clr \
	-wd4339 \
	-Wv:18 \
	-wd4267 \

ifeq ($(COM_IS_CLANG),TRUE)

gb_CFLAGS += \
	-Wdeclaration-after-statement \
	-Wendif-labels \
	-Wshadow \
	-Wstrict-prototypes \
	-Wundef \
	-Wunused-macros \

gb_CXXFLAGS += \
	-Wendif-labels \
	-Wimplicit-fallthrough \
	-Wno-missing-braces \
	-Wno-missing-braces \
	-Wnon-virtual-dtor \
	-Woverloaded-virtual \
	-Wshadow \
	-Wundef \
	-Wunused-macros \

endif

ifeq ($(COM_IS_CLANG),TRUE)
gb_COMPILER_TEST_FLAGS := -Xclang -plugin-arg-loplugin -Xclang --unit-test-mode
ifeq ($(COMPILER_PLUGIN_TOOL),)
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/obj/plugin.dll -Xclang -add-plugin -Xclang loplugin
ifneq ($(COMPILER_PLUGIN_WARNINGS_ONLY),)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang \
    --warnings-only='$(COMPILER_PLUGIN_WARNINGS_ONLY)'
endif
else
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/obj/plugin.dll -Xclang -plugin -Xclang loplugin $(foreach plugin,$(COMPILER_PLUGIN_TOOL), -Xclang -plugin-arg-loplugin -Xclang $(plugin))
ifneq ($(UPDATE_FILES),)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang --scope=$(UPDATE_FILES)
endif
endif
ifeq ($(COMPILER_PLUGINS_DEBUG),TRUE)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang --debug
endif
gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS := \
    -Xclang -plugin-arg-loplugin -Xclang --warnings-as-errors
else
gb_COMPILER_TEST_FLAGS :=
gb_COMPILER_PLUGINS :=
gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS :=
endif

# Helper class

ifeq ($(GNUMAKE_WIN_NATIVE),TRUE)
gb_Helper_set_ld_path := PATH="$(shell cygpath -w $(INSTDIR)/$(LIBO_URE_LIB_FOLDER));$(shell cygpath -w $(INSTDIR)/$(LIBO_BIN_FOLDER));$$PATH"

define gb_Helper_prepend_ld_path
PATH="$(shell cygpath -w $(INSTDIR)/$(LIBO_URE_LIB_FOLDER));$(shell cygpath -w $(INSTDIR)/$(LIBO_BIN_FOLDER));$(1);$$PATH"
endef

# $(1): one directory pathname to append to the ld path
define gb_Helper_extend_ld_path
$(gb_Helper_set_ld_path)';$(shell cygpath -w $(1))'
endef

else
gb_Helper_set_ld_path := PATH="$(shell cygpath -u $(INSTDIR)/$(LIBO_URE_LIB_FOLDER)):$(shell cygpath -u $(INSTDIR)/$(LIBO_BIN_FOLDER)):$$PATH"

define gb_Helper_prepend_ld_path
PATH="$(shell cygpath -u $(INSTDIR)/$(LIBO_URE_LIB_FOLDER)):$(shell cygpath -u $(INSTDIR)/$(LIBO_BIN_FOLDER)):$(1):$$PATH"
endef

# $(1): one directory pathname to append to the ld path
define gb_Helper_extend_ld_path
$(gb_Helper_set_ld_path):$(shell cygpath -u $(1))
endef

endif

# vim: set noet sw=4:
