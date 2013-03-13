# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libvisio,visio))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libvisio,inc/external/libvisio,\
	src/lib/libvisio.h \
	src/lib/VisioDocument.h \
	src/lib/VSDStringVector.h \
))

$(eval $(call gb_ExternalPackage_use_external_project,libvisio,libvisio))

ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,libvisio,lib/visio-0.0.lib,src/lib/.libs/libvisio-0.0.lib))
else
$(eval $(call gb_ExternalPackage_add_file,libvisio,lib/libvisio-0.0.a,src/lib/.libs/libvisio-0.0.a))
endif


# vim: set noet sw=4 ts=4:
