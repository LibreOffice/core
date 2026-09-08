/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cstring>
#include <string_view>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/text/XText.hpp>

#include <osl/file.hxx>
#include <rtl/string.hxx>
#include <test/unoapi_test.hxx>

#include <lib/l10ntranslate.hxx>

using namespace com::sun::star;
using namespace ::cpo;
using namespace desktop::l10n;

/* Tests for the translate-on-copy utility behind
 * kit::Office::translateDocument().
 *
 * The keyDerivation / msgstrTokens / sheetRefRewrite cases below are the C++
 * half of the shared parity table; the Python half lives in
 * online browser/introdocs/tests/test_l10n_tooling.py and must be kept in
 * sync case by case.  A change to the key spec must touch both files.
 */
class L10nTranslateTest : public UnoApiTest
{
public:
    L10nTranslateTest()
        : UnoApiTest(u"/desktop/qa/data/"_ustr)
    {
    }

    OUString tempFileUrl(const char* pName)
    {
        OUString aDirUrl;
        CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, osl::FileBase::getTempDirURL(aDirUrl));
        return aDirUrl + "/cokit-l10ntranslate-" + OUString::createFromAscii(pName);
    }

    void testKeyDerivation();
    void testMsgstrTokens();
    void testXmlSafeText();
    void testL10nStreamParsing();
    void testTranslateFull();
    void testTranslateMissAndUnmatchedLocale();
    void testIllegalXmlCharacterInTranslation();
    void testRetranslateInPlace();
    void testTemplatelessFallback();
    void testUnreadableStreamStripsMarkers();
    void testEmptyLanguageStripsMarkers();
    void testRegionLocaleWinsOverBase();
    void testUnusableSheetNameKeepsSource();
    void testEmptyTranslationKeepsSource();
    void testTextlessTranslationKeepsSource();
    void testEditedFileIsLeftAlone();
    void testBrokenTemplateFallsBackToMember();
    void testNoStreamByteIdentical();
    void testZipFidelity();
    void testLoadTranslatedSmoke();

    CPPUNIT_TEST_SUITE(L10nTranslateTest);
    CPPUNIT_TEST(testKeyDerivation);
    CPPUNIT_TEST(testMsgstrTokens);
    CPPUNIT_TEST(testXmlSafeText);
    CPPUNIT_TEST(testL10nStreamParsing);
    CPPUNIT_TEST(testTranslateFull);
    CPPUNIT_TEST(testTranslateMissAndUnmatchedLocale);
    CPPUNIT_TEST(testIllegalXmlCharacterInTranslation);
    CPPUNIT_TEST(testRetranslateInPlace);
    CPPUNIT_TEST(testTemplatelessFallback);
    CPPUNIT_TEST(testUnreadableStreamStripsMarkers);
    CPPUNIT_TEST(testEmptyLanguageStripsMarkers);
    CPPUNIT_TEST(testRegionLocaleWinsOverBase);
    CPPUNIT_TEST(testUnusableSheetNameKeepsSource);
    CPPUNIT_TEST(testEmptyTranslationKeepsSource);
    CPPUNIT_TEST(testTextlessTranslationKeepsSource);
    CPPUNIT_TEST(testEditedFileIsLeftAlone);
    CPPUNIT_TEST(testBrokenTemplateFallsBackToMember);
    CPPUNIT_TEST(testNoStreamByteIdentical);
    CPPUNIT_TEST(testZipFidelity);
    CPPUNIT_TEST(testLoadTranslatedSmoke);
    CPPUNIT_TEST_SUITE_END();
};

void L10nTranslateTest::testKeyDerivation()
{
    // Parity cases K1..K22 (keep in sync with test_l10n_tooling.py)
    static const struct
    {
        const char* pId;
        const char* pXml;
        UnitState eState;
        const char* pKey;
    } aCases[] = {
        { "K1",
          "<text:p><text:span text:style-name=\"T5\">_3. Press </text:span>"
          "<text:span text:style-name=\"T6\">Enter</text:span>"
          "<text:span text:style-name=\"T7\"> to calculate the total number of items</text:span>"
          "</text:p>",
          UnitState::Marked,
          "<1>3. Press </1><2>Enter</2><3> to calculate the total number of items</3>" },
        { "K2", "<text:p><text:span text:style-name=\"T1\">_Hello</text:span></text:p>",
          UnitState::Marked, "Hello" },
        { "K3", "<text:p>_Plain paragraph text</text:p>", UnitState::Marked,
          "Plain paragraph text" },
        { "K4",
          "<text:p><text:span text:style-name=\"T1\">_</text:span>"
          "<text:span text:style-name=\"T2\">\xe2\x9e\x9c</text:span> Try it</text:p>",
          UnitState::Marked, "<1>\xe2\x9e\x9c</1> Try it" },
        { "K5", "<text:p>__foo</text:p>", UnitState::Marked, "_foo" },
        { "K6", "<text:p>Hello</text:p>", UnitState::NotMarked, "" },
        { "K7",
          "<text:p><text:span text:style-name=\"T1\">Hello </text:span>"
          "<text:span text:style-name=\"T2\">_world</text:span></text:p>",
          UnitState::NotMarked, "" },
        { "K8", "<text:p>_a<text:s/>b</text:p>", UnitState::Forbidden, "" },
        { "K9", "<text:p>_a<text:tab/>b</text:p>", UnitState::Forbidden, "" },
        { "K10", "<text:p>_a<text:line-break/>b</text:p>", UnitState::Forbidden, "" },
        { "K11", "<text:p>_a<text:a xlink:href=\"http://x/\">link</text:a></text:p>",
          UnitState::Forbidden, "" },
        { "K12",
          "<text:p><text:span text:style-name=\"T1\">_a"
          "<text:span text:style-name=\"T2\">nested</text:span></text:span></text:p>",
          UnitState::Forbidden, "" },
        { "K13", "<text:p>_</text:p>", UnitState::Marked, "" },
        { "K14",
          "<text:p><text:span text:style-name=\"T1\">_a</text:span>"
          "<text:span text:style-name=\"T2\"></text:span>"
          "<text:span text:style-name=\"T3\">b</text:span></text:p>",
          UnitState::Marked, "<1>a</1><2>b</2>" },
        { "K15", "<text:p></text:p>", UnitState::NotMarked, "" },
        { "K16", "<text:p>_one <text:span text:style-name=\"T1\">two</text:span> three</text:p>",
          UnitState::Marked, "one <1>two</1> three" },
        { "K17", "<text:p>_hello<!-- x -->world</text:p>", UnitState::Marked, "helloworld" },
        { "K18", "<text:p>_a<!-- x -->b<text:span text:style-name=\"T1\">c</text:span></text:p>",
          UnitState::Marked, "ab<1>c</1>" },
        { "K19", "<text:p>_hello<?pi x?>world</text:p>", UnitState::Marked, "helloworld" },
        { "K20", "<text:p>_a<text:span text:style-name=\"T2\"/>b</text:p>", UnitState::Marked,
          "ab" },
        { "K21", "<text:p>_x<text:span text:style-name=\"T1\">a<!-- c -->b</text:span></text:p>",
          UnitState::Marked, "x<1>ab</1>" },
        { "K22", "<text:p>_a<![CDATA[b]]>c</text:p>", UnitState::Marked, "abc" },
    };

    for (const auto& rCase : aCases)
    {
        OString aKey;
        const UnitState eState = deriveKeyFromParagraphXml(rCase.pXml, aKey);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(rCase.pId, static_cast<int>(rCase.eState),
                                     static_cast<int>(eState));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(rCase.pId, OString(rCase.pKey), aKey);
    }
}

void L10nTranslateTest::testMsgstrTokens()
{
    // Parity cases M1..M13 (keep in sync with test_l10n_tooling.py)
    std::vector<MsgstrToken> aTokens;

    // M1: straight order
    CPPUNIT_ASSERT(parseMsgstrTokens("<1>a</1><2>b</2><3>c</3>"_ostr, 3, aTokens));
    CPPUNIT_ASSERT_EQUAL(size_t(3), aTokens.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aTokens[0].nRun);
    CPPUNIT_ASSERT_EQUAL("a"_ostr, aTokens[0].aText);

    // M2: reorder
    CPPUNIT_ASSERT(parseMsgstrTokens("<2>b</2><1>a</1>"_ostr, 2, aTokens));
    CPPUNIT_ASSERT_EQUAL(size_t(2), aTokens.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aTokens[0].nRun);

    // M3: repetition plus untagged literal
    CPPUNIT_ASSERT(parseMsgstrTokens("<1>a</1> mitte <1>b</1>"_ostr, 1, aTokens));
    CPPUNIT_ASSERT_EQUAL(size_t(3), aTokens.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aTokens[1].nRun);
    CPPUNIT_ASSERT_EQUAL(" mitte "_ostr, aTokens[1].aText);

    // M4: plain text only
    CPPUNIT_ASSERT(parseMsgstrTokens("plain text"_ostr, 3, aTokens));
    CPPUNIT_ASSERT_EQUAL(size_t(1), aTokens.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aTokens[0].nRun);

    // M5: omission
    CPPUNIT_ASSERT(parseMsgstrTokens("<1>a</1><3>c</3>"_ostr, 3, aTokens));
    CPPUNIT_ASSERT_EQUAL(size_t(2), aTokens.size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aTokens[1].nRun);

    // M6: unknown run index
    CPPUNIT_ASSERT(!parseMsgstrTokens("<4>x</4>"_ostr, 3, aTokens));
    // M7: unclosed tag
    CPPUNIT_ASSERT(!parseMsgstrTokens("<1>unclosed"_ostr, 1, aTokens));
    // M8: nesting
    CPPUNIT_ASSERT(!parseMsgstrTokens("<1>a<2>b</2></1>"_ostr, 2, aTokens));
    // M9: stray close
    CPPUNIT_ASSERT(!parseMsgstrTokens("a </2> b"_ostr, 2, aTokens));

    // M10: a lone "<" is literal text
    CPPUNIT_ASSERT(parseMsgstrTokens("a < b > c"_ostr, 1, aTokens));
    CPPUNIT_ASSERT_EQUAL(size_t(1), aTokens.size());
    CPPUNIT_ASSERT_EQUAL("a < b > c"_ostr, aTokens[0].aText);

    // M11: zero is not a valid run
    CPPUNIT_ASSERT(!parseMsgstrTokens("<0>x</0>"_ostr, 1, aTokens));

    // M12: empty msgstr parses to zero tokens
    CPPUNIT_ASSERT(parseMsgstrTokens(""_ostr, 1, aTokens));
    CPPUNIT_ASSERT(aTokens.empty());

    // M13: a tagless msgid (tag count 0) rejects any tag
    CPPUNIT_ASSERT(!parseMsgstrTokens("<1>x</1>"_ostr, 0, aTokens));
    CPPUNIT_ASSERT(parseMsgstrTokens("plain"_ostr, 0, aTokens));
    CPPUNIT_ASSERT_EQUAL(size_t(1), aTokens.size());
}

void L10nTranslateTest::testXmlSafeText()
{
    // Parity cases X1..X6 (keep in sync with test_l10n_tooling.py); X7 is
    // the encoding half, which has no Python counterpart because polib has
    // already decoded the .po by the time the check runs there.

    // X1: ordinary text, accents and astral characters are all fine
    CPPUNIT_ASSERT(isXmlSafeUtf8("Druecken Sie Enter"_ostr));
    CPPUNIT_ASSERT(isXmlSafeUtf8("Einf\xc3\xbchrung \xf0\x9f\x91\x8b"_ostr));
    // X2: the three control characters XML allows
    CPPUNIT_ASSERT(isXmlSafeUtf8("a\tb\nc\rd"_ostr));
    // X3: vertical tab and form feed are not among them
    CPPUNIT_ASSERT(!isXmlSafeUtf8("Enter\x0b"
                                  "now"_ostr));
    CPPUNIT_ASSERT(!isXmlSafeUtf8("Enter\x0c"
                                  "now"_ostr));
    // X4: NUL and the other C0 controls
    CPPUNIT_ASSERT(!isXmlSafeUtf8(OString("a\0b", 3)));
    CPPUNIT_ASSERT(!isXmlSafeUtf8("a\x01"
                                  "b"_ostr));
    CPPUNIT_ASSERT(!isXmlSafeUtf8("a\x1f"
                                  "b"_ostr));
    // X5: 0x7F-0x9F are legal in XML 1.0, and ODF is 1.0
    CPPUNIT_ASSERT(isXmlSafeUtf8("a\x7f"
                                 "b\xc2\x85"
                                 "c\xc2\x9f"
                                 "d"_ostr));
    // X6: the non-characters at the end of the BMP, and lone surrogates
    CPPUNIT_ASSERT(!isXmlSafeUtf8("a\xef\xbf\xbe"
                                  "b"_ostr));
    CPPUNIT_ASSERT(!isXmlSafeUtf8("a\xef\xbf\xbf"
                                  "b"_ostr));
    CPPUNIT_ASSERT(isXmlSafeUtf8("a\xef\xbf\xbd"
                                 "b"_ostr));
    CPPUNIT_ASSERT(!isXmlSafeUtf8("a\xed\xa0\x80"
                                  "b"_ostr));
    // X7: bytes that are not valid UTF-8 at all
    CPPUNIT_ASSERT(!isXmlSafeUtf8("a\xff"
                                  "b"_ostr));
    CPPUNIT_ASSERT(!isXmlSafeUtf8("a\xc3"_ostr)); // truncated sequence
    CPPUNIT_ASSERT(!isXmlSafeUtf8("a\xc0\x80"
                                  "b"_ostr)); // overlong NUL
}

void L10nTranslateTest::testL10nStreamParsing()
{
    L10nStream aStream;

    // L1: fallback matching, comments and blank lines
    parseL10nStream("# comment\nde,fr\n\nHello\nde\tHallo\nfr\tBonjour\n"_ostr, u"de-DE"_ustr,
                    aStream);
    CPPUNIT_ASSERT_EQUAL("de"_ostr, aStream.aMatchedLocale);
    CPPUNIT_ASSERT_EQUAL("Hallo"_ostr, aStream.aMap["Hello"_ostr]);

    // L2: first match in file order wins
    parseL10nStream("fr,de-DE,de\n\nHello\nde-DE\tHallo DE\nde\tHallo\n"_ostr, u"de-DE"_ustr,
                    aStream);
    CPPUNIT_ASSERT_EQUAL("de-DE"_ostr, aStream.aMatchedLocale);
    CPPUNIT_ASSERT_EQUAL("Hallo DE"_ostr, aStream.aMap["Hello"_ostr]);

    // L3: no match
    parseL10nStream("de,fr\n\nHello\nde\tHallo\n"_ostr, u"tr-TR"_ustr, aStream);
    CPPUNIT_ASSERT(aStream.aMatchedLocale.isEmpty());
    CPPUNIT_ASSERT(aStream.aMap.empty());

    // L5: only the first tab separates locale and value
    parseL10nStream("de\n\nKey\nde\ta\tb\n"_ostr, u"de-DE"_ustr, aStream);
    CPPUNIT_ASSERT_EQUAL("a\tb"_ostr, aStream.aMap["Key"_ostr]);

    // L6: the "-" placeholder locale list (no .po files) never matches
    parseL10nStream("-\n\nHello\n"_ostr, u"de-DE"_ustr, aStream);
    CPPUNIT_ASSERT(aStream.aMatchedLocale.isEmpty());
    CPPUNIT_ASSERT(aStream.aMap.empty());

    // L7: a value we could not serialise as XML never enters the map, so
    // the key misses and its marker is merely stripped; the locale still
    // counts as matched, and its other entries are unaffected
    parseL10nStream("de\n\nBad\nde\tHal\x0b"
                    "lo\n\nGood\nde\tHallo\n"_ostr,
                    u"de-DE"_ustr, aStream);
    CPPUNIT_ASSERT_EQUAL("de"_ostr, aStream.aMatchedLocale);
    CPPUNIT_ASSERT(!aStream.aMap.contains("Bad"_ostr));
    CPPUNIT_ASSERT_EQUAL("Hallo"_ostr, aStream.aMap["Good"_ostr]);
}

namespace
{
std::vector<sal_uInt8> toBytes(std::string_view aText)
{
    return std::vector<sal_uInt8>(aText.begin(), aText.end());
}

constexpr std::string_view aMimetype = "application/vnd.oasis.opendocument.spreadsheet";

constexpr std::string_view aManifestXml
    = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<manifest:manifest"
      " xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\""
      " manifest:version=\"1.3\">"
      "<manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.3\""
      " manifest:media-type=\"application/vnd.oasis.opendocument.spreadsheet\"/>"
      "<manifest:file-entry manifest:full-path=\"content.xml\""
      " manifest:media-type=\"text/xml\"/>"
      "<manifest:file-entry manifest:full-path=\"styles.xml\""
      " manifest:media-type=\"text/xml\"/>"
      // the embedder registers the stream and the l10n templates
      // in the manifest; unlisted members make the package validation
      // reject the document as inconsistent
      "<manifest:file-entry manifest:full-path=\"l10n\""
      " manifest:media-type=\"text/plain\"/>"
      "<manifest:file-entry manifest:full-path=\"l10n_template.xml\""
      " manifest:media-type=\"text/xml\"/>"
      "<manifest:file-entry manifest:full-path=\"l10n_template_styles.xml\""
      " manifest:media-type=\"text/xml\"/>"
      "</manifest:manifest>\n";

constexpr std::string_view aSmokeManifestXml
    = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<manifest:manifest"
      " xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\""
      " manifest:version=\"1.3\">"
      "<manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.3\""
      " manifest:media-type=\"application/vnd.oasis.opendocument.spreadsheet\"/>"
      "<manifest:file-entry manifest:full-path=\"content.xml\""
      " manifest:media-type=\"text/xml\"/>"
      "<manifest:file-entry manifest:full-path=\"l10n\""
      " manifest:media-type=\"text/plain\"/>"
      "<manifest:file-entry manifest:full-path=\"l10n_template.xml\""
      " manifest:media-type=\"text/xml\"/>"
      "</manifest:manifest>\n";

constexpr std::string_view aContentXml
    = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<office:document-content"
      " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
      " xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
      " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\""
      " xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
      " xmlns:calcext=\"urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0\""
      " office:version=\"1.3\">"
      "<office:automatic-styles>"
      "<style:style style:name=\"cond1\" style:family=\"table-cell\">"
      "<style:map style:base-cell-address=\"'_Try it'.A1\"/>"
      "</style:style>"
      "</office:automatic-styles>"
      "<office:body><office:spreadsheet>"
      "<table:content-validations>"
      "<table:content-validation table:name=\"val1\""
      " table:base-cell-address=\"'_Try it'.B2\">"
      "<table:help-message table:title=\"_Try formatting\" table:display=\"true\">"
      "<text:p>_Explore the Format tab</text:p>"
      "</table:help-message>"
      "</table:content-validation>"
      "</table:content-validations>"
      "<table:table table:name=\"_Try it\">"
      "<table:table-column/>"
      "<table:table-row>"
      "<table:table-cell office:value-type=\"string\">"
      "<text:p><text:span text:style-name=\"T5\">_3. Press </text:span>"
      "<text:span text:style-name=\"T6\">Enter</text:span>"
      "<text:span text:style-name=\"T7\"> to calculate</text:span></text:p>"
      "</table:table-cell>"
      "<table:table-cell office:value-type=\"string\">"
      "<text:p>_Missing key paragraph</text:p>"
      "</table:table-cell>"
      "<table:table-cell office:value-type=\"string\">"
      "<text:p>_Visit <text:span text:style-name=\"T9\">the Insert tab</text:span>"
      " now</text:p>"
      "</table:table-cell>"
      "<table:table-cell office:value-type=\"string\">"
      "<text:p><text:span text:style-name=\"T8\">_Single span cell</text:span></text:p>"
      "</table:table-cell>"
      "<table:table-cell office:value-type=\"string\">"
      "<text:p>_hello<!-- x -->world</text:p>"
      "</table:table-cell>"
      "<table:table-cell office:value-type=\"string\">"
      "<text:p>_Decorated</text:p>"
      "</table:table-cell>"
      "</table:table-row>"
      "<calcext:conditional-formats>"
      "<calcext:conditional-format"
      " calcext:target-range-address=\"'_Try it'.B3:'_Try it'.B9\"/>"
      "</calcext:conditional-formats>"
      "</table:table>"
      "</office:spreadsheet></office:body></office:document-content>\n";

constexpr std::string_view aStylesXml
    = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<office:document-styles"
      " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
      " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\">"
      "<office:master-styles><text:p>_Master text</text:p></office:master-styles>"
      "</office:document-styles>\n";

constexpr std::string_view aL10nStream = "# supported locales\n"
                                         "de,fr\n"
                                         "# strings and their translations\n"
                                         "\n"
                                         "<1>3. Press </1><2>Enter</2><3> to calculate</3>\n"
                                         "de\t<2>Enter</2><1>3. Dr\xc3\xbc"
                                         "cken Sie </1><3> zum Rechnen</3>\n"
                                         "fr\t<1>3. Appuyez </1><2>Entr\xc3\xa9"
                                         "e</2><3> pour calculer</3>\n"
                                         "\n"
                                         "Try it\n"
                                         "de\tProbier's\n"
                                         "\n"
                                         "Visit <1>the Insert tab</1> now\n"
                                         "de\tBesuchen Sie jetzt <1>den Einf\xc3\xbc"
                                         "gen-Tab</1>\n"
                                         "\n"
                                         "Single span cell\n"
                                         "de\tEinzelzelle\n"
                                         "\n"
                                         "Try formatting\n"
                                         "de\tFormatierung testen\n"
                                         "\n"
                                         "Explore the Format tab\n"
                                         "de\tEntdecke den Format-Tab\n"
                                         "\n"
                                         "Master text\n"
                                         "de\tMastertext\n"
                                         "\n"
                                         "helloworld\n"
                                         "de\tHallowelt\n"
                                         "\n"
                                         "Decorated\n"
                                         "de\t<1>Verziert</1>\n";

const sal_uInt8 aBlobBytes[] = { 0x00, 0xff, 0x10, 0x20, 0x00, 0x00, 0x7f, 0x42 };

// a minimal but valid ODS for the load smoke test (the full fixture above
// deliberately exercises XML the importers would reject)
constexpr std::string_view aSmokeContentXml
    = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<office:document-content"
      " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
      " xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
      " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\""
      " office:version=\"1.3\">"
      "<office:body><office:spreadsheet>"
      "<table:table table:name=\"_Try it\">"
      "<table:table-column/>"
      "<table:table-row>"
      "<table:table-cell office:value-type=\"string\">"
      "<text:p><text:span text:style-name=\"T5\">_3. Press </text:span>"
      "<text:span text:style-name=\"T6\">Enter</text:span>"
      "<text:span text:style-name=\"T7\"> to calculate</text:span></text:p>"
      "</table:table-cell>"
      "</table:table-row>"
      "</table:table>"
      "</office:spreadsheet></office:body></office:document-content>\n";

std::vector<zip::Entry> makeSmokeFixtureEntries()
{
    std::vector<zip::Entry> aEntries;
    aEntries.push_back({ "mimetype"_ostr, 0, toBytes(aMimetype), {} });
    aEntries.push_back({ "META-INF/manifest.xml"_ostr, 8, toBytes(aSmokeManifestXml), {} });
    aEntries.push_back({ "content.xml"_ostr, 8, toBytes(aSmokeContentXml), {} });
    aEntries.push_back({ "l10n"_ostr, 8, toBytes(aL10nStream), {} });
    aEntries.push_back({ "l10n_template.xml"_ostr, 8, toBytes(aSmokeContentXml), {} });
    return aEntries;
}

std::vector<zip::Entry> makeFixtureEntries(bool bWithStream)
{
    std::vector<zip::Entry> aEntries;
    aEntries.push_back({ "mimetype"_ostr, 0, toBytes(aMimetype), {} });
    aEntries.push_back({ "META-INF/manifest.xml"_ostr, 8, toBytes(aManifestXml), {} });
    aEntries.push_back({ "content.xml"_ostr, 8, toBytes(aContentXml), {} });
    aEntries.push_back({ "styles.xml"_ostr, 8, toBytes(aStylesXml), {} });
    aEntries.push_back({ "media/blob.bin"_ostr,
                         0,
                         std::vector<sal_uInt8>(aBlobBytes, aBlobBytes + sizeof(aBlobBytes)),
                         {} });
    if (bWithStream)
    {
        aEntries.push_back({ "l10n"_ostr, 8, toBytes(aL10nStream), {} });
        // the marked l10n templates the rewrite reads from
        aEntries.push_back({ "l10n_template.xml"_ostr, 8, toBytes(aContentXml), {} });
        aEntries.push_back({ "l10n_template_styles.xml"_ostr, 8, toBytes(aStylesXml), {} });
    }
    return aEntries;
}

std::vector<zip::Entry> makeFixtureEntriesWithStream(std::string_view aStream)
{
    std::vector<zip::Entry> aEntries = makeFixtureEntries(true);
    for (zip::Entry& rEntry : aEntries)
        if (rEntry.aName == "l10n")
            rEntry.aData = toBytes(aStream);
    return aEntries;
}

OString entryText(const std::vector<zip::Entry>& rEntries, std::string_view aName)
{
    for (const zip::Entry& rEntry : rEntries)
        if (std::string_view(rEntry.aName) == aName)
            return OString(reinterpret_cast<const char*>(rEntry.aData.data()),
                           static_cast<sal_Int32>(rEntry.aData.size()));
    CPPUNIT_FAIL("zip entry not found");
}


void assertContains(std::string_view aHaystack, std::string_view aNeedle)
{
    if (aHaystack.find(aNeedle) == std::string_view::npos)
        CPPUNIT_FAIL(
            OString(OString::Concat("missing '") + aNeedle + "' in:\n" + aHaystack).getStr());
}

/// CPPUNIT_ASSERT_EQUAL cannot print a byte vector, and a raw == in a
/// CPPUNIT_ASSERT reads to loplugin:cppunitassertequals as one that should.
bool bytesEqual(const std::vector<sal_uInt8>& rLeft, const std::vector<sal_uInt8>& rRight)
{
    return rLeft == rRight;
}

void writeAllBytes(const OUString& rURL, const std::vector<sal_uInt8>& rBytes)
{
    osl::File::remove(rURL);
    osl::File aFile(rURL);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, aFile.open(osl_File_OpenFlag_Write
                                                           | osl_File_OpenFlag_Create));
    sal_uInt64 nWritten = 0;
    aFile.write(rBytes.data(), rBytes.size(), nWritten);
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(rBytes.size()), nWritten);
}

/// Scramble one member's stored payload so it cannot be decompressed, while
/// every header stays intact and the package still parses.

std::vector<sal_uInt8> readAllBytes(const OUString& rURL)
{
    std::vector<sal_uInt8> aBytes;
    osl::File aFile(rURL);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, aFile.open(osl_File_OpenFlag_Read));
    sal_uInt64 nSize = 0;
    aFile.getSize(nSize);
    aBytes.resize(nSize);
    sal_uInt64 nRead = 0;
    aFile.read(aBytes.data(), nSize, nRead);
    return aBytes;
}

void corruptMemberPayload(const OUString& rURL, std::string_view aName)
{
    std::vector<sal_uInt8> aBytes = readAllBytes(rURL);
    auto readLE = [&aBytes](size_t nAt, size_t nLen) {
        size_t nValue = 0;
        for (size_t n = 0; n < nLen; ++n)
            nValue |= static_cast<size_t>(aBytes[nAt + n]) << (8 * n);
        return nValue;
    };
    for (size_t i = 0; i + 30 + aName.size() <= aBytes.size(); ++i)
    {
        if (aBytes[i] != 0x50 || aBytes[i + 1] != 0x4b || aBytes[i + 2] != 0x03
            || aBytes[i + 3] != 0x04)
            continue;
        const size_t nNameLen = readLE(i + 26, 2);
        if (nNameLen != aName.size()
            || std::memcmp(aBytes.data() + i + 30, aName.data(), nNameLen) != 0)
            continue;
        const size_t nStart = i + 30 + nNameLen + readLE(i + 28, 2);
        const size_t nCompressed = readLE(i + 18, 4);
        CPPUNIT_ASSERT(nCompressed > 0);
        CPPUNIT_ASSERT(nStart + nCompressed <= aBytes.size());
        for (size_t n = nStart; n < nStart + nCompressed; ++n)
            aBytes[n] = static_cast<sal_uInt8>(aBytes[n] ^ 0xff);
        writeAllBytes(rURL, aBytes);
        return;
    }
    CPPUNIT_FAIL("no local header for the member");
}

} // namespace

void L10nTranslateTest::testTranslateFull()
{
    const OUString aIn = tempFileUrl("full-in.ods");
    const OUString aOut = tempFileUrl("full-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntries(true)));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));

    std::vector<zip::Entry> aEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aEntries));
    const OString aContent = entryText(aEntries, "content.xml");

    // multi-run hit with tag reorder: spans rebuilt in msgstr order,
    // keeping each segment's original span style
    assertContains(aContent, "<text:span text:style-name=\"T6\">Enter</text:span>"
                             "<text:span text:style-name=\"T5\">3. Dr\xc3\xbc"
                             "cken Sie </text:span>"
                             "<text:span text:style-name=\"T7\"> zum Rechnen</text:span>");
    // miss: marker stripped, text otherwise untouched
    assertContains(aContent, "<text:p>Missing key paragraph</text:p>");
    // mixed paragraph: untagged msgstr text becomes bare paragraph text,
    // <1> becomes the styled span, here moved to the end
    assertContains(aContent, "<text:p>Besuchen Sie jetzt "
                             "<text:span text:style-name=\"T9\">den Einf\xc3\xbc"
                             "gen-Tab</text:span></text:p>");
    // single-run paragraph: a plain msgstr takes that run's span formatting
    assertContains(aContent, "<text:p><text:span text:style-name=\"T8\">Einzelzelle</text:span>"
                             "</text:p>");
    // comment-split paragraph: the bare runs around the comment merge into
    // one run, whose key matches and translates like any plain paragraph
    assertContains(aContent, "<text:p>Hallowelt</text:p>");
    // a tag on a tagless msgid is invalid: the entry is rejected and the
    // paragraph falls back to the stripped marker
    assertContains(aContent, "<text:p>Decorated</text:p>");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aContent.indexOf("Verziert"));
    // sheet renamed, and every referencing attribute rewritten with the
    // translated name's "''" quote escaping
    assertContains(aContent, "table:name=\"Probier's\"");
    assertContains(aContent, "style:base-cell-address=\"'Probier''s'.A1\"");
    assertContains(aContent, "table:base-cell-address=\"'Probier''s'.B2\"");
    assertContains(aContent, "calcext:target-range-address=\"'Probier''s'.B3:'Probier''s'.B9\"");
    // validation help message: title attribute and body paragraph
    assertContains(aContent, "table:title=\"Formatierung testen\"");
    assertContains(aContent, "<text:p>Entdecke den Format-Tab</text:p>");
    // styles.xml is rewritten too
    assertContains(entryText(aEntries, "styles.xml"), "<text:p>Mastertext</text:p>");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testTranslateMissAndUnmatchedLocale()
{
    const OUString aIn = tempFileUrl("miss-in.ods");
    const OUString aOut = tempFileUrl("miss-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntries(true)));

    // no locale match: markers stripped, nothing translated
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::CopiedUntranslated),
                         static_cast<int>(translateDocument(aIn, aOut, u"tr-TR"_ustr)));

    std::vector<zip::Entry> aEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aEntries));
    const OString aContent = entryText(aEntries, "content.xml");
    assertContains(aContent, "<text:span text:style-name=\"T5\">3. Press </text:span>");
    assertContains(aContent, "<text:p>Visit <text:span text:style-name=\"T9\">"
                             "the Insert tab</text:span> now</text:p>");
    assertContains(aContent, "table:name=\"Try it\"");
    assertContains(aContent, "style:base-cell-address=\"'Try it'.A1\"");
    assertContains(aContent, "table:title=\"Try formatting\"");
    assertContains(aContent, "<text:p>Explore the Format tab</text:p>");
    // the marker-strip fallback leaves the comment in place
    assertContains(aContent, "<text:p>hello<!-- x -->world</text:p>");
    assertContains(entryText(aEntries, "styles.xml"), "<text:p>Master text</text:p>");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testIllegalXmlCharacterInTranslation()
{
    // A translation carrying a character XML forbids must not reach the DOM.
    // The dump escapes only "&", "<" and ">", so writing it out would give a
    // content.xml that no longer parses - and, because the rewrite may run
    // in place, would replace a good file with an unloadable one.  The entry
    // is dropped instead, which degrades to the ordinary miss path.
    const OString aPoisoned
        = OString(aL10nStream.data(), static_cast<sal_Int32>(aL10nStream.size()))
              .replaceFirst("de\tEntdecke den Format-Tab"_ostr, "de\tEntdecke\x0b"
                                                                "den Format-Tab"_ostr);
    std::vector<zip::Entry> aEntries = makeFixtureEntries(true);
    for (zip::Entry& rEntry : aEntries)
        if (rEntry.aName == "l10n"_ostr)
            rEntry.aData = toBytes(
                std::string_view(aPoisoned.getStr(), static_cast<size_t>(aPoisoned.getLength())));

    const OUString aIn = tempFileUrl("badchar-in.ods");
    const OUString aOut = tempFileUrl("badchar-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, aEntries));
    // the locale still matched, so this is a normal translation
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));

    std::vector<zip::Entry> aOutEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aOutEntries));
    const OString aContent = entryText(aOutEntries, "content.xml");
    // the dropped key falls back to the stripped English text
    assertContains(aContent, "<text:p>Explore the Format tab</text:p>");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aContent.indexOf("Entdecke"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aContent.indexOf('\x0b'));
    // every other entry of the same locale still translates
    assertContains(aContent, "table:title=\"Formatierung testen\"");
    assertContains(aContent, "table:name=\"Probier's\"");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testRetranslateInPlace()
{
    const OUString aFile = tempFileUrl("inplace.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aFile, makeFixtureEntries(true)));

    // first open: German, in place
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aFile, aFile, u"de-DE"_ustr)));
    std::vector<zip::Entry> aEntries;
    CPPUNIT_ASSERT(zip::readArchive(aFile, aEntries));
    assertContains(entryText(aEntries, "content.xml"), "table:title=\"Formatierung testen\"");
    // the l10n template keeps its markers for the next translation
    assertContains(entryText(aEntries, "l10n_template.xml"), "_3. Press ");
    assertContains(entryText(aEntries, "l10n_template_styles.xml"), "_Master text");

    // a later open under another locale re-translates the very same file
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aFile, aFile, u"fr-FR"_ustr)));
    aEntries.clear();
    CPPUNIT_ASSERT(zip::readArchive(aFile, aEntries));
    const OString aContent = entryText(aEntries, "content.xml");
    assertContains(aContent, "<text:span text:style-name=\"T5\">3. Appuyez </text:span>");
    // keys with no fr translation fall back to stripped English
    assertContains(aContent, "table:title=\"Try formatting\"");

    // the same locale again changes nothing: the file must stay untouched
    const std::vector<sal_uInt8> aBefore = readAllBytes(aFile);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aFile, aFile, u"fr-FR"_ustr)));
    CPPUNIT_ASSERT(bytesEqual(aBefore, readAllBytes(aFile)));

    // and a file with no stream is never touched in place
    const OUString aPlain = tempFileUrl("inplace-plain.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aPlain, makeFixtureEntries(false)));
    const std::vector<sal_uInt8> aPlainBefore = readAllBytes(aPlain);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::CopiedUntranslated),
                         static_cast<int>(translateDocument(aPlain, aPlain, u"de-DE"_ustr)));
    CPPUNIT_ASSERT(bytesEqual(aPlainBefore, readAllBytes(aPlain)));

    osl::File::remove(aFile);
    osl::File::remove(aPlain);
}

void L10nTranslateTest::testTemplatelessFallback()
{
    // a file with a stream but no l10n templates: the members themselves
    // are the source, so a one-shot translation still works
    std::vector<zip::Entry> aEntries = makeFixtureEntries(true);
    aEntries.erase(std::remove_if(aEntries.begin(), aEntries.end(),
                                  [](const zip::Entry& rEntry) {
                                      return rEntry.aName.startsWith("l10n_template");
                                  }),
                   aEntries.end());
    const OUString aIn = tempFileUrl("notmpl-in.ods");
    const OUString aOut = tempFileUrl("notmpl-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, aEntries));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));
    std::vector<zip::Entry> aOutEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aOutEntries));
    assertContains(entryText(aOutEntries, "content.xml"), "table:name=\"Probier's\"");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testUnreadableStreamStripsMarkers()
{
    // the package parses but the stream member's payload is corrupt: the
    // document still opens, with its markers removed
    const OUString aIn = tempFileUrl("badstream-in.ods");
    const OUString aOut = tempFileUrl("badstream-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntries(true)));
    corruptMemberPayload(aIn, "l10n");

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::CopiedUntranslated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));
    std::vector<zip::Entry> aOutEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aOutEntries));
    const OString aContent = entryText(aOutEntries, "content.xml");
    assertContains(aContent, "table:name=\"Try it\"");
    assertContains(aContent, "<text:p>Explore the Format tab</text:p>");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aContent.indexOf("_Try it"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aContent.indexOf(">_"));
    assertContains(entryText(aOutEntries, "styles.xml"), "<text:p>Master text</text:p>");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testEmptyLanguageStripsMarkers()
{
    // no language asked for: nothing is translated, whatever the machine's
    // own locale happens to be, and the markers still come off
    const OUString aIn = tempFileUrl("nolang-in.ods");
    const OUString aOut = tempFileUrl("nolang-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntries(true)));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::CopiedUntranslated),
                         static_cast<int>(translateDocument(aIn, aOut, OUString())));
    std::vector<zip::Entry> aOutEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aOutEntries));
    const OString aContent = entryText(aOutEntries, "content.xml");
    assertContains(aContent, "table:name=\"Try it\"");
    assertContains(aContent, "<text:p>Explore the Format tab</text:p>");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aContent.indexOf(">_"));

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testRegionLocaleWinsOverBase()
{
    // the stream lists the base tag first, as sorting the locales does; a
    // reader of the regional variant still gets the regional translation
    static constexpr std::string_view aStream = "# supported locales\n"
                                                "pt,pt-BR\n"
                                                "\n"
                                                "Try it\n"
                                                "pt\tExperimenta\n"
                                                "pt-BR\tExperimente\n"
                                                "\n";
    const OUString aIn = tempFileUrl("region-in.ods");
    const OUString aOut = tempFileUrl("region-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntriesWithStream(aStream)));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"pt-BR"_ustr)));
    std::vector<zip::Entry> aEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aEntries));
    assertContains(entryText(aEntries, "content.xml"), "table:name=\"Experimente\"");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testUnusableSheetNameKeepsSource()
{
    // a translation holding a character a sheet name cannot carry leaves the
    // sheet with its English name rather than an unopenable document
    static constexpr std::string_view aStream = "# supported locales\n"
                                                "de\n"
                                                "\n"
                                                "Try it\n"
                                                "de\tProbier: mal\n"
                                                "\n";
    const OUString aIn = tempFileUrl("badname-in.ods");
    const OUString aOut = tempFileUrl("badname-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntriesWithStream(aStream)));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));
    std::vector<zip::Entry> aEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aEntries));
    const OString aContent = entryText(aEntries, "content.xml");
    assertContains(aContent, "table:name=\"Try it\"");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aContent.indexOf("Probier: mal"));

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testEmptyTranslationKeepsSource()
{
    // an empty translation would empty the paragraph, so the English source
    // stays and only the marker comes off
    static constexpr std::string_view aStream = "# supported locales\n"
                                                "de\n"
                                                "\n"
                                                "Explore the Format tab\n"
                                                "de\t\n"
                                                "\n";
    const OUString aIn = tempFileUrl("emptytrans-in.ods");
    const OUString aOut = tempFileUrl("emptytrans-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntriesWithStream(aStream)));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));
    std::vector<zip::Entry> aEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aEntries));
    const OString aContent = entryText(aEntries, "content.xml");
    assertContains(aContent, "<text:p>Explore the Format tab</text:p>");
    assertContains(aContent, "table:title=\"Try formatting\"");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testTextlessTranslationKeepsSource()
{
    // "<1></1>" is not an empty msgstr, but it holds no text either, so
    // rebuilding from it would empty the paragraph just the same
    static constexpr std::string_view aStream = "# supported locales\n"
                                                "de\n"
                                                "\n"
                                                "Visit <1>the Insert tab</1> now\n"
                                                "de\t<1></1>\n"
                                                "\n";
    const OUString aIn = tempFileUrl("textless-in.ods");
    const OUString aOut = tempFileUrl("textless-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntriesWithStream(aStream)));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));
    std::vector<zip::Entry> aEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aEntries));
    const OString aContent = entryText(aEntries, "content.xml");
    assertContains(aContent, "<text:p>Visit <text:span text:style-name=\"T9\">"
                             "the Insert tab</text:span> now</text:p>");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testEditedFileIsLeftAlone()
{
    const OUString aFile = tempFileUrl("edited.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aFile, makeFixtureEntries(true)));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aFile, aFile, u"de-DE"_ustr)));

    // the user rewrites a paragraph and saves; a save that reuses the
    // package keeps the stream and the templates
    std::vector<zip::Entry> aEntries;
    CPPUNIT_ASSERT(zip::readArchive(aFile, aEntries));
    for (zip::Entry& rEntry : aEntries)
    {
        if (rEntry.aName != "content.xml")
            continue;
        OString aText(reinterpret_cast<const char*>(rEntry.aData.data()),
                      static_cast<sal_Int32>(rEntry.aData.size()));
        aText = aText.replaceAll("Entdecke den Format-Tab"_ostr, "Meine eigene Notiz"_ostr);
        rEntry.aData = toBytes(std::string_view(aText.getStr(), aText.getLength()));
    }
    CPPUNIT_ASSERT(zip::writeArchive(aFile, aEntries));

    // opening it again in another language must keep what the user wrote
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::CopiedUntranslated),
                         static_cast<int>(translateDocument(aFile, aFile, u"fr-FR"_ustr)));
    std::vector<zip::Entry> aAfter;
    CPPUNIT_ASSERT(zip::readArchive(aFile, aAfter));
    const OString aContent = entryText(aAfter, "content.xml");
    assertContains(aContent, "Meine eigene Notiz");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aContent.indexOf("3. Appuyez"));

    // out of place the edited file is still copied to the output, so a
    // caller that asked for one always gets one
    const OUString aOut = tempFileUrl("edited-out.ods");
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::CopiedUntranslated),
                         static_cast<int>(translateDocument(aFile, aOut, u"fr-FR"_ustr)));
    CPPUNIT_ASSERT(bytesEqual(readAllBytes(aFile), readAllBytes(aOut)));

    osl::File::remove(aFile);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testBrokenTemplateFallsBackToMember()
{
    // an l10n template that does not parse is skipped and the member itself,
    // still carrying its markers, is the source of the rewrite
    std::vector<zip::Entry> aEntries = makeFixtureEntries(true);
    for (zip::Entry& rEntry : aEntries)
        if (rEntry.aName == "l10n_template.xml")
            rEntry.aData = toBytes("<office:document-content");
    const OUString aIn = tempFileUrl("badtmpl-in.ods");
    const OUString aOut = tempFileUrl("badtmpl-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, aEntries));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));
    std::vector<zip::Entry> aOutEntries;
    CPPUNIT_ASSERT(zip::readArchive(aOut, aOutEntries));
    assertContains(entryText(aOutEntries, "content.xml"), "table:name=\"Probier's\"");
    assertContains(entryText(aOutEntries, "styles.xml"), "<text:p>Mastertext</text:p>");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testNoStreamByteIdentical()
{
    const OUString aIn = tempFileUrl("nostream-in.ods");
    const OUString aOut = tempFileUrl("nostream-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntries(false)));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::CopiedUntranslated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));

    // the copy is byte-identical, markers and all
    osl::File aFileIn(aIn), aFileOut(aOut);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, aFileIn.open(osl_File_OpenFlag_Read));
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, aFileOut.open(osl_File_OpenFlag_Read));
    sal_uInt64 nSizeIn = 0, nSizeOut = 0;
    aFileIn.getSize(nSizeIn);
    aFileOut.getSize(nSizeOut);
    CPPUNIT_ASSERT_EQUAL(nSizeIn, nSizeOut);
    std::vector<sal_uInt8> aBytesIn(nSizeIn), aBytesOut(nSizeOut);
    sal_uInt64 nRead = 0;
    aFileIn.read(aBytesIn.data(), nSizeIn, nRead);
    aFileOut.read(aBytesOut.data(), nSizeOut, nRead);
    CPPUNIT_ASSERT(bytesEqual(aBytesIn, aBytesOut));

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testZipFidelity()
{
    const OUString aIn = tempFileUrl("fidelity-in.ods");
    const OUString aOut = tempFileUrl("fidelity-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeFixtureEntries(true)));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"de-DE"_ustr)));

    std::vector<zip::Entry> aInEntries, aOutEntries;
    CPPUNIT_ASSERT(zip::readArchive(aIn, aInEntries));
    CPPUNIT_ASSERT(zip::readArchive(aOut, aOutEntries));
    CPPUNIT_ASSERT_EQUAL(aInEntries.size(), aOutEntries.size());

    // mimetype must stay the first entry, STORED, unchanged
    CPPUNIT_ASSERT_EQUAL("mimetype"_ostr, aOutEntries[0].aName);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), aOutEntries[0].nMethod);
    CPPUNIT_ASSERT(bytesEqual(aOutEntries[0].aData, toBytes(aMimetype)));

    for (size_t i = 0; i < aInEntries.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(aInEntries[i].aName, aOutEntries[i].aName);
        if (aInEntries[i].aName == "content.xml" || aInEntries[i].aName == "styles.xml"
            || aInEntries[i].aName == "l10n")
            continue;
        // untouched members: same method and byte-identical raw payload
        CPPUNIT_ASSERT_EQUAL(aInEntries[i].nMethod, aOutEntries[i].nMethod);
        CPPUNIT_ASSERT_MESSAGE(aInEntries[i].aName.getStr(),
                               bytesEqual(aInEntries[i].aRawData, aOutEntries[i].aRawData));
    }

    // the l10n stream is retained, and records the locale it was translated
    // into so a later open can tell an untouched file from an edited one
    assertContains(entryText(aOutEntries, "l10n"), "# applied: de");

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

void L10nTranslateTest::testLoadTranslatedSmoke()
{
    const OUString aIn = tempFileUrl("smoke-in.ods");
    const OUString aOut = tempFileUrl("smoke-out.ods");
    CPPUNIT_ASSERT(zip::writeArchive(aIn, makeSmokeFixtureEntries()));

    // French translations are in document order, so the cell reads naturally
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(TranslateResult::Translated),
                         static_cast<int>(translateDocument(aIn, aOut, u"fr-FR"_ustr)));

    loadFromURL(aOut);
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xSheets->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xCell(xSheet->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"3. Appuyez Entrée pour calculer"_ustr, xCell->getString());

    // "Try it" has no fr translation: the sheet name is the stripped marker
    uno::Reference<container::XNameAccess> xByName(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xByName->hasByName(u"Try it"_ustr));

    osl::File::remove(aIn);
    osl::File::remove(aOut);
}

CPPUNIT_TEST_SUITE_REGISTRATION(L10nTranslateTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
