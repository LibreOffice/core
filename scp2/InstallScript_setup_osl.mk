# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallScript_InstallScript,setup_osl))

$(eval $(call gb_InstallScript_use_modules,setup_osl,\
	scp2/accessories \
	scp2/base \
	scp2/calc \
	scp2/draw \
	scp2/graphicfilter \
	scp2/impress \
	scp2/math \
	scp2/ooo \
	$(if $(filter TRUE,$(ENABLE_FIREBIRD_SDBC)),\
		scp2/firebird \
	) \
	scp2/python \
	scp2/ure \
	scp2/writer \
	scp2/xsltfilter \
	$(if $(filter WNT,$(OS)),\
		$(if $(DISABLE_ACTIVEX),,scp2/activex) \
		scp2/quickstart \
		scp2/windows \
		$(if $(filter MSC,$(COM)),\
			scp2/winexplorerext \
		) \
	) \
	$(if $(filter TRUE,$(ENABLE_CRASHDUMP)),\
		scp2/crashrep \
	) \
	$(if $(filter YES,$(WITH_EXTENSION_INTEGRATION)),\
		scp2/extensions \
	) \
	$(if $(filter TRUE,$(ENABLE_EVOAB2) $(ENABLE_GCONF) $(ENABLE_GNOMEVFS) $(ENABLE_GIO) $(ENABLE_GTK) $(ENABLE_GTK3)),\
		scp2/gnome \
	) \
	$(if $(filter TRUE,$(ENABLE_KDE) $(ENABLE_KDE4)),\
		scp2/kde \
	) \
	$(if $(filter TRUE,$(ENABLE_ONLINE_UPDATE)),\
		scp2/onlineupdate \
	) \
	$(if $(filter TRUE,$(ENABLE_TDE)),\
		scp2/tde \
	) \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
