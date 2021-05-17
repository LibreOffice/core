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

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/math.hxx>

using namespace ::com::sun::star;

namespace
{
/// Gets one child of xShape, which one is specified by nIndex.
uno::Reference<drawing::XShape> getChildShape(const uno::Reference<drawing::XShape>& xShape,
                                              sal_Int32 nIndex)
{
    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    CPPUNIT_ASSERT(xGroup->getCount() > nIndex);

    uno::Reference<drawing::XShape> xRet(xGroup->getByIndex(nIndex), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xRet.is());

    return xRet;
}
}

char const DATA_DIRECTORY[] = "/oox/qa/unit/data/";

/// oox shape tests.
class OoxShapeTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void load(const OUString& rURL);
};

void OoxShapeTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void OoxShapeTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void OoxShapeTest::load(const OUString& rFileName)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFileName;
    mxComponent = loadFromDesktop(aURL);
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testGroupTransform)
{
    load(u"tdf141463_GroupTransform.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(getChildShape(xGroup, 0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed in several properties.

    sal_Int32 nAngle;
    xPropSet->getPropertyValue("ShearAngle") >>= nAngle;
    // Failed with - Expected: 0
    //             - Actual  : -810
    // i.e. the shape was sheared although shearing does not exist in oox
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nAngle);

    xPropSet->getPropertyValue("RotateAngle") >>= nAngle;
    // Failed with - Expected: 26000 (is in 1/100deg)
    //             - Actual  : 26481 (is in 1/100deg)
    // 100deg in PowerPoint UI = 360deg - 100deg in LO.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(26000), nAngle);

    sal_Int32 nActual = xShape->getSize().Width;
    // The group has ext.cy=2880000 and chExt.cy=4320000 resulting in Y-scale=2/3.
    // The child has ext 2880000 x 1440000. Because of rotation angle 80deg, the Y-scale has to be
    // applied to the width, resulting in 2880000 * 2/3 = 1920000EMU = 5333Hmm
    // ToDo: Expected value currently 1 off.
    // Failed with - Expected: 5332
    //             - Actual  : 5432
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5332), nActual);
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testMultipleGroupShapes)
{
    load("multiple-group-shapes.docx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the 2 group shapes from the document were imported as a single one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testCustomshapePosition)
{
    load("customshape-position.docx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    sal_Int32 nY{};
    xShape->getPropertyValue("VertOrientPosition") >>= nY;
    // <wp:posOffset>581025</wp:posOffset> in the document.
    sal_Int32 nExpected = rtl::math::round(581025.0 / 360);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1614
    // - Actual  : 0
    // i.e. the position of the shape was lost on import due to the rounded corners.
    CPPUNIT_ASSERT_EQUAL(nExpected, nY);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
