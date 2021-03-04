/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <basic/basrdll.hxx>
#include <cppunit/TestSuite.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace
{
class GlobalAsNewTest : public CppUnit::TestFixture
{
    void testMaintainsValueAcrossCalls();

    CPPUNIT_TEST_SUITE(GlobalAsNewTest);
    CPPUNIT_TEST(testMaintainsValueAcrossCalls);
    CPPUNIT_TEST_SUITE_END();

    BasicDLL lib;
    StarBASICRef interpreter;

    SbModuleRef module()
    {
        interpreter = new StarBASIC();
        auto mod = interpreter->MakeModule("GlobalAsNew", R"BAS(
Global aDate As New "com.sun.star.util.Date"

Function GetDateAsString As String
    DIM local_Date As New "com.sun.star.util.Date"
    GetDateAsString = TRIM(STR(aDate.Year)) + "-" + TRIM(STR(local_Date.Year)) + TRIM(STR(aDate.Month)) + "-" + TRIM(STR(aDate.Day))
End Function

Function SetDate
   aDate.Month = 6
   aDate.Day   = 30
   aDate.Year  = 2019
   SetDate = GetDateAsString()
End Function

        )BAS");
        CPPUNIT_ASSERT(mod->Compile());
        CPPUNIT_ASSERT_EQUAL(interpreter->GetErrBasic(), ERRCODE_NONE);
        CPPUNIT_ASSERT_EQUAL(mod->GetError(), ERRCODE_NONE);
        CPPUNIT_ASSERT(mod->IsCompiled());
        return mod;
    }
};

void GlobalAsNewTest::testMaintainsValueAcrossCalls()
{
    auto m = module();
    auto GetDateAsString = m->FindMethod("GetDateAsString", SbxClassType::Method);
    CPPUNIT_ASSERT_MESSAGE("Could not Find GetDateAsString in module", GetDateAsString != nullptr);

    OUString uninit_val("0-00-0");
    SbxVariableRef returned
        = new SbxMethod{ *GetDateAsString }; //This incantation may call a VB function or sub?
    CPPUNIT_ASSERT(returned->IsString());
    CPPUNIT_ASSERT_EQUAL(uninit_val, returned->GetOUString());

    auto SetDate = m->FindMethod("SetDate", SbxClassType::Method);
    CPPUNIT_ASSERT_MESSAGE("Could not Find SetDate in module", SetDate != nullptr);
    returned = new SbxMethod{ *SetDate };
    CPPUNIT_ASSERT(returned->IsString());
    OUString set_val("2019-06-30");
    CPPUNIT_ASSERT_EQUAL(set_val, returned->GetOUString());

    returned = new SbxMethod{ *GetDateAsString };
    CPPUNIT_ASSERT(returned->IsString());
    //tdf#88442 The global should have maintained its state!
    CPPUNIT_ASSERT_EQUAL(set_val, returned->GetOUString());
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(GlobalAsNewTest);

} // namespace
