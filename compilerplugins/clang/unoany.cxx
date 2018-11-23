/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "plugin.hxx"


class UnoAny:
    public loplugin::FilteringPlugin<UnoAny>
{
public:
    explicit UnoAny(loplugin::InstantiationData const & data): FilteringPlugin(data) {}

    void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const * expr);
};

bool UnoAny::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const * expr)
{
    if (ignoreLocation(expr)) {
        return true;
    }
    StringRef aFileName = getFileNameOfSpellingLoc(
            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr)));
    if (loplugin::isSamePathname(aFileName, SRCDIR "/include/com/sun/star/uno/Any.hxx")) {
        return true;
    }
    if (expr->getOperator() != OO_Equal) {
        return true;
    }
    if (!loplugin::TypeCheck(expr->getArg(0)->getType()).Class("Any").
         Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace())
    {
        return true;
    }
    if (auto expr2 = dyn_cast<MaterializeTemporaryExpr>(expr->getArg(1))) {
        if (auto expr3 = dyn_cast<CXXBindTemporaryExpr>(expr2->GetTemporaryExpr())) {
            if (auto expr4 = dyn_cast<CallExpr>(expr3->getSubExpr())) {
                if (loplugin::DeclCheck(expr4->getDirectCallee()).Function("makeAny").
                    Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()) {
                    report(
                            DiagnosticsEngine::Warning,
                            ("unnecessary copy, rather use <<= operator directly with the 'makeAny'"
                             " argument"),
                            expr->getOperatorLoc())
                      << expr->getSourceRange();
                    return true;
                }
            }
        }
        if (isa<CXXFunctionalCastExpr>(expr2->GetTemporaryExpr())) {
            //expr->getArg(1)->dump();
            report(
                    DiagnosticsEngine::Warning,
                    ("unnecessary copy, rather use <<= operator directly with the 'Any' constructor"
                     " argument"),
                    expr->getOperatorLoc())
              << expr->getSourceRange();
            return true;
        }
    }
    //expr->getArg(1)->dump();
    return true;
}

loplugin::Plugin::Registration<UnoAny> X("unoany");


/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
