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


# Library class

# defined globally in TargetLocations.mk
#  gb_Library_DLLDIR := $(WORKDIR)/LinkTarget/Library
# defined by platform
#  gb_Library_DEFS
#  gb_Library_ILIBFILENAMES
#  gb_Library_FILENAMES
#  gb_Library_Library_platform

gb_Library_LAYER_DIRS := \
	URELIB:$(INSTROOT)/$(LIBO_URE_LIB_FOLDER) \
	OOO:$(INSTROOT)/$(LIBO_LIB_FOLDER) \
	SHLXTHDL:$(INSTROOT)/$(LIBO_LIB_FOLDER)/shlxthdl \
	OXT:$(WORKDIR)/LinkTarget/ExtensionLibrary \
	NONE:$(gb_Library_DLLDIR) \

gb_Library_LAYER_DIRS_FOR_BUILD := \
	URELIB:$(INSTROOT_FOR_BUILD)/$(LIBO_URE_LIB_FOLDER_FOR_BUILD) \
	OOO:$(INSTROOT_FOR_BUILD)/$(LIBO_LIB_FOLDER_FOR_BUILD) \
	SHLXTHDL:$(INSTROOT_FOR_BUILD)/$(LIBO_LIB_FOLDER_FOR_BUILD)/shlxthdl \
	OXT:$(WORKDIR_FOR_BUILD)/LinkTarget/ExtensionLibrary \
	NONE:$(gb_Library_DLLDIR_FOR_BUILD) \

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
.PHONY : $(WORKDIR)/Clean/Library/%
$(WORKDIR)/Clean/Library/% :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Library_get_exports_target,$*) \
			$(AUXTARGETS))

gb_Library__get_dir_for_layer = $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_LAYER_DIRS)))
gb_Library__get_dir_for_layer_for_build = $(patsubst $(1):%,%,$(filter $(1):%,$(call gb_Library_LAYER_DIRS_FOR_BUILD)))
gb_Library_get_instdir = $(call gb_Library__get_dir_for_layer,$(call gb_Library_get_layer,$(1)))
gb_Library_get_instdir_for_build = $(call gb_Library__get_dir_for_layer_for_build,$(call gb_Library_get_layer,$(1)))

gb_Library_get_ilib_target = $(if $(filter $(1),$(gb_Library_RTVERLIBS) $(gb_Library_UNOVERLIBS)),$(call gb_Library_get_sdk_link_dir)/$(call gb_Library_get_ilibfilename,$(1)),$(gb_Library_DLLDIR)/$(call gb_Library_get_ilibfilename,$(1)))

define gb_Library_Library
$(call gb_Postprocess_register_target,AllLibraries,Library,$(1))
ifeq (,$$(filter $(1),$$(gb_Library_KNOWNLIBS)))
$$(eval $$(call gb_Output_info,Currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Library $(1) must be registered in Repository.mk or RepositoryExternal.mk))
endif

$(if $(gb_Package_PRESTAGEDIR),\
    $(if $(wildcard $(gb_Package_PRESTAGEDIR)/$(call gb_Library_get_instdir,$(1))/$(call gb_Library_get_runtime_filename,$(1))),\
        $(call gb_Library__Library_impl_copy,$(0),$(call gb_Library_get_instdir,$(1))/$(call gb_Library_get_runtime_filename,$(1))),\
        $(call gb_Library__Library_impl,$(1),$(call gb_Library_get_linktarget,$(1)))\
    ),
    $(call gb_Library__Library_impl,$(1),$(call gb_Library_get_linktarget,$(1)))\
)

endef

define gb_Library__Library_impl_copy
$(call gb_Package_Package,Library_Copy_$(1),$(gb_Package_PRESTAGEDIR))
$(call gb_Package_add_file,Library_Copy_$(1),$(2),$(2))
endef

# Note: there may be targets in 3 different directories: the library itself,
# the exports target (and other misc. MSVC files) (always in
# $(WORKDIR)/LinkTarget), and the import library, which may be in SDK;
# the first 2 are always created by gb_LinkTarget_LinkTarget
# Also: the directory dependencies must be on the headers_target because
# MSVC will write a PDB file when compiling objects.
#
# call gb_Library__Library_impl,library,linktarget
define gb_Library__Library_impl
$(call gb_LinkTarget_LinkTarget,$(2),Library_$(1),$(call gb_Library_get_layer,$(1)))
$(call gb_LinkTarget_set_targettype,$(2),Library)
$(call gb_LinkTarget_add_libs,$(2),$(gb_STDLIBS))
$(call gb_LinkTarget_add_defs,$(2),\
	$(gb_Library_DEFS) \
)
$(call gb_Library_get_exports_target,$(1)) : $(call gb_Library_get_target,$(1))
$(call gb_LinkTarget_get_headers_target,$(2)) : \
	| $(dir $(call gb_Library_get_ilib_target,$(1))).dir
$(call gb_Library_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS :=
$(call gb_Library_Library_platform,$(1),$(2),$(call gb_Library_get_ilib_target,$(1)))

$$(eval $$(call gb_Module_register_target,$(call gb_Library_get_exports_target,$(1)),$(call gb_Library_get_clean_target,$(1))))

$(call gb_Helper_make_userfriendly_targets,$(1),Library,$(call gb_Library_get_exports_target,$(1)))

endef

# we actually (ab)use ILIBTARGET here to store the unversioned symlink -
# it serves a similar purpose to an MSVC import library, as input for linker
# call gb_Library__add_soversion_link,library,linkname
define gb_Library__add_soversion_link
$(call gb_LinkTarget_set_ilibtarget,$(call gb_Library_get_linktarget,$(1)),$(2))

endef

define gb_Library__set_soversion_script
$(call gb_LinkTarget_set_soversion_script,$(call gb_Library_get_linktarget,$(1)),$(2))
$(call gb_Library__add_soversion_link,$(1),$(call gb_Library_get_versionlink_target,$(1)))

endef

# for libraries that maintain stable ABI: set SOVERSION and version script
# $(call gb_Library_set_soversion_script,versionscript)
define gb_Library_set_soversion_script
$(if $(2),,$(call gb_Output_error,gb_Library_set_soversion_script: no script))
$(if $(3),$(call gb_Output_error,gb_Library_set_soversion_script: too many arguments))
$(call gb_Library__set_soversion_script_platform,$(1),$(2))
endef

gb_Library__get_component_var = $(call gb_Library__get_workdir_linktargetname,$(1))<>COMPONENTFILE
gb_Library__get_component = $($(call gb_Library__get_component_var,$(1)))

# The dependency from workdir component target to outdir library should ensure
# that gb_CppunitTest_use_component can transitively depend on the library.
# But the component target also must be delivered; use the target
# gb_Library_get_exports_target for that purpose, since it is already
# the "final" target of the Library...
#
# call gb_Library_set_componentfile,library,componentfile,rdb
define gb_Library_set_componentfile
$(call gb_ComponentTarget_ComponentTarget,$(2),\
	$(call gb_Library__get_componentprefix,$(gb_Library__get_name)),\
	$(call gb_Library_get_runtime_filename,$(gb_Library__get_name)),$(3))
$(call gb_Library_get_exports_target,$(gb_Library__get_name)) :| \
	$(call gb_ComponentTarget_get_target,$(2))
$(call gb_ComponentTarget_get_target,$(2)) :| \
	$(call gb_Library_get_target,$(gb_Library__get_name))
$(call gb_Library_get_clean_target,$(gb_Library__get_name)) : \
	$(call gb_ComponentTarget_get_clean_target,$(2))
$(eval $(call gb_Library__get_component_var,$(1)) += $(2))

endef

# call gb_Library_set_componentfiles,library,componentfiles,rdb
define gb_Library_set_componentfiles
$(foreach comp,$(2),$(call gb_Library_set_componentfile,$(1),$(comp),$(3)))
endef

gb_Library__get_name = $(if $(filter $(1),$(gb_MERGEDLIBS)),merged,$(1))

gb_Library__get_componentprefix = \
	$(call gb_Library__get_layer_componentprefix,$(call \
		gb_Library_get_layer,$(1)))

gb_Library__get_layer_componentprefix = \
	$(patsubst $(1):%,%,$(or \
		$(filter $(1):%,$(gb_Library__COMPONENTPREFIXES)), \
		$(call gb_Output_error,no ComponentTarget native prefix for layer '$(1)')))

# The \d gets turned into a dollar sign by a $(subst) call in
# gb_ComponentTarget__command in ComponentTarget.mk. As far as I
# understand, there is nothing magic to it, it is not some
# Make/awk/sed/whatever syntax.

gb_Library__COMPONENTPREFIXES := \
    NONE:vnd.sun.star.expand:\dLO_BUILD_LIB_DIR/ \
    OOO:vnd.sun.star.expand:\dLO_LIB_DIR/ \
    URELIB:vnd.sun.star.expand:\dURE_INTERNAL_LIB_DIR/ \
    OXT:./ \
	SHLXTHDL:ERROR_NOT_ALLOWED \


gb_Library_get_runtime_filename = $(call gb_Library_get_filename,$(1))
gb_Library_get_runtime_filename_for_build = $(call gb_Library_get_filename_for_build,$(1))

# instead of setting nodep use gb_Library_set_plugin_for_nodep
#
# call gb_Library_set_plugin_for,library,loader,nodep
define gb_Library_set_plugin_for
ifneq (,$$(filter-out $(gb_Library_KNOWNPLUGINS),$(1)))
$$(eval $$(call gb_Output_info,currently known plugins are: $(sort $(gb_Library_KNOWNPLUGINS)),ALL))
$$(eval $$(call gb_Output_error,Unknown plugin(s) '$$(filter-out $(gb_Library_KNOWNPLUGINS),$(1)))'. Plugins must be registered in Repository.mk or RepositoryExternal.mk))
endif

$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktarget,$(2))) : PLUGINS += $(1)
$(eval $(call gb_LinkTarget__add_plugin,$(call gb_Library_get_linktarget,$(2)),$(1)))
$(eval $(call gb_LinkTarget__set_plugin_for,$(call gb_Library_get_linktarget,$(1)),$(2),$(3),Library_$(1)))
endef

# call gb_Library_set_plugin_for_nodep,library,loader
gb_Library_set_plugin_for_nodep = $(call gb_Library_set_plugin_for,$(1),$(2),$(true))

# forward the call to the gb_LinkTarget implementation
# (note: because the function name is in $(1), the other args are shifted by 1)
define gb_Library__forward_to_Linktarget
$(call gb_LinkTarget_$(subst gb_Library_,,$(1)),$(call gb_Library_get_linktarget,$(2)),$(3),$(4),Library_$(2))

endef

# copy pasta for forwarding: this could be (and was) done more elegantly, but
# these here can be found by both git grep and ctags
gb_Library_add_cobject = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_cobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_cxxobject = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_cxxobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_objcobject = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_objcobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_objcxxobject = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_objcxxobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_cxxclrobject = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_cxxclrobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_generated_cxxclrobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_asmobject = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_asmobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_exception_objects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_x64_generated_exception_objects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_generated_cobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_generated_exception_objects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_generated_objcobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_generated_objcxxobjects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_library_objects = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_grammar = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_grammars = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_scanner = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_scanners = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_cflags = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_cxxflags = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_objcxxflags = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_objcflags = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_cxxclrflags = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_defs = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_include = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_ldflags = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_ldflags = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_x64 = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_x86 = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_libs = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_disable_standard_system_libs = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_system_darwin_frameworks = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_system_win32_libs = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_library_path_flags = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_api = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_sdk_api = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_udk_api = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_internal_api = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_internal_bootstrap_api = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_internal_comprehensive_api = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_libraries = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_static_libraries = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_external = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_externals = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_custom_headers = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_package = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_packages = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_unpacked = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_sdi_headers = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_precompiled_header = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_reuse_precompiled_header = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_common_precompiled_header = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_export_objects_list = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_add_nativeres = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_nativeres = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_visibility_default = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_warnings_not_errors = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_warnings_disabled = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_external_code = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_generated_cxx_suffix = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_clang = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_clang_precompiled_header = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_glxtest = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_use_vclmain = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Library_set_is_ure_library_or_dependency = $(call gb_Library__forward_to_Linktarget,$(0),$(1),$(2),$(3))

# vim: set noet sw=4:
