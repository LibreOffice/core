
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <iostream>
#include <unordered_set>

#include "plugin.hxx"
#include "check.hxx"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/StmtVisitor.h"

/*
 Mike Kaganski found a bug where the code was looping over a block and
 not using the index var, and the loop was unnecessary.
 So he wanted to have a look for other places like that.
*/
namespace
{
class UnusedIndex : public RecursiveASTVisitor<UnusedIndex>, public loplugin::Plugin
{
public:
    explicit UnusedIndex(loplugin::InstantiationData const& data)
        : Plugin(data)
    {
    }

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool TraverseForStmt(ForStmt* stmt);
    bool VisitDeclRefExpr(DeclRefExpr const* stmt);

private:
    VarDecl const* mLoopVarDecl = nullptr;
    std::unordered_set<VarDecl const*> mFoundSet;
};

bool UnusedIndex::TraverseForStmt(ForStmt* stmt)
{
    if (ignoreLocation(stmt))
        return true;
    auto savedCopy = mLoopVarDecl;

    mLoopVarDecl = nullptr;
    if (stmt->getInit())
    {
        auto declStmt = dyn_cast<DeclStmt>(stmt->getInit());
        if (declStmt && declStmt->isSingleDecl())
        {
            auto varDecl = dyn_cast<VarDecl>(declStmt->getSingleDecl());
            if (varDecl)
                mLoopVarDecl = varDecl;
        }
    }

    // deliberately ignore the other parts of the for stmt, except for the body
    auto ret = RecursiveASTVisitor::TraverseStmt(stmt->getBody());

    if (mLoopVarDecl && mFoundSet.erase(mLoopVarDecl) == 0)
        report(DiagnosticsEngine::Warning, "loop variable not used", mLoopVarDecl->getLocStart())
            << mLoopVarDecl->getSourceRange();
    mLoopVarDecl = savedCopy;
    return ret;
}

bool UnusedIndex::VisitDeclRefExpr(DeclRefExpr const* stmt)
{
    auto varDecl = dyn_cast<VarDecl>(stmt->getDecl());
    if (!varDecl)
        return true;
    if (mLoopVarDecl && mLoopVarDecl == varDecl)
        mFoundSet.insert(varDecl);
    return true;
}

loplugin::Plugin::Registration<UnusedIndex> X("unusedindex", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
