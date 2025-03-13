/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>

#include <comphelper/configuration.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf165642_glossaryFootnote)
{
    loadAndSave("tdf165642_glossaryFootnote.docx");
    // round-trip'ing the settings.xml file as is, it contains footnote/endnote references
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/glossary/settings.xml"_ustr);
    assertXPath(pXmlSettings, "//w:endnotePr", 1);
    assertXPath(pXmlSettings, "//w:footnotePr", 1);

    // thus, the footnote and endnote files must also be round-tripped
    parseExport(u"word/glossary/endnotes.xml"_ustr);
    parseExport(u"word/glossary/footnotes.xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf83844)
{
    createSwDoc("tdf83844.fodt");

    auto fnVerify = [this] {
        auto pXmlDoc = parseLayoutDump();

        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                    u"A A A A ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                    u"B B B B B B B B ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]", "portion",
                    u"C C C C C C C C ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[4]", "portion",
                    u"D D D D");
    };

    fnVerify();
    saveAndReload(mpFilter);
    fnVerify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf83844Hanging)
{
    createSwDoc("tdf83844-hanging.fodt");

    auto fnVerify = [this] {
        auto pXmlDoc = parseLayoutDump();

        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                    u"A A A A A A A A A ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                    u"B B B B B B ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]", "portion",
                    u"C C C C C C C ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[4]", "portion",
                    u"D D");
    };

    fnVerify();
    saveAndReload(mpFilter);
    fnVerify();
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
