/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <rtl/ustring.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

#include <swmodeltestbase.hxx>
#include <bordertest.hxx>

#define TWIP_TO_MM100(TWIP) ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

class Test : public SwModelTestBase
{
public:
    void testFdo45553();
    void testN192129();
    void testFdo45543();
    void testN695479();
    void testFdo42465();
    void testFdo45187();
    void testFdo46662();
    void testN750757();
    void testFdo45563();
    void testFdo43965();
    void testN751020();
    void testFdo47326();
    void testFdo47036();
    void testFdo46955();
    void testFdo45394();
    void testFdo48104();
    void testFdo47107();
    void testFdo45182();
    void testFdo44176();
    void testFdo39053();
    void testFdo48356();
    void testFdo48023();
    void testFdo48876();
    void testFdo48193();
    void testFdo44211();
    void testFdo48037();
    void testFdo47764();
    void testFdo38786();
    void testN757651();
    void testFdo49501();
    void testFdo49271();
    void testFdo49692();
    void testFdo45190();
    void testFdo50539();
    void testFdo50665();
    void testFdo49659();
    void testFdo46966();
    void testFdo52066();
    void testFdo48033_53594();
    void testFdo36089();
    void testFdo49892();
    void testFdo48446();
    void testFdo47495();
    void testAllGapsWord();
    void testFdo52052();
    void testInk();
    void testFdo52389();
    void testFdo49655_62805();
    void testFdo52475();
    void testFdo55493();
    void testCopyPastePageStyle();
    void testCopyPasteFootnote();
    void testShptxtPard();
    void testDoDhgt();
    void testDplinehollow();
    void testLeftmarginDefault();
    void testDppolyline();
    void testFdo56512();
    void testFdo52989();
    void testFdo48442();
    void testFdo55525();
    void testFdo57708();
    void testFdo54473();
    void testFdo49934();
    void testFdo57886();
    void testFdo58076();
    void testFdo57678();
    void testFdo45183();
    void testFdo54612();
    void testFdo58933();
    void testFdo44053();
    void testFdo48440();
    void testFdo58646line();
    void testFdo58646();
    void testFdo59419();
    void testFdo58076_2();
    void testFdo59953();
    void testFdo59638();
    void testFdo60722();
    void testFdo61909();
    void testFdo62288();
    void testFdo37716();
    void testFdo51916();
    void testFdo61193();
    void testFdo63023();
    void testFdo42109();
    void testFdo62977();
    void testN818997();
    void testFdo64671();
    void testPageBackground();
    void testFdo62044();
    void testPoshPosv();
    void testN825305();
    void testParaBottomMargin();
    void testN823655();
    void testFdo66040();
    void testN823675();
    void testFdo47802();
    void testFdo39001();
    void testGroupshape();
    void testFdo66565();
    void testFdo54900();
    void testFdo64637();
    void testN820504();
    void testFdo67365();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(run);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void run();
    /// Copy&paste helper.
    void paste(OUString aFilename, uno::Reference<text::XTextRange> xTextRange = uno::Reference<text::XTextRange>())
    {
        uno::Reference<document::XFilter> xFilter(m_xSFactory->createInstance("com.sun.star.comp.Writer.RtfFilter"), uno::UNO_QUERY_THROW);
        uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
        xImporter->setTargetDocument(mxComponent);
        uno::Sequence<beans::PropertyValue> aDescriptor(xTextRange.is() ? 3 : 2);
        aDescriptor[0].Name = "InputStream";
        SvStream* pStream = utl::UcbStreamHelper::CreateStream(getURLFromSrc("/sw/qa/extras/rtfimport/data/") + aFilename, STREAM_WRITE);
        uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
        aDescriptor[0].Value <<= xStream;
        aDescriptor[1].Name = "IsNewDoc";
        aDescriptor[1].Value <<= sal_False;
        if (xTextRange.is())
        {
            aDescriptor[2].Name = "TextInsertModeRange";
            aDescriptor[2].Value <<= xTextRange;
        }
        xFilter->filter(aDescriptor);
    }

};

void Test::run()
{
    MethodEntry<Test> aMethods[] = {
        {"fdo45553.rtf", &Test::testFdo45553},
        {"n192129.rtf", &Test::testN192129},
        {"fdo45543.rtf", &Test::testFdo45543},
        {"n695479.rtf", &Test::testN695479},
        {"fdo42465.rtf", &Test::testFdo42465},
        {"fdo45187.rtf", &Test::testFdo45187},
        {"fdo46662.rtf", &Test::testFdo46662},
        {"n750757.rtf", &Test::testN750757},
        {"fdo45563.rtf", &Test::testFdo45563},
        {"fdo43965.rtf", &Test::testFdo43965},
        {"n751020.rtf", &Test::testN751020},
        {"fdo47326.rtf", &Test::testFdo47326},
        {"fdo47036.rtf", &Test::testFdo47036},
        {"fdo46955.rtf", &Test::testFdo46955},
        {"fdo45394.rtf", &Test::testFdo45394},
        {"fdo48104.rtf", &Test::testFdo48104},
        {"fdo47107.rtf", &Test::testFdo47107},
        {"fdo45182.rtf", &Test::testFdo45182},
        {"fdo44176.rtf", &Test::testFdo44176},
        {"fdo39053.rtf", &Test::testFdo39053},
        {"fdo48356.rtf", &Test::testFdo48356},
        {"fdo48023.rtf", &Test::testFdo48023},
        {"fdo48876.rtf", &Test::testFdo48876},
        {"fdo48193.rtf", &Test::testFdo48193},
        {"fdo44211.rtf", &Test::testFdo44211},
        {"fdo48037.rtf", &Test::testFdo48037},
        {"fdo47764.rtf", &Test::testFdo47764},
        {"fdo38786.rtf", &Test::testFdo38786},
        {"n757651.rtf", &Test::testN757651},
        {"fdo49501.rtf", &Test::testFdo49501},
        {"fdo49271.rtf", &Test::testFdo49271},
        {"fdo49692.rtf", &Test::testFdo49692},
        {"fdo45190.rtf", &Test::testFdo45190},
        {"fdo50539.rtf", &Test::testFdo50539},
        {"fdo50665.rtf", &Test::testFdo50665},
        {"fdo49659.rtf", &Test::testFdo49659},
        {"fdo46966.rtf", &Test::testFdo46966},
        {"fdo52066.rtf", &Test::testFdo52066},
        {"fdo48033.rtf", &Test::testFdo48033_53594},
        {"fdo53594.rtf", &Test::testFdo48033_53594},
        {"fdo36089.rtf", &Test::testFdo36089},
        {"fdo49892.rtf", &Test::testFdo49892},
        {"fdo48446.rtf", &Test::testFdo48446},
        {"fdo47495.rtf", &Test::testFdo47495},
        {"all_gaps_word.rtf", &Test::testAllGapsWord},
        {"fdo52052.rtf", &Test::testFdo52052},
        {"ink.rtf", &Test::testInk},
        {"fdo52389.rtf", &Test::testFdo52389},
        {"fdo49655.rtf", &Test::testFdo49655_62805},
        {"fdo62805.rtf", &Test::testFdo49655_62805},
        {"fdo52475.rtf", &Test::testFdo52475},
        {"fdo55493.rtf", &Test::testFdo55493},
        {"copypaste-pagestyle.rtf", &Test::testCopyPastePageStyle},
        {"copypaste-footnote.rtf", &Test::testCopyPasteFootnote},
        {"shptxt-pard.rtf", &Test::testShptxtPard},
        {"do-dhgt.rtf", &Test::testDoDhgt},
        {"dplinehollow.rtf", &Test::testDplinehollow},
        {"leftmargin-default.rtf", &Test::testLeftmarginDefault},
        {"dppolyline.rtf", &Test::testDppolyline},
        {"fdo56512.rtf", &Test::testFdo56512},
        {"fdo52989.rtf", &Test::testFdo52989},
        {"fdo48442.rtf", &Test::testFdo48442},
        {"fdo55525.rtf", &Test::testFdo55525},
        {"fdo57708.rtf", &Test::testFdo57708},
        {"fdo54473.rtf", &Test::testFdo54473},
        {"fdo49934.rtf", &Test::testFdo49934},
        {"fdo57886.rtf", &Test::testFdo57886},
        {"fdo58076.rtf", &Test::testFdo58076},
        {"fdo57678.rtf", &Test::testFdo57678},
        {"fdo45183.rtf", &Test::testFdo45183},
        {"fdo54612.rtf", &Test::testFdo54612},
        {"fdo58933.rtf", &Test::testFdo58933},
        {"fdo44053.rtf", &Test::testFdo44053},
        {"fdo48440.rtf", &Test::testFdo48440},
        {"fdo58646line.rtf", &Test::testFdo58646line},
        {"fdo58646.rtf", &Test::testFdo58646},
        {"fdo59419.rtf", &Test::testFdo59419},
        {"fdo58076-2.rtf", &Test::testFdo58076_2},
        {"fdo59953.rtf", &Test::testFdo59953},
        {"fdo59638.rtf", &Test::testFdo59638},
        {"fdo60722.rtf", &Test::testFdo60722},
        {"fdo61909.rtf", &Test::testFdo61909},
        {"fdo62288.rtf", &Test::testFdo62288},
        {"fdo37716.rtf", &Test::testFdo37716},
        {"fdo51916.rtf", &Test::testFdo51916},
        {"hello.rtf", &Test::testFdo61193},
        {"fdo63023.rtf", &Test::testFdo63023},
        {"fdo42109.rtf", &Test::testFdo42109},
        {"fdo62977.rtf", &Test::testFdo62977},
        {"n818997.rtf", &Test::testN818997},
        {"fdo64671.rtf", &Test::testFdo64671},
        {"page-background.rtf", &Test::testPageBackground},
        {"fdo62044.rtf", &Test::testFdo62044},
        {"posh-posv.rtf", &Test::testPoshPosv},
        {"n825305.rtf", &Test::testN825305},
        {"para-bottom-margin.rtf", &Test::testParaBottomMargin},
        {"n823655.rtf", &Test::testN823655},
        {"fdo66040.rtf", &Test::testFdo66040},
        {"n823675.rtf", &Test::testN823675},
        {"fdo47802.rtf", &Test::testFdo47802},
        {"fdo39001.rtf", &Test::testFdo39001},
        {"groupshape.rtf", &Test::testGroupshape},
        {"groupshape-notext.rtf", &Test::testGroupshape},
        {"fdo66565.rtf", &Test::testFdo66565},
        {"fdo54900.rtf", &Test::testFdo54900},
        {"fdo64637.rtf", &Test::testFdo64637},
        {"n820504.rtf", &Test::testN820504},
        {"fdo67365.rtf", &Test::testFdo67365},
    };
    header();
    for (unsigned int i = 0; i < SAL_N_ELEMENTS(aMethods); ++i)
    {
        MethodEntry<Test>& rEntry = aMethods[i];
        AllSettings aSavedSettings(Application::GetSettings());
        if (OString(rEntry.pName) == "fdo48023.rtf")
        {
            AllSettings aSettings(aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("ru"));
            Application::SetSettings(aSettings);
        }
        else if (OString(rEntry.pName) == "fdo44211.rtf")
        {
            AllSettings aSettings(aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("lt"));
            Application::SetSettings(aSettings);
        }
        load("/sw/qa/extras/rtfimport/data/", rEntry.pName);
        if (OString(rEntry.pName) == "fdo48023.rtf" || OString(rEntry.pName) == "fdo44211.rtf")
            Application::SetSettings(aSavedSettings);
        (this->*rEntry.pMethod)();
        finish();
    }
}

