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

#include "plugin.hxx"
#include "compat.hxx"
#include "clang/AST/CXXInheritance.h"

// Idea from bubli. Check that the index variable in a for loop is able to cover the range
// revealed by the terminating condition.
// If not, we might end up in an endless loop, or just not process certain parts.

namespace
{

class LoopVarTooSmall:
    public RecursiveASTVisitor<LoopVarTooSmall>, public loplugin::Plugin
{
public:
    explicit LoopVarTooSmall(InstantiationData const & data): Plugin(data) {}

    virtual void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitForStmt( const ForStmt* stmt );

private:
    StringRef getFilename(SourceLocation loc);

};

StringRef LoopVarTooSmall::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name;
}

bool LoopVarTooSmall::VisitForStmt( const ForStmt* stmt )
{
    if (ignoreLocation( stmt ))
        return true;
    // ignore SAL for now
    StringRef aFileName = getFilename(stmt->getLocStart());
    if (aFileName.startswith(SRCDIR "/sal/")) {
        return true;
    }

    const Stmt* initStmt = stmt->getInit();
    if (!initStmt || !isa<DeclStmt>(initStmt))
        return true;
    const DeclStmt* declStmt = dyn_cast<DeclStmt>(initStmt);
    if (!declStmt->getDeclGroup().isSingleDecl())
        return true;
    const Decl* decl = declStmt->getSingleDecl();
    if (!decl || !isa<VarDecl>(decl))
        return true;
    const VarDecl* varDecl = dyn_cast<VarDecl>(decl);
    QualType qt = varDecl->getType();
    if (!qt->isIntegralType(compiler.getASTContext()))
        return true;
    uint64_t qt1BitWidth = compiler.getASTContext().getTypeSize(qt);

    if (!stmt->getCond() || !isa<BinaryOperator>(stmt->getCond()))
        return true;
    const BinaryOperator* binOp = dyn_cast<BinaryOperator>(stmt->getCond());
    if (binOp->getOpcode() != BO_LT && binOp->getOpcode() != BO_LE)
        return true;
    const Expr* binOpRHS = binOp->getRHS();
    // ignore complex expressions for now
    if (isa<BinaryOperator>(binOpRHS))
        return true;
    if (isa<ImplicitCastExpr>(binOpRHS)) {
        const ImplicitCastExpr* castExpr = dyn_cast<ImplicitCastExpr>(binOpRHS);
        binOpRHS = castExpr->getSubExpr();
    }
    QualType qt2 = binOpRHS->getType();
    if (!qt2->isIntegralType(compiler.getASTContext()))
        return true;
    // check for comparison with constants where the compiler just tends to give me the type as "int"
    llvm::APSInt aIntResult;
    uint64_t qt2BitWidth = compiler.getASTContext().getTypeSize(qt2);
    if (binOpRHS->EvaluateAsInt(aIntResult, compiler.getASTContext())) {
        if (aIntResult.getSExtValue() > 0 && aIntResult.getSExtValue() < 1<<7) {
            qt2BitWidth = 8;
        } else if (aIntResult.getSExtValue() > 0 && aIntResult.getSExtValue() < 1<<15) {
            qt2BitWidth = 16;
        } else if (aIntResult.getSExtValue() > 0 && aIntResult.getSExtValue() < 1L<<31) {
            qt2BitWidth = 32;
        }
    }

    if (qt1BitWidth < qt2BitWidth) {
        report(
            DiagnosticsEngine::Warning,
            "loop index type is smaller than length type. " + qt.getAsString() + " < " + qt2.getAsString(),
            stmt->getLocStart())
                << stmt->getSourceRange();
//        stmt->getCond()->dump();
    }
    return true;
}


loplugin::Plugin::Registration< LoopVarTooSmall > X("loopvartoosmall", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
