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
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/**
    Look for repeated addition to OUString/OString.

    Eg.
      OUString x = "xxx";
      x += b;

    which can be simplified to
        x = "xxx" + b

    which is more efficient, because of the OUStringConcat magic.
*/

namespace
{
class StringAdd : public loplugin::FilteringPlugin<StringAdd>
{
public:
    explicit StringAdd(loplugin::InstantiationData const& data)
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
        // there is an ifdef here, but my check is not working, not sure why
        if (fn == SRCDIR "/pyuno/source/module/pyuno_runtime.cxx")
            return false;
        // TODO the += depends on the result of the preceding assign, so can't merge
        if (fn == SRCDIR "/editeng/source/misc/svxacorr.cxx")
            return false;
        return true;
    }

    virtual void run() override
    {
        if (!preRun())
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCompoundStmt(CompoundStmt const*);
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const*);

private:
    const VarDecl* findAssignOrAdd(Stmt const*);
    void checkForCompoundAssign(Stmt const* stmt1, Stmt const* stmt2, VarDecl const* varDecl);

    Expr const* ignore(Expr const*);
    std::string getSourceAsString(SourceLocation startLoc, SourceLocation endLoc);
    bool isSideEffectFree(Expr const*);
};

bool StringAdd::VisitCompoundStmt(CompoundStmt const* compoundStmt)
{
    if (ignoreLocation(compoundStmt))
        return true;

    auto it = compoundStmt->body_begin();
    while (true)
    {
        if (it == compoundStmt->body_end())
            break;
        const VarDecl* foundVar = findAssignOrAdd(*it);
        // reference types have slightly weird behaviour
        if (foundVar && !foundVar->getType()->isReferenceType())
        {
            auto stmt1 = *it;
            ++it;
            if (it == compoundStmt->body_end())
                break;
            checkForCompoundAssign(stmt1, *it, foundVar);
            continue;
        }
        else
            ++it;
    }

    return true;
}

const VarDecl* StringAdd::findAssignOrAdd(Stmt const* stmt)
{
    if (auto exprCleanup = dyn_cast<ExprWithCleanups>(stmt))
        stmt = exprCleanup->getSubExpr();
    if (auto switchCase = dyn_cast<SwitchCase>(stmt))
        stmt = switchCase->getSubStmt();

    if (auto declStmt = dyn_cast<DeclStmt>(stmt))
        if (declStmt->isSingleDecl())
            if (auto varDeclLHS = dyn_cast_or_null<VarDecl>(declStmt->getSingleDecl()))
            {
                auto tc = loplugin::TypeCheck(varDeclLHS->getType());
                if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
                    && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
                    return nullptr;
                if (varDeclLHS->getStorageDuration() == SD_Static)
                    return nullptr;
                if (!varDeclLHS->hasInit())
                    return nullptr;
                if (!isSideEffectFree(varDeclLHS->getInit()))
                    return nullptr;
                return varDeclLHS;
            }
    if (auto operatorCall = dyn_cast<CXXOperatorCallExpr>(stmt))
        if (operatorCall->getOperator() == OO_Equal || operatorCall->getOperator() == OO_PlusEqual)
            if (auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(operatorCall->getArg(0))))
                if (auto varDeclLHS = dyn_cast<VarDecl>(declRefExprLHS->getDecl()))
                {
                    auto tc = loplugin::TypeCheck(varDeclLHS->getType());
                    if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
                        && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
                        return nullptr;
                    auto rhs = operatorCall->getArg(1);
                    if (!isSideEffectFree(rhs))
                        return nullptr;
                    return varDeclLHS;
                }
    return nullptr;
}

void StringAdd::checkForCompoundAssign(Stmt const* stmt1, Stmt const* stmt2, VarDecl const* varDecl)
{
    // OString additions are frequently wrapped in these
    if (auto exprCleanup = dyn_cast<ExprWithCleanups>(stmt2))
        stmt2 = exprCleanup->getSubExpr();
    if (auto switchCase = dyn_cast<SwitchCase>(stmt2))
        stmt2 = switchCase->getSubStmt();
    auto operatorCall = dyn_cast<CXXOperatorCallExpr>(stmt2);
    if (!operatorCall)
        return;
    if (operatorCall->getOperator() != OO_PlusEqual)
        return;
    auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(operatorCall->getArg(0)));
    if (!declRefExprLHS)
        return;
    if (declRefExprLHS->getDecl() != varDecl)
        return;
    auto rhs = operatorCall->getArg(1);
    if (!isSideEffectFree(rhs))
        return;
    // if we cross a #ifdef boundary
    auto s
        = getSourceAsString(stmt1->getSourceRange().getBegin(), stmt2->getSourceRange().getEnd());
    if (s.find("#if") != std::string::npos)
        return;
    report(DiagnosticsEngine::Warning, "simplify by merging with the preceding assignment",
           compat::getBeginLoc(stmt2))
        << stmt2->getSourceRange();
}

