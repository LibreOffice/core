/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Warn about certain redundant casts:
//
// * A reinterpret_cast<T*>(...) whose result is then implicitly cast to a void
//   pointer
//
// * A static_cast<T*>(e) where e is of void pointer type and whose result is
//   then implicitly cast to a void pointer
//
// C-style casts are ignored because it makes this plugin simpler, and they
// should eventually be eliminated via loplugin:cstylecast and/or
// -Wold-style-cast.  That implies that this plugin is only relevant for C++
// code.

#include "plugin.hxx"

namespace {

bool isVoidPointer(QualType type) {
    return type->isPointerType()
        && type->getAs<PointerType>()->getPointeeType()->isVoidType();
}

class RedundantCast:
    public RecursiveASTVisitor<RedundantCast>, public loplugin::Plugin
{
public:
    explicit RedundantCast(InstantiationData const & data): Plugin(data) {}

    virtual void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr const * expr);
};

bool RedundantCast::VisitImplicitCastExpr(const ImplicitCastExpr * expr) {
    if (ignoreLocation(expr) || expr->getCastKind() != CK_BitCast
        || !isVoidPointer(expr->getType())
        || !expr->getSubExpr()->getType()->isPointerType())
    {
        return true;
    }
    Expr const * e = expr->getSubExpr()->IgnoreParenImpCasts();
    while (isa<CXXConstCastExpr>(e)) {
        e = dyn_cast<CXXConstCastExpr>(e)->getSubExpr()->IgnoreParenImpCasts();
    }
    if (isa<CXXReinterpretCastExpr>(e)) {
        report(
            DiagnosticsEngine::Warning,
            ("redundant reinterpret_cast, result is implicitly cast to void"
             " pointer"),
            e->getExprLoc())
            << e->getSourceRange();
    } else if (isa<CXXStaticCastExpr>(e)
               && isVoidPointer(
                   dyn_cast<CXXStaticCastExpr>(e)->getSubExpr()
                   ->IgnoreParenImpCasts()->getType()))
    {
        report(
            DiagnosticsEngine::Warning,
            ("redundant static_cast from void pointer, result is implicitly"
             " cast to void pointer"),
            e->getExprLoc())
            << e->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<RedundantCast> X("redundantcast");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
