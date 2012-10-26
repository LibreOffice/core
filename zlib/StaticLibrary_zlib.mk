# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,zlib))

$(eval $(call gb_StaticLibrary_use_unpacked,zlib,zlib))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,zlib))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,zlib,\
	UnpackedTarball/zlib/adler32 \
	UnpackedTarball/zlib/compress \
	UnpackedTarball/zlib/crc32 \
	UnpackedTarball/zlib/deflate \
	UnpackedTarball/zlib/inffast \
	UnpackedTarball/zlib/inflate \
	UnpackedTarball/zlib/inftrees \
	UnpackedTarball/zlib/trees \
	UnpackedTarball/zlib/zutil \
))

# vim: set noet sw=4 ts=4:
