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




# Static Library class

# defined globally in gbuild.mk
#  gb_StaticLibrary_OUTDIRLOCATION := $(OUTDIR)/lib
# defined by platform
#  gb_StaticLibrary_DEFS
#  gb_StaticLibrary_FILENAMES
#  gb_StaticLibrary_TARGETS

gb_StaticLibrary__get_linktargetname = StaticLibrary/$(call gb_StaticLibrary_get_filename,$(1))

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
.PHONY : $(WORKDIR)/Clean/OutDir/lib/%$(gb_StaticLibrary_PLAINEXT)
$(WORKDIR)/Clean/OutDir/lib/%$(gb_StaticLibrary_PLAINEXT) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(OUTDIR)/lib/$*$(gb_StaticLibrary_PLAINEXT) \
			$(AUXTARGETS))

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
$(gb_StaticLibrary_OUTDIRLOCATION)/%$(gb_StaticLibrary_PLAINEXT) : 
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Deliver_deliver,$<,$@) \
			$(foreach target,$(AUXTARGETS), && $(call gb_Deliver_deliver,$(dir $<)/$(notdir $(target)),$(target))))

define gb_StaticLibrary_StaticLibrary
ifeq (,$$(findstring $(1),$$(gb_StaticLibrary_KNOWNLIBS)))
$$(eval $$(call gb_Output_info,Currently known static libraries are: $(sort $(gb_StaticLibrary_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Static library $(1) must be registered in Repository.mk))
endif
$(call gb_StaticLibrary_get_target,$(1)) : AUXTARGETS :=
$(call gb_StaticLibrary__StaticLibrary_impl,$(1),$(call gb_StaticLibrary__get_linktargetname,$(1)))

endef

define gb_StaticLibrary__StaticLibrary_impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype,$(2),StaticLibrary)
$(call gb_LinkTarget_add_defs,$(2),\
	$(gb_StaticLibrary_DEFS) \
)
$(call gb_StaticLibrary_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_StaticLibrary_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_StaticLibrary_StaticLibrary_platform,$(1),$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_StaticLibrary_get_target,$(1)),$(call gb_StaticLibrary_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_StaticLibrary_get_target,$(1)),$(call gb_LinkTarget_get_target,$(2)),$(1))

endef

define gb_StaticLibrary_forward_to_Linktarget
gb_StaticLibrary_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_StaticLibrary__get_linktargetname,$$(1)),$$(2),$$(3))

endef

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
	add_package_headers \
	add_sdi_headers \
	add_precompiled_header \
,\
	$(call gb_StaticLibrary_forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4 ts=4:
