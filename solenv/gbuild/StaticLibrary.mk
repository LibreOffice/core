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

# call gb_StaticLibrary__StaticLibrary_impl,staticlib,linktarget
define gb_StaticLibrary__StaticLibrary_impl
$(call gb_LinkTarget_LinkTarget,$(2),StaticLibrary_$(1),NONE)
$(call gb_LinkTarget_set_targettype,$(2),StaticLibrary)
$(call gb_StaticLibrary_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_StaticLibrary_get_clean_target,$(1)) : AUXTARGETS :=
$(call gb_StaticLibrary_StaticLibrary_platform,$(1),$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_StaticLibrary_get_target,$(1)),$(call gb_StaticLibrary_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),StaticLibrary)

endef

define gb_StaticLibrary_forward_to_Linktarget
gb_StaticLibrary_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_StaticLibrary_get_linktarget,$$(1)),$$(2),$$(3),StaticLibrary_$$(1))

endef

$(eval $(foreach method,\
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_asmobject \
	add_asmobjects \
	add_exception_objects \
	add_x64_generated_exception_objects \
	add_generated_cobjects \
	add_x64_generated_cobjects \
	add_generated_cxxobjects \
	add_generated_exception_objects \
	add_cflags \
	set_cflags \
	add_cxxflags \
	set_cxxflags \
	add_objcflags \
	add_objcxxflags \
	set_objcxxflags \
	add_defs \
	set_defs \
	set_include \
	add_ldflags \
	set_ldflags \
	set_x64 \
	add_libs \
	set_library_path_flags \
	use_api \
	use_sdk_api \
	use_udk_api \
	use_internal_api \
	use_internal_bootstrap_api \
	use_internal_comprehensive_api \
	use_external \
	use_externals \
	add_custom_headers \
	use_custom_headers \
	add_package_headers \
	use_package \
	use_packages \
	use_unpacked \
	use_external_project \
	use_static_libraries \
	add_sdi_headers \
	set_precompiled_header \
	add_precompiled_header \
	set_warnings_not_errors \
	set_generated_cxx_suffix \
,\
	$(call gb_StaticLibrary_forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4:
