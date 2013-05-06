# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libwpg,libwpg))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libwpg,inc/external/libwpg,\
	src/lib/libwpg.h \
	src/lib/WPGPaintInterface.h \
	src/lib/WPGraphics.h \
))

$(eval $(call gb_ExternalPackage_use_external_project,libwpg,libwpg))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_ExternalPackage_add_file,libwpg,lib/wpg-0.2.lib,src/lib/.libs/libwpg-0.2.lib))
else
$(eval $(call gb_ExternalPackage_add_file,libwpg,lib/libwpg-0.2.a,src/lib/.libs/libwpg-0.2.a))
endif

# vim: set noet sw=4 ts=4:
