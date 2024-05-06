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
class GlobalArrayTest : public CppUnit::TestFixture
{
    void testMaintainsValueAcrossCalls();

    CPPUNIT_TEST_SUITE(GlobalArrayTest);
    CPPUNIT_TEST(testMaintainsValueAcrossCalls);
    CPPUNIT_TEST_SUITE_END();

    BasicDLL lib;
    StarBASICRef interpreter;

    SbModuleRef Module()
    {
        interpreter = new StarBASIC();
        auto mod = interpreter->MakeModule(u"GlobalArray"_ustr, uR"BAS(

Type testType
    iNr    As Integer
    sType  As String
End Type

Global aTestTypes(2) As New testType

Function Macro1 As String
    aTestTypes(0).iNr = 1
    aTestTypes(0).sType = "A"
    Macro1 = aTestTypes(0).iNr & aTestTypes(0).sType
End Function

Function Macro2 As String
    aTestTypes(1).iNr = 2
    aTestTypes(1).sType = "B"
    Macro2 = aTestTypes(0).iNr & aTestTypes(0).sType & aTestTypes(1).iNr & aTestTypes(1).sType
End Function

        )BAS"_ustr);
        CPPUNIT_ASSERT(mod->Compile());
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, StarBASIC::GetErrBasic());
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, SbxBase::GetError());
        CPPUNIT_ASSERT(mod->IsCompiled());
        return mod;
    }
};

void GlobalArrayTest::testMaintainsValueAcrossCalls()
{
    auto m = Module();
    auto Macro1 = m->FindMethod(u"Macro1"_ustr, SbxClassType::Method);
    CPPUNIT_ASSERT_MESSAGE("Could not Find Macro1 in module", Macro1 != nullptr);

    // There is no SbxMethod::call(), the basic code is exercised here in the copy ctor
    SbxVariableRef returned = new SbxMethod{ *Macro1 };
    CPPUNIT_ASSERT(returned->IsString());
    CPPUNIT_ASSERT_EQUAL(u"1A"_ustr, returned->GetOUString());

    auto Macro2 = m->FindMethod(u"Macro2"_ustr, SbxClassType::Method);
    CPPUNIT_ASSERT_MESSAGE("Could not Find Macro2 in module", Macro2 != nullptr);
    returned = new SbxMethod{ *Macro2 };
    CPPUNIT_ASSERT(returned->IsString());
    // tdf#145371 - check if the global array has maintained its state
    // Without the fix in place, this test would have failed with:
    // - Expected: 1A2B
    // - Actual  : 02B
    CPPUNIT_ASSERT_EQUAL(u"1A2B"_ustr, returned->GetOUString());
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(GlobalArrayTest);

} // namespace
