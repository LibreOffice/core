# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extensions_mdibundle,$(SRCDIR)/extensions/source/macosx/spotlight/mdimporter))

$(eval $(call gb_Package_set_outdir,extensions_mdibundle,$(INSTROOT)))

$(eval $(call gb_Package_add_files,extensions_mdibundle,Library/Spotlight/OOoSpotlightImporter.mdimporter/Contents,\
    Info.plist \
    en.lproj/schema.strings \
    schema.xml \
))

# vim: set noet sw=4 ts=4:
