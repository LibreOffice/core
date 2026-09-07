/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/FunctionTestBase.hxx"

#include <address.hxx>
#include <document.hxx>

class LetFunctionTest : public FunctionTestBase
{
};

CPPUNIT_TEST_FIXTURE(LetFunctionTest, testLetXlsxRoundTrip)
{
    createScDoc("functions/let/xlsx/LetFixture.xlsx");

    auto checkVerdicts = [this](const char* pStage) {
        ScDocument* pDocument = getScDoc();
        assertRowChecks(*pDocument, 0, 200, pStage);
        CPPUNIT_ASSERT_EQUAL(1.0, pDocument->GetValue(ScAddress(0, 2, 0)));
    };

    checkVerdicts("on import");
    getScDoc()->CalcAll();
    checkVerdicts("after a recalculation");
    saveAndReload(TestFilter::XLSX);
    checkVerdicts("after a save and reload");
    getScDoc()->CalcAll();
    checkVerdicts("after a recalculation of the reloaded document");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
