/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/configuration.hxx>
#include <docsh.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotxdoc.hxx>

// If you want to add a test for a language that doesn't exists yet
// copy an existing document and adapt "fo:language" and "fo:country"

class SwAutoCorrect : public SwModelTestBase
{
public:
    SwAutoCorrect()
        : SwModelTestBase(u"/sw/qa/extras/autocorrect/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwAutoCorrect, nl_BE)
{
    createSwDoc("nl-BE.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    // tdf#92029: Without the fix in place, this test would have failed with
    // - Expected: Ik ben ’s morgens opgestaan
    // - Actual  : Ik ben ‘s morgens opgestaan
    emulateTyping(*pTextDoc, u"Ik ben 's morgens opgestaan");
    OUString sReplaced(u"Ik ben ’s morgens opgestaan"_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwAutoCorrect, fr_FR)
{
    createSwDoc("fr-FR.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    dispatchCommand(mxComponent, u".uno:GoToEndOfDoc"_ustr, {});

    // tdf#158703: Typing ":" after the spaces should start auto-correction, which is expected to
    // remove the spaces, and insert an NBSP instead. It must not crash.
    emulateTyping(*pTextDoc, u"Foo             :");
    CPPUNIT_ASSERT_EQUAL(u"Foo\u00A0:"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwAutoCorrect, pt_BR)
{
    createSwDoc("pt-BR.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // tdf#44293
    emulateTyping(*pTextDoc, u"1a 1o ");
    CPPUNIT_ASSERT_EQUAL(u"1.a 1.o "_ustr, getParagraph(1)->getString());
    emulateTyping(*pTextDoc, u"1ra 1ro ");
    CPPUNIT_ASSERT_EQUAL(u"1.a 1.o 1.a 1.o "_ustr, getParagraph(1)->getString());
    emulateTyping(*pTextDoc, u"43as 43os 43ras 43ros ");
    CPPUNIT_ASSERT_EQUAL(u"1.a 1.o 1.a 1.o 43.as 43.os 43.as 43.os "_ustr,
                         getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwAutoCorrect, de_DE)
{
    Resetter resetter([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::AutoCorrect::SingleQuoteAtStart::set(0, pBatch);
        officecfg::Office::Common::AutoCorrect::SingleQuoteAtEnd::set(0, pBatch);
        return pBatch->commit();
    });
    // Set Single Quotes › and ‹
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::AutoCorrect::SingleQuoteAtStart::set(8250, pBatch);
    officecfg::Office::Common::AutoCorrect::SingleQuoteAtEnd::set(8249, pBatch);
    pBatch->commit();

    createSwDoc("de-DE.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    // tdf#151801: Single starting quote: 'word -> ›word
    emulateTyping(*pTextDoc, u"'word");
    OUString sReplaced(u"\u203Aword"_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // tdf#151801: Single ending quote: ›word' -> ›word‹
    emulateTyping(*pTextDoc, u"'");
    sReplaced += u"\u2039";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // tdf#151801: Use apostrophe without preceding starting quote: word' -> word’
    emulateTyping(*pTextDoc, u" word'");
    sReplaced += u" word\u2019";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
