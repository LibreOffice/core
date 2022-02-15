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
#include <fstream>
#include <set>

#include <clang/AST/CXXInheritance.h>

#include "check.hxx"
#include "plugin.hxx"

/**
 * Look for:
 *     (&x)->y
 * which can be transformed to:
 *      x.y
 * And
 *    &*x
 * which can be:
 *    x
 *
 * @TODO
 *    (*x).y
 *  which can be:
 *    x->y
 */

namespace {

class RedundantPointerOps:
    public loplugin::FilteringPlugin<RedundantPointerOps>
{
public:
    explicit RedundantPointerOps(loplugin::InstantiationData const & data): FilteringPlugin(data) {}

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFunctionDecl(FunctionDecl const *);
    bool VisitMemberExpr(MemberExpr const *);
    bool VisitUnaryOperator(UnaryOperator const *);
};

bool RedundantPointerOps::VisitFunctionDecl(FunctionDecl const * functionDecl)
{
    if (ignoreLocation(functionDecl))
        return true;
//    if (functionDecl->getIdentifier() && functionDecl->getName() == "function6b")
//        functionDecl->dump();
    return true;
}

bool RedundantPointerOps::VisitMemberExpr(MemberExpr const * memberExpr)
{
    if (ignoreLocation(memberExpr))
        return true;
    if (memberExpr->getBeginLoc().isMacroID())
        return true;
    auto base = memberExpr->getBase()->IgnoreParenImpCasts();
            //parentStmt(parentStmt(memberExpr))->dump();
    if (memberExpr->isArrow())
    {
        if (auto unaryOp = dyn_cast<UnaryOperator>(base))
        {
            if (unaryOp->getOpcode() == UO_AddrOf)
                report(
                    DiagnosticsEngine::Warning,
                    "'&' followed by '->' operating on %0, rather use '.'",
                    memberExpr->getBeginLoc())
                    << memberExpr->getBase()->getType()->getPointeeType()
                    << memberExpr->getSourceRange();

        }
        else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(base))
        {
            if (operatorCallExpr->getOperator() == OO_Amp)
                report(
                    DiagnosticsEngine::Warning,
                    "'&' followed by '->' operating on %0, rather use '.'",
                    memberExpr->getBeginLoc())
                    << memberExpr->getBase()->getType()->getPointeeType()
                    << memberExpr->getSourceRange();

        }
        else if (auto cxxMemberCallExpr = dyn_cast<CXXMemberCallExpr>(base))
        {
            auto methodDecl = cxxMemberCallExpr->getMethodDecl();
            if (methodDecl->getIdentifier() && methodDecl->getName() == "get")
            {
                auto const e = cxxMemberCallExpr->getImplicitObjectArgument();
                if (loplugin::isSmartPointerType(e))
                    report(
                        DiagnosticsEngine::Warning,
                        "'get()' followed by '->' operating on %0, just use '->'",
                        memberExpr->getBeginLoc())
                        << e->IgnoreImpCasts()->getType().getLocalUnqualifiedType()
                        << memberExpr->getSourceRange();
            }
        }
    }
//    else
//    {
//        if (auto unaryOp = dyn_cast<UnaryOperator>(base))
//        {
//            if (unaryOp->getOpcode() == UO_Deref)
//                report(
//                    DiagnosticsEngine::Warning, "'*' followed by '.', rather use '->'",
//                    memberExpr->getLocStart())
//                    << memberExpr->getSourceRange();
//
//        }
//    }
    return true;
}

bool RedundantPointerOps::VisitUnaryOperator(UnaryOperator const * unaryOperator)
{
    if (ignoreLocation(unaryOperator))
        return true;
    if (unaryOperator->getBeginLoc().isMacroID())
        return true;
    if (unaryOperator->getOpcode() != UO_Deref)
        return true;
    auto subExpr = unaryOperator->getSubExpr()->IgnoreParenImpCasts();
    auto innerOp = dyn_cast<UnaryOperator>(subExpr);
    if (innerOp && innerOp->getOpcode() == UO_AddrOf)
        report(
            DiagnosticsEngine::Warning, "'&' followed by '*' operating on %0, rather use '.'",
            unaryOperator->getBeginLoc())
            << innerOp->getSubExpr()->getType() << unaryOperator->getSourceRange();
    if (auto cxxMemberCallExpr = dyn_cast<CXXMemberCallExpr>(subExpr))
    {
        auto methodDecl = cxxMemberCallExpr->getMethodDecl();
        if (methodDecl->getIdentifier() && methodDecl->getName() == "get")
        {
            auto const e = cxxMemberCallExpr->getImplicitObjectArgument();
            if (loplugin::isSmartPointerType(e))
                report(
                    DiagnosticsEngine::Warning,
                    "'*' followed by '.get()' operating on %0, just use '*'",
                    unaryOperator->getBeginLoc())
                    << e->IgnoreImpCasts()->getType().getLocalUnqualifiedType()
                    << unaryOperator->getSourceRange();
        }
    }
    return true;
}

loplugin::Plugin::Registration< RedundantPointerOps > redundantpointerops("redundantpointerops");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
