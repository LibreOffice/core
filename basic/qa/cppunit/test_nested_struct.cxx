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
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <basic/sbuno.hxx>

namespace
{
    using namespace com::sun::star;
    class Nested_Struct : public test::BootstrapFixture
    {
        public:
        Nested_Struct(): BootstrapFixture(true, false) {};
        void testAssign1();
        void testAssign1Alt(); 
        void testOldAssign();
        void testOldAssignAlt(); 
        void testUnfixedVarAssign();
        void testUnfixedVarAssignAlt(); 
        void testFixedVarAssign();
        void testFixedVarAssignAlt(); 
        void testUnoAccess(); 

        
        CPPUNIT_TEST_SUITE(Nested_Struct);

        
        CPPUNIT_TEST(testAssign1);
        CPPUNIT_TEST(testAssign1Alt);
        CPPUNIT_TEST(testOldAssign);
        CPPUNIT_TEST(testOldAssignAlt);
        CPPUNIT_TEST(testUnfixedVarAssign);
        CPPUNIT_TEST(testUnfixedVarAssignAlt);
        CPPUNIT_TEST(testFixedVarAssign);
        CPPUNIT_TEST(testFixedVarAssignAlt);
        CPPUNIT_TEST(testUnoAccess);

        
        CPPUNIT_TEST_SUITE_END();
    };



OUString sTestSource1(
    "Function doUnitTest() as Integer\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\"\n"
    "b0.HorizontalLine.OuterLineWidth = 9\n"
    "doUnitTest = b0.HorizontalLine.OuterLineWidth\n"
    "End Function\n"
);

OUString sTestSource1Alt(
    "Function doUnitTest() as Object\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\"\n"
    "b0.HorizontalLine.OuterLineWidth = 9\n"
    "doUnitTest = b0\n"
    "End Function\n"
);







OUString sTestSource2(
    "Function doUnitTest()\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\", l as new \"com.sun.star.table.BorderLine\"\n"
    "l = b0.HorizontalLine\n"
    "l.OuterLineWidth = 9\n"
    "b0.HorizontalLine = l\n"
    "doUnitTest = b0.HorizontalLine.OuterLineWidth\n"
"End Function\n"
);

OUString sTestSource2Alt(
    "Function doUnitTest()\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\", l as new \"com.sun.star.table.BorderLine\"\n"
    "l = b0.HorizontalLine\n"
    "l.OuterLineWidth = 9\n"
    "b0.HorizontalLine = l\n"
    "doUnitTest = b0\n"
"End Function\n"
);





OUString sTestSource3(
    "Function doUnitTest()\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\"\n"
    "l = b0.HorizontalLine\n"
    "l.OuterLineWidth = 9\n"
    "b0.HorizontalLine = l\n"
    "l.OuterLineWidth = 4\n"
    "doUnitTest = b0.HorizontalLine.OuterLineWidth + l.OuterLineWidth\n"
"End Function\n"
);

OUString sTestSource3Alt(
    "Function doUnitTest()\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\"\n"
    "l = b0.HorizontalLine\n"
    "l.OuterLineWidth = 9\n"
    "b0.HorizontalLine = l\n"
    "l.OuterLineWidth = 4\n"
    "Dim result(1)\n"
    "result(0) = b0\n"
    "result(1) = l\n"
    "doUnitTest = result\n"
"End Function\n"
);



OUString sTestSource4(
    "Function doUnitTest()\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\", l as new \"com.sun.star.table.BorderLine\"\n"
    "l = b0.HorizontalLine\n"
    "l.OuterLineWidth = 9\n"
    "b0.HorizontalLine = l\n"
    "l.OuterLineWidth = 4\n"
    "doUnitTest = b0.HorizontalLine.OuterLineWidth + l.OuterLineWidth\n"
"End Function\n"
);

OUString sTestSource4Alt(
    "Function doUnitTest()\n"
    "Dim b0 as new \"com.sun.star.table.TableBorder\", l as new \"com.sun.star.table.BorderLine\"\n"
    "l = b0.HorizontalLine\n"
    "l.OuterLineWidth = 9\n"
    "b0.HorizontalLine = l\n"
    "l.OuterLineWidth = 4\n"
    "Dim result(1)\n"
    "result(0) = b0\n"
    "result(1) = l\n"
    "doUnitTest = result\n"
"End Function\n"
);






OUString sTestSource5(
    "Function doUnitTest() as Object\n"
    "Dim aWinDesc as new \"com.sun.star.awt.WindowDescriptor\"\n"
    "Dim aRect as new \"com.sun.star.awt.Rectangle\"\n"
    "aRect.X = 200\n"
    "aWinDesc.Bounds = aRect\n"
    "doUnitTest = aWinDesc\n"
"End Function\n"
);


void Nested_Struct::testAssign1()
{
    MacroSnippet myMacro( sTestSource1 );
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testAssign1 fails with compile error",!myMacro.HasError() );
    SbxVariableRef pNew = myMacro.Run();
    CPPUNIT_ASSERT(pNew->GetInteger() == 9 );
}

void Nested_Struct::testAssign1Alt()
{
    MacroSnippet myMacro( sTestSource1Alt );
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testAssign1Alt fails with compile error",!myMacro.HasError() );
    SbxVariableRef pNew = myMacro.Run();
    uno::Any aRet = sbxToUnoValue( pNew );
    table::TableBorder aBorder;
    aRet >>= aBorder;

    int result = aBorder.HorizontalLine.OuterLineWidth;
    CPPUNIT_ASSERT_EQUAL( 9, result );
}

void Nested_Struct::testOldAssign()
{
    MacroSnippet myMacro( sTestSource2 );
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testOldAssign fails with compile error",!myMacro.HasError() );
    SbxVariableRef pNew = myMacro.Run();
    CPPUNIT_ASSERT(pNew->GetInteger() == 9 );
}

void Nested_Struct::testOldAssignAlt()
{
    MacroSnippet myMacro( sTestSource2Alt );
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testOldAssign fails with compile error",!myMacro.HasError() );
    SbxVariableRef pNew = myMacro.Run();
    uno::Any aRet = sbxToUnoValue( pNew );
    table::TableBorder aBorder;
    aRet >>= aBorder;

    int result = aBorder.HorizontalLine.OuterLineWidth;
    CPPUNIT_ASSERT_EQUAL( 9, result );
}

void Nested_Struct::testUnfixedVarAssign()
{
    MacroSnippet myMacro( sTestSource3 );
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testUnfixedVarAssign fails with compile error",!myMacro.HasError() );
    
    SbxVariableRef pNew = myMacro.Run();
    CPPUNIT_ASSERT(pNew->GetInteger() == 13 );
}

void Nested_Struct::testUnfixedVarAssignAlt()
{
    MacroSnippet myMacro( sTestSource3Alt );
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testUnfixedVarAssignAlt fails with compile error",!myMacro.HasError() );
    SbxVariableRef pNew = myMacro.Run();
    uno::Any aRet = sbxToUnoValue( pNew );

    uno::Sequence< uno::Any > aResult;
    bool bRes = aRet >>= aResult;
    CPPUNIT_ASSERT_EQUAL(true, bRes );

    int result = aResult.getLength();
    
    CPPUNIT_ASSERT_EQUAL(2, result );

    table::TableBorder aBorder;
    aResult[0] >>= aBorder;

    table::BorderLine aBorderLine;
    aResult[1] >>= aBorderLine;
    result = aBorder.HorizontalLine.OuterLineWidth;
    CPPUNIT_ASSERT_EQUAL(9, result );
    result = aBorderLine.OuterLineWidth;
    CPPUNIT_ASSERT_EQUAL(4, result );
}

void Nested_Struct::testFixedVarAssign()
{
    MacroSnippet myMacro( sTestSource4 );
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testFixedVarAssign fails with compile error",!myMacro.HasError() );
    SbxVariableRef pNew = myMacro.Run();
    CPPUNIT_ASSERT(pNew->GetInteger() == 13 );
}

void Nested_Struct::testFixedVarAssignAlt()
{
    MacroSnippet myMacro( sTestSource4Alt );
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testFixedVarAssignAlt fails with compile error",!myMacro.HasError() );
    SbxVariableRef pNew = myMacro.Run();
    uno::Any aRet = sbxToUnoValue( pNew );

    uno::Sequence< uno::Any > aResult;
    bool bRes = aRet >>= aResult;
    CPPUNIT_ASSERT_EQUAL(true, bRes );

    int result = aResult.getLength();
    
    CPPUNIT_ASSERT_EQUAL(2, result );

    table::TableBorder aBorder;
    aResult[0] >>= aBorder;

    table::BorderLine aBorderLine;
    aResult[1] >>= aBorderLine;
    result = aBorder.HorizontalLine.OuterLineWidth;
    CPPUNIT_ASSERT_EQUAL(9, result );
    result = aBorderLine.OuterLineWidth;
    CPPUNIT_ASSERT_EQUAL(4, result );
}

void Nested_Struct::testUnoAccess()
{
    MacroSnippet myMacro( sTestSource5 );
    myMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("testUnoAccess fails with compile error",!myMacro.HasError() );
    SbxVariableRef pNew = myMacro.Run();
    uno::Any aRet = sbxToUnoValue( pNew );
    awt::WindowDescriptor aWinDesc;
    aRet >>= aWinDesc;

    int result = aWinDesc.Bounds.X;
    CPPUNIT_ASSERT_EQUAL(200, result );
}

  
  CPPUNIT_TEST_SUITE_REGISTRATION(Nested_Struct);
} 
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
