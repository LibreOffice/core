/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotools/mediadescriptor.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <svx/svdpage.hxx>

#include "sdmodeltestbase.hxx"

using namespace css;

class SdPDFFilterTest : public SdModelTestBase
{
private:
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;

protected:
    void load(std::u16string_view pDir, const char* pName)
    {
        OUString const& rURL = m_directories.getURLFromSrc(pDir) + OUString::createFromAscii(pName);

        if (mxComponent.is())
            mxComponent->dispose();
        // Output name early, so in the case of a hang, the name of the hanging input file is visible.
        if (pName)
            std::cout << pName << ",";
        mxComponent = loadFromDesktop(rURL);
    }

    void save()
    {
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
        aMediaDescriptor["SelectPdfVersion"] <<= static_cast<sal_Int32>(1);
        xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    }

public:
    SdPDFFilterTest() { maTempFile.EnableKillingFile(); }

    virtual void setUp() override
    {
        SdModelTestBase::setUp();

        mxDesktop.set(
            css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    }

    virtual void tearDown() override
    {
        if (mxComponent.is())
            mxComponent->dispose();

        SdModelTestBase::tearDown();
    }

    void testTdf145873()
    {
        load(u"/sd/qa/unit/data/pptx/", "tdf145873.pptx");
        save();
        sd::DrawDocShellRef xDocShRef = loadURL(maTempFile.GetURL(), PDF);
        SdDrawDocument* pDoc = xDocShRef->GetDoc();
        CPPUNIT_ASSERT_MESSAGE("no document", pDoc != nullptr);

        const SdrPage* pPage = GetPage(1, xDocShRef);
        CPPUNIT_ASSERT_MESSAGE("no page", pPage != nullptr);

        // tdf#145873: Without the fix in place, this test would have failed with
        // - Expected: 3
        // - Actual  : 305
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(305), pPage->GetObjCount());

        // Shape 3 (Line) -> Navigator sidebar - Development Tools
        SdrObject* pObject = pPage->GetObj(3);
        CPPUNIT_ASSERT_MESSAGE("no object", pObject != nullptr);

        // tdf#145873: Without the fix #2 in place, this test would have failed with
        // - Expected: 6
        // - Actual  : 4
        CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(6), pObject->GetLogicRect().getHeight());

        xDocShRef->DoClose();
    }

    CPPUNIT_TEST_SUITE(SdPDFFilterTest);
    CPPUNIT_TEST(testTdf145873);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdPDFFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
