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
#include <unotools/syslocaleoptions.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>

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

    // tdf#158703: Typing ":" after the spaces should start auto-correction, which is expected to
    // remove the spaces, and insert an NBSP instead. It must not crash.
    emulateTyping(*pTextDoc, u"Foo             :");
    CPPUNIT_ASSERT_EQUAL(u"Foo\u00A0:"_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // tdf#38394: testing autocorrect of French l'" -> l'« (instead of l'»)
    emulateTyping(*pTextDoc, u"l'\"");
    OUString sReplaced(u"l\u2019« "_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // tdf#132301 autocorrect of qu'«
    emulateTyping(*pTextDoc, u" qu'\"");
    sReplaced += u" qu\u2019« ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
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

    // Reset
    officecfg::Office::Common::AutoCorrect::SingleQuoteAtStart::set(0, pBatch);
    officecfg::Office::Common::AutoCorrect::SingleQuoteAtEnd::set(0, pBatch);
    pBatch->commit();

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // tdf#128860: Second level ending quote: ‚word' -> ,word‘
    emulateTyping(*pTextDoc, u",word'");
    OUString sReplaced2(u",word\u2019"_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
    // tdf#128860: Us apostrophe without preceding starting quote: word' -> word’
    emulateTyping(*pTextDoc, u" word'");
    sReplaced2 += u" word\u2019";
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
    // tdf#128860: But only after letters: word.' -> word.‘
    emulateTyping(*pTextDoc, u" word.'");
    sReplaced2 += u" word.‘";
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwAutoCorrect, ru_RU)
{
    createSwDoc("ru-RU.fodt");
    SwDoc* pDoc = getSwDoc();

    // On Windows, it will detect that system input language is en-US (despite "typing" e.g. Cyrillic characters),
    // and will change Russian into English (US); in the latter language,
    // the replacement from single quote will not become “, but ’.
    SvtSysLocaleOptions aOptions;
    aOptions.SetIgnoreLanguageChange(true);
    aOptions.Commit();

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    // tdf#123786: Second level ending quote: „word' -> „word“
    emulateTyping(*pTextDoc, u"„слово'");
    OUString sReplaced(u"„слово“"_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // Us apostrophe without preceding starting quote: word' -> word’
    emulateTyping(*pTextDoc, u" слово'");
    sReplaced += u" слово’";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // But only after letters: word.' -> word.“
    emulateTyping(*pTextDoc, u" слово.'");
    sReplaced += u" слово.“";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwAutoCorrect, en_US)
{
    // Default lang is en-US
    createSwDoc();

    SwXTextDocument& rTextDoc = dynamic_cast<SwXTextDocument&>(*mxComponent);

    // tdf#106164: testing autocorrect of we're -> We're on start of first paragraph
    emulateTyping(rTextDoc, u"we're ");
    CPPUNIT_ASSERT_EQUAL(u"We\u2019re "_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // tdf#59666: Testing missing autocorrect of single Greek letters
    emulateTyping(rTextDoc, u"π ");
    CPPUNIT_ASSERT_EQUAL(u"\u03C0 "_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    //tdf#74363: testing autocorrect of initial capitals on start of first paragraph
    //Inserting one all-lowercase word into the first paragraph
    emulateTyping(rTextDoc, u"testing ");
    //The word should be capitalized due to autocorrect
    CPPUNIT_ASSERT_EQUAL(u"Testing "_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    emulateTyping(rTextDoc, u"Foo - 11’--’22 ");
    // tdf#155407: Without the fix in place, this would fail with
    // - Expected: Foo – 11’—’22
    // - Actual  : Foo – 11’--’22
    CPPUNIT_ASSERT_EQUAL(u"Foo – 11’—’22 "_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    emulateTyping(rTextDoc, u"Bar -- 111--222 ");
    // tdf#155407: Without the fix in place, this would fail with
    // - Expected: Bar – 111–222
    // - Actual  : Bar – 111-–22
    CPPUNIT_ASSERT_EQUAL(u"Bar – 111–222 "_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    emulateTyping(rTextDoc, u"This - is replaced. - But this is not replaced ");
    // tdf#159797: Without the fix in place, this would fail with
    // - Expected: This – is replaced. – But this is not replaced.
    // - Actual  : This – is replaced. - But this is not replaced.
    CPPUNIT_ASSERT_EQUAL(u"This – is replaced. – But this is not replaced "_ustr,
                         getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Check quotation marks are added to the selection
    emulateTyping(rTextDoc, u"\"");

    CPPUNIT_ASSERT_EQUAL(u"\u201CThis – is replaced. – But this is not replaced \u201D"_ustr,
                         getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Use delete before typing the quotation mark
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    // tdf#54409: testing autocorrect of "tset -> "test with typographical double quotation mark U+201C
    emulateTyping(rTextDoc, u"\"test ");
    OUString sReplaced(u"\u201Ctest "_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // testing autocorrect of test" -> test" with typographical double quotation mark U+201D
    emulateTyping(rTextDoc, u"and tset\" ");
    OUString sReplaced2(sReplaced + u"and test\u201D ");
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
    // testing autocorrect of "tset" -> "test" with typographical double quotation mark U+201C and U+201D
    emulateTyping(rTextDoc, u"\"tset\" ");
    OUString sReplaced3(sReplaced2 + u"\u201Ctest\u201D ");
    CPPUNIT_ASSERT_EQUAL(sReplaced3, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // testing autocorrect of i' -> I' on start of first paragraph
    emulateTyping(rTextDoc, u"i'");
    // tdf#108423: The word "i" should be capitalized due to autocorrect, followed by a typographical apostrophe
    OUString sIApostrophe(u"I\u2019"_ustr);
    CPPUNIT_ASSERT_EQUAL(sIApostrophe, getParagraph(1)->getString());
    emulateTyping(rTextDoc, u" i'");
    OUString sText(sIApostrophe + u" " + sIApostrophe);
    CPPUNIT_ASSERT_EQUAL(sText, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // tdf#57640: Without the fix in place, this test would have failed with
    // - Expected: ǅ
    // - Actual  : Ǆ
    emulateTyping(rTextDoc, u"ǆ  ");
    CPPUNIT_ASSERT_EQUAL(u"ǅ  "_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwAutoCorrect, hu_HU)
{
    createSwDoc("hu-HU.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    // tdf#133524: 1. Testing autocorrect of >> and <<
    // Example: »word«
    emulateTyping(*pTextDoc, u">>");
    OUString sReplaced(u"»"_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // <<
    emulateTyping(*pTextDoc, u"word<<");
    sReplaced += u"word«";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // tdf#133524: 2. Testing autocorrect of " to >> and << inside „...”
    // Example: „Sentence and »word«.”
    // opening primary level quote
    emulateTyping(*pTextDoc, u" \"");
    sReplaced += u" „";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // opening second level quote
    emulateTyping(*pTextDoc, u"Sentence and \"");
    sReplaced += u"Sentence and »";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // closing second level quote
    emulateTyping(*pTextDoc, u"word\"");
    sReplaced += u"word«";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // closing primary level quote
    emulateTyping(*pTextDoc, u".\"");
    sReplaced += u".”";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // tdf#134940 avoid premature replacement of "--" in "-->"
    emulateTyping(*pTextDoc, u" -->");
    OUString sReplaced2(sReplaced + u" -->");
    // This was "–>" instead of "-->"
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
    emulateTyping(*pTextDoc, u" ");
    sReplaced += u" → ";
    // This was "–>" instead of "→"
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // tdf#83037
    emulateTyping(*pTextDoc, u"-> ");
    sReplaced += u"→ ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    emulateTyping(*pTextDoc, u"<- ");
    sReplaced += u"← ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    emulateTyping(*pTextDoc, u"<-- ");
    sReplaced += u"← ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    emulateTyping(*pTextDoc, u"<--> ");
    sReplaced += u"↔ ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwAutoCorrect, ro_RO)
{
    createSwDoc("ro-RO.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    // tdf#133524: 1. Testing autocorrect of " to << and >> inside „...”
    // Example: „Sentence and «word».”
    // opening primary level quote
    emulateTyping(*pTextDoc, u"\"");
    OUString sReplaced(u"„"_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // opening second level quote
    emulateTyping(*pTextDoc, u"Sentence and \"");
    sReplaced += u"Sentence and «";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // closing second level quote
    emulateTyping(*pTextDoc, u"word\"");
    sReplaced += u"word»";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // closing primary level quote
    emulateTyping(*pTextDoc, u".\"");
    sReplaced += u".”";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // 2. Testing recognition of closing double quotation mark ”
    emulateTyping(*pTextDoc, u" \"");
    sReplaced += u" „";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // 3. Testing recognition of alternative closing double quotation mark “
    emulateTyping(*pTextDoc, u"Alternative.“ \"");
    sReplaced += u"Alternative.“ „";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
