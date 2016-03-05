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
#include "plugin.hxx"
#include "compat.hxx"

/**
  Check for calls to CPPUNIT_ASSERT when it should be using CPPUNIT_ASSERT_EQUALS
*/

namespace {

class CppunitAssertEquals:
    public RecursiveASTVisitor<CppunitAssertEquals>, public loplugin::Plugin
{
public:
    explicit CppunitAssertEquals(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCallExpr(const CallExpr*);
    bool VisitBinaryOperator(const BinaryOperator*);
private:
    void checkExpr(const Stmt* stmt);
};

bool CppunitAssertEquals::VisitCallExpr(const CallExpr* callExpr)
{
    if (ignoreLocation(callExpr)) {
        return true;
    }
    if (callExpr->getDirectCallee() == nullptr) {
        return true;
    }
    const FunctionDecl* functionDecl = callExpr->getDirectCallee()->getCanonicalDecl();
    if (functionDecl->getOverloadedOperator() != OO_EqualEqual) {
        return true;
    }
    checkExpr(callExpr);
    return true;
}

bool CppunitAssertEquals::VisitBinaryOperator(const BinaryOperator* binaryOp)
{
    if (ignoreLocation(binaryOp)) {
        return true;
    }
    if (binaryOp->getOpcode() != BO_EQ) {
        return true;
    }
    checkExpr(binaryOp);
    return true;
}

/*
 * check that we are not a compound expression
 */
void CppunitAssertEquals::checkExpr(const Stmt* stmt)
{
    const Stmt* parent = parentStmt(stmt);
    if (!parent || !isa<ParenExpr>(parent)) {
        return;
    }
    parent = parentStmt(parent);
    if (!parent || !isa<UnaryOperator>(parent)) {
        return;
    }
    parent = parentStmt(parent);
    if (!parent || !isa<CallExpr>(parent)) {
        return;
    }
    const CallExpr* callExpr = dyn_cast<CallExpr>(parent);
    if (!callExpr || callExpr->getDirectCallee() == nullptr) {
        return;
    }
    const FunctionDecl* functionDecl = callExpr->getDirectCallee()->getCanonicalDecl();
    if (!functionDecl || functionDecl->getQualifiedNameAsString().find("Asserter::failIf") == std::string::npos) {
        return;
    }
    report(
        DiagnosticsEngine::Warning, "rather call CPPUNIT_ASSERT_EQUALS",
        stmt->getSourceRange().getBegin())
      << stmt->getSourceRange();
}

loplugin::Plugin::Registration< CppunitAssertEquals > X("cppunitassertequals", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
