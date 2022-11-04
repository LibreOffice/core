/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <test/htmltesttools.hxx>
#include <test/unoapixml_test.hxx>
#include <comphelper/processfactory.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace utl;

class ScHTMLExportTest : public UnoApiXmlTest, public HtmlTestTools
{
public:
    ScHTMLExportTest()
        : UnoApiXmlTest("/sc/qa/extras/testdocuments/")
    {}

    void testHtmlSkipImage()
    {
        loadFromURL(u"BaseForHTMLExport.ods");
        save("HTML (StarCalc)");
        htmlDocUniquePtr pDoc = parseHtml(maTempFile);
        CPPUNIT_ASSERT (pDoc);

        assertXPath(pDoc, "/html/body", 1);
        assertXPath(pDoc, "/html/body/table/tr/td/img", 1);

        setFilterOptions("SkipImages");
        save("HTML (StarCalc)");

        pDoc = parseHtml(maTempFile);
        CPPUNIT_ASSERT (pDoc);
        assertXPath(pDoc, "/html/body", 1);
        assertXPath(pDoc, "/html/body/table/tr/td/img", 0);
    }

    CPPUNIT_TEST_SUITE(ScHTMLExportTest);
    CPPUNIT_TEST(testHtmlSkipImage);
    CPPUNIT_TEST_SUITE_END();

};

CPPUNIT_TEST_SUITE_REGISTRATION(ScHTMLExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
