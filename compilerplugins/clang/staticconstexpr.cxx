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
In MSVC, non-static constexpr objects are initialized at run-time

So make sure that function-local vars are declared static.

*/
namespace
{
class StaticConstexpr final : public loplugin::FilteringPlugin<StaticConstexpr>
{
public:
    explicit StaticConstexpr(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitVarDecl(const VarDecl* varDecl)
    {
        if (ignoreLocation(varDecl))
            return true;
        if (!varDecl->isConstexpr())
            return true;
        if (!varDecl->isLocalVarDecl())
            return true;
        if (varDecl->isStaticLocal())
            return true;
        if (auto const functionDecl = dyn_cast_or_null<FunctionDecl>(varDecl->getDeclContext()))
        {
            // cannot convert these, definition of a static variable in a constexpr function is a C++23 extension
            if (functionDecl->isConstexpr())
                return true;
        }
        if (varDecl->getType()->isBuiltinType() || varDecl->getType()->isEnumeralType())
            return true;
        // ignore the o3tl::getConversionMulDiv stuff
        loplugin::TypeCheck tc(varDecl->getType());
        if (tc.ClassOrStruct("pair").StdNamespace())
            return true;
        if (tc.Struct("m_and_d").Namespace("detail").Namespace("o3tl"))
            return true;
        if (tc.ClassOrStruct("TypedWhichId"))
            return true;
        if (tc.ClassOrStruct("Color"))
            return true;
        report(DiagnosticsEngine::Warning,
               "function-local constexpr vars should be declared static", varDecl->getBeginLoc())
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

loplugin::Plugin::Registration<StaticConstexpr> staticconstexpr("staticconstexpr");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
