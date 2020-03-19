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
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

// This checker aims to pull buried assignments out of complex expressions,
// where they are quite hard to notice amidst the other conditional logic.

namespace
{
class BuriedAssign : public loplugin::FilteringPlugin<BuriedAssign>
{
public:
    explicit BuriedAssign(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);

        // code where I don't have a better alternative
        if (fn == SRCDIR "/sal/osl/unx/profile.cxx")
            return;
        if (fn == SRCDIR "/sal/rtl/uri.cxx")
            return;
        if (fn == SRCDIR "/sal/osl/unx/process.cxx")
            return;
        if (fn == SRCDIR "/sal/rtl/bootstrap.cxx")
            return;
        if (fn == SRCDIR "/i18npool/source/textconversion/genconv_dict.cxx")
            return;
        if (fn == SRCDIR "/soltools/cpp/_lex.c")
            return;
        if (fn == SRCDIR "/soltools/cpp/_macro.c")
            return;
        if (fn == SRCDIR "/stoc/source/inspect/introspection.cxx")
            return;
        if (fn == SRCDIR "/tools/source/fsys/urlobj.cxx")
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(BinaryOperator const*);
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const*);
    bool VisitCompoundStmt(CompoundStmt const*);
    bool VisitIfStmt(IfStmt const*);
    bool VisitLabelStmt(LabelStmt const*);
    bool VisitForStmt(ForStmt const*);
    bool VisitCXXForRangeStmt(CXXForRangeStmt const*);
    bool VisitWhileStmt(WhileStmt const*);
    bool VisitCaseStmt(CaseStmt const*);
    bool VisitDefaultStmt(DefaultStmt const*);
    bool VisitVarDecl(VarDecl const*);
    bool VisitCXXFoldExpr(CXXFoldExpr const*);

private:
    void MarkChildrenAsHandled(Stmt const* stmt);
    void MarkAsHandled(Stmt const* stmt);
    void MarkParenAssign(Expr const*);

    std::unordered_set<const Stmt*> m_handled;
};

static bool isAssignmentOp(clang::BinaryOperatorKind op)
{
    // We ignore BO_ShrAssign i.e. >>= because we use that everywhere for
    // extracting data from css::uno::Any
    return op == BO_Assign || op == BO_MulAssign || op == BO_DivAssign || op == BO_RemAssign
           || op == BO_AddAssign || op == BO_SubAssign || op == BO_ShlAssign || op == BO_AndAssign
           || op == BO_XorAssign || op == BO_OrAssign;
}

static bool isAssignmentOp(clang::OverloadedOperatorKind Opc)
{
    // Same logic as CXXOperatorCallExpr::isAssignmentOp(), which our supported clang
    // doesn't have yet.
    // Except that we ignore OO_GreaterGreaterEqual i.e. >>= because we use that everywhere for
    // extracting data from css::uno::Any
    return Opc == OO_Equal || Opc == OO_StarEqual || Opc == OO_SlashEqual || Opc == OO_PercentEqual
           || Opc == OO_PlusEqual || Opc == OO_MinusEqual || Opc == OO_LessLessEqual
           || Opc == OO_AmpEqual || Opc == OO_CaretEqual || Opc == OO_PipeEqual;
}

bool BuriedAssign::VisitBinaryOperator(BinaryOperator const* binaryOp)
{
    if (ignoreLocation(binaryOp))
        return true;
    if (binaryOp->getBeginLoc().isMacroID())
        return true;
    if (!isAssignmentOp(binaryOp->getOpcode()))
        return true;
    if (auto rhs = dyn_cast<BinaryOperator>(compat::IgnoreImplicit(binaryOp->getRHS())))
    {
        // Ignore chained assignment.
        // TODO limit this to only ordinary assignment
        if (isAssignmentOp(binaryOp->getOpcode()) && isAssignmentOp(rhs->getOpcode()))
            m_handled.insert(rhs);
    }
    if (!m_handled.insert(binaryOp).second)
        return true;
    report(DiagnosticsEngine::Warning, "buried assignment, rather put on own line",
           compat::getBeginLoc(binaryOp))
        << binaryOp->getSourceRange();
    //getParentStmt(getParentStmt(binaryOp))->dump();
    return true;
}

bool BuriedAssign::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* cxxOper)
{
    if (ignoreLocation(cxxOper))
        return true;
    if (cxxOper->getBeginLoc().isMacroID())
        return true;
    if (!isAssignmentOp(cxxOper->getOperator()))
        return true;
    if (auto rhs = dyn_cast<CXXOperatorCallExpr>(compat::IgnoreImplicit(cxxOper->getArg(1))))
    {
        // Ignore chained assignment.
        // TODO limit this to only ordinary assignment
        if (isAssignmentOp(cxxOper->getOperator()) && isAssignmentOp(rhs->getOperator()))
            m_handled.insert(rhs);
    }
    if (!m_handled.insert(cxxOper).second)
        return true;
    report(DiagnosticsEngine::Warning, "buried assignment, rather put on own line",
           compat::getBeginLoc(cxxOper))
        << cxxOper->getSourceRange();
    //getParentStmt(getParentStmt(getParentStmt(getParentStmt(cxxOper))))->dump();
    return true;
}

bool BuriedAssign::VisitCompoundStmt(CompoundStmt const* compoundStmt)
{
    MarkChildrenAsHandled(compoundStmt);
    return true;
}

void BuriedAssign::MarkChildrenAsHandled(Stmt const* stmt)
{
    if (!stmt)
        return;
    if (ignoreLocation(stmt))
        return;
    for (auto i = stmt->child_begin(); i != stmt->child_end(); ++i)
        MarkAsHandled(*i);
}

void BuriedAssign::MarkAsHandled(Stmt const* stmt)
{
    if (!stmt)
        return;
    if (auto expr = dyn_cast<Expr>(stmt))
    {
        expr = compat::IgnoreImplicit(expr);
        if (auto binaryOp = dyn_cast<BinaryOperator>(expr))
            if (binaryOp->getOpcode() == BO_Comma)
            {
                MarkAsHandled(binaryOp->getLHS());
                MarkAsHandled(binaryOp->getRHS());
            }
        m_handled.insert(expr);
    }
}

bool BuriedAssign::VisitIfStmt(IfStmt const* ifStmt)
{
    if (ignoreLocation(ifStmt))
        return true;
    auto cond = compat::IgnoreImplicit(ifStmt->getCond());
    if (auto binaryOp = dyn_cast<BinaryOperator>(cond))
    {
        MarkAsHandled(compat::IgnoreImplicit(binaryOp->getLHS())->IgnoreParens());
        MarkAsHandled(compat::IgnoreImplicit(binaryOp->getRHS())->IgnoreParens());
    }
    else if (auto parenExpr = dyn_cast<ParenExpr>(cond))
    {
        MarkAsHandled(compat::IgnoreImplicit(parenExpr->getSubExpr()));
    }
    else if (auto unaryOp = dyn_cast<UnaryOperator>(cond))
    {
        MarkAsHandled(compat::IgnoreImplicit(unaryOp->getSubExpr())->IgnoreParens());
    }
    MarkAsHandled(ifStmt->getThen());
    MarkAsHandled(ifStmt->getElse());
    return true;
}

bool BuriedAssign::VisitCaseStmt(CaseStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkAsHandled(stmt->getSubStmt());
    return true;
}

bool BuriedAssign::VisitDefaultStmt(DefaultStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkAsHandled(stmt->getSubStmt());
    return true;
}

bool BuriedAssign::VisitWhileStmt(WhileStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkParenAssign(stmt->getCond());
    MarkAsHandled(stmt->getBody());
    return true;
}

/** stuff like
 *    while ((x = foo())
 * is considered idiomatic.
 */
void BuriedAssign::MarkParenAssign(Expr const* expr)
{
    if (!expr)
        return;
    expr = compat::IgnoreImplicit(expr);
    if (auto parenExpr = dyn_cast<ParenExpr>(expr))
    {
        expr = compat::IgnoreImplicit(parenExpr->getSubExpr());
        if (auto binaryOp = dyn_cast<BinaryOperator>(expr))
        {
            if (binaryOp->getOpcode() == BO_Assign)
                m_handled.insert(binaryOp);
        }
        else if (auto cxxOper = dyn_cast<CXXOperatorCallExpr>(expr))
        {
            if (cxxOper->getOperator() == OO_Equal)
                m_handled.insert(cxxOper);
        }
    }
    else if (auto binaryOp = dyn_cast<BinaryOperator>(expr))
    {
        MarkAsHandled(compat::IgnoreImplicit(binaryOp->getLHS())->IgnoreParens());
        MarkAsHandled(compat::IgnoreImplicit(binaryOp->getRHS())->IgnoreParens());
    }
}

bool BuriedAssign::VisitForStmt(ForStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkAsHandled(stmt->getInit());
    MarkAsHandled(stmt->getInc());
    MarkAsHandled(stmt->getBody());
    return true;
}

bool BuriedAssign::VisitCXXForRangeStmt(CXXForRangeStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkAsHandled(stmt->getBody());
    return true;
}

bool BuriedAssign::VisitLabelStmt(LabelStmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkAsHandled(stmt->getSubStmt());
    return true;
}

bool BuriedAssign::VisitVarDecl(VarDecl const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    if (stmt->getInit())
    {
        if (auto binaryOp = dyn_cast<BinaryOperator>(compat::IgnoreImplicit(stmt->getInit())))
        {
            if (binaryOp->getOpcode() == BO_Assign)
                m_handled.insert(binaryOp);
        }
        else if (auto cxxConstruct
                 = dyn_cast<CXXConstructExpr>(compat::IgnoreImplicit(stmt->getInit())))
        {
            if (cxxConstruct->getNumArgs() == 1)
                if (auto cxxOper = dyn_cast<CXXOperatorCallExpr>(
                        compat::IgnoreImplicit(cxxConstruct->getArg(0))))
                    if (cxxOper->getOperator() == OO_Equal)
                        m_handled.insert(cxxOper);
        }
    }
    return true;
}

bool BuriedAssign::VisitCXXFoldExpr(CXXFoldExpr const* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    MarkParenAssign(stmt->getLHS());
    MarkParenAssign(stmt->getRHS());
    return true;
}

// off by default because it uses getParentStmt so it's more expensive to run
loplugin::Plugin::Registration<BuriedAssign> X("buriedassign", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
