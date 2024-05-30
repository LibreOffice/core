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

#include <unotools/tempfile.hxx>
#include <unotools/mediadescriptor.hxx>
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
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(2011, 2102), Size(2106 + 1, 298 + 1)),
                         pImportedObject->GetLogicRect());

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
        utl::MediaDescriptor aMediaDescriptor;
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
        mxComponent = loadFromDesktop(maTempFile.GetURL());
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
