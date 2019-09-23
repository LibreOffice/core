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

    virtual void run() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        if (fn == SRCDIR "/sal/qa/rtl/oustring/rtl_OUString2.cxx"
            || fn == SRCDIR "/sal/qa/rtl/strings/test_oustring_concat.cxx"
            || fn == SRCDIR "/sal/qa/rtl/strings/test_ostring_concat.cxx"
            || fn == SRCDIR "/sal/qa/OStringBuffer/rtl_OStringBuffer.cxx")
            return;
        // there is an ifdef here, but my check is not working, not sure why
        if (fn == SRCDIR "/pyuno/source/module/pyuno_runtime.cxx")
            return;
        // TODO the += depends on the result of the preceding assign, so can't merge
        if (fn == SRCDIR "/editeng/source/misc/svxacorr.cxx")
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCompoundStmt(CompoundStmt const*);

private:
    const VarDecl* findAssignOrAdd(Stmt const*);
    Expr const* ignore(Expr const*);
    void checkForCompoundAssign(Stmt const* stmt1, Stmt const* stmt2, VarDecl const* varDecl);
    std::string getSourceAsString(SourceLocation startLoc, SourceLocation endLoc);
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
                    return varDeclLHS;
                }
    return nullptr;
}

void StringAdd::checkForCompoundAssign(Stmt const* stmt1, Stmt const* stmt2, VarDecl const* varDecl)
{
    // OString additions are frequently wrapped in these
    if (auto exprCleanup = dyn_cast<ExprWithCleanups>(stmt2))
        stmt2 = exprCleanup->getSubExpr();

    if (auto operatorCall = dyn_cast<CXXOperatorCallExpr>(stmt2))
        if (operatorCall->getOperator() == OO_PlusEqual)
            if (auto declRefExprLHS = dyn_cast<DeclRefExpr>(ignore(operatorCall->getArg(0))))
                if (declRefExprLHS->getDecl() == varDecl)
                {
                    // if we cross a #ifdef boundary
                    auto s = getSourceAsString(stmt1->getSourceRange().getBegin(),
                                               stmt2->getSourceRange().getEnd());
                    if (s.find("#if") != std::string::npos)
                        return;
                    report(DiagnosticsEngine::Warning,
                           "simplify by merging with the preceding assignment",
                           compat::getBeginLoc(stmt2))
                        << stmt2->getSourceRange();
                }
}

Expr const* StringAdd::ignore(Expr const* expr)
{
    return expr->IgnoreImplicit()->IgnoreParens()->IgnoreImplicit();
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

loplugin::Plugin::Registration<StringAdd> X("stringadd");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
