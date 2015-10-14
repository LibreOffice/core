# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_WinResTarget_WinResTarget,sofficebin/officeloader))

$(eval $(call gb_WinResTarget_set_include,sofficebin/officeloader,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sysui/desktop \
))

$(eval $(call gb_WinResTarget_add_defs,sofficebin/officeloader,\
    -DRES_APP_NAME=soffice \
    -DRES_APP_FILEDESC="$(PRODUCTNAME)" \
    -DRES_APP_ICON=icons/soffice.ico \
    -DRES_APP_VENDOR="$(OOO_VENDOR)" \
))

$(eval $(call gb_WinResTarget_add_dependencies,sofficebin/officeloader,\
    sysui/desktop/icons/soffice.ico \
	sysui/desktop/icons/oasis-database.ico \
	sysui/desktop/icons/oasis-drawing-template.ico \
	sysui/desktop/icons/oasis-drawing.ico \
	sysui/desktop/icons/oasis-formula.ico \
	sysui/desktop/icons/oasis-master-document.ico \
	sysui/desktop/icons/oasis-presentation-template.ico \
	sysui/desktop/icons/oasis-presentation.ico \
	sysui/desktop/icons/oasis-spreadsheet-template.ico \
	sysui/desktop/icons/oasis-spreadsheet.ico \
	sysui/desktop/icons/oasis-text-template.ico \
	sysui/desktop/icons/oasis-text.ico \
	sysui/desktop/icons/oasis-web-template.ico \
	sysui/desktop/icons/database.ico \
	sysui/desktop/icons/drawing-template.ico \
	sysui/desktop/icons/drawing.ico \
	sysui/desktop/icons/formula.ico \
	sysui/desktop/icons/master-document.ico \
	sysui/desktop/icons/presentation-template.ico \
	sysui/desktop/icons/presentation.ico \
	sysui/desktop/icons/spreadsheet-template.ico \
	sysui/desktop/icons/spreadsheet.ico \
	sysui/desktop/icons/text-template.ico \
	sysui/desktop/icons/text.ico \
	sysui/desktop/icons/oxt-extension.ico \
))

$(eval $(call gb_WinResTarget_set_rcfile,sofficebin/officeloader,desktop/util/officeloader))

# vim: set ts=4 sw=4 et:
