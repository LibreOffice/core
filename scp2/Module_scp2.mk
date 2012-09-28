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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Module_Module,scp2))

$(eval $(call gb_Module_add_targets,scp2,\
	CustomTarget_langmacros \
	InstallModule_accessories \
	InstallModule_base \
	InstallModule_calc \
	InstallModule_canvas \
	InstallModule_draw \
	InstallModule_extensions \
	InstallModule_graphicfilter \
	InstallModule_impress \
	InstallModule_math \
	InstallModule_onlineupdate \
	InstallModule_ooo \
	InstallModule_python \
	InstallModule_sdkoo \
	InstallModule_smoketest \
	InstallModule_ure \
	InstallModule_ure_standalone \
	InstallModule_writer \
	InstallModule_xsltfilter \
	InstallScript_sdkoo \
	InstallScript_setup_osl \
	InstallScript_test \
	$(if $(filter WNT,$(GUI)),\
		InstallModule_activex \
		InstallModule_quickstart \
		InstallModule_windows \
		$(if $(filter MSC,$(COM)),\
			InstallModule_winexplorerext \
		) \
	) \
	$(if $(filter YES,$(WITH_BINFILTER)),\
		InstallModule_binfilter \
	) \
	$(if $(filter TRUE,$(ENABLE_CRASHDUMP)),\
		InstallModule_crashrep \
	) \
	$(if $(filter TRUE,$(ENABLE_EVOAB2) $(ENABLE_GCONF) $(ENABLE_GNOMEVFS) $(ENABLE_GIO) $(ENABLE_GTK) $(ENABLE_GTK3)),\
		InstallModule_gnome \
	) \
	$(if $(filter TRUE,$(SOLAR_JAVA)),\
		InstallModule_javafilter \
	) \
	$(if $(filter TRUE,$(ENABLE_KDE) $(ENABLE_KDE4)),\
		InstallModule_kde \
	) \
	$(if $(filter TRUE,$(ENABLE_TDE)),\
		InstallModule_tde \
	) \
	$(if $(filter-out YES,$(SYSTEM_STDLIBS)),\
		InstallModule_stdlibs \
	) \
	$(if $(filter-out MACOSX,$(OS)),\
		InstallScript_ure \
	) \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
