/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>

#include <extendedprimitive2dxmldump.hxx>
#include <rtl/ustring.hxx>
#include <vcl/virdev.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdorect.hxx>
#include <svx/unopage.hxx>
#include <svx/svdview.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnstwit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/itempool.hxx>
#include <svx/svdomedia.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

#include <sdr/contact/objectcontactofobjlistpainter.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/svdraw/ code.
class SvdrawTest : public UnoApiXmlTest
{
public:
    SvdrawTest()
        : UnoApiXmlTest(u"svx/qa/unit/data/"_ustr)
    {
    }

protected:
    SdrPage* getFirstDrawPageWithAssert();
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

    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = aDumper.dumpAndParse(xPrimitiveSequence);
    CPPUNIT_ASSERT(pXmlDoc);
    return pXmlDoc;
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testSemiTransparentText)
{
    // Create a new Draw document with a rectangle.
    mxComponent = loadFromDesktop(u"private:factory/sdraw"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    xShape->setPosition(awt::Point(1000, 1000));

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    xDrawPage->add(xShape);

    // Add semi-transparent text on the rectangle.
    uno::Reference<text::XTextRange> xShapeText(xShape, uno::UNO_QUERY);
    xShapeText->getText()->setString(u"hello"_ustr);

    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue(u"CharColor"_ustr, uno::Any(COL_RED));
    sal_Int16 nTransparence = 75;
    xShapeProperties->setPropertyValue(u"CharTransparence"_ustr, uno::Any(nTransparence));

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
        = getXPath(pDocument, "//unifiedtransparence"_ostr, "transparence"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(nTransparence, fTransparence);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testHandlePathObjScale)
{
    // Given a path object:
    mxComponent = loadFromDesktop(u"private:factory/sdraw"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.ClosedBezierShape"_ustr), uno::UNO_QUERY);

    // When setting its scale by both using setSize() and scaling in a transform matrix:
    // Set size and basic properties.
    xShape->setPosition(awt::Point(2512, 6062));
    xShape->setSize(awt::Size(112, 112));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"FillStyle"_ustr, uno::Any(drawing::FillStyle_SOLID));
    xShapeProps->setPropertyValue(u"LineStyle"_ustr, uno::Any(drawing::LineStyle_SOLID));
    xShapeProps->setPropertyValue(u"FillColor"_ustr, uno::Any(static_cast<sal_Int32>(0)));
    // Add it to the draw page.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
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
    xShapeProps->setPropertyValue(u"PolyPolygonBezier"_ustr, uno::Any(aPolyPolygonBezierCoords));
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
    xShapeProps->setPropertyValue(u"Transformation"_ustr, uno::Any(aMatrix));

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
    mxComponent = loadFromDesktop(u"private:factory/sdraw"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xRect1(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xRect1->setPosition(awt::Point(1000, 1000));
    xRect1->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XShape> xRect2(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xRect2->setPosition(awt::Point(1000, 1000));
    xRect2->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XShapes> xGroup(
        xFactory->createInstance(u"com.sun.star.drawing.GroupShape"_ustr), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xGroupShape(xGroup, uno::UNO_QUERY);
    xDrawPage->add(xGroupShape);
    xGroup->add(xRect1);
    xGroup->add(xRect2);
    uno::Reference<text::XTextRange> xRect2Text(xRect2, uno::UNO_QUERY);
    xRect2Text->setString(u"x"_ustr);
    uno::Sequence<beans::PropertyValue> aGrabBag = {
        comphelper::makePropertyValue(u"OOXLayout"_ustr, true),
    };
    uno::Reference<beans::XPropertySet> xGroupProps(xGroup, uno::UNO_QUERY);
    xGroupProps->setPropertyValue(u"InteropGrabBag"_ustr, uno::Any(aGrabBag));

    // When editing the shape text of the 2nd rectangle (insert a char at the start).
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SdrView* pSdrView = pViewShell->GetDrawView();
    SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xRect2);
    pSdrView->SdrBeginTextEdit(pObject);
    EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
    rEditView.InsertText(u"y"_ustr);
    pSdrView->SdrEndTextEdit();

    // Then make sure that grab-bag is empty to avoid losing the new text.
    xGroupProps->getPropertyValue(u"InteropGrabBag"_ustr) >>= aGrabBag;
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
    rtl::Reference<SdrPage> pPage(new SdrPage(*pModel, false));
    pPage->SetSize(Size(1000, 1000));
    pModel->InsertPage(pPage.get(), 0);

    tools::Rectangle aSize(Point(), Size(100, 100));
    rtl::Reference<SdrRectObj> pRectangle = new SdrRectObj(*pModel, aSize);
    pPage->NbcInsertObject(pRectangle.get());
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

    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = aDumper.dumpAndParse(xPrimitiveSequence);

    assertXPath(pXmlDoc, "/primitive2D"_ostr, 1);

    OString aBasePath("/primitive2D/sdrrectangle/group/polypolygoncolor"_ostr);
    assertXPath(pXmlDoc, aBasePath, "color"_ostr, u"#729fcf"_ustr);

    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "height"_ostr,
                u"99"_ustr); // weird Rectangle is created with size 100
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "width"_ostr, u"99"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "minx"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "miny"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "maxx"_ostr, u"99"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/polypolygon", "maxy"_ostr, u"99"_ustr);

    aBasePath = "/primitive2D/sdrrectangle/group/polypolygoncolor/polypolygon/polygon"_ostr;

    assertXPath(pXmlDoc, aBasePath + "/point", 5);
    assertXPath(pXmlDoc, aBasePath + "/point[1]", "x"_ostr, u"49.5"_ustr); // hmm, weird, why?
    assertXPath(pXmlDoc, aBasePath + "/point[1]", "y"_ostr, u"99"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/point[2]", "x"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/point[2]", "y"_ostr, u"99"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/point[3]", "x"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/point[3]", "y"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/point[4]", "x"_ostr, u"99"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/point[4]", "y"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/point[5]", "x"_ostr, u"99"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/point[5]", "y"_ostr, u"99"_ustr);

    aBasePath = "/primitive2D/sdrrectangle/group/polygonstroke"_ostr;
    assertXPath(pXmlDoc, aBasePath, 1);

    assertXPath(pXmlDoc, aBasePath + "/line", "color"_ostr, u"#3465a4"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/line", "width"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/line", "linejoin"_ostr, u"Round"_ustr);
    assertXPath(pXmlDoc, aBasePath + "/line", "linecap"_ostr, u"BUTT"_ustr);

    assertXPathContent(pXmlDoc, aBasePath + "/polygon", u"49.5,99 0,99 0,0 99,0 99,99"_ustr);

    // If solid line, then there is no line stroke information
    assertXPath(pXmlDoc, aBasePath + "/stroke", 0);

    pPage->RemoveObject(0);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testAutoHeightMultiColShape)
{
    // Given a document containing a shape that has:
    // 1) automatic height (resize shape to fix text)
    // 2) multiple columns (2)
    loadFromFile(u"auto-height-multi-col-shape.pptx");

    // Make sure the in-file shape height is kept, even if nominally the shape height is
    // automatic:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
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
    loadFromFile(u"FontWork.odg");

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

    assertXPath(pXmlDoc, "/primitive2D"_ostr, 1);

    assertXPath(pXmlDoc, "//scene"_ostr, "projectionMode"_ostr, u"Perspective"_ustr);
    assertXPath(pXmlDoc, "//scene/extrude3D[1]/fill"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pXmlDoc, "//scene/extrude3D[1]/object3Dattributes/material"_ostr, "color"_ostr,
                u"#ff0000"_ustr);
    // ODF default 50% is represented by Specular Intensity = 2^5. The relationship is not linear.
    assertXPath(pXmlDoc, "//scene/extrude3D[1]/object3Dattributes/material"_ostr,
                "specularIntensity"_ostr, u"32"_ustr);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testTdf148000_EOLinCurvedText)
{
    std::vector<OUString> aFilenames
        = { u"tdf148000_EOLinCurvedText.pptx"_ustr, u"tdf148000_EOLinCurvedText_New.odp"_ustr,
            u"tdf148000_EOLinCurvedText_Legacy.odp"_ustr };

    for (int i = 0; i < 3; i++)
    {
        loadFromFile(aFilenames[i]);

        SdrPage* pSdrPage = getFirstDrawPageWithAssert();

        xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

        // this is a group shape, hence 2 nested objectinfo
        OString aBasePath = "/primitive2D/objectinfo[4]/objectinfo/unhandled/group/unhandled/group/"
                            "polypolygoncolor/polypolygon/"_ostr;

        // The text is: "O" + eop + "O" + eol + "O"
        // It should be displayed as 3 line of text. (1 "O" letter in every line)
        sal_Int32 nY1 = getXPath(pXmlDoc, aBasePath + "polygon[1]/point[1]", "y"_ostr).toInt32();
        sal_Int32 nY2 = getXPath(pXmlDoc, aBasePath + "polygon[3]/point[1]", "y"_ostr).toInt32();
        sal_Int32 nY3 = getXPath(pXmlDoc, aBasePath + "polygon[5]/point[1]", "y"_ostr).toInt32();

        sal_Int32 nDiff21 = nY2 - nY1;
        sal_Int32 nDiff32 = nY3 - nY2;

        // the 2. "O" must be positioned much lower as the 1. "O". (the eop break the line)
        CPPUNIT_ASSERT_GREATER(sal_Int32(300), nDiff21);
        if (i < 2)
        {
            // the 3. "O" must be positioned even lower with 1 line. (the eol must break the line as well)
            CPPUNIT_ASSERT_LESS(sal_Int32(50), abs(nDiff32 - nDiff21));
        }
        else
        {
            // In legacy mode, the 3. "O" must be positioned about the same high as the 2. "O"
            // the eol not break the line.
            CPPUNIT_ASSERT_LESS(sal_Int32(50), nDiff32);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testTdf148000_CurvedTextWidth)
{
    std::vector<OUString> aFilenames
        = { u"tdf148000_CurvedTextWidth.pptx"_ustr, u"tdf148000_CurvedTextWidth_New.odp"_ustr,
            u"tdf148000_CurvedTextWidth_Legacy.odp"_ustr };

    for (int i = 0; i < 3; i++)
    {
        loadFromFile(aFilenames[i]);

        SdrPage* pSdrPage = getFirstDrawPageWithAssert();

        xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

        OString aBasePath = "/primitive2D/objectinfo[4]/objectinfo/unhandled/group/unhandled/group/"
                            "polypolygoncolor/polypolygon/"_ostr;

        // The text is: 7 line od "OOOOOOO"
        // Take the x coord of the 4 "O" on the corners
        sal_Int32 nX1 = getXPath(pXmlDoc, aBasePath + "polygon[1]/point[1]", "x"_ostr).toInt32();
        sal_Int32 nX2 = getXPath(pXmlDoc, aBasePath + "polygon[13]/point[1]", "x"_ostr).toInt32();
        sal_Int32 nX3 = getXPath(pXmlDoc, aBasePath + "polygon[85]/point[1]", "x"_ostr).toInt32();
        sal_Int32 nX4 = getXPath(pXmlDoc, aBasePath + "polygon[97]/point[1]", "x"_ostr).toInt32();

        if (i < 2)
        {
            // All the lines should be positioned similar (start/end is similar)
            CPPUNIT_ASSERT_LESS(sal_Int32(150), abs(nX3 - nX1));
            CPPUNIT_ASSERT_LESS(sal_Int32(150), abs(nX4 - nX2));
        }
        else
        {
            // In legacy mode, the outer lines become much wider
            CPPUNIT_ASSERT_GREATER(sal_Int32(1500), nX3 - nX1);
            CPPUNIT_ASSERT_GREATER(sal_Int32(1500), nX2 - nX4);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testSurfaceMetal)
{
    loadFromFile(u"tdf140321_metal.odp");

    SdrPage* pSdrPage = getFirstDrawPageWithAssert();

    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // ODF specifies for metal = true specular color as rgb(200,200,200) and adding 15 to specularity
    // Together with extrusion-first-light-level 67% and extrusion-specularity 80% factor is
    // 0.67*0.8 * 200/255 = 0.42 and color #6b6b6b
    assertXPath(pXmlDoc, "(//material)[1]"_ostr, "specular"_ostr, u"#6b6b6b"_ustr);
    // 3D specularIntensity = 2^(50/10) + 15 = 47, with default extrusion-shininess 50%
    assertXPath(pXmlDoc, "(//material)[1]"_ostr, "specularIntensity"_ostr, u"47"_ustr);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testExtrusionPhong)
{
    loadFromFile(u"tdf140321_phong.odp");

    SdrPage* pSdrPage = getFirstDrawPageWithAssert();

    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // The rendering method and normals kind were always 'Flat' without the patch.
    assertXPath(pXmlDoc, "//scene"_ostr, "shadeMode"_ostr, u"Phong"_ustr);
    assertXPath(pXmlDoc, "//object3Dattributes"_ostr, "normalsKind"_ostr, u"Specific"_ustr);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testSurfaceMattePPT)
{
    loadFromFile(u"tdf140321_Matte_import.ppt");

    SdrPage* pSdrPage = getFirstDrawPageWithAssert();

    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // The preset 'matte' sets the specularity of material to 0. But that alone does not make the
    // rendering 'matte' in LO. To get a 'matte' effect in LO, specularity of the light need to be
    // false in addition. To get this, first light is set off and values from first light are copied
    // to forth light, as only first light is specular. Because first and third lights are off, the
    // forth light is the second one in the dump. The gray color corresponding to
    // FirstLightLevel = 38000/2^16 is #949494.
    assertXPath(pXmlDoc, "(//material)[1]"_ostr, "specular"_ostr, u"#000000"_ustr);
    assertXPath(pXmlDoc, "(//light)[2]"_ostr, "color"_ostr, u"#949494"_ustr);
    // To make the second light soft, part of its intensity is moved to lights 5,6,7 and 8.
    assertXPath(pXmlDoc, "(//light)[1]"_ostr, "color"_ostr, u"#1e1e1e"_ustr);
    assertXPath(pXmlDoc, "(//light)[3]"_ostr, "color"_ostr, u"#3b3b3b"_ustr);
    // The 3D property specularIntensity is not related to 'extrusion-specularity' but to
    // 'extrusion-shininess'. specularIntensity = 2^(shininess/10), here default 32.
    assertXPath(pXmlDoc, "(//material)[1]"_ostr, "specularIntensity"_ostr, u"32"_ustr);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testMaterialSpecular)
{
    loadFromFile(u"tdf140321_material_specular.odp");

    SdrPage* pSdrPage = getFirstDrawPageWithAssert();

    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);
    CPPUNIT_ASSERT(pXmlDoc);

    // 3D specular color is derived from properties 'extrusion-specularity' and 'extrusion-first-light
    // -level'. 3D specularIntensity is derived from property 'draw:extrusion-shininess'. Both are
    // object properties, not scene properties. Those were wrong in various forms before the patch.
    // Specularity = 77% * first-light-level 67% = 0.5159, which corresponds to gray color #848484.
    assertXPath(pXmlDoc, "(//material)[1]"_ostr, "specular"_ostr, u"#848484"_ustr);
    // extrusion-shininess 50% corresponds to 3D specularIntensity 32, use 2^(50/10).
    assertXPath(pXmlDoc, "(//material)[1]"_ostr, "specularIntensity"_ostr, u"32"_ustr);
    // extrusion-first-light-level 67% corresponds to gray color #ababab, use 255 * 0.67.
    assertXPath(pXmlDoc, "(//light)[1]"_ostr, "color"_ostr, u"#ababab"_ustr);
    // The first light is harsh, the second light soft. So the 3D scene should have 6 lights (1+1+4).
    assertXPath(pXmlDoc, "//light"_ostr, 6);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testVideoSnapshot)
{
    // Given a slide with a media shape, containing a 4 sec video, red-green-blue-black being the 4
    // seconds:
    loadFromFile(u"video-snapshot.pptx");
    SdrPage* pSdrPage = getFirstDrawPageWithAssert();
    auto pSdrMediaObj = dynamic_cast<SdrMediaObj*>(pSdrPage->GetObj(0));

    // When getting the red snapshot of the video:
    Graphic aSnapshot(pSdrMediaObj->getSnapshot());

    // Then make sure the color is correct:
    const BitmapEx& rBitmap = aSnapshot.GetBitmapExRef();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[ff0000ff]
    // - Actual  : rgba[000000ff]
    // i.e. the preview was black, not ~red; since we seeked 3 secs into the video, while PowerPoint
    // doesn't do that.
    CPPUNIT_ASSERT_EQUAL(Color(0xfe, 0x0, 0x0), rBitmap.GetPixelColor(0, 0));

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 321
    // - Actual  : 640
    // i.e. ~25% crop from left and right should result in half width, but it was not reduced.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(321), rBitmap.GetSizePixel().getWidth());
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testPageViewDrawLayerClip)
{
    // Given a document with 2 pages, first page footer has an off-page line shape:
    loadFromFile(u"page-view-draw-layer-clip.docx");

    // When saving that document to PDF:
    save(u"writer_pdf_Export"_ustr);

    // Then make sure that line shape gets clipped:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pDoc = parsePDFExport();
    if (!pDoc)
    {
        return;
    }
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage1 = pDoc->openPage(0);
    CPPUNIT_ASSERT_EQUAL(3, pPage1->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage2 = pDoc->openPage(1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. the 2nd page had a line shape from the first page's footer.
    CPPUNIT_ASSERT_EQUAL(2, pPage2->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testRectangleObjectMove)
{
    std::unique_ptr<SdrModel> pModel(new SdrModel(nullptr, nullptr, true));
    rtl::Reference<SdrPage> pPage(new SdrPage(*pModel, false));
    pPage->SetSize(Size(50000, 50000));
    pModel->InsertPage(pPage.get(), 0);

    tools::Rectangle aRect(Point(), Size(100, 100));
    rtl::Reference<SdrRectObj> pRectangleObject = new SdrRectObj(*pModel, aRect);
    pPage->NbcInsertObject(pRectangleObject.get());

    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(), Size(100, 100)),
                         pRectangleObject->GetLogicRect());
    pRectangleObject->NbcMove({ 100, 100 });
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(100, 100), Size(100, 100)),
                         pRectangleObject->GetLogicRect());

    pPage->RemoveObject(0);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testRectangleObjectRotate)
{
    std::unique_ptr<SdrModel> pModel(new SdrModel(nullptr, nullptr, true));
    rtl::Reference<SdrPage> pPage(new SdrPage(*pModel, false));
    pPage->SetSize(Size(50000, 50000));
    pModel->InsertPage(pPage.get(), 0);

    {
        tools::Rectangle aObjectSize(Point(), Size(100, 100));
        rtl::Reference<SdrRectObj> pRectangleObject = new SdrRectObj(*pModel, aObjectSize);
        pPage->NbcInsertObject(pRectangleObject.get());

        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 0), Size(100, 100)),
                             pRectangleObject->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 0), Size(100, 100)),
                             pRectangleObject->GetSnapRect());
        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(-1, -1), Size(102, 102)),
                             pRectangleObject->GetCurrentBoundRect());

        auto angle = 9000_deg100;
        double angleRadians = toRadians(angle);
        pRectangleObject->NbcRotate(aObjectSize.Center(), angle, std::sin(angleRadians),
                                    std::cos(angleRadians));

        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 98), Size(100, 100)),
                             pRectangleObject->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, -1), Size(100, 100)),
                             pRectangleObject->GetSnapRect());
        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(-1, -2), Size(102, 102)),
                             pRectangleObject->GetCurrentBoundRect());

        pPage->RemoveObject(0);
    }

    {
        tools::Rectangle aObjectSize(Point(), Size(100, 100));
        rtl::Reference<SdrRectObj> pRectangleObject = new SdrRectObj(*pModel, aObjectSize);
        pPage->NbcInsertObject(pRectangleObject.get());

        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 0), Size(100, 100)),
                             pRectangleObject->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 0), Size(100, 100)),
                             pRectangleObject->GetSnapRect());
        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(-1, -1), Size(102, 102)),
                             pRectangleObject->GetCurrentBoundRect());

        auto angle = -4500_deg100;
        double angleRadians = toRadians(angle);
        pRectangleObject->NbcRotate(aObjectSize.Center(), angle, std::sin(angleRadians),
                                    std::cos(angleRadians));

        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(49, -20), Size(100, 100)),
                             pRectangleObject->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(-21, -20), Size(141, 141)),
                             pRectangleObject->GetSnapRect());
        CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(-22, -21), Size(143, 143)),
                             pRectangleObject->GetCurrentBoundRect());

        pPage->RemoveObject(0);
    }
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testRotatePoint)
{
    {
        auto angle = 18000_deg100;
        double angleRadians = toRadians(angle);
        Point aPoint(2000, 1000);
        Point aReference(1000, 1000);
        RotatePoint(aPoint, aReference, std::sin(angleRadians), std::cos(angleRadians));

        CPPUNIT_ASSERT_EQUAL(Point(0, 1000), aPoint);
    }

    {
        auto angle = 9000_deg100;
        double angleRadians = toRadians(angle);
        Point aPoint(2000, 1000);
        Point aReference(1000, 1000);
        RotatePoint(aPoint, aReference, std::sin(angleRadians), std::cos(angleRadians));

        CPPUNIT_ASSERT_EQUAL(Point(1000, 0), aPoint);
    }

    {
        auto angle = 18000_deg100;
        double angleRadians = toRadians(angle);
        Point aPoint(100, 100);
        Point aReference(200, 200);
        RotatePoint(aPoint, aReference, std::sin(angleRadians), std::cos(angleRadians));

        CPPUNIT_ASSERT_EQUAL(Point(300, 300), aPoint);
    }
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testClipVerticalTextOverflow)
{
    // File contains a slide with 4 rectangle shapes with text inside
    // each have <a:bodyPr vertOverflow="clip">
    // 1-) Text overflowing the rectangle
    // 2-) Text not overflowing the rectangle
    // 3-) (Vertical text) Text overflowing the rectangle
    // 4-) (Vertical text) Text not overflowing the rectangle
    loadFromFile(u"clip-vertical-overflow.pptx");

    SdrPage* pSdrPage = getFirstDrawPageWithAssert();
    xmlDocUniquePtr pDocument = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // Test vertically overflowing text
    // Without the accompanying fix in place, this test would have failed with:
    // equality assertion failed
    // - Expected: 6
    // - Actual  : 13
    // - In <>, XPath contents of child does not match
    // i.e. the vertically overflowing text wasn't clipped & overflowing text
    // was drawn anyways.
    assertXPathContent(pDocument, "count((//sdrblocktext)[4]//textsimpleportion)"_ostr, u"6"_ustr);

    // make sure text is aligned correctly after the overflowing text is clipped
    assertXPath(pDocument, "((//sdrblocktext)[4]//textsimpleportion)[1]"_ostr, "y"_ostr,
                u"3749"_ustr);
    assertXPath(pDocument, "((//sdrblocktext)[4]//textsimpleportion)[6]"_ostr, "y"_ostr,
                u"7559"_ustr);

    // make sure the text that isn't overflowing is still aligned properly
    assertXPathContent(pDocument, "count((//sdrblocktext)[5]//textsimpleportion)"_ostr, u"3"_ustr);
    assertXPath(pDocument, "((//sdrblocktext)[5]//textsimpleportion)[1]"_ostr, "y"_ostr,
                u"5074"_ustr);
    assertXPath(pDocument, "((//sdrblocktext)[5]//textsimpleportion)[3]"_ostr, "y"_ostr,
                u"6598"_ustr);

    // Test vertically overflowing text, with vertical text direction
    assertXPathContent(pDocument, "count((//sdrblocktext)[6]//textsimpleportion)"_ostr, u"12"_ustr);
    // make sure text is aligned correctly after the overflowing text is clipped
    assertXPath(pDocument, "((//sdrblocktext)[6]//textsimpleportion)[1]"_ostr, "x"_ostr,
                u"13093"_ustr);
    assertXPath(pDocument, "((//sdrblocktext)[6]//textsimpleportion)[12]"_ostr, "x"_ostr,
                u"4711"_ustr);

    // make sure the text that isn't overflowing is still aligned properly
    assertXPathContent(pDocument, "count((//sdrblocktext)[7]//textsimpleportion)"_ostr, u"3"_ustr);
    assertXPath(pDocument, "((//sdrblocktext)[7]//textsimpleportion)[1]"_ostr, "x"_ostr,
                u"25417"_ustr);
    assertXPath(pDocument, "((//sdrblocktext)[7]//textsimpleportion)[3]"_ostr, "x"_ostr,
                u"23893"_ustr);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testContourText)
{
    loadFromFile(u"tdf84507_polygoncontourtext.fodg");
    SdrPage* pSdrPage = getFirstDrawPageWithAssert();
    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // The shape is rotated by 180°. The rotated shape has position (10000|12000) and size 6000x4000.
    // Text should be inside the shape and start at the bottom-right of the shape because of 180°
    // rotation. Without fix the text was rotated but positioned left-top of the shape. The first
    // line of text has started at (10000|7353), last line at (10000|5007).
    assertXPath(pXmlDoc, "(//textsimpleportion)[1]"_ostr, "x"_ostr, "15998");
    assertXPath(pXmlDoc, "(//textsimpleportion)[1]"_ostr, "y"_ostr, "11424");
    assertXPath(pXmlDoc, "(//textsimpleportion)[4]"_ostr, "x"_ostr, "15998");
    assertXPath(pXmlDoc, "(//textsimpleportion)[4]"_ostr, "y"_ostr, "9291");
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testContourTextCJK)
{
    loadFromFile(u"tdf128433_rectanglecontourtext_CJK.fodg");
    SdrPage* pSdrPage = getFirstDrawPageWithAssert();
    xmlDocUniquePtr pXmlDoc = lcl_dumpAndParseFirstObjectWithAssert(pSdrPage);

    // The rectangle has position (10000|4000) and size 4000x6000. The text in the rectangle is set
    // to tb-rl writing mode. Without fix the text was positioned left from the shape. The first line
    // of text has started at (9327|4000), the last line at (8489|4000).
    // The expected values are for font "Microsoft Yahei". Substitute fonts can have a different
    // metric despite having the same font size. Thus test with tolerance.
    // First line
    assertXPathDoubleValue(pXmlDoc, "(//textsimpleportion)[1]"_ostr, "x"_ostr, 13327.0, 150.0);
    // Second line
    assertXPathDoubleValue(pXmlDoc, "(//textsimpleportion)[3]"_ostr, "x"_ostr, 12489.0, 300.0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
