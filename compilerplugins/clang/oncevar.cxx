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
#include <unordered_map>

#include "plugin.hxx"
#include "check.hxx"
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
        for (auto it = maVarUsesMap.cbegin(); it != maVarUsesMap.cend(); ++it)
        {
            if (it->second == 1)
            {
                report(DiagnosticsEngine::Warning,
                        "OUString var used only once, should be inlined",
                       it->first)
                    << maVarDeclSourceRangeMap[it->first];
                report(DiagnosticsEngine::Note,
                       "to this spot",
                       maVarUseSourceRangeMap[it->first].getBegin())
                    << maVarUseSourceRangeMap[it->first];
            }
        }
    }

    bool VisitDeclRefExpr( const DeclRefExpr* );

private:
    StringRef getFilename(SourceLocation loc);

    struct SourceLocationHash
    {
        size_t operator()( SourceLocation const & sl ) const
        {
            return sl.getRawEncoding();
        }
    };
    std::unordered_map<SourceLocation, int, SourceLocationHash> maVarUsesMap;
    std::unordered_map<SourceLocation, SourceRange, SourceLocationHash> maVarDeclSourceRangeMap;
    std::unordered_map<SourceLocation, SourceRange, SourceLocationHash> maVarUseSourceRangeMap;
};

StringRef OnceVar::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name;
}

bool OnceVar::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    if (ignoreLocation(declRefExpr)) {
        return true;
    }
    const Decl* decl = declRefExpr->getDecl();
    if (!isa<VarDecl>(decl) || isa<ParmVarDecl>(decl)) {
        return true;
    }
    const VarDecl * varDecl = dyn_cast<VarDecl>(decl)->getCanonicalDecl();
    if (!varDecl->hasInit()) {
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

    // ignore cases like:
    //     const OUString("xxx") xxx;
    //     rtl_something(xxx.pData);
    // and
    //      foo(&xxx);
    // where we cannot inline the declaration.
    auto const tc = loplugin::TypeCheck(varDecl->getType());
    if (tc.Class("OUString").Namespace("rtl").GlobalNamespace()
        && (isa<MemberExpr>(parentStmt(declRefExpr))
            || isa<UnaryOperator>(parentStmt(declRefExpr))))
    {
        maVarUsesMap[loc] = 2;
        return true;
    }

    if (maVarUsesMap.find(loc) == maVarUsesMap.end()) {
        maVarUsesMap[loc] = 1;
        maVarDeclSourceRangeMap[loc] = varDecl->getSourceRange();
        maVarUseSourceRangeMap[loc] = declRefExpr->getSourceRange();
    } else {
        maVarUsesMap[loc]++;
    }
    return true;
}

loplugin::Plugin::Registration< OnceVar > X("oncevar");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
