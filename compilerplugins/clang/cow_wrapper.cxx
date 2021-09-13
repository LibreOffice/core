/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <unordered_set>
#include "plugin.hxx"
#include "check.hxx"

/*
Look for places where we are using cow_wrapper, but we are calling a const method on the impl object
with a non-const pointer, which means we will unnnecessarily trigger a copy.
*/

namespace
{
class Cow_Wrapper : public loplugin::FilteringPlugin<Cow_Wrapper>
{
public:
    explicit Cow_Wrapper(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override { return true; }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr*);
};

bool Cow_Wrapper::VisitCXXMemberCallExpr(const CXXMemberCallExpr* memberCallExpr)
{
    if (ignoreLocation(memberCallExpr))
        return true;
    auto methodDecl = memberCallExpr->getMethodDecl();
    if (!methodDecl || !methodDecl->isConst())
        return true;

    auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(
        compat::IgnoreImplicit(memberCallExpr->getImplicitObjectArgument()));
    if (!operatorCallExpr)
        return true;
    if (operatorCallExpr->getOperator() != OO_Arrow)
        return true;
    auto arrowMethodDecl = dyn_cast_or_null<CXXMethodDecl>(operatorCallExpr->getDirectCallee());
    if (!arrowMethodDecl)
        return true;
    if (arrowMethodDecl->isConst())
        return true;
    auto dc = loplugin::DeclCheck(arrowMethodDecl->getParent())
                  .Class("cow_wrapper")
                  .Namespace("o3tl")
                  .GlobalNamespace();
    if (!dc)
        return true;

    report(DiagnosticsEngine::Warning,
           "calling const method on o3tl::cow_wrapper impl class via non-const pointer, rather use "
           "std::as_const to prevent triggering an unnecessary copy",
           compat::getBeginLoc(memberCallExpr))
        << memberCallExpr->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<Cow_Wrapper> cow_wrapper("cow_wrapper", true);

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
