# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

gb_AWK := awk
gb_YACC := bison

gb_CLASSPATHSEP := :
gb_LICENSE := LICENSE
gb_README = README_$(1)

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

gb_SHORTSTDC3 := 1
gb_SHORTSTDCPP3 := 6

gb_CPPU_ENV := gcc3

gb_AFLAGS := $(AFLAGS)

gb_COMPILERDEFS := \
	-DCPPU_ENV=$(gb_CPPU_ENV) \

gb_CFLAGS_COMMON := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wundef \
	-Wunused-macros \
	-fmessage-length=0 \
	-fno-common \
	-pipe \

gb_CXXFLAGS_COMMON := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wundef \
	-Wunused-macros \
	-fmessage-length=0 \
	-fno-common \
	-pipe \

ifneq ($(HAVE_THREADSAFE_STATICS),TRUE)
gb_CXXFLAGS_COMMON += -fno-threadsafe-statics
endif

ifeq ($(strip $(gb_GCOV)),YES)
gb_CFLAGS_COMMON += -fprofile-arcs -ftest-coverage
gb_CXXFLAGS_COMMON += -fprofile-arcs -ftest-coverage
gb_LinkTarget_LDFLAGS += -fprofile-arcs -lgcov
gb_COMPILERDEFAULTOPTFLAGS := -O0
endif


ifeq ($(HAVE_GCC_VISIBILITY_FEATURE),TRUE)
gb_VISIBILITY_FLAGS := -DHAVE_GCC_VISIBILITY_FEATURE -fvisibility=hidden
ifneq ($(HAVE_GCC_VISIBILITY_BROKEN),TRUE)
gb_CXXFLAGS_COMMON += -fvisibility-inlines-hidden
endif
endif

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -Werror
gb_CXXFLAGS_WERROR := -Werror
endif

ifneq ($(MERGELIBS),)
gb_CFLAGS_COMMON += -DLIBO_MERGELIBS
gb_CXXFLAGS_COMMON += -DLIBO_MERGELIBS
endif

ifeq ($(ENABLE_LTO),TRUE)
gb_LTOFLAGS := -flto
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

gb_PrecompiledHeader_EXCEPTIONFLAGS := $(gb_LinkTarget_EXCEPTIONFLAGS)

# optimization level
gb_COMPILEROPTFLAGS := $(gb_COMPILERDEFAULTOPTFLAGS)
gb_COMPILERNOOPTFLAGS := -O0 -fstrict-aliasing -fstrict-overflow

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


gb_LinkTarget_INCLUDE :=\
    $(subst -I. , ,$(SOLARINC)) \
    -I$(BUILDDIR)/config_$(gb_Side) \

ifeq ($(COM_GCC_IS_CLANG),TRUE)
ifeq ($(COMPILER_PLUGIN_TOOL),)
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/obj/plugin.so -Xclang -add-plugin -Xclang loplugin
else
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/obj/plugin.so -Xclang -plugin -Xclang loplugin -Xclang -plugin-arg-loplugin -Xclang $(COMPILER_PLUGIN_TOOL)
ifneq ($(UPDATE_FILES),)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang --scope=$(UPDATE_FILES)
endif
endif
# extra EF variable to make the command line shorter (just like is done with $(SRCDIR) etc.)
gb_COMPILER_PLUGINS_SETUP := EF=$(SRCDIR)/include/sal/log-areas.dox && ICECC_EXTRAFILES=$$EF CCACHE_EXTRAFILES=$$EF
else
gb_COMPILER_PLUGINS :=
gb_COMPILER_PLUGINS_SETUP :=
endif

# Executable class

gb_Executable_EXT_for_build :=

# Helper class

ifeq ($(OS_FOR_BUILD),MACOSX)
gb_Helper_LIBRARY_PATH_VAR := DYLD_LIBRARY_PATH
else ifeq ($(OS_FOR_BUILD),AIX)
gb_Helper_LIBRARY_PATH_VAR := LIBPATH
else ifeq ($(OS_FOR_BUILD),WNT)
# In theory possible if cross-compiling to some Unix from Windows,
# in practice strongly discouraged to even try that
gb_Helper_LIBRARY_PATH_VAR := PATH
else
gb_Helper_LIBRARY_PATH_VAR := LD_LIBRARY_PATH
endif

gb_Helper_set_ld_path := $(gb_Helper_LIBRARY_PATH_VAR)=$${$(gb_Helper_LIBRARY_PATH_VAR):+$$$(gb_Helper_LIBRARY_PATH_VAR):}"$(OUTDIR_FOR_BUILD)/lib:$(INSTROOT_FOR_BUILD)/$(LIBO_URE_LIB_FOLDER_FOR_BUILD):$(INSTROOT_FOR_BUILD)/$(LIBO_LIB_FOLDER_FOR_BUILD)"

# $(1): list of directory pathnames to append at the end of the ld path
define gb_Helper_extend_ld_path
$(gb_Helper_set_ld_path)$(foreach dir,$(1),:$(dir))
endef

# Convert path to file URL.
define gb_Helper_make_url
file://$(strip $(1))
endef

gb_Helper_OUTDIRLIBDIR := $(OUTDIR)/lib

gb_Helper_get_rcfile = $(1)rc
