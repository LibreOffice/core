# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mwaw))

$(eval $(call gb_Library_use_unpacked,mwaw,libmwaw))

$(eval $(call gb_Library_use_externals,mwaw,\
    revenge \
))

$(eval $(call gb_Library_set_warnings_disabled,mwaw))

$(eval $(call gb_Library_set_precompiled_header,mwaw,external/libmwaw/inc/pch/precompiled_mwaw))

$(eval $(call gb_Library_set_include,mwaw,\
    -I$(call gb_UnpackedTarball_get_dir,libmwaw)/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,mwaw,\
	-DBUILD_MWAW \
	-DNDEBUG \
	-D_WINDLL \
))

$(eval $(call gb_Library_add_generated_exception_objects,mwaw,\
	UnpackedTarball/libmwaw/src/lib/ActaParser \
	UnpackedTarball/libmwaw/src/lib/ActaText \
	UnpackedTarball/libmwaw/src/lib/ApplePictParser \
	UnpackedTarball/libmwaw/src/lib/BeagleWksBMParser \
	UnpackedTarball/libmwaw/src/lib/BeagleWksDBParser \
	UnpackedTarball/libmwaw/src/lib/BeagleWksDRParser \
	UnpackedTarball/libmwaw/src/lib/BeagleWksParser \
	UnpackedTarball/libmwaw/src/lib/BeagleWksSSParser \
	UnpackedTarball/libmwaw/src/lib/BeagleWksStructManager \
	UnpackedTarball/libmwaw/src/lib/BeagleWksText \
	UnpackedTarball/libmwaw/src/lib/Canvas5BMParser \
	UnpackedTarball/libmwaw/src/lib/Canvas5Graph \
	UnpackedTarball/libmwaw/src/lib/Canvas5Image \
	UnpackedTarball/libmwaw/src/lib/Canvas5Parser \
	UnpackedTarball/libmwaw/src/lib/Canvas5Structure \
	UnpackedTarball/libmwaw/src/lib/Canvas5StyleManager \
	UnpackedTarball/libmwaw/src/lib/CanvasGraph \
	UnpackedTarball/libmwaw/src/lib/CanvasParser \
	UnpackedTarball/libmwaw/src/lib/CanvasStyleManager \
	UnpackedTarball/libmwaw/src/lib/ClarisDrawGraph \
	UnpackedTarball/libmwaw/src/lib/ClarisDrawParser \
	UnpackedTarball/libmwaw/src/lib/ClarisDrawStyleManager \
	UnpackedTarball/libmwaw/src/lib/ClarisDrawText \
	UnpackedTarball/libmwaw/src/lib/ClarisWksBMParser \
	UnpackedTarball/libmwaw/src/lib/ClarisWksDatabase \
	UnpackedTarball/libmwaw/src/lib/ClarisWksDbaseContent \
	UnpackedTarball/libmwaw/src/lib/ClarisWksDocument \
	UnpackedTarball/libmwaw/src/lib/ClarisWksGraph \
	UnpackedTarball/libmwaw/src/lib/ClarisWksPRParser \
	UnpackedTarball/libmwaw/src/lib/ClarisWksParser \
	UnpackedTarball/libmwaw/src/lib/ClarisWksPresentation \
	UnpackedTarball/libmwaw/src/lib/ClarisWksSSParser \
	UnpackedTarball/libmwaw/src/lib/ClarisWksSpreadsheet \
	UnpackedTarball/libmwaw/src/lib/ClarisWksStruct \
	UnpackedTarball/libmwaw/src/lib/ClarisWksStyleManager \
	UnpackedTarball/libmwaw/src/lib/ClarisWksTable \
	UnpackedTarball/libmwaw/src/lib/ClarisWksText \
	UnpackedTarball/libmwaw/src/lib/CorelPainterParser \
	UnpackedTarball/libmwaw/src/lib/CricketDrawParser \
	UnpackedTarball/libmwaw/src/lib/DocMkrParser \
	UnpackedTarball/libmwaw/src/lib/DocMkrText \
	UnpackedTarball/libmwaw/src/lib/DrawTableParser \
	UnpackedTarball/libmwaw/src/lib/EDocParser \
	UnpackedTarball/libmwaw/src/lib/FreeHandParser \
	UnpackedTarball/libmwaw/src/lib/FullWrtGraph \
	UnpackedTarball/libmwaw/src/lib/FullWrtParser \
	UnpackedTarball/libmwaw/src/lib/FullWrtStruct \
	UnpackedTarball/libmwaw/src/lib/FullWrtText \
	UnpackedTarball/libmwaw/src/lib/GreatWksBMParser \
	UnpackedTarball/libmwaw/src/lib/GreatWksDBParser \
	UnpackedTarball/libmwaw/src/lib/GreatWksDRParser \
	UnpackedTarball/libmwaw/src/lib/GreatWksDocument \
	UnpackedTarball/libmwaw/src/lib/GreatWksGraph \
	UnpackedTarball/libmwaw/src/lib/GreatWksParser \
	UnpackedTarball/libmwaw/src/lib/GreatWksSSParser \
	UnpackedTarball/libmwaw/src/lib/GreatWksText \
	UnpackedTarball/libmwaw/src/lib/HanMacWrdJGraph \
	UnpackedTarball/libmwaw/src/lib/HanMacWrdJParser \
	UnpackedTarball/libmwaw/src/lib/HanMacWrdJText \
	UnpackedTarball/libmwaw/src/lib/HanMacWrdKGraph \
	UnpackedTarball/libmwaw/src/lib/HanMacWrdKParser \
	UnpackedTarball/libmwaw/src/lib/HanMacWrdKText \
	UnpackedTarball/libmwaw/src/lib/JazzSSParser \
	UnpackedTarball/libmwaw/src/lib/JazzWriterParser \
	UnpackedTarball/libmwaw/src/lib/LightWayTxtGraph \
	UnpackedTarball/libmwaw/src/lib/LightWayTxtParser \
	UnpackedTarball/libmwaw/src/lib/LightWayTxtText \
	UnpackedTarball/libmwaw/src/lib/MWAWCell \
	UnpackedTarball/libmwaw/src/lib/MWAWChart \
	UnpackedTarball/libmwaw/src/lib/MWAWDebug \
	UnpackedTarball/libmwaw/src/lib/MWAWDocument \
	UnpackedTarball/libmwaw/src/lib/MWAWEntry \
	UnpackedTarball/libmwaw/src/lib/MWAWFont \
	UnpackedTarball/libmwaw/src/lib/MWAWFontConverter \
	UnpackedTarball/libmwaw/src/lib/MWAWFontSJISConverter \
	UnpackedTarball/libmwaw/src/lib/MWAWGraphicDecoder \
	UnpackedTarball/libmwaw/src/lib/MWAWGraphicEncoder \
	UnpackedTarball/libmwaw/src/lib/MWAWGraphicListener \
	UnpackedTarball/libmwaw/src/lib/MWAWGraphicShape \
	UnpackedTarball/libmwaw/src/lib/MWAWGraphicStyle \
	UnpackedTarball/libmwaw/src/lib/MWAWHeader \
	UnpackedTarball/libmwaw/src/lib/MWAWInputStream \
	UnpackedTarball/libmwaw/src/lib/MWAWList \
	UnpackedTarball/libmwaw/src/lib/MWAWListener \
	UnpackedTarball/libmwaw/src/lib/MWAWOLEParser \
	UnpackedTarball/libmwaw/src/lib/MWAWPageSpan \
	UnpackedTarball/libmwaw/src/lib/MWAWParagraph \
	UnpackedTarball/libmwaw/src/lib/MWAWParser \
	UnpackedTarball/libmwaw/src/lib/MWAWPict \
	UnpackedTarball/libmwaw/src/lib/MWAWPictBitmap \
	UnpackedTarball/libmwaw/src/lib/MWAWPictData \
	UnpackedTarball/libmwaw/src/lib/MWAWPictMac \
	UnpackedTarball/libmwaw/src/lib/MWAWPosition \
	UnpackedTarball/libmwaw/src/lib/MWAWPresentationListener \
	UnpackedTarball/libmwaw/src/lib/MWAWPrinter \
	UnpackedTarball/libmwaw/src/lib/MWAWPropertyHandler \
	UnpackedTarball/libmwaw/src/lib/MWAWRSRCParser \
	UnpackedTarball/libmwaw/src/lib/MWAWSection \
	UnpackedTarball/libmwaw/src/lib/MWAWSpreadsheetDecoder \
	UnpackedTarball/libmwaw/src/lib/MWAWSpreadsheetEncoder \
	UnpackedTarball/libmwaw/src/lib/MWAWSpreadsheetListener \
	UnpackedTarball/libmwaw/src/lib/MWAWStream \
	UnpackedTarball/libmwaw/src/lib/MWAWStringStream \
	UnpackedTarball/libmwaw/src/lib/MWAWSubDocument \
	UnpackedTarball/libmwaw/src/lib/MWAWTable \
	UnpackedTarball/libmwaw/src/lib/MWAWTextListener \
	UnpackedTarball/libmwaw/src/lib/MacDocParser \
	UnpackedTarball/libmwaw/src/lib/MacDraft5Parser \
	UnpackedTarball/libmwaw/src/lib/MacDraft5StyleManager \
	UnpackedTarball/libmwaw/src/lib/MacDraftParser \
	UnpackedTarball/libmwaw/src/lib/MacDrawParser \
	UnpackedTarball/libmwaw/src/lib/MacDrawProParser \
	UnpackedTarball/libmwaw/src/lib/MacDrawProStyleManager \
	UnpackedTarball/libmwaw/src/lib/MacPaintParser \
	UnpackedTarball/libmwaw/src/lib/MacWrtParser \
	UnpackedTarball/libmwaw/src/lib/MacWrtProParser \
	UnpackedTarball/libmwaw/src/lib/MacWrtProStructures \
	UnpackedTarball/libmwaw/src/lib/MaxWrtParser \
	UnpackedTarball/libmwaw/src/lib/MarinerWrtGraph \
	UnpackedTarball/libmwaw/src/lib/MarinerWrtParser \
	UnpackedTarball/libmwaw/src/lib/MarinerWrtText \
	UnpackedTarball/libmwaw/src/lib/MindWrtParser \
	UnpackedTarball/libmwaw/src/lib/MoreParser \
	UnpackedTarball/libmwaw/src/lib/MoreText \
	UnpackedTarball/libmwaw/src/lib/MouseWrtParser \
	UnpackedTarball/libmwaw/src/lib/MsWks3Text \
	UnpackedTarball/libmwaw/src/lib/MsWks4Text \
	UnpackedTarball/libmwaw/src/lib/MsWks4Zone \
	UnpackedTarball/libmwaw/src/lib/MsWksDBParser \
	UnpackedTarball/libmwaw/src/lib/MsWksDRParser \
	UnpackedTarball/libmwaw/src/lib/MsWksDocument \
	UnpackedTarball/libmwaw/src/lib/MsWksGraph \
	UnpackedTarball/libmwaw/src/lib/MsWksParser \
	UnpackedTarball/libmwaw/src/lib/MsWksSSParser \
	UnpackedTarball/libmwaw/src/lib/MsWksTable \
	UnpackedTarball/libmwaw/src/lib/MsWrd1Parser \
	UnpackedTarball/libmwaw/src/lib/MsWrdParser \
	UnpackedTarball/libmwaw/src/lib/MsWrdStruct \
	UnpackedTarball/libmwaw/src/lib/MsWrdText \
	UnpackedTarball/libmwaw/src/lib/MsWrdTextStyles \
	UnpackedTarball/libmwaw/src/lib/MultiplanParser \
	UnpackedTarball/libmwaw/src/lib/NisusWrtGraph \
	UnpackedTarball/libmwaw/src/lib/NisusWrtParser \
	UnpackedTarball/libmwaw/src/lib/NisusWrtStruct \
	UnpackedTarball/libmwaw/src/lib/NisusWrtText \
	UnpackedTarball/libmwaw/src/lib/PixelPaintParser \
	UnpackedTarball/libmwaw/src/lib/PowerPoint1Parser \
	UnpackedTarball/libmwaw/src/lib/PowerPoint3OLE \
	UnpackedTarball/libmwaw/src/lib/PowerPoint3Parser \
	UnpackedTarball/libmwaw/src/lib/PowerPoint7Graph \
	UnpackedTarball/libmwaw/src/lib/PowerPoint7Parser \
	UnpackedTarball/libmwaw/src/lib/PowerPoint7Struct \
	UnpackedTarball/libmwaw/src/lib/PowerPoint7Text \
	UnpackedTarball/libmwaw/src/lib/RagTime5Chart \
	UnpackedTarball/libmwaw/src/lib/RagTime5ClusterManager \
	UnpackedTarball/libmwaw/src/lib/RagTime5Document \
	UnpackedTarball/libmwaw/src/lib/RagTime5Formula \
	UnpackedTarball/libmwaw/src/lib/RagTime5Graph \
	UnpackedTarball/libmwaw/src/lib/RagTime5Layout \
	UnpackedTarball/libmwaw/src/lib/RagTime5Parser \
	UnpackedTarball/libmwaw/src/lib/RagTime5Pipeline \
	UnpackedTarball/libmwaw/src/lib/RagTime5SSParser \
	UnpackedTarball/libmwaw/src/lib/RagTime5Spreadsheet \
	UnpackedTarball/libmwaw/src/lib/RagTime5StructManager \
	UnpackedTarball/libmwaw/src/lib/RagTime5StyleManager \
	UnpackedTarball/libmwaw/src/lib/RagTime5Text \
	UnpackedTarball/libmwaw/src/lib/RagTimeParser \
	UnpackedTarball/libmwaw/src/lib/RagTimeSpreadsheet \
	UnpackedTarball/libmwaw/src/lib/RagTimeStruct \
	UnpackedTarball/libmwaw/src/lib/RagTimeText \
	UnpackedTarball/libmwaw/src/lib/ReadySetGoParser \
	UnpackedTarball/libmwaw/src/lib/ScoopParser \
	UnpackedTarball/libmwaw/src/lib/ScriptWriterParser \
	UnpackedTarball/libmwaw/src/lib/StudentWritingCParser \
	UnpackedTarball/libmwaw/src/lib/StyleParser \
	UnpackedTarball/libmwaw/src/lib/SuperPaintParser \
	UnpackedTarball/libmwaw/src/lib/TeachTxtParser \
	UnpackedTarball/libmwaw/src/lib/WingzGraph \
	UnpackedTarball/libmwaw/src/lib/WingzParser \
	UnpackedTarball/libmwaw/src/lib/WordMakerParser \
	UnpackedTarball/libmwaw/src/lib/WriteNowEntry \
	UnpackedTarball/libmwaw/src/lib/WriteNowParser \
	UnpackedTarball/libmwaw/src/lib/WriteNowText \
	UnpackedTarball/libmwaw/src/lib/WriterPlsParser \
	UnpackedTarball/libmwaw/src/lib/ZWrtParser \
	UnpackedTarball/libmwaw/src/lib/ZWrtText \
	UnpackedTarball/libmwaw/src/lib/libmwaw_internal \
))

# vim: set noet sw=4 ts=4:
