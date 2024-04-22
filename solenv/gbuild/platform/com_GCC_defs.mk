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

gb_COMPILEROPTFLAGS += $(HARDENING_OPT_CFLAGS)

gb_AFLAGS := $(AFLAGS)

gb_COMPILERDEFS := \
	-DBOOST_SYSTEM_NO_DEPRECATED \
	-DCPPU_ENV=$(CPPU_ENV) \
	$(if $(filter EMSCRIPTEN,$(OS)),-U_FORTIFY_SOURCE) \

# enable debug STL
ifeq ($(ENABLE_DBGUTIL),TRUE)
ifneq ($(HAVE_LIBSTDCPP),)
gb_COMPILERDEFS_STDLIB_DEBUG = -D_GLIBCXX_DEBUG
else
ifneq ($(LIBCPP_DEBUG),)
gb_COMPILERDEFS_STDLIB_DEBUG = $(LIBCPP_DEBUG)
endif
endif
gb_COMPILERDEFS += $(gb_COMPILERDEFS_STDLIB_DEBUG)
endif

gb_CFLAGS_COMMON := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wstrict-prototypes \
	-Wundef \
	-Wunreachable-code \
	$(if $(or $(and $(COM_IS_CLANG),$(or $(findstring icecc,$(CC)),$(findstring icecc,$(CCACHE_PREFIX)))),$(findstring sccache,$(CC))),,-Wunused-macros) \
	$(if $(COM_IS_CLANG),-Wembedded-directive) \
	$(if $(COM_IS_CLANG),-Wshadow-all) \
	-finput-charset=UTF-8 \
	-fmessage-length=0 \
	-fno-common \
	-pipe \
	$(if $(ENABLE_HARDENING_FLAGS),$(HARDENING_CFLAGS)) \
	$(if $(filter EMSCRIPTEN,$(OS)),-fno-stack-protector,-fstack-protector-strong) \

gb_CXXFLAGS_COMMON := \
	-Wall \
	-Wno-missing-braces \
	-Wendif-labels \
	-Wextra \
	-Wundef \
	-Wunreachable-code \
	-Wshadow \
	$(if $(or $(and $(COM_IS_CLANG),$(or $(findstring icecc,$(CXX)),$(findstring icecc,$(CCACHE_PREFIX)))),$(findstring sccache,$(CXX))),,-Wunused-macros) \
	$(if $(COM_IS_CLANG),-Wembedded-directive) \
	-finput-charset=UTF-8 \
	-fmessage-length=0 \
	-fno-common \
	-pipe \
	$(if $(ENABLE_HARDENING_FLAGS),$(HARDENING_CFLAGS)) \
	$(if $(filter EMSCRIPTEN,$(OS)),-fno-stack-protector,-fstack-protector-strong) \

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

gb_CXXFLAGS_include := -include$(gb_SPACE)

ifeq ($(strip $(gb_GCOV)),YES)
gb_CFLAGS_COMMON += -fprofile-arcs -ftest-coverage
gb_CXXFLAGS_COMMON += -fprofile-arcs -ftest-coverage
gb_LinkTarget_LDFLAGS += -fprofile-arcs -lgcov
gb_COMPILEROPTFLAGS := -O0
endif

ifeq ($(DISABLE_DYNLOADING),TRUE)
gb_CFLAGS_COMMON += -ffunction-sections -fdata-sections
gb_CXXFLAGS_COMMON += -ffunction-sections -fdata-sections
ifneq ($(OS),EMSCRIPTEN)
gb_LinkTarget_LDFLAGS += -Wl,--gc-sections
endif
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

gb_LinkTarget_LDFLAGS += $(if $(filter EMSCRIPTEN,$(OS)),-fno-stack-protector,-fstack-protector-strong)

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
    -Wno-variadic-macros -Wno-deprecated-declarations
endif

ifeq ($(ENABLE_LTO),TRUE)
ifeq ($(COM_IS_CLANG),TRUE)
gb_LTOFLAGS := -flto=thin
ifeq (,$(index,iOS MACOSX,$(OS)))
gb_LTOPLUGINFLAGS := --plugin $(if $(LD_PLUGIN),$(LD_PLUGIN),LLVMgold.so)
endif
else
# use parallelism based on make's job handling
gb_LTOFLAGS := -flto=jobserver -fuse-linker-plugin -O2
# clang does not support -flto=<number>
gb_CLANG_LTOFLAGS := -flto=thin
endif
endif

gb_LinkTarget_EXCEPTIONFLAGS := \
	-fexceptions

ifeq ($(ENABLE_DBGUTIL),)
# Clang doesn't have this option
ifeq ($(HAVE_GCC_FNO_ENFORCE_EH_SPECS),TRUE)
gb_LinkTarget_EXCEPTIONFLAGS += \
	-fno-enforce-eh-specs
gb_FilterOutClangCFLAGS += -fno-enforce-eh-specs
endif
endif

gb_PrecompiledHeader_EXCEPTIONFLAGS := $(gb_LinkTarget_EXCEPTIONFLAGS)

# We turn on and off this one depending on whether icecream and/or ccache are used,
# and changing cxxflags cause PCH rebuilds, so e.g. a plain temporary 'CCACHE_DISABLE=1'
# would cause a rebuild. Ignore the flag there, it's irrelevant for PCH use anyway.
gb_PrecompiledHeader_ignore_flags_for_flags_file := -Wunused-macros

# optimization level
gb_COMPILERNOOPTFLAGS := -O0 -Wp,-U_FORTIFY_SOURCE -fstrict-aliasing -fstrict-overflow
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

ifeq ($(HAVE_EXTERNAL_DWARF),TRUE)
gb_DEBUGINFO_FLAGS+=-gsplit-dwarf
# GCC 11 defaults to -gdwarf-5, which GDB 10 doesn't support in split debug info
ifeq ($(COM_IS_CLANG),)
gb_DEBUGINFO_FLAGS+=-gdwarf-4
endif
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
gb_COMPILER_PLUGINS := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/clang/plugin.so -Xclang -add-plugin -Xclang loplugin
ifneq ($(COMPILER_PLUGIN_WARNINGS_ONLY),)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang \
    --warnings-only='$(COMPILER_PLUGIN_WARNINGS_ONLY)'
endif
ifeq ($(COMPILER_PLUGINS_DEBUG),TRUE)
gb_COMPILER_PLUGINS += -Xclang -plugin-arg-loplugin -Xclang --debug
endif
gb_COMPILER_PLUGINS_TOOL := -Xclang -load -Xclang $(BUILDDIR)/compilerplugins/clang/plugin.so -Xclang -plugin -Xclang loplugin $(foreach plugin,$(COMPILER_PLUGIN_TOOL), -Xclang -plugin-arg-loplugin -Xclang $(plugin))
ifneq ($(UPDATE_FILES),)
gb_COMPILER_PLUGINS_TOOL += -Xclang -plugin-arg-loplugin -Xclang --scope=$(UPDATE_FILES)
endif
ifeq ($(COMPILER_PLUGINS_DEBUG),TRUE)
gb_COMPILER_PLUGINS_TOOL += -Xclang -plugin-arg-loplugin -Xclang --debug
endif

# Set CCACHE_CPP2=1 to prevent Clang generating spurious warnings.

# For Emscripten, the emcc command is a Python script that outputs annoying warnings like
# .../emscripten/tools/building.py:638: ResourceWarning: unclosed file <_io.TextIOWrapper name='/tmp/emscripten_temp_0fvvg__1/conftest.js.jso.js' mode='w' encoding='utf-8'>
# into stderr, which makes a configure script think that there is a problem in
# compiling even a microscopic test program with an option like -Werror which
# surely *is* supported. Avoid this by setting PYTHONWARNINGS=ignore.

gb_COMPILER_SETUP += CCACHE_CPP2=1 $(if $(filter EMSCRIPTEN,$(OS)),PYTHONWARNINGS=ignore)
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
gb_COMPILER_PLUGINS_TOOL :=
gb_COMPILER_PLUGINS_SETUP :=
gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS :=
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
endif

# vim: set noet sw=4:
