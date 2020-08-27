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
#include <rtl/ustring.hxx>
#include <editeng/unoprnms.hxx>

#include <cppunit/TestAssert.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/Desktop.hpp>

using namespace ::com::sun::star;

namespace
{
const OUStringLiteral sDataDirectory(u"svx/qa/unit/data/");

/// Tests not about special features of custom shapes, but about shapes in general.
class ClassicshapesTest : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
    uno::Reference<lang::XComponent> mxComponent;
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex, sal_uInt8 nPageIndex);

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
};

uno::Reference<drawing::XShape> ClassicshapesTest::getShape(sal_uInt8 nShapeIndex,
                                                            sal_uInt8 nPageIndex)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(nPageIndex),
                                                 uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(nShapeIndex), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get xShape", xShape.is());
    return xShape;
}

CPPUNIT_TEST_FIXTURE(ClassicshapesTest, testTdf98584ShearVertical)
{
    // The document contains draw:rect, draw:polygon and draw:path objects.
    // They are vertical sheared by skewY(-0.927295218002) or by matrix(1 2 0 1 1cm 1cm).
    // Notice, skewY and matrix are interpreted on file open, but not written on file save.
    // They are converted to rotate * shear horizontal * scale.
    // Besides using a wrong sign in shear angle, error was, that TRSetGeometry of SdrPathObj did
    // not consider the additional scaling (tdf#98565).
    const OUString sURL(m_directories.getURLFromSrc(sDataDirectory) + "tdf98584_ShearVertical.odg");
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());

    OUString sErrors; // sErrors collects the errors and should be empty in case all is OK.
    // All tests have a small tolerance for to avoid failing because of rounding errors.

    // Tests skewY
    sal_Int32 nShearE = -5313; // expected angle for horizontal shear
    sal_Int32 nRotE = 30687; // = -5313 expected angle for generated rotation
    // expected width of shape, should not change on vertical shearing
    sal_Int32 nWidthE = 5001;

    for (sal_uInt8 nPageIndex = 0; nPageIndex < 3; ++nPageIndex)
    {
        awt::Rectangle aFrameRect;
        uno::Reference<drawing::XShape> xShape(getShape(0, nPageIndex));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRect;
        const sal_Int32 nWidthA(aFrameRect.Width);
        if (abs(nWidthE - nWidthA) > 2)
            sErrors += "skewY page " + OUString::number(nPageIndex) + " width expected "
                       + OUString::number(nWidthE) + ", actual " + OUString::number(nWidthA) + "\n";
        sal_Int32 nShearA(0);
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_SHEARANGLE) >>= nShearA;
        if (abs(nShearE - nShearA) > 2)
            sErrors += "skewY page" + OUString::number(nPageIndex) + " shear angle expected "
                       + OUString::number(nShearE) + ", actual " + OUString::number(nShearA) + "\n";
        sal_Int32 nRotA(0);
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nRotA;
        if (abs(nRotE - nRotA) > 2)
            sErrors += "skewY page" + OUString::number(nPageIndex) + " rotate angle expected "
                       + OUString::number(nRotE) + ", actual " + OUString::number(nRotA) + "\n";
    }

    // Tests matrix
    nShearE = -6343;
    nRotE = 29657;
    nWidthE = 5001;

    for (sal_uInt8 nPageIndex = 3; nPageIndex < 6; ++nPageIndex)
    {
        awt::Rectangle aFrameRect;
        uno::Reference<drawing::XShape> xShape(getShape(0, nPageIndex));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRect;
        const sal_Int32 nWidthA(aFrameRect.Width);
        if (abs(nWidthE - nWidthA) > 2)
            sErrors += "matrix page " + OUString::number(nPageIndex) + " width expected "
                       + OUString::number(nWidthE) + ", actual " + OUString::number(nWidthA) + "\n";
        sal_Int32 nShearA(0);
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_SHEARANGLE) >>= nShearA;
        if (abs(nShearE - nShearA) > 2)
            sErrors += "matrix page" + OUString::number(nPageIndex) + " shear angle expected "
                       + OUString::number(nShearE) + ", actual " + OUString::number(nShearA) + "\n";
        sal_Int32 nRotA(0);
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nRotA;
        if (abs(nRotE - nRotA) > 2)
            sErrors += "matrix page" + OUString::number(nPageIndex) + " rotate angle expected "
                       + OUString::number(nRotE) + ", actual " + OUString::number(nRotA) + "\n";
    }

    CPPUNIT_ASSERT_EQUAL(OUString(), sErrors);
}

CPPUNIT_TEST_FIXTURE(ClassicshapesTest, testTdf98583ShearHorizontal)
{
    // The document contains rectangles with LT 3000,5000 and RB 5000,9000.
    // skewX (-0.78539816339744830961) = skewX(-45deg) is applied on the first page
    // matrix(1 0 1 1 0cm 0cm) on the second page. Both should result in a parallelogram with
    // LT 8000,5000 and RB 14000, 9000, which means width 6001, height 4001.
    // Error was, that not the mathematical matrix was used, but the API matrix, which has
    // wrong sign in shear angle.
    const OUString sURL(m_directories.getURLFromSrc(sDataDirectory)
                        + "tdf98583_ShearHorizontal.odp");
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.presentation.PresentationDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());

    OUString sErrors; // sErrors collects the errors and should be empty in case all is OK.
    // All tests have a small tolerance for to avoid failing because of rounding errors.
    const sal_Int32 nLeftE(8000); // expected values
    const sal_Int32 nTopE(5000);
    const sal_Int32 nWidthE(6001);
    const sal_Int32 nHeightE(4001);
    for (sal_uInt8 nPageIndex = 0; nPageIndex < 2; ++nPageIndex)
    {
        awt::Rectangle aFrameRect;
        uno::Reference<drawing::XShape> xShape(getShape(0, nPageIndex));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRect;
        const sal_Int32 nLeftA(aFrameRect.X);
        const sal_Int32 nTopA(aFrameRect.Y);
        const sal_Int32 nWidthA(aFrameRect.Width);
        const sal_Int32 nHeightA(aFrameRect.Height);
        if (abs(nLeftE - nLeftA) > 2 || abs(nTopE - nTopA) > 2)
            sErrors += "page " + OUString::number(nPageIndex) + " LT expected "
                       + OUString::number(nLeftE) + " | " + OUString::number(nTopE) + ", actual "
                       + OUString::number(nLeftA) + " | " + OUString::number(nTopA) + "\n";
        if (abs(nWidthE - nWidthA) > 2 || abs(nHeightE - nHeightA) > 2)
            sErrors += "page " + OUString::number(nPageIndex) + " width x height expected "
                       + OUString::number(nWidthE) + " x " + OUString::number(nHeightE)
                       + ", actual " + OUString::number(nWidthA) + " x "
                       + OUString::number(nHeightA) + "\n";
    }

    CPPUNIT_ASSERT_EQUAL(OUString(), sErrors);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
