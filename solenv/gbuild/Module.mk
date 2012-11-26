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




# Module class

gb_Module_ALLMODULES :=
gb_Module_MODULELOCATIONS :=
gb_Module_TARGETSTACK :=
gb_Module_CHECKTARGETSTACK :=
gb_Module_SUBSEQUENTCHECKTARGETSTACK :=
gb_Module_CLEANTARGETSTACK :=

.PHONY : $(call gb_Module_get_clean_target,%)
$(call gb_Module_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),MOD,5)
	$(call gb_Output_announce_title,module $* cleared.)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Module_get_target,$*) $(call gb_Module_get_check_target,$*) $(call gb_Module_get_subsequentcheck_target,$*))

$(call gb_Module_get_check_target,%) :
	$(call gb_Output_announce,$*,$(true),CHK,5)
	$(call gb_Output_announce_title,module $* checks done.)
	-$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		touch $@)

$(call gb_Module_get_subsequentcheck_target,%) :
	$(call gb_Output_announce,$*,$(true),SCK,5)
	$(call gb_Output_announce_title,module $* subsequentchecks done.)
	-$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		touch $@)

$(call gb_Module_get_target,%) :
	$(call gb_Output_announce,$*,$(true),MOD,5)
	$(call gb_Output_announce_title,module $* done.)
	-$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		touch $@)

.PHONY : all allandcheck clean check subsequentcheck
.DEFAULT_GOAL := allandcheck

allandcheck : all check

# compatibility with the old build system
ifneq ($(strip $(OOO_SUBSEQUENT_TESTS)),)
.DEFAULT_GOAL := subsequentcheck
endif

all : 
	$(call gb_Output_announce,top level modules: $(foreach module,$(filter-out deliverlog,$^),$(notdir $(module))),$(true),ALL,6)
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(true),ALL,6)
	$(call gb_Output_announce_title,all done.)
	$(call gb_Output_announce_bell)

check :
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(true),CHK,6)
	$(call gb_Output_announce_title,all tests checked.)
	$(call gb_Output_announce_bell)

subsequentcheck : all 
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(true),SCK,6)
	$(call gb_Output_announce_title,all subsequent tests checked.)
	$(call gb_Output_announce_bell)

clean : 
	$(call gb_Output_announce,top level modules: $(foreach module,$^,$(notdir $(module))),$(false),ALL,6)
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(false),ALL,6)
	$(call gb_Output_announce_title,all cleared.)
	$(call gb_Output_announce_bell)

define gb_Module_Module
gb_Module_ALLMODULES += $(1)
gb_Module_MODULELOCATIONS += $(1):$(dir $(realpath $(lastword $(MAKEFILE_LIST))))
gb_Module_TARGETSTACK := $(call gb_Module_get_target,$(1)) $(gb_Module_TARGETSTACK)
gb_Module_CHECKTARGETSTACK := $(call gb_Module_get_check_target,$(1)) $(gb_Module_CHECKTARGETSTACK)
gb_Module_SUBSEQUENTCHECKTARGETSTACK := $(call gb_Module_get_subsequentcheck_target,$(1)) $(gb_Module_SUBSEQUENTCHECKTARGETSTACK)
gb_Module_CLEANTARGETSTACK := $(call gb_Module_get_clean_target,$(1)) $(gb_Module_CLEANTARGETSTACK)

endef

# This is called inside the included file and pushes one target on each stack.
# This has to be called with full late evaluation ($$(eval $$(call ))) and
# should never be inlined ($(call )) as the calls defining it might be sourced
# before gb_Module.
define gb_Module_register_target
gb_Module_CURRENTTARGET := $(1)
gb_Module_CURRENTCLEANTARGET := $(2)

endef

# Here we include the file (in it there will be a call to gb_Module_register_target)
define gb_Module__read_targetfile
gb_Module_CURRENTTARGET :=
gb_Module_CURRENTCLEANTARGET :=
include $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Module_MODULELOCATIONS)))$(2).mk
ifneq ($$(words $$(gb_Module_CURRENTTARGET)) $$(words $$(gb_Module_CURRENTCLEANTARGET)),1 1)
$$(eval $$(call gb_Output_error,No $(3) registered while reading $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Module_MODULELOCATIONS)))$(2).mk!))
endif

endef

define gb_Module_add_target
$(call gb_Module__read_targetfile,$(1),$(2),target)

$(call gb_Module_get_target,$(1)) : $$(gb_Module_CURRENTTARGET)
$(call gb_Module_get_clean_target,$(1)) : $$(gb_Module_CURRENTCLEANTARGET)

endef

define gb_Module_add_check_target
$(call gb_Module__read_targetfile,$(1),$(2),check target)

$(call gb_Module_get_check_target,$(1)) : $$(gb_Module_CURRENTTARGET)
$(call gb_Module_get_clean_target,$(1)) : $$(gb_Module_CURRENTCLEANTARGET)

endef

define gb_Module_add_subsequentcheck_target
$(call gb_Module__read_targetfile,$(1),$(2),subsequentcheck target)

$(call gb_Module_get_subsequentcheck_target,$(1)) : $$(gb_Module_CURRENTTARGET)
$(call gb_Module_get_clean_target,$(1)) : $$(gb_Module_CURRENTCLEANTARGET)

endef

define gb_Module_add_moduledir
include $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Module_MODULELOCATIONS)))/$(2)/Module_$(2).mk
$(call gb_Module_get_target,$(1)) : $$(firstword $$(gb_Module_TARGETSTACK))
$(call gb_Module_get_check_target,$(1)) : $$(firstword $$(gb_Module_CHECKTARGETSTACK))
$(call gb_Module_get_subsequentcheck_target,$(1)) : $$(firstword $$(gb_Module_SUBSEQUENTCHECKTARGETSTACK))
$(call gb_Module_get_clean_target,$(1)) : $$(firstword $$(gb_Module_CLEANTARGETSTACK))
gb_Module_TARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_TARGETSTACK)),$$(gb_Module_TARGETSTACK))
gb_Module_CHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CHECKTARGETSTACK)),$$(gb_Module_CHECKTARGETSTACK))
gb_Module_SUBSEQUENTCHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_SUBSEQUENTCHECKTARGETSTACK)),$$(gb_Module_SUBSEQUENTCHECKTARGETSTACK))
gb_Module_CLEANTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CLEANTARGETSTACK)),$$(gb_Module_CLEANTARGETSTACK))

endef

define gb_Module_add_targets
$(foreach target,$(2),$(call gb_Module_add_target,$(1),$(target)))

endef

define gb_Module_add_check_targets
$(foreach target,$(2),$(call gb_Module_add_check_target,$(1),$(target)))

endef

define gb_Module_add_subsequentcheck_targets
$(foreach target,$(2),$(call gb_Module_add_subsequentcheck_target,$(1),$(target)))

endef

define gb_Module_add_moduledirs
$(foreach target,$(2),$(call gb_Module_add_moduledir,$(1),$(target)))
endef

define gb_Module_make_global_targets
ifneq ($$(gb_Module_TARGETSTACK),)
$$(eval $$(call gb_Output_error,Corrupted module target stack!1))
endif

include $(1)

all : $$(firstword $$(gb_Module_TARGETSTACK))
check : $$(firstword $$(gb_Module_CHECKTARGETSTACK))
subsequentcheck : $$(firstword $$(gb_Module_SUBSEQUENTCHECKTARGETSTACK))
clean : $$(firstword $$(gb_Module_CLEANTARGETSTACK))

ifneq ($$(words $$(gb_Module_TARGETSTACK)),1)
$$(eval $$(call gb_Output_error,Corrupted module target stack! $(gb_Module_TARGETSTACK)))
endif

gb_Module_TARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_TARGETSTACK)),$$(gb_Module_TARGETSTACK))
gb_Module_CHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CHECKTARGETSTACK)),$$(gb_Module_CHECKTARGETSTACK))
gb_Module_SUBSEQUENTCHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_SUBSEQUENTCHECKTARGETSTACK)),$$(gb_Module_SUBSEQUENTCHECKTARGETSTACK))
gb_Module_CLEANTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CLEANTARGETSTACK)),$$(gb_Module_CLEANTARGETSTACK))

ifneq ($$(and $$(gb_Module_TARGETSTACK),$$(gb_Module_CHECKTARGETSTACK),$$(gb_Module_SUBSEQUENTCHECKTARGETSTACK)),)
$$(eval $$(call gb_Output_error,Corrupted module target stack!3))
endif

endef

# vim: set noet sw=4 ts=4:
