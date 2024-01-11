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

$(eval $(call gb_StaticLibrary_set_precompiled_header,poppler,external/poppler/inc/pch/precompiled_poppler))

$(eval $(call gb_StaticLibrary_use_externals,poppler,\
	libjpeg \
	zlib \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,poppler))

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

$(eval $(call gb_StaticLibrary_add_generated_cobjects,poppler,\
	UnpackedTarball/poppler/poppler/CourierWidths.pregenerated \
	UnpackedTarball/poppler/poppler/CourierBoldWidths.pregenerated \
	UnpackedTarball/poppler/poppler/CourierBoldObliqueWidths.pregenerated \
	UnpackedTarball/poppler/poppler/CourierObliqueWidths.pregenerated \
	UnpackedTarball/poppler/poppler/HelveticaWidths.pregenerated \
	UnpackedTarball/poppler/poppler/HelveticaBoldWidths.pregenerated \
	UnpackedTarball/poppler/poppler/HelveticaBoldObliqueWidths.pregenerated \
	UnpackedTarball/poppler/poppler/HelveticaObliqueWidths.pregenerated \
	UnpackedTarball/poppler/poppler/SymbolWidths.pregenerated \
	UnpackedTarball/poppler/poppler/TimesBoldWidths.pregenerated \
	UnpackedTarball/poppler/poppler/TimesBoldItalicWidths.pregenerated \
	UnpackedTarball/poppler/poppler/TimesItalicWidths.pregenerated \
	UnpackedTarball/poppler/poppler/TimesRomanWidths.pregenerated \
	UnpackedTarball/poppler/poppler/ZapfDingbatsWidths.pregenerated \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,poppler,\
	UnpackedTarball/poppler/goo/gbase64 \
	UnpackedTarball/poppler/goo/gbasename \
	UnpackedTarball/poppler/goo/gfile \
	UnpackedTarball/poppler/goo/glibc \
	UnpackedTarball/poppler/goo/glibc_strtok_r \
	UnpackedTarball/poppler/goo/grandom \
	UnpackedTarball/poppler/goo/gstrtod \
	UnpackedTarball/poppler/goo/GooString \
	UnpackedTarball/poppler/goo/GooTimer \
	UnpackedTarball/poppler/goo/ImgWriter \
	UnpackedTarball/poppler/goo/JpegWriter \
	UnpackedTarball/poppler/goo/NetPBMWriter \
	UnpackedTarball/poppler/goo/PNGWriter \
	UnpackedTarball/poppler/goo/TiffWriter \
	UnpackedTarball/poppler/fofi/FoFiBase \
	UnpackedTarball/poppler/fofi/FoFiEncodings \
	UnpackedTarball/poppler/fofi/FoFiIdentifier \
	UnpackedTarball/poppler/fofi/FoFiTrueType \
	UnpackedTarball/poppler/fofi/FoFiType1 \
	UnpackedTarball/poppler/fofi/FoFiType1C \
	UnpackedTarball/poppler/poppler/Annot \
	UnpackedTarball/poppler/poppler/AnnotStampImageHelper \
	UnpackedTarball/poppler/poppler/Array \
	UnpackedTarball/poppler/poppler/BBoxOutputDev \
	UnpackedTarball/poppler/poppler/CachedFile \
	UnpackedTarball/poppler/poppler/Catalog \
	UnpackedTarball/poppler/poppler/CertificateInfo \
	UnpackedTarball/poppler/poppler/CharCodeToUnicode \
	UnpackedTarball/poppler/poppler/CMap \
	UnpackedTarball/poppler/poppler/DCTStream \
	UnpackedTarball/poppler/poppler/DateInfo \
	UnpackedTarball/poppler/poppler/Decrypt \
	UnpackedTarball/poppler/poppler/Dict \
	UnpackedTarball/poppler/poppler/Error \
	UnpackedTarball/poppler/poppler/FDPDFDocBuilder \
	UnpackedTarball/poppler/poppler/FILECacheLoader \
	UnpackedTarball/poppler/poppler/FileSpec \
	UnpackedTarball/poppler/poppler/FlateEncoder \
	UnpackedTarball/poppler/poppler/FontEncodingTables \
	UnpackedTarball/poppler/poppler/FontInfo \
	UnpackedTarball/poppler/poppler/Form \
	UnpackedTarball/poppler/poppler/Function \
	UnpackedTarball/poppler/poppler/Gfx \
	UnpackedTarball/poppler/poppler/GfxFont \
	UnpackedTarball/poppler/poppler/GfxState \
	UnpackedTarball/poppler/poppler/GlobalParams \
	UnpackedTarball/poppler/poppler/Hints \
	UnpackedTarball/poppler/poppler/ImageEmbeddingUtils \
	UnpackedTarball/poppler/poppler/JArithmeticDecoder \
	UnpackedTarball/poppler/poppler/JBIG2Stream \
	UnpackedTarball/poppler/poppler/JSInfo \
	UnpackedTarball/poppler/poppler/Lexer \
	UnpackedTarball/poppler/poppler/Linearization \
	UnpackedTarball/poppler/poppler/Link \
	UnpackedTarball/poppler/poppler/LocalPDFDocBuilder \
	UnpackedTarball/poppler/poppler/MarkedContentOutputDev \
	UnpackedTarball/poppler/poppler/Movie \
	UnpackedTarball/poppler/poppler/NameToCharCode \
	UnpackedTarball/poppler/poppler/Object \
	UnpackedTarball/poppler/poppler/OptionalContent \
	UnpackedTarball/poppler/poppler/Outline \
	UnpackedTarball/poppler/poppler/OutputDev \
	UnpackedTarball/poppler/poppler/Page \
	UnpackedTarball/poppler/poppler/PageLabelInfo \
	UnpackedTarball/poppler/poppler/PageTransition \
	UnpackedTarball/poppler/poppler/Parser \
	UnpackedTarball/poppler/poppler/PDFDoc \
	UnpackedTarball/poppler/poppler/PDFDocBuilder \
	UnpackedTarball/poppler/poppler/PDFDocEncoding \
	UnpackedTarball/poppler/poppler/PDFDocFactory \
	UnpackedTarball/poppler/poppler/PreScanOutputDev \
	UnpackedTarball/poppler/poppler/ProfileData \
	UnpackedTarball/poppler/poppler/PSOutputDev \
	UnpackedTarball/poppler/poppler/PSTokenizer \
	UnpackedTarball/poppler/poppler/Rendition \
	UnpackedTarball/poppler/poppler/SecurityHandler \
	UnpackedTarball/poppler/poppler/SignatureInfo \
	UnpackedTarball/poppler/poppler/Sound \
	UnpackedTarball/poppler/poppler/SplashOutputDev \
	UnpackedTarball/poppler/poppler/Stream \
	UnpackedTarball/poppler/poppler/StructElement \
	UnpackedTarball/poppler/poppler/StructTreeRoot \
	UnpackedTarball/poppler/poppler/TextOutputDev \
	UnpackedTarball/poppler/poppler/UTF \
	UnpackedTarball/poppler/poppler/UnicodeMap \
	UnpackedTarball/poppler/poppler/UnicodeMapFuncs \
	UnpackedTarball/poppler/poppler/UnicodeTypeTable \
	UnpackedTarball/poppler/poppler/ViewerPreferences \
	UnpackedTarball/poppler/poppler/XRef \
	UnpackedTarball/poppler/splash/Splash \
	UnpackedTarball/poppler/splash/SplashBitmap \
	UnpackedTarball/poppler/splash/SplashClip \
	UnpackedTarball/poppler/splash/SplashFont \
	UnpackedTarball/poppler/splash/SplashFontEngine \
	UnpackedTarball/poppler/splash/SplashFontFile \
	UnpackedTarball/poppler/splash/SplashFontFileID \
	UnpackedTarball/poppler/splash/SplashPath \
	UnpackedTarball/poppler/splash/SplashPattern \
	UnpackedTarball/poppler/splash/SplashScreen \
	UnpackedTarball/poppler/splash/SplashState \
	UnpackedTarball/poppler/splash/SplashXPath \
	UnpackedTarball/poppler/splash/SplashXPathScanner \
))

# vim: set noet sw=4 ts=4:
