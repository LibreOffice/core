# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

# TODO is this really supposed to be delivered to the officecfg registry?
$(eval $(call gb_Configuration_Configuration,updchk))

$(eval $(call gb_Configuration_add_spool_modules,updchk,extensions/source/update/check,\
	org/openoffice/Office/Jobs-onlineupdate.xcu \
	org/openoffice/Office/Addons-onlineupdate.xcu \
))

# The resulting solver/*/pck/updchk_*.zip are merged into registry_*.xcd in
# postprocess/packregistry/makefile.mk (in principle, localizations of an
# install:module should go into their own per-lang xcd files, but they are
# currently all merged into a global per-lang registry_*.xcd, see e.g. how
# localized "Title" property values of
# /org.openoffice.Office.Common/Menus/New/m0 (install:module="writer") end up in
# registry_*.xcd instead of a, say, writer_*.xcd):
$(eval $(call gb_Configuration_add_localized_datas,updchk,extensions/source/update/check,\
	org/openoffice/Office/Addons.xcu \
))

# vim:set noet sw=4 ts=4:
