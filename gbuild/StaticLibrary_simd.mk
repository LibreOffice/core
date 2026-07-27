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

$(eval $(call gb_StaticLibrary_set_generated_c_base,simd,$(SRCDIR)/..))

$(eval $(call gb_StaticLibrary_set_include,simd, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(SRCDIR)/.. \
    $$(INCLUDE) \
))

# The delta code is hand-vectorised for AVX2:
$(eval $(call gb_StaticLibrary_add_cflags,simd, \
    -mavx2 \
    -O3 \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,simd, \
    kit/DeltaSimd \
))

# vim: set noet sw=4 ts=4:
