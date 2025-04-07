# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,argon2))

$(eval $(call gb_UnpackedTarball_set_tarball,argon2,$(ARGON2_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,argon2,\
	external/argon2/0001-Fix-possible-compiler-error-due-to-undefined-_MSC_VE.patch \
	$(if $(filter WNT_AARCH64,$(OS)_$(CPUNAME)),external/argon2/0002-Add-WinARM64-vcxproj-config.patch) \
	external/argon2/private-symbols.patch.0 \
))

# vim: set noet sw=4 ts=4:
