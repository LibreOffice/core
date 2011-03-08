/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* DocumentCollector: Collects sections and runs of text from a
 * file (and styles to go along with them) and writes them
 * to a target file
 *
 * Copyright (C) 2002-2004 William Lachance (william.lachance@sympatico.ca)
 * Copyright (C) 2003-2004 Net Integration Technologies (http://www.net-itech.com)
 * Copyright (C) 2004 Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libwpd.sourceforge.net
 *
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef _DOCUMENTCOLLECTOR_HXX
#define _DOCUMENTCOLLECTOR_HXX
#include "SectionStyle.hxx"

#if defined _MSC_VER
#pragma warning( push, 1 )
#endif
#include <libwps/libwps.h>
#include <libwpd/libwpd.h>
#if defined _MSC_VER
#pragma warning( pop )
#endif
#include <vector>
#include <map>
#include <stack>
#include <string.h>

#include "DocumentHandlerInterface.hxx"

class DocumentElement;
class TagOpenElement;
class FontStyle;
class ListStyle;

class ParagraphStyle;
class SpanStyle;
class TableStyle;
class PageSpan;

// the state we use for writing the final document
typedef struct _WriterDocumentState WriterDocumentState;
struct _WriterDocumentState
{
    _WriterDocumentState();

    bool mbFirstElement;
    bool mbFirstParagraphInPageSpan;
    bool mbInFakeSection;
    bool mbListElementOpenedAtCurrentLevel;
    bool mbTableCellOpened;
    bool mbHeaderRow;
    bool mbInNote;
    bool mbInTextBox;
    bool mbInFrame;
};

// list state
typedef struct _WriterListState WriterListState;
struct _WriterListState
{
    _WriterListState();

    ListStyle *mpCurrentListStyle;
    unsigned int miCurrentListLevel;
    unsigned int miLastListLevel;
    unsigned int miLastListNumber;
    bool mbListContinueNumbering;
    bool mbListElementParagraphOpened;
    std::stack<bool> mbListElementOpened;
};

enum WriterListType { unordered, ordered };

struct ltstr
{
  bool operator()(const WPXString & s1, const WPXString & s2) const
  {
    return strcmp(s1.cstr(), s2.cstr()) < 0;
  }
};

class DocumentCollector : public WPXDocumentInterface
{
public:
    DocumentCollector(WPXInputStream *pInput, DocumentHandlerInterface *pHandler);
    virtual ~DocumentCollector();
    bool filter();

    // WPXDocumentInterface's callbacks
    virtual void setDocumentMetaData(const WPXPropertyList &propList);
    virtual void startDocument() {}
    virtual void endDocument() {}

    virtual void definePageStyle(const WPXPropertyList&) {}
    virtual void openPageSpan(const WPXPropertyList &propList);
    virtual void closePageSpan() {}

    virtual void defineSectionStyle(const WPXPropertyList&, const WPXPropertyListVector&) {}
    virtual void openSection(const WPXPropertyList &propList, const WPXPropertyListVector &columns);
    virtual void closeSection();

    virtual void openHeader(const WPXPropertyList &propList);
    virtual void closeHeader();
    virtual void openFooter(const WPXPropertyList &propList);
    virtual void closeFooter();

    virtual void defineParagraphStyle(const WPXPropertyList&, const WPXPropertyListVector&) {}
    virtual void openParagraph(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops);
    virtual void closeParagraph();

    virtual void defineCharacterStyle(const WPXPropertyList&) {}
    virtual void openSpan(const WPXPropertyList &propList);
    virtual void closeSpan();

    virtual void insertTab();
    virtual void insertSpace();
    virtual void insertText(const WPXString &text);
     virtual void insertLineBreak();
    virtual void insertField(const WPXString &type, const WPXPropertyList &propList);

    virtual void defineOrderedListLevel(const WPXPropertyList &propList);
    virtual void defineUnorderedListLevel(const WPXPropertyList &propList);
    virtual void openOrderedListLevel(const WPXPropertyList &propList);
    virtual void openUnorderedListLevel(const WPXPropertyList &propList);
    virtual void closeOrderedListLevel();
    virtual void closeUnorderedListLevel();
    virtual void openListElement(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops);
    virtual void closeListElement();

    virtual void openFootnote(const WPXPropertyList &propList);
    virtual void closeFootnote();
    virtual void openEndnote(const WPXPropertyList &propList);
    virtual void closeEndnote();
    virtual void openComment(const WPXPropertyList &propList);
    virtual void closeComment();
    virtual void openTextBox(const WPXPropertyList &propList);
    virtual void closeTextBox();

     virtual void openTable(const WPXPropertyList &propList, const WPXPropertyListVector &columns);
     virtual void openTableRow(const WPXPropertyList &propList);
    virtual void closeTableRow();
     virtual void openTableCell(const WPXPropertyList &propList);
    virtual void closeTableCell();
    virtual void insertCoveredTableCell(const WPXPropertyList &propList);
     virtual void closeTable();

    virtual void openFrame(const WPXPropertyList & propList);
    virtual void closeFrame();

    virtual void insertBinaryObject(const WPXPropertyList &propList, const WPXBinaryData &data);
    virtual void insertEquation(const WPXPropertyList & /* propList */, const WPXString & /* data */) {}

    virtual bool parseSourceDocument(WPXInputStream &input) = 0;

protected:
    void _resetDocumentState();
    bool _writeTargetDocument(DocumentHandlerInterface *pHandler);
    void _writeDefaultStyles(DocumentHandlerInterface *pHandler);
    void _writeMasterPages(DocumentHandlerInterface *pHandler);
    void _writePageLayouts(DocumentHandlerInterface *pHandler);
    void _allocateFontName(const WPXString &);

private:
    void _openListLevel(TagOpenElement *pListLevelOpenElement);
    void _closeListLevel();

    WPXInputStream *mpInput;
    DocumentHandlerInterface *mpHandler;
    bool mbUsed; // whether or not it has been before (you can only use me once!)

    std::stack<WriterDocumentState> mWriterDocumentStates;

    std::stack<WriterListState> mWriterListStates;

    // paragraph styles
    std::map<WPXString, ParagraphStyle *, ltstr> mTextStyleHash;

        // span styles
        std::map<WPXString, SpanStyle *, ltstr> mSpanStyleHash;

    // font styles
    std::map<WPXString, FontStyle *, ltstr> mFontHash;

    // section styles
    std::vector<SectionStyle *> mSectionStyles;
    double mfSectionSpaceAfter;

    // table styles
    std::vector<TableStyle *> mTableStyles;

    // frame styles
    std::vector<DocumentElement *> mFrameStyles;

    std::vector<DocumentElement *> mFrameAutomaticStyles;

    // metadata
    std::vector<DocumentElement *> mMetaData;

    // list styles
    unsigned int miNumListStyles;

    // style elements
    std::vector<DocumentElement *> mStylesElements;
    // content elements
    std::vector<DocumentElement *> mBodyElements;
    // the current set of elements that we're writing to
    std::vector<DocumentElement *> * mpCurrentContentElements;

    // page state
    std::vector<PageSpan *> mPageSpans;
    PageSpan *mpCurrentPageSpan;
    int miNumPageStyles;

    // list styles
    std::vector<ListStyle *> mListStyles;

    // object state
    unsigned miObjectNumber;

    // table state
    TableStyle *mpCurrentTableStyle;

    const bool mbIsFlatXML;

    const char * mpPassword;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
