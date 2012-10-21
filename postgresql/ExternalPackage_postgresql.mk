# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,postgresql,postgresql))

$(eval $(call gb_ExternalPackage_use_external_project,postgresql,postgresql))

$(eval $(call gb_ExternalPackage_add_unpacked_files,postgresql,inc/postgresql,\
	src/include/postgres_ext.h \
	src/interfaces/libpq/libpq-fe.h \
))

$(eval $(call gb_ExternalPackage_add_files,postgresql,inc/postgresql,\
	src/interfaces/libpq/libpq-flags.mk \
))

ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_files,postgresql,lib,\
	src/interfaces/libpq/libpq.lib \
))
else
$(eval $(call gb_ExternalPackage_add_files,postgresql,lib,\
	src/interfaces/libpq/libpq.a \
))
endif
# vim: set noet sw=4 ts=4:
