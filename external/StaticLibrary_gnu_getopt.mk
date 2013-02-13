# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,gnu_getopt))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,gnu_getopt))

$(eval $(call gb_StaticLibrary_use_unpacked,gnu_getopt,glibc))

$(eval $(call gb_StaticLibrary_set_include,gnu_getopt,\
    -I$(call gb_UnpackedTarball_get_dir,glibc) \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,gnu_getopt,\
    -DHAVE_STRING_H \
    -DHAVE_CONFIG_H \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,gnu_getopt,\
    UnpackedTarball/glibc/posix/getopt \
    UnpackedTarball/glibc/posix/getopt1 \
))

# vim: set noet sw=4 ts=4:
