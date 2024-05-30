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
        : UnoApiXmlTest(u"/sd/qa/unit/data/"_ustr)
    {
    }

    void testHTMLExport()
    {
        loadFromFile(u"HtmlExportTestDocument.odp");
        save(u"impress_html_Export"_ustr);
        htmlDocUniquePtr htmlDoc = parseHtml(maTempFile);

        assertXPath(htmlDoc, "/html"_ostr, 1);
        assertXPath(htmlDoc, "/html/body"_ostr, 1);
        assertXPath(htmlDoc, "/html/body/h1"_ostr, 4);
        assertXPath(htmlDoc, "/html/body/table"_ostr, 1);
        assertXPath(htmlDoc, "/html/body/table/tr"_ostr, 5);
        assertXPath(htmlDoc, "/html/body/ul"_ostr, 1);
        assertXPath(htmlDoc, "/html/body/ul/li"_ostr, 2);

        assertXPath(htmlDoc, "/html/head/meta[1]"_ostr, "content"_ostr,
                    u"text/html; charset=utf-8"_ustr);
        assertXPath(htmlDoc, "/html/head/meta[2]"_ostr, "name"_ostr, u"generator"_ustr);
        assertXPath(htmlDoc, "/html/head/meta[3]"_ostr, "name"_ostr, u"created"_ustr);
        assertXPath(htmlDoc, "/html/head/meta[3]"_ostr, "content"_ostr,
                    u"2014-04-09T17:05:41.987922038"_ustr);
    }

    void testTdf154989()
    {
        loadFromFile(u"tdf154989.odg");
        save(u"XHTML Draw File"_ustr);
        xmlDocUniquePtr pXmlDoc = parseXml(maTempFile);

        assertXPath(pXmlDoc, "/xhtml:html"_ostr, 1);
        // 1 page
        assertXPath(pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div"_ostr, 1);
        // 4 shapes
        assertXPath(pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div"_ostr, 4);
        // 3 of them are text boxes
        assertXPath(pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div/xhtml:div"_ostr, 3);
        // not sure if the order of these is stable?
        assertXPathContent(
            pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div[2]/xhtml:div/xhtml:p"_ostr,
            u"before"_ustr);
        assertXPathContent(
            pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div[3]/xhtml:div/xhtml:p"_ostr,
            u"above"_ustr);
        assertXPathContent(
            pXmlDoc, "/xhtml:html/xhtml:body/xhtml:div[1]/xhtml:div[4]/xhtml:div/xhtml:p"_ostr,
            u"below"_ustr);
    }

    CPPUNIT_TEST_SUITE(SdHTMLFilterTest);
    CPPUNIT_TEST(testHTMLExport);
    CPPUNIT_TEST(testTdf154989);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdHTMLFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
