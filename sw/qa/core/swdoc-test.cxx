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
 *       Thorsten Behrens <tbehrens@novell.com>
 *
 * Contributor(s):
 *   Thorsten Behrens <tbehrens@novell.com>
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include "precompiled_sw.hxx"

#include "preextstl.h"
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include "postextstl.h"

#include <osl/file.hxx>
#include <osl/process.h>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>

#include "init.hxx"
#include "swtypes.hxx"
#include "doc.hxx"

using namespace ::com::sun::star;

/* Implementation of Swdoc-Test class */

class SwDocTest : public CppUnit::TestFixture
{
public:
    SwDocTest();
    ~SwDocTest();

    virtual void setUp();
    virtual void tearDown();

    void randomTest()
    {
        CPPUNIT_ASSERT_MESSAGE("SwDoc::IsRedlineOn()", !m_pDoc->IsRedlineOn());
    }

    CPPUNIT_TEST_SUITE(SwDocTest);
    CPPUNIT_TEST(randomTest);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XMultiComponentFactory> m_xFactory;
    SwDoc *m_pDoc;
};

SwDocTest::SwDocTest()
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();
    m_xFactory = m_xContext->getServiceManager();

    uno::Reference<lang::XMultiServiceFactory> xSM(m_xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);

    InitVCL(xSM);

    SwDLL::Init();
}

void SwDocTest::setUp()
{
    m_pDoc = new SwDoc;
}

SwDocTest::~SwDocTest()
{
}

void SwDocTest::tearDown()
{
    delete m_pDoc;
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwDocTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
