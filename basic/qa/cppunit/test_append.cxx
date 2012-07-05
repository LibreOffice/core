/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "basictest.hxx"
#include "osl/file.hxx"
#include "osl/process.h"

#include "basic/sbmod.hxx"
#include "basic/sbmeth.hxx"
namespace
{
    class EnableTest : public BasicTestBase
    {
        public:
        EnableTest() {};
        void testDimEnable();
        void testEnableRuntime();
        // Adds code needed to register the test suite
        CPPUNIT_TEST_SUITE(EnableTest);

        // Declares the method as a test to call
        CPPUNIT_TEST(testDimEnable);
        CPPUNIT_TEST(testEnableRuntime);

        // End of test suite definition
        CPPUNIT_TEST_SUITE_END();
    };

rtl::OUString sTestEnableRuntime(
    "Function Test as Integer\n"
    "Dim Enable as Integer\n"
    "Enable = 1\n"
    "Enable = Enable + 2\n"
    "Test = Enable\n"
    "End Function\n"
);

rtl::OUString sTestDimEnable(
    "Sub Test\n"
    "Dim Enable as String\n"
    "End Sub\n"
);

void EnableTest::testEnableRuntime()
{
    CPPUNIT_ASSERT_MESSAGE( "No resource manager", basicDLL().GetBasResMgr() != NULL );
    StarBASICRef pBasic = new StarBASIC();
    ResetError();
    StarBASIC::SetGlobalErrorHdl( LINK( this, EnableTest, BasicErrorHdl ) );

    SbModule* pMod = pBasic->MakeModule( rtl::OUString( "TestModule" ), sTestEnableRuntime );
    pMod->Compile();
    CPPUNIT_ASSERT_MESSAGE("testEnableRuntime fails with compile error",!HasError() );
    SbMethod* pMeth = static_cast<SbMethod*>(pMod->Find( rtl::OUString("Test"),  SbxCLASS_METHOD ));
    CPPUNIT_ASSERT_MESSAGE("testEnableRuntime no method found", pMeth );
    SbxVariableRef refTemp = pMeth;
    // forces a broadcast
    SbxVariableRef pNew = new  SbxMethod( *((SbxMethod*)pMeth));
    CPPUNIT_ASSERT(pNew->GetInteger() == 3 );
}

void EnableTest::testDimEnable()
{
    CPPUNIT_ASSERT_MESSAGE( "No resource manager", basicDLL().GetBasResMgr() != NULL );
    StarBASICRef pBasic = new StarBASIC();
    StarBASIC::SetGlobalErrorHdl( LINK( this, EnableTest, BasicErrorHdl ) );

    ResetError();

    SbModule* pMod = pBasic->MakeModule( rtl::OUString( "TestModule" ), sTestDimEnable );
    pMod->Compile();

    CPPUNIT_ASSERT_MESSAGE("Dim causes compile error", !HasError() );
}

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(EnableTest);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
