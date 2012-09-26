# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

# InstallScriptTarget class

gb_InstallScriptTarget_TARGET := $(SOLARENV)/bin/par2script.pl
gb_InstallScriptTarget_COMMAND := $(PERL) $(gb_InstallScriptTarget_TARGET)

gb_InstallScriptTarget__make_arglist = $(subst $(WHITESPACE),$(COMMA),$(strip $(1)))

define gb_InstallScriptTarget__get_files
$(notdir $(shell cat $(foreach module,$(1),$(call gb_InstallModule_get_filelist,$(module)))))
endef

define gb_InstallScriptTarget__get_dirs
$(sort $(dir $(shell cat $(foreach module,$(1),$(call gb_InstallModule_get_filelist,$(module))))))
endef

# Pass first arg if make is running in silent mode, second arg otherwise
define gb_InstallScriptTarget__if_silent
$(if $(findstring s,$(filter-out --%,$(MAKEFLAGS))),$(1),$(2))
endef

define gb_InstallScriptTarget__command
$(call gb_Output_announce,$(2),$(true),INS,4)
$(call gb_Helper_abbreviate_dirs,\
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,\
		$(call gb_InstallScriptTarget__make_arglist,\
			$(call gb_InstallScriptTarget__get_files,$(SCP_MODULES))) \
	) && \
	$(gb_InstallScriptTarget_COMMAND) \
		$(call gb_InstallScriptTarget__if_silent,-q) \
		-i $(call gb_InstallScriptTarget__make_arglist,$(call gb_InstallScriptTarget__get_dirs,$(SCP_MODULES))) \
	   	-o $(1) \
	   	@@$${RESPONSEFILE} && \
	rm -f $${RESPONSEFILE} \
)
endef

$(dir $(call gb_InstallScriptTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_InstallScriptTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_InstallScriptTarget_get_target,%) : $(gb_InstallScriptTarget_TARGET)
	$(call gb_InstallScriptTarget__command,$@,$*)

.PHONY : $(call gb_InstallScriptTarget_get_clean_target,%)
$(call gb_InstallScriptTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),INS,4)
	rm -f $(call gb_InstallScriptTarget_get_target,$*)

# gb_InstallScriptTarget_InstallScriptTarget(<target>)
define gb_InstallScriptTarget_InstallScriptTarget
$(call gb_InstallScriptTarget_get_target,$(1)) :| $(dir $(call gb_InstallScriptTarget_get_target,$(1))).dir
$(call gb_InstallScriptTarget_get_target,$(1)) : SCP_MODULES :=

endef

define gb_InstallScriptTarget_use_module
$(call gb_InstallScriptTarget_get_target,$(1)) : $(call gb_InstallModule_get_target,$(2))
$(call gb_InstallScriptTarget_get_clean_target,$(1)) : $(call gb_InstallModule_get_clean_target,$(2))
$(call gb_InstallScriptTarget_get_target,$(1)) : SCP_MODULES += $(2)

endef

define gb_InstallScriptTarget_use_modules
$(foreach module,$(2),$(call gb_InstallScriptTarget_use_module,$(1),$(module)))

endef

# InstallScript class

# gb_InstallScript_InstallScript(<target>)
define gb_InstallScript_InstallScript
$(call gb_InstallScriptTarget_InstallScriptTarget,$(1))

$(call gb_InstallScript_get_target,$(1)) : $(call gb_InstallScriptTarget_get_target,$(1))
$(call gb_InstallScript_get_clean_target,$(1)) : $(call gb_InstallScriptTarget_get_clean_target,$(1))

$(call gb_Deliver_add_deliverable,$(call gb_InstallScript_get_target,$(1)),$(call gb_InstallScriptTarget_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_InstallScript_get_target,$(1)),$(call gb_InstallScript_get_clean_target,$(1))))

endef

define gb_InstallScript_use_module
$(call gb_InstallScriptTarget_use_module,$(1),$(2))

endef

define gb_InstallScript_use_modules
$(call gb_InstallScriptTarget_use_modules,$(1),$(2))

endef

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
