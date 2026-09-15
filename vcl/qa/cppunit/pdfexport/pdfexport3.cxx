/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <memory>
#include <string_view>

#include <config_fonts.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <tools/zcodec.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <rtl/math.hxx>
#include <o3tl/string_view.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <printdata.hxx>
#include <unotxdoc.hxx>
#include <doc.hxx>
#include <docsh.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/pdf/PDFFormFieldType.hxx>
#include <vcl/pdf/PDFPageObjectType.hxx>
#include <vcl/pdfread.hxx>
#include <comphelper/propertyvalue.hxx>
#include <cmath>

#include <set>
static std::ostream& operator<<(std::ostream& rStream, const std::set<rtl::OString>& rSet);

#include <test/unoapi_test.hxx>

using namespace ::com::sun::star;

static std::ostream& operator<<(std::ostream& rStream, const std::set<OString>& rSet)
{
    rStream << "{ ";
    for (auto it = rSet.begin(); it != rSet.end(); ++it)
    {
        if (it != rSet.begin())
            rStream << ", ";
        rStream << *it;
    }
    rStream << " }";
    return rStream;
}

namespace
{
/// Tests the PDF export filter.
class PdfExportTest3 : public UnoApiTest
{
public:
    PdfExportTest3()
        : UnoApiTest(u"/vcl/qa/cppunit/pdfexport/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf157397)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "PDFUACompliance", uno::Any(true) },
        { "ExportFormFields", uno::Any(true) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"PDF_export_with_formcontrol.fodt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    vcl::filter::PDFObjectElement* pDocument(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
            if (pS1 && pS1->GetValue() == "Document")
            {
                pDocument = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pDocument);

    auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pDocument->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids1);
    // assume there are no MCID ref at this level
    auto pKids1v = pKids1->GetElements();
    auto pRefKid12 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[2]);
    CPPUNIT_ASSERT(pRefKid12);
    auto pObject12 = pRefKid12->LookupObject();
    CPPUNIT_ASSERT(pObject12);
    auto pType12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject12->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType12->GetValue());
    auto pS12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject12->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text body"_ostr, pS12->GetValue());

    auto pKids12 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject12->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids12);
    // assume there are no MCID ref at this level
    auto pKids12v = pKids12->GetElements();
    auto pRefKid120 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids12v[0]);
    CPPUNIT_ASSERT(pRefKid120);
    auto pObject120 = pRefKid120->LookupObject();
    CPPUNIT_ASSERT(pObject120);
    auto pType120 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject120->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType120->GetValue());
    auto pS120 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject120->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS120->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject120->Lookup("K"_ostr));
        int nMCID(0);
        int nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pObjR)
            {
                ++nRef;
                auto pOType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
                auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObjR->LookupElement("Obj"_ostr));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAContents = dynamic_cast<vcl::filter::PDFHexStringElement*>(
                    pAnnot->Lookup("Contents"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    u"wiki-seite"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAA
                    = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
                CPPUNIT_ASSERT(pAA);
                auto pAAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Action"_ostr, pAAType->GetValue());
                auto pAAS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("URI"_ostr, pAAS->GetValue());
                auto pAAURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pAA->LookupElement("URI"_ostr));
                CPPUNIT_ASSERT_EQUAL("https://klexikon.zum.de/wiki/Kl%C3%A4ranlage"_ostr,
                                     pAAURI->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, nMCID);
        CPPUNIT_ASSERT_EQUAL(1, nRef);
    }

    auto pRefKid13 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[3]);
    CPPUNIT_ASSERT(pRefKid13);
    auto pObject13 = pRefKid13->LookupObject();
    CPPUNIT_ASSERT(pObject13);
    auto pType13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject13->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType13->GetValue());
    auto pS13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject13->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text body"_ostr, pS13->GetValue());

    auto pKids13 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject13->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids13);
    // assume there are no MCID ref at this level
    auto pKids13v = pKids13->GetElements();
    auto pRefKid130 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids13v[0]);
    CPPUNIT_ASSERT(pRefKid130);
    auto pObject130 = pRefKid130->LookupObject();
    CPPUNIT_ASSERT(pObject130);
    auto pType130 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject130->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType130->GetValue());
    auto pS130 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject130->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS130->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject130->Lookup("K"_ostr));
        int nMCID(0);
        int nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pObjR)
            {
                ++nRef;
                auto pOType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
                auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObjR->LookupElement("Obj"_ostr));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAContents = dynamic_cast<vcl::filter::PDFHexStringElement*>(
                    pAnnot->Lookup("Contents"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    u"https://de.wikipedia.org/wiki/Kl\u00E4ranlage#Mechanische_Vorreinigung"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAA
                    = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
                CPPUNIT_ASSERT(pAA);
                auto pAAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Action"_ostr, pAAType->GetValue());
                auto pAAS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("URI"_ostr, pAAS->GetValue());
                auto pAAURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pAA->LookupElement("URI"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    "https://de.wikipedia.org/wiki/Kl%C3%A4ranlage#Mechanische_Vorreinigung"_ostr,
                    pAAURI->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, nMCID);
        CPPUNIT_ASSERT_EQUAL(1, nRef);
    }

    auto pRefKid14 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[4]);
    CPPUNIT_ASSERT(pRefKid14);
    auto pObject14 = pRefKid14->LookupObject();
    CPPUNIT_ASSERT(pObject14);
    auto pType14 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject14->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType14->GetValue());
    auto pS14 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject14->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text body"_ostr, pS14->GetValue());

    auto pKids14 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject14->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids14);
    // assume there are no MCID ref at this level
    auto pKids14v = pKids14->GetElements();
    auto pRefKid140 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids14v[0]);
    CPPUNIT_ASSERT(pRefKid140);
    auto pObject140 = pRefKid140->LookupObject();
    CPPUNIT_ASSERT(pObject140);
    auto pType140 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject140->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType140->GetValue());
    auto pS140 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject140->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS140->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject140->Lookup("K"_ostr));
        int nMCID(0);
        int nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pObjR)
            {
                ++nRef;
                auto pOType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
                auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObjR->LookupElement("Obj"_ostr));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAContents = dynamic_cast<vcl::filter::PDFHexStringElement*>(
                    pAnnot->Lookup("Contents"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    u"tour"_ustr, ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAA
                    = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
                CPPUNIT_ASSERT(pAA);
                auto pAAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Action"_ostr, pAAType->GetValue());
                auto pAAS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("URI"_ostr, pAAS->GetValue());
                auto pAAURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pAA->LookupElement("URI"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    "https://vr-easy.com/tour/usr/220113-virtuellerschulausflug/#pano=24"_ostr,
                    pAAURI->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, nMCID);
        CPPUNIT_ASSERT_EQUAL(1, nRef);
    }

    auto pRefKid16 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[6]);
    CPPUNIT_ASSERT(pRefKid16);
    auto pObject16 = pRefKid16->LookupObject();
    CPPUNIT_ASSERT(pObject16);
    auto pType16 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject16->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType16->GetValue());
    auto pS16 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject16->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text body"_ostr, pS16->GetValue());

    auto pKids16 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject16->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids16);
    // assume there are no MCID ref at this level
    auto pKids16v = pKids16->GetElements();
    auto pRefKid160 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids16v[0]);
    CPPUNIT_ASSERT(pRefKid160);
    auto pObject160 = pRefKid160->LookupObject();
    CPPUNIT_ASSERT(pObject160);
    auto pType160 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject160->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType160->GetValue());
    auto pS160 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject160->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Form"_ostr, pS160->GetValue());
    auto pA160Dict = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject160->Lookup("A"_ostr));
    CPPUNIT_ASSERT(pA160Dict);
    auto pA160O = dynamic_cast<vcl::filter::PDFNameElement*>(pA160Dict->LookupElement("O"_ostr));
    CPPUNIT_ASSERT(pA160O);
    CPPUNIT_ASSERT_EQUAL("PrintField"_ostr, pA160O->GetValue());
    auto pA160Role
        = dynamic_cast<vcl::filter::PDFNameElement*>(pA160Dict->LookupElement("Role"_ostr));
    CPPUNIT_ASSERT(pA160Role);
    CPPUNIT_ASSERT_EQUAL("tv"_ostr, pA160Role->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject160->Lookup("K"_ostr));
        int nMCID(0);
        int nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pObjR)
            {
                ++nRef;
                auto pOType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
                auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObjR->LookupElement("Obj"_ostr));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT_EQUAL("Widget"_ostr, pASubtype->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, nMCID);
        CPPUNIT_ASSERT_EQUAL(1, nRef);
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf135192)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf135192-1.fodp");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nTable(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
            if (pS1 && pS1->GetValue() == "Table")
            {
                int nTR(0);
                auto pKids1
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids1);
                // there can be additional children, such as MCID ref
                for (auto pKid1 : pKids1->GetElements())
                {
                    auto pRefKid1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKid1);
                    if (pRefKid1)
                    {
                        auto pObject2 = pRefKid1->LookupObject();
                        if (pObject2)
                        {
                            auto pType2 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                pObject2->Lookup("Type"_ostr));
                            if (pType2 && pType2->GetValue() == "StructElem")
                            {
                                auto pS2 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                    pObject2->Lookup("S"_ostr));
                                if (pS2 && pS2->GetValue() == "TR")
                                {
                                    int nTD(0);
                                    auto pKids2 = dynamic_cast<vcl::filter::PDFArrayElement*>(
                                        pObject2->Lookup("K"_ostr));
                                    CPPUNIT_ASSERT(pKids2);
                                    for (auto pKid2 : pKids2->GetElements())
                                    {
                                        auto pRefKid2
                                            = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                                                pKid2);
                                        if (pRefKid2)
                                        {
                                            auto pObject3 = pRefKid2->LookupObject();
                                            if (pObject3)
                                            {
                                                auto pType3
                                                    = dynamic_cast<vcl::filter::PDFNameElement*>(
                                                        pObject3->Lookup("Type"_ostr));
                                                if (pType3 && pType3->GetValue() == "StructElem")
                                                {
                                                    auto pS3 = dynamic_cast<
                                                        vcl::filter::PDFNameElement*>(
                                                        pObject3->Lookup("S"_ostr));
                                                    if (nTR == 0 && pS3 && pS3->GetValue() == "TH")
                                                    {
                                                        int nOTable(0);
                                                        auto pAttrs = dynamic_cast<
                                                            vcl::filter::PDFArrayElement*>(
                                                            pObject3->Lookup("A"_ostr));
                                                        CPPUNIT_ASSERT(pAttrs != nullptr);
                                                        for (const auto& rAttrRef :
                                                             pAttrs->GetElements())
                                                        {
                                                            auto pAttrDict = dynamic_cast<
                                                                vcl::filter::PDFDictionaryElement*>(
                                                                rAttrRef);
                                                            CPPUNIT_ASSERT(pAttrDict != nullptr);
                                                            auto pOwner = dynamic_cast<
                                                                vcl::filter::PDFNameElement*>(
                                                                pAttrDict->LookupElement("O"_ostr));
                                                            CPPUNIT_ASSERT(pOwner != nullptr);
                                                            if (pOwner->GetValue() == "Table")
                                                            {
                                                                auto pScope = dynamic_cast<
                                                                    vcl::filter::PDFNameElement*>(
                                                                    pAttrDict->LookupElement(
                                                                        "Scope"_ostr));
                                                                CPPUNIT_ASSERT(pScope != nullptr);
                                                                CPPUNIT_ASSERT_EQUAL(
                                                                    "Column"_ostr,
                                                                    pScope->GetValue());
                                                                ++nOTable;
                                                            }
                                                        }
                                                        CPPUNIT_ASSERT_EQUAL(1, nOTable);
                                                        ++nTD;
                                                    }
                                                    else if (nTR != 0 && pS3
                                                             && pS3->GetValue() == "TD")
                                                    {
                                                        ++nTD;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    CPPUNIT_ASSERT_EQUAL(3, nTD);
                                    ++nTR;
                                }
                            }
                        }
                    }
                }
                CPPUNIT_ASSERT_EQUAL(2, nTR);
                ++nTable;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nTable);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf154955)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"grouped-shape.fodt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    enum
    {
        Default,
        Artifact,
        Tagged
    } state
        = Default;

    int nLine(0);
    int nTagged(0);
    int nArtifacts(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n";
            if (o3tl::starts_with(line, "/Artifact "))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Artifact;
                ++nArtifacts;
            }
            else if (o3tl::starts_with(line, "/Figure<</MCID "))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Tagged;
                ++nTagged;
            }
            else if (line == "EMC")
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected end", state != Default);
                state = Default;
            }
            else if (nLine > 1) // first line is expected "0.1 w"
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected content outside MCS", state != Default);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unclosed MCS", Default, state);
    CPPUNIT_ASSERT_EQUAL(2, nTagged);
    CPPUNIT_ASSERT_GREATEREQUAL(1, nArtifacts);

    int nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pS && pS->GetValue() == "Figure")
            {
                switch (nFigure)
                {
                    case 0:
                        CPPUNIT_ASSERT_EQUAL(u"Two rectangles - Grouped"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                    case 1:
                        CPPUNIT_ASSERT_EQUAL(u"these ones are green"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                }

                auto pParentRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("P"_ostr));
                CPPUNIT_ASSERT(pParentRef);
                auto pParent(pParentRef->LookupObject());
                CPPUNIT_ASSERT(pParent);
                auto pParentType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pParentType->GetValue());
                auto pParentS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pParentS->GetValue());

                ++nFigure;
            }
        }
    }
    // the problem was that there were 4 shapes (the sub-shapes of the 2 groups)
    CPPUNIT_ASSERT_EQUAL(2, nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf155190)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf155190.odt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nDiv(0);
    int nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));

        auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
        // start with the text box
        if (pType1 && pType1->GetValue() == "StructElem" && pS1 && pS1->GetValue() == "Div")
        {
            ++nDiv;
            auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"_ostr));
            CPPUNIT_ASSERT(pKids1);
            for (auto pKid1 : pKids1->GetElements())
            {
                auto pRefKid1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKid1);
                if (pRefKid1)
                {
                    auto pObject2 = pRefKid1->LookupObject();
                    CPPUNIT_ASSERT(pObject2);
                    auto pType2
                        = dynamic_cast<vcl::filter::PDFNameElement*>(pObject2->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType2);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType2->GetValue());
                    auto pS2
                        = dynamic_cast<vcl::filter::PDFNameElement*>(pObject2->Lookup("S"_ostr));
                    CPPUNIT_ASSERT_EQUAL("FigureCaption"_ostr, pS2->GetValue());
                    auto pKids2
                        = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject2->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids2);
                    // there are additional children, MCID ref
                    for (auto pKid2 : pKids2->GetElements())
                    {
                        auto pRefKid2 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKid2);
                        if (pRefKid2)
                        {
                            auto pObject3 = pRefKid2->LookupObject();
                            CPPUNIT_ASSERT(pObject3);
                            auto pType3 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                pObject3->Lookup("Type"_ostr));
                            if (pType3 && pType3->GetValue() == "StructElem")
                            {
                                auto pS3 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                    pObject3->Lookup("S"_ostr));
                                CPPUNIT_ASSERT_EQUAL("Figure"_ostr, pS3->GetValue());
                                auto pAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                    pObject3->Lookup("Alt"_ostr));
                                CPPUNIT_ASSERT_EQUAL(
                                    u"Picture of apples"_ustr,
                                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAlt));
                                auto pKids3 = dynamic_cast<vcl::filter::PDFArrayElement*>(
                                    pObject3->Lookup("K"_ostr));
                                CPPUNIT_ASSERT(pKids3);
                                // the problem was that this didn't reference an MCID
                                CPPUNIT_ASSERT(!pKids3->GetElements().empty());
                                ++nFigure;
                            }
                        }
                    }
                }
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nDiv);
    CPPUNIT_ASSERT_EQUAL(1, nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testMediaShapeAnnot)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"vid.odt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(size_t(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    // check /Annot - produced by sw
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        "Annot"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        "Screen"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());

    auto pA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
    CPPUNIT_ASSERT(pA);
    auto pR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pA->LookupElement("R"_ostr));
    CPPUNIT_ASSERT(pR);
    auto pC = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pR->LookupElement("C"_ostr));
    CPPUNIT_ASSERT(pC);
    auto pCT = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pC->LookupElement("CT"_ostr));
    CPPUNIT_ASSERT_EQUAL("video/webm"_ostr, pCT->GetValue());
    auto pD = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pC->LookupElement("D"_ostr));
    CPPUNIT_ASSERT(pD);
    auto pDesc = dynamic_cast<vcl::filter::PDFHexStringElement*>(pD->LookupElement("Desc"_ostr));
    CPPUNIT_ASSERT(pDesc);
    CPPUNIT_ASSERT_EQUAL(u"alternativloser text\nand some description"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pDesc));
    auto pAlt = dynamic_cast<vcl::filter::PDFArrayElement*>(pC->LookupElement("Alt"_ostr));
    CPPUNIT_ASSERT(pAlt);
    auto pLang = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pAlt->GetElement(0));
    CPPUNIT_ASSERT_EQUAL(""_ostr, pLang->GetValue());
    auto pAltText = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAlt->GetElement(1));
    CPPUNIT_ASSERT_EQUAL(u"alternativloser text\nand some description"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAltText));

    auto pStructParent
        = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"_ostr));
    CPPUNIT_ASSERT(pStructParent);

    vcl::filter::PDFReferenceElement* pStructElemRef(nullptr);

    // check ParentTree to find StructElem
    int nRoots(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructTreeRoot")
        {
            ++nRoots;
            auto pParentTree = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                pObject1->Lookup("ParentTree"_ostr));
            CPPUNIT_ASSERT(pParentTree);
            auto pNumTree = pParentTree->LookupObject();
            CPPUNIT_ASSERT(pNumTree);
            auto pNums = dynamic_cast<vcl::filter::PDFArrayElement*>(pNumTree->Lookup("Nums"_ostr));
            CPPUNIT_ASSERT(pNums);
            int nFound(0);
            for (size_t i = 0; i < pNums->GetElements().size(); i += 2)
            {
                auto pI = dynamic_cast<vcl::filter::PDFNumberElement*>(pNums->GetElement(i));
                if (pI->GetValue() == pStructParent->GetValue())
                {
                    ++nFound;
                    CPPUNIT_ASSERT(i < pNums->GetElements().size() - 1);
                    pStructElemRef
                        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pNums->GetElement(i + 1));
                    CPPUNIT_ASSERT(pStructElemRef);
                }
            }
            CPPUNIT_ASSERT_EQUAL(1, nFound);
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nRoots);

    // check /StructElem - produced by drawinglayer
    CPPUNIT_ASSERT(pStructElemRef);
    auto pStructElem(pStructElemRef->LookupObject());
    CPPUNIT_ASSERT(pStructElem);

    auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType->GetValue());
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pS->GetValue());
    auto pSEAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(pStructElem->Lookup("Alt"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"alternativloser text - and some description"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pSEAlt));
    auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pStructElem->Lookup("K"_ostr));
    int nMCID(0);
    int nRef(0);
    for (size_t i = 0; i < pKids->GetElements().size(); ++i)
    {
        auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
        auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
        if (pNum)
        {
            ++nMCID;
        }
        if (pObjR)
        {
            ++nRef;
            auto pOType
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
            CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
            auto pAnnotRef
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->LookupElement("Obj"_ostr));
            CPPUNIT_ASSERT_EQUAL(pAnnot, pAnnotRef->LookupObject());
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nMCID);
    CPPUNIT_ASSERT_EQUAL(1, nRef);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testFlyFrameHyperlinkAnnot)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"image-hyperlink-alttext.fodt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(size_t(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    // check /Annot - produced by sw
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        "Annot"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        "Link"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());

    auto pContents
        = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Ship to Bugzilla"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pContents));

    auto pStructParent
        = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"_ostr));
    CPPUNIT_ASSERT(pStructParent);

    vcl::filter::PDFReferenceElement* pStructElemRef(nullptr);

    // check ParentTree to find StructElem
    int nRoots(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructTreeRoot")
        {
            ++nRoots;
            auto pParentTree = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                pObject1->Lookup("ParentTree"_ostr));
            CPPUNIT_ASSERT(pParentTree);
            auto pNumTree = pParentTree->LookupObject();
            CPPUNIT_ASSERT(pNumTree);
            auto pNums = dynamic_cast<vcl::filter::PDFArrayElement*>(pNumTree->Lookup("Nums"_ostr));
            CPPUNIT_ASSERT(pNums);
            int nFound(0);
            for (size_t i = 0; i < pNums->GetElements().size(); i += 2)
            {
                auto pI = dynamic_cast<vcl::filter::PDFNumberElement*>(pNums->GetElement(i));
                if (pI->GetValue() == pStructParent->GetValue())
                {
                    ++nFound;
                    CPPUNIT_ASSERT(i < pNums->GetElements().size() - 1);
                    pStructElemRef
                        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pNums->GetElement(i + 1));
                    CPPUNIT_ASSERT(pStructElemRef);
                }
            }
            CPPUNIT_ASSERT_EQUAL(1, nFound);
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nRoots);

    // check /StructElem - produced by sw painting code
    CPPUNIT_ASSERT(pStructElemRef);
    auto pStructElem(pStructElemRef->LookupObject());
    CPPUNIT_ASSERT(pStructElem);

    auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType->GetValue());
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS->GetValue());
    auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pStructElem->Lookup("K"_ostr));
    int nMCID(0);
    int nRef(0);
    for (size_t i = 0; i < pKids->GetElements().size(); ++i)
    {
        auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
        auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
        if (pNum)
        {
            ++nMCID;
        }
        if (pObjR)
        {
            ++nRef;
            auto pOType
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
            CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
            auto pAnnotRef
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->LookupElement("Obj"_ostr));
            CPPUNIT_ASSERT_EQUAL(pAnnot, pAnnotRef->LookupObject());
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nMCID);
    CPPUNIT_ASSERT_EQUAL(1, nRef);

    // the Link is inside a Figure
    auto pParentRef
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pStructElem->Lookup("P"_ostr));
    CPPUNIT_ASSERT(pParentRef);
    auto pParent(pParentRef->LookupObject());
    CPPUNIT_ASSERT(pParent);
    auto pParentType = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pParentType->GetValue());
    auto pParentS = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Figure"_ostr, pParentS->GetValue());
    auto pAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(pParent->Lookup("Alt"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Ship drawing - Very cute"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAlt));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testFormControlAnnot)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "PDFUACompliance", uno::Any(true) },
        { "ExportFormFields", uno::Any(true) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"formcontrol.fodt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(size_t(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    // check /Annot
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        "Annot"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        "Widget"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());
    auto pT = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pAnnot->Lookup("T"_ostr));
    CPPUNIT_ASSERT(pT);
    CPPUNIT_ASSERT_EQUAL("Check Box 1"_ostr, pT->GetValue());
    auto pTU = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("TU"_ostr));
    CPPUNIT_ASSERT(pTU);
    CPPUNIT_ASSERT_EQUAL(u"helpful text"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pTU));

    auto pStructParent
        = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"_ostr));
    CPPUNIT_ASSERT(pStructParent);

    vcl::filter::PDFReferenceElement* pStructElemRef(nullptr);

    // check ParentTree to find StructElem
    int nRoots(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructTreeRoot")
        {
            ++nRoots;
            auto pParentTree = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                pObject1->Lookup("ParentTree"_ostr));
            CPPUNIT_ASSERT(pParentTree);
            auto pNumTree = pParentTree->LookupObject();
            CPPUNIT_ASSERT(pNumTree);
            auto pNums = dynamic_cast<vcl::filter::PDFArrayElement*>(pNumTree->Lookup("Nums"_ostr));
            CPPUNIT_ASSERT(pNums);
            int nFound(0);
            for (size_t i = 0; i < pNums->GetElements().size(); i += 2)
            {
                auto pI = dynamic_cast<vcl::filter::PDFNumberElement*>(pNums->GetElement(i));
                if (pI->GetValue() == pStructParent->GetValue())
                {
                    ++nFound;
                    CPPUNIT_ASSERT(i < pNums->GetElements().size() - 1);
                    pStructElemRef
                        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pNums->GetElement(i + 1));
                    CPPUNIT_ASSERT(pStructElemRef);
                }
            }
            CPPUNIT_ASSERT_EQUAL(1, nFound);
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nRoots);

    // check /StructElem
    CPPUNIT_ASSERT(pStructElemRef);
    auto pStructElem(pStructElemRef->LookupObject());
    CPPUNIT_ASSERT(pStructElem);

    auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType->GetValue());
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Form"_ostr, pS->GetValue());
    auto pAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(pStructElem->Lookup("Alt"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"textuelle alternative - a box to check"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAlt));
    auto pA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pStructElem->Lookup("A"_ostr));
    CPPUNIT_ASSERT(pA);
    auto pO = dynamic_cast<vcl::filter::PDFNameElement*>(pA->LookupElement("O"_ostr));
    CPPUNIT_ASSERT(pO);
    CPPUNIT_ASSERT_EQUAL("PrintField"_ostr, pO->GetValue());
    auto pRole = dynamic_cast<vcl::filter::PDFNameElement*>(pA->LookupElement("Role"_ostr));
    CPPUNIT_ASSERT(pRole);
    CPPUNIT_ASSERT_EQUAL("cb"_ostr, pRole->GetValue());
    auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pStructElem->Lookup("K"_ostr));
    int nMCID(0);
    int nRef(0);
    for (size_t i = 0; i < pKids->GetElements().size(); ++i)
    {
        auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
        auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
        if (pNum)
        {
            ++nMCID;
        }
        if (pObjR)
        {
            ++nRef;
            auto pOType
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
            CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
            auto pAnnotRef
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->LookupElement("Obj"_ostr));
            CPPUNIT_ASSERT_EQUAL(pAnnot, pAnnotRef->LookupObject());
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nMCID);
    CPPUNIT_ASSERT_EQUAL(1, nRef);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf142129)
{
    loadFromFile(u"master.odm");

    // update linked section
    dispatchCommand(mxComponent, u".uno:UpdateAllLinks"_ustr, {});

    // Enable Outlines export
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "ExportBookmarks", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    auto* pCatalog = aDocument.GetCatalog();
    CPPUNIT_ASSERT(pCatalog);
    auto* pCatalogDictionary = pCatalog->GetDictionary();
    CPPUNIT_ASSERT(pCatalogDictionary);
    auto* pOutlinesObject = pCatalogDictionary->LookupObject("Outlines"_ostr);
    CPPUNIT_ASSERT(pOutlinesObject);
    auto* pOutlinesDictionary = pOutlinesObject->GetDictionary();
#if 0
    // Type isn't actually written currently
    auto* pType
        = dynamic_cast<vcl::filter::PDFNameElement*>(pOutlinesDictionary->LookupElement("Type"));
    CPPUNIT_ASSERT(pType);
    CPPUNIT_ASSERT_EQUAL(OString("Outlines"), pType->GetValue());
#endif

    auto* pFirst = dynamic_cast<vcl::filter::PDFReferenceElement*>(
        pOutlinesDictionary->LookupElement("First"_ostr));
    CPPUNIT_ASSERT(pFirst);
    auto* pFirstD = pFirst->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirstD);
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirstD->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(u"Preface"_ustr, ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                              *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                  pFirstD->LookupElement("Title"_ostr))));

    auto* pFirst1
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFirstD->LookupElement("First"_ostr));
    CPPUNIT_ASSERT(pFirst1);
    auto* pFirst1D = pFirst1->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirst1D);
    // here is a hidden section with headings "Copyright" etc.; check that
    // there are no outline entries for it
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirst1D->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(u"Who is this book for?"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                             *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                 pFirst1D->LookupElement("Title"_ostr))));

    auto* pFirst2
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFirst1D->LookupElement("Next"_ostr));
    auto* pFirst2D = pFirst2->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirst2D);
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirst2D->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(u"What\u2019s in this book?"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                             *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                 pFirst2D->LookupElement("Title"_ostr))));

    auto* pFirst3
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFirst2D->LookupElement("Next"_ostr));
    auto* pFirst3D = pFirst3->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirst3D);
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirst3D->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(u"Minimum requirements for using LibreOffice"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                             *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                 pFirst3D->LookupElement("Title"_ostr))));

    CPPUNIT_ASSERT_EQUAL(static_cast<vcl::filter::PDFElement*>(nullptr),
                         pFirst3D->LookupElement("Next"_ostr));
    CPPUNIT_ASSERT_EQUAL(static_cast<vcl::filter::PDFElement*>(nullptr),
                         pFirstD->LookupElement("Next"_ostr));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testPdfImageRotate180)
{
    // Create an empty document.
    loadFromURL(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    // Insert the PDF image.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphicObject(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    OUString aURL = createFileURL(u"pdf-image-rotate-180.pdf");
    xGraphicObject->setPropertyValue(u"GraphicURL"_ustr, uno::Any(aURL));
    uno::Reference<drawing::XShape> xShape(xGraphicObject, uno::UNO_QUERY);
    xShape->setSize(awt::Size(1000, 1000));
    uno::Reference<text::XTextContent> xTextContent(xGraphicObject, uno::UNO_QUERY);
    xText->insertTextContent(xCursor->getStart(), xTextContent, /*bAbsorb=*/false);

    // Save as PDF.
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Make sure that the page -> form -> form has a child image.
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pPageObject->getType());
    // 2: white background and the actual object.
    CPPUNIT_ASSERT_EQUAL(2, pPageObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pFormObject = pPageObject->getFormObject(1);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pFormObject->getType());
    CPPUNIT_ASSERT_EQUAL(1, pFormObject->getFormObjectCount());

    // Check if the inner form object (original page object in the pdf image) has the correct
    // rotation.
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pInnerFormObject = pFormObject->getFormObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pInnerFormObject->getType());
    CPPUNIT_ASSERT_EQUAL(1, pInnerFormObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pImage = pInnerFormObject->getFormObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Image, pImage->getType());
    basegfx::B2DHomMatrix aMat = pInnerFormObject->getMatrix();
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate = 0;
    double fShearX = 0;
    aMat.decompose(aScale, aTranslate, fRotate, fShearX);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -1
    // - Actual  : 1
    // i.e. the 180 degrees rotation didn't happen (via a combination of horizontal + vertical
    // flip).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aScale.getX(), 0.01);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf144222)
{
// Assume Windows has the font for U+4E2D
#ifdef _WIN32
    loadFromFile(u"tdf144222.ods");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    int nPageObjectCount = pPdfPage->getObjectCount();
    const OUString sChar = u"\u4E2D"_ustr;
    basegfx::B2DRectangle aRect1, aRect2;
    int nCount = 0;

    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPdfPageObject = pPdfPage->getObject(i);
        if (pPdfPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            ++nCount;
            OUString sText = pPdfPageObject->getText(pTextPage);
            if (sText == sChar)
                aRect1 = pPdfPageObject->getBounds();
            else
                aRect2 = pPdfPageObject->getBounds();
        }
    }

    CPPUNIT_ASSERT_EQUAL(2, nCount);
    CPPUNIT_ASSERT(!aRect1.isEmpty());
    CPPUNIT_ASSERT(!aRect2.isEmpty());
    CPPUNIT_ASSERT(!aRect1.overlaps(aRect2));
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf145873)
{
    // Import the bugdoc and export as PDF.
    loadFromFile(u"tdf145873.pptx");
    save(TestFilter::PDF_WRITER);

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    int nPageObjectCount = pPdfPage->getObjectCount();

    // tdf#145873: Without the fix #1 in place, this test would have failed with
    // - Expected: 66
    // - Actual  : 3
    CPPUNIT_ASSERT_EQUAL(66, nPageObjectCount);

    auto pObject = pPdfPage->getObject(4);
    CPPUNIT_ASSERT_MESSAGE("no object", pObject != nullptr);

    // tdf#145873: Without the fix #2 in place, this test would have failed with
    // - Expected: 13.40
    // - Actual  : 3.57...
    // - Delta   : 0.1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.40, pObject->getBounds().getWidth(), 0.1);
    // - Expected: 13.79
    // - Actual  : 3.74...
    // - Delta   : 0.1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.79, pObject->getBounds().getHeight(), 0.1);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testPdfImageHyperlink)
{
    // Given a Draw file, containing a PDF image, which has a hyperlink in it:
    // When saving to PDF:
    loadFromFile(u"pdf-image-hyperlink.odg");
    save(TestFilter::PDF_WRITER);

    // Then make sure that link is preserved:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    // Without the accompanying fix in place, this test would have failed, the hyperlink of the PDF
    // image was lost.
    CPPUNIT_ASSERT(pPdfPage->hasLinks());

    // Also test the precision of the form XObject.
    // Given a full-page form XObject, page height is 27.94 cm (792 points):
    // When writing the reciprocal of the object height to PDF:
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pFormObject;
    for (int i = 0; i < pPdfPage->getObjectCount(); ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pObject = pPdfPage->getObject(i);
        if (pObject->getType() == vcl::pdf::PDFPageObjectType::Form)
        {
            pFormObject = std::move(pObject);
            break;
        }
    }
    CPPUNIT_ASSERT(pFormObject);
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pInnerFormObject;
    for (int i = 0; i < pFormObject->getFormObjectCount(); ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pObject = pFormObject->getFormObject(i);
        if (pObject->getType() == vcl::pdf::PDFPageObjectType::Form)
        {
            pInnerFormObject = std::move(pObject);
            break;
        }
    }
    CPPUNIT_ASSERT(pInnerFormObject);
    // Then make sure that enough digits are used, so the point size is unchanged:
    basegfx::B2DHomMatrix aMatrix = pInnerFormObject->getMatrix();
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate{};
    double fShearX{};
    aMatrix.decompose(aScale, aTranslate, fRotate, fShearX);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0.0012626264
    // - Actual  : 0.00126
    // i.e. the rounded reciprocal was 794 points, not the original 792.
    CPPUNIT_ASSERT_EQUAL(0.001262626, rtl::math::round(aScale.getY(), 9));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testURIs)
{
    struct
    {
        OUString in;
        OString out;
        bool relativeFsys;
    } URIs[] = { {
                     u"http://example.com/"_ustr,
                     "http://example.com/"_ostr,
                     true,
                 },
                 {
                     u"file://localfile.odt/"_ustr,
                     "file://localfile.odt/"_ostr,
                     true,
                 },
                 {
                     // tdf 143216
                     u"http://username:password@example.com"_ustr,
                     "http://username:password@example.com"_ostr,
                     true,
                 },
                 {
                     u"git://git.example.org/project/example"_ustr,
                     "git://git.example.org/project/example"_ostr,
                     true,
                 },
                 {
                     // The odt/pdf gets substituted due to 'ConvertOOoTargetToPDFTarget'
                     u"filebypath.odt"_ustr,
                     "filebypath.pdf"_ostr,
                     true,
                 },
                 {
                     // The odt/pdf gets substituted due to 'ConvertOOoTargetToPDFTarget'
                     // but this time with ExportLinksRelativeFsys off the path is added
                     u"filebypath.odt"_ustr,
                     OUStringToOString(utl::GetTempNameBaseDirectory(), RTL_TEXTENCODING_UTF8)
                         + "filebypath.pdf",
                     false,
                 },
                 {
                     // This also gets made relative due to 'ExportLinksRelativeFsys'
                     utl::GetTempNameBaseDirectory() + "fileintempdir.odt",
                     "fileintempdir.pdf"_ostr,
                     true,
                 } };

    // Create an empty document.
    // Note: The test harness gets very upset if we try and create multiple
    // documents, or recreate it; so reuse one instance for all the links
    loadFromURL(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"Test pdf"_ustr, /*bAbsorb=*/false);

    // Set the name so it can do relative name replacement
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    xModel->attachResource(maTempFile.GetURL(), xModel->getArgs());

    for (const auto& uri : URIs)
    {
        // Test the filename rewriting
        uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
            { "ExportLinksRelativeFsys", uno::Any(uri.relativeFsys) },
            { "ConvertOOoTargetToPDFTarget", uno::Any(true) },
        }));
        comphelper::SequenceAsHashMap aMediaDescriptor;
        aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

        // Add a link (based on testNestedHyperlink in rtfexport3)
        xCursor->gotoStart(/*bExpand=*/false);
        xCursor->gotoEnd(/*bExpand=*/true);
        uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
        xCursorProps->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(uri.in));
        xCursorProps->setPropertyValue(u"HyperLinkName"_ustr, uno::Any(u"Testname"_ustr));

        // Save as PDF.
        save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

        // Use the filter rather than the pdfium route, as per the tdf105093 test, it's
        // easier to parse the annotations
        vcl::filter::PDFDocument aDocument;

        // Parse the export result.
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        CPPUNIT_ASSERT(aDocument.Read(aStream));

        // The document has one page.
        std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
        CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());
        auto pAnnots
            = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
        CPPUNIT_ASSERT(pAnnots);

        // There should be one annotation
        CPPUNIT_ASSERT_EQUAL(size_t(1), pAnnots->GetElements().size());
        auto pAnnotReference
            = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
        CPPUNIT_ASSERT(pAnnotReference);
        vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
        CPPUNIT_ASSERT(pAnnot);
        // We're expecting something like /Type /Annot /A << /Type /Action /S /URI /URI (path)
        CPPUNIT_ASSERT_EQUAL(
            "Annot"_ostr,
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
        CPPUNIT_ASSERT_EQUAL(
            "Link"_ostr,
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());
        auto pAction = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
        CPPUNIT_ASSERT(pAction);
        auto pURIElem = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
            pAction->LookupElement("URI"_ostr));
        CPPUNIT_ASSERT(pURIElem);
        // Check it matches
        CPPUNIT_ASSERT_EQUAL(uri.out, pURIElem->GetValue());
        // tdf#148934 check a11y
        CPPUNIT_ASSERT_EQUAL(u"Testname"_ustr, ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                   *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                       pAnnot->Lookup("Contents"_ostr))));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testPdfImageAnnots)
{
    // Given a document with a PDF image that has 2 comments (popup, text) and a hyperlink:
    // When saving to PDF:
    loadFromFile(u"pdf-image-annots.odg");
    save(TestFilter::PDF_WRITER);

    // Then make sure only the hyperlink is kept, since Draw itself has its own comments:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 3
    // i.e. not only the hyperlink but also the 2 comments were exported, leading to duplication.
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getAnnotationCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testPdfImageEncryption)
{
    // Given an empty document, with an inserted PDF image:
    loadFromURL(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphicObject(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    OUString aURL = createFileURL(u"rectangles.pdf");
    xGraphicObject->setPropertyValue(u"GraphicURL"_ustr, uno::Any(aURL));
    uno::Reference<drawing::XShape> xShape(xGraphicObject, uno::UNO_QUERY);
    xShape->setSize(awt::Size(1000, 1000));
    uno::Reference<text::XTextContent> xTextContent(xGraphicObject, uno::UNO_QUERY);
    xText->insertTextContent(xCursor->getStart(), xTextContent, /*bAbsorb=*/false);

    // When saving as encrypted PDF:
    save(TestFilter::PDF_WRITER, /*rParams*/ {}, /*pPassword*/ "secret");

    // Then make sure that the image is not lost:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get(), "secret"_ostr);
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pPageObject->getType());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    // i.e. instead of the white background and the actual form child, the image was lost due to
    // missing encryption.
    CPPUNIT_ASSERT_EQUAL(2, pPageObject->getFormObjectCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testBitmapScaledown)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // Given a document with an upscaled and rotated barcode bitmap in it:
    // When saving as PDF:
    loadFromFile(u"bitmap-scaledown.odt");
    save(TestFilter::PDF_WRITER);

    // Then verify that the bitmap is not downscaled:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    int nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Image)
            continue;

        std::unique_ptr<vcl::pdf::PDFiumBitmap> pBitmap = pPageObject->getImageBitmap();
        CPPUNIT_ASSERT(pBitmap);
        // In-file sizes: good is 2631x380, bad is 1565x14.
        int nWidth = pBitmap->getWidth();
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 2616
        // - Actual  : 1565
        // i.e. the bitmap in the pdf result was small enough to be blurry.
        CPPUNIT_ASSERT_EQUAL(2616, nWidth);
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf139627)
{
#if HAVE_MORE_FONTS
    loadFromFile(u"justified-arabic-kashida.odt");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // 7 objects, 3 text, others are path
    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(7, nPageObjectCount);

    // 3 text objects
    OUString sText[3];

    /* With "Noto Sans Arabic" font, these are the X ranges on Linux:
        0: ( 61.75 - 415.94)
        1: (479.70 - 422.40)
        2: (209.40 - 453.2)
    */
    basegfx::B2DRectangle aRect[3];

    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject;

    int nTextObjectCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            sText[nTextObjectCount] = pPageObject->getText(pTextPage);
            aRect[nTextObjectCount] = pPageObject->getBounds();
            ++nTextObjectCount;
        }
    }
    CPPUNIT_ASSERT_EQUAL(3, nTextObjectCount);

    // Text: جِـرم (which means "mass" in Persian)
    // Rendered as (left to right): "reh + mim" - "kasreh" - "jeh + tatweel"
    int rehmim = 0, kasreh = 1, jehtatweel = 2;

    CPPUNIT_ASSERT_EQUAL(u"رم"_ustr, sText[rehmim].trim());
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\xD8\xAC\xD9\x90"), sText[kasreh].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, sText[jehtatweel].trim());

    // "Kasreh" should be within "jeh" character
    CPPUNIT_ASSERT_GREATER(aRect[jehtatweel].getMinX(), aRect[kasreh].getMinX());
    CPPUNIT_ASSERT_LESS(aRect[jehtatweel].getMaxX(), aRect[kasreh].getMaxX());

    // "Tatweel" should cover "jeh" and "reh"+"mim" to avoid gap
    // Checking right gap
    //CPPUNIT_ASSERT_GREATER(aRect[jehtatweel].getMinX(), aRect[tatweel].getMaxX());
    // Checking left gap
    // Kashida fails to reach to rehmim before the series of patches starting
    // with 3901e029bd39575f700e69a73818565d62226a23. The visible symptom is
    // a gap in the left of Kashida.
    CPPUNIT_ASSERT_LESS(aRect[rehmim].getMaxX(), aRect[jehtatweel].getMinX());

    // Overlappings of Kashida and surrounding characters is ~9% of the width
    // of the "jeh" character, while using "Noto Arabic Sans" font in this
    // specific example.
    // We set the hard limit of 10% here.
    CPPUNIT_ASSERT_LESS(0.1, fabs(aRect[rehmim].getMaxX() - aRect[jehtatweel].getMinX())
                                 / aRect[jehtatweel].getWidth());
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testRexportRefToKids)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"ref-to-kids.pdf");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aPages.size());

    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);

    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);

    // Without the fix LookupObject for all /Im's will fail.
    for (auto const& rPair : pXObjects->GetItems())
    {
        if (rPair.first.startsWith("Im"))
            CPPUNIT_ASSERT(pXObjects->LookupObject(rPair.first));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testRexportFilterSingletonArray)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // the test fails with tagged PDF enabled
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "UseTaggedPDF", uno::Any(false) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"ref-to-kids.pdf");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aPages.size());

    // Directly go to the inner XObject Im5 that contains the rectangle drawings.
    auto pInnerIm = aDocument.LookupObject(5);
    CPPUNIT_ASSERT(pInnerIm);

    auto pFilter = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerIm->Lookup("Filter"_ostr));
    CPPUNIT_ASSERT(pFilter);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Filter must be FlateDecode", "FlateDecode"_ostr,
                                 pFilter->GetValue());

    vcl::filter::PDFStreamElement* pStream = pInnerIm->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    // Without the fix, the stream is doubly compressed,
    // hence one decompression will not yield the "re" expressions.
    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* pEnd = pStart + aUncompressed.GetSize();
    OString aImage = "100 0 30 50 re B*\n70 67 50 30 re B*\n"_ostr;
    auto it = std::search(pStart, pEnd, aImage.getStr(), aImage.getStr() + aImage.getLength());
    CPPUNIT_ASSERT(it != pEnd);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testRexportMediaBoxOrigin)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"ref-to-kids.pdf");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aPages.size());

    // Directly go to the inner XObject Im12 that contains the rectangle drawings in page 2.
    auto pInnerIm = aDocument.LookupObject(12);
    CPPUNIT_ASSERT(pInnerIm);

    static constexpr sal_Int32 aOrigin[2] = { -800, -600 };
    sal_Int32 aSize[2] = { 0, 0 };

    auto pBBox = dynamic_cast<vcl::filter::PDFArrayElement*>(pInnerIm->Lookup("BBox"_ostr));
    CPPUNIT_ASSERT(pBBox);
    const auto& rElements2 = pBBox->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(4), rElements2.size());
    for (sal_Int32 nIdx = 0; nIdx < 4; ++nIdx)
    {
        const auto* pNumElement = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements2[nIdx]);
        CPPUNIT_ASSERT(pNumElement);
        if (nIdx < 2)
            CPPUNIT_ASSERT_EQUAL(aOrigin[nIdx], static_cast<sal_Int32>(pNumElement->GetValue()));
        else
            aSize[nIdx - 2] = static_cast<sal_Int32>(pNumElement->GetValue()) - aOrigin[nIdx - 2];
    }

    auto pMatrix = dynamic_cast<vcl::filter::PDFArrayElement*>(pInnerIm->Lookup("Matrix"_ostr));
    CPPUNIT_ASSERT(pMatrix);
    const auto& rElements = pMatrix->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(6), rElements.size());
    sal_Int32 aMatTranslate[6]
        = { // Rotation by $\theta$ $cos(\theta), sin(\theta), -sin(\theta), cos(\theta)$
            0, -1, 1, 0,
            // Translate x,y
            -aOrigin[1] - aSize[1] / vcl::PDF_INSERT_MAGIC_SCALE_FACTOR / 2
                + aSize[0] / vcl::PDF_INSERT_MAGIC_SCALE_FACTOR / 2,
            aOrigin[0] + aSize[0] / vcl::PDF_INSERT_MAGIC_SCALE_FACTOR / 2
                + aSize[1] / vcl::PDF_INSERT_MAGIC_SCALE_FACTOR / 2
          };

    for (sal_Int32 nIdx = 0; nIdx < 6; ++nIdx)
    {
        const auto* pNumElement = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[nIdx]);
        CPPUNIT_ASSERT(pNumElement);
        CPPUNIT_ASSERT_EQUAL(aMatTranslate[nIdx], static_cast<sal_Int32>(pNumElement->GetValue()));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testRexportResourceItemReference)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"ref-to-kids.pdf");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aPages.size());

    // Directly go to the inner XObject Im12 that has reference to Font in page 2.
    auto pInnerIm = aDocument.LookupObject(12);
    CPPUNIT_ASSERT(pInnerIm);

    auto pResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pInnerIm->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pResources);
    auto pFontsReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pResources->LookupElement("Font"_ostr));
    CPPUNIT_ASSERT(pFontsReference);

    auto pFontsObject = pFontsReference->LookupObject();
    CPPUNIT_ASSERT(pFontsObject);

    auto pFontDict
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pFontsObject->Lookup("FF132"_ostr));
    CPPUNIT_ASSERT(pFontDict);

    auto pFontDescriptor = pFontDict->LookupObject("FontDescriptor"_ostr);
    CPPUNIT_ASSERT(pFontDescriptor);

    auto pFontWidths = pFontDict->LookupObject("Widths"_ostr);
    CPPUNIT_ASSERT(pFontWidths);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf152246)
{
    // Import the bugdoc and export as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "ExportFormFields", uno::Any(true) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"content-control-rtl.docx");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    // Position array
    static constexpr double aPos[5][4] = { { 55.699, 706.701, 132.401, 722.499 },
                                           { 197.499, 706.701, 274.201, 722.499 },
                                           { 302.349, 679.101, 379.051, 694.899 },
                                           { 479.599, 679.101, 556.301, 694.899 },
                                           { 55.699, 651.501, 132.401, 667.299 } };

    // Get page annotations.
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(size_t(5), pAnnots->GetElements().size());
    for (sal_Int32 i = 0; i < 5; ++i)
    {
        auto pAnnotReference
            = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[i]);
        CPPUNIT_ASSERT(pAnnotReference);
        vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
        CPPUNIT_ASSERT(pAnnot);
        CPPUNIT_ASSERT_EQUAL(
            "Annot"_ostr,
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
        CPPUNIT_ASSERT_EQUAL(
            "Widget"_ostr,
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());

        auto pRect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
        CPPUNIT_ASSERT(pRect);
        const auto& rElements = pRect->GetElements();
        CPPUNIT_ASSERT_EQUAL(size_t(4), rElements.size());
        for (sal_Int32 nIdx = 0; nIdx < 4; ++nIdx)
        {
            const auto* pNumElement = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[nIdx]);
            CPPUNIT_ASSERT(pNumElement);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(aPos[i][nIdx], pNumElement->GetValue(), 1e-6);
        }
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf155161)
{
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf155161.odt");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // Check that all fonts in the document are not Type 3 fonts.
    std::set<OString> aFontNames;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "Font")
        {
            auto pSubtype
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Subtype"_ostr));
            CPPUNIT_ASSERT(pSubtype);
            CPPUNIT_ASSERT("Type3"_ostr != pSubtype->GetValue());
            auto pName
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"_ostr));
            CPPUNIT_ASSERT(pName);
            aFontNames.insert(pName->GetValue().copy(7)); // skip the subset id
        }
    }

    // There must be two fonts
    std::set<OString> aExpected{ "Cantarell-Regular"_ostr, "Cantarell-Bold"_ostr };
    CPPUNIT_ASSERT_EQUAL(aExpected, aFontNames);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTrueTypeCompositeFont)
{
    // Subsets of fonts with glyf outlines are embedded as composite fonts, so
    // that the subset no longer needs a cmap of our own making.
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"SimpleTestDocument.fodt");
    save(TestFilter::PDF_WRITER);
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    vcl::filter::PDFObjectElement* pType0 = nullptr;
    vcl::filter::PDFObjectElement* pCIDFont = nullptr;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType || pType->GetValue() != "Font")
            continue;
        auto pSubtype = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Subtype"_ostr));
        CPPUNIT_ASSERT(pSubtype);
        if (pSubtype->GetValue() == "Type0")
            pType0 = pObject;
        else if (pSubtype->GetValue() == "CIDFontType2")
            pCIDFont = pObject;
    }
    CPPUNIT_ASSERT(pType0);
    CPPUNIT_ASSERT(pCIDFont);

    // The CIDs are the glyph IDs of the subset
    auto pCIDToGIDMap
        = dynamic_cast<vcl::filter::PDFNameElement*>(pCIDFont->Lookup("CIDToGIDMap"_ostr));
    CPPUNIT_ASSERT(pCIDToGIDMap);
    CPPUNIT_ASSERT_EQUAL("Identity"_ostr, pCIDToGIDMap->GetValue());

    // and it is not a simple font any more
    CPPUNIT_ASSERT(!pType0->Lookup("FirstChar"_ostr));
    CPPUNIT_ASSERT(!pType0->Lookup("Widths"_ostr));

    // The font program is still a TrueType one
    auto pDescriptorRef
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pCIDFont->Lookup("FontDescriptor"_ostr));
    CPPUNIT_ASSERT(pDescriptorRef);
    auto pDescriptor = pDescriptorRef->LookupObject();
    CPPUNIT_ASSERT(pDescriptor);
    CPPUNIT_ASSERT(pDescriptor->Lookup("FontFile2"_ostr));

    // and the text can still be extracted
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage);
    int nChars = pPdfTextPage->countChars();
    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = pPdfTextPage->getUnicode(i);
    CPPUNIT_ASSERT_EQUAL(u"This is a test document."_ustr, OUString(aChars.data(), aChars.size()));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testPDFA1CIDSet)
{
    // PDF/A-1 requires a CIDSet listing the CIDs of the font program.
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "SelectPdfVersion", uno::Any(sal_Int32(1)) }, // PDF/A-1b
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf171869.odt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    vcl::filter::PDFObjectElement* pCIDFont = nullptr;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pSubtype = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Subtype"_ostr));
        if (pSubtype && pSubtype->GetValue() == "CIDFontType0")
            pCIDFont = pObject;
    }
    CPPUNIT_ASSERT(pCIDFont);

    auto pDescriptorRef
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pCIDFont->Lookup("FontDescriptor"_ostr));
    CPPUNIT_ASSERT(pDescriptorRef);
    auto pDescriptor = pDescriptorRef->LookupObject();
    CPPUNIT_ASSERT(pDescriptor);

    auto pCIDSetRef
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pDescriptor->Lookup("CIDSet"_ostr));
    CPPUNIT_ASSERT(pCIDSetRef);
    auto pCIDSetObject = pCIDSetRef->LookupObject();
    CPPUNIT_ASSERT(pCIDSetObject);
    auto pCIDSetStream = pCIDSetObject->GetStream();
    CPPUNIT_ASSERT(pCIDSetStream);

    SvMemoryStream aCIDSet;
    ZCodec aCIDSetCodec;
    aCIDSetCodec.BeginCompression();
    pCIDSetStream->GetMemory().Seek(0);
    aCIDSetCodec.Decompress(pCIDSetStream->GetMemory(), aCIDSet);
    CPPUNIT_ASSERT(aCIDSetCodec.EndCompression());

    // The charset is identity, so the CIDs are those of the three glyphs of the
    // subset: one byte with the top three bits set, and nothing else.
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(1), aCIDSet.GetSize());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xE0), static_cast<const sal_uInt8*>(aCIDSet.GetData())[0]);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testNoCIDSetWithoutPDFA1)
{
    // Not PDF/A-1, so no CIDSet.
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf171869.odt");
    save(TestFilter::PDF_WRITER);
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    bool bSawDescriptor = false;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType || pType->GetValue() != "FontDescriptor")
            continue;
        bSawDescriptor = true;
        CPPUNIT_ASSERT(!pObject->Lookup("CIDSet"_ostr));
    }
    CPPUNIT_ASSERT(bSawDescriptor);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf171869)
{
    // Document using an embedded CID-keyed font (a Source Han Sans subset)
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf171869.odt");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The only fonts must be a Type 0 wrapper and its CIDFontType0 descendant.
    vcl::filter::PDFObjectElement* pType0 = nullptr;
    vcl::filter::PDFObjectElement* pCIDFont = nullptr;
    int nFonts = 0;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType || pType->GetValue() != "Font")
            continue;
        nFonts++;
        auto pSubtype = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Subtype"_ostr));
        CPPUNIT_ASSERT(pSubtype);
        if (pSubtype->GetValue() == "Type0")
            pType0 = pObject;
        else if (pSubtype->GetValue() == "CIDFontType0")
            pCIDFont = pObject;
    }
    CPPUNIT_ASSERT_EQUAL(2, nFonts);
    CPPUNIT_ASSERT(pType0);
    CPPUNIT_ASSERT(pCIDFont);

    auto pBaseFont = dynamic_cast<vcl::filter::PDFNameElement*>(pType0->Lookup("BaseFont"_ostr));
    CPPUNIT_ASSERT(pBaseFont);
    CPPUNIT_ASSERT_EQUAL("SourceHanSans-Regular"_ostr, pBaseFont->GetValue().copy(7));

    // The font program is a bare CFF (CIDFontType0C)
    auto pDescriptorRef
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pCIDFont->Lookup("FontDescriptor"_ostr));
    CPPUNIT_ASSERT(pDescriptorRef);
    auto pDescriptor = pDescriptorRef->LookupObject();
    CPPUNIT_ASSERT(pDescriptor);
    auto pFontFileRef
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pDescriptor->Lookup("FontFile3"_ostr));
    CPPUNIT_ASSERT(pFontFileRef);
    auto pFontFile = pFontFileRef->LookupObject();
    CPPUNIT_ASSERT(pFontFile);
    auto pFontFileSubtype
        = dynamic_cast<vcl::filter::PDFNameElement*>(pFontFile->Lookup("Subtype"_ostr));
    CPPUNIT_ASSERT(pFontFileSubtype);
    CPPUNIT_ASSERT_EQUAL("CIDFontType0C"_ostr, pFontFileSubtype->GetValue());

    auto pEncodingRef
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pType0->Lookup("Encoding"_ostr));
    CPPUNIT_ASSERT(pEncodingRef);
    auto pEncoding = pEncodingRef->LookupObject();
    CPPUNIT_ASSERT(pEncoding);
    auto pEncodingStream = pEncoding->GetStream();
    CPPUNIT_ASSERT(pEncodingStream);
    SvMemoryStream aObjectStream;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    pEncodingStream->GetMemory().Seek(0);
    aZCodec.Decompress(pEncodingStream->GetMemory(), aObjectStream);
    CPPUNIT_ASSERT(aZCodec.EndCompression());
    std::string aCMap(static_cast<const char*>(aObjectStream.GetData()), aObjectStream.GetSize());

    // The subset always has an identity charset, so the code of each glyph is
    // also its CID.
    CPPUNIT_ASSERT(aCMap.find("begincidrange\n"
                              "<00> <02> 0\n"
                              "endcidrange")
                   != std::string::npos);

    // Check the text can be extracted (i.e. the ToUnicode CMap works)
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage);
    int nChars = pPdfTextPage->countChars();
    CPPUNIT_ASSERT_EQUAL(2, nChars);
    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = pPdfTextPage->getUnicode(i);
    OUString aActualText(aChars.data(), aChars.size());
    CPPUNIT_ASSERT_EQUAL(u"世솅"_ustr, aActualText);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf48707_1)
{
    // Import the bugdoc and export as PDF.
    loadFromFile(u"tdf48707-1.fodt");
    save(TestFilter::PDF_WRITER);

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    auto pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    int nPageObjectCount = pPage->getObjectCount();

    CPPUNIT_ASSERT_EQUAL(6, nPageObjectCount);

    auto pTextPage = pPage->getTextPage();

    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(i);
        // The text and path objects (underline and overline) should all be red.
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, pPageObject->getFillColor());
        else
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, pPageObject->getStrokeColor());
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf48707_2)
{
    // Import the bugdoc and export as PDF.
    loadFromFile(u"tdf48707-2.fodt");
    save(TestFilter::PDF_WRITER);

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    auto pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    int nPageObjectCount = pPage->getObjectCount();

    CPPUNIT_ASSERT_EQUAL(13, nPageObjectCount);

    auto pTextPage = pPage->getTextPage();

    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Path)
            continue;

        // The table-like paths should be red, underline and overline should be black.
        if (i >= 8)
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPageObject->getStrokeColor());
        else
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, pPageObject->getStrokeColor());
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf156528)
{
    loadFromFile(u"wide_page1.fodt");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    // The document has two pages
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());

    // 1st page (5100 mm width x 210 mm high, UserUnit = 2)
    auto pPdfPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(5100.0 / 2, o3tl::Length::mm, o3tl::Length::pt),
                                 pPdfPage->getWidth(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(210.0 / 2, o3tl::Length::mm, o3tl::Length::pt),
                                 pPdfPage->getHeight(), 1);

    // 1 object (rectangle 5060 mm width x 170 mm high, UserUnit = 2)
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    auto pRect = pPdfPage->getObject(0);
    CPPUNIT_ASSERT(pRect);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Path, pRect->getType());
    auto bounds = pRect->getBounds();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(5060.0 / 2, o3tl::Length::mm, o3tl::Length::pt),
                                 bounds.getWidth(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(170.0 / 2, o3tl::Length::mm, o3tl::Length::pt),
                                 bounds.getHeight(), 1);

    // 2nd page (210 mm width x 297 mm high, UserUnit = 1)
    pPdfPage = pPdfDocument->openPage(1);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(210.0, o3tl::Length::mm, o3tl::Length::pt),
                                 pPdfPage->getWidth(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(297.0, o3tl::Length::mm, o3tl::Length::pt),
                                 pPdfPage->getHeight(), 1);

    // 1 object (rectangle 170 mm width x 257 mm high, UserUnit = 1)
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    pRect = pPdfPage->getObject(0);
    CPPUNIT_ASSERT(pRect);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Path, pRect->getType());
    bounds = pRect->getBounds();
    // Without the fix, this would fail with
    // - Expected: 481.889763779528
    // - Actual  : 241.925001144409
    // - Delta   : 1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(170.0, o3tl::Length::mm, o3tl::Length::pt),
                                 bounds.getWidth(), 1);
    //
    // - Expected: 728.503937007874
    // - Actual  : 365.25
    // - Delta   : 1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(257.0, o3tl::Length::mm, o3tl::Length::pt),
                                 bounds.getHeight(), 1);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf113866)
{
    loadFromFile(u"tdf113866.odt");

    // Set -- Printer Settings->Options->Print text in Black -- to true
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentDeviceAccess& rDocAccess = pDoc->getIDocumentDeviceAccess();
    SwPrintData aDocPrintData = rDocAccess.getPrintData();
    aDocPrintData.SetPrintBlackFont(true);
    rDocAccess.setPrintData(aDocPrintData);

    // Export to pdf
    save(TestFilter::PDF_WRITER);

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    // Non-NULL pPdfDocument means pdfium is available.
    if (pPdfDocument != nullptr)
    {
        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
        CPPUNIT_ASSERT(pPdfPage);

        int nPageObjectCount = pPdfPage->getObjectCount();
        for (int i = 0; i < nPageObjectCount; ++i)
        {
            std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);

            if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
                // Without the bug fix in place the test will fail with
                // - Expected: rgba[008000ff]
                // - Actual  : rgba[000000ff]
                // With the bug fixed, the green text in the test doc will stay green,
                // when exported to pdf, while Print Text in Black is true
                CPPUNIT_ASSERT_EQUAL(COL_GREEN, pPageObject->getFillColor());
        }
    }
}

// Form controls coordinates scrambled when exporting to pdf with unchecked form creation in Writer
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf159817)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "ExportFormFields", uno::Any(false) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    loadFromFile(u"tdf159817.fodt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    // So I extracted these values by using SAL_WARN(... << ...getMinimum()), but it appears
    // that the C++ stream operators do not output double values with sufficient resolution for me
    // to recreate those values in code, sigh, so resort to rounding things.
    auto roundPoint = [&pPdfPage](int i) {
        auto p = pPdfPage->getObject(i)->getBounds().getMinimum();
        return basegfx::B2DPoint(std::floor(p.getX() * 10) / 10.0,
                                 std::floor(p.getY() * 10) / 10.0);
    };
    // before the fix these co-ordinates would have been way further down the page
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(8.6, 677.3), roundPoint(13));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(9.3, 677.9), roundPoint(14));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(9.8, 678.5), roundPoint(15));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(85.0, 677.3), roundPoint(16));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(85.6, 677.9), roundPoint(17));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(170.1, 677.3), roundPoint(18));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(170.6, 677.9), roundPoint(19));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(0.0, 654.0), roundPoint(20));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(0.6, 654.6), roundPoint(21));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.3, 655.5), roundPoint(22));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.3, 655.5), roundPoint(23));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.2, 655.5), roundPoint(24));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.2, 655.5), roundPoint(25));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.4, 655.5), roundPoint(26));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.4, 655.5), roundPoint(27));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.1, 655.5), roundPoint(28));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.1, 655.5), roundPoint(29));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.5, 655.5), roundPoint(30));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.5, 655.5), roundPoint(31));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.0, 655.5), roundPoint(32));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.0, 655.5), roundPoint(33));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(28.3, 641.4), roundPoint(34));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(28.3, 623.7), roundPoint(35));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(28.3, 623.8), roundPoint(36));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(138.6, 623.7), roundPoint(37));
}

void GetPdfPageTextObjectsAndBounds(std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument,
                                    int nPage, std::vector<OUString>& rOutText,
                                    std::vector<basegfx::B2DRectangle>& rOutRect)
{
    CPPUNIT_ASSERT_GREATER(nPage, pPdfDocument->getPageCount());

    auto pPdfPage = pPdfDocument->openPage(nPage);
    CPPUNIT_ASSERT(pPdfPage);
    auto pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    int nPageObjectCount = pPdfPage->getObjectCount();

    for (int i = 0; i < nPageObjectCount; ++i)
    {
        auto pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            rOutText.push_back(pPageObject->getText(pTextPage));
            rOutRect.push_back(pPageObject->getBounds());
        }
    }
}

// Tests that kerning is correctly applied across color changes
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf61444)
{
    loadFromFile(u"tdf61444.odt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(4), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"Wait"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u"W"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_EQUAL(u"ai"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_EQUAL(u"t"_ustr, aText.at(3).trim());

    // Both lines should have the same kerning, so should end at approximately the same X coordinate
    auto solid_extent = aRect.at(0).getMaxX();
    auto color_extent = aRect.at(3).getMaxX();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(solid_extent, color_extent, /*delta*/ 0.15);
}

// tdf#124116 - Tests that track-changes inside a grapheme cluster does not break positioning
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf124116TrackUntrack)
{
    loadFromFile(u"tdf124116-hebrew-track-untrack.odt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    // The underlying document has 4 lines:
    // - שמחַ plain
    // - שמחַ tracked
    // - שמחַ with patah tracked
    // - שמחַ with everything except patah tracked
    // ---
    // However, due to the way text items are inserted for Hebrew, there will be 10:
    // - het with an improperly spaced patah, then שמ for the first 2 lines
    // - as above, followed by a blank for the next 2 representing the actual diacritic
    // ---
    // This test will likely need to be rewritten if tdf#158329 is fixed.
    CPPUNIT_ASSERT_EQUAL(size_t(10), aText.size());

    // All that matters for this test is that the patah is positioned well under the het
    auto het_x0 = aRect.at(4).getMinX();
    auto patah_x0 = aRect.at(6).getMinX();
    CPPUNIT_ASSERT_GREATER(10.0, patah_x0 - het_x0);

    auto het_x1 = aRect.at(7).getMinX();
    auto patah_x1 = aRect.at(9).getMinX();
    CPPUNIT_ASSERT_GREATER(10.0, patah_x1 - het_x1);
}

// tdf#134226 - Tests that shaping is not broken by invisible spans
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf134226)
{
    loadFromFile(u"tdf134226-shadda-in-hidden-span.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(8), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"ة"_ustr, aText[0].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[1].trim());
    CPPUNIT_ASSERT_EQUAL(u"ق\r\n\u0651"_ustr, aText[2].trim());
    CPPUNIT_ASSERT_EQUAL(u"ش"_ustr, aText[3].trim());
    CPPUNIT_ASSERT_EQUAL(u"ق\u0651"_ustr, aText[4].trim());
    CPPUNIT_ASSERT_EQUAL(u"ش"_ustr, aText[5].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[6].trim());
    CPPUNIT_ASSERT_EQUAL(u"ة"_ustr, aText[7].trim());

    // Verify that the corresponding text segments are positioned roughly equally
    auto fnEqualPos
        = [](const basegfx::B2DRectangle& stExpected, const basegfx::B2DRectangle& stFound) {
              CPPUNIT_ASSERT_DOUBLES_EQUAL(stExpected.getMinX(), stFound.getMinX(), /*delta*/ 0.15);
              CPPUNIT_ASSERT_DOUBLES_EQUAL(stExpected.getMaxX(), stFound.getMaxX(), /*delta*/ 0.15);
          };

    fnEqualPos(aRect[0], aRect[7]);
    fnEqualPos(aRect[1], aRect[6]);
    fnEqualPos(aRect[2], aRect[4]);
    fnEqualPos(aRect[3], aRect[5]);
}

// tdf#71956 - Tests that glyphs can be individually styled
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf71956)
{
    loadFromFile(u"tdf71956-styled-diacritics.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(12), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"ه"_ustr, aText[0].trim());
    CPPUNIT_ASSERT_EQUAL(u"ل\r\n\u064e\u0651\u0670"_ustr, aText[1].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[2].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[3].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[4].trim());
    CPPUNIT_ASSERT_EQUAL(u"ل"_ustr, aText[5].trim());
    CPPUNIT_ASSERT_EQUAL(u"ل"_ustr, aText[6].trim());
    CPPUNIT_ASSERT_EQUAL(u"ل\u064e\u0651\u0670"_ustr, aText[7].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[8].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[9].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[10].trim());
    CPPUNIT_ASSERT_EQUAL(u"ه"_ustr, aText[11].trim());

    // Verify that the corresponding text segments are positioned roughly equally
    auto fnEqualPos
        = [](const basegfx::B2DRectangle& stExpected, const basegfx::B2DRectangle& stFound) {
              CPPUNIT_ASSERT_DOUBLES_EQUAL(stExpected.getMinX(), stFound.getMinX(), /*delta*/ 0.15);
              CPPUNIT_ASSERT_DOUBLES_EQUAL(stExpected.getMaxX(), stFound.getMaxX(), /*delta*/ 0.15);
          };

    fnEqualPos(aRect[0], aRect[11]);
    fnEqualPos(aRect[1], aRect[10]);
    fnEqualPos(aRect[2], aRect[8]);
    fnEqualPos(aRect[3], aRect[9]);
    fnEqualPos(aRect[4], aRect[7]);
    fnEqualPos(aRect[5], aRect[6]);
}

// tdf#101686 - Verifies that drawinglayer clears RTL flags while drawing Writer text boxes
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf101686)
{
    loadFromFile(u"tdf101686.fodt");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first pace
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex*/ 0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(3, nPageObjectCount);

    std::vector<OUString> aText;

    int nTextObjectCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        auto pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            aText.push_back(pPageObject->getText(pTextPage));
            ++nTextObjectCount;
        }
    }

    CPPUNIT_ASSERT_EQUAL(3, nTextObjectCount);

    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr, aText[0].trim());

    // Without the fix, one of these two will be "xobtxeT"
    CPPUNIT_ASSERT_EQUAL(u"Textbox"_ustr, aText[1].trim());
    CPPUNIT_ASSERT_EQUAL(u"Textbox"_ustr, aText[2].trim());
}

// tdf#162161 reexport appears to have blank image
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testRexportXnViewColorspace)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"xnview-colorspace.pdf");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    // Get access to the only image on the only page.
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);

    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    auto pSubResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pSubResources);
    pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pSubResources->LookupElement("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pXObjects->GetItems().size());
    pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    pSubResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pSubResources);
    pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pSubResources->LookupElement("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pXObjects->GetItems().size());
    pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Dig all the way down to this element which is originally
    // 8 0 obj
    // /DeviceRGB
    // endobj
    // and appeared blank when we lost the /DeviceRGB line
    auto pColorspace = pXObject->LookupObject("ColorSpace"_ostr);
    CPPUNIT_ASSERT(pColorspace);
    auto pColorSpaceElement = pColorspace->GetNameElement();
    CPPUNIT_ASSERT(pColorSpaceElement);
    CPPUNIT_ASSERT_EQUAL("DeviceRGB"_ostr, pColorSpaceElement->GetValue());
}

// tdf#157390 - Verifies metrics are correct for PDF export mixing horizontal and vertical CJK
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf157390)
{
    loadFromFile(u"tdf157390-overlapping-kanji.fodt");
    save(TestFilter::PDF_WRITER);

    auto pPdfDocument = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    auto pPdfPage = pPdfDocument->openPage(/*nIndex*/ 0);
    CPPUNIT_ASSERT(pPdfPage);

    auto pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    // This bug manifests as aberrant character advances in the middle horizontal text.

    // Locate the text on the page
    auto aStr = u"無い有る有る"_ustr;

    int nBaseIndex = 0;
    for (int i = 0; i < pTextPage->countChars(); ++i)
    {
        if (pTextPage->getUnicode(i) == static_cast<unsigned int>(aStr[0]))
        {
            nBaseIndex = i;
            break;
        }
    }

    CPPUNIT_ASSERT(nBaseIndex + 6 <= pTextPage->countChars());

    // Extract the character rects
    std::vector<basegfx::B2DRectangle> aRects;
    for (int i = 0; i < 6; ++i)
    {
        auto nPageIndex = nBaseIndex + i;
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(aStr[i]), pTextPage->getUnicode(nPageIndex));
        aRects.push_back(pTextPage->getCharBox(nPageIndex, /*fPageHeight*/ 1000.0));
    }

    // Verify glyph advances don't exceed some pessimistic range
    double nGuess = aRects.at(0).getMinX();
    for (const auto& stRect : aRects)
    {
        std::cout << stRect << std::endl;

        CPPUNIT_ASSERT_GREATER(nGuess - 0.1 * stRect.getWidth(), stRect.getMinX());
        CPPUNIT_ASSERT_LESS(nGuess + 0.5 * stRect.getWidth(), stRect.getMinX());

        nGuess = stRect.getMaxX();
    }
}

// tdf#162205 - Verifies bidi portions on vertical left-to-right pages are rendered correctly
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf162205Ltr)
{
    loadFromFile(u"tdf162205-ltr.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(10), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"T"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u"h"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_EQUAL(u"e"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_EQUAL(u"\u0644"_ustr, aText.at(4).trim()); // lam
    CPPUNIT_ASSERT_EQUAL(u"\u0627"_ustr, aText.at(5).trim()); // alef
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_EQUAL(u"T"_ustr, aText.at(7).trim());
    CPPUNIT_ASSERT_EQUAL(u"h"_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_EQUAL(u"e"_ustr, aText.at(9).trim());

    // When the bug occurs, the Arabic portion is rendered far to the left of the English portions.
    // Verify that Arabic characters are within range.
    auto fnWithinRange = [](const auto& stExpected, const auto& stFound) {
        CPPUNIT_ASSERT_LESS(20.0, std::abs(stExpected.getMinX() - stFound.getMinX()));
    };

    fnWithinRange(aRect.at(0), aRect.at(4));
    fnWithinRange(aRect.at(7), aRect.at(5));
}

// tdf#162205 - Verifies bidi portions on vertical left-to-right pages are rendered correctly
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf162205Rtl)
{
    loadFromFile(u"tdf162205-rtl.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(10), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"T"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u"h"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_EQUAL(u"e"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_EQUAL(u"\u0644"_ustr, aText.at(4).trim()); // lam
    CPPUNIT_ASSERT_EQUAL(u"\u0627"_ustr, aText.at(5).trim()); // alef
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_EQUAL(u"T"_ustr, aText.at(7).trim());
    CPPUNIT_ASSERT_EQUAL(u"h"_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_EQUAL(u"e"_ustr, aText.at(9).trim());

    // When the bug occurs, the Arabic portion is rendered far to the left of the English portions.
    // Verify that Arabic characters are within range.
    auto fnWithinRange = [](const auto& stExpected, const auto& stFound) {
        CPPUNIT_ASSERT_LESS(20.0, std::abs(stExpected.getMinX() - stFound.getMinX()));
    };

    fnWithinRange(aRect.at(0), aRect.at(4));
    fnWithinRange(aRect.at(7), aRect.at(5));
}

// tdf#162194 - Verifies soft hyphens inside ligatures are rendered correctly.
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf162194SoftHyphen)
{
    loadFromFile(u"tdf162194-soft-hyphen.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(4), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"Waffle"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u"AAA Waf"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_EQUAL(u"-"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_EQUAL(u"fle"_ustr, aText.at(3).trim());
}

// tdf#160786 - Tests that Calc format code with repeat char is measured correctly
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf160786)
{
    loadFromFile(u"tdf160786.fods");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(5), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, aText.at(3).trim());

    // The currency line is padded with an unknown number of 'f' characters. It doesn't matter how
    // many are used, as long as the cell is padded to the expected width. Just verify that this
    // text object is the expected one.
    CPPUNIT_ASSERT(o3tl::trim(aText.at(4)).starts_with(u"$"));

    // The currency cell must not overlap the adjacent cell
    CPPUNIT_ASSERT_GREATEREQUAL(aRect.at(3).getMaxX(), aRect.at(4).getMinX());

    // The currency cell must be padded to occupy its space reasonably well.
    // As a heuristic, ensure the free space is no more than the width of "A"
    CPPUNIT_ASSERT_LESS(aRect.at(3).getMaxX() + aRect.at(3).getWidth(), aRect.at(4).getMinX());
}

// tdf#151748 - Textboxes should validate kashida positions
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf151748KashidaSpace)
{
    loadFromFile(u"tdf151748.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(17), aText.size());

    // Box 1: Not enough room for kashida
    CPPUNIT_ASSERT_EQUAL(u"تخوردگی خط"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u"وتوسط"_ustr, aText.at(1).trim());

    // Box 2: One kashida toward end
    CPPUNIT_ASSERT_EQUAL(u"وردگی"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(3).trim()); // Kashida
    CPPUNIT_ASSERT_EQUAL(u"تخ خط"_ustr, aText.at(4).trim());
    CPPUNIT_ASSERT_EQUAL(u"وتوسط"_ustr, aText.at(5).trim());

    // Box 3: Two kashida
    CPPUNIT_ASSERT_EQUAL(u"وردگی"_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(7).trim()); // Kashida
    CPPUNIT_ASSERT_EQUAL(u"تخ ط"_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(9).trim()); // Kashida
    CPPUNIT_ASSERT_EQUAL(u"خ"_ustr, aText.at(10).trim());
    CPPUNIT_ASSERT_EQUAL(u"وتوسط"_ustr, aText.at(11).trim());

    // Box 4: One kashida (text size change)
    CPPUNIT_ASSERT_EQUAL(u"خط"_ustr, aText.at(12).trim());
    CPPUNIT_ASSERT_EQUAL(u"وردگی"_ustr, aText.at(13).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(14).trim()); // Kashida
    CPPUNIT_ASSERT_EQUAL(u"تخ"_ustr, aText.at(15).trim());
    CPPUNIT_ASSERT_EQUAL(u"وتوسط"_ustr, aText.at(16).trim());
}

// tdf#163105 - Writer kashida justification should expand spaces
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf163105SwKashidaSpaceExpansion)
{
    loadFromFile(u"tdf163105-kashida-spaces.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(5), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"یده"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_EQUAL(u"کش ه"_ustr, aText.at(2).trim()); // This span is whitespace justified
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_EQUAL(u"نویس"_ustr, aText.at(4).trim());

    // Without the fix, this will be less than 25
    CPPUNIT_ASSERT_GREATER(150.0, aRect.at(2).getWidth());
}

// tdf#163105 - Writer should use font information when choosing kashida positions
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf163105Writer)
{
    loadFromFile(u"tdf163105-writer.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    // The fix allows kashida justification in this document.
    // Without the fix, this will be 1.
    CPPUNIT_ASSERT_EQUAL(size_t(5), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"ارسی"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_EQUAL(u"ف تن"_ustr, aText.at(2).trim()); // This span is whitespace justified
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_EQUAL(u"م"_ustr, aText.at(4).trim());

    // Without the fix, this will be greater than X
    CPPUNIT_ASSERT_LESS(170.0, aRect.at(2).getWidth());
}

// tdf#163105 - Edit Engine should use font information when choosing kashida positions
CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf163105Editeng)
{
    loadFromFile(u"tdf163105-editeng.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    // The fix allows kashida justification in this document.
    // Without the fix, this will be 1.
    CPPUNIT_ASSERT_EQUAL(size_t(5), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"ارسی"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_EQUAL(u"ف تن"_ustr, aText.at(2).trim()); // This span is whitespace justified
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_EQUAL(u"م"_ustr, aText.at(4).trim());

    CPPUNIT_ASSERT_LESS(170.0, aRect.at(2).getWidth());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf140767SyriacJustification)
{
    loadFromFile(u"tdf140767.odt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(11), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"ܝ"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u"ܛܺ"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_EQUAL(u"ܚܰ"_ustr, aText.at(4).trim());
    CPPUNIT_ASSERT_EQUAL(u"ܕ"_ustr, aText.at(5).trim()); // This span is whitespace justified
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_EQUAL(u"ܓܰ"_ustr, aText.at(7).trim());
    CPPUNIT_ASSERT_EQUAL(u"ܒ"_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(9).trim());
    CPPUNIT_ASSERT_EQUAL(u"ܐܰ"_ustr, aText.at(10).trim());

    // Without kashida justification, this space will be 224.328
    CPPUNIT_ASSERT_LESS(90.0, aRect.at(5).getWidth());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf36709FirstLineIndentEm)
{
    loadFromFile(u"tdf36709.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(16), aText.size());

    // Lines from the Writer portion
    CPPUNIT_ASSERT_EQUAL(u"0 em constant size"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(57.256, aRect.at(0).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"1 em constant size"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(69.856, aRect.at(1).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"2 em constant size"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(81.328, aRect.at(2).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"3 em constant size"_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(93.376, aRect.at(3).getMinX(), /*delta*/ 2.0);

    CPPUNIT_ASSERT_EQUAL(u"2 em variable size"_ustr, aText.at(4).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(81.328, aRect.at(4).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"2 em variable size"_ustr, aText.at(5).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(89.504, aRect.at(5).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"2 em variable size"_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(97.680, aRect.at(6).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"2 em variable size"_ustr, aText.at(7).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(105.856, aRect.at(7).getMinX(), /*delta*/ 2.0);

    // Lines from the Edit Engine portion
    CPPUNIT_ASSERT_EQUAL(u"0 em constant size"_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(62.106, aRect.at(8).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"1 em constant size"_ustr, aText.at(9).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(76.010, aRect.at(9).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"2 em constant size"_ustr, aText.at(10).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(88.778, aRect.at(10).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"3 em constant size"_ustr, aText.at(11).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(102.126, aRect.at(11).getMinX(), /*delta*/ 2.0);

    CPPUNIT_ASSERT_EQUAL(u"2 em variable size"_ustr, aText.at(12).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(88.778, aRect.at(12).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"2 em variable size"_ustr, aText.at(13).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(97.754, aRect.at(13).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"2 em variable size"_ustr, aText.at(14).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(106.830, aRect.at(14).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"2 em variable size"_ustr, aText.at(15).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(115.906, aRect.at(15).getMinX(), /*delta*/ 2.0);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf163913LeftRightMarginEm)
{
    loadFromFile(u"tdf163913.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(9), aText.size());

    // Lines from the Writer portion
    CPPUNIT_ASSERT_EQUAL(u"AAAAAAAAAAA"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.800, aRect.at(0).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"AAAA"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.800, aRect.at(1).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"AAAAAAAA"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(117.400, aRect.at(2).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"AAAAAAA"_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(117.400, aRect.at(3).getMinX(), /*delta*/ 2.0);

    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText.at(4).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.800, aRect.at(4).getMinX(), /*delta*/ 2.0);

    // Lines from the Edit Engine portion
    CPPUNIT_ASSERT_EQUAL(u"AAAAAAAAAAA"_ustr, aText.at(5).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.800, aRect.at(5).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"AAAA"_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.800, aRect.at(6).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"AAAAAAAA"_ustr, aText.at(7).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(123.750, aRect.at(7).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_EQUAL(u"AAAAAAA"_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(123.750, aRect.at(8).getMinX(), /*delta*/ 2.0);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testFormRoundtrip)
{
    // Loads and saves a PDF with filled forms. This checks the forms survive the round-trip.

    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // Need to properly set the PDF export options
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw_pdf_Export");
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "UseTaggedPDF", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;

    loadFromFile(u"FilledUpForm.pdf");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the round-tripped document with PDFium
    auto pPdfDocument = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    // Should be 1 page
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(1);

    // 5 annotations means 5 form fields
    CPPUNIT_ASSERT_EQUAL(5, pPage->getAnnotationCount());

    // Check each form
    {
        std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(0);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::CheckBox,
                             pAnnotation->getFormFieldType(pPdfDocument.get()));
    }

    {
        std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(1);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::ComboBox,
                             pAnnotation->getFormFieldType(pPdfDocument.get()));
    }

    {
        std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(2);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::TextField,
                             pAnnotation->getFormFieldType(pPdfDocument.get()));
    }

    {
        std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(3);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::TextField,
                             pAnnotation->getFormFieldType(pPdfDocument.get()));
    }

    {
        std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPage->getAnnotation(4);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFFormFieldType::TextField,
                             pAnnotation->getFormFieldType(pPdfDocument.get()));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf162750SmallCapsLigature)
{
    loadFromFile(u"tdf162750.fodt");
    save(TestFilter::PDF_WRITER);

    auto pPdfDocument = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    auto pPdfPage = pPdfDocument->openPage(/*nIndex*/ 0);
    CPPUNIT_ASSERT(pPdfPage);
    auto pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    int nPageObjectCount = pPdfPage->getObjectCount();

    CPPUNIT_ASSERT_EQUAL(3, nPageObjectCount);

    std::vector<OUString> aText;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        auto pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            aText.push_back(pPageObject->getText(pTextPage));
        }
    }

    CPPUNIT_ASSERT_EQUAL(size_t(3), aText.size());
    CPPUNIT_ASSERT_EQUAL(u"ffi"_ustr, aText.at(0).trim());

    // Without the fix, this will be "ffi"
    CPPUNIT_ASSERT_EQUAL(u"f"_ustr, aText.at(1).trim());

    CPPUNIT_ASSERT_EQUAL(u"FI"_ustr, aText.at(2).trim());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf164106SplitReorderedClusters)
{
    loadFromFile(u"tdf164106.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(14), aText.size());

    auto fnCompareIndices = [&](size_t nSplit, size_t nCombined) {
        CPPUNIT_ASSERT_EQUAL(aText.at(nSplit).trim(), aText.at(nCombined).trim());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aRect.at(nSplit).getMinX(), aRect.at(nCombined).getMinX(),
                                     /*delta*/ 0.2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aRect.at(nSplit).getMaxX(), aRect.at(nCombined).getMaxX(),
                                     /*delta*/ 0.2);
    };

    fnCompareIndices(0, 7);
    fnCompareIndices(1, 8);
    fnCompareIndices(2, 9);
    fnCompareIndices(3, 10);
    fnCompareIndices(4, 11);
    fnCompareIndices(5, 12);
    fnCompareIndices(6, 13);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testPDFAttachmentsWithEncryptedFile)
{
    // Encrypt the document and use the hybrid mode.
    // The original ODF document will be saved to the PDF as an attachment.

    uno::Sequence<beans::PropertyValue> aFilterData
        = { comphelper::makePropertyValue("IsAddStream", true) };
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor["FilterData"] <<= aFilterData;

    loadFromFile(u"SimpleTestDocument.fodt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList(),
         /*pPassword*/ "secret");

    // Parse the round-tripped document with PDFium
    auto pPdfDocument = parsePDFExport(vcl::pdf::PDFiumLibrary::get(), "secret"_ostr);

    // Should be 1 page
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Should have 1 attachment
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getAttachmentCount());

    // Get the attachment
    auto pAttachment = pPdfDocument->getAttachment(0);
    CPPUNIT_ASSERT(pAttachment);

    // Check the filename of the attachment
    CPPUNIT_ASSERT_EQUAL(u"Original.odt"_ustr, pAttachment->getName());

    // Write the attachment to the buffer
    std::vector<sal_uInt8> aBuffer;
    CPPUNIT_ASSERT(pAttachment->getFile(aBuffer));
    CPPUNIT_ASSERT_GREATER(size_t(0), aBuffer.size());

    // Create a temp file and store the content of the attachment
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    {
        SvFileStream aOutputStream(aTempFile.GetURL(), StreamMode::WRITE | StreamMode::TRUNC);
        aOutputStream.WriteBytes(aBuffer.data(), aBuffer.size());
    }

    dispose();
    // Load the attached document from the temp file
    UnoApiTest::loadFromURL(aTempFile.GetURL());

    // Check the content - first paragraph
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextDocument.is());
    uno::Reference<container::XEnumerationAccess> xParagraphEnumAccess(xTextDocument->getText(),
                                                                       uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraphEnumAccess.is());
    uno::Reference<container::XEnumeration> xParagraphEnum
        = xParagraphEnumAccess->createEnumeration();
    uno::Reference<text::XTextContent> const xElement(xParagraphEnum->nextElement(),
                                                      uno::UNO_QUERY);
    CPPUNIT_ASSERT(xElement.is());
    uno::Reference<text::XTextRange> const xParagraph(xElement, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());

    CPPUNIT_ASSERT_EQUAL(u"This is a test document."_ustr, xParagraph->getString());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTextBoxRuby)
{
    // This test exercises a work-in-progress Edit Engine ruby feature.
    // It is expected that this test will fail and need to be updated
    // as the feature is refined.

    loadFromFile(u"textbox-ruby.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(17), aText.size());

    // Lines from the Writer portion
    CPPUNIT_ASSERT_EQUAL(u"Prototype test for ruby characters in Edit Engine"_ustr,
                         aText.at(0).trim());

    // Lines from the Edit Engine portion
    CPPUNIT_ASSERT_EQUAL(u"Left-aligned:"_ustr, aText.at(1).trim());

    CPPUNIT_ASSERT_EQUAL(u"top1"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(171.0, aRect.at(2).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(734.0, aRect.at(2).getMaxY(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_EQUAL(u"BASE"_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(171.0, aRect.at(3).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(719.0, aRect.at(3).getMinY(), /*delta*/ 5.0);

    CPPUNIT_ASSERT_EQUAL(u"Centered:"_ustr, aText.at(4).trim());

    CPPUNIT_ASSERT_EQUAL(u"top2"_ustr, aText.at(5).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(165.0, aRect.at(5).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(715.0, aRect.at(5).getMaxY(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_EQUAL(u"BASE"_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(156.0, aRect.at(6).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(701.0, aRect.at(6).getMinY(), /*delta*/ 5.0);

    CPPUNIT_ASSERT_EQUAL(u"Right-aligned:"_ustr, aText.at(7).trim());

    CPPUNIT_ASSERT_EQUAL(u"top3"_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(198.0, aRect.at(8).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(697.0, aRect.at(8).getMaxY(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_EQUAL(u"BASE"_ustr, aText.at(9).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(178.0, aRect.at(9).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(682.0, aRect.at(9).getMinY(), /*delta*/ 5.0);

    CPPUNIT_ASSERT_EQUAL(u"Below:"_ustr, aText.at(10).trim());

    CPPUNIT_ASSERT_EQUAL(u"top4"_ustr, aText.at(11).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(153.0, aRect.at(11).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(667.0, aRect.at(11).getMaxY(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_EQUAL(u"BASE"_ustr, aText.at(12).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(144.0, aRect.at(12).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(668.0, aRect.at(12).getMinY(), /*delta*/ 5.0);

    CPPUNIT_ASSERT_EQUAL(u"Line wrapped: other"_ustr, aText.at(13).trim());

    CPPUNIT_ASSERT_EQUAL(u"top5"_ustr, aText.at(14).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(133.0, aRect.at(14).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(650.0, aRect.at(14).getMaxY(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_EQUAL(u"BASE BASE"_ustr, aText.at(15).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(106.0, aRect.at(15).getMinX(), /*delta*/ 5.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(636.0, aRect.at(15).getMinY(), /*delta*/ 5.0);

    CPPUNIT_ASSERT_EQUAL(u"other"_ustr, aText.at(16).trim());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf166044ContFootnoteOnlyOnePgNum)
{
    loadFromFile(u"tdf166044-cont-footnote-one-pgnum.fodt");
    save(TestFilter::PDF_WRITER);

    auto pPdfDocument = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());

    auto pPdfPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPdfPage);
    auto pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(36, nPageObjectCount);

    auto pContNoticeObject = pPdfPage->getObject(33);
    CPPUNIT_ASSERT(pContNoticeObject);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Text, pContNoticeObject->getType());

    // Without the fix, this assert would fail with:
    // - Expected: ມະນດ
    // - Actual  : ມະນດ2
    CPPUNIT_ASSERT_EQUAL(u"ມະນດ"_ustr, pContNoticeObject->getText(pTextPage));

    auto pPgNumObject = pPdfPage->getObject(34);
    CPPUNIT_ASSERT(pPgNumObject);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Text, pPgNumObject->getType());
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, pPgNumObject->getText(pTextPage));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf118350StartEndParaAlign)
{
    loadFromFile(u"tdf118350-start-end-para-align.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(24), aText.size());

    // Lines from the Writer portion
    CPPUNIT_ASSERT_EQUAL(u"This paragraph is LTR aligned left."_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(57.0, aRect.at(0).getMinX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is LTR aligned start."_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(57.0, aRect.at(1).getMinX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is LTR aligned right."_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(550.0, aRect.at(2).getMaxX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is LTR aligned end."_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(550.0, aRect.at(3).getMaxX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is RTL aligned left"_ustr, aText.at(4).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(5).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(57.0, aRect.at(5).getMinX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is RTL aligned start"_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(7).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(550.0, aRect.at(6).getMaxX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is RTL aligned right"_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(9).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(550.0, aRect.at(8).getMaxX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is RTL aligned end"_ustr, aText.at(10).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(11).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(57.0, aRect.at(11).getMinX(), /*delta*/ 10.0);

    // Lines from the Edit Engine portion
    CPPUNIT_ASSERT_EQUAL(u"This paragraph is LTR aligned left."_ustr, aText.at(12).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(99.0, aRect.at(12).getMinX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is LTR aligned start."_ustr, aText.at(13).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(99.0, aRect.at(13).getMinX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is LTR aligned right."_ustr, aText.at(14).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(537.0, aRect.at(14).getMaxX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is LTR aligned end."_ustr, aText.at(15).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(537.0, aRect.at(15).getMaxX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is RTL aligned left"_ustr, aText.at(16).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(17).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(99.0, aRect.at(17).getMinX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is RTL aligned start"_ustr, aText.at(18).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(19).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(537.0, aRect.at(18).getMaxX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is RTL aligned right"_ustr, aText.at(20).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(21).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(537.0, aRect.at(20).getMaxX(), /*delta*/ 10.0);

    CPPUNIT_ASSERT_EQUAL(u"This paragraph is RTL aligned end"_ustr, aText.at(22).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(23).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(99.0, aRect.at(23).getMinX(), /*delta*/ 10.0);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf72640LabelAlignIgnoresDirOff)
{
    loadFromFile(u"tdf72640-label-align-ignores-dir-off.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*page*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(10), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u"LTR start"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(80.0, aRect.at(0).getMinX(), /*delta*/ 1.0);

    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_EQUAL(u"RTL start"_ustr, aText.at(4).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(535.0, aRect.at(2).getMaxX(), /*delta*/ 1.0);

    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aText.at(5).trim());
    CPPUNIT_ASSERT_EQUAL(u"LTR end"_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0, aRect.at(5).getMinX(), /*delta*/ 1.0);

    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, aText.at(7).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_EQUAL(u"RTL end"_ustr, aText.at(9).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(585.0, aRect.at(7).getMaxX(), /*delta*/ 1.0);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf72640LabelAlignIgnoresDirOn)
{
    loadFromFile(u"tdf72640-label-align-ignores-dir-on.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*page*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(10), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u"LTR start"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(80.0, aRect.at(0).getMinX(), /*delta*/ 1.0);

    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_EQUAL(u"RTL start"_ustr, aText.at(4).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(585.0, aRect.at(2).getMaxX(), /*delta*/ 1.0);

    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aText.at(5).trim());
    CPPUNIT_ASSERT_EQUAL(u"LTR end"_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0, aRect.at(5).getMinX(), /*delta*/ 1.0);

    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, aText.at(7).trim());
    CPPUNIT_ASSERT_EQUAL(u"."_ustr, aText.at(8).trim());
    CPPUNIT_ASSERT_EQUAL(u"RTL end"_ustr, aText.at(9).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(535.0, aRect.at(7).getMaxX(), /*delta*/ 1.0);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, tdf150076BackgroundPdf)
{
    loadFromFile(u"tdf150076.odt");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    // Without the fix in place, this fails with:
    // equality assertion failed
    // - Expected: 5
    // - Actual  : 3
    // Where 5 means form (i.e. embedded PDF) and 3 means it's just an image
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pPageObject->getType());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf155557UnderlineKashidaPortion)
{
    loadFromFile(u"tdf155557-underline-kashida-portion.fodt");
    save(TestFilter::PDF_WRITER);

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    auto pPdfPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPdfPage);

    std::vector<basegfx::B2DRectangle> aStrokeRect;

    int nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        auto pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Path)
        {
            aStrokeRect.push_back(pPageObject->getBounds());
        }
    }

    CPPUNIT_ASSERT_EQUAL(size_t(2), aStrokeRect.size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(547.8, aStrokeRect.at(0).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(555.9, aStrokeRect.at(0).getMaxX(), /*delta*/ 1.0);

    // Without the fix in place, this fails with:
    // double equality assertion failed
    // - Expected: 56.1
    // - Actual  : 534.2
    // - Delta   : 1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.1, aStrokeRect.at(1).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(548.9, aStrokeRect.at(1).getMaxX(), /*delta*/ 1.0);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testTdf117941RtlStrikeoutChars)
{
    loadFromFile(u"tdf117941-rtl-strike-chars.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*page*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(8), aText.size());

    CPPUNIT_ASSERT_EQUAL(u"AAAAAAAA"_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.8, aRect.at(0).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(261.2, aRect.at(0).getMaxX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_EQUAL(u"//////////////"_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.8, aRect.at(1).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(264.7, aRect.at(1).getMaxX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_EQUAL(u"אאאאאאאא"_ustr, aText.at(2).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(354.5, aRect.at(2).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(553.8, aRect.at(2).getMaxX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_EQUAL(u"//////////////"_ustr, aText.at(3).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(353.5, aRect.at(3).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(558.3, aRect.at(3).getMaxX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_EQUAL(u"AAAAAAAA"_ustr, aText.at(4).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.8, aRect.at(4).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(261.2, aRect.at(4).getMaxX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_EQUAL(u"XXXXXXXXX"_ustr, aText.at(5).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(56.8, aRect.at(5).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(267.6, aRect.at(5).getMaxX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_EQUAL(u"אאאאאאאא"_ustr, aText.at(6).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(354.5, aRect.at(6).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(553.8, aRect.at(6).getMaxX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_EQUAL(u"XXXXXXXXX"_ustr, aText.at(7).trim());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(353.1, aRect.at(7).getMinX(), /*delta*/ 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(565.2, aRect.at(7).getMaxX(), /*delta*/ 1.0);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest3, testDropCapPaint)
{
    loadFromFile(u"drop_fly_paint.fodt");
    save(TestFilter::PDF_WRITER);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;
    GetPdfPageTextObjectsAndBounds(parsePDFExport(vcl::pdf::PDFiumLibrary::get()), /*nPage*/ 0,
                                   aText, aRect);

    CPPUNIT_ASSERT_EQUAL(size_t(7), aText.size());

    // Rest of the drop-capped paragraph's 3 forced lines.
    CPPUNIT_ASSERT_EQUAL(u"rop Cap Painting Test Document."_ustr, aText.at(0).trim());
    CPPUNIT_ASSERT_EQUAL(u"Second forced line of the drop cap paragraph."_ustr, aText.at(1).trim());
    CPPUNIT_ASSERT_EQUAL(u"Third forced line of the drop cap paragraph."_ustr, aText.at(2).trim());

    // The drop cap glyph itself must be non-whitespace.
    CPPUNIT_ASSERT_EQUAL(u"D"_ustr, aText.at(3).trim());

    // Must be noticeably taller than a body line (spans 3 lines), not
    // just present-but-same-height.
    double nDropCapHeight = aRect.at(3).getMaxY() - aRect.at(3).getMinY();
    double nBodyLineHeight = aRect.at(2).getMaxY() - aRect.at(2).getMinY();
    CPPUNIT_ASSERT(nDropCapHeight > nBodyLineHeight * 2.5);

    // Sits left of the wrapped text beside it, not overlapping/underneath.
    CPPUNIT_ASSERT(aRect.at(3).getMaxX() < aRect.at(0).getMinX());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(61.33, aRect.at(3).getMinX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(94.00, aRect.at(3).getMaxX(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(696.45, aRect.at(3).getMinY(), /*delta*/ 2.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(734.43, aRect.at(3).getMaxY(), /*delta*/ 2.0);

    // Second paragraph: text wrapping around the anchored fly frame.
    CPPUNIT_ASSERT_EQUAL(u"Fly frame paragraph line one."_ustr, aText.at(4).trim());
    CPPUNIT_ASSERT_EQUAL(u"Fly frame paragraph line two."_ustr, aText.at(5).trim());
    CPPUNIT_ASSERT_EQUAL(u"Fly frame paragraph line three."_ustr, aText.at(6).trim());

    // Every line starts well to the right of the drop-cap paragraph's own
    // margin, confirming wrap="parallel" is actually pushing text aside.
    CPPUNIT_ASSERT(aRect.at(4).getMinX() > aRect.at(0).getMinX() + 10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(114.53, aRect.at(4).getMinX(), /*delta*/ 2.0);
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
