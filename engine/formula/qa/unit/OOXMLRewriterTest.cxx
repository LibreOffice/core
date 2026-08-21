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

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <OOXMLRewriter.hxx>

#include <formula/opcode.hxx>
#include <formula/token.hxx>
#include <formula/tokenarray.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

#include <memory>

using namespace formula;

namespace
{
class OOXMLRewriterTest : public CppUnit::TestFixture
{
protected:
    /** Add an operand. The rewrite reads opcodes and stack types, never whether a token is a
        reference, so any push token does. A number is the shortest to build. */
    static void addOperand(FormulaTokenArray& rTokens) { rTokens.AddDouble(1.0); }

    static std::unique_ptr<FormulaTokenArray> rewrite(const FormulaTokenArray& rTokens)
    {
        return OOXMLRewriter(rTokens).releaseTokens();
    }

    /** The token sequence as a readable line, so a failure says what came out. The names are
        the tokens' own, not the text: a union stays ~ here, though OOXML writes it as a
        comma. */
    static OUString describe(const FormulaTokenArray& rTokens)
    {
        OUStringBuffer aBuffer;
        for (sal_uInt16 nPosition = 0; nPosition < rTokens.GetLen(); ++nPosition)
        {
            if (!aBuffer.isEmpty())
                aBuffer.append(' ');
            aBuffer.append(name(rTokens.TokenAt(nPosition)->GetOpCode()));
        }
        return aBuffer.makeStringAndClear();
    }

private:
    static OUString name(OpCode eOp)
    {
        switch (eOp)
        {
            case ocPush:
                return u"operand"_ustr;
            case ocOpen:
                return u"("_ustr;
            case ocClose:
                return u")"_ustr;
            case ocUnion:
                return u"~"_ustr;
            case ocAdd:
                return u"+"_ustr;
            case ocSingleValue:
                return u"_xlfn.SINGLE"_ustr;
            case ocSpill:
                return u"#"_ustr;
            case ocAnchorArray:
                return u"_xlfn.ANCHORARRAY"_ustr;
            case ocOffset:
                return u"OFFSET"_ustr;
            case ocPercentSign:
                return u"%"_ustr;
            case ocMissing:
                return u"missing"_ustr;
            case ocBad:
                return u"bad"_ustr;
            case ocErrRef:
                return u"#REF!"_ustr;
            default:
                return u"?"_ustr;
        }
    }
};

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testSpillBecomesACallInFrontOfItsOperand)
{
    // A1# -> _xlfn.ANCHORARRAY(A1). The operator moves and gets an opcode of its own, so
    // nothing later has to tell the two spellings apart by position.
    FormulaTokenArray aTokens;
    addOperand(aTokens);
    aTokens.AddOpCode(ocSpill);

    const std::unique_ptr<FormulaTokenArray> pRewritten = rewrite(aTokens);

    CPPUNIT_ASSERT(pRewritten);
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.ANCHORARRAY ( operand )"_ustr, describe(*pRewritten));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testSingleValueKeepsItsPlaceAndGainsParentheses)
{
    // =@A1 -> _xlfn.SINGLE(A1). The @ is already in front, so only the parentheses are new.
    FormulaTokenArray aTokens;
    aTokens.AddOpCode(ocSingleValue);
    addOperand(aTokens);

    const std::unique_ptr<FormulaTokenArray> pRewritten = rewrite(aTokens);

    CPPUNIT_ASSERT(pRewritten);
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE ( operand )"_ustr, describe(*pRewritten));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testAnOperandInParenthesesKeepsTheOnesItHas)
{
    // =@(A1) comes out like =@A1 - the parentheses already there serve, no second pair.
    FormulaTokenArray aTokens;
    aTokens.AddOpCode(ocSingleValue);
    aTokens.AddOpCode(ocOpen);
    addOperand(aTokens);
    aTokens.AddOpCode(ocClose);

    const std::unique_ptr<FormulaTokenArray> pRewritten = rewrite(aTokens);

    CPPUNIT_ASSERT(pRewritten);
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE ( operand )"_ustr, describe(*pRewritten));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testUnionGainsParenthesesButKeepsItsOpcode)
{
    // A1~B1 -> (A1,B1). ocUnion already spells as a comma, so only the parentheses are new.
    FormulaTokenArray aTokens;
    addOperand(aTokens);
    aTokens.AddOpCode(ocUnion);
    addOperand(aTokens);

    const std::unique_ptr<FormulaTokenArray> pRewritten = rewrite(aTokens);

    CPPUNIT_ASSERT(pRewritten);
    CPPUNIT_ASSERT_EQUAL(u"( operand ~ operand )"_ustr, describe(*pRewritten));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testSpillInsideSingleValueNestsTheTwoCalls)
{
    // =@A1# -> _xlfn.SINGLE(_xlfn.ANCHORARRAY(A1)), the @ taking in the whole of the #.
    FormulaTokenArray aTokens;
    aTokens.AddOpCode(ocSingleValue);
    addOperand(aTokens);
    aTokens.AddOpCode(ocSpill);

    const std::unique_ptr<FormulaTokenArray> pRewritten = rewrite(aTokens);

    CPPUNIT_ASSERT(pRewritten);
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE ( _xlfn.ANCHORARRAY ( operand ) )"_ustr,
                         describe(*pRewritten));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testSingleValueClosesBeforeAnArithmeticOperator)
{
    // =@A1+B1 -> _xlfn.SINGLE(A1)+B1. The plus is above the @, so only A1 is under it.
    FormulaTokenArray aTokens;
    aTokens.AddOpCode(ocSingleValue);
    addOperand(aTokens);
    aTokens.AddOpCode(ocAdd);
    addOperand(aTokens);

    const std::unique_ptr<FormulaTokenArray> pRewritten = rewrite(aTokens);

    CPPUNIT_ASSERT(pRewritten);
    CPPUNIT_ASSERT_EQUAL(u"_xlfn.SINGLE ( operand ) + operand"_ustr, describe(*pRewritten));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testLiteralFirstArgumentOfOffsetBecomesAnError)
{
    // OFFSET needs a reference in its first argument, so a literal there is swapped for
    // #REF!, which is all any reader accepts.
    FormulaTokenArray aTokens;
    aTokens.AddOpCode(ocOffset);
    aTokens.AddOpCode(ocOpen);
    aTokens.AddDouble(1.0);
    aTokens.AddOpCode(ocClose);

    const std::unique_ptr<FormulaTokenArray> pRewritten = rewrite(aTokens);

    CPPUNIT_ASSERT(pRewritten);
    CPPUNIT_ASSERT_EQUAL(u"OFFSET ( #REF! )"_ustr, describe(*pRewritten));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testAUnionAlreadyInParenthesesNeedsNoRewrite)
{
    // =(A1~B1) already has the parentheses OOXML wants, so there is nothing to change.
    FormulaTokenArray aTokens;
    aTokens.AddOpCode(ocOpen);
    addOperand(aTokens);
    aTokens.AddOpCode(ocUnion);
    addOperand(aTokens);
    aTokens.AddOpCode(ocClose);

    CPPUNIT_ASSERT(!rewrite(aTokens));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testAnAtWithNothingToEncloseIsDropped)
{
    // The call name comes from the wrapper, so an @ that got none has to go rather than write
    // _xlfn.SINGLE on its own.
    FormulaTokenArray aAlone;
    aAlone.AddOpCode(ocSingleValue);

    const std::unique_ptr<FormulaTokenArray> pAlone = rewrite(aAlone);

    CPPUNIT_ASSERT(pAlone);
    CPPUNIT_ASSERT_EQUAL(OUString(), describe(*pAlone));

    // The operand of the first @ is the second one, which writes nothing either.
    FormulaTokenArray aNested;
    aNested.AddOpCode(ocSingleValue);
    aNested.AddOpCode(ocSingleValue);

    const std::unique_ptr<FormulaTokenArray> pNested = rewrite(aNested);

    CPPUNIT_ASSERT(pNested);
    CPPUNIT_ASSERT_EQUAL(OUString(), describe(*pNested));

    // A missing argument is a token, but it writes no text, so it fills no operand.
    FormulaTokenArray aMissing;
    aMissing.AddOpCode(ocSingleValue);
    aMissing.AddOpCode(ocMissing);

    const std::unique_ptr<FormulaTokenArray> pMissing = rewrite(aMissing);

    CPPUNIT_ASSERT(pMissing);
    CPPUNIT_ASSERT_EQUAL(u"missing"_ustr, describe(*pMissing));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testASpillWithNoFactorBeforeItIsDropped)
{
    // A percent sign ends the factor, so the # that follows has nothing to enclose. The pair
    // would span no tokens at all, so it goes and the # with it.
    FormulaTokenArray aTokens;
    addOperand(aTokens);
    aTokens.AddOpCode(ocPercentSign);
    aTokens.AddOpCode(ocSpill);

    const std::unique_ptr<FormulaTokenArray> pRewritten = rewrite(aTokens);

    CPPUNIT_ASSERT(pRewritten);
    CPPUNIT_ASSERT_EQUAL(u"operand %"_ustr, describe(*pRewritten));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testTextThatDidNotParseTakesNoWrapper)
{
    // Parentheses cannot go around text that never parsed, so a wrapper waiting on it is
    // dropped and the operator it would have named goes with it. A prefix sign stays.
    FormulaTokenArray aSigned;
    aSigned.AddOpCode(ocSingleValue);
    aSigned.AddOpCode(ocAdd);
    aSigned.AddBad(u"PW value"_ustr);

    const std::unique_ptr<FormulaTokenArray> pSigned = rewrite(aSigned);

    CPPUNIT_ASSERT(pSigned);
    CPPUNIT_ASSERT_EQUAL(u"+ bad"_ustr, describe(*pSigned));

    // Both @ wrappers go at once.
    FormulaTokenArray aNested;
    aNested.AddOpCode(ocSingleValue);
    aNested.AddOpCode(ocSingleValue);
    aNested.AddBad(u"PW value"_ustr);

    const std::unique_ptr<FormulaTokenArray> pNested = rewrite(aNested);

    CPPUNIT_ASSERT(pNested);
    CPPUNIT_ASSERT_EQUAL(u"bad"_ustr, describe(*pNested));

    // A # behind such text has nothing to bind to either.
    FormulaTokenArray aSpilled;
    aSpilled.AddOpCode(ocSingleValue);
    aSpilled.AddBad(u"PW value"_ustr);
    aSpilled.AddOpCode(ocSpill);

    const std::unique_ptr<FormulaTokenArray> pSpilled = rewrite(aSpilled);

    CPPUNIT_ASSERT(pSpilled);
    CPPUNIT_ASSERT_EQUAL(u"bad"_ustr, describe(*pSpilled));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testAUnionReachingSuchTextNeedsNoRewrite)
{
    // The list loses its parentheses for the same reason. With nothing else to change and no
    // token to drop, there is no rewrite at all.
    FormulaTokenArray aTokens;
    addOperand(aTokens);
    aTokens.AddOpCode(ocUnion);
    aTokens.AddBad(u"PW value"_ustr);

    CPPUNIT_ASSERT(!rewrite(aTokens));
}

CPPUNIT_TEST_FIXTURE(OOXMLRewriterTest, testAnExpressionThatNeedsNoRewrite)
{
    // A1+B1 reads the same in both grammars.
    FormulaTokenArray aTokens;
    addOperand(aTokens);
    aTokens.AddOpCode(ocAdd);
    addOperand(aTokens);

    CPPUNIT_ASSERT(!rewrite(aTokens));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
