/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "htmldataprovider.hxx"
#include <salhelper/thread.hxx>

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <comphelper/string.hxx>

namespace sc {

class HTMLFetchThread : public salhelper::Thread
{
    ScDocument& mrDocument;
    OUString maURL;
    OUString maID;

    Idle* mpIdle;

    void handleTable(xmlNodePtr pTable);
    void handleRow(xmlNodePtr pRow, SCROW nRow);
    void skipHeadBody(xmlNodePtr pSkip, SCROW& rRow);
    void handleCell(xmlNodePtr pCell, SCROW nRow, SCCOL nCol);

public:
    HTMLFetchThread(ScDocument& rDoc, const OUString&, const OUString& rID, Idle* pIdle);

    virtual void execute() override;
};

HTMLFetchThread::HTMLFetchThread(ScDocument& rDoc, const OUString& rURL, const OUString& rID, Idle* pIdle):
    salhelper::Thread("HTML Fetch Thread"),
    mrDocument(rDoc),
    maURL(rURL),
    maID(rID),
    mpIdle(pIdle)
{
}

namespace {

OString toString(const xmlChar* pStr)
{
    return OString(reinterpret_cast<const char*>(pStr), xmlStrlen(pStr));
}

}

void HTMLFetchThread::handleCell(xmlNodePtr pCellNode, SCROW nRow, SCCOL nCol)
{
    for (xmlNodePtr cur_node = pCellNode->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_TEXT_NODE)
        {
            OUString aString = OStringToOUString(toString(cur_node->content), RTL_TEXTENCODING_UTF8);
            OUString aOldString;
            do
            {
                aOldString = aString;
                aString = comphelper::string::strip(aString, ' ');
                aString = comphelper::string::strip(aString, '\n');
                aString = comphelper::string::strip(aString, '\r');
                aString = comphelper::string::strip(aString, '\t');
            }
            while (aOldString != aString);

            mrDocument.SetString(nCol, nRow, 0, aString);
        }
    }
}

void HTMLFetchThread::handleRow(xmlNodePtr pRowNode, SCROW nRow)
{
    sal_Int32 nCol = 0;
    for (xmlNodePtr cur_node = pRowNode->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            OString aNodeName = toString(cur_node->name);
            if (aNodeName == "td" || aNodeName == "th")
            {
                handleCell(cur_node, nRow, nCol);
                ++nCol;
            }
        }
    }
}

void HTMLFetchThread::skipHeadBody(xmlNodePtr pSkipElement, SCROW& rRow)
{
    for (xmlNodePtr cur_node = pSkipElement->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            OString aNodeName = toString(cur_node->name);
            if (aNodeName == "tr")
            {
                handleRow(cur_node, rRow);
                ++rRow;
            }

        }
    }
}

void HTMLFetchThread::handleTable(xmlNodePtr pTable)
{
    sal_Int32 nRow = 0;
    for (xmlNodePtr cur_node = pTable->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            OString aNodeName = toString(cur_node->name);
            if (aNodeName == "tr")
            {
                handleRow(cur_node, nRow);
                ++nRow;
            }
            else if (aNodeName == "thead" || aNodeName == "tbody")
            {
                skipHeadBody(cur_node, nRow);
            }
        }
    }
}

void HTMLFetchThread::execute()
{
    OStringBuffer aBuffer(64000);
    std::unique_ptr<SvStream> pStream = DataProvider::FetchStreamFromURL(maURL, aBuffer);

    htmlDocPtr pHtmlPtr = htmlParseDoc(reinterpret_cast<xmlChar*>(const_cast<char*>(aBuffer.getStr())), nullptr);

    OString aID = OUStringToOString(maID, RTL_TEXTENCODING_UTF8);
    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pHtmlPtr);
    xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(aID.getStr()), pXmlXpathCtx);
    xmlNodeSetPtr pXmlNodes = pXmlXpathObj->nodesetval;

    if (pXmlNodes->nodeNr == 0)
        return;

    xmlNodePtr pNode = pXmlNodes->nodeTab[0];
    handleTable(pNode);

    xmlXPathFreeNodeSet(pXmlNodes);
    xmlXPathFreeNodeSetList(pXmlXpathObj);
    xmlXPathFreeContext(pXmlXpathCtx);

    SolarMutexGuard aGuard;
    mpIdle->Start();
}

HTMLDataProvider::HTMLDataProvider(ScDocument* pDoc, const OUString& rURL, ScDBDataManager* pDBManager,
        const OUString& rID):
    maID(rID),
    maURL(rURL),
    mpDocument(pDoc),
    mpDBDataManager(pDBManager),
    maIdle("HTMLDataProvider CopyHandler")
{
    maIdle.SetInvokeHandler(LINK(this, HTMLDataProvider, ImportFinishedHdl));
}

HTMLDataProvider::~HTMLDataProvider()
{
    if (mxHTMLFetchThread.is())
    {
        mxHTMLFetchThread->join();
    }
}

void HTMLDataProvider::Import()
{
    // already importing data
    if (mpDoc)
        return;

    mpDoc.reset(new ScDocument(SCDOCMODE_CLIP));
    mpDoc->ResetClip(mpDocument, (SCTAB)0);
    mxHTMLFetchThread = new HTMLFetchThread(*mpDoc, maURL, maID, &maIdle);
    mxHTMLFetchThread->launch();
}

IMPL_LINK_NOARG(HTMLDataProvider, ImportFinishedHdl, Timer*, void)
{
    mpDBDataManager->WriteToDoc(*mpDoc);
    mxHTMLFetchThread.clear();
    mpDoc.reset();
}

const OUString& HTMLDataProvider::GetURL() const
{
    return maURL;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
