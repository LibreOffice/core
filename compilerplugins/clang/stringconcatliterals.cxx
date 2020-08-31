/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"
#include "check.hxx"

namespace {

Expr const * stripCtor(Expr const * expr) {
    auto e1 = expr;
    if (auto const e = dyn_cast<CXXFunctionalCastExpr>(e1)) {
        e1 = e->getSubExpr()->IgnoreParenImpCasts();
    }
    if (auto const e = dyn_cast<CXXBindTemporaryExpr>(e1)) {
        e1 = e->getSubExpr()->IgnoreParenImpCasts();
    }
    auto const e2 = dyn_cast<CXXConstructExpr>(e1);
    if (e2 == nullptr) {
        return expr;
    }
    auto qt = loplugin::DeclCheck(e2->getConstructor());
    if (qt.MemberFunction().Class("OStringLiteral").Namespace("rtl").GlobalNamespace()) {
        if (e2->getNumArgs() == 1) {
            return e2->getArg(0)->IgnoreParenImpCasts();
        }
        return expr;
    }
    if (!((qt.MemberFunction().Class("OString").Namespace("rtl")
           .GlobalNamespace())
          || (qt.MemberFunction().Class("OUString").Namespace("rtl")
              .GlobalNamespace())
          || qt.MemberFunction().Struct("OUStringLiteral").Namespace("rtl").GlobalNamespace()))
    {
        return expr;
    }
    if (e2->getNumArgs() != 2) {
        return expr;
    }
    return e2->getArg(0)->IgnoreParenImpCasts();
}

class StringConcatLiterals:
    public loplugin::FilteringPlugin<StringConcatLiterals>
{
public:
    explicit StringConcatLiterals(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCallExpr(CallExpr const * expr);

private:
    bool isStringLiteral(Expr const * expr);
};

bool StringConcatLiterals::VisitCallExpr(CallExpr const * expr) {
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

    // We add an extra " " in the TOOLS_WARN_EXCEPTION macro, which triggers this plugin
    if (loplugin::isSamePathname(
            getFilenameOfLocation(
                compiler.getSourceManager().getSpellingLoc(
                    compiler.getSourceManager().getImmediateMacroCallerLoc(
                        compiler.getSourceManager().getImmediateMacroCallerLoc(
                            compiler.getSourceManager().getImmediateMacroCallerLoc(
                                compat::getBeginLoc(expr)))))),
            SRCDIR "/include/tools/diagnose_ex.h"))
        return true;

    StringRef name {
        getFilenameOfLocation(
            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr))) };
    if (loplugin::isSamePathname(
            name, SRCDIR "/sal/qa/rtl/oustringbuffer/test_oustringbuffer_assign.cxx")
        || loplugin::isSamePathname(
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

bool StringConcatLiterals::isStringLiteral(Expr const * expr) {
    expr = stripCtor(expr);
    if (!isa<clang::StringLiteral>(expr)) {
        return false;
    }
    return true;
}

loplugin::Plugin::Registration<StringConcatLiterals> stringconcatliterals("stringconcatliterals");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
