# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

gb_AWK := awk

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

# shell setup (env.vars) for the compiler
gb_COMPILER_SETUP :=

ifeq ($(strip $(gb_COMPILEROPTFLAGS)),)
gb_COMPILEROPTFLAGS := -O2
endif

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
	-Wunreachable-code \
	$(if $(or $(and $(COM_IS_CLANG),$(or $(findstring icecc,$(CC)),$(findstring icecc,$(CCACHE_PREFIX)))),$(findstring sccache,$(CC))),,-Wunused-macros) \
	$(if $(COM_IS_CLANG),-Wembedded-directive) \
	-finput-charset=UTF-8 \
	-fmessage-length=0 \
	-fno-common \
	-pipe \
	-fstack-protector-strong \
	$(if $(gb_COLOR),-fdiagnostics-color=always) \

gb_CXXFLAGS_COMMON := \
	-Wall \
	-Wno-missing-braces \
	-Wnon-virtual-dtor \
	-Wendif-labels \
	-Wextra \
	-Wundef \
	-Wunreachable-code \
	$(if $(or $(and $(COM_IS_CLANG),$(or $(findstring icecc,$(CXX)),$(findstring icecc,$(CCACHE_PREFIX)))),$(findstring sccache,$(CXX))),,-Wunused-macros) \
	$(if $(COM_IS_CLANG),-Wembedded-directive) \
	-finput-charset=UTF-8 \
	-fmessage-length=0 \
	-fno-common \
	-pipe \
	-fstack-protector-strong \
	$(if $(gb_COLOR),-fdiagnostics-color=always) \

ifeq ($(HAVE_WDEPRECATED_COPY_DTOR),TRUE)
gb_CXXFLAGS_COMMON += -Wdeprecated-copy-dtor
endif

gb_CXXFLAGS_DISABLE_WARNINGS = -w

ifeq ($(HAVE_BROKEN_GCC_WMAYBE_UNINITIALIZED),TRUE)
gb_CXXFLAGS_COMMON += -Wno-maybe-uninitialized
endif

ifeq ($(HAVE_BROKEN_GCC_WSTRINGOP_OVERFLOW),TRUE)
gb_CXXFLAGS_COMMON += -Wno-stringop-overflow
endif

gb_CXXFLAGS_Wundef = -Wno-undef

ifeq ($(strip $(gb_GCOV)),YES)
gb_CFLAGS_COMMON += -fprofile-arcs -ftest-coverage
gb_CXXFLAGS_COMMON += -fprofile-arcs -ftest-coverage
gb_LinkTarget_LDFLAGS += -fprofile-arcs -lgcov
gb_COMPILEROPTFLAGS := -O0
endif

ifeq ($(DISABLE_DYNLOADING),TRUE)
gb_CFLAGS_COMMON += -ffunction-sections -fdata-sections
gb_CXXFLAGS_COMMON += -ffunction-sections -fdata-sections
gb_LinkTarget_LDFLAGS += -Wl,--gc-sections
endif

ifeq ($(COM_IS_CLANG),TRUE)
gb_CXXFLAGS_COMMON += \
	-Wimplicit-fallthrough \
	-Wunused-exception-parameter \
	-Wrange-loop-analysis
else
gb_CFLAGS_COMMON += \
    -Wduplicated-cond \
    -Wlogical-op \
    -Wshift-overflow=2
gb_CXXFLAGS_COMMON += \
    -Wduplicated-cond \
    -Wlogical-op \
    -Wshift-overflow=2 \
    -Wunused-const-variable=1
endif

# GCC 8 -Wcast-function-type (included in -Wextra) unhelpfully even warns on reinterpret_cast
# between incompatible function types:
ifeq ($(shell expr '$(GCC_VERSION)' '>=' 800),1)
gb_CXXFLAGS_COMMON += \
    -Wno-cast-function-type
endif

# If CC or CXX already include -fvisibility=hidden, don't duplicate it
ifeq (,$(filter -fvisibility=hidden,$(CC)))
gb_VISIBILITY_FLAGS := -fvisibility=hidden
endif
gb_VISIBILITY_FLAGS_CXX := -fvisibility-inlines-hidden
gb_CXXFLAGS_COMMON += $(gb_VISIBILITY_FLAGS_CXX)

gb_LinkTarget_LDFLAGS += -fstack-protector-strong

ifneq ($(gb_ENABLE_PCH),)
ifeq ($(COM_IS_CLANG),TRUE)
# Clang by default includes in the PCH timestamps of the files it was
# generated from, which would make the PCH be a "new" file for ccache
# even if the file has not actually changed. Disabling the timestamp
# prevents this at the cost of risking using an outdated PCH (which
# should be unlikely, given that gbuild has dependencies set up
# for our includes and system includes are unlikely to change).
gb_NO_PCH_TIMESTAMP := -Xclang -fno-pch-timestamp
else
gb_CFLAGS_COMMON += -fpch-preprocess -Winvalid-pch
gb_CXXFLAGS_COMMON += -fpch-preprocess -Winvalid-pch
gb_NO_PCH_TIMESTAMP :=
endif
endif

gb_CFLAGS_WERROR = $(if $(ENABLE_WERROR),-Werror)

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
ifneq (,$(index,iOS MACOSX,$(OS)))
gb_LTOFLAGS := -flto=thin
else
gb_LTOFLAGS := -flto
gb_LTOPLUGINFLAGS := --plugin LLVMgold.so
endif
else
gb_LTOFLAGS := -flto$(if $(filter-out 0,$(PARALLELISM)),=$(PARALLELISM)) -fuse-linker-plugin -O2
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
gb_FilterOutClangCFLAGS += -fno-enforce-eh-specs
endif
endif

gb_PrecompiledHeader_EXCEPTIONFLAGS := $(gb_LinkTarget_EXCEPTIONFLAGS)

# optimization level
gb_COMPILERNOOPTFLAGS := -O0 -fstrict-aliasing -fstrict-overflow
gb_COMPILERDEBUGOPTFLAGS := -Og

ifeq ($(OS),ANDROID)
gb_DEBUGINFO_FLAGS=-glldb
# Clang does not know -ggdb2 or some other options
else ifeq ($(HAVE_GCC_GGDB2),TRUE)
gb_DEBUGINFO_FLAGS=-ggdb2
else
gb_DEBUGINFO_FLAGS=-g2
endif
gb_LINKER_DEBUGINFO_FLAGS=

ifeq ($(HAVE_GCC_SPLIT_DWARF),TRUE)
gb_DEBUGINFO_FLAGS+=-gsplit-dwarf
endif

ifeq ($(HAVE_CLANG_DEBUG_INFO_KIND_CONSTRUCTOR),TRUE)
gb_DEBUGINFO_FLAGS+=-Xclang -debug-info-kind=constructor
endif

ifeq ($(ENABLE_GDB_INDEX),TRUE)
gb_LINKER_DEBUGINFO_FLAGS += -Wl,--gdb-index
gb_DEBUGINFO_FLAGS += -ggnu-pubnames
endif

gb_LinkTarget_INCLUDE :=\
    $(SOLARINC) \
    -I$(BUILDDIR)/config_$(gb_Side) \

ifeq ($(COM_IS_CLANG),TRUE)
gb_COMPILER_TEST_FLAGS := -Xclang -plugin-arg-loplugin -Xclang --unit-test-mode
ifeq ($(COMPILER_PLUGIN_TOOL),)
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/clang/plugin.so -Xclang -add-plugin -Xclang loplugin
ifneq ($(COMPILER_PLUGIN_WARNINGS_ONLY),)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang \
    --warnings-only='$(COMPILER_PLUGIN_WARNINGS_ONLY)'
endif
else
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/clang/plugin.so -Xclang -plugin -Xclang loplugin $(foreach plugin,$(COMPILER_PLUGIN_TOOL), -Xclang -plugin-arg-loplugin -Xclang $(plugin))
ifneq ($(UPDATE_FILES),)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang --scope=$(UPDATE_FILES)
endif
endif
ifeq ($(COMPILER_PLUGINS_DEBUG),TRUE)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang --debug
endif
# set CCACHE_CPP2=1 to prevent clang generating spurious warnings
gb_COMPILER_SETUP += CCACHE_CPP2=1
gb_COMPILER_PLUGINS_SETUP := ICECC_EXTRAFILES=$(SRCDIR)/include/sal/log-areas.dox CCACHE_EXTRAFILES=$(SRCDIR)/include/sal/log-areas.dox SCCACHE_EXTRAFILES=$(SRCDIR)/include/sal/log-areas.dox
gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS := \
    -Xclang -plugin-arg-loplugin -Xclang --warnings-as-errors
else
# Set CCACHE_CPP2 to prevent GCC -Werror=implicit-fallthrough= when ccache strips comments from C
# code (which still needs /*fallthrough*/-style comments to silence that warning):
ifeq ($(ENABLE_WERROR),TRUE)
gb_COMPILER_SETUP += CCACHE_CPP2=1
endif
gb_COMPILER_TEST_FLAGS :=
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
else ifeq ($(OS_FOR_BUILD),HAIKU)
gb_Helper_LIBRARY_PATH_VAR := LIBRARY_PATH
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

ifneq ($(gb_ENABLE_PCH),)
# Enable use of .sum files for PCHs.
gb_COMPILER_SETUP += CCACHE_PCH_EXTSUM=1
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
