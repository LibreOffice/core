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
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/text/ColumnSeparatorStyle.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <unotools/localedatawrapper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/unolingu.hxx>
#include <officecfg/Office/Common.hxx>
#include <unoprnms.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/odfexport/data/", "writer8") {}
};

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedFontProps)
{
    loadAndReload("embedded-font-props.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
#if !defined(MACOSX)
    // Test that font style/weight of embedded fonts is exposed.
    // Test file is a normal ODT, except EmbedFonts is set to true in settings.xml.
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // These failed, the attributes were missing.
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[1]"_ostr, "font-style"_ostr, "normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[1]"_ostr, "font-weight"_ostr, "normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[2]"_ostr, "font-style"_ostr, "normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[2]"_ostr, "font-weight"_ostr, "bold");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[3]"_ostr, "font-style"_ostr, "italic");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[3]"_ostr, "font-weight"_ostr, "normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[4]"_ostr, "font-style"_ostr, "italic");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[4]"_ostr, "font-weight"_ostr, "bold");
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
            { "FilterName", uno::Any(OUString("SVG")) }
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
    uno::Reference<container::XNameAccess> xStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( false , getProperty<bool>(xStyle, "GridDisplay"));
    CPPUNIT_ASSERT_EQUAL( false , getProperty<bool>(xStyle, "GridPrint"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106733)
{
    loadAndReload("tdf106733.fodt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    OUString autostyle = getXPath(pXmlDoc, "//office:body/office:text/text:p[2]/text:span"_ostr,
                                  "style-name"_ostr);
    OString autostyle_span_xpath = "//style:style[@style:name='" + autostyle.toUtf8() + "']";

    // keep fo:hyphenate="false" in direct formatting
    assertXPath(
        pXmlDoc,
        autostyle_span_xpath + "/style:text-properties",
        "hyphenate"_ostr, "false");

    // keep fo:hyphenate="false" in character style
    xmlDocUniquePtr pXmlDoc2 = parseExport("styles.xml");
    assertXPath(
        pXmlDoc2,
        "//style:style[@style:name='Strong_20_Emphasis']/style:text-properties"_ostr,
        "hyphenate"_ostr, "false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_page)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // fo:hyphenation-keep="page"
    loadAndReload("tdf132599_page.fodt");
    // This was 2 (not truncated hyphenated line)
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_auto)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // fo:hyphenation-keep="auto"
    loadAndReload("tdf132599_auto.fodt");
    // not truncated hyphenated line
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_spread)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // fo:hyphenation-keep="page" loext:hyphenation-keep-type="spread"
    loadAndReload("tdf132599_page.fodt");
    // shift last line of right page, resulting 3 pages
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132599_spread_left_page)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
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
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
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
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
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
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // last line of the last column is shifted, according to
    // fo:hyphenation-keep="page" loext:hyphenation-keep-type="page"
    loadAndReload("tdf132599_page_in_last_column.fodt");
    // shift last line of the first page, resulting 3 pages
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158885_compound_remain)
{
    loadAndReload("tdf158885_compound-remain.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    assertXPath(pXmlDoc, "//style:style[@style:family='paragraph']/style:text-properties[@loext:hyphenation-compound-remain-char-count='3']"_ostr, 1);
}

DECLARE_ODFEXPORT_TEST(testReferenceLanguage, "referencelanguage.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // Test loext:reference-language attribute of reference fields
    // (used from LibreOffice 6.1, and proposed for next ODF)
    OUString const aFieldTexts[] = { "A 2", "Az Isten", "Az 50-esek",
        "A 2018-asok", "Az egyebek", "A fejezetek",
        u"Az „Őseinket...”"_ustr, "a 2",
        "Az v", "az 1", "Az e)", "az 1",
        "Az (5)", "az 1", "A 2", "az 1" };
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    // update "A (4)" to "Az (5)"
    uno::Reference<util::XRefreshable>(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY_THROW)->refresh();

    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    uno::Any aHu(OUString("Hu"));
    uno::Any ahu(OUString("hu"));
    for (auto const& sFieldText : aFieldTexts)
    {
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        if (xServiceInfo->supportsService("com.sun.star.text.textfield.GetReference"))
        {
            uno::Reference<beans::XPropertySet> xPropertySet(aField, uno::UNO_QUERY);
            uno::Any aLang = xPropertySet->getPropertyValue("ReferenceFieldLanguage");
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
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    assertXPath(pXmlDoc, "//style:style[@style:family='ruby']/style:ruby-properties[@loext:ruby-position='inter-character']"_ostr, 1);
    assertXPath(pXmlDoc, "//style:style[@style:family='ruby']/style:ruby-properties[@style:ruby-position='below']"_ostr, 1);
}

DECLARE_ODFEXPORT_TEST(testAllowOverlap, "allow-overlap.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: !getProperty<bool>(xShape, "AllowOverlap")
    // i.e. the custom AllowOverlap=false shape property was lost on import/export.
    CPPUNIT_ASSERT(!getProperty<bool>(xShape, "AllowOverlap"));
    xShape = getShape(2);
    CPPUNIT_ASSERT(!getProperty<bool>(xShape, "AllowOverlap"));
}

CPPUNIT_TEST_FIXTURE(Test, testSignatureLineProperties)
{
    loadAndReload("signatureline-properties.fodt");
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT(xShape.is());

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, "IsSignatureLine"));
    CPPUNIT_ASSERT_EQUAL(OUString("{3C24159B-3B98-4F60-AB52-00E7721758E9}"),
                         getProperty<OUString>(xShape, "SignatureLineId"));
    CPPUNIT_ASSERT_EQUAL(OUString("John Doe"),
                         getProperty<OUString>(xShape, "SignatureLineSuggestedSignerName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Farmer"),
                         getProperty<OUString>(xShape, "SignatureLineSuggestedSignerTitle"));
    CPPUNIT_ASSERT_EQUAL(OUString("john@farmers.org"),
                         getProperty<OUString>(xShape, "SignatureLineSuggestedSignerEmail"));
    CPPUNIT_ASSERT_EQUAL(OUString("Please farm here."),
                         getProperty<OUString>(xShape, "SignatureLineSigningInstructions"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, "SignatureLineCanAddComment"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, "SignatureLineShowSignDate"));

    // tdf#130917 This needs to be always set when importing a doc, ooxml export expects it.
    uno::Reference<graphic::XGraphic> xUnsignedGraphic;
    uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY);
    xProps->getPropertyValue("SignatureLineUnsignedImage") >>= xUnsignedGraphic;
    CPPUNIT_ASSERT_EQUAL(true, xUnsignedGraphic.is());
}

DECLARE_ODFEXPORT_TEST(testQrCodeGenProperties, "qrcode-properties.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT(xShape.is());

    css::drawing::BarCode aBarCode = getProperty<css::drawing::BarCode>(xShape, "BarCodeProperties");

    CPPUNIT_ASSERT_EQUAL(OUString("www.libreoffice.org"),
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
        sal_Int16(SvxNumberFormat::NEWLINE), hashMap["LabelFollowedBy"].get<sal_Int16>());
}

DECLARE_ODFEXPORT_TEST(testSpellOutNumberingTypes, "spellout-numberingtypes.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // ordinal indicator, ordinal and cardinal number numbering styles (from LibreOffice 6.1)
    OUString const aFieldTexts[] = { "1st", "Erste", "Eins",  "1.", "Premier", "Un", u"1ᵉʳ"_ustr, "First", "One" };
    // fallback for old platforms without std::codecvt and std::regex supports
    OUString const aFieldTextFallbacks[] = { "Ordinal-number 1", "Ordinal 1", "1" };
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    // update text field content
    uno::Reference<util::XRefreshable>(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY_THROW)->refresh();

    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    for (size_t i = 0; i < SAL_N_ELEMENTS(aFieldTexts); i++)
    {
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        if (xServiceInfo->supportsService("com.sun.star.text.textfield.PageNumber"))
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
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("BookmarkNonHidden"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("BookmarkHidden"));

    // <text:bookmark-start text:name="BookmarkNonHidden"/>
    uno::Reference<beans::XPropertySet> xBookmark1(xBookmarksByName->getByName("BookmarkNonHidden"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xBookmark1, UNO_NAME_BOOKMARK_CONDITION));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xBookmark1, UNO_NAME_BOOKMARK_HIDDEN));

    // <text:bookmark-start text:name="BookmarkHidden"/>
    uno::Reference<beans::XPropertySet> xBookmark2(xBookmarksByName->getByName("BookmarkHidden"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xBookmark2, UNO_NAME_BOOKMARK_CONDITION));
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
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("BookmarkVisible"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("BookmarkHidden"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("BookmarkVisibleWithCondition"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("BookmarkNotHiddenWithCondition"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("BookmarkHiddenWithCondition"));

    // <text:bookmark-start text:name="BookmarkVisible"/>
    uno::Reference<beans::XPropertySet> xBookmark1(xBookmarksByName->getByName("BookmarkVisible"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xBookmark1, UNO_NAME_BOOKMARK_CONDITION));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xBookmark1, UNO_NAME_BOOKMARK_HIDDEN));

    // <text:bookmark-start text:name="BookmarkHidden" loext:condition="" loext:hidden="true"/>
    uno::Reference<beans::XPropertySet> xBookmark2(xBookmarksByName->getByName("BookmarkHidden"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xBookmark2, UNO_NAME_BOOKMARK_CONDITION));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xBookmark2, UNO_NAME_BOOKMARK_HIDDEN));

    // <text:bookmark-start text:name="BookmarkVisibleWithCondition" loext:condition="0==1" loext:hidden="true"/>
    uno::Reference<beans::XPropertySet> xBookmark3(xBookmarksByName->getByName("BookmarkVisibleWithCondition"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("0==1"), getProperty<OUString>(xBookmark3, UNO_NAME_BOOKMARK_CONDITION));
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
    uno::Reference<beans::XPropertySet> xBookmark5(xBookmarksByName->getByName("BookmarkHiddenWithCondition"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1==1"), getProperty<OUString>(xBookmark5, UNO_NAME_BOOKMARK_CONDITION));
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
        = getProperty<uno::Reference<graphic::XGraphic>>(xShape, "ReplacementGraphic");
    CPPUNIT_ASSERT(xReplacementGraphic.is());
}

CPPUNIT_TEST_FIXTURE(Test, tdf99631)
{
    loadAndReload("tdf99631.docx");
    // check import of VisualArea settings of the embedded XLSX OLE objects
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/settings.xml");
    assertXPathContent(pXmlDoc, "//config:config-item[@config:name='VisibleAreaWidth']"_ostr, "4516");
    assertXPathContent(pXmlDoc, "//config:config-item[@config:name='VisibleAreaHeight']"_ostr, "903");

    xmlDocUniquePtr pXmlDoc2 = parseExport("Object 2/settings.xml");
    assertXPathContent(pXmlDoc2, "//config:config-item[@config:name='VisibleAreaWidth']"_ostr, "4516");
    assertXPathContent(pXmlDoc2, "//config:config-item[@config:name='VisibleAreaHeight']"_ostr, "1355");
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
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3150) , getProperty<sal_Int64>(xTableRows->getByIndex(0), "Height"));
}

CPPUNIT_TEST_FIXTURE(Test, tdf128504)
{
    loadAndReload("tdf128504.docx");
    uno::Reference<text::XTextRange> xPara = getParagraph(6);
    uno::Reference<beans::XPropertySet> xRun(getRun(xPara,1), uno::UNO_QUERY);
    OUString unVisitedStyleName = getProperty<OUString>(xRun, "UnvisitedCharStyleName");
    CPPUNIT_ASSERT(!unVisitedStyleName.equalsIgnoreAsciiCase("Internet Link"));
    OUString visitedStyleName = getProperty<OUString>(xRun, "VisitedCharStyleName");
    CPPUNIT_ASSERT(!visitedStyleName.equalsIgnoreAsciiCase("Visited Internet Link"));
}

DECLARE_ODFEXPORT_TEST(tdf121658, "tdf121658.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<container::XNameAccess> xParaStyles(getStyles("ParagraphStyles"));
    uno::Reference<beans::XPropertySet> xStyle1(xParaStyles->getByName(
            "Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle1, "ParaHyphenationNoCaps"));
}

DECLARE_ODFEXPORT_TEST(tdf149248, "tdf149248.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getParagraph(2), "ParaHyphenationNoLastWord"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getParagraph(4), "ParaHyphenationNoLastWord"));
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
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), getProperty<sal_uInt16>(getParagraph(2), "ParaHyphenationMinWordLength"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(7), getProperty<sal_uInt16>(getParagraph(4), "ParaHyphenationMinWordLength"));
}

DECLARE_ODFEXPORT_TEST(tdf149420, "tdf149420.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), getProperty<sal_uInt16>(getParagraph(2), "ParaHyphenationZone"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(567), getProperty<sal_uInt16>(getParagraph(4), "ParaHyphenationZone"));
}

DECLARE_ODFEXPORT_TEST(testArabicZeroNumbering, "arabic-zero-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero3Numbering, "arabic-zero3-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 65
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO3.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO3),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero4Numbering, "arabic-zero4-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 66
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO4.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO4),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero5Numbering, "arabic-zero5-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 67
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO5.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO5),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testRovasNumbering, "rovas-numbering.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 68
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not SZEKELY_ROVAS.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::SZEKELY_ROVAS),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testPageContentTop, "page-content-top.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    sal_Int16 nExpected = text::RelOrientation::PAGE_PRINT_AREA_TOP;
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int16>(xShape, "VertOrientRelation"));
}

DECLARE_ODFEXPORT_TEST(testPageContentBottom, "page-content-bottom.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    sal_Int16 nExpected = text::RelOrientation::PAGE_PRINT_AREA_BOTTOM;
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int16>(xShape, "VertOrientRelation"));
}

CPPUNIT_TEST_FIXTURE(Test, tdf124470)
{
    loadAndReload("tdf124470TableAndEmbeddedUsedFonts.odt");
    // Table styles were exported out of place, inside font-face-decls.
    // Without the fix in place, this will fail already in ODF validation:
    // "content.xml[2,2150]:  Error: tag name "style:style" is not allowed. Possible tag names are: <font-face>"

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    assertXPath(pXmlDoc, "/office:document-content/office:font-face-decls/style:style"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:family='table']"_ostr, 1);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:family='table-column']"_ostr, 2);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:family='paragraph']"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, tdf135942)
{
    loadAndReload("nestedTableInFooter.odt");
    // All table autostyles should be collected, including nested, and must not crash.

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");

    assertXPath(pXmlDoc, "/office:document-styles/office:automatic-styles/style:style[@style:family='table']"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, tdf150927)
{
    // Similar to tdf135942

    loadAndReload("table-in-frame-in-table-in-header-base.odt");
    // All table autostyles should be collected, including nested, and must not crash.

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");

    assertXPath(pXmlDoc, "/office:document-styles/office:automatic-styles/style:style[@style:family='table']"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testPersonalMetaData)
{
    // 1. Remove personal info, keep user info
    auto pBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::set(true, pBatch);
    officecfg::Office::Common::Security::Scripting::KeepDocUserInfoOnSaving::set(true, pBatch);
    pBatch->commit();

    loadAndReload("personalmetadata.odt");
    xmlDocUniquePtr pXmlDoc = parseExport("meta.xml");
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:initial-creator"_ostr, 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:creation-date"_ostr, 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/dc:date"_ostr, 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/dc:creator"_ostr, 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:printed-by"_ostr, 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:print-date"_ostr, 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:editing-duration"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:editing-cycles"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:template"_ostr, 0);
    pXmlDoc = parseExport("settings.xml");
    assertXPath(
        pXmlDoc,
        "/office:document-settings/office:settings/config:config-item-set[2]/config:config-item[@config:name='PrinterName']"_ostr,
        0);
    assertXPath(
        pXmlDoc,
        "/office:document-settings/office:settings/config:config-item-set[2]/config:config-item[@config:name='PrinterSetup']"_ostr,
        0);

    // 2. Remove user info too
    officecfg::Office::Common::Security::Scripting::KeepDocUserInfoOnSaving::set(false, pBatch);
    pBatch->commit();

    loadAndReload("personalmetadata.odt");
    pXmlDoc = parseExport("meta.xml");
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:initial-creator"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:creation-date"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/dc:date"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/dc:creator"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:printed-by"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:print-date"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:editing-duration"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:editing-cycles"_ostr, 0);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/meta:template"_ostr, 0);
    pXmlDoc = parseExport("settings.xml");
    assertXPath(
        pXmlDoc,
        "/office:document-settings/office:settings/config:config-item-set[2]/config:config-item[@config:name='PrinterName']"_ostr,
        0);
    assertXPath(
        pXmlDoc,
        "/office:document-settings/office:settings/config:config-item-set[2]/config:config-item[@config:name='PrinterSetup']"_ostr,
        0);
}

CPPUNIT_TEST_FIXTURE(Test, tdf151100)
{
    // Similar to tdf135942

    loadAndReload("tdf151100.docx");
    // All table autostyles should be collected, including nested, and must not crash.

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");

    assertXPath(pXmlDoc, "/office:document-styles/office:automatic-styles/style:style[@style:family='table']"_ostr, 1);
}

DECLARE_ODFEXPORT_TEST(testGutterLeft, "gutter-left.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xPageStyle;
    getStyles("PageStyles")->getByName("Standard") >>= xPageStyle;
    sal_Int32 nGutterMargin{};
    xPageStyle->getPropertyValue("GutterMargin") >>= nGutterMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1270
    // - Actual  : 0
    // i.e. gutter margin was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nGutterMargin);
}

DECLARE_ODFEXPORT_TEST(testTdf52065_centerTabs, "testTdf52065_centerTabs.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    sal_Int32 nTabStop = parseDump("//body/txt[4]/SwParaPortion/SwLineLayout/child::*[3]"_ostr, "width"_ostr).toInt32();
    // Without the fix, the text was unseen, with a tabstop width of 64057. It should be 3057
    CPPUNIT_ASSERT(nTabStop < 4000);
    CPPUNIT_ASSERT(3000 < nTabStop);
    CPPUNIT_ASSERT_EQUAL(u"Pečiatka zamestnávateľa"_ustr, parseDump("//body/txt[4]/SwParaPortion/SwLineLayout/child::*[4]"_ostr, "portion"_ostr));

    // tdf#149547: __XXX___invalid CharacterStyles should not be imported/exported
    CPPUNIT_ASSERT(!getStyles("CharacterStyles")->hasByName("__XXX___invalid"));
}

DECLARE_ODFEXPORT_TEST(testTdf104254_noHeaderWrapping, "tdf104254_noHeaderWrapping.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nParaHeight = getXPath(pXmlDoc, "//header/txt[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    // The wrapping on header images is supposed to be ignored (since OOo for MS compat reasons),
    // thus making the text run underneath the image. Before, height was 1104. Now it is 552.
    CPPUNIT_ASSERT_MESSAGE("Paragraph should fit on a single line", nParaHeight < 600);
}

DECLARE_ODFEXPORT_TEST(testTdf131025_noZerosInTable, "tdf131025_noZerosInTable.odt")
{
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    uno::Reference<text::XTextTable> xTable(xTables->getByName("Table1"), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("C3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("5 gp"), xCell->getString());
}

DECLARE_ODFEXPORT_TEST(testTdf153090, "Custom-Style-TOC.docx")
{
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes(xIndexSupplier->getDocumentIndexes());
    uno::Reference<text::XDocumentIndex> xTOC(xIndexes->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("_CustomImageCaption"), getProperty<OUString>(xTOC, "CreateFromParagraphStyle"));
    // tdf#153659 this was imported as "table of figures" instead of "Figure Index 1"
    // thus custom settings were not retained after ToF update
    CPPUNIT_ASSERT_EQUAL(OUString("Figure Index 1"), getProperty<OUString>(getParagraph(1), "ParaStyleName"));

    xTOC->update();
    OUString const tocContent(xTOC->getAnchor()->getString());
    CPPUNIT_ASSERT(tocContent.indexOf("1. Abb. Ein Haus") != -1);
    CPPUNIT_ASSERT(tocContent.indexOf("2. Abb.Ein Schiff!") != -1);
    CPPUNIT_ASSERT(tocContent.indexOf(u"1. ábra Small house with Hungarian description category") != -1);
}

DECLARE_ODFEXPORT_TEST(testTdf143793_noBodyWrapping, "tdf143793_noBodyWrapping.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    // Preserve old document wrapping. Compat "Use OOo 1.1 text wrapping around objects"
    // Originally, the body text did not wrap around spill-over header images
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Fits on one page", 1, getPages() );

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nParaHeight = getXPath(pXmlDoc, "//page[1]/header/txt[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    // The header text should wrap around the header image in OOo 1.1 and prior,
    // thus taking up two lines instead of one. One line is 276. It should be 552.
    CPPUNIT_ASSERT_MESSAGE("Header text should fill two lines", nParaHeight > 400);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf137199)
{
    loadAndReload("tdf137199.docx");
    CPPUNIT_ASSERT_EQUAL(OUString(">1<"), getProperty<OUString>(getParagraph(1), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("1)"), getProperty<OUString>(getParagraph(2), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("HELLO1WORLD!"), getProperty<OUString>(getParagraph(3), "ListLabelString"));

    CPPUNIT_ASSERT_EQUAL(OUString("HELLO2WORLD!"), getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

DECLARE_ODFEXPORT_TEST(testTdf143605, "tdf143605.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // With numbering type "none" there should be just prefix & suffix
    CPPUNIT_ASSERT_EQUAL(OUString("."), getProperty<OUString>(getParagraph(1), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf57317_autoListName)
{
    createSwDoc("tdf57317_autoListName.odt");
    // The list style (from styles.xml) overrides a duplicate named auto-style
    //uno::Any aNumStyle = getStyles("NumberingStyles")->getByName("L1");
    //CPPUNIT_ASSERT(aNumStyle.hasValue());
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(">1<"), getProperty<OUString>(xPara, "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("L1"), getProperty<OUString>(xPara, "NumberingStyleName"));

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:DefaultBullet", {});

    // This was failing with a duplicate auto numbering style name of L1 instead of a unique name,
    // thus it was showing the same info as before the bullet modification.
    saveAndReload("writer8");
    xPara.set(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));

    uno::Reference<container::XIndexAccess> xLevels(xPara->getPropertyValue("NumberingRules"),
                                                    uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "BulletChar")
            return;
    }
    CPPUNIT_FAIL("no BulletChar property");
}

CPPUNIT_TEST_FIXTURE(Test, testListFormatDocx)
{
    loadAndReload("listformat.docx");
    // Ensure in resulting ODT we also have not just prefix/suffix, but custom delimiters
    CPPUNIT_ASSERT_EQUAL(OUString(">1<"), getProperty<OUString>(getParagraph(1), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1/1<<"), getProperty<OUString>(getParagraph(2), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1/1/1<<"), getProperty<OUString>(getParagraph(3), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1/1/2<<"), getProperty<OUString>(getParagraph(4), "ListLabelString"));

    // Check also that in numbering styles we have num-list-format defined
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='1']"_ostr, "num-list-format"_ostr, ">%1%<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='2']"_ostr, "num-list-format"_ostr, ">>%1%/%2%<<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='3']"_ostr, "num-list-format"_ostr, ">>%1%/%2%/%3%<<");

    // But for compatibility there are still prefix/suffix
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='1']"_ostr, "num-prefix"_ostr, ">");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='1']"_ostr, "num-suffix"_ostr, "<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='2']"_ostr, "num-prefix"_ostr, ">>");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='2']"_ostr, "num-suffix"_ostr, "<<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='3']"_ostr, "num-prefix"_ostr, ">>");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='3']"_ostr, "num-suffix"_ostr, "<<");
}

DECLARE_ODFEXPORT_TEST(testShapeWithHyperlink, "shape-with-hyperlink.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (isExported())
    {
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        // Check how conversion from prefix/suffix to list format did work
        assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p/draw:a"_ostr,
                    "href"_ostr, "http://shape.com/");
    }
}

DECLARE_ODFEXPORT_TEST(testShapesHyperlink, "shapes-hyperlink.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(5, getShapes());
    uno::Reference<beans::XPropertySet> const xPropSet1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice.org/"), getProperty<OUString>(xPropSet1, "Hyperlink"));

    uno::Reference<beans::XPropertySet> const xPropSet2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice2.org/"), getProperty<OUString>(xPropSet2, "Hyperlink"));

    uno::Reference<beans::XPropertySet> const xPropSet3(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice3.org/"), getProperty<OUString>(xPropSet3, "Hyperlink"));

    uno::Reference<beans::XPropertySet> const xPropSet4(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice4.org/"), getProperty<OUString>(xPropSet4, "Hyperlink"));

    uno::Reference<beans::XPropertySet> const xPropSet5(getShape(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice5.org/"), getProperty<OUString>(xPropSet5, "Hyperlink"));
}

DECLARE_ODFEXPORT_TEST(testListFormatOdt, "listformat.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Ensure in resulting ODT we also have not just prefix/suffix, but custom delimiters
    CPPUNIT_ASSERT_EQUAL(OUString(">1<"), getProperty<OUString>(getParagraph(1), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1.1<<"), getProperty<OUString>(getParagraph(2), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1.1.1<<"), getProperty<OUString>(getParagraph(3), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(">>1.1.2<<"), getProperty<OUString>(getParagraph(4), "ListLabelString"));

    if (isExported())
    {
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        // Check how conversion from prefix/suffix to list format did work
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='1']"_ostr, "num-list-format"_ostr, ">%1%<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='2']"_ostr, "num-list-format"_ostr, ">>%1%.%2%<<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='3']"_ostr, "num-list-format"_ostr, ">>%1%.%2%.%3%<<");

        // But for compatibility there are still prefix/suffix as they were before
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='1']"_ostr, "num-prefix"_ostr, ">");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='1']"_ostr, "num-suffix"_ostr, "<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='2']"_ostr, "num-prefix"_ostr, ">>");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='2']"_ostr, "num-suffix"_ostr, "<<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='3']"_ostr, "num-prefix"_ostr, ">>");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='3']"_ostr, "num-suffix"_ostr, "<<");
    }
}

CPPUNIT_TEST_FIXTURE(Test, testStyleLink)
{
    // Given a document with a para and a char style that links each other, when loading that
    // document:
    createSwDoc("style-link.fodt");

    // Then make sure the char style links the para one:
    uno::Any aCharStyle = getStyles("CharacterStyles")->getByName("List Paragraph Char");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: List Paragraph
    // - Actual  :
    // i.e. the linked style was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"), getProperty<OUString>(aCharStyle, "LinkStyle"));
    uno::Any aParaStyle = getStyles("ParagraphStyles")->getByName("List Paragraph");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: List Paragraph Char
    // - Actual  :
    // i.e. the linked style was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph Char"), getProperty<OUString>(aParaStyle, "LinkStyle"));
}

CPPUNIT_TEST_FIXTURE(Test, tdf120972)
{
    loadAndReload("table_number_format_3.docx");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    OUString cDecimal(SvtSysLocale().GetLocaleData().getNumDecimalSep()[0]);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/style:tab-stop"_ostr,
        "char"_ostr, cDecimal);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:name='P2']/style:paragraph-properties/style:tab-stops/style:tab-stop"_ostr,
        "char"_ostr, cDecimal);
}

DECLARE_ODFEXPORT_TEST(testTdf114287, "tdf114287.odt")
{
    uno::Reference<container::XIndexAccess> const xLevels1(
        getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(2), "NumberingRules"));
    uno::Reference<container::XNamed> const xNum1(xLevels1, uno::UNO_QUERY);
    ::comphelper::SequenceAsHashMap props1(xLevels1->getByIndex(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-700), props1["FirstLineIndent"].get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1330), props1["IndentAt"].get<sal_Int32>());

    // 1: automatic style applies list-style-name and margin-left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1000), getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), getProperty<sal_Int32>(getParagraph(2), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), "ParaRightMargin"));

    // list is continued
    uno::Reference<container::XNamed> const xNum2(
        getProperty<uno::Reference<container::XNamed>>(getParagraph(9), "NumberingRules"));
    CPPUNIT_ASSERT_EQUAL(xNum1->getName(), xNum2->getName());

    // 2: style applies list-style-name and margin-left, list applies list-style-name
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1000), getProperty<sal_Int32>(getParagraph(9), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), getProperty<sal_Int32>(getParagraph(9), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(9), "ParaRightMargin"));

    // list is continued
    uno::Reference<container::XNamed> const xNum3(
        getProperty<uno::Reference<container::XNamed>>(getParagraph(16), "NumberingRules"));
    CPPUNIT_ASSERT_EQUAL(xNum1->getName(), xNum3->getName());

    // 3: style applies margin-left, automatic style applies list-style-name
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1000), getProperty<sal_Int32>(getParagraph(16), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), getProperty<sal_Int32>(getParagraph(16), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(16), "ParaRightMargin"));

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/prtBounds"_ostr, "left"_ostr, "2268");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/prtBounds"_ostr, "right"_ostr, "11339");
    // the list style name of the list is the same as the list style name of the
    // paragraph, but in any case the margins of the paragraph take precedence
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/infos/prtBounds"_ostr, "left"_ostr, "2268");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/infos/prtBounds"_ostr, "right"_ostr, "11339");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[16]/infos/prtBounds"_ostr, "left"_ostr, "357");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[16]/infos/prtBounds"_ostr, "right"_ostr, "11339");
}

DECLARE_ODFEXPORT_TEST(testSectionColumnSeparator, "section-columns-separator.fodt")
{
    // tdf#150235: due to wrong types used in column export, 'style:height' and 'style:style'
    // attributes were exported incorrectly for 'style:column-sep' element
    auto xSection = getProperty<uno::Reference<uno::XInterface>>(getParagraph(1), "TextSection");
    auto xColumns = getProperty<uno::Reference<text::XTextColumns>>(xSection, "TextColumns");
    CPPUNIT_ASSERT(xColumns);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xColumns->getColumnCount());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 50
    // - Actual  : 100
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(50),
                         getProperty<sal_Int32>(xColumns, "SeparatorLineRelativeHeight"));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(css::text::ColumnSeparatorStyle::DOTTED,
                         getProperty<sal_Int16>(xColumns, "SeparatorLineStyle"));

    // Check the rest of the properties, too
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xColumns, "IsAutomatic"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(600), getProperty<sal_Int32>(xColumns, "AutomaticDistance"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), getProperty<sal_Int32>(xColumns, "SeparatorLineWidth"));
    CPPUNIT_ASSERT_EQUAL(Color(0x99, 0xAA, 0xBB),
                         getProperty<Color>(xColumns, "SeparatorLineColor"));
    CPPUNIT_ASSERT_EQUAL(
        css::style::VerticalAlignment_BOTTOM,
        getProperty<css::style::VerticalAlignment>(xColumns, "SeparatorLineVerticalAlignment"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xColumns, "SeparatorLineIsOn"));
}

DECLARE_ODFEXPORT_TEST(testTdf78510, "WordTest_edit.odt")
{
    uno::Reference<container::XIndexAccess> const xLevels1(
        getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules"));
    ::comphelper::SequenceAsHashMap props1(xLevels1->getByIndex(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1000), props1["FirstLineIndent"].get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), props1["IndentAt"].get<sal_Int32>());

    // 1: inherited from paragraph style and overridden by list
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaRightMargin"));
    // 2: as 1 + paragraph sets firstline
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(2), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), "ParaRightMargin"));
    // 3: as 1 + paragraph sets textleft
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(3), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3000), getProperty<sal_Int32>(getParagraph(3), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(3), "ParaRightMargin"));
    // 4: as 1 + paragraph sets firstline, textleft
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2000), getProperty<sal_Int32>(getParagraph(4), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3000), getProperty<sal_Int32>(getParagraph(4), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(4), "ParaRightMargin"));
    // 5: as 1 + paragraph sets firstline
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2000), getProperty<sal_Int32>(getParagraph(5), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(5), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(5), "ParaRightMargin"));
    // 6: as 1
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(6), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(6), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(6), "ParaRightMargin"));

    uno::Reference<container::XIndexAccess> const xLevels8(
        getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(8), "NumberingRules"));
    ::comphelper::SequenceAsHashMap props8(xLevels8->getByIndex(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), props8["FirstLineIndent"].get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), props8["IndentAt"].get<sal_Int32>());

    // 8: inherited from paragraph style and overridden by list
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(8), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(8), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(8), "ParaRightMargin"));
    // 9: as 8 + paragraph sets firstline
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(getParagraph(9), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(9), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(9), "ParaRightMargin"));
    // 10: as 8 + paragraph sets textleft
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(10), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3000), getProperty<sal_Int32>(getParagraph(10), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(10), "ParaRightMargin"));
    // 11: as 8 + paragraph sets firstline, textleft
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2000), getProperty<sal_Int32>(getParagraph(11), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3000), getProperty<sal_Int32>(getParagraph(11), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(11), "ParaRightMargin"));
    // 12: as 8 + paragraph sets firstline
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2000), getProperty<sal_Int32>(getParagraph(12), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(12), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(12), "ParaRightMargin"));
    // 13: as 8
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(13), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(13), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(13), "ParaRightMargin"));

    // unfortunately it appears that the portions don't have a position
    // so it's not possible to check the first-line-offset that's applied
    // (the first-line-indent is computed on the fly in SwTextMargin when
    // painting)
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/prtBounds"_ostr, "left"_ostr, "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/prtBounds"_ostr, "left"_ostr, "1134");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/prtBounds"_ostr, "left"_ostr, "1134");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/infos/prtBounds"_ostr, "left"_ostr, "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/infos/prtBounds"_ostr, "left"_ostr, "0");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/infos/prtBounds"_ostr, "left"_ostr, "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/infos/prtBounds"_ostr, "right"_ostr, "9359");

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/infos/prtBounds"_ostr, "left"_ostr, "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/infos/prtBounds"_ostr, "left"_ostr, "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/infos/prtBounds"_ostr, "left"_ostr, "1701");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/infos/prtBounds"_ostr, "left"_ostr, "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/infos/prtBounds"_ostr, "left"_ostr, "-567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/infos/prtBounds"_ostr, "right"_ostr, "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/infos/prtBounds"_ostr, "left"_ostr, "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/infos/prtBounds"_ostr, "right"_ostr, "9359");
    }

    // now check the positions where text is actually painted -
    // wonder how fragile this is...
    // FIXME some platform difference, 1st one is 2306 on Linux, 3087 on WNT ?
    // some Mac has 3110
#if !defined(_WIN32) && !defined(MACOSX)
    {
        SwDocShell *const pShell(dynamic_cast<SwXTextDocument&>(*mxComponent).GetDocShell());
        std::shared_ptr<GDIMetaFile> pMetaFile = pShell->GetPreviewMetaFile();
        MetafileXmlDump aDumper;
        xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *pMetaFile);

        // 1: inherited from paragraph style and overridden by list
        // bullet char is extra

        assertXPath(pXmlDoc, "//textarray[1]"_ostr, "x"_ostr, "2306");
        // text is after a tab from list - haven't checked if that is correct?
        assertXPath(pXmlDoc, "//textarray[2]"_ostr, "x"_ostr, "2873");
        // second line
        assertXPath(pXmlDoc, "//textarray[3]"_ostr, "x"_ostr, "2873");
        // 2: as 1 + paragraph sets firstline
        assertXPath(pXmlDoc, "//textarray[4]"_ostr, "x"_ostr, "3440");
        assertXPath(pXmlDoc, "//textarray[5]"_ostr, "x"_ostr, "3593");
        assertXPath(pXmlDoc, "//textarray[6]"_ostr, "x"_ostr, "2873");
        // 3: as 1 + paragraph sets textleft
        assertXPath(pXmlDoc, "//textarray[7]"_ostr, "x"_ostr, "2873");
        assertXPath(pXmlDoc, "//textarray[8]"_ostr, "x"_ostr, "3440");
        assertXPath(pXmlDoc, "//textarray[9]"_ostr, "x"_ostr, "3440");
        // 4: as 1 + paragraph sets firstline, textleft
        assertXPath(pXmlDoc, "//textarray[10]"_ostr, "x"_ostr, "2306");
        assertXPath(pXmlDoc, "//textarray[11]"_ostr, "x"_ostr, "3440");
        assertXPath(pXmlDoc, "//textarray[12]"_ostr, "x"_ostr, "3440");
        // 5: as 1 + paragraph sets firstline
        assertXPath(pXmlDoc, "//textarray[13]"_ostr, "x"_ostr, "1739");
        assertXPath(pXmlDoc, "//textarray[14]"_ostr, "x"_ostr, "2873");
        assertXPath(pXmlDoc, "//textarray[15]"_ostr, "x"_ostr, "2873");
        // 6: as 1
        assertXPath(pXmlDoc, "//textarray[16]"_ostr, "x"_ostr, "2306");
        assertXPath(pXmlDoc, "//textarray[17]"_ostr, "x"_ostr, "2873");

        // 8: inherited from paragraph style and overridden by list
        assertXPath(pXmlDoc, "//textarray[18]"_ostr, "x"_ostr, "2873");
        assertXPath(pXmlDoc, "//textarray[19]"_ostr, "x"_ostr, "3746");
        assertXPath(pXmlDoc, "//textarray[20]"_ostr, "x"_ostr, "2306");
        // 9: as 8 + paragraph sets firstline
        assertXPath(pXmlDoc, "//textarray[21]"_ostr, "x"_ostr, "3440");
        assertXPath(pXmlDoc, "//textarray[22]"_ostr, "x"_ostr, "3746");
        assertXPath(pXmlDoc, "//textarray[23]"_ostr, "x"_ostr, "2306");
        // 10: as 8 + paragraph sets textleft
        assertXPath(pXmlDoc, "//textarray[24]"_ostr, "x"_ostr, "4007");
        assertXPath(pXmlDoc, "//textarray[25]"_ostr, "x"_ostr, "4880");
        assertXPath(pXmlDoc, "//textarray[26]"_ostr, "x"_ostr, "3440");
        // 11: as 8 + paragraph sets firstline, textleft
        assertXPath(pXmlDoc, "//textarray[27]"_ostr, "x"_ostr, "2306");
        assertXPath(pXmlDoc, "//textarray[28]"_ostr, "x"_ostr, "3440");
        assertXPath(pXmlDoc, "//textarray[29]"_ostr, "x"_ostr, "3440");
        // 12: as 8 + paragraph sets firstline
        assertXPath(pXmlDoc, "//textarray[30]"_ostr, "x"_ostr, "1172");
        assertXPath(pXmlDoc, "//textarray[31]"_ostr, "x"_ostr, "1739");
        assertXPath(pXmlDoc, "//textarray[32]"_ostr, "x"_ostr, "2306");
        // 13: as 8
        assertXPath(pXmlDoc, "//textarray[33]"_ostr, "x"_ostr, "2873");
        assertXPath(pXmlDoc, "//textarray[34]"_ostr, "x"_ostr, "3746");
    }
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testParagraphMarkerMarkupRoundtrip)
{
    loadAndReload("ParagraphMarkerMarkup.fodt");
    // Test that the markup stays at save-and-reload
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    OUString autostyle
        = getXPath(pXmlDoc, "//office:body/office:text/text:p"_ostr, "marker-style-name"_ostr);
    OString style_text_properties
        = "/office:document-content/office:automatic-styles/style:style[@style:name='"
          + autostyle.toUtf8() + "']/style:text-properties";
    assertXPath(pXmlDoc, style_text_properties, "font-size"_ostr, "9pt");
    assertXPath(pXmlDoc, style_text_properties, "color"_ostr, "#ff0000");
}

CPPUNIT_TEST_FIXTURE(Test, testCommentStyles)
{
    createSwDoc();

    auto xFactory(mxComponent.queryThrow<lang::XMultiServiceFactory>());
    auto xComment(xFactory->createInstance("com.sun.star.text.textfield.Annotation").queryThrow<text::XTextContent>());
    auto xCommentText(getProperty<uno::Reference<text::XTextRange>>(xComment, "TextRange"));
    xCommentText->setString("Hello World");
    xCommentText.queryThrow<beans::XPropertySet>()->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading")));

    xComment->attach(getParagraph(1)->getEnd());

    saveAndReload("writer8");

    auto xFields(mxComponent.queryThrow<text::XTextFieldsSupplier>()->getTextFields()->createEnumeration());
    xComment.set(xFields->nextElement().queryThrow<text::XTextContent>());
    CPPUNIT_ASSERT(xComment.queryThrow<lang::XServiceInfo>()->supportsService("com.sun.star.text.textfield.Annotation"));

    xCommentText.set(getProperty<uno::Reference<text::XTextRange>>(xComment, "TextRange"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading"), getProperty<OUString>(xCommentText, "ParaStyleName"));

    auto xStyleFamilies(mxComponent.queryThrow<style::XStyleFamiliesSupplier>()->getStyleFamilies());
    auto xParaStyles(xStyleFamilies->getByName("ParagraphStyles"));
    auto xStyle(xParaStyles.queryThrow<container::XNameAccess>()->getByName("Heading"));
    CPPUNIT_ASSERT_EQUAL(getProperty<float>(xStyle, "CharHeight"), getProperty<float>(xCommentText, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE,
                         xCommentText.queryThrow<beans::XPropertyState>()->getPropertyState("CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150408_IsLegal)
{
    loadAndReload("IsLegal.fodt");

    // Second level's numbering should use Arabic numbers for first level reference
    auto xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("CH I"), getProperty<OUString>(xPara, "ListLabelString"));
    xPara = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(OUString("Sect 1.01"), getProperty<OUString>(xPara, "ListLabelString"));
    xPara = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString("CH II"), getProperty<OUString>(xPara, "ListLabelString"));
    xPara = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(OUString("Sect 2.01"), getProperty<OUString>(xPara, "ListLabelString"));

    // Test that the markup stays at save-and-reload
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    assertXPath(
        pXmlDoc,
        "/office:document-styles/office:styles/text:outline-style/text:outline-level-style[2]"_ostr,
        "is-legal"_ostr, "true");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159382)
{
    // Testing NoGapAfterNoteNumber compat option

    createSwDoc("footnote_spacing_hanging_para.docx");
    // 1. Make sure that DOCX import sets NoGapAfterNoteNumber option, and creates
    // correct layout
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true), xSettings->getPropertyValue(
                                                 u"NoGapAfterNoteNumber"_ustr));

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        sal_Int32 width
            = getXPath(pXmlDoc,
                       "/root/page/ftncont/ftn/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                       "width"_ostr)
                  .toInt32();
        CPPUNIT_ASSERT(width);
        CPPUNIT_ASSERT_LESS(sal_Int32(100), width); // It was 720, i.e. 0.5 inch
    }

    saveAndReload(mpFilter);
    // 2. Make sure that exported document has NoGapAfterNoteNumber option set,
    // and has correct layout
    {
        xmlDocUniquePtr pXmlDoc = parseExport("settings.xml");
        assertXPathContent(
            pXmlDoc,
            "//config:config-item[@config:name='NoGapAfterNoteNumber']"_ostr, "true");

        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true), xSettings->getPropertyValue(
                                                 u"NoGapAfterNoteNumber"_ustr));

        pXmlDoc = parseLayoutDump();
        sal_Int32 width = getXPath(
            pXmlDoc, "/root/page/ftncont/ftn/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
            "width"_ostr).toInt32();
        CPPUNIT_ASSERT(width);
        CPPUNIT_ASSERT_LESS(sal_Int32(100), width);
    }

    createSwDoc("footnote_spacing_hanging_para.doc");
    // 3. Make sure that DOC import sets NoGapAfterNoteNumber option, and creates
    // correct layout
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true), xSettings->getPropertyValue(
                                                 u"NoGapAfterNoteNumber"_ustr));

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        sal_Int32 width
            = getXPath(pXmlDoc,
                       "/root/page/ftncont/ftn/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                       "width"_ostr)
                  .toInt32();
        CPPUNIT_ASSERT(width);
        CPPUNIT_ASSERT_LESS(sal_Int32(100), width);
    }

    createSwDoc("footnote_spacing_hanging_para.rtf");
    // 4. Make sure that RTF import sets NoGapAfterNoteNumber option, and creates
    // correct layout
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true), xSettings->getPropertyValue(
                                                 u"NoGapAfterNoteNumber"_ustr));

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        sal_Int32 width
            = getXPath(pXmlDoc,
                       "/root/page/ftncont/ftn/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                       "width"_ostr)
                  .toInt32();
        CPPUNIT_ASSERT(width);
        CPPUNIT_ASSERT_LESS(sal_Int32(100), width);
    }

    createSwDoc();
    // 5. Make sure that a new Writer document has this setting set to false
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(false), xSettings->getPropertyValue(
                                                  u"NoGapAfterNoteNumber"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159438)
{
    // Given a text with bookmarks, where an end of one bookmark is the position of another,
    // and the start of a third
    loadAndReload("bookmark_order.fodt");
    auto xPara = getParagraph(1);

    // Check that the order of runs is correct (bookmarks don't overlap)

    {
        auto run = getRun(xPara, 1);
        CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr, getProperty<OUString>(run, "TextPortionType"));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(run, "IsStart"));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(run, "IsCollapsed"));
        auto named = getProperty<uno::Reference<container::XNamed>>(run, "Bookmark");
        CPPUNIT_ASSERT_EQUAL(u"bookmark1"_ustr, named->getName());
    }

    {
        auto run = getRun(xPara, 2);
        CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(run, "TextPortionType"));
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, run->getString());
    }

    {
        auto run = getRun(xPara, 3);
        CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr, getProperty<OUString>(run, "TextPortionType"));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(run, "IsStart"));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(run, "IsCollapsed"));
        auto named = getProperty<uno::Reference<container::XNamed>>(run, "Bookmark");
        CPPUNIT_ASSERT_EQUAL(u"bookmark1"_ustr, named->getName());
    }

    {
        auto run = getRun(xPara, 4);
        CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr, getProperty<OUString>(run, "TextPortionType"));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(run, "IsStart"));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(run, "IsCollapsed"));
        auto named = getProperty<uno::Reference<container::XNamed>>(run, "Bookmark");
        CPPUNIT_ASSERT_EQUAL(u"bookmark2"_ustr, named->getName());
    }

    {
        auto run = getRun(xPara, 5);
        CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr, getProperty<OUString>(run, "TextPortionType"));
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(run, "IsStart"));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(run, "IsCollapsed"));
        auto named = getProperty<uno::Reference<container::XNamed>>(run, "Bookmark");
        CPPUNIT_ASSERT_EQUAL(u"bookmark3"_ustr, named->getName());
    }

    {
        auto run = getRun(xPara, 6);
        CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(run, "TextPortionType"));
        CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, run->getString());
    }

    {
        auto run = getRun(xPara, 7);
        CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr, getProperty<OUString>(run, "TextPortionType"));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(run, "IsStart"));
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(run, "IsCollapsed"));
        auto named = getProperty<uno::Reference<container::XNamed>>(run, "Bookmark");
        CPPUNIT_ASSERT_EQUAL(u"bookmark3"_ustr, named->getName());
    }

    // Test that the markup stays at save-and-reload
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    assertXPathNodeName(pXmlDoc, "//office:body/office:text/text:p/*[1]"_ostr,
                        "bookmark-start"_ostr);
    assertXPath(pXmlDoc, "//office:body/office:text/text:p/*[1]"_ostr, "name"_ostr,
                u"bookmark1"_ustr);

    // Without the fix in place, this would fail with
    // - Expected: bookmark-end
    // - Actual  : bookmark-start
    // - In XPath '//office:body/office:text/text:p/*[2]' name of node is incorrect
    assertXPathNodeName(pXmlDoc, "//office:body/office:text/text:p/*[2]"_ostr, "bookmark-end"_ostr);
    assertXPath(pXmlDoc, "//office:body/office:text/text:p/*[2]"_ostr, "name"_ostr,
                u"bookmark1"_ustr);

    assertXPathNodeName(pXmlDoc, "//office:body/office:text/text:p/*[3]"_ostr, "bookmark"_ostr);
    assertXPath(pXmlDoc, "//office:body/office:text/text:p/*[3]"_ostr, "name"_ostr,
                u"bookmark2"_ustr);

    assertXPathNodeName(pXmlDoc, "//office:body/office:text/text:p/*[4]"_ostr,
                        "bookmark-start"_ostr);
    assertXPath(pXmlDoc, "//office:body/office:text/text:p/*[4]"_ostr, "name"_ostr,
                u"bookmark3"_ustr);

    assertXPathNodeName(pXmlDoc, "//office:body/office:text/text:p/*[5]"_ostr, "bookmark-end"_ostr);
    assertXPath(pXmlDoc, "//office:body/office:text/text:p/*[5]"_ostr, "name"_ostr,
                u"bookmark3"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160700)
{
    // Given a document with an empty numbered paragraph, and a cross-reference to it
    loadAndReload("tdf160700.odt");

    // Refresh fields and ensure cross-reference to numbered para is okay
    auto xTextFieldsSupplier(mxComponent.queryThrow<text::XTextFieldsSupplier>());
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());

    xFieldsAccess.queryThrow<util::XRefreshable>()->refresh();

    auto xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    auto xTextField(xFields->nextElement().queryThrow<text::XTextField>());
    // Save must not create markup with text:bookmark-end element before text:bookmark-start
    // Without the fix, this would fail with
    // - Expected: 1
    // - Actual  : Error: Reference source not found
    // i.e., the bookmark wasn't imported, and the field had no proper source
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xTextField->getPresentation(false));

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Check that we export the bookmark in the empty paragraph as a single text:bookmark
    // element. Another valid markup is text:bookmark-start followed by text:bookmark-end
    // (in that order). The problem was, that text:bookmark-end was before text:bookmark-start.
    assertXPathChildren(pXmlDoc, "//office:text/text:list/text:list-item/text:p"_ostr, 1);
    assertXPath(pXmlDoc, "//office:text/text:list/text:list-item/text:p/text:bookmark"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160253_ordinary_numbering)
{
    // Given a document with a list, and an out-of-the-list paragraph in the middle, having an
    // endnote, which has a paragraph in another list.
    // Before the fix, this already failed with
    //   Error: "list2916587379" is referenced by an IDREF, but not defined.
    loadAndReload("tdf160253_ordinary_numbering.fodt");

    // Make sure that the fourth paragraph has correct number - it was "1." before the fix
    CPPUNIT_ASSERT_EQUAL(u"3."_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));

    // Make sure that we emit an identifier for the first list, and refer to it in the continuation
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // This failed before the fix, because 'xml:id' attribute wasn't emitted
    OUString firstListId
        = getXPath(pXmlDoc, "//office:body/office:text/text:list[1]"_ostr, "id"_ostr);
    CPPUNIT_ASSERT(!firstListId.isEmpty());
    assertXPath(pXmlDoc, "//office:body/office:text/text:list[2]"_ostr, "continue-list"_ostr,
                firstListId);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160253_outline_numbering)
{
    // Given a document with an outline (chapter) numbering, and a paragraph in the middle, having
    // an endnote, which has a paragraph in a list.
    // Before the fix, this already failed with
    //   Error: "list2916587379" is referenced by an IDREF, but not defined.
    loadAndReload("tdf160253_outline_numbering.fodt");

    // Make sure that the third paragraph has correct number - it was "1" before the fix
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));

    // The difference with the ordinary numbering is that for outline numbering, the list element
    // isn't really necessary. It is a TODO to fix the output, and not export the list.
    // xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // assertXPath(pXmlDoc, "//office:body/office:text/text:list"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTableInFrameAnchoredToPage)
{
    // Given a table in a frame anchored to a page:
    // it must not assert on export because of missing format for an exported table
    loadAndReload("table_in_frame_to_page.fodt");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    auto AutoStyleUsedIn = [this, &pXmlDoc](const OString& path, const OString& attr) -> OString
    {
        const OUString styleName = getXPath(pXmlDoc, path, attr);
        return "//office:automatic-styles/style:style[@style:name='" + styleName.toUtf8() + "']";
    };
    constexpr OString xPathTextBox = "//office:body/office:text/draw:frame/draw:text-box"_ostr;

    // Check also, that autostyles defined inside that frame are stored correctly. If not, then
    // these paragraphs would refer to styles in <office::styles>, not in <office:automatic-styles>,
    // without the 'italic' and 'bold' attributes.
    OString P = AutoStyleUsedIn(xPathTextBox + "/text:p", "style-name"_ostr);
    assertXPath(pXmlDoc, P + "/style:text-properties", "font-weight"_ostr, u"bold"_ustr);

    P = AutoStyleUsedIn(xPathTextBox + "/table:table/table:table-row[1]/table:table-cell[1]/text:p",
                        "style-name"_ostr);
    assertXPath(pXmlDoc, P + "/style:text-properties", "font-style"_ostr, u"italic"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDeletedTableAutostylesExport)
{
    // Given a document with deleted table:
    // it must not assert on export because of missing format for an exported table
    loadAndReload("deleted_table.fodt");
}

CPPUNIT_TEST_FIXTURE(Test, testMidnightRedlineDatetime)
{
    // Given a document with a tracked change with a midnight datetime:
    // make sure that it succeeds export and import validation. Before the fix, this failed:
    // - Error: "2001-01-01" does not satisfy the "dateTime" type
    // because "2001-01-01T00:00:00" became "2001-01-01" on roundtrip.
    loadAndReload("midnight_redline.fodt");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPathContent(pXmlDoc,
                       "//office:body/office:text/text:tracked-changes/text:changed-region/"
                       "text:deletion/office:change-info/dc:date"_ostr,
                       u"2001-01-01T00:00:00"_ustr);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
