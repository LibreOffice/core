# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# outdir target pattern

gb_CliLibrary_get_target = $(gb_Helper_OUTDIRLIBDIR)/$(1)$(gb_CliLibrary_EXT)
gb_CliNativeLibrary_get_target = $(gb_Helper_OUTDIRLIBDIR)/$(1)$(gb_CliNativeLibrary_EXT)
gb_CliUnoApi_get_target = $(gb_Helper_OUTDIRLIBDIR)/$(1)$(gb_CliUnoApi_EXT)
gb_Pagein_get_outdir_target = $(OUTDIR)/bin/pagein-$(1)
gb_PackagePart_get_destinations = \
	$(INSTDIR) \
	$(OUTDIR)/bin \
	$(OUTDIR)/lib \
	$(OUTDIR)/par \
	$(OUTDIR)/pck \
	$(OUTDIR)/res \
	$(OUTDIR)/share \
	$(OUTDIR)/unittest \
	$(OUTDIR)/xml \

gb_InstallScript_get_target = $(OUTDIR)/bin/$(1)$(gb_InstallScript_EXT)
gb_ResTarget_get_outdir_target = $(OUTDIR)/bin/$(1).res
gb_ResTarget_get_unittest_target = \
    $(OUTDIR)/unittest/install/$(LIBO_SHARE_RESOURCE_FOLDER)/$(1).res
gb_UnoApi_get_target = $(OUTDIR)/bin/$(1).rdb
gb_Jar_get_outdir_target = $(OUTDIR)/bin/$(1).jar
gb_Configuration_registry = $(OUTDIR)/xml/registry
gb_XcsTarget_get_outdir_target = \
    $(gb_Configuration_registry)/schema$(if $(1),/)$(1)
gb_XcuDataTarget_get_outdir_target = $(gb_Configuration_registry)/data/$(1)
gb_XcuLangpackTarget_get_outdir_target = $(gb_Configuration_registry)/spool/$(1)
gb_XcuModuleTarget_get_outdir_target = $(gb_Configuration_registry)/spool/$(1)

define gb_Executable_get_target
$(patsubst $(1):%,$(OUTDIR)/bin/%,$(filter $(1):%,$(gb_Executable_FILENAMES)))
endef

define gb_Executable_get_target_for_build
$(patsubst $(1):%,$(OUTDIR_FOR_BUILD)/bin/%,$(filter $(1):%,$(gb_Executable_FILENAMES_FOR_BUILD)))
endef

define gb_Library_get_target
$(patsubst $(1):%,$(gb_Library_OUTDIRLOCATION)/%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_StaticLibrary_get_target
$(gb_StaticLibrary_OUTDIRLOCATION)/$(call gb_StaticLibrary_get_filename,$(1))
endef


# workdir target patterns

gb_AutoInstall_get_target = $(WORKDIR)/AutoInstall/$(1)
gb_AllLangHelp_get_target = $(WORKDIR)/AllLangHelp/$(1)
gb_AllLangPackage_get_target = $(WORKDIR)/AllLangPackage/$(1)
gb_AllLangResTarget_get_target = $(WORKDIR)/AllLangRes/$(1)
gb_AsmObject_get_target = $(WORKDIR)/AsmObject/$(1).o
gb_CObject_get_target = $(WORKDIR)/CObject/$(1).o
gb_GenCObject_get_target = $(WORKDIR)/GenCObject/$(1).o
gb_CliAssembly_get_target = $(WORKDIR)/CliAssembly/$(1).done
gb_CliAssemblyTarget_get_target = $(WORKDIR)/CliAssemblyTarget/$(1).done
gb_CliAssemblyTarget_get_assembly_target = $(WORKDIR)/CliAssemblyTarget/$(1)$(gb_CliAssemblyTarget_POLICYEXT)
gb_CliConfigTarget_get_target = $(WORKDIR)/CliConfigTarget/$(1).config
gb_CliLibraryTarget_get_target = $(WORKDIR)/CliLibraryTarget/$(1)$(gb_CliLibraryTarget_EXT)
gb_CliNativeLibraryTarget_get_external_target = $(WORKDIR)/CliNativeLibraryTarget/$(1).external
gb_CliNativeLibraryTarget_get_target = $(WORKDIR)/CliNativeLibraryTarget/$(1)$(gb_CliNativeLibraryTarget_EXT)
gb_CliUnoApiTarget_get_target = $(WORKDIR)/CliUnoApiTarget/$(1)$(gb_CliUnoApiTarget_EXT)
gb_ComponentTarget_get_target = $(WORKDIR)/ComponentTarget/$(1).component
gb_ComponentTarget_get_target_for_build = $(WORKDIR_FOR_BUILD)/ComponentTarget/$(1).component
gb_Configuration_get_preparation_target = $(WORKDIR)/Configuration/$(1).prepared
gb_CppunitTest_get_target = $(WORKDIR)/CppunitTest/$(1).test
gb_CppunitTestFakeExecutable_get_target = \
    $(OUTDIR)/unittest/install/$(LIBO_BIN_FOLDER)/$(1)
gb_CustomPackage_get_target = $(WORKDIR)/CustomPackage/$(1).filelist
gb_CustomTarget_get_repo_target = $(WORKDIR)/CustomTarget/$(2)_$(1).done
gb_CustomTarget_get_target = $(WORKDIR)/CustomTarget/$(1).done
gb_CustomTarget_get_workdir = $(WORKDIR)/CustomTarget/$(1)
gb_DescriptionTranslateTarget_get_target = $(WORKDIR)/DescriptionTranslateTarget/$(1).xml
gb_Dictionary_get_target = $(WORKDIR)/Dictionary/$(1).done
gb_CxxObject_get_target = $(WORKDIR)/CxxObject/$(1).o
gb_GenCxxObject_get_target = $(WORKDIR)/GenCxxObject/$(1).o
gb_Executable_get_headers_target = $(WORKDIR)/Headers/Executable/$(1)
gb_Executable_get_runtime_target = $(WORKDIR_FOR_BUILD)/Executable/$(1).run
gb_Extension_get_target = $(WORKDIR)/Extension/$(1).oxt
gb_Extension_get_rootdir = $(WORKDIR)/Extension/$(1)/root
gb_Extension_get_workdir = $(WORKDIR)/Extension/$(1)
gb_ExternalPackage_get_target = $(WORKDIR)/ExternalPackage/$(1)
gb_ExternalProject_get_statedir = $(WORKDIR)/ExternalProject/$(1)
gb_ExternalProject_get_preparation_target = $(WORKDIR)/ExternalProject/$(1).prepare
gb_ExternalProject_get_state_target = $(WORKDIR)/ExternalProject/$(1)/$(2)
gb_ExternalProject_get_target = $(WORKDIR)/ExternalProject/$(1).done
gb_Gallery_get_target = $(WORKDIR)/Gallery/$(1).done
gb_Gallery_get_workdir = $(WORKDIR)/Gallery/$(1)
gb_GeneratedPackage_get_target = $(WORKDIR)/GeneratedPackage/$(1).filelist
gb_GeneratedPackage_get_target_for_build = $(WORKDIR_FOR_BUILD)/GeneratedPackage/$(1).filelist
gb_HelpIndexTarget_get_target = $(WORKDIR)/HelpIndexTarget/$(1).done
gb_HelpJarTarget_get_target = $(WORKDIR)/HelpJarTarget/$(1).done
gb_HelpLinkTarget_get_preparation_target = $(WORKDIR)/HelpLinkTarget/$(1).prepare
gb_HelpLinkTarget_get_target = $(WORKDIR)/HelpLinkTarget/$(1).done
gb_HelpTarget_get_filelist = $(WORKDIR)/HelpTarget/$(1).filelist
gb_HelpTarget_get_linked_target = $(WORKDIR)/HelpTarget/$(1).translate
gb_HelpTarget_get_target = $(WORKDIR)/HelpTarget/$(1).zip
gb_HelpTarget_get_translation_target = $(call gb_HelpTarget_get_filelist,$(1))
gb_HelpTarget_get_workdir = $(WORKDIR)/HelpTarget/$(1)
gb_HelpTranslatePartTarget_get_target = $(WORKDIR)/HelpTranslatePartTarget/$(1)/done
gb_HelpTranslatePartTarget_get_translated_target = $(WORKDIR)/HelpTranslatePartTarget/$(1)/$(2).xhp
gb_HelpTranslatePartTarget_get_workdir = $(WORKDIR)/HelpTranslatePartTarget/$(1)
gb_HelpTranslateTarget_get_target = $(WORKDIR)/HelpTranslateTarget/$(1).done
gb_HelpTreeTarget_get_target = $(WORKDIR)/HelpTreeTarget/$(1).tree
gb_InstallModule_get_filelist = $(call gb_InstallModuleTarget_get_filelist,$(1))
gb_InstallModule_get_target = $(WORKDIR)/InstallModule/$(1).done
gb_InstallModuleTarget_get_external_target = $(WORKDIR)/InstallModuleTarget/$(1).external
gb_InstallModuleTarget_get_filelist = $(WORKDIR)/InstallModuleTarget/$(1).filelist
gb_InstallModuleTarget_get_target = $(WORKDIR)/InstallModuleTarget/$(1).filelist
gb_InstallScriptTarget_get_target = $(WORKDIR)/InstallScriptTarget/$(1)$(gb_InstallScript_EXT)
gb_InternalUnoApi_get_target = $(WORKDIR)/InternalUnoApi/$(1).done
gb_Jar_get_target = $(WORKDIR)/Jar/$(1).jar
gb_Jar_get_classsetname = Jar/$(1)
gb_JavaClassSet_get_classdir = $(WORKDIR)/JavaClassSet/$(1)
gb_JavaClassSet_get_repo_target = $(WORKDIR)/JavaClassSet/$(2)/$(1).done
gb_JavaClassSet_get_target = $(WORKDIR)/JavaClassSet/$(1)/done
gb_JunitTest_get_classsetname = JunitTest/$(1)
gb_JunitTest_get_target = $(WORKDIR)/JunitTest/$(1)/done
gb_JunitTest_get_userdir = $(WORKDIR)/JunitTest/$(1)/user
gb_PythonTest_get_target = $(WORKDIR)/PythonTest/$(1)/done
gb_LinkTarget_get_headers_target = $(WORKDIR)/Headers/$(1)
gb_LinkTarget_get_target = $(WORKDIR)/LinkTarget/$(1)
gb_LinkTarget_get_objects_list = $(WORKDIR)/LinkTarget/$(1).objectlist
gb_Module_get_almost_target = $(WORKDIR)/Module/almost/$(1)
gb_Module_get_check_target = $(WORKDIR)/Module/check/$(1)
gb_Module_get_slowcheck_target = $(WORKDIR)/Module/slowcheck/$(1)
gb_Module_get_subsequentcheck_target = $(WORKDIR)/Module/subsequentcheck/$(1)
gb_Module_get_target = $(WORKDIR)/Module/$(1)
gb_ObjCxxObject_get_target = $(WORKDIR)/ObjCxxObject/$(1).o
gb_ObjCObject_get_target = $(WORKDIR)/ObjCObject/$(1).o
gb_Pagein_get_target = $(WORKDIR)/Pagein/$(1)
gb_Package_get_preparation_target = $(WORKDIR)/Package/prepared/$(1)
gb_Package_get_target = $(WORKDIR)/Package/$(1).filelist
gb_Package_get_target_for_build = $(WORKDIR_FOR_BUILD)/Package/$(1).filelist
gb_PackageSet_get_target = $(WORKDIR)/PackageSet/$(1).filelist
gb_Postprocess_get_target = $(WORKDIR)/Postprocess/$(1)
gb_PrecompiledHeader_get_dep_target = $(WORKDIR)/Dep/PrecompiledHeader/$(gb_PrecompiledHeader_DEBUGDIR)/$(1).hxx.gch.d
gb_PrecompiledHeader_get_target = $(WORKDIR)/PrecompiledHeader/$(gb_PrecompiledHeader_DEBUGDIR)/$(1).hxx.gch
gb_PrecompiledHeader_get_timestamp = $(WORKDIR)/PrecompiledHeader/$(gb_PrecompiledHeader_DEBUGDIR)/Timestamps/$(1)
gb_PropertiesTranslateTarget_get_target = $(WORKDIR)/PropertiesTranslateTarget/$(1).properties
gb_Pyuno_get_target = $(WORKDIR)/Pyuno/$(1).done
gb_Rdb_get_target = $(WORKDIR)/Rdb/$(1).rdb
gb_Rdb_get_target_for_build = $(WORKDIR_FOR_BUILD)/Rdb/$(1).rdb
gb_ResTarget_get_imagelist_target = $(WORKDIR)/ResTarget/$(1).ilst
gb_ResTarget_get_target = $(WORKDIR)/ResTarget/$(1).res
gb_ScpMergeTarget_get_target = $(WORKDIR)/ScpMergeTarget/$(1).ulf
gb_ScpPreprocessTarget_get_target = $(WORKDIR)/ScpPreprocessTarget/$(1).pre
gb_ScpTarget_get_external_target = $(WORKDIR)/ScpTarget/$(1).external
gb_ScpTarget_get_target = $(WORKDIR)/ScpTarget/$(1).par
gb_ScpTemplateTarget_get_dir = $(abspath $(WORKDIR)/ScpTemplateTarget/$(dir $(1)))
gb_ScpTemplateTarget_get_target = $(abspath $(WORKDIR)/ScpTemplateTarget/$(dir $(1))$(subst pack,modules,$(subst module_,all,$(notdir $(1)))).inc)
gb_SdiTarget_get_target = $(WORKDIR)/SdiTarget/$(1)
gb_SrsPartMergeTarget_get_target = $(WORKDIR)/SrsPartMergeTarget/$(1)
gb_SrsPartTarget_get_target = $(WORKDIR)/SrsPartTarget/$(1)
gb_SrsTarget_get_headers_target = $(WORKDIR)/SrsTarget/$(1).headers
gb_SrsTarget_get_target = $(WORKDIR)/SrsTarget/$(1).srs
gb_SrsTemplatePartTarget_get_target = $(WORKDIR)/SrsTemplatePartTarget/$(firstword $(subst /, ,$(1)))/$(subst _tmpl,,$(notdir $(1)))
gb_SrsTemplateTarget_get_include_dir = $(WORKDIR)/SrsTemplatePartTarget/$(firstword $(subst /, ,$(1)))
gb_SrsTemplateTarget_get_target = $(WORKDIR)/SrsTemplateTarget/$(1)
gb_ThesaurusIndexTarget_get_target = $(WORKDIR)/ThesaurusIndexTarget/$(basename $(1)).idx
gb_UIConfig_get_imagelist_target = $(WORKDIR)/UIConfig/$(1).ilst
gb_UIConfig_get_target = $(WORKDIR)/UIConfig/$(1).done
gb_UIImageListTarget_get_target = $(WORKDIR)/UIImageListTarget/$(1).ilst
gb_UILocalizeTarget_get_target = $(WORKDIR)/UILocalizeTarget/$(1).done
gb_UILocalizeTarget_get_workdir = $(WORKDIR)/UILocalizeTarget/$(1)
gb_UIMenubarTarget_get_target = $(WORKDIR)/UIMenubarTarget/$(1).xml
gb_UnoApiTarget_get_target = $(WORKDIR)/UnoApiTarget/$(1).rdb
gb_UnoApiHeadersTarget_get_bootstrap_dir = $(WORKDIR)/UnoApiHeadersTarget/$(1)/$(call gb_UnoApiHeadersTarget_select_variant,$(1),bootstrap)
gb_UnoApiHeadersTarget_get_comprehensive_dir = $(WORKDIR)/UnoApiHeadersTarget/$(1)/$(call gb_UnoApiHeadersTarget_select_variant,$(1),comprehensive)
gb_UnoApiHeadersTarget_get_dir = $(WORKDIR)/UnoApiHeadersTarget/$(1)/$(call gb_UnoApiHeadersTarget_select_variant,$(1),normal)
gb_UnoApiHeadersTarget_get_real_bootstrap_dir = $(WORKDIR)/UnoApiHeadersTarget/$(1)/bootstrap
gb_UnoApiHeadersTarget_get_real_comprehensive_dir = $(WORKDIR)/UnoApiHeadersTarget/$(1)/comprehensive
gb_UnoApiHeadersTarget_get_real_dir = $(WORKDIR)/UnoApiHeadersTarget/$(1)/normal
gb_UnoApiHeadersTarget_get_real_bootstrap_target = $(WORKDIR)/UnoApiHeadersTarget/$(1)/bootstrap.done
gb_UnoApiHeadersTarget_get_real_comprehensive_target = $(WORKDIR)/UnoApiHeadersTarget/$(1)/comprehensive.done
gb_UnoApiHeadersTarget_get_real_target = $(WORKDIR)/UnoApiHeadersTarget/$(1)/normal.done
gb_UnoApiHeadersTarget_get_bootstrap_target = $(WORKDIR)/UnoApiHeadersTarget/$(1)/$(call gb_UnoApiHeadersTarget_select_variant,$(1),bootstrap).done
gb_UnoApiHeadersTarget_get_comprehensive_target = $(WORKDIR)/UnoApiHeadersTarget/$(1)/$(call gb_UnoApiHeadersTarget_select_variant,$(1),comprehensive).done
gb_UnoApiHeadersTarget_get_target = $(WORKDIR)/UnoApiHeadersTarget/$(1)/$(call gb_UnoApiHeadersTarget_select_variant,$(1),normal).done
gb_UnoApiPartTarget_get_target = $(WORKDIR)/UnoApiPartTarget/$(1)
gb_UnpackedTarball_get_dir = $(WORKDIR)/UnpackedTarball/$(1)
gb_UnpackedTarball_get_pristine_dir = $(WORKDIR)/UnpackedTarball/$(1).org
gb_UnpackedTarball_get_final_target = $(WORKDIR)/UnpackedTarball/$(1).update
# NOTE: the .done target is hardcoded in solenv/bin/concat-deps.c !
gb_UnpackedTarball_get_target = $(WORKDIR)/UnpackedTarball/$(1).done
gb_UnpackedTarball_get_preparation_target = $(WORKDIR)/UnpackedTarball/$(1).prepare
gb_UnpackedTarget_get_target = $(WORKDIR)/UnpackedTarget/$(1)
gb_WinResTarget_get_target = $(WORKDIR)/WinResTarget/$(1)$(gb_WinResTarget_POSTFIX)
# workdir targets: $(1) is prefix/path
gb_Configuration_get_target = $(WORKDIR)/Configuration/$(1).done
gb_YaccTarget_get_grammar_target = $(WORKDIR)/YaccTarget/$(1).cxx
gb_YaccTarget_get_header_target = $(WORKDIR)/YaccTarget/$(1).hxx
gb_YaccTarget_get_target = $(WORKDIR)/YaccTarget/$(1).done

gb_LexTarget_get_scanner_target = $(WORKDIR)/LexTarget/$(1).cxx
gb_LexTarget_get_target = $(WORKDIR)/LexTarget/$(1).done

gb_XcsTarget_get_target = $(WORKDIR)/XcsTarget/$(1)
gb_XcuDataTarget_get_target = $(WORKDIR)/XcuDataTarget/$(1)
gb_XcuLangpackTarget_get_target = $(WORKDIR)/XcuLangpackTarget/$(1)
gb_XcuModuleTarget_get_target = $(WORKDIR)/XcuModuleTarget/$(1)
gb_XcuMergeTarget_get_target = $(WORKDIR)/XcuMergeTarget/$(1)
gb_XcuResTarget_get_target = $(WORKDIR)/XcuResTarget/$(1)
gb_Zip_get_target = $(WORKDIR)/Zip/$(1).zip

gb_Library__get_final_target = $(WORKDIR)/Dummy/$(1)

define gb_Library_get_headers_target
$(patsubst $(1):%,$(WORKDIR)/Headers/Library/%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_StaticLibrary_get_headers_target
$(WORKDIR)/Headers/StaticLibrary/$(call gb_StaticLibrary_get_filename,$(1))
endef

$(eval $(call gb_Helper_make_clean_targets,\
	AutoInstall \
	AllLangHelp \
	AllLangPackage \
	AllLangResTarget \
	CliAssembly \
	CliAssemblyTarget \
	CliConfigTarget \
	CliLibraryTarget \
	CliNativeLibraryTarget \
	CliUnoApiTarget \
	ComponentTarget \
	CustomPackage \
	DescriptionTranslateTarget \
	Dictionary \
	Executable \
	ExternalPackage \
	Extension \
	Gallery \
	GeneratedPackage \
	HelpTarget \
	HelpIndexTarget \
	HelpJarTarget \
	HelpLinkTarget \
	HelpTranslatePartTarget \
	HelpTranslateTarget \
	HelpTreeTarget \
	InstallModule \
	InstallModuleTarget \
	InstallScriptTarget \
	InternalUnoApi \
	JavaClassSet \
	Jar \
	JunitTest \
	LinkTarget \
	Module \
	PackagePart \
	Package \
	PackageSet \
	Pagein \
	Postprocess \
	PrecompiledHeader \
	PropertiesTranslateTarget \
	Pyuno \
	PythonTest \
	Rdb \
	ResTarget \
	ScpMergeTarget \
	ScpPreprocessTarget \
	ScpTarget \
	ScpTemplateTarget \
	SdiTarget \
	SrsTarget \
	SrsTemplateTarget \
	ThesaurusIndexTarget \
	CppunitTest \
	CppunitTestFakeExecutable \
	CustomTarget \
	ExternalProject \
	UIConfig \
	UIImageListTarget \
	UILocalizeTarget \
	UIMenubarTarget \
	UnoApiHeadersTarget \
	UnoApiTarget \
	UnpackedTarball \
	UnpackedTarget \
	WinResTarget \
	YaccTarget \
	LexTarget \
	Zip \
	XcsTarget \
	XcuDataTarget \
	XcuModuleTarget \
	XcuLangpackTarget \
	XcuMergeTarget \
	XcuResTarget \
	Configuration \
))

$(eval $(call gb_Helper_make_outdir_clean_targets,\
	CliLibrary \
	CliNativeLibrary \
	CliUnoApi \
	InstallScript \
	Library \
	StaticLibrary \
	UnoApi \
))

$(eval $(call gb_Helper_make_dep_targets,\
	CObject \
	CxxObject \
	ObjCObject \
	ObjCxxObject \
	AsmObject \
	GenCObject \
	GenCxxObject \
	LinkTarget \
	SdiTarget \
	SrsPartTarget \
	SrsTarget \
	UnoApiTarget \
	UnoApiPartTarget \
))

# other getters

define gb_Library_get_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_LAYER)))
endef

