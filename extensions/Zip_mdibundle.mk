# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Zip_Zip,mdibundle,$(WORKDIR)/Zip/mdibundle))

$(eval $(call gb_Zip_add_file,mdibundle,schema.xml,$(SRCDIR)/extensions/source/macosx/spotlight/mdimporter/schema.xml))
$(eval $(call gb_Zip_add_file,mdibundle,Info.plist,$(SRCDIR)/extensions/source/macosx/spotlight/mdimporter/Info.plist))
$(eval $(call gb_Zip_add_file,mdibundle,en.lproj/schema.strings,$(SRCDIR)/extensions/source/macosx/spotlight/mdimporter/en.lproj/schema.strings))
$(eval $(call gb_Zip_add_file,mdibundle,MacOS/OOoSpotlightImporter,$(OUTDIR)/lib/libOOoSpotlightImporter.dylib))

# vim:set noet sw=4 ts=4:
