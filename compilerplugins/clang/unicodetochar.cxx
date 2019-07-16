/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <stack>

#include "check.hxx"
#include "plugin.hxx"

// In C++, find implicit conversions from char16_t (aka sal_Unicode) to char.
// Such places are probably meant to properly work on char16_t instead.

namespace {

class UnicodeToChar final:
    public loplugin::FilteringPlugin<UnicodeToChar>
{
public:
    explicit UnicodeToChar(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    bool PreTraverseCStyleCastExpr(CStyleCastExpr * expr) {
        subExprs_.push(expr->getSubExpr());
        return true;
    }
    bool PostTraverseCStyleCastExpr(CStyleCastExpr *, bool ) {
        subExprs_.pop();
        return true;
    }
    bool TraverseCStyleCastExpr(CStyleCastExpr * expr) {
        PreTraverseCStyleCastExpr(expr);
        bool ret = RecursiveASTVisitor::TraverseCStyleCastExpr(expr);
        PostTraverseCStyleCastExpr(expr, ret);
        return ret;
    }

    bool PreTraverseCXXStaticCastExpr(CXXStaticCastExpr * expr) {
        subExprs_.push(expr->getSubExpr());
        return true;
    }
    bool PostTraverseCXXStaticCastExpr(CXXStaticCastExpr *, bool) {
        subExprs_.pop();
        return true;
    }
    bool TraverseCXXStaticCastExpr(CXXStaticCastExpr * expr) {
        PreTraverseCXXStaticCastExpr(expr);
        bool ret = RecursiveASTVisitor::TraverseCXXStaticCastExpr(expr);
        PostTraverseCXXStaticCastExpr(expr, ret);
        return ret;
    }

    bool PreTraverseCXXFunctionalCastExpr(CXXFunctionalCastExpr * expr) {
        subExprs_.push(expr->getSubExpr());
        return true;
    }
    bool PostTraverseCXXFunctionalCastExpr(CXXFunctionalCastExpr *, bool) {
        subExprs_.pop();
        return true;
    }
    bool TraverseCXXFunctionalCastExpr(CXXFunctionalCastExpr * expr) {
        PreTraverseCXXFunctionalCastExpr(expr);
        bool ret = RecursiveASTVisitor::TraverseCXXFunctionalCastExpr(expr);
        PostTraverseCXXFunctionalCastExpr(expr, ret);
        return ret;
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr const * expr) {
        if ((!subExprs_.empty() && expr == subExprs_.top())
            || ignoreLocation(expr))
        {
            return true;
        }
        if (!(loplugin::TypeCheck(expr->getType()).Char()
              && expr->getSubExpr()->getType()->isSpecificBuiltinType(
                  clang::BuiltinType::Char16)))
        {
            return true;
        }
        APSInt res;
        if (compat::EvaluateAsInt(expr->getSubExpr(), res, compiler.getASTContext())
            && res >= 0 && res <= 0x7F)
        {
            return true;
        }
        report(
            DiagnosticsEngine::Warning,
            "suspicious implicit cast from %0 to %1",
            expr->getExprLoc())
            << expr->getSubExpr()->getType() << expr->getType()
            << expr->getSourceRange();
        return true;
    }

    bool preRun() override {
        return compiler.getLangOpts().CPlusPlus;
    }

    void run() override {
        if (preRun()) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

private:
    std::stack<Expr const *> subExprs_;
};

static loplugin::Plugin::Registration<UnicodeToChar> unicodetochar("unicodetochar");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
