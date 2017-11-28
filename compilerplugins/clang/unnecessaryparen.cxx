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
#include <unordered_set>

#include <clang/AST/CXXInheritance.h>
#include "compat.hxx"
#include "plugin.hxx"

/**
look for unnecessary parentheses
*/

namespace {

// Like clang::Stmt::IgnoreImplicit (lib/AST/Stmt.cpp), but also ignoring CXXConstructExpr and
// looking through implicit UserDefinedConversion's member function call:
Expr const * ignoreAllImplicit(Expr const * expr) {
    if (auto const e = dyn_cast<ExprWithCleanups>(expr)) {
        expr = e->getSubExpr();
    }
    if (auto const e = dyn_cast<CXXConstructExpr>(expr)) {
        if (e->getNumArgs() == 1) {
            expr = e->getArg(0);
        }
    }
    if (auto const e = dyn_cast<MaterializeTemporaryExpr>(expr)) {
        expr = e->GetTemporaryExpr();
    }
    if (auto const e = dyn_cast<CXXBindTemporaryExpr>(expr)) {
        expr = e->getSubExpr();
    }
    while (auto const e = dyn_cast<ImplicitCastExpr>(expr)) {
        expr = e->getSubExpr();
        if (e->getCastKind() == CK_UserDefinedConversion) {
            auto const ce = cast<CXXMemberCallExpr>(expr);
            assert(ce->getNumArgs() == 0);
            expr = ce->getImplicitObjectArgument();
        }
    }
    return expr;
}

class UnnecessaryParen:
    public RecursiveASTVisitor<UnnecessaryParen>, public loplugin::Plugin
{
public:
    explicit UnnecessaryParen(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        StringRef fn( compiler.getSourceManager().getFileEntryForID(
                          compiler.getSourceManager().getMainFileID())->getName() );
        // fixing this, makes the source in the .y files look horrible
        if (loplugin::isSamePathname(fn, WORKDIR "/YaccTarget/unoidl/source/sourceprovider-parser.cxx"))
             return;
        if (loplugin::isSamePathname(fn, WORKDIR "/YaccTarget/idlc/source/parser.cxx"))
             return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitParenExpr(const ParenExpr *);
    bool VisitIfStmt(const IfStmt *);
    bool VisitDoStmt(const DoStmt *);
    bool VisitWhileStmt(const WhileStmt *);
    bool VisitSwitchStmt(const SwitchStmt *);
    bool VisitCaseStmt(const CaseStmt *);
    bool VisitReturnStmt(const ReturnStmt* );
    bool VisitCallExpr(const CallExpr *);
    bool VisitVarDecl(const VarDecl *);
    bool VisitCXXOperatorCallExpr(const CXXOperatorCallExpr *);
    bool VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr const *);
    bool VisitConditionalOperator(ConditionalOperator const * expr);
    bool VisitMemberExpr(const MemberExpr *f);
private:
    void VisitSomeStmt(Stmt const * stmt, const Expr* cond, StringRef stmtName);

    // Hack for libxml2's BAD_CAST object-like macro (expanding to "(xmlChar *)"), which is
    // typically used as if it were a function-like macro, e.g., as "BAD_CAST(pName)" in
    // SwNode::dumpAsXml (sw/source/core/docnode/node.cxx):
    bool isPrecededBy_BAD_CAST(Expr const * expr);

    std::unordered_set<ParenExpr const *> handled_;
};

bool UnnecessaryParen::VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr const * expr)
{
    if (expr->getKind() == UETT_SizeOf && !expr->isArgumentType()) {
        if (auto const e = dyn_cast<ParenExpr>(ignoreAllImplicit(expr->getArgumentExpr()))) {
            handled_.insert(e);
        }
    }
    return true;
}

bool UnnecessaryParen::VisitConditionalOperator(ConditionalOperator const * expr) {
    if (auto const e = dyn_cast<ParenExpr>(ignoreAllImplicit(expr->getCond()))) {
        if (isa<CXXBoolLiteralExpr>(e->getSubExpr())) {
            handled_.insert(e);
        }
    }
    return true;
}

bool UnnecessaryParen::VisitParenExpr(const ParenExpr* parenExpr)
{
    if (ignoreLocation(parenExpr))
        return true;
    if (parenExpr->getLocStart().isMacroID())
        return true;
    if (handled_.find(parenExpr) != handled_.end())
        return true;

    auto subExpr = ignoreAllImplicit(parenExpr->getSubExpr());

    if (auto subParenExpr = dyn_cast<ParenExpr>(subExpr))
    {
        if (subParenExpr->getLocStart().isMacroID())
            return true;
        report(
            DiagnosticsEngine::Warning, "parentheses around parentheses",
            parenExpr->getLocStart())
            << parenExpr->getSourceRange();
        handled_.insert(subParenExpr);
    }

    // Somewhat redundantly add parenExpr to handled_, so that issues within InitListExpr don't get
    // reported twice (without having to change TraverseInitListExpr to only either traverse the
    // syntactic or semantic form, as other plugins do):

    if (isa<DeclRefExpr>(subExpr)) {
        if (!isPrecededBy_BAD_CAST(parenExpr)) {
            report(
                DiagnosticsEngine::Warning, "unnecessary parentheses around identifier",
                parenExpr->getLocStart())
                << parenExpr->getSourceRange();
            handled_.insert(parenExpr);
        }
    } else if (isa<CharacterLiteral>(subExpr) || isa<FloatingLiteral>(subExpr)
               || isa<ImaginaryLiteral>(subExpr) || isa<CXXNullPtrLiteralExpr>(subExpr))
        //TODO: isa<IntegerLiteral>(subExpr)
        //TODO: isa<CXXBoolLiteralExpr>(subExpr) || isa<ObjCBoolLiteralExpr>(subExpr)
    {
        auto const loc = subExpr->getLocStart();
        if (loc.isMacroID() && compiler.getSourceManager().isAtStartOfImmediateMacroExpansion(loc))
        {
            // just in case the macro could also expand to something that /would/ require
            // parentheses here
            return true;
        }
        report(
            DiagnosticsEngine::Warning, "unnecessary parentheses around literal",
            parenExpr->getLocStart())
            << parenExpr->getSourceRange();
        handled_.insert(parenExpr);
    } else if (auto const e = dyn_cast<clang::StringLiteral>(subExpr)) {
        if (e->getNumConcatenated() == 1 && !isPrecededBy_BAD_CAST(parenExpr)) {
            report(
                DiagnosticsEngine::Warning,
                "unnecessary parentheses around single-token string literal",
                parenExpr->getLocStart())
                << parenExpr->getSourceRange();
            handled_.insert(parenExpr);
        }
    } else if (isa<CXXNamedCastExpr>(subExpr)) {
        report(
            DiagnosticsEngine::Warning, "unnecessary parentheses around cast",
            parenExpr->getLocStart())
            << parenExpr->getSourceRange();
        handled_.insert(parenExpr);
    }

    return true;
}

bool UnnecessaryParen::VisitIfStmt(const IfStmt* ifStmt)
{
    VisitSomeStmt(ifStmt, ifStmt->getCond(), "if");
    return true;
}

bool UnnecessaryParen::VisitDoStmt(const DoStmt* doStmt)
{
    VisitSomeStmt(doStmt, doStmt->getCond(), "do");
    return true;
}

bool UnnecessaryParen::VisitWhileStmt(const WhileStmt* whileStmt)
{
    VisitSomeStmt(whileStmt, whileStmt->getCond(), "while");
    return true;
}

bool UnnecessaryParen::VisitSwitchStmt(const SwitchStmt* switchStmt)
{
    VisitSomeStmt(switchStmt, switchStmt->getCond(), "switch");
    return true;
}

bool UnnecessaryParen::VisitCaseStmt(const CaseStmt* caseStmt)
{
    VisitSomeStmt(caseStmt, caseStmt->getLHS(), "case");
    return true;
}

bool UnnecessaryParen::VisitReturnStmt(const ReturnStmt* returnStmt)
{
    if (ignoreLocation(returnStmt))
        return true;

    if (!returnStmt->getRetValue())
        return true;
    auto parenExpr = dyn_cast<ParenExpr>(ignoreAllImplicit(returnStmt->getRetValue()));
    if (!parenExpr)
        return true;
    if (parenExpr->getLocStart().isMacroID())
        return true;
    // assignments need extra parentheses or they generate a compiler warning
    auto binaryOp = dyn_cast<BinaryOperator>(parenExpr->getSubExpr());
    if (binaryOp && binaryOp->getOpcode() == BO_Assign)
        return true;

    // only non-operator-calls for now
    auto subExpr = parenExpr->getSubExpr();
    if (isa<CallExpr>(subExpr) && !isa<CXXOperatorCallExpr>(subExpr))
    {
        report(
            DiagnosticsEngine::Warning, "parentheses immediately inside return statement",
            parenExpr->getLocStart())
            << parenExpr->getSourceRange();
        handled_.insert(parenExpr);
    }
    return true;
}

void UnnecessaryParen::VisitSomeStmt(const Stmt * stmt, const Expr* cond, StringRef stmtName)
{
    if (ignoreLocation(stmt))
        return;

    auto parenExpr = dyn_cast<ParenExpr>(ignoreAllImplicit(cond));
    if (parenExpr) {
        if (parenExpr->getLocStart().isMacroID())
            return;
        // Used to silence -Wunreachable-code:
        if (isa<CXXBoolLiteralExpr>(parenExpr->getSubExpr())
            && stmtName == "if")
        {
            handled_.insert(parenExpr);
            return;
        }
        // assignments need extra parentheses or they generate a compiler warning
        auto binaryOp = dyn_cast<BinaryOperator>(parenExpr->getSubExpr());
        if (binaryOp && binaryOp->getOpcode() == BO_Assign)
            return;
        if (auto const opCall = dyn_cast<CXXOperatorCallExpr>(parenExpr->getSubExpr())) {
            if (opCall->getOperator() == OO_Equal) {
                return;
            }
        }
        report(
            DiagnosticsEngine::Warning, "parentheses immediately inside %0 statement",
            parenExpr->getLocStart())
            << stmtName
            << parenExpr->getSourceRange();
        handled_.insert(parenExpr);
    }
}

bool UnnecessaryParen::VisitCallExpr(const CallExpr* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    if (callExpr->getNumArgs() != 1 || isa<CXXOperatorCallExpr>(callExpr))
        return true;

    auto parenExpr = dyn_cast<ParenExpr>(ignoreAllImplicit(callExpr->getArg(0)));
    if (!parenExpr)
        return true;
    if (parenExpr->getLocStart().isMacroID())
        return true;
    // assignments need extra parentheses or they generate a compiler warning
    auto binaryOp = dyn_cast<BinaryOperator>(parenExpr->getSubExpr());
    if (binaryOp && binaryOp->getOpcode() == BO_Assign)
        return true;
    report(
        DiagnosticsEngine::Warning, "parentheses immediately inside single-arg call",
        parenExpr->getLocStart())
        << parenExpr->getSourceRange();
    handled_.insert(parenExpr);
    return true;
}

bool UnnecessaryParen::VisitCXXOperatorCallExpr(const CXXOperatorCallExpr* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    if (callExpr->getNumArgs() != 2)
        return true;

    // Same logic as CXXOperatorCallExpr::isAssignmentOp(), which our supported clang
    // doesn't have yet.
    auto Opc = callExpr->getOperator();
    if (Opc != OO_Equal && Opc != OO_StarEqual &&
        Opc != OO_SlashEqual && Opc != OO_PercentEqual &&
        Opc != OO_PlusEqual && Opc != OO_MinusEqual &&
        Opc != OO_LessLessEqual && Opc != OO_GreaterGreaterEqual &&
        Opc != OO_AmpEqual && Opc != OO_CaretEqual &&
        Opc != OO_PipeEqual)
        return true;

    auto parenExpr = dyn_cast<ParenExpr>(ignoreAllImplicit(callExpr->getArg(1)));
    if (!parenExpr)
        return true;
    if (parenExpr->getLocStart().isMacroID())
        return true;
    // Sometimes parentheses make the RHS of an assignment easier to read by
    // visually disambiguating the = from a call to ==
    auto sub = parenExpr->getSubExpr();
    if (isa<BinaryOperator>(sub)
        || isa<CXXOperatorCallExpr>(sub)
        || isa<ConditionalOperator>(sub))
        return true;

    report(
        DiagnosticsEngine::Warning, "parentheses immediately inside assignment",
        parenExpr->getLocStart())
        << parenExpr->getSourceRange();
    handled_.insert(parenExpr);
    return true;
}

bool UnnecessaryParen::VisitVarDecl(const VarDecl* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    if (!varDecl->getInit())
        return true;

    auto parenExpr = dyn_cast<ParenExpr>(ignoreAllImplicit(varDecl->getInit()));
    if (!parenExpr)
        return true;
    if (parenExpr->getLocStart().isMacroID())
        return true;
    auto sub = parenExpr->getSubExpr();
    if (isa<BinaryOperator>(sub)
        || isa<CXXOperatorCallExpr>(sub)
        || isa<ConditionalOperator>(sub)
            // these two are for "parentheses were disambiguated as a function declaration [-Werror,-Wvexing-parse]"
        || isa<CXXBindTemporaryExpr>(sub)
        || isa<CXXFunctionalCastExpr>(sub))
        return true;

    report(
        DiagnosticsEngine::Warning, "parentheses immediately inside vardecl statement",
        parenExpr->getLocStart())
        << parenExpr->getSourceRange();
    handled_.insert(parenExpr);
    return true;
}

bool UnnecessaryParen::VisitMemberExpr(const MemberExpr* memberExpr)
{
    if (ignoreLocation(memberExpr))
        return true;

    auto parenExpr = dyn_cast<ParenExpr>(ignoreAllImplicit(memberExpr->getBase()));
    if (!parenExpr)
        return true;
    if (handled_.find(parenExpr) != handled_.end())
        return true;
    if (parenExpr->getLocStart().isMacroID())
        return true;

    auto sub = parenExpr->getSubExpr();
    if (isa<CallExpr>(sub)) {
        if (isa<CXXOperatorCallExpr>(sub))
           return true;
    } else if (isa<CXXConstructExpr>(sub)) {
        // warn
    } else if (isa<MemberExpr>(sub)) {
        // warn
    } else if (isa<DeclRefExpr>(sub)) {
        // warn
    } else
        return true;

    report(
        DiagnosticsEngine::Warning, "unnecessary parentheses around member expr",
        parenExpr->getLocStart())
        << parenExpr->getSourceRange();
    handled_.insert(parenExpr);
    return true;
}

bool UnnecessaryParen::isPrecededBy_BAD_CAST(Expr const * expr) {
    if (expr->getLocStart().isMacroID()) {
        return false;
    }
    SourceManager& SM = compiler.getSourceManager();
    const char *p1 = SM.getCharacterData( expr->getLocStart().getLocWithOffset(-10) );
    const char *p2 = SM.getCharacterData( expr->getLocStart() );
    return std::string(p1, p2 - p1).find("BAD_CAST") != std::string::npos;
}

loplugin::Plugin::Registration< UnnecessaryParen > X("unnecessaryparen", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
