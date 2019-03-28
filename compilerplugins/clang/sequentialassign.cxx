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
#include <unordered_map>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/**
    This is a kind of simplified dead-store analysis.

    We are looking for patterns like:
      Foo x = a;
      x = b;
    which can be simplified to
        x = b

    or
        Foo x = a;
        x = f(x)
    which can be simplified to
        Foo x = f(a)

   TODO Improve this plugin to make it safer. We should really be checking the following
   conditions inside the RHS of the second statement:
     If the variable is having it's address taken, or a non-const method called on it,
     on passed by non-const-ref.
*/

namespace
{
//static bool startswith(const std::string& rStr, const char* pSubStr) {
//    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
//}
class SequentialAssign : public loplugin::FilteringPlugin<SequentialAssign>
{
public:
    explicit SequentialAssign(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCompoundStmt(CompoundStmt const*);

private:
    const VarDecl* findSimpleAssign(Stmt const*);
    const VarDecl* findSimpleVarDecl(Expr const*);
    Expr const* ignore(Expr const*);
    void checkForSecondAssign(Stmt const* stmt, VarDecl const* varDecl);
};

bool SequentialAssign::VisitCompoundStmt(CompoundStmt const* compoundStmt)
{
    if (ignoreLocation(compoundStmt))
        return true;

    auto it = compoundStmt->body_begin();
    while (true)
    {
        if (it == compoundStmt->body_end())
            break;
        auto stmt = *it;
        const VarDecl* foundVars = findSimpleAssign(stmt);
        if (!foundVars)
        {
            ++it;
            continue;
        }
        ++it;
        if (it == compoundStmt->body_end())
            break;
        checkForSecondAssign(*it, foundVars);
    }

    return true;
}

void SequentialAssign::checkForSecondAssign(Stmt const* stmt, VarDecl const* varDecl)
{
    if (auto exprCleanup = dyn_cast<ExprWithCleanups>(stmt))
        stmt = exprCleanup->getSubExpr();

    if (auto operatorCall = dyn_cast<CXXOperatorCallExpr>(stmt))
    {
        if (operatorCall->getOperator() == OO_Equal)
        {
            if (auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(operatorCall->getArg(0))))
                if (declRefExprLHS->getDecl() == varDecl)
                {
                    report(DiagnosticsEngine::Warning,
                           "simplify by merging with the preceding assignment",
                           compat::getBeginLoc(stmt))
                        << stmt->getSourceRange();
                }
        }
        return;
    }

    if (auto binaryOp = dyn_cast<BinaryOperator>(stmt))
    {
        if (binaryOp->getOpcode() == BO_Assign)
        {
            if (auto declRefExpr = dyn_cast<DeclRefExpr>(ignore(binaryOp->getLHS())))
                if (declRefExpr->getDecl() == varDecl)
                {
                    report(DiagnosticsEngine::Warning,
                           "simplify by merging with the preceding assignment",
                           compat::getBeginLoc(stmt))
                        << stmt->getSourceRange();
                }
        }
    }
}

const VarDecl* SequentialAssign::findSimpleAssign(Stmt const* stmt)
{
    if (auto declStmt = dyn_cast<DeclStmt>(stmt))
        if (declStmt->isSingleDecl())
            if (auto varDeclLHS = dyn_cast_or_null<VarDecl>(declStmt->getSingleDecl()))
                if (auto init = varDeclLHS->getInit())
                    if (findSimpleVarDecl(init))
                        return varDeclLHS;
    if (auto operatorCall = dyn_cast<CXXOperatorCallExpr>(stmt))
        if (operatorCall->getOperator() == OO_Equal)
            if (auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(operatorCall->getArg(0))))
                if (auto varDeclLHS = dyn_cast<VarDecl>(declRefExprLHS->getDecl()))
                    if (findSimpleVarDecl(operatorCall->getArg(1)))
                        return varDeclLHS;
    if (auto binaryOp = dyn_cast<BinaryOperator>(stmt))
        if (binaryOp->getOpcode() == BO_Assign)
            if (auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(binaryOp->getLHS())))
                if (auto varDeclLHS = dyn_cast<VarDecl>(declRefExprLHS->getDecl()))
                    if (findSimpleVarDecl(binaryOp->getRHS()))
                        return varDeclLHS;
    return nullptr;
}

const VarDecl* SequentialAssign::findSimpleVarDecl(Expr const* expr)
{
    expr = ignore(expr);
    if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(expr))
    {
        if (cxxConstructExpr->getNumArgs() == 1)
        {
            expr = cxxConstructExpr->getArg(0);
            expr = expr->IgnoreParens()->IgnoreImplicit()->IgnoreParens();
        }
    }
    if (auto declRefExpr = dyn_cast<DeclRefExpr>(expr))
        if (auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl()))
            return varDecl;
    return nullptr;
}

Expr const* SequentialAssign::ignore(Expr const* expr)
{
    return expr->IgnoreImplicit()->IgnoreParens()->IgnoreImplicit();
}

// Off by default because of safety concerns, see TODO at top
loplugin::Plugin::Registration<SequentialAssign> X("sequentialassign", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
