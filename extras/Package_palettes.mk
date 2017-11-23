# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_palettes,$(SRCDIR)/extras/source/palettes))

$(eval $(call gb_Package_add_files,extras_palettes,$(LIBO_SHARE_FOLDER)/palette,\
	arrowhd.soe \
	chart-palettes.soc \
	classic.sog \
	freecolour-hlc.soc \
	hatching.soh \
	html.soc \
	libreoffice.soc \
	modern.sog \
	standard.sob \
	standard.soc \
	standard.sod \
	standard.soe \
	standard.sog \
	standard.soh \
	standard.sop \
	styles.sod \
	tonal.soc \
))

# vim: set noet sw=4 ts=4:
