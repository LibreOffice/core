/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/unoapi_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>

#include <basic/sbxdef.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#if !defined(MACOSX) && !defined(_WIN32)

class DBAccessTest : public UnoApiTest
{
public:
    DBAccessTest();

    void test();

    CPPUNIT_TEST_SUITE(DBAccessTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

};


DBAccessTest::DBAccessTest()
      : UnoApiTest("/dbaccess/qa/extras/testdocuments")
{
}

void DBAccessTest::test()
{
    const OUString aFileNameBase("testdb.odb");
    OUString aFileName;
    createFileURL(aFileNameBase, aFileName);
    uno::Reference< lang::XComponent > xComponent = loadFromDesktop(aFileName);
    CPPUNIT_ASSERT(xComponent.is());
}

CPPUNIT_TEST_SUITE_REGISTRATION(DBAccessTest);

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
