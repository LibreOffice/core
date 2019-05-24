# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(gb_FULLDEPS),$(true))
ifneq (,$(CCACHE_HARDLINK))
# cannot move hardlink over itself, so create dep file directly, even if that
# might leave a broken file behind in case the build is interrupted forcefully
define gb_cxx_dep_generation_options
-MMD -MT $(1) -MP -MF $(2)
endef
define gb_cxx_dep_copy
endef
else
define gb_cxx_dep_generation_options
-MMD -MT $(1) -MP -MF $(2)_
endef
define gb_cxx_dep_copy
&& mv $(1)_ $(1)
endef
endif
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

# $(call gb_CObject__command_pattern,object,flags,source,dep-file,compiler-plugins,symbols)
define gb_CObject__command_pattern
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && cd $(SRCDIR) && \
	$(gb_COMPILER_SETUP) \
	$(if $(5),$(gb_COMPILER_PLUGINS_SETUP)) \
	$(if $(filter %.c %.m,$(3)), $(gb_CC), $(gb_CXX)) \
		$(DEFS) \
		$(gb_LTOFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),$(if $(ENABLE_WERROR),$(if $(PLUGIN_WARNINGS_AS_ERRORS),$(gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS))),$(gb_CFLAGS_WERROR)) \
		$(if $(5),$(gb_COMPILER_PLUGINS)) \
		$(if $(COMPILER_TEST),-fsyntax-only -ferror-limit=0 -Xclang -verify) \
		$(if $(WARNINGS_DISABLED),$(call gb_Helper_disable_warnings,$(2)),$(2)) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		-c $(3) \
		-o $(1) \
		$(if $(COMPILER_TEST),,$(call gb_cxx_dep_generation_options,$(1),$(4))) \
		$(INCLUDE) \
		$(PCHFLAGS) \
		$(if $(COMPILER_TEST),,$(call gb_cxx_dep_copy,$(4))) \
		)
endef

# PrecompiledHeader class

ifeq ($(COM_IS_CLANG),TRUE)
gb_PrecompiledHeader_get_enableflags = -include-pch $(call gb_PrecompiledHeader_get_target,$(1),$(2))
gb_PrecompiledHeader_EXT := .pch
else
gb_PrecompiledHeader_get_enableflags = \
-include $(dir $(call gb_PrecompiledHeader_get_target,$(1),$(2)))$(notdir $(subst .gch,,$(call gb_PrecompiledHeader_get_target,$(1),$(2))))
gb_PrecompiledHeader_EXT := .gch
endif

# Clang and gcc do not need any extra .o file for PCH
gb_PrecompiledHeader_get_objectfile =

define gb_PrecompiledHeader__command
$(call gb_Output_announce,$(2),$(true),PCH,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(call gb_PrecompiledHeader_get_dep_target,$(2),$(7))) && \
	CCACHE_DISABLE=1 $(gb_COMPILER_SETUP) \
	$(gb_CXX) \
		-x c++-header \
		$(if $(WARNINGS_DISABLED),$(call gb_Helper_disable_warnings,$(4) $(5)),$(4) $(5)) \
		$(gb_COMPILERDEPFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		$(gb_NO_PCH_TIMESTAMP) \
		$(6) \
		$(call gb_cxx_dep_generation_options,$(1),$(call gb_PrecompiledHeader_get_dep_target_tmp,$(2),$(7))) \
		-c $(patsubst %.cxx,%.hxx,$(3)) \
		-o$(1) \
		$(call gb_cxx_dep_copy,$(call gb_PrecompiledHeader_get_dep_target_tmp,$(2),$(7))) \
		)
endef

ifeq ($(COM_IS_CLANG),TRUE)
# Clang has -fno-pch-timestamp, just checksum the file for CCACHE_PCH_EXTSUM
define gb_PrecompiledHeader__sum_command
	$(SHA256SUM) $(1) >$(1).sum
endef
else
# GCC does not generate the same .gch for the same input, so checksum the (preprocessed) input
define gb_PrecompiledHeader__sum_command
$(call gb_Helper_abbreviate_dirs,\
	CCACHE_DISABLE=1 $(gb_COMPILER_SETUP) \
	$(gb_CXX) \
		-x c++-header \
		$(if $(WARNINGS_DISABLED),$(call gb_Helper_disable_warnings,$(4) $(5)),$(4) $(5)) \
		$(gb_COMPILERDEPFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		$(gb_NO_PCH_TIMESTAMP) \
		$(6) \
		-E $(patsubst %.cxx,%.hxx,$(3)) \
		-o- \
		| $(SHA256SUM) >$(1).sum \
		)
endef
endif


# YaccTarget class

define gb_YaccTarget__command
$(call gb_Output_announce,$(2),$(true),YAC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(3)) && \
	$(gb_YACC) $(T_YACCFLAGS) -v --defines=$(4) -o $(5) $(1) && touch $(3) )

endef

# CppunitTest class

ifeq ($(strip $(DEBUGCPPUNIT)),TRUE)
gb_CppunitTest_GDBTRACE := gdb -nx -ex "add-auto-load-safe-path $(INSTDIR)" -ex "set environment $(subst =, ,$(gb_CppunitTest_CPPTESTPRECOMMAND))" --batch --command=$(SRCDIR)/solenv/bin/gdbtrycatchtrace-stdout -return-child-result --args
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

# macOS sort(1) cannot read a response file
define gb_UIConfig__command
$(call gb_Helper_abbreviate_dirs,\
	$(SORT) -u $(UI_IMAGELISTS) /dev/null > $@ \
)

endef

define gb_UIConfig__gla11y_command
$(call gb_Helper_abbreviate_dirs,\
	$(gb_UIConfig_LXML_PATH) $(gb_Helper_set_ld_path) \
	$(call gb_ExternalExecutable_get_command,python) \
	$(gb_UIConfig_gla11y_SCRIPT) $(gb_UIConfig_gla11y_PARAMETERS) -o $@ $(UIFILES)
)

endef

# vim: set noet sw=4 ts=4:
