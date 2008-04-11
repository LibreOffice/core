#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.46 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=csstext
PACKAGE=com$/sun$/star$/text

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AccessibleEndnoteView.idl\
    AccessibleFootnoteView.idl\
    AccessibleHeaderFooterView.idl\
    AccessiblePageView.idl\
    AccessibleParagraphView.idl\
    AccessibleTextDocumentPageView.idl\
    AccessibleTextDocumentView.idl\
    AccessibleTextEmbeddedObject.idl\
    AccessibleTextFrameView.idl\
    AccessibleTextGraphicObject.idl\
    AdvancedTextDocument.idl\
    AuthorDisplayFormat.idl\
    AutoTextContainer.idl\
    AutoTextEntry.idl\
    AutoTextGroup.idl\
    BaseFrame.idl\
    BaseFrameProperties.idl\
    BaseIndex.idl\
    BaseIndexMark.idl\
    Bibliography.idl\
    BibliographyDataType.idl\
    BibliographyDataField.idl\
    Bookmark.idl\
    Bookmarks.idl\
    Cell.idl\
    CellProperties.idl\
    CellRange.idl\
    ChainedTextFrame.idl\
    ChapterFormat.idl\
    ChapterNumberingRule.idl\
    CharacterCompressionType.idl\
    ContentIndex.idl\
    ContentIndexMark.idl\
    ControlCharacter.idl\
    DateDisplayFormat.idl\
    DefaultNumberingProvider.idl\
    Defaults.idl\
    DependentTextField.idl\
    DocumentIndex.idl\
    DocumentIndexes.idl\
    DocumentIndexFormat.idl\
    DocumentIndexLevelFormat.idl\
    DocumentIndexMark.idl\
    DocumentIndexMarkAsian.idl\
    DocumentIndexParagraphStyles.idl\
    DocumentSettings.idl\
    DocumentStatistic.idl\
    Endnote.idl\
        EndnoteSettings.idl\
    FilenameDisplayFormat.idl\
    FontEmphasis.idl\
    FontRelief.idl\
    Footnote.idl\
     FootnoteNumbering.idl\
     Footnotes.idl\
     FootnoteSettings.idl\
    GenericTextDocument.idl\
    GlobalDocument.idl\
     GlobalSettings.idl\
     GraphicCrop.idl\
     HoriOrientation.idl\
     HoriOrientationFormat.idl\
     HorizontalAdjust.idl\
     HypertextDocument.idl\
     IllustrationIndex.idl\
     InvalidTextContentException.idl\
        LabelFollow.idl\
     LineNumberingProperties.idl\
     LineNumberingSettings.idl\
        MailMerge.idl\
        MailMergeEvent.idl\
        MailMergeType.idl\
     NotePrintMode.idl\
     NumberingLevel.idl\
     NumberingRules.idl\
     NumberingStyle.idl\
     ObjectIndex.idl\
     PageFootnoteInfo.idl\
     PageNumberType.idl\
     PagePrintSettings.idl\
     Paragraph.idl\
     ParagraphEnumeration.idl\
     ParagraphVertAlign.idl\
     PlaceholderType.idl\
        PositionAndSpaceMode.idl\
        PositionLayoutDir.idl\
        PrintPreviewSettings.idl\
     PrintSettings.idl\
     RedlinePortion.idl\
     ReferenceFieldPart.idl\
     ReferenceFieldSource.idl\
     ReferenceMark.idl\
     ReferenceMarks.idl\
     RelOrientation.idl\
     RubyAdjust.idl\
     SectionFileLink.idl\
     SetVariableType.idl\
     Shape.idl\
     SizeType.idl\
        TableColumns.idl\
     TableColumnSeparator.idl\
     TableIndex.idl\
        TableRows.idl\
     TemplateDisplayFormat.idl\
     Text.idl\
     TextColumn.idl\
    TextColumns.idl\
     TextColumnSequence.idl\
     TextContent.idl\
     TextContentAnchorType.idl\
     TextContentCollection.idl\
     TextCursor.idl\
     TextDocument.idl\
     TextDocumentView.idl\
     TextEmbeddedObject.idl\
     TextEmbeddedObjects.idl\
     TextField.idl\
     TextFieldEnumeration.idl\
     TextFieldMaster.idl\
     TextFieldMasters.idl\
     TextFields.idl\
     TextFrame.idl\
     TextFrames.idl\
     TextGraphicObject.idl\
     TextGraphicObjects.idl\
     TextGridMode.idl\
      TextLayoutCursor.idl\
    TextMarkupType.idl \
      TextPageStyle.idl\
      TextPortion.idl\
      TextPortionEnumeration.idl\
      TextRange.idl\
      TextRanges.idl\
      TextSection.idl\
      TextSections.idl\
      TextSortable.idl\
      TextSortDescriptor.idl\
        TextSortDescriptor2.idl\
      TextTable.idl\
      TextTableCursor.idl\
      TextTableRow.idl\
      TextTables.idl\
      TextViewCursor.idl\
      TimeDisplayFormat.idl\
      UserDataPart.idl\
      UserDefinedIndex.idl\
      UserFieldFormat.idl\
      UserIndex.idl\
      UserIndexMark.idl\
      VertOrientation.idl\
      VertOrientationFormat.idl\
      ViewSettings.idl\
    WebDocument.idl\
        WrapInfluenceOnPosition.idl\
        WrapTextMode.idl\
      WritingMode.idl\
      WritingMode2.idl\
      XAutoTextContainer.idl\
      XAutoTextEntry.idl\
      XAutoTextGroup.idl\
      XBookmarkInsertTool.idl\
      XBookmarksSupplier.idl\
      XChapterNumberingSupplier.idl\
      XDefaultNumberingProvider.idl\
      XDependentTextField.idl\
      XDocumentIndex.idl\
      XDocumentIndexesSupplier.idl\
      XDocumentIndexMark.idl\
      XEndnotesSettingsSupplier.idl\
      XEndnotesSupplier.idl\
      XFootnote.idl\
      XFootnotesSettingsSupplier.idl\
      XFootnotesSupplier.idl\
      XHeaderFooter.idl\
      XHeaderFooterPageStyle.idl\
      XLineNumberingSupplier.idl\
      XLineNumberingProperties.idl\
        XMailMergeBroadcaster.idl\
        XMailMergeListener.idl\
      XModule.idl\
      XNumberingFormatter.idl\
      XNumberingRulesSupplier.idl\
      XNumberingTypeInfo.idl\
      XPageCursor.idl\
      XPagePrintable.idl\
      XParagraphCursor.idl\
      XReferenceMarksSupplier.idl\
      XRelativeTextContentInsert.idl\
      XRelativeTextContentRemove.idl\
      XRubySelection.idl\
      XSentenceCursor.idl\
      XSimpleText.idl\
      XText.idl\
      XTextColumns.idl\
      XTextContent.idl\
      XTextCursor.idl\
      XTextDocument.idl\
      XTextEmbeddedObject.idl\
      XTextEmbeddedObjectsSupplier.idl\
      XTextField.idl\
      XTextFieldsSupplier.idl\
      XTextFrame.idl\
      XTextFramesSupplier.idl\
      XTextGraphicObjectsSupplier.idl\
    XTextMarkup.idl\
      XTextRange.idl\
      XTextRangeCompare.idl\
      XTextRangeMover.idl\
      XTextSection.idl\
      XTextSectionsSupplier.idl\
      XTextShapesSupplier.idl\
      XTextTable.idl\
      XTextTableCursor.idl\
      XTextTablesSupplier.idl\
      XTextViewCursor.idl\
      XTextViewCursorSupplier.idl\
    XWordCursor.idl \
    XParagraphAppend.idl \
    XTextPortionAppend.idl \
    XTextContentAppend.idl \
    XTextConvert.idl \
    XTextAppendAndConvert.idl \
    XTextAppend.idl \
        XFlatParagraph.idl \
        XFlatParagraphIterator.idl \
        XFlatParagraphIteratorProvider.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
