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

// Find matches of
//
//   foo(s.getStr())
//
// (for the rtl string classes) that can be written as just
//
//   foo(s)
//
// and warn about them, which prevents constructing unnecessary temporaries.

namespace
{
class UnnecessaryGetStr final : public loplugin::FilteringPlugin<UnnecessaryGetStr>
{
public:
    explicit UnnecessaryGetStr(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitCallExpr(CallExpr* callExpr)
    {
        if (ignoreLocation(callExpr))
            return true;
        const FunctionDecl* func = callExpr->getDirectCallee();
        if (!func)
            return true;
        unsigned const n = std::min(func->getNumParams(), callExpr->getNumArgs());
        for (unsigned i = 0; i != n; ++i)
        {
            auto arg = callExpr->getArg(i);
            if (auto matTemp = dyn_cast<MaterializeTemporaryExpr>(arg))
            {
                auto cxxConstruct = dyn_cast<CXXConstructExpr>(matTemp->IgnoreImplicit());
                if (!cxxConstruct || cxxConstruct->getNumArgs() < 1
                    || cxxConstruct->getNumArgs() > 2)
                    continue;
                auto const tc1 = loplugin::TypeCheck(cxxConstruct->getConstructor()->getParent());
                if (!(tc1.Class("OString").Namespace("rtl").GlobalNamespace()
                      || tc1.Class("OUString").Namespace("rtl").GlobalNamespace()))
                    continue;
                auto e = dyn_cast<CXXMemberCallExpr>(cxxConstruct->getArg(0)->IgnoreImplicit());
                if (!e)
                    continue;
                auto const t = e->getObjectType();
                auto const tc2 = loplugin::TypeCheck(t);
                if (!(tc2.Class("OString").Namespace("rtl").GlobalNamespace()
                      || tc2.Class("OUString").Namespace("rtl").GlobalNamespace()
                      || tc2.Class("OStringBuffer").Namespace("rtl").GlobalNamespace()
                      || tc2.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
                      || tc2.ClassOrStruct("StringNumber").Namespace("rtl").GlobalNamespace()))
                    continue;
                if (!loplugin::DeclCheck(e->getMethodDecl()).Function("getStr"))
                    continue;
                report(DiagnosticsEngine::Warning,
                       "unnecessary call to 'getStr' when passing to OString arg", e->getExprLoc())
                    << e->getSourceRange();
            }
            else if (auto impCast = dyn_cast<ImplicitCastExpr>(arg))
            {
                auto cxxConstruct = dyn_cast<CXXConstructExpr>(impCast->getSubExpr());
                if (!cxxConstruct || cxxConstruct->getNumArgs() < 1
                    || cxxConstruct->getNumArgs() > 2)
                    continue;
                auto const tc1 = loplugin::TypeCheck(cxxConstruct->getConstructor()->getParent());
                if (!(tc1.ClassOrStruct("basic_string_view").StdNamespace()))
                    continue;
                auto e = dyn_cast<CXXMemberCallExpr>(cxxConstruct->getArg(0)->IgnoreImplicit());
                if (!e)
                    continue;
                auto const t = e->getObjectType();
                auto const tc2 = loplugin::TypeCheck(t);
                if (tc2.Class("OString").Namespace("rtl").GlobalNamespace()
                    || tc2.Class("OUString").Namespace("rtl").GlobalNamespace()
                    || tc2.Class("OStringBuffer").Namespace("rtl").GlobalNamespace()
                    || tc2.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
                    || tc2.ClassOrStruct("StringNumber").Namespace("rtl").GlobalNamespace())
                {
                    if (loplugin::DeclCheck(e->getMethodDecl()).Function("getStr"))
                        report(DiagnosticsEngine::Warning,
                               "unnecessary call to 'getStr' when passing to string_view arg",
                               e->getExprLoc())
                            << e->getSourceRange();
                }
                else if (tc2.Class("basic_string").StdNamespace())
                {
                    if (loplugin::DeclCheck(e->getMethodDecl()).Function("c_str"))
                        report(DiagnosticsEngine::Warning,
                               "unnecessary call to 'c_str' when passing to string_view arg",
                               e->getExprLoc())
                            << e->getSourceRange();
                }
            }
        }
        return true;
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

private:
    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }
};

loplugin::Plugin::Registration<UnnecessaryGetStr> unnecessarygetstr("unnecessarygetstr");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
