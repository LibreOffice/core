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
	SDKBIN:$(INSTDIR)/$(gb_Package_SDKDIRNAME)/bin \
	NONE:$(gb_Executable_BINDIR) \

gb_Executable_LAYER_DIRS_FOR_BUILD := \
	UREBIN:$(INSTROOT_FOR_BUILD)/$(LIBO_URE_BIN_FOLDER_FOR_BUILD) \
	OOO:$(INSTROOT_FOR_BUILD)/$(LIBO_BIN_FOLDER_FOR_BUILD) \
	SDKBIN:$(INSTDIR_FOR_BUILD)/$(gb_Package_SDKDIRNAME)/bin \
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
gb_Executable_get_install_target = $(call gb_Executable__get_dir_for_exe,$(1))/$(call gb_Executable_get_filename,$(1))
gb_Executable_get_install_target_for_build = $(call gb_Executable__get_dir_for_exe_for_build,$(1))/$(call gb_Executable_get_filename,$(1))

define gb_Executable_Executable
$(call gb_Postprocess_register_target,AllExecutables,Executable,$(1))
ifeq (,$$(findstring $(1),$$(gb_Executable_KNOWN)))
$$(eval $$(call gb_Output_info,Currently known executables: $(sort $(gb_Executable_KNOWN)),ALL))
$$(eval $$(call gb_Output_error,Executable $(1) must be registered in Repository.mk))
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

$$(eval $$(call gb_Module_register_target,$(call gb_Executable_get_target,$(1)),$(call gb_Executable_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Executable)

endef

define gb_Executable_set_targettype_gui
$(call gb_LinkTarget_get_target,$(call gb_Executable_get_linktarget,$(1))) : TARGETGUI := $(2)
endef

define gb_Executable_add_auxtarget
$(call gb_Output_error,gb_Executable_add_auxtarget should no longer be necessary)
endef

define gb_Executable_forward_to_Linktarget
gb_Executable_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_Executable_get_linktarget,$$(1)),$$(2),$$(3),Executable_$$(1))

endef

$(eval $(foreach method,\
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_objcobject \
	add_objcobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_grammar \
	add_grammars \
	add_scanner \
	add_scanners \
	add_exception_objects \
	add_generated_cobjects \
	add_generated_cxxobjects \
	add_generated_exception_objects \
	add_cflags \
	set_cflags \
	add_cxxflags \
	set_cxxflags \
	add_objcflags \
	add_objcxxflags \
	set_objcflags \
	set_objcxxflags \
	add_defs \
	set_defs \
	set_include \
	add_ldflags \
	set_ldflags \
	add_libs \
	disable_standard_system_libs \
	use_system_darwin_frameworks \
	use_system_win32_libs \
	set_library_path_flags \
	use_api \
	use_sdk_api \
	use_udk_api \
	add_internal_api \
	use_internal_api \
	add_internal_bootstrap_api \
	use_internal_bootstrap_api \
	add_internal_comprehensive_api \
	use_internal_comprehensive_api \
	add_linked_libs \
	use_libraries \
	add_linked_static_libs \
	use_static_libraries \
	use_external \
	use_externals \
	add_custom_headers \
	use_custom_headers \
	add_package_headers \
	use_package \
	use_packages \
	use_unpacked \
	add_sdi_headers \
	set_precompiled_header \
	add_precompiled_header \
	add_nativeres \
	set_warnings_not_errors \
	set_generated_cxx_suffix \
,\
	$(call gb_Executable_forward_to_Linktarget,$(method))\
))

# Run-time use

# Add dependencies needed for running the executable
#
# gb_Executable_add_runtime_dependencies executable dependencies
define gb_Executable_add_runtime_dependencies
$(call gb_Executable_get_runtime_target,$(1)) : $(2)

endef

# vim: set noet sw=4:
