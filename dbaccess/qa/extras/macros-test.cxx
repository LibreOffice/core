/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <com/sun/star/frame/Desktop.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>

#include <basic/sbxdef.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


class DBAccessTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    DBAccessTest();

    void createFileURL(const OUString& aFileBase, const OUString& aFileExtension, OUString& rFilePath);

    virtual void setUp();
    virtual void tearDown();

    void test();

    CPPUNIT_TEST_SUITE(DBAccessTest);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(test);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    OUString m_aBaseString;
};


void DBAccessTest::createFileURL(const OUString& aFileBase, const OUString& aFileExtension, OUString& rFilePath)
{
    rFilePath = getSrcRootURL() + m_aBaseString + "/" + aFileBase + aFileExtension;
}

DBAccessTest::DBAccessTest()
      : m_aBaseString(RTL_CONSTASCII_USTRINGPARAM("/dbaccess/qa/extras/testdocuments"))
{
}

void DBAccessTest::test()
{
    const OUString aFileNameBase("testdb.");
    const OUString aFileExtension("odb");
    OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    uno::Reference< lang::XComponent > xComponent = loadFromDesktop(aFileName);
    CPPUNIT_ASSERT(xComponent.is());
}

void DBAccessTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    mxDesktop = com::sun::star::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) );
    CPPUNIT_ASSERT(mxDesktop.is());
}

void DBAccessTest::tearDown()
{
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(DBAccessTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
