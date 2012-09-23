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

ifeq ($(GUI),UNX)
readlicense_oo_READMEs := $(foreach lang,$(readlicense_oo_LANGS),$(readlicense_oo_DIR)/README_$(lang))
else
readlicense_oo_READMEs := $(foreach lang,$(readlicense_oo_LANGS),$(readlicense_oo_DIR)/readme_$(lang).txt)
endif

readlicense_XRMEXTARGET := $(call gb_Executable_get_target_for_build,xrmex)
readlicense_XRMEXCOMMAND := $(gb_Helper_set_ld_path) $(readlicense_XRMEXTARGET)

readlicense_PRJNAME := readlicense_oo

$(call gb_CustomTarget_get_target,readlicense_oo/readme) : \
	$(readlicense_oo_DIR)/readme.dtd \
	$(readlicense_oo_DIR)/readme_text.xsl \
	$(readlicense_oo_DIR)/localize.sdf \
	$(readlicense_oo_DIR)/readme.xrm \
	$(readlicense_oo_READMEs)

$(readlicense_oo_DIR)/readme.dtd : $(SRCDIR)/readlicense_oo/docs/readme.dtd \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),COP,1)
	$(call gb_Helper_abbreviate_dirs, \
	$(GNUCOPY) $< $@)

$(readlicense_oo_DIR)/readme_text.xsl : $(SRCDIR)/readlicense_oo/docs/readme.xsl \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),COP,1)
	$(call gb_Helper_abbreviate_dirs, \
	$(GNUCOPY) $< $@)

$(readlicense_oo_DIR)/localize.sdf : $(SRCDIR)/readlicense_oo/docs/readme.xsl \
		$(SRCDIR)/readlicense_oo/docs/readme.dtd \
		$(SRCDIR)/readlicense_oo/docs/readme/readme.xrm \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),TOU,1)
	$(call gb_Helper_abbreviate_dirs, \
	touch $@)

ifeq ($(strip $(gb_WITH_LANG)),)
$(readlicense_oo_DIR)/readme.xrm : $(SRCDIR)/readlicense_oo/docs/readme/readme.xrm \
		$(readlicense_oo_DIR)/localize.sdf \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),COP,1)
	$(call gb_Helper_abbreviate_dirs, \
	$(GNUCOPY) $< $@)
else
$(readlicense_oo_DIR)/readme.xrm : $(SRCDIR)/readlicense_oo/docs/readme/readme.xrm \
		$(readlicense_XRMEXTARGET) \
		$(readlicense_oo_DIR)/localize.sdf \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XRM,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(readlicense_XRMEXCOMMAND) \
			-p $(readlicense_PRJNAME) \
			-i $< \
			-o $@ \
			-m $(readlicense_oo_DIR)/localize.sdf \
			-l all)
ifeq ($(GUI),WNT)
		$(PERL) -pi -e 's/\n/\r\n/' $@
endif
endif

$(readlicense_oo_READMEs) : $(readlicense_oo_DIR)/readme_text.xsl \
		$(readlicense_oo_DIR)/readme.xrm \
		| $(readlicense_oo_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XSL,1)
	$(call gb_Helper_abbreviate_dirs, \
	$(gb_XSLTPROC) --nonet --novalid -o $@ \
	--stringparam os1 $(OS)	--stringparam gui1 $(GUI) --stringparam com1 $(COM) \
	--stringparam cp1 $(CPUNAME) --stringparam type text \
	--stringparam lang1 $(word 2,$(subst _, ,$(basename $(notdir $@)))) \
	$< $(readlicense_oo_DIR)/readme.xrm)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
