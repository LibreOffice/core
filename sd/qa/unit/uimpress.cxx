/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/svapp.hxx>
#include <sddll.hxx>
#include <drawdoc.hxx>

#include <iostream>
#include <vector>

using namespace ::com::sun::star;

namespace {

class Test : public CppUnit::TestFixture {
public:
    Test();
    virtual ~Test();

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
    SdDrawDocument* m_pDoc;
};

Test::Test()
    : m_pDoc(nullptr)
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();

    uno::Reference<lang::XMultiComponentFactory> xFactory(m_xContext->getServiceManager());
    uno::Reference<lang::XMultiServiceFactory> xSM(xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);

    InitVCL();

    SdDLL::Init();
}

void Test::setUp()
{
    m_pDoc = new SdDrawDocument(DOCUMENT_TYPE_IMPRESS, nullptr);
}

void Test::tearDown()
{
    delete m_pDoc;
}

Test::~Test()
{
    uno::Reference< lang::XComponent >(m_xContext, uno::UNO_QUERY_THROW)->dispose();
}

void Test::testAddPage()
{
    SdrPage* pPage = m_pDoc->AllocPage(false);
    m_pDoc->InsertPage(pPage);
    CPPUNIT_ASSERT_MESSAGE("added one page to model",
                           m_pDoc->GetPageCount()==1);
    m_pDoc->DeletePage(0);
    CPPUNIT_ASSERT_MESSAGE("removed one page to model",
                           m_pDoc->GetPageCount()==0);

    SdrPage* pMasterPage = m_pDoc->AllocPage(true);
    m_pDoc->InsertMasterPage(pMasterPage);
    CPPUNIT_ASSERT_MESSAGE("added one master page to model",
                           m_pDoc->GetMasterPageCount()==1);
    m_pDoc->DeleteMasterPage(0);
    CPPUNIT_ASSERT_MESSAGE("removed one master page to model",
                           m_pDoc->GetMasterPageCount()==0);
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
