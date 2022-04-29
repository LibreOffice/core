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

define instsetoo_native_install_command
$(if $(GNUPARALLEL), \
    $(call gb_Helper_print_on_error, \
    cd $(dir $@) && \
    $(GNUPARALLEL) -j $(PARALLELISM) $(SRCDIR)/solenv/bin/call_installer.sh $(if $(verbose),-verbose,-quiet) -- $(1) \
    ,$@.log) \
, \
    $(call gb_Helper_print_on_error, \
    cd $(dir $@) \
    $(foreach curpkg,$(1),\
    && $(SRCDIR)/solenv/bin/call_installer.sh $(if $(verbose),-verbose,-quiet) $(curpkg) \
    ),$@.log))
endef

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/install.phony:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	rm -rf $(instsetoo_OUT)
ifeq (TRUE,$(LIBO_TEST_INSTALL))
	$(call instsetoo_native_install_command, "openoffice:en-US:::archive:nostrip")
	unzip -q -d $(TESTINSTALLDIR) $(instsetoo_OUT)/$(PRODUCTNAME_WITHOUT_SPACES)/archive/install/en-US/LibreOffice*_archive.zip
	mv $(TESTINSTALLDIR)/LibreOffice*_archive/LibreOffice*/* $(TESTINSTALLDIR)/
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive/LibreOffice*
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive
ifeq (ODK,$(filter ODK,$(BUILD_TYPE)))
	$(call instsetoo_native_install_command, "sdkoo:en-US:_SDK::archive:nostrip")
	unzip -q -d $(TESTINSTALLDIR) $(instsetoo_OUT)/$(PRODUCTNAME_WITHOUT_SPACES)_SDK/archive/install/en-US/LibreOffice*_archive_sdk.zip
	mv $(TESTINSTALLDIR)/LibreOffice*_archive_sdk/LibreOffice*_SDK/sdk \
        $(TESTINSTALLDIR)/
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive_sdk/LibreOffice*_SDK
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive_sdk
endif
else # LIBO_TEST_INSTALL
	$(call instsetoo_native_install_command, \
		$(foreach pkgformat,$(PKGFORMAT),\
			$(if $(filter WNT,$(OS)), \
				"openoffice:$(subst $(WHITESPACE),$(COMMA),$(strip $(instsetoo_native_WITH_LANG))):::$(pkgformat):$(if $(filter-out archive,$(pkgformat)),strip,nostrip)" \
				$(if $(filter ODK,$(BUILD_TYPE)), \
					"sdkoo:en-US:_SDK::$(pkgformat):nostrip") \
				$(if $(filter HELP,$(BUILD_TYPE)), \
					$(foreach lang,$(gb_HELP_LANGS), \
						"ooohelppack:$(lang)::-helppack:$(pkgformat):nostrip" )) \
			, \
				":en-US:::$(pkgformat):$(if $(filter-out archive,$(pkgformat)),strip,nostrip)" \
				$(if $(filter ODK,$(BUILD_TYPE)), \
					":en-US:_SDK::$(pkgformat):nostrip") \
				$(if $(and $(filter HELP,$(BUILD_TYPE)), $(filter-out MACOSX,$(OS))), \
					$(foreach lang,$(gb_HELP_LANGS), \
						":$(lang)::-helppack:$(pkgformat):nostrip" )) \
				$(foreach lang,$(instsetoo_native_WITH_LANG), \
					":$(lang)::-languagepack:$(pkgformat):nostrip" ) )))
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
