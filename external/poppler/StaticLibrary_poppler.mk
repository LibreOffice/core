# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,poppler))

$(eval $(call gb_StaticLibrary_use_unpacked,poppler,poppler))

$(eval $(call gb_StaticLibrary_use_external,poppler,libjpeg))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,poppler))

$(eval $(call gb_StaticLibrary_set_include,poppler,\
	-I$(WORKDIR)/UnpackedTarball/poppler \
	-I$(WORKDIR)/UnpackedTarball/poppler/fofi \
	-I$(WORKDIR)/UnpackedTarball/poppler/goo \
	-I$(WORKDIR)/UnpackedTarball/poppler/poppler \
	$$(INCLUDE) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_defs,poppler,\
	-DWIN32_LEAN_AND_MEAN \
	-D_CRT_SECURE_NO_WARNINGS \
))
$(eval $(call gb_StaticLibrary_add_cxxflags,poppler,\
	/Zc:wchar_t- \
))
endif

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,poppler,cc))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,poppler,\
	UnpackedTarball/poppler/goo/gfile \
	UnpackedTarball/poppler/goo/GooTimer \
	UnpackedTarball/poppler/goo/GooString \
	UnpackedTarball/poppler/goo/FixedPoint \
	UnpackedTarball/poppler/goo/NetPBMWriter \
	UnpackedTarball/poppler/goo/PNGWriter \
	UnpackedTarball/poppler/goo/TiffWriter \
	UnpackedTarball/poppler/goo/JpegWriter \
	UnpackedTarball/poppler/goo/ImgWriter \
	UnpackedTarball/poppler/goo/gstrtod \
	UnpackedTarball/poppler/goo/grandom \
	UnpackedTarball/poppler/goo/glibc \
	UnpackedTarball/poppler/goo/glibc_strtok_r \
	UnpackedTarball/poppler/fofi/FoFiBase \
	UnpackedTarball/poppler/fofi/FoFiEncodings \
	UnpackedTarball/poppler/fofi/FoFiTrueType \
	UnpackedTarball/poppler/fofi/FoFiType1 \
	UnpackedTarball/poppler/fofi/FoFiType1C \
	UnpackedTarball/poppler/fofi/FoFiIdentifier \
	UnpackedTarball/poppler/poppler/Annot \
	UnpackedTarball/poppler/poppler/Array \
	UnpackedTarball/poppler/poppler/BuiltinFont \
	UnpackedTarball/poppler/poppler/BuiltinFontTables \
	UnpackedTarball/poppler/poppler/CachedFile \
	UnpackedTarball/poppler/poppler/Catalog \
	UnpackedTarball/poppler/poppler/CertificateInfo \
	UnpackedTarball/poppler/poppler/CharCodeToUnicode \
	UnpackedTarball/poppler/poppler/CMap \
	UnpackedTarball/poppler/poppler/DateInfo \
	UnpackedTarball/poppler/poppler/Decrypt \
	UnpackedTarball/poppler/poppler/Dict \
	UnpackedTarball/poppler/poppler/Error \
	UnpackedTarball/poppler/poppler/FileSpec \
	UnpackedTarball/poppler/poppler/FontEncodingTables \
	UnpackedTarball/poppler/poppler/Form \
	UnpackedTarball/poppler/poppler/FontInfo \
	UnpackedTarball/poppler/poppler/Function \
	UnpackedTarball/poppler/poppler/Gfx \
	UnpackedTarball/poppler/poppler/GfxFont \
	UnpackedTarball/poppler/poppler/GfxState \
	UnpackedTarball/poppler/poppler/GlobalParams \
	UnpackedTarball/poppler/poppler/Hints \
	UnpackedTarball/poppler/poppler/JArithmeticDecoder \
	UnpackedTarball/poppler/poppler/JBIG2Stream \
	UnpackedTarball/poppler/poppler/Lexer \
	UnpackedTarball/poppler/poppler/Link \
	UnpackedTarball/poppler/poppler/Linearization \
	UnpackedTarball/poppler/poppler/LocalPDFDocBuilder \
	UnpackedTarball/poppler/poppler/MarkedContentOutputDev \
	UnpackedTarball/poppler/poppler/NameToCharCode \
	UnpackedTarball/poppler/poppler/Object \
	UnpackedTarball/poppler/poppler/OptionalContent \
	UnpackedTarball/poppler/poppler/Outline \
	UnpackedTarball/poppler/poppler/OutputDev \
	UnpackedTarball/poppler/poppler/Page \
	UnpackedTarball/poppler/poppler/PageTransition \
	UnpackedTarball/poppler/poppler/Parser \
	UnpackedTarball/poppler/poppler/PDFDoc \
	UnpackedTarball/poppler/poppler/PDFDocEncoding \
	UnpackedTarball/poppler/poppler/PDFDocFactory \
	UnpackedTarball/poppler/poppler/ProfileData \
	UnpackedTarball/poppler/poppler/PreScanOutputDev \
	UnpackedTarball/poppler/poppler/PSTokenizer \
	UnpackedTarball/poppler/poppler/SignatureInfo \
	UnpackedTarball/poppler/poppler/Stream \
	UnpackedTarball/poppler/poppler/StructTreeRoot \
	UnpackedTarball/poppler/poppler/StructElement \
	UnpackedTarball/poppler/poppler/UnicodeMap \
	UnpackedTarball/poppler/poppler/UnicodeMapFuncs \
	UnpackedTarball/poppler/poppler/UnicodeTypeTable \
	UnpackedTarball/poppler/poppler/UTF \
	UnpackedTarball/poppler/poppler/XRef \
	UnpackedTarball/poppler/poppler/PSOutputDev \
	UnpackedTarball/poppler/poppler/TextOutputDev \
	UnpackedTarball/poppler/poppler/PageLabelInfo \
	UnpackedTarball/poppler/poppler/SecurityHandler \
	UnpackedTarball/poppler/poppler/StdinCachedFile \
	UnpackedTarball/poppler/poppler/StdinPDFDocBuilder \
	UnpackedTarball/poppler/poppler/Sound \
	UnpackedTarball/poppler/poppler/ViewerPreferences \
	UnpackedTarball/poppler/poppler/Movie \
	UnpackedTarball/poppler/poppler/Rendition \
	UnpackedTarball/poppler/poppler/DCTStream \
))

# vim: set noet sw=4 ts=4:
