# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,zstd))

$(eval $(call gb_StaticLibrary_use_unpacked,zstd,zstd))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,zstd))

$(eval $(call gb_StaticLibrary_set_include,zstd,\
    -I$(gb_UnpackedTarball_workdir)/zstd/lib \
    -I$(gb_UnpackedTarball_workdir)/zstd/lib/common \
    -I$(gb_UnpackedTarball_workdir)/zstd/lib/decompress \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,zstd,\
    UnpackedTarball/zstd/lib/common/entropy_common \
    UnpackedTarball/zstd/lib/common/error_private \
    UnpackedTarball/zstd/lib/common/fse_decompress \
    UnpackedTarball/zstd/lib/common/xxhash \
    UnpackedTarball/zstd/lib/common/zstd_common \
    UnpackedTarball/zstd/lib/decompress/huf_decompress \
    UnpackedTarball/zstd/lib/decompress/zstd_ddict \
    UnpackedTarball/zstd/lib/decompress/zstd_decompress \
    UnpackedTarball/zstd/lib/decompress/zstd_decompress_block \
))

$(eval $(call gb_StaticLibrary_add_cflags,zstd,-DZSTD_DISABLE_ASM))

ifeq ($(ENABLE_DEBUG),TRUE)
$(eval $(call gb_StaticLibrary_add_cflags,zstd,-DZSTD_DEBUG=1))
endif

# vim: set noet sw=4 ts=4:
