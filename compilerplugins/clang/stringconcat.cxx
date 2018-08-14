/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "check.hxx"

namespace {

Expr const * stripCtor(Expr const * expr) {
    auto e0 = expr;
    auto const e1 = dyn_cast<CXXFunctionalCastExpr>(e0);
    if (e1 != nullptr) {
        e0 = e1->getSubExpr()->IgnoreParenImpCasts();
    }
    auto const e2 = dyn_cast<CXXBindTemporaryExpr>(e0);
    if (e2 == nullptr) {
        return expr;
    }
    auto const e3 = dyn_cast<CXXConstructExpr>(
        e2->getSubExpr()->IgnoreParenImpCasts());
    if (e3 == nullptr) {
        return expr;
    }
    auto qt = loplugin::DeclCheck(e3->getConstructor());
    if (!((qt.MemberFunction().Class("OString").Namespace("rtl")
           .GlobalNamespace())
          || (qt.MemberFunction().Class("OUString").Namespace("rtl")
              .GlobalNamespace())))
    {
        return expr;
    }
    if (e3->getNumArgs() != 2) {
        return expr;
    }
    return e3->getArg(0)->IgnoreParenImpCasts();
}

class StringConcat:
    public loplugin::FilteringPlugin<StringConcat>
{
public:
    explicit StringConcat(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCallExpr(CallExpr const * expr);

private:
    bool isStringLiteral(Expr const * expr);
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
        || !isStringLiteral(expr->getArg(1)->IgnoreParenImpCasts()))
    {
        return true;
    }
    SourceLocation leftLoc;
    auto const leftExpr = expr->getArg(0)->IgnoreParenImpCasts();
    if (isStringLiteral(leftExpr)) {
        leftLoc = compat::getBeginLoc(leftExpr);
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
            || !isStringLiteral(left->getArg(1)->IgnoreParenImpCasts()))
        {
            return true;
        }
        leftLoc = compat::getBeginLoc(left->getArg(1));
    }
    StringRef name {
        getFileNameOfSpellingLoc(
            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr))) };
    if (loplugin::isSamePathname(
            name, SRCDIR "/sal/qa/rtl/strings/test_ostring_concat.cxx")
        || loplugin::isSamePathname(
            name, SRCDIR "/sal/qa/rtl/strings/test_oustring_concat.cxx"))
    {
        return true;
    }
    CXXOperatorCallExpr const * op = dyn_cast<CXXOperatorCallExpr>(expr);
    report(
        DiagnosticsEngine::Warning,
        "replace '%0' between string literals with juxtaposition",
        op == nullptr ? expr->getExprLoc() : op->getOperatorLoc())
        << (oo == OverloadedOperatorKind::OO_Plus ? "+" : "<<")
        << SourceRange(leftLoc, compat::getEndLoc(expr->getArg(1)));
    return true;
}

bool StringConcat::isStringLiteral(Expr const * expr) {
    expr = stripCtor(expr);
    if (!isa<clang::StringLiteral>(expr)) {
        return false;
    }
    // OSL_THIS_FUNC may be defined as "" in include/osl/diagnose.h, so don't
    // warn about expressions like 'SAL_INFO(..., OSL_THIS_FUNC << ":")' or
    // 'OUString(OSL_THIS_FUNC) + ":"':
    auto loc = compat::getBeginLoc(expr);
    while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
        loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
    }
    return !compiler.getSourceManager().isMacroBodyExpansion(loc)
        || (Lexer::getImmediateMacroName(
                loc, compiler.getSourceManager(), compiler.getLangOpts())
            != "OSL_THIS_FUNC");
}

loplugin::Plugin::Registration<StringConcat> X("stringconcat");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
