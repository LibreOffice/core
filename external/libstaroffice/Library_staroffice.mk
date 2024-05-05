# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,staroffice))

$(eval $(call gb_Library_use_unpacked,staroffice,libstaroffice))

$(eval $(call gb_Library_use_externals,staroffice,\
    revenge \
))

$(eval $(call gb_Library_set_warnings_disabled,staroffice))

$(eval $(call gb_Library_set_precompiled_header,staroffice,external/libstaroffice/inc/pch/precompiled_staroffice))

$(eval $(call gb_Library_set_include,staroffice,\
    -I$(gb_UnpackedTarball_workdir)/libstaroffice/inc \
    -I$(gb_UnpackedTarball_workdir)/libstaroffice/src/lib \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,staroffice,\
	-DBUILD_STOFF \
	-D_WINDLL \
	-DNDEBUG \
))

$(eval $(call gb_Library_add_generated_exception_objects,staroffice,\
	UnpackedTarball/libstaroffice/src/lib/SDAParser \
	UnpackedTarball/libstaroffice/src/lib/SDCParser \
	UnpackedTarball/libstaroffice/src/lib/SDGParser \
	UnpackedTarball/libstaroffice/src/lib/SDWParser \
	UnpackedTarball/libstaroffice/src/lib/SDXParser \
	UnpackedTarball/libstaroffice/src/lib/STOFFCell \
	UnpackedTarball/libstaroffice/src/lib/STOFFCellStyle \
	UnpackedTarball/libstaroffice/src/lib/STOFFChart \
	UnpackedTarball/libstaroffice/src/lib/STOFFDebug \
	UnpackedTarball/libstaroffice/src/lib/STOFFDocument \
	UnpackedTarball/libstaroffice/src/lib/STOFFEntry \
	UnpackedTarball/libstaroffice/src/lib/STOFFFont \
	UnpackedTarball/libstaroffice/src/lib/STOFFFrameStyle \
	UnpackedTarball/libstaroffice/src/lib/STOFFGraphicDecoder \
	UnpackedTarball/libstaroffice/src/lib/STOFFGraphicEncoder \
	UnpackedTarball/libstaroffice/src/lib/STOFFGraphicListener \
	UnpackedTarball/libstaroffice/src/lib/STOFFGraphicShape \
	UnpackedTarball/libstaroffice/src/lib/STOFFGraphicStyle \
	UnpackedTarball/libstaroffice/src/lib/STOFFHeader \
	UnpackedTarball/libstaroffice/src/lib/STOFFInputStream \
	UnpackedTarball/libstaroffice/src/lib/STOFFList \
	UnpackedTarball/libstaroffice/src/lib/STOFFListener \
	UnpackedTarball/libstaroffice/src/lib/STOFFOLEParser \
	UnpackedTarball/libstaroffice/src/lib/STOFFPageSpan \
	UnpackedTarball/libstaroffice/src/lib/STOFFParagraph \
	UnpackedTarball/libstaroffice/src/lib/STOFFParser \
	UnpackedTarball/libstaroffice/src/lib/STOFFPosition \
	UnpackedTarball/libstaroffice/src/lib/STOFFPropertyHandler \
	UnpackedTarball/libstaroffice/src/lib/STOFFSection \
	UnpackedTarball/libstaroffice/src/lib/STOFFSpreadsheetDecoder \
	UnpackedTarball/libstaroffice/src/lib/STOFFSpreadsheetEncoder \
	UnpackedTarball/libstaroffice/src/lib/STOFFSpreadsheetListener \
	UnpackedTarball/libstaroffice/src/lib/STOFFStarMathToMMLConverter \
	UnpackedTarball/libstaroffice/src/lib/STOFFStringStream \
	UnpackedTarball/libstaroffice/src/lib/STOFFSubDocument \
	UnpackedTarball/libstaroffice/src/lib/STOFFTable \
	UnpackedTarball/libstaroffice/src/lib/STOFFTextListener \
	UnpackedTarball/libstaroffice/src/lib/SWFieldManager \
	UnpackedTarball/libstaroffice/src/lib/StarAttribute \
	UnpackedTarball/libstaroffice/src/lib/StarBitmap \
	UnpackedTarball/libstaroffice/src/lib/StarCellAttribute \
	UnpackedTarball/libstaroffice/src/lib/StarCellFormula \
	UnpackedTarball/libstaroffice/src/lib/StarCharAttribute \
	UnpackedTarball/libstaroffice/src/lib/StarEncoding \
	UnpackedTarball/libstaroffice/src/lib/StarEncodingChinese \
	UnpackedTarball/libstaroffice/src/lib/StarEncodingJapanese \
	UnpackedTarball/libstaroffice/src/lib/StarEncodingKorean \
	UnpackedTarball/libstaroffice/src/lib/StarEncodingOtherKorean \
	UnpackedTarball/libstaroffice/src/lib/StarEncodingTradChinese \
	UnpackedTarball/libstaroffice/src/lib/StarEncryption \
	UnpackedTarball/libstaroffice/src/lib/StarFileManager \
	UnpackedTarball/libstaroffice/src/lib/StarFormatManager \
	UnpackedTarball/libstaroffice/src/lib/StarFrameAttribute \
	UnpackedTarball/libstaroffice/src/lib/StarGraphicAttribute \
	UnpackedTarball/libstaroffice/src/lib/StarGraphicStruct \
	UnpackedTarball/libstaroffice/src/lib/StarItem \
	UnpackedTarball/libstaroffice/src/lib/StarItemPool \
	UnpackedTarball/libstaroffice/src/lib/StarLanguage \
	UnpackedTarball/libstaroffice/src/lib/StarLayout \
	UnpackedTarball/libstaroffice/src/lib/StarObject \
	UnpackedTarball/libstaroffice/src/lib/StarObjectChart \
	UnpackedTarball/libstaroffice/src/lib/StarObjectDraw \
	UnpackedTarball/libstaroffice/src/lib/StarObjectMath \
	UnpackedTarball/libstaroffice/src/lib/StarObjectModel \
	UnpackedTarball/libstaroffice/src/lib/StarObjectNumericRuler \
	UnpackedTarball/libstaroffice/src/lib/StarObjectPageStyle \
	UnpackedTarball/libstaroffice/src/lib/StarObjectSmallGraphic \
	UnpackedTarball/libstaroffice/src/lib/StarObjectSmallText \
	UnpackedTarball/libstaroffice/src/lib/StarObjectSpreadsheet \
	UnpackedTarball/libstaroffice/src/lib/StarObjectText \
	UnpackedTarball/libstaroffice/src/lib/StarPageAttribute \
	UnpackedTarball/libstaroffice/src/lib/StarParagraphAttribute \
	UnpackedTarball/libstaroffice/src/lib/StarState \
	UnpackedTarball/libstaroffice/src/lib/StarTable \
	UnpackedTarball/libstaroffice/src/lib/StarWriterStruct \
	UnpackedTarball/libstaroffice/src/lib/StarZone \
	UnpackedTarball/libstaroffice/src/lib/libstaroffice_internal \
))

# vim: set noet sw=4 ts=4:
