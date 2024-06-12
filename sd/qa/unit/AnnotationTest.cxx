/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <comphelper/propertysequence.hxx>
#include <vcl/scheduler.hxx>
#include <svx/unoapi.hxx>
#include <svx/annotation/Annotation.hxx>
#include <svx/annotation/ObjectAnnotationData.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdview.hxx>

#include <DrawDocShell.hxx>
#include <unomodel.hxx>
#include <sdpage.hxx>
#include <ViewShell.hxx>

using namespace css;

class AnnotationTest : public SdModelTestBase
{
public:
    AnnotationTest()
        : SdModelTestBase("/sd/qa/unit/data/")
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

        xAnnotation->setAuthor("A");
        uno::Reference<text::XText> xText(xAnnotation->getTextRange());
        xText->setString("X");

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

        xAnnotation->setAuthor("B");
        uno::Reference<text::XText> xText(xAnnotation->getTextRange());
        xText->setString("XXX");

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

    dispatchCommand(mxComponent, ".uno:InsertAnnotation", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationDelete)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:InsertAnnotation", {});
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", {});
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->GetObjCount());

    sal_uInt32 nID = pPage->getAnnotations().front()->GetId();

    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 0), pPage->getAnnotations().at(0)->GetId());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 1), pPage->getAnnotations().at(1)->GetId());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 2), pPage->getAnnotations().at(2)->GetId());

    auto xAnnotation = pPage->getAnnotations().at(1);

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "Id", uno::Any(OUString::number(xAnnotation->GetId())) },
    }));
    dispatchCommand(mxComponent, ".uno:DeleteAnnotation", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->getAnnotations().size());

    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 0), pPage->getAnnotations().at(0)->GetId());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 2), pPage->getAnnotations().at(1)->GetId());
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationInsertUndoRedo)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:InsertAnnotation", {});
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::Annotation, pObject->GetObjIdentifier());

    sal_uInt32 nID = pPage->getAnnotations().front()->GetId();

    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 0), pPage->getAnnotations().at(0)->GetId());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 1), pPage->getAnnotations().at(1)->GetId());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 0), pPage->getAnnotations().at(0)->GetId());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 0), pPage->getAnnotations().at(0)->GetId());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 1), pPage->getAnnotations().at(1)->GetId());

    dispatchCommand(mxComponent, ".uno:DeleteAnnotation", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 0), pPage->getAnnotations().at(0)->GetId());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->getAnnotations().size());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 0), pPage->getAnnotations().at(0)->GetId());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(nID + 1), pPage->getAnnotations().at(1)->GetId());
}

CPPUNIT_TEST_FIXTURE(AnnotationTest, testAnnotationUpdate)
{
    createSdDrawDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pPage->GetObjCount());

    uno::Sequence<beans::PropertyValue> aArgs;

    aArgs = comphelper::InitPropertySequence({
        { "Text", uno::Any(u"Comment"_ustr) },
    });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(SdrObjKind::Annotation, pObject->GetObjIdentifier());

    auto& pAnnotationData = pObject->getAnnotationData();
    CPPUNIT_ASSERT(pAnnotationData);
    sal_Int32 nID = pAnnotationData->mxAnnotation->GetId();

    CPPUNIT_ASSERT_EQUAL(tools::Long(0), pObject->GetLogicRect().Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), pObject->GetLogicRect().Top());

    pObject->Move({ 200, 200 });

    CPPUNIT_ASSERT_EQUAL(tools::Long(200), pObject->GetLogicRect().Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), pObject->GetLogicRect().Top());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, pAnnotationData->mxAnnotation->getPosition().X, 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, pAnnotationData->mxAnnotation->getPosition().Y, 1E-4);

    aArgs = comphelper::InitPropertySequence({ { "Id", uno::Any(OUString::number(nID)) },
                                               { "PositionX", uno::Any(sal_Int32(1440)) },
                                               { "PositionY", uno::Any(sal_Int32(14400)) } });

    dispatchCommand(mxComponent, ".uno:EditAnnotation", aArgs);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.4, pAnnotationData->mxAnnotation->getPosition().X, 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(254.0, pAnnotationData->mxAnnotation->getPosition().Y, 1E-4);

    CPPUNIT_ASSERT_EQUAL(tools::Long(2540), pObject->GetLogicRect().Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(25400), pObject->GetLogicRect().Top());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
