# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libmwaw,mwaw))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libmwaw,inc/external/libmwaw,\
	src/lib/libmwaw.hxx \
	src/lib/MWAWDocument.hxx \
	src/lib/MWAWPropertyHandler.hxx \
))

$(eval $(call gb_ExternalPackage_use_external_project,libmwaw,libmwaw))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_ExternalPackage_add_file,libmwaw,lib/mwaw-0.1.lib,build/win32/Release/lib/libmwaw-0.1.lib))
else
$(eval $(call gb_ExternalPackage_add_file,libmwaw,lib/libmwaw-0.1.a,src/lib/.libs/libmwaw-0.1.a))
endif


# vim: set noet sw=4 ts=4:
