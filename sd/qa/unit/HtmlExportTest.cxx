/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>
#include <test/htmltesttools.hxx>

using namespace css;

class SdHTMLFilterTest : public UnoApiXmlTest, public HtmlTestTools
{
public:
    SdHTMLFilterTest()
        : UnoApiXmlTest("/sd/qa/unit/data/")
    {
    }

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        xmlXPathRegisterNs(pXmlXPathCtx, BAD_CAST("xhtml"),
                           BAD_CAST("http://www.w3.org/1999/xhtml"));
    }

    void testHTMLExport()
    {
        loadFromURL(u"HtmlExportTestDocument.odp");
        save("impress_html_Export");
        htmlDocUniquePtr htmlDoc = parseHtml(maTempFile);

        assertXPath(htmlDoc, "/html", 1);
        assertXPath(htmlDoc, "/html/body", 1);
        assertXPath(htmlDoc, "/html/body/h1", 4);
        assertXPath(htmlDoc, "/html/body/table", 1);
        assertXPath(htmlDoc, "/html/body/table/tr", 5);
        assertXPath(htmlDoc, "/html/body/ul", 1);
        assertXPath(htmlDoc, "/html/body/ul/li", 2);

        assertXPath(htmlDoc, "/html/head/meta[1]", "content", "text/html; charset=utf-8");
        assertXPath(htmlDoc, "/html/head/meta[2]", "name", "generator");
        assertXPath(htmlDoc, "/html/head/meta[3]", "name", "created");
        assertXPath(htmlDoc, "/html/head/meta[3]", "content", "2014-04-09T17:05:41.987922038");
    }

    void testTdf154989()
    {
        loadFromURL(u"tdf154989.odg");
        save("XHTML Draw File");
        xmlDocUniquePtr pXmlDoc = parseXml(maTempFile);

        assertXPath(pXmlDoc, "/xhtml:html", 1);
        // 1 page
        assertXPath(pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div", 1);
        // 4 shapes
        assertXPath(pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div", 4);
        // 3 of them are text boxes
        assertXPath(pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div/xhtml:div", 3);
        // not sure if the order of these is stable?
        assertXPathContent(pXmlDoc,
                           "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div[2]/xhtml:div/xhtml:p",
                           "before");
        assertXPathContent(
            pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div[3]/xhtml:div/xhtml:p", "above");
        assertXPathContent(
            pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div[4]/xhtml:div/xhtml:p", "below");
    }

    CPPUNIT_TEST_SUITE(SdHTMLFilterTest);
    CPPUNIT_TEST(testHTMLExport);
    CPPUNIT_TEST(testTdf154989);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdHTMLFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
