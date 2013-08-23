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
#  gb_Library_OUTDIRLOCATION := $(OUTDIR)/lib
#  gb_Library_DLLDIR := $(WORKDIR)/LinkTarget/Library
# defined by platform
#  gb_Library_DEFS
#  gb_Library_DLLFILENAMES
#  gb_Library_FILENAMES
#  gb_Library_Library_platform

gb_Library_LAYER_DIRS := \
    OOO:program \
    URELIB:ure/$(notdir $(gb_Helper_OUTDIRLIBDIR))

# doesn't do anything, just used for hooking up component target
.PHONY: $(call gb_Library__get_final_target,%)

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
.PHONY : $(WORKDIR)/Clean/OutDir/lib/%$(gb_Library_PLAINEXT)
$(WORKDIR)/Clean/OutDir/lib/%$(gb_Library_PLAINEXT) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(OUTDIR)/lib/$*$(gb_Library_PLAINEXT) \
			$(AUXTARGETS))

gb_Library_get_packagename = Library/$(1)

gb_Library__get_dir_for_layer = $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_LAYER_DIRS)))
gb_Library_get_instdir = $(call gb_Library__get_dir_for_layer,$(call gb_Library_get_layer,$(1)))

define gb_Library_Library
$(call gb_Postprocess_register_target,AllLibraries,Library,$(1))
ifeq (,$$(findstring $(1),$$(gb_Library_KNOWNLIBS)))
$$(eval $$(call gb_Output_info,Currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Library $(1) must be registered in Repository.mk))
endif
$(call gb_Library_get_target,$(1)) : SOVERSION :=

$(if $(gb_Package_PRESTAGEDIR),\
    $(if $(wildcard $(gb_Package_PRESTAGEDIR)/$(call gb_Library_get_instdir,$(1))/$(call gb_Library_get_runtime_filename,$(1))),\
        $(call gb_Library__Library_impl_copy,$(0),$(call gb_Library_get_instdir,$(1))/$(call gb_Library_get_runtime_filename,$(1))),\
        $(call gb_Library__Library_impl,$(1),$(call gb_Library_get_linktargetname,$(1)))\
    ),
    $(call gb_Library__Library_impl,$(1),$(call gb_Library_get_linktargetname,$(1)))\
)

endef

define gb_Library__Library_impl_copy
$(call gb_Package_Package,Library_Copy_$(1),$(gb_Package_PRESTAGEDIR))
$(call gb_Package_set_outdir,Library_Copy_$(1),$(INSTDIR))
$(call gb_Package_add_file,Library_Copy_$(1),$(2),$(2))
$(OUTDIR)/lib/$(notdir $(2)) : $(INSTDIR)/$(2)
endef

define gb_Library__Library_impl
$(call gb_LinkTarget_LinkTarget,$(2),Library_$(1))
$(call gb_LinkTarget_set_targettype,$(2),Library)
$(call gb_LinkTarget_add_libs,$(2),$(gb_STDLIBS))
$(call gb_LinkTarget_add_defs,$(2),\
	$(gb_Library_DEFS) \
)
$(call gb_Library__get_final_target,$(1)) : $(call gb_Library_get_target,$(1))
$(call gb_Library_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2)) \
	| $(dir $(call gb_Library_get_target,$(1))).dir
$(call gb_Library_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS :=
$(call gb_Library_Library_platform,$(1),$(2),$(gb_Library_DLLDIR)/$(call gb_Library_get_dllname,$(1)))

ifneq ($(gb_RUNNABLE_INSTDIR),)
$(if $(filter $(call gb_Library_get_layer,$(1)):%,$(gb_Library_LAYER_DIRS)),\
    $(call gb_Library__Library_package,$(1),$(call gb_Library_get_packagename,$(1)),$(call gb_Library_get_runtime_filename,$(1))) \
)
endif

$$(eval $$(call gb_Module_register_target,$(call gb_Library__get_final_target,$(1)),$(call gb_Library_get_clean_target,$(1))))

$(call gb_Helper_make_userfriendly_targets,$(1),Library,$(call gb_Library__get_final_target,$(1)))
$(call gb_Deliver_add_deliverable,$(call gb_Library_get_target,$(1)),$(call gb_LinkTarget_get_target,$(2)),$(1))

endef

# gb_Library__Library_package library package filename
define gb_Library__Library_package
$(call gb_Package_Package_internal,$(2),$(gb_Helper_OUTDIRLIBDIR))
$(call gb_Package_set_outdir,$(2),$(INSTDIR))
$(call gb_Package_add_file,$(2),$(call gb_Library_get_instdir,$(1))/$(3),$(3))

$(call gb_Library__get_final_target,$(1)) : $(call gb_Package_get_target,$(2))
$(call gb_Package_get_target,$(2)) : $(call gb_Library_get_target,$(1))
$(call gb_Library_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(2))

endef

# Custom definition that does not simply forward to LinkTarget,
# because there are cases where the auxtargets are not delivered to solver...
# The auxtarget is delivered via the rule in Package.mk.
# gb_Library_add_auxtarget library outdirauxtarget
define gb_Library_add_auxtarget
$(call gb_LinkTarget_add_auxtarget,$(call gb_Library_get_linktargetname,$(1)),$(dir $(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,$(1))))/$(notdir $(2)))
$(call gb_Library_get_target,$(1)) : $(2)
$(2) : $(dir $(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,$(1))))/$(notdir $(2))
$(2) :| $(dir $(2)).dir
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS += $(2)

endef

define gb_Library_add_auxtargets
$(foreach aux,$(2),$(call gb_Library_add_auxtarget,$(1),$(aux)))

endef

# gb_Library__add_soversion_link library package linkname
define gb_Library__add_soversion_link
$(call gb_Library_add_auxtarget,$(1),$(3))
ifneq ($(gb_RUNNABLE_INSTDIR),)
$(call gb_Package_add_file,$(2),$(call gb_Library_get_instdir,$(1))/$(notdir $(3)),$(notdir $(3)))
endif

endef

define gb_Library__set_soversion_script
$(call gb_LinkTarget_set_soversion_script,$(call gb_Library_get_linktargetname,$(1)),$(2),$(3))
$(call gb_Library_get_target,$(1)) : SOVERSION := $(2)
$(call gb_Library__add_soversion_link,$(1),$(call gb_Library_get_packagename,$(1)),$(call gb_Library_get_target,$(1)).$(2))

endef

# for libraries that maintain stable ABI: set SOVERSION and version script
# $(call gb_Library_set_soversion_script,soversion,versionscript)
define gb_Library_set_soversion_script
$(if $(2),,$(call gb_Output_error,gb_Library_set_soversion_script: no version))
$(if $(3),,$(call gb_Output_error,gb_Library_set_soversion_script: no script))
$(call gb_Library__set_soversion_script_platform,$(1),$(2),$(3))
endef

# The dependency from workdir component target to outdir library should ensure
# that gb_CppunitTest_use_component can transitively depend on the library.
# But the component target also must be delivered, so a new phony target
# gb_Library__get_final_target has been invented for that purpose...
define gb_Library_set_componentfile
$(call gb_Library_get_target,$(gb_Library__get_name)) : \
	COMPONENT := $$(if $$(and $$(COMPONENT),$(filter-out $(gb_MERGEDLIBS) $(gb_URELIBS),$(1))),\
	  $$(call gb_Output_error,$(1) already has a component file $$(COMPONENT)))$(2)
$(call gb_ComponentTarget_ComponentTarget,$(2),\
	$(call gb_Library__get_componentprefix,$(gb_Library__get_name)),\
	$(call gb_Library_get_runtime_filename,$(gb_Library__get_name)))
$(call gb_Library__get_final_target,$(gb_Library__get_name)) : \
	$(call gb_ComponentTarget_get_target,$(2))
$(call gb_ComponentTarget_get_target,$(2)) :| \
	$(call gb_Library_get_target,$(gb_Library__get_name))
$(call gb_Library_get_clean_target,$(gb_Library__get_name)) : \
	$(call gb_ComponentTarget_get_clean_target,$(2))
endef

gb_Library__get_name = $(if $(filter $(1),$(gb_MERGEDLIBS)),merged,$(if $(filter $(1),$(gb_URELIBS)),urelibs,$(1)))

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
    NONE:vnd.sun.star.expand:\dLO_LIB_DIR/ \
    OOO:vnd.sun.star.expand:\dLO_LIB_DIR/ \
    URELIB:vnd.sun.star.expand:\dURE_INTERNAL_LIB_DIR/ \
    OXT:./ \

define gb_Library__forward_to_Linktarget
gb_Library_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_Library_get_linktargetname,$$(1)),$$(2),$$(3),Library_$$(1))

endef

gb_Library_get_runtime_filename = \
 $(or $(call gb_Library_get_dllname,$(1)),$(call gb_Library_get_filename,$(1)))

$(eval $(foreach method,\
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_objcobject \
	add_objcobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_asmobject \
	add_asmobjects \
	add_exception_objects \
	add_x64_generated_exception_objects \
	add_generated_cobjects \
	add_generated_cxxobjects \
	add_generated_exception_objects \
	add_library_objects \
	use_library_objects \
	add_grammar \
	add_grammars \
	add_scanner \
	add_scanners \
	add_cflags \
	set_cflags \
	add_cxxflags \
	set_cxxflags \
	add_objcxxflags \
	add_objcflags \
	set_objcflags \
	set_objcxxflags \
	add_defs \
	set_defs \
	set_include \
	add_ldflags \
	set_ldflags \
	set_x64 \
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
	use_package \
	use_packages \
	use_unpacked \
	use_headers \
	add_package_headers \
	add_sdi_headers \
	set_precompiled_header \
	add_precompiled_header \
	export_objects_list \
	add_nativeres \
	set_nativeres \
	set_visibility_default \
	set_warnings_not_errors \
	set_generated_cxx_suffix \
,\
	$(call gb_Library__forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4:
