/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>

#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/util/XBlurEffect.hpp>
#include <com/sun/star/text/XTextPortionAppend.hpp>
#include <com/sun/star/text/XParagraphAppend.hpp>
#include <com/sun/star/util/XComplexColor.hpp>
#include <com/sun/star/util/XGlowEffect.hpp>
#include <com/sun/star/util/XInnerShadowEffect.hpp>
#include <com/sun/star/util/XReflectionEffect.hpp>
#include <com/sun/star/util/XSoftEdgeEffect.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <docmodel/theme/Theme.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/uno/UnoBlurEffect.hxx>
#include <docmodel/uno/UnoGlowEffect.hxx>
#include <docmodel/uno/UnoInnerShadowEffect.hxx>
#include <docmodel/uno/UnoReflectionEffect.hxx>
#include <docmodel/uno/UnoSoftEdgeEffect.hxx>
#include <test/unoapixml_test.hxx>
#include <unotools/tempfile.hxx>
#include <svx/EffectItems.hxx>
#include <svx/svddef.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/unomid.hxx>
#include <svx/unopage.hxx>
#include <vcl/virdev.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <extendedprimitive2dxmldump.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <unotools/streamwrap.hxx>
#include <vcl/filter/PngImageReader.hxx>

#include <sdr/contact/objectcontactofobjlistpainter.hxx>

using namespace ::com::sun::star;
using namespace ::cpo;

namespace
{
/// Tests for svx/source/unodraw/ code.
class UnodrawTest : public UnoApiXmlTest
{
public:
    UnodrawTest()
        : UnoApiXmlTest(u"svx/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(UnodrawTest, testWriterGraphicExport)
{
    // Load a document with a Writer picture in it.
    loadFromFile(u"unodraw-writer-image.odt");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<lang::XComponent> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // Export it as JPEG.
    uno::Reference<drawing::XGraphicExportFilter> xExportFilter
        = drawing::GraphicExportFilter::create(m_xContext);
    // This resulted in a css::lang::IllegalArgumentException for a Writer
    // picture.
    xExportFilter->setSourceDocument(xShape);

    cpo::uno::Sequence<beans::PropertyValue> aProperties(comphelper::InitPropertySequence(
        { { u"URL"_ustr, cpo::uno::Any(maTempFile.GetURL()) },
          { u"MediaType"_ustr, cpo::uno::Any(u"image/jpeg"_ustr) } }));
    CPPUNIT_ASSERT(xExportFilter->filter(aProperties));
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testTdf93998)
{
    loadFromFile(u"tdf93998.odp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());

    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPage.is());

    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
    uno::Reference<awt::XControlModel> xModel(
        xFactory->createInstance(u"com.sun.star.awt.UnoControlDialogModel"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());

    uno::Reference<beans::XPropertySet> xModelProps(xModel, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModelProps.is());

    // This resulted in a uno::RuntimeException, assigning a shape to a dialog model's image was
    // broken.
    xModelProps->setPropertyValue(u"ImageURL"_ustr, xShape->getPropertyValue(u"GraphicURL"_ustr));
    uno::Reference<graphic::XGraphic> xGraphic;
    xModelProps->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testTableShadowDirect)
{
    // Create an Impress document an insert a table shape.
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.TableShape"_ustr), uno::UNO_QUERY);
    xShape->setPosition(awt::Point(1000, 1000));
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    xDrawPage->add(xShape);

    // Create a red shadow on it without touching its style.
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with throwing a
    // beans.UnknownPropertyException, as shadow-as-direct-formatting on tables were not possible.
    xShapeProps->setPropertyValue(u"Shadow"_ustr, cpo::uno::Any(true));
    Color nRed = COL_LIGHTRED;
    xShapeProps->setPropertyValue(u"ShadowColor"_ustr, cpo::uno::Any(nRed));
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"ShadowColor"_ustr) >>= nRed);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nRed);

    // Add text.
    uno::Reference<table::XCellRange> xTable(xShapeProps->getPropertyValue(u"Model"_ustr),
                                             uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);
    xCell->setString(u"A1"_ustr);

    // Generates drawinglayer primitives for the shape.
    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    SdrPage* pSdrPage = pDrawPage->GetSdrPage();
    ScopedVclPtrInstance<VirtualDevice> aVirtualDevice;
    sdr::contact::ObjectContactOfObjListPainter aObjectContact(*aVirtualDevice,
                                                               { pSdrPage->GetObj(0) }, nullptr);
    const sdr::contact::ViewObjectContact& rDrawPageVOContact
        = pSdrPage->GetViewContact().GetViewObjectContact(aObjectContact);
    sdr::contact::DisplayInfo aDisplayInfo;
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence;
    rDrawPageVOContact.getPrimitive2DSequenceHierarchy(aDisplayInfo, xPrimitiveSequence);

    // Check the primitives.
    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    assertXPath(pDocument, "//shadow", /*nNumberOfNodes=*/1);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. there was shadow for the cell text, while here PowerPoint-compatible output is expected,
    // which has no shadow for cell text (only for cell borders and cell background).
    assertXPath(pDocument, "//shadow//sdrblocktext", /*nNumberOfNodes=*/0);
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testTitleShapeBullets)
{
    // Create a title shape with 2 paragraphs in it.
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    // A default document contains a title shape and a text shape on the first slide.
    uno::Reference<drawing::XShape> xTitleShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xTitleShapeInfo(xTitleShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(
        xTitleShapeInfo->supportsService(u"com.sun.star.presentation.TitleTextShape"_ustr));
    uno::Reference<text::XTextRange> xTitleShapeText(xTitleShape, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTitleShapeText->getText();
    uno::Reference<text::XTextRange> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"foo"_ustr, /*bAbsorb=*/false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::APPEND_PARAGRAPH,
                                  /*bAbsorb=*/false);
    xText->insertString(xCursor, u"bar"_ustr, /*bAbsorb=*/false);

    // Check that the title shape has 2 paragraphs.
    uno::Reference<container::XEnumerationAccess> xTextEA(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xTextE = xTextEA->createEnumeration();
    // Has a first paragraph.
    CPPUNIT_ASSERT(xTextE->hasMoreElements());
    xTextE->nextElement();
    // Has a second paragraph.
    // Without the accompanying fix in place, this test would have failed, because the 2 paragraphs
    // were merged together (e.g. 1 bullet instead of 2 bullets for bulleted paragraphs).
    CPPUNIT_ASSERT(xTextE->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testParagraphsAfterSetString)
{
    // Given a title shape whose text was set and then got a second paragraph:
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    xText->setString(u"foo"_ustr);
    uno::Reference<text::XParagraphAppend> xParagraphAppend(xText, uno::UNO_QUERY);
    xParagraphAppend->finishParagraph({});
    uno::Reference<text::XTextPortionAppend> xPortionAppend(xText, uno::UNO_QUERY);
    xPortionAppend->appendTextPortion(u"bar"_ustr, {});

    // Then the paragraphs of the text are both paragraphs:
    uno::Reference<container::XEnumerationAccess> xTextEA(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xTextE = xTextEA->createEnumeration();
    CPPUNIT_ASSERT(xTextE->hasMoreElements());
    xTextE->nextElement();
    // Without the accompanying fix in place, this failed, the enumeration ended after the text
    // that setString had set.
    CPPUNIT_ASSERT(xTextE->hasMoreElements());
    uno::Reference<text::XTextRange> xSecond(xTextE->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, xSecond->getString());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testPngExport)
{
    // Given an empty Impress document:
    loadFromURL(u"private:factory/simpress"_ustr);

    // When exporting that document to PNG with a JSON size:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aStream);
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterName"_ustr] <<= u"impress_png_Export"_ustr;
    aMediaDescriptor[u"FilterOptions"_ustr]
        <<= u"{\"PixelHeight\":{\"type\":\"long\",\"value\":\"192\"},"
            "\"PixelWidth\":{\"type\":\"long\",\"value\":\"192\"}}"_ustr;
    aMediaDescriptor[u"OutputStream"_ustr] <<= xOut;
    xStorable->storeToURL(u"private:stream"_ustr, aMediaDescriptor.getAsConstPropertyValueList());

    // Then make sure that the size request is handled:
    aStream.Seek(STREAM_SEEK_TO_BEGIN);
    vcl::PngImageReader aPngReader(aStream);
    Bitmap aBitmap;
    aPngReader.read(aBitmap);
    Size aSize = aBitmap.GetSizePixel();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 192
    // - Actual  : 595
    // i.e. it was not possible to influence the size from the cmdline.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(192), aSize.getHeight());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(192), aSize.getWidth());
}

uno::Reference<drawing::XShape> insertRectangle(const uno::Reference<lang::XComponent>& xComponent)
{
    uno::Reference<lang::XMultiServiceFactory> xFactory(xComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setPosition(awt::Point(1000, 1000));
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    xDrawPage->add(xShape);
    return xShape;
}

model::ComplexColor getComplexColor(const uno::Reference<beans::XPropertySet>& xProperties,
                                    const OUString& rPropertyName)
{
    uno::Reference<util::XComplexColor> xComplexColor;
    CPPUNIT_ASSERT(xProperties->getPropertyValue(rPropertyName) >>= xComplexColor);
    CPPUNIT_ASSERT(xComplexColor.is());
    return model::color::getFromXComplexColor(xComplexColor);
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testFillThemeColorSetBeforePlainColor)
{
    // Given a shape in an Impress document:
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<drawing::XShape> xShape = insertRectangle(mxComponent);

    // When one call sets the fill to the theme color accent1 and then to a plain color:
    model::ComplexColor aThemeColor = model::ComplexColor::Theme(model::ThemeColorType::Accent1);
    uno::Reference<beans::XMultiPropertySet> xMultiProperties(xShape, uno::UNO_QUERY);
    xMultiProperties->setPropertyValues(
        { u"FillComplexColor"_ustr, u"FillColor"_ustr },
        { uno::Any(model::color::createXComplexColor(aThemeColor)), uno::Any(Color(0x123456)) });

    // Then the fill keeps the theme color:
    uno::Reference<beans::XPropertySet> xProperties(xShape, uno::UNO_QUERY);
    model::ComplexColor aFillColor = getComplexColor(xProperties, u"FillComplexColor"_ustr);
    // Without the fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : -1
    // i.e. the plain color cleared the theme color that the same call had set.
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aFillColor.getThemeColorType());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testTextThemeColorSetBeforePlainColor)
{
    // Given a shape in an Impress document with some text:
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<drawing::XShape> xShape = insertRectangle(mxComponent);
    uno::Reference<text::XTextRange> xShapeText(xShape, uno::UNO_QUERY);
    xShapeText->setString(u"abc"_ustr);

    // When one call sets the text to the theme color accent1 and then to a plain color:
    uno::Reference<text::XTextCursor> xCursor = xShapeText->getText()->createTextCursor();
    xCursor->gotoEnd(/*bExpand=*/true);
    model::ComplexColor aThemeColor = model::ComplexColor::Theme(model::ThemeColorType::Accent1);
    uno::Reference<beans::XMultiPropertySet> xMultiProperties(xCursor, uno::UNO_QUERY);
    xMultiProperties->setPropertyValues(
        { u"CharComplexColor"_ustr, u"CharColor"_ustr },
        { uno::Any(model::color::createXComplexColor(aThemeColor)), uno::Any(Color(0x123456)) });

    // Then the text keeps the theme color:
    uno::Reference<beans::XPropertySet> xProperties(xCursor, uno::UNO_QUERY);
    model::ComplexColor aCharColor = getComplexColor(xProperties, u"CharComplexColor"_ustr);
    // Without the fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : -1
    // i.e. the plain color cleared the theme color that the same call had set.
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aCharColor.getThemeColorType());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testGlowThemeColorTakesTheColorOfTheTheme)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<drawing::XShape> xShape = insertRectangle(mxComponent);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"GlowEffectRadius"_ustr, uno::Any(sal_Int32(500)));
    xShapeProps->setPropertyValue(u"GlowEffectColor"_ustr, uno::Any(Color(0xff0000)));

    auto pColorSet = std::make_shared<model::ColorSet>(u"Test"_ustr);
    pColorSet->add(model::ThemeColorType::Accent2, Color(0x4472c4));
    auto pTheme = std::make_shared<model::Theme>(u"Test"_ustr);
    pTheme->setColorSet(pColorSet);
    SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);
    CPPUNIT_ASSERT(pObject);
    pObject->getSdrPageFromSdrObject()->getSdrPageProperties().setTheme(pTheme);

    // The glow draws with the color that the theme gives to the theme color.
    model::ComplexColor aAccent;
    aAccent.setThemeColor(model::ThemeColorType::Accent2);
    xShapeProps->setPropertyValue(u"GlowEffectComplexColor"_ustr,
                                  uno::Any(model::color::createXComplexColor(aAccent)));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4472c4),
                         xShapeProps->getPropertyValue(u"GlowEffectColor"_ustr).get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testEffectJSONWithWrongValuesIsRejected)
{
    SdrGlowEffectItem aGlow(SDRATTR_GLOW);
    aGlow.setRadius(500_hmm);

    // A radius that is no number, a theme color without its transformations and a color that is
    // no hex number are rejected, and the glow stays as it was.
    CPPUNIT_ASSERT(!aGlow.PutValue(uno::Any(u"{\"Radius\":\"wide\"}"_ustr), MID_EFFECT_JSON));
    CPPUNIT_ASSERT(!aGlow.PutValue(
        uno::Any(u"{\"Radius\":1,\"Color\":{\"ThemeIndex\":4,\"RGB\":\"ff0000\"}}"_ustr),
        MID_EFFECT_JSON));
    CPPUNIT_ASSERT(!aGlow.PutValue(
        uno::Any(u"{\"Radius\":1,\"Color\":{\"ThemeIndex\":-1,\"RGB\":\"transparent\"}}"_ustr),
        MID_EFFECT_JSON));
    CPPUNIT_ASSERT_EQUAL(500_hmm, aGlow.getEffect().maRadius);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aGlow.getColor());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testGlowEffectProperties)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<beans::XPropertySet> xShapeProps(insertRectangle(mxComponent), uno::UNO_QUERY);

    // The single values of the glow are views of one glow.
    xShapeProps->setPropertyValue(u"GlowEffectRadius"_ustr, uno::Any(sal_Int32(500)));
    xShapeProps->setPropertyValue(u"GlowEffectColor"_ustr, uno::Any(Color(0x4472c4)));
    xShapeProps->setPropertyValue(u"GlowEffectTransparency"_ustr, uno::Any(sal_Int16(40)));

    uno::Reference<util::XGlowEffect> xGlow;
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"GlowEffect"_ustr) >>= xGlow);
    std::optional<model::GlowEffect> oGlow = model::effect::getFromXGlowEffect(xGlow);
    CPPUNIT_ASSERT(oGlow);
    CPPUNIT_ASSERT_EQUAL(500_hmm, oGlow->maRadius);
    CPPUNIT_ASSERT_EQUAL(Color(0x4472c4), oGlow->maColor.getRGB());

    // A changed transparency keeps the radius and the color.
    xShapeProps->setPropertyValue(u"GlowEffectTransparency"_ustr, uno::Any(sal_Int16(25)));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(500),
                         xShapeProps->getPropertyValue(u"GlowEffectRadius"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4472c4),
                         xShapeProps->getPropertyValue(u"GlowEffectColor"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(25),
        xShapeProps->getPropertyValue(u"GlowEffectTransparency"_ustr).get<sal_Int16>());

    // The whole glow set on another shape gives it the same single values.
    uno::Reference<beans::XPropertySet> xOtherProps(insertRectangle(mxComponent), uno::UNO_QUERY);
    xOtherProps->setPropertyValue(u"GlowEffect"_ustr,
                                  xShapeProps->getPropertyValue(u"GlowEffect"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(500),
                         xOtherProps->getPropertyValue(u"GlowEffectRadius"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(25),
        xOtherProps->getPropertyValue(u"GlowEffectTransparency"_ustr).get<sal_Int16>());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testSoftEdgeEffectProperties)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<beans::XPropertySet> xShapeProps(insertRectangle(mxComponent), uno::UNO_QUERY);

    // The radius is a view of the whole soft edge.
    xShapeProps->setPropertyValue(u"SoftEdgeRadius"_ustr, uno::Any(sal_Int32(350)));
    uno::Reference<util::XSoftEdgeEffect> xSoftEdge;
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"SoftEdgeEffect"_ustr) >>= xSoftEdge);
    std::optional<model::SoftEdgeEffect> oSoftEdge
        = model::effect::getFromXSoftEdgeEffect(xSoftEdge);
    CPPUNIT_ASSERT(oSoftEdge);
    CPPUNIT_ASSERT_EQUAL(350_hmm, oSoftEdge->maRadius);

    // The whole soft edge set on another shape gives it the same radius.
    uno::Reference<beans::XPropertySet> xOtherProps(insertRectangle(mxComponent), uno::UNO_QUERY);
    xOtherProps->setPropertyValue(u"SoftEdgeEffect"_ustr, uno::Any(xSoftEdge));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(350),
                         xOtherProps->getPropertyValue(u"SoftEdgeRadius"_ustr).get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testEffectJSONTurnsTheEffectOnAndOff)
{
    SdrReflectionEffectItem aReflection(SDRATTR_REFLECTION);
    CPPUNIT_ASSERT(!aReflection.isEnabled());

    // The JSON of an effect that can be off has whether it is on, next to its values.
    CPPUNIT_ASSERT(aReflection.PutValue(
        uno::Any(u"{\"Enabled\":true,\"Distance\":36000}"_ustr), MID_EFFECT_JSON));
    CPPUNIT_ASSERT(aReflection.isEnabled());
    CPPUNIT_ASSERT_EQUAL(36000_emu, aReflection.getEffect().maDistance);

    // The JSON that the item gives reads back to the same reflection.
    uno::Any aJSON;
    CPPUNIT_ASSERT(aReflection.QueryValue(aJSON, MID_EFFECT_JSON));
    SdrReflectionEffectItem aCopy(SDRATTR_REFLECTION);
    CPPUNIT_ASSERT(aCopy.PutValue(aJSON, MID_EFFECT_JSON));
    CPPUNIT_ASSERT(aCopy.isEnabled());
    CPPUNIT_ASSERT_EQUAL(36000_emu, aCopy.getEffect().maDistance);
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testReflectionEffectObject)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<beans::XPropertySet> xShapeProps(insertRectangle(mxComponent), uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"ReflectionEffectEnabled"_ustr, uno::Any(true));
    xShapeProps->setPropertyValue(u"ReflectionEffectDistance"_ustr, uno::Any(sal_Int32(250)));

    // The whole reflection has the distance, and a new reflection is a mirror image below the
    // shape.
    uno::Reference<util::XReflectionEffect> xReflection;
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"ReflectionEffect"_ustr) >>= xReflection);
    std::optional<model::ReflectionEffect> oReflection
        = model::effect::getFromXReflectionEffect(xReflection);
    CPPUNIT_ASSERT(oReflection);
    CPPUNIT_ASSERT_EQUAL(250_hmm, oReflection->maDistance);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5400000), oReflection->mnDirection.get());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-100000), oReflection->mnScaleY);

    // The whole reflection set on another shape gives it the same values, without turning the
    // reflection on.
    uno::Reference<beans::XPropertySet> xOtherProps(insertRectangle(mxComponent), uno::UNO_QUERY);
    xOtherProps->setPropertyValue(u"ReflectionEffect"_ustr, uno::Any(xReflection));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(250),
        xOtherProps->getPropertyValue(u"ReflectionEffectDistance"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT(!xOtherProps->getPropertyValue(u"ReflectionEffectEnabled"_ustr).get<bool>());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testBlurEffectProperties)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<beans::XPropertySet> xShapeProps(insertRectangle(mxComponent), uno::UNO_QUERY);

    // A new shape has no blur.
    CPPUNIT_ASSERT(!xShapeProps->getPropertyValue(u"BlurEffectEnabled"_ustr).get<bool>());

    // A blur set on the shape reads back unchanged.
    model::BlurEffect aBlur;
    aBlur.maRadius = 12700_emu;
    aBlur.mbGrow = false;
    xShapeProps->setPropertyValue(u"BlurEffect"_ustr,
                                  uno::Any(model::effect::createXBlurEffect(aBlur)));
    xShapeProps->setPropertyValue(u"BlurEffectEnabled"_ustr, uno::Any(true));

    uno::Reference<util::XBlurEffect> xBlur;
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"BlurEffect"_ustr) >>= xBlur);
    std::optional<model::BlurEffect> oBlur = model::effect::getFromXBlurEffect(xBlur);
    CPPUNIT_ASSERT(oBlur);
    CPPUNIT_ASSERT_EQUAL(12700_emu, oBlur->maRadius);
    CPPUNIT_ASSERT(!oBlur->mbGrow);
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"BlurEffectEnabled"_ustr).get<bool>());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testInnerShadowEffectProperties)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<beans::XPropertySet> xShapeProps(insertRectangle(mxComponent), uno::UNO_QUERY);

    // A new shape has no inner shadow.
    CPPUNIT_ASSERT(!xShapeProps->getPropertyValue(u"InnerShadowEffectEnabled"_ustr).get<bool>());

    // An inner shadow set on the shape reads back unchanged, with its theme color.
    model::InnerShadowEffect aInnerShadow;
    aInnerShadow.maBlurRadius = 63500_emu;
    aInnerShadow.maDistance = 38100_emu;
    aInnerShadow.mnDirection = 2700000_deg60000;
    aInnerShadow.maColor = model::ComplexColor::Theme(model::ThemeColorType::Accent3);
    xShapeProps->setPropertyValue(u"InnerShadowEffect"_ustr,
                                  uno::Any(model::effect::createXInnerShadowEffect(aInnerShadow)));
    xShapeProps->setPropertyValue(u"InnerShadowEffectEnabled"_ustr, uno::Any(true));

    uno::Reference<util::XInnerShadowEffect> xInnerShadow;
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"InnerShadowEffect"_ustr) >>= xInnerShadow);
    std::optional<model::InnerShadowEffect> oInnerShadow
        = model::effect::getFromXInnerShadowEffect(xInnerShadow);
    CPPUNIT_ASSERT(oInnerShadow);
    CPPUNIT_ASSERT_EQUAL(63500_emu, oInnerShadow->maBlurRadius);
    CPPUNIT_ASSERT_EQUAL(38100_emu, oInnerShadow->maDistance);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2700000), oInnerShadow->mnDirection.get());
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, oInnerShadow->maColor.getThemeColorType());
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"InnerShadowEffectEnabled"_ustr).get<bool>());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testShadowTransformProperties)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<beans::XPropertySet> xShapeProps(insertRectangle(mxComponent), uno::UNO_QUERY);

    // A new shape has a shadow without skew that does not turn with the shape.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         xShapeProps->getPropertyValue(u"ShadowSkewX"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         xShapeProps->getPropertyValue(u"ShadowSkewY"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT(!xShapeProps->getPropertyValue(u"ShadowRotateWithShape"_ustr).get<bool>());

    // Values set on the shape read back unchanged, and the skew keeps 1/60000 degree.
    xShapeProps->setPropertyValue(u"ShadowSkewX"_ustr, uno::Any(sal_Int32(-1200001)));
    xShapeProps->setPropertyValue(u"ShadowSkewY"_ustr, uno::Any(sal_Int32(900000)));
    xShapeProps->setPropertyValue(u"ShadowRotateWithShape"_ustr, uno::Any(true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1200001),
                         xShapeProps->getPropertyValue(u"ShadowSkewX"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(900000),
                         xShapeProps->getPropertyValue(u"ShadowSkewY"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"ShadowRotateWithShape"_ustr).get<bool>());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testShadowComplexColor)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<beans::XPropertySet> xShapeProps(insertRectangle(mxComponent), uno::UNO_QUERY);

    // A theme color set as the shadow color reads back as the theme color.
    model::ComplexColor aThemeColor = model::ComplexColor::Theme(model::ThemeColorType::Accent2);
    xShapeProps->setPropertyValue(u"ShadowComplexColor"_ustr,
                                  uno::Any(model::color::createXComplexColor(aThemeColor)));
    CPPUNIT_ASSERT_EQUAL(
        model::ThemeColorType::Accent2,
        getComplexColor(xShapeProps, u"ShadowComplexColor"_ustr).getThemeColorType());

    // A different plain shadow color replaces the theme color.
    xShapeProps->setPropertyValue(u"ShadowColor"_ustr, uno::Any(Color(0x123456)));
    CPPUNIT_ASSERT_EQUAL(
        model::ThemeColorType::Unknown,
        getComplexColor(xShapeProps, u"ShadowComplexColor"_ustr).getThemeColorType());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testReflectionProperties)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<beans::XPropertySet> xShapeProps(insertRectangle(mxComponent), uno::UNO_QUERY);

    // A new shape has no reflection, and the values it would have fade from opaque at the shape
    // to invisible at the far end.
    CPPUNIT_ASSERT(!xShapeProps->getPropertyValue(u"ReflectionEffectEnabled"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        xShapeProps->getPropertyValue(u"ReflectionEffectDistance"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        xShapeProps->getPropertyValue(u"ReflectionEffectBlurRadius"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(0),
        xShapeProps->getPropertyValue(u"ReflectionEffectStartTransparency"_ustr).get<sal_Int16>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(0),
        xShapeProps->getPropertyValue(u"ReflectionEffectStartPosition"_ustr).get<sal_Int16>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(100),
        xShapeProps->getPropertyValue(u"ReflectionEffectEndTransparency"_ustr).get<sal_Int16>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(100),
        xShapeProps->getPropertyValue(u"ReflectionEffectEndPosition"_ustr).get<sal_Int16>());

    // Values set on the shape read back unchanged.
    xShapeProps->setPropertyValue(u"ReflectionEffectEnabled"_ustr, cpo::uno::Any(true));
    xShapeProps->setPropertyValue(u"ReflectionEffectDistance"_ustr, cpo::uno::Any(sal_Int32(250)));
    xShapeProps->setPropertyValue(u"ReflectionEffectEndPosition"_ustr,
                                  cpo::uno::Any(sal_Int16(35)));
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"ReflectionEffectEnabled"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(250),
        xShapeProps->getPropertyValue(u"ReflectionEffectDistance"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(35),
        xShapeProps->getPropertyValue(u"ReflectionEffectEndPosition"_ustr).get<sal_Int16>());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
