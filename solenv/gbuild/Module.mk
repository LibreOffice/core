# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


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

.PHONY : $(call gb_Module_get_clean_target,%)
$(call gb_Module_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),MOD,5)
	$(call gb_Output_announce_title,module $* cleared.)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Module_get_target,$*) $(call gb_Module_get_check_target,$*) $(call gb_Module_get_slowcheck_target,$*) $(call gb_Module_get_subsequentcheck_target,$*))

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

.PHONY : build all clean unitcheck slowcheck subsequentcheck dev-install showmodules
.DEFAULT_GOAL := all

ifeq ($(strip $(gb_PARTIALBUILD)),)
check : subsequentcheck

# execute debugrun at the end
ifneq ($(filter dev-install build all,$(MAKECMDGOALS)),)
debugrun :| $(filter dev-install build all,$(MAKECMDGOALS))
endif

ifneq ($(OS),WNT)
define gb_Module_BUILDHINT

LibreOffice build successfully finished.

To install, issue: $(MAKE) install
Developers might prefer this way: $(MAKE) dev-install -o build
To run smoketest, issue: $(MAKE) check
For crosscompiles, please consult README.cross how to install it.

endef
else
define gb_Module_BUILDHINT

LibreOffice build succesfully finished.

Please consult instsetoo_native/README on installing it.

endef
endif

define gb_Module_DEVINSTALLHINT

Developer installation finished, you can now execute:

on Linux:
make debugrun

on Mac OS X:
open install/LibreOffice.app

endef
else
gb_Module_BUILDHINT=
gb_Module_DEVINSTALLHINT=

debugrun :| build
endif

all : build unitcheck
	$(info $(gb_Module_BUILDHINT))

build : 
	$(call gb_Output_announce,top level modules: $(foreach module,$(filter-out deliverlog $(WORKDIR)/bootstrap $(SRCDIR)/src.downloaded,$^),$(notdir $(module))),$(true),ALL,6)
	$(call gb_Output_announce,loaded modules: $(sort $(gb_Module_ALLMODULES)),$(true),ALL,6)
	$(call gb_Output_announce_title,build done.)
	$(call gb_Output_announce_bell)
	$(info $(gb_Module_BUILDHINT))

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

dev-install :
	@rm -f $(SRCDIR)/install && \
	ln -s $(DEVINSTALLDIR)/opt/ $(SRCDIR)/install
	$(info $(gb_Module_DEVINSTALLHINT))

showmodules :
	$(info $(strip $(gb_Module_ALLMODULES)))
	@true 

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

ifneq (showmodules,$(MAKECMDGOALS))
define gb_Module_add_target
$(call gb_Module__read_targetfile,$(1),$(2),target)

$(call gb_Module_get_target,$(1)) : $$(gb_Module_CURRENTTARGET)
$(call gb_Module_get_clean_target,$(1)) : $$(gb_Module_CURRENTCLEANTARGET)

endef

ifeq ($(strip $(SKIP_TESTS)),)
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
endif # SKIP_TESTS

define gb_Module_add_subsequentcheck_target
$(call gb_Module__read_targetfile,$(1),$(2),subsequentcheck target)

$(call gb_Module_get_subsequentcheck_target,$(1)) : $$(gb_Module_CURRENTTARGET)
$(call gb_Module_get_clean_target,$(1)) : $$(gb_Module_CURRENTCLEANTARGET)

endef
endif

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
$(foreach target,$(2),$(call gb_Module_add_target,$(1),$(target)))

endef

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
