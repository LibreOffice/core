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

namespace
{
class ConstexprLiteral final : public loplugin::FilteringPlugin<ConstexprLiteral>
{
public:
    explicit ConstexprLiteral(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitVarDecl(const VarDecl* varDecl)
    {
        if (ignoreLocation(varDecl))
            return true;
        if (varDecl->isConstexpr())
            return true;
        if (!loplugin::TypeCheck(varDecl->getType())
                 .Class("OUStringLiteral")
                 .Namespace("rtl")
                 .GlobalNamespace())
            return true;
        report(DiagnosticsEngine::Warning, "OUStringLiteral should be declared constexpr",
               varDecl->getBeginLoc())
            << varDecl->getSourceRange();
        return true;
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

loplugin::Plugin::Registration<ConstexprLiteral> constexprliteral("constexprliteral");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
