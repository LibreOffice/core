# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,orcus))

$(eval $(call gb_UnpackedTarball_set_tarball,orcus,$(ORCUS_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,orcus,0))

orcus_patches :=
# -Werror=unused-variable -Werror=unused-private-field -Werror=shadow
orcus_patches += liborcus_0.1.0-warnings.patch
# make config.sub recognize arm-linux-androideabi
orcus_patches += liborcus_0.1.0-configure.patch
# fix MinGW build
orcus_patches += liborcus_0.1.0-mingw.patch
# disable boost "auto lib" in MSVC build
orcus_patches += liborcus_0.1.0-boost_disable_auto_lib.patch

$(eval $(call gb_UnpackedTarball_fix_end_of_line,orcus,\
	vsprojects/liborcus-static-nozip/liborcus-static-nozip.vcproj \
))

$(eval $(call gb_UnpackedTarball_add_patches,orcus,\
	$(foreach patch,$(orcus_patches),liborcus/$(patch)) \
))

# vim: set noet sw=4 ts=4:
