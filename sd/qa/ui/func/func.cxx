/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sdmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/propertyvalue.hxx>
#include <vcl/scheduler.hxx>

#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <sdpage.hxx>
#include <unomodel.hxx>

using namespace com::sun::star;

namespace
{
/// Covers sd/source/ui/func/ fixes.
class Test : public SdModelTestBase
{
public:
    Test()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testNoneToBullet)
{
    // Given a document with a shape, the only paragraph has a numbering of type "none":
    createSdImpressDoc("odp/none-to-bullet.odp");
    sd::ViewShell* pViewShell = getSdDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pShape = pPage->GetObj(0);
    CPPUNIT_ASSERT(pShape);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pShape, pView->GetSdrPageView());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!pView->IsTextEdit());

    // When turning the "none" numbering to a bullet:
    // Start text edit:
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    typeString(pImpressDocument, u"x");
    CPPUNIT_ASSERT(pView->IsTextEdit());
    // Do the switch:
    dispatchCommand(mxComponent, u".uno:DefaultBullet"_ustr, {});
    // End text edit:
    typeKey(pImpressDocument, KEY_ESCAPE);

    // Then make sure we switch to a bullet with reasonable defaults:
    CPPUNIT_ASSERT(!pView->IsTextEdit());
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraphFromShape(0, xShape),
                                                   uno::UNO_QUERY);
    // Check the list level 1 properties:
    uno::Reference<container::XIndexAccess> xNumberingRules;
    xParagraph->getPropertyValue(u"NumberingRules"_ustr) >>= xNumberingRules;
    comphelper::SequenceAsHashMap aNumberingRule(xNumberingRules->getByIndex(0));
    sal_Int32 nLeftMargin{};
    aNumberingRule[u"LeftMargin"_ustr] >>= nLeftMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1200
    // - Actual  : 0
    // i.e. there was no left margin at all, first and later lines did not match on the left hand
    // side.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1200), nLeftMargin);
    sal_Int32 nFirstLineOffset{};
    aNumberingRule[u"FirstLineOffset"_ustr] >>= nFirstLineOffset;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-900), nFirstLineOffset);
}

CPPUNIT_TEST_FIXTURE(Test, testNoneToLibraryBullet)
{
    // Given a document with a shape, the only paragraph has a numbering of type "none":
    createSdImpressDoc("odp/none-to-bullet.odp");
    sd::ViewShell* pViewShell = getSdDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pShape = pPage->GetObj(0);
    CPPUNIT_ASSERT(pShape);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pShape, pView->GetSdrPageView());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!pView->IsTextEdit());

    // When turning the "none" numbering to a bullet from the library:
    // Start text edit:
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    typeString(pImpressDocument, u"x");
    CPPUNIT_ASSERT(pView->IsTextEdit());
    // Do the switch, taking the second option from the list:
    uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue("BulletIndex", static_cast<sal_uInt16>(2)) };
    dispatchCommand(mxComponent, u".uno:SetBullet"_ustr, aArgs);
    // End text edit:
    typeKey(pImpressDocument, KEY_ESCAPE);

    // Then make sure we switch to a bullet with the correct character:
    CPPUNIT_ASSERT(!pView->IsTextEdit());
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraphFromShape(0, xShape),
                                                   uno::UNO_QUERY);
    // Check the list level 1 properties:
    uno::Reference<container::XIndexAccess> xNumberingRules;
    xParagraph->getPropertyValue(u"NumberingRules"_ustr) >>= xNumberingRules;
    comphelper::SequenceAsHashMap aNumberingRule(xNumberingRules->getByIndex(0));
    OUString aBulletChar;
    aNumberingRule[u"BulletChar"_ustr] >>= aBulletChar;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ○ (white bullet)
    // - Actual  : ● (black circle)
    // i.e. the bullet char was the default, not the selected one.
    CPPUNIT_ASSERT_EQUAL(u"\u25CB"_ustr, aBulletChar);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
