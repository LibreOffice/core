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
#include <map>
#include <set>

#include "plugin.hxx"
#include "compat.hxx"
#include "clang/AST/CXXInheritance.h"

// Check for local variables that we are calling delete on

namespace
{

class MemoryVar:
    public RecursiveASTVisitor<MemoryVar>, public loplugin::Plugin
{
public:
    explicit MemoryVar(InstantiationData const & data): Plugin(data) {}

    virtual void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseFunctionDecl(FunctionDecl*);
    bool VisitCXXDeleteExpr(const CXXDeleteExpr*);
    bool VisitCXXNewExpr(const CXXNewExpr* );
    bool VisitBinaryOperator(const BinaryOperator*);
    bool VisitReturnStmt(const ReturnStmt*);

private:
    bool mbChecking;
    std::set<SourceLocation> maVarUsesSet;
    std::set<SourceLocation> maVarNewSet;
    std::set<SourceLocation> maVarIgnoreSet;
    std::map<SourceLocation,SourceRange> maVarDeclSourceRangeMap;
    std::map<SourceLocation,SourceRange> maVarDeleteSourceRangeMap;
    StringRef getFilename(SourceLocation loc);
};

StringRef MemoryVar::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name;
}

bool MemoryVar::TraverseFunctionDecl(FunctionDecl * decl)
{
    if (ignoreLocation(decl)) {
        return true;
    }
    if (!decl->hasBody() || !decl->isThisDeclarationADefinition()) {
        return true;
    }

    maVarUsesSet.clear();
    maVarNewSet.clear();
    maVarIgnoreSet.clear();
    maVarDeclSourceRangeMap.clear();
    maVarDeleteSourceRangeMap.clear();

    assert(!mbChecking);
    mbChecking = true;
    TraverseStmt(decl->getBody());
    mbChecking = false;

    for (const auto& varLoc : maVarUsesSet)
    {
        // checking the location of the var instead of the function because for some reason
        // I'm not getting accurate results from clang right now
        StringRef aFileName = getFilename(varLoc);
        // TODO these files are doing some weird stuff I don't know how to ignore yet
        if (aFileName.startswith(SRCDIR "/vcl/source/filter")) {
           return true;
        }
        if (aFileName.startswith(SRCDIR "/sw/source/core/layout/frmtool.cxx")) {
           return true;
        }


        if (maVarNewSet.find(varLoc) == maVarNewSet.end())
            continue;
        if (maVarIgnoreSet.find(varLoc) != maVarIgnoreSet.end())
            continue;

        report(DiagnosticsEngine::Warning,
                "calling new and delete on a local var, rather use std::unique_ptr",
               varLoc)
            << maVarDeclSourceRangeMap[varLoc];
        report(DiagnosticsEngine::Note,
               "delete called here",
               maVarDeleteSourceRangeMap[varLoc].getBegin())
            << maVarDeleteSourceRangeMap[varLoc];
        cout << "xxxx " << aFileName.str() << endl;
    }
    return true;
}

bool MemoryVar::VisitCXXDeleteExpr(const CXXDeleteExpr *deleteExpr)
{
    if (!mbChecking)
        return true;
    if (ignoreLocation(deleteExpr)) {
        return true;
    }
    const Expr* argumentExpr = deleteExpr->getArgument();
    if (isa<CastExpr>(argumentExpr)) {
        argumentExpr = dyn_cast<CastExpr>(argumentExpr)->getSubExpr();
    }
    const DeclRefExpr* declRefExpr = dyn_cast<DeclRefExpr>(argumentExpr);
    if (!declRefExpr)
        return true;
    const Decl* decl = declRefExpr->getDecl();
    if (!isa<VarDecl>(decl) || isa<ParmVarDecl>(decl)) {
        return true;
    }
    const VarDecl * varDecl = dyn_cast<VarDecl>(decl)->getCanonicalDecl();
    if (varDecl->hasGlobalStorage()) {
        return true;
    }

    SourceLocation loc = varDecl->getLocation();

    if (maVarUsesSet.find(loc) == maVarUsesSet.end()) {
        maVarUsesSet.insert(loc);
        maVarDeclSourceRangeMap[loc] = varDecl->getSourceRange();
        maVarDeleteSourceRangeMap[loc] = declRefExpr->getSourceRange();
    }
    return true;
}

bool MemoryVar::VisitCXXNewExpr(const CXXNewExpr *newExpr)
{
    if (!mbChecking)
        return true;
    if (ignoreLocation(newExpr)) {
        return true;
    }
    const Stmt* stmt = parentStmt(newExpr);

    const DeclStmt* declStmt = dyn_cast<DeclStmt>(stmt);
    if (declStmt) {
        const VarDecl* varDecl = dyn_cast<VarDecl>(declStmt->getSingleDecl());
        if (varDecl) {
            varDecl = varDecl->getCanonicalDecl();
            SourceLocation loc = varDecl->getLocation();
            maVarNewSet.insert(loc);
        }
        return true;
    }

    const BinaryOperator* binaryOp = dyn_cast<BinaryOperator>(stmt);
    if (binaryOp && binaryOp->getOpcode() == BO_Assign) {
        const DeclRefExpr* declRefExpr = dyn_cast<DeclRefExpr>(binaryOp->getLHS());
        if (declRefExpr) {
            const VarDecl* varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl());
            if (varDecl) {
                varDecl = varDecl->getCanonicalDecl();
                SourceLocation loc = varDecl->getLocation();
                maVarNewSet.insert(loc);
            }
        }
    }
    return true;
}

// Ignore cases where the variable in question is assigned to another variable
bool MemoryVar::VisitBinaryOperator(const BinaryOperator *binaryOp)
{
    if (!mbChecking)
        return true;
    if (ignoreLocation(binaryOp)) {
        return true;
    }
    if (binaryOp->getOpcode() != BO_Assign) {
        return true;
    }
    const Expr* expr = binaryOp->getRHS();
    // unwrap casts
    while (isa<CastExpr>(expr)) {
        expr = dyn_cast<CastExpr>(expr)->getSubExpr();
    }
    const DeclRefExpr* declRefExpr = dyn_cast<DeclRefExpr>(expr);
    if (!declRefExpr) {
        return true;
    }
    const VarDecl* varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl());
    if (!varDecl) {
        return true;
    }
    varDecl = varDecl->getCanonicalDecl();
    maVarIgnoreSet.insert(varDecl->getLocation());
    return true;
}

// Ignore cases where the variable in question is returned from a function
bool MemoryVar::VisitReturnStmt(const ReturnStmt *returnStmt)
{
    if (!mbChecking)
        return true;
    if (ignoreLocation(returnStmt)) {
        return true;
    }
    const Expr* expr = returnStmt->getRetValue();
    if (!expr) {
        return true;
    }
    // unwrap casts
    while (isa<CastExpr>(expr)) {
        expr = dyn_cast<CastExpr>(expr)->getSubExpr();
    }
    const DeclRefExpr* declRefExpr = dyn_cast<DeclRefExpr>(expr);
    if (!declRefExpr) {
        return true;
    }
    const VarDecl* varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl());
    if (!varDecl) {
        return true;
    }
    varDecl = varDecl->getCanonicalDecl();
    maVarIgnoreSet.insert(varDecl->getLocation());
    return true;
}

loplugin::Plugin::Registration< MemoryVar > X("memoryvar");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
