/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "plugin.hxx"
#include <clang/Lex/Lexer.h>
#include <fstream>
#include <set>

namespace loplugin
{
/*
This is a compile check.

Check that DBG_UNHANDLED_EXCEPTION is always the first statement in a catch block, otherwise
it does not work properly.
*/

class DbgUnhandledException : public loplugin::FilteringPlugin<DbgUnhandledException>
{
public:
    explicit DbgUnhandledException(InstantiationData const& data);
    virtual void run() override;
    bool VisitCallExpr(CallExpr const* call);
    bool TraverseCXXCatchStmt(CXXCatchStmt*);
    bool PreTraverseCXXCatchStmt(CXXCatchStmt*);
    bool PostTraverseCXXCatchStmt(CXXCatchStmt*, bool traverseOk);

private:
    std::stack<CXXCatchStmt const*> currCatchStmt;
};

DbgUnhandledException::DbgUnhandledException(const InstantiationData& data)
    : FilteringPlugin(data)
{
}

void DbgUnhandledException::run()
{
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
}

bool DbgUnhandledException::PreTraverseCXXCatchStmt(CXXCatchStmt* catchStmt)
{
    currCatchStmt.push(catchStmt);
    return true;
}

bool DbgUnhandledException::PostTraverseCXXCatchStmt(CXXCatchStmt* catchStmt, bool)
{
    assert(currCatchStmt.top() == catchStmt);
    currCatchStmt.pop();
    return true;
}

bool DbgUnhandledException::TraverseCXXCatchStmt(CXXCatchStmt* catchStmt)
{
    if (!PreTraverseCXXCatchStmt(catchStmt))
        return false;
    bool ret = RecursiveASTVisitor::TraverseCXXCatchStmt(catchStmt);
    if (!PostTraverseCXXCatchStmt(catchStmt, ret))
        return false;
    return ret;
}

bool DbgUnhandledException::VisitCallExpr(const CallExpr* call)
{
    if (ignoreLocation(call))
        return true;
    const FunctionDecl* func = call->getDirectCallee();
    if (!func)
        return true;

    if (!func->getIdentifier() || func->getName() != "DbgUnhandledException")
        return true;

    if (currCatchStmt.empty())
    {
        report(DiagnosticsEngine::Warning, "DBG_UNHANDLED_EXCEPTION outside catch block",
               compat::getBeginLoc(call));
        return true;
    }
    auto catchBlock = dyn_cast<CompoundStmt>(currCatchStmt.top()->getHandlerBlock());
    if (!catchBlock)
    {
        report(DiagnosticsEngine::Warning,
               "something wrong with DBG_UNHANDLED_EXCEPTION, no CompoundStmt?",
               compat::getBeginLoc(call));
        return true;
    }
    if (catchBlock->size() < 1)
    {
        report(DiagnosticsEngine::Warning,
               "something wrong with DBG_UNHANDLED_EXCEPTION, CompoundStmt size == 0?",
               compat::getBeginLoc(call));
        return true;
    }

    Stmt const* firstStmt = *catchBlock->body_begin();
    if (auto exprWithCleanups = dyn_cast<ExprWithCleanups>(firstStmt))
        firstStmt = exprWithCleanups->getSubExpr();
    if (firstStmt != call)
    {
        report(DiagnosticsEngine::Warning,
               "DBG_UNHANDLED_EXCEPTION must be first statement in catch block",
               compat::getBeginLoc(call));
    }
    return true;
}

static Plugin::Registration<DbgUnhandledException> dbgunhandledexception("dbgunhandledexception");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
