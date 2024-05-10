/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <cppunit/TestAssert.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests related to glue points defined in the custom shape geometry.
class GluePointTest : public UnoApiTest
{
public:
    GluePointTest()
        : UnoApiTest(u"svx/qa/unit/data/"_ustr)
    {
    }

protected:
    // get shape nShapeIndex from page 0
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex);
};

uno::Reference<drawing::XShape> GluePointTest::getShape(sal_uInt8 nShapeIndex)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(nShapeIndex), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get xShape", xShape.is());
    return xShape;
}

// Glue points from custom shape geometry. Values are relative to viewBox.
// Usable if values are constant and not calculated by formula.
bool lcl_getGeometryGluePoints(
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair>& rGluePoints,
    const uno::Reference<drawing::XShape>& xShape)
{
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY_THROW);
    uno::Any anotherAny = xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr);
    uno::Sequence<beans::PropertyValue> aCustomShapeGeometry;
    if (!(anotherAny >>= aCustomShapeGeometry))
        return false;
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aCustomShapeGeometry)
    {
        if (rProp.Name == "Path")
        {
            rProp.Value >>= aPathProps;
            break;
        }
    }

    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "GluePoints")
        {
            rProp.Value >>= rGluePoints;
            break;
        }
    }
    if (rGluePoints.getLength() > 0)
        return true;
    else
        return false;
}

CPPUNIT_TEST_FIXTURE(GluePointTest, testTdf157543_5PointStar)
{
    loadFromFile(u"tdf157543_5PointStar.ppt");
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aGluePoints;
    CPPUNIT_ASSERT(lcl_getGeometryGluePoints(aGluePoints, getShape(0)));
    // Without fix only two glue points exist.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aGluePoints.getLength());
    // coordinates according "Microsoft Office Drawing 97-2007 Binary Format Specification"
    sal_Int32 aExpectedX[] = { 10800, 0, 4200, 17400, 21600 };
    sal_Int32 aExpectedY[] = { 0, 8259, 21600, 21600, 8259 };
    for (sal_uInt8 i = 0; i < 5; i++)
    {
        sal_Int32 aActualX;
        aGluePoints[i].First.Value >>= aActualX;
        sal_Int32 aActualY;
        aGluePoints[i].Second.Value >>= aActualY;
        CPPUNIT_ASSERT_EQUAL(aExpectedX[i], aActualX);
        CPPUNIT_ASSERT_EQUAL(aExpectedY[i], aActualY);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
