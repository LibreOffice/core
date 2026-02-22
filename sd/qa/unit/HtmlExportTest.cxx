/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/htmltesttools.hxx>

using namespace css;

class SdHTMLFilterTest : public UnoApiTest, public HtmlTestTools
{
public:
    SdHTMLFilterTest()
        : UnoApiTest(u"/sd/qa/unit/data/"_ustr)
    {
    }

    void testHTMLExport()
    {
        loadFromFile(u"HtmlExportTestDocument.odp");
        save(TestFilter::HTML_IMPRESS);
        htmlDocUniquePtr htmlDoc = parseHtml(maTempFile);

        assertXPath(htmlDoc, "/html", 1);
        assertXPath(htmlDoc, "/html/body", 1);
        assertXPath(htmlDoc, "/html/body/h1", 4);
        assertXPath(htmlDoc, "/html/body/table", 1);
        assertXPath(htmlDoc, "/html/body/table/tr", 5);
        assertXPath(htmlDoc, "/html/body/ul", 1);
        assertXPath(htmlDoc, "/html/body/ul/li", 2);

        assertXPath(htmlDoc, "/html/head/meta[1]", "content", u"text/html; charset=utf-8");
        assertXPath(htmlDoc, "/html/head/meta[2]", "name", u"generator");
        assertXPath(htmlDoc, "/html/head/meta[3]", "name", u"created");
        assertXPath(htmlDoc, "/html/head/meta[3]", "content", u"2014-04-09T17:05:41.987922038");
    }

    void testTdf154989()
    {
        loadFromFile(u"tdf154989.odg");
        save(TestFilter::XHTML_DRAW);
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
                           u"before");
        assertXPathContent(pXmlDoc,
                           "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div[3]/xhtml:div/xhtml:p",
                           u"above");
        assertXPathContent(pXmlDoc,
                           "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div[4]/xhtml:div/xhtml:p",
                           u"below");
    }

    CPPUNIT_TEST_SUITE(SdHTMLFilterTest);
    CPPUNIT_TEST(testHTMLExport);
    CPPUNIT_TEST(testTdf154989);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdHTMLFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
