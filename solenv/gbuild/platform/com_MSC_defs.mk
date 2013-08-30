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

gb_COMPILERDEFS := \
	-D_CRT_NON_CONFORMING_SWPRINTFS \
	-D_CRT_NONSTDC_NO_DEPRECATE \
	-D_CRT_SECURE_NO_DEPRECATE \
	-D_MT \
	-D_DLL \
	-DCPPU_ENV=$(gb_CPPU_ENV) \
	-DM1500 \
	$(if $(findstring 110_70,$(VCVER)_$(WINDOWS_SDK_VERSION)),-D_USING_V110_SDK71_) \

ifeq ($(CPUNAME),INTEL)
gb_COMPILERDEFS += \
	-DBOOST_MEM_FN_ENABLE_CDECL \

endif

gb_RCDEFS := \
	-DWINVER=0x0400 \
	-DWIN32 \

gb_RCFLAGS :=

gb_AFLAGS := $(AFLAGS)

# Do we really need to disable to many warnings? It seems to me that
# many of these warnings are for custructs that we have been actively
# cleaning away from the code, to avoid warnings when building with
# gcc or Clang and -Wall -Werror.

# C4127: conditional expression is constant

# C4189: 'identifier' : local variable is initialized but not referenced

# C4242: 'identifier' : conversion from 'type1' to 'type2', possible
#   loss of data

# C4244: nonstandard extension used : formal parameter 'identifier'
#   was previously defined as a type

# C4245: 'conversion' : conversion from 'type1' to 'type2',
#   signed/unsigned mismatch

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

# C4355: 'this' : used in base member initializer list

# C4373: '%$S': virtual function overrides '%$pS', previous versions
#   of the compiler did not override when parameters only differed by
#   const/volatile qualifiers.

# C4481: nonstandard extension used: override specifier 'override'
# (MSVC 2010 warns about this, even though it's C++11 keyword)

# C4505: 'function' : unreferenced local function has been removed

# C4512: 'class' : assignment operator could not be generated

# C4611: interaction between 'function' and C++ object destruction is
#   non-portable

# C4626: 'derived class' : assignment operator could not be generated
#   because a base class assignment operator is inaccessible

# C4706: assignment within conditional expression

# C4800: 'type' : forcing value to bool 'true' or 'false' (performance
#   warning)

# C4913: user defined binary operator ',' exists but no overload could
#    convert all operands, default built-in binary operator ',' used

# C4996: 'function': was declared deprecated
#   Also generated for C++ library functions that "may be unsafe"

gb_CFLAGS := \
	-Gd \
	-GR \
	-Gs \
	-GS \
	$(if $(MSVC_USE_DEBUG_RUNTIME),-MDd,-MD) \
	-nologo \
	-W4 \
	-wd4127 \
	-wd4189 \
	-wd4242 \
	-wd4244 \
	-wd4245 \
	-wd4251 \
	-wd4355 \
	-wd4505 \
	-wd4512 \
	-wd4626 \
	-wd4706 \
	-wd4800 \
	-Zc:wchar_t- \
	-Zm500 \

ifeq ($(CPUNAME),X86_64)

gb_CFLAGS += \
	-wd4267 \

endif

gb_CXXFLAGS := \
	-Gd \
	-GR \
	-Gs \
	-GS \
	-Gy \
	$(if $(MSVC_USE_DEBUG_RUNTIME),-MDd,-MD) \
	-nologo \
	-W4 \
	-wd4127 \
	-wd4189 \
	-wd4244 \
	-wd4245 \
	-wd4250 \
	-wd4251 \
	-wd4275 \
	-wd4290 \
	-wd4351 \
	-wd4355 \
	-wd4373 \
	-wd4481 \
	-wd4505 \
	-wd4512 \
	-wd4611 \
	-wd4706 \
	-wd4800 \
	-wd4913 \
	-Zc:wchar_t- \
	-Zm500 \

ifneq ($(MSVC_USE_DEBUG_RUNTIME),)
gb_CXXFLAGS += \
	-wd4996 \

endif

# New warning(s) in Visual Studio 2010, let's try disabling these only
# for that specific compiler version, in case a later one will not
# need them disabled.
ifeq ($(VCVER),100)

# (none currently)

endif

ifneq ($(VCVER),100)
# rc.exe does not support -nologo in 6.1.6723.1 that is in the Windows SDK 6.0A
gb_RCFLAGS += -nologo
endif

# C4005: 'identifier' : macro redefinition

gb_PCHWARNINGS = \
	-we4650 \
	-we4651 \
	-we4652 \
	-we4653 \
	-we4005 \

gb_STDLIBS := \
	advapi32.lib \

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -WX
gb_CXXFLAGS_WERROR := -WX
endif

ifneq ($(MERGELIBS),)
gb_CFLAGS += -DLIBO_MERGELIBS
gb_CXXFLAGS += -DLIBO_MERGELIBS
endif

gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \
	-EHa \

gb_PrecompiledHeader_EXCEPTIONFLAGS := $(gb_LinkTarget_EXCEPTIONFLAGS)

gb_LinkTarget_LDFLAGS := \
	$(if $(findstring s,$(filter-out --%,$(MAKEFLAGS))),-nologo,) \
	$(patsubst %,-LIBPATH:%,$(filter-out .,$(subst ;, ,$(subst \,/,$(ILIB))))) \

gb_DEBUG_CFLAGS := -Zi

# this does not use CFLAGS so it is not overridable
ifneq ($(ENABLE_CRASHDUMP),)
gb_CFLAGS+=-Zi
gb_CXXFLAGS+=-Zi
endif

ifeq ($(gb_SYMBOL),$(true))
gb_CFLAGS+=$(gb_DEBUG_CFLAGS)
gb_CXXFLAGS+=$(gb_DEBUG_CFLAGS)
endif

gb_COMPILEROPTFLAGS := -Ob1 -Oxs -Oy-
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

# Helper class

gb_Helper_OUTDIRLIBDIR := $(OUTDIR)/bin
gb_Helper_OUTDIR_FOR_BUILDLIBDIR := $(OUTDIR_FOR_BUILD)/bin

# need windows path with backslashes here
gb_Helper_set_ld_path := PATH="$(shell cygpath -w $(OUTDIR)/bin)"

# vim: set noet sw=4:
