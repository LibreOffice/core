/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* OdtGenerator: Collects sections and runs of text from a
 * wordperfect file (and styles to go along with them) and writes them
 * to a Writer target document
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <libwpd/libwpd.h>
#include <vector>
#include <map>
#include <stack>
#include <string>

#include "OdtGenerator.hxx"
#include "DocumentElement.hxx"
#include "TextRunStyle.hxx"
#include "FontStyle.hxx"
#include "ListStyle.hxx"
#include "PageSpan.hxx"
#include "SectionStyle.hxx"
#include "TableStyle.hxx"
#include "FilterInternal.hxx"
#include "InternalHandler.hxx"

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
    // a map id -> last list style defined with such id
    std::map<int, ListStyle *> mIdListStyleMap;
};

enum WriterListType { unordered, ordered };

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
    mpCurrentListStyle(0),
    miCurrentListLevel(0),
    miLastListLevel(0),
    miLastListNumber(0),
    mbListContinueNumbering(false),
    mbListElementParagraphOpened(false),
    mbListElementOpened(),
    mIdListStyleMap()
{
}

class OdtGeneratorPrivate
{
public:
    OdtGeneratorPrivate(OdfDocumentHandler *pHandler, const OdfStreamType streamType);
    ~OdtGeneratorPrivate();
    bool _writeTargetDocument(OdfDocumentHandler *pHandler);
    void _writeDefaultStyles(OdfDocumentHandler *pHandler);
    void _writeMasterPages(OdfDocumentHandler *pHandler);
    void _writePageLayouts(OdfDocumentHandler *pHandler);

    void _openListLevel(TagOpenElement *pListLevelOpenElement);
    void _closeListLevel();

    /** stores a list style: update mListStyles,
        mWriterListStates.top().mpCurrentListStyle and the different
        maps
     */
    void _storeListStyle(ListStyle *listStyle);
    /** retrieves the list style corresponding to a given id. */
    void _retrieveListStyle(int id);

    OdfEmbeddedObject _findEmbeddedObjectHandler(const WPXString &mimeType);
    OdfEmbeddedImage _findEmbeddedImageHandler(const WPXString &mimeType);

    OdfDocumentHandler *mpHandler;
    bool mbUsed; // whether or not it has been before (you can only use me once!)

    std::stack<WriterDocumentState> mWriterDocumentStates;

    std::stack<WriterListState> mWriterListStates;

    // paragraph styles
    ParagraphStyleManager mParagraphManager;

    // span styles
    SpanStyleManager mSpanManager;

    // font styles
    FontStyleManager mFontManager;

    // section styles
    std::vector<SectionStyle *> mSectionStyles;

    // table styles
    std::vector<TableStyle *> mTableStyles;

    // frame styles
    std::vector<DocumentElement *> mFrameStyles;

    std::vector<DocumentElement *> mFrameAutomaticStyles;

    // embedded object handlers
    std::map<WPXString, OdfEmbeddedObject, ltstr > mObjectHandlers;
    std::map<WPXString, OdfEmbeddedImage, ltstr > mImageHandlers;

    // metadata
    std::vector<DocumentElement *> mMetaData;

    // list styles
    unsigned int miNumListStyles;

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
    // a map id -> last list style defined with id
    std::map<int, ListStyle *> mIdListStyleMap;

    // object state
    unsigned miObjectNumber;

    // table state
    TableStyle *mpCurrentTableStyle;

    const OdfStreamType mxStreamType;

    const char *mpPassword;

};

OdtGeneratorPrivate::OdtGeneratorPrivate(OdfDocumentHandler *pHandler, const OdfStreamType streamType) :
    mpHandler(pHandler),
    mbUsed(false),
    mWriterDocumentStates(),
    mWriterListStates(),
    mParagraphManager(), mSpanManager(), mFontManager(),
    mSectionStyles(), mTableStyles(), mFrameStyles(), mFrameAutomaticStyles(),
    mObjectHandlers(), mImageHandlers(), mMetaData(),
    miNumListStyles(0),
    mBodyElements(),
    mpCurrentContentElements(&mBodyElements),
    mPageSpans(),
    mpCurrentPageSpan(0),
    miNumPageStyles(0),
    mListStyles(),
    mIdListStyleMap(),
    miObjectNumber(0),
    mpCurrentTableStyle(0),
    mxStreamType(streamType),
    mpPassword(0)
{
    mWriterDocumentStates.push(WriterDocumentState());
    mWriterListStates.push(WriterListState());
}

OdtGeneratorPrivate::~OdtGeneratorPrivate()
{
    // clean up the mess we made
    WRITER_DEBUG_MSG(("WriterWordPerfect: Cleaning up our mess..\n"));

    WRITER_DEBUG_MSG(("Destroying the body elements\n"));
    for (std::vector<DocumentElement *>::iterator iterBody = mBodyElements.begin(); iterBody != mBodyElements.end(); ++iterBody)
    {
        delete (*iterBody);
        (*iterBody) = 0;
    }

    mParagraphManager.clean();
    mSpanManager.clean();
    mFontManager.clean();

    for (std::vector<ListStyle *>::iterator iterListStyles = mListStyles.begin();
            iterListStyles != mListStyles.end(); ++iterListStyles)
    {
        delete(*iterListStyles);
    }
    for (std::vector<SectionStyle *>::iterator iterSectionStyles = mSectionStyles.begin();
            iterSectionStyles != mSectionStyles.end(); ++iterSectionStyles)
    {
        delete(*iterSectionStyles);
    }
    for (std::vector<TableStyle *>::iterator iterTableStyles = mTableStyles.begin();
            iterTableStyles != mTableStyles.end(); ++iterTableStyles)
    {
        delete((*iterTableStyles));
    }

    for (std::vector<PageSpan *>::iterator iterPageSpans = mPageSpans.begin();
            iterPageSpans != mPageSpans.end(); ++iterPageSpans)
    {
        delete(*iterPageSpans);
    }
    for (std::vector<DocumentElement *>::iterator iterFrameStyles = mFrameStyles.begin();
            iterFrameStyles != mFrameStyles.end(); ++iterFrameStyles)
    {
        delete(*iterFrameStyles);
    }
    for (std::vector<DocumentElement *>::iterator iterFrameAutomaticStyles = mFrameAutomaticStyles.begin();
            iterFrameAutomaticStyles != mFrameAutomaticStyles.end(); ++iterFrameAutomaticStyles)
    {
        delete(*iterFrameAutomaticStyles);
    }
    for (std::vector<DocumentElement *>::iterator iterMetaData = mMetaData.begin();
            iterMetaData != mMetaData.end(); ++iterMetaData)
    {
        delete(*iterMetaData);
    }
}

OdfEmbeddedObject OdtGeneratorPrivate::_findEmbeddedObjectHandler(const WPXString &mimeType)
{
    std::map<WPXString, OdfEmbeddedObject, ltstr>::iterator i = mObjectHandlers.find(mimeType);
    if (i != mObjectHandlers.end())
        return i->second;

    return 0;
}

OdfEmbeddedImage OdtGeneratorPrivate::_findEmbeddedImageHandler(const WPXString &mimeType)
{
    std::map<WPXString, OdfEmbeddedImage, ltstr>::iterator i = mImageHandlers.find(mimeType);
    if (i != mImageHandlers.end())
        return i->second;

    return 0;
}

OdtGenerator::OdtGenerator(OdfDocumentHandler *pHandler, const OdfStreamType streamType) :
    mpImpl(new OdtGeneratorPrivate(pHandler, streamType))
{
}

OdtGenerator::~OdtGenerator()
{
    if (mpImpl)
        delete mpImpl;
}

void OdtGeneratorPrivate::_writeDefaultStyles(OdfDocumentHandler *pHandler)
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
            iter != mFrameStyles.end(); ++iter)
        (*iter)->write(pHandler);

    pHandler->endElement("office:styles");
}

void OdtGeneratorPrivate::_writeMasterPages(OdfDocumentHandler *pHandler)
{
    TagOpenElement("office:master-styles").write(mpHandler);
    int pageNumber = 1;
    for (unsigned int i=0; i<mPageSpans.size(); ++i)
    {
        bool bLastPage;
        (i == (mPageSpans.size() - 1)) ? bLastPage = true : bLastPage = false;
        mPageSpans[i]->writeMasterPages(pageNumber, i, bLastPage, pHandler);
        pageNumber += mPageSpans[i]->getSpan();
    }
    pHandler->endElement("office:master-styles");
}

void OdtGeneratorPrivate::_writePageLayouts(OdfDocumentHandler *pHandler)
{
    for (unsigned int i=0; i<mPageSpans.size(); ++i)
    {
        mPageSpans[i]->writePageLayout(i, pHandler);
    }
}

bool OdtGeneratorPrivate::_writeTargetDocument(OdfDocumentHandler *pHandler)
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
    docContentPropList.insert("xmlns:number", "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0");
    docContentPropList.insert("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
    docContentPropList.insert("xmlns:chart", "urn:oasis:names:tc:opendocument:xmlns:chart:1.0");
    docContentPropList.insert("xmlns:dr3d", "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0");
    docContentPropList.insert("xmlns:math", "http://www.w3.org/1998/Math/MathML");
    docContentPropList.insert("xmlns:form", "urn:oasis:names:tc:opendocument:xmlns:form:1.0");
    docContentPropList.insert("xmlns:script", "urn:oasis:names:tc:opendocument:xmlns:script:1.0");
    docContentPropList.insert("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
    docContentPropList.insert("office:version", "1.0");
    if (mxStreamType == ODF_FLAT_XML)
    {
        docContentPropList.insert("office:mimetype", "application/vnd.oasis.opendocument.text");
        mpHandler->startElement("office:document", docContentPropList);
    }
    else
        mpHandler->startElement("office:document-content", docContentPropList);

    // write out the metadata
    TagOpenElement("office:meta").write(mpHandler);
    for (std::vector<DocumentElement *>::const_iterator iterMetaData = mMetaData.begin(); iterMetaData != mMetaData.end(); ++iterMetaData)
    {
        (*iterMetaData)->write(mpHandler);
    }
    mpHandler->endElement("office:meta");

    // write out the font styles
    mFontManager.writeFontsDeclaration(mpHandler);

    WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Writing out the styles..\n"));

    // write default styles
    _writeDefaultStyles(mpHandler);

    TagOpenElement("office:automatic-styles").write(mpHandler);

    for (std::vector<DocumentElement *>::const_iterator iterFrameAutomaticStyles = mFrameAutomaticStyles.begin();
            iterFrameAutomaticStyles != mFrameAutomaticStyles.end(); ++iterFrameAutomaticStyles)
    {
        (*iterFrameAutomaticStyles)->write(pHandler);
    }

    mFontManager.write(pHandler); // do nothing
    mParagraphManager.write(pHandler);
    mSpanManager.write(pHandler);

    // writing out the sections styles
    for (std::vector<SectionStyle *>::const_iterator iterSectionStyles = mSectionStyles.begin(); iterSectionStyles != mSectionStyles.end(); ++iterSectionStyles)
    {
        (*iterSectionStyles)->write(pHandler);
    }

    // writing out the lists styles
    for (std::vector<ListStyle *>::const_iterator iterListStyles = mListStyles.begin(); iterListStyles != mListStyles.end(); ++iterListStyles)
    {
        (*iterListStyles)->write(pHandler);
    }

    // writing out the table styles
    for (std::vector<TableStyle *>::const_iterator iterTableStyles = mTableStyles.begin(); iterTableStyles != mTableStyles.end(); ++iterTableStyles)
    {
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

    for (std::vector<DocumentElement *>::const_iterator iterBodyElements = mBodyElements.begin(); iterBodyElements != mBodyElements.end(); ++iterBodyElements)
    {
        (*iterBodyElements)->write(pHandler);
    }
    WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Finished writing all doc els..\n"));

    pHandler->endElement("office:text");
    pHandler->endElement("office:body");
    if (mxStreamType == ODF_FLAT_XML)
        pHandler->endElement("office:document");
    else
        pHandler->endElement("office:document-content");

    pHandler->endDocument();

    return true;
}


void OdtGenerator::setDocumentMetaData(const WPXPropertyList &propList)
{
    WPXPropertyList::Iter i(propList);
    for (i.rewind(); i.next(); )
    {
        // filter out libwpd elements
        if (strncmp(i.key(), "libwpd", 6) != 0 && strncmp(i.key(), "dcterms", 7) != 0)
        {
            mpImpl->mMetaData.push_back(new TagOpenElement(i.key()));
            WPXString sStringValue(i()->getStr(), true);
            mpImpl->mMetaData.push_back(new CharDataElement(sStringValue.cstr()));
            mpImpl->mMetaData.push_back(new TagCloseElement(i.key()));
        }
    }

}

void OdtGenerator::openPageSpan(const WPXPropertyList &propList)
{
    PageSpan *pPageSpan = new PageSpan(propList);
    mpImpl->mPageSpans.push_back(pPageSpan);
    mpImpl->mpCurrentPageSpan = pPageSpan;
    mpImpl->miNumPageStyles++;

    mpImpl->mWriterDocumentStates.top().mbFirstParagraphInPageSpan = true;
}

void OdtGenerator::openHeader(const WPXPropertyList &propList)
{
    std::vector<DocumentElement *> * pHeaderFooterContentElements = new std::vector<DocumentElement *>;

    if (propList["libwpd:occurence"]->getStr() == "even")
        mpImpl->mpCurrentPageSpan->setHeaderLeftContent(pHeaderFooterContentElements);
    else
        mpImpl->mpCurrentPageSpan->setHeaderContent(pHeaderFooterContentElements);

    mpImpl->mpCurrentContentElements = pHeaderFooterContentElements;
}

void OdtGenerator::closeHeader()
{
    mpImpl->mpCurrentContentElements = &(mpImpl->mBodyElements);
}

void OdtGenerator::openFooter(const WPXPropertyList &propList)
{
    std::vector<DocumentElement *> * pHeaderFooterContentElements = new std::vector<DocumentElement *>;

    if (propList["libwpd:occurence"]->getStr() == "even")
        mpImpl->mpCurrentPageSpan->setFooterLeftContent(pHeaderFooterContentElements);
    else
        mpImpl->mpCurrentPageSpan->setFooterContent(pHeaderFooterContentElements);

    mpImpl->mpCurrentContentElements = pHeaderFooterContentElements;
}

void OdtGenerator::closeFooter()
{
    mpImpl->mpCurrentContentElements = &(mpImpl->mBodyElements);
}

void OdtGenerator::openSection(const WPXPropertyList &propList, const WPXPropertyListVector &columns)
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
        WPXString sSectionName;
        sSectionName.sprintf("Section%i", mpImpl->mSectionStyles.size());

        SectionStyle *pSectionStyle = new SectionStyle(propList, columns, sSectionName.cstr());
        mpImpl->mSectionStyles.push_back(pSectionStyle);

        TagOpenElement *pSectionOpenElement = new TagOpenElement("text:section");
        pSectionOpenElement->addAttribute("text:style-name", pSectionStyle->getName());
        pSectionOpenElement->addAttribute("text:name", pSectionStyle->getName());
        mpImpl->mpCurrentContentElements->push_back(pSectionOpenElement);
    }
    else
        mpImpl->mWriterDocumentStates.top().mbInFakeSection = true;
}

void OdtGenerator::closeSection()
{
    if (!mpImpl->mWriterDocumentStates.top().mbInFakeSection)
        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:section"));
    else
        mpImpl->mWriterDocumentStates.top().mbInFakeSection = false;
}

void OdtGenerator::openParagraph(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops)
{
    // FIXMENOW: What happens if we open a footnote inside a table? do we then inherit the footnote's style
    // from "Table Contents"

    WPXPropertyList finalPropList(propList);
    if (mpImpl->mWriterDocumentStates.top().mbFirstParagraphInPageSpan && mpImpl->mpCurrentContentElements == &(mpImpl->mBodyElements))
    {
        WPXString sPageStyleName;
        sPageStyleName.sprintf("Page_Style_%i", mpImpl->miNumPageStyles);
        finalPropList.insert("style:master-page-name", sPageStyleName);
        mpImpl->mWriterDocumentStates.top().mbFirstElement = false;
        mpImpl->mWriterDocumentStates.top().mbFirstParagraphInPageSpan = false;
    }

    if (mpImpl->mWriterDocumentStates.top().mbTableCellOpened)
    {
        if (mpImpl->mWriterDocumentStates.top().mbHeaderRow)
            finalPropList.insert("style:parent-style-name", "Table_Heading");
        else
            finalPropList.insert("style:parent-style-name", "Table_Contents");
    }
    else
        finalPropList.insert("style:parent-style-name", "Standard");

    WPXString sName = mpImpl->mParagraphManager.findOrAdd(finalPropList, tabStops);

    // create a document element corresponding to the paragraph, and append it to our list of document elements
    TagOpenElement *pParagraphOpenElement = new TagOpenElement("text:p");
    pParagraphOpenElement->addAttribute("text:style-name", sName);
    mpImpl->mpCurrentContentElements->push_back(pParagraphOpenElement);
}

void OdtGenerator::closeParagraph()
{
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:p"));
}

void OdtGenerator::openSpan(const WPXPropertyList &propList)
{
    if (propList["style:font-name"])
        mpImpl->mFontManager.findOrAdd(propList["style:font-name"]->getStr().cstr());

    // Get the style
    WPXString sName = mpImpl->mSpanManager.findOrAdd(propList);

    // create a document element corresponding to the paragraph, and append it to our list of document elements
    TagOpenElement *pSpanOpenElement = new TagOpenElement("text:span");
    pSpanOpenElement->addAttribute("text:style-name", sName.cstr());
    mpImpl->mpCurrentContentElements->push_back(pSpanOpenElement);
}

void OdtGenerator::closeSpan()
{
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:span"));
}

void OdtGeneratorPrivate::_storeListStyle(ListStyle *listStyle)
{
    if (!listStyle || listStyle == mWriterListStates.top().mpCurrentListStyle)
    {
        return;
    }
    mListStyles.push_back(listStyle);
    mWriterListStates.top().mpCurrentListStyle = listStyle;
    mWriterListStates.top().mIdListStyleMap[listStyle->getListID()]=listStyle;
    mIdListStyleMap[listStyle->getListID()]=listStyle;
}

void OdtGeneratorPrivate::_retrieveListStyle(int id)
{
    // first look if the current style is ok
    if (mWriterListStates.top().mpCurrentListStyle &&
            id == mWriterListStates.top().mpCurrentListStyle->getListID())
    {
        return;
    }

    // use the current map
    if (mWriterListStates.top().mIdListStyleMap.find(id) !=
            mWriterListStates.top().mIdListStyleMap.end())
    {
        mWriterListStates.top().mpCurrentListStyle =
            mWriterListStates.top().mIdListStyleMap.find(id)->second;
        return;
    }

    // use the global map
    if (mIdListStyleMap.find(id) != mIdListStyleMap.end())
    {
        mWriterListStates.top().mpCurrentListStyle =
            mIdListStyleMap.find(id)->second;
        return;
    }

    WRITER_DEBUG_MSG(("impossible to find a list with id=%d\n",id));
}

void OdtGenerator::defineOrderedListLevel(const WPXPropertyList &propList)
{
    int id = 0;
    if (propList["libwpd:id"])
        id = propList["libwpd:id"]->getInt();

    ListStyle *pListStyle = 0;
    if (mpImpl->mWriterListStates.top().mpCurrentListStyle && mpImpl->mWriterListStates.top().mpCurrentListStyle->getListID() == id)
        pListStyle = mpImpl->mWriterListStates.top().mpCurrentListStyle;

    // this rather appalling conditional makes sure we only start a
    // new list (rather than continue an old one) if: (1) we have no
    // prior list or the prior list has another listId OR (2) we can
    // tell that the user actually is starting a new list at level 1
    // (and only level 1)
    if (pListStyle == 0 ||
            (propList["libwpd:level"] && propList["libwpd:level"]->getInt()==1 &&
             (propList["text:start-value"] && propList["text:start-value"]->getInt() != int(mpImpl->mWriterListStates.top().miLastListNumber+1))))
    {
        WRITER_DEBUG_MSG(("Attempting to create a new ordered list style (listid: %i)\n", id));
        WPXString sName;
        sName.sprintf("OL%i", mpImpl->miNumListStyles);
        mpImpl->miNumListStyles++;
        pListStyle = new ListStyle(sName.cstr(), id);
        mpImpl->_storeListStyle(pListStyle);
        mpImpl->mWriterListStates.top().mbListContinueNumbering = false;
        mpImpl->mWriterListStates.top().miLastListNumber = 0;
    }
    else
        mpImpl->mWriterListStates.top().mbListContinueNumbering = true;

    // Iterate through ALL list styles with the same WordPerfect list id and define a level if it is not already defined
    // This solves certain problems with lists that start and finish without reaching certain levels and then begin again
    // and reach those levels. See gradguide0405_PC.wpd in the regression suite
    for (std::vector<ListStyle *>::iterator iterListStyles = mpImpl->mListStyles.begin(); iterListStyles != mpImpl->mListStyles.end(); ++iterListStyles)
    {
        if ((* iterListStyles) && (* iterListStyles)->getListID() == id && propList["libwpd:level"])
            (* iterListStyles)->updateListLevel((propList["libwpd:level"]->getInt() - 1), propList, true);
    }
}

void OdtGenerator::defineUnorderedListLevel(const WPXPropertyList &propList)
{
    int id = 0;
    if (propList["libwpd:id"])
        id = propList["libwpd:id"]->getInt();

    ListStyle *pListStyle = 0;
    if (mpImpl->mWriterListStates.top().mpCurrentListStyle && mpImpl->mWriterListStates.top().mpCurrentListStyle->getListID() == id)
        pListStyle = mpImpl->mWriterListStates.top().mpCurrentListStyle;

    if (pListStyle == 0)
    {
        WRITER_DEBUG_MSG(("Attempting to create a new unordered list style (listid: %i)\n", id));
        WPXString sName;
        sName.sprintf("UL%i", mpImpl->miNumListStyles);
        mpImpl->miNumListStyles++;
        pListStyle = new ListStyle(sName.cstr(), id);
        mpImpl->_storeListStyle(pListStyle);
    }

    // See comment in OdtGenerator::defineOrderedListLevel
    for (std::vector<ListStyle *>::iterator iterListStyles = mpImpl->mListStyles.begin(); iterListStyles != mpImpl->mListStyles.end(); ++iterListStyles)
    {
        if ((* iterListStyles) && (* iterListStyles)->getListID() == id && propList["libwpd:level"])
            (* iterListStyles)->updateListLevel((propList["libwpd:level"]->getInt() - 1), propList, false);
    }
}

void OdtGenerator::openOrderedListLevel(const WPXPropertyList &propList)
{
    if (mpImpl->mWriterListStates.top().mbListElementParagraphOpened)
    {
        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:p"));
        mpImpl->mWriterListStates.top().mbListElementParagraphOpened = false;
    }
    if (mpImpl->mWriterListStates.top().mbListElementOpened.empty() && propList["libwpd:id"])
    {
        // first item of a list, be sure to use the list with given id
        mpImpl->_retrieveListStyle(propList["libwpd:id"]->getInt());
    }
    TagOpenElement *pListLevelOpenElement = new TagOpenElement("text:list");
    mpImpl->_openListLevel(pListLevelOpenElement);

    if (mpImpl->mWriterListStates.top().mbListContinueNumbering)
    {
        pListLevelOpenElement->addAttribute("text:continue-numbering", "true");
    }

    mpImpl->mpCurrentContentElements->push_back(pListLevelOpenElement);
}

void OdtGenerator::openUnorderedListLevel(const WPXPropertyList &propList)
{
    if (mpImpl->mWriterListStates.top().mbListElementParagraphOpened)
    {
        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:p"));
        mpImpl->mWriterListStates.top().mbListElementParagraphOpened = false;
    }
    if (mpImpl->mWriterListStates.top().mbListElementOpened.empty() && propList["libwpd:id"])
    {
        // first item of a list, be sure to use the list with given id
        mpImpl->_retrieveListStyle(propList["libwpd:id"]->getInt());
    }
    TagOpenElement *pListLevelOpenElement = new TagOpenElement("text:list");
    mpImpl->_openListLevel(pListLevelOpenElement);

    mpImpl->mpCurrentContentElements->push_back(pListLevelOpenElement);
}

void OdtGeneratorPrivate::_openListLevel(TagOpenElement *pListLevelOpenElement)
{
    if (!mWriterListStates.top().mbListElementOpened.empty() &&
            !mWriterListStates.top().mbListElementOpened.top())
    {
        mpCurrentContentElements->push_back(new TagOpenElement("text:list-item"));
        mWriterListStates.top().mbListElementOpened.top() = true;
    }

    mWriterListStates.top().mbListElementOpened.push(false);
    if (mWriterListStates.top().mbListElementOpened.size() == 1)
    {
        // add a sanity check ( to avoid a crash if mpCurrentListStyle is NULL)
        if (mWriterListStates.top().mpCurrentListStyle)
        {
            pListLevelOpenElement->addAttribute("text:style-name", mWriterListStates.top().mpCurrentListStyle->getName());
        }
    }
}

void OdtGenerator::closeOrderedListLevel()
{
    mpImpl->_closeListLevel();
}

void OdtGenerator::closeUnorderedListLevel()
{
    mpImpl->_closeListLevel();
}

void OdtGeneratorPrivate::_closeListLevel()
{
    if (mWriterListStates.top().mbListElementOpened.empty())
    {
        // this implies that openListLevel was not called, so it is better to stop here
        WRITER_DEBUG_MSG(("attempting to close an unexisting level\n"));
        return;
    }
    if (mWriterListStates.top().mbListElementOpened.top())
    {
        mpCurrentContentElements->push_back(new TagCloseElement("text:list-item"));
        mWriterListStates.top().mbListElementOpened.top() = false;
    }

    mpCurrentContentElements->push_back(new TagCloseElement("text:list"));
    mWriterListStates.top().mbListElementOpened.pop();
}

void OdtGenerator::openListElement(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops)
{
    mpImpl->mWriterListStates.top().miLastListLevel = mpImpl->mWriterListStates.top().miCurrentListLevel;
    if (mpImpl->mWriterListStates.top().miCurrentListLevel == 1)
        mpImpl->mWriterListStates.top().miLastListNumber++;

    if (mpImpl->mWriterListStates.top().mbListElementOpened.top())
    {
        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:list-item"));
        mpImpl->mWriterListStates.top().mbListElementOpened.top() = false;
    }

    WPXPropertyList finalPropList(propList);
#if 0
    // this property is ignored in TextRunStyle.c++
    if (mpImpl->mWriterListStates.top().mpCurrentListStyle)
        finalPropList.insert("style:list-style-name", mpImpl->mWriterListStates.top().mpCurrentListStyle->getName());
#endif
    finalPropList.insert("style:parent-style-name", "Standard");
    WPXString paragName = mpImpl->mParagraphManager.findOrAdd(finalPropList, tabStops);

    TagOpenElement *pOpenListItem = new TagOpenElement("text:list-item");
    if (propList["text:start-value"] && propList["text:start-value"]->getInt() > 0)
        pOpenListItem->addAttribute("text:start-value", propList["text:start-value"]->getStr());
    mpImpl->mpCurrentContentElements->push_back(pOpenListItem);

    TagOpenElement *pOpenListElementParagraph = new TagOpenElement("text:p");
    pOpenListElementParagraph->addAttribute("text:style-name", paragName);
    mpImpl->mpCurrentContentElements->push_back(pOpenListElementParagraph);

    if (mpImpl->mpCurrentContentElements == &(mpImpl->mBodyElements))
        mpImpl->mWriterDocumentStates.top().mbFirstParagraphInPageSpan = false;

    mpImpl->mWriterListStates.top().mbListElementOpened.top() = true;
    mpImpl->mWriterListStates.top().mbListElementParagraphOpened = true;
    mpImpl->mWriterListStates.top().mbListContinueNumbering = false;
}

void OdtGenerator::closeListElement()
{
    // this code is kind of tricky, because we don't actually close the list element (because this list element
    // could contain another list level in OOo's implementation of lists). that is done in the closeListLevel
    // code (or when we open another list element)

    if (mpImpl->mWriterListStates.top().mbListElementParagraphOpened)
    {
        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:p"));
        mpImpl->mWriterListStates.top().mbListElementParagraphOpened = false;
    }
}

void OdtGenerator::openFootnote(const WPXPropertyList &propList)
{
    mpImpl->mWriterListStates.push(WriterListState());
    TagOpenElement *pOpenFootNote = new TagOpenElement("text:note");
    pOpenFootNote->addAttribute("text:note-class", "footnote");
    if (propList["libwpd:number"])
    {
        WPXString tmpString("ftn");
        tmpString.append(propList["libwpd:number"]->getStr());
        pOpenFootNote->addAttribute("text:id", tmpString);
    }
    mpImpl->mpCurrentContentElements->push_back(pOpenFootNote);

    TagOpenElement *pOpenFootCitation = new TagOpenElement("text:note-citation");
    if (propList["text:label"])
    {
        WPXString tmpString(propList["text:label"]->getStr(),true);
        pOpenFootCitation->addAttribute("text:label", tmpString);
    }
    mpImpl->mpCurrentContentElements->push_back(pOpenFootCitation);

    if (propList["text:label"])
        mpImpl->mpCurrentContentElements->push_back(new CharDataElement(propList["text:label"]->getStr().cstr()));
    else if (propList["libwpd:number"])
        mpImpl->mpCurrentContentElements->push_back(new CharDataElement(propList["libwpd:number"]->getStr().cstr()));
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:note-citation"));

    mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("text:note-body"));

    mpImpl->mWriterDocumentStates.top().mbInNote = true;
}

void OdtGenerator::closeFootnote()
{
    mpImpl->mWriterDocumentStates.top().mbInNote = false;
    if (mpImpl->mWriterListStates.size() > 1)
        mpImpl->mWriterListStates.pop();

    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:note-body"));
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:note"));
}

void OdtGenerator::openEndnote(const WPXPropertyList &propList)
{
    mpImpl->mWriterListStates.push(WriterListState());
    TagOpenElement *pOpenEndNote = new TagOpenElement("text:note");
    pOpenEndNote->addAttribute("text:note-class", "endnote");
    if (propList["libwpd:number"])
    {
        WPXString tmpString("edn");
        tmpString.append(propList["libwpd:number"]->getStr());
        pOpenEndNote->addAttribute("text:id", tmpString);
    }
    mpImpl->mpCurrentContentElements->push_back(pOpenEndNote);

    TagOpenElement *pOpenEndCitation = new TagOpenElement("text:note-citation");
    if (propList["text:label"])
    {
        WPXString tmpString(propList["text:label"]->getStr(),true);
        pOpenEndCitation->addAttribute("text:label", tmpString);
    }
    mpImpl->mpCurrentContentElements->push_back(pOpenEndCitation);

    if (propList["text:label"])
        mpImpl->mpCurrentContentElements->push_back(new CharDataElement(propList["text:label"]->getStr().cstr()));
    else if (propList["libwpd:number"])
        mpImpl->mpCurrentContentElements->push_back(new CharDataElement(propList["libwpd:number"]->getStr().cstr()));
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:note-citation"));

    mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("text:note-body"));

    mpImpl->mWriterDocumentStates.top().mbInNote = true;
}

void OdtGenerator::closeEndnote()
{
    mpImpl->mWriterDocumentStates.top().mbInNote = false;
    if (mpImpl->mWriterListStates.size() > 1)
        mpImpl->mWriterListStates.pop();

    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:note-body"));
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:note"));
}

void OdtGenerator::openComment(const WPXPropertyList &)
{
    mpImpl->mWriterListStates.push(WriterListState());
    mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("office:annotation"));

    mpImpl->mWriterDocumentStates.top().mbInNote = true;
}

void OdtGenerator::closeComment()
{
    mpImpl->mWriterDocumentStates.top().mbInNote = false;
    if (mpImpl->mWriterListStates.size() > 1)
        mpImpl->mWriterListStates.pop();

    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("office:annotation"));
}

void OdtGenerator::openTable(const WPXPropertyList &propList, const WPXPropertyListVector &columns)
{
    if (!mpImpl->mWriterDocumentStates.top().mbInNote)
    {
        WPXString sTableName;
        sTableName.sprintf("Table%i", mpImpl->mTableStyles.size());

        // FIXME: we base the table style off of the page's margin left, ignoring (potential) wordperfect margin
        // state which is transmitted inside the page. could this lead to unacceptable behaviour?
        // WLACH_REFACTORING: characterize this behaviour, probably should nip it at the bud within libwpd
        TableStyle *pTableStyle = new TableStyle(propList, columns, sTableName.cstr());

        if (mpImpl->mWriterDocumentStates.top().mbFirstElement && mpImpl->mpCurrentContentElements == &(mpImpl->mBodyElements))
        {
            WPXString sMasterPageName("Page_Style_1");
            pTableStyle->setMasterPageName(sMasterPageName);
            mpImpl->mWriterDocumentStates.top().mbFirstElement = false;
        }

        mpImpl->mTableStyles.push_back(pTableStyle);

        mpImpl->mpCurrentTableStyle = pTableStyle;

        TagOpenElement *pTableOpenElement = new TagOpenElement("table:table");

        pTableOpenElement->addAttribute("table:name", sTableName.cstr());
        pTableOpenElement->addAttribute("table:style-name", sTableName.cstr());
        mpImpl->mpCurrentContentElements->push_back(pTableOpenElement);

        for (int i=0; i<pTableStyle->getNumColumns(); ++i)
        {
            TagOpenElement *pTableColumnOpenElement = new TagOpenElement("table:table-column");
            WPXString sColumnStyleName;
            sColumnStyleName.sprintf("%s.Column%i", sTableName.cstr(), (i+1));
            pTableColumnOpenElement->addAttribute("table:style-name", sColumnStyleName.cstr());
            mpImpl->mpCurrentContentElements->push_back(pTableColumnOpenElement);

            TagCloseElement *pTableColumnCloseElement = new TagCloseElement("table:table-column");
            mpImpl->mpCurrentContentElements->push_back(pTableColumnCloseElement);
        }
    }
}

void OdtGenerator::openTableRow(const WPXPropertyList &propList)
{
    if (mpImpl->mWriterDocumentStates.top().mbInNote)
        return;
    if (!mpImpl->mpCurrentTableStyle)
    {
        WRITER_DEBUG_MSG(("OdtGenerator::openTableRow called with no table\n"));
        return;
    }
    if (propList["libwpd:is-header-row"] && (propList["libwpd:is-header-row"]->getInt()))
    {
        mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("table:table-header-rows"));
        mpImpl->mWriterDocumentStates.top().mbHeaderRow = true;
    }

    WPXString sTableRowStyleName;
    sTableRowStyleName.sprintf("%s.Row%i", mpImpl->mpCurrentTableStyle->getName().cstr(), mpImpl->mpCurrentTableStyle->getNumTableRowStyles());
    TableRowStyle *pTableRowStyle = new TableRowStyle(propList, sTableRowStyleName.cstr());
    mpImpl->mpCurrentTableStyle->addTableRowStyle(pTableRowStyle);

    TagOpenElement *pTableRowOpenElement = new TagOpenElement("table:table-row");
    pTableRowOpenElement->addAttribute("table:style-name", sTableRowStyleName);
    mpImpl->mpCurrentContentElements->push_back(pTableRowOpenElement);
}

void OdtGenerator::closeTableRow()
{
    if (!mpImpl->mWriterDocumentStates.top().mbInNote && mpImpl->mpCurrentTableStyle)
    {
        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("table:table-row"));
        if (mpImpl->mWriterDocumentStates.top().mbHeaderRow)
        {
            mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("table:table-header-rows"));
            mpImpl->mWriterDocumentStates.top().mbHeaderRow = false;
        }
    }
}

void OdtGenerator::openTableCell(const WPXPropertyList &propList)
{
    if (mpImpl->mWriterDocumentStates.top().mbInNote)
        return;
    if (!mpImpl->mpCurrentTableStyle)
    {
        WRITER_DEBUG_MSG(("OdtGenerator::openTableCell called with no table\n"));
        return;
    }

    WPXString sTableCellStyleName;
    sTableCellStyleName.sprintf( "%s.Cell%i", mpImpl->mpCurrentTableStyle->getName().cstr(), mpImpl->mpCurrentTableStyle->getNumTableCellStyles());
    TableCellStyle *pTableCellStyle = new TableCellStyle(propList, sTableCellStyleName.cstr());
    mpImpl->mpCurrentTableStyle->addTableCellStyle(pTableCellStyle);

    TagOpenElement *pTableCellOpenElement = new TagOpenElement("table:table-cell");
    pTableCellOpenElement->addAttribute("table:style-name", sTableCellStyleName);
    if (propList["table:number-columns-spanned"])
        pTableCellOpenElement->addAttribute("table:number-columns-spanned",
                                            propList["table:number-columns-spanned"]->getStr().cstr());
    if (propList["table:number-rows-spanned"])
        pTableCellOpenElement->addAttribute("table:number-rows-spanned",
                                            propList["table:number-rows-spanned"]->getStr().cstr());
    // pTableCellOpenElement->addAttribute("table:value-type", "string");
    mpImpl->mpCurrentContentElements->push_back(pTableCellOpenElement);

    mpImpl->mWriterDocumentStates.top().mbTableCellOpened = true;
}

void OdtGenerator::closeTableCell()
{
    if (!mpImpl->mWriterDocumentStates.top().mbInNote && mpImpl->mpCurrentTableStyle)
    {
        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("table:table-cell"));
        mpImpl->mWriterDocumentStates.top().mbTableCellOpened = false;
    }
}

void OdtGenerator::insertCoveredTableCell(const WPXPropertyList &)
{
    if (!mpImpl->mWriterDocumentStates.top().mbInNote && mpImpl->mpCurrentTableStyle)
    {
        mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("table:covered-table-cell"));
        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("table:covered-table-cell"));
    }
}

void OdtGenerator::closeTable()
{
    if (!mpImpl->mWriterDocumentStates.top().mbInNote)
    {
        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("table:table"));
    }
}


void OdtGenerator::insertTab()
{
    mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("text:tab"));
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:tab"));
}

void OdtGenerator::insertSpace()
{
    mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("text:s"));
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:s"));
}

void OdtGenerator::insertLineBreak()
{
    mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("text:line-break"));
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("text:line-break"));
}

void OdtGenerator::insertField(const WPXString &type, const WPXPropertyList &propList)
{
    if (!type.len())
        return;

    TagOpenElement *openElement = new TagOpenElement(type);
    if (type == "text:page-number")
        openElement->addAttribute("text:select-page", "current");

    if (propList["style:num-format"])
        openElement->addAttribute("style:num-format", propList["style:num-format"]->getStr());

    mpImpl->mpCurrentContentElements->push_back(openElement);
    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement(type));
}

void OdtGenerator::insertText(const WPXString &text)
{
    DocumentElement *pText = new TextElement(text);
    mpImpl->mpCurrentContentElements->push_back(pText);
}

void OdtGenerator::openFrame(const WPXPropertyList &propList)
{
    mpImpl->mWriterListStates.push(WriterListState());

    // First, let's create a Frame Style for this box
    TagOpenElement *frameStyleOpenElement = new TagOpenElement("style:style");
    WPXString frameStyleName;
    frameStyleName.sprintf("GraphicFrame_%i", mpImpl->miObjectNumber);
    frameStyleOpenElement->addAttribute("style:name", frameStyleName);
    frameStyleOpenElement->addAttribute("style:family", "graphic");

    mpImpl->mFrameStyles.push_back(frameStyleOpenElement);

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

    mpImpl->mFrameStyles.push_back(frameStylePropertiesOpenElement);

    mpImpl->mFrameStyles.push_back(new TagCloseElement("style:graphic-properties"));

    mpImpl->mFrameStyles.push_back(new TagCloseElement("style:style"));

    // Now, let's create an automatic style for this frame
    TagOpenElement *frameAutomaticStyleElement = new TagOpenElement("style:style");
    WPXString frameAutomaticStyleName;
    frameAutomaticStyleName.sprintf("fr%i", mpImpl->miObjectNumber);
    frameAutomaticStyleElement->addAttribute("style:name", frameAutomaticStyleName);
    frameAutomaticStyleElement->addAttribute("style:family", "graphic");
    frameAutomaticStyleElement->addAttribute("style:parent-style-name", frameStyleName);

    mpImpl->mFrameAutomaticStyles.push_back(frameAutomaticStyleElement);

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

    // check if the frame has border, background attributes
    static char const *(bordersString[])=
    {"fo:border","fo:border-top","fo:border-left","fo:border-bottom","fo:border-right"};
    for (int b = 0; b < 5; b++)
    {
        if (propList[bordersString[b]])
            frameAutomaticStylePropertiesElement->addAttribute(bordersString[b], propList[bordersString[b]]->getStr());
    }
    if (propList["style:border-line-width"])
        frameAutomaticStylePropertiesElement->addAttribute("style:border-line-width", propList["style:border-line-width"]->getStr());
    if (propList["fo:background-color"])
        frameAutomaticStylePropertiesElement->addAttribute("fo:background-color", propList["fo:background-color"]->getStr());
    if (propList["style:background-transparency"])
        frameAutomaticStylePropertiesElement->addAttribute("style:background-transparency", propList["style:background-transparency"]->getStr());

    if (propList["fo:clip"])
        frameAutomaticStylePropertiesElement->addAttribute("fo:clip", propList["fo:clip"]->getStr());

    frameAutomaticStylePropertiesElement->addAttribute("draw:ole-draw-aspect", "1");

    mpImpl->mFrameAutomaticStyles.push_back(frameAutomaticStylePropertiesElement);

    mpImpl->mFrameAutomaticStyles.push_back(new TagCloseElement("style:graphic-properties"));

    mpImpl->mFrameAutomaticStyles.push_back(new TagCloseElement("style:style"));

    // And write the frame itself
    TagOpenElement *drawFrameOpenElement = new TagOpenElement("draw:frame");

    drawFrameOpenElement->addAttribute("draw:style-name", frameAutomaticStyleName);
    WPXString objectName;
    objectName.sprintf("Object%i", mpImpl->miObjectNumber++);
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

    mpImpl->mpCurrentContentElements->push_back(drawFrameOpenElement);

    mpImpl->mWriterDocumentStates.top().mbInFrame = true;
}

void OdtGenerator::closeFrame()
{
    if (mpImpl->mWriterListStates.size() > 1)
        mpImpl->mWriterListStates.pop();

    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("draw:frame"));

    mpImpl->mWriterDocumentStates.top().mbInFrame = false;
}

void OdtGenerator::insertBinaryObject(const WPXPropertyList &propList, const WPXBinaryData &data)
{
    if (!data.size())
        return;
    if (!mpImpl->mWriterDocumentStates.top().mbInFrame) // Embedded objects without a frame simply don't make sense for us
        return;
    if (!propList["libwpd:mimetype"])
        return;

    OdfEmbeddedObject tmpObjectHandler = mpImpl->_findEmbeddedObjectHandler(propList["libwpd:mimetype"]->getStr());
    OdfEmbeddedImage tmpImageHandler = mpImpl->_findEmbeddedImageHandler(propList["libwpd:mimetype"]->getStr());

    if (tmpObjectHandler || tmpImageHandler)
    {
        if (tmpObjectHandler)
        {
            std::vector<DocumentElement *> tmpContentElements;
            InternalHandler tmpHandler(&tmpContentElements);

            if (tmpObjectHandler(data, &tmpHandler, ODF_FLAT_XML) && !tmpContentElements.empty())
            {
                mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("draw:object"));
                for (std::vector<DocumentElement *>::const_iterator iter = tmpContentElements.begin(); iter != tmpContentElements.end(); ++iter)
                    mpImpl->mpCurrentContentElements->push_back(*iter);
                mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("draw:object"));
            }
        }
        if (tmpImageHandler)
        {
            WPXBinaryData output;
            if (tmpImageHandler(data, output))
            {
                mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("draw:image"));

                mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("office:binary-data"));

                WPXString binaryBase64Data = output.getBase64Data();

                mpImpl->mpCurrentContentElements->push_back(new CharDataElement(binaryBase64Data.cstr()));

                mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("office:binary-data"));

                mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("draw:image"));
            }
        }
    }
    else
        // assuming we have a binary image or a object_ole that we can just insert as it is
    {
        if (propList["libwpd:mimetype"]->getStr() == "object/ole")
            mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("draw:object-ole"));
        else
            mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("draw:image"));

        mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("office:binary-data"));

        WPXString binaryBase64Data = data.getBase64Data();

        mpImpl->mpCurrentContentElements->push_back(new CharDataElement(binaryBase64Data.cstr()));

        mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("office:binary-data"));

        if (propList["libwpd:mimetype"]->getStr() == "object/ole")
            mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("draw:object-ole"));
        else
            mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("draw:image"));
    }
}

void OdtGenerator::openTextBox(const WPXPropertyList &)
{
    if (!mpImpl->mWriterDocumentStates.top().mbInFrame) // Text box without a frame simply doesn't make sense for us
        return;
    mpImpl->mWriterListStates.push(WriterListState());
    mpImpl->mWriterDocumentStates.push(WriterDocumentState());
    mpImpl->mpCurrentContentElements->push_back(new TagOpenElement("draw:text-box"));
    mpImpl->mWriterDocumentStates.top().mbInTextBox = true;
    mpImpl->mWriterDocumentStates.top().mbFirstElement = false;
}

void OdtGenerator::closeTextBox()
{
    if (!mpImpl->mWriterDocumentStates.top().mbInTextBox)
        return;
    if (mpImpl->mWriterListStates.size() > 1)
        mpImpl->mWriterListStates.pop();
    if (mpImpl->mWriterDocumentStates.size() > 1)
        mpImpl->mWriterDocumentStates.pop();

    mpImpl->mpCurrentContentElements->push_back(new TagCloseElement("draw:text-box"));
}

void OdtGenerator::defineSectionStyle(WPXPropertyList const &, WPXPropertyListVector const &)
{
}

void OdtGenerator::insertEquation(WPXPropertyList const &, WPXString const &)
{
}

void OdtGenerator::endDocument()
{
    // Write out the collected document
    mpImpl->_writeTargetDocument(mpImpl->mpHandler);
}

void OdtGenerator::startDocument()
{
}

void OdtGenerator::closePageSpan()
{
}

void OdtGenerator::definePageStyle(WPXPropertyList const &)
{
}

void OdtGenerator::defineParagraphStyle(WPXPropertyList const &, WPXPropertyListVector const &)
{
}

void OdtGenerator::defineCharacterStyle(WPXPropertyList const &)
{
}

void OdtGenerator::registerEmbeddedObjectHandler(const WPXString &mimeType, OdfEmbeddedObject objectHandler)
{
    mpImpl->mObjectHandlers[mimeType] = objectHandler;
}

void OdtGenerator::registerEmbeddedImageHandler(const WPXString &mimeType, OdfEmbeddedImage imageHandler)
{
    mpImpl->mImageHandlers[mimeType] = imageHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
