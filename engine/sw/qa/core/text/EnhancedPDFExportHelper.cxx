/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <algorithm>
#include <unordered_set>
#include <vector>

#include <comphelper/propertyvalue.hxx>
#include <tools/stream.hxx>
#include <vcl/filter/pdfdocument.hxx>

namespace
{
/// Covers sw/source/core/text/EnhancedPDFExportHelper.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf171022)
{
    createSwDoc("structure-destinations.fodt");

    cpo::uno::Sequence aFilterData{ comphelper::makePropertyValue(u"UseTaggedPDF"_ustr, true),
                                    comphelper::makePropertyValue(u"SelectPdfVersion"_ustr,
                                                                  sal_Int32(20)) };
    save(TestFilter::PDF_WRITER,
         { comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData) });

    vcl::filter::PDFDocument aDocument;
    CPPUNIT_ASSERT(aDocument.Read(*maTempFile.GetStream(StreamMode::READ)));

    // the outline item, the footnote's two links, the shape's link and the citation's
    std::vector<OString> aOutlineTypes;
    std::vector<OString> aLinkTypes;
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pAction = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject->Lookup("A"_ostr));
        if (!pAction)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pAction->LookupElement("S"_ostr));
        if (!pType || pType->GetValue() != "GoTo")
            continue;
        // ISO 14289-2 8.8: the destination inside the document is a structure destination, so
        // it names the element and not only the page. veraPDF passes a structure destination
        // that points at nothing, so check what it points at.
        auto pPage = dynamic_cast<vcl::filter::PDFArrayElement*>(pAction->LookupElement("D"_ostr));
        auto pStructure
            = dynamic_cast<vcl::filter::PDFArrayElement*>(pAction->LookupElement("SD"_ostr));
        CPPUNIT_ASSERT(pPage);
        CPPUNIT_ASSERT(pStructure);
        CPPUNIT_ASSERT(!pPage->GetElements().empty());
        CPPUNIT_ASSERT(!pStructure->GetElements().empty());

        auto pPageRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pPage->GetElements()[0]);
        auto pStructureRef
            = dynamic_cast<vcl::filter::PDFReferenceElement*>(pStructure->GetElements()[0]);
        CPPUNIT_ASSERT(pPageRef);
        CPPUNIT_ASSERT(pStructureRef);

        CPPUNIT_ASSERT(pPageRef->LookupObject());
        CPPUNIT_ASSERT(pStructureRef->LookupObject());
        auto pPageType = dynamic_cast<vcl::filter::PDFNameElement*>(
            pPageRef->LookupObject()->Lookup("Type"_ostr));
        auto pStructureType = dynamic_cast<vcl::filter::PDFNameElement*>(
            pStructureRef->LookupObject()->Lookup("Type"_ostr));
        CPPUNIT_ASSERT(pPageType);
        CPPUNIT_ASSERT(pStructureType);
        CPPUNIT_ASSERT_EQUAL("Page"_ostr, pPageType->GetValue());
        CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pStructureType->GetValue());

        // which element it is, not merely that it is one: a jump that leaves the cursor behind
        // names the paragraph the link sits in, and that reads as a plausible structure
        // destination to a validator
        auto pStructureS = dynamic_cast<vcl::filter::PDFNameElement*>(
            pStructureRef->LookupObject()->Lookup("S"_ostr));
        CPPUNIT_ASSERT(pStructureS);
        // an outline item has a Title, a link annotation does not
        if (pObject->Lookup("Title"_ostr))
            aOutlineTypes.push_back(pStructureS->GetValue());
        else
            aLinkTypes.push_back(pStructureS->GetValue());
    }

    // the outline item names the heading it was made from
    CPPUNIT_ASSERT_EQUAL(size_t(1), aOutlineTypes.size());
    CPPUNIT_ASSERT_EQUAL("H1"_ostr, aOutlineTypes[0]);

    // the shape's bookmark sits at the document's first position on purpose: the cursor is
    // already there, and nothing moves
    std::sort(aLinkTypes.begin(), aLinkTypes.end());
    CPPUNIT_ASSERT_EQUAL(size_t(4), aLinkTypes.size());
    CPPUNIT_ASSERT_EQUAL("Bibliography 1"_ostr, aLinkTypes[0]);
    CPPUNIT_ASSERT_EQUAL("Footnote"_ostr, aLinkTypes[1]);
    CPPUNIT_ASSERT_EQUAL("H1"_ostr, aLinkTypes[2]);
    CPPUNIT_ASSERT_EQUAL("Standard"_ostr, aLinkTypes[3]);
}

CPPUNIT_TEST_FIXTURE(Test, testFootnoteNoteType)
{
    createSwDoc("footnote-tag.fodt");

    // the type of the one element a footnote frame opens
    const auto aFootnoteType = [](vcl::filter::PDFDocument& rDocument) -> OString {
        for (const auto& rDocElement : rDocument.GetElements())
        {
            auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
            if (!pObject)
                continue;
            auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pType && (pType->GetValue() == "Note" || pType->GetValue() == "FENote"))
                return pType->GetValue();
        }
        return OString();
    };

    save(TestFilter::PDF_WRITER,
         { comphelper::makePropertyValue(
             u"FilterData"_ustr,
             cpo::uno::Sequence{
                 comphelper::makePropertyValue(u"UseTaggedPDF"_ustr, true),
                 comphelper::makePropertyValue(u"SelectPdfVersion"_ustr, sal_Int32(20)) }) });

    vcl::filter::PDFDocument aPDF20;
    CPPUNIT_ASSERT(aPDF20.Read(*maTempFile.GetStream(StreamMode::READ)));

    // Without the fix this was Note, which ISO 14289-2 8.2.5.14 takes only where a role map
    // sends it to the PDF 2.0 namespace
    CPPUNIT_ASSERT_EQUAL("FENote"_ostr, aFootnoteType(aPDF20));

    save(TestFilter::PDF_WRITER,
         { comphelper::makePropertyValue(
             u"FilterData"_ustr,
             cpo::uno::Sequence{
                 comphelper::makePropertyValue(u"UseTaggedPDF"_ustr, true) }) });

    vcl::filter::PDFDocument aPDF17;
    maTempFile.CloseStream();
    CPPUNIT_ASSERT(aPDF17.Read(*maTempFile.GetStream(StreamMode::READ)));

    // the only footnote type ISO 32000-1 has, FENote being a PDF 2.0 addition
    CPPUNIT_ASSERT_EQUAL("Note"_ostr, aFootnoteType(aPDF17));
}

CPPUNIT_TEST_FIXTURE(Test, testTOCItemRef)
{
    createSwDoc("toc-structure-ref.fodt");

    cpo::uno::Sequence aFilterData{ comphelper::makePropertyValue(u"UseTaggedPDF"_ustr, true),
                                    comphelper::makePropertyValue(u"SelectPdfVersion"_ustr,
                                                                  sal_Int32(20)) };
    save(TestFilter::PDF_WRITER,
         { comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData) });

    vcl::filter::PDFDocument aDocument;
    CPPUNIT_ASSERT(aDocument.Read(*maTempFile.GetStream(StreamMode::READ)));

    // Without the fix a TOC item had no Ref at all, and ISO 14289-2 8.2.5.8 asks each of them to
    // name the element its entry reaches
    OStringBuffer aTargets;
    std::unordered_set<sal_Int32> aSeen;
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;

        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
        if (!pType || pType->GetValue() != "TOCI")
            continue;

        auto pRefs = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("Ref"_ostr));
        CPPUNIT_ASSERT(pRefs);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pRefs->GetElements().size());

        auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pRefs->GetElements()[0]);
        CPPUNIT_ASSERT(pRef);
        CPPUNIT_ASSERT(pRef->LookupObject());
        // the heading of this entry, not the one the entry before it reached
        CPPUNIT_ASSERT(aSeen.insert(pRef->GetObjectValue()).second);

        auto pTargetType = dynamic_cast<vcl::filter::PDFNameElement*>(
            pRef->LookupObject()->Lookup("S"_ostr));
        CPPUNIT_ASSERT(pTargetType);
        aTargets.append(pTargetType->GetValue() + " ");
    }

    // one item per heading, each naming its own
    CPPUNIT_ASSERT_EQUAL("H1 H2 "_ostr, aTargets.makeStringAndClear());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
