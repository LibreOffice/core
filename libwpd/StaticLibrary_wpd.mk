# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,wpdlib))

$(eval $(call gb_StaticLibrary_use_unpacked,wpdlib,wpd))

$(eval $(call gb_StaticLibrary_use_package,wpdlib,libwpd_inc))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,wpdlib,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,wpdlib,\
	UnpackedTarball/wpd/src/lib/libwpd_internal \
	UnpackedTarball/wpd/src/lib/libwpd_math \
	UnpackedTarball/wpd/src/lib/WP1BottomMarginGroup \
	UnpackedTarball/wpd/src/lib/WP1CenterTextGroup \
	UnpackedTarball/wpd/src/lib/WP1ContentListener \
	UnpackedTarball/wpd/src/lib/WP1ExtendedCharacterGroup \
	UnpackedTarball/wpd/src/lib/WP1FileStructure \
	UnpackedTarball/wpd/src/lib/WP1FixedLengthGroup \
	UnpackedTarball/wpd/src/lib/WP1FlushRightGroup \
	UnpackedTarball/wpd/src/lib/WP1FontIdGroup \
	UnpackedTarball/wpd/src/lib/WP1FootnoteEndnoteGroup \
	UnpackedTarball/wpd/src/lib/WP1HeaderFooterGroup \
	UnpackedTarball/wpd/src/lib/WP1Heuristics \
	UnpackedTarball/wpd/src/lib/WP1JustificationGroup \
	UnpackedTarball/wpd/src/lib/WP1LeftIndentGroup \
	UnpackedTarball/wpd/src/lib/WP1LeftRightIndentGroup \
	UnpackedTarball/wpd/src/lib/WP1Listener \
	UnpackedTarball/wpd/src/lib/WP1MarginReleaseGroup \
	UnpackedTarball/wpd/src/lib/WP1MarginResetGroup \
	UnpackedTarball/wpd/src/lib/WP1Parser \
	UnpackedTarball/wpd/src/lib/WP1Part \
	UnpackedTarball/wpd/src/lib/WP1PictureGroup \
	UnpackedTarball/wpd/src/lib/WP1PointSizeGroup \
	UnpackedTarball/wpd/src/lib/WP1SetTabsGroup \
	UnpackedTarball/wpd/src/lib/WP1SpacingResetGroup \
	UnpackedTarball/wpd/src/lib/WP1StylesListener \
	UnpackedTarball/wpd/src/lib/WP1SubDocument \
	UnpackedTarball/wpd/src/lib/WP1SuppressPageCharacteristicsGroup \
	UnpackedTarball/wpd/src/lib/WP1TopMarginGroup \
	UnpackedTarball/wpd/src/lib/WP1UnsupportedFixedLengthGroup \
	UnpackedTarball/wpd/src/lib/WP1UnsupportedVariableLengthGroup \
	UnpackedTarball/wpd/src/lib/WP1VariableLengthGroup \
	UnpackedTarball/wpd/src/lib/WP3AttributeGroup \
	UnpackedTarball/wpd/src/lib/WP3ContentListener \
	UnpackedTarball/wpd/src/lib/WP3DefinitionGroup \
	UnpackedTarball/wpd/src/lib/WP3DisplayGroup \
	UnpackedTarball/wpd/src/lib/WP3DoubleByteScriptCharacterGroup \
	UnpackedTarball/wpd/src/lib/WP3EndOfLinePageGroup \
	UnpackedTarball/wpd/src/lib/WP3ExtendedCharacterGroup \
	UnpackedTarball/wpd/src/lib/WP3FileStructure \
	UnpackedTarball/wpd/src/lib/WP3FixedLengthGroup \
	UnpackedTarball/wpd/src/lib/WP3FontGroup \
	UnpackedTarball/wpd/src/lib/WP3FootnoteEndnoteGroup \
	UnpackedTarball/wpd/src/lib/WP3Header \
	UnpackedTarball/wpd/src/lib/WP3HeaderFooterGroup \
	UnpackedTarball/wpd/src/lib/WP3IndentGroup \
	UnpackedTarball/wpd/src/lib/WP3Listener \
	UnpackedTarball/wpd/src/lib/WP3MiscellaneousGroup \
	UnpackedTarball/wpd/src/lib/WP3PageFormatGroup \
	UnpackedTarball/wpd/src/lib/WP3Parser \
	UnpackedTarball/wpd/src/lib/WP3Part \
	UnpackedTarball/wpd/src/lib/WP3Resource \
	UnpackedTarball/wpd/src/lib/WP3ResourceFork \
	UnpackedTarball/wpd/src/lib/WP3SingleByteFunction \
	UnpackedTarball/wpd/src/lib/WP3StylesListener \
	UnpackedTarball/wpd/src/lib/WP3SubDocument \
	UnpackedTarball/wpd/src/lib/WP3TabGroup \
	UnpackedTarball/wpd/src/lib/WP3TablesGroup \
	UnpackedTarball/wpd/src/lib/WP3UndoGroup \
	UnpackedTarball/wpd/src/lib/WP3UnsupportedFixedLengthGroup \
	UnpackedTarball/wpd/src/lib/WP3UnsupportedVariableLengthGroup \
	UnpackedTarball/wpd/src/lib/WP3VariableLengthGroup \
	UnpackedTarball/wpd/src/lib/WP3WindowGroup \
	UnpackedTarball/wpd/src/lib/WP42ContentListener \
	UnpackedTarball/wpd/src/lib/WP42DefineColumnsGroup \
	UnpackedTarball/wpd/src/lib/WP42ExtendedCharacterGroup \
	UnpackedTarball/wpd/src/lib/WP42FileStructure \
	UnpackedTarball/wpd/src/lib/WP42HeaderFooterGroup \
	UnpackedTarball/wpd/src/lib/WP42Heuristics \
	UnpackedTarball/wpd/src/lib/WP42Listener \
	UnpackedTarball/wpd/src/lib/WP42MarginResetGroup \
	UnpackedTarball/wpd/src/lib/WP42MultiByteFunctionGroup \
	UnpackedTarball/wpd/src/lib/WP42Parser \
	UnpackedTarball/wpd/src/lib/WP42Part \
	UnpackedTarball/wpd/src/lib/WP42StylesListener \
	UnpackedTarball/wpd/src/lib/WP42SubDocument \
	UnpackedTarball/wpd/src/lib/WP42SuppressPageCharacteristicsGroup \
	UnpackedTarball/wpd/src/lib/WP42UnsupportedMultiByteFunctionGroup \
	UnpackedTarball/wpd/src/lib/WP5AttributeGroup \
	UnpackedTarball/wpd/src/lib/WP5BoxGroup \
	UnpackedTarball/wpd/src/lib/WP5ContentListener \
	UnpackedTarball/wpd/src/lib/WP5DefinitionGroup \
	UnpackedTarball/wpd/src/lib/WP5ExtendedCharacterGroup \
	UnpackedTarball/wpd/src/lib/WP5FileStructure \
	UnpackedTarball/wpd/src/lib/WP5FixedLengthGroup \
	UnpackedTarball/wpd/src/lib/WP5FontGroup \
	UnpackedTarball/wpd/src/lib/WP5FontNameStringPoolPacket \
	UnpackedTarball/wpd/src/lib/WP5FootnoteEndnoteGroup \
	UnpackedTarball/wpd/src/lib/WP5GeneralPacketData \
	UnpackedTarball/wpd/src/lib/WP5GeneralPacketIndex \
	UnpackedTarball/wpd/src/lib/WP5GraphicsInformationPacket \
	UnpackedTarball/wpd/src/lib/WP5Header \
	UnpackedTarball/wpd/src/lib/WP5HeaderFooterGroup \
	UnpackedTarball/wpd/src/lib/WP5IndentGroup \
	UnpackedTarball/wpd/src/lib/WP5Listener \
	UnpackedTarball/wpd/src/lib/WP5ListFontsUsedPacket \
	UnpackedTarball/wpd/src/lib/WP5PageFormatGroup \
	UnpackedTarball/wpd/src/lib/WP5Parser \
	UnpackedTarball/wpd/src/lib/WP5Part \
	UnpackedTarball/wpd/src/lib/WP5PrefixData \
	UnpackedTarball/wpd/src/lib/WP5SingleByteFunction \
	UnpackedTarball/wpd/src/lib/WP5SpecialHeaderIndex \
	UnpackedTarball/wpd/src/lib/WP5StylesListener \
	UnpackedTarball/wpd/src/lib/WP5SubDocument \
	UnpackedTarball/wpd/src/lib/WP5TabGroup \
	UnpackedTarball/wpd/src/lib/WP5TableEOLGroup \
	UnpackedTarball/wpd/src/lib/WP5TableEOPGroup \
	UnpackedTarball/wpd/src/lib/WP5UnsupportedFixedLengthGroup \
	UnpackedTarball/wpd/src/lib/WP5UnsupportedVariableLengthGroup \
	UnpackedTarball/wpd/src/lib/WP5VariableLengthGroup \
	UnpackedTarball/wpd/src/lib/WP60Header \
	UnpackedTarball/wpd/src/lib/WP61Header \
	UnpackedTarball/wpd/src/lib/WP6AttributeGroup \
	UnpackedTarball/wpd/src/lib/WP6BoxGroup \
	UnpackedTarball/wpd/src/lib/WP6CharacterGroup \
	UnpackedTarball/wpd/src/lib/WP6ColumnGroup \
	UnpackedTarball/wpd/src/lib/WP6CommentAnnotationPacket \
	UnpackedTarball/wpd/src/lib/WP6ContentListener \
	UnpackedTarball/wpd/src/lib/WP6DefaultInitialFontPacket \
	UnpackedTarball/wpd/src/lib/WP6DisplayNumberReferenceGroup \
	UnpackedTarball/wpd/src/lib/WP6EOLGroup \
	UnpackedTarball/wpd/src/lib/WP6ExtendedCharacterGroup \
	UnpackedTarball/wpd/src/lib/WP6ExtendedDocumentSummaryPacket \
	UnpackedTarball/wpd/src/lib/WP6FileStructure \
	UnpackedTarball/wpd/src/lib/WP6FillStylePacket \
	UnpackedTarball/wpd/src/lib/WP6FixedLengthGroup \
	UnpackedTarball/wpd/src/lib/WP6FontDescriptorPacket \
	UnpackedTarball/wpd/src/lib/WP6FootnoteEndnoteGroup \
	UnpackedTarball/wpd/src/lib/WP6GeneralTextPacket \
	UnpackedTarball/wpd/src/lib/WP6GraphicsBoxStylePacket \
	UnpackedTarball/wpd/src/lib/WP6GraphicsCachedFileDataPacket \
	UnpackedTarball/wpd/src/lib/WP6GraphicsFilenamePacket \
	UnpackedTarball/wpd/src/lib/WP6Header \
	UnpackedTarball/wpd/src/lib/WP6HeaderFooterGroup \
	UnpackedTarball/wpd/src/lib/WP6HighlightGroup \
	UnpackedTarball/wpd/src/lib/WP6Listener \
	UnpackedTarball/wpd/src/lib/WP6NumberingMethodGroup \
	UnpackedTarball/wpd/src/lib/WP6OutlineStylePacket \
	UnpackedTarball/wpd/src/lib/WP6PageGroup \
	UnpackedTarball/wpd/src/lib/WP6ParagraphGroup \
	UnpackedTarball/wpd/src/lib/WP6Parser \
	UnpackedTarball/wpd/src/lib/WP6Part \
	UnpackedTarball/wpd/src/lib/WP6PrefixData \
	UnpackedTarball/wpd/src/lib/WP6PrefixDataPacket \
	UnpackedTarball/wpd/src/lib/WP6PrefixIndice \
	UnpackedTarball/wpd/src/lib/WP6SetNumberGroup \
	UnpackedTarball/wpd/src/lib/WP6SingleByteFunction \
	UnpackedTarball/wpd/src/lib/WP6StyleGroup \
	UnpackedTarball/wpd/src/lib/WP6StylesListener \
	UnpackedTarball/wpd/src/lib/WP6SubDocument \
	UnpackedTarball/wpd/src/lib/WP6TabGroup \
	UnpackedTarball/wpd/src/lib/WP6TableStylePacket \
	UnpackedTarball/wpd/src/lib/WP6UndoGroup \
	UnpackedTarball/wpd/src/lib/WP6UnsupportedFixedLengthGroup \
	UnpackedTarball/wpd/src/lib/WP6UnsupportedVariableLengthGroup \
	UnpackedTarball/wpd/src/lib/WP6VariableLengthGroup \
	UnpackedTarball/wpd/src/lib/WPDocument \
	UnpackedTarball/wpd/src/lib/WPXBinaryData \
	UnpackedTarball/wpd/src/lib/WPXContentListener \
	UnpackedTarball/wpd/src/lib/WPXEncryption \
	UnpackedTarball/wpd/src/lib/WPXHeader \
	UnpackedTarball/wpd/src/lib/WPXListener \
	UnpackedTarball/wpd/src/lib/WPXMemoryStream \
	UnpackedTarball/wpd/src/lib/WPXPageSpan \
	UnpackedTarball/wpd/src/lib/WPXParser \
	UnpackedTarball/wpd/src/lib/WPXProperty \
	UnpackedTarball/wpd/src/lib/WPXPropertyList \
	UnpackedTarball/wpd/src/lib/WPXPropertyListVector \
	UnpackedTarball/wpd/src/lib/WPXString \
	UnpackedTarball/wpd/src/lib/WPXStylesListener \
	UnpackedTarball/wpd/src/lib/WPXSubDocument \
	UnpackedTarball/wpd/src/lib/WPXTable \
))

# vim: set noet sw=4 ts=4:
