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

# YaccTarget class

ifeq ($(ANCIENT_BISON),YES)
#
# There are ancient versions of bison out there, which do not handle
# well .cxx extensions, nor do they support --defines=<file>. The
# result is that the header is named <foo>.cxx.h instead of <foo>.hxx
# so we queue a mv to rename the header accordingly.
# One example is XCode versions 2.x, which are used on OSX ppc
# machines.
#
define gb_YaccTarget__command
$(call gb_Output_announce,$(2),$(true),YAC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(3)) && \
	$(gb_YACC) $(T_YACCFLAGS) -d -o $(5) $(1) && mv $(5).h $(4) && touch $(3) )

endef

else
define gb_YaccTarget__command
$(call gb_Output_announce,$(2),$(true),YAC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(3)) && \
	$(gb_YACC) $(T_YACCFLAGS) --defines=$(4) -o $(5) $(1) && touch $(3) )

endef
endif

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

gb_COMPILERDEFS := \
	-D$(COM) \
	-DCPPU_ENV=gcc3 \
	-DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH) \

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

ifeq ($(gb_PRODUCT),$(true))
# Clang doesn't have this option
ifneq ($(COM_GCC_IS_CLANG),TRUE)
gb_LinkTarget_EXCEPTIONFLAGS += \
	-fno-enforce-eh-specs
endif
endif

gb_LinkTarget_NOEXCEPTIONFLAGS := \
	-DEXCEPTIONS_OFF \
	-fno-exceptions \


# optimization level
ifneq ($(gb_DEBUGLEVEL),0)
gb_COMPILEROPTFLAGS := -O0
else
gb_COMPILEROPTFLAGS := $(gb_COMPILERDEFAULTOPTFLAGS)
endif
gb_COMPILERNOOPTFLAGS := -O0

gb_LinkTarget_INCLUDE := $(filter-out %/stl, $(subst -I. , ,$(SOLARINC)))
gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))



ifeq ($(OS_FOR_BUILD),MACOSX)
gb_Helper_LIBRARY_PATH_VAR := DYLD_LIBRARY_PATH
else ifeq ($(OS_FOR_BUILD),WNT)
# In theory possible if cross-compiling to some Unix from Windows,
# in practice strongly discouraged to even try that
gb_Helper_LIBRARY_PATH_VAR := PATH
else
gb_Helper_LIBRARY_PATH_VAR := LD_LIBRARY_PATH
endif

gb_Helper_set_ld_path := $(gb_Helper_LIBRARY_PATH_VAR)="$(OUTDIR_FOR_BUILD)/lib"

# $(1): list of directory pathnames to append at the end of the ld path
define gb_Helper_extend_ld_path
$(gb_Helper_set_ld_path)$(foreach dir,$(1),:$(dir))
endef


# convert parameters filesystem root to native notation
# does some real work only on windows, make sure not to
# break the dummy implementations on unx*
define gb_Helper_convert_native
$(1)
endef

# Convert path to native notation
define gb_Helper_native_path
$(1)
endef

gb_Helper_OUTDIRLIBDIR := $(OUTDIR)/lib
gb_Helper_OUTDIR_FOR_BUILDLIBDIR := $(OUTDIR_FOR_BUILD)/lib

gb_Helper_abbreviate_dirs_native = $(gb_Helper_abbreviate_dirs)

gb_Helper_get_rcfile = $(1)rc
