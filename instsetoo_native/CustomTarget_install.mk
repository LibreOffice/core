# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,instsetoo_native/install))

$(eval $(call gb_CustomTarget_register_targets,instsetoo_native/install,\
	install.phony \
	$(addprefix win_ulffiles/,$(subst .ulf,.mlf,$(notdir $(wildcard \
		$(SRCDIR)/instsetoo_native/inc_openoffice/windows/msi_languages/*.ulf)))) \
))

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/install.phony: \
	$(SOLARENV)/bin/make_installer.pl \
	$(addprefix $(call gb_CustomTarget_get_workdir,instsetoo_native/install)/win_ulffiles/, \
		$(subst .ulf,.mlf,$(notdir $(wildcard \
		$(SRCDIR)/instsetoo_native/inc_openoffice/windows/msi_languages/*.ulf))))

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/win_ulffiles/%.mlf: \
		$(SRCDIR)/instsetoo_native/inc_openoffice/windows/msi_languages/%.ulf
	$(gb_ScpConvertTarget_COMMAND) -t $(OUTDIR)/bin/msi-encodinglist.txt -o $@ $<

ifeq ($(OS),WNT)
export PYTHONPATH := $(OUTDIR_FOR_BUILD)/lib/python;$(OUTDIR_FOR_BUILD)/lib/python/lib-dynload
else
export PYTHONPATH := $(OUTDIR_FOR_BUILD)/lib/python:$(OUTDIR_FOR_BUILD)/lib/python/lib-dynload
endif
export ENABLE_DOWNLOADSETS ?= TRUE
ifeq ($(OS),LINUX)
export DONT_REMOVE_PACKAGE := TRUE
endif
export OUT := $(WORKDIR)/installation
export LOCAL_OUT := $(OUT)
export LOCAL_COMMON_OUT := $(OUT)

instsetoo_native_WITH_LANG := en-US $(filter-out en-US,$(gb_WITH_LANG))

ifeq (WNT,$(OS))
define instsetoo_native_msitemplates
TEMPLATE_DIR=$(dir $@)msi_templates \
&& rm -rf $${TEMPLATE_DIR} \
&& mkdir -p $${TEMPLATE_DIR}/Binary \
&& $(GNUCOPY) $(SRCDIR)/instsetoo_native/inc_$(1)/windows/msi_templates/*.* $${TEMPLATE_DIR} \
&& $(GNUCOPY) $(SRCDIR)/instsetoo_native/inc_common/windows/msi_templates/Binary/*.* $${TEMPLATE_DIR}/Binary
endef
else
instsetoo_native_msitemplates :=
endif

define instsetoo_native_install_command
$(call instsetoo_native_msitemplates,$(1))
cd $(dir $@) \
$(foreach pkgformat,$(5),\
&& $(PERL) -w $< \
	-f $(SRCDIR)/instsetoo_native/util/openoffice.lst \
	-l $(subst $(WHITESPACE),$(COMMA),$(2)) \
	-p LibreOffice$(if $(filter TRUE,$(ENABLE_RELEASE_BUILD)),,_Dev)$(3) \
	-u $(OUT) \
	-buildid $(BUILD) \
	$(if $(filter WNT,$(OS)), \
		-msitemplate $(dir $@)msi_templates \
		-msilanguage $(dir $@)win_ulffiles \
	) \
	$(4) \
	-format $(pkgformat) \
	$(if $(filter TRUE,$(VERBOSE)),-verbose, \
		$(if $(findstring s,$(MAKEFLAGS)),-quiet)) \
)
endef

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/install.phony:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	rm -rf $(OUT)
ifeq (TRUE,$(LIBO_DEV_INSTALL))
	$(call instsetoo_native_install_command,openoffice,en-US,,,archive)
	unzip -q -d $(DEVINSTALLDIR) $(OUT)/LibreOffice/archive/install/en-US/LibreOffice*_archive.zip
	mv $(DEVINSTALLDIR)/LibreOffice*_archive/* $(DEVINSTALLDIR)/opt
else # LIBO_DEV_INSTALL
	$(call instsetoo_native_install_command,openoffice,$(instsetoo_native_WITH_LANG),,,$(PKGFORMAT))
ifeq (ODK,$(filter ODK,$(BUILD_TYPE)))
	$(call instsetoo_native_install_command,sdkoo,en-US,_SDK,,$(PKGFORMAT))
endif
ifeq (,$(filter WNT MACOSX,$(OS)))
	$(call instsetoo_native_install_command,not-used,en-US,_Test,,$(PKGFORMAT))
endif
ifeq (HELP,$(filter HELP,$(BUILD_TYPE))$(filter MACOSX,$(OS)))
	$(foreach lang,$(filter $(instsetoo_native_WITH_LANG),$(shell \
	find $(SRCDIR)/helpcontent2/source/auxiliary/ -type d 2>/dev/null | sed -e "s|^.*/||")), \
		$(call instsetoo_native_install_command,ooohelppack,$(lang),,-helppack,$(PKGFORMAT)))
endif
ifneq (WNT,$(OS))
	$(foreach lang,$(instsetoo_native_WITH_LANG),\
		$(call instsetoo_native_install_command,ooolangpack,$(lang),,-languagepack,$(PKGFORMAT)))
endif
endif # LIBO_DEV_INSTALL

# vim: set noet sw=4 ts=4:
