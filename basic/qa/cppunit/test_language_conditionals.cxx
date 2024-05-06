/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "basictest.hxx"

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace
{
class Language_Conditionals : public CppUnit::TestFixture
{
public:
    void testIfNot();
    void testIfAndNot();
    void testNENot();

    CPPUNIT_TEST_SUITE(Language_Conditionals);

    CPPUNIT_TEST(testIfNot);
    CPPUNIT_TEST(testIfAndNot);
    CPPUNIT_TEST(testNENot);

    CPPUNIT_TEST_SUITE_END();
};

void Language_Conditionals::testIfNot()
{
    { // need a block to ensure MacroSnippet is cleaned properly
        MacroSnippet myMacro(u"Option VBASupport 1\n"
                             "Option Explicit\n"
                             "\n"
                             "Function doUnitTest() As Integer\n"
                             "Dim op1 As Boolean\n"
                             "op1 = False\n"
                             "If Not op1 Then\n"
                             "doUnitTest = 1\n"
                             "Else\n"
                             "doUnitTest = 0\n"
                             "End If\n"
                             "End Function\n"_ustr);
        myMacro.Compile();
        CPPUNIT_ASSERT(!myMacro.HasError());
        SbxVariableRef pNew = myMacro.Run();
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), pNew->GetInteger());
    }
    { // need a block to ensure MacroSnippet is cleaned properly
        MacroSnippet myMacro(u"Option VBASupport 0\n"
                             "Option Explicit\n"
                             "\n"
                             "Function doUnitTest() As Integer\n"
                             "Dim op1 As Boolean\n"
                             "op1 = False\n"
                             "If Not op1 Then\n"
                             "doUnitTest = 1\n"
                             "Else\n"
                             "doUnitTest = 0\n"
                             "End If\n"
                             "End Function\n"_ustr);
        myMacro.Compile();
        CPPUNIT_ASSERT(!myMacro.HasError());
        SbxVariableRef pNew = myMacro.Run();
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), pNew->GetInteger());
    }
}

void Language_Conditionals::testIfAndNot()
{
    { // need a block to ensure MacroSnippet is cleaned properly
        MacroSnippet myMacro(u"Option VBASupport 1\n"
                             "Option Explicit\n"
                             "\n"
                             "Function doUnitTest() As Integer\n"
                             "Dim op1 As Boolean\n"
                             "Dim op2 As Boolean\n"
                             "op1 = True\n"
                             "op2 = False\n"
                             "If op1 And Not op2 Then\n"
                             "doUnitTest = 1\n"
                             "Else\n"
                             "doUnitTest = 0\n"
                             "End If\n"
                             "End Function\n"_ustr);
        myMacro.Compile();
        CPPUNIT_ASSERT(!myMacro.HasError());
        SbxVariableRef pNew = myMacro.Run();
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), pNew->GetInteger());
    }
    { // need a block to ensure MacroSnippet is cleaned properly
        MacroSnippet myMacro(u"Option VBASupport 0\n"
                             "Option Explicit\n"
                             "\n"
                             "Function doUnitTest() As Integer\n"
                             "Dim op1 As Boolean\n"
                             "Dim op2 As Boolean\n"
                             "op1 = True\n"
                             "op2 = False\n"
                             "If op1 And Not op2 Then\n"
                             "doUnitTest = 1\n"
                             "Else\n"
                             "doUnitTest = 0\n"
                             "End If\n"
                             "End Function\n"_ustr);
        myMacro.Compile();
        CPPUNIT_ASSERT(!myMacro.HasError());
        SbxVariableRef pNew = myMacro.Run();
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), pNew->GetInteger());
    }
}

void Language_Conditionals::testNENot()
{
    { // need a block to ensure MacroSnippet is cleaned properly
        MacroSnippet myMacro(u"Option VBASupport 1\n"
                             "Option Explicit\n"
                             "\n"
                             "Function doUnitTest() As Integer\n"
                             "Dim op1 As Boolean\n"
                             "Dim op2 As Boolean\n"
                             "op1 = False\n"
                             "op2 = False\n"
                             "If op1 <> Not op2 Then\n"
                             "doUnitTest = 1\n"
                             "Else\n"
                             "doUnitTest = 0\n"
                             "End If\n"
                             "End Function\n"_ustr);
        myMacro.Compile();
        CPPUNIT_ASSERT(!myMacro.HasError());
        SbxVariableRef pNew = myMacro.Run();
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), pNew->GetInteger());
    }
    { // need a block to ensure MacroSnippet is cleaned properly
        MacroSnippet myMacro(u"Option VBASupport 0\n"
                             "Option Explicit\n"
                             "\n"
                             "Function doUnitTest() As Integer\n"
                             "Dim op1 As Boolean\n"
                             "Dim op2 As Boolean\n"
                             "op1 = False\n"
                             "op2 = False\n"
                             "If op1 <> Not op2 Then\n"
                             "doUnitTest = 1\n"
                             "Else\n"
                             "doUnitTest = 0\n"
                             "End If\n"
                             "End Function\n"_ustr);
        myMacro.Compile();
        CPPUNIT_ASSERT(!myMacro.HasError());
        SbxVariableRef pNew = myMacro.Run();
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), pNew->GetInteger());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Language_Conditionals);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
