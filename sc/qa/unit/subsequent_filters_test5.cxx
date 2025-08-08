/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// core, please keep it alphabetically ordered
#include "helper/qahelper.hxx"

#include <dbdata.hxx>

using namespace css;
using namespace css::uno;

/* Implementation of Filters test, volume 5*/

class ScFiltersTest5 : public ScModelTestBase
{
public:
    ScFiltersTest5()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf157689)
{
    // testing the correct import of autofilter on multiple sheets
    createScDoc("xlsx/tdf157689.xlsx");

    ScDocument* pDoc = getScDoc();

    ScDBData* pAnonDBData = pDoc->GetAnonymousDBData(0);
    CPPUNIT_ASSERT(pAnonDBData);
    ScRange aFilterRange;
    pAnonDBData->GetArea(aFilterRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 3, 0), aFilterRange); // A1:B4
    CPPUNIT_ASSERT(pAnonDBData->HasAutoFilter());

    pAnonDBData = pDoc->GetAnonymousDBData(1);
    CPPUNIT_ASSERT(pAnonDBData);
    pAnonDBData->GetArea(aFilterRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 1, 1, 4, 1), aFilterRange); // A1:B5
    CPPUNIT_ASSERT(pAnonDBData->HasAutoFilter());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf151505)
{
    // testing the correct import of autofilter from XLSB
    createScDoc("xlsb/tdf151505.xlsb");

    ScDocument* pDoc = getScDoc();

    ScDBData* pAnonDBData = pDoc->GetAnonymousDBData(0);
    CPPUNIT_ASSERT(pAnonDBData);
    ScRange aFilterRange;
    pAnonDBData->GetArea(aFilterRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 4, 0), aFilterRange); // A1:B5
    CPPUNIT_ASSERT(pAnonDBData->HasAutoFilter());

    // also check for the correct handling of the autofilter buttons
    auto nFlag = pDoc->GetAttr(0, 0, 0, ATTR_MERGE_FLAG)->GetValue();
    CPPUNIT_ASSERT(nFlag & ScMF::Auto);
    nFlag = pDoc->GetAttr(1, 0, 0, ATTR_MERGE_FLAG)->GetValue();
    CPPUNIT_ASSERT(nFlag & ScMF::Auto);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf167134_LOOKUP_extRef)
{
    // testing the correct handling of external references in LOOKUP parameters
    // The file lookup_target.fods uses external links to file lookup_source.fods
    // The test requires that both are located in directory sc/qa/unit/data/fods;
    createScDoc("fods/lookup_target.fods");

    ScDocument* pDoc = getScDoc();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->ReloadAllLinks();
    pDocSh->DoHardRecalc();

    // compare the data loaded from external links (column 0)
    // with the expected result stored in the test file (column 1)
    for (SCROW nRow = 3; nRow <= 26; nRow++)
    {
        OUString aResult = pDoc->GetString(ScAddress(0, nRow, 0));
        OUString aExpected = pDoc->GetString(ScAddress(1, nRow, 0));
        CPPUNIT_ASSERT_EQUAL(aExpected, aResult);
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
