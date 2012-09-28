# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libcdr_inc,cdr))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libcdr_inc,inc/external/libcdr,\
	src/lib/libcdr.h \
	src/lib/CDRDocument.h \
	src/lib/CDRStringVector.h \
	src/lib/CMXDocument.h \
))

# vim: set noet sw=4 ts=4:
