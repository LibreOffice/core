/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"
#include <test/nanpayload_test.hxx>

class TestNanPayload : public ScUcalcTestBase
{
};

CPPUNIT_TEST_FIXTURE(TestNanPayload, testNanPayload)
{
#ifndef DISABLE_NAN_TESTS
    // Some tests, such as testExternalRefFunctions, testFuncGCD, and testFuncGCD, evaluates
    // spreadsheet functions' error code, which need NaN payload feature of the hardware
    NanPayloadTest nanTest;
    CPPUNIT_ASSERT_MESSAGE(nanTest.getMessage(), nanTest.getIfSupported());
#endif
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
