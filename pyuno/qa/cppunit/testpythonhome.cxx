/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/unoapi_test.hxx>

#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/task/XJob.hpp>

namespace
{
class PythonHomeTest : public UnoApiTest
{
public:
    PythonHomeTest()
        : UnoApiTest(u"/pyuno/qa/extras"_ustr)
    {
    }

private:
    void testHome();

    CPPUNIT_TEST_SUITE(PythonHomeTest);
    CPPUNIT_TEST(testHome);
    CPPUNIT_TEST_SUITE_END();
};

void PythonHomeTest::testHome()
{
    // Tests that the Python environment has the Python home correctly configured. See tdf#169387

    css::uno::Reference<css::loader::XImplementationLoader> xLoader(
        m_xFactory->createInstanceWithContext(u"com.sun.star.loader.Python"_ustr, m_xContext),
        css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::lang::XSingleComponentFactory> xFactory(
        xLoader->activate(u"org.libreoffice.pyuno.check_python_home"_ustr, u""_ustr,
                          createFileURL(u"check_python_home.py"),
                          css::uno::Reference<css::registry::XRegistryKey>()),
        css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::task::XJob> xJob(xFactory->createInstanceWithContext(m_xContext),
                                              css::uno::UNO_QUERY_THROW);

    // Execute the job contained in the test service. This should throw an exception if the test
    // fails.
    xJob->execute(css::uno::Sequence<css::beans::NamedValue>());
}

CPPUNIT_TEST_SUITE_REGISTRATION(PythonHomeTest);

} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
