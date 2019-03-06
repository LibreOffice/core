# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,zlib_x64))

$(eval $(call gb_StaticLibrary_set_x64,zlib_x64,YES))

$(eval $(call gb_StaticLibrary_use_unpacked,zlib_x64,zlib))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,zlib_x64))

$(eval $(call gb_StaticLibrary_set_include,zlib_x64,\
	-I$(call gb_UnpackedTarball_get_dir,zlib) \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_x64_generated_cobjects,zlib_x64,\
	UnpackedTarball/zlib/x64/adler32 \
	UnpackedTarball/zlib/x64/compress \
	UnpackedTarball/zlib/x64/crc32 \
	UnpackedTarball/zlib/x64/deflate \
	UnpackedTarball/zlib/x64/inffast \
	UnpackedTarball/zlib/x64/inflate \
	UnpackedTarball/zlib/x64/inftrees \
	UnpackedTarball/zlib/x64/trees \
	UnpackedTarball/zlib/x64/zutil \
))

ifeq ($(ENABLE_DEBUG),TRUE)
$(eval $(call gb_StaticLibrary_add_cflags,zlib_x64,-DZLIB_DEBUG))
endif

# vim: set noet sw=4 ts=4:
