# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,lzma))

$(eval $(call gb_StaticLibrary_use_unpacked,lzma,lzma))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,lzma))

$(eval $(call gb_StaticLibrary_set_include,lzma,\
    -I$(gb_UnpackedTarball_workdir)/lzma/dos \
    -I$(gb_UnpackedTarball_workdir)/lzma/src/common \
    -I$(gb_UnpackedTarball_workdir)/lzma/src/liblzma/api \
    -I$(gb_UnpackedTarball_workdir)/lzma/src/liblzma/check \
    -I$(gb_UnpackedTarball_workdir)/lzma/src/liblzma/common \
    -I$(gb_UnpackedTarball_workdir)/lzma/src/liblzma/delta \
    -I$(gb_UnpackedTarball_workdir)/lzma/src/liblzma/lz \
    -I$(gb_UnpackedTarball_workdir)/lzma/src/liblzma/lzma \
    -I$(gb_UnpackedTarball_workdir)/lzma/src/liblzma/rangecoder \
    -I$(gb_UnpackedTarball_workdir)/lzma/src/liblzma/simple \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_cflags,lzma,\
    -DHAVE_STDINT_H \
    -DHAVE_STDBOOL_H \
    -DHAVE_CONFIG_H \
    $(if $(filter WNT,$(OS)),-DMYTHREAD_VISTA,-DMYTHREAD_POSIX) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,lzma,\
    UnpackedTarball/lzma/src/common/tuklib_cpucores \
    UnpackedTarball/lzma/src/common/tuklib_physmem \
    UnpackedTarball/lzma/src/liblzma/check/check \
    UnpackedTarball/lzma/src/liblzma/check/crc32_fast \
    UnpackedTarball/lzma/src/liblzma/check/crc64_fast \
    UnpackedTarball/lzma/src/liblzma/check/sha256 \
    UnpackedTarball/lzma/src/liblzma/common/alone_decoder \
    UnpackedTarball/lzma/src/liblzma/common/alone_encoder \
    UnpackedTarball/lzma/src/liblzma/common/auto_decoder \
    UnpackedTarball/lzma/src/liblzma/common/block_buffer_decoder \
    UnpackedTarball/lzma/src/liblzma/common/block_buffer_encoder \
    UnpackedTarball/lzma/src/liblzma/common/block_decoder \
    UnpackedTarball/lzma/src/liblzma/common/block_encoder \
    UnpackedTarball/lzma/src/liblzma/common/block_header_decoder \
    UnpackedTarball/lzma/src/liblzma/common/block_header_encoder \
    UnpackedTarball/lzma/src/liblzma/common/block_util \
    UnpackedTarball/lzma/src/liblzma/common/common \
    UnpackedTarball/lzma/src/liblzma/common/easy_buffer_encoder \
    UnpackedTarball/lzma/src/liblzma/common/easy_decoder_memusage \
    UnpackedTarball/lzma/src/liblzma/common/easy_encoder \
    UnpackedTarball/lzma/src/liblzma/common/easy_encoder_memusage \
    UnpackedTarball/lzma/src/liblzma/common/easy_preset \
    UnpackedTarball/lzma/src/liblzma/common/filter_buffer_decoder \
    UnpackedTarball/lzma/src/liblzma/common/filter_buffer_encoder \
    UnpackedTarball/lzma/src/liblzma/common/filter_common \
    UnpackedTarball/lzma/src/liblzma/common/filter_decoder \
    UnpackedTarball/lzma/src/liblzma/common/filter_encoder \
    UnpackedTarball/lzma/src/liblzma/common/filter_flags_decoder \
    UnpackedTarball/lzma/src/liblzma/common/filter_flags_encoder \
    UnpackedTarball/lzma/src/liblzma/common/hardware_cputhreads \
    UnpackedTarball/lzma/src/liblzma/common/hardware_physmem \
    UnpackedTarball/lzma/src/liblzma/common/index \
    UnpackedTarball/lzma/src/liblzma/common/index_decoder \
    UnpackedTarball/lzma/src/liblzma/common/index_encoder \
    UnpackedTarball/lzma/src/liblzma/common/index_hash \
    UnpackedTarball/lzma/src/liblzma/common/outqueue \
    UnpackedTarball/lzma/src/liblzma/common/stream_buffer_decoder \
    UnpackedTarball/lzma/src/liblzma/common/stream_buffer_encoder \
    UnpackedTarball/lzma/src/liblzma/common/stream_decoder \
    UnpackedTarball/lzma/src/liblzma/common/stream_encoder \
    UnpackedTarball/lzma/src/liblzma/common/stream_encoder_mt \
    UnpackedTarball/lzma/src/liblzma/common/stream_flags_common \
    UnpackedTarball/lzma/src/liblzma/common/stream_flags_decoder \
    UnpackedTarball/lzma/src/liblzma/common/stream_flags_encoder \
    UnpackedTarball/lzma/src/liblzma/common/vli_decoder \
    UnpackedTarball/lzma/src/liblzma/common/vli_encoder \
    UnpackedTarball/lzma/src/liblzma/common/vli_size \
    UnpackedTarball/lzma/src/liblzma/delta/delta_common \
    UnpackedTarball/lzma/src/liblzma/delta/delta_decoder \
    UnpackedTarball/lzma/src/liblzma/delta/delta_encoder \
    UnpackedTarball/lzma/src/liblzma/lzma/fastpos_table \
    UnpackedTarball/lzma/src/liblzma/lzma/lzma2_decoder \
    UnpackedTarball/lzma/src/liblzma/lzma/lzma2_encoder \
    UnpackedTarball/lzma/src/liblzma/lzma/lzma_decoder \
    UnpackedTarball/lzma/src/liblzma/lzma/lzma_encoder \
    UnpackedTarball/lzma/src/liblzma/lzma/lzma_encoder_optimum_fast \
    UnpackedTarball/lzma/src/liblzma/lzma/lzma_encoder_optimum_normal \
    UnpackedTarball/lzma/src/liblzma/lzma/lzma_encoder_presets \
    UnpackedTarball/lzma/src/liblzma/lz/lz_decoder \
    UnpackedTarball/lzma/src/liblzma/lz/lz_encoder \
    UnpackedTarball/lzma/src/liblzma/lz/lz_encoder_mf \
    UnpackedTarball/lzma/src/liblzma/rangecoder/price_table \
    UnpackedTarball/lzma/src/liblzma/simple/arm \
    UnpackedTarball/lzma/src/liblzma/simple/arm64 \
    UnpackedTarball/lzma/src/liblzma/simple/armthumb \
    UnpackedTarball/lzma/src/liblzma/simple/ia64 \
    UnpackedTarball/lzma/src/liblzma/simple/powerpc \
    UnpackedTarball/lzma/src/liblzma/simple/simple_coder \
    UnpackedTarball/lzma/src/liblzma/simple/simple_decoder \
    UnpackedTarball/lzma/src/liblzma/simple/simple_encoder \
    UnpackedTarball/lzma/src/liblzma/simple/sparc \
    UnpackedTarball/lzma/src/liblzma/simple/x86 \
))

# vim: set noet sw=4 ts=4:
