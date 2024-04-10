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
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "compat.hxx"
#include "config_clang.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/**
    Look for *StringBuffer append sequences which can be converted to *String + sequences.
*/

namespace
{
class BufferAdd : public loplugin::FilteringPlugin<BufferAdd>
{
public:
    explicit BufferAdd(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/rtl/oustring/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/rtl/oustringbuffer/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/rtl/strings/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/OStringBuffer/"))
            return false;
        // some false +
        if (loplugin::isSamePathname(fn, SRCDIR "/unoidl/source/sourcetreeprovider.cxx"))
            return false;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/sw/source/writerfilter/dmapper/StyleSheetTable.cxx"))
            return false;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/sw/source/writerfilter/dmapper/GraphicImport.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sdext/source/pdfimport/pdfparse/pdfparse.cxx"))
            return false;
        return true;
    }

    void postRun() override
    {
        for (auto const& pair : goodMap)
            if (!isa<ParmVarDecl>(pair.first) &&
                // reference types have slightly weird behaviour
                !pair.first->getType()->isReferenceType()
                && badMap.find(pair.first) == badMap.end())
                report(DiagnosticsEngine::Warning,
                       "convert this append sequence into a *String + sequence",
                       pair.first->getBeginLoc())
                    << pair.first->getSourceRange();
    }

    virtual void run() override
    {
        if (!preRun())
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        postRun();
    }

    bool VisitStmt(Stmt const*);
    bool VisitCallExpr(CallExpr const*);
    bool VisitCXXConstructExpr(CXXConstructExpr const*);
    bool VisitUnaryOperator(UnaryOperator const*);

private:
    void findBufferAssignOrAdd(const Stmt* parentStmt, Stmt const*);
    Expr const* ignore(Expr const*);
    bool isSideEffectFree(Expr const*);
    bool isMethodOkToMerge(CXXMemberCallExpr const*);
    void addToGoodMap(const VarDecl* varDecl, const Stmt* parentStmt);

    std::unordered_map<const VarDecl*, const Stmt*> goodMap;
    std::unordered_set<const VarDecl*> badMap;
};

bool BufferAdd::VisitStmt(Stmt const* stmt)
{
    if (ignoreLocation(stmt))
        return true;

    if (!isa<CompoundStmt>(stmt) && !isa<CXXCatchStmt>(stmt) && !isa<CXXForRangeStmt>(stmt)
        && !isa<CXXTryStmt>(stmt) && !isa<DoStmt>(stmt) && !isa<ForStmt>(stmt) && !isa<IfStmt>(stmt)
        && !isa<SwitchStmt>(stmt) && !isa<WhileStmt>(stmt))
        return true;

    for (auto it = stmt->child_begin(); it != stmt->child_end(); ++it)
        if (*it)
            findBufferAssignOrAdd(stmt, *it);

    return true;
}

bool BufferAdd::VisitCallExpr(CallExpr const* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;

    for (unsigned i = 0; i != callExpr->getNumArgs(); ++i)
    {
        auto a = ignore(callExpr->getArg(i));
        if (auto declRefExpr = dyn_cast<DeclRefExpr>(a))
            if (auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl()))
                badMap.insert(varDecl);
    }
    return true;
}

bool BufferAdd::VisitCXXConstructExpr(CXXConstructExpr const* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;

    for (unsigned i = 0; i != callExpr->getNumArgs(); ++i)
    {
        auto a = ignore(callExpr->getArg(i));
        if (auto declRefExpr = dyn_cast<DeclRefExpr>(a))
            if (auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl()))
                badMap.insert(varDecl);
    }
    return true;
}

bool BufferAdd::VisitUnaryOperator(const UnaryOperator* unaryOp)
{
    if (ignoreLocation(unaryOp))
        return true;
    if (unaryOp->getOpcode() != UO_AddrOf)
        return true;
    auto a = ignore(unaryOp->getSubExpr());
    if (auto declRefExpr = dyn_cast<DeclRefExpr>(a))
        if (auto varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl()))
            badMap.insert(varDecl);
    return true;
}

void BufferAdd::findBufferAssignOrAdd(const Stmt* parentStmt, Stmt const* stmt)
{
    if (auto exprCleanup = dyn_cast<ExprWithCleanups>(stmt))
        stmt = exprCleanup->getSubExpr();
    if (auto switchCase = dyn_cast<SwitchCase>(stmt))
        stmt = switchCase->getSubStmt();
    if (auto declStmt = dyn_cast<DeclStmt>(stmt))
    {
        if (declStmt->isSingleDecl())
            if (auto varDeclLHS = dyn_cast_or_null<VarDecl>(declStmt->getSingleDecl()))
            {
                auto tc = loplugin::TypeCheck(varDeclLHS->getType());
                if (!tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
                    && !tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace())
                    return;
                if (varDeclLHS->getStorageDuration() == SD_Static)
                    return;
                if (!varDeclLHS->hasInit())
                    return;
                auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(ignore(varDeclLHS->getInit()));
                if (cxxConstructExpr)
                {
                    addToGoodMap(varDeclLHS, parentStmt);
                    return;
                }
                if (!isSideEffectFree(varDeclLHS->getInit()))
                    badMap.insert(varDeclLHS);
                else
                    addToGoodMap(varDeclLHS, parentStmt);
            }
        return;
    }

    // check for single calls to buffer method

    if (auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(stmt))
    {
        if (auto declRefExprLHS
            = dyn_cast<DeclRefExpr>(ignore(memberCallExpr->getImplicitObjectArgument())))
        {
            auto methodDecl = memberCallExpr->getMethodDecl();
            if (methodDecl && methodDecl->getIdentifier())
                if (auto varDeclLHS = dyn_cast<VarDecl>(declRefExprLHS->getDecl()))
                {
                    auto tc = loplugin::TypeCheck(varDeclLHS->getType());
                    if (tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
                        || tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace())
                    {
                        if (isMethodOkToMerge(memberCallExpr))
                            addToGoodMap(varDeclLHS, parentStmt);
                        else
                            badMap.insert(varDeclLHS);
                    }
                }
            return;
        }
    }

    // now check for chained append calls

    auto expr = dyn_cast<Expr>(stmt);
    if (!expr)
        return;
    auto tc = loplugin::TypeCheck(expr->getType());
    if (!tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace())
        return;

    // unwrap the chain (which runs from right to left)
    const VarDecl* varDeclLHS = nullptr;
    bool good = true;
    while (true)
    {
        auto memberCallExpr = dyn_cast<CXXMemberCallExpr>(expr);
        if (!memberCallExpr)
            break;
        good &= isMethodOkToMerge(memberCallExpr);

        if (auto declRefExprLHS
            = dyn_cast<DeclRefExpr>(ignore(memberCallExpr->getImplicitObjectArgument())))
        {
            varDeclLHS = dyn_cast<VarDecl>(declRefExprLHS->getDecl());
            break;
        }
        expr = memberCallExpr->getImplicitObjectArgument();
    }

    if (varDeclLHS)
    {
        if (good)
            addToGoodMap(varDeclLHS, parentStmt);
        else
            badMap.insert(varDeclLHS);
    }
}

void BufferAdd::addToGoodMap(const VarDecl* varDecl, const Stmt* parentStmt)
{
    // check that vars are all inside the same compoundstmt, if they are not, we cannot combine them
    auto it = goodMap.find(varDecl);
    if (it != goodMap.end())
    {
        if (it->second == parentStmt)
            return;
        // don't treat these as parents, otherwise we eliminate .append.append sequences
        if (isa<MemberExpr>(parentStmt))
            return;
        if (isa<CXXMemberCallExpr>(parentStmt))
            return;
        badMap.insert(varDecl);
    }
    else
        goodMap.emplace(varDecl, parentStmt);
}

bool BufferAdd::isMethodOkToMerge(CXXMemberCallExpr const* memberCall)
{
    auto methodDecl = memberCall->getMethodDecl();
    if (methodDecl->getNumParams() == 0)
        return true;

    if (auto const id = methodDecl->getIdentifier())
    {
        auto name = id->getName();
        if (name == "appendUninitialized" || name == "setLength" || name == "remove"
            || name == "insert" || name == "appendAscii" || name == "appendUtf32")
            return false;
    }

    auto rhs = memberCall->getArg(0);
    if (!isSideEffectFree(rhs))
        return false;
    return true;
}

Expr const* BufferAdd::ignore(Expr const* expr)
{
    return expr->IgnoreImplicit()->IgnoreParens()->IgnoreImplicit();
}

bool BufferAdd::isSideEffectFree(Expr const* expr)
{
    expr = ignore(expr);
    // I don't think the OUStringAppend functionality can handle this efficiently
    if (isa<ConditionalOperator>(expr))
        return false;
    // Multiple statements have a well defined evaluation order (sequence points between them)
    // but a single expression may be evaluated in arbitrary order;
    // if there are side effects in one of the sub-expressions that have an effect on another subexpression,
    // the result may be incorrect, and you don't necessarily notice in tests because the order is compiler-dependent.
    // for example see commit afd743141f7a7dd05914d0872c9afe079f16fe0c where such a refactoring introduced such a bug.
    // So only consider simple RHS expressions.
    if (!expr->HasSideEffects(compiler.getASTContext()))
        return true;

    // check for chained adds which are side-effect free
    if (auto operatorCall = dyn_cast<CXXOperatorCallExpr>(expr))
    {
        auto op = operatorCall->getOperator();
        if (op == OO_PlusEqual || op == OO_Plus)
            if (isSideEffectFree(operatorCall->getArg(0))
                && isSideEffectFree(operatorCall->getArg(1)))
                return true;
    }

    if (auto callExpr = dyn_cast<CallExpr>(expr))
    {
        // check for calls through OUString::number/OUString::unacquired
        if (auto calleeMethodDecl = dyn_cast_or_null<CXXMethodDecl>(callExpr->getCalleeDecl()))
            if (calleeMethodDecl && calleeMethodDecl->getIdentifier())
            {
                if (callExpr->getNumArgs() > 0)
                {
                    auto tc = loplugin::TypeCheck(calleeMethodDecl->getParent());
                    if (tc.Class("OUString") || tc.Class("OString"))
                    {
                        if (isSideEffectFree(callExpr->getArg(0)))
                            return true;
                    }
                }
            }
        if (auto calleeFunctionDecl = dyn_cast_or_null<FunctionDecl>(callExpr->getCalleeDecl()))
            if (calleeFunctionDecl && calleeFunctionDecl->getIdentifier())
            {
                auto name = calleeFunctionDecl->getName();
                // check for calls through OUStringToOString
                if (name == "OUStringToOString" || name == "OStringToOUString")
                    if (isSideEffectFree(callExpr->getArg(0)))
                        return true;
                // allowlist some known-safe methods
                if (compat::ends_with(name, "ResId") || name == "GetXMLToken")
                    if (isSideEffectFree(callExpr->getArg(0)))
                        return true;
            }
        // O[U]String::operator std::[u16]string_view:
        if (auto const d = dyn_cast_or_null<CXXConversionDecl>(callExpr->getCalleeDecl()))
        {
            auto tc = loplugin::TypeCheck(d->getParent());
            if (tc.Class("OString") || tc.Class("OUString"))
            {
                return true;
            }
        }
    }

    // sometimes we have a constructor call on the RHS
    if (auto constructExpr = dyn_cast<CXXConstructExpr>(expr))
    {
        auto dc = loplugin::DeclCheck(constructExpr->getConstructor());
        if (dc.MemberFunction().Class("OUString") || dc.MemberFunction().Class("OString")
            || dc.MemberFunction().Class("OUStringBuffer")
            || dc.MemberFunction().Class("OStringBuffer"))
            if (constructExpr->getNumArgs() == 0 || isSideEffectFree(constructExpr->getArg(0)))
                return true;
        // Expr::HasSideEffects does not like stuff that passes through OUStringLiteral
        auto dc2 = loplugin::DeclCheck(constructExpr->getConstructor()->getParent());
        if (dc2.Class("OUStringLiteral").Namespace("rtl").GlobalNamespace())
            return true;
    }

    // when adding literals, we sometimes get this
    if (auto functionalCastExpr = dyn_cast<CXXFunctionalCastExpr>(expr))
    {
        auto tc = loplugin::TypeCheck(functionalCastExpr->getType());
        if (tc.Class("OUStringLiteral").Namespace("rtl").GlobalNamespace())
            return isSideEffectFree(functionalCastExpr->getSubExpr());
    }

    return false;
}

loplugin::Plugin::Registration<BufferAdd> bufferadd("bufferadd");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
