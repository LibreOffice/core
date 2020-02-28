# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,dtoa))

$(eval $(call gb_StaticLibrary_use_unpacked,dtoa,dtoa))

# A place that duplicates these settings is jurt/source/pipe/staticsalhack.cxx:
$(eval $(call gb_StaticLibrary_add_cflags,dtoa,-DIEEE_8087))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,dtoa))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,dtoa,\
    UnpackedTarball/dtoa/src/dtoa \
))

# vim: set noet sw=4 ts=4:
