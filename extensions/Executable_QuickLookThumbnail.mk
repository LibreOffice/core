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

$(eval $(call gb_Executable_Executable,QuickLookThumbnail))

$(eval $(call gb_Executable_set_include,QuickLookThumbnail,\
	-I$(SRCDIR)/extensions/source/macosx/common \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_add_ldflags,QuickLookThumbnail,\
	-e _NSExtensionMain -fobjc-link-runtime -fapplication-extension \
))

$(eval $(call gb_Executable_add_objcobjects,QuickLookThumbnail,\
	extensions/source/macosx/quicklookthumbnail/ThumbnailProvider \
))

$(eval $(call gb_Executable_use_system_darwin_frameworks,QuickLookThumbnail,\
	AppKit \
	QuickLookThumbnailing \
))

$(eval $(call gb_Executable_use_static_libraries,QuickLookThumbnail,\
	extensions_macosx_common \
))

$(eval $(call gb_Executable_use_external,QuickLookThumbnail,zlib))

# vim:set noet sw=4 ts=4:
