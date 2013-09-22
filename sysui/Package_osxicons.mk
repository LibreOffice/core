# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,sysui_osxicons,$(SRCDIR)/sysui/desktop/icons))

$(eval $(call gb_Package_set_outdir,sysui_osxicons,$(INSTROOT)))

$(eval $(call gb_Package_add_files_with_dir,sysui_osxicons,Resources,\
    text.icns \
    oasis-text.icns \
    spreadsheet.icns \
    oasis-spreadsheet.icns \
    presentation.icns \
    oasis-presentation.icns \
    drawing.icns \
    oasis-drawing.icns \
    master-document.icns \
    oasis-master-document.icns \
    formula.icns \
    oasis-formula.icns \
    text-template.icns \
    oasis-text-template.icns \
    spreadsheet-template.icns \
    oasis-spreadsheet-template.icns \
    presentation-template.icns \
    oasis-presentation-template.icns \
    drawing-template.icns \
    oasis-drawing-template.icns \
    oasis-database.icns \
    extension.icns \
    main.icns \
))

# vim: set ts=4 sw=4 et:
