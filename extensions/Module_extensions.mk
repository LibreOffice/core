# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Jan Holesovsky <kendy@suse.cz> (initial developer)
# Copyright (C) 2011 Peter Foley <pefoley2@verizon.net>
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
	Package_uiconfig \
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
	Library_oleautobridge2 \
))
endif # DISABLE_ATL

endif # WNT

ifeq ($(ENABLE_NSPLUGIN),YES)
$(eval $(call gb_Module_add_targets,extensions,\
	Executable_nsplugin \
	Library_npsoplugin \
	StaticLibrary_npsoenv \
	WinResTarget_npsoplugin \
))
endif

ifeq ($(WITH_MOZILLA),YES)

$(eval $(call gb_Module_add_targets,extensions,\
	Library_pl \
))

ifeq ($(GUI),UNX)
ifneq ($(GUIBASE),aqua)
$(eval $(call gb_Module_add_targets,extensions,\
	StaticLibrary_plugcon \
	Executable_pluginapp.bin \
))
endif
endif

endif # WITH_MOZILLA=YES

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_OOoSpotlightImporter \
	Zip_mdibundle \
))
endif # OS=MACOSX

$(eval $(call gb_Module_add_subsequentcheck_targets,extensions,\
    JunitTest_extensions_unoapi \
))

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
