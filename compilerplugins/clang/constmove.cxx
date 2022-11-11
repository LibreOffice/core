/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Find occurrences of std::move on const-qualified types.  While there might theoretically be
// legitimate uses for such (for which this plugin would generate false positives and would need to
// be updated), in practice they tend to point at suspicious code that should be cleaned up in some
// way.

#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "plugin.hxx"

namespace
{
class ConstMove final : public loplugin::FilteringPlugin<ConstMove>
{
public:
    explicit ConstMove(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCallExpr(CallExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const t = expr->getType();
        if (!t.isConstQualified())
        {
            return true;
        }
        auto const d = expr->getDirectCallee();
        if (d == nullptr)
        {
            return true;
        }
        if (!loplugin::DeclCheck(d).Function("move").StdNamespace())
        {
            return true;
        }
        switch (expr->getNumArgs())
        {
            case 0:
                return true;
            case 1:
                break;
            default:
                if (!isa<CXXDefaultArgExpr>(expr->getArg(1)))
                {
                    return true;
                }
                break;
        }
        report(DiagnosticsEngine::Warning, "suspicious 'std::move' from %0 to const-qualified %1",
               expr->getExprLoc())
            << expr->getArg(0)->IgnoreImplicit()->getType() << t << expr->getSourceRange();
        return true;
    }
};

static loplugin::Plugin::Registration<ConstMove> constmove("constmove");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
