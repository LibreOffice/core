# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,scp2))

$(eval $(call gb_Module_add_targets,scp2,\
	AutoInstall \
	CustomTarget_langmacros \
	InstallModule_accessories \
	InstallModule_base \
	InstallModule_calc \
	InstallModule_draw \
	InstallModule_extensions \
	$(if $(filter TRUE,$(ENABLE_FIREBIRD_SDBC)),\
		InstallModule_firebird \
	) \
	InstallModule_graphicfilter \
	InstallModule_impress \
	InstallModule_math \
	InstallModule_onlineupdate \
	InstallModule_ooo \
	InstallModule_python \
	InstallModule_smoketest \
	InstallModule_ure \
	InstallModule_writer \
	InstallModule_xsltfilter \
	InstallScript_setup_osl \
	InstallScript_test \
	$(if $(filter ODK,$(BUILD_TYPE)), \
		InstallModule_sdkoo \
		InstallScript_sdkoo \
	) \
	$(if $(filter WNT,$(OS)),\
		InstallModule_activex \
		InstallModule_quickstart \
		InstallModule_windows \
		$(if $(filter MSC,$(COM)),\
			InstallModule_winexplorerext \
		) \
	) \
	$(if $(filter TRUE,$(ENABLE_CRASHDUMP)),\
		InstallModule_crashrep \
	) \
	$(if $(filter TRUE,$(ENABLE_EVOAB2) $(ENABLE_GCONF) $(ENABLE_GNOMEVFS) $(ENABLE_GIO) $(ENABLE_GTK) $(ENABLE_GTK3)),\
		InstallModule_gnome \
	) \
	$(if $(filter TRUE,$(ENABLE_KDE) $(ENABLE_KDE4)),\
		InstallModule_kde \
	) \
	$(if $(filter TRUE,$(ENABLE_TDE)),\
		InstallModule_tde \
	) \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
