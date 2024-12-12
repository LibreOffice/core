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


# Static Library class

# defined globally in gbuild.mk
# defined by platform
#  gb_StaticLibrary_get_filename
#  gb_StaticLibrary_PLAINEXT
#  gb_StaticLibrary_StaticLibrary_platform


# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
.PHONY : $(WORKDIR)/Clean/StaticLibrary/%
$(WORKDIR)/Clean/StaticLibrary/% :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(AUXTARGETS))

define gb_StaticLibrary_StaticLibrary
$(call gb_StaticLibrary__StaticLibrary_impl,$(1),$(call gb_StaticLibrary_get_linktarget,$(1)))

endef

define gb_StaticLibrary_register_target
endef

# call gb_StaticLibrary__StaticLibrary_impl,staticlib,linktarget
define gb_StaticLibrary__StaticLibrary_impl
$(call gb_StaticLibrary_register_target, $(1), $(2))
$(call gb_LinkTarget_LinkTarget,$(2),StaticLibrary_$(1),NONE)
$(call gb_LinkTarget_set_targettype,$(2),StaticLibrary)
$(call gb_StaticLibrary_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_StaticLibrary_get_clean_target,$(1)) : AUXTARGETS :=
$(call gb_StaticLibrary_StaticLibrary_platform,$(1),$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_StaticLibrary_get_target,$(1)),$(call gb_StaticLibrary_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),StaticLibrary)

endef

# forward the call to the gb_LinkTarget implementation
# (note: because the function name is in $(1), the other args are shifted by 1)
define gb_StaticLibrary__forward_to_Linktarget
$(call gb_LinkTarget_$(subst gb_StaticLibrary_,,$(1)),$(call gb_StaticLibrary_get_linktarget,$(2)),$(3),$(4),StaticLibrary_$(2))

endef

# copy pasta for forwarding: this could be (and was) done more elegantly, but
# these here can be found by both git grep and ctags
gb_StaticLibrary_add_cobject = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_cobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_cxxobject = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_objcobject = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_objcobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_cxxobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_objcxxobject = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_objcxxobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_cxxclrobject = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_cxxclrobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_asmobject = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_asmobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_exception_objects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_x64_generated_exception_objects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_generated_cobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_x64_generated_cobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_generated_exception_objects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_generated_objcobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_generated_objcxxobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_generated_nasmobjects = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_cflags = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_cxxflags = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_objcflags = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_objcxxflags = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_nasmflags = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_cxxclrflags = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_defs = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_include = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_ldflags = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_ldflags = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_x64 = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_libs = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_library_path_flags = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_api = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_sdk_api = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_udk_api = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_internal_api = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_internal_bootstrap_api = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_internal_comprehensive_api = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_external = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_externals = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_custom_headers = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_package = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_packages = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_unpacked = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_external_project = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_static_libraries = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_add_sdi_headers = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_precompiled_header = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_reuse_precompiled_header = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_common_precompiled_header = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_warnings_not_errors = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_warnings_disabled = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_external_code = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_generated_cxx_suffix = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_use_clang = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))
gb_StaticLibrary_set_clang_precompiled_header = $(call gb_StaticLibrary__forward_to_Linktarget,$(0),$(1),$(2),$(3))

# vim: set noet sw=4:
