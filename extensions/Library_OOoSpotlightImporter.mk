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

$(eval $(call gb_Library_Bundle,OOoSpotlightImporter))

$(eval $(call gb_Library_add_objcobjects,OOoSpotlightImporter,\
	extensions/source/macosx/spotlight/GetMetadataForFile \
	extensions/source/macosx/spotlight/OOoContentDataParser \
	extensions/source/macosx/spotlight/OOoMetaDataParser \
	extensions/source/macosx/spotlight/OOoSpotlightImporter \
	extensions/source/macosx/spotlight/main \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,OOoSpotlightImporter,\
	CoreFoundation \
	Foundation \
	CoreServices \
))

$(eval $(call gb_Library_use_external,OOoSpotlightImporter,zlib))

# vim:set noet sw=4 ts=4:
