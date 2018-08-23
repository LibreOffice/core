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



# Executable class

# defined by platform
#  gb_Executable_Executable_platform

.PHONY : $(call gb_Executable_get_clean_target,%)
$(call gb_Executable_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Executable_get_target,$*) \
			$(AUXTARGETS))

$(call gb_Executable_get_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Deliver_deliver,$<,$@) \
			$(foreach target,$(AUXTARGETS), && $(call gb_Deliver_deliver,$(dir $<)/$(notdir $(target)),$(target))))

define gb_Executable_Executable
$(call gb_Executable__Executable_impl,$(1),Executable/$(1)$(gb_Executable_EXT))

endef

define gb_Executable__Executable_impl
$(call gb_Executable_set_targettype_gui,$(2))
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype,$(2),Executable)
$(call gb_Executable_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_Executable_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_Executable_Executable_platform,$(1),$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_Executable_get_target,$(1)),$(call gb_Executable_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_Executable_get_target,$(1)),$(call gb_LinkTarget_get_target,$(2)),$(1))

endef

define gb_Executable_set_targettype_gui
$(call gb_Executable_get_target,$(1)) : gb_Executable_TARGETGUI := $(2)
endef


define gb_Executable_forward_to_Linktarget
gb_Executable_$(1) = $$(call gb_LinkTarget_$(1),Executable/$$(1)$$(gb_Executable_EXT),$$(2),$$(3))

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
	set_private_api \
	add_linked_libs \
	add_linked_static_libs \
	use_external \
	use_externals \
	add_package_headers \
	add_sdi_headers \
	add_precompiled_header \
,\
	$(call gb_Executable_forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4 ts=4:
