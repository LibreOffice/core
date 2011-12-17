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
	AllLangResTarget_bib \
	AllLangResTarget_pcr \
	AllLangResTarget_san \
	AllLangResTarget_upd \
	AllLangResTarget_updchk \
	Configuration_updchk \
	Library_bib \
	Library_log \
	Library_pcr \
	Library_scn \
	Library_updatecheckui \
	Library_updatefeed \
	Library_updchk \
	Package_bib \
	Package_pcr \
))

ifeq ($(OS),WNT)
ifeq ($(DISABLE_ATL),)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_oleautobridge \
	Library_oleautobridge2 \
))
endif
endif

ifneq ($(WITH_MOZILLA),NO)

$(eval $(call gb_Module_add_targets,extensions,\
	Library_pl \
	Executable_nsplugin \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_npsoplugin \
	WinResTarget_npsoplugin \
))
endif # GUI=WNT

ifeq ($(GUI),UNX)

$(eval $(call gb_Module_add_targets,extensions,\
	Executable_pluginapp.bin \
))

ifneq ($(ENABLE_GTK),)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_npsoplugin \
))
endif # ENABLE_GTK

endif # GUI=UNX

endif # WITH_MOZILLA=YES

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_OOoSpotlightImporter \
	Zip_mdibundle \
))
endif # OS=MACOSX

ifeq ($(WITH_LDAP),YES)
$(eval $(call gb_Module_add_targets,extensions,\
	Library_ldapbe2 \
))
endif # WITH_LDAP=YES

$(eval $(call gb_Module_add_check_targets,extensions,\
    CppunitTest_extensions_test_update \
))

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
