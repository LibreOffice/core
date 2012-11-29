# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,librelogo/locproperties))

librelogo_DIR := $(call gb_CustomTarget_get_workdir,librelogo/locproperties)

ifneq ($(strip $(gb_WITH_LANG)),)
librelogo_LANGS := $(subst -,_,$(filter-out qtz,$(filter-out en-US,$(gb_WITH_LANG))))
librelogo_LOCPROPERTIES := $(foreach lang,$(librelogo_LANGS),$(librelogo_DIR)/LibreLogo_$(lang).properties)
else
librelogo_LOCPROPERTIES :=
endif

librelogo_PROPMERGETARGET := $(call gb_Executable_get_target_for_build,propex)
librelogo_PROPMERGECOMMAND := $(gb_Helper_set_ld_path) $(librelogo_PROPMERGETARGET)

$(call gb_CustomTarget_get_target,librelogo/locproperties) : $(librelogo_LOCPROPERTIES)

$(librelogo_DIR)/LibreLogo_%.properties : \
		$(SRCDIR)/librelogo/source/pythonpath/LibreLogo_en_US.properties \
		$(librelogo_PROPMERGETARGET) \
		| $(librelogo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRP,1)
	$(call gb_Helper_abbreviate_dirs, \
		LANG=$(subst _,-,$(subst $(librelogo_DIR)/LibreLogo_,,$(subst .properties,,$@))) && \
		MERGEINPUT=`$(gb_MKTEMP)` && \
		echo $(gb_POLOCATION)/$${LANG}/librelogo/source/pythonpath.po > $${MERGEINPUT} && \
		$(librelogo_PROPMERGECOMMAND) \
			-p librelogo \
			-i $< \
			-o $@ \
			-m $${MERGEINPUT} \
			-l $${LANG} && \
		rm -rf $${MERGEINPUT} \
	)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
