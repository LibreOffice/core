/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "check.hxx"

/**
  Check for calls to CPPUNIT_ASSERT when it should be using CPPUNIT_ASSERT_EQUALS
*/

namespace {

class CppunitAssertEquals:
    public RecursiveASTVisitor<CppunitAssertEquals>, public loplugin::Plugin
{
public:
    explicit CppunitAssertEquals(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCallExpr(const CallExpr*);

private:
    void checkExpr(
        SourceRange range, StringRef name, Expr const * expr, bool negated);

    void reportEquals(SourceRange range, StringRef name, bool negative);
};

bool CppunitAssertEquals::VisitCallExpr(const CallExpr* callExpr)
{
    auto const decl = callExpr->getDirectCallee();
    if (decl == nullptr
        || !(loplugin::DeclCheck(decl).Function("failIf").Struct("Asserter")
             .Namespace("CppUnit").GlobalNamespace()))
    {
        return true;
    }
    // Don't use callExpr->getLocStart() or callExpr->getExprLoc(), as those
    // fall into a nested use of the CPPUNIT_NS macro; CallExpr::getRParenLoc
    // happens to be readily available and cause good results:
    auto loc = callExpr->getRParenLoc();
    while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
        loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
    }
    if (!compiler.getSourceManager().isMacroBodyExpansion(loc)
        || ignoreLocation(
            compiler.getSourceManager().getImmediateMacroCallerLoc(loc)))
    {
        return true;
    }
    auto name = Lexer::getImmediateMacroName(
        loc, compiler.getSourceManager(), compiler.getLangOpts());
    if (name != "CPPUNIT_ASSERT" && name != "CPPUNIT_ASSERT_MESSAGE") {
        return true;
    }
    if (decl->getNumParams() != 3) {
        report(
            DiagnosticsEngine::Warning,
            ("TODO: suspicious CppUnit::Asserter::failIf call with %0"
             " parameters"),
            callExpr->getExprLoc())
            << decl->getNumParams() << callExpr->getSourceRange();
        return true;
    }
    auto const e1 = callExpr->getArg(0)->IgnoreParenImpCasts();
    Expr const * e2 = nullptr;
    if (auto const e3 = dyn_cast<UnaryOperator>(e1)) {
        if (e3->getOpcode() == UO_LNot) {
            e2 = e3->getSubExpr();
        }
    } else if (auto const e4 = dyn_cast<CXXOperatorCallExpr>(e1)) {
        if (e4->getOperator() == OO_Exclaim) {
            e2 = e4->getArg(0);
        }
    }
    if (e2 == nullptr) {
        report(
            DiagnosticsEngine::Warning,
            ("TODO: suspicious CppUnit::Asserter::failIf call not wrapping"
             " !(...)"),
            callExpr->getExprLoc())
            << callExpr->getSourceRange();
        return true;
    }
    auto range = compiler.getSourceManager().getImmediateExpansionRange(loc);
    checkExpr(
        SourceRange(range.first, range.second), name,
        e2->IgnoreParenImpCasts(), false);
    return true;
}

void CppunitAssertEquals::checkExpr(
    SourceRange range, StringRef name, Expr const * expr, bool negated)
{
    if (auto const e = dyn_cast<UnaryOperator>(expr)) {
        if (e->getOpcode() == UO_LNot) {
            checkExpr(
                range, name, e->getSubExpr()->IgnoreParenImpCasts(), !negated);
        }
        return;
    }
    if (auto const e = dyn_cast<BinaryOperator>(expr)) {
        auto const op = e->getOpcode();
        if ((!negated && op == BO_EQ) || (negated && op == BO_NE)) {
            reportEquals(range, name, op == BO_NE);
            return;
        }
#if 0 // TODO: enable later
        if ((!negated && op == BO_LAnd) || (negated && op == BO_LOr)) {
            report(
                DiagnosticsEngine::Warning,
                "rather split into two %0", e->getExprLoc())
                << name << range;
            return;
        }
#endif
        return;
    }
    if (auto const e = dyn_cast<CXXOperatorCallExpr>(expr)) {
        auto const op = e->getOperator();
        if ((!negated && op == OO_EqualEqual)
            || (negated && op == OO_ExclaimEqual))
        {
            reportEquals(range, name, op == OO_ExclaimEqual);
            return;
        }
        return;
    }
}

void CppunitAssertEquals::reportEquals(
    SourceRange range, StringRef name, bool negative)
{
    report(
        DiagnosticsEngine::Warning,
        ("rather call"
         " %select{CPPUNIT_ASSERT_EQUAL|CPPUNIT_ASSERT_EQUAL_MESSAGE}0 (or"
         " rewrite as an explicit operator %select{==|!=}1 call when the"
         " operator itself is the topic)"),
        range.getBegin())
        << (name == "CPPUNIT_ASSERT_MESSAGE") << negative << range;
}

loplugin::Plugin::Registration< CppunitAssertEquals > X("cppunitassertequals");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
