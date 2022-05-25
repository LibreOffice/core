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
    MacroSnippet aMacro("Sub doUnitTest(argName)\n"
                        "  If False Then\n"
                        "    Dim argName\n"
                        "  End If\n"
                        "End Sub\n");
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
    CPPUNIT_ASSERT_EQUAL(ERRCODE_BASIC_VAR_DEFINED, aMacro.getError().StripDynamic());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testDoubleArgument)
{
    MacroSnippet aMacro("Sub doUnitTest(argName, argName)\n"
                        "End Sub\n");
    aMacro.Compile();
    CPPUNIT_ASSERT(aMacro.HasError());
    CPPUNIT_ASSERT_EQUAL(ERRCODE_BASIC_VAR_DEFINED, aMacro.getError().StripDynamic());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf149157)
{
    MacroSnippet aMacro("Function extentComment() As Integer\n"
                        "  ' _\n"
                        "  If Not extentComment Then\n"
                        "     extentComment = 1\n"
                        "  End If\n"
                        "End Function\n");
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf149157_compatible)
{
    MacroSnippet aMacro("Option Compatible\n"
                        "Function extentComment() As Integer\n"
                        "  ' _\n"
                        "\n"
                        "  If Not extentComment Then\n"
                        "     extentComment = 1\n"
                        "  End If\n"
                        "End Function\n");
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTdf149157_vba)
{
    MacroSnippet aMacro("Option VBASupport 1\n"
                        "Function extentComment() As Integer\n"
                        "  ' _\n"
                        "\n"
                        "  If Not extentComment Then\n"
                        "     extentComment = 1\n"
                        "  End If\n"
                        "End Function\n");
    aMacro.Compile();
    CPPUNIT_ASSERT(!aMacro.HasError());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
