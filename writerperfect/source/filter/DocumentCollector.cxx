/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* DocumentCollector: Collects sections and runs of text from a
 * file (and styles to go along with them) and writes them
 * to a Writer target document
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

#if defined _MSC_VER
#pragma warning( push, 1 )
#endif
#include <libwpd/libwpd.h>
#if defined _MSC_VER
#pragma warning( pop )
#endif
#include <string.h> // for strcmp

#include "DocumentCollector.hxx"
#include "DocumentElement.hxx"
#include "DocumentHandler.hxx"
#include "InternalHandler.hxx"
#include "TextRunStyle.hxx"
#include "FontStyle.hxx"
#include "ListStyle.hxx"
#include "PageSpan.hxx"
#include "SectionStyle.hxx"
#include "TableStyle.hxx"
#include "FilterInternal.hxx"
#include "WriterProperties.hxx"
#include "OdgExporter.hxx"

_WriterDocumentState::_WriterDocumentState() :
    mbFirstElement(true),
    mbFirstParagraphInPageSpan(true),
    mbInFakeSection(false),
    mbListElementOpenedAtCurrentLevel(false),
    mbTableCellOpened(false),
    mbHeaderRow(false),
    mbInNote(false),
    mbInTextBox(false),
    mbInFrame(false)
{
}

_WriterListState::_WriterListState() :
    mpCurrentListStyle(NULL),
    miCurrentListLevel(0),
    miLastListLevel(0),
    miLastListNumber(0),
    mbListContinueNumbering(false),
    mbListElementParagraphOpened(false),
    mbListElementOpened()
{
}

DocumentCollector::DocumentCollector(WPXInputStream *pInput, DocumentHandlerInterface *pHandler) :
        mpInput(pInput),
        mpHandler(pHandler),
    mbUsed(false),
    mWriterDocumentStates(),
    mWriterListStates(),
    mfSectionSpaceAfter(0.0),
    miNumListStyles(0),
    mpCurrentContentElements(&mBodyElements),
    mpCurrentPageSpan(NULL),
    miNumPageStyles(0),
    miObjectNumber(0),
    mbIsFlatXML(true),
    mpPassword(NULL)
{
    mWriterDocumentStates.push(WriterDocumentState());
    mWriterListStates.push(WriterListState());
}

DocumentCollector::~DocumentCollector()
{
}

bool DocumentCollector::filter()
{
    // The contract for DocumentCollector is that it will only be used once after it is
    // instantiated
    if (mbUsed)
        return false;

    mbUsed = true;

    // parse & write
        // WLACH_REFACTORING: Remove these args..
     if (!parseSourceDocument(*mpInput))
        return false;
    if (!_writeTargetDocument(mpHandler))
        return false;

     // clean up the mess we made
     WRITER_DEBUG_MSG(("WriterWordPerfect: Cleaning up our mess..\n"));

    WRITER_DEBUG_MSG(("Destroying the body elements\n"));
    for (std::vector<DocumentElement *>::iterator iterBody = mBodyElements.begin(); iterBody != mBodyElements.end(); iterBody++) {
        delete (*iterBody);
        (*iterBody) = NULL;
    }

    WRITER_DEBUG_MSG(("Destroying the styles elements\n"));
    for (std::vector<DocumentElement *>::iterator iterStyles = mStylesElements.begin(); iterStyles != mStylesElements.end(); iterStyles++) {
         delete (*iterStyles);
        (*iterStyles) = NULL; // we may pass over the same element again (in the case of headers/footers spanning multiple pages)
                      // so make sure we don't do a double del
    }

    WRITER_DEBUG_MSG(("Destroying the rest of the styles elements\n"));
    for (std::map<WPXString, ParagraphStyle *, ltstr>::iterator iterTextStyle = mTextStyleHash.begin(); iterTextStyle != mTextStyleHash.end(); iterTextStyle++) {
        delete (iterTextStyle->second);
    }
    for (std::map<WPXString, SpanStyle *, ltstr>::iterator iterSpanStyle = mSpanStyleHash.begin(); iterSpanStyle != mSpanStyleHash.end(); iterSpanStyle++) {
        delete(iterSpanStyle->second);
    }

    for (std::map<WPXString, FontStyle *, ltstr>::iterator iterFont = mFontHash.begin(); iterFont != mFontHash.end(); iterFont++) {
        delete(iterFont->second);
    }

    for (std::vector<ListStyle *>::iterator iterListStyles = mListStyles.begin(); iterListStyles != mListStyles.end(); iterListStyles++) {
        delete (*iterListStyles);
    }
    for (std::vector<SectionStyle *>::iterator iterSectionStyles = mSectionStyles.begin(); iterSectionStyles != mSectionStyles.end(); iterSectionStyles++) {
        delete (*iterSectionStyles);
    }
    for (std::vector<TableStyle *>::iterator iterTableStyles = mTableStyles.begin(); iterTableStyles != mTableStyles.end(); iterTableStyles++) {
        delete((*iterTableStyles));
    }

    for (std::vector<PageSpan *>::iterator iterPageSpans = mPageSpans.begin(); iterPageSpans != mPageSpans.end(); iterPageSpans++) {
        delete (*iterPageSpans);
    }
    for (std::vector<DocumentElement *>::iterator iterFrameStyles = mFrameStyles.begin(); iterFrameStyles != mFrameStyles.end(); iterFrameStyles++) {
        delete(*iterFrameStyles);
    }
    for (std::vector<DocumentElement *>::iterator iterFrameAutomaticStyles = mFrameAutomaticStyles.begin();
        iterFrameAutomaticStyles != mFrameAutomaticStyles.end(); iterFrameAutomaticStyles++) {
        delete(*iterFrameAutomaticStyles);
    }
    for (std::vector<DocumentElement *>::iterator iterMetaData = mMetaData.begin(); iterMetaData != mMetaData.end(); iterMetaData++) {
        delete(*iterMetaData);
    }

     return true;
}

void DocumentCollector::_writeDefaultStyles(DocumentHandlerInterface *pHandler)
{
    TagOpenElement("office:styles").write(pHandler);

    TagOpenElement defaultParagraphStyleOpenElement("style:default-style");
    defaultParagraphStyleOpenElement.addAttribute("style:family", "paragraph");
    defaultParagraphStyleOpenElement.write(pHandler);

    TagOpenElement defaultParagraphStylePropertiesOpenElement("style:paragraph-properties");
    defaultParagraphStylePropertiesOpenElement.addAttribute("style:tab-stop-distance", "0.5in");
    defaultParagraphStylePropertiesOpenElement.write(pHandler);
    TagCloseElement defaultParagraphStylePropertiesCloseElement("style:paragraph-properties");
    defaultParagraphStylePropertiesCloseElement.write(pHandler);

    pHandler->endElement("style:default-style");

    TagOpenElement defaultTableRowStyleOpenElement("style:default-style");
    defaultTableRowStyleOpenElement.addAttribute("style:family", "table-row");
    defaultTableRowStyleOpenElement.write(pHandler);

    TagOpenElement defaultTableRowPropertiesOpenElement("style:table-row-properties");
    defaultTableRowPropertiesOpenElement.addAttribute("fo:keep-together", "auto");
    defaultTableRowPropertiesOpenElement.write(pHandler);

    pHandler->endElement("style:table-row-properties");
    pHandler->endElement("style:default-style");

    TagOpenElement standardStyleOpenElement("style:style");
        standardStyleOpenElement.addAttribute("style:name", "Standard");
        standardStyleOpenElement.addAttribute("style:family", "paragraph");
        standardStyleOpenElement.addAttribute("style:class", "text");
        standardStyleOpenElement.write(pHandler);

    pHandler->endElement("style:style");

        TagOpenElement textBodyStyleOpenElement("style:style");
    textBodyStyleOpenElement.addAttribute("style:name", "Text_Body");
    textBodyStyleOpenElement.addAttribute("style:display-name", "Text Body");
        textBodyStyleOpenElement.addAttribute("style:family", "paragraph");
        textBodyStyleOpenElement.addAttribute("style:parent-style-name", "Standard");
        textBodyStyleOpenElement.addAttribute("style:class", "text");
        textBodyStyleOpenElement.write(pHandler);

    pHandler->endElement("style:style");

        TagOpenElement tableContentsStyleOpenElement("style:style");
    tableContentsStyleOpenElement.addAttribute("style:name", "Table_Contents");
    tableContentsStyleOpenElement.addAttribute("style:display-name", "Table Contents");
        tableContentsStyleOpenElement.addAttribute("style:family", "paragraph");
    tableContentsStyleOpenElement.addAttribute("style:parent-style-name", "Text_Body");
        tableContentsStyleOpenElement.addAttribute("style:class", "extra");
        tableContentsStyleOpenElement.write(pHandler);

    pHandler->endElement("style:style");

        TagOpenElement tableHeadingStyleOpenElement("style:style");
    tableHeadingStyleOpenElement.addAttribute("style:name", "Table_Heading");
    tableHeadingStyleOpenElement.addAttribute("style:display-name", "Table Heading");
        tableHeadingStyleOpenElement.addAttribute("style:family", "paragraph");
    tableHeadingStyleOpenElement.addAttribute("style:parent-style-name", "Table_Contents");
        tableHeadingStyleOpenElement.addAttribute("style:class", "extra");
        tableHeadingStyleOpenElement.write(pHandler);

    pHandler->endElement("style:style");

    for (std::vector<DocumentElement *>::const_iterator iter = mFrameStyles.begin();
        iter != mFrameStyles.end(); iter++)
        (*iter)->write(pHandler);

    pHandler->endElement("office:styles");
}

void DocumentCollector::_writeMasterPages(DocumentHandlerInterface *pHandler)
{
    TagOpenElement("office:master-styles").write(mpHandler);
    int pageNumber = 1;
    for (unsigned int i=0; i<mPageSpans.size(); i++)
    {
        bool bLastPage;
        (i == (mPageSpans.size() - 1)) ? bLastPage = true : bLastPage = false;
        mPageSpans[i]->writeMasterPages(pageNumber, i, bLastPage, pHandler);
        pageNumber += mPageSpans[i]->getSpan();
    }
    pHandler->endElement("office:master-styles");
}

void DocumentCollector::_writePageLayouts(DocumentHandlerInterface *pHandler)
{
    for (unsigned int i=0; i<mPageSpans.size(); i++)
    {
        mPageSpans[i]->writePageLayout(i, pHandler);
    }
}

bool DocumentCollector::_writeTargetDocument(DocumentHandlerInterface *pHandler)
{        
    WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Printing out the header stuff..\n"));

    WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Start Document\n"));
    mpHandler->startDocument();

    WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: preamble\n"));
        WPXPropertyList docContentPropList;
    docContentPropList.insert("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
    docContentPropList.insert("xmlns:meta", "urn:oasis:names:tc:opendocument:xmlns:meta:1.0");
    docContentPropList.insert("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    docContentPropList.insert("xmlns:config", "urn:oasis:names:tc:opendocument:xmlns:config:1.0");
    docContentPropList.insert("xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0");
    docContentPropList.insert("xmlns:table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0");
    docContentPropList.insert("xmlns:draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0");
    docContentPropList.insert("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
    docContentPropList.insert("xmlns:xlink", "http://www.w3.org/1999/xlink");
    docContentPropList.insert("xmlns:number", "http://openoffice.org/2000/datastyle");
    docContentPropList.insert("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
    docContentPropList.insert("xmlns:chart", "urn:oasis:names:tc:opendocument:xmlns:chart:1.0");
    docContentPropList.insert("xmlns:dr3d", "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0");
    docContentPropList.insert("xmlns:math", "http://www.w3.org/1998/Math/MathML");
    docContentPropList.insert("xmlns:form", "urn:oasis:names:tc:opendocument:xmlns:form:1.0");
    docContentPropList.insert("xmlns:script", "urn:oasis:names:tc:opendocument:xmlns:script:1.0");
    docContentPropList.insert("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
    docContentPropList.insert("office:version", "1.0");
    docContentPropList.insert("office:mimetype", "application/vnd.oasis.opendocument.text");
    mpHandler->startElement("office:document", docContentPropList);

    // write out the metadata
    TagOpenElement("office:meta").write(mpHandler);
    for (std::vector<DocumentElement *>::const_iterator iterMetaData = mMetaData.begin(); iterMetaData != mMetaData.end(); iterMetaData++) {
        (*iterMetaData)->write(mpHandler);
    }
    mpHandler->endElement("office:meta");

    // write out the font styles
    TagOpenElement("office:font-face-decls").write(mpHandler);
    for (std::map<WPXString, FontStyle *, ltstr>::iterator iterFont = mFontHash.begin(); iterFont != mFontHash.end(); iterFont++) {
        iterFont->second->write(mpHandler);
    }
    TagOpenElement symbolFontOpen("style:font-face");
    symbolFontOpen.addAttribute("style:name", "StarSymbol");
    symbolFontOpen.addAttribute("svg:font-family", "StarSymbol");
    symbolFontOpen.addAttribute("style:font-charset", "x-symbol");
    symbolFontOpen.write(mpHandler);
    mpHandler->endElement("style:font-face");

    mpHandler->endElement("office:font-face-decls");

     WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Writing out the styles..\n"));

    // write default styles
    _writeDefaultStyles(mpHandler);

    TagOpenElement("office:automatic-styles").write(mpHandler);

    for (std::vector<DocumentElement *>::const_iterator iterFrameAutomaticStyles = mFrameAutomaticStyles.begin();
        iterFrameAutomaticStyles != mFrameAutomaticStyles.end(); iterFrameAutomaticStyles++)
    {
        (*iterFrameAutomaticStyles)->write(pHandler);
    }

    for (std::map<WPXString, ParagraphStyle *, ltstr>::const_iterator iterTextStyle = mTextStyleHash.begin();
             iterTextStyle != mTextStyleHash.end(); iterTextStyle++) 
        {
        // writing out the paragraph styles
        if (strcmp((iterTextStyle->second)->getName().cstr(), "Standard")) 
                {
            // don't write standard paragraph "no styles" style
            (iterTextStyle->second)->write(pHandler);
        }
    }

        // span styles..
    for (std::map<WPXString, SpanStyle *, ltstr>::const_iterator iterSpanStyle = mSpanStyleHash.begin();
             iterSpanStyle != mSpanStyleHash.end(); iterSpanStyle++) 
        {
                (iterSpanStyle->second)->write(pHandler);
    }

     // writing out the sections styles
    for (std::vector<SectionStyle *>::const_iterator iterSectionStyles = mSectionStyles.begin(); iterSectionStyles != mSectionStyles.end(); iterSectionStyles++) {
        (*iterSectionStyles)->write(pHandler);
    }

    // writing out the lists styles
    for (std::vector<ListStyle *>::const_iterator iterListStyles = mListStyles.begin(); iterListStyles != mListStyles.end(); iterListStyles++) {
        (*iterListStyles)->write(pHandler);
    }

     // writing out the table styles
    for (std::vector<TableStyle *>::const_iterator iterTableStyles = mTableStyles.begin(); iterTableStyles != mTableStyles.end(); iterTableStyles++) {
        (*iterTableStyles)->write(pHandler);
    }

    // writing out the page masters
    _writePageLayouts(pHandler);


    pHandler->endElement("office:automatic-styles");

    _writeMasterPages(pHandler);

     WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Writing out the document..\n"));
     // writing out the document
    TagOpenElement("office:body").write(mpHandler);
    TagOpenElement("office:text").write(mpHandler);

    for (std::vector<DocumentElement *>::const_iterator iterBodyElements = mBodyElements.begin(); iterBodyElements != mBodyElements.end(); iterBodyElements++) {
        (*iterBodyElements)->write(pHandler);
    }
     WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Finished writing all doc els..\n"));

    pHandler->endElement("office:text");
    pHandler->endElement("office:body");
    pHandler->endElement("office:document");

    pHandler->endDocument();

    return true;
}


WPXString propListToStyleKey(const WPXPropertyList & xPropList)
{
        WPXString sKey;
        WPXPropertyList::Iter i(xPropList);
        for (i.rewind(); i.next(); )
        {
                WPXString sProp;
                sProp.sprintf("[%s:%s]", i.key(), i()->getStr().cstr());
                sKey.append(sProp);
        }

        return sKey;
}

WPXString getParagraphStyleKey(const WPXPropertyList & xPropList, const WPXPropertyListVector & xTabStops)
{
        WPXString sKey = propListToStyleKey(xPropList);
        
        WPXString sTabStops;
        sTabStops.sprintf("[num-tab-stops:%i]", xTabStops.count());
        WPXPropertyListVector::Iter i(xTabStops);
        for (i.rewind(); i.next();)
        {
                sTabStops.append(propListToStyleKey(i()));
        }
        sKey.append(sTabStops);

        return sKey;
}

// _allocateFontName: add a (potentially mapped) font style to the hash if it's not already there, do nothing otherwise
void DocumentCollector::_allocateFontName(const WPXString & sFontName)
{
    if (mFontHash.find(sFontName) == mFontHash.end())
    {
        FontStyle *pFontStyle = new FontStyle(sFontName.cstr(), sFontName.cstr());
        mFontHash[sFontName] = pFontStyle;
    }
}

void DocumentCollector::setDocumentMetaData(const WPXPropertyList &propList)
{
        WPXPropertyList::Iter i(propList);
        for (i.rewind(); i.next(); )
        {
                // filter out libwpd elements
                if (strncmp(i.key(), "libwpd", 6) != 0 && strncmp(i.key(), "dcterms", 7) != 0)
        {
            mMetaData.push_back(new TagOpenElement(i.key()));
            WPXString sStringValue(i()->getStr(), true);
            mMetaData.push_back(new CharDataElement(sStringValue.cstr()));
            mMetaData.push_back(new TagCloseElement(i.key()));
        }
        }

}

void DocumentCollector::openPageSpan(const WPXPropertyList &propList)
{
    PageSpan *pPageSpan = new PageSpan(propList);
    mPageSpans.push_back(pPageSpan);
    mpCurrentPageSpan = pPageSpan;
    miNumPageStyles++;

    mWriterDocumentStates.top().mbFirstParagraphInPageSpan = true;
}

void DocumentCollector::openHeader(const WPXPropertyList &propList)
{
    std::vector<DocumentElement *> * pHeaderFooterContentElements = new std::vector<DocumentElement *>;

    if (propList["libwpd:occurence"]->getStr() == "even")
                mpCurrentPageSpan->setHeaderLeftContent(pHeaderFooterContentElements);
        else
                mpCurrentPageSpan->setHeaderContent(pHeaderFooterContentElements);

    mpCurrentContentElements = pHeaderFooterContentElements;
}

void DocumentCollector::closeHeader()
{
    mpCurrentContentElements = &mBodyElements;
}

void DocumentCollector::openFooter(const WPXPropertyList &propList)
{
    std::vector<DocumentElement *> * pHeaderFooterContentElements = new std::vector<DocumentElement *>;

    if (propList["libwpd:occurence"]->getStr() == "even")
                mpCurrentPageSpan->setFooterLeftContent(pHeaderFooterContentElements);
        else
                mpCurrentPageSpan->setFooterContent(pHeaderFooterContentElements);

    mpCurrentContentElements = pHeaderFooterContentElements;
}

void DocumentCollector::closeFooter()
{
    mpCurrentContentElements = &mBodyElements;
}

void DocumentCollector::openSection(const WPXPropertyList &propList, const WPXPropertyListVector &columns)
{
        int iNumColumns = columns.count();
    double fSectionMarginLeft = 0.0;
    double fSectionMarginRight = 0.0;
    if (propList["fo:margin-left"])
        fSectionMarginLeft = propList["fo:margin-left"]->getDouble();
    if (propList["fo:margin-right"])
        fSectionMarginRight = propList["fo:margin-right"]->getDouble();

    if (iNumColumns > 1 || fSectionMarginLeft != 0 || fSectionMarginRight != 0)
    {
        if (propList["fo:margin-bottom"])
            mfSectionSpaceAfter = propList["fo:margin-bottom"]->getDouble();
        else if (propList["libwpd:margin-bottom"])
            mfSectionSpaceAfter =  propList["libwpd:margin-bottom"]->getDouble();

        WPXString sSectionName;
        sSectionName.sprintf("Section%i", mSectionStyles.size());
        
        SectionStyle *pSectionStyle = new SectionStyle(propList, columns, sSectionName.cstr());
        mSectionStyles.push_back(pSectionStyle);
        
        TagOpenElement *pSectionOpenElement = new TagOpenElement("text:section");
        pSectionOpenElement->addAttribute("text:style-name", pSectionStyle->getName());
        pSectionOpenElement->addAttribute("text:name", pSectionStyle->getName());
        mpCurrentContentElements->push_back(pSectionOpenElement);
    }
    else
        mWriterDocumentStates.top().mbInFakeSection = true;
}

void DocumentCollector::closeSection()
{
    if (!mWriterDocumentStates.top().mbInFakeSection)
        mpCurrentContentElements->push_back(new TagCloseElement("text:section"));
    else
        mWriterDocumentStates.top().mbInFakeSection = false;

    mfSectionSpaceAfter = 0.0;
}

void DocumentCollector::openParagraph(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops)
{
    // FIXMENOW: What happens if we open a footnote inside a table? do we then inherit the footnote's style
    // from "Table Contents"
    
    WPXPropertyList *pPersistPropList = new WPXPropertyList(propList);
    ParagraphStyle *pStyle = NULL;

    if (mWriterDocumentStates.top().mbFirstElement && mpCurrentContentElements == &mBodyElements)
    {
        // we don't have to go through the fuss of determining if the paragraph style is 
        // unique in this case, because if we are the first document element, then we
        // are singular. Neither do we have to determine what our parent style is-- we can't
        // be inside a table in this case (the table would be the first document element 
        //in that case)
        pPersistPropList->insert("style:parent-style-name", "Standard");
        WPXString sName;
        sName.sprintf("FS");

        WPXString sParagraphHashKey("P|FS");
        pPersistPropList->insert("style:master-page-name", "Page_Style_1");
                pStyle = new ParagraphStyle(pPersistPropList, tabStops, sName);
        mTextStyleHash[sParagraphHashKey] = pStyle;
        mWriterDocumentStates.top().mbFirstElement = false;
        mWriterDocumentStates.top().mbFirstParagraphInPageSpan = false;
     }
    else
    {
        if (mWriterDocumentStates.top().mbFirstParagraphInPageSpan && mpCurrentContentElements == &mBodyElements)
        {
            WPXString sPageStyleName;
            sPageStyleName.sprintf("Page_Style_%i", miNumPageStyles);
            pPersistPropList->insert("style:master-page-name", sPageStyleName);
            mWriterDocumentStates.top().mbFirstParagraphInPageSpan = false;
        }

        if (mWriterDocumentStates.top().mbTableCellOpened)
        {
            if (mWriterDocumentStates.top().mbHeaderRow)
                pPersistPropList->insert("style:parent-style-name", "Table_Heading");
            else
                pPersistPropList->insert("style:parent-style-name", "Table_Contents");
        }
        else
            pPersistPropList->insert("style:parent-style-name", "Standard");

                WPXString sKey = getParagraphStyleKey(*pPersistPropList, tabStops);

        if (mTextStyleHash.find(sKey) == mTextStyleHash.end())
        {
            WPXString sName;
            sName.sprintf("S%i", mTextStyleHash.size()); 
            
            pStyle = new ParagraphStyle(pPersistPropList, tabStops, sName);
    
            mTextStyleHash[sKey] = pStyle;
        }
        else
        {
            pStyle = mTextStyleHash[sKey];
            delete pPersistPropList;
        }
    }
    // create a document element corresponding to the paragraph, and append it to our list of document elements
    TagOpenElement *pParagraphOpenElement = new TagOpenElement("text:p");
    pParagraphOpenElement->addAttribute("text:style-name", pStyle->getName());
    mpCurrentContentElements->push_back(pParagraphOpenElement);
}

void DocumentCollector::closeParagraph()
{
    mpCurrentContentElements->push_back(new TagCloseElement("text:p"));
}

void DocumentCollector::openSpan(const WPXPropertyList &propList)
{
        if (propList["style:font-name"])
                _allocateFontName(propList["style:font-name"]->getStr());
    WPXString sSpanHashKey = propListToStyleKey(propList);
    WRITER_DEBUG_MSG(("WriterWordPerfect: Span Hash Key: %s\n", sSpanHashKey.cstr()));

    // Get the style
        WPXString sName;
    if (mSpanStyleHash.find(sSpanHashKey) == mSpanStyleHash.end())
        {
        // allocate a new paragraph style
        sName.sprintf("Span%i", mSpanStyleHash.size());
        SpanStyle *pStyle = new SpanStyle(sName.cstr(), propList);                

        mSpanStyleHash[sSpanHashKey] = pStyle;
    }
    else 
        {
        sName.sprintf("%s", mSpanStyleHash.find(sSpanHashKey)->second->getName().cstr());
    }

    // create a document element corresponding to the paragraph, and append it to our list of document elements
    TagOpenElement *pSpanOpenElement = new TagOpenElement("text:span");
    pSpanOpenElement->addAttribute("text:style-name", sName.cstr());
    mpCurrentContentElements->push_back(pSpanOpenElement);
}

void DocumentCollector::closeSpan()
{
    mpCurrentContentElements->push_back(new TagCloseElement("text:span"));
}

void DocumentCollector::defineOrderedListLevel(const WPXPropertyList &propList)
{
        int id = 0;
        if (propList["libwpd:id"])
                id = propList["libwpd:id"]->getInt();

     OrderedListStyle *pOrderedListStyle = NULL;
    if (mWriterListStates.top().mpCurrentListStyle && mWriterListStates.top().mpCurrentListStyle->getListID() == id)
        pOrderedListStyle = static_cast<OrderedListStyle *>(mWriterListStates.top().mpCurrentListStyle); // FIXME: using a dynamic cast here causes oo to crash?!

    // this rather appalling conditional makes sure we only start a new list (rather than continue an old
    // one) if: (1) we have no prior list OR (2) the prior list is actually definitively different
    // from the list that is just being defined (listIDs differ) OR (3) we can tell that the user actually
    // is starting a new list at level 1 (and only level 1)
    if (pOrderedListStyle == NULL || pOrderedListStyle->getListID() != id  ||
        (propList["libwpd:level"] && propList["libwpd:level"]->getInt()==1 && 
         (propList["text:start-value"] && propList["text:start-value"]->getInt() != (mWriterListStates.top().miLastListNumber+1))))
    {
        WRITER_DEBUG_MSG(("Attempting to create a new ordered list style (listid: %i)\n", id));
        WPXString sName;
        sName.sprintf("OL%i", miNumListStyles);
        miNumListStyles++;
        pOrderedListStyle = new OrderedListStyle(sName.cstr(), id);
        mListStyles.push_back(pOrderedListStyle);
        mWriterListStates.top().mpCurrentListStyle = pOrderedListStyle;
        mWriterListStates.top().mbListContinueNumbering = false;
        mWriterListStates.top().miLastListNumber = 0;
    }
    else
        mWriterListStates.top().mbListContinueNumbering = true;

    // Iterate through ALL list styles with the same WordPerfect list id and define a level if it is not already defined
    // This solves certain problems with lists that start and finish without reaching certain levels and then begin again
    // and reach those levels. See gradguide0405_PC.wpd in the regression suite
    for (std::vector<ListStyle *>::iterator iterOrderedListStyles = mListStyles.begin(); iterOrderedListStyles != mListStyles.end(); iterOrderedListStyles++)
    {
        if ((* iterOrderedListStyles)->getListID() == id)
            (* iterOrderedListStyles)->updateListLevel((propList["libwpd:level"]->getInt() - 1), propList);
    }
}

void DocumentCollector::defineUnorderedListLevel(const WPXPropertyList &propList)
{
        int id = 0;
        if (propList["libwpd:id"])
                id = propList["libwpd:id"]->getInt();

     UnorderedListStyle *pUnorderedListStyle = NULL;
    if (mWriterListStates.top().mpCurrentListStyle && mWriterListStates.top().mpCurrentListStyle->getListID() == id)
        pUnorderedListStyle = static_cast<UnorderedListStyle *>(mWriterListStates.top().mpCurrentListStyle); // FIXME: using a dynamic cast here causes oo to crash?!

    if (pUnorderedListStyle == NULL) {
        WRITER_DEBUG_MSG(("Attempting to create a new unordered list style (listid: %i)\n", id));
        WPXString sName;
        sName.sprintf("UL%i", miNumListStyles);
        miNumListStyles++;
        pUnorderedListStyle = new UnorderedListStyle(sName.cstr(), id);
        mListStyles.push_back(pUnorderedListStyle);
        mWriterListStates.top().mpCurrentListStyle = pUnorderedListStyle;
    }

    // See comment in DocumentCollector::defineOrderedListLevel
    for (std::vector<ListStyle *>::iterator iterUnorderedListStyles = mListStyles.begin(); iterUnorderedListStyles != mListStyles.end(); iterUnorderedListStyles++)
    {
        if ((* iterUnorderedListStyles)->getListID() == id)
            (* iterUnorderedListStyles)->updateListLevel((propList["libwpd:level"]->getInt() - 1), propList);
    }
}

void DocumentCollector::openOrderedListLevel(const WPXPropertyList & /* propList */)
{
    if (mWriterListStates.top().mbListElementParagraphOpened)
    {
        mpCurrentContentElements->push_back(new TagCloseElement("text:p"));
        mWriterListStates.top().mbListElementParagraphOpened = false;
    }
    TagOpenElement *pListLevelOpenElement = new TagOpenElement("text:list");
    _openListLevel(pListLevelOpenElement);

    if (mWriterListStates.top().mbListContinueNumbering) {
        pListLevelOpenElement->addAttribute("text:continue-numbering", "true");
    }

    mpCurrentContentElements->push_back(pListLevelOpenElement);
}

void DocumentCollector::openUnorderedListLevel(const WPXPropertyList & /* propList */)
{
    if (mWriterListStates.top().mbListElementParagraphOpened)
    {
        mpCurrentContentElements->push_back(new TagCloseElement("text:p"));
        mWriterListStates.top().mbListElementParagraphOpened = false;
    }
    TagOpenElement *pListLevelOpenElement = new TagOpenElement("text:list");
    _openListLevel(pListLevelOpenElement);

    mpCurrentContentElements->push_back(pListLevelOpenElement);
}

void DocumentCollector::_openListLevel(TagOpenElement *pListLevelOpenElement)
{
    if (!mWriterListStates.top().mbListElementOpened.empty() &&
        !mWriterListStates.top().mbListElementOpened.top())
    {
        mpCurrentContentElements->push_back(new TagOpenElement("text:list-item"));
        mWriterListStates.top().mbListElementOpened.top() = true;
    }

    mWriterListStates.top().mbListElementOpened.push(false);
    if (mWriterListStates.top().mbListElementOpened.size() == 1) {
        pListLevelOpenElement->addAttribute("text:style-name", mWriterListStates.top().mpCurrentListStyle->getName());
    }
}

void DocumentCollector::closeOrderedListLevel()
{
    _closeListLevel();
}

void DocumentCollector::closeUnorderedListLevel()
{
    _closeListLevel();
}

void DocumentCollector::_closeListLevel()
{
    if (mWriterListStates.top().mbListElementOpened.top())
    {
        mpCurrentContentElements->push_back(new TagCloseElement("text:list-item"));
        mWriterListStates.top().mbListElementOpened.top() = false;
    }

    mpCurrentContentElements->push_back(new TagCloseElement("text:list"));

    if (!mWriterListStates.top().mbListElementOpened.empty())
    {
        mWriterListStates.top().mbListElementOpened.pop();
    }
}

void DocumentCollector::openListElement(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops)
{
    mWriterListStates.top().miLastListLevel = mWriterListStates.top().miCurrentListLevel;
    if (mWriterListStates.top().miCurrentListLevel == 1)
        mWriterListStates.top().miLastListNumber++;

    if (mWriterListStates.top().mbListElementOpened.top())
    {
        mpCurrentContentElements->push_back(new TagCloseElement("text:list-item"));
        mWriterListStates.top().mbListElementOpened.top() = false;
    }

    ParagraphStyle *pStyle = NULL;

    WPXPropertyList *pPersistPropList = new WPXPropertyList(propList);
    pPersistPropList->insert("style:list-style-name", mWriterListStates.top().mpCurrentListStyle->getName());
    pPersistPropList->insert("style:parent-style-name", "Standard");

        WPXString sKey = getParagraphStyleKey(*pPersistPropList, tabStops);

        if (mTextStyleHash.find(sKey) == mTextStyleHash.end()) 
        {
                WPXString sName;
                sName.sprintf("S%i", mTextStyleHash.size()); 
        
                pStyle = new ParagraphStyle(pPersistPropList, tabStops, sName);
                
                mTextStyleHash[sKey] = pStyle;
        }
        else
        {
                pStyle = mTextStyleHash[sKey];
                delete pPersistPropList;
        }

    mpCurrentContentElements->push_back(new TagOpenElement("text:list-item"));

    TagOpenElement *pOpenListElementParagraph = new TagOpenElement("text:p");
    pOpenListElementParagraph->addAttribute("text:style-name", pStyle->getName());
    mpCurrentContentElements->push_back(pOpenListElementParagraph);

    if (mpCurrentContentElements == &mBodyElements)
        mWriterDocumentStates.top().mbFirstParagraphInPageSpan = false;
        
    mWriterListStates.top().mbListElementOpened.top() = true;
    mWriterListStates.top().mbListElementParagraphOpened = true;
    mWriterListStates.top().mbListContinueNumbering = false;
}

void DocumentCollector::closeListElement()
{
    // this code is kind of tricky, because we don't actually close the list element (because this list element
    // could contain another list level in OOo's implementation of lists). that is done in the closeListLevel
    // code (or when we open another list element)

    if (mWriterListStates.top().mbListElementParagraphOpened)
    {
        mpCurrentContentElements->push_back(new TagCloseElement("text:p"));
        mWriterListStates.top().mbListElementParagraphOpened = false;
    }
}

void DocumentCollector::openFootnote(const WPXPropertyList &propList)
{
    mWriterListStates.push(WriterListState());
    TagOpenElement *pOpenFootNote = new TagOpenElement("text:note");
    pOpenFootNote->addAttribute("text:note-class", "footnote");
    if (propList["libwpd:number"])
    {
        WPXString tmpString("ftn");
        tmpString.append(propList["libwpd:number"]->getStr());
        pOpenFootNote->addAttribute("text:id", tmpString);
    }
    mpCurrentContentElements->push_back(pOpenFootNote);

    mpCurrentContentElements->push_back(new TagOpenElement("text:note-citation"));
        if (propList["libwpd:number"])
    mpCurrentContentElements->push_back(new CharDataElement(propList["libwpd:number"]->getStr().cstr()));
    mpCurrentContentElements->push_back(new TagCloseElement("text:note-citation"));

    mpCurrentContentElements->push_back(new TagOpenElement("text:note-body"));
    
    mWriterDocumentStates.top().mbInNote = true;
}

void DocumentCollector::closeFootnote()
{
    mWriterDocumentStates.top().mbInNote = false;
    if (mWriterListStates.size() > 1)
        mWriterListStates.pop();

    mpCurrentContentElements->push_back(new TagCloseElement("text:note-body"));
    mpCurrentContentElements->push_back(new TagCloseElement("text:note"));
}

void DocumentCollector::openEndnote(const WPXPropertyList &propList)
{
    mWriterListStates.push(WriterListState());
    TagOpenElement *pOpenEndNote = new TagOpenElement("text:note");
    pOpenEndNote->addAttribute("text:note-class", "endnote");
    if (propList["libwpd:number"])
    {
        WPXString tmpString("edn");
        tmpString.append(propList["libwpd:number"]->getStr());
        pOpenEndNote->addAttribute("text:id", tmpString);
    }
    mpCurrentContentElements->push_back(pOpenEndNote);

    mpCurrentContentElements->push_back(new TagOpenElement("text:note-citation"));
        if (propList["libwpd:number"])
    mpCurrentContentElements->push_back(new CharDataElement(propList["libwpd:number"]->getStr().cstr()));
    mpCurrentContentElements->push_back(new TagCloseElement("text:note-citation"));

    mpCurrentContentElements->push_back(new TagOpenElement("text:note-body"));

    mWriterDocumentStates.top().mbInNote = true;
}

void DocumentCollector::closeEndnote()
{
    mWriterDocumentStates.top().mbInNote = false;
    if (mWriterListStates.size() > 1)
        mWriterListStates.pop();

    mpCurrentContentElements->push_back(new TagCloseElement("text:note-body"));
    mpCurrentContentElements->push_back(new TagCloseElement("text:note"));
}

void DocumentCollector::openComment(const WPXPropertyList & /*propList*/)
{
    mWriterListStates.push(WriterListState());
    mpCurrentContentElements->push_back(new TagOpenElement("office:annotation"));

    mWriterDocumentStates.top().mbInNote = true;
}

void DocumentCollector::closeComment()
{
    mWriterDocumentStates.top().mbInNote = false;
    if (mWriterListStates.size() > 1)
        mWriterListStates.pop();

    mpCurrentContentElements->push_back(new TagCloseElement("office:annotation"));
}

void DocumentCollector::openTable(const WPXPropertyList &propList, const WPXPropertyListVector &columns)
{
    if (!mWriterDocumentStates.top().mbInNote)
    {
    WPXString sTableName;
    sTableName.sprintf("Table%i", mTableStyles.size());

    // FIXME: we base the table style off of the page's margin left, ignoring (potential) wordperfect margin
    // state which is transmitted inside the page. could this lead to unacceptable behaviour?
        // WLACH_REFACTORING: characterize this behaviour, probably should nip it at the bud within libwpd
    TableStyle *pTableStyle = new TableStyle(propList, columns, sTableName.cstr());

    if (mWriterDocumentStates.top().mbFirstElement && mpCurrentContentElements == &mBodyElements)
    {
        WPXString sMasterPageName("Page_Style_1");
        pTableStyle->setMasterPageName(sMasterPageName);
        mWriterDocumentStates.top().mbFirstElement = false;
    }

    mTableStyles.push_back(pTableStyle);

    mpCurrentTableStyle = pTableStyle;

    TagOpenElement *pTableOpenElement = new TagOpenElement("table:table");

    pTableOpenElement->addAttribute("table:name", sTableName.cstr());
    pTableOpenElement->addAttribute("table:style-name", sTableName.cstr());
    mpCurrentContentElements->push_back(pTableOpenElement);

    for (int i=0; i<pTableStyle->getNumColumns(); i++) 
        {
        TagOpenElement *pTableColumnOpenElement = new TagOpenElement("table:table-column");
        WPXString sColumnStyleName;
        sColumnStyleName.sprintf("%s.Column%i", sTableName.cstr(), (i+1));
        pTableColumnOpenElement->addAttribute("table:style-name", sColumnStyleName.cstr());
        mpCurrentContentElements->push_back(pTableColumnOpenElement);

        TagCloseElement *pTableColumnCloseElement = new TagCloseElement("table:table-column");
        mpCurrentContentElements->push_back(pTableColumnCloseElement);
    }
}
}

void DocumentCollector::openTableRow(const WPXPropertyList &propList)
{
    if (!mWriterDocumentStates.top().mbInNote)
    {
    if (propList["libwpd:is-header-row"] && (propList["libwpd:is-header-row"]->getInt()))
    {
        mpCurrentContentElements->push_back(new TagOpenElement("table:table-header-rows"));
        mWriterDocumentStates.top().mbHeaderRow = true;
    }

    WPXString sTableRowStyleName;
    sTableRowStyleName.sprintf("%s.Row%i", mpCurrentTableStyle->getName().cstr(), mpCurrentTableStyle->getNumTableRowStyles());
    TableRowStyle *pTableRowStyle = new TableRowStyle(propList, sTableRowStyleName.cstr());
    mpCurrentTableStyle->addTableRowStyle(pTableRowStyle);
    
    TagOpenElement *pTableRowOpenElement = new TagOpenElement("table:table-row");
    pTableRowOpenElement->addAttribute("table:style-name", sTableRowStyleName);
    mpCurrentContentElements->push_back(pTableRowOpenElement);
    }
}

void DocumentCollector::closeTableRow()
{
    if (!mWriterDocumentStates.top().mbInNote)
    {
        mpCurrentContentElements->push_back(new TagCloseElement("table:table-row"));
        if (mWriterDocumentStates.top().mbHeaderRow)
        {
            mpCurrentContentElements->push_back(new TagCloseElement("table:table-header-rows"));
            mWriterDocumentStates.top().mbHeaderRow = false;
        }
    }
}

void DocumentCollector::openTableCell(const WPXPropertyList &propList)
{
    if (!mWriterDocumentStates.top().mbInNote)
    {
    WPXString sTableCellStyleName;
    sTableCellStyleName.sprintf( "%s.Cell%i", mpCurrentTableStyle->getName().cstr(), mpCurrentTableStyle->getNumTableCellStyles());
    TableCellStyle *pTableCellStyle = new TableCellStyle(propList, sTableCellStyleName.cstr());
    mpCurrentTableStyle->addTableCellStyle(pTableCellStyle);

    TagOpenElement *pTableCellOpenElement = new TagOpenElement("table:table-cell");
    pTableCellOpenElement->addAttribute("table:style-name", sTableCellStyleName);
    if (propList["table:number-columns-spanned"])
                pTableCellOpenElement->addAttribute("table:number-columns-spanned", 
                                                    propList["table:number-columns-spanned"]->getStr().cstr());
        if (propList["table:number-rows-spanned"])
                pTableCellOpenElement->addAttribute("table:number-rows-spanned",
                                                    propList["table:number-rows-spanned"]->getStr().cstr());
        // pTableCellOpenElement->addAttribute("table:value-type", "string");
        mpCurrentContentElements->push_back(pTableCellOpenElement);

        mWriterDocumentStates.top().mbTableCellOpened = true;
    }
}

void DocumentCollector::closeTableCell()
{
    if (!mWriterDocumentStates.top().mbInNote)
    {
        mpCurrentContentElements->push_back(new TagCloseElement("table:table-cell"));
        mWriterDocumentStates.top().mbTableCellOpened = false;
    }
}

void DocumentCollector::insertCoveredTableCell(const WPXPropertyList & /* propList */)
{
    if (!mWriterDocumentStates.top().mbInNote)
    {
        mpCurrentContentElements->push_back(new TagOpenElement("table:covered-table-cell"));
        mpCurrentContentElements->push_back(new TagCloseElement("table:covered-table-cell"));
    }
}

void DocumentCollector::closeTable()
{
    if (!mWriterDocumentStates.top().mbInNote)
    {
        mpCurrentContentElements->push_back(new TagCloseElement("table:table"));
    }
}


void DocumentCollector::insertTab()
{
    mpCurrentContentElements->push_back(new TagOpenElement("text:tab"));
    mpCurrentContentElements->push_back(new TagCloseElement("text:tab"));
}

void DocumentCollector::insertSpace()
{
    mpCurrentContentElements->push_back(new TagOpenElement("text:s"));
    mpCurrentContentElements->push_back(new TagCloseElement("text:s"));
}

void DocumentCollector::insertLineBreak()
{
    mpCurrentContentElements->push_back(new TagOpenElement("text:line-break"));
    mpCurrentContentElements->push_back(new TagCloseElement("text:line-break"));
}

void DocumentCollector::insertField(const WPXString &type, const WPXPropertyList &propList)
{
    if (!type.len())
        return;

    TagOpenElement *openElement = new TagOpenElement(type.cstr());
    if (type == "text:page-number")
        openElement->addAttribute("text:select-page", "current");

    if (propList["style:num-format"])
        openElement->addAttribute("style:num-format", propList["style:num-format"]->getStr());

    mpCurrentContentElements->push_back(openElement);
    mpCurrentContentElements->push_back(new TagCloseElement(type.cstr()));
}

void DocumentCollector::insertText(const WPXString &text)
{
    DocumentElement *pText = new TextElement(text);
    mpCurrentContentElements->push_back(pText);
}

void DocumentCollector::openFrame(const WPXPropertyList &propList)
{
    mWriterListStates.push(WriterListState());

    // First, let's create a Frame Style for this box
    TagOpenElement *frameStyleOpenElement = new TagOpenElement("style:style");
    WPXString frameStyleName;
    frameStyleName.sprintf("GraphicFrame_%i", miObjectNumber);
    frameStyleOpenElement->addAttribute("style:name", frameStyleName);
    frameStyleOpenElement->addAttribute("style:family", "graphic");

    mFrameStyles.push_back(frameStyleOpenElement);

    TagOpenElement *frameStylePropertiesOpenElement = new TagOpenElement("style:graphic-properties");

    if (propList["text:anchor-type"])
        frameStylePropertiesOpenElement->addAttribute("text:anchor-type", propList["text:anchor-type"]->getStr());
    else
        frameStylePropertiesOpenElement->addAttribute("text:anchor-type","paragraph");

    if (propList["text:anchor-page-number"])
        frameStylePropertiesOpenElement->addAttribute("text:anchor-page-number", propList["text:anchor-page-number"]->getStr());

    if (propList["svg:x"])
        frameStylePropertiesOpenElement->addAttribute("svg:x", propList["svg:x"]->getStr());

    if (propList["svg:y"])
        frameStylePropertiesOpenElement->addAttribute("svg:y", propList["svg:y"]->getStr());

    if (propList["svg:width"])
        frameStylePropertiesOpenElement->addAttribute("svg:width", propList["svg:width"]->getStr());

    if (propList["svg:height"])
        frameStylePropertiesOpenElement->addAttribute("svg:height", propList["svg:height"]->getStr());

    if (propList["style:rel-width"])
        frameStylePropertiesOpenElement->addAttribute("style:rel-width", propList["style:rel-width"]->getStr());

    if (propList["style:rel-height"])
        frameStylePropertiesOpenElement->addAttribute("style:rel-height", propList["style:rel-height"]->getStr());

    if (propList["fo:max-width"])
        frameStylePropertiesOpenElement->addAttribute("fo:max-width", propList["fo:max-width"]->getStr());

    if (propList["fo:max-height"])
        frameStylePropertiesOpenElement->addAttribute("fo:max-height", propList["fo:max-height"]->getStr());

    if (propList["style:wrap"])
        frameStylePropertiesOpenElement->addAttribute("style:wrap", propList["style:wrap"]->getStr());

    mFrameStyles.push_back(frameStylePropertiesOpenElement);

    mFrameStyles.push_back(new TagCloseElement("style:graphic-properties"));

    mFrameStyles.push_back(new TagCloseElement("style:style"));

    // Now, let's create an automatic style for this frame
    TagOpenElement *frameAutomaticStyleElement = new TagOpenElement("style:style");
    WPXString frameAutomaticStyleName;
    frameAutomaticStyleName.sprintf("fr%i", miObjectNumber);
    frameAutomaticStyleElement->addAttribute("style:name", frameAutomaticStyleName);
    frameAutomaticStyleElement->addAttribute("style:family", "graphic");
    frameAutomaticStyleElement->addAttribute("style:parent-style-name", frameStyleName);

    mFrameAutomaticStyles.push_back(frameAutomaticStyleElement);

    TagOpenElement *frameAutomaticStylePropertiesElement = new TagOpenElement("style:graphic-properties");
    if (propList["style:horizontal-pos"])
        frameAutomaticStylePropertiesElement->addAttribute("style:horizontal-pos", propList["style:horizontal-pos"]->getStr());
    else
        frameAutomaticStylePropertiesElement->addAttribute("style:horizontal-pos", "left");

    if (propList["style:horizontal-rel"])
        frameAutomaticStylePropertiesElement->addAttribute("style:horizontal-rel", propList["style:horizontal-rel"]->getStr());
    else
        frameAutomaticStylePropertiesElement->addAttribute("style:horizontal-rel", "paragraph");

    if (propList["style:vertical-pos"])
        frameAutomaticStylePropertiesElement->addAttribute("style:vertical-pos", propList["style:vertical-pos"]->getStr());
    else
        frameAutomaticStylePropertiesElement->addAttribute("style:vertical-pos", "top");

    if (propList["style:vertical-rel"])
        frameAutomaticStylePropertiesElement->addAttribute("style:vertical-rel", propList["style:vertical-rel"]->getStr());
    else
        frameAutomaticStylePropertiesElement->addAttribute("style:vertical-rel", "page-content");

    if (propList["fo:max-width"])
        frameAutomaticStylePropertiesElement->addAttribute("fo:max-width", propList["fo:max-width"]->getStr());

    if (propList["fo:max-height"])
        frameAutomaticStylePropertiesElement->addAttribute("fo:max-height", propList["fo:max-height"]->getStr());

    frameAutomaticStylePropertiesElement->addAttribute("draw:ole-draw-aspect", "1");

    mFrameAutomaticStyles.push_back(frameAutomaticStylePropertiesElement);

    mFrameAutomaticStyles.push_back(new TagCloseElement("style:graphic-properties"));

    mFrameAutomaticStyles.push_back(new TagCloseElement("style:style"));

    // And write the frame itself
    TagOpenElement *drawFrameOpenElement = new TagOpenElement("draw:frame");

    drawFrameOpenElement->addAttribute("draw:style-name", frameAutomaticStyleName);
    WPXString objectName;
    objectName.sprintf("Object%i", miObjectNumber++);
    drawFrameOpenElement->addAttribute("draw:name", objectName);
    if (propList["text:anchor-type"])
        drawFrameOpenElement->addAttribute("text:anchor-type", propList["text:anchor-type"]->getStr());
    else
        drawFrameOpenElement->addAttribute("text:anchor-type","paragraph");

    if (propList["text:anchor-page-number"])
        drawFrameOpenElement->addAttribute("text:anchor-page-number", propList["text:anchor-page-number"]->getStr());

    if (propList["svg:x"])
        drawFrameOpenElement->addAttribute("svg:x", propList["svg:x"]->getStr());

    if (propList["svg:y"])
        drawFrameOpenElement->addAttribute("svg:y", propList["svg:y"]->getStr());

    if (propList["svg:width"])
        drawFrameOpenElement->addAttribute("svg:width", propList["svg:width"]->getStr());

    if (propList["svg:height"])
        drawFrameOpenElement->addAttribute("svg:height", propList["svg:height"]->getStr());

    if (propList["style:rel-width"])
        drawFrameOpenElement->addAttribute("style:rel-width", propList["style:rel-width"]->getStr());

    if (propList["style:rel-height"])
        drawFrameOpenElement->addAttribute("style:rel-height", propList["style:rel-height"]->getStr());

    mpCurrentContentElements->push_back(drawFrameOpenElement);

    mWriterDocumentStates.top().mbInFrame = true;
}

void DocumentCollector::closeFrame()
{
    if (mWriterListStates.size() > 1)
        mWriterListStates.pop();

    mpCurrentContentElements->push_back(new TagCloseElement("draw:frame"));

    mWriterDocumentStates.top().mbInFrame = false;
}

void DocumentCollector::insertBinaryObject(const WPXPropertyList &propList, const WPXBinaryData &data)
{
    if (!data.size())
        return;
    if (!mWriterDocumentStates.top().mbInFrame) // Embedded objects without a frame simply don't make sense for us
        return;
    if (!propList["libwpd:mimetype"])
        return;

    if (propList["libwpd:mimetype"]->getStr() == "image/x-wpg")
    {
        std::vector<DocumentElement *> tmpContentElements;
        InternalHandler tmpHandler(&tmpContentElements);
        OdgExporter exporter(&tmpHandler);

        libwpg::WPGFileFormat fileFormat = libwpg::WPG_AUTODETECT;

        if (!libwpg::WPGraphics::isSupported(const_cast<WPXInputStream *>(data.getDataStream())))
            fileFormat = libwpg::WPG_WPG1;

        if (libwpg::WPGraphics::parse(const_cast<WPXInputStream *>(data.getDataStream()), &exporter, fileFormat) && !tmpContentElements.empty())
        {
            mpCurrentContentElements->push_back(new TagOpenElement("draw:object"));
            for (std::vector<DocumentElement *>::const_iterator iter = tmpContentElements.begin(); iter != tmpContentElements.end(); iter++)
                mpCurrentContentElements->push_back(*iter);
            mpCurrentContentElements->push_back(new TagCloseElement("draw:object"));
        }
    }
    else
    {
        mpCurrentContentElements->push_back(new TagOpenElement("draw:image"));

        mpCurrentContentElements->push_back(new TagOpenElement("office:binary-data"));

        WPXString binaryBase64Data = data.getBase64Data();

        mpCurrentContentElements->push_back(new CharDataElement(binaryBase64Data.cstr()));

        mpCurrentContentElements->push_back(new TagCloseElement("office:binary-data"));

        mpCurrentContentElements->push_back(new TagCloseElement("draw:image"));
    }
}

void DocumentCollector::openTextBox(const WPXPropertyList & /*propList*/)
{
    if (!mWriterDocumentStates.top().mbInFrame) // Text box without a frame simply doesn't make sense for us
        return;
    mWriterListStates.push(WriterListState());
    mWriterDocumentStates.push(WriterDocumentState());
    mpCurrentContentElements->push_back(new TagOpenElement("draw:text-box"));
    mWriterDocumentStates.top().mbInTextBox = true;
    mWriterDocumentStates.top().mbFirstElement = false;
}

void DocumentCollector::closeTextBox()
{
    if (!mWriterDocumentStates.top().mbInTextBox)
        return;
    if (mWriterListStates.size() > 1)
        mWriterListStates.pop();
    if (mWriterDocumentStates.size() > 1)
        mWriterDocumentStates.pop();

    mpCurrentContentElements->push_back(new TagCloseElement("draw:text-box"));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
