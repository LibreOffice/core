/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/CellColorHandler.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, test129205)
{
    loadFromFile(u"tdf129205.docx");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    drawing::FillStyle eFillStyle = drawing::FillStyle::FillStyle_NONE;
    xPara->getPropertyValue("FillStyle") >>= eFillStyle;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: drawing::FillStyle_NONE
    // - Actual  : FillStyle_SOLID
    // i.e. the paragraph had a solid fill, making the header image invisible.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, eFillStyle);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
