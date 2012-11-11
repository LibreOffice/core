/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):  Thorsten Behrens <tbehrens@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
    ~Test();

    virtual void setUp();
    virtual void tearDown();

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
    : m_pDoc(0)
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
    m_pDoc = new SdDrawDocument(DOCUMENT_TYPE_IMPRESS, NULL);
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
                           m_pDoc->GetCustomShowList(sal_True));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
