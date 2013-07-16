# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

instsetoo_ULFLIST := ActionTe \
	Control \
	CustomAc \
	Error \
	LaunchCo \
	Property \
	RadioBut \
	UIText

$(eval $(call gb_CustomTarget_CustomTarget,instsetoo_native/install))

$(eval $(call gb_CustomTarget_register_targets,instsetoo_native/install,\
	install.phony \
	$(if $(filter-out WNT,$(OS)),\
	bin/find-requires-gnome.sh \
	bin/find-requires-x11.sh) \
	$(foreach ulf,$(instsetoo_ULFLIST),win_ulffiles/$(ulf).ulf) \
))

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/install.phony: \
	$(SOLARENV)/bin/make_installer.pl \
	$(foreach ulf,$(instsetoo_ULFLIST),$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/win_ulffiles/$(ulf).ulf) \
	$(call gb_Postprocess_get_target,AllModulesButInstsetNative)

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/bin/find-requires-%.sh: $(SRCDIR)/instsetoo_native/inc_openoffice/unix/find-requires-%.sh
	cat $< | tr -d "\015" > $@
	chmod a+x $@

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/win_ulffiles/%.ulf: \
		| $(call gb_Postprocess_get_target,AllModulesButInstsetNative)

$(eval $(call gb_CustomTarget_ulfex_rule,\
	$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/win_ulffiles/%.ulf,\
	$(SRCDIR)/instsetoo_native/inc_openoffice/windows/msi_languages/%.ulf,\
	$(foreach lang,$(gb_TRANS_LANGS),\
		$(gb_POLOCATION)/$(lang)/instsetoo_native/inc_openoffice/windows/msi_languages.po)))

export ENABLE_DOWNLOADSETS ?= TRUE
ifeq ($(OS),LINUX)
export DONT_REMOVE_PACKAGE := TRUE
endif
export instsetoo_OUT := $(WORKDIR)/installation
export LOCAL_OUT := $(instsetoo_OUT)
export LOCAL_COMMON_OUT := $(instsetoo_OUT)

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
	-f $(BUILDDIR)/instsetoo_native/util/openoffice.lst \
	-l $(subst $(WHITESPACE),$(COMMA),$(strip $(2))) \
	-p LibreOffice$(if $(filter TRUE,$(ENABLE_RELEASE_BUILD)),,_Dev)$(3) \
	-u $(instsetoo_OUT) \
	-buildid $(LIBO_VERSION_PATCH) \
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
	rm -rf $(instsetoo_OUT)
ifeq (TRUE,$(LIBO_DEV_INSTALL))
	$(call instsetoo_native_install_command,openoffice,en-US,,,archive)
	unzip -q -d $(DEVINSTALLDIR) $(instsetoo_OUT)/LibreOffice$(if $(filter TRUE,$(ENABLE_RELEASE_BUILD)),,_Dev)/archive/install/en-US/LibreOffice*_archive.zip
	mv $(DEVINSTALLDIR)/LibreOffice*_archive/* $(DEVINSTALLDIR)/opt
	rmdir $(DEVINSTALLDIR)/LibreOffice*_archive
ifeq (ODK,$(filter ODK,$(BUILD_TYPE)))
	$(call instsetoo_native_install_command,sdkoo,en-US,_SDK,,archive)
	unzip -q -d $(DEVINSTALLDIR) $(instsetoo_OUT)/LibreOffice$(if $(filter TRUE,$(ENABLE_RELEASE_BUILD)),,_Dev)_SDK/archive/install/en-US/LibreOffice*_archive_sdk.zip
	mv $(DEVINSTALLDIR)/LibreOffice*_archive_sdk/LibreOffice*_SDK/sdk \
        $(DEVINSTALLDIR)/opt/
	rmdir $(DEVINSTALLDIR)/LibreOffice*_archive_sdk/LibreOffice*_SDK
	rmdir $(DEVINSTALLDIR)/LibreOffice*_archive_sdk
endif
else # LIBO_DEV_INSTALL
	$(call instsetoo_native_install_command,openoffice,$(if $(filter WNT,$(OS)),$(instsetoo_native_WITH_LANG),en-US),,,$(PKGFORMAT))
ifeq (ODK,$(filter ODK,$(BUILD_TYPE)))
	$(call instsetoo_native_install_command,sdkoo,en-US,_SDK,,$(PKGFORMAT))
endif
ifeq (,$(filter WNT MACOSX,$(OS)))
	$(call instsetoo_native_install_command,not-used,en-US,_Test,,$(PKGFORMAT))
endif
ifeq (HELP,$(filter HELP,$(BUILD_TYPE))$(filter MACOSX,$(OS)))
	$(foreach lang,$(gb_HELP_LANGS),\
		$(call instsetoo_native_install_command,ooohelppack,$(lang),,-helppack,$(PKGFORMAT)))
endif
ifneq (WNT,$(OS))
	$(foreach lang,$(instsetoo_native_WITH_LANG),\
		$(call instsetoo_native_install_command,ooolangpack,$(lang),,-languagepack,$(PKGFORMAT)))
endif
endif # LIBO_DEV_INSTALL

# vim: set noet sw=4 ts=4:
