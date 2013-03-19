/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include "plugin.hxx"

// Find those calls of rtl::OUString::equalsIgnoreAsciiCaseAscii and
// rtl::OUString::equalsIgnoreAsciiCaseAsciiL that could be simplified to call
// rtl::OUString::equalsIgnoreAsciiCase instead:

namespace {

class LiteralAlternative:
    public RecursiveASTVisitor<LiteralAlternative>, public loplugin::Plugin
{
public:
    explicit LiteralAlternative(ASTContext & context): Plugin(context) {}

    virtual void run() { TraverseDecl(context.getTranslationUnitDecl()); }

    bool VisitCallExpr(CallExpr * expr);
};

bool LiteralAlternative::VisitCallExpr(CallExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    FunctionDecl const * fdecl = expr->getDirectCallee();
    if (fdecl == nullptr) {
        return true;
    }
    std::string qname { fdecl->getQualifiedNameAsString() };
    if (qname == "rtl::OUString::equalsIgnoreAsciiCaseAscii"
        && fdecl->getNumParams() == 1 && expr->getNumArgs() == 1)
    {
        // equalsIgnoreAsciiCaseAscii("foo") -> equalsIngoreAsciiCase("foo"):
        StringLiteral const * lit
            = dyn_cast<StringLiteral>(expr->getArg(0)->IgnoreParenImpCasts());
        if (lit != nullptr) {
            report(
                DiagnosticsEngine::Warning,
                ("rewrite call of rtl::OUString::equalsIgnoreAsciiCaseAscii"
                 " with string literal argument as call of"
                 " rtl::OUString::equalsIgnoreAsciiCase"),
                expr->getExprLoc());
                //TODO: a better loc (the "equalsIgnoreAsciiCaseAscii" part)?
        }
        return true;
    }
    if (qname == "rtl::OUString::equalsIgnoreAsciiCaseAsciiL"
        && fdecl->getNumParams() == 2 && expr->getNumArgs() == 2)
    {
        // equalsIgnoreAsciiCaseAsciiL("foo", 3) -> equalsIngoreAsciiCase("foo")
        // especially also for
        // equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("foo")), where
        // RTL_CONSTASCII_STRINGPARAM expands to complicated expressions
        // involving (&(X)[0] sub-expressions (and it might or might not be
        // better to handle that at the level of non-expanded macros instead,
        // but I have not found out how to do that yet anyway):
        APSInt res;
        if (expr->getArg(1)->isIntegerConstantExpr(res, context)) {
            Expr const * arg0 = expr->getArg(0)->IgnoreParenImpCasts();
            StringLiteral const * lit = dyn_cast<StringLiteral>(arg0);
            bool match = false;
            if (lit != nullptr) {
                match = res == lit->getLength();
            } else {
                UnaryOperator const * op = dyn_cast<UnaryOperator>(arg0);
                if (op != nullptr && op->getOpcode() == UO_AddrOf) {
                    ArraySubscriptExpr const * subs
                        = dyn_cast<ArraySubscriptExpr>(
                            op->getSubExpr()->IgnoreParenImpCasts());
                    if (subs != nullptr) {
                        lit = dyn_cast<StringLiteral>(
                            subs->getBase()->IgnoreParenImpCasts());
                        match = lit != nullptr
                            && subs->getIdx()->isIntegerConstantExpr(
                                res, context)
                            && res == 0;
                    }
                }
            }
            if (match) {
                report(
                    DiagnosticsEngine::Warning,
                    ("rewrite call of"
                     " rtl::OUString::equalsIgnoreAsciiCaseAsciiL with string"
                     " literal and matching length arguments as call of"
                     " rtl::OUString::equalsIgnoreAsciiCase"),
                    expr->getExprLoc());
                    //TODO: a better loc (the "equalsIgnoreAsciiCaseAsciiL"
                    // part)?
            }
        }
        return true;
    }
    return true;
}

loplugin::Plugin::Registration< LiteralAlternative > X("literalalternative");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
