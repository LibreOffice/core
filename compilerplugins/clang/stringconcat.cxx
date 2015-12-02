/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"

namespace {

class StringConcat:
    public RecursiveASTVisitor<StringConcat>, public loplugin::Plugin
{
public:
    explicit StringConcat(InstantiationData const & data): Plugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCallExpr(CallExpr const * expr);
};

bool StringConcat::VisitCallExpr(CallExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    FunctionDecl const * fdecl = expr->getDirectCallee();
    if (fdecl == nullptr) {
        return true;
    }
    OverloadedOperatorKind oo = fdecl->getOverloadedOperator();
    if ((oo != OverloadedOperatorKind::OO_Plus
         && oo != OverloadedOperatorKind::OO_LessLess)
        || fdecl->getNumParams() != 2 || expr->getNumArgs() != 2
        || !isa<StringLiteral>(expr->getArg(1)->IgnoreParenImpCasts()))
    {
        return true;
    }
    SourceLocation leftLoc;
    auto const leftExpr = expr->getArg(0)->IgnoreParenImpCasts();
    if (isa<StringLiteral>(leftExpr)) {
        leftLoc = leftExpr->getLocStart();
    } else {
        CallExpr const * left = dyn_cast<CallExpr>(leftExpr);
        if (left == nullptr) {
            return true;
        }
        FunctionDecl const * ldecl = left->getDirectCallee();
        if (ldecl == nullptr) {
            return true;
        }
        OverloadedOperatorKind loo = ldecl->getOverloadedOperator();
        if ((loo != OverloadedOperatorKind::OO_Plus
             && loo != OverloadedOperatorKind::OO_LessLess)
            || ldecl->getNumParams() != 2 || left->getNumArgs() != 2
            || !isa<StringLiteral>(left->getArg(1)->IgnoreParenImpCasts()))
        {
            return true;
        }
        leftLoc = left->getArg(1)->getLocStart();
    }
    StringRef name {
        compiler.getSourceManager().getFilename(
            compiler.getSourceManager().getSpellingLoc(expr->getLocStart())) };
    if (name == SRCDIR "/sal/qa/rtl/strings/test_ostring_concat.cxx"
        || name == SRCDIR "/sal/qa/rtl/strings/test_oustring_concat.cxx")
    {
        return true;
    }
    CXXOperatorCallExpr const * op = dyn_cast<CXXOperatorCallExpr>(expr);
    report(
        DiagnosticsEngine::Warning,
        "replace '%0' between string literals with juxtaposition",
        op == nullptr ? expr->getExprLoc() : op->getOperatorLoc())
        << (oo == OverloadedOperatorKind::OO_Plus ? "+" : "<<")
        << SourceRange(leftLoc, expr->getArg(1)->getLocEnd());
    return true;
}

loplugin::Plugin::Registration<StringConcat> X("stringconcat");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
