/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/uiwriter/data3/";
} // namespace

/// Third set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest3 : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf129382)
{
    load(DATA_DIRECTORY, "tdf129382.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws = xDrawPageSupplier->getDrawPage();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDraws->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDraws->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126626)
{
    load(DATA_DIRECTORY, "tdf126626.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws = xDrawPageSupplier->getDrawPage();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDraws->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDraws->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf128739)
{
    load(DATA_DIRECTORY, "tdf128739.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Fehler: Verweis nicht gefunden"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Fehler: Verweis nicht gefunden"), getParagraph(1)->getString());
    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126340)
{
    load(DATA_DIRECTORY, "tdf126340.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), getParagraph(1)->getString());
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
