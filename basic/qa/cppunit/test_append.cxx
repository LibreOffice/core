/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "basictest.hxx"
#include <osl/file.hxx>
#include <osl/process.h>

#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
namespace
{
    class EnableTest : public test::BootstrapFixture
    {
        public:
        EnableTest() : BootstrapFixture(true, false) {};
        void testDimEnable();
        void testEnableRuntime();
        
        CPPUNIT_TEST_SUITE(EnableTest);

        
        CPPUNIT_TEST(testDimEnable);
        CPPUNIT_TEST(testEnableRuntime);

        
        CPPUNIT_TEST_SUITE_END();
    };

OUString sTestEnableRuntime(
    "Function doUnitTest as Integer\n"
    "Dim Enable as Integer\n"
    "Enable = 1\n"
    "Enable = Enable + 2\n"
    "doUnitTest = Enable\n"
    "End Function\n"
);

OUString sTestDimEnable(
    "Sub doUnitTest\n"
    "Dim Enable as String\n"
    "End Sub\n"
);

void EnableTest::testEnableRuntime()
{
    MacroSnippet myMacro(sTestEnableRuntime);
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testEnableRuntime fails with compile error",!myMacro.HasError() );
    SbxVariableRef pNew = myMacro.Run();
    CPPUNIT_ASSERT(pNew->GetInteger() == 3 );
}

void EnableTest::testDimEnable()
{
    MacroSnippet myMacro(sTestDimEnable);
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("Dim causes compile error", !myMacro.HasError() );
}

  
  CPPUNIT_TEST_SUITE_REGISTRATION(EnableTest);
} 
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
