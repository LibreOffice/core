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




# PackagePart class

$(foreach destination,$(call gb_PackagePart_get_destinations), $(destination)/%) :
	$(call gb_Deliver_deliver,$<,$@)

define gb_PackagePart_PackagePart
$(OUTDIR)/$(1) : $(2)
$(2) :| $(3)
$(call gb_Deliver_add_deliverable,$(OUTDIR)/$(1),$(2),$(3))
endef


# Package class

.PHONY : $(call gb_Package_get_clean_target,%)
$(call gb_Package_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PKG,2)
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(FILES)) \
	&& cat $${RESPONSEFILE} | xargs rm -f \
 	&& rm -f $${RESPONSEFILE}

$(call gb_Package_get_preparation_target,%) :
	mkdir -p $(dir $@) && touch $@

$(call gb_Package_get_target,%) :
	$(call gb_Output_announce,$*,$(true),PKG,2)
	mkdir -p $(dir $@) && touch $@


define gb_Package_Package
gb_Package_SOURCEDIR_$(1) := $(2)
$(call gb_Package_get_clean_target,$(1)) : FILES := $(call gb_Package_get_target,$(1)) $(call gb_Package_get_preparation_target,$(1))
$$(eval $$(call gb_Module_register_target,$(call gb_Package_get_target,$(1)),$(call gb_Package_get_clean_target,$(1))))
$(call gb_Package_get_target,$(1)) : $(call gb_Package_get_preparation_target,$(1))

endef

define gb_Package_add_file
$(call gb_Package_get_target,$(1)) : $(OUTDIR)/$(2)
$(call gb_Package_get_clean_target,$(1)) : FILES += $(OUTDIR)/$(2)
$(call gb_PackagePart_PackagePart,$(2),$$(gb_Package_SOURCEDIR_$(1))/$(3),$(call gb_Package_get_preparation_target,$(1)))

endef

#
# Add multiple files $(3), which are located in directory $(2) to the package $(1). 
#
define gb_Package_add_files
$(foreach header,$(3), $(call gb_Package_add_file,$(strip $(1)),$(strip $(2))/$(header),$(header)))
endef

define gb_Package_add_customtarget
$(call gb_Package_get_preparation_target,$(1)) : $(call gb_CustomTarget_get_target,$(2))
$(call gb_Package_get_clean_target,$(1)) : $(call gb_CustomTarget_get_clean_target,$(2))
$(eval $(call gb_CustomTarget_CustomTarget,$(2),$(3)))

endef

# vim: set noet sw=4 ts=4:
