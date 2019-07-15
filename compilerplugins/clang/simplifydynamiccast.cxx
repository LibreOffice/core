/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include <clang/AST/CXXInheritance.h>
#include "compat.hxx"
#include "plugin.hxx"

namespace
{
class SimplifyDynamicCast : public loplugin::FilteringPlugin<SimplifyDynamicCast>
{
public:
    explicit SimplifyDynamicCast(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        //        StringRef fn(handler.getMainFileName());
        //        if (loplugin::isSamePathname(fn, WORKDIR "/YaccTarget/unoidl/source/sourceprovider-parser.cxx"))
        //             return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseIfStmt(IfStmt*);
    bool VisitCXXStaticCastExpr(CXXStaticCastExpr const*);

private:
    std::vector<QualType> dynamicCastVec;
    std::vector<Decl const*> dynamicCastSubExprVec;
    std::vector<IfStmt const*> ifVec;
};

bool SimplifyDynamicCast::TraverseIfStmt(IfStmt* ifStmt)
{
    auto condExpr = ifStmt->getCond()->IgnoreParenImpCasts();
    auto dynamicCastExpr = dyn_cast<CXXDynamicCastExpr>(condExpr);
    if (!dynamicCastExpr)
    {
        if (auto binaryOp = dyn_cast<BinaryOperator>(condExpr))
        {
            if (binaryOp->getOpcode() == BO_NE)
                dynamicCastExpr
                    = dyn_cast<CXXDynamicCastExpr>(binaryOp->getLHS()->IgnoreParenImpCasts());
        }
    }
    Decl const* subExprDecl = nullptr;
    if (dynamicCastExpr)
    {
        auto subExprDeclRefExpr
            = dyn_cast<DeclRefExpr>(dynamicCastExpr->getSubExpr()->IgnoreParenImpCasts());
        if (!subExprDeclRefExpr)
            dynamicCastExpr = nullptr;
        else
            subExprDecl = subExprDeclRefExpr->getDecl();
    }
    if (dynamicCastExpr)
    {
        auto qt = dynamicCastExpr->getTypeAsWritten();
        dynamicCastVec.push_back(qt);
        dynamicCastSubExprVec.push_back(subExprDecl);
        ifVec.push_back(ifStmt);
    }
    bool ret = RecursiveASTVisitor::TraverseIfStmt(ifStmt);
    if (dynamicCastExpr)
    {
        dynamicCastVec.pop_back();
        dynamicCastSubExprVec.pop_back();
        ifVec.pop_back();
    }
    return ret;
}

bool SimplifyDynamicCast::VisitCXXStaticCastExpr(CXXStaticCastExpr const* staticCastExpr)
{
    if (ignoreLocation(staticCastExpr))
        return true;
    if (dynamicCastVec.empty())
        return true;

    auto qt = staticCastExpr->getTypeAsWritten();
    auto it = std::find(dynamicCastVec.begin(), dynamicCastVec.end(), qt);
    if (it == dynamicCastVec.end())
        return true;
    int idx = it - dynamicCastVec.begin();
    auto subExprDecl = dyn_cast<DeclRefExpr>(staticCastExpr->getSubExpr()->IgnoreParenImpCasts());
    if (!subExprDecl)
        return true;
    if (dynamicCastSubExprVec[idx] != subExprDecl->getDecl())
        return true;
    report(DiagnosticsEngine::Warning, "simplify, use var in if",
           compat::getBeginLoc(staticCastExpr))
        << staticCastExpr->getSourceRange();
    auto ifStmt = ifVec[idx];
    report(DiagnosticsEngine::Note, "if here", compat::getBeginLoc(ifStmt))
        << ifStmt->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<SimplifyDynamicCast> X("simplifydynamiccast", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
