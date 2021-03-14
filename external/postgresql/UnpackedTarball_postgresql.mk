# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,postgresql))

$(eval $(call gb_UnpackedTarball_set_tarball,postgresql,$(POSTGRESQL_TARBALL),,postgresql))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,postgresql,config))

$(eval $(call gb_UnpackedTarball_add_patches,postgresql, \
	external/postgresql/windows.patch.0 \
	external/postgresql/postgresql.exit.patch.0 \
	external/postgresql/postgres-msvc-build.patch.1 \
	$(if $(filter WNT_ARM64,$(OS)_$(CPUNAME)), external/postgresql/arm64.patch.1) \
))

ifeq ($(CROSS_COMPILING),)
$(eval $(call gb_UnpackedTarball_add_file,postgresql,src/tools/msvc/config.pl,external/postgresql/config.pl))
endif

# vim: set noet sw=4 ts=4:
