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
	$(if $(filter TRUE,$(WINDOWS_BUILD_SIGNING)),msi_signing.done) \
	$(if $(filter-out WNT,$(OS)),\
	bin/find-requires-gnome.sh \
	bin/find-requires-x11.sh) \
	$(foreach ulf,$(instsetoo_ULFLIST),win_ulffiles/$(ulf).ulf) \
))

.PHONY: $(call gb_CustomTarget_get_workdir,instsetoo_native/install)/install.phony

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/install.phony: \
	$(SRCDIR)/solenv/bin/make_installer.pl \
	$(foreach ulf,$(instsetoo_ULFLIST),$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/win_ulffiles/$(ulf).ulf) \
	$(if $(filter-out WNT,$(OS)),\
		$(addprefix $(call gb_CustomTarget_get_workdir,instsetoo_native/install)/,\
			bin/find-requires-gnome.sh \
			bin/find-requires-x11.sh) \
	) \
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
&& for I in $(SRCDIR)/instsetoo_native/inc_$(1)/windows/msi_templates/*.* ; do $(GREP) -v '^#' "$$I" > $${TEMPLATE_DIR}/`basename $$I` || true ; done \
&& $(GNUCOPY) $(SRCDIR)/instsetoo_native/inc_common/windows/msi_templates/Binary/*.* $${TEMPLATE_DIR}/Binary
endef
else
instsetoo_native_msitemplates :=
endif

define instsetoo_native_install_command
$(call instsetoo_native_msitemplates,$(1))
$(call gb_Helper_print_on_error, \
cd $(dir $@) \
$(foreach pkgformat,$(5),\
&& $(if $(filter-out archive,$(pkgformat)),ENABLE_STRIP=1) $(PERL) -w $< \
	-f $(BUILDDIR)/instsetoo_native/util/openoffice.lst \
	-l $(subst $(WHITESPACE),$(COMMA),$(strip $(2))) \
	-p $(PRODUCTNAME_WITHOUT_SPACES)$(3) \
	-u $(instsetoo_OUT) \
	-buildid $(if $(filter deb0 rpm0,$(pkgformat)$(LIBO_VERSION_PATCH)),1,$(LIBO_VERSION_PATCH)) \
	$(if $(filter WNT,$(OS)), \
		-msitemplate $(dir $@)msi_templates \
		-msilanguage $(dir $@)win_ulffiles \
	) \
	$(4) \
	-format $(pkgformat) \
	$(if $(verbose),-verbose,-quiet) \
),$@.log)
endef

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/install.phony:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	rm -rf $(instsetoo_OUT)
ifeq (TRUE,$(LIBO_TEST_INSTALL))
	$(call instsetoo_native_install_command,openoffice,en-US,,,archive)
	unzip -q -d $(TESTINSTALLDIR) $(instsetoo_OUT)/$(PRODUCTNAME_WITHOUT_SPACES)/archive/install/en-US/LibreOffice*_archive.zip
	mv $(TESTINSTALLDIR)/LibreOffice*_archive/LibreOffice*/* $(TESTINSTALLDIR)/
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive/LibreOffice*
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive
ifeq (ODK,$(filter ODK,$(BUILD_TYPE)))
	$(call instsetoo_native_install_command,sdkoo,en-US,_SDK,,archive)
	unzip -q -d $(TESTINSTALLDIR) $(instsetoo_OUT)/$(PRODUCTNAME_WITHOUT_SPACES)_SDK/archive/install/en-US/LibreOffice*_archive_sdk.zip
	mv $(TESTINSTALLDIR)/LibreOffice*_archive_sdk/LibreOffice*_SDK/sdk \
        $(TESTINSTALLDIR)/
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive_sdk/LibreOffice*_SDK
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive_sdk
endif
else # LIBO_TEST_INSTALL
	$(call instsetoo_native_install_command,openoffice,$(if $(filter WNT,$(OS)),$(instsetoo_native_WITH_LANG),en-US),,,$(PKGFORMAT))
ifeq (ODK,$(filter ODK,$(BUILD_TYPE)))
	$(call instsetoo_native_install_command,sdkoo,en-US,_SDK,,$(PKGFORMAT))
endif
ifeq (HELP,$(filter HELP,$(BUILD_TYPE))$(filter MACOSX,$(OS)))
	$(foreach lang,$(gb_HELP_LANGS),\
		$(call instsetoo_native_install_command,ooohelppack,$(lang),,-helppack,$(PKGFORMAT)))
endif
ifneq (WNT,$(OS))
	$(foreach lang,$(instsetoo_native_WITH_LANG),\
		$(call instsetoo_native_install_command,ooolangpack,$(lang),,-languagepack,$(PKGFORMAT)))
endif
endif # LIBO_TEST_INSTALL
	touch $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

TIMESTAMPURL ?= "http://timestamp.globalsign.com/scripts/timestamp.dll"
$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_signing.done: \
        $(if $(filter HELP,$(BUILD_TYPE)),$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_helppack_signing.done) \
        $(if $(filter ODK,$(BUILD_TYPE)),$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_sdk_signing.done) \
        $(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_main_signing.done
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	touch $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_helppack_signing.done \
$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_sdk_signing.done \
$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_main_signing.done \
    : $(SRCDIR)/postprocess/signing/signing.pl $(call gb_CustomTarget_get_workdir,instsetoo_native/install)/install.phony

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_main_signing.done:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(PERL) $(SRCDIR)/postprocess/signing/signing.pl \
			-l $(subst .done,_log.txt,$@) \
			$(if $(verbose),-v) \
			$(if $(PFXFILE),-f $(PFXFILE)) \
			$(if $(PFXPASSWORD),-p $(PFXPASSWORD)) \
			$(if $(TIMESTAMPURL),-t $(TIMESTAMPURL)) \
			-d $(PRODUCTNAME_WITHOUT_SPACES)\ $(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH) \
			$(WORKDIR)/installation/$(PRODUCTNAME_WITHOUT_SPACES)/msi/install/*/*.msi \
	&& touch $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_helppack_signing.done:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(PERL) $(SRCDIR)/postprocess/signing/signing.pl \
			-l $(subst .done,_log.txt,$@) \
			$(if $(verbose),-v) \
			$(if $(PFXFILE),-f $(PFXFILE)) \
			$(if $(PFXPASSWORD),-p $(PFXPASSWORD)) \
			$(if $(TIMESTAMPURL),-t $(TIMESTAMPURL)) \
			-d $(PRODUCTNAME_WITHOUT_SPACES)\ $(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)\ Helppack \
			$(WORKDIR)/installation/$(PRODUCTNAME_WITHOUT_SPACES)_helppack/msi/install/*/*.msi \
	&& touch $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_sdk_signing.done:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(PERL) $(SRCDIR)/postprocess/signing/signing.pl \
			-l $(subst .done,_log.txt,$@) \
			$(if $(verbose),-v) \
			$(if $(PFXFILE),-f $(PFXFILE)) \
			$(if $(PFXPASSWORD),-p $(PFXPASSWORD)) \
			$(if $(TIMESTAMPURL),-t $(TIMESTAMPURL)) \
			-d $(PRODUCTNAME_WITHOUT_SPACES)\ $(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)\ SDK \
			$(WORKDIR)/installation/$(PRODUCTNAME_WITHOUT_SPACES)_SDK/msi/install/*/*.msi \
	&& touch $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

# vim: set noet sw=4 ts=4:
