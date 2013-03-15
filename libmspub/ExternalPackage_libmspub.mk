# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libmspub,mspub))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libmspub,inc/external/libmspub,\
	src/lib/libmspub.h \
	src/lib/MSPUBDocument.h \
	src/lib/MSPUBStringVector.h \
))

$(eval $(call gb_ExternalPackage_use_external_project,libmspub,libmspub))

ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,libmspub,lib/mspub-0.0.lib,src/lib/.libs/libmspub-0.0.lib))
else
$(eval $(call gb_ExternalPackage_add_file,libmspub,lib/libmspub-0.0.a,src/lib/.libs/libmspub-0.0.a))
endif


# vim: set noet sw=4 ts=4:
