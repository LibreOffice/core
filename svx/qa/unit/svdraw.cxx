/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>

#include <drawinglayer/tools/primitive2dxmldump.hxx>
#include <rtl/ustring.hxx>
#include <vcl/virdev.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdorect.hxx>
#include <svx/unopage.hxx>
#include <svx/svdview.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnstwit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/itempool.hxx>
#include <svx/svdomedia.hxx>

#include <sdr/contact/objectcontactofobjlistpainter.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/svdraw/ code.
class SvdrawTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
protected:
    uno::Reference<lang::XComponent> mxComponent;
    SdrPage* getFirstDrawPageWithAssert();

public:
    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();
        mxDesktop.set(frame::Desktop::create(m_xContext));
    }

    virtual void tearDown() override
    {
        if (mxComponent.is())
        {
            mxComponent->dispose();
        }
        test::BootstrapFixture::tearDown();
    }
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

SdrPage* SvdrawTest::getFirstDrawPageWithAssert()
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPage.is());

    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    return pDrawPage->GetSdrPage();
}

xmlDocUniquePtr lcl_dumpAndParseFirstObjectWithAssert(SdrPage* pSdrPage)
{
    ScopedVclPtrInstance<VirtualDevice> aVirtualDevice;
    sdr::contact::ObjectContactOfObjListPainter aObjectContact(*aVirtualDevice,
                                                               { pSdrPage->GetObj(0) }, nullptr);
    const auto& rDrawPageVOContact
        = pSdrPage->GetViewContact().GetViewObjectContact(aObjectContact);
    sdr::contact::DisplayInfo aDisplayInfo;
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence;
    rDrawPageVOContact.getPrimitive2DSequenceHierarchy(aDisplayInfo, xPrimitiveSequence);

    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = aDumper.dumpAndParse(xPrimitiveSequence);
    CPPUNIT_ASSERT(pXmlDoc);
    return pXmlDoc;
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testSemiTransparentText)
{
    // Create a new Draw document with a rectangle.
    getComponent() = loadFromDesktop("private:factory/sdraw");
    uno::Reference<lang::XMultiServiceFactory> xFactory(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    xShape->setPosition(awt::Point(1000, 1000));

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    xDrawPage->add(xShape);

    // Add semi-transparent text on the rectangle.
    uno::Reference<text::XTextRange> xShapeText(xShape, uno::UNO_QUERY);
    xShapeText->getText()->setString("hello");

    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("CharColor", uno::Any(COL_RED));
    sal_Int16 nTransparence = 75;
    xShapeProperties->setPropertyValue("CharTransparence", uno::Any(nTransparence));

    // Generates drawinglayer primitives for the page.
    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    SdrPage* pSdrPage = pDrawPage->GetSdrPage();
    xmlDocUniquePtr pDocument = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // Make sure the text is semi-transparent.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//unifiedtransparence' number of nodes is incorrect
    // i.e. the text was just plain red, not semi-transparent.
    sal_Int16 fTransparence
        = getXPath(pDocument, "//unifiedtransparence", "transparence").toInt32();
    CPPUNIT_ASSERT_EQUAL(nTransparence, fTransparence);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testHandlePathObjScale)
{
    // Given a path object:
    getComponent() = loadFromDesktop("private:factory/sdraw");
    uno::Reference<lang::XMultiServiceFactory> xFactory(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.ClosedBezierShape"), uno::UNO_QUERY);

    // When setting its scale by both using setSize() and scaling in a transform matrix:
    // Set size and basic properties.
    xShape->setPosition(awt::Point(2512, 6062));
    xShape->setSize(awt::Size(112, 112));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue("FillStyle", uno::Any(drawing::FillStyle_SOLID));
    xShapeProps->setPropertyValue("LineStyle", uno::Any(drawing::LineStyle_SOLID));
    xShapeProps->setPropertyValue("FillColor", uno::Any(static_cast<sal_Int32>(0)));
    // Add it to the draw page.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    xDrawPage->add(xShape);
    // Set polygon coordinates.
    drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords;
    aPolyPolygonBezierCoords.Coordinates = {
        {
            awt::Point(2624, 6118),
            awt::Point(2624, 6087),
            awt::Point(2599, 6062),
            awt::Point(2568, 6062),
            awt::Point(2537, 6062),
            awt::Point(2512, 6087),
            awt::Point(2512, 6118),
            awt::Point(2512, 6149),
            awt::Point(2537, 6175),
            awt::Point(2568, 6174),
            awt::Point(2599, 6174),
            awt::Point(2625, 6149),
            awt::Point(2624, 6118),
        },
    };
    aPolyPolygonBezierCoords.Flags = {
        {
            drawing::PolygonFlags_NORMAL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_NORMAL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_NORMAL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_NORMAL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_NORMAL,
        },
    };
    xShapeProps->setPropertyValue("PolyPolygonBezier", uno::Any(aPolyPolygonBezierCoords));
    drawing::HomogenMatrix3 aMatrix;
    aMatrix.Line1.Column1 = 56;
    aMatrix.Line2.Column1 = -97;
    aMatrix.Line3.Column1 = 0;
    aMatrix.Line1.Column2 = 97;
    aMatrix.Line2.Column2 = 56;
    aMatrix.Line3.Column2 = 0;
    aMatrix.Line1.Column3 = 3317;
    aMatrix.Line2.Column3 = 5583;
    aMatrix.Line3.Column3 = 1;
    xShapeProps->setPropertyValue("Transformation", uno::Any(aMatrix));

    // Then make sure the scaling is only applied once:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 113
    // - Actual  : 12566
    // i.e. the scaling was applied twice.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(113), xShape->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testTextEditEmptyGrabBag)
{
    // Given a document with a groupshape, which has 2 children.
    getComponent() = loadFromDesktop("private:factory/sdraw");
    uno::Reference<lang::XMultiServiceFactory> xFactory(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xRect1(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xRect1->setPosition(awt::Point(1000, 1000));
    xRect1->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XShape> xRect2(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xRect2->setPosition(awt::Point(1000, 1000));
    xRect2->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XShapes> xGroup(
        xFactory->createInstance("com.sun.star.drawing.GroupShape"), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xGroupShape(xGroup, uno::UNO_QUERY);
    xDrawPage->add(xGroupShape);
    xGroup->add(xRect1);
    xGroup->add(xRect2);
    uno::Reference<text::XTextRange> xRect2Text(xRect2, uno::UNO_QUERY);
    xRect2Text->setString("x");
    uno::Sequence<beans::PropertyValue> aGrabBag = {
        comphelper::makePropertyValue("OOXLayout", true),
    };
    uno::Reference<beans::XPropertySet> xGroupProps(xGroup, uno::UNO_QUERY);
    xGroupProps->setPropertyValue("InteropGrabBag", uno::Any(aGrabBag));

    // When editing the shape text of the 2nd rectangle (insert a char at the start).
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SdrView* pSdrView = pViewShell->GetDrawView();
    SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xRect2);
    pSdrView->SdrBeginTextEdit(pObject);
    EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
    rEditView.InsertText("y");
    pSdrView->SdrEndTextEdit();

    // Then make sure that grab-bag is empty to avoid losing the new text.
    xGroupProps->getPropertyValue("InteropGrabBag") >>= aGrabBag;
    // Without the accompanying fix in place, this test would have failed with:
    // assertion failed
    // - Expression: !aGrabBag.hasElements()
    // i.e. the grab-bag was still around after modifying the shape, and that grab-bag contained the
    // old text.
    CPPUNIT_ASSERT(!aGrabBag.hasElements());
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testRectangleObject)
{
    std::unique_ptr<SdrModel> pModel(new SdrModel(nullptr, nullptr, true));
    pModel->GetItemPool().FreezeIdRanges();

    rtl::Reference<SdrPage> pPage(new SdrPage(*pModel, false));
    pPage->SetSize(Size(1000, 1000));
    pModel->InsertPage(pPage.get(), 0);

    tools::Rectangle aSize(Point(), Size(100, 100));
    auto* pRectangle = new SdrRectObj(*pModel, aSize);
    pPage->NbcInsertObject(pRectangle);
    pRectangle->SetMergedItem(XLineStyleItem(drawing::LineStyle_SOLID));
    pRectangle->SetMergedItem(XLineStartWidthItem(200));

    ScopedVclPtrInstance<VirtualDevice> aVirtualDevice;
    aVirtualDevice->SetOutputSize(Size(2000, 2000));

    SdrView aView(*pModel, aVirtualDevice);
    aView.hideMarkHandles();
    aView.ShowSdrPage(pPage.get());

    sdr::contact::ObjectContactOfObjListPainter aObjectContact(*aVirtualDevice,
                                                               { pPage->GetObj(0) }, nullptr);
    const sdr::contact::ViewObjectContact& rDrawPageVOContact
        = pPage->GetViewContact().GetViewObjectContact(aObjectContact);

    sdr::contact::DisplayInfo aDisplayInfo;
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence;
    rDrawPageVOContact.getPrimitive2DSequenceHierarchy(aDisplayInfo, xPrimitiveSequence);

    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = aDumper.dumpAndParse(xPrimitiveSequence);

    assertXPath(pXmlDoc, "/primitive2D", 1);

    OString aBasePath("/primitive2D/sdrrectangle/polypolygoncolor");
    assertXPath(pXmlDoc, aBasePath, "color", "#729fcf");

    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "height",
                "99"); // weird Rectangle is created with size 100
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "width", "99");
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "minx", "0");
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "miny", "0");
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "maxx", "99");
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "maxy", "99");

    aBasePath = "/primitive2D/sdrrectangle/polypolygoncolor/polypolygon/polygon";

    assertXPath(pXmlDoc, aBasePath + "/point", 5);
    assertXPath(pXmlDoc, aBasePath + "/point[1]", "x", "49.5"); // hmm, weird, why?
    assertXPath(pXmlDoc, aBasePath + "/point[1]", "y", "99");
    assertXPath(pXmlDoc, aBasePath + "/point[2]", "x", "0");
    assertXPath(pXmlDoc, aBasePath + "/point[2]", "y", "99");
    assertXPath(pXmlDoc, aBasePath + "/point[3]", "x", "0");
    assertXPath(pXmlDoc, aBasePath + "/point[3]", "y", "0");
    assertXPath(pXmlDoc, aBasePath + "/point[4]", "x", "99");
    assertXPath(pXmlDoc, aBasePath + "/point[4]", "y", "0");
    assertXPath(pXmlDoc, aBasePath + "/point[5]", "x", "99");
    assertXPath(pXmlDoc, aBasePath + "/point[5]", "y", "99");

    aBasePath = "/primitive2D/sdrrectangle/polygonstroke";
    assertXPath(pXmlDoc, aBasePath, 1);

    assertXPath(pXmlDoc, aBasePath + "/line", "color", "#3465a4");
    assertXPath(pXmlDoc, aBasePath + "/line", "width", "0");
    assertXPath(pXmlDoc, aBasePath + "/line", "linejoin", "Round");
    assertXPath(pXmlDoc, aBasePath + "/line", "linecap", "BUTT");

    assertXPathContent(pXmlDoc, aBasePath + "/polygon", "49.5,99 0,99 0,0 99,0 99,99");

    // If solid line, then there is no line stroke information
    assertXPath(pXmlDoc, aBasePath + "/stroke", 0);

    pPage->RemoveObject(0);

    SdrObject* pObject(pRectangle);
    SdrObject::Free(pObject);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testAutoHeightMultiColShape)
{
    // Given a document containing a shape that has:
    // 1) automatic height (resize shape to fix text)
    // 2) multiple columns (2)
    OUString aURL
        = m_directories.getURLFromSrc(u"svx/qa/unit/data/auto-height-multi-col-shape.pptx");

    // When loading that document:
    getComponent().set(loadFromDesktop(aURL, "com.sun.star.presentation.PresentationDocument"));

    // Make sure the in-file shape height is kept, even if nominally the shape height is
    // automatic:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6882
    // - Actual  : 3452
    // i.e. the shape height was smaller than expected, leading to a 2 columns layout instead of
    // laying out all the text in the first column.
    // 2477601 is from slide1.xml, <a:ext cx="4229467" cy="2477601"/>.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        static_cast<sal_Int32>(o3tl::convert(2477601, o3tl::Length::emu, o3tl::Length::mm100)),
        xShape->getSize().Height, 1);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testFontWorks)
{
    OUString aURL = m_directories.getURLFromSrc(u"svx/qa/unit/data/FontWork.odg");
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.drawing.DrawingDocument");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());

    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    SdrPage* pSdrPage = pDrawPage->GetSdrPage();
    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    assertXPath(pXmlDoc, "/primitive2D", 1);

    assertXPath(pXmlDoc, "//scene", "projectionMode", "Perspective");
    assertXPath(pXmlDoc, "//scene/extrude3D[1]/fill", "color", "#ff0000");
    assertXPath(pXmlDoc, "//scene/extrude3D[1]/object3Dattributes/material", "color", "#ff0000");
    // ODF default 50% is represented by Specular Intensity = 2^5. The relationship is not linear.
    assertXPath(pXmlDoc, "//scene/extrude3D[1]/object3Dattributes/material", "specularIntensity",
                "32");
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testSurfaceMetal)
{
    OUString aURL = m_directories.getURLFromSrc(u"svx/qa/unit/data/tdf140321_metal.odp");
    mxComponent = loadFromDesktop(aURL, "com.sun.star.presentation.PresentationDocument");

    SdrPage* pSdrPage = getFirstDrawPageWithAssert();

    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // ODF specifies for metal = true specular color as rgb(200,200,200) and adding 15 to specularity
    // Together with extrusion-first-light-level 67% and extrusion-specularity 80% factor is
    // 0.67*0.8 * 200/255 = 0.42 and color #6b6b6b
    assertXPath(pXmlDoc, "(//material)[1]", "specular", "#6b6b6b");
    // 3D specularIntensity = 2^(50/10) + 15 = 47, with default extrusion-shininess 50%
    assertXPath(pXmlDoc, "(//material)[1]", "specularIntensity", "47");
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testExtrusionPhong)
{
    OUString aURL = m_directories.getURLFromSrc(u"svx/qa/unit/data/tdf140321_phong.odp");
    mxComponent = loadFromDesktop(aURL, "com.sun.star.presentation.PresentationDocument");

    SdrPage* pSdrPage = getFirstDrawPageWithAssert();

    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // The rendering method and normals kind were always 'Flat' without the patch.
    assertXPath(pXmlDoc, "//scene", "shadeMode", "Phong");
    assertXPath(pXmlDoc, "//object3Dattributes", "normalsKind", "Specific");
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testSurfaceMattePPT)
{
    OUString aURL = m_directories.getURLFromSrc(u"svx/qa/unit/data/tdf140321_Matte_import.ppt");
    mxComponent = loadFromDesktop(aURL, "com.sun.star.presentation.PresentationDocument");

    SdrPage* pSdrPage = getFirstDrawPageWithAssert();

    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // The preset 'matte' sets the specularity of material to 0. But that alone does not make the
    // rendering 'matte' in LO. To get a 'matte' effect in LO, specularity of the light need to be
    // false in addition. To get this, first light is set off and values from first light are copied
    // to forth light, as only first light is specular. Because first and third lights are off, the
    // forth light is the second one in the dump. The gray color corresponding to
    // FirstLightLevel = 38000/2^16 is #949494.
    assertXPath(pXmlDoc, "(//material)[1]", "specular", "#000000");
    assertXPath(pXmlDoc, "(//light)[2]", "color", "#949494");
    // To make the second light soft, part of its intensity is moved to lights 5,6,7 and 8.
    assertXPath(pXmlDoc, "(//light)[1]", "color", "#1e1e1e");
    assertXPath(pXmlDoc, "(//light)[3]", "color", "#3b3b3b");
    // The 3D property specularIntensity is not related to 'extrusion-specularity' but to
    // 'extrusion-shininess'. specularIntensity = 2^(shininess/10), here default 32.
    assertXPath(pXmlDoc, "(//material)[1]", "specularIntensity", "32");
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testMaterialSpecular)
{
    OUString aURL
        = m_directories.getURLFromSrc(u"svx/qa/unit/data/tdf140321_material_specular.odp");
    mxComponent = loadFromDesktop(aURL, "com.sun.star.presentation.PresentationDocument");

    SdrPage* pSdrPage = getFirstDrawPageWithAssert();

    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);
    CPPUNIT_ASSERT(pXmlDoc);

    // 3D specular color is derived from properties 'extrusion-specularity' and 'extrusion-first-light
    // -level'. 3D specularIntensity is derived from property 'draw:extrusion-shininess'. Both are
    // object properties, not scene properties. Those were wrong in various forms before the patch.
    // Specularity = 77% * first-light-level 67% = 0.5159, which corresponds to gray color #848484.
    assertXPath(pXmlDoc, "(//material)[1]", "specular", "#848484");
    // extrusion-shininess 50% corresponds to 3D specularIntensity 32, use 2^(50/10).
    assertXPath(pXmlDoc, "(//material)[1]", "specularIntensity", "32");
    // extrusion-first-light-level 67% corresponds to gray color #ababab, use 255 * 0.67.
    assertXPath(pXmlDoc, "(//light)[1]", "color", "#ababab");
    // The first light is harsh, the second light soft. So the 3D scene should have 6 lights (1+1+4).
    assertXPath(pXmlDoc, "//light", 6);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testVideoSnapshot)
{
    // Given a slide with a media shape, containing a 4 sec video, red-green-blue-black being the 4
    // seconds:
    OUString aURL = m_directories.getURLFromSrc(u"svx/qa/unit/data/video-snapshot.pptx");
    mxComponent = loadFromDesktop(aURL, "com.sun.star.presentation.PresentationDocument");
    SdrPage* pSdrPage = getFirstDrawPageWithAssert();
    auto pSdrMediaObj = dynamic_cast<SdrMediaObj*>(pSdrPage->GetObj(0));

    // When getting the red snapshot of the video:
    Graphic aSnapshot(pSdrMediaObj->getSnapshot());

    // Then make sure the color is correct:
    const BitmapEx& rBitmap = aSnapshot.GetBitmapExRef();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[ff0000ff]
    // - Actual  : rgba[000000ff]
    // i.e. the preview was black, not red; since we seeked 3 secs into the video, while PowerPoint
    // doesn't do that.
    CPPUNIT_ASSERT_EQUAL(Color(0xff, 0x0, 0x0), rBitmap.GetPixelColor(0, 0));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
