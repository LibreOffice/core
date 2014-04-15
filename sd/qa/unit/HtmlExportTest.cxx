/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

#include <rtl/byteseq.hxx>
#include <boost/scoped_array.hpp>

using namespace css;
using namespace rtl;

class SdHTMLFilterTest : public SdModelTestBase
{
    htmlDocPtr parseHtml(utl::TempFile& aTempFile)
    {
        SvFileStream aFileStream(aTempFile.GetURL(), STREAM_READ);
        sal_Size nSize = aFileStream.remainingSize();

        boost::scoped_array<sal_uInt8> pBuffer(new sal_uInt8[nSize + 1]);

        aFileStream.Read(pBuffer.get(), nSize);

        pBuffer[nSize] = 0;
        printf("Content: %s\n", reinterpret_cast<char*>(pBuffer.get()));
        return htmlParseDoc(reinterpret_cast<xmlChar*>(pBuffer.get()), NULL);
    }

    xmlNodeSetPtr getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath)
    {
        xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc);
        xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(rXPath.getStr()), pXmlXpathCtx);
        return pXmlXpathObj->nodesetval;
    }

    void assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfNodes)
    {
        xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                     nNumberOfNodes, xmlXPathNodeSetGetLength(pXmlNodes));
    }

    htmlDocPtr exportAndparseHtml(sd::DrawDocShellRef& xDocShRef)
    {
        FileFormat* pFormat = getFormat(HTML);
        OUString aExt = OUString( "." ) + OUString::createFromAscii(pFormat->pName);
        utl::TempFile aTempFile(OUString(), &aExt);
        aTempFile.EnableKillingFile();
        exportTo(xDocShRef, pFormat, aTempFile);
        return parseHtml(aTempFile);
    }

public:

    void testHTMLExport()
    {
        sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/HtmlExportTestDocument.odp"));
        htmlDocPtr htmlDoc = exportAndparseHtml(xDocShRef);

        assertXPath(htmlDoc, "/html", 1);
        assertXPath(htmlDoc, "/html/body", 1);
        assertXPath(htmlDoc, "/html/body/h1", 4);
        assertXPath(htmlDoc, "/html/body/table", 1);
        assertXPath(htmlDoc, "/html/body/table/tr", 5);
        assertXPath(htmlDoc, "/html/body/ul", 1);
        assertXPath(htmlDoc, "/html/body/ul/li", 2);
    }

    CPPUNIT_TEST_SUITE(SdHTMLFilterTest);
    CPPUNIT_TEST(testHTMLExport);
    CPPUNIT_TEST_SUITE_END();
};



CPPUNIT_TEST_SUITE_REGISTRATION(SdHTMLFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
