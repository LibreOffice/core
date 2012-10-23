# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,freetype,freetype))

$(eval $(call gb_ExternalPackage_use_external_project,freetype,freetype))

$(eval $(call gb_ExternalPackage_add_unpacked_files,freetype,inc/external,\
	include/ft2build.h \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,freetype,inc/external/freetype,\
	include/freetype/freetype.h \
	include/freetype/ftadvanc.h \
	include/freetype/ftbbox.h \
	include/freetype/ftbdf.h \
	include/freetype/ftbitmap.h \
	include/freetype/ftbzip2.h \
	include/freetype/ftcache.h \
	include/freetype/ftchapters.h \
	include/freetype/ftcid.h \
	include/freetype/fterrdef.h \
	include/freetype/fterrors.h \
	include/freetype/ftgasp.h \
	include/freetype/ftglyph.h \
	include/freetype/ftgxval.h \
	include/freetype/ftgzip.h \
	include/freetype/ftimage.h \
	include/freetype/ftincrem.h \
	include/freetype/ftlcdfil.h \
	include/freetype/ftlist.h \
	include/freetype/ftlzw.h \
	include/freetype/ftmac.h \
	include/freetype/ftmm.h \
	include/freetype/ftmodapi.h \
	include/freetype/ftmoderr.h \
	include/freetype/ftotval.h \
	include/freetype/ftoutln.h \
	include/freetype/ftpfr.h \
	include/freetype/ftrender.h \
	include/freetype/ftsizes.h \
	include/freetype/ftsnames.h \
	include/freetype/ftstroke.h \
	include/freetype/ftsynth.h \
	include/freetype/ftsystem.h \
	include/freetype/fttrigon.h \
	include/freetype/fttypes.h \
	include/freetype/ftwinfnt.h \
	include/freetype/ftxf86.h \
	include/freetype/t1tables.h \
	include/freetype/ttnameid.h \
	include/freetype/tttables.h \
	include/freetype/tttags.h \
	include/freetype/ttunpat.h \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,freetype,inc/external/freetype/config,\
	include/freetype/config/ftconfig.h \
	include/freetype/config/ftheader.h \
	include/freetype/config/ftmodule.h \
	include/freetype/config/ftoption.h \
	include/freetype/config/ftstdlib.h \
))

$(eval $(call gb_ExternalPackage_add_files,freetype,lib,\
	objs/.libs/libfreetype.a \
))

$(eval $(call gb_ExternalPackage_add_files,freetype,bin,\
	builds/unix/freetype-config \
))

# vim: set noet sw=4 ts=4:
