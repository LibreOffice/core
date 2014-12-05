# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,jpeg-turbo))

$(eval $(call gb_UnpackedTarball_set_tarball,jpeg-turbo,$(JPEG_TURBO_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,jpeg-turbo,\
	external/jpeg-turbo/jpeg-turbo.build.patch.1 \
	$(if $(filter WNT,$(OS)),external/jpeg-turbo/jpeg-turbo.win_build.patch.1) \
	external/jpeg-turbo/0001-Patch-contributed-by-MulticoreWare-Inc.patch.1 \
	external/jpeg-turbo/0002-Patch-contributed-by-MulticoreWare-Inc.patch.1 \
	external/jpeg-turbo/0003-Patch-contributed-by-MulticoreWare-Inc.patch.1 \
	external/jpeg-turbo/0004-Patch-contributed-by-MulticoreWare-Inc.patch.1 \
	external/jpeg-turbo/0005-Oops.-These-should-have-been-included-with-the-previ.patch.1 \
	external/jpeg-turbo/0006-Temporarily-hack-the-OpenCL-code-so-that-it-works-co.patch.1 \
	external/jpeg-turbo/0007-Temporary-hack-to-enable-debug-output-when-the-envir.patch.1 \
	external/jpeg-turbo/0008-Do-not-enable-OpenCL-if-a-compilation-error-occurs.patch.1 \
	external/jpeg-turbo/0009-Introduce-jpeg_read_as_texture-Directly-reads-jpeg-a.patch.1 \
	external/jpeg-turbo/0010-opencl-Do-not-break-the-binary-compatibility.patch.1 \
	external/jpeg-turbo/0011-opencl-Use-clew-instead-of-own-wrappers-dlopen-etc.patch.1 \
	external/jpeg-turbo/0012-opencl-Use-functions-from-clew.patch.1 \
))

# vim: set noet sw=4 ts=4:
