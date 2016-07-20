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

ifeq ($(strip $(gb_COMPILEROPTFLAGS)),)
gb_COMPILEROPTFLAGS := -O2
endif

gb_SHORTSTDC3 := 1
gb_SHORTSTDCPP3 := 6

gb_CPPU_ENV := gcc3

gb_AFLAGS := $(AFLAGS)

gb_COMPILERDEFS := \
	-DBOOST_ERROR_CODE_HEADER_ONLY \
	-DBOOST_SYSTEM_NO_DEPRECATED \
	-DCPPU_ENV=$(gb_CPPU_ENV) \

gb_CFLAGS_COMMON := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wstrict-prototypes \
	-Wundef \
	-Wunused-macros \
	-fmessage-length=0 \
	-fno-common \
	-pipe \

gb_CXXFLAGS_COMMON := \
	-Wall \
	-Wno-missing-braces \
	-Wnon-virtual-dtor \
	-Wendif-labels \
	-Wextra \
	-Wundef \
	-Wunused-macros \
	-fmessage-length=0 \
	-fno-common \
	-pipe \

gb_CXXFLAGS_Wundef = -Wno-undef

ifneq ($(HAVE_THREADSAFE_STATICS),TRUE)
gb_CXXFLAGS_COMMON += -fno-threadsafe-statics
endif

ifeq ($(strip $(gb_GCOV)),YES)
gb_CFLAGS_COMMON += -fprofile-arcs -ftest-coverage
gb_CXXFLAGS_COMMON += -fprofile-arcs -ftest-coverage
gb_LinkTarget_LDFLAGS += -fprofile-arcs -lgcov
gb_COMPILEROPTFLAGS := -O0
endif

ifeq ($(shell expr '$(GCC_VERSION)' '>=' 600),1)
gb_CFLAGS_COMMON += \
    -Wduplicated-cond \
    -Wlogical-op \
    -Wnull-dereference \
    -Wshift-overflow=2
gb_CXXFLAGS_COMMON += \
    -Wduplicated-cond \
    -Wlogical-op \
    -Wnull-dereference \
    -Wshift-overflow=2 \
    -Wunused-const-variable=1
endif

ifeq ($(COM_IS_CLANG),TRUE)
gb_CXXFLAGS_COMMON += -Wimplicit-fallthrough
endif


ifeq ($(HAVE_GCC_VISIBILITY_FEATURE),TRUE)
gb_VISIBILITY_FLAGS := -DHAVE_GCC_VISIBILITY_FEATURE
# If CC or CXX already include -fvisibility=hidden, don't duplicate it
ifeq (,$(filter -fvisibility=hidden,$(CC)))
gb__visibility_hidden := -fvisibility=hidden
ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
gb__visibility_hidden := -fvisibility-ms-compat
endif
endif
gb_VISIBILITY_FLAGS += $(gb__visibility_hidden)
endif
ifneq ($(HAVE_GCC_VISIBILITY_BROKEN),TRUE)
gb_VISIBILITY_FLAGS_CXX := -fvisibility-inlines-hidden
endif
endif
gb_CXXFLAGS_COMMON += $(gb_VISIBILITY_FLAGS_CXX)

ifeq ($(HAVE_GCC_STACK_PROTECTOR_STRONG),TRUE)
gb_CFLAGS_COMMON += -fstack-protector-strong
gb_CXXFLAGS_COMMON += -fstack-protector-strong
gb_LinkTarget_LDFLAGS += -fstack-protector-strong
endif

ifeq ($(ENABLE_PCH),TRUE)
ifneq ($(COM_IS_CLANG),TRUE)
gb_CFLAGS_COMMON += -fpch-preprocess -Winvalid-pch
gb_CXXFLAGS_COMMON += -fpch-preprocess -Winvalid-pch
endif
endif

gb_CFLAGS_WERROR := $(if $(ENABLE_WERROR),-Werror)

# This is the default in non-C++11 mode
ifeq ($(COM_IS_CLANG),TRUE)
gb_CXX03FLAGS := -std=gnu++98 -Werror=c++11-extensions -Wno-c++11-long-long \
    -Wno-deprecated-declarations
else
gb_CXX03FLAGS := -std=gnu++98 -Wno-long-long \
    -Wno-variadic-macros -Wno-non-virtual-dtor -Wno-deprecated-declarations
endif

# On Windows MSVC only supports C90 so force gnu89 (especially in clang) to
# to catch potential gnu89/C90 incompatibilities locally.
gb_CFLAGS_COMMON += -std=gnu89

ifeq ($(ENABLE_LTO),TRUE)
ifeq ($(COM_IS_CLANG),TRUE)
gb_LTOFLAGS := -flto
gb_LTOPLUGINFLAGS := --plugin LLVMgold.so
else
gb_LTOFLAGS := -flto=$(PARALLELISM) -fuse-linker-plugin -O2
endif
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
gb_COMPILERNOOPTFLAGS := -O0 -fstrict-aliasing -fstrict-overflow

# Clang does not know -ggdb2 or some other options
ifeq ($(HAVE_GCC_GGDB2),TRUE)
gb_DEBUGINFO_FLAGS=-ggdb2
else
gb_DEBUGINFO_FLAGS=-g2
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

gb_DEBUG_CFLAGS := $(FINLINE_LIMIT0) $(FNO_INLINE)
gb_DEBUG_CXXFLAGS := $(FNO_DEFAULT_INLINE)


gb_LinkTarget_INCLUDE :=\
    $(subst -I. , ,$(SOLARINC)) \
    -I$(BUILDDIR)/config_$(gb_Side) \

ifeq ($(COM_IS_CLANG),TRUE)
ifeq ($(COMPILER_PLUGIN_TOOL),)
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/obj/plugin.so -Xclang -add-plugin -Xclang loplugin
ifneq ($(COMPILER_PLUGIN_WARNINGS_ONLY),)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang \
    --warnings-only='$(COMPILER_PLUGIN_WARNINGS_ONLY)'
endif
else
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/obj/plugin.so -Xclang -plugin -Xclang loplugin $(foreach plugin,$(COMPILER_PLUGIN_TOOL), -Xclang -plugin-arg-loplugin -Xclang $(plugin))
ifneq ($(UPDATE_FILES),)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang --scope=$(UPDATE_FILES)
endif
endif
# set CCACHE_CPP2=1 to prevent clang generating spurious warnings
gb_COMPILER_SETUP := CCACHE_CPP2=1
gb_COMPILER_PLUGINS_SETUP := ICECC_EXTRAFILES=$(SRCDIR)/include/sal/log-areas.dox CCACHE_EXTRAFILES=$(SRCDIR)/include/sal/log-areas.dox
gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS := \
    -Xclang -plugin-arg-loplugin -Xclang --warnings-as-errors
else
gb_COMPILER_SETUP :=
gb_COMPILER_PLUGINS :=
gb_COMPILER_PLUGINS_SETUP :=
gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS :=
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

gb_Helper_set_ld_path := $(gb_Helper_LIBRARY_PATH_VAR)=$${$(gb_Helper_LIBRARY_PATH_VAR):+$$$(gb_Helper_LIBRARY_PATH_VAR):}"$(INSTROOT_FOR_BUILD)/$(LIBO_URE_LIB_FOLDER_FOR_BUILD):$(INSTROOT_FOR_BUILD)/$(LIBO_LIB_FOLDER_FOR_BUILD)"

# $(1): list of : separated directory pathnames to append to the ld path
define gb_Helper_extend_ld_path
$(gb_Helper_set_ld_path):$(1)
endef

# Convert path to file URL.
define gb_Helper_make_url
file://$(strip $(1))
endef

gb_Helper_get_rcfile = $(1)rc

# vim: set noet sw=4:
