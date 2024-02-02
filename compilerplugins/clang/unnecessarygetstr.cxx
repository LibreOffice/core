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
#include <unordered_set>

#include "check.hxx"
#include "plugin.hxx"
#include "config_clang.h"

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

    bool VisitCallExpr(const CallExpr* callExpr)
    {
        if (ignoreLocation(callExpr))
            return true;
        const FunctionDecl* func = callExpr->getDirectCallee();
        if (!func)
            return true;
        if (loplugin::DeclCheck(func)
                .Function("createFromAscii")
                .Class("OUString")
                .Namespace("rtl")
                .GlobalNamespace())
        {
            checkForGetStr(callExpr->getArg(0), "OUString::createFromAscii",
                           /*isOStringConstructor*/ false);
        }
        return true;
    }

    bool VisitCXXConstructExpr(const CXXConstructExpr* constructExpr)
    {
        if (ignoreLocation(constructExpr))
            return true;
        auto tc = loplugin::TypeCheck(constructExpr->getType());
        if (tc.ClassOrStruct("basic_stringstream").StdNamespace())
        {
            // ignore the implicit-conversion nodes that are added here
            if (constructExpr->getNumArgs() > 0)
                nodesToIgnore.insert(constructExpr->getArg(0)->IgnoreImplicit());
        }
        else if (tc.ClassOrStruct("basic_string").StdNamespace())
        {
            if (constructExpr->getNumArgs() == 1 || constructExpr->getNumArgs() == 2)
            {
                if (nodesToIgnore.find(constructExpr) == nodesToIgnore.end())
                    checkForGetStr(constructExpr->getArg(0), "string constructor",
                                   /*isOStringConstructor*/ false);
            }
        }
        else if (tc.ClassOrStruct("basic_string_view").StdNamespace())
        {
            if (constructExpr->getNumArgs() == 1)
                checkForGetStr(constructExpr->getArg(0), "string_view constructor",
                               /*isOStringConstructor*/ false);
        }
        else if (tc.Class("OString").Namespace("rtl").GlobalNamespace())
        {
            if (constructExpr->getNumArgs() == 1 || constructExpr->getNumArgs() == 2)
                checkForGetStr(constructExpr->getArg(0), "OString constructor",
                               /*isOStringConstructor*/ true);
        }
        else if (tc.Class("OUString").Namespace("rtl").GlobalNamespace())
        {
            if (constructExpr->getNumArgs() == 2)
                checkForGetStr(constructExpr->getArg(0), "OUString constructor",
                               /*isOStringConstructor*/ false);
        }
        return true;
    }

    bool preRun() override
    {
        if (!compiler.getLangOpts().CPlusPlus)
            return false;
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/"))
            return false;
        return true;
    }

private:
    void checkForGetStr(const Expr* arg, const char* msg, bool isOStringConstructor)
    {
        auto e = dyn_cast<CXXMemberCallExpr>(arg->IgnoreImplicit());
        if (!e)
            return;
        auto const t = e->getObjectType();
        auto const tc2 = loplugin::TypeCheck(t);
        if (tc2.Class("OString").Namespace("rtl").GlobalNamespace()
            || tc2.Class("OUString").Namespace("rtl").GlobalNamespace()
            || tc2.Class("OStringBuffer").Namespace("rtl").GlobalNamespace()
            || tc2.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
            || tc2.ClassOrStruct("StringNumber").Namespace("rtl").GlobalNamespace())
        {
            if (loplugin::DeclCheck(e->getMethodDecl()).Function("getStr"))
            {
                StringRef fileName = getFilenameOfLocation(
                    compiler.getSourceManager().getSpellingLoc(e->getBeginLoc()));
                if (!loplugin::hasPathnamePrefix(fileName, SRCDIR "/include/rtl/"))
                    report(DiagnosticsEngine::Warning,
                           "unnecessary call to 'getStr' when passing to %0", e->getExprLoc())
                        << msg << e->getSourceRange();
            }
        }
        // we do need to use c_str() when passing to an OString
        else if (!isOStringConstructor && tc2.Class("basic_string").StdNamespace())
        {
            if (loplugin::DeclCheck(e->getMethodDecl()).Function("c_str"))
                report(DiagnosticsEngine::Warning, "unnecessary call to 'c_str' when passing to %0",
                       e->getExprLoc())
                    << msg << e->getSourceRange();
        }
    }
    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    std::unordered_set<const Expr*> nodesToIgnore;
};

loplugin::Plugin::Registration<UnnecessaryGetStr> unnecessarygetstr("unnecessarygetstr");
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
