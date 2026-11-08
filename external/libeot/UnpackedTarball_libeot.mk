# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

libeot_patches :=
libeot_patches += 0001-remove-unused-err.h.patch
libeot_patches += debug-no-werror.patch.0
libeot_patches += ubsan.patch.0
# upstreamed as: https://github.com/umanwizard/libeot/pull/14
libeot_patches += 0001-const-up-tripletEncodings.patch
# upstreamed as: https://github.com/umanwizard/libeot/pull/20
libeot_patches += 0001-compare-a-stream-copy-s-length-with-the-room-left-no.patch
# backported from upstream master e166d46
libeot_patches += 0002-Get-rid-of-subtle-UB.patch
# upstreamed as: https://github.com/umanwizard/libeot/pull/20
libeot_patches += 0003-grow-the-decoder-s-output-buffer-by-at-least-one-byt.patch
libeot_patches += 0004-Resolves-3-check-that-a-copy-item-fits-in-the-decode.patch
# backported from upstream master d0bd08f
libeot_patches += 0005-fix-another-use-of-uninitialized-value.patch
# upstreamed as: https://github.com/umanwizard/libeot/pull/20
libeot_patches += 0006-set-the-jmp_buf-that-the-decoder-s-error-paths-longj.patch
libeot_patches += 0007-check-for-the-end-of-the-input-before-reading-the-ne.patch
libeot_patches += 0008-check-the-MTX-block-offsets-are-in-order-before-usin.patch
libeot_patches += 0009-check-that-a-copy-item-s-source-starts-within-the-de.patch
libeot_patches += 0010-stop-DecodeLength-once-the-length-passes-out_len.patch
libeot_patches += 0011-check-the-results-of-the-MTX_mem-allocations.patch

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libeot))

$(eval $(call gb_UnpackedTarball_set_tarball,libeot,$(LIBEOT_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libeot,1))

$(eval $(call gb_UnpackedTarball_add_patches,libeot,\
	$(foreach patch,$(libeot_patches),external/libeot/$(patch)) \
))

# vim: set noet sw=4 ts=4:
