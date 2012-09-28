# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libvisio_inc,visio))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libvisio_inc,inc/external/libvisio,\
	src/lib/libvisio.h \
	src/lib/VisioDocument.h \
	src/lib/VSDStringVector.h \
))

# vim: set noet sw=4 ts=4:
