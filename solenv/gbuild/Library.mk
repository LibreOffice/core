#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************




# Library class

# defined globally in TargetLocations.mk
#  gb_Library_OUTDIRLOCATION := $(OUTDIR)/lib
#  gb_Library_DLLDIR := $(WORKDIR)/LinkTarget/Library
# defined by platform
#  gb_Library_COMPONENTPREFIXES
#  gb_Library_DEFS
#  gb_Library_DLLFILENAMES
#  gb_Library_FILENAMES
#  gb_Library_Library_platform
#  gb_Library_TARGETS

gb_Library__get_linktargetname = Library/$(call gb_Library_get_filename,$(1))

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
.PHONY : $(WORKDIR)/Clean/OutDir/lib/%$(gb_Library_PLAINEXT)
$(WORKDIR)/Clean/OutDir/lib/%$(gb_Library_PLAINEXT) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(OUTDIR)/lib/$*$(gb_Library_PLAINEXT) \
			$(AUXTARGETS))

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
$(gb_Library_OUTDIRLOCATION)/%$(gb_Library_PLAINEXT) : 
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Deliver_deliver,$<,$@) \
			$(foreach target,$(AUXTARGETS), && $(call gb_Deliver_deliver,$(dir $<)/$(notdir $(target)),$(target))))

define gb_Library_Library
ifeq (,$$(findstring $(1),$$(gb_Library_KNOWNLIBS)))
$$(eval $$(call gb_Output_info,Currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Library $(1) must be registered in Repository.mk))
endif
$(call gb_Library_get_target,$(1)) : AUXTARGETS :=
$(call gb_Library__Library_impl,$(1),$(call gb_Library__get_linktargetname,$(1)))
$(call gb_Library_add_default_nativeres,$(1),default)

endef

define gb_Library__Library_impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype,$(2),Library)
$(call gb_LinkTarget_add_defs,$(2),\
	$(gb_Library_DEFS) \
)
$(call gb_Library_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_Library_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_Library_Library_platform,$(1),$(2),$(gb_Library_DLLDIR)/$(call gb_Library_get_dllname,$(1)))
$$(eval $$(call gb_Module_register_target,$(call gb_Library_get_target,$(1)),$(call gb_Library_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_Library_get_target,$(1)),$(call gb_LinkTarget_get_target,$(2)),$(1))

endef

define gb_Library_set_componentfile
$(call gb_ComponentTarget_ComponentTarget,$(2),$(call gb_Library__get_componentprefix,$(1)),$(call gb_Library_get_runtime_filename,$(1)))
$(call gb_Library_get_target,$(1)) : $(call gb_ComponentTarget_get_outdir_target,$(2)) $(call gb_ComponentTarget_get_outdir_inbuild_target,$(2))
$(call gb_Library_get_clean_target,$(1)) : $(call gb_ComponentTarget_get_clean_target,$(2))

endef

gb_Library__get_componentprefix = \
    $(call gb_Library__get_layer_componentprefix,$(call \
        gb_Library_get_layer,$(1)))

gb_Library__get_layer_componentprefix = \
    $(patsubst $(1):%,%,$(or \
        $(filter $(1):%,$(gb_Library_COMPONENTPREFIXES)), \
        $(call gb_Output_error,no ComponentTarget native prefix for layer '$(1)')))


define gb_Library__forward_to_Linktarget
gb_Library_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_Library__get_linktargetname,$$(1)),$$(2),$$(3))

endef

gb_Library_get_runtime_filename = \
 $(or $(call gb_Library_get_dllname,$(1)),$(call gb_Library_get_filename,$(1)))

$(eval $(foreach method,\
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_exception_objects \
	add_noexception_objects \
	add_generated_exception_objects \
	set_yaccflags \
	add_cflags \
	set_cflags \
	add_cxxflags \
	set_cxxflags \
	add_objcxxflags \
	set_objcxxflags \
	add_defs \
	set_defs \
	set_include \
	add_ldflags \
	set_ldflags \
	add_libs \
	set_library_path_flags \
	add_api \
	add_linked_libs \
	add_linked_static_libs \
	add_external_libs \
	use_external \
	use_externals \
	add_package_headers \
	add_sdi_headers \
	add_precompiled_header \
,\
	$(call gb_Library__forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4 ts=4:
