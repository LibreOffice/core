#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    IllustrationsIndex.idl\
    InContentMetadata.idl\
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
    TextMarkupDescriptor.idl\
    TextMarkupType.idl \
      TextPageStyle.idl\
      TextPortion.idl\
      TextPortionEnumeration.idl\
      TextRange.idl\
      TextRanges.idl\
      TextRangeContentProperties.idl\
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
    XRedline.idl\
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
    XMarkingAccess.idl\
    XTextCopy.idl\
      XTextDocument.idl\
      XTextEmbeddedObject.idl\
      XTextEmbeddedObjectsSupplier.idl\
      XTextField.idl\
      XTextFieldsSupplier.idl\
      XTextFrame.idl\
      XTextFramesSupplier.idl\
      XTextGraphicObjectsSupplier.idl\
    XTextMarkup.idl\
    XMultiTextMarkup.idl\
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
    XFormField.idl \
    XFlatParagraph.idl \
    XFlatParagraphIterator.idl \
    XFlatParagraphIteratorProvider.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
