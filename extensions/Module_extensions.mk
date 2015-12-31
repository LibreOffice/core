# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,extensions))

$(eval $(call gb_Module_add_targets,extensions,\
	Library_res \
	$(if $(filter IOS ANDROID,$(OS)),, \
		Library_abp \
		Library_log \
		Library_scn) \
))

$(eval $(call gb_Module_add_l10n_targets,extensions,\
	AllLangResTarget_abp \
	AllLangResTarget_scn \
	AllLangResTarget_upd \
	UIConfig_sabpilot \
	UIConfig_scanner \
))

ifneq ($(filter-out IOS ANDROID,$(OS)),)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_ldapbe2 \
))
endif

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,extensions,\
	Library_bib \
	Library_dbp \
	Library_pcr \
))
$(eval $(call gb_Module_add_l10n_targets,extensions,\
	AllLangResTarget_bib \
	AllLangResTarget_dbp \
	AllLangResTarget_pcr \
	UIConfig_sbibliography \
	UIConfig_spropctrlr \
))
endif

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,extensions,\
	Library_updatefeed \
))

ifeq ($(ENABLE_ONLINE_UPDATE),TRUE)
$(eval $(call gb_Module_add_targets,extensions,\
	Configuration_updchk \
	Library_updatecheckui \
	Library_updchk \
))
$(eval $(call gb_Module_add_l10n_targets,extensions,\
	AllLangResTarget_updchk \
))

$(eval $(call gb_Module_add_check_targets,extensions,\
    CppunitTest_extensions_test_update \
))
endif
endif

ifeq ($(OS),WNT)

ifeq ($(DISABLE_ATL),)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_oleautobridge \
))
endif # DISABLE_ATL

endif # WNT

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_OOoSpotlightImporter \
	Package_mdibundle \
	Package_OOoSpotlightImporter \
))
endif # OS=MACOSX

$(eval $(call gb_Module_add_subsequentcheck_targets,extensions,\
    JunitTest_extensions_unoapi \
))

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
