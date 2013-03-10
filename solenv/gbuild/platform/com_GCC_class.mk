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

ifeq ($(gb_FULLDEPS),$(true))
define gb_cxx_dep_generation_options
-MMD -MT $(1) -MP -MF $(2)_
endef
define gb_cxx_dep_copy
&& mv $(1)_ $(1)
endef
else
define gb_cxx_dep_generation_options
endef
define gb_cxx_dep_copy
endef
endif

# AsmObject class

gb_AsmObject_get_source = $(1)/$(2).s

# $(call gb_AsmObject__command,object,relative-source,source,dep-file)
define gb_AsmObject__command
$(call gb_Output_announce,$(2),$(true),ASM,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && cd $(SRCDIR) && \
	$(gb_CC) \
		$(gb_LTOFLAGS) \
		$(gb_AFLAGS) \
		-c $(3) \
		-o $(1)) \
		$(INCLUDE) && \
	echo "$(1) : $(3)" > $(4)
endef

# CObject class

# $(call gb_CObject__command,object,relative-source,source,dep-file)
define gb_CObject__command
$(call gb_Output_announce,$(2).c,$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && cd $(SRCDIR) && \
	$(if $(COMPILER_PLUGINS),$(gb_COMPILER_PLUGINS_SETUP)) \
	$(gb_CC) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(if $(COMPILER_PLUGINS),$(gb_COMPILER_PLUGINS)) \
		$(T_CFLAGS) $(T_CFLAGS_APPEND) \
		-c $(3) \
		-o $(1) \
		$(call gb_cxx_dep_generation_options,$(1),$(4)) \
		-I$(dir $(3)) \
		$(INCLUDE) \
		$(call gb_cxx_dep_copy,$(4)) \
		)
endef

# Used to run a compiler plugin tool.
# $(call gb_CObject__tool_command,relative-source,source)
define gb_CObject__tool_command
$(call gb_Output_announce,$(1).c,$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
        ICECC=no CCACHE_DISABLE=1 \
	$(gb_CC) \
		$(DEFS) \
		$(T_CFLAGS) $(T_CFLAGS_APPEND) \
		-c $(2) \
		-I$(dir $(2)) \
		$(INCLUDE) \
		$(gb_COMPILER_PLUGINS) \
		)
endef

# CxxObject class

# $(call gb_CxxObject__command,object,relative-source,source,dep-file)
define gb_CxxObject__command
$(call gb_Output_announce,$(2).cxx,$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && cd $(SRCDIR) && \
	$(if $(COMPILER_PLUGINS),$(gb_COMPILER_PLUGINS_SETUP)) \
	$(gb_CXX) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CXXFLAGS_WERROR)) \
		$(if $(COMPILER_PLUGINS),$(gb_COMPILER_PLUGINS)) \
		$(T_CXXFLAGS) $(T_CXXFLAGS_APPEND) \
		-c $(3) \
		-o $(1) \
		$(call gb_cxx_dep_generation_options,$(1),$(4)) \
		-I$(dir $(3)) \
		$(INCLUDE) \
		$(PCHFLAGS) \
		$(call gb_cxx_dep_copy,$(4)) \
		)
endef

# Used to run a compiler plugin tool.
# $(call gb_CxxObject__tool_command,relative-source,source)
define gb_CxxObject__tool_command
$(call gb_Output_announce,$(1).cxx,$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs,\
        ICECC=no CCACHE_DISABLE=1 \
	$(gb_CXX) \
		$(DEFS) \
		$(T_CXXFLAGS) $(T_CXXFLAGS_APPEND) \
		-c $(2) \
		-I$(dir $(2)) \
		$(INCLUDE) \
		$(gb_COMPILER_PLUGINS) \
		)
endef

# ObjCxxObject class

define gb_ObjCxxObject__command
$(call gb_Output_announce,$(2).mm,$(true),OCX,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CXX) \
		$(DEFS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CXXFLAGS_WERROR)) \
		$(T_OBJCXXFLAGS) $(T_OBJCXXFLAGS_APPEND) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(1) \
		-MP -MF $(4) \
		-I$(dir $(3)) \
		$(INCLUDE))
endef

# ObjCObject class

define gb_ObjCObject__command
$(call gb_Output_announce,$(2).m,$(true),OCC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CC) \
		$(DEFS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(T_OBJCFLAGS) $(T_OBJCFLAGS_APPEND) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(1) \
		-MP -MF $(4) \
		-I$(dir $(3)) \
		$(INCLUDE))
endef

define gb_SrsPartTarget__command_dep
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(call gb_SrsPartTarget_get_dep_target,$(1))) && cd $(SRCDIR) && \
	$(gb_GCCP) \
		-MM -MT $(call gb_SrsPartTarget_get_target,$(1)) \
		$(INCLUDE) \
		$(DEFS) \
		-c -x c++-header $(2) \
		-o $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef

# PrecompiledHeader class

ifeq ($(COM_GCC_IS_CLANG),TRUE)
gb_PrecompiledHeader_get_enableflags = -include-pch $(call gb_PrecompiledHeader_get_target,$(1))
else
gb_PrecompiledHeader_get_enableflags = -include $(notdir $(subst .gch,,$(call gb_PrecompiledHeader_get_target,$(1)))) \
				       -I $(dir $(call gb_PrecompiledHeader_get_target,$(1)))
endif

# Clang and gcc do not need any extra .o file for PCH
gb_PrecompiledHeader_get_objectfile =

define gb_PrecompiledHeader__command
$(call gb_Output_announce,$(2),$(true),PCH,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(call gb_PrecompiledHeader_get_dep_target,$(2))) && \
	$(gb_CXX) \
		-x c++-header \
		$(4) $(5) \
		$(gb_COMPILERDEPFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(6) \
		$(call gb_cxx_dep_generation_options,$(1),$(call gb_PrecompiledHeader_get_dep_target,$(2))) \
		-c $(patsubst %.cxx,%.hxx,$(3)) \
		-o$(1) \
		$(call gb_cxx_dep_copy,$(call gb_PrecompiledHeader_get_dep_target,$(2))) \
		)
endef

# NoexPrecompiledHeader class

ifeq ($(COM_GCC_IS_CLANG),TRUE)
gb_NoexPrecompiledHeader_get_enableflags = -include-pch $(call gb_NoexPrecompiledHeader_get_target,$(1))
else
gb_NoexPrecompiledHeader_get_enableflags = -include $(notdir $(subst .gch,,$(call gb_NoexPrecompiledHeader_get_target,$(1)))) \
				       -I $(dir $(call gb_NoexPrecompiledHeader_get_target,$(1)))
endif

gb_NoexPrecompiledHeader_get_objectfile =

define gb_NoexPrecompiledHeader__command
$(call gb_Output_announce,$(2),$(true),PCH,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(call gb_NoexPrecompiledHeader_get_dep_target,$(2))) && \
	$(gb_CXX) \
		-x c++-header \
		$(4) $(5) \
		$(gb_COMPILERDEPFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(6) \
		$(call gb_cxx_dep_generation_options,$(1),$(call gb_NoexPrecompiledHeader_get_dep_target,$(2))) \
		-c $(patsubst %.cxx,%.hxx,$(3)) \
		-o$(1) \
		$(call gb_cxx_dep_copy,$(call gb_NoexPrecompiledHeader_get_dep_target,$(2))) \
		)
endef

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

# vim: set noet sw=4 ts=4:
