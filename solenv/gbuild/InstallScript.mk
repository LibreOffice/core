# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# InstallScript class

gb_InstallScript_TARGET := $(SRCDIR)/solenv/bin/par2script.pl
gb_InstallScript_COMMAND := $(PERL) $(gb_InstallScript_TARGET)

gb_InstallScript__make_arglist = $(subst $(WHITESPACE),$(COMMA),$(strip $(1)))

define gb_InstallScript__get_files
$(notdir $(shell cat $(foreach module,$(1),$(call gb_InstallModule_get_filelist,$(module)))))
endef

define gb_InstallScript__get_dirs
$(sort $(dir $(shell cat $(foreach module,$(1),$(call gb_InstallModule_get_filelist,$(module))))))
endef

# Pass first arg if make is running in silent mode, second arg otherwise
define gb_InstallScript__if_silent
$(if $(findstring s,$(filter-out --%,$(MAKEFLAGS))),$(1),$(2))
endef

define gb_InstallScript__command
$(call gb_Output_announce,$(2),$(true),INS,4)
$(call gb_Helper_abbreviate_dirs,\
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,\
		$(call gb_InstallScript__make_arglist,\
			$(call gb_InstallScript__get_files,$(SCP_MODULES))) \
	) && \
	$(gb_InstallScript_COMMAND) \
		$(call gb_InstallScript__if_silent,-q) \
		-i $(call gb_InstallScript__make_arglist,$(call gb_InstallScript__get_dirs,$(SCP_MODULES))) \
	   	-o $(1) \
	   	@@$${RESPONSEFILE} && \
	rm -f $${RESPONSEFILE} \
)
endef

$(dir $(call gb_InstallScript_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_InstallScript_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_InstallScript_get_target,%) : $(gb_InstallScript_TARGET)
	$(call gb_InstallScript__command,$@,$*)

.PHONY : $(call gb_InstallScript_get_clean_target,%)
$(call gb_InstallScript_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),INS,4)
	rm -f $(call gb_InstallScript_get_target,$*)

# gb_InstallScript_InstallScript(<target>)
define gb_InstallScript_InstallScript
$(call gb_InstallScript_get_target,$(1)) :| $(dir $(call gb_InstallScript_get_target,$(1))).dir
$(call gb_InstallScript_get_target,$(1)) : SCP_MODULES :=

$$(eval $$(call gb_Module_register_target,InstallScript_$(1),$(call gb_InstallScript_get_target,$(1)),$(call gb_InstallScript_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),InstallScript)

endef

define gb_InstallScript_use_module
$(call gb_InstallScript_get_target,$(1)) : $(call gb_InstallModule_get_target,$(2))
$(call gb_InstallScript_get_clean_target,$(1)) : $(call gb_InstallModule_get_clean_target,$(2))
$(call gb_InstallScript_get_target,$(1)) : SCP_MODULES += $(2)

endef

define gb_InstallScript_use_modules
$(foreach module,$(2),$(call gb_InstallScript_use_module,$(1),$(module)))

endef

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
