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

#include "plugin.hxx"
#include "compat.hxx"
#include "clang/AST/CXXInheritance.h"

// Idea from tml.
// Check for OUString variables that are
//   (1) initialised from a string literal
//   (2) only used in one spot
// In which case, we might as well inline it.

namespace
{

class OnceVar:
    public RecursiveASTVisitor<OnceVar>, public loplugin::Plugin
{
public:
    explicit OnceVar(InstantiationData const & data): Plugin(data) {}

    virtual void run() override {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool TraverseFunctionDecl( FunctionDecl* stmt );
    bool VisitDeclRefExpr( const DeclRefExpr* );

private:
    bool mbChecking;
    std::map<SourceLocation,int> maVarUsesMap;
    std::map<SourceLocation,SourceRange> maVarDeclSourceRangeMap;
};


bool OnceVar::TraverseFunctionDecl(FunctionDecl * decl)
{
    if (ignoreLocation(decl)) {
        return true;
    }
    if (!decl->hasBody()) {
        return true;
    }
    if ( decl != decl->getCanonicalDecl() ) {
        return true;
    }
    maVarUsesMap.clear();
    maVarDeclSourceRangeMap.clear();
    mbChecking = true;
    TraverseStmt(decl->getBody());
    mbChecking = false;
    for (auto it = maVarUsesMap.cbegin(); it != maVarUsesMap.cend(); ++it)
    {
        if (it->second == 1)
        {
            report(DiagnosticsEngine::Warning,
                    "OUString var used only once, should be inlined",
                   it->first)
                << maVarDeclSourceRangeMap[it->first];
        }
    }
    return true;
}

bool OnceVar::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    if (!mbChecking)
        return true;
    const Decl* decl = declRefExpr->getDecl();
    if (!isa<VarDecl>(decl) || isa<ParmVarDecl>(decl)) {
        return true;
    }
    const VarDecl * varDecl = dyn_cast<VarDecl>(decl)->getCanonicalDecl();
    if (!varDecl->hasInit() || varDecl->hasGlobalStorage()) {
        return true;
    }
    if (varDecl->getType().getUnqualifiedType().getAsString().find("OUString") == std::string::npos) {
        return true;
    }
    const CXXConstructExpr* constructExpr = dyn_cast<CXXConstructExpr>(varDecl->getInit());
    if (!constructExpr || constructExpr->getNumArgs() < 1) {
        return true;
    }
    if (!isa<StringLiteral>(constructExpr->getArg(0))) {
        return true;
    }

    SourceLocation loc = varDecl->getLocation();
    if (maVarUsesMap.find(loc) == maVarUsesMap.end()) {
        maVarUsesMap[loc] = 1;
        maVarDeclSourceRangeMap[loc] = varDecl->getSourceRange();
    } else {
        maVarUsesMap[loc]++;
    }
    return true;
}

loplugin::Plugin::Registration< OnceVar > X("oncevar");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