// Check for generating temporaries when adding strings
//
bool StringAdd::VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* operatorCall)
{
    if (ignoreLocation(operatorCall))
        return true;
    if (operatorCall->getOperator() != OO_Plus)
        return true;
    auto tc = loplugin::TypeCheck(operatorCall->getType()->getUnqualifiedDesugaredType());
    if (!tc.Struct("OUStringConcat").Namespace("rtl").GlobalNamespace()
        && !tc.Struct("OStringConcat").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
        return true;

    auto check = [operatorCall, this, &tc](unsigned arg) {
        auto const e
            = dyn_cast<CXXFunctionalCastExpr>(operatorCall->getArg(arg)->IgnoreParenImpCasts());
        if (e == nullptr)
            return;
        auto tc3 = loplugin::TypeCheck(e->getType());
        if (!tc3.Class("OUString").Namespace("rtl").GlobalNamespace()
            && !tc3.Class("OString").Namespace("rtl").GlobalNamespace())
            return;
        // we don't have OStringLiteral1, so char needs to generate a temporary
        if (tc.Class("OString").Namespace("rtl").GlobalNamespace()
            || tc.Struct("OStringConcat").Namespace("rtl").GlobalNamespace())
            if (auto bindTemp = dyn_cast<CXXBindTemporaryExpr>(e->getSubExpr()))
                if (auto cxxConstruct = dyn_cast<CXXConstructExpr>(bindTemp->getSubExpr()))
                    if (loplugin::TypeCheck(
                            cxxConstruct->getConstructor()->getParamDecl(0)->getType())
                            .Char())
                        return;
        report(DiagnosticsEngine::Warning,
               ("avoid constructing %0 from %1 on %select{L|R}2HS of + (where %select{R|L}2HS is of"
                " type %3)"),
               compat::getBeginLoc(e))
            << e->getType().getLocalUnqualifiedType() << e->getSubExprAsWritten()->getType() << arg
            << operatorCall->getArg(1 - arg)->IgnoreImpCasts()->getType() << e->getSourceRange();
    };

    check(0);
    check(1);
    return true;
}

Expr const* StringAdd::ignore(Expr const* expr)
{
    return compat::IgnoreImplicit(compat::IgnoreImplicit(expr)->IgnoreParens());
}

bool StringAdd::isSideEffectFree(Expr const* expr)
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
                auto name = calleeMethodDecl->getName();
                if (callExpr->getNumArgs() > 0
                    && (name == "number" || name == "unacquired" || name == "boolean"
                        || name == "copy"))
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
                // whitelist some known-safe methods
                if (name.endswith("ResId") || name == "GetXMLToken")
                    if (isSideEffectFree(callExpr->getArg(0)))
                        return true;
            }
    }

    // sometimes we have a constructor call on the RHS
    if (auto constructExpr = dyn_cast<CXXConstructExpr>(expr))
    {
        auto dc = loplugin::DeclCheck(constructExpr->getConstructor());
        if (dc.MemberFunction().Class("OUString") || dc.MemberFunction().Class("OString"))
            if (constructExpr->getNumArgs() == 0 || isSideEffectFree(constructExpr->getArg(0)))
                return true;
        // Expr::HasSideEffects does not like stuff that passes through OUStringLiteral
        auto dc2 = loplugin::DeclCheck(constructExpr->getConstructor()->getParent());
        if (dc2.Struct("OUStringLiteral").Namespace("rtl").GlobalNamespace())
            return true;
    }

    // when adding literals, we sometimes get this
    if (auto functionalCastExpr = dyn_cast<CXXFunctionalCastExpr>(expr))
    {
        auto tc = loplugin::TypeCheck(functionalCastExpr->getType());
        if (tc.Struct("OUStringLiteral").Namespace("rtl").GlobalNamespace())
            return isSideEffectFree(functionalCastExpr->getSubExpr());
    }

    return false;
}

std::string StringAdd::getSourceAsString(SourceLocation startLoc, SourceLocation endLoc)
{
    SourceManager& SM = compiler.getSourceManager();
    char const* p1 = SM.getCharacterData(startLoc);
    char const* p2 = SM.getCharacterData(endLoc);
    p2 += Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    // sometimes we get weird results, probably from crossing internal LLVM buffer boundaries
    if (p2 - p1 < 0)
        return std::string();
    return std::string(p1, p2 - p1);
}

loplugin::Plugin::Registration<StringAdd> stringadd("stringadd");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
