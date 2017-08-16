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
#include <unotools/configmgr.hxx>
#include "helper/qahelper.hxx"

#include "global.hxx"
#include "document.hxx"

#include <tools/stream.hxx>

class ScCacheTest : public CppUnit::TestFixture
{
public:

    void testCacheSimple();
    void testCacheString();

    CPPUNIT_TEST_SUITE(ScCacheTest);
    CPPUNIT_TEST(testCacheSimple);
    CPPUNIT_TEST(testCacheString);
    CPPUNIT_TEST_SUITE_END();

public:
    virtual void setUp() override
    {
        utl::ConfigManager::EnableAvoidConfig();
        ScDLL::Init();
        ScGlobal::Init();
    }
};

void ScCacheTest::testCacheSimple()
{
    ScDocument aDoc;
    aDoc.InsertTab(0, "test");
    for (SCROW nRow = 0; nRow < 10; ++nRow)
        aDoc.SetValue(0, nRow, 0, nRow);

    aDoc.SetValue(0, 100000, 0, -10);

    SvMemoryStream aStrm;
    aDoc.StoreTabToCache(0, aStrm);

    aStrm.Seek(0);

    ScDocument aNewDoc;
    aNewDoc.InsertTab(0, "test");
    aNewDoc.RestoreTabFromCache(0, aStrm);

    for (SCROW nRow = 0; nRow < 10; ++nRow)
        ASSERT_DOUBLES_EQUAL(nRow, aNewDoc.GetValue(0, nRow, 0));
}

void ScCacheTest::testCacheString()
{
    ScDocument aDoc;
    aDoc.InsertTab(0, "test");

    aDoc.SetString(0, 0, 0, "TestString");
    aDoc.SetString(0, 1, 0, "asjdaonfdssda");
    aDoc.SetString(0, 2, 0, "da");

    SvMemoryStream aStrm;
    aDoc.StoreTabToCache(0, aStrm);

    aStrm.Seek(0);

    ScDocument aNewDoc;
    aNewDoc.InsertTab(0, "test");
    aNewDoc.RestoreTabFromCache(0, aStrm);

    CPPUNIT_ASSERT_EQUAL(OUString("TestString"), aNewDoc.GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("asjdaonfdssda"), aNewDoc.GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("da"), aNewDoc.GetString(0, 2, 0));
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCacheTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
