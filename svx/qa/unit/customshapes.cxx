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
const OUString sDataDirectory("/svx/qa/unit/data/");

/// Tests for svx/source/customshapes/ code.
class CustomshapesTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<lang::XComponent> mxComponent;

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

    void testViewBoxLeftTop();

    CPPUNIT_TEST_SUITE(CustomshapesTest);
    CPPUNIT_TEST(testViewBoxLeftTop);
    CPPUNIT_TEST_SUITE_END();
};

void CustomshapesTest::testViewBoxLeftTop()
{
    // tdf#121890 formula values "left" and "top" are wrongly calculated
    // Load a document with two custom shapes of type "non-primitive"
    OUString aURL
        = m_directories.getURLFromSrc(sDataDirectory) + "viewBox_positive_twolines_strict.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);

    // Get the shape "leftright". Error was, that the identifier "left" was always set to zero, thus
    // the path was outside the frame rectangle for a viewBox having a positive "left" value.
    uno::Reference<drawing::XShape> xShapeLR(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape 'leftright'", xShapeLR.is());
    uno::Reference<beans::XPropertySet> xShapeLRProps(xShapeLR, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape 'leftright' properties", xShapeLRProps.is());
    awt::Rectangle aFrameRectLR;
    xShapeLRProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRectLR;
    awt::Rectangle aBoundRectLR;
    xShapeLRProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectLR;
    // difference should be zero, but allow some rounding errors
    CPPUNIT_ASSERT_LESS(static_cast<long>(3), labs(aFrameRectLR.X - aBoundRectLR.X));

    // Get the shape "topbottom". Error was, that the identifier "top" was always set to zero, thus
    // the path was outside the frame rectangle for a viewBox having a positive "top" value.
    uno::Reference<drawing::XShape> xShapeTB(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape 'topbottom'", xShapeTB.is());
    uno::Reference<beans::XPropertySet> xShapeTBProps(xShapeTB, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape 'topbottom' properties", xShapeTBProps.is());
    awt::Rectangle aFrameRectTB;
    xShapeTBProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRectTB;
    awt::Rectangle aBoundRectTB;
    xShapeTBProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectTB;
    // difference should be zero, but allow some rounding errors
    CPPUNIT_ASSERT_LESS(static_cast<long>(3), labs(aFrameRectTB.Y - aBoundRectTB.Y));
}

CPPUNIT_TEST_SUITE_REGISTRATION(CustomshapesTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
