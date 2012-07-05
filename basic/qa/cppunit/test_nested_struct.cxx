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
    class Nested_Struct : public BasicTestBase
    {
        public:
        Nested_Struct() {};
        void testAssign1();
        void testOldAssign();
        // Adds code needed to register the test suite
        CPPUNIT_TEST_SUITE(Nested_Struct);

        // Declares the method as a test to call
        CPPUNIT_TEST(testAssign1);
        CPPUNIT_TEST(testOldAssign);

        // End of test suite definition
        CPPUNIT_TEST_SUITE_END();
    };

// tests the new behaviour, we should be able to
// directly modify the value of the nested 'HorizontalLine' struct
rtl::OUString sTestSource1(
    "Function simpleNestStructAccess() as Integer\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\"\n"
    "b0.HorizontalLine.OuterLineWidth = 9\n"
    "simpleNestStructAccess = b0.HorizontalLine.OuterLineWidth\n"
    "End Function\n"
);

// tests the old behaviour, we should still be able
// to use the old workaround of
// a) creating a new instance BorderLine,
// b) cloning the new instance with the value of b0.HorizontalLine
// c) modifying the new instance
// d) setting b0.HorizontalLine with the value of the new instance
rtl::OUString sTestSource2(
    "Function simpleRegressionTestOld()\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\", l as new \"com.sun.star.table.BorderLine\"\n"
    "l = b0.HorizontalLine\n"
    "l.OuterLineWidth = 9\n"
    "b0.HorizontalLine = l\n"
    "simpleRegressionTestOld = b0.HorizontalLine.OuterLineWidth\n"
"End Function\n"
);

void Nested_Struct::testAssign1()
{
    CPPUNIT_ASSERT_MESSAGE( "No resource manager", basicDLL().GetBasResMgr() != NULL );
    StarBASICRef pBasic = new StarBASIC();
    ResetError();
    StarBASIC::SetGlobalErrorHdl( LINK( this, Nested_Struct, BasicErrorHdl ) );

    SbModule* pMod = pBasic->MakeModule( rtl::OUString( "TestModule" ), sTestSource1 );
    pMod->Compile();
    CPPUNIT_ASSERT_MESSAGE("testAssign1 fails with compile error",!HasError() );
    SbMethod* pMeth = static_cast<SbMethod*>(pMod->Find( rtl::OUString("simpleNestStructAccess"),  SbxCLASS_METHOD ));
    CPPUNIT_ASSERT_MESSAGE("testAssign1 no method found", pMeth );
    SbxVariableRef refTemp = pMeth;
    // forces a broadcast
    SbxVariableRef pNew = new  SbxMethod( *((SbxMethod*)pMeth));
    CPPUNIT_ASSERT(pNew->GetInteger() == 9 );
}

void Nested_Struct::testOldAssign()
{
    CPPUNIT_ASSERT_MESSAGE( "No resource manager", basicDLL().GetBasResMgr() != NULL );
    StarBASICRef pBasic = new StarBASIC();
    ResetError();
    StarBASIC::SetGlobalErrorHdl( LINK( this, Nested_Struct, BasicErrorHdl ) );

    SbModule* pMod = pBasic->MakeModule( rtl::OUString( "TestModule" ), sTestSource2 );
    pMod->Compile();
    CPPUNIT_ASSERT_MESSAGE("testOldAssign fails with compile error",!HasError() );
    SbMethod* pMeth = static_cast<SbMethod*>(pMod->Find( rtl::OUString("simpleRegressionTestOld"),  SbxCLASS_METHOD ));
    CPPUNIT_ASSERT_MESSAGE("testOldAssign no method found", pMeth );
    SbxVariableRef refTemp = pMeth;
    // forces a broadcast
    SbxVariableRef pNew = new  SbxMethod( *((SbxMethod*)pMeth));
    CPPUNIT_ASSERT(pNew->GetInteger() == 9 );
}

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(Nested_Struct);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
