/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <set>

#include "plugin.hxx"

// Find places where we call a method with values == the values specified in the parameter defaults.
// i.e. where the code might as well not specify anything.

namespace {

class DefaultParams:
    public RecursiveASTVisitor<DefaultParams>, public loplugin::Plugin
{
public:
    explicit DefaultParams(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCallExpr(CallExpr * callExpr);
};

bool DefaultParams::VisitCallExpr(CallExpr * callExpr) {
    if (ignoreLocation(callExpr)) {
        return true;
    }
    if (callExpr->getDirectCallee() == nullptr) {
        return true;
    }
    const FunctionDecl* functionDecl = callExpr->getDirectCallee()->getCanonicalDecl();
    auto n = functionDecl->getNumParams();
    if (n == 0 || !functionDecl->getParamDecl(n - 1)->hasDefaultArg()) {
        return true;
    }
    assert(callExpr->getNumArgs() <= n); // can be < in template code
    for (unsigned i = callExpr->getNumArgs(); i != 0;) {
        --i;
        Expr* arg = callExpr->getArg(i);
        if (arg->isDefaultArgument()) {
            continue;
        }
        // ignore this, it seems to trigger an infinite recursion
        if (isa<UnaryExprOrTypeTraitExpr>(arg))
            break;
        const ParmVarDecl* parmVarDecl = functionDecl->getParamDecl(i);
        if (!parmVarDecl->hasDefaultArg()
            || parmVarDecl->hasUninstantiatedDefaultArg())
        {
            break;
        }
        const Expr* defaultArgExpr = parmVarDecl->getDefaultArg();
        if (!defaultArgExpr) {
            break;
        }
        APSInt x1, x2;
        if (!((defaultArgExpr->isNullPointerConstant(
                   compiler.getASTContext(), Expr::NPC_NeverValueDependent)
               && arg->isNullPointerConstant(
                   compiler.getASTContext(), Expr::NPC_NeverValueDependent))
              || (defaultArgExpr->EvaluateAsInt(x1, compiler.getASTContext())
                  && arg->EvaluateAsInt(x2, compiler.getASTContext())
                  && x1 == x2)))
        {
            break;
        }
        report(
            DiagnosticsEngine::Warning,
            "not necessary to pass this argument, it defaults to the same value",
            arg->getSourceRange().getBegin())
            << arg->getSourceRange();
        report(
            DiagnosticsEngine::Note,
            "default method parameter declaration here",
            parmVarDecl->getSourceRange().getBegin())
            << parmVarDecl->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration< DefaultParams > X("defaultparams");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
