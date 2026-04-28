/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <comphelper/scopeguard.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <unotools/tempfile.hxx>
#include <svx/svdograf.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/pdf/PDFAnnotationSubType.hxx>

#include <Annotation.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <sdpage.hxx>
#include <unomodel.hxx>

using namespace css;

namespace
{
class EnvVarGuard
{
public:
    EnvVarGuard(const char* var, const char* val)
    {
        if (getenv(var) == nullptr)
        {
            sVar = var;
            SetEnv(sVar, val);
        }
    }
    ~EnvVarGuard()
    {
        if (sVar)
            SetEnv(sVar, nullptr);
    }

private:
    static void SetEnv(const char* var, const char* val)
    {
#ifdef _WIN32
        if (!val)
            val = ""; // remove
        _putenv_s(var, val);
#else
        if (val)
            setenv(var, val, false);
        else
            unsetenv(var);
#endif
    }

    const char* sVar = nullptr;
};
}

class SdrPdfImportTest : public UnoApiTest
{
public:
    SdrPdfImportTest()
        : UnoApiTest(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

// Load the PDF in Draw, which will load the PDF as an Graphic, then
// mark the graphic object and trigger "break" function. This should
// convert the PDF content into objects/shapes.
CPPUNIT_TEST_FIXTURE(SdrPdfImportTest, testImportSimpleText)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    // We need to enable PDFium import (and make sure to disable after the test)
    EnvVarGuard UsePDFiumGuard("LO_IMPORT_USE_PDFIUM", "1");

    loadFromFile(u"SimplePDF.pdf");
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);

    // Get the first page - there should be only one.
    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    // Check there is one object on the page only
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());

    // Get the first object - there should be only one.
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObject);

    // Check the object is a graphic object
    SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
    CPPUNIT_ASSERT(pGraphicObject);
    // Check the graphic is a vector graphic and that it is PDF
    Graphic aGraphic = pGraphicObject->GetGraphic();
    auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    // Mark the object
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());

    // Execute the break operation - to turn the PDF into shapes/objects
    pViewShell->GetDrawView()->DoImportMarkedMtf();

    // Check there is one object on the page only
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());

    // Get the object
    SdrObject* pImportedObject = pPage->GetObj(0);
    CPPUNIT_ASSERT(pImportedObject);

    // Check the object position
#if !defined _WIN32
    CPPUNIT_ASSERT_EQUAL(Point(2004, 1980), pImportedObject->GetLogicRect().GetPos());
#else
    // need to check why windows appears to be different
    CPPUNIT_ASSERT_EQUAL(Point(1998, 1980), pImportedObject->GetLogicRect().GetPos());
#endif

    // Check the object size
#if !defined _WIN32
    CPPUNIT_ASSERT_EQUAL(Size(2165, 508), pImportedObject->GetLogicRect().GetSize());
#else
    // need to check why windows appears to be different
    CPPUNIT_ASSERT_EQUAL(Size(3944, 508), pImportedObject->GetLogicRect().GetSize());
#endif

    // Object should be a text object containing one paragraph with
    // content "This is PDF!"

    SdrTextObj* pTextObject = DynCastSdrTextObj(pImportedObject);
    CPPUNIT_ASSERT(pTextObject);
    OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
    const EditTextObject& aEdit = pOutlinerParagraphObject->GetTextObject();
    OUString sText = aEdit.GetText(0);
    CPPUNIT_ASSERT_EQUAL(u"This is PDF!"_ustr, sText);
}

CPPUNIT_TEST_FIXTURE(SdrPdfImportTest, testAnnotationsImportExport)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    // We need to enable PDFium import (and make sure to disable after the test)
    EnvVarGuard UsePDFiumGuard("LO_IMPORT_USE_PDFIUM", "1");

    EnvVarGuard DisablePDFCompressionGuard("VCL_DEBUG_DISABLE_PDFCOMPRESSION", "1");

    auto pPdfiumLibrary = vcl::pdf::PDFiumLibrary::get();

    loadFromFile(u"PdfWithAnnotation.pdf");
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);

    BinaryDataContainer aContainer;

    {
        // Get the first page - there should be only one.
        SdPage* pPage = pViewShell->GetActualPage();
        CPPUNIT_ASSERT(pPage);

        // Check the number of annotations
        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());

        // Get the first object - there should be only one.
        SdrObject* pObject = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObject);

        // Check the object is a graphic object
        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);

        // Check the graphic is a vector graphic and that it is PDF
        Graphic aGraphic = pGraphicObject->GetGraphic();
        auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

        // Write the PDF
        aContainer = pVectorGraphicData->getBinaryDataContainer();
    }

    { // check graphic PDF has annotations

        CPPUNIT_ASSERT_EQUAL(false, aContainer.isEmpty());

        auto pPDFDocument
            = pPdfiumLibrary->openDocument(aContainer.getData(), aContainer.getSize(), OString());
        auto pPDFPage = pPDFDocument->openPage(0);

        CPPUNIT_ASSERT_EQUAL(2, pPDFPage->getAnnotationCount());

        auto pPDFAnnotation1 = pPDFPage->getAnnotation(0);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Text,
                             pPDFAnnotation1->getSubType()); // Text annotation

        auto pPDFAnnotation2 = pPDFPage->getAnnotation(1);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup,
                             pPDFAnnotation2->getSubType()); // Pop-up annotation
    }

    { // save as PDF and check annotations
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        comphelper::SequenceAsHashMap aMediaDescriptor;
        aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
        uno::Sequence<beans::PropertyValue> aFilterData(
            comphelper::InitPropertySequence({ { "ExportBookmarks", uno::Any(true) } }));
        aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
        xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

        // Check PDF for annotations
        auto pPDFDocument = parsePDFExport();
        CPPUNIT_ASSERT(pPDFDocument);
        CPPUNIT_ASSERT_EQUAL(1, pPDFDocument->getPageCount());

        auto pPDFPage = pPDFDocument->openPage(0);
        CPPUNIT_ASSERT(pPDFPage);

        CPPUNIT_ASSERT_EQUAL(2, pPDFPage->getAnnotationCount());

        auto pPDFAnnotation1 = pPDFPage->getAnnotation(0);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Text,
                             pPDFAnnotation1->getSubType()); // Text annotation

        auto pPDFAnnotation2 = pPDFPage->getAnnotation(1);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Popup,
                             pPDFAnnotation2->getSubType()); // Pop-up annotation

        // Load document again
        loadFromURL(maTempFile.GetURL());
        auto pNewImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
        sd::ViewShell* pNewViewShell = pNewImpressDocument->GetDocShell()->GetViewShell();
        CPPUNIT_ASSERT(pNewViewShell);

        SdPage* pPage = pNewViewShell->GetActualPage();
        CPPUNIT_ASSERT(pPage);

        // We expect only 1 annotation in the document because the PDF
        // annotations are dependent on each-other:
        // parent annotation "Text" and the child annotation "Pop-up"

        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->getAnnotations().size());

        // check annotation
        auto xAnnotation = pPage->getAnnotations().at(0);

        CPPUNIT_ASSERT_DOUBLES_EQUAL(90.33, xAnnotation->getPosition().X, 1E-3);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(12.07, xAnnotation->getPosition().Y, 1E-3);

        CPPUNIT_ASSERT_EQUAL(u"TheAuthor"_ustr, xAnnotation->getAuthor());
        CPPUNIT_ASSERT_EQUAL(OUString(), xAnnotation->getInitials());

        auto xText = xAnnotation->getTextRange();

        CPPUNIT_ASSERT_EQUAL(u"This is the annotation text!"_ustr, xText->getString());

        auto aDateTime = xAnnotation->getDateTime();
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2020), aDateTime.Year);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), aDateTime.Month);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(18), aDateTime.Day);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(12), aDateTime.Hours);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(11), aDateTime.Minutes);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(53), aDateTime.Seconds);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), aDateTime.NanoSeconds);
        CPPUNIT_ASSERT_EQUAL(false, bool(aDateTime.IsUTC));
    }
}

CPPUNIT_TEST_FIXTURE(SdrPdfImportTest, testImportThreadedComments)
{
    // Sample PDF threaded_comments.pdf carries five Text annotations on page 0:
    //   4 - Alice,   root
    //   5 - Bob,     reply to Alice
    //   6 - Charlie, state-change targeting Alice:
    //                /IRT 4 /State (Completed) /StateModel (Review) /F 30
    //   7 - Dave,    has /State (Completed) /StateModel (Review) but no /IRT - Acrobat
    //                ignores /State on a non-state-change annotation, and so do we;
    //                Dave imports as a regular comment with no state.
    //   8 - Eve,     /IRT 4 + /State + /StateModel but missing the Hidden flag -
    //                structurally a state-change but the flags disqualify it.
    //                Acrobat treats such an annotation as malformed and skips it;
    //                we do the same: no sd::Annotation is produced for Eve.
    // After import we expect three sd::Annotations: Alice, Bob, Dave.
    // Charlie's state-change is collapsed into Alice's m_Resolved.
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    EnvVarGuard UsePDFiumGuard("LO_IMPORT_USE_PDFIUM", "1");

    loadFromFile(u"pdf/threaded_comments.pdf");
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    CPPUNIT_ASSERT_EQUAL(size_t(3), pPage->getAnnotations().size());

    rtl::Reference<sdr::annotation::Annotation> xRoot, xReply, xDave;
    for (auto const& x : pPage->getAnnotations())
    {
        if (x->getAuthor() == u"Alice"_ustr)
            xRoot = x;
        else if (x->getAuthor() == u"Bob"_ustr)
            xReply = x;
        else if (x->getAuthor() == u"Dave"_ustr)
            xDave = x;
        // Eve was skipped as malformed.
        CPPUNIT_ASSERT(x->getAuthor() != u"Eve"_ustr);
    }
    CPPUNIT_ASSERT(xRoot);
    CPPUNIT_ASSERT(xReply);
    CPPUNIT_ASSERT(xDave);

    // Every imported PDF annotation is threaded.
    CPPUNIT_ASSERT(xRoot->IsThreaded());
    CPPUNIT_ASSERT(xReply->IsThreaded());
    CPPUNIT_ASSERT(xDave->IsThreaded());

    // Alice is the root; Charlie's Review/Completed state-change was collapsed onto her.
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), xRoot->GetParentId());
    CPPUNIT_ASSERT(xRoot->IsResolved());

    // Bob replies to Alice; no state of his own.
    CPPUNIT_ASSERT_EQUAL(xRoot->GetId(), xReply->GetParentId());
    CPPUNIT_ASSERT(!xReply->IsResolved());

    // Dave has /State but no /IRT - not a state-change; /State is discarded.
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), xDave->GetParentId());
    CPPUNIT_ASSERT(!xDave->IsResolved());

    // Dispatch .uno:ResolveComment on Dave (a root, unresolved on import) and check the
    // model flipped.
    uno::Sequence<beans::PropertyValue> aResolveArgs(comphelper::InitPropertySequence({
        { "Id", uno::Any(OUString::number(xDave->GetId())) },
    }));
    dispatchCommand(mxComponent, u".uno:ResolveComment"_ustr, aResolveArgs);
    CPPUNIT_ASSERT(xDave->IsResolved());

    // Round-trip: re-export the loaded doc to PDF and verify the reply's /IRT, Alice's
    // surviving state-change, and the newly-added state-change from the slot dispatch are
    // all written. Charlie's original state-change was collapsed on import; on export a
    // fresh one is written for each resolved annotation with /State (Completed)
    // /StateModel (Review) and /F = Hidden | Print | NoZoom | NoRotate.
    EnvVarGuard DisablePDFCompressionGuard("VCL_DEBUG_DISABLE_PDFCOMPRESSION", "1");

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterName"_ustr] <<= u"draw_pdf_Export"_ustr;
    // ExportNotes defaults to false.
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "ExportNotes", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    auto pPDFDocument = parsePDFExport();
    CPPUNIT_ASSERT(pPDFDocument);
    CPPUNIT_ASSERT_EQUAL(1, pPDFDocument->getPageCount());
    auto pPDFPage = pPDFDocument->openPage(0);
    CPPUNIT_ASSERT(pPDFPage);

    // Three visible sd::Annotations (Alice, Bob, Dave) -> 3 Text + 3 Popup + 2 hidden
    // state-changes (one for Alice's resolved flag carried over from the import, one for
    // Dave's resolved flag stamped by the .uno:ResolveComment dispatch above) = 8 annotations.
    CPPUNIT_ASSERT_EQUAL(8, pPDFPage->getAnnotationCount());

    std::unique_ptr<vcl::pdf::PDFiumAnnotation> xPdfAlice, xPdfBob, xPdfDave;
    std::vector<std::unique_ptr<vcl::pdf::PDFiumAnnotation>> aStateChanges;
    for (int i = 0; i < pPDFPage->getAnnotationCount(); ++i)
    {
        auto xAnnot = pPDFPage->getAnnotation(i);
        if (xAnnot->getSubType() != vcl::pdf::PDFAnnotationSubType::Text)
            continue;
        // The state-change we emit carries /F = Hidden | Print | NoZoom | NoRotate = 30.
        if (xAnnot->getFlags() == 30)
        {
            aStateChanges.push_back(std::move(xAnnot));
            continue;
        }
        const OUString sAuthor = xAnnot->getString(vcl::pdf::constDictionaryKeyTitle);
        if (sAuthor == u"Alice")
            xPdfAlice = std::move(xAnnot);
        else if (sAuthor == u"Bob")
            xPdfBob = std::move(xAnnot);
        else if (sAuthor == u"Dave")
            xPdfDave = std::move(xAnnot);
    }
    CPPUNIT_ASSERT(xPdfAlice);
    CPPUNIT_ASSERT(xPdfBob);
    CPPUNIT_ASSERT(xPdfDave);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aStateChanges.size());

    // Alice and Dave have no /IRT (thread roots, no parent).
    CPPUNIT_ASSERT(!xPdfAlice->hasKey(vcl::pdf::constDictionaryKey_InReplyTo));
    CPPUNIT_ASSERT(!xPdfDave->hasKey(vcl::pdf::constDictionaryKey_InReplyTo));

    // Bob's /IRT resolves to Alice.
    CPPUNIT_ASSERT(xPdfBob->hasKey(vcl::pdf::constDictionaryKey_InReplyTo));
    auto xBobParent = xPdfBob->getLinked(vcl::pdf::constDictionaryKey_InReplyTo);
    CPPUNIT_ASSERT(xBobParent);
    CPPUNIT_ASSERT_EQUAL(pPDFPage->getAnnotationIndex(xPdfAlice),
                         pPDFPage->getAnnotationIndex(xBobParent));

    // One state-change targets Alice, one targets Dave; both carry Review/Completed.
    bool bAliceResolved = false;
    bool bDaveResolved = false;
    for (auto const& xSC : aStateChanges)
    {
        CPPUNIT_ASSERT(xSC->hasKey(vcl::pdf::constDictionaryKey_InReplyTo));
        auto xTarget = xSC->getLinked(vcl::pdf::constDictionaryKey_InReplyTo);
        CPPUNIT_ASSERT(xTarget);
        CPPUNIT_ASSERT_EQUAL(u"Completed"_ustr, xSC->getString(vcl::pdf::constDictionaryKey_State));
        CPPUNIT_ASSERT_EQUAL(u"Review"_ustr,
                             xSC->getString(vcl::pdf::constDictionaryKey_StateModel));
        const int nTargetIdx = pPDFPage->getAnnotationIndex(xTarget);
        if (nTargetIdx == pPDFPage->getAnnotationIndex(xPdfAlice))
            bAliceResolved = true;
        else if (nTargetIdx == pPDFPage->getAnnotationIndex(xPdfDave))
            bDaveResolved = true;
    }
    CPPUNIT_ASSERT(bAliceResolved);
    CPPUNIT_ASSERT(bDaveResolved);
}

CPPUNIT_TEST_FIXTURE(SdrPdfImportTest, testInsertAnnotationAtPosition)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;
    EnvVarGuard UsePDFiumGuard("LO_IMPORT_USE_PDFIUM", "1");

    loadFromFile(u"pdf/threaded_comments.pdf");
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    const size_t nBefore = pPage->getAnnotations().size();

    // PositionX/PositionY are in mm/100. 2540 mm/100 = 1 inch; 5080 mm/100 = 2 inches.
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence({
        { "Text", uno::Any(u"Click-to-place"_ustr) },
        { "PositionX", uno::Any(sal_Int32(2540)) },
        { "PositionY", uno::Any(sal_Int32(5080)) },
    }));
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aArgs);

    CPPUNIT_ASSERT_EQUAL(nBefore + 1, pPage->getAnnotations().size());
    const auto& xNew = pPage->getAnnotations().back();
    CPPUNIT_ASSERT_EQUAL(u"Click-to-place"_ustr, xNew->getTextRange()->getString());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.4, xNew->getPosition().X, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(50.8, xNew->getPosition().Y, 0.01);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
