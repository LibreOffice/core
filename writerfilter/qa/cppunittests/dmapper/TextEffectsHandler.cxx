/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/TextEffectsHandler.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/writerfilter/qa/cppunittests/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSemiTransparentText)
{
    // Load a document with a single paragraph: second text portion has semi-transparent text.
    loadFromURL(u"semi-transparent-text.docx");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    xPortionEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    sal_Int16 nCharTransparence = 0;
    xPortion->getPropertyValue("CharTransparence") >>= nCharTransparence;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 74
    // - Actual  : 0
    // i.e. text was imported as regular text with solid color only.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(74), nCharTransparence);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
