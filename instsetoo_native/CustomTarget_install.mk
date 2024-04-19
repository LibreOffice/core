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

ifeq (WNT,$(OS))
instsetoo_installer_langs := $(subst $(WHITESPACE),$(COMMA),$(strip en-US $(filter-out en-US,$(gb_WITH_LANG))))
else
instsetoo_installer_langs := en-US
endif

# delimiter is U+2027 Hyphenation point - files with colon in their name confuse the heck out of
# make and cannot be used as targets or prerequisites. For passing to call_installer.sh it is
# substituted by the : so that cut doesn't stumble over the delimiter
ifeq (TRUE,$(filter TRUE,$(LIBO_TEST_INSTALL) $(ENABLE_WIX)))
instsetoo_installer_targets = openoffice‧en-US‧‧‧archive‧nostrip
ifeq (ODK,$(filter ODK,$(BUILD_TYPE)))
instsetoo_installer_targets += sdkoo‧en-US‧_SDK‧‧archive‧nostrip
endif
else
instsetoo_installer_targets := $(foreach pkgformat,$(PKGFORMAT),\
        openoffice‧$(instsetoo_installer_langs)‧‧‧$(pkgformat)‧$(if $(filter-out archive,$(pkgformat)),strip,nostrip) \
        $(if $(filter ODK,$(BUILD_TYPE)),sdkoo‧en-US‧_SDK‧‧$(pkgformat)‧nostrip) \
        $(if $(and $(filter HELP,$(BUILD_TYPE)),$(filter-out MACOSX,$(OS))), \
            $(foreach lang,$(gb_HELP_LANGS),ooohelppack‧$(lang)‧‧-helppack‧$(pkgformat)‧nostrip)) \
        $(if $(filter-out WNT,$(OS)), \
            $(foreach lang,$(filter-out en-US,$(gb_WITH_LANG)),ooolangpack‧$(lang)‧‧-languagepack‧$(pkgformat)‧nostrip)))
endif

LIBO_VERSION = $(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)

instsetoo_wipe:
	$(call gb_Output_announce,wiping installation output dir,$(true),WPE,6)
	rm -rf $(instsetoo_OUT)

# list both as prerequisites so that make won't treat the $(template) one as intermediate /
# won't attempt to delete it after the $(template)/Binary and the rest of the chain was made
instsetoo_msi_templates: $(foreach template,openoffice ooohelppack sdkoo,$(addprefix \
        $(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_templates/,$(template) $(template)/Binary))

# use awk instead of grep to not have to deal with grep exiting with error on files with no comments
$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_templates/%: $(SRCDIR)/instsetoo_native/inc_%/windows/msi_templates/*.*
	$(call gb_Output_announce,setting up msi templates for type $(@F),$(true),AWK,4)
	rm -rf $@ && mkdir -p $@ && cd $@ $(foreach file,$(^F),&& awk '!/^#/{print}' $(<D)/$(file) > $(file))

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_templates/%/Binary: $(SRCDIR)/instsetoo_native/inc_common/windows/msi_templates/Binary/*.* \
            $(call gb_CustomTarget_get_workdir,instsetoo_native/install)/msi_templates/%
	$(call gb_Output_announce,setting up msi templates for type $* - copying binary assets,$(true),CPY,4)
	rm -rf $@ && mkdir -p $@ && cd $@ && cp $(SRCDIR)/instsetoo_native/inc_common/windows/msi_templates/Binary/*.* ./

gb_Make_JobLimiter := $(WORKDIR)/job-limiter.exe

$(gb_Make_JobLimiter): $(SRCDIR)/solenv/bin/job-limiter.cpp
	cd $(WORKDIR) && \
	$(CXX_FOR_BUILD) $(SOLARINC) -EHsc $^ -link -LIBPATH:$(subst ;, -LIBPATH:,$(ILIB_FOR_BUILD)) || rm -f $@

# with all languages the logfile name would be too long when building the windows installation set,
# that's the reason for the substitution to multilang below in case more than just en-US is packaged
# also for windows msi packaging parallel execution is reduced by the job-limiter. This only has any
# effect when building with help and multiple languages, and it also won't affect the real time for
# packaging (since packaging the main installer takes longer than packaging sdk and all helppacks
# even with the reduced parallelism (the higher the parallelism, the higher the chance for random
# failures during the cscript call to WiLangId.vbs)
$(instsetoo_installer_targets): $(SRCDIR)/solenv/bin/make_installer.pl \
        $(foreach ulf,$(instsetoo_ULFLIST),$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/win_ulffiles/$(ulf).ulf) \
        $(if $(filter-out WNT,$(OS)),\
            $(addprefix $(call gb_CustomTarget_get_workdir,instsetoo_native/install)/,\
                bin/find-requires-gnome.sh \
                bin/find-requires-x11.sh) \
        ,instsetoo_msi_templates) \
        $(call gb_Postprocess_get_target,AllModulesButInstsetNative) \
        | instsetoo_wipe $(if $(filter msi,$(PKGFORMAT)),$(gb_Make_JobLimiter))
	$(call gb_Output_announce,$(if $(filter en-US$(COMMA)%,$(instsetoo_installer_langs)),$(subst $(instsetoo_installer_langs),multilang,$@),$@),$(true),INS,1)
	$(if $(filter %msi‧nostrip,$@),$(gb_Make_JobLimiter) grab)
	$(call gb_Trace_StartRange,$@,INSTALLER)
	$(call gb_Helper_print_on_error, \
	    $(if $(MSYSTEM),export PERLIO=:unix PERL=$(STRAWBERRY_PERL) &&) \
	    $(SRCDIR)/solenv/bin/call_installer.sh $(if $(verbose),-verbose,-quiet) $(subst ‧,:,$@),\
	    $(call gb_CustomTarget_get_workdir,instsetoo_native/install)/$(if $(filter en-US$(COMMA)%,$(instsetoo_installer_langs)),$(subst $(instsetoo_installer_langs),multilang,$@),$@).log)
	$(if $(filter %msi‧nostrip,$@),$(gb_Make_JobLimiter) release)
	$(call gb_Trace_EndRange,$@,INSTALLER)

$(call gb_CustomTarget_get_workdir,instsetoo_native/install)/install.phony: $(instsetoo_installer_targets)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(if $(ENABLE_WIX),$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/msicreator/create_installer.py $(BUILDDIR) $(SRCDIR) $(LIBO_VERSION) $(PRODUCTNAME_WITHOUT_SPACES))
ifeq (TRUE,$(LIBO_TEST_INSTALL))
	unzip -q -d $(TESTINSTALLDIR) $(instsetoo_OUT)/$(PRODUCTNAME_WITHOUT_SPACES)/archive/install/en-US/LibreOffice*_archive.zip
	mv $(TESTINSTALLDIR)/LibreOffice*_archive/LibreOffice*/* $(TESTINSTALLDIR)/
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive/LibreOffice*
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive
ifeq (ODK,$(filter ODK,$(BUILD_TYPE)))
	unzip -q -d $(TESTINSTALLDIR) $(instsetoo_OUT)/$(PRODUCTNAME_WITHOUT_SPACES)_SDK/archive/install/en-US/LibreOffice*_archive_sdk.zip
	mv $(TESTINSTALLDIR)/LibreOffice*_archive_sdk/LibreOffice*_SDK/sdk \
        $(TESTINSTALLDIR)/
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive_sdk/LibreOffice*_SDK
	rmdir $(TESTINSTALLDIR)/LibreOffice*_archive_sdk
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
