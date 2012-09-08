# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,png))

$(eval $(call gb_StaticLibrary_use_unpacked,png,png))

$(eval $(call gb_StaticLibrary_use_package,png,libpng_inc))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,png,\
	UnpackedTarball/png/pngerror \
	UnpackedTarball/png/pngget \
	UnpackedTarball/png/pngmem \
	UnpackedTarball/png/pngpread \
	UnpackedTarball/png/pngread \
	UnpackedTarball/png/pngrio \
	UnpackedTarball/png/pngrtran \
	UnpackedTarball/png/pngrutil \
	UnpackedTarball/png/pngset \
	UnpackedTarball/png/pngtest \
	UnpackedTarball/png/pngtrans \
	UnpackedTarball/png/pngwio \
	UnpackedTarball/png/pngwrite \
	UnpackedTarball/png/pngwtran \
	UnpackedTarball/png/pngwutil \
))

# vim: set noet sw=4 ts=4:
