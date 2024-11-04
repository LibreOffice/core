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
#include <stack>

#include "check.hxx"
#include "plugin.hxx"
#include "config_clang.h"

/**
look for places where we take a reference (i.e. "&") to a temporary value.

Which is rather dodgy.

*/
namespace
{
class RefToTemp final : public loplugin::FilteringPlugin<RefToTemp>
{
public:
    explicit RefToTemp(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitVarDecl(const VarDecl* varDecl)
    {
        if (ignoreLocation(varDecl))
            return true;
        // reference parameters might have a default temporary value, which is ok
        if (isa<ParmVarDecl>(varDecl))
            return true;
        if (!varDecl->getType()->isReferenceType())
            return true;
        if (!varDecl->getInit())
            return true;
        if (!isTemporaryValue(varDecl->getInit()))
            return true;
        report(DiagnosticsEngine::Warning, "taking reference to temporary value",
               varDecl->getBeginLoc())
            << varDecl->getSourceRange();
        return true;
    }

    bool isTemporaryValue(const Expr* expr)
    {
        if (dyn_cast<MaterializeTemporaryExpr>(expr))
            return true;
        if (auto exprWithCleanups = dyn_cast<ExprWithCleanups>(expr))
            if (dyn_cast_or_null<MaterializeTemporaryExpr>(
                    exprWithCleanups->getSubExpr()->IgnoreImpCasts()))
                return true;
        return false;
    }

    bool preRun() override
    {
        if (!compiler.getLangOpts().CPlusPlus)
            return false;
        return true;
    }

private:
    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

loplugin::Plugin::Registration<RefToTemp> reftotemp("reftotemp");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
