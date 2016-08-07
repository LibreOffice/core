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



gb_ComponentTarget_REPOS := $(gb_REPOS)

gb_ComponentTarget_XSLTCOMMANDFILE := $(SOLARENV)/bin/createcomponent.xslt
gb_ComponentTarget_get_source = $(1)/$(2).component

define gb_ComponentTarget__command
$(call gb_Output_announce,$(3),$(true),CMP,1)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(gb_XSLTPROC) --nonet --stringparam uri \
    	'$(subst \d,$$,$(COMPONENTPREFIX))$(LIBFILENAME)' -o $(1) \
		$(gb_ComponentTarget_XSLTCOMMANDFILE) $(2))
endef

# creates 2 componentfiles: the first is for the installation set,
# the second is for using the component during the build.
# bit of a hack, hopefully inbuild can be removed when solver layout is fixed.
define gb_ComponentTarget__rules
$$(call gb_ComponentTarget_get_inbuild_target,%) : $$(call gb_ComponentTarget_get_source,$(1),%) | $(gb_XSLTPROCTARGET)
	$$(call gb_ComponentTarget__command,$$@,$$<,$$*)

$$(call gb_ComponentTarget_get_target,%) : $$(call gb_ComponentTarget_get_source,$(1),%) | $(gb_XSLTPROCTARGET)
	$$(call gb_ComponentTarget__command,$$@,$$<,$$*)

endef

$(call gb_ComponentTarget_get_clean_target,%) :
	$(call gb_Output_announce,$$*,$(false),CMP,1)
	rm -f $(call gb_ComponentTarget_get_outdir_target,$*) \
		$(call gb_ComponentTarget_get_target,$*) \
		$(call gb_ComponentTarget_get_outdir_inbuild_target,$*) \
		$(call gb_ComponentTarget_get_inbuild_target,$*) \

$(foreach repo,$(gb_ComponentTarget_REPOS),$(eval $(call gb_ComponentTarget__rules,$(repo))))

$(call gb_ComponentTarget_get_target,%) :
	$(eval $(call gb_Outpt_error,Unable to find component file $(call gb_ComponentTarget_get_source,,$*) in the repositories: $(gb_ComponentTarget_REPOS) or xlstproc is missing.))

$(call gb_ComponentTarget_get_external_target,%) :
	$(call gb_Deliver_deliver,$<,$@)

define gb_ComponentTarget_ComponentTarget
$(call gb_ComponentTarget_get_target,$(1)) : LIBFILENAME := $(3)
$(call gb_ComponentTarget_get_inbuild_target,$(1)) : LIBFILENAME := $(3)
$(call gb_ComponentTarget_get_target,$(1)) : COMPONENTPREFIX := $(2)
$(call gb_ComponentTarget_get_inbuild_target,$(1)) : \
	COMPONENTPREFIX := $(call gb_Library__get_layer_componentprefix,NONE)
$(call gb_ComponentTarget_get_outdir_target,$(1)) : $(call gb_ComponentTarget_get_target,$(1))
$(call gb_ComponentTarget_get_outdir_inbuild_target,$(1)) : \
	$(call gb_ComponentTarget_get_inbuild_target,$(1))
$(call gb_Deliver_add_deliverable,$(call gb_ComponentTarget_get_outdir_target,$(1)),$(call gb_ComponentTarget_get_target,$(1)),$(1))
$(call gb_Deliver_add_deliverable,$(call gb_ComponentTarget_get_outdir_inbuild_target,$(1)),$(call gb_ComponentTarget_get_inbuild_target,$(1)),$(1))

endef

# vim: set noet sw=4 ts=4:
