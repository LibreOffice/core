# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,simd))

$(eval $(call gb_StaticLibrary_set_generated_c_base,simd,$(online_srcdir)))

$(eval $(call gb_StaticLibrary_set_include,simd, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(online_srcdir) \
    $$(INCLUDE) \
))

# The delta code is hand-vectorised for AVX2, so it is compiled with -mavx2
# -O3.  configure puts that into SIMD_CFLAGS, forwarded here as
# ONLINE.SIMD_CFLAGS, and leaves it empty where the compiler has no AVX2 (the
# arm64 and ppc64 builds, where -mavx2 is not even a valid option); config.h
# then has ENABLE_SIMD 0 and the file compiles to nothing.
$(eval $(call gb_StaticLibrary_add_cflags,simd, \
    $(ONLINE.SIMD_CFLAGS) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,simd, \
    kit/DeltaSimd \
))

# vim: set noet sw=4 ts=4:
