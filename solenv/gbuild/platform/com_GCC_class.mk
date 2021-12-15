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
		$(T_LTOFLAGS) \
		$(gb_AFLAGS) \
		$(if $(HAVE_ASM_END_BRANCH_INS_SUPPORT),-DEND_BRANCH_INS_SUPPORT) \
		-c $(3) \
		-o $(1)) \
		$(INCLUDE) && \
	echo "$(1) : $(3)" > $(4)
endef

# CObject class

# $(call gb_CObject__compiler,source,compiler)
define gb_CObject__compiler
	$(if $(filter %.c %.m,$(1)), \
		$(if $(2), $(2), $(gb_CC)), \
		$(if $(2), $(2), $(gb_CXX)))
endef

# When gb_LinkTarget_use_clang is used, filter out GCC flags that Clang doesn't know.
# $(call gb_CObject__filter_out_clang_cflags,cflags)
define gb_CObject__filter_out_clang_cflags
    $(filter-out $(gb_FilterOutClangCFLAGS),$(1))
endef

# $(call gb_CObject__command_pattern,object,flags,source,dep-file,compiler-plugins,compiler)
define gb_CObject__command_pattern
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && cd $(SRCDIR) && \
	$(gb_COMPILER_SETUP) \
	$(if $(5),$(gb_COMPILER_PLUGINS_SETUP)) \
	$(call gb_CObject__compiler,$(3),$(6)) \
		$(DEFS) \
		$(T_LTOFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(WARNINGS_NOT_ERRORS),$(if $(ENABLE_WERROR),$(if $(PLUGIN_WARNINGS_AS_ERRORS),$(gb_COMPILER_PLUGINS_WARNINGS_AS_ERRORS))),$(gb_CFLAGS_WERROR)) \
		$(if $(5),$(gb_COMPILER_PLUGINS)) \
		$(if $(COMPILER_TEST),-fsyntax-only -ferror-limit=0 -Xclang -verify) \
		$(if $(6), $(call gb_CObject__filter_out_clang_cflags,$(2)),$(2)) \
		$(if $(WARNINGS_DISABLED),$(gb_CXXFLAGS_DISABLE_WARNINGS)) \
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
# Workaround: Apple Clang version 12.0.5 sometimes tries to compile instead of generating PCH
# when used just with -c c++-header, so help it by being explicit.
gb_PrecompiledHeader_emit_pch := -Xclang -emit-pch
else
gb_PrecompiledHeader_get_enableflags = \
-include $(dir $(call gb_PrecompiledHeader_get_target,$(1),$(2)))$(notdir $(subst .gch,,$(call gb_PrecompiledHeader_get_target,$(1),$(2))))
gb_PrecompiledHeader_EXT := .gch
gb_PrecompiledHeader_emit_pch :=
endif

gb_PrecompiledHeader_extra_pch_cxxflags += $(PCH_INSTANTIATE_TEMPLATES)

# Clang supports building extra object file where it puts code that would be shared by all users of the PCH.
# Unlike with MSVC it is built as a separate step. The relevant options are used only when generating the PCH
# and when creating the PCH's object file, normal compilations using the PCH do not need extra options.
gb_PrecompiledHeader_pch_with_obj += $(BUILDING_PCH_WITH_OBJ)
gb_PrecompiledHeader_extra_pch_cxxflags += $(BUILDING_PCH_WITH_OBJ)
ifneq ($(BUILDING_PCH_WITH_OBJ),)
# If using Clang's PCH extra object, we may need to strip unused sections, otherwise inline and template functions
# emitted in that object may in some cases cause unresolved references to private symbols in other libraries.
gb_LinkTarget_LDFLAGS += $(LD_GC_SECTIONS)
gb_PrecompiledHeader_pch_with_obj += -ffunction-sections -fdata-sections
# Enable generating more shared code and debuginfo in the PCH object file.
gb_PrecompiledHeader_extra_pch_cxxflags += $(PCH_DEBUGINFO)
ifeq ($(ENABLE_OPTIMIZED),)
# -fmodules-codegen appears to be worth it only if not optimizing, otherwise optimizing all the functions emitted
# in the PCH object file may take way too long, especially given that many of those may get thrown away
gb_PrecompiledHeader_extra_pch_cxxflags += $(PCH_CODEGEN)
endif
endif

# This is for MSVC's object file built directly as a side-effect of building the PCH.
gb_PrecompiledHeader_get_objectfile =

# $(call gb_PrecompiledHeader__command,pchfile,pchtarget,source,cxxflags,includes,linktargetmakefilename,compiler)
define gb_PrecompiledHeader__command
$(call gb_Output_announce,$(2),$(true),PCH,1)
	$(call gb_Trace_StartRange,$(2),PCH)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(call gb_PrecompiledHeader_get_dep_target,$(2),$(6))) && \
	cd $(BUILDDIR)/ && \
	CCACHE_DISABLE=1 $(gb_COMPILER_SETUP) \
	$(if $(7),$(7),$(gb_CXX)) \
		-x c++-header $(gb_PrecompiledHeader_emit_pch) \
		$(if $(7), $(call gb_CObject__filter_out_clang_cflags,$(4)),$(4)) \
		$(if $(WARNINGS_DISABLED),$(gb_CXXFLAGS_DISABLE_WARNINGS)) \
		$(gb_COMPILERDEPFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		$(gb_NO_PCH_TIMESTAMP) \
		$(gb_PrecompiledHeader_extra_pch_cxxflags) \
		$(5) \
		$(call gb_cxx_dep_generation_options,$(1),$(call gb_PrecompiledHeader_get_dep_target_tmp,$(2),$(6))) \
		-c $(patsubst %.cxx,%.hxx,$(3)) \
		-o$(1) \
		$(call gb_cxx_dep_copy,$(call gb_PrecompiledHeader_get_dep_target_tmp,$(2),$(6))) \
		)
	$(call gb_Trace_EndRange,$(2),PCH)
endef

ifeq ($(COM_IS_CLANG),TRUE)
# Clang has -fno-pch-timestamp, just checksum the file for CCACHE_PCH_EXTSUM
# $(call gb_PrecompiledHeader__sum_command,pchfile,pchtarget,source,cxxflags,includes,linktargetmakefilename,compiler)
define gb_PrecompiledHeader__sum_command
	$(SHA256SUM) $(1) >$(1).sum
endef
else
# GCC does not generate the same .gch for the same input, so checksum the (preprocessed) input
# $(call gb_PrecompiledHeader__sum_command,pchfile,pchtarget,source,cxxflags,includes,linktargetmakefilename,compiler)
define gb_PrecompiledHeader__sum_command
$(call gb_Helper_abbreviate_dirs,\
	CCACHE_DISABLE=1 $(gb_COMPILER_SETUP) \
	$(if $(7),$(7),$(gb_CXX)) \
		-x c++-header \
		$(4) \
		$(if $(WARNINGS_DISABLED),$(gb_CXXFLAGS_DISABLE_WARNINGS)) \
		$(gb_COMPILERDEPFLAGS) \
		$(if $(VISIBILITY),,$(gb_VISIBILITY_FLAGS)) \
		$(if $(EXTERNAL_CODE),$(gb_CXXFLAGS_Wundef),$(gb_DEFS_INTERNAL)) \
		$(gb_NO_PCH_TIMESTAMP) \
		$(5) \
		-E $(patsubst %.cxx,%.hxx,$(3)) \
		-o- \
		| $(SHA256SUM) >$(1).sum \
		)
endef
endif

# not needed
gb_PrecompiledHeader__create_reuse_files =
gb_PrecompiledHeader__copy_reuse_files =

# YaccTarget class

define gb_YaccTarget__command
$(call gb_Output_announce,$(2),$(true),YAC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(3)) && \
	$(BISON) $(T_YACCFLAGS) -v --defines=$(4) -o $(5) $(1) && touch $(3) )

endef

# CppunitTest class

ifeq ($(strip $(DEBUGCPPUNIT)),TRUE)
ifeq ($(strip $(CPPUNITTRACE)),)
CPPUNITTRACE := gdb --args
endif
ifeq ($(filter gdb,$(CPPUNITTRACE)),)
$(error For DEBUGCPPUNIT=TRUE, CPPUNITTRACE must be a command that runs gdb)
endif
gb_CppunitTest_DEBUGCPPUNIT := -nx --batch --command=$(SRCDIR)/solenv/bin/gdbtrycatchtrace-stdout
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
	$(gb_UIConfig_LXML_PATH) $(if $(SYSTEM_LIBXML)$(SYSTEM_LIBXSLT),,$(gb_Helper_set_ld_path)) \
	$(call gb_ExternalExecutable_get_command,python) \
	$(gb_UIConfig_gla11y_SCRIPT) $(gb_UIConfig_gla11y_PARAMETERS) -o $@ $(UIFILES)
)

endef

# vim: set noet sw=4 ts=4:
