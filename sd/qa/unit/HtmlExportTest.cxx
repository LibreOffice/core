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

class SdXHTMLFilterTest : public UnoApiXmlTest, public HtmlTestTools
{
public:
    SdXHTMLFilterTest()
        : UnoApiXmlTest("/sd/qa/unit/data/")
    {
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

    CPPUNIT_TEST_SUITE(SdXHTMLFilterTest);
    CPPUNIT_TEST(testTdf154989);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdXHTMLFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
