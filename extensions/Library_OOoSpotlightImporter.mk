# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Bundle,OOoSpotlightImporter))

$(eval $(call gb_Library_set_include,OOoSpotlightImporter,\
	-I$(SRCDIR)/extensions/source/macosx/common \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_objcobjects,OOoSpotlightImporter,\
	extensions/source/macosx/spotlight/GetMetadataForFile \
	extensions/source/macosx/spotlight/main \
))

$(eval $(call gb_Library_use_static_libraries,OOoSpotlightImporter,\
	extensions_macosx_common \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,OOoSpotlightImporter,\
	AppKit \
))

$(eval $(call gb_Library_use_external,OOoSpotlightImporter,zlib))

# vim:set noet sw=4 ts=4:
