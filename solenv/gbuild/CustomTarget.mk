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



gb_CustomTarget_REPOSITORYNAMES := $(gb_Helper_REPOSITORYNAMES)

# N.B.: putting the "+" there at the start activates the GNU make job server
define gb_CustomTarget__command
+$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(call gb_CustomTarget_get_workdir,$(2)) && \
	$(MAKE) -C $(call gb_CustomTarget_get_workdir,$(2)) -f $< && \
	touch $(1))

endef

$(call gb_CustomTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),MAK,3)
	$(call gb_CustomTarget__command,$@,$*)

.PHONY: $(call gb_CustomTarget_get_clean_target,%)
$(call gb_CustomTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),MAK,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_CustomTarget_get_workdir,$*) && \
		rm -f $(call gb_CustomTarget_get_target,$*))

define gb_CustomTarget__get_makefile
$(1)/$(2)/Makefile
endef

define gb_CustomTarget_CustomTarget
$(if $(filter $(2),$(gb_CustomTarget_REPOSITORYNAMES)),,\
 $(error CustomTarget: no or invalid repository given; known repositories: \
  $(gb_JavaClassSet_REPOSITORYNAMES)))
gb_CustomTarget_REPO_$(1) := $(2)
$(call gb_CustomTarget_get_target,$(1)) : \
  $(call gb_CustomTarget__get_makefile,$($(2)),$(1))
$(call gb_CustomTarget_get_workdir,$(1))/% : \
  $(call gb_CustomTarget_get_target,$(1))
$(eval $(call gb_Module_register_target,$(call gb_CustomTarget_get_target,$(1)),$(call gb_CustomTarget_get_clean_target,$(1))))
endef


define gb_CustomTarget_add_dependency
$(eval $(call gb_CustomTarget_get_target,$(1)) : \
	$($(gb_CustomTarget_REPO_$(1)))/$(2))
endef

define gb_CustomTarget_add_dependencies
$(foreach dependency,$(2),$(call gb_CustomTarget_add_dependency,$(1),$(dependency)))
endef

define gb_CustomTarget_add_outdir_dependency
$(eval $(call gb_CustomTarget_get_target,$(1)) : $(2))
endef

define gb_CustomTarget_add_outdir_dependencies
$(foreach dependency,$(2),$(call gb_CustomTarget_add_outdir_dependency,$(1),$(dependency)))

endef

# vim: set noet sw=4 ts=4:
