# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

gb_AWK := awk
gb_YACC := bison

gb_CLASSPATHSEP := :

# use CC/CXX if they are nondefaults
ifneq ($(origin CC),default)
gb_CC := $(CC)
gb_GCCP := $(CC)
else
gb_CC := gcc
gb_GCCP := gcc
endif

ifneq ($(origin CXX),default)
gb_CXX := $(CXX)
else
gb_CXX := g++
endif

ifneq ($(origin AR),default)
gb_AR := $(AR)
else
gb_AR := $(shell $(CC) -print-prog-name=ar)
endif

ifeq ($(strip $(gb_COMPILERDEFAULTOPTFLAGS)),)
gb_COMPILERDEFAULTOPTFLAGS := -O2
endif

gb_CCVER := $(shell $(gb_CC) -dumpversion | $(gb_AWK) -F. -- '{ print $$1*10000+$$2*100+$$3 }')

gb_CPPU_ENV := gcc3

gb_COMPILERDEFS := \
	-D$(COM) \
	-DCPPU_ENV=$(gb_CPPU_ENV) \
	-DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH) \

ifeq ($(HAVE_GCC_BUILTIN_ATOMIC),TRUE)
gb_COMPILERDEFS += \
    -DHAVE_GCC_BUILTIN_ATOMIC \

endif

gb_CFLAGS_COMMON := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-fmessage-length=0 \
	-fno-common \
	-pipe \

gb_CXXFLAGS_COMMON := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-fmessage-length=0 \
	-fno-common \
	-pipe \

ifneq ($(HAVE_THREADSAFE_STATICS),TRUE)
gb_CXXFLAGS_COMMON += -fno-threadsafe-statics
endif

ifeq ($(HAVE_GCC_VISIBILITY_FEATURE),TRUE)
gb_VISIBILITY_FLAGS := -DHAVE_GCC_VISIBILITY_FEATURE -fvisibility=hidden
ifneq ($(HAVE_GCC_VISIBILITY_BROKEN),TRUE)
gb_CXXFLAGS_COMMON += -fvisibility-inlines-hidden
endif
endif

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -Werror -DLIBO_WERROR
gb_CXXFLAGS_WERROR := -Werror -DLIBO_WERROR
endif

ifeq ($(MERGELIBS),TRUE)
gb_CFLAGS_COMMON += -DLIBO_MERGELIBS
gb_CXXFLAGS_COMMON += -DLIBO_MERGELIBS
endif

ifeq ($(ENABLE_LTO),TRUE)
gb_Library_LTOFLAGS := -flto
endif

gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \
	-fexceptions

ifeq ($(gb_ENABLE_DBGUTIL),$(false))
# Clang doesn't have this option
ifeq ($(HAVE_GCC_FNO_ENFORCE_EH_SPECS),TRUE)
gb_LinkTarget_EXCEPTIONFLAGS += \
	-fno-enforce-eh-specs
endif
endif

gb_LinkTarget_NOEXCEPTIONFLAGS := \
	-DEXCEPTIONS_OFF \
	-fno-exceptions \


# optimization level
gb_COMPILEROPTFLAGS := $(gb_COMPILERDEFAULTOPTFLAGS)
gb_COMPILERNOOPTFLAGS := -O0

# Clang does not know -ggdb2 or some other options
ifeq ($(HAVE_GCC_GGDB2),TRUE)
GGDB2=-ggdb2
else
GGDB2=-g2
endif

ifeq ($(HAVE_GCC_FINLINE_LIMIT),TRUE)
FINLINE_LIMIT0=-finline-limit=0
endif

ifeq ($(HAVE_GCC_FNO_INLINE),TRUE)
FNO_INLINE=-fno-inline
endif

ifeq ($(HAVE_GCC_FNO_DEFAULT_INLINE),TRUE)
FNO_DEFAULT_INLINE=-fno-default-inline
endif

gb_DEBUG_CFLAGS := $(GGDB2) $(FINLINE_LIMIT0) $(FNO_INLINE)
gb_DEBUG_CXXFLAGS := $(FNO_DEFAULT_INLINE)


gb_LinkTarget_INCLUDE := $(subst -I. , ,$(SOLARINC))

ifeq ($(COM_GCC_IS_CLANG),TRUE)
ifeq ($(COMPILER_PLUGIN_TOOL),)
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(SRCDIR)/compilerplugins/obj/plugin.so -Xclang -add-plugin -Xclang loplugin
else
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(SRCDIR)/compilerplugins/obj/plugin.so -Xclang -plugin -Xclang loplugin -Xclang -plugin-arg-loplugin -Xclang $(COMPILER_PLUGIN_TOOL)
endif
gb_COMPILER_PLUGINS_SETUP := ICECC_EXTRAFILES=$(SRCDIR)/sal/inc/sal/log-areas.dox
else
gb_COMPILER_PLUGINS :=
gb_COMPILER_PLUGINS_SETUP :=
endif

# Executable class

gb_Executable_EXT_for_build :=

# Helper class

ifeq ($(OS_FOR_BUILD),MACOSX)
gb_Helper_LIBRARY_PATH_VAR := DYLD_LIBRARY_PATH
else ifeq ($(OS_FOR_BUILD),WNT)
# In theory possible if cross-compiling to some Unix from Windows,
# in practice strongly discouraged to even try that
gb_Helper_LIBRARY_PATH_VAR := PATH
else
gb_Helper_LIBRARY_PATH_VAR := LD_LIBRARY_PATH
endif

gb_Helper_set_ld_path := $(gb_Helper_LIBRARY_PATH_VAR)=$${$(gb_Helper_LIBRARY_PATH_VAR):+$$$(gb_Helper_LIBRARY_PATH_VAR):}"$(OUTDIR_FOR_BUILD)/lib"

# $(1): list of directory pathnames to append at the end of the ld path
define gb_Helper_extend_ld_path
$(gb_Helper_set_ld_path)$(foreach dir,$(1),:$(dir))
endef

# Convert path to file URL.
define gb_Helper_make_url
file://$(strip $(1))
endef

gb_Helper_symlinked_native = $(1)

gb_Helper_OUTDIRLIBDIR := $(OUTDIR)/lib
gb_Helper_OUTDIR_FOR_BUILDLIBDIR := $(OUTDIR_FOR_BUILD)/lib

gb_Helper_get_rcfile = $(1)rc
