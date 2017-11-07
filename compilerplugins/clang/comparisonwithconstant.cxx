/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include "compat.hxx"
#include "plugin.hxx"

/**
  Look for comparisons where the constant is on the left, it should be on the right.
 */

namespace {

class ComparisonWithConstant :
    public RecursiveASTVisitor<ComparisonWithConstant>, public loplugin::RewritePlugin
{
public:
    explicit ComparisonWithConstant(loplugin::InstantiationData const & data): RewritePlugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    // Deliberately drop RecursiveASTVisitor::TraverseBinEQ's DataRecursionQueue
    // parameter; TraveseBinEQ must use stack instead of data recursion for any
    // children's VisitBinaryOperator to see changes to occurrence_ by a parent
    // VisitBinaryOperator:
    bool TraverseBinEQ(BinaryOperator * S)
    {
        auto const saved = occurrence_;
        auto const ret = RecursiveASTVisitor::TraverseBinEQ(S);
        occurrence_ = saved;
        return ret;
    }

    // Deliberately drop RecursiveASTVisitor::TraverseBinNE's DataRecursionQueue
    // parameter; TraveseBinNE must use stack instead of data recursion for any
    // children's VisitBinaryOperator to see changes to occurrence_ by a parent
    // VisitBinaryOperator:
    bool TraverseBinNE(BinaryOperator * S)
    {
        auto const saved = occurrence_;
        auto const ret = RecursiveASTVisitor::TraverseBinNE(S);
        occurrence_ = saved;
        return ret;
    }

    bool VisitBinaryOperator(const BinaryOperator *);
private:
    bool rewrite(const BinaryOperator *);
    std::string getExprAsString(SourceRange range);
    SourceRange ignoreMacroExpansions(SourceRange range);

    bool occurrence_ = false;
};

bool ComparisonWithConstant::VisitBinaryOperator(const BinaryOperator* binaryOp)
{
    if (ignoreLocation(binaryOp)) {
        return true;
    }
    if (!(binaryOp->getOpcode() == BO_EQ || binaryOp->getOpcode() == BO_NE)) {
        return true;
    }
    // protect against clang assert
    if (binaryOp->getLHS()->isValueDependent() || binaryOp->getRHS()->isValueDependent()) {
        return true;
    }
    if (!binaryOp->getLHS()->isEvaluatable(compiler.getASTContext())) {
        return true;
    }
    if (binaryOp->getRHS()->isEvaluatable(compiler.getASTContext())) {
        return true;
    }
    if (occurrence_ || !rewrite(binaryOp))
    {
        report(
            DiagnosticsEngine::Warning, "Rather put constant on right when comparing",
            binaryOp->getSourceRange().getBegin())
            << binaryOp->getSourceRange();
    }
    occurrence_ = true;
    return true;
}


bool ComparisonWithConstant::rewrite(const BinaryOperator * binaryOp) {
    if (rewriter == nullptr) {
        return false;
    }

    auto lhsRange = ignoreMacroExpansions(binaryOp->getLHS()->getSourceRange());
    if (!lhsRange.isValid()) {
        return false;
    }
    auto rhsRange = ignoreMacroExpansions(binaryOp->getRHS()->getSourceRange());
    if (!rhsRange.isValid()) {
        return false;
    }

    const std::string lhsString = getExprAsString(lhsRange);
    const std::string rhsString = getExprAsString(rhsRange);

    // switch LHS and RHS
    if (!replaceText(lhsRange, rhsString)) {
        return false;
    }
    if (!replaceText(rhsRange, lhsString)) {
        return false;
    }

    return true;
}

// get the expression contents
std::string ComparisonWithConstant::getExprAsString(SourceRange range)
{
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = range.getBegin();
    SourceLocation endLoc = range.getEnd();
    const char *p1 = SM.getCharacterData( startLoc );
    const char *p2 = SM.getCharacterData( endLoc );
    unsigned n = Lexer::MeasureTokenLength( endLoc, SM, compiler.getLangOpts());
    return std::string( p1, p2 - p1 + n);
}

SourceRange ComparisonWithConstant::ignoreMacroExpansions(SourceRange range) {
    while (compiler.getSourceManager().isMacroArgExpansion(range.getBegin())) {
        range.setBegin(
            compiler.getSourceManager().getImmediateMacroCallerLoc(
                range.getBegin()));
    }
    if (range.getBegin().isMacroID()) {
        SourceLocation loc;
        if (Lexer::isAtStartOfMacroExpansion(
                range.getBegin(), compiler.getSourceManager(),
                compiler.getLangOpts(), &loc))
        {
            range.setBegin(loc);
        }
    }
    while (compiler.getSourceManager().isMacroArgExpansion(range.getEnd())) {
        range.setEnd(
            compiler.getSourceManager().getImmediateMacroCallerLoc(
                range.getEnd()));
    }
    if (range.getEnd().isMacroID()) {
        SourceLocation loc;
        if (Lexer::isAtEndOfMacroExpansion(
                range.getEnd(), compiler.getSourceManager(),
                compiler.getLangOpts(), &loc))
        {
            range.setEnd(loc);
        }
    }
    return range.getBegin().isMacroID() || range.getEnd().isMacroID()
        ? SourceRange() : range;
}

loplugin::Plugin::Registration< ComparisonWithConstant > X("comparisonwithconstant", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
