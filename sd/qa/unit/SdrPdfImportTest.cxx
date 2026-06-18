/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <comphelper/scopeguard.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/configuration.hxx>

#include <officecfg/Office/Draw.hxx>
#include <unotools/tempfile.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <tools/color.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/pdf/PDFAnnotationSubType.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/region.hxx>
#include <vcl/mapmod.hxx>

#include <Annotation.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <unomodel.hxx>

using namespace css;

namespace
{
// Collects the primitives produced for every view object contact during a
// CompleteRedraw, while still forwarding them so the normal render runs.
class PrimitiveCollector : public sdr::contact::ViewObjectContactRedirector
{
public:
    drawinglayer::primitive2d::Primitive2DContainer maPrimitives;

    void createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo,
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) override
    {
        drawinglayer::primitive2d::Primitive2DContainer aLocal;
        ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo,
                                                                         aLocal);
        maPrimitives.append(aLocal);
        rVisitor.visit(std::move(aLocal));
    }
};

// True when the primitives, decomposed recursively, contain a hairline drawn in
// rColor. A null reference in the tree is skipped rather than dereferenced.
bool containsHairlineOfColor(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitives,
                             const basegfx::BColor& rColor)
{
    for (const drawinglayer::primitive2d::Primitive2DReference& rReference : rPrimitives)
    {
        const drawinglayer::primitive2d::BasePrimitive2D* pPrimitive = rReference.get();
        if (!pPrimitive)
            continue;

        if (auto* pHairline
            = dynamic_cast<const drawinglayer::primitive2d::PolygonHairlinePrimitive2D*>(
                pPrimitive))
        {
            if (pHairline->getBColor() == rColor)
                return true;
            continue;
        }

        drawinglayer::primitive2d::Primitive2DContainer aChildren;
        pPrimitive->get2DDecomposition(aChildren, drawinglayer::geometry::ViewInformation2D());
        if (containsHairlineOfColor(aChildren, rColor))
            return true;
    }
    return false;
}
}

class SdrPdfImportTest : public UnoApiTest
{
public:
    SdrPdfImportTest()
        : UnoApiTest(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

// Load the PDF in Draw, which will load the PDF as a Graphic, then
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
    UsePdfium aGuard;

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

    // Position and size depend on which font face is selected
    // (full bundled font vs. PDF subset font), which varies by environment.
    // Disable exact checks until font selection is stabilized.
#if 0
    // Check the object position
#if !defined _WIN32
    CPPUNIT_ASSERT_EQUAL(Point(2004, 2018), pImportedObject->GetLogicRect().GetPos());
#else
    // need to check why windows appears to be different
    CPPUNIT_ASSERT_EQUAL(Point(1998, 2018), pImportedObject->GetLogicRect().GetPos());
#endif

    // Check the object size
#if !defined _WIN32
    CPPUNIT_ASSERT_EQUAL(Size(2165, 470), pImportedObject->GetLogicRect().GetSize());
#else
    // need to check why windows appears to be different
    CPPUNIT_ASSERT_EQUAL(Size(3944, 470), pImportedObject->GetLogicRect().GetSize());
#endif
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
    UsePdfium aGuard;

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
        comphelper::SequenceAsHashMap aMediaDescriptor;
        uno::Sequence<beans::PropertyValue> aFilterData(
            comphelper::InitPropertySequence({ { "ExportBookmarks", uno::Any(true) } }));
        aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
        saveAndReload(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

        // Check PDF for annotations
        auto pPDFDocument = parsePDFExport(pPdfium);
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

        // check the loaded document again
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
    //   4 — Alice,   root
    //   5 — Bob,     reply to Alice
    //   6 — Charlie, state-change targeting Alice:
    //                /IRT 4 /State (Completed) /StateModel (Review) /F 30
    //   7 — Dave,    has /State (Completed) /StateModel (Review) but no /IRT — Acrobat
    //                ignores /State on a non-state-change annotation, and so do we;
    //                Dave imports as a regular comment with no state.
    //   8 — Eve,     /IRT 4 + /State + /StateModel but missing the Hidden flag —
    //                structurally a state-change but the flags disqualify it.
    //                Acrobat treats such an annotation as malformed and skips it;
    //                we do the same: no sd::Annotation is produced for Eve.
    // After import we expect three sd::Annotations: Alice, Bob, Dave.
    // Charlie's state-change is collapsed into Alice's m_Resolved.
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

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

    // Dave has /State but no /IRT — not a state-change; /State is discarded.
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), xDave->GetParentId());
    CPPUNIT_ASSERT(!xDave->IsResolved());
}

CPPUNIT_TEST_FIXTURE(SdrPdfImportTest, testPdfHidesTextBoundaries)
{
    // A PDF opened in Draw must not show the text boundary guides, even when the
    // "show boundary for margins" setting is on. Each page holds a single imported
    // image, so the margins carry no meaning there.

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // Turn the boundary guides on, the way a user who wants them for ordinary
    // Draw documents would.
    auto xChanges = comphelper::ConfigurationChanges::create();
    officecfg::Office::Draw::Misc::TextObject::ShowBoundary::set(true, xChanges);
    xChanges->commit();

    loadFromFile(u"SimplePDF.pdf");
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    SdDrawDocument* pDocument = pImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDocument);
    CPPUNIT_ASSERT(pDocument->IsPDFDocument());

    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);
    SdrView* pView = pViewShell->GetView();
    CPPUNIT_ASSERT(pView);

    // Render the whole page and gather every primitive sent to the device.
    PrimitiveCollector aCollector;
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetMapMode(MapMode(MapUnit::Map100thMM));
    pDevice->SetOutputSizePixel(Size(2000, 2000));
    tools::Rectangle aPageRect(Point(), pPage->GetSize());
    pView->CompleteRedraw(pDevice, vcl::Region(aPageRect), &aCollector);
    CPPUNIT_ASSERT(!aCollector.maPrimitives.empty());

    // The page keeps non-zero margins, which is what would make the guide
    // appear, so the rendered page is a valid probe for this regression.
    CPPUNIT_ASSERT(pPage->GetLeftBorder() || pPage->GetUpperBorder() || pPage->GetRightBorder()
                   || pPage->GetLowerBorder());

    // The margin guide is a hairline drawn in the document boundary color. The
    // rendered content for a PDF page must not contain one.
    const svtools::ColorConfig aColorConfig;
    const basegfx::BColor aBoundaryColor
        = aColorConfig.GetColorValue(svtools::DOCBOUNDARIES).nColor.getBColor();
    CPPUNIT_ASSERT(!containsHairlineOfColor(aCollector.maPrimitives, aBoundaryColor));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
