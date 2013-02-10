# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,sysui/icons,$(SRCDIR)/sysui/desktop/icons))

$(eval $(call gb_Package_add_file,sysui/icons,bin/soffice.ico,main_app.ico))

$(eval $(call gb_Package_add_files,sysui/icons,res/icons,\
	oasis-database.ico \
	oasis-drawing-template.ico \
	oasis-drawing.ico \
	oasis-empty-template.ico \
	oasis-empty-document.ico \
	oasis-formula.ico \
	oasis-master-document.ico \
	oasis-presentation-template.ico \
	oasis-presentation.ico \
	oasis-spreadsheet-template.ico \
	oasis-spreadsheet.ico \
	oasis-text-template.ico \
	oasis-text.ico \
	oasis-web-template.ico \
	database.ico \
	drawing-template.ico \
	drawing.ico \
	empty-template.ico \
	empty-document.ico \
	formula.ico \
	master-document.ico \
	presentation-template.ico \
	presentation.ico \
	spreadsheet-template.ico \
	spreadsheet.ico \
	text-template.ico \
	text.ico \
	impress_app.ico \
	calc_app.ico \
	draw_app.ico \
	base_app.ico \
	writer_app.ico \
	math_app.ico \
	main_app.ico \
	open.ico \
	oxt-extension.ico \
))

# vim: set noet sw=4 ts=4:
