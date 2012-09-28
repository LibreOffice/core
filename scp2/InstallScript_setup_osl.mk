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
#  (initial developer) \
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

$(eval $(call gb_InstallScript_InstallScript,setup_osl))

$(eval $(call gb_InstallScript_use_modules,setup_osl,\
	scp2/accessories \
	scp2/base \
	scp2/calc \
	scp2/canvas \
	scp2/draw \
	scp2/graphicfilter \
	scp2/impress \
	scp2/math \
	scp2/ooo \
	scp2/python \
	scp2/ure \
	scp2/writer \
	scp2/xsltfilter \
	$(if $(filter WNT,$(GUI)),\
		scp2/activex \
		scp2/quickstart \
		scp2/windows \
		$(if $(filter MSC,$(COM)),\
			scp2/winexplorerext \
		) \
	) \
	$(if $(filter YES,$(WITH_BINFILTER)),\
		scp2/binfilter \
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
	$(if $(filter TRUE,$(SOLAR_JAVA)),\
		scp2/javafilter \
	) \
	$(if $(filter TRUE,$(ENABLE_KDE) $(ENABLE_KDE4)),\
		scp2/kde \
	) \
	$(if $(filter TRUE,$(ENABLE_ONLINE_UPDATE)),\
		scp2/onlineupdate \
	) \
	$(if $(filter-out YES,$(SYSTEM_STDLIBS)),\
		scp2/stdlibs \
	) \
	$(if $(filter TRUE,$(ENABLE_TDE)),\
		scp2/tde \
	) \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
