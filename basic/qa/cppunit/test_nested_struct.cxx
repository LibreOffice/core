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
        // Adds code needed to register the test suite
        CPPUNIT_TEST_SUITE(Nested_Struct);

        // Declares the method as a test to call
        CPPUNIT_TEST(testAssign1);

        // End of test suite definition
        CPPUNIT_TEST_SUITE_END();
    };

rtl::OUString sTestSource1(
    "Function simpleNestStructAccess() as Integer\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\"\n"
    "b0.HorizontalLine.OuterLineWidth = 9\n"
    "simpleNestStructAccess = b0.HorizontalLine.OuterLineWidth\n"
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

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(Nested_Struct);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
