/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <address.hxx>
#include <compiler.hxx>
#include <document.hxx>
#include <formulabuffer.hxx>
#include <tokenarray.hxx>
#include <tokenstringcontext.hxx>

#include <formula/grammar.hxx>
#include <formula/opcode.hxx>

class RedundantParenthesesStripperTest : public ScModelTestBase
{
public:
    RedundantParenthesesStripperTest()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

namespace
{
// Compile the formula with the OOXML grammar, strip the redundant
// inner parenthesis pair after every ocSingleValue, and read the
// parse array back as an OOXML formula string.
OUString stripAndPrint(ScDocument& rDocument, const OUString& rFormula)
{
    ScAddress aPosition(0, 0, 0);
    ScCompiler aComp(rDocument, aPosition, formula::FormulaGrammar::GRAM_OOXML, true, false);
    std::unique_ptr<ScTokenArray> pArray = aComp.CompileString(rFormula);
    CPPUNIT_ASSERT(pArray);
    oox::xls::stripRedundantParentheses(*pArray, { ocSingleValue });
    sc::TokenStringContext aContext(rDocument, formula::FormulaGrammar::GRAM_OOXML);
    return pArray->CreateString(aContext, aPosition);
}

// Compile with the OOXML grammar, lift every _xlfn.ANCHORARRAY
// wrapper past its parenthesised operand, then read the parse
// array back in native grammar.
OUString liftAnchorArrayAndPrintNative(ScDocument& rDocument, const OUString& rFormula)
{
    ScAddress aPosition(0, 0, 0);
    ScCompiler aComp(rDocument, aPosition, formula::FormulaGrammar::GRAM_OOXML, true, false);
    std::unique_ptr<ScTokenArray> pArray = aComp.CompileString(rFormula);
    CPPUNIT_ASSERT(pArray);
    oox::xls::liftAnchorArrayToPostfix(*pArray);
    sc::TokenStringContext aContext(rDocument, formula::FormulaGrammar::GRAM_NATIVE);
    return pArray->CreateString(aContext, aPosition);
}
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testStripsInnerPairAroundExpression)
{
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE(A1+1)"_ustr,
                         stripAndPrint(*getScDoc(), u"_xlfn.SINGLE((A1+1))"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testStripsInnerPairAroundBareReference)
{
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE(A1)"_ustr,
                         stripAndPrint(*getScDoc(), u"_xlfn.SINGLE((A1))"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testLeavesSinglePairAlone)
{
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE(A1+1)"_ustr,
                         stripAndPrint(*getScDoc(), u"_xlfn.SINGLE(A1+1)"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testLeavesNonSpanningInnerPair)
{
    // The (A1) covers only the left side of the addition, so the
    // outer pair is the @ argument and is not redundant.
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE((A1)+B1)"_ustr,
                         stripAndPrint(*getScDoc(), u"_xlfn.SINGLE((A1)+B1)"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testLeavesFunctionCallArgument)
{
    // The parentheses around A1:A4 belong to the TRANSPOSE call, not
    // to a redundant wrapper.
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE(TRANSPOSE(A1:A4))"_ustr,
                         stripAndPrint(*getScDoc(), u"_xlfn.SINGLE(TRANSPOSE(A1:A4))"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testPeelsOneLevelOfNestedRedundantPairs)
{
    // One pass peels one level. The remaining redundant pair would
    // need another invocation to drop.
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE((A1+1))"_ustr,
                         stripAndPrint(*getScDoc(), u"_xlfn.SINGLE(((A1+1)))"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest,
                     testStripsAroundFunctionCallButLeavesInnerArgument)
{
    // The trigger list only covers ocSingleValue, so redundant pairs
    // inside the function call stay.
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE(TRANSPOSE((A1:A4)))"_ustr,
                         stripAndPrint(*getScDoc(), u"_xlfn.SINGLE((TRANSPOSE((A1:A4))))"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testLeavesNonTriggerOpcodeAlone)
{
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"SUM((A1+B1))"_ustr, stripAndPrint(*getScDoc(), u"SUM((A1+B1))"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testLiftsAnchorArrayAroundBareReference)
{
    // _xlfn.ANCHORARRAY(A1) reads back as A1# in native grammar.
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"A1#"_ustr,
                         liftAnchorArrayAndPrintNative(*getScDoc(), u"_xlfn.ANCHORARRAY(A1)"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testLiftsAnchorArrayInsideAddition)
{
    // The wrapper composes with arithmetic: _xlfn.ANCHORARRAY(A1)+10
    // reads back as A1#+10 in native grammar.
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"A1#+10"_ustr, liftAnchorArrayAndPrintNative(
                                             *getScDoc(), u"_xlfn.ANCHORARRAY(A1)+10"_ustr));
}

CPPUNIT_TEST_FIXTURE(RedundantParenthesesStripperTest, testLiftsAnchorArrayAroundParenthesisedReference)
{
    // A parenthesised operand is taken whole: _xlfn.ANCHORARRAY(((A1)))
    // reads back as ((A1))# in native grammar, with the inner
    // parentheses left in place.
    createScDoc();
    CPPUNIT_ASSERT_EQUAL(u"((A1))#"_ustr, liftAnchorArrayAndPrintNative(
                                              *getScDoc(), u"_xlfn.ANCHORARRAY(((A1)))"_ustr));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
