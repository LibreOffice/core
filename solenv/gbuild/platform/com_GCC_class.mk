# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
		-x assembler-with-cpp \
		$(gb_LTOFLAGS) \
		$(gb_AFLAGS) \
		-c $(3) \
		-o $(1)) \
		$(INCLUDE) && \
	echo "$(1) : $(3)" > $(4)
endef

# CObject class

# $(call gb_CObject__command_pattern,object,flags,source,dep-file,compiler-plugins)
define gb_CObject__command_pattern
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && cd $(SRCDIR) && \
	$(if $(5),$(gb_COMPILER_PLUGINS_SETUP)) \
	$(if $(filter %.c %.m,$(3)), $(gb_CC), $(gb_CXX)) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(if $(5),$(gb_COMPILER_PLUGINS)) \
		$(2) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
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
# $(call gb_CObject__tool_command,relative-source,source)
define gb_CObject__tool_command
$(call gb_Output_announce,$(1).c,$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
        ICECC=no CCACHE_DISABLE=1 \
	$(gb_CC) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(gb_COMPILER_PLUGINS) \
		$(T_CFLAGS) $(T_CFLAGS_APPEND) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		-c $(2) \
		-I$(dir $(2)) \
		$(INCLUDE) \
		)
endef
define gb_ObjCObject__tool_command
$(call gb_Output_announce,$(1).m,$(true),OCC,3)
$(call gb_Helper_abbreviate_dirs,\
        ICECC=no CCACHE_DISABLE=1 \
	$(gb_CC) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(gb_COMPILER_PLUGINS) \
		$(T_OBJCFLAGS) $(T_OBJCFLAGS_APPEND) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		-c $(2) \
		-I$(dir $(2)) \
		$(INCLUDE) \
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
		$(gb_LTOFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(gb_COMPILER_PLUGINS) \
		$(T_CXXFLAGS) $(T_CXXFLAGS_APPEND) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		-c $(2) \
		-I$(dir $(2)) \
		$(INCLUDE) \
		)
endef
define gb_ObjCxxObject__tool_command
$(call gb_Output_announce,$(1).mm,$(true),OCX,3)
$(call gb_Helper_abbreviate_dirs,\
        ICECC=no CCACHE_DISABLE=1 \
	$(gb_CXX) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(gb_COMPILER_PLUGINS) \
		$(T_OBJCXXFLAGS) $(T_OBJCXXFLAGS_APPEND) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		-c $(2) \
		-I$(dir $(2)) \
		$(INCLUDE) \
		)
endef

define gb_SrsPartTarget__command_dep
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(call gb_SrsPartTarget_get_dep_target,$(1))) && cd $(SRCDIR) && \
	$(gb_GCCP) \
		-MM -MP -MT $(call gb_SrsPartTarget_get_target,$(1)) \
		$(INCLUDE) \
		$(DEFS) \
		-c -x c++-header $(2) \
		-o $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef

# PrecompiledHeader class

ifeq ($(COM_IS_CLANG),TRUE)
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
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		$(6) \
		$(call gb_cxx_dep_generation_options,$(1),$(call gb_PrecompiledHeader_get_dep_target_tmp,$(2))) \
		-c $(patsubst %.cxx,%.hxx,$(3)) \
		-o$(1) \
		$(call gb_cxx_dep_copy,$(call gb_PrecompiledHeader_get_dep_target_tmp,$(2))) \
		)
endef

# YaccTarget class

define gb_YaccTarget__command
$(call gb_Output_announce,$(2),$(true),YAC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(3)) && \
	$(gb_YACC) $(T_YACCFLAGS) -v --defines=$(4) -o $(5) $(1) && touch $(3) )

endef

# CppunitTest class

ifeq ($(strip $(DEBUGCPPUNIT)),TRUE)
gb_CppunitTest_GDBTRACE := gdb -nx -ex "add-auto-load-safe-path $(INSTDIR)" --batch --command=$(SRCDIR)/solenv/bin/gdbtrycatchtrace-stdout -return-child-result --args
endif

# ExternalProject class

gb_ExternalProject_use_autoconf :=
gb_ExternalProject_use_nmake :=

# StaticLibrary class

gb_StaticLibrary_get_filename = lib$(1).a
gb_StaticLibrary_PLAINEXT := .a
gb_StaticLibrary_StaticLibrary_platform :=

gb_LinkTarget_get_linksearchpath_for_layer = \
	-L$(WORKDIR)/LinkTarget/StaticLibrary \
	-L$(call gb_Library_get_sdk_link_dir) \
	$(foreach layer,\
		$(subst +, ,$(patsubst $(1):%.,%,\
			$(filter $(1):%.,$(gb_LinkTarget_LAYER_LINKPATHS)))),\
		$(patsubst $(layer):%,-L%,\
			$(filter $(layer):%,$(gb_Library_LAYER_DIRS))))


gb_ICU_PRECOMMAND := $(call gb_Helper_extend_ld_path,$(WORKDIR_FOR_BUILD)/UnpackedTarball/icu/source/lib)

# UIConfig class

# Mac OS X sort(1) cannot read a response file
define gb_UIConfig__command
$(call gb_Helper_abbreviate_dirs,\
	sort -u $(UI_IMAGELISTS) /dev/null > $@ \
)

endef

# vim: set noet sw=4 ts=4:
