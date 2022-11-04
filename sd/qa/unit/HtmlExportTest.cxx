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

    CPPUNIT_TEST_SUITE(SdHTMLFilterTest);
    CPPUNIT_TEST(testHTMLExport);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdHTMLFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
