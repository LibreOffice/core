/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <rtl/ustring.hxx>
#include <editeng/unoprnms.hxx>

#include <cppunit/TestAssert.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests not about special features of custom shapes, but about shapes in general.
class ClassicshapesTest : public UnoApiTest
{
public:
    ClassicshapesTest()
        : UnoApiTest(u"svx/qa/unit/data/"_ustr)
    {
    }

protected:
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex, sal_uInt8 nPageIndex);
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
    loadFromFile(u"tdf98584_ShearVertical.odg");

    // Tests skewY
    for (sal_uInt8 nPageIndex = 0; nPageIndex < 3; ++nPageIndex)
    {
        awt::Rectangle aFrameRect;
        uno::Reference<drawing::XShape> xShape(getShape(0, nPageIndex));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRect;
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Width on skewY page " + OUString::number(nPageIndex))
                .toUtf8()
                .getStr(),
            5001.0, aFrameRect.Width, 2.0);
        double nShearA = {};
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_SHEARANGLE) >>= nShearA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Share angle on skewY page " + OUString::number(nPageIndex))
                .toUtf8()
                .getStr(),
            -5313.0, nShearA, 2.0);
        double nRotA = {};
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nRotA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Rotate angle on skewY page " + OUString::number(nPageIndex))
                .toUtf8()
                .getStr(),
            30687.0, nRotA, 2.0);
    }

    // Tests matrix
    for (sal_uInt8 nPageIndex = 3; nPageIndex < 6; ++nPageIndex)
    {
        awt::Rectangle aFrameRect;
        uno::Reference<drawing::XShape> xShape(getShape(0, nPageIndex));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRect;
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Width on matrix page " + OUString::number(nPageIndex))
                .toUtf8()
                .getStr(),
            5001.0, aFrameRect.Width, 2.0);
        double nShearA = {};
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_SHEARANGLE) >>= nShearA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Share angle on matrix page " + OUString::number(nPageIndex))
                .toUtf8()
                .getStr(),
            -6343.0, nShearA, 2.0);
        double nRotA = {};
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nRotA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Rotate angle on matrix page " + OUString::number(nPageIndex))
                .toUtf8()
                .getStr(),
            29657.0, nRotA, 2.0);
    }
}

CPPUNIT_TEST_FIXTURE(ClassicshapesTest, testTdf98583ShearHorizontal)
{
    // The document contains rectangles with LT 3000,5000 and RB 5000,9000.
    // skewX (-0.78539816339744830961) = skewX(-45deg) is applied on the first page
    // matrix(1 0 1 1 0cm 0cm) on the second page. Both should result in a parallelogram with
    // LT 8000,5000 and RB 14000, 9000, which means width 6001, height 4001.
    // Error was, that not the mathematical matrix was used, but the API matrix, which has
    // wrong sign in shear angle.
    loadFromFile(u"tdf98583_ShearHorizontal.odp");

    for (sal_uInt8 nPageIndex = 0; nPageIndex < 2; ++nPageIndex)
    {
        awt::Rectangle aFrameRect;
        uno::Reference<drawing::XShape> xShape(getShape(0, nPageIndex));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
        xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRect;

        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Left Position on page " + OUString::number(nPageIndex))
                .toUtf8()
                .getStr(),
            8000.0, aFrameRect.X, 2.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Top Position on page " + OUString::number(nPageIndex))
                .toUtf8()
                .getStr(),
            5000.0, aFrameRect.Y, 2.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Width on page " + OUString::number(nPageIndex)).toUtf8().getStr(),
            6001.0, aFrameRect.Width, 2.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OUString("Incorrect Height on page " + OUString::number(nPageIndex)).toUtf8().getStr(),
            4001.0, aFrameRect.Height, 2.0);
    }
}

CPPUNIT_TEST_FIXTURE(ClassicshapesTest, testTdf130076Flip)
{
    // The document contains sections of a circle, one of which is scaled
    // (1, -1), one of which is scaled (-1,1), one of which is transformed
    // by a matrix equivalent to a vertical flip, and another which is
    // transformed by a matrix equivalent to a horizontal flip. Error was
    // that the transformation was made before the CircleKind was set,
    // resulting in the flip being performed incorrectly.
    loadFromFile(u"tdf130076_FlipOnSectorSection.odg");

    for (sal_uInt8 nPageIndex = 0; nPageIndex < 2; ++nPageIndex)
    {
        double nAngle1(0.0), nAngle2(0.0);
        uno::Reference<drawing::XShape> xShape(getShape(1, nPageIndex));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xShape2(getShape(2, nPageIndex));
        uno::Reference<beans::XPropertySet> xShapeProps2(xShape2, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"CircleStartAngle"_ustr) >>= nAngle1);
        CPPUNIT_ASSERT(xShapeProps2->getPropertyValue(u"CircleStartAngle"_ustr) >>= nAngle2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OUString("Incorrect vertical flip starting angle on page "
                                              + OUString::number(nPageIndex))
                                         .toUtf8()
                                         .getStr(),
                                     26000.0, nAngle1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OUString("Incorrect horizontal flip starting angle on page "
                                              + OUString::number(nPageIndex))
                                         .toUtf8()
                                         .getStr(),
                                     26000.0, nAngle2);
    }
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
