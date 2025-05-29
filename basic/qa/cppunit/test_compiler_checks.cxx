/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include "basictest.hxx"
#include <basic/sberrors.hxx>
#include <unotest/bootstrapfixturebase.hxx>

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testRedefineArgument)
{
    MacroSnippet aMacro(u"Sub doUnitTest(argName)\n"
                        "  If False Then\n"
                        "    Dim argName\n"
                        "  End If\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
    CPPUNIT_ASSERT_EQUAL(ERRCODE_BASIC_VAR_DEFINED, aMacro.getError().GetCode());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testDoubleArgument)
{
    MacroSnippet aMacro(u"Sub doUnitTest(argName, argName)\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
    CPPUNIT_ASSERT_EQUAL(ERRCODE_BASIC_VAR_DEFINED, aMacro.getError().GetCode());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf149157)
{
    MacroSnippet aMacro(u"Function extentComment() As Integer\n"
                        "  ' _\n"
                        "  If Not extentComment Then\n"
                        "     extentComment = 1\n"
                        "  End If\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf149157_compatible)
{
    MacroSnippet aMacro(u"Option Compatible\n"
                        "Function extentComment() As Integer\n"
                        "  ' _\n"
                        "\n"
                        "  If Not extentComment Then\n"
                        "     extentComment = 1\n"
                        "  End If\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf149157_vba)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Function extentComment() As Integer\n"
                        "  ' _\n"
                        "\n"
                        "  If Not extentComment Then\n"
                        "     extentComment = 1\n"
                        "  End If\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf149402)
{
    MacroSnippet aMacro(u"Function extentComment() As Integer\n"
                        "  ' _ \n"
                        "  If Not extentComment Then\n"
                        "     extentComment = 1\n"
                        "  Else\n"
                        "  End If\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf149402_compatible)
{
    MacroSnippet aMacro(u"Option Compatible\n"
                        "Function extentComment() As Integer\n"
                        "  ' _ \n"
                        "  If Not extentComment Then\n"
                        "     extentComment = 1\n"
                        "  Else\n"
                        "  End If\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf149402_vba)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Function extentComment() As Integer\n"
                        "  ' _ \n"
                        "  If Not extentComment Then\n"
                        "     extentComment = 1\n"
                        "  Else\n"
                        "  End If\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf93727_if)
{
    // #If and #End directive
    MacroSnippet aMacro(u"Sub doUnitTest\n"
                        "  #If 1 = 1 Then\n"
                        "      Const a = 10\n"
                        "  #End If\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("#If directive causes compile error", !aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf93727_else)
{
    // #Else directive
    MacroSnippet aMacro(u"Sub doUnitTest\n"
                        "a = 0\n"
                        "#If 1 = 0 Then\n"
                        "    a = 10\n"
                        "#Else\n"
                        "    a = 20\n"
                        "#End If\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("#Else directive causes compile error", !aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf93727_elseif)
{
    // #ElseIf directive
    MacroSnippet aMacro(u"Sub doUnitTest\n"
                        "a = 0\n"
                        "  #If 1 = 0 Then\n"
                        "      a = 10\n"
                        "  #ElseIf 2 = 2 Then\n"
                        "      a = 20\n"
                        "  #End If\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("#ElseIf directive causes compile error", !aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf93727_const)
{
    // #Const directive
    MacroSnippet aMacro(u"#Const MaxValue = 1000\n"
                        "Sub doUnitTest\n"
                        "   Dim value As Integer\n"
                        "   value = MaxValue\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT_MESSAGE("#Const directive causes compile error", !aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162983_property_get_end_function)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Property Get doUnitTest\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162983_property_get_end_sub)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Property Get doUnitTest\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162983_sub_end_function)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Sub doUnitTest\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162983_sub_end_property)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Sub doUnitTest\n"
                        "End Property\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162983_function_end_sub)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Function doUnitTest\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162983_function_end_property)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Function doUnitTest\n"
                        "End Property\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162986_property_set_end_sub)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Property Set doUnitTest\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162986_property_set_end_function)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Property Set doUnitTest\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162986_property_let_end_sub)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Property Let doUnitTest\n"
                        "End Sub\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf162986_property_let_end_function)
{
    MacroSnippet aMacro(u"Option VBASupport 1\n"
                        "Property Let doUnitTest\n"
                        "End Function\n"_ustr);
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testMissingEndIf)
{
    {
        MacroSnippet aMacro(u"Sub doUnitTest(argName)\n"
                            "  If False Then\n"
                            "End Sub\n"_ustr);
        aMacro.Compile();
        CPPUNIT_ASSERT(aMacro.HasError());
        CPPUNIT_ASSERT_EQUAL(ERRCODE_BASIC_BAD_BLOCK, aMacro.getError().GetCode());
        // Without the fix, this was "End Sub"
        CPPUNIT_ASSERT_EQUAL(u"End If"_ustr, aMacro.getError().GetArg1());
    }
    {
        MacroSnippet aMacro(u"Sub doUnitTest(argName)\n"
                            "  If False Then\n"
                            "  End Sub\n"
                            "End Sub\n"_ustr);
        aMacro.Compile();
        // Without the fix, this compiled without errors
        CPPUNIT_ASSERT(aMacro.HasError());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
