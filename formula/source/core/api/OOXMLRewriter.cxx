/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <OOXMLRewriter.hxx>

#include <formula/token.hxx>
#include <formula/tokenarray.hxx>

#include <algorithm>
#include <memory>
#include <ranges>
#include <vector>

namespace formula
{
namespace
{
const sal_uInt16 NO_POSITION = SAL_MAX_UINT16;

// Tokens the OOXML shape has no place for: the call name of the wrapper around their operand
// stands for them.
bool lclIsDropped(OpCode eOp) { return eOp == ocSingleValue || eOp == ocSpill; }

// Tokens that write no text, so an operand made of nothing else leaves its wrapper with
// nothing to enclose and the wrapper goes.
bool lclWritesNoText(OpCode eOp) { return lclIsDropped(eOp) || eOp == ocMissing; }

// A parenthesis pair to put around a token range, with a call name before it. What the first
// pass hands the second.
struct OOXMLWrapper
{
    // Half-open range of token positions.
    sal_uInt16 mnBegin;
    sal_uInt16 mnEnd;
    // Call name before the opening parenthesis, ocNone for a bare pair.
    OpCode meOperator;
    // False when the range already is a parenthesised group, so its own pair serves.
    bool mbOwnParentheses;
};

// Operators that can come before an operand. A sign there is always the unary form, no
// matter which of the two opcodes the parser used. @ and # are left out, they get their own
// wrappers.
bool lclIsPrefixOperator(OpCode eOp)
{
    if (eOp == ocSingleValue || eOp == ocSpill)
        return false;
    return eOp == ocAdd || eOp == ocSub || isUnaryOperatorOpCode(eOp);
}

bool lclIsFactorSeparator(OpCode eOp)
{
    // Tokens between two factors at the same parenthesis depth - a new operand starts after
    // them.
    if (isBinaryOperatorOpCode(eOp))
        return true;
    return eOp == ocSep || eOp == ocArrayColSep || eOp == ocArrayRowSep;
}

// Tokens that end the operand of a preceding @. Range, intersection, union and call bind
// tighter than the @ so they stay inside, everything above it in the grammar ends it.
bool lclEndsSingleValueOperand(OpCode eOp)
{
    if (eOp == ocRange || eOp == ocIntersect || eOp == ocUnion || eOp == ocCall)
        return false;
    return lclIsFactorSeparator(eOp) || eOp == ocPercentSign;
}

// Single pass over the token array to collect what the OOXML spelling needs.
//
// Precedence comes from the order the tokens appear in, so this is a second statement of the
// precedence the parser already applied. Change how an operator binds in one place and the
// other one needs the same change.
class RewriteCollector
{
public:
    RewriteCollector(const FormulaTokenArray& rTokens, std::vector<OOXMLWrapper>& rWrappers,
                     std::vector<sal_uInt16>& rErrorPositions)
        : mrTokens(rTokens)
        , mrWrappers(rWrappers)
        , mrErrorPositions(rErrorPositions)
    {
    }

    void collect();

private:
    // An @ we have not recorded a wrapper for yet.
    struct PendingSingleValue
    {
        // Parenthesis scope the operand is in, 1 is the outermost.
        size_t mnScope;
        // First token position of the operand.
        sal_uInt16 mnBegin;
        // Set once a token that actually carries an operand shows up in the range.
        bool mbHasOperand;
    };

    // Positions tracked for one open parenthesis scope.
    struct Scope
    {
        // First token position inside the scope.
        sal_uInt16 mnContentStart;
        // Start of the union-level expression we are reading.
        sal_uInt16 mnExpressionStart;
        // Start of the current factor: an operand plus its parentheses and arguments.
        sal_uInt16 mnFactorStart;
        // Start of an open union list, NO_POSITION if there is none.
        sal_uInt16 mnListStart;
        // The scope is a grouping parenthesis, not a call's argument list.
        bool mbGroupParenthesis;
    };

    OpCode opCodeAt(sal_uInt16 nPosition) const;
    sal_uInt16 literalOffsetArgument(sal_uInt16 nPosition) const;
    bool isCallBeforeArguments(OpCode eOp, sal_uInt16 nPosition) const;
    bool isOneParenthesizedGroup(sal_uInt16 nBegin, sal_uInt16 nEnd) const;
    bool hasPendingAtStart(sal_uInt16 nContentStart) const;

    void markPendingOperand();
    static void movePastToken(Scope& rScope, sal_uInt16 nPosition);
    void closeSingleValues(size_t nScope, sal_uInt16 nEnd);
    void cancelSingleValues(size_t nScope);
    void closeUnionList(sal_uInt16 nEnd, bool bMayTakeScopeParenthesis);

    const FormulaTokenArray& mrTokens;
    std::vector<OOXMLWrapper>& mrWrappers;
    std::vector<sal_uInt16>& mrErrorPositions;
    std::vector<PendingSingleValue> maPendingSingles;
    std::vector<Scope> maScopes;
};

OpCode RewriteCollector::opCodeAt(sal_uInt16 nPosition) const
{
    return mrTokens.TokenAt(nPosition)->GetOpCode();
}

// Position of the first argument if the token at nPosition opens an OFFSET() call with a
// literal string or number there, NO_POSITION otherwise. OFFSET() needs a reference, so a
// literal cannot be written out in a way any reader accepts.
sal_uInt16 RewriteCollector::literalOffsetArgument(sal_uInt16 nPosition) const
{
    if (nPosition + 2 >= mrTokens.GetLen())
        return NO_POSITION;
    const FormulaToken* pName = mrTokens.TokenAt(nPosition);
    if (pName->GetOpCode() != ocOffset || pName->GetType() != svByte)
        return NO_POSITION;
    const FormulaToken* pOpen = mrTokens.TokenAt(sal_uInt16(nPosition + 1));
    if (pOpen->GetOpCode() != ocOpen || pOpen->GetType() != svSep)
        return NO_POSITION;
    const FormulaToken* pArgument = mrTokens.TokenAt(sal_uInt16(nPosition + 2));
    if (pArgument->GetOpCode() != ocPush)
        return NO_POSITION;
    const StackVar eType = pArgument->GetType();
    if (eType != svString && eType != svDouble)
        return NO_POSITION;
    return sal_uInt16(nPosition + 2);
}

// AND, OR and the call operator have operator opcodes but are written as calls, so the
// parenthesis after the name opens an argument list, not a group.
bool RewriteCollector::isCallBeforeArguments(OpCode eOp, sal_uInt16 nPosition) const
{
    if (eOp != ocAnd && eOp != ocOr && eOp != ocCall)
        return false;
    for (sal_uInt16 nNext = sal_uInt16(nPosition + 1); nNext < mrTokens.GetLen(); ++nNext)
    {
        const OpCode eNext = opCodeAt(nNext);
        if (!isWhitespaceOpCode(eNext))
            return eNext == ocOpen;
    }
    return false;
}

// The range is a single parenthesised group, so we can reuse its parentheses for the
// wrapper.
bool RewriteCollector::isOneParenthesizedGroup(sal_uInt16 nBegin, sal_uInt16 nEnd) const
{
    if (nEnd - nBegin < 2 || opCodeAt(nBegin) != ocOpen)
        return false;
    sal_Int32 nDepth = 0;
    for (sal_uInt16 nPosition = nBegin; nPosition < nEnd; ++nPosition)
    {
        // Inline matrix braces and table reference brackets have separators of their own,
        // so count them as scopes too.
        const OpCode eOp = opCodeAt(nPosition);
        if (eOp == ocOpen || eOp == ocArrayOpen || eOp == ocTableRefOpen)
        {
            ++nDepth;
        }
        else if (eOp == ocClose || eOp == ocArrayClose || eOp == ocTableRefClose)
        {
            --nDepth;
            if (nDepth == 0)
                return nPosition == nEnd - 1;
        }
        else if (nDepth == 1 && (eOp == ocUnion || eOp == ocSep))
        {
            // A separator directly inside means the parentheses hold arguments together,
            // so we cannot reuse them.
            return false;
        }
    }
    return false;
}

// A pending operand starts at or before the scope's own tokens, so the scope's parentheses
// are already taken.
bool RewriteCollector::hasPendingAtStart(sal_uInt16 nContentStart) const
{
    for (const PendingSingleValue& rSingle : maPendingSingles | std::views::reverse)
    {
        if (rSingle.mnScope < maScopes.size())
            break;
        if (rSingle.mnBegin <= nContentStart)
            return true;
    }
    return false;
}

// A token that writes no text cannot be the start of anything, so move any start that sits
// on it forward.
void RewriteCollector::movePastToken(Scope& rScope, sal_uInt16 nPosition)
{
    if (rScope.mnContentStart == nPosition)
        rScope.mnContentStart = sal_uInt16(nPosition + 1);
    if (rScope.mnExpressionStart == nPosition)
        rScope.mnExpressionStart = sal_uInt16(nPosition + 1);
    if (rScope.mnFactorStart == nPosition)
        rScope.mnFactorStart = sal_uInt16(nPosition + 1);
}

// Mark pending operands as filled. A mark is never cleared, so walking from the back can
// stop at the first marked one.
void RewriteCollector::markPendingOperand()
{
    for (PendingSingleValue& rSingle : maPendingSingles | std::views::reverse)
    {
        if (rSingle.mbHasOperand)
            break;
        rSingle.mbHasOperand = true;
    }
}

// Record the wrappers pending from nScope and deeper, innermost first. Each operand runs
// from its own start to nEnd.
void RewriteCollector::closeSingleValues(size_t nScope, sal_uInt16 nEnd)
{
    while (!maPendingSingles.empty() && maPendingSingles.back().mnScope >= nScope)
    {
        const PendingSingleValue& rSingle = maPendingSingles.back();
        // An empty operand gets no wrapper.
        if (rSingle.mbHasOperand)
        {
            mrWrappers.push_back({ rSingle.mnBegin, nEnd, ocSingleValue,
                                   !isOneParenthesizedGroup(rSingle.mnBegin, nEnd) });
            // The wrapper now encloses the expression, so the expression starts at the
            // wrapper. Whitespace the scope start had skipped is inside the wrapper too,
            // so pull that start back as well.
            if (maScopes.back().mnExpressionStart > rSingle.mnBegin)
                maScopes.back().mnExpressionStart = rSingle.mnBegin;
            if (maScopes.back().mnContentStart > rSingle.mnBegin)
                maScopes.back().mnContentStart = rSingle.mnBegin;
        }
        maPendingSingles.pop_back();
    }
}

// Throw away the wrappers pending from nScope and deeper.
void RewriteCollector::cancelSingleValues(size_t nScope)
{
    while (!maPendingSingles.empty() && maPendingSingles.back().mnScope >= nScope)
        maPendingSingles.pop_back();
}

// Record the parentheses for the scope's open union list. If the list exactly fills a
// grouping parenthesis and bMayTakeScopeParenthesis allows it, reuse that pair.
void RewriteCollector::closeUnionList(sal_uInt16 nEnd, bool bMayTakeScopeParenthesis)
{
    Scope& rScope = maScopes.back();
    if (rScope.mnListStart == NO_POSITION)
        return;
    if (!(bMayTakeScopeParenthesis && rScope.mbGroupParenthesis
          && rScope.mnListStart == rScope.mnContentStart))
    {
        mrWrappers.push_back({ rScope.mnListStart, nEnd, ocNone, true });
    }
    rScope.mnListStart = NO_POSITION;
}

void RewriteCollector::collect()
{
    const sal_uInt16 nLength = mrTokens.GetLen();
    maScopes.push_back({ 0, 0, 0, NO_POSITION, false });
    // The next token is where an operand starts, so a plus or minus there is the unary form.
    bool bOperandPosition = true;
    // The current factor is raw text that never parsed - we cannot put wrapper parentheses
    // around that.
    bool bBadFactor = false;

    for (sal_uInt16 nPosition = 0; nPosition < nLength; ++nPosition)
    {
        const OpCode eOp = opCodeAt(nPosition);

        const sal_uInt16 nLiteralOffsetArgument = literalOffsetArgument(nPosition);
        if (nLiteralOffsetArgument != NO_POSITION)
            mrErrorPositions.push_back(nLiteralOffsetArgument);

        // The @ writes no text of its own. Remember where the operand starts and which scope
        // it is in, the wrapper gets recorded once we know how far the operand reaches.
        if (eOp == ocSingleValue)
        {
            Scope& rScope = maScopes.back();
            // Any start sitting on the @ moves on to the operand after it, and so does the
            // factor it begins.
            movePastToken(rScope, nPosition);
            rScope.mnFactorStart = sal_uInt16(nPosition + 1);
            maPendingSingles.push_back({ maScopes.size(), sal_uInt16(nPosition + 1), false });
            continue;
        }

        // The # is written as a call around the factor before it and binds tighter than any
        // operator that can come before that factor. Raw text that never parsed takes no
        // parentheses, so drop a # after one, and drop a # with no factor before it at all.
        if (eOp == ocSpill)
        {
            if (!bBadFactor)
            {
                mrWrappers.push_back(
                    { maScopes.back().mnFactorStart, nPosition, ocAnchorArray, true });
                markPendingOperand();
            }
            bOperandPosition = false;
            continue;
        }

        const bool bWhitespace = isWhitespaceOpCode(eOp);
        // A sign where an operand starts is always the unary form, whichever opcode the
        // parser used, and it belongs to the operand after it.
        const bool bPrefix = bOperandPosition && lclIsPrefixOperator(eOp);
        const bool bCallName = !bPrefix && isCallBeforeArguments(eOp, nPosition);
        const bool bSeparator = !bPrefix && !bCallName && lclIsFactorSeparator(eOp);
        // The union list and the @ operand cover the same part of the grammar, so the same
        // tokens end both.
        const bool bEndsOperand = !bPrefix && !bCallName && lclEndsSingleValueOperand(eOp);

        // A structured table reference comes through here token by token, so its brackets
        // scope like parentheses and the separators inside stay with the reference.
        if (eOp == ocOpen || eOp == ocTableRefOpen)
        {
            markPendingOperand();
            // A grouping parenthesis is where an operand starts, a call's parenthesis comes
            // after the function name.
            maScopes.push_back({ sal_uInt16(nPosition + 1), sal_uInt16(nPosition + 1),
                                 sal_uInt16(nPosition + 1), NO_POSITION,
                                 eOp == ocOpen && bOperandPosition });
            bOperandPosition = true;
            bBadFactor = false;
            continue;
        }
        if (eOp == ocClose || eOp == ocTableRefClose)
        {
            if (maScopes.size() > 1)
            {
                // The union list closes before the @ wrappers, so its parentheses end up
                // inside theirs.
                const bool bMayTakeScopeParenthesis
                    = !hasPendingAtStart(maScopes.back().mnContentStart);
                closeUnionList(nPosition, bMayTakeScopeParenthesis);
                closeSingleValues(maScopes.size(), nPosition);
                maScopes.pop_back();
            }
            markPendingOperand();
            bOperandPosition = false;
            bBadFactor = false;
            continue;
        }

        // An operator higher up in the grammar ends both the union list and the @ operand,
        // so their wrappers close before it.
        if (bEndsOperand)
        {
            closeUnionList(nPosition, false);
            closeSingleValues(maScopes.size(), nPosition);
        }

        if (eOp == ocBad)
        {
            // Raw text that never parsed cannot take parentheses, so drop the wrappers
            // waiting on it.
            maScopes.back().mnListStart = NO_POSITION;
            cancelSingleValues(maScopes.size());
            bBadFactor = true;
        }

        Scope& rScope = maScopes.back();
        // Whitespace before a scope, expression or factor is not part of it.
        if (bWhitespace)
            movePastToken(rScope, nPosition);
        if (!bPrefix && !bWhitespace && !lclWritesNoText(eOp))
            markPendingOperand();
        if (eOp == ocUnion && rScope.mnListStart == NO_POSITION)
        {
            // The first union operator opens the list, the ones after it just extend it.
            rScope.mnListStart = rScope.mnExpressionStart;
        }
        if (bPrefix || (bEndsOperand && bSeparator))
        {
            // A prefix sign or an infix operator starts a new expression after it. A postfix
            // one does not, it belongs to the operand before it.
            rScope.mnExpressionStart = sal_uInt16(nPosition + 1);
        }
        if (bPrefix || bSeparator || bEndsOperand)
        {
            // Every operator and separator ends the factor before it, so the next factor
            // starts after. A function name does not.
            rScope.mnFactorStart = sal_uInt16(nPosition + 1);
        }
        if (bSeparator)
            bBadFactor = false;
        if (!bWhitespace)
            bOperandPosition = bPrefix || bSeparator;
    }

    // Anything still pending reaches the end of the formula. Union lists close first so
    // their parentheses end up inside the @ wrappers.
    while (maScopes.size() > 1)
    {
        closeUnionList(nLength, false);
        maScopes.pop_back();
    }
    closeUnionList(nLength, false);
    closeSingleValues(1, nLength);

    // A wrapper recorded later always reaches at least as far as an earlier one, so sorting
    // by end descending puts the wider of two wrappers with the same begin first. The
    // reverse before that handles the ones that also end at the same position.
    std::reverse(mrWrappers.begin(), mrWrappers.end());
    std::stable_sort(mrWrappers.begin(), mrWrappers.end(),
                     [](const OOXMLWrapper& rLeft, const OOXMLWrapper& rRight) {
                         if (rLeft.mnBegin != rRight.mnBegin)
                             return rLeft.mnBegin < rRight.mnBegin;
                         return rLeft.mnEnd > rRight.mnEnd;
                     });
}

// Copy rTokens with the recorded call names and parentheses put in, and #REF! for the tokens
// that have no OOXML spelling.
std::unique_ptr<FormulaTokenArray> lclBuild(const FormulaTokenArray& rTokens,
                                            const std::vector<OOXMLWrapper>& rWrappers,
                                            const std::vector<sal_uInt16>& rErrorPositions)
{
    auto pRewritten = std::make_unique<FormulaTokenArray>();
    pRewritten->AddRecalcMode(rTokens.GetRecalcMode());

    // The end position of every pair opened so far, innermost last.
    std::vector<sal_uInt16> aOpenEnds;
    size_t nNextWrapper = 0;
    const sal_uInt16 nLength = rTokens.GetLen();
    // One past the last token, so the pairs reaching the end get closed.
    for (sal_uInt16 nPosition = 0; nPosition <= nLength; ++nPosition)
    {
        while (!aOpenEnds.empty() && aOpenEnds.back() <= nPosition)
        {
            pRewritten->AddOpCode(ocClose);
            aOpenEnds.pop_back();
        }
        while (nNextWrapper < rWrappers.size() && rWrappers[nNextWrapper].mnBegin <= nPosition)
        {
            const OOXMLWrapper& rWrapper = rWrappers[nNextWrapper++];
            // Nothing between the ends, so there is nothing to enclose and the pair goes.
            // A # with no factor in front of it is the case that gets here.
            if (rWrapper.mnBegin == rWrapper.mnEnd)
                continue;
            if (rWrapper.meOperator != ocNone)
                pRewritten->AddOpCode(rWrapper.meOperator);
            if (rWrapper.mbOwnParentheses)
            {
                pRewritten->AddOpCode(ocOpen);
                aOpenEnds.push_back(rWrapper.mnEnd);
            }
        }
        if (nPosition == nLength)
            break;

        const FormulaToken* pToken = rTokens.TokenAt(nPosition);
        const OpCode eOp = pToken->GetOpCode();
        if (lclIsDropped(eOp))
        {
            // The wrapper's call name stands for these. A # that got no wrapper had
            // nothing to bind to, so it goes.
            continue;
        }
        if (std::binary_search(rErrorPositions.begin(), rErrorPositions.end(), nPosition))
            pRewritten->AddOpCode(ocErrRef);
        else
            pRewritten->AddToken(*pToken);
    }
    return pRewritten;
}

// Whether the OOXML shape differs from the array as it stands. Dropping a token counts, so a
// lone @ with no operand is a change even though it takes no wrapper.
bool lclNeedsRewrite(const FormulaTokenArray& rTokens, const std::vector<OOXMLWrapper>& rWrappers,
                     const std::vector<sal_uInt16>& rErrorPositions)
{
    if (!rWrappers.empty() || !rErrorPositions.empty())
        return true;
    for (sal_uInt16 nPosition = 0; nPosition < rTokens.GetLen(); ++nPosition)
    {
        if (lclIsDropped(rTokens.TokenAt(nPosition)->GetOpCode()))
            return true;
    }
    return false;
}

} // namespace

OOXMLRewriter::OOXMLRewriter(const FormulaTokenArray& rTokens)
{
    std::vector<OOXMLWrapper> aWrappers;
    std::vector<sal_uInt16> aErrorPositions;
    RewriteCollector(rTokens, aWrappers, aErrorPositions).collect();
    if (!lclNeedsRewrite(rTokens, aWrappers, aErrorPositions))
        return;
    mpRewritten = lclBuild(rTokens, aWrappers, aErrorPositions);
}

} // namespace formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
