/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2010 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
    OUString aSep("/");
    OUStringBuffer aBuffer( getSrcRootURL() );
    aBuffer.append(m_aBaseString);
    aBuffer.append(aSep).append(aFileBase).append(aFileExtension);
    rFilePath = aBuffer.makeStringAndClear();
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
}

void DBAccessTest::tearDown()
{
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(DBAccessTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
