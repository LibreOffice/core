# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,sysui/icons,$(SRCDIR)/sysui/desktop/icons))

$(eval $(call gb_Package_set_outdir,sysui/icons,$(OUTDIR)))

$(eval $(call gb_Package_add_file,sysui/icons,bin/soffice.ico,main_app.ico))

$(eval $(call gb_Package_add_files,sysui/icons,res/icons,\
	oasis-empty-template.ico \
	oasis-empty-document.ico \
	empty-template.ico \
	empty-document.ico \
	open.ico \
))

# vim: set noet sw=4 ts=4:
