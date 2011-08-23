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
#include "TextRunStyle.hxx"
#include "FontStyle.hxx"
#include "ListStyle.hxx"
#include "PageSpan.hxx"
#include "SectionStyle.hxx"
#include "TableStyle.hxx"
#include "FilterInternal.hxx"
#include "WriterProperties.hxx"

_WriterDocumentState::_WriterDocumentState() :
    mbFirstElement(true),
    mbInFakeSection(false),
    mbListElementOpenedAtCurrentLevel(false),
    mbTableCellOpened(false),
    mbHeaderRow(false),
    mbInNote(false)
{
}

DocumentCollector::DocumentCollector(WPSInputStream *pInput, DocumentHandler *pHandler) :
        mpInput(pInput),
        mpHandler(pHandler),
    mbUsed(false),
    mfSectionSpaceAfter(0.0f),
    miNumListStyles(0),
    mpCurrentContentElements(&mBodyElements),
    mpCurrentPageSpan(NULL),
    miNumPageStyles(0),
    mpCurrentListStyle(NULL),
    miCurrentListLevel(0),
    miLastListLevel(0),
    miLastListNumber(0),
    mbListContinueNumbering(false),
    mbListElementOpened(false),
    mbListElementParagraphOpened(false)
{
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
        delete((*iterBody));
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
        delete iterTextStyle->second;
    }
    for (std::map<WPXString, SpanStyle *, ltstr>::iterator iterSpanStyle = mSpanStyleHash.begin(); iterSpanStyle != mSpanStyleHash.end(); iterSpanStyle++) {
        delete iterSpanStyle->second;
    }

    for (std::map<WPXString, FontStyle *, ltstr>::iterator iterFont = mFontHash.begin(); iterFont != mFontHash.end(); iterFont++) {
        delete iterFont->second;
    }

    for (std::vector<ListStyle *>::iterator iterListStyles = mListStyles.begin(); iterListStyles != mListStyles.end(); iterListStyles++) {
        delete (*iterListStyles);
    }
    for (std::vector<SectionStyle *>::iterator iterSectionStyles = mSectionStyles.begin(); iterSectionStyles != mSectionStyles.end(); iterSectionStyles++) {
        delete (*iterSectionStyles);
    }
    for (std::vector<TableStyle *>::iterator iterTableStyles = mTableStyles.begin(); iterTableStyles != mTableStyles.end(); iterTableStyles++) {
        delete (*iterTableStyles);
    }

    for (std::vector<PageSpan *>::iterator iterPageSpans = mPageSpans.begin(); iterPageSpans != mPageSpans.end(); iterPageSpans++) {
        delete (*iterPageSpans);
    }

     return true;
}

void DocumentCollector::_writeDefaultStyles(DocumentHandler *pHandler)
{
    TagOpenElement stylesOpenElement("office:styles");
    stylesOpenElement.write(pHandler);

    TagOpenElement defaultParagraphStyleOpenElement("style:default-style");
    defaultParagraphStyleOpenElement.addAttribute("style:family", "paragraph");
    defaultParagraphStyleOpenElement.write(pHandler);

    TagOpenElement defaultParagraphStylePropertiesOpenElement("style:properties");
    defaultParagraphStylePropertiesOpenElement.addAttribute("style:family", "paragraph");
    defaultParagraphStylePropertiesOpenElement.addAttribute("style:tab-stop-distance", "0.5inch");
    defaultParagraphStylePropertiesOpenElement.write(pHandler);
    TagCloseElement defaultParagraphStylePropertiesCloseElement("style:properties");
    defaultParagraphStylePropertiesCloseElement.write(pHandler);

    TagCloseElement defaultParagraphStyleCloseElement("style:default-style");
    defaultParagraphStyleCloseElement.write(pHandler);

    TagOpenElement standardStyleOpenElement("style:style");
        standardStyleOpenElement.addAttribute("style:name", "Standard");
        standardStyleOpenElement.addAttribute("style:family", "paragraph");
        standardStyleOpenElement.addAttribute("style:class", "text");
        standardStyleOpenElement.write(pHandler);
        TagCloseElement standardStyleCloseElement("style:style");
        standardStyleCloseElement.write(pHandler);

        TagOpenElement textBodyStyleOpenElement("style:style");
        textBodyStyleOpenElement.addAttribute("style:name", "Text Body");
        textBodyStyleOpenElement.addAttribute("style:family", "paragraph");
        textBodyStyleOpenElement.addAttribute("style:parent-style-name", "Standard");
        textBodyStyleOpenElement.addAttribute("style:class", "text");
        textBodyStyleOpenElement.write(pHandler);
        TagCloseElement textBodyStyleCloseElement("style:style");
        textBodyStyleCloseElement.write(pHandler);

        TagOpenElement tableContentsStyleOpenElement("style:style");
        tableContentsStyleOpenElement.addAttribute("style:name", "Table Contents");
        tableContentsStyleOpenElement.addAttribute("style:family", "paragraph");
        tableContentsStyleOpenElement.addAttribute("style:parent-style-name", "Text Body");
        tableContentsStyleOpenElement.addAttribute("style:class", "extra");
        tableContentsStyleOpenElement.write(pHandler);
        TagCloseElement tableContentsStyleCloseElement("style:style");
        tableContentsStyleCloseElement.write(pHandler);

        TagOpenElement tableHeadingStyleOpenElement("style:style");
        tableHeadingStyleOpenElement.addAttribute("style:name", "Table Heading");
        tableHeadingStyleOpenElement.addAttribute("style:family", "paragraph");
        tableHeadingStyleOpenElement.addAttribute("style:parent-style-name", "Table Contents");
        tableHeadingStyleOpenElement.addAttribute("style:class", "extra");
        tableHeadingStyleOpenElement.write(pHandler);
        TagCloseElement tableHeadingStyleCloseElement("style:style");
        tableHeadingStyleCloseElement.write(pHandler);

    TagCloseElement stylesCloseElement("office:styles");
    stylesCloseElement.write(pHandler);

}

void DocumentCollector::_writeMasterPages(DocumentHandler *pHandler)
{
        WPXPropertyList xBlankAttrList;

    pHandler->startElement("office:master-styles", xBlankAttrList);
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

void DocumentCollector::_writePageMasters(DocumentHandler *pHandler)
{
    for (unsigned int i=0; i<mPageSpans.size(); i++)
    {
        mPageSpans[i]->writePageMaster(i, pHandler);
    }
}

bool DocumentCollector::_writeTargetDocument(DocumentHandler *pHandler)
{        
    WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Printing out the header stuff..\n"));
    WPXPropertyList xBlankAttrList;

    WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Start Document\n"));
    mpHandler->startDocument();

    WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: preamble\n"));
        WPXPropertyList docContentPropList;
    docContentPropList.insert("xmlns:office", "http://openoffice.org/2000/office");
    docContentPropList.insert("xmlns:style", "http://openoffice.org/2000/style");
    docContentPropList.insert("xmlns:text", "http://openoffice.org/2000/text");
    docContentPropList.insert("xmlns:table", "http://openoffice.org/2000/table");
    docContentPropList.insert("xmlns:draw", "http://openoffice.org/2000/draw");
    docContentPropList.insert("xmlns:fo", "http://www.w3.org/1999/XSL/Format");
    docContentPropList.insert("xmlns:xlink", "http://www.w3.org/1999/xlink");
    docContentPropList.insert("xmlns:number", "http://openoffice.org/2000/datastyle");
    docContentPropList.insert("xmlns:svg", "http://www.w3.org/2000/svg");
    docContentPropList.insert("xmlns:chart", "http://openoffice.org/2000/chart");
    docContentPropList.insert("xmlns:dr3d", "http://openoffice.org/2000/dr3d");
    docContentPropList.insert("xmlns:math", "http://www.w3.org/1998/Math/MathML");
    docContentPropList.insert("xmlns:form", "http://openoffice.org/2000/form");
    docContentPropList.insert("xmlns:script", "http://openoffice.org/2000/script");
    docContentPropList.insert("office:class", "text");
    docContentPropList.insert("office:version", "1.0");
        mpHandler->startElement("office:document-content", docContentPropList);

    // write out the font styles
    mpHandler->startElement("office:font-decls", xBlankAttrList);
    for (std::map<WPXString, FontStyle *, ltstr>::iterator iterFont = mFontHash.begin(); iterFont != mFontHash.end(); iterFont++) {
        iterFont->second->write(mpHandler);
    }
    TagOpenElement symbolFontOpen("style:font-decl");
    symbolFontOpen.addAttribute("style:name", "StarSymbol");
    symbolFontOpen.addAttribute("fo:font-family", "StarSymbol");
    symbolFontOpen.addAttribute("style:font-charset", "x-symbol");
    symbolFontOpen.write(mpHandler);
        mpHandler->endElement("style:font-decl");

    mpHandler->endElement("office:font-decls");


     WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Writing out the styles..\n"));

    // write default styles
    _writeDefaultStyles(mpHandler);

    mpHandler->startElement("office:automatic-styles", xBlankAttrList);

    for (std::map<WPXString, ParagraphStyle *, ltstr>::iterator iterTextStyle = mTextStyleHash.begin(); 
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
    for (std::map<WPXString, SpanStyle *, ltstr>::iterator iterSpanStyle = mSpanStyleHash.begin(); 
             iterSpanStyle != mSpanStyleHash.end(); iterSpanStyle++) 
        {
                (iterSpanStyle->second)->write(pHandler);
    }

     // writing out the sections styles
    for (std::vector<SectionStyle *>::iterator iterSectionStyles = mSectionStyles.begin(); iterSectionStyles != mSectionStyles.end(); iterSectionStyles++) {
        (*iterSectionStyles)->write(pHandler);
    }

    // writing out the lists styles
    for (std::vector<ListStyle *>::iterator iterListStyles = mListStyles.begin(); iterListStyles != mListStyles.end(); iterListStyles++) {
        (*iterListStyles)->write(pHandler);
    }

     // writing out the table styles
    for (std::vector<TableStyle *>::iterator iterTableStyles = mTableStyles.begin(); iterTableStyles != mTableStyles.end(); iterTableStyles++) {
        (*iterTableStyles)->write(pHandler);
    }

    // writing out the page masters
    _writePageMasters(pHandler);


    pHandler->endElement("office:automatic-styles");

    _writeMasterPages(pHandler);

     WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Writing out the document..\n"));
     // writing out the document
    pHandler->startElement("office:body", xBlankAttrList);

    for (std::vector<DocumentElement *>::iterator iterBodyElements = mBodyElements.begin(); iterBodyElements != mBodyElements.end(); iterBodyElements++) {
        (*iterBodyElements)->write(pHandler);
    }
     WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Finished writing all doc els..\n"));

    pHandler->endElement("office:body");
    pHandler->endElement("office:document-content");

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

void DocumentCollector::openPageSpan(const WPXPropertyList &propList)
{
    PageSpan *pPageSpan = new PageSpan(propList);
    mPageSpans.push_back(pPageSpan);
    mpCurrentPageSpan = pPageSpan;
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
    float fSectionMarginLeft = 0.0f;
    float fSectionMarginRight = 0.0f;
    if (propList["fo:margin-left"])
        fSectionMarginLeft = propList["fo:margin-left"]->getFloat();
    if (propList["fo:margin-right"])
        fSectionMarginRight = propList["fo:margin-right"]->getFloat();

    if (iNumColumns > 1 || fSectionMarginLeft != 0 || fSectionMarginRight != 0)
    {
        mfSectionSpaceAfter = propList["fo:margin-bottom"]->getFloat();
        WPXString sSectionName;
        sSectionName.sprintf("Section%i", mSectionStyles.size());
        
        SectionStyle *pSectionStyle = new SectionStyle(propList, columns, sSectionName.cstr());
        mSectionStyles.push_back(pSectionStyle);
        
        TagOpenElement *pSectionOpenElement = new TagOpenElement("text:section");
        pSectionOpenElement->addAttribute("text:style-name", pSectionStyle->getName());
        pSectionOpenElement->addAttribute("text:name", pSectionStyle->getName());
        mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pSectionOpenElement));
    }
    else
        mWriterDocumentState.mbInFakeSection = true;
}

void DocumentCollector::closeSection()
{
    if (!mWriterDocumentState.mbInFakeSection)
        mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:section")));
    else
        mWriterDocumentState.mbInFakeSection = false;

    // open as many paragraphs as needed to simulate section space after
    // WLACH_REFACTORING: disable this for now..
    #if 0
    for (float f=0.0f; f<mfSectionSpaceAfter; f+=1.0f) {
        vector<WPXTabStop> dummyTabStops;
        openParagraph(WPX_PARAGRAPH_JUSTIFICATION_LEFT, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dummyTabStops, false, false);
        closeParagraph();
    }
    #endif
    mfSectionSpaceAfter = 0.0f;
}

void DocumentCollector::openParagraph(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops)
{
    // FIXMENOW: What happens if we open a footnote inside a table? do we then inherit the footnote's style
    // from "Table Contents"
    
    WPXPropertyList *pPersistPropList = new WPXPropertyList(propList);
    ParagraphStyle *pStyle = NULL;

    if (mWriterDocumentState.mbFirstElement && mpCurrentContentElements == &mBodyElements)
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
        pPersistPropList->insert("style:master-page-name", "Page Style 1");
                pStyle = new ParagraphStyle(pPersistPropList, tabStops, sName);
        mTextStyleHash[sParagraphHashKey] = pStyle;
        mWriterDocumentState.mbFirstElement = false;
     }
    else
    {
        if (mWriterDocumentState.mbTableCellOpened)
        {
            if (mWriterDocumentState.mbHeaderRow)
                pPersistPropList->insert("style:parent-style-name", "Table Heading");
            else
                pPersistPropList->insert("style:parent-style-name", "Table Contents");
        }
        else
            pPersistPropList->insert("style:parent-style-name", "Standard");

                WPXString sKey = getParagraphStyleKey(*pPersistPropList, tabStops);

        if (mTextStyleHash.find(sKey) == mTextStyleHash.end()) {
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
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pParagraphOpenElement));
}

void DocumentCollector::closeParagraph()
{
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:p")));
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
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pSpanOpenElement));
}

void DocumentCollector::closeSpan()
{
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:span")));
}

void DocumentCollector::defineOrderedListLevel(const WPXPropertyList &propList)
{
        int id = 0;
        if (propList["libwpd:id"])
                id = propList["libwpd:id"]->getInt();

     OrderedListStyle *pOrderedListStyle = NULL;
    if (mpCurrentListStyle && mpCurrentListStyle->getListID() == id)
        pOrderedListStyle = static_cast<OrderedListStyle *>(mpCurrentListStyle); // FIXME: using a dynamic cast here causes oo to crash?!

    // this rather appalling conditional makes sure we only start a new list (rather than continue an old
    // one) if: (1) we have no prior list OR (2) the prior list is actually definitively different
    // from the list that is just being defined (listIDs differ) OR (3) we can tell that the user actually
    // is starting a new list at level 1 (and only level 1)
    if (pOrderedListStyle == NULL || pOrderedListStyle->getListID() != id  ||
        (propList["libwpd:level"] && propList["libwpd:level"]->getInt()==1 && 
             (propList["text:start-value"] && (unsigned int)(propList["text:start-value"]->getInt()) != (miLastListNumber+1))))
    {
        WRITER_DEBUG_MSG(("Attempting to create a new ordered list style (listid: %i)\n", id));
        WPXString sName;
        sName.sprintf("OL%i", miNumListStyles);
        miNumListStyles++;
        pOrderedListStyle = new OrderedListStyle(sName.cstr(), propList["libwpd:id"]->getInt());
        mListStyles.push_back(static_cast<ListStyle *>(pOrderedListStyle));
        mpCurrentListStyle = static_cast<ListStyle *>(pOrderedListStyle);
        mbListContinueNumbering = false;
        miLastListNumber = 0;
    }
    else
        mbListContinueNumbering = true;

    // Iterate through ALL list styles with the same WordPerfect list id and define a level if it is not already defined
    // This solves certain problems with lists that start and finish without reaching certain levels and then begin again
    // and reach those levels. See gradguide0405_PC.wpd in the regression suite
    for (std::vector<ListStyle *>::iterator iterOrderedListStyles = mListStyles.begin(); iterOrderedListStyles != mListStyles.end(); iterOrderedListStyles++)
    {
        if ((* iterOrderedListStyles)->getListID() == propList["libwpd:id"]->getInt())
            (* iterOrderedListStyles)->updateListLevel((propList["libwpd:level"]->getInt() - 1), propList);
    }
}

void DocumentCollector::defineUnorderedListLevel(const WPXPropertyList &propList)
{
        int id = 0;
        if (propList["libwpd:id"])
                id = propList["libwpd:id"]->getInt();

     UnorderedListStyle *pUnorderedListStyle = NULL;
    if (mpCurrentListStyle && mpCurrentListStyle->getListID() == id)
        pUnorderedListStyle = static_cast<UnorderedListStyle *>(mpCurrentListStyle); // FIXME: using a dynamic cast here causes oo to crash?!

    if (pUnorderedListStyle == NULL) {
        WRITER_DEBUG_MSG(("Attempting to create a new unordered list style (listid: %i)\n", id));
        WPXString sName;
        sName.sprintf("UL%i", miNumListStyles);
        pUnorderedListStyle = new UnorderedListStyle(sName.cstr(), id);
        mListStyles.push_back(static_cast<ListStyle *>(pUnorderedListStyle));
        mpCurrentListStyle = static_cast<ListStyle *>(pUnorderedListStyle);
    }

    // See comment in DocumentCollector::defineOrderedListLevel
    for (std::vector<ListStyle *>::iterator iterUnorderedListStyles = mListStyles.begin(); iterUnorderedListStyles != mListStyles.end(); iterUnorderedListStyles++)
    {
        if ((* iterUnorderedListStyles)->getListID() == propList["libwpd:id"]->getInt())
            (* iterUnorderedListStyles)->updateListLevel((propList["libwpd:level"]->getInt() - 1), propList);
    }
}

void DocumentCollector::openOrderedListLevel(const WPXPropertyList & /* propList */)
{
    miCurrentListLevel++;
    TagOpenElement *pListLevelOpenElement = new TagOpenElement("text:ordered-list");
    _openListLevel(pListLevelOpenElement);

    if (mbListContinueNumbering) {
        pListLevelOpenElement->addAttribute("text:continue-numbering", "true");
    }

    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pListLevelOpenElement));
}

void DocumentCollector::openUnorderedListLevel(const WPXPropertyList & /* propList */)
{
    miCurrentListLevel++;
    TagOpenElement *pListLevelOpenElement = new TagOpenElement("text:unordered-list");
    _openListLevel(pListLevelOpenElement);

    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pListLevelOpenElement));
}

void DocumentCollector::_openListLevel(TagOpenElement *pListLevelOpenElement)
{
      if (!mbListElementOpened && miCurrentListLevel > 1)
      {
          mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:list-item")));
      }
    else if (mbListElementParagraphOpened)
    {
        mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:p")));
        mbListElementParagraphOpened = false;
    }

    if (miCurrentListLevel==1) {
        pListLevelOpenElement->addAttribute("text:style-name", mpCurrentListStyle->getName());
    }

    mbListElementOpened = false;
}

void DocumentCollector::closeOrderedListLevel()
{
    _closeListLevel("ordered-list");
}

void DocumentCollector::closeUnorderedListLevel()
{
    _closeListLevel("unordered-list");
}

void DocumentCollector::_closeListLevel(const char *szListType)
{
    if (mbListElementOpened)
        mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:list-item")));

    miCurrentListLevel--;

    WPXString sCloseElement;
    sCloseElement.sprintf("text:%s", szListType);
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement(sCloseElement.cstr())));

    if (miCurrentListLevel > 0)
        mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:list-item")));
    mbListElementOpened = false;
}

void DocumentCollector::openListElement(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops)
{
    miLastListLevel = miCurrentListLevel;
    if (miCurrentListLevel == 1)
        miLastListNumber++;

    if (mbListElementOpened)
        mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:list-item")));

    ParagraphStyle *pStyle = NULL;

    WPXPropertyList *pPersistPropList = new WPXPropertyList(propList);
    pPersistPropList->insert("style:list-style-name", mpCurrentListStyle->getName());
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

    TagOpenElement *pOpenListElement = new TagOpenElement("text:list-item");
    TagOpenElement *pOpenListElementParagraph = new TagOpenElement("text:p");

    pOpenListElementParagraph->addAttribute("text:style-name", pStyle->getName());

    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pOpenListElement));
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pOpenListElementParagraph));
        
    mbListElementOpened = true;
    mbListElementParagraphOpened = true;
    mbListContinueNumbering = false;
}

void DocumentCollector::closeListElement()
{
    // this code is kind of tricky, because we don't actually close the list element (because this list element
    // could contain another list level in OOo's implementation of lists). that is done in the closeListLevel
    // code (or when we open another list element)

    if (mbListElementParagraphOpened)
    {
        mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:p")));
        mbListElementParagraphOpened = false;
    }
}

void DocumentCollector::openFootnote(const WPXPropertyList &propList)
{
    TagOpenElement *pOpenFootNote = new TagOpenElement("text:footnote");
    if (propList["libwpd:number"])
    {
        WPXString tmpString("ftn");
        tmpString.append(propList["libwpd:number"]->getStr());
        pOpenFootNote->addAttribute("text:id", tmpString);
    }
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pOpenFootNote));

    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:footnote-citation")));
        if (propList["libwpd:number"])
                mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new CharDataElement(propList["libwpd:number"]->getStr().cstr())));
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:footnote-citation")));

    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:footnote-body")));
    
    mWriterDocumentState.mbInNote = true;
}

void DocumentCollector::closeFootnote()
{
    mWriterDocumentState.mbInNote = false;

    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:footnote-body")));
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:footnote")));
}

void DocumentCollector::openEndnote(const WPXPropertyList &propList)
{
    TagOpenElement *pOpenEndNote = new TagOpenElement("text:endnote");
    if (propList["libwpd:number"])
    {
        WPXString tmpString("edn");
        tmpString.append(propList["libwpd:number"]->getStr());
        pOpenEndNote->addAttribute("text:id", tmpString);
    }
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pOpenEndNote));

    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:endnote-citation")));
        if (propList["libwpd:number"])
                mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new CharDataElement(propList["libwpd:number"]->getStr().cstr())));
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:endnote-citation")));

    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:endnote-body")));

}
void DocumentCollector::closeEndnote()
{
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:endnote-body")));
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:endnote")));
}

void DocumentCollector::openTable(const WPXPropertyList &propList, const WPXPropertyListVector &columns)
{
    WPXString sTableName;
    sTableName.sprintf("Table%i", mTableStyles.size());

    // FIXME: we base the table style off of the page's margin left, ignoring (potential) wordperfect margin
    // state which is transmitted inside the page. could this lead to unacceptable behaviour?
        // WLACH_REFACTORING: characterize this behaviour, probably should nip it at the bud within libwpd
    TableStyle *pTableStyle = new TableStyle(propList, columns, sTableName.cstr());

    if (mWriterDocumentState.mbFirstElement && mpCurrentContentElements == &mBodyElements)
    {
        WPXString sMasterPageName("Page Style 1");
        pTableStyle->setMasterPageName(sMasterPageName);
        mWriterDocumentState.mbFirstElement = false;
    }

    mTableStyles.push_back(pTableStyle);

    mpCurrentTableStyle = pTableStyle;

    TagOpenElement *pTableOpenElement = new TagOpenElement("table:table");

    pTableOpenElement->addAttribute("table:name", sTableName.cstr());
    pTableOpenElement->addAttribute("table:style-name", sTableName.cstr());
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pTableOpenElement));

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

void DocumentCollector::openTableRow(const WPXPropertyList &propList)
{
    if (propList["libwpd:is-header-row"] && (propList["libwpd:is-header-row"]->getInt()))
    {
        mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("table:table-header-rows")));
        mWriterDocumentState.mbHeaderRow = true;
    }

    WPXString sTableRowStyleName;
    sTableRowStyleName.sprintf("%s.Row%i", mpCurrentTableStyle->getName().cstr(), mpCurrentTableStyle->getNumTableRowStyles());
    TableRowStyle *pTableRowStyle = new TableRowStyle(propList, sTableRowStyleName.cstr());
    mpCurrentTableStyle->addTableRowStyle(pTableRowStyle);
    
    TagOpenElement *pTableRowOpenElement = new TagOpenElement("table:table-row");
    pTableRowOpenElement->addAttribute("table:style-name", sTableRowStyleName);
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pTableRowOpenElement));
}

void DocumentCollector::closeTableRow()
{
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:table-row")));
    if (mWriterDocumentState.mbHeaderRow)
    {
        mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:table-header-rows")));
        mWriterDocumentState.mbHeaderRow = false;
    }
}

void DocumentCollector::openTableCell(const WPXPropertyList &propList)
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
    pTableCellOpenElement->addAttribute("table:value-type", "string");
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pTableCellOpenElement));

    mWriterDocumentState.mbTableCellOpened = true;
}

void DocumentCollector::closeTableCell()
{
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:table-cell")));
    mWriterDocumentState.mbTableCellOpened = false;
}

void DocumentCollector::insertCoveredTableCell(const WPXPropertyList & /* propList */)
{
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("table:covered-table-cell")));
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:covered-table-cell")));
}

void DocumentCollector::closeTable()
{
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:table")));
}

void DocumentCollector::insertTab()
{
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:tab-stop")));
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:tab-stop")));
}

void DocumentCollector::insertLineBreak()
{
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:line-break")));
    mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:line-break")));
}

void DocumentCollector::insertText(const WPXString &text)
{
    DocumentElement *pText = new TextElement(text);
    mpCurrentContentElements->push_back(pText);
}
