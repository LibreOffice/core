# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,sysui_osxicons,$(SRCDIR)/sysui/desktop))

$(eval $(call gb_Package_set_outdir,sysui_osxicons,$(INSTDIR)))

$(eval $(call gb_Package_add_files_with_dir,sysui_osxicons,Resources,\
    icons/text.icns \
    icons/oasis-text.icns \
    icons/spreadsheet.icns \
    icons/oasis-spreadsheet.icns \
    icons/presentation.icns \
    icons/oasis-presentation.icns \
    icons/drawing.icns \
    icons/oasis-drawing.icns \
    icons/master-document.icns \
    icons/oasis-master-document.icns \
    icons/formula.icns \
    icons/oasis-formula.icns \
    icons/text-template.icns \
    icons/oasis-text-template.icns \
    icons/spreadsheet-template.icns \
    icons/oasis-spreadsheet-template.icns \
    icons/presentation-template.icns \
    icons/oasis-presentation-template.icns \
    icons/drawing-template.icns \
    icons/oasis-drawing-template.icns \
    icons/oasis-database.icns \
    icons/extension.icns \
    icons/main.icns \
	macosx/Info.plist \
))

# vim: set ts=4 sw=4 et:
