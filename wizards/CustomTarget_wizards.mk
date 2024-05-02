# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,wizards/locproperties))

wizards_DIR := $(gb_CustomTarget_workdir)/wizards/locproperties

define wizards_Properties_Properties
$(call wizards_Properties__Properties_impl,$(wizards_DIR)/resources_$(1).properties,$(SRCDIR)/wizards/source/resources/resources_en_US.properties,$(gb_POLOCATION)/$(2)/wizards/source/resources.po,$(2))

endef

define wizards_Properties__Properties_impl
$(1) : LANGUAGE := $(4)
$(1) : POFILE := $(3)
$(1) : SOURCE := $(2)

$(call gb_CustomTarget_get_target,wizards/locproperties) : $(1)
$(1) : $(2) \
	$(wildcard $(3))

endef

ifneq ($(strip $(gb_WITH_LANG)),)
wizards_LANGS := $(filter-out en-US,$(gb_WITH_LANG))
$(eval $(foreach lang,$(wizards_LANGS),$(call wizards_Properties_Properties,$(subst -,_,$(lang)),$(lang))))
endif

$(wizards_DIR)/resources_%.properties : \
		$(call gb_Executable_get_runtime_dependencies,propex) \
		| $(wizards_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRP,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRP)
	$(call gb_Helper_abbreviate_dirs, \
		$(if $(filter-out qtz,$(LANGUAGE)), \
			MERGEINPUT=$(call gb_var2file,$(shell $(gb_MKTEMP)),$(POFILE)) && \
			$(call gb_Executable_get_command,propex) \
				-i $(SOURCE) \
				-o $@ \
				-m $${MERGEINPUT} \
				-l $(LANGUAGE) && \
			rm -rf $${MERGEINPUT} \
			, \
			$(call gb_Executable_get_command,propex) \
				-i $(SOURCE) \
				-o $@ \
				-m \
				-l $(LANGUAGE) \
		) \
	)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRP)


# vim:set shiftwidth=4 tabstop=4 noexpandtab:
