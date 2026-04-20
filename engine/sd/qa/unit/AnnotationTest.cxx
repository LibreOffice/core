/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <test/unoapi_test.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <comphelper/propertysequence.hxx>
#include <tools/json_writer.hxx>
#include <vcl/scheduler.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svx/unoapi.hxx>
#include <svx/annotation/Annotation.hxx>
#include <svx/annotation/ObjectAnnotationData.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>

#include <DrawDocShell.hxx>
#include <unomodel.hxx>
#include <sdpage.hxx>
#include <ViewShell.hxx>
#include <DrawViewShell.hxx>

using namespace css;

class AnnotationTest : public SdModelTestBase
{
public:
    AnnotationTest()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotation)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    {
        rtl::Reference<sdr::annotation::Annotation> xAnnotation = pPage->createAnnotation();
        CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

        xAnnotation->setAuthor(u"A"_ustr);
        uno::Reference<text::XText> xText(xAnnotation->getTextRange());
        xText->setString(u"X"_ustr);

        xAnnotation->setPosition(geometry::RealPoint2D(0.0, 0.0));
        xAnnotation->setSize(geometry::RealSize2D(10.0, 10.0));

        pPage->addAnnotation(xAnnotation, -1);

        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());
        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    }

    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::Annotation, pObject->GetObjIdentifier());

    {
        rtl::Reference<sdr::annotation::Annotation> xAnnotation = pPage->createAnnotation();
        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());

        xAnnotation->setAuthor(u"B"_ustr);
        uno::Reference<text::XText> xText(xAnnotation->getTextRange());
        xText->setString(u"XXX"_ustr);

        xAnnotation->setPosition(geometry::RealPoint2D(10.0, 10.0));
        xAnnotation->setSize(geometry::RealSize2D(10.0, 10.0));
        sdr::annotation::CreationInfo aInfo;
        aInfo.meType = sdr::annotation::AnnotationType::Square;
        aInfo.maColor = COL_LIGHTRED;
        xAnnotation->setCreationInfo(aInfo);

        pPage->addAnnotation(xAnnotation, -1);

        CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->getAnnotations().size());
        CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    }
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationInsert)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationDeleteAll)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->GetObjCount());

    dispatchCommand(mxComponent, u".uno:DeleteAllAnnotation"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->getAnnotations().size());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->getAnnotations().size());
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationDeleteAllByAuthor)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->GetObjCount());

    pPage->getAnnotations().at(0)->setAuthor(u"A"_ustr);
    pPage->getAnnotations().at(1)->setAuthor(u"A"_ustr);
    pPage->getAnnotations().at(2)->setAuthor(u"A"_ustr);

    // tdf#161899: Without the fix in place, this test would have crashed here
    const SfxStringItem aItem(SID_DELETEALLBYAUTHOR_POSTIT, u"A"_ustr);
    pViewShell->GetDispatcher()->ExecuteList(SID_DELETEALLBYAUTHOR_POSTIT, SfxCallMode::ASYNCHRON,
                                             { &aItem });
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->getAnnotations().size());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->getAnnotations().size());
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationDelete)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->getAnnotations().size());

    sal_uInt64 nAnnot1ID = pPage->getAnnotations().at(0)->GetId();
    sal_uInt64 nAnnot3ID = pPage->getAnnotations().at(2)->GetId();

    auto xAnnotation = pPage->getAnnotations().at(1);

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "Id", uno::Any(OUString::number(xAnnotation->GetId())) },
    }));
    dispatchCommand(mxComponent, u".uno:DeleteAnnotation"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->getAnnotations().size());

    CPPUNIT_ASSERT_EQUAL(nAnnot1ID, pPage->getAnnotations().at(0)->GetId());
    CPPUNIT_ASSERT_EQUAL(nAnnot3ID, pPage->getAnnotations().at(1)->GetId());
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationInsertUndoRedo)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::Annotation, pObject->GetObjIdentifier());

    sal_uInt64 nAnnot1ID = pPage->getAnnotations().at(0)->GetId();
    sal_uInt64 nAnnot2ID = pPage->getAnnotations().at(1)->GetId();

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(nAnnot1ID, pPage->getAnnotations().at(0)->GetId());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(nAnnot1ID, pPage->getAnnotations().at(0)->GetId());
    CPPUNIT_ASSERT_EQUAL(nAnnot2ID, pPage->getAnnotations().at(1)->GetId());

    dispatchCommand(mxComponent, u".uno:DeleteAnnotation"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(nAnnot1ID, pPage->getAnnotations().at(0)->GetId());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(nAnnot1ID, pPage->getAnnotations().at(0)->GetId());
    CPPUNIT_ASSERT_EQUAL(nAnnot2ID, pPage->getAnnotations().at(1)->GetId());
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationPositionUpdate)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    // Check current page
    SdPage* pPage = pViewShell->GetActualPage();

    // Should have no objects yet
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    // Insert new annotation
    uno::Sequence<beans::PropertyValue> aArgs;
    aArgs = comphelper::InitPropertySequence({
        { "Text", uno::Any(u"Comment"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aArgs);

    // 1 object in the page now
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    // .. and is an annotation object
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::Annotation, pObject->GetObjIdentifier());
    auto& pAnnotationData = pObject->getAnnotationData();
    CPPUNIT_ASSERT(pAnnotationData);

    // Remember the annotation ID
    sal_uInt64 nID = pAnnotationData->mxAnnotation->GetId();

    // Current annotation position
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), pObject->GetLogicRect().Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), pObject->GetLogicRect().Top());

    // Move the object
    pObject->Move({ 200, 200 });

    // Object at a new position
    CPPUNIT_ASSERT_EQUAL(tools::Long(200),
                         pObject->GetLogicRect().Left()); // in 100th of an mm (hmm)
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), pObject->GetLogicRect().Top());

    // Position of the annotation would be the same as the object (2.0 mm equals 200 hmm)
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, pAnnotationData->mxAnnotation->getPosition().X, 1E-4); // mm
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, pAnnotationData->mxAnnotation->getPosition().Y, 1E-4);

    // Change the annotation position
    aArgs = comphelper::InitPropertySequence(
        { { "Id", uno::Any(OUString::number(nID)) },
          { "PositionX", uno::Any(sal_Int32(1440)) }, // 1440 twips = 2540 hmm
          { "PositionY", uno::Any(sal_Int32(14400)) } });

    dispatchCommand(mxComponent, u".uno:EditAnnotation"_ustr, aArgs);

    // Position of the annotation changed again
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.4, pAnnotationData->mxAnnotation->getPosition().X,
                                 1E-4); // in mm
    CPPUNIT_ASSERT_DOUBLES_EQUAL(254.0, pAnnotationData->mxAnnotation->getPosition().Y, 1E-4);

    // Position of the annotation object changed also to the same value
    CPPUNIT_ASSERT_EQUAL(tools::Long(2540), pObject->GetLogicRect().Left()); // in hmm
    CPPUNIT_ASSERT_EQUAL(tools::Long(25400), pObject->GetLogicRect().Top());
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationDuplicatePage)
{
    // Check the annotation object is properly cloned when duplicating the page.

    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    auto* pDrawViewShell = dynamic_cast<sd::DrawViewShell*>(pViewShell);
    CPPUNIT_ASSERT(pDrawViewShell);

    SdDrawDocument* pDocument = pXImpressDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDocument);

    // 1 Page
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pDocument->GetSdPageCount(PageKind::Standard));

    // No objects yet
    SdrPage* pPage1 = pDrawViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage1->GetObjCount());

    // Inserted new annotation
    uno::Sequence<beans::PropertyValue> aArgs;

    aArgs = comphelper::InitPropertySequence({
        { "Text", uno::Any(u"Comment"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aArgs);

    // Check state
    {
        // 1 Annotation in the page
        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage1->getAnnotations().size());

        // 1 Object in the page
        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage1->GetObjCount());

        // And the object is an annotation
        SdrObject* pObject = pPage1->GetObj(0);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::Annotation, pObject->GetObjIdentifier());
        CPPUNIT_ASSERT(pObject->isAnnotationObject());
        auto& rAnnotationData = pObject->getAnnotationData();

        // Get the annotation from the page 1
        auto xAnnotation = pPage1->getAnnotations().at(0);

        // Annotation in the object is the same as the one in the page
        CPPUNIT_ASSERT_EQUAL(xAnnotation.get(), rAnnotationData->mxAnnotation.get());

        // Check text of the annotation
        CPPUNIT_ASSERT_EQUAL(u"Comment"_ustr, xAnnotation->GetText());
    }

    // Let's duplicate the page
    dispatchCommand(mxComponent, u".uno:DuplicatePage"_ustr, {});

    // 2 Pages
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), pDocument->GetSdPageCount(PageKind::Standard));

    // Let's switch to the 2nd page
    CPPUNIT_ASSERT_EQUAL(true, pDrawViewShell->SwitchPage(1));

    // Check state
    {
        // Get the current page
        SdrPage* pPage2 = pDrawViewShell->GetActualPage();

        // Should not be the same page as previous page
        CPPUNIT_ASSERT(pPage2 != pPage1);

        // 1 Annotation in the page
        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage2->getAnnotations().size());

        // 1 Object in the page
        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage2->GetObjCount());

        // And the object is an annotation
        SdrObject* pObject = pPage2->GetObj(0);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::Annotation, pObject->GetObjIdentifier());
        CPPUNIT_ASSERT(pObject->isAnnotationObject());
        auto& rAnnotationData = pObject->getAnnotationData();

        // Get the annotation from the page 2
        auto xAnnotation = pPage2->getAnnotations().at(0);

        // Annotation in the object is the same as the one in the page
        CPPUNIT_ASSERT_EQUAL(xAnnotation.get(), rAnnotationData->mxAnnotation.get());

        // Check text of the annotation
        CPPUNIT_ASSERT_EQUAL(u"Comment"_ustr, xAnnotation->GetText());

        // Annotation in page 1 is not the same instance as annotation in page 2
        // We verify the annotation was copied
        CPPUNIT_ASSERT(pPage1->getAnnotations().at(0).get()
                       != pPage2->getAnnotations().at(0).get());
    }
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationDuplicatePageAndUndo)
{
    // Insert annotation, duplicate, undo

    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    auto* pDrawViewShell = dynamic_cast<sd::DrawViewShell*>(pViewShell);
    CPPUNIT_ASSERT(pDrawViewShell);

    SdDrawDocument* pDocument = pXImpressDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDocument);

    // Inserted new annotation
    uno::Sequence<beans::PropertyValue> aArgs;

    aArgs = comphelper::InitPropertySequence({
        { "Text", uno::Any(u"Comment"_ustr) },
    });

    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aArgs);

    // Duplicate
    dispatchCommand(mxComponent, u".uno:DuplicatePage"_ustr, {});

    // Undo
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationTextUpdate)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    // Check current page
    SdPage* pPage = pViewShell->GetActualPage();

    // Should have no objects yet
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    // Should have no annotations yet
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->getAnnotations().size());

    // New free text annotation
    rtl::Reference<sdr::annotation::Annotation> xAnnotation = pPage->createAnnotation();
    xAnnotation->setAuthor(u"B"_ustr);

    uno::Reference<text::XText> xText(xAnnotation->getTextRange());
    xText->setString(u"XXX"_ustr);

    xAnnotation->setPosition(geometry::RealPoint2D(1000.0, 1000.0));
    xAnnotation->setSize(geometry::RealSize2D(5000.0, 2000.0));

    sdr::annotation::CreationInfo aInfo;
    aInfo.meType = sdr::annotation::AnnotationType::FreeText;
    xAnnotation->setCreationInfo(aInfo);

    pPage->addAnnotation(xAnnotation, -1);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());

    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::Text, pObject->GetObjIdentifier());

    SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject);

    {
        OUString sText = pTextObject->GetOutlinerParaObject()->GetTextObject().GetText(0);
        CPPUNIT_ASSERT_EQUAL(u"XXX"_ustr, sText);

        CPPUNIT_ASSERT_EQUAL(u"XXX"_ustr, xAnnotation->GetText());
    }

    SdrView* pView = pViewShell->GetView();
    pView->SdrBeginTextEdit(pTextObject);
    CPPUNIT_ASSERT_EQUAL(true, pView->IsTextEdit());

    auto* pOutlinerView = pView->GetTextEditOutlinerView();
    CPPUNIT_ASSERT(pOutlinerView);
    auto& rEditView = pOutlinerView->GetEditView();
    rEditView.SetSelection(ESelection::All());
    rEditView.InsertText(u"ABC"_ustr);

    pView->SdrEndTextEdit();
    CPPUNIT_ASSERT_EQUAL(false, pView->IsTextEdit());

    {
        OUString sText = pTextObject->GetOutlinerParaObject()->GetTextObject().GetText(0);
        CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, sText);

        CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, pPage->getAnnotations().at(0)->GetText());
    }
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationThreadedFields)
{
    // Threaded-comment fields (threaded/resolved/parentId) are carried by the
    // annotation model, survive toData/fromData and clone, and surface in the
    // Kit JSON so the Online client can show the resolve UI.
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();

    // A classic (non-threaded) annotation.
    rtl::Reference<sdr::annotation::Annotation> xFlat = pPage->createAnnotation();
    xFlat->setAuthor(u"Flat"_ustr);
    xFlat->setPosition(geometry::RealPoint2D(0.0, 0.0));
    xFlat->setSize(geometry::RealSize2D(10.0, 10.0));
    xFlat->getTextRange()->setString(u"flat"_ustr);
    pPage->addAnnotation(xFlat, -1);

    CPPUNIT_ASSERT(!xFlat->IsThreaded());
    CPPUNIT_ASSERT(!xFlat->IsResolved());
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), xFlat->GetParentId());

    // A threaded root annotation, resolved.
    rtl::Reference<sdr::annotation::Annotation> xRoot = pPage->createAnnotation();
    xRoot->setAuthor(u"Root"_ustr);
    xRoot->setPosition(geometry::RealPoint2D(20.0, 0.0));
    xRoot->setSize(geometry::RealSize2D(10.0, 10.0));
    xRoot->getTextRange()->setString(u"root"_ustr);
    xRoot->SetThreaded(true);
    xRoot->SetResolved(true);
    pPage->addAnnotation(xRoot, -1);

    CPPUNIT_ASSERT(xRoot->IsThreaded());
    CPPUNIT_ASSERT(xRoot->IsResolved());
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), xRoot->GetParentId());

    // A threaded reply referencing the root.
    rtl::Reference<sdr::annotation::Annotation> xReply = pPage->createAnnotation();
    xReply->setAuthor(u"Reply"_ustr);
    xReply->setPosition(geometry::RealPoint2D(40.0, 0.0));
    xReply->setSize(geometry::RealSize2D(10.0, 10.0));
    xReply->getTextRange()->setString(u"reply"_ustr);
    xReply->SetThreaded(true);
    xReply->SetParentId(xRoot->GetId());
    pPage->addAnnotation(xReply, -1);

    CPPUNIT_ASSERT(xReply->IsThreaded());
    CPPUNIT_ASSERT(!xReply->IsResolved());
    CPPUNIT_ASSERT_EQUAL(xRoot->GetId(), xReply->GetParentId());

    // toData / fromData round-trip carries the three new fields.
    sdr::annotation::AnnotationData aSaved;
    xReply->toData(aSaved);
    CPPUNIT_ASSERT(aSaved.m_Threaded);
    CPPUNIT_ASSERT(!aSaved.m_Resolved);
    CPPUNIT_ASSERT_EQUAL(xRoot->GetId(), aSaved.m_ParentId);

    // Flip the live annotation, then restore via fromData.
    xReply->SetThreaded(false);
    xReply->SetResolved(true);
    xReply->SetParentId(0);
    xReply->fromData(aSaved);
    CPPUNIT_ASSERT(xReply->IsThreaded());
    CPPUNIT_ASSERT(!xReply->IsResolved());
    CPPUNIT_ASSERT_EQUAL(xRoot->GetId(), xReply->GetParentId());

    // clone() propagates the fields.
    rtl::Reference<sdr::annotation::Annotation> xClone = xRoot->clone(pPage);
    CPPUNIT_ASSERT(xClone->IsThreaded());
    CPPUNIT_ASSERT(xClone->IsResolved());
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), xClone->GetParentId());

    // Per-annotation ToJSON emits the new keys only when threaded.
    auto parseJson = [](std::string_view aText) {
        std::stringstream aStream{ std::string(aText) };
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        return aTree;
    };

    {
        auto aTree = parseJson(xFlat->ToJSON(sdr::annotation::CommentNotificationType::Modify));
        const auto& rComment = aTree.get_child("comment");
        CPPUNIT_ASSERT(!rComment.get_child_optional("threaded"));
        CPPUNIT_ASSERT(!rComment.get_child_optional("resolved"));
        CPPUNIT_ASSERT(!rComment.get_child_optional("parentId"));
    }
    {
        auto aTree = parseJson(xRoot->ToJSON(sdr::annotation::CommentNotificationType::Modify));
        const auto& rComment = aTree.get_child("comment");
        CPPUNIT_ASSERT_EQUAL(std::string("true"), rComment.get<std::string>("threaded"));
        CPPUNIT_ASSERT_EQUAL(std::string("true"), rComment.get<std::string>("resolved"));
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), rComment.get<sal_uInt64>("parentId"));
    }
    {
        auto aTree = parseJson(xReply->ToJSON(sdr::annotation::CommentNotificationType::Modify));
        const auto& rComment = aTree.get_child("comment");
        CPPUNIT_ASSERT_EQUAL(std::string("true"), rComment.get<std::string>("threaded"));
        CPPUNIT_ASSERT_EQUAL(std::string("false"), rComment.get<std::string>("resolved"));
        CPPUNIT_ASSERT_EQUAL(xRoot->GetId(), rComment.get<sal_uInt64>("parentId"));
    }

    // Doc-level getPostIts dump: flat annotation has no threaded keys; root and
    // reply carry the expected values.
    tools::JsonWriter aWriter;
    pXImpressDocument->getPostIts(aWriter);
    auto aDocTree = parseJson(aWriter.finishAndGetAsOString());

    int nVerified = 0;
    for (const auto& rEntry : aDocTree.get_child("comments"))
    {
        const auto& rComment = rEntry.second;
        const sal_uInt64 nId = rComment.get<sal_uInt64>("id");
        if (nId == xFlat->GetId())
        {
            CPPUNIT_ASSERT(!rComment.get_child_optional("threaded"));
            CPPUNIT_ASSERT(!rComment.get_child_optional("resolved"));
            CPPUNIT_ASSERT(!rComment.get_child_optional("parentId"));
            ++nVerified;
        }
        else if (nId == xRoot->GetId())
        {
            CPPUNIT_ASSERT_EQUAL(std::string("true"), rComment.get<std::string>("threaded"));
            CPPUNIT_ASSERT_EQUAL(std::string("true"), rComment.get<std::string>("resolved"));
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), rComment.get<sal_uInt64>("parentId"));
            ++nVerified;
        }
        else if (nId == xReply->GetId())
        {
            CPPUNIT_ASSERT_EQUAL(std::string("true"), rComment.get<std::string>("threaded"));
            CPPUNIT_ASSERT_EQUAL(std::string("false"), rComment.get<std::string>("resolved"));
            CPPUNIT_ASSERT_EQUAL(xRoot->GetId(), rComment.get<sal_uInt64>("parentId"));
            ++nVerified;
        }
    }
    CPPUNIT_ASSERT_EQUAL(3, nVerified);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
