/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <unotools/configmgr.hxx>
#include "helper/qahelper.hxx"

#include <global.hxx>
#include <document.hxx>
#include <scdll.hxx>

#include <tools/stream.hxx>

class ScCacheTest : public CppUnit::TestFixture
{
public:
    void testCacheSimple();
    void testCacheString();
    void testCacheFormula();

    CPPUNIT_TEST_SUITE(ScCacheTest);
    CPPUNIT_TEST(testCacheSimple);
    CPPUNIT_TEST(testCacheString);
    CPPUNIT_TEST(testCacheFormula);
    CPPUNIT_TEST_SUITE_END();

public:
    ScCacheTest()
    {
        utl::ConfigManager::EnableFuzzing();
        ScDLL::Init();
        ScGlobal::Init();
    }
    ~ScCacheTest() { ScGlobal::Clear(); }
};

void ScCacheTest::testCacheSimple()
{
    ScDocumentRef pDoc(new ScDocument);
    pDoc->InsertTab(0, "test");
    for (SCROW nRow = 0; nRow < 10; ++nRow)
        pDoc->SetValue(0, nRow, 0, nRow);

    pDoc->SetValue(0, 100000, 0, -10);

    SvMemoryStream aStrm;
    pDoc->StoreTabToCache(0, aStrm);

    aStrm.Seek(0);

    ScDocumentRef pNewDoc(new ScDocument);
    pNewDoc->InsertTab(0, "test");
    pNewDoc->RestoreTabFromCache(0, aStrm);

    for (SCROW nRow = 0; nRow < 10; ++nRow)
        ASSERT_DOUBLES_EQUAL(nRow, pNewDoc->GetValue(0, nRow, 0));
}

void ScCacheTest::testCacheString()
{
    ScDocumentRef pDoc(new ScDocument);
    pDoc->InsertTab(0, "test");

    pDoc->SetString(0, 0, 0, "TestString");
    pDoc->SetString(0, 1, 0, "asjdaonfdssda");
    pDoc->SetString(0, 2, 0, "da");

    SvMemoryStream aStrm;
    pDoc->StoreTabToCache(0, aStrm);

    aStrm.Seek(0);

    ScDocumentRef pNewDoc(new ScDocument);
    pNewDoc->InsertTab(0, "test");
    pNewDoc->RestoreTabFromCache(0, aStrm);

    CPPUNIT_ASSERT_EQUAL(OUString("TestString"), pNewDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("asjdaonfdssda"), pNewDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("da"), pNewDoc->GetString(0, 2, 0));
}

void ScCacheTest::testCacheFormula()
{
    ScDocumentRef pDoc(new ScDocument);
    pDoc->InsertTab(0, "test");

    pDoc->SetString(0, 0, 0, "=B1");
    pDoc->SetString(0, 1, 0, "=B2");
    pDoc->SetString(0, 2, 0, "=B3");
    pDoc->SetString(0, 3, 0, "=B4");
    pDoc->SetString(0, 4, 0, "=B5");
    pDoc->SetString(0, 5, 0, "=B1");

    SvMemoryStream aStrm;
    pDoc->StoreTabToCache(0, aStrm);

    aStrm.Seek(0);

    ScDocumentRef pNewDoc(new ScDocument);
    pNewDoc->InsertTab(0, "test");
    pNewDoc->RestoreTabFromCache(0, aStrm);

    std::vector<OUString> aFormulas = { "=B1", "=B2", "=B3", "=B4", "=B5", "=B1" };
    for (SCROW nRow = 0; nRow <= 5; ++nRow)
    {
        OUString aFormula = pNewDoc->GetFormula(0, nRow, 0);
        CPPUNIT_ASSERT_EQUAL(aFormulas[nRow], aFormula);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCacheTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
