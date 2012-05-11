# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
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
# [ Copyright (C) 2011 Peter Foley <pefoley2@verizon.net> (initial developer) ]
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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
