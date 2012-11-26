# *************************************************************
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
# *************************************************************

# WinResTarget class

gb_WinResTarget_DEFAULTDEFS := $(gb_RCDEFS)

define gb_WinResTarget_WinResTarget
$(call gb_WinResTarget_WinResTarget_init,$(1))
$$(eval $$(call gb_Module_register_target,$(call gb_WinResTarget_get_target,$(1)),$(call gb_WinResTarget_get_clean_target,$(1))))

endef

define gb_WinResTarget_WinResTarget_init
$(call gb_WinResTarget_get_target,$(1)) : DEFS := $(gb_WinResTarget_DEFAULTDEFS)
$(call gb_WinResTarget_get_target,$(1)) : INCLUDE := $(SOLARINC)
$(call gb_WinResTarget_get_clean_target,$(1)) : RCFILE :=
$(call gb_WinResTarget_get_target,$(1)) : RCFILE :=
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_target,$(1)) : $(call gb_WinResTarget_get_dep_target,$(1))
ifneq ($(wildcard $(call gb_WinResTarget_get_dep_target,$(1))),)
include $(call gb_WinResTarget_get_dep_target,$(1))
else
$(firstword $(MAKEFILE_LIST)) : $(call gb_WinResTarget_get_dep_target,$(1))
endif
$(call gb_WinResTarget_get_dep_target,$(1)) : DEFS := $$(gb_WinResTarget_DEFAULTDEFS)
$(call gb_WinResTarget_get_dep_target,$(1)) : INCLUDE := $$(gb_WinResTarget_INCLUDE)
$(call gb_WinResTarget_get_dep_target,$(1)) : RCFILE :=
endif

endef

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_dep_target,%) : $(gb_Helper_MISCDUMMY)
		mkdir -p $(dir $@) && \
			echo '$(call gb_WinResTarget_get_target,$*) : $$(gb_Helper_PHONY)' > $@
endif


$(call gb_WinResTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),RES,1)
	$(call gb_WinResTarget__command_dep,$*,$<)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@))
	$(call gb_WinResTarget__command,$@)

$(call gb_WinResTarget_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_WinResTarget_get_target,$*))

define gb_WinResTarget_set_defs
$(call gb_WinResTarget_get_target,$(1)) : DEFS := $(2)
$(call gb_WinResTarget_get_dep_target,$(1)) : DEFS := $(2)

endef

define gb_WinResTarget_set_include
$(call gb_WinResTarget_get_target,$(1)) : INCLUDE := $(2)

endef

define gb_WinResTarget_add_file
$(call gb_WinResTarget_get_clean_target,$(1)) : RCFILE=$(gb_Helper_SRCDIR_NATIVE)/$(2).rc
$(call gb_WinResTarget_get_target,$(1)) : RCFILE=$(foreach file,$(gb_REPOS),$(realpath $(file)/$(strip $(2)).rc))
$(call gb_WinResTarget_get_target,$(1)) : $(foreach file,$(gb_REPOS),$(realpath $(file)/$(strip $(2)).rc))

endef

define gb_WinResTarget_add_dependency
$(call gb_WinResTarget_get_target,$(1)) : $(foreach file,$(2),$(foreach repo,$(gb_REPOS),$(realpath $(repo)/$(strip $(file)))))

endef
