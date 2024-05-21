/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <comphelper/configuration.hxx>
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
        comphelper::EnableFuzzing();
        ScDLL::Init();
        ScGlobal::Init();
    }
    ~ScCacheTest() { ScGlobal::Clear(); }
};

void ScCacheTest::testCacheSimple()
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    for (SCROW nRow = 0; nRow < 10; ++nRow)
        aDoc.SetValue(0, nRow, 0, nRow);

    aDoc.SetValue(0, 100000, 0, -10);

    SvMemoryStream aStrm;
    aDoc.StoreTabToCache(0, aStrm);

    aStrm.Seek(0);

    ScDocument aNewDoc;
    aNewDoc.InsertTab(0, u"test"_ustr);
    aNewDoc.RestoreTabFromCache(0, aStrm);

    for (SCROW nRow = 0; nRow < 10; ++nRow)
        ASSERT_DOUBLES_EQUAL(nRow, aNewDoc.GetValue(0, nRow, 0));
}

void ScCacheTest::testCacheString()
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);

    aDoc.SetString(0, 0, 0, u"TestString"_ustr);
    aDoc.SetString(0, 1, 0, u"asjdaonfdssda"_ustr);
    aDoc.SetString(0, 2, 0, u"da"_ustr);

    SvMemoryStream aStrm;
    aDoc.StoreTabToCache(0, aStrm);

    aStrm.Seek(0);

    ScDocument aNewDoc;
    aNewDoc.InsertTab(0, u"test"_ustr);
    aNewDoc.RestoreTabFromCache(0, aStrm);

    CPPUNIT_ASSERT_EQUAL(u"TestString"_ustr, aNewDoc.GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"asjdaonfdssda"_ustr, aNewDoc.GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"da"_ustr, aNewDoc.GetString(0, 2, 0));
}

void ScCacheTest::testCacheFormula()
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);

    aDoc.SetString(0, 0, 0, u"=B1"_ustr);
    aDoc.SetString(0, 1, 0, u"=B2"_ustr);
    aDoc.SetString(0, 2, 0, u"=B3"_ustr);
    aDoc.SetString(0, 3, 0, u"=B4"_ustr);
    aDoc.SetString(0, 4, 0, u"=B5"_ustr);
    aDoc.SetString(0, 5, 0, u"=B1"_ustr);

    SvMemoryStream aStrm;
    aDoc.StoreTabToCache(0, aStrm);

    aStrm.Seek(0);

    ScDocument aNewDoc;
    aNewDoc.InsertTab(0, u"test"_ustr);
    aNewDoc.RestoreTabFromCache(0, aStrm);

    std::vector<OUString> aFormulas
        = { u"=B1"_ustr, u"=B2"_ustr, u"=B3"_ustr, u"=B4"_ustr, u"=B5"_ustr, u"=B1"_ustr };
    for (SCROW nRow = 0; nRow <= 5; ++nRow)
    {
        OUString aFormula = aNewDoc.GetFormula(0, nRow, 0);
        CPPUNIT_ASSERT_EQUAL(aFormulas[nRow], aFormula);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCacheTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
