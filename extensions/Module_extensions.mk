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
	AllLangResTarget_abp \
	AllLangResTarget_scn \
	AllLangResTarget_upd \
	Library_res \
	$(if $(filter IOS ANDROID,$(OS)),, \
		Library_abp \
		Library_log \
		Library_scn) \
))

ifneq ($(filter-out IOS ANDROID,$(OS)),)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_ldapbe2 \
))
endif

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,extensions,\
	AllLangResTarget_bib \
	AllLangResTarget_dbp \
	AllLangResTarget_pcr \
	Library_bib \
	Library_dbp \
	Library_pcr \
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
	AllLangResTarget_updchk \
	Configuration_updchk \
	Library_updatecheckui \
	Library_updchk \
))

$(eval $(call gb_Module_add_check_targets,extensions,\
    CppunitTest_extensions_test_update \
))
endif
endif

ifeq ($(OS),WNT)

ifeq ($(COM),MSC)
ifneq ($(DISABLE_ACTIVEX),TRUE)
$(eval $(call gb_Module_add_targets,extensions,\
	WinResTarget_activex \
	Library_so_activex \
	CustomTarget_so_activex_idl \
))

ifeq ($(BUILD_X64),TRUE)
$(eval $(call gb_Module_add_targets,extensions,\
	CustomTarget_so_activex_x64 \
	Library_so_activex_x64 \
))
endif # BUILD_X64
endif # DISABLE_ACTIVEX
endif # COM=MSC

ifeq ($(DISABLE_ATL),)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_oleautobridge \
))
endif # DISABLE_ATL

endif # WNT

ifeq ($(ENABLE_NPAPI_FROM_BROWSER),TRUE)

$(eval $(call gb_Module_add_targets,extensions,\
	Library_pl \
))

ifneq ($(OS),WNT)
ifneq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,extensions,\
	StaticLibrary_plugcon \
	Executable_pluginapp.bin \
))
endif
endif

endif # ENABLE_NPAPI_FROM_BROWSER=TRUE

ifeq ($(ENABLE_NPAPI_INTO_BROWSER),TRUE)

$(eval $(call gb_Module_add_targets,extensions,\
	Executable_nsplugin \
	Library_npsoplugin \
	StaticLibrary_npsoenv \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,extensions,\
	WinResTarget_npsoplugin \
))
endif

endif # ENABLE_NPAPI_INTO_BROWSER=TRUE

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
