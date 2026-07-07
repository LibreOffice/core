# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libjxl))

$(eval $(call gb_StaticLibrary_use_unpacked,libjxl,libjxl))

$(eval $(call gb_StaticLibrary_use_externals,libjxl,\
    brotli \
    highway \
    lcms2 \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,libjxl))

$(eval $(call gb_StaticLibrary_set_include,libjxl,\
    -I$(WORKDIR)/UnpackedTarball/libjxl \
    -I$(WORKDIR)/UnpackedTarball/libjxl/lib/include \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,libjxl,\
    -DJPEGXL_ENABLE_TRANSCODE_JPEG=OFF \
    -DJXL_INTERNAL_LIBRARY_BUILD \
    -DJXL_THREADS_INTERNAL_LIBRARY_BUILD \
    -Djxl_cms_EXPORTS \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,libjxl,cc))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,libjxl,\
    UnpackedTarball/libjxl/lib/jxl/decode \
    UnpackedTarball/libjxl/lib/jxl/encode \
    UnpackedTarball/libjxl/lib/threads/resizable_parallel_runner \
    UnpackedTarball/libjxl/lib/jxl/memory_manager_internal \
    UnpackedTarball/libjxl/lib/jxl/headers \
    UnpackedTarball/libjxl/lib/jxl/icc_codec \
    UnpackedTarball/libjxl/lib/jxl/dec_xyb \
    UnpackedTarball/libjxl/lib/jxl/dec_frame \
    UnpackedTarball/libjxl/lib/jxl/box_content_decoder \
    UnpackedTarball/libjxl/lib/jxl/color_encoding_internal \
    UnpackedTarball/libjxl/lib/jxl/fields \
    UnpackedTarball/libjxl/lib/jxl/image_metadata \
    UnpackedTarball/libjxl/lib/jxl/frame_header \
    UnpackedTarball/libjxl/lib/jxl/quant_weights \
    UnpackedTarball/libjxl/lib/jxl/quantizer \
    UnpackedTarball/libjxl/lib/jxl/icc_codec_common \
    UnpackedTarball/libjxl/lib/jxl/loop_filter \
    UnpackedTarball/libjxl/lib/jxl/dec_modular \
    UnpackedTarball/libjxl/lib/jxl/modular/modular_image \
    UnpackedTarball/libjxl/lib/jxl/image \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/render_pipeline \
    UnpackedTarball/libjxl/lib/jxl/modular/transform/transform \
    UnpackedTarball/libjxl/lib/jxl/modular/transform/palette \
    UnpackedTarball/libjxl/lib/jxl/modular/transform/squeeze \
    UnpackedTarball/libjxl/lib/jxl/dec_patch_dictionary \
    UnpackedTarball/libjxl/lib/jxl/dec_cache \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_write \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_from_linear \
    UnpackedTarball/libjxl/lib/jxl/blending \
    UnpackedTarball/libjxl/lib/jxl/alpha \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_tone_mapping \
    UnpackedTarball/libjxl/lib/jxl/image_bundle \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_epf \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_to_linear \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_spot \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_cms \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_blending \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_xyb \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_ycbcr \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_noise \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_splines \
    UnpackedTarball/libjxl/lib/jxl/splines \
    UnpackedTarball/libjxl/lib/jxl/passes_state \
    UnpackedTarball/libjxl/lib/jxl/chroma_from_luma \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_upsampling \
    UnpackedTarball/libjxl/lib/jxl/dec_ans \
    UnpackedTarball/libjxl/lib/jxl/dec_huffman \
    UnpackedTarball/libjxl/lib/jxl/dec_context_map \
    UnpackedTarball/libjxl/lib/jxl/huffman_table \
    UnpackedTarball/libjxl/lib/jxl/dec_noise \
    UnpackedTarball/libjxl/lib/jxl/modular/encoding/encoding \
    UnpackedTarball/libjxl/lib/jxl/simd_util \
    UnpackedTarball/libjxl/lib/jxl/opsin_params \
    UnpackedTarball/libjxl/lib/jxl/entropy_coder \
    UnpackedTarball/libjxl/lib/jxl/toc \
    UnpackedTarball/libjxl/lib/jxl/compressed_dc \
    UnpackedTarball/libjxl/lib/jxl/coeff_order \
    UnpackedTarball/libjxl/lib/jxl/dec_group \
    UnpackedTarball/libjxl/lib/jxl/modular/encoding/dec_ma \
    UnpackedTarball/libjxl/lib/jxl/epf \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/low_memory_render_pipeline \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/simple_render_pipeline \
    UnpackedTarball/libjxl/lib/jxl/modular/transform/rct \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_chroma_upsampling \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_gaborish \
    UnpackedTarball/libjxl/lib/jxl/render_pipeline/stage_patches \
    UnpackedTarball/libjxl/lib/jxl/ac_strategy \
    UnpackedTarball/libjxl/lib/jxl/dec_group_border \
    UnpackedTarball/libjxl/lib/jxl/ans_common \
))

# vim: set noet sw=4 ts=4:
