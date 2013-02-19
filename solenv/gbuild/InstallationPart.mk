
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

define gb_InstallationPart__rule
$(1)/.dir :
	$$(if $$(wildcard $$(dir $$@)),,mkdir -p $$(dir $$@))
$(1)/%/.dir :
	$$(if $$(wildcard $$(dir $$@)),,mkdir -p $$(dir $$@))
$(1)/% :
	install -t $$(dir $$@) $$<
endef

$(foreach destination,$(call gb_InstallationPart_get_destinations), \
	$(eval $(call gb_InstallationPart__rule,$(destination))))

.PHONY : $(call gb_InstallationPart_get_clean_target,%)
$(call gb_InstallationPart_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),INS,2)
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(FILES)) \
	&& cat $${RESPONSEFILE} | xargs $(if $(filter MACOSX,$(OS_FOR_BUILD)),-n 1000) rm -f \
	&& rm -f $${RESPONSEFILE}

$(call gb_InstallationPart_get_target,%) :
	$(call gb_Output_announce,$*,$(true),INS,2)
	mkdir -p $(dir $@) && touch $@

define gb_InstallationPart_InstallationPart
$$(eval $$(call gb_Module_register_target,$(call gb_InstallationPart_get_target,$(1)),$(call gb_InstallationPart_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),InstallationPart)

endef

define gb_InstallationPart__add_library
$(call gb_InstallationPart_get_target,$(1)) : $(call gb_Library_get_installlocation,$(2))/$(call gb_Library_get_filename,$(2))
$(call gb_Library_get_installlocation,$(2))/$(call gb_Library_get_filename,$(2)) : $(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,$(2)))
$(call gb_Library_get_installlocation,$(2))/$(call gb_Library_get_filename,$(2)) : $(call gb_Library_get_installlocation,$(2))/.dir
$(call gb_InstallationPart_get_clean_target,$(1)) : FILES += $(call gb_Library_get_installlocation,$(2))

endef

define gb_InstallationPart_add_library_installmodule
$(foreach lib,$(gb_Library_MODULE_$(2)),$(call gb_InstallationPart__add_library,$(1),$(lib)))

endef

# vim: set noet sw=4 ts=4:
