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

# Executable class

# defined by platform
#  gb_Executable_Executable_platform

gb_Executable_LAYER_DIRS := \
	UREBIN:$(INSTROOT)/$(LIBO_URE_BIN_FOLDER) \
	OOO:$(INSTROOT)/$(LIBO_BIN_FOLDER) \
	SDKBIN:$(INSTDIR)/$(SDKDIRNAME)/bin \
	NONE:$(gb_Executable_BINDIR) \

gb_Executable_LAYER_DIRS_FOR_BUILD := \
	UREBIN:$(INSTROOT_FOR_BUILD)/$(LIBO_URE_BIN_FOLDER_FOR_BUILD) \
	OOO:$(INSTROOT_FOR_BUILD)/$(LIBO_BIN_FOLDER_FOR_BUILD) \
	SDKBIN:$(INSTDIR_FOR_BUILD)/$(SDKDIRNAME_FOR_BUILD)/bin \
	NONE:$(gb_Executable_BINDIR_FOR_BUILD) \

$(dir $(call gb_Executable_get_runtime_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_Executable_get_runtime_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_Executable_get_runtime_target,%) :
	touch $@

.PHONY : $(call gb_Executable_get_clean_target,%)
$(call gb_Executable_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Executable_get_target,$*) \
			$(call gb_Executable_get_runtime_target,$*) \
			$(AUXTARGETS))

gb_Executable__get_dir_for_layer = $(patsubst $(1):%,%,$(filter $(1):%,$(call gb_Executable_LAYER_DIRS)))
gb_Executable__get_dir_for_layer_for_build = $(patsubst $(1):%,%,$(filter $(1):%,$(call gb_Executable_LAYER_DIRS_FOR_BUILD)))
gb_Executable__get_dir_for_exe = $(call gb_Executable__get_dir_for_layer,$(call gb_Executable_get_layer,$(1)))
gb_Executable__get_dir_for_exe_for_build = $(call gb_Executable__get_dir_for_layer_for_build,$(call gb_Executable_get_layer,$(1)))

define gb_Executable_Executable
$(call gb_Postprocess_register_target,AllExecutables,Executable,$(1))
ifeq (,$$(findstring $(1),$$(gb_Executable_KNOWN)))
$$(eval $$(call gb_Output_info,Currently known executables: $(sort $(gb_Executable_KNOWN)),ALL))
$$(eval $$(call gb_Output_error,Executable $(1) must be registered in Repository.mk or RepositoryExternal.mk))
endif
$(call gb_Executable__Executable_impl,$(1),$(call gb_Executable_get_linktarget,$(1)))

endef

# call gb_Executable__Executable_impl,exe,linktarget
define gb_Executable__Executable_impl
$(call gb_LinkTarget_LinkTarget,$(2),Executable_$(1),$(call gb_Executable_get_layer,$(1)))
$(call gb_LinkTarget_set_targettype,$(2),Executable)
$(call gb_LinkTarget_add_libs,$(2),$(gb_STDLIBS))
$(call gb_Executable_get_runtime_target,$(1)) :| $(dir $(call gb_Executable_get_runtime_target,$(1))).dir
$(call gb_Executable_get_runtime_target,$(1)) : $(call gb_Executable_get_target_for_build,$(1))
$(call gb_Executable_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_Executable_get_clean_target,$(1)) : AUXTARGETS :=
$(call gb_Executable_Executable_platform,$(1),$(2),$(gb_Executable_BINDIR)/$(1).lib)
ifeq ($(OS),EMSCRIPTEN)
$(call gb_LinkTarget_get_target,$(call gb_Executable_get_linktarget,$(1))) : \
    $(gb_CustomTarget_workdir)/static/emscripten_fs_image/soffice.data \
    $(gb_CustomTarget_workdir)/static/emscripten_fs_image/soffice.data.js.metadata
endif

$$(eval $$(call gb_Module_register_target,$(call gb_Executable_get_target,$(1)),$(call gb_Executable_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Executable)

endef

define gb_Executable_set_targettype_gui
$(call gb_Executable_get_linktarget_target,$(1)) : TARGETGUI := $(2)
endef

# forward the call to the gb_LinkTarget implementation
# (note: because the function name is in $(1), the other args are shifted by 1)
define gb_Executable__forward_to_Linktarget
$(call gb_LinkTarget_$(subst gb_Executable_,,$(1)),$(call gb_Executable_get_linktarget,$(2)),$(3),$(4),Executable_$(2))

endef

# copy pasta for forwarding: this could be (and was) done more elegantly, but
# these here can be found by both git grep and ctags
gb_Executable_add_cobject = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_cobjects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_cxxobject = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_cxxobjects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_objcobject = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_objcobjects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_objcxxobject = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_objcxxobjects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_cxxclrobject = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_cxxclrobjects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_grammar = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_grammars = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_scanner = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_scanners = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_exception_objects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_generated_cobjects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_generated_exception_objects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_generated_objcobjects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_generated_objcxxobjects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_cflags = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_cxxflags = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_objcflags = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_objcxxflags = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_cxxclrflags = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_defs = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_include = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_ldflags = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_ldflags = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_x86 = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_libs = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_disable_standard_system_libs = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_system_darwin_frameworks = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_system_win32_libs = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_library_path_flags = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_api = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_sdk_api = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_udk_api = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_internal_api = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_internal_bootstrap_api = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_internal_comprehensive_api = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_library_objects = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_libraries = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_static_libraries = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_external = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_externals = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_custom_headers = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_package = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_packages = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_unpacked = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_sdi_headers = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_precompiled_header = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_reuse_precompiled_header = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_common_precompiled_header = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_nativeres = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_warnings_not_errors = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_warnings_disabled = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_external_code = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_generated_cxx_suffix = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_clang = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_set_clang_precompiled_header = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_use_vclmain = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_Executable_add_prejs = $(call gb_Executable__forward_to_Linktarget,$(0),$(1),$(2),$(3))

# Run-time use

# Add dependencies needed for running the executable
#
# gb_Executable_add_runtime_dependencies executable dependencies
define gb_Executable_add_runtime_dependencies
$(call gb_Executable_get_runtime_target,$(1)) : $(2)

endef

# vim: set noet sw=4:
