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
#include "compat.hxx"
#include "config_clang.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/**
    Look for repeated addition to OUString/OString/OUStringBuffer/OStringBuffer.

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
        // TODO this file has a boatload of buffer appends' and I don't feel like fixing them all now
        if (fn == SRCDIR "/vcl/source/gdi/pdfwriter_impl.cxx")
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
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr const*);

private:
    enum class Summands
    {
        OnlyCompileTimeConstants,
        OnlySideEffectFree,
        SideEffect
    };

    struct VarDeclAndSummands
    {
        const VarDecl* varDecl;
        Summands summands;
    };

    VarDeclAndSummands findAssignOrAdd(Stmt const*);
    bool checkForCompoundAssign(Stmt const* stmt1, Stmt const* stmt2, VarDeclAndSummands& varDecl);

    Expr const* ignore(Expr const*);
    bool isSideEffectFree(Expr const*);
    bool isCompileTimeConstant(Expr const*);
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
        VarDeclAndSummands foundVar = findAssignOrAdd(*it);
        // reference types have slightly weird behaviour
        if (foundVar.varDecl && !foundVar.varDecl->getType()->isReferenceType())
        {
            auto stmt1 = *it;
            ++it;
            while (it != compoundStmt->body_end())
            {
                if (!checkForCompoundAssign(stmt1, *it, foundVar))
                {
                    break;
                }
                stmt1 = *it;
                ++it;
            }
        }
        else
            ++it;
    }

    return true;
}

StringAdd::VarDeclAndSummands StringAdd::findAssignOrAdd(Stmt const* stmt)
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
                    && !tc.Class("OString").Namespace("rtl").GlobalNamespace()
                    && !tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
                    && !tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace())
                    return {};
                if (varDeclLHS->getStorageDuration() == SD_Static)
                    return {};
                if (!varDeclLHS->hasInit())
                    return {};
                if (tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
                    || tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace())
                {
                    // ignore the constructor that gives the buffer a default size
                    if (auto cxxConstructor = dyn_cast<CXXConstructExpr>(varDeclLHS->getInit()))
                        if (auto constructorDecl = cxxConstructor->getConstructor())
                            if ((constructorDecl->getNumParams() == 1
                                 && loplugin::TypeCheck(constructorDecl->getParamDecl(0)->getType())
                                        .Typedef("sal_Int32")
                                        .GlobalNamespace())
                                || (constructorDecl->getNumParams() == 2
                                    && constructorDecl->getParamDecl(0)->getType()->isIntegralType(
                                           compiler.getASTContext())
                                    && constructorDecl->getParamDecl(1)
                                           ->getType()
                                           ->isSpecificBuiltinType(BuiltinType::Int)))
                                return {};
                }
                return { varDeclLHS, (isCompileTimeConstant(varDeclLHS->getInit())
                                          ? Summands::OnlyCompileTimeConstants
                                          : (isSideEffectFree(varDeclLHS->getInit())
                                                 ? Summands::OnlySideEffectFree
                                                 : Summands::SideEffect)) };
            }
    if (auto operatorCall = dyn_cast<CXXOperatorCallExpr>(stmt))
        if (operatorCall->getOperator() == OO_Equal || operatorCall->getOperator() == OO_PlusEqual)
            if (auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(operatorCall->getArg(0))))
                if (auto varDeclLHS = dyn_cast<VarDecl>(declRefExprLHS->getDecl()))
                {
                    auto tc = loplugin::TypeCheck(varDeclLHS->getType());
                    if (!tc.Class("OUString").Namespace("rtl").GlobalNamespace()
                        && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
                        return {};
                    auto rhs = operatorCall->getArg(1);
                    return { varDeclLHS,
                             (isCompileTimeConstant(rhs)
                                  ? Summands::OnlyCompileTimeConstants
                                  : (isSideEffectFree(rhs) ? Summands::OnlySideEffectFree
                                                           : Summands::SideEffect)) };
                }
    if (auto memberCall = dyn_cast<CXXMemberCallExpr>(stmt))
        if (auto cxxMethodDecl = dyn_cast_or_null<CXXMethodDecl>(memberCall->getDirectCallee()))
            if (cxxMethodDecl->getIdentifier() && cxxMethodDecl->getName() == "append")
                if (auto declRefExprLHS
                    = dyn_cast<DeclRefExpr>(ignore(memberCall->getImplicitObjectArgument())))
                    if (auto varDeclLHS = dyn_cast<VarDecl>(declRefExprLHS->getDecl()))
                    {
                        auto tc = loplugin::TypeCheck(varDeclLHS->getType());
                        if (!tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
                            && !tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace())
                            return {};
                        auto rhs = memberCall->getArg(0);
                        return { varDeclLHS,
                                 (isCompileTimeConstant(rhs)
                                      ? Summands::OnlyCompileTimeConstants
                                      : (isSideEffectFree(rhs) ? Summands::OnlySideEffectFree
                                                               : Summands::SideEffect)) };
                    }
    return {};
}

bool StringAdd::checkForCompoundAssign(Stmt const* stmt1, Stmt const* stmt2,
                                       VarDeclAndSummands& varDecl)
{
    // OString additions are frequently wrapped in these
    if (auto exprCleanup = dyn_cast<ExprWithCleanups>(stmt2))
        stmt2 = exprCleanup->getSubExpr();
    if (auto switchCase = dyn_cast<SwitchCase>(stmt2))
        stmt2 = switchCase->getSubStmt();

    const DeclRefExpr* declRefExprLHS;
    const Expr* rhs;
    auto tc = loplugin::TypeCheck(varDecl.varDecl->getType());
    if (tc.Class("OString") || tc.Class("OUString"))
    {
        auto operatorCall = dyn_cast<CXXOperatorCallExpr>(stmt2);
        if (!operatorCall)
            return false;
        if (operatorCall->getOperator() != OO_PlusEqual)
            return false;
        declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(operatorCall->getArg(0)));
        rhs = operatorCall->getArg(1);
    }
    else
    {
        // OUStringBuffer, OStringBuffer
        auto memberCall = dyn_cast<CXXMemberCallExpr>(stmt2);
        if (!memberCall)
            return false;
        auto cxxMethodDecl = dyn_cast_or_null<CXXMethodDecl>(memberCall->getDirectCallee());
        if (!cxxMethodDecl)
            return false;
        if (!cxxMethodDecl->getIdentifier() || cxxMethodDecl->getName() != "append")
            return false;
        declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(memberCall->getImplicitObjectArgument()));
        rhs = memberCall->getArg(0);
    }
    if (!declRefExprLHS)
        return false;
    if (declRefExprLHS->getDecl() != varDecl.varDecl)
        return false;
    // if either side is a compile-time-constant, then we don't care about
    // side-effects
    bool const ctcRhs = isCompileTimeConstant(rhs);
    if (!ctcRhs)
    {
        auto const sefRhs = isSideEffectFree(rhs);
        auto const oldSummands = varDecl.summands;
        varDecl.summands = sefRhs ? Summands::OnlySideEffectFree : Summands::SideEffect;
        if (oldSummands != Summands::OnlyCompileTimeConstants
            && (oldSummands == Summands::SideEffect || !sefRhs))
        {
            return true;
        }
    }
    SourceRange mergeRange(stmt1->getSourceRange().getBegin(), stmt2->getSourceRange().getEnd());
    // if we cross a #ifdef boundary
    if (containsPreprocessingConditionalInclusion(mergeRange))
    {
        varDecl.summands
            = ctcRhs ? Summands::OnlyCompileTimeConstants
                     : isSideEffectFree(rhs) ? Summands::OnlySideEffectFree : Summands::SideEffect;
        return true;
    }
    // If there is a comment between two calls, rather don't suggest merge
    // IMO, code clarity trumps efficiency (as far as plugin warnings go, anyway).
    if (containsComment(mergeRange))
        return true;
    // I don't think the OUStringAppend functionality can handle this efficiently
    if (isa<ConditionalOperator>(ignore(rhs)))
        return false;
    report(DiagnosticsEngine::Warning, "simplify by merging with the preceding assign/append",
           stmt2->getBeginLoc())
        << stmt2->getSourceRange();
    return true;
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
    if (!tc.Struct("StringConcat").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OString").Namespace("rtl").GlobalNamespace())
        return true;

    auto check = [operatorCall, this](unsigned arg) {
        auto const e
            = dyn_cast<CXXFunctionalCastExpr>(operatorCall->getArg(arg)->IgnoreParenImpCasts());
        if (e == nullptr)
            return;
        auto tc3 = loplugin::TypeCheck(e->getType());
        if (!tc3.Class("OUString").Namespace("rtl").GlobalNamespace()
            && !tc3.Class("OString").Namespace("rtl").GlobalNamespace()
            && !tc3.Class("OUStringLiteral").Namespace("rtl").GlobalNamespace()
            && !tc3.Class("OStringLiteral").Namespace("rtl").GlobalNamespace()
            && !tc3.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
            && !tc3.Class("OStringBuffer").Namespace("rtl").GlobalNamespace())
            return;
        report(DiagnosticsEngine::Warning,
               ("rather use O[U]String::Concat than constructing %0 from %1 on %select{L|R}2HS of "
                "+ (where %select{R|L}2HS is of"
                " type %3)"),
               e->getBeginLoc())
            << e->getType().getLocalUnqualifiedType() << e->getSubExprAsWritten()->getType() << arg
            << operatorCall->getArg(1 - arg)->IgnoreImpCasts()->getType() << e->getSourceRange();
    };

    check(0);
    check(1);
    return true;
}

