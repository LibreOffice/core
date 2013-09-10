# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplwizbitmap,$(SRCDIR)/extras/source/templates/wizard/bitmap))

$(eval $(call gb_Package_set_outdir,extras_tplwizbitmap,$(gb_INSTROOT)))

$(eval $(call gb_Package_add_files,extras_tplwizbitmap,$(LIBO_SHARE_FOLDER)/template/wizard/bitmap,\
	cancel_down.bmp \
	cancel_up.bmp \
	down.bmp \
	end.bmp \
	euro_1.bmp \
	euro_2.bmp \
	euro_3.bmp \
	ftpconnected.gif \
	ftpconnecting.gif \
	ftperror.gif \
	ftpunknown.gif \
	Import_1.bmp \
	Import_3.bmp \
	Import_4.bmp \
	maximize.bmp \
	minimize.bmp \
	MS-Import_2-1.bmp \
	MS-Import_2-2.bmp \
	MS-Import_2-3.bmp \
	okay_down.bmp \
	okay_up.bmp \
	report.bmp \
	tutorial_background.gif \
	up.bmp \
	XML-Import_2-1.bmp \
	XML-Import_2-2.bmp \
	XML-Import_2-3.bmp \
	XML-Import_2-4.bmp \
))

# vim: set noet sw=4 ts=4:
