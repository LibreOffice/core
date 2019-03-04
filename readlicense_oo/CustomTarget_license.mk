# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,readlicense_oo/license))

readlicense_oo_DIR := $(call gb_CustomTarget_get_workdir,readlicense_oo/license)
readlicense_oo_LICENSE_xml := $(SRCDIR)/readlicense_oo/license/license.xml

$(call gb_CustomTarget_get_target,readlicense_oo/license) : $(readlicense_oo_DIR)/license.html \
	$(readlicense_oo_DIR)/license.fodt

ifeq ($(OS),WNT)
$(call gb_CustomTarget_get_target,readlicense_oo/license) : $(readlicense_oo_DIR)/license.txt
else
$(call gb_CustomTarget_get_target,readlicense_oo/license) : $(readlicense_oo_DIR)/LICENSE
endif

$(readlicense_oo_DIR)/license.html : \
		$(SRCDIR)/readlicense_oo/license/license_html.xsl \
		$(readlicense_oo_LICENSE_xml) \
		| $(readlicense_oo_DIR)/.dir \
		  $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XSL,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_ExternalExecutable_get_command,xsltproc) --nonet --novalid -o $@ \
			--stringparam build_type "$(BUILD_TYPE)" \
			--stringparam os "$(OS)" \
			--stringparam themes "$(WITH_THEMES)" \
			$(if $(MPL_SUBSET),,--stringparam no_mpl_subset no_mpl_subset) \
			$< \
			$(readlicense_oo_LICENSE_xml) \
		$(if $(filter WNT,$(OS)), \
			&& $(gb_AWK) 'sub("$$","\r")' $@ > $@.tmp \
			&& mv $@.tmp $@ \
		) \
	)

$(readlicense_oo_DIR)/LICENSE : \
		$(SRCDIR)/readlicense_oo/license/license_plain_text.xsl \
		$(readlicense_oo_LICENSE_xml) \
		| $(readlicense_oo_DIR)/.dir \
		  $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XSL,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_ExternalExecutable_get_command,xsltproc) --nonet --novalid -o $@ \
			--stringparam build_type "$(BUILD_TYPE)" \
			--stringparam os "$(OS)" \
			--stringparam themes "$(WITH_THEMES)" \
			$(if $(MPL_SUBSET),,--stringparam no_mpl_subset no_mpl_subset) \
			$< \
			$(readlicense_oo_LICENSE_xml) \
	)

ifeq ($(OS),WNT)
$(readlicense_oo_DIR)/license.txt : \
		$(SRCDIR)/readlicense_oo/license/LICENSE \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),AWK,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(gb_AWK) 'sub("$$","\r")' $< > $@.tmp && mv $@.tmp $@ \
	)
endif

$(readlicense_oo_DIR)/license.fodt : \
		$(readlicense_oo_DIR)/license.html \
		$(call gb_Postprocess_get_target,AllLibraries) \
		$(call gb_Postprocess_get_target,AllExecutables) \
		$(call gb_Postprocess_get_target,AllModuleTests) \
		$(call gb_Postprocess_get_target,AllModuleSlowtests) \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SOF,1)
	$(INSTDIR)/$(if $(filter MACOSX,$(OS)),$(PRODUCTNAME_WITHOUT_SPACES).app/Contents/)program/soffice$(if $(filter WNT,$(OS)),.exe) \
		--convert-to fodt:"OpenDocument Text Flat XML" --outdir $(readlicense_oo_DIR) $<

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
