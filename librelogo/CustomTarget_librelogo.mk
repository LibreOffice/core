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

define librelogo_Properties_Properties
$(call librelogo_Properties__Properties_impl,$(librelogo_DIR)/LibreLogo_$(1).properties,$(SRCDIR)/librelogo/source/pythonpath/LibreLogo_en_US.properties,$(gb_POLOCATION)/$(2)/librelogo/source/pythonpath.po,$(2))

endef

define librelogo_Properties__Properties_impl
$(1) : LANG := $(4)
$(1) : POFILE := $(3)
$(1) : SOURCE := $(2)

$(call gb_CustomTarget_get_target,librelogo/locproperties) : $(1)
$(1) : $(2) \
	$(wildcard $(3))

endef

ifneq ($(strip $(gb_WITH_LANG)),)
librelogo_LANGS := $(filter-out en-US,$(gb_WITH_LANG))
$(eval $(foreach lang,$(librelogo_LANGS),$(call librelogo_Properties_Properties,$(subst -,_,$(lang)),$(lang))))
endif

$(librelogo_DIR)/LibreLogo_%.properties : \
		$(call gb_Executable_get_runtime_dependencies,propex) \
		| $(librelogo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRP,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(if $(filter-out qtz,$(LANG)), \
			MERGEINPUT=$(call var2file,$(shell $(gb_MKTEMP)),100,$(POFILE)) && \
			$(call gb_Executable_get_command,propex) \
				-i $(SOURCE) \
				-o $@ \
				-m $${MERGEINPUT} \
				-l $(LANG) && \
			rm -rf $${MERGEINPUT} \
			, \
			$(call gb_Executable_get_command,propex) \
				-i $(SOURCE) \
				-o $@ \
				-m \
				-l $(LANG) \
		) \
	)


# vim:set shiftwidth=4 tabstop=4 noexpandtab:
