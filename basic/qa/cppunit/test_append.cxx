/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "sal/config.h"
#include "sal/precppunit.hxx"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include <test/bootstrapfixture.hxx>

#include "osl/file.hxx"
#include "osl/process.h"

#include "basic/sbstar.hxx"
#include "basic/sbmod.hxx"
#include "basic/sbmeth.hxx"
#include "basic/basrdll.hxx"
namespace
{
    class EnableTest : public test::BootstrapFixture
    {
        private:
        bool mbError;
        public:
        EnableTest() : mbError(false) {};
        void testDimEnable();
        void testEnableRuntime();
        // Adds code needed to register the test suite
        CPPUNIT_TEST_SUITE(EnableTest);

        // Declares the method as a test to call
        CPPUNIT_TEST(testDimEnable);
        CPPUNIT_TEST(testEnableRuntime);

        // End of test suite definition
        CPPUNIT_TEST_SUITE_END();

        DECL_LINK( BasicErrorHdl, StarBASIC * );
        bool HasError() { return mbError; }
        BasicDLL& basicDLL()
        {
            static BasicDLL maDll; // we need a dll instance for resouce manager etc.
            return maDll;
        }
    };

IMPL_LINK( EnableTest, BasicErrorHdl, StarBASIC *, /*pBasic*/)
{
    fprintf(stderr,"Got error: \n\t%s!!!\n", rtl::OUStringToOString( StarBASIC::GetErrorText(), RTL_TEXTENCODING_UTF8 ).getStr() );
    mbError = true;
    return 0;
}

void EnableTest::testEnableRuntime()
{
    CPPUNIT_ASSERT_MESSAGE( "No resource manager", basicDLL().GetBasResMgr() != NULL );
    StarBASICRef pBasic = new StarBASIC();
    StarBASIC::SetGlobalErrorHdl( LINK( this, EnableTest, BasicErrorHdl ) );

    rtl::OUString    sSource("Function Test as Integer\n");
    sSource += rtl::OUString("Dim Enable as Integer\n");
    sSource += rtl::OUString("Enable = 1\n");
    sSource += rtl::OUString("Enable = Enable + 2\n");
    sSource += rtl::OUString("Test = Enable\n");
    sSource += rtl::OUString("End Function\n");

    SbModule* pMod = pBasic->MakeModule( rtl::OUString( "TestModule" ), sSource );
    pMod->Compile();
    CPPUNIT_ASSERT_MESSAGE("testEnableRuntime fails with compile error",!mbError );
    SbMethod* pMeth = static_cast<SbMethod*>(pMod->Find( rtl::OUString("Test"),  SbxCLASS_METHOD ));
    CPPUNIT_ASSERT_MESSAGE("testEnableRuntime no method found", pMeth );
    SbxVariableRef refTemp = pMeth;
    // forces a broadcast
    SbxVariableRef pNew = new  SbxMethod( *((SbxMethod*)pMeth));
    CPPUNIT_ASSERT(pNew->GetInteger() == 3 );
    StarBASIC::SetGlobalErrorHdl( Link() );

}
void EnableTest::testDimEnable()
{
    CPPUNIT_ASSERT_MESSAGE( "No resource manager", basicDLL().GetBasResMgr() != NULL );
    StarBASICRef pBasic = new StarBASIC();
    StarBASIC::SetGlobalErrorHdl( LINK( this, EnableTest, BasicErrorHdl ) );

    rtl::OUString sSource("Sub Test\n");
    sSource += rtl::OUString("Dim Enable as String\n");
    sSource += rtl::OUString("End Sub\n");

    SbModule* pMod = pBasic->MakeModule( rtl::OUString( "TestModule" ), sSource );
    pMod->Compile();

    CPPUNIT_ASSERT_MESSAGE("Dim causes compile error", !mbError );
    StarBASIC::SetGlobalErrorHdl( Link() );
}

  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(EnableTest);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
