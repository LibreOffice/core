# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,skia))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,skia))

$(eval $(call gb_StaticLibrary_use_unpacked,skia,skia))

#$(eval $(call gb_Library_set_precompiled_header,skia,$(SRCDIR)/external/skia/inc/pch/precompiled_skia))

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

# TODO
$(eval $(call gb_Library_add_defs,skia,\
    -DSK_USER_CONFIG_HEADER="<$(SRCDIR)/external/skia/configs/SkUserConfig.h>" \
))

$(eval $(call gb_StaticLibrary_use_externals,skia,\
	freetype \
	fontconfig \
))

$(eval $(call gb_StaticLibrary_add_cxxflags,skia, \
	-mssse3 \
))

$(eval $(call gb_StaticLibrary_set_include,skia,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,skia) \
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
