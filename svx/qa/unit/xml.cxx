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

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <rtl/ustring.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/xml/ code.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
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
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

CPPUNIT_TEST_FIXTURE(Test, test3DObjectFallback)
{
    // Load a document which has a 3D model we don't understand, but has a fallback PNG.
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(u"svx/qa/unit/data/3d-object-fallback.odp");
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("Graphic") >>= xGraphic;
    // Without the accompanying fix in place, this test would have failed, we could not read
    // Models/Fallbacks/duck.png, as we assumed a format like Pictures/something.png, i.e. a single
    // slash in the path.
    CPPUNIT_ASSERT(xGraphic.is());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
