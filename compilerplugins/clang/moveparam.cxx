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

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <unordered_set>
#include "plugin.hxx"
#include "check.hxx"

/*
Look for places where we can pass by Primitive2DContainer param and so avoid
unnecessary copies.
*/

namespace
{
class MoveParam : public loplugin::FilteringPlugin<MoveParam>
{
public:
    explicit MoveParam(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override { return true; }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool PreTraverseConstructorInitializer(CXXCtorInitializer*);
    bool PostTraverseConstructorInitializer(CXXCtorInitializer*, bool);
    bool TraverseConstructorInitializer(CXXCtorInitializer*);
    bool VisitCXXOperatorCallExpr(const CXXOperatorCallExpr*);
};

bool MoveParam::VisitCXXOperatorCallExpr(const CXXOperatorCallExpr* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    if (!callExpr->isAssignmentOp())
        return true;
    if (!loplugin::TypeCheck(callExpr->getType())
             .Class("Primitive2DContainer")
             .Namespace("primitive2d"))
        return true;
    auto declRef = dyn_cast<DeclRefExpr>(callExpr->getArg(1)->IgnoreParenImpCasts());
    if (!declRef)
        return true;

    auto parmVarDecl = dyn_cast_or_null<ParmVarDecl>(declRef->getDecl());
    if (!parmVarDecl)
        return true;

    if (!loplugin::TypeCheck(parmVarDecl->getType()).LvalueReference().Const())
        return true;

    report(DiagnosticsEngine::Warning, "rather use move && param", compat::getBeginLoc(callExpr));

    return true;
}

bool MoveParam::PreTraverseConstructorInitializer(CXXCtorInitializer* init)
{
    if (ignoreLocation(init->getSourceLocation()))
        return true;
    const FieldDecl* fieldDecl = init->getAnyMember();
    if (!fieldDecl)
        return true;

    auto dc = loplugin::TypeCheck(fieldDecl->getType())
                  .Class("Primitive2DContainer")
                  .Namespace("primitive2d")
                  .Namespace("drawinglayer")
                  .GlobalNamespace();
    if (!dc)
        return true;

    auto constructExpr = dyn_cast_or_null<CXXConstructExpr>(init->getInit());
    if (!constructExpr || constructExpr->getNumArgs() != 1)
        return true;

    auto declRef = dyn_cast<DeclRefExpr>(constructExpr->getArg(0)->IgnoreParenImpCasts());
    if (!declRef)
        return true;

    auto parmVarDecl = dyn_cast_or_null<ParmVarDecl>(declRef->getDecl());
    if (!parmVarDecl)
        return true;

    if (!loplugin::TypeCheck(parmVarDecl->getType()).LvalueReference().Const())
        return true;

    report(DiagnosticsEngine::Warning, "rather use move && param", init->getSourceLocation());

    return true;
}
bool MoveParam::PostTraverseConstructorInitializer(CXXCtorInitializer*, bool) { return true; }
bool MoveParam::TraverseConstructorInitializer(CXXCtorInitializer* init)
{
    bool ret = true;
    if (PreTraverseConstructorInitializer(init))
    {
        ret = FilteringPlugin<MoveParam>::TraverseConstructorInitializer(init);
        PostTraverseConstructorInitializer(init, ret);
    }
    return ret;
}

loplugin::Plugin::Registration<MoveParam> moveparam("moveparam", true);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
