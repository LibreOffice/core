/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "compat.hxx"
#include "config_clang.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/**
Look for places where we are assigning a temporary O[U]String to a std::*string_view, which leads
to a view pointing to freed memory.
*/

namespace
{
class StringViewDangle : public loplugin::FilteringPlugin<StringViewDangle>
{
public:
    explicit StringViewDangle(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override { return true; }

    virtual void run() override
    {
        if (!preRun())
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const*);
    bool VisitVarDecl(VarDecl const*);
};

static const Expr* IgnoreImplicitAndConversionOperator(const Expr* expr)
{
    expr = expr->IgnoreImplicit();
    if (auto memberCall = dyn_cast<CXXMemberCallExpr>(expr))
    {
        if (auto conversionDecl = dyn_cast_or_null<CXXConversionDecl>(memberCall->getMethodDecl()))
        {
            if (!conversionDecl->isExplicit())
                expr = memberCall->getImplicitObjectArgument()->IgnoreImpCasts();
        }
    }
    return expr;
}

bool StringViewDangle::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* cxxOperatorCallExpr)
{
    if (ignoreLocation(cxxOperatorCallExpr))
        return true;

    auto op = cxxOperatorCallExpr->getOperator();
    if (op != OO_Equal)
        return true;
    if (!loplugin::TypeCheck(cxxOperatorCallExpr->getType())
             .ClassOrStruct("basic_string_view")
             .StdNamespace())
        return true;
    auto expr = IgnoreImplicitAndConversionOperator(cxxOperatorCallExpr->getArg(1));
    auto tc = loplugin::TypeCheck(expr->getType());
    if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
        return true;
    if (!isa<MaterializeTemporaryExpr>(expr))
        return true;
    report(DiagnosticsEngine::Warning, "view pointing into temporary i.e. dangling",
           cxxOperatorCallExpr->getExprLoc())
        << cxxOperatorCallExpr->getSourceRange();
    return true;
}

bool StringViewDangle::VisitVarDecl(VarDecl const* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    if (!loplugin::TypeCheck(varDecl->getType()).ClassOrStruct("basic_string_view").StdNamespace())
        return true;
    if (!varDecl->hasInit())
        return true;
    auto expr = IgnoreImplicitAndConversionOperator(varDecl->getInit());
    auto tc = loplugin::TypeCheck(expr->getType());
    if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
        return true;
    if (!isa<MaterializeTemporaryExpr>(expr))
        return true;
    report(DiagnosticsEngine::Warning, "view pointing into temporary i.e. dangling",
           varDecl->getLocation())
        << varDecl->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<StringViewDangle> stringviewdangle("stringviewdangle");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
