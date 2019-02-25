/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "htmldataprovider.hxx"
#include <datamapper.hxx>
#include <datatransformation.hxx>
#include <salhelper/thread.hxx>
#include <vcl/svapp.hxx>
#include <tools/stream.hxx>

#include <libxml/HTMLparser.h>

#include <libxml/xpath.h>

#include <comphelper/string.hxx>
#include <utility>

namespace sc {

class HTMLFetchThread : public salhelper::Thread
{
    ScDocument& mrDocument;
    OUString const maURL;
    OUString const maID;
    const std::vector<std::shared_ptr<sc::DataTransformation>> maDataTransformations;
    std::function<void()> const maImportFinishedHdl;

    void handleTable(xmlNodePtr pTable);
    void handleRow(xmlNodePtr pRow, SCROW nRow);
    void skipHeadBody(xmlNodePtr pSkip, SCROW& rRow);
    void handleCell(xmlNodePtr pCell, SCROW nRow, SCCOL nCol);

public:
    HTMLFetchThread(ScDocument& rDoc, const OUString&, const OUString& rID, std::function<void()> aImportFinishedHdl,
            const std::vector<std::shared_ptr<sc::DataTransformation>>& rTransformations);

    virtual void execute() override;
};

HTMLFetchThread::HTMLFetchThread(
    ScDocument& rDoc, const OUString& rURL, const OUString& rID,
    std::function<void()> aImportFinishedHdl,
    const std::vector<std::shared_ptr<sc::DataTransformation>>& rTransformations)
    : salhelper::Thread("HTML Fetch Thread")
    , mrDocument(rDoc)
    , maURL(rURL)
    , maID(rID)
    , maDataTransformations(rTransformations)
    , maImportFinishedHdl(std::move(aImportFinishedHdl))
{
}

namespace {

OString toString(const xmlChar* pStr)
{
    return OString(reinterpret_cast<const char*>(pStr), xmlStrlen(pStr));
}

OUString trim_string(const OUString& aStr)
{
    OUString aOldString;
    OUString aString = aStr;
    do
    {
        aOldString = aString;
        aString = comphelper::string::strip(aString, ' ');
        aString = comphelper::string::strip(aString, '\n');
        aString = comphelper::string::strip(aString, '\r');
        aString = comphelper::string::strip(aString, '\t');
    }
    while (aOldString != aString);

    return aString;
}

OUString get_node_str(xmlNodePtr pNode)
{
    OUStringBuffer aStr;
    for (xmlNodePtr cur_node = pNode->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_TEXT_NODE)
        {
            OUString aString = OStringToOUString(toString(cur_node->content), RTL_TEXTENCODING_UTF8);
            aStr.append(trim_string(aString));
        }
        else if (cur_node->type == XML_ELEMENT_NODE)
        {
            aStr.append(get_node_str(cur_node));
        }
    }

    return aStr.makeStringAndClear();
}

}

void HTMLFetchThread::handleCell(xmlNodePtr pCellNode, SCROW nRow, SCCOL nCol)
{
    OUStringBuffer aStr;
    for (xmlNodePtr cur_node = pCellNode->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_TEXT_NODE)
        {
            OUString aString = OStringToOUString(toString(cur_node->content), RTL_TEXTENCODING_UTF8);
            aStr.append(trim_string(aString));
        }
        else if (cur_node->type == XML_ELEMENT_NODE)
        {
            aStr.append(get_node_str(cur_node));
        }
    }

    if (!aStr.isEmpty())
    {
        OUString aCellStr = aStr.makeStringAndClear();
        mrDocument.SetString(nCol, nRow, 0, aCellStr);
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

    if (aBuffer.isEmpty())
        return;

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

    for (auto& itr : maDataTransformations)
    {
        itr->Transform(mrDocument);
    }

    SolarMutexGuard aGuard;
    maImportFinishedHdl();
}

HTMLDataProvider::HTMLDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource):
    DataProvider(rDataSource),
    mpDocument(pDoc)
{
}

HTMLDataProvider::~HTMLDataProvider()
{
    if (mxHTMLFetchThread.is())
    {
        SolarMutexReleaser aReleaser;
        mxHTMLFetchThread->join();
    }
}

void HTMLDataProvider::Import()
{
    // already importing data
    if (mpDoc)
        return;

    mpDoc.reset(new ScDocument(SCDOCMODE_CLIP));
    mpDoc->ResetClip(mpDocument, SCTAB(0));
    mxHTMLFetchThread = new HTMLFetchThread(*mpDoc, mrDataSource.getURL(), mrDataSource.getID(),
            std::bind(&HTMLDataProvider::ImportFinished, this), mrDataSource.getDataTransformation());
    mxHTMLFetchThread->launch();

    if (mbDeterministic)
    {
        SolarMutexReleaser aReleaser;
        mxHTMLFetchThread->join();
    }
}

std::map<OUString, OUString> HTMLDataProvider::getDataSourcesForURL(const OUString& /*rURL*/)
{
    std::map<OUString, OUString> aMap;

    OStringBuffer aBuffer(64000);
    std::unique_ptr<SvStream> pStream = DataProvider::FetchStreamFromURL(mrDataSource.getURL(), aBuffer);

    if (aBuffer.isEmpty())
        return std::map<OUString, OUString>();

    htmlDocPtr pHtmlPtr = htmlParseDoc(reinterpret_cast<xmlChar*>(const_cast<char*>(aBuffer.getStr())), nullptr);

    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pHtmlPtr);
    xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST("//table"), pXmlXpathCtx);
    xmlNodeSetPtr pXmlNodes = pXmlXpathObj->nodesetval;

    for (int i = 0; i < pXmlNodes->nodeNr; ++i)
    {
        xmlChar* pVal = xmlGetProp(pXmlNodes->nodeTab[i], BAD_CAST("id"));

        if (pVal)
        {
            OUString aID = OStringToOUString(toString(pVal), RTL_TEXTENCODING_UTF8);
            aMap.insert(std::pair<OUString, OUString>(aID, "//table[@id=\""+ aID + "\""));
            xmlFree(pVal);
        }
    }

    return aMap;
}

void HTMLDataProvider::ImportFinished()
{
    mrDataSource.getDBManager()->WriteToDoc(*mpDoc);
    mxHTMLFetchThread.clear();
    mpDoc.reset();
}

const OUString& HTMLDataProvider::GetURL() const
{
    return mrDataSource.getURL();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
