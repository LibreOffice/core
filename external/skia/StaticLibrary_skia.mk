# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,skia))

$(eval $(call gb_StaticLibrary_use_externals,skia,\
	freetype \
	fontconfig \
))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,skia))

$(eval $(call gb_StaticLibrary_use_unpacked,skia,skia))

$(eval $(call gb_StaticLibrary_add_defs,skia,\
	-DSK_GAMMA_SRGB \
	-DSK_GAMMA_APPLY_TO_A8 \
	-DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 \
	-DSK_SCALAR_IS_FLOAT \
	-DSK_CAN_USE_FLOAT \
	-DSK_SUPPORT_GPU=0 \
	-DSK_SAMPLES_FOR_X \
	-DSK_BUILD_FOR_UNIX \
	-DSK_USE_POSIX_THREADS \
	-DSK_RELEASE \
	-DGR_RELEASE=1 \
	-DNDEBUG \
))

$(eval $(call gb_StaticLibrary_add_cxxflags,skia, \
	-Wno-unused-parameter \
	-Wno-shadow \
	-mssse3 \
))

$(eval $(call gb_StaticLibrary_set_include,skia,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/codec \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/core \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/config \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/gpu \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/pipe \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/lazy \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/ports \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/images \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/utils \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/utils/mac \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/utils/win \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/views \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/effects \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/xml \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/pathops \
	-I$(call gb_UnpackedTarball_get_dir,skia)/include/private \
	-I$(call gb_UnpackedTarball_get_dir,skia)/src/core \
	-I$(call gb_UnpackedTarball_get_dir,skia)/src/lazy \
	-I$(call gb_UnpackedTarball_get_dir,skia)/src/image \
	-I$(call gb_UnpackedTarball_get_dir,skia)/src/gpu \
	-I$(call gb_UnpackedTarball_get_dir,skia)/src/gpu/effects \
	-I$(call gb_UnpackedTarball_get_dir,skia)/src/gl \
	-I$(call gb_UnpackedTarball_get_dir,skia)/src/utils \
	-I$(call gb_UnpackedTarball_get_dir,skia)/src/sfnt \
	-I$(call gb_UnpackedTarball_get_dir,skia)/src/shaders \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,skia,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,skia,\
	UnpackedTarball/skia/src/core/SkAAClip \
	UnpackedTarball/skia/src/core/SkAnnotation \
	UnpackedTarball/skia/src/core/SkAlphaRuns \
	UnpackedTarball/skia/src/core/SkBitmap \
	UnpackedTarball/skia/src/core/SkBitmapProcState \
	UnpackedTarball/skia/src/core/SkBitmapProcState_matrixProcs \
	UnpackedTarball/skia/src/core/SkBuffer \
	UnpackedTarball/skia/src/core/SkCanvas \
	UnpackedTarball/skia/src/core/SkColor \
	UnpackedTarball/skia/src/core/SkColorFilter \
	UnpackedTarball/skia/src/core/SkDevice \
	UnpackedTarball/skia/src/core/SkPoint \
))

# vim: set noet sw=4 ts=4:
