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
                    DiagnosticsEngine::Warning, "'&' followed by '->', rather use '.'",
                    compat::getBeginLoc(memberExpr))
                    << memberExpr->getSourceRange();

        }
        else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(base))
        {
            if (operatorCallExpr->getOperator() == OO_Amp)
                report(
                    DiagnosticsEngine::Warning, "'&' followed by '->', rather use '.'",
                    compat::getBeginLoc(memberExpr))
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
    auto innerOp = dyn_cast<UnaryOperator>(unaryOperator->getSubExpr()->IgnoreParenImpCasts());
    if (!innerOp || innerOp->getOpcode() != UO_AddrOf)
        return true;

    report(
        DiagnosticsEngine::Warning, "'&' followed by '*', rather use '.'",
        compat::getBeginLoc(unaryOperator))
        << unaryOperator->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< RedundantPointerOps > redundantpointerops("redundantpointerops");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
