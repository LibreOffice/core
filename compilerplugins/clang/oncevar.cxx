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
#include "typecheck.hxx"
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
    explicit OnceVar(InstantiationData const & data): Plugin(data), mbChecking(false) {}

    virtual void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool TraverseFunctionDecl( FunctionDecl* stmt );
    bool VisitDeclRefExpr( const DeclRefExpr* );

private:
    bool mbChecking;
    std::map<SourceLocation,int> maVarUsesMap;
    std::map<SourceLocation,SourceRange> maVarDeclSourceRangeMap;
    std::map<SourceLocation,SourceRange> maVarUseSourceRangeMap;
    StringRef getFilename(SourceLocation loc);
};

StringRef OnceVar::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name;
}

bool OnceVar::TraverseFunctionDecl(FunctionDecl * decl)
{
    assert(!mbChecking);
    if (ignoreLocation(decl)) {
        return true;
    }
    if (!decl->hasBody()) {
        return true;
    }
    if ( decl != decl->getCanonicalDecl() ) {
        return true;
    }

    // some places, it makes the code worse
    StringRef aFileName = getFilename(decl->getBody()->getLocStart());
    if (aFileName.startswith(SRCDIR "/sal/qa/osl/module/osl_Module.cxx")) {
        return true;
    }
    // #ifdefs confuse things here
    if (aFileName.startswith(SRCDIR "/sal/osl/unx/thread.cxx")
        || aFileName.startswith(SRCDIR "/sot/source/base/formats.cxx")
        || aFileName.startswith(SRCDIR "/desktop/source/app/check_ext_deps.cxx")) {
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
                    "var used only once, should be inlined",
                   it->first)
                << maVarDeclSourceRangeMap[it->first];
            report(DiagnosticsEngine::Note,
                   "to this spot",
                   maVarUseSourceRangeMap[it->first].getBegin())
                << maVarUseSourceRangeMap[it->first];
        }
    }
    return true;
}

static bool checkForPassByReference(const CallExpr* callExpr, const Stmt* declRefExpr)
{
    const FunctionDecl* callee = callExpr->getDirectCallee();
    if (!callee) {
        return true;
    }
    for (unsigned i=0; i<callExpr->getNumArgs(); ++i) {
        if (callExpr->getArg(i) == declRefExpr) {
           if (i >= callee->getNumParams()) {
               return true;
           }
           if (callee->getParamDecl(i)->getType()->isReferenceType()) {
               return true;
           }
           break;
       }
   }
   return false;
}

static bool checkForPassByReference(const CXXConstructExpr* constructExpr, const Stmt* declRefExpr)
{
    const CXXConstructorDecl* callee = constructExpr->getConstructor();
    for (unsigned i=0; i<constructExpr->getNumArgs(); ++i) {
        if (constructExpr->getArg(i) == declRefExpr) {
           if (i >= callee->getNumParams()) {
               return true;
           }
           if (callee->getParamDecl(i)->getType()->isReferenceType()) {
               return true;
           }
           break;
       }
   }
   return false;
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
    if (varDecl->hasGlobalStorage()) {
        return true;
    }
    const bool bOUStringType = (bool) loplugin::TypeCheck(varDecl->getType()).Class("OUString").Namespace("rtl");
    const bool bScalarType = varDecl->getType().getUnqualifiedType()->isScalarType();
    if (!bOUStringType && !bScalarType) {
        return true;
    }
    if (!varDecl->hasInit()) {
         return true;
    }
    if (bOUStringType) {
        const CXXConstructExpr* constructExpr = dyn_cast<CXXConstructExpr>(varDecl->getInit());
        if (!constructExpr || constructExpr->getNumArgs() < 1) {
            return true;
        }
        if (!isa<StringLiteral>(constructExpr->getArg(0))) {
            return true;
        }
    } else {
        if (loplugin::TypeCheck(varDecl->getType()).Const()) {
            return true;
        }
        if (!varDecl->getType()->isArithmeticType()) {
            return true;
        }
        if (!varDecl->getInit()->isEvaluatable(compiler.getASTContext())) {
            return true;
        }
        APFloat res(0.0);
        if (!varDecl->getInit()->EvaluateAsFloat(res, compiler.getASTContext())) {
            return true;
        }
    }

    SourceLocation loc = varDecl->getLocation();

    // ignore cases like:
    //     const OUString("xxx") xxx;
    //     rtl_something(xxx.pData);
    // and
    //      foo(&xxx);
    // where we cannot inline the declaration.
    const Stmt* parent = parentStmt(declRefExpr);
    if (parent) {
        if (isa<MemberExpr>(parent) || isa<UnaryOperator>(parent)) {
            maVarUsesMap[loc] = 2;
            return true;
        }
        // check for var being passed by reference
        if (isa<CallExpr>(parent)) {
            const CallExpr* callExpr = dyn_cast<CallExpr>(parent);
            if (checkForPassByReference(callExpr, declRefExpr)) {
                maVarUsesMap[loc] = 2;
                return true;
            }
        }
        else if (isa<CXXConstructExpr>(parent)) {
            const CXXConstructExpr* constructExpr = dyn_cast<CXXConstructExpr>(parent);
            if (checkForPassByReference(constructExpr, declRefExpr)) {
                maVarUsesMap[loc] = 2;
                return true;
            }
        }
        else if (isa<ImplicitCastExpr>(parent)) {
            const Stmt* parent2 = parentStmt(parent);
            if (parent2 && isa<CallExpr>(parent2)) {
                const CallExpr* callExpr = dyn_cast<CallExpr>(parent2);
                if (checkForPassByReference(callExpr, parent)) {
                    maVarUsesMap[loc] = 2;
                    return true;
                }
            }
        }
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