void Test::testFdo45553()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRange(xRangeEnum->nextElement(), uno::UNO_QUERY);
            OUString aStr = xRange->getString();
            if ( aStr == "space-before" )
                CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(120)), getProperty<sal_Int32>(xRange, "ParaTopMargin"));
            else if ( aStr == "space-after" )
                CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(240)), getProperty<sal_Int32>(xRange, "ParaBottomMargin"));
        }
    }
}

void Test::testN192129()
{
    // We expect that the result will be 16x16px.
    Size aExpectedSize(16, 16);
    MapMode aMap(MAP_100TH_MM);
    aExpectedSize = Application::GetDefaultDevice()->PixelToLogic( aExpectedSize, aMap );

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    awt::Size aActualSize(xShape->getSize());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Width()), aActualSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Height()), aActualSize.Height);
}

void Test::testFdo45543()
{
    CPPUNIT_ASSERT_EQUAL(5, getLength());
}

void Test::testN695479()
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    // Negative ABSH should mean fixed size.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, getProperty<sal_Int16>(xPropertySet, "SizeType"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(300)), getProperty<sal_Int32>(xPropertySet, "Height"));

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    bool bFrameFound = false, bDrawFound = false;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xDraws->getByIndex(i), uno::UNO_QUERY);
        if (xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
        {
            // Both frames should be anchored to the first paragraph.
            bFrameFound = true;
            uno::Reference<text::XTextContent> xTextContent(xServiceInfo, uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
            uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(OUString("plain"), xText->getString());

            if (i == 0)
                // Additonally, the frist frame should have double border at the bottom.
                CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::DOUBLE, getProperty<table::BorderLine2>(xPropertySet, "BottomBorder").LineStyle);
        }
        else if (xServiceInfo->supportsService("com.sun.star.drawing.LineShape"))
        {
            // The older "drawing objects" syntax should be recognized.
            bDrawFound = true;
            xPropertySet.set(xServiceInfo, uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA, getProperty<sal_Int16>(xPropertySet, "HoriOrientRelation"));
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xPropertySet, "VertOrientRelation"));
        }
    }
    CPPUNIT_ASSERT(bFrameFound);
    CPPUNIT_ASSERT(bDrawFound);
}

void Test::testFdo42465()
{
    CPPUNIT_ASSERT_EQUAL(3, getLength());
}

void Test::testFdo45187()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // There should be two shapes.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());

    // They should be anchored to different paragraphs.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xAnchor0 = uno::Reference<text::XTextContent>(xDraws->getByIndex(0), uno::UNO_QUERY)->getAnchor();
    uno::Reference<text::XTextRange> xAnchor1 = uno::Reference<text::XTextContent>(xDraws->getByIndex(1), uno::UNO_QUERY)->getAnchor();
    // Was 0 ("starts at the same position"), should be 1 ("starts before")
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xTextRangeCompare->compareRegionStarts(xAnchor0, xAnchor1));
}

void Test::testFdo46662()
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum3"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(1) >>= aProps; // 2nd level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if ( rProp.Name == "ParentNumbering" )
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2), rProp.Value.get<sal_Int16>());
        else if ( rProp.Name == "Suffix" )
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rProp.Value.get<OUString>().getLength());
    }
}

void Test::testN750757()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    CPPUNIT_ASSERT_EQUAL(sal_Bool(false), getProperty<sal_Bool>(xParaEnum->nextElement(), "ParaContextMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Bool(true), getProperty<sal_Bool>(xParaEnum->nextElement(), "ParaContextMargin"));
}

void Test::testFdo45563()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    int i = 0;
    while (xParaEnum->hasMoreElements())
    {
        xParaEnum->nextElement();
        i++;
    }
    CPPUNIT_ASSERT_EQUAL(4, i);
}

void Test::testFdo43965()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // First paragraph: the parameter of \up was ignored
    uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xRangeEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(36), getProperty<sal_Int32>(xPropertySet, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(xPropertySet, "CharEscapementHeight"));

    // Second paragraph: Word vs Writer border default problem
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(26), getProperty<table::BorderLine2>(xParaEnum->nextElement(), "TopBorder").LineWidth);

    // Finally, make sure that we have two pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

void Test::testN751020()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(200)), getProperty<sal_Int32>(xParaEnum->nextElement(), "ParaBottomMargin"));
}

void Test::testFdo47326()
{
    // This was 15 only, as \super buffered text, then the contents of it got lost.
    CPPUNIT_ASSERT_EQUAL(19, getLength());
}

void Test::testFdo47036()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    int nAtCharacter = 0;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        if (getProperty<text::TextContentAnchorType>(xDraws->getByIndex(i), "AnchorType") == text::TextContentAnchorType_AT_CHARACTER)
            nAtCharacter++;
    }
    // The image at the document start was ignored.
    CPPUNIT_ASSERT_EQUAL(1, nAtCharacter);

    // There should be 2 textboxes, not 4
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

void Test::testFdo46955()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
            CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, getProperty<sal_Int16>(xRangeEnum->nextElement(), "CharCaseMap"));
    }
}

void Test::testFdo45394()
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "HeaderText");
    OUString aActual = xHeaderText->getString();
    // Encoding in the header was wrong.
    OUString aExpected("ПК РИК", 11, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

void Test::testFdo48104()
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

void Test::testFdo47107()
{
    uno::Reference<container::XNameAccess> xNumberingStyles(getStyles("NumberingStyles"));
    // Make sure numbered and bullet legacy syntax is recognized, this used to throw a NoSuchElementException
    xNumberingStyles->getByName("WWNum1");
    xNumberingStyles->getByName("WWNum2");
}

void Test::testFdo45182()
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // Encoding in the footnote was wrong.
    OUString aExpected("živností", 10, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

void Test::testFdo44176()
{
    uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xFirstPage(xPageStyles->getByName("First Page"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDefault(xPageStyles->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    sal_Int32 nFirstTop = 0, nDefaultTop = 0, nDefaultHeader = 0;
    xFirstPage->getPropertyValue("TopMargin") >>= nFirstTop;
    xDefault->getPropertyValue("TopMargin") >>= nDefaultTop;
    xDefault->getPropertyValue("HeaderHeight") >>= nDefaultHeader;
    CPPUNIT_ASSERT_EQUAL(nFirstTop, nDefaultTop + nDefaultHeader);
}

void Test::testFdo39053()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    int nAsCharacter = 0;
    for (int i = 0; i < xDraws->getCount(); ++i)
        if (getProperty<text::TextContentAnchorType>(xDraws->getByIndex(i), "AnchorType") == text::TextContentAnchorType_AS_CHARACTER)
            nAsCharacter++;
    // The image in binary format was ignored.
    CPPUNIT_ASSERT_EQUAL(1, nAsCharacter);
}

void Test::testFdo48356()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    int i = 0;
    while (xParaEnum->hasMoreElements())
    {
        xParaEnum->nextElement();
        i++;
    }
    // The document used to be imported as two paragraphs.
    CPPUNIT_ASSERT_EQUAL(1, i);
}

void Test::testFdo48023()
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    // Implicit encoding detection based on locale was missing
    OUString aExpected("Программист", 22, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

void Test::testFdo48876()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::MINIMUM, getProperty<style::LineSpacing>(xParaEnum->nextElement(), "ParaLineSpacing").Mode);
}

void Test::testFdo48193()
{
    CPPUNIT_ASSERT_EQUAL(7, getLength());
}

void Test::testFdo44211()
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    OUString aExpected("ąčę", 6, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

void Test::testFdo48037()
{
    uno::Reference<util::XNumberFormatsSupplier> xNumberSupplier(mxComponent, uno::UNO_QUERY_THROW);
    lang::Locale aUSLocale, aFRLocale;
    aUSLocale.Language = "en";
    aFRLocale.Language = "fr";
    sal_Int32 nExpected = xNumberSupplier->getNumberFormats()->addNewConverted("d MMMM yyyy", aUSLocale, aFRLocale);

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xPropertySet(xFields->nextElement(), uno::UNO_QUERY);
    sal_Int32 nActual = 0;
    xPropertySet->getPropertyValue("NumberFormat") >>= nActual;

    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

void Test::testFdo47764()
{
    // \cbpat with zero argument should mean the auto (-1) color, not a default color (black)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(getParagraph(1), "ParaBackColor"));
}

void Test::testFdo38786()
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    // \chpgn was ignored, so exception was thrown
    xFields->nextElement();
}

void Test::testN757651()
{
    // The bug was that due to buggy layout the text expanded to two pages.
    if (Application::GetDefaultDevice()->IsFontAvailable(OUString("Times New Roman")))
        CPPUNIT_ASSERT_EQUAL(1, getPages());
}

void Test::testFdo49501()
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_True, getProperty<sal_Bool>(xStyle, "IsLandscape"));
    sal_Int32 nExpected(TWIP_TO_MM100(567));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "RightMargin"));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "BottomMargin"));
}

void Test::testFdo49271()
{
    CPPUNIT_ASSERT_EQUAL(25.f, getProperty<float>(getParagraph(2), "CharHeight"));
}

void Test::testFdo49692()
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rProp.Value.get<OUString>().getLength());
    }
}

void Test::testFdo45190()
{
    // inherited \fi should be reset
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));

    // but direct one not
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(-100)), getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
}

void Test::testFdo50539()
{
    // \chcbpat with zero argument should mean the auto (-1) color, not a default color (black)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(getRun(getParagraph(1), 1), "CharBackColor"));
}

void Test::testFdo50665()
{
    // Access the second run, which is a textfield
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 2), uno::UNO_QUERY);
    // This used to be the default, as character properties were ignored.
    CPPUNIT_ASSERT_EQUAL(OUString("Book Antiqua"), getProperty<OUString>(xRun, "CharFontName"));
}

void Test::testFdo49659()
{
    // Both tables were ignored: 1) was in the header, 2) was ignored due to missing empty par at the end of the doc
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    // The graphic was also empty
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphic(getProperty< uno::Reference<beans::XPropertySet> >(xDraws->getByIndex(0), "Graphic"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL, getProperty<sal_Int8>(xGraphic, "GraphicType"));
}

void Test::testFdo46966()
{
    /*
     * The problem was the top margin was 1440 (1 inch), but it should be 720 (0.5 inch).
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.TopMargin
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(720)), getProperty<sal_Int32>(xPropertySet, "TopMargin"));
}

void Test::testFdo52066()
{
    /*
     * The problem was that the height of the shape was too big.
     *
     * xray ThisComponent.DrawPage(0).Size.Height
     */
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(19)), xShape->getSize().Height);
}

void Test::testFdo48033_53594()
{
    /*
     * The problem was that the picture (48033) or OLE object (53594) was in the first cell,
     * instead of the second one.
     *
     * oTable = ThisComponent.TextTables(0)
     * oParas = oTable.getCellByName("B1").Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * xray oRun.TextPortionType ' Frame, was Text
     */
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"), getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
}

void Test::testFdo36089()
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(-50), getProperty<sal_Int16>(getRun(getParagraph(1), 2), "CharEscapement"));
}

void Test::testFdo49892()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        OUString aDescription = getProperty<OUString>(xDraws->getByIndex(i), "Description");
        if (aDescription == "red")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (aDescription == "green")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (aDescription == "blue")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (aDescription == "rect")
        {
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xDraws->getByIndex(i), "HoriOrientRelation"));
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xDraws->getByIndex(i), "VertOrientRelation"));
        }
    }
}

void Test::testFdo48446()
{
    OUString aExpected("Имя", 6, RTL_TEXTENCODING_UTF8);
    getParagraph(1, aExpected);
}

void Test::testFdo47495()
{
    // Used to have 4 paragraphs, as a result the original bugdoc had 2 pages instead of 1.
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

void Test::testAllGapsWord()
{
    BorderTest borderTest;
    borderTest.testTheBorders(mxComponent);
}

void Test::testFdo52052()
{
    // Make sure the textframe containing the text "third" appears on the 3rd page.
    CPPUNIT_ASSERT_EQUAL(OUString("third"), parseDump("/root/page[3]/body/txt/anchored/fly/txt/text()"));
}

void Test::testInk()
{
    /*
     * The problem was that the second segment had wrong command count and wrap type.
     *
     * oShape = ThisComponent.DrawPage(0)
     * oPathPropVec = oShape.CustomShapeGeometry(1).Value
     * oSegments = oPathPropVec(1).Value
     * msgbox oSegments(1).Count ' was 0x2000 | 10, should be 10
     * msgbox oShape.Surround ' was 2, should be 1
     */
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xDraws->getByIndex(0), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            rProp.Value >>= aPathProps;
    }
    uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Segments")
            rProp.Value >>= aSegments;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), aSegments[1].Count);
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(xDraws->getByIndex(0), "Surround"));
}

void Test::testFdo52389()
{
    // The last '!' character at the end of the document was lost
    CPPUNIT_ASSERT_EQUAL(6, getLength());
}

void Test::testFdo49655_62805()
{
    /*
     * 49655 :
     * The problem was that the table was not imported due to the '  ' string in the middle of the table definition.
     *
     * xray ThisComponent.TextTables.Count 'was 0
     *
     * 62805 :
     * The problem was that the table was not imported due to the absence of \pard after \row.
     * The table was instead in a group (the '}' replace the \pard).
     */
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

void Test::testFdo52475()
{
    // The problem was that \chcbpat0 resulted in no color, instead of COL_AUTO.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(getRun(getParagraph(1), 3), "CharBackColor"));
}

void Test::testFdo55493()
{
    // The problem was that the width of the PNG was detected as 15,24cm, instead of 3.97cm
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3969), xShape->getSize().Width);
}

void Test::testCopyPastePageStyle()
{
    // The problem was that RTF import during copy&paste did not ignore page styles.
    // Once we have more copy&paste tests, makes sense to refactor this to some helper method.
    paste("copypaste-pagestyle-paste.rtf");

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21001), getProperty<sal_Int32>(xPropertySet, "Width")); // Was letter, i.e. 21590
}

void Test::testCopyPasteFootnote()
{
    // The RTF import did not handle the case when the position wasn't the main document XText, but something different, e.g. a footnote.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    paste("copypaste-footnote-paste.rtf", xTextRange);

    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), xTextRange->getString());
}

void Test::testFdo61193()
{
    // Pasting content that contained a footnote caused a crash.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo61193.rtf", xEnd);
}

void Test::testShptxtPard()
{
    // The problem was that \pard inside \shptxt caused loss of shape text
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("shape text"), xText->getString());
}

void Test::testDoDhgt()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        sal_Int32 nFillColor = getProperty<sal_Int32>(xDraws->getByIndex(i), "FillColor");
        if (nFillColor == 0xc0504d) // red
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (nFillColor == 0x9bbb59) // green
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (nFillColor == 0x4f81bd) // blue
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
    }
}

void Test::testDplinehollow()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_NONE, getProperty<drawing::LineStyle>(xPropertySet, "LineStyle"));
}

void Test::testLeftmarginDefault()
{
    // The default left/right margin was incorrect when the top margin was set to zero.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2540), getProperty<sal_Int32>(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "LeftMargin"));
}

void Test::testDppolyline()
{
    // This was completely ignored, for now, just make sure we have all 4 lines.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDraws->getCount());
}

void Test::testFdo56512()
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    OUString aExpected("עוסק מורשה ", 20, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

void Test::testFdo52989()
{
    // Same as n#192129, but for JPEG files.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(423), xShape->getSize().Width);
}

void Test::testFdo48442()
{
    // The problem was that \pvmrg is the default in RTF, but not in Writer.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA, getProperty<sal_Int16>(xShape, "VertOrientRelation")); // was FRAME
}

void Test::testFdo55525()
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Negative left margin was ~missing, -191
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1877), getProperty<sal_Int32>(xTable, "LeftMargin"));
    // Cell width of A1 was 3332 (e.g. not set, 30% percent of total width)
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(896), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[0].Position);
}

void Test::testFdo57708()
{
    // There were two issues: the doc was of 2 pages and the picture was missing.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // Two objects: a picture and a textframe.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
}

void Test::testFdo54473()
{
    // The problem was that character styles were not imported due to a typo.
    CPPUNIT_ASSERT_EQUAL(OUString("Anot"), getProperty<OUString>(getRun(getParagraph(1), 1, "Text "), "CharStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("ForeignTxt"), getProperty<OUString>(getRun(getParagraph(1), 3, "character "), "CharStyleName"));
}

void Test::testFdo49934()
{
    // Column break without columns defined should be a page break, but it was just ignored.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

void Test::testFdo57886()
{
    // Was 'int from <?> to <?> <?>'.
    CPPUNIT_ASSERT_EQUAL(OUString("int from {firstlower} to {firstupper} {firstbody}"), getFormula(getRun(getParagraph(1), 1)));
}

void Test::testFdo58076()
{
    // An additional section was created, so the default page style didn't have the custom margins.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2251), getProperty<sal_Int32>(xStyle, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1752), getProperty<sal_Int32>(xStyle, "RightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, "BottomMargin"));
}

void Test::testFdo57678()
{
    // Paragraphs of the two tables were not converted to tables.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

void Test::testFdo45183()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // Was text::WrapTextMode_PARALLEL, i.e. shpfblwtxt didn't send the shape below text.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(xDraws->getByIndex(0), "Surround"));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // Was 247, resulting in a table having width almost zero and height of 10+ pages.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16237), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

void Test::testFdo54612()
{
    // \dpptx without a \dppolycount caused a crash.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());
}

void Test::testFdo58933()
{
    // The problem was that the table had an additional cell in its first line.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was 4.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getCellNames().getLength());
}

void Test::testFdo44053()
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The with of the table's A1 and A2 cell should equal.
    CPPUNIT_ASSERT_EQUAL(getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[0].Position,
            getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position);
}

void Test::testFdo48440()
{
    // Page break was ignored.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

void Test::testFdo58646line()
{
    // \line symbol was ignored
    getParagraph(1, "foo\nbar");
}

void Test::testFdo58646()
{
    // Page break was ignored inside a continous section, on title page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

void Test::testFdo59419()
{
    // Junk to be ignored broke import of the table.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
}

void Test::testFdo58076_2()
{
    // Position of the picture wasn't correct.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(8345)), getProperty<sal_Int32>(xDraws->getByIndex(0), "HoriOrientPosition"));
}

void Test::testFdo59953()
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of A1 was 4998 (e.g. not set / not wide enough, ~50% of total width)
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7649), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[0].Position);
}

void Test::testFdo59638()
{
    // The problem was that w:lvlOverride inside w:num was ignores by dmapper.

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "BulletChar")
        {
            // Was '*', should be 'o'.
            CPPUNIT_ASSERT_EQUAL(OUString("\xEF\x82\xB7", 3, RTL_TEXTENCODING_UTF8), rProp.Value.get<OUString>());
            return;
        }
    }
    CPPUNIT_FAIL("no BulletChar property");
}

void Test::testFdo60722()
{
    // The problem was that the larger shape was over the smaller one, and not the other way around.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(OUString("larger"), getProperty<OUString>(xShape, "Description"));

    xShape.set(xDraws->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(xShape, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(OUString("smaller"), getProperty<OUString>(xShape, "Description"));

    // Color of the line was blue, and it had zero width.
    xShape.set(xDraws->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(26), getProperty<sal_uInt32>(xShape, "LineWidth"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), getProperty<sal_uInt32>(xShape, "LineColor"));
}

void Test::testFdo61909()
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);
    // Was the Writer default font.
    CPPUNIT_ASSERT_EQUAL(OUString("Courier New"), getProperty<OUString>(xTextRange, "CharFontName"));
    // Was 0x008000.
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<sal_uInt32>(xTextRange, "CharBackColor"));
}

void Test::testFdo62288()
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    // Margins were inherited from the previous cell, even there was a \pard there.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaLeftMargin"));
}

void Test::testFdo37716()
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    // \nowrap got ignored, so Surround was text::WrapTextMode_PARALLEL
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_NONE, getProperty<text::WrapTextMode>(xFrames->getByIndex(0), "Surround"));
}

void Test::testFdo51916()
{
    // Complex nested table caused a crash.
}

void Test::testFdo63023()
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "HeaderText");
    // Back color was black (0) in the header, due to missing color table in the substream.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFF99), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xHeaderText), 1), "CharBackColor"));
}

void Test::testFdo42109()
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    // Make sure the page number is imported as a field in the B1 cell.
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 1), "TextPortionType"));
}

void Test::testFdo62977()
{
    // The middle character was imported as '?' instead of the proper unicode value.
    getRun(getParagraph(1), 1, OUString("\xE5\xB9\xB4\xEF\xBC\x94\xE6\x9C\x88", 9, RTL_TEXTENCODING_UTF8));
}

void Test::testN818997()
{
    // \page was ignored between two \shp tokens.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

void Test::testFdo64671()
{
    // Additional '}' was inserted before the special character.
    getRun(getParagraph(1), 1, OUString("\xC5\xBD", 2, RTL_TEXTENCODING_UTF8));
}

void Test::testPageBackground()
{
    // The problem was that \background was ignored.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x92D050), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

void Test::testFdo62044()
{
    // The problem was that RTF import during copy&paste did not ignore existing paragraph styles.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo62044-paste.rtf", xEnd);

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("ParagraphStyles")->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(10.f, getProperty<float>(xPropertySet, "CharHeight")); // Was 18, i.e. reset back to original value.
}

void Test::testPoshPosv()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(xDraws->getByIndex(0), "HoriOrient"));
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(xDraws->getByIndex(0), "VertOrient"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xDraws->getByIndex(0), "FrameIsAutomaticHeight"));
}

void Test::testN825305()
{
    // The problem was that the textbox wasn't transparent, due to unimplemented fFilled == 0.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertyState> xPropertyState(xDraws->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(xDraws->getByIndex(1), "BackColorTransparency"));
    beans::PropertyState ePropertyState = xPropertyState->getPropertyState("BackColorTransparency");
    // Was beans::PropertyState_DEFAULT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);
}

void Test::testParaBottomMargin()
{
    // This was 353, i.e. bottom margin of the paragraph was 0.35cm instead of 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

void Test::testN823655()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(xDraws->getByIndex(0), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get< uno::Sequence<beans::PropertyValue> >();
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Coordinates")
            aCoordinates = rProp.Value.get< uno::Sequence<drawing::EnhancedCustomShapeParameterPair> >();
    }
    // The first coordinate pair of this freeform shape was 286,0 instead of 0,286.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(286), aCoordinates[0].Second.Value.get<sal_Int32>());
}

void Test::testFdo66040()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // This was 0 (no shapes were imported), we want two textframes.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());

    // The second paragraph of the first shape should be actually a table, with "A" in its A1 cell.
    uno::Reference<text::XTextRange> xTextRange(xDraws->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2, xText), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("A"), uno::Reference<text::XTextRange>(xTable->getCellByName("A1"), uno::UNO_QUERY)->getString());

    // Make sure the second shape has the correct position and size.
    uno::Reference<drawing::XShape> xShape(xDraws->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14420), getProperty<sal_Int32>(xShape, "HoriOrientPosition"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1032), getProperty<sal_Int32>(xShape, "VertOrientPosition"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14000), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21001), xShape->getSize().Height);
}

void Test::testN823675()
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    awt::FontDescriptor aFont;

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "BulletFont")
            aFont = rProp.Value.get<awt::FontDescriptor>();
    }
    // This was empty, i.e. no font name was set for the bullet numbering.
    CPPUNIT_ASSERT_EQUAL(OUString("Symbol"), aFont.Name);
}

void Test::testFdo47802()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // Shape inside table was ignored.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

void Test::testFdo39001()
{
    // Document was of 4 pages, \sect at the end of the doc wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

void Test::testGroupshape()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // There should be a single groupshape with 2 children.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
    uno::Reference<drawing::XShapes> xGroupshape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroupshape->getCount());
}

void Test::testFdo66565()
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of A2 was 554, should be 453/14846*10000
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(304), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position);
}

void Test::testFdo54900()
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // Paragraph was aligned to left, should be center.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraphOfText(1, xCell->getText()), "ParaAdjust")));
}

void Test::testFdo64637()
{
    // The problem was that the custom "Company" property was added twice, the second invocation resulted in an exception.
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDocumentPropertiesSupplier->getDocumentProperties()->getUserDefinedProperties(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getProperty<OUString>(xPropertySet, "Company"));
}

void Test::testN820504()
{
    // The shape was anchored at-page instead of at-character (that's incorrect as Word only supports at-character and as-character).
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(xDraws->getByIndex(0), "AnchorType"));
}

void Test::testFdo67365()
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xRows = xTable->getRows();
    // The table only had 3 rows.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xRows->getCount());
    // This was 4999, i.e. the two cells of the row had equal widths instead of a larger and a smaller cell.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5290), getProperty< uno::Sequence<text::TableColumnSeparator> >(xRows->getByIndex(2), "TableColumnSeparators")[0].Position);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A2"), uno::UNO_QUERY);
    // Paragraph was aligned to center, should be left.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraphOfText(1, xCell->getText()), "ParaAdjust")));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
