# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,pixman))

$(eval $(call gb_StaticLibrary_use_unpacked,pixman,cairo))

$(eval $(call gb_StaticLibrary_set_include,pixman,\
	-I$(gb_UnpackedTarball_workdir)/pixman \
	-I$(gb_UnpackedTarball_workdir)/pixman/pixman \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,pixman,\
	-DHAVE_CONFIG_H \
	-wd4100 \
	-wd4127 \
	-wd4132 \
	-wd4146 \
	-wd4189 \
	-wd4245 \
	-wd4267 \
	-wd4389 \
	-wd4456 \
	-wd4457 \
	-wd4701 \
	-wd5286 \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,pixman,\
	$(addprefix UnpackedTarball/pixman/pixman/, \
		pixman-access \
		pixman-access-accessors \
		pixman-arm-neon \
		pixman-arm-simd \
		pixman-arm \
		pixman-bits-image \
		pixman-combine-float \
		pixman-combine32 \
		pixman-conical-gradient \
		pixman-edge \
		pixman-edge-accessors \
		pixman-fast-path \
		pixman-filter \
		pixman-general \
		pixman-glyph \
		pixman-gradient-walker \
		pixman-image \
		pixman-implementation \
		pixman-linear-gradient \
		pixman-matrix \
		pixman-mips-dspr2 \
		pixman-mips \
		$(if $(filter-out AARCH64,$(CPUNAME)), \
			pixman-mmx \
		) \
		pixman-noop \
		pixman-ppc \
		pixman-radial-gradient \
		pixman-region16 \
		pixman-region32 \
		pixman-region64f \
		pixman-riscv \
		pixman-solid-fill \
		$(if $(filter-out AARCH64,$(CPUNAME)), \
			pixman-sse2 \
			pixman-ssse3 \
		) \
		pixman-timer \
		pixman-trap \
		pixman-utils \
		pixman-x86 \
		pixman \
	) \
))

# vim: set noet sw=4 ts=4:
