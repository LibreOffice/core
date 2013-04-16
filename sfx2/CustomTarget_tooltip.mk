# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,tooltip/tooltip_properties))

tooltip_DIR := $(call gb_CustomTarget_get_workdir,tooltip/tooltip_properties)

define tooltip_Properties_Properties
$(call tooltip_Properties__Properties_impl,$(tooltip_DIR)/shared_$(1).properties,$(SRCDIR)/sfx2/uiconfig/shared_en_US.properties,$(gb_POLOCATION)/$(2)/sfx2/shared_tooltip.po,$(2))

endef

define tooltip_Properties__Properties_impl
$(1) : LANG := $(4)
$(1) : POFILE := $(3)
$(1) : SOURCE := $(2)

$(call gb_CustomTarget_get_target,tooltip/tooltip_properties) : $(1)
$(1) : $(2) \
	$(wildcard $(3))

endef

ifneq ($(strip $(gb_WITH_LANG)),)
tooltip_LANGS := $(filter-out qtz,$(filter-out en-US,$(gb_WITH_LANG)))
$(eval $(foreach lang,$(tooltip_LANGS),$(call tooltip_Properties_Properties,$(subst -,_,$(lang)),$(lang))))
endif

$(tooltip_DIR)/shared_%.properties : \
		$(call gb_Executable_get_runtime_dependencies,propex) \
		| $(tooltip_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRP,1)
	$(call gb_Helper_abbreviate_dirs, \
		MERGEINPUT=`$(gb_MKTEMP)` && \
		echo $(POFILE) > $${MERGEINPUT} && \
		$(call gb_Executable_get_command,propex) \
			-i $(SOURCE) \
			-o $@ \
			-m $${MERGEINPUT} \
			-l $(LANG) && \
		rm -rf $${MERGEINPUT} \
	)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
