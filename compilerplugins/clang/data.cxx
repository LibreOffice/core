/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

// For std::array or std::vector x, replace &x[0] with x.data().

namespace
{
class Data final : public loplugin::FilteringPlugin<Data>
{
public:
    explicit Data(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitUnaryOperator(UnaryOperator const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        if (expr->getOpcode() != UO_AddrOf)
        {
            return true;
        }
        auto const e1 = dyn_cast<CXXOperatorCallExpr>(expr->getSubExpr()->IgnoreParenImpCasts());
        if (e1 == nullptr)
        {
            return true;
        }
        if (e1->getOperator() != OO_Subscript)
        {
            return true;
        }
        auto const t = e1->getArg(0)->getType();
        auto const chk = loplugin::TypeCheck(t);
        if (!(chk.Class("array").StdNamespace() || chk.Class("vector").StdNamespace()))
        {
            return true;
        }
        auto const e2 = e1->getArg(1);
        if (e2->isValueDependent())
        {
            return true;
        }
        APSInt v;
        if (!compat::EvaluateAsInt(e2, v, compiler.getASTContext()))
        {
            return true;
        }
        if (v != 0)
        {
            return true;
        }
        report(DiagnosticsEngine::Warning,
               "use 'data' member function to access first element of %0", expr->getExprLoc())
            << t << expr->getSourceRange();
        return true;
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

static loplugin::Plugin::Registration<Data> data("data");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
