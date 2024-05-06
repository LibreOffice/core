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
#include <cppunit/extensions/HelperMacros.h>

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

    SbModuleRef Module()
    {
        interpreter = new StarBASIC();
        auto mod = interpreter->MakeModule(u"GlobalAsNew"_ustr, uR"BAS(
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

        )BAS"_ustr);
        CPPUNIT_ASSERT(mod->Compile());
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, StarBASIC::GetErrBasic());
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, SbxBase::GetError());
        CPPUNIT_ASSERT(mod->IsCompiled());
        return mod;
    }
};

void GlobalAsNewTest::testMaintainsValueAcrossCalls()
{
    auto m = Module();
    auto GetDateAsString = m->FindMethod(u"GetDateAsString"_ustr, SbxClassType::Method);
    CPPUNIT_ASSERT_MESSAGE("Could not Find GetDateAsString in module", GetDateAsString != nullptr);

    // There is no SbxMethod::call(), the basic code is exercised here in the copy ctor
    SbxVariableRef returned = new SbxMethod{ *GetDateAsString };
    CPPUNIT_ASSERT(returned->IsString());
    //0-00-0 is the result of reading the default-initialized date
    CPPUNIT_ASSERT_EQUAL(u"0-00-0"_ustr, returned->GetOUString());

    auto SetDate = m->FindMethod(u"SetDate"_ustr, SbxClassType::Method);
    CPPUNIT_ASSERT_MESSAGE("Could not Find SetDate in module", SetDate != nullptr);
    returned = new SbxMethod{ *SetDate };
    CPPUNIT_ASSERT(returned->IsString());
    OUString set_val(u"2019-06-30"_ustr);
    CPPUNIT_ASSERT_EQUAL(set_val, returned->GetOUString());

    returned = new SbxMethod{ *GetDateAsString };
    CPPUNIT_ASSERT(returned->IsString());
    //tdf#88442 The global should have maintained its state!
    CPPUNIT_ASSERT_EQUAL(set_val, returned->GetOUString());
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(GlobalAsNewTest);

} // namespace
