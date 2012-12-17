# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,gnu_readdir_r))

$(eval $(call gb_StaticLibrary_use_unpacked,gnu_readdir_r,glibc))

$(eval $(call gb_StaticLibrary_set_include,gnu_readdir_r,\
    -I$(call gb_UnpackedTarball_get_dir,glibc) \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,gnu_readdir_r,\
    -DHAVE_STRING_H \
    -DHAVE_CONFIG_H \
))

$(eval $(call gb_StaticLibrary_add_cobjects,gnu_readdir_r,\
    external/glibc/readdir_r \
))

# vim: set noet sw=4 ts=4:
