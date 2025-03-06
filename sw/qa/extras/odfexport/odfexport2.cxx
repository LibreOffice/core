/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/drawing/BarCode.hpp>
#include <com/sun/star/drawing/BarCodeErrorCorrection.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XGraphicExportFilter.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/unolingu.hxx>
#include <unoprnms.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/odfexport/data/"_ustr, u"writer8"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedFontProps)
{
    loadAndReload("embedded-font-props.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
#if !defined(MACOSX)
    // Test that font style/weight of embedded fonts is exposed.
    // Test file is a normal ODT, except EmbedFonts is set to true in settings.xml.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // These failed, the attributes were missing.
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[1]", "font-style", u"normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[1]", "font-weight", u"normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[2]", "font-style", u"normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[2]", "font-weight", u"bold");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[3]", "font-style", u"italic");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[3]", "font-weight", u"normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[4]", "font-style", u"italic");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[4]", "font-weight", u"bold");
#endif
}

DECLARE_ODFEXPORT_TEST(testTdf100492, "tdf100492.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT(xShape.is());

    // Save the first shape to a SVG
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter = drawing::GraphicExportFilter::create(comphelper::getProcessComponentContext());
    uno::Reference<lang::XComponent> xSourceDoc(xShape, uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xSourceDoc);

    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
            { "OutputStream", uno::Any(xOutputStream) },
            { "FilterName", uno::Any(u"SVG"_ustr) }
        }));
    xGraphicExporter->filter(aDescriptor);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // TODO: Disabled. Parsing of SVG gives just root node without any children.
    // Reason of such behavior unclear. So XPATH assert fails.

    // Parse resulting SVG as XML file.
    // xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);

    // Check amount of paths required to draw an arrow.
    // Since there are still some empty paths in output test can fail later. There are just two
    // really used and visible paths.
    //assertXPath(pXmlDoc, "/svg/path", 4);
}

DECLARE_ODFEXPORT_TEST(testTdf77961, "tdf77961.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<container::XNameAccess> xStyles(getStyles(u"PageStyles"_ustr));
    uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( false , getProperty<bool>(xStyle, u"GridDisplay"_ustr));
    CPPUNIT_ASSERT_EQUAL( false , getProperty<bool>(xStyle, u"GridPrint"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106733)
{
    loadAndReload("tdf106733.fodt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    OUString autostyle = getXPath(pXmlDoc, "//office:body/office:text/text:p[2]/text:span",
                                  "style-name");
    OString autostyle_span_xpath = "//style:style[@style:name='" + autostyle.toUtf8() + "']";

    // keep fo:hyphenate="false" in direct formatting
    assertXPath(
        pXmlDoc,
        autostyle_span_xpath + "/style:text-properties",
        "hyphenate", u"false");

    // keep fo:hyphenate="false" in character style
    xmlDocUniquePtr pXmlDoc2 = parseExport(u"styles.xml"_ustr);
    assertXPath(
        pXmlDoc2,
        "//style:style[@style:name='Strong_20_Emphasis']/style:text-properties",
        "hyphenate", u"false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_page)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // fo:hyphenation-keep="page" defined in direct paragraph formatting
    loadAndReload("tdf132599_page.fodt");
    // This was 2 (not truncated hyphenated line)
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_auto)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // fo:hyphenation-keep="auto" defined in direct paragraph formatting
    loadAndReload("tdf132599_auto.fodt");
    // not truncated hyphenated line
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_spread)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // fo:hyphenation-keep="page" loext:hyphenation-keep-type="spread"
    loadAndReload("tdf132599_spread.fodt");
    // shift last line of right page, resulting 3 pages
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_spread_left_page)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // fo:hyphenation-keep="page" loext:hyphenation-keep-type="spread"
    loadAndReload("tdf132599_spread-left-page.fodt");
    // do not shift last line of left page
    // This was 4 (shifted last line of left page, when it's hyphenated)
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_column)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // last line of the left column is shifted, according to
    // fo:hyphenation-keep="page" loext:hyphenation-keep-type="column"
    loadAndReload("tdf132599_column.fodt");
    // shift last line of the first column, resulting 3 pages
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_page_in_not_last_column)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // last line of the not last column is not shifted, according to
    // fo:hyphenation-keep="page" loext:hyphenation-keep-type="page"
    loadAndReload("tdf132599_page_in_not_last_column.fodt");
    // do not shift last line of the first column, resulting 2 pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_page_in_last_column)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // last line of the last column is shifted, according to
    // fo:hyphenation-keep="page" loext:hyphenation-keep-type="page"
    loadAndReload("tdf132599_page_in_last_column.fodt");
    // shift last line of the first page, resulting 3 pages
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_always)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // last full line of the paragraph column is not hyphenated
    // fo:hyphenation-keep="page" loext:hyphenation-keep-type="always"
    loadAndReload("tdf132599_always.fodt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//style:style[@style:family='paragraph']/style:paragraph-properties[@loext:hyphenation-keep-type='always']", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_page_in_table)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // last full line of the table is not hyphenated on page 1
    // fo:hyphenation-keep="page" loext:hyphenation-keep-type=""
    loadAndReload("tdf132599_page_in_table.fodt");
    // This was 2 (not handling hyphenation-keep in tables)
    CPPUNIT_ASSERT_EQUAL(3, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//style:style[@style:family='paragraph']/style:paragraph-properties[@loext:hyphenation-keep-type='page']", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_page_in_default_paragraph_style)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // fo:hyphenation-keep="page" defined in default paragraph style
    loadAndReload("tdf160518_page_in_default_paragraph_style.fodt");
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_auto_in_default_paragraph_style)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // fo:hyphenation-keep="auto" defined in default paragraph style
    loadAndReload("tdf160518_auto_in_default_paragraph_style.fodt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_page_in_text_body_style)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // fo:hyphenation-keep="page" defined in text body style
    loadAndReload("tdf160518_page_in_text_body_style.fodt");
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_auto_in_text_body_style)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // fo:hyphenation-keep="auto" defined in text body style
    loadAndReload("tdf160518_auto_in_text_body_style.fodt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165354_page)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // fo:hyphenation-keep="page" defined in direct paragraph formatting
    loadAndReload("tdf165354_page.fodt");
    // This was 3 in tdf132599, but now 2, shifting only the last hyphenated word,
    // not the full line, because of loext:hyphenation-keep-line=true
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158885_compound_remain)
{
    loadAndReload("tdf158885_compound-remain.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    assertXPath(pXmlDoc, "//style:style[@style:family='paragraph']/style:text-properties[@loext:hyphenation-compound-remain-char-count='3']", 1);
}

DECLARE_ODFEXPORT_TEST(testReferenceLanguage, "referencelanguage.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // Test loext:reference-language attribute of reference fields
    // (used from LibreOffice 6.1, and proposed for next ODF)
    OUString const aFieldTexts[] = { u"A 2"_ustr, u"Az Isten"_ustr, u"Az 50-esek"_ustr,
        u"A 2018-asok"_ustr, u"Az egyebek"_ustr, u"A fejezetek"_ustr,
        u"Az „Őseinket...”"_ustr, u"a 2"_ustr,
        u"Az v"_ustr, u"az 1"_ustr, u"Az e)"_ustr, u"az 1"_ustr,
        u"Az (5)"_ustr, u"az 1"_ustr, u"A 2"_ustr, u"az 1"_ustr };
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    // update "A (4)" to "Az (5)"
    uno::Reference<util::XRefreshable>(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY_THROW)->refresh();

    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    uno::Any aHu(u"Hu"_ustr);
    uno::Any ahu(u"hu"_ustr);
    for (auto const& sFieldText : aFieldTexts)
    {
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        if (xServiceInfo->supportsService(u"com.sun.star.text.textfield.GetReference"_ustr))
        {
            uno::Reference<beans::XPropertySet> xPropertySet(aField, uno::UNO_QUERY);
            uno::Any aLang = xPropertySet->getPropertyValue(u"ReferenceFieldLanguage"_ustr);
            CPPUNIT_ASSERT_EQUAL(true, aLang == aHu || aLang == ahu);
            uno::Reference<text::XTextContent> xField(aField, uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(sFieldText, xField->getAnchor()->getString());
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testRubyPosition)
{
    loadAndReload("ruby-position.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    assertXPath(pXmlDoc, "//style:style[@style:family='ruby']/style:ruby-properties[@loext:ruby-position='inter-character']", 1);
    assertXPath(pXmlDoc, "//style:style[@style:family='ruby']/style:ruby-properties[@style:ruby-position='below']", 1);
}

DECLARE_ODFEXPORT_TEST(testAllowOverlap, "allow-overlap.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: !getProperty<bool>(xShape, "AllowOverlap")
    // i.e. the custom AllowOverlap=false shape property was lost on import/export.
    CPPUNIT_ASSERT(!getProperty<bool>(xShape, u"AllowOverlap"_ustr));
    xShape = getShape(2);
    CPPUNIT_ASSERT(!getProperty<bool>(xShape, u"AllowOverlap"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testSignatureLineProperties)
{
    loadAndReload("signatureline-properties.fodt");
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT(xShape.is());

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, u"IsSignatureLine"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"{3C24159B-3B98-4F60-AB52-00E7721758E9}"_ustr,
                         getProperty<OUString>(xShape, u"SignatureLineId"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"John Doe"_ustr,
                         getProperty<OUString>(xShape, u"SignatureLineSuggestedSignerName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Farmer"_ustr,
                         getProperty<OUString>(xShape, u"SignatureLineSuggestedSignerTitle"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"john@farmers.org"_ustr,
                         getProperty<OUString>(xShape, u"SignatureLineSuggestedSignerEmail"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Please farm here."_ustr,
                         getProperty<OUString>(xShape, u"SignatureLineSigningInstructions"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, u"SignatureLineCanAddComment"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, u"SignatureLineShowSignDate"_ustr));

    // tdf#130917 This needs to be always set when importing a doc, ooxml export expects it.
    uno::Reference<graphic::XGraphic> xUnsignedGraphic;
    uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY);
    xProps->getPropertyValue(u"SignatureLineUnsignedImage"_ustr) >>= xUnsignedGraphic;
    CPPUNIT_ASSERT_EQUAL(true, xUnsignedGraphic.is());
}

DECLARE_ODFEXPORT_TEST(testQrCodeGenProperties, "qrcode-properties.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT(xShape.is());

    css::drawing::BarCode aBarCode = getProperty<css::drawing::BarCode>(xShape, u"BarCodeProperties"_ustr);

    CPPUNIT_ASSERT_EQUAL(u"www.libreoffice.org"_ustr,
                         aBarCode.Payload);
    CPPUNIT_ASSERT_EQUAL(css::drawing::BarCodeErrorCorrection::LOW,
                         aBarCode.ErrorCorrection);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5),
                         aBarCode.Border);
}

DECLARE_ODFEXPORT_TEST(testChapterNumberingNewLine, "chapter-number-new-line.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XChapterNumberingSupplier> xNumberingSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xNumberingRules = xNumberingSupplier->getChapterNumberingRules();
    comphelper::SequenceAsHashMap hashMap(xNumberingRules->getByIndex(0));

    //This failed Actual Value was LISTTAB instead of NEWLINE
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(SvxNumberFormat::NEWLINE), hashMap[u"LabelFollowedBy"_ustr].get<sal_Int16>());
}

DECLARE_ODFEXPORT_TEST(testSpellOutNumberingTypes, "spellout-numberingtypes.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // ordinal indicator, ordinal and cardinal number numbering styles (from LibreOffice 6.1)
    OUString const aFieldTexts[] = { u"1st"_ustr, u"Erste"_ustr, u"Eins"_ustr,  u"1."_ustr, u"Premier"_ustr, u"Un"_ustr, u"1ᵉʳ"_ustr, u"First"_ustr, u"One"_ustr };
    // fallback for old platforms without std::codecvt and std::regex supports
    OUString const aFieldTextFallbacks[] = { u"Ordinal-number 1"_ustr, u"Ordinal 1"_ustr, u"1"_ustr };
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    // update text field content
    uno::Reference<util::XRefreshable>(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY_THROW)->refresh();

    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    for (size_t i = 0; i < SAL_N_ELEMENTS(aFieldTexts); i++)
    {
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        if (xServiceInfo->supportsService(u"com.sun.star.text.textfield.PageNumber"_ustr))
        {
            uno::Reference<text::XTextContent> xField(aField, uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(true, aFieldTexts[i].equals(xField->getAnchor()->getString()) ||
                           aFieldTextFallbacks[i%3].equals(xField->getAnchor()->getString()));
        }
    }
}

DECLARE_ODFEXPORT_TEST(tdf133507_contextualSpacingSection, "tdf133507_contextualSpacingSection.odt")
{
    // Previously this was one page (no UL spacing) or three pages (every para had spacing)
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

// MAILMERGE Add conditional to expand / collapse bookmarks
DECLARE_ODFEXPORT_TEST(tdf101856_overlapped, "tdf101856_overlapped.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // get bookmark interface
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();

    // check: we have 2 bookmarks
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"BookmarkNonHidden"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"BookmarkHidden"_ustr));

    // <text:bookmark-start text:name="BookmarkNonHidden"/>
    uno::Reference<beans::XPropertySet> xBookmark1(xBookmarksByName->getByName(u"BookmarkNonHidden"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xBookmark1, UNO_NAME_BOOKMARK_CONDITION));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xBookmark1, UNO_NAME_BOOKMARK_HIDDEN));

    // <text:bookmark-start text:name="BookmarkHidden"/>
    uno::Reference<beans::XPropertySet> xBookmark2(xBookmarksByName->getByName(u"BookmarkHidden"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xBookmark2, UNO_NAME_BOOKMARK_CONDITION));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xBookmark2, UNO_NAME_BOOKMARK_HIDDEN));
}

// MAILMERGE Add conditional to expand / collapse bookmarks
DECLARE_ODFEXPORT_TEST(tdf101856, "tdf101856.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // get bookmark interface
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();

    // check: we have 2 bookmarks
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"BookmarkVisible"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"BookmarkHidden"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"BookmarkVisibleWithCondition"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"BookmarkNotHiddenWithCondition"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"BookmarkHiddenWithCondition"_ustr));

    // <text:bookmark-start text:name="BookmarkVisible"/>
    uno::Reference<beans::XPropertySet> xBookmark1(xBookmarksByName->getByName(u"BookmarkVisible"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xBookmark1, UNO_NAME_BOOKMARK_CONDITION));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xBookmark1, UNO_NAME_BOOKMARK_HIDDEN));

    // <text:bookmark-start text:name="BookmarkHidden" loext:condition="" loext:hidden="true"/>
    uno::Reference<beans::XPropertySet> xBookmark2(xBookmarksByName->getByName(u"BookmarkHidden"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xBookmark2, UNO_NAME_BOOKMARK_CONDITION));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xBookmark2, UNO_NAME_BOOKMARK_HIDDEN));

    // <text:bookmark-start text:name="BookmarkVisibleWithCondition" loext:condition="0==1" loext:hidden="true"/>
    uno::Reference<beans::XPropertySet> xBookmark3(xBookmarksByName->getByName(u"BookmarkVisibleWithCondition"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"0==1"_ustr, getProperty<OUString>(xBookmark3, UNO_NAME_BOOKMARK_CONDITION));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xBookmark3, UNO_NAME_BOOKMARK_HIDDEN));

    // <text:bookmark-start text:name="BookmarkNotHiddenWithCondition" loext:condition="1==1" loext:hidden="false"/>
    //
    // The following test doesn't work, while during output in the case of loext:hidden="false".
    // no additional parameters are written. Implementation should be reviewed.
    //
//    uno::Reference<beans::XPropertySet> xBookmark4(xBookmarksByName->getByName("BookmarkNotHiddenWithCondition"), uno::UNO_QUERY);
//    CPPUNIT_ASSERT_EQUAL(OUString("1==1"), getProperty<OUString>(xBookmark4, UNO_NAME_BOOKMARK_CONDITION));
//    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xBookmark4, UNO_NAME_BOOKMARK_HIDDEN));

    // <text:bookmark-start text:name="BookmarkHiddenWithCondition" loext:condition="1==1" loext:hidden="true"/>
    uno::Reference<beans::XPropertySet> xBookmark5(xBookmarksByName->getByName(u"BookmarkHiddenWithCondition"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1==1"_ustr, getProperty<OUString>(xBookmark5, UNO_NAME_BOOKMARK_CONDITION));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xBookmark5, UNO_NAME_BOOKMARK_HIDDEN));
}

DECLARE_ODFEXPORT_TEST(tdf118502, "tdf118502.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Make sure the replacement graphic is still there
    // (was gone because the original graphic was not recognized during load)
    auto xReplacementGraphic
        = getProperty<uno::Reference<graphic::XGraphic>>(xShape, u"ReplacementGraphic"_ustr);
    CPPUNIT_ASSERT(xReplacementGraphic.is());
}

CPPUNIT_TEST_FIXTURE(Test, tdf99631)
{
    loadAndReload("tdf99631.docx");
    // check import of VisualArea settings of the embedded XLSX OLE objects
    xmlDocUniquePtr pXmlDoc = parseExport(u"Object 1/settings.xml"_ustr);
    assertXPathContent(pXmlDoc, "//config:config-item[@config:name='VisibleAreaWidth']", u"4516");
    assertXPathContent(pXmlDoc, "//config:config-item[@config:name='VisibleAreaHeight']", u"903");

    xmlDocUniquePtr pXmlDoc2 = parseExport(u"Object 2/settings.xml"_ustr);
    assertXPathContent(pXmlDoc2, "//config:config-item[@config:name='VisibleAreaWidth']", u"4516");
    assertXPathContent(pXmlDoc2, "//config:config-item[@config:name='VisibleAreaHeight']", u"1355");
}

CPPUNIT_TEST_FIXTURE(Test, tdf163575)
{
    // crashes/assert at export time
    loadAndReload("tdf163575.docx");
}

CPPUNIT_TEST_FIXTURE(Test, tdf145871)
{
    loadAndReload("tdf145871.odt");
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    // Without the fix in place, this test would have failed with
    // - Expected: 3150
    // - Actual  : 5851
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3150) , getProperty<sal_Int64>(xTableRows->getByIndex(0), u"Height"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, tdf128504)
{
    loadAndReload("tdf128504.docx");
    uno::Reference<text::XTextRange> xPara = getParagraph(6);
    uno::Reference<beans::XPropertySet> xRun(getRun(xPara,1), uno::UNO_QUERY);
    OUString unVisitedStyleName = getProperty<OUString>(xRun, u"UnvisitedCharStyleName"_ustr);
    CPPUNIT_ASSERT(!unVisitedStyleName.equalsIgnoreAsciiCase("Internet Link"));
    OUString visitedStyleName = getProperty<OUString>(xRun, u"VisitedCharStyleName"_ustr);
    CPPUNIT_ASSERT(!visitedStyleName.equalsIgnoreAsciiCase("Visited Internet Link"));
}

DECLARE_ODFEXPORT_TEST(tdf121658, "tdf121658.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<container::XNameAccess> xParaStyles(getStyles(u"ParagraphStyles"_ustr));
    uno::Reference<beans::XPropertySet> xStyle1(xParaStyles->getByName(
            u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle1, u"ParaHyphenationNoCaps"_ustr));
}

DECLARE_ODFEXPORT_TEST(tdf149248, "tdf149248.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getParagraph(2), u"ParaHyphenationNoLastWord"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getParagraph(4), u"ParaHyphenationNoLastWord"_ustr));
}

DECLARE_ODFEXPORT_TEST(testThemeCrash, "tdf156905.odt")
{
    // first it started to crash at import time, later at export time
    CPPUNIT_ASSERT_EQUAL(4, getPages());
}

DECLARE_ODFEXPORT_TEST(testTdf150394, "tdf150394.odt")
{
    // crashes at import time
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_ODFEXPORT_TEST(tdf149324, "tdf149324.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), getProperty<sal_uInt16>(getParagraph(2), u"ParaHyphenationMinWordLength"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(7), getProperty<sal_uInt16>(getParagraph(4), u"ParaHyphenationMinWordLength"_ustr));
}

DECLARE_ODFEXPORT_TEST(tdf149420, "tdf149420.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), getProperty<sal_uInt16>(getParagraph(2), u"ParaHyphenationZone"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(567), getProperty<sal_uInt16>(getParagraph(4), u"ParaHyphenationZone"_ustr));
}

DECLARE_ODFEXPORT_TEST(testArabicZeroNumbering, "arabic-zero-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero3Numbering, "arabic-zero3-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 65
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO3.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO3),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero4Numbering, "arabic-zero4-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 66
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO4.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO4),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero5Numbering, "arabic-zero5-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 67
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO5.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO5),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testRovasNumbering, "rovas-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 68
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not SZEKELY_ROVAS.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::SZEKELY_ROVAS),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testPageContentTop, "page-content-top.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    sal_Int16 nExpected = text::RelOrientation::PAGE_PRINT_AREA_TOP;
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));
}

DECLARE_ODFEXPORT_TEST(testPageContentBottom, "page-content-bottom.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    sal_Int16 nExpected = text::RelOrientation::PAGE_PRINT_AREA_BOTTOM;
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, tdf124470)
{
    loadAndReload("tdf124470TableAndEmbeddedUsedFonts.odt");
    // Table styles were exported out of place, inside font-face-decls.
    // Without the fix in place, this will fail already in ODF validation:
    // "content.xml[2,2150]:  Error: tag name "style:style" is not allowed. Possible tag names are: <font-face>"

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    assertXPath(pXmlDoc, "/office:document-content/office:font-face-decls/style:style", 0);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:family='table']", 1);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:family='table-column']", 2);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:family='paragraph']", 1);
}

DECLARE_ODFEXPORT_TEST(testSaveFieldmarkStyle, "tdf165315.rtf")
{
    uno::Reference<beans::XPropertySet> xRunProperties(getRun(getParagraph(2), 2), uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xBookmark = getProperty<uno::Reference<text::XTextContent>>(xRunProperties, u"Bookmark"_ustr);
    CPPUNIT_ASSERT(xBookmark.is());
    uno::Reference<beans::XPropertySet> xRangePropSet(xBookmark->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xRangePropSet.is());
    double nFontHeight = getProperty<double>(xRangePropSet, u"CharHeight"_ustr);
    CPPUNIT_ASSERT(nFontHeight < 12.5);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
