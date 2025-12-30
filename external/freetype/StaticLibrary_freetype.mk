# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This is de facto just for Windows for now

$(eval $(call gb_StaticLibrary_StaticLibrary,freetype))

$(eval $(call gb_StaticLibrary_use_unpacked,freetype,freetype))

$(eval $(call gb_StaticLibrary_set_include,freetype,\
	-I$(gb_UnpackedTarball_workdir)/freetype/include \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,freetype,\
	-wd4005 \
	-wd4267 \
	-DDLG_STATIC \
	-DZ_PREFIX \
	-DFT2_BUILD_LIBRARY \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,freetype,\
	$(addprefix UnpackedTarball/freetype/, \
		builds/windows/ftdebug \
		builds/windows/ftsystem \
		src/autofit/autofit \
		src/base/ftbase \
		src/base/ftbbox \
		src/base/ftbdf \
		src/base/ftbitmap \
		src/base/ftcid \
		src/base/ftfstype \
		src/base/ftgasp \
		src/base/ftglyph \
		src/base/ftgxval \
		src/base/ftinit \
		src/base/ftmm \
		src/base/ftotval \
		src/base/ftpatent \
		src/base/ftpfr \
		src/base/ftstroke \
		src/base/ftsynth \
		src/base/fttype1 \
		src/base/ftwinfnt \
		src/bdf/bdf \
		src/cache/ftcache \
		src/cff/cff \
		src/cid/type1cid \
		src/dlg/dlg \
		src/gzip/ftgzip \
		src/lzw/ftlzw \
		src/pcf/pcf \
		src/pfr/pfr \
		src/psaux/psaux \
		src/pshinter/pshinter \
		src/psnames/psmodule \
		src/raster/raster \
		src/sdf/sdf \
		src/sfnt/sfnt \
		src/smooth/smooth \
		src/svg/svg \
		src/truetype/truetype \
		src/type1/type1 \
		src/type42/type42 \
		src/winfonts/winfnt \
	) \
))

# vim: set noet sw=4 ts=4:
