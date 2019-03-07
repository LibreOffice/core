/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>

#include "check.hxx"
#include "plugin.hxx"

// Warn about checks whether a container is empty done via an (expensive) call to obtain the
// container's size.  For now only handles cases involving strlen.

namespace
{
BinaryOperatorKind revert(BinaryOperatorKind op)
{
    switch (op)
    {
        case BO_LT:
            return BO_GE;
        case BO_GT:
            return BO_LE;
        case BO_LE:
            return BO_GT;
        case BO_GE:
            return BO_LT;
        case BO_EQ:
        case BO_NE:
            return op;
        default:
            assert(false);
    }
}

class Empty : public loplugin::FilteringPlugin<Empty>
{
public:
    explicit Empty(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitBinLT(BinaryOperator const* expr)
    {
        visitComparison(expr);
        return true;
    }

    bool VisitBinGT(BinaryOperator const* expr)
    {
        visitComparison(expr);
        return true;
    }

    bool VisitBinLE(BinaryOperator const* expr)
    {
        visitComparison(expr);
        return true;
    }

    bool VisitBinGE(BinaryOperator const* expr)
    {
        visitComparison(expr);
        return true;
    }

    bool VisitBinEQ(BinaryOperator const* expr)
    {
        visitComparison(expr);
        return true;
    }

    bool VisitBinNE(BinaryOperator const* expr)
    {
        visitComparison(expr);
        return true;
    }

private:
    void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    void visitComparison(BinaryOperator const* expr, CallExpr const* lhs, Expr const* rhs,
                         BinaryOperatorKind op)
    {
        auto const fdecl = lhs->getDirectCallee();
        if (fdecl == nullptr)
        {
            return;
        }
        loplugin::DeclCheck dc(fdecl);
        if (!(dc.Function("strlen").StdNamespace() || dc.Function("strlen").GlobalNamespace()))
        {
            return;
        }
        APSInt val;
        if (!rhs->isIntegerConstantExpr(val, compiler.getASTContext()))
        {
            return;
        }
        switch (op)
        {
            case BO_LT:
                if (val.getExtValue() == 1)
                {
                    report(DiagnosticsEngine::Warning,
                           "replace a comparison like 'strlen(e) < 1' with 'e[0] == '\\0''",
                           expr->getExprLoc())
                        << expr->getSourceRange();
                }
                break;
            case BO_GT:
                if (val.getExtValue() == 0)
                {
                    report(DiagnosticsEngine::Warning,
                           "replace a comparison like 'strlen(e) > 0' with 'e[0] != '\\0''",
                           expr->getExprLoc())
                        << expr->getSourceRange();
                }
                break;
            case BO_LE:
                if (val.getExtValue() == 0)
                {
                    report(DiagnosticsEngine::Warning,
                           "replace a comparison like 'strlen(e) <= 0' with 'e[0] == '\\0''",
                           expr->getExprLoc())
                        << expr->getSourceRange();
                }
                break;
            case BO_GE:
                if (val.getExtValue() == 1)
                {
                    report(DiagnosticsEngine::Warning,
                           "replace a comparison like 'strlen(e) >= 1' with 'e[0] != '\\0''",
                           expr->getExprLoc())
                        << expr->getSourceRange();
                }
                break;
            case BO_EQ:
                if (val.getExtValue() == 0)
                {
                    report(DiagnosticsEngine::Warning,
                           "replace a comparison like 'strlen(e) == 0' with 'e[0] == '\\0''",
                           expr->getExprLoc())
                        << expr->getSourceRange();
                }
                break;
            case BO_NE:
                if (val.getExtValue() == 0)
                {
                    report(DiagnosticsEngine::Warning,
                           "replace a comparison like 'strlen(e) != 0' with 'e[0] != '\\0''",
                           expr->getExprLoc())
                        << expr->getSourceRange();
                }
                break;
            default:
                assert(false);
        }
    }

    void visitComparison(BinaryOperator const* expr)
    {
        if (ignoreLocation(expr))
        {
            return;
        }
        if (auto const call = dyn_cast<CallExpr>(expr->getLHS()->IgnoreParenImpCasts()))
        {
            visitComparison(expr, call, expr->getRHS(), expr->getOpcode());
        }
        else if (auto const call = dyn_cast<CallExpr>(expr->getRHS()->IgnoreParenImpCasts()))
        {
            visitComparison(expr, call, expr->getLHS(), revert(expr->getOpcode()));
        }
    }
};

loplugin::Plugin::Registration<Empty> emptyRegistration("empty");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
