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
# make config.sub recognize arm-linux-androideabi
orcus_patches += liborcus_0.1.0-configure.patch
# disable boost "auto lib" in MSVC build
orcus_patches += liborcus_0.1.0-boost_disable_auto_lib.patch
# <https://gitorious.org/orcus/orcus/merge_requests/2#
# f60d6eecee72349993a392a9a63ddf3383d3b8c8-
# f60d6eecee72349993a392a9a63ddf3383d3b8c8@2>:
orcus_patches += liborcus_0.1.0-boost-include.patch.1

$(eval $(call gb_UnpackedTarball_fix_end_of_line,orcus,\
	vsprojects/liborcus-static-nozip/liborcus-static-nozip.vcproj \
))

$(eval $(call gb_UnpackedTarball_add_patches,orcus,\
	$(foreach patch,$(orcus_patches),liborcus/$(patch)) \
))

$(eval $(call gb_UnpackedTarball_add_file,orcus,vsprojects/liborcus-static-nozip/liborcus-static-nozip.vcxproj,liborcus/vsprojects/liborcus-static-nozip/liborcus-static-nozip.vcxproj))
$(eval $(call gb_UnpackedTarball_add_file,orcus,vsprojects/liborcus/DefaultConfig.props,liborcus/vsprojects/liborcus/DefaultConfig.props))
$(eval $(call gb_UnpackedTarball_add_file,orcus,vsprojects/liborcus/zLib.props,liborcus/vsprojects/liborcus/zLib.props))

# vim: set noet sw=4 ts=4:
