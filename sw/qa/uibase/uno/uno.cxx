/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/text/XTextViewTextRangeSupplier.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <vcl/scheduler.hxx>

#include <docsh.hxx>
#include <edtwin.hxx>
#include <unotextrange.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/uibase/uno/data/";

/// Covers sw/source/uibase/uno/ fixes.
class SwUibaseUnoTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testLockControllers)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY_THROW);
        xModel->lockControllers();
    }
    {
        uno::Reference<util::XCloseable> xCloseable(mxComponent, uno::UNO_QUERY_THROW);
        xCloseable->close(false);
    }
    // Without the accompanying fix in place, this test would have crashed.
    mxComponent.clear();
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testCondFieldCachedValue)
{
    load(DATA_DIRECTORY, "cond-field-cached-value.docx");
    Scheduler::ProcessEventsToIdle();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  :
    // i.e. the conditional field lost its cached content.
    getParagraph(2, "1");
}

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testCreateTextRangeByPixelPosition)
{
    // Given a document with 2 characters, and the pixel position of the point between them:
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert2("AZ");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    Point aLogic = pWrtShell->GetCharRect().Center();
    SwView* pView = pDocShell->GetView();
    SwEditWin& rEditWin = pView->GetEditWin();
    Point aPixel = rEditWin.LogicToPixel(aLogic);

    // When converting that pixel position to a document model position (text range):
    uno::Reference<frame::XModel2> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xControllers = xModel->getControllers();
    uno::Reference<text::XTextViewTextRangeSupplier> xController(xControllers->nextElement(),
                                                                 uno::UNO_QUERY);
    awt::Point aPoint(aPixel.getX(), aPixel.getY());
    uno::Reference<text::XTextRange> xTextRange
        = xController->createTextRangeByPixelPosition(aPoint);

    // Then make sure that text range points after the first character:
    auto pTextRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    SwPaM aPaM(pDoc->GetNodes());
    pTextRange->GetPositions(aPaM);
    sal_Int32 nActual = aPaM.GetPoint()->nContent.GetIndex();
    // Without the needed PixelToLogic() call in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the returned text range pointed before the first character, not between the first and
    // the second character.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nActual);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