bool StringAdd::VisitCXXMemberCallExpr(CXXMemberCallExpr const* methodCall)
{
    if (ignoreLocation(methodCall))
        return true;

    auto methodDecl = methodCall->getMethodDecl();
    if (!methodDecl || !methodDecl->getIdentifier() || methodDecl->getName() != "append"
        || methodCall->getNumArgs() == 0)
        return true;
    auto tc1 = loplugin::TypeCheck(methodCall->getType());
    if (!tc1.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
        && !tc1.Class("OStringBuffer").Namespace("rtl").GlobalNamespace())
        return true;
    auto arg = methodCall->getArg(0);
    // I don't think the OUStringAppend functionality can handle this efficiently
    if (isa<ConditionalOperator>(ignore(arg)))
        return true;

    auto methodCall2 = dyn_cast<CXXMemberCallExpr>(ignore(methodCall->getImplicitObjectArgument()));
    if (!methodCall2)
        return true;
    auto tc = loplugin::TypeCheck(methodCall2->getType());
    if (!tc.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
        && !tc.Class("OStringBuffer").Namespace("rtl").GlobalNamespace())
        return true;
    auto methodDecl2 = methodCall2->getMethodDecl();
    if (!methodDecl2->getIdentifier() || methodDecl2->getName() != "append"
        || methodCall2->getNumArgs() == 0)
        return true;
    arg = methodCall2->getArg(0);
    // I don't think the OUStringAppend functionality can handle this efficiently
    if (isa<ConditionalOperator>(ignore(arg)))
        return true;
    report(DiagnosticsEngine::Warning,
           "chained append, rather use single append call and + operator",
           methodCall2->getBeginLoc())
        << methodCall2->getSourceRange();

    return true;
}

Expr const* StringAdd::ignore(Expr const* expr)
{
    return expr->IgnoreImplicit()->IgnoreParens()->IgnoreImplicit();
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
        {
            if (calleeMethodDecl->getIdentifier())
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
            else if (auto const d = dyn_cast<CXXConversionDecl>(calleeMethodDecl))
            {
                if (loplugin::TypeCheck(d->getConversionType())
                        .ClassOrStruct("basic_string_view")
                        .StdNamespace())
                {
                    auto const tc = loplugin::TypeCheck(calleeMethodDecl->getParent());
                    if (tc.Class("OUString").Namespace("rtl").GlobalNamespace()
                        || tc.Class("OString").Namespace("rtl").GlobalNamespace())
                    {
                        if (isSideEffectFree(callExpr->getCallee()))
                            return true;
                    }
                }
            }
            // Aggressively assume that calls to const member functions are side effect free (if
            // all of the call's sub-expressions are):
            if (calleeMethodDecl->isConst())
            {
                auto sef = true;
                // Other options besides CXXMemberCallExpr are e.g. CXXOperatorCallExpr which
                // does not have such a target expression:
                if (auto const mce = dyn_cast<CXXMemberCallExpr>(callExpr))
                {
                    if (!isSideEffectFree(mce->getImplicitObjectArgument()))
                    {
                        sef = false;
                    }
                }
                if (sef)
                {
                    for (unsigned i = 0; i != callExpr->getNumArgs(); ++i)
                    {
                        if (!isSideEffectFree(callExpr->getArg(i)))
                        {
                            sef = false;
                            break;
                        }
                    }
                }
                if (sef)
                {
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
        if (dc2.Class("OUStringLiteral").Namespace("rtl").GlobalNamespace()
            || dc2.Class("OStringLiteral").Namespace("rtl").GlobalNamespace())
            return true;
    }

    // when adding literals, we sometimes get this
    if (auto functionalCastExpr = dyn_cast<CXXFunctionalCastExpr>(expr))
    {
        auto tc = loplugin::TypeCheck(functionalCastExpr->getType());
        if (tc.Class("OUStringLiteral").Namespace("rtl").GlobalNamespace()
            || tc.Class("OStringLiteral").Namespace("rtl").GlobalNamespace())
            return isSideEffectFree(functionalCastExpr->getSubExpr());
    }

    return false;
}

bool StringAdd::isCompileTimeConstant(Expr const* expr)
{
    expr = expr->IgnoreImplicit();
    if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(expr))
        if (cxxConstructExpr->getNumArgs() > 0)
            expr = cxxConstructExpr->getArg(0);
    return isa<clang::StringLiteral>(expr);
}

loplugin::Plugin::Registration<StringAdd> stringadd("stringadd");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
