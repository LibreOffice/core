# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,libwpd))

$(eval $(call gb_Library_use_unpacked,libwpd,libwpd))

$(eval $(call gb_Library_use_externals,libwpd,\
    boost_headers \
	revenge \
))

$(eval $(call gb_Library_set_warnings_not_errors,libwpd))

$(eval $(call gb_Library_set_include,libwpd,\
    -I$(call gb_UnpackedTarball_get_dir,libwpd)/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,libwpd,\
	-DBOOST_ALL_NO_LIB \
	-DDLL_EXPORT \
	-DLIBWPD_BUILD \
	-DNDEBUG \
))

$(eval $(call gb_Library_add_generated_exception_objects,libwpd,\
	UnpackedTarball/libwpd/src/lib/WP1BottomMarginGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1CenterTextGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1ContentListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP1ExtendedCharacterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1FileStructure.cpp \
	UnpackedTarball/libwpd/src/lib/WP1FixedLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1FlushRightGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1FontIdGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1FootnoteEndnoteGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1HeaderFooterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1Heuristics.cpp \
	UnpackedTarball/libwpd/src/lib/WP1JustificationGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1LeftIndentGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1LeftRightIndentGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1Listener.cpp \
	UnpackedTarball/libwpd/src/lib/WP1MarginReleaseGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1MarginResetGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1Parser.cpp \
	UnpackedTarball/libwpd/src/lib/WP1Part.cpp \
	UnpackedTarball/libwpd/src/lib/WP1PictureGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1PointSizeGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1SetTabsGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1SpacingResetGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1StylesListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP1SubDocument.cpp \
	UnpackedTarball/libwpd/src/lib/WP1SuppressPageCharacteristicsGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1TopMarginGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1UnsupportedFixedLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1UnsupportedVariableLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP1VariableLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3AttributeGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3ContentListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP3DefinitionGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3DisplayGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3DoubleByteScriptCharacterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3EndOfLinePageGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3ExtendedCharacterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3FileStructure.cpp \
	UnpackedTarball/libwpd/src/lib/WP3FixedLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3FontGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3FootnoteEndnoteGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3Header.cpp \
	UnpackedTarball/libwpd/src/lib/WP3HeaderFooterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3IndentGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3Listener.cpp \
	UnpackedTarball/libwpd/src/lib/WP3MiscellaneousGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3PageFormatGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3Parser.cpp \
	UnpackedTarball/libwpd/src/lib/WP3Part.cpp \
	UnpackedTarball/libwpd/src/lib/WP3Resource.cpp \
	UnpackedTarball/libwpd/src/lib/WP3ResourceFork.cpp \
	UnpackedTarball/libwpd/src/lib/WP3SingleByteFunction.cpp \
	UnpackedTarball/libwpd/src/lib/WP3StylesListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP3SubDocument.cpp \
	UnpackedTarball/libwpd/src/lib/WP3TabGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3TablesGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3UndoGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3UnsupportedFixedLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3UnsupportedVariableLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3VariableLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP3WindowGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP42ContentListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP42DefineColumnsGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP42ExtendedCharacterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP42FileStructure.cpp \
	UnpackedTarball/libwpd/src/lib/WP42HeaderFooterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP42Heuristics.cpp \
	UnpackedTarball/libwpd/src/lib/WP42Listener.cpp \
	UnpackedTarball/libwpd/src/lib/WP42MarginResetGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP42MultiByteFunctionGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP42Parser.cpp \
	UnpackedTarball/libwpd/src/lib/WP42Part.cpp \
	UnpackedTarball/libwpd/src/lib/WP42StylesListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP42SubDocument.cpp \
	UnpackedTarball/libwpd/src/lib/WP42SuppressPageCharacteristicsGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP42UnsupportedMultiByteFunctionGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5AttributeGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5BoxGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5ContentListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP5DefinitionGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5ExtendedCharacterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5FileStructure.cpp \
	UnpackedTarball/libwpd/src/lib/WP5FixedLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5FontGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5FontNameStringPoolPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP5FootnoteEndnoteGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5GeneralPacketData.cpp \
	UnpackedTarball/libwpd/src/lib/WP5GeneralPacketIndex.cpp \
	UnpackedTarball/libwpd/src/lib/WP5GraphicsInformationPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP5Header.cpp \
	UnpackedTarball/libwpd/src/lib/WP5HeaderFooterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5IndentGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5ListFontsUsedPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP5Listener.cpp \
	UnpackedTarball/libwpd/src/lib/WP5PageFormatGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5Parser.cpp \
	UnpackedTarball/libwpd/src/lib/WP5Part.cpp \
	UnpackedTarball/libwpd/src/lib/WP5PrefixData.cpp \
	UnpackedTarball/libwpd/src/lib/WP5SingleByteFunction.cpp \
	UnpackedTarball/libwpd/src/lib/WP5SpecialHeaderIndex.cpp \
	UnpackedTarball/libwpd/src/lib/WP5StylesListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP5SubDocument.cpp \
	UnpackedTarball/libwpd/src/lib/WP5TabGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5TableEOLGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5TableEOPGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5UnsupportedFixedLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5UnsupportedVariableLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP5VariableLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP60Header.cpp \
	UnpackedTarball/libwpd/src/lib/WP61Header.cpp \
	UnpackedTarball/libwpd/src/lib/WP6AttributeGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6BoxGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6CharacterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6ColumnGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6CommentAnnotationPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6ContentListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP6DefaultInitialFontPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6DisplayNumberReferenceGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6EOLGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6ExtendedCharacterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6ExtendedDocumentSummaryPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6FileStructure.cpp \
	UnpackedTarball/libwpd/src/lib/WP6FillStylePacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6FixedLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6FontDescriptorPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6FootnoteEndnoteGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6GeneralTextPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6GraphicsBoxStylePacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6GraphicsCachedFileDataPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6GraphicsFilenamePacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6Header.cpp \
	UnpackedTarball/libwpd/src/lib/WP6HeaderFooterGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6HighlightGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6Listener.cpp \
	UnpackedTarball/libwpd/src/lib/WP6NumberingMethodGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6OutlineStylePacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6PageGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6ParagraphGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6Parser.cpp \
	UnpackedTarball/libwpd/src/lib/WP6Part.cpp \
	UnpackedTarball/libwpd/src/lib/WP6PrefixData.cpp \
	UnpackedTarball/libwpd/src/lib/WP6PrefixDataPacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6PrefixIndice.cpp \
	UnpackedTarball/libwpd/src/lib/WP6SetNumberGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6SingleByteFunction.cpp \
	UnpackedTarball/libwpd/src/lib/WP6StyleGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6StylesListener.cpp \
	UnpackedTarball/libwpd/src/lib/WP6SubDocument.cpp \
	UnpackedTarball/libwpd/src/lib/WP6TabGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6TableStylePacket.cpp \
	UnpackedTarball/libwpd/src/lib/WP6UndoGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6UnsupportedFixedLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6UnsupportedVariableLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WP6VariableLengthGroup.cpp \
	UnpackedTarball/libwpd/src/lib/WPDocument.cpp \
	UnpackedTarball/libwpd/src/lib/WPXContentListener.cpp \
	UnpackedTarball/libwpd/src/lib/WPXEncryption.cpp \
	UnpackedTarball/libwpd/src/lib/WPXHeader.cpp \
	UnpackedTarball/libwpd/src/lib/WPXListener.cpp \
	UnpackedTarball/libwpd/src/lib/WPXMemoryStream.cpp \
	UnpackedTarball/libwpd/src/lib/WPXPageSpan.cpp \
	UnpackedTarball/libwpd/src/lib/WPXParser.cpp \
	UnpackedTarball/libwpd/src/lib/WPXStylesListener.cpp \
	UnpackedTarball/libwpd/src/lib/WPXSubDocument.cpp \
	UnpackedTarball/libwpd/src/lib/WPXTable.cpp \
	UnpackedTarball/libwpd/src/lib/libwpd_internal.cpp \
	UnpackedTarball/libwpd/src/lib/libwpd_math.cpp \
))

# vim: set noet sw=4 ts=4:
