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


# Overview of dependencies and tasks of Module
#
# target                      task                         depends on
# Module                      build the product            all product targets
#                              excluding tests             recursive Modules
# Module/unitcheck            run unit tests               all unit tests
#                                                          recursive Module/checks
# Module/slowcheck            run all slow unit tests
# Module/subsequentcheck      run system tests             all system tests
#                                                          recursive Module/subsequentchecks
# build (global)              build the product            top-level Module
# unitcheck (global)          run unit tests               top-level Module/unitcheck
# slowcheck (global)          run slow unit tests          top-level Module/slowcheck
# subsequentcheck (global)    run system tests             top-level Module/subsequentcheck
# all (global)                default goal                 build unitcheck


# Module class

gb_Module_ALLMODULES :=
gb_Module_MODULELOCATIONS :=
gb_Module_TARGETSTACK :=
gb_Module_CHECKTARGETSTACK :=
gb_Module_SLOWCHECKTARGETSTACK :=
gb_Module_SUBSEQUENTCHECKTARGETSTACK :=
gb_Module_CLEANTARGETSTACK :=

# The currently read gbuild makefile.
#
# gbuild classes should use this if they need to depend on their makefile
# (e.g., to make sure a zip file is rebuilt if files are removed from it).
# Because makefiles may include other makefiles, it is not safe to rely
# on $(MAKEFILE_LIST).
gb_Module_CURRENTMAKEFILE :=

$(call gb_Module_get_almost_target,%) :
	$(call gb_Output_announce,$*,$(false),ALM,5)
	-$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		touch $@)

.PHONY : $(call gb_Module_get_clean_target,%)
$(call gb_Module_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),MOD,5)
	$(call gb_Output_announce_title,module $* cleared.)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Module_get_target,$*) $(call gb_Module_get_almost_target,$*) $(call gb_Module_get_check_target,$*) $(call gb_Module_get_slowcheck_target,$*) $(call gb_Module_get_subsequentcheck_target,$*))

$(call gb_Module_get_check_target,%) :
	$(call gb_Output_announce,$*,$(true),CHK,5)
	$(call gb_Output_announce_title,module $* checks done.)
	-$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		touch $@)

$(call gb_Module_get_slowcheck_target,%) :
	$(call gb_Output_announce,$*,$(true),SLC,5)
	$(call gb_Output_announce_title,module $* slowchecks done.)
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

.PHONY : all build unitcheck slowcheck subsequentcheck clean check debugrun help showmodules translations
.DEFAULT_GOAL := all

all : build $(if $(CROSS_COMPILING),,unitcheck $(if $(gb_PARTIAL_BUILD),,slowcheck))

build-tools : $(gb_BUILD_TOOLS)
	$(call gb_Output_announce,loaded tools: $(gb_BUILD_TOOLS),$(true),ALL,6)
	$(call gb_Output_announce_title,build-tools done.)
	$(call gb_Output_announce_bell)

build :
	$(call gb_Output_announce,top level modules: $(foreach module,$(filter-out deliverlog $(WORKDIR)/bootstrap,$^),$(notdir $(module))),$(true),ALL,6)
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(true),ALL,6)
	$(call gb_Output_announce_title,build done.)
	$(call gb_Output_announce_bell)

unitcheck :
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(true),CHK,6)
	$(call gb_Output_announce_title,all unittests checked.)
	$(call gb_Output_announce_bell)

slowcheck :
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(true),SLC,6)
	$(call gb_Output_announce_title,all slowtests checked.)
	$(call gb_Output_announce_bell)

# removing the dependency on build for now until we can make a full build with gbuild
#subsequentcheck : all
subsequentcheck :
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(true),SCK,6)
	$(call gb_Output_announce_title,all subsequent tests checked.)
	$(call gb_Output_announce_bell)

clean :
	$(call gb_Output_announce,top level modules: $(foreach module,$^,$(notdir $(module))),$(false),ALL,6)
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(false),ALL,6)
	$(call gb_Output_announce_title,all cleared.)
	$(call gb_Output_announce_bell)

check : unitcheck slowcheck
	$(call gb_Output_announce_title,all tests checked.)
	$(call gb_Output_announce_bell)

debugrun :
	$(call gb_Module_DEBUGRUNCOMMAND)

help :
	@cat $(SRCDIR)/solenv/gbuild/gbuild.help.txt

showmodules :
	$(info $(strip $(gb_Module_ALLMODULES)))
	@true

translations : $(WORKDIR)/pot.done

$(WORKDIR)/pot.done : $(foreach exec,cfgex helpex localize transex3 \
									propex uiex ulfex xrmex treex, \
							$(call gb_Executable_get_target_for_build,$(exec)))
	$(call gb_Output_announce,$(subst .pot,,$(subst $(WORKDIR)/,,$@)),$(true),POT,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && $(call gb_Helper_execute,localize) $(SRCDIR) $(dir $@)/pot \
		&& find $(dir $@)/pot -type f -printf "%P\n" | sed -e "s/\.pot/.po/" > $(dir $@)/LIST \
		&& touch $@)

# enable if: no "-MODULE/" defined AND ["all" defined OR "MODULE/" defined]
gb_Module__debug_enabled = \
 $(and $(if $(filter -$(1)/,$(ENABLE_DEBUGINFO_FOR)),,$(true)),\
       $(filter all $(1)/,$(ENABLE_DEBUGINFO_FOR)))

define gb_Module_Module
gb_Module_ALLMODULES += $(1)
gb_Module_MODULELOCATIONS += $(1):$(dir $(realpath $(lastword $(MAKEFILE_LIST))))
gb_Module_TARGETSTACK := $(call gb_Module_get_target,$(1)) $(gb_Module_TARGETSTACK)
gb_Module_CHECKTARGETSTACK := $(call gb_Module_get_check_target,$(1)) $(gb_Module_CHECKTARGETSTACK)
gb_Module_SLOWCHECKTARGETSTACK := $(call gb_Module_get_slowcheck_target,$(1)) $(gb_Module_SLOWCHECKTARGETSTACK)
gb_Module_SUBSEQUENTCHECKTARGETSTACK := $(call gb_Module_get_subsequentcheck_target,$(1)) $(gb_Module_SUBSEQUENTCHECKTARGETSTACK)
gb_Module_CLEANTARGETSTACK := $(call gb_Module_get_clean_target,$(1)) $(gb_Module_CLEANTARGETSTACK)
gb_Module_CURRENTMODULE_DEBUG_ENABLED := $(call gb_Module__debug_enabled,$(1))
gb_Module_CURRENTMODULE_NAME := $(1)
$(call gb_Helper_make_userfriendly_targets,$(1),Module)
$(if $(filter-out libreoffice instsetoo_native android ios,$(1)),\
    $(call gb_Postprocess_register_target,AllModulesButInstsetNative,Module,$(1)))

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
gb_Module_CURRENTMAKEFILE := $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Module_MODULELOCATIONS)))$(2).mk
include $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Module_MODULELOCATIONS)))$(2).mk
gb_Module_CURRENTMAKEFILE :=
ifneq ($$(words $$(gb_Module_CURRENTTARGET)) $$(words $$(gb_Module_CURRENTCLEANTARGET)),1 1)
$$(eval $$(call gb_Output_error,No $(3) registered while reading $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Module_MODULELOCATIONS)))$(2).mk!))
endif

endef

define gb_Module_add_target
$(call gb_Module__read_targetfile,$(1),$(2),target)

$(call gb_Module_get_almost_target,$(1)) : $$(gb_Module_CURRENTTARGET)
$(call gb_Module_get_clean_target,$(1)) : $$(gb_Module_CURRENTCLEANTARGET)

endef

define gb_Module_add_check_target
$(call gb_Module__read_targetfile,$(1),$(2),check target)

$(call gb_Module_get_check_target,$(1)) : $$(gb_Module_CURRENTTARGET)
$(call gb_Module_get_clean_target,$(1)) : $$(gb_Module_CURRENTCLEANTARGET)

endef

define gb_Module_add_slowcheck_target
$(call gb_Module__read_targetfile,$(1),$(2),slowcheck target)

$(call gb_Module_get_slowcheck_target,$(1)) : $$(gb_Module_CURRENTTARGET)
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
$(call gb_Module_get_slowcheck_target,$(1)) : $$(firstword $$(gb_Module_SLOWCHECKTARGETSTACK))
$(call gb_Module_get_subsequentcheck_target,$(1)) : $$(firstword $$(gb_Module_SUBSEQUENTCHECKTARGETSTACK))
$(call gb_Module_get_clean_target,$(1)) : $$(firstword $$(gb_Module_CLEANTARGETSTACK))
gb_Module_TARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_TARGETSTACK)),$$(gb_Module_TARGETSTACK))
gb_Module_CHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CHECKTARGETSTACK)),$$(gb_Module_CHECKTARGETSTACK))
gb_Module_SLOWCHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_SLOWCHECKTARGETSTACK)),$$(gb_Module_SLOWCHECKTARGETSTACK))
gb_Module_SUBSEQUENTCHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_SUBSEQUENTCHECKTARGETSTACK)),$$(gb_Module_SUBSEQUENTCHECKTARGETSTACK))
gb_Module_CLEANTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CLEANTARGETSTACK)),$$(gb_Module_CLEANTARGETSTACK))

endef

define gb_Module_add_targets
$(call gb_Module_get_target,$(1)) : $(call gb_Module_get_almost_target,$(1))
$(foreach target,$(2),$(call gb_Module_add_target,$(1),$(target)))

endef

gb_Module_add_targets_for_build = $(call gb_Module_add_targets,$(1),$(2))

define gb_Module_add_check_targets
$(foreach target,$(2),$(call gb_Module_add_check_target,$(1),$(target)))

endef

define gb_Module_add_slowcheck_targets
$(foreach target,$(2),$(call gb_Module_add_slowcheck_target,$(1),$(target)))

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

build : $$(firstword $$(gb_Module_TARGETSTACK))
unitcheck : $$(firstword $$(gb_Module_CHECKTARGETSTACK))
slowcheck : $$(firstword $$(gb_Module_SLOWCHECKTARGETSTACK))
subsequentcheck : $$(firstword $$(gb_Module_SUBSEQUENTCHECKTARGETSTACK))
clean : $$(firstword $$(gb_Module_CLEANTARGETSTACK))

ifneq ($$(words $$(gb_Module_TARGETSTACK)),1)
$$(eval $$(call gb_Output_error,Corrupted module target stack! $(gb_Module_TARGETSTACK)))
endif

gb_Module_TARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_TARGETSTACK)),$$(gb_Module_TARGETSTACK))
gb_Module_CHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CHECKTARGETSTACK)),$$(gb_Module_CHECKTARGETSTACK))
gb_Module_SLOWCHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_SLOWCHECKTARGETSTACK)),$$(gb_Module_SLOWCHECKTARGETSTACK))
gb_Module_SUBSEQUENTCHECKTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_SUBSEQUENTCHECKTARGETSTACK)),$$(gb_Module_SUBSEQUENTCHECKTARGETSTACK))
gb_Module_CLEANTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CLEANTARGETSTACK)),$$(gb_Module_CLEANTARGETSTACK))

ifneq ($$(and $$(gb_Module_TARGETSTACK),$$(gb_Module_CHECKTARGETSTACK),$$(gb_Module_SLOWCHECKTARGETSTACK),$$(gb_Module_SUBSEQUENTCHECKTARGETSTACK)),)
$$(eval $$(call gb_Output_error,Corrupted module target stack!3))
endif

$$(eval $$(gb_Extensions_final_hook))
endef

# vim: set noet sw=4:
