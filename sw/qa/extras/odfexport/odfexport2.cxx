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
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/text/ColumnSeparatorStyle.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <unotools/localedatawrapper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unoprnms.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

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
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[1]", "font-style", "normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[1]", "font-weight", "normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[2]", "font-style", "normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[2]", "font-weight", "bold");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[3]", "font-style", "italic");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[3]", "font-weight", "normal");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[4]", "font-style", "italic");
    assertXPath(pXmlDoc, "//style:font-face[@style:name='Liberation Serif']/svg:font-face-src/svg:font-face-uri[4]", "font-weight", "bold");
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

DECLARE_ODFEXPORT_TEST(testReferenceLanguage, "referencelanguage.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // Test loext:reference-language attribute of reference fields
    // (used from LibreOffice 6.1, and proposed for next ODF)
    OUString const aFieldTexts[] = { "A 2", "Az Isten", "Az 50-esek",
        "A 2018-asok", "Az egyebek", "A fejezetek",
        u"Az „Őseinket...”", "a 2",
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
    OUString const aFieldTexts[] = { "1st", "Erste", "Eins",  "1.", "Premier", "Un", u"1ᵉʳ", "First", "One" };
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
    assertXPathContent(pXmlDoc, "//config:config-item[@config:name='VisibleAreaWidth']", "4516");
    assertXPathContent(pXmlDoc, "//config:config-item[@config:name='VisibleAreaHeight']", "903");

    xmlDocUniquePtr pXmlDoc2 = parseExport("Object 2/settings.xml");
    assertXPathContent(pXmlDoc2, "//config:config-item[@config:name='VisibleAreaWidth']", "4516");
    assertXPathContent(pXmlDoc2, "//config:config-item[@config:name='VisibleAreaHeight']", "1355");
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

    assertXPath(pXmlDoc, "/office:document-content/office:font-face-decls/style:style", 0);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:family='table']", 1);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:family='table-column']", 2);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:family='paragraph']", 1);
}

CPPUNIT_TEST_FIXTURE(Test, tdf135942)
{
    loadAndReload("nestedTableInFooter.odt");
    // All table autostyles should be collected, including nested, and must not crash.

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");

    assertXPath(pXmlDoc, "/office:document-styles/office:automatic-styles/style:style[@style:family='table']", 2);
}

CPPUNIT_TEST_FIXTURE(Test, tdf150927)
{
    // Similar to tdf135942

    loadAndReload("table-in-frame-in-table-in-header-base.odt");
    // All table autostyles should be collected, including nested, and must not crash.

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");

    assertXPath(pXmlDoc, "/office:document-styles/office:automatic-styles/style:style[@style:family='table']", 2);
}

CPPUNIT_TEST_FIXTURE(Test, tdf151100)
{
    // Similar to tdf135942

    loadAndReload("tdf151100.docx");
    // All table autostyles should be collected, including nested, and must not crash.

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");

    assertXPath(pXmlDoc, "/office:document-styles/office:automatic-styles/style:style[@style:family='table']", 1);
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
    sal_Int32 nTabStop = parseDump("//body/txt[4]/SwParaPortion/SwLineLayout/child::*[3]", "width").toInt32();
    // Without the fix, the text was unseen, with a tabstop width of 64057. It should be 3057
    CPPUNIT_ASSERT(nTabStop < 4000);
    CPPUNIT_ASSERT(3000 < nTabStop);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Pečiatka zamestnávateľa"), parseDump("//body/txt[4]/SwParaPortion/SwLineLayout/child::*[4]", "portion"));

    // tdf#149547: __XXX___invalid CharacterStyles should not be imported/exported
    CPPUNIT_ASSERT(!getStyles("CharacterStyles")->hasByName("__XXX___invalid"));
}

DECLARE_ODFEXPORT_TEST(testTdf104254_noHeaderWrapping, "tdf104254_noHeaderWrapping.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nParaHeight = getXPath(pXmlDoc, "//header/txt[1]/infos/bounds", "height").toInt32();
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

    sal_Int32 nParaHeight = getXPath(pXmlDoc, "//page[1]/header/txt[1]/infos/bounds", "height").toInt32();
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
    for (beans::PropertyValue const& rProp : std::as_const(aProps))
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
        "text:list-level-style-number[@text:level='1']", "num-list-format", ">%1%<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='2']", "num-list-format", ">>%1%/%2%<<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='3']", "num-list-format", ">>%1%/%2%/%3%<<");

    // But for compatibility there are still prefix/suffix
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='1']", "num-prefix", ">");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='1']", "num-suffix", "<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='2']", "num-prefix", ">>");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='2']", "num-suffix", "<<");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='3']", "num-prefix", ">>");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/text:list-style[@style:name='WWNum1']/"
        "text:list-level-style-number[@text:level='3']", "num-suffix", "<<");
}

DECLARE_ODFEXPORT_TEST(testShapeWithHyperlink, "shape-with-hyperlink.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (isExported())
    {
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        // Check how conversion from prefix/suffix to list format did work
        assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p/draw:a",
                    "href", "http://shape.com/");
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
            "text:list-level-style-number[@text:level='1']", "num-list-format", ">%1%<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='2']", "num-list-format", ">>%1%.%2%<<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='3']", "num-list-format", ">>%1%.%2%.%3%<<");

        // But for compatibility there are still prefix/suffix as they were before
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='1']", "num-prefix", ">");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='1']", "num-suffix", "<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='2']", "num-prefix", ">>");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='2']", "num-suffix", "<<");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='3']", "num-prefix", ">>");
        assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/text:list-style[@style:name='L1']/"
            "text:list-level-style-number[@text:level='3']", "num-suffix", "<<");
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
        "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/style:tab-stop",
        "char", cDecimal);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:name='P2']/style:paragraph-properties/style:tab-stops/style:tab-stop",
        "char", cDecimal);
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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/prtBounds", "left", "2268");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/prtBounds", "right", "11339");
    // the list style name of the list is the same as the list style name of the
    // paragraph, but in any case the margins of the paragraph take precedence
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/infos/prtBounds", "left", "2268");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/infos/prtBounds", "right", "11339");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[16]/infos/prtBounds", "left", "357");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[16]/infos/prtBounds", "right", "11339");
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/prtBounds", "left", "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/prtBounds", "left", "1134");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/prtBounds", "left", "1134");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/infos/prtBounds", "left", "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/infos/prtBounds", "left", "0");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/infos/prtBounds", "left", "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/infos/prtBounds", "right", "9359");

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/infos/prtBounds", "left", "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/infos/prtBounds", "left", "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/infos/prtBounds", "left", "1701");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/infos/prtBounds", "left", "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/infos/prtBounds", "left", "-567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/infos/prtBounds", "right", "9359");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/infos/prtBounds", "left", "567");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/infos/prtBounds", "right", "9359");
    }

    // now check the positions where text is actually painted -
    // wonder how fragile this is...
    // FIXME some platform difference, 1st one is 2306 on Linux, 3087 on WNT ?
    // some Mac has 3110
#if !defined(WNT) && !defined(MACOSX)
    {
        SwDocShell *const pShell(dynamic_cast<SwXTextDocument&>(*mxComponent).GetDocShell());
        std::shared_ptr<GDIMetaFile> pMetaFile = pShell->GetPreviewMetaFile();
        MetafileXmlDump aDumper;
        xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *pMetaFile);

        // 1: inherited from paragraph style and overridden by list
        // bullet char is extra

        assertXPath(pXmlDoc, "//textarray[1]", "x", "2306");
        // text is after a tab from list - haven't checked if that is correct?
        assertXPath(pXmlDoc, "//textarray[2]", "x", "2873");
        // second line
        assertXPath(pXmlDoc, "//textarray[3]", "x", "2873");
        // 2: as 1 + paragraph sets firstline
        assertXPath(pXmlDoc, "//textarray[4]", "x", "3440");
        assertXPath(pXmlDoc, "//textarray[5]", "x", "3593");
        assertXPath(pXmlDoc, "//textarray[6]", "x", "2873");
        // 3: as 1 + paragraph sets textleft
        assertXPath(pXmlDoc, "//textarray[7]", "x", "2873");
        assertXPath(pXmlDoc, "//textarray[8]", "x", "3440");
        assertXPath(pXmlDoc, "//textarray[9]", "x", "3440");
        // 4: as 1 + paragraph sets firstline, textleft
        assertXPath(pXmlDoc, "//textarray[10]", "x", "2306");
        assertXPath(pXmlDoc, "//textarray[11]", "x", "3440");
        assertXPath(pXmlDoc, "//textarray[12]", "x", "3440");
        // 5: as 1 + paragraph sets firstline
        assertXPath(pXmlDoc, "//textarray[13]", "x", "1739");
        assertXPath(pXmlDoc, "//textarray[14]", "x", "2873");
        assertXPath(pXmlDoc, "//textarray[15]", "x", "2873");
        // 6: as 1
        assertXPath(pXmlDoc, "//textarray[16]", "x", "2306");
        assertXPath(pXmlDoc, "//textarray[17]", "x", "2873");

        // 8: inherited from paragraph style and overridden by list
        assertXPath(pXmlDoc, "//textarray[18]", "x", "2873");
        assertXPath(pXmlDoc, "//textarray[19]", "x", "3746");
        assertXPath(pXmlDoc, "//textarray[20]", "x", "2306");
        // 9: as 8 + paragraph sets firstline
        assertXPath(pXmlDoc, "//textarray[21]", "x", "3440");
        assertXPath(pXmlDoc, "//textarray[22]", "x", "3746");
        assertXPath(pXmlDoc, "//textarray[23]", "x", "2306");
        // 10: as 8 + paragraph sets textleft
        assertXPath(pXmlDoc, "//textarray[24]", "x", "4007");
        assertXPath(pXmlDoc, "//textarray[25]", "x", "4880");
        assertXPath(pXmlDoc, "//textarray[26]", "x", "3440");
        // 11: as 8 + paragraph sets firstline, textleft
        assertXPath(pXmlDoc, "//textarray[27]", "x", "2306");
        assertXPath(pXmlDoc, "//textarray[28]", "x", "3440");
        assertXPath(pXmlDoc, "//textarray[29]", "x", "3440");
        // 12: as 8 + paragraph sets firstline
        assertXPath(pXmlDoc, "//textarray[30]", "x", "1172");
        assertXPath(pXmlDoc, "//textarray[31]", "x", "1739");
        assertXPath(pXmlDoc, "//textarray[32]", "x", "2306");
        // 13: as 8
        assertXPath(pXmlDoc, "//textarray[33]", "x", "2873");
        assertXPath(pXmlDoc, "//textarray[34]", "x", "3746");
    }
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testParagraphMarkerMarkupRoundtrip)
{
    loadAndReload("ParagraphMarkerMarkup.fodt");
    // Test that the markup stays at save-and-reload
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p", "marker-style-name", "T2");
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name='T2']/style:text-properties", "font-size", "9pt");
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name='T2']/style:text-properties", "color", "#ff0000");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
