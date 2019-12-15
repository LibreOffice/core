/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>
#include <test/drawing/captionshape.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/Point.hpp>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void CaptionShape::testCaptionShapeProperties()
{
    uno::Reference<beans::XPropertySet> xCaptionShape(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "CornerRadius";
    sal_Int32 nCornerRadiusGet = 5;
    sal_Int32 nCornerRadiusSet = 0;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue CornerRadius",
                           xCaptionShape->getPropertyValue(propName) >>= nCornerRadiusGet);

    aNewValue <<= nCornerRadiusGet + 5;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= nCornerRadiusSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue CornerRadius", nCornerRadiusGet + 5,
                                 nCornerRadiusSet);

    propName = "CaptionPoint";
    awt::Point aCaptionPointGet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue of CaptionPoint",
                           xCaptionShape->getPropertyValue(propName) >>= aCaptionPointGet);

    awt::Point aCaptionPointSet;
    aNewValue <<= awt::Point(5, 5);
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= aCaptionPointSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Able to set PropertyValue of CaptionPoint", aCaptionPointSet,
                                 awt::Point(5, 5));

    propName = "CaptionType";
    sal_Int16 captionTypeGet = 0;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue of CaptionType",
                           xCaptionShape->getPropertyValue(propName) >>= captionTypeGet);

    sal_Int16 captionTypeSet = 0;
    const sal_Int16 FIVE = 5;
    aNewValue <<= FIVE;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionTypeSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionType", FIVE,
                                 captionTypeSet);

    propName = "CaptionIsFixedAngle";
    bool captionIsFixedAngleGet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue of CaptionPointIsFixedAngle",
                           xCaptionShape->getPropertyValue(propName) >>= captionIsFixedAngleGet);

    bool captionIsFixedAngleSet = false;
    aNewValue <<= !captionIsFixedAngleGet;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionIsFixedAngleSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionPointisFixedAngle",
                                 captionIsFixedAngleSet, !captionIsFixedAngleGet);

    propName = "CaptionAngle";
    sal_Int32 captionAngleGet = {};
    CPPUNIT_ASSERT_MESSAGE("Unable to get the PropertyValue of CaptionAngle",
                           xCaptionShape->getPropertyValue(propName) >>= captionAngleGet);

    sal_Int32 captionAngleSet = 0;
    aNewValue <<= captionAngleGet + 5;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionAngleSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionAngle", captionAngleSet,
                                 captionAngleGet + 5);

    propName = "CaptionGap";
    sal_Int32 captionGapGet = {};
    CPPUNIT_ASSERT_MESSAGE("Unable to get the PropertyValue of CaptionGap",
                           xCaptionShape->getPropertyValue(propName) >>= captionGapGet);

    sal_Int32 captionGapSet = 0;
    aNewValue <<= captionGapGet + 5;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionGapSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionGap", captionGapSet,
                                 captionGapGet + 5);

    propName = "CaptionEscapeDirection";
    sal_Int32 captionEscapeDirectionGet = {};
    CPPUNIT_ASSERT_MESSAGE("Unable to get the PropertyValue of CaptionEscapeDirection",
                           xCaptionShape->getPropertyValue(propName) >>= captionEscapeDirectionGet);

    sal_Int32 captionEscapeDirectionSet = 0;
    aNewValue <<= captionEscapeDirectionGet + 5;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionEscapeDirectionSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionEscapeDirection",
                                 captionEscapeDirectionSet, captionEscapeDirectionGet + 5);

    propName = "CaptionIsEscapeRelative";
    bool captionIsEscapeRelativeGet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue of CaptionIsEscapeRelative",
                           xCaptionShape->getPropertyValue(propName)
                           >>= captionIsEscapeRelativeGet);

    bool captionIsEscapeRelativeSet = false;
    aNewValue <<= !captionIsEscapeRelativeGet;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionIsEscapeRelativeSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionIsEscapeRelative",
                                 captionIsEscapeRelativeSet, !captionIsEscapeRelativeGet);

    propName = "CaptionEscapeRelative";
    sal_Int32 captionEscapeRelativeGet = {};
    CPPUNIT_ASSERT_MESSAGE("Unable to get the PropertyValue of CaptionEscapeRelative",
                           xCaptionShape->getPropertyValue(propName) >>= captionEscapeRelativeGet);

    sal_Int32 captionEscapeRelativeSet = 0;
    aNewValue <<= captionEscapeRelativeGet + 5;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionEscapeRelativeSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionEscapeRelative",
                                 captionEscapeRelativeSet, captionEscapeRelativeGet + 5);

    propName = "CaptionEscapeAbsolute";
    sal_Int32 captionEscapeAbsoluteGet = {};
    CPPUNIT_ASSERT_MESSAGE("Unable to get the PropertyValue of CaptionEscapeAbsolute",
                           xCaptionShape->getPropertyValue(propName) >>= captionEscapeAbsoluteGet);

    sal_Int32 captionEscapeAbsoluteSet = 0;
    aNewValue <<= captionEscapeAbsoluteGet + 5;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionEscapeAbsoluteSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionEscapeAbsolute",
                                 captionEscapeAbsoluteSet, captionEscapeAbsoluteGet + 5);

    propName = "CaptionLineLength";
    sal_Int32 captionLineLengthGet = {};
    CPPUNIT_ASSERT_MESSAGE("Unable to get the PropertyValue of CaptionLineLength",
                           xCaptionShape->getPropertyValue(propName) >>= captionLineLengthGet);

    sal_Int32 captionLineLengthSet = 0;
    aNewValue <<= captionLineLengthGet + 5;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionLineLengthSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionEscapeAbsolute",
                                 captionLineLengthSet, captionLineLengthGet + 5);

    propName = "CaptionIsFitLineLength";
    bool captionIsFitLineLengthGet = false;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue of CaptionIsFitLineLength",
                           xCaptionShape->getPropertyValue(propName) >>= captionIsFitLineLengthGet);

    bool captionIsFitLineLengthSet = false;
    aNewValue <<= !captionIsFitLineLengthGet;
    xCaptionShape->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xCaptionShape->getPropertyValue(propName) >>= captionIsFitLineLengthSet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set the PropertyValue of CaptionIsFitLineLength",
                                 captionIsFitLineLengthSet, !captionIsFitLineLengthGet);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
