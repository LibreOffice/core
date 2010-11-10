#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


# Module class

gb_Module_ALLMODULES :=
gb_Module_MODULELOCATIONS :=
gb_Module_TARGETSTACK :=
gb_Module_CLEANTARGETSTACK :=

.PHONY : $(call gb_Module_get_clean_target,%)
$(call gb_Module_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up module $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_Module_get_target,$*))

$(call gb_Module_get_target,%) :
    $(call gb_Helper_announce,Completed module $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && \
        touch $@)

all : 
    $(call gb_Helper_announce,Build for modules $(foreach module,$^,$(notdir $(module))) finished (loaded modules: $(sort $(gb_Module_ALLMODULES))).)

clean : 
    $(call gb_Helper_announce,Cleanup for modules $(foreach module,$^,$(notdir $(module))) finished (loaded modules: $(sort $(gb_Module_ALLMODULES))).)

.PHONY : all clean
.DEFAULT_GOAL := all

define gb_Module_Module
gb_Module_ALLMODULES += $(1)
gb_Module_MODULELOCATIONS += $(1):$(dir $(realpath $(lastword $(MAKEFILE_LIST))))
$(call gb_Module_register_target,$(call gb_Module_get_target,$(1)),$(call gb_Module_get_clean_target,$(1)))

endef

# include the file and pop one target from each stack
define gb_Module_add_target
include $(patsubst $(1):%,%,$(filter $(1):%,$(gb_Module_MODULELOCATIONS)))/$(2).mk
$(call gb_Module_get_target,$(1)) : $$(firstword $$(gb_Module_TARGETSTACK))
$(call gb_Module_get_clean_target,$(1)) : $$(firstword $$(gb_Module_CLEANTARGETSTACK))
gb_Module_TARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_TARGETSTACK)),$$(gb_Module_TARGETSTACK))
gb_Module_CLEANTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CLEANTARGETSTACK)),$$(gb_Module_CLEANTARGETSTACK))

endef

define gb_Module_add_moduledir
$(call gb_Module_add_target,$(1),$(2)/Module_$(2))
endef

define gb_Module_add_targets
$(foreach target,$(2),$(call gb_Module_add_target,$(1),$(target)))
endef

define gb_Module_add_moduledirs
$(foreach target,$(2),$(call gb_Module_add_moduledir,$(1),$(target)))
endef

define gb_Module_make_global_targets
ifneq ($$(gb_Module_TARGETSTACK),)
$$(warn corrupted module target stack!)
endif

include $(1)

all : $$(firstword $$(gb_Module_TARGETSTACK))
clean : $$(firstword $$(gb_Module_CLEANTARGETSTACK))
install : all
uninstall : clean

ifneq ($$(words $$(gb_Module_TARGETSTACK)),1)
$$(warn corrupted module target stack!)
endif

gb_Module_TARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_TARGETSTACK)),$$(gb_Module_TARGETSTACK))
gb_Module_CLEANTARGETSTACK := $$(wordlist 2,$$(words $$(gb_Module_CLEANTARGETSTACK)),$$(gb_Module_CLEANTARGETSTACK))

ifneq ($$(gb_Module_TARGETSTACK),)
$$(warn corrupted module target stack!)
endif

endef

# vim: set noet sw=4 ts=4:
