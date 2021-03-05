/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/svapp.hxx>
#include <svx/svdpage.hxx>
#include <sddll.hxx>
#include <drawdoc.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

using namespace ::com::sun::star;

namespace {

class Test : public CppUnit::TestFixture {
public:
    Test();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testAddPage();
    void testCustomShow();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testAddPage);
    CPPUNIT_TEST(testCustomShow);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< uno::XComponentContext > m_xContext;
    std::unique_ptr<SdDrawDocument> m_pDoc;
};

Test::Test()
    : m_xContext(cppu::defaultBootstrap_InitialComponentContext())
{

    uno::Reference<lang::XMultiComponentFactory> xFactory(m_xContext->getServiceManager());
    uno::Reference<lang::XMultiServiceFactory> xSM(xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as it's passed around
    comphelper::setProcessServiceFactory(xSM);

    InitVCL();

    SdDLL::Init();
}

void Test::setUp()
{
    m_pDoc.reset(new SdDrawDocument(DocumentType::Impress, nullptr));
}

void Test::tearDown()
{
    m_pDoc.reset();
}

void Test::testAddPage()
{
    rtl::Reference<SdrPage> pPage = m_pDoc->AllocPage(false);
    m_pDoc->InsertPage(pPage.get());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("added one page to model",
                                 static_cast<sal_uInt16>(1), m_pDoc->GetPageCount());
    m_pDoc->DeletePage(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("removed one page to model",
                                 static_cast<sal_uInt16>(0), m_pDoc->GetPageCount());

    rtl::Reference<SdrPage> pMasterPage = m_pDoc->AllocPage(true);
    m_pDoc->InsertMasterPage(pMasterPage.get());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("added one master page to model",
                                 static_cast<sal_uInt16>(1), m_pDoc->GetMasterPageCount());
    m_pDoc->DeleteMasterPage(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("removed one master page to model",
                                 static_cast<sal_uInt16>(0), m_pDoc->GetMasterPageCount());
}

void Test::testCustomShow()
{
    CPPUNIT_ASSERT_MESSAGE("test generation of custom show list!",
                           m_pDoc->GetCustomShowList(true));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
