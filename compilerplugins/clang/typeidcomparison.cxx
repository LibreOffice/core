/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Find (in-)equality comparisons between typeid expressions that can never succeed.  For now, just
// detects cases where the two involved types are structurally different, one a pointer type and the
// other a non-pointer type.

#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"

namespace
{
class TypeidComparison final : public loplugin::FilteringPlugin<TypeidComparison>
{
public:
    explicit TypeidComparison(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    // For CXXRewrittenBinaryOperator `typeid(...) != typeid(...)`:
    bool shouldVisitImplicitCode() const { return true; }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const op = expr->getOperator();
        if (op != OO_EqualEqual && op != OO_ExclaimEqual)
        {
            return true;
        }
        assert(expr->getNumArgs() == 2);
        auto const e1 = dyn_cast<CXXTypeidExpr>(expr->getArg(0)->IgnoreParenImpCasts());
        if (e1 == nullptr)
        {
            return true;
        }
        auto const e2 = dyn_cast<CXXTypeidExpr>(expr->getArg(1)->IgnoreParenImpCasts());
        if (e2 == nullptr)
        {
            return true;
        }
        auto const t1 = getOperandType(e1);
        auto const t2 = getOperandType(e2);
        if (t1->isPointerType() == t2->isPointerType())
        {
            return true;
        }
        report(DiagnosticsEngine::Warning,
               "comparison of type info of mixed pointer and non-pointer types %0 and %1 can never "
               "succeed",
               expr->getExprLoc())
            << t1 << t2 << expr->getSourceRange();
        return true;
    }

private:
    QualType getOperandType(CXXTypeidExpr const* expr)
    {
        return expr->isTypeOperand() ? expr->getTypeOperand(compiler.getASTContext())
                                     : expr->getExprOperand()->getType();
    }
};

static loplugin::Plugin::Registration<TypeidComparison> typeidcomparison("typeidcomparison");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
