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
#include "compat.hxx"
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
    //functionDecl->dump();
    return true;
}

bool RedundantPointerOps::VisitMemberExpr(MemberExpr const * memberExpr)
{
    if (ignoreLocation(memberExpr))
        return true;
    if (compat::getBeginLoc(memberExpr).isMacroID())
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
                    compat::getBeginLoc(memberExpr))
                    << memberExpr->getBase()->getType()->getPointeeType()
                    << memberExpr->getSourceRange();

        }
        else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(base))
        {
            if (operatorCallExpr->getOperator() == OO_Amp)
                report(
                    DiagnosticsEngine::Warning,
                    "'&' followed by '->' operating on %0, rather use '.'",
                    compat::getBeginLoc(memberExpr))
                    << memberExpr->getBase()->getType()->getPointeeType()
                    << memberExpr->getSourceRange();

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
    if (compat::getBeginLoc(unaryOperator).isMacroID())
        return true;
    if (unaryOperator->getOpcode() != UO_Deref)
        return true;
    auto subExpr = unaryOperator->getSubExpr()->IgnoreParenImpCasts();
    auto innerOp = dyn_cast<UnaryOperator>(subExpr);
    if (innerOp && innerOp->getOpcode() == UO_AddrOf)
        report(
            DiagnosticsEngine::Warning, "'&' followed by '*' operating on %0, rather use '.'",
            compat::getBeginLoc(unaryOperator))
            << innerOp->getSubExpr()->getType() << unaryOperator->getSourceRange();
    if (auto cxxMemberCallExpr = dyn_cast<CXXMemberCallExpr>(subExpr))
    {
        auto methodDecl = cxxMemberCallExpr->getMethodDecl();
        if (methodDecl->getIdentifier() && methodDecl->getName() == "get")
        {
            auto const e = cxxMemberCallExpr->getImplicitObjectArgument();
            // First check the object type as written, in case the get member function is
            // declared at a base class of std::unique_ptr or std::shared_ptr:
            auto const t = e->IgnoreImpCasts()->getType();
            auto const tc1 = loplugin::TypeCheck(t);
            if (!(tc1.ClassOrStruct("unique_ptr").StdNamespace()
                  || tc1.ClassOrStruct("shared_ptr").StdNamespace()))
            {
                // Then check the object type coerced to the type of the get member function, in
                // case the type-as-written is derived from one of these types (tools::SvRef is
                // final, but the rest are not; but note that this will fail when the type-as-
                // written is derived from std::unique_ptr or std::shared_ptr for which the get
                // member function is declared at a base class):
                auto const tc2 = loplugin::TypeCheck(e->getType());
                if (!((tc2.ClassOrStruct("unique_ptr").StdNamespace()
                       || tc2.ClassOrStruct("shared_ptr").StdNamespace()
                       || (tc2.Class("Reference").Namespace("uno").Namespace("star")
                           .Namespace("sun").Namespace("com").GlobalNamespace())
                       || tc2.Class("Reference").Namespace("rtl").GlobalNamespace()
                       || tc2.Class("SvRef").Namespace("tools").GlobalNamespace())))
                {
                    return true;
                }
            }
            report(
                DiagnosticsEngine::Warning,
                "'*' followed by '.get()' operating on %0, just use '*'",
                compat::getBeginLoc(unaryOperator))
                << t.getLocalUnqualifiedType() << unaryOperator->getSourceRange();

        }
    }
    return true;
}

loplugin::Plugin::Registration< RedundantPointerOps > redundantpointerops("redundantpointerops");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
