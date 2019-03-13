# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplwizbitmap,$(SRCDIR)/extras/source/templates/wizard/bitmap))

$(eval $(call gb_Package_add_files,extras_tplwizbitmap,$(LIBO_SHARE_FOLDER)/template/wizard/bitmap,\
	euro_1.png \
	euro_2.png \
	euro_3.png \
	ftpconnected.gif \
	ftpconnecting.gif \
	ftperror.gif \
	ftpunknown.gif \
	Import_1.png \
	Import_3.png \
	Import_4.png \
	maximize.png \
	minimize.png \
	MS-Import_2-1.png \
	MS-Import_2-2.png \
	MS-Import_2-3.png \
	tutorial_background.gif \
	XML-Import_2-1.png \
	XML-Import_2-2.png \
	XML-Import_2-3.png \
	XML-Import_2-4.png \
))

# vim: set noet sw=4 ts=4:
