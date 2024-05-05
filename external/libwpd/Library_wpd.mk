# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,wpd))

$(eval $(call gb_Library_use_unpacked,wpd,libwpd))

$(eval $(call gb_Library_use_externals,wpd,\
	boost_headers \
	revenge \
))

$(eval $(call gb_Library_set_warnings_disabled,wpd))

$(eval $(call gb_Library_set_include,wpd,\
    -I$(gb_UnpackedTarball_workdir)/libwpd/inc \
    -I$(gb_UnpackedTarball_workdir)/libwpd/src/lib \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,wpd,external/libwpd/inc/pch/precompiled_wpd))

$(eval $(call gb_Library_add_defs,wpd,\
	-DBOOST_ALL_NO_LIB \
	-DDLL_EXPORT \
	-DLIBWPD_BUILD \
	-DNDEBUG \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,wpd,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,wpd,\
	UnpackedTarball/libwpd/src/lib/WP1BottomMarginGroup \
	UnpackedTarball/libwpd/src/lib/WP1CenterTextGroup \
	UnpackedTarball/libwpd/src/lib/WP1ContentListener \
	UnpackedTarball/libwpd/src/lib/WP1ExtendedCharacterGroup \
	UnpackedTarball/libwpd/src/lib/WP1FileStructure \
	UnpackedTarball/libwpd/src/lib/WP1FixedLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP1FlushRightGroup \
	UnpackedTarball/libwpd/src/lib/WP1FontIdGroup \
	UnpackedTarball/libwpd/src/lib/WP1FootnoteEndnoteGroup \
	UnpackedTarball/libwpd/src/lib/WP1HeaderFooterGroup \
	UnpackedTarball/libwpd/src/lib/WP1Heuristics \
	UnpackedTarball/libwpd/src/lib/WP1JustificationGroup \
	UnpackedTarball/libwpd/src/lib/WP1LeftIndentGroup \
	UnpackedTarball/libwpd/src/lib/WP1LeftRightIndentGroup \
	UnpackedTarball/libwpd/src/lib/WP1Listener \
	UnpackedTarball/libwpd/src/lib/WP1MarginReleaseGroup \
	UnpackedTarball/libwpd/src/lib/WP1MarginResetGroup \
	UnpackedTarball/libwpd/src/lib/WP1Parser \
	UnpackedTarball/libwpd/src/lib/WP1Part \
	UnpackedTarball/libwpd/src/lib/WP1PictureGroup \
	UnpackedTarball/libwpd/src/lib/WP1PointSizeGroup \
	UnpackedTarball/libwpd/src/lib/WP1SetTabsGroup \
	UnpackedTarball/libwpd/src/lib/WP1SpacingResetGroup \
	UnpackedTarball/libwpd/src/lib/WP1StylesListener \
	UnpackedTarball/libwpd/src/lib/WP1SubDocument \
	UnpackedTarball/libwpd/src/lib/WP1SuppressPageCharacteristicsGroup \
	UnpackedTarball/libwpd/src/lib/WP1TopMarginGroup \
	UnpackedTarball/libwpd/src/lib/WP1UnsupportedFixedLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP1UnsupportedVariableLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP1VariableLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP3AttributeGroup \
	UnpackedTarball/libwpd/src/lib/WP3ContentListener \
	UnpackedTarball/libwpd/src/lib/WP3DefinitionGroup \
	UnpackedTarball/libwpd/src/lib/WP3DisplayGroup \
	UnpackedTarball/libwpd/src/lib/WP3DoubleByteScriptCharacterGroup \
	UnpackedTarball/libwpd/src/lib/WP3EndOfLinePageGroup \
	UnpackedTarball/libwpd/src/lib/WP3ExtendedCharacterGroup \
	UnpackedTarball/libwpd/src/lib/WP3FileStructure \
	UnpackedTarball/libwpd/src/lib/WP3FixedLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP3FontGroup \
	UnpackedTarball/libwpd/src/lib/WP3FootnoteEndnoteGroup \
	UnpackedTarball/libwpd/src/lib/WP3Header \
	UnpackedTarball/libwpd/src/lib/WP3HeaderFooterGroup \
	UnpackedTarball/libwpd/src/lib/WP3IndentGroup \
	UnpackedTarball/libwpd/src/lib/WP3Listener \
	UnpackedTarball/libwpd/src/lib/WP3MiscellaneousGroup \
	UnpackedTarball/libwpd/src/lib/WP3PageFormatGroup \
	UnpackedTarball/libwpd/src/lib/WP3Parser \
	UnpackedTarball/libwpd/src/lib/WP3Part \
	UnpackedTarball/libwpd/src/lib/WP3Resource \
	UnpackedTarball/libwpd/src/lib/WP3ResourceFork \
	UnpackedTarball/libwpd/src/lib/WP3SingleByteFunction \
	UnpackedTarball/libwpd/src/lib/WP3StylesListener \
	UnpackedTarball/libwpd/src/lib/WP3SubDocument \
	UnpackedTarball/libwpd/src/lib/WP3TabGroup \
	UnpackedTarball/libwpd/src/lib/WP3TablesGroup \
	UnpackedTarball/libwpd/src/lib/WP3UndoGroup \
	UnpackedTarball/libwpd/src/lib/WP3UnsupportedFixedLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP3UnsupportedVariableLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP3VariableLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP3WindowGroup \
	UnpackedTarball/libwpd/src/lib/WP42ContentListener \
	UnpackedTarball/libwpd/src/lib/WP42DefineColumnsGroup \
	UnpackedTarball/libwpd/src/lib/WP42ExtendedCharacterGroup \
	UnpackedTarball/libwpd/src/lib/WP42FileStructure \
	UnpackedTarball/libwpd/src/lib/WP42HeaderFooterGroup \
	UnpackedTarball/libwpd/src/lib/WP42Heuristics \
	UnpackedTarball/libwpd/src/lib/WP42Listener \
	UnpackedTarball/libwpd/src/lib/WP42MarginResetGroup \
	UnpackedTarball/libwpd/src/lib/WP42MultiByteFunctionGroup \
	UnpackedTarball/libwpd/src/lib/WP42Parser \
	UnpackedTarball/libwpd/src/lib/WP42Part \
	UnpackedTarball/libwpd/src/lib/WP42StylesListener \
	UnpackedTarball/libwpd/src/lib/WP42SubDocument \
	UnpackedTarball/libwpd/src/lib/WP42SuppressPageCharacteristicsGroup \
	UnpackedTarball/libwpd/src/lib/WP42UnsupportedMultiByteFunctionGroup \
	UnpackedTarball/libwpd/src/lib/WP5AttributeGroup \
	UnpackedTarball/libwpd/src/lib/WP5BoxGroup \
	UnpackedTarball/libwpd/src/lib/WP5ContentListener \
	UnpackedTarball/libwpd/src/lib/WP5DefinitionGroup \
	UnpackedTarball/libwpd/src/lib/WP5ExtendedCharacterGroup \
	UnpackedTarball/libwpd/src/lib/WP5FileStructure \
	UnpackedTarball/libwpd/src/lib/WP5FixedLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP5FontGroup \
	UnpackedTarball/libwpd/src/lib/WP5FontNameStringPoolPacket \
	UnpackedTarball/libwpd/src/lib/WP5FootnoteEndnoteGroup \
	UnpackedTarball/libwpd/src/lib/WP5GeneralPacketData \
	UnpackedTarball/libwpd/src/lib/WP5GeneralPacketIndex \
	UnpackedTarball/libwpd/src/lib/WP5GraphicsInformationPacket \
	UnpackedTarball/libwpd/src/lib/WP5Header \
	UnpackedTarball/libwpd/src/lib/WP5HeaderFooterGroup \
	UnpackedTarball/libwpd/src/lib/WP5IndentGroup \
	UnpackedTarball/libwpd/src/lib/WP5ListFontsUsedPacket \
	UnpackedTarball/libwpd/src/lib/WP5Listener \
	UnpackedTarball/libwpd/src/lib/WP5PageFormatGroup \
	UnpackedTarball/libwpd/src/lib/WP5Parser \
	UnpackedTarball/libwpd/src/lib/WP5Part \
	UnpackedTarball/libwpd/src/lib/WP5PrefixData \
	UnpackedTarball/libwpd/src/lib/WP5SingleByteFunction \
	UnpackedTarball/libwpd/src/lib/WP5SpecialHeaderIndex \
	UnpackedTarball/libwpd/src/lib/WP5StylesListener \
	UnpackedTarball/libwpd/src/lib/WP5SubDocument \
	UnpackedTarball/libwpd/src/lib/WP5TabGroup \
	UnpackedTarball/libwpd/src/lib/WP5TableEOLGroup \
	UnpackedTarball/libwpd/src/lib/WP5TableEOPGroup \
	UnpackedTarball/libwpd/src/lib/WP5UnsupportedFixedLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP5UnsupportedVariableLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP5VariableLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP60Header \
	UnpackedTarball/libwpd/src/lib/WP61Header \
	UnpackedTarball/libwpd/src/lib/WP6AttributeGroup \
	UnpackedTarball/libwpd/src/lib/WP6BoxGroup \
	UnpackedTarball/libwpd/src/lib/WP6CharacterGroup \
	UnpackedTarball/libwpd/src/lib/WP6ColumnGroup \
	UnpackedTarball/libwpd/src/lib/WP6CommentAnnotationPacket \
	UnpackedTarball/libwpd/src/lib/WP6ContentListener \
	UnpackedTarball/libwpd/src/lib/WP6DefaultInitialFontPacket \
	UnpackedTarball/libwpd/src/lib/WP6DisplayNumberReferenceGroup \
	UnpackedTarball/libwpd/src/lib/WP6EOLGroup \
	UnpackedTarball/libwpd/src/lib/WP6ExtendedCharacterGroup \
	UnpackedTarball/libwpd/src/lib/WP6ExtendedDocumentSummaryPacket \
	UnpackedTarball/libwpd/src/lib/WP6FileStructure \
	UnpackedTarball/libwpd/src/lib/WP6FillStylePacket \
	UnpackedTarball/libwpd/src/lib/WP6FixedLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP6FontDescriptorPacket \
	UnpackedTarball/libwpd/src/lib/WP6FootnoteEndnoteGroup \
	UnpackedTarball/libwpd/src/lib/WP6GeneralTextPacket \
	UnpackedTarball/libwpd/src/lib/WP6GraphicsBoxStylePacket \
	UnpackedTarball/libwpd/src/lib/WP6GraphicsCachedFileDataPacket \
	UnpackedTarball/libwpd/src/lib/WP6GraphicsFilenamePacket \
	UnpackedTarball/libwpd/src/lib/WP6Header \
	UnpackedTarball/libwpd/src/lib/WP6HeaderFooterGroup \
	UnpackedTarball/libwpd/src/lib/WP6HighlightGroup \
	UnpackedTarball/libwpd/src/lib/WP6HyperlinkPacket \
	UnpackedTarball/libwpd/src/lib/WP6Listener \
	UnpackedTarball/libwpd/src/lib/WP6NumberingMethodGroup \
	UnpackedTarball/libwpd/src/lib/WP6OutlineStylePacket \
	UnpackedTarball/libwpd/src/lib/WP6PageGroup \
	UnpackedTarball/libwpd/src/lib/WP6ParagraphGroup \
	UnpackedTarball/libwpd/src/lib/WP6Parser \
	UnpackedTarball/libwpd/src/lib/WP6Part \
	UnpackedTarball/libwpd/src/lib/WP6PrefixData \
	UnpackedTarball/libwpd/src/lib/WP6PrefixDataPacket \
	UnpackedTarball/libwpd/src/lib/WP6PrefixIndice \
	UnpackedTarball/libwpd/src/lib/WP6SetNumberGroup \
	UnpackedTarball/libwpd/src/lib/WP6SingleByteFunction \
	UnpackedTarball/libwpd/src/lib/WP6StyleGroup \
	UnpackedTarball/libwpd/src/lib/WP6StylesListener \
	UnpackedTarball/libwpd/src/lib/WP6SubDocument \
	UnpackedTarball/libwpd/src/lib/WP6TabGroup \
	UnpackedTarball/libwpd/src/lib/WP6TableStylePacket \
	UnpackedTarball/libwpd/src/lib/WP6UndoGroup \
	UnpackedTarball/libwpd/src/lib/WP6UnsupportedFixedLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP6UnsupportedVariableLengthGroup \
	UnpackedTarball/libwpd/src/lib/WP6VariableLengthGroup \
	UnpackedTarball/libwpd/src/lib/WPDocument \
	UnpackedTarball/libwpd/src/lib/WPXContentListener \
	UnpackedTarball/libwpd/src/lib/WPXEncryption \
	UnpackedTarball/libwpd/src/lib/WPXHeader \
	UnpackedTarball/libwpd/src/lib/WPXHeaderFooter \
	UnpackedTarball/libwpd/src/lib/WPXListener \
	UnpackedTarball/libwpd/src/lib/WPXMemoryStream \
	UnpackedTarball/libwpd/src/lib/WPXPageSpan \
	UnpackedTarball/libwpd/src/lib/WPXParser \
	UnpackedTarball/libwpd/src/lib/WPXStylesListener \
	UnpackedTarball/libwpd/src/lib/WPXSubDocument \
	UnpackedTarball/libwpd/src/lib/WPXTable \
	UnpackedTarball/libwpd/src/lib/WPXTableList \
	UnpackedTarball/libwpd/src/lib/libwpd_internal \
	UnpackedTarball/libwpd/src/lib/libwpd_math \
))

# vim: set noet sw=4 ts=4:
