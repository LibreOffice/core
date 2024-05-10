/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <rtl/ustring.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/xml/ code.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"svx/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, test3DObjectFallback)
{
    // Load a document which has a 3D model we don't understand, but has a fallback PNG.
    loadFromFile(u"3d-object-fallback.odp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
    // Without the accompanying fix in place, this test would have failed, we could not read
    // Models/Fallbacks/duck.png, as we assumed a format like Pictures/something.png, i.e. a single
    // slash in the path.
    CPPUNIT_ASSERT(xGraphic.is());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