define gb_Executable_get_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Executable_LAYER)))
endef

define gb_Library_get_filename
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_Executable_get_filename
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Executable_FILENAMES)))
endef

# Get dependencies needed for running the executable
#
# This is not strictly necessary, but it makes the use more similar to
# ExternalExecutable.
#
# gb_Executable_get_runtime_dependencies executable
define gb_Executable_get_runtime_dependencies
$(call gb_Executable_get_runtime_target,$(1))
endef

# Get complete command-line for running the executable
#
# This includes setting library path.
#
# gb_Executable_get_command executable
define gb_Executable_get_command
$(gb_Helper_set_ld_path) $(2) $(call gb_Executable_get_target_for_build,$(1))
endef

gb_Executable_get_linktargetname = Executable/$(call gb_Executable_get_filename,$(1))
gb_Library_get_linktargetname = Library/$(call gb_Library_get_filename,$(1))
gb_StaticLibrary_get_linktargetname = StaticLibrary/$(call gb_StaticLibrary_get_filename,$(1))

# static members declared here because they are used globally

gb_Executable_BINDIR = $(WORKDIR)/LinkTarget/Executable
gb_Library_OUTDIRLOCATION = $(OUTDIR)/lib
gb_Library_DLLDIR = $(WORKDIR)/LinkTarget/Library
gb_CppunitTest_DLLDIR = $(WORKDIR)/LinkTarget/CppunitTest
gb_StaticLibrary_OUTDIRLOCATION = $(OUTDIR)/lib

# static variables declared here because they are used globally

gb_POLOCATION := $(SRCDIR)/translations/source

# vim: set noet sw=4:
