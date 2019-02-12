# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libpng))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,libpng))

$(eval $(call gb_StaticLibrary_use_unpacked,libpng,libpng))

$(eval $(call gb_StaticLibrary_use_externals,libpng,\
	zlib \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libpng,\
	UnpackedTarball/libpng/png \
	UnpackedTarball/libpng/pngerror \
	UnpackedTarball/libpng/pngget \
	UnpackedTarball/libpng/pngmem \
	UnpackedTarball/libpng/pngpread \
	UnpackedTarball/libpng/pngread \
	UnpackedTarball/libpng/pngrio \
	UnpackedTarball/libpng/pngrtran \
	UnpackedTarball/libpng/pngrutil \
	UnpackedTarball/libpng/pngset \
	UnpackedTarball/libpng/pngtrans \
	UnpackedTarball/libpng/pngwio \
	UnpackedTarball/libpng/pngwrite \
	UnpackedTarball/libpng/pngwtran \
	UnpackedTarball/libpng/pngwutil \
	$(if $(filter ARM ARM64,$(CPUNAME)),\
	    UnpackedTarball/libpng/arm/arm_init \
	    UnpackedTarball/libpng/arm/filter_neon_intrinsics \
	) \
))

# vim: set noet sw=4 ts=4:
