# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,readlicense_oo/readme))

readlicense_oo_DIR := $(call gb_CustomTarget_get_workdir,readlicense_oo/readme)

# gb_WITH_LANG is empty if --with-lang is not set
# what we need here is: gb_WITH_LANG_OR_DEFAULT ;-)
readlicense_oo_LANGS := en-US $(filter-out en-US,$(gb_WITH_LANG))

$(call gb_CustomTarget_get_target,readlicense_oo/readme) : \
	$(foreach lang,$(readlicense_oo_LANGS),$(readlicense_oo_DIR)/$(call gb_README,$(lang)))

ifeq ($(strip $(gb_WITH_LANG)),)
readlicense_oo_README_XRM := $(SRCDIR)/readlicense_oo/docs/readme.xrm
else
readlicense_oo_README_XRM := $(readlicense_oo_DIR)/readme.xrm

$(readlicense_oo_DIR)/readme.xrm : \
		$(SRCDIR)/readlicense_oo/docs/readme.xrm \
		$(call gb_Executable_get_runtime_dependencies,xrmex) \
		$(foreach lang,$(filter-out qtz,$(filter-out en-US,$(gb_WITH_LANG))),$(gb_POLOCATION)/$(lang)/readlicense_oo/docs.po) \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XRM,1)
	$(call gb_Helper_abbreviate_dirs, \
        MERGEINPUT=`$(gb_MKTEMP)` && \
        echo $(foreach lang,$(filter-out qtz,$(filter-out en-US,$(gb_WITH_LANG))),$(gb_POLOCATION)/$(lang)/readlicense_oo/docs.po) > $${MERGEINPUT} && \
		$(call gb_Executable_get_command,xrmex) \
			-i $< \
			-o $@ \
			-m $${MERGEINPUT} \
			-l all && \
        rm -rf $${MERGEINPUT})

endif

readlicense_oo_README_SED := \
	-e 's,$${PRODUCTNAME},$(PRODUCTNAME),g' \
	-e 's,$${PRODUCTVERSION},$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR),g' \

$(readlicense_oo_DIR)/$(call gb_README,%) : \
		$(SRCDIR)/readlicense_oo/docs/readme.xsl \
		$(readlicense_oo_README_XRM) \
		| $(readlicense_oo_DIR)/.dir \
		  $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XSL,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_ExternalExecutable_get_command,xsltproc) --nonet --novalid -o $@.out \
			--stringparam com1 $(COM) \
			--stringparam cp1 $(CPUNAME) \
			--stringparam lang1 $(word 2,$(subst _, ,$(basename $(notdir $@)))) \
			--stringparam os1 $(OS) \
			--stringparam type text \
			$< \
			$(readlicense_oo_README_XRM) \
		$(if $(filter WNT,$(OS)), \
			&& $(gb_AWK) 'sub("$$","\r")' $@.out > $@.tmp \
			&& mv $@.tmp $@.out \
		) \
		&& sed $(readlicense_oo_README_SED) $@.out > $@ \
		&& rm $@.out \
	)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
