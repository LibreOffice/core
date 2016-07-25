/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <set>
#include <iostream>
#include <fstream>

#include "clang/AST/Attr.h"

#include "plugin.hxx"
#include "compat.hxx"

/*
 Find methods with default params, where the callers never specify the default param i.e.
 might as well remove it.

 The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unuseddefaultparams' check
  $ ./compilerplugins/clang/unuseddefaultparams.py
*/

namespace {

struct MyFuncInfo
{
    std::string access;
    std::string returnType;
    std::string nameAndParams;
    std::string sourceLocation;
};
bool operator < (const MyFuncInfo &lhs, const MyFuncInfo &rhs)
{
    return std::tie(lhs.returnType, lhs.nameAndParams)
         < std::tie(rhs.returnType, rhs.nameAndParams);
}


// try to limit the voluminous output a little
static std::set<MyFuncInfo> callSet;
static std::set<MyFuncInfo> definitionSet;

class UnusedDefaultParams:
    public RecursiveASTVisitor<UnusedDefaultParams>, public loplugin::Plugin
{
public:
    explicit UnusedDefaultParams(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile

        std::string output;
        for (const MyFuncInfo & s : definitionSet)
            output += "defn:\t" + s.access + "\t" + s.returnType + "\t" + s.nameAndParams + "\t" + s.sourceLocation + "\n";
        for (const MyFuncInfo & s : callSet)
            output += "call:\t" + s.returnType + "\t" + s.nameAndParams + "\n";
        ofstream myfile;
        myfile.open( SRCDIR "/loplugin.unuseddefaultparams.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitCallExpr(CallExpr * callExpr);
    bool VisitFunctionDecl( const FunctionDecl* functionDecl );
    bool VisitDeclRefExpr( const DeclRefExpr* declRefExpr );
private:
    MyFuncInfo niceName(const FunctionDecl* functionDecl);
};

MyFuncInfo UnusedDefaultParams::niceName(const FunctionDecl* functionDecl)
{
    if (functionDecl->getInstantiatedFromMemberFunction())
        functionDecl = functionDecl->getInstantiatedFromMemberFunction();
    else if (functionDecl->getClassScopeSpecializationPattern())
        functionDecl = functionDecl->getClassScopeSpecializationPattern();
// workaround clang-3.5 issue
#if CLANG_VERSION >= 30600
    else if (functionDecl->getTemplateInstantiationPattern())
        functionDecl = functionDecl->getTemplateInstantiationPattern();
#endif

    MyFuncInfo aInfo;
    switch (functionDecl->getAccess())
    {
    case AS_public: aInfo.access = "public"; break;
    case AS_private: aInfo.access = "private"; break;
    case AS_protected: aInfo.access = "protected"; break;
    default: aInfo.access = "unknown"; break;
    }
    aInfo.returnType = compat::getReturnType(*functionDecl).getCanonicalType().getAsString();

    if (isa<CXXMethodDecl>(functionDecl)) {
        const CXXRecordDecl* recordDecl = dyn_cast<CXXMethodDecl>(functionDecl)->getParent();
        aInfo.nameAndParams += recordDecl->getQualifiedNameAsString();
        aInfo.nameAndParams += "::";
    }
    aInfo.nameAndParams += functionDecl->getNameAsString() + "(";
    bool bFirst = true;
    for (const ParmVarDecl *pParmVarDecl : compat::parameters(*functionDecl)) {
        if (bFirst)
            bFirst = false;
        else
            aInfo.nameAndParams += ",";
        aInfo.nameAndParams += pParmVarDecl->getType().getCanonicalType().getAsString();
    }
    aInfo.nameAndParams += ")";
    if (isa<CXXMethodDecl>(functionDecl) && dyn_cast<CXXMethodDecl>(functionDecl)->isConst()) {
        aInfo.nameAndParams += " const";
    }

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( functionDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));

    return aInfo;
}

bool UnusedDefaultParams::VisitCallExpr(CallExpr * callExpr) {
    if (ignoreLocation(callExpr)) {
        return true;
    }
    const FunctionDecl* functionDecl;
    if (isa<CXXMemberCallExpr>(callExpr)) {
        functionDecl = dyn_cast<CXXMemberCallExpr>(callExpr)->getMethodDecl();
    }
    else {
        functionDecl = callExpr->getDirectCallee();
    }
    if (functionDecl == nullptr) {
        return true;
    }
    functionDecl = functionDecl->getCanonicalDecl();
    // method overrides don't always specify the same default params (althogh they probably should)
    // so we need to work our way up to the root method
    while (isa<CXXMethodDecl>(functionDecl)) {
        const CXXMethodDecl* methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
        if (methodDecl->size_overridden_methods()==0)
            break;
        functionDecl = *methodDecl->begin_overridden_methods();
    }
    // work our way back to the root definition for template methods
    if (functionDecl->getInstantiatedFromMemberFunction())
        functionDecl = functionDecl->getInstantiatedFromMemberFunction();
    else if (functionDecl->getClassScopeSpecializationPattern())
        functionDecl = functionDecl->getClassScopeSpecializationPattern();
// workaround clang-3.5 issue
#if CLANG_VERSION >= 30600
    else if (functionDecl->getTemplateInstantiationPattern())
        functionDecl = functionDecl->getTemplateInstantiationPattern();
#endif
    auto n = functionDecl->getNumParams();
    if (n == 0 || !functionDecl->getParamDecl(n - 1)->hasDefaultArg()) {
        return true;
    }
    assert(callExpr->getNumArgs() <= n); // can be < in template code
    for (unsigned i = callExpr->getNumArgs(); i != 0;) {
        --i;
        Expr* arg = callExpr->getArg(i);
        if (arg->isDefaultArgument()) {
            continue;
        }
        // ignore this, it seems to trigger an infinite recursion
        if (isa<UnaryExprOrTypeTraitExpr>(arg))
            break;
        const ParmVarDecl* parmVarDecl = functionDecl->getParamDecl(i);
        if (!parmVarDecl->hasDefaultArg()
            || parmVarDecl->hasUninstantiatedDefaultArg())
        {
            break;
        }
        const Expr* defaultArgExpr = parmVarDecl->getDefaultArg();
        if (!defaultArgExpr) {
            break;
        }
        MyFuncInfo funcInfo = niceName(functionDecl);
        callSet.insert(funcInfo);
        break;
    }
    return true;
}

bool UnusedDefaultParams::VisitFunctionDecl( const FunctionDecl* functionDecl )
{
    functionDecl = functionDecl->getCanonicalDecl();
    if( !functionDecl || !functionDecl->getLocation().isValid() || ignoreLocation( functionDecl )) {
        return true;
    }
    const CXXMethodDecl* methodDecl = dyn_cast_or_null<CXXMethodDecl>(functionDecl);

    // ignore method overrides, since the call will show up as being directed to the root method
    if (methodDecl && (methodDecl->size_overridden_methods() != 0 || methodDecl->hasAttr<OverrideAttr>())) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getNameInfo().getLoc()))) {
        return true;
    }
    if (isa<CXXDestructorDecl>(functionDecl)) {
        return true;
    }
    if (isa<CXXConstructorDecl>(functionDecl)) {
        return true;
    }
    if (functionDecl->isDeleted()) {
        return true;
    }
    auto n = functionDecl->getNumParams();
    if (n == 0 || !functionDecl->getParamDecl(n - 1)->hasDefaultArg()) {
        return true;
    }

    if( functionDecl->getLocation().isValid() )
    {
        MyFuncInfo funcInfo = niceName(functionDecl);
        definitionSet.insert(funcInfo);
    }
    return true;
}

// this catches places that take the address of a method
bool UnusedDefaultParams::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const Decl* functionDecl = declRefExpr->getDecl();
    if (!isa<FunctionDecl>(functionDecl)) {
        return true;
    }
    MyFuncInfo funcInfo = niceName(dyn_cast<FunctionDecl>(functionDecl));
    callSet.insert(funcInfo);
    return true;
}

loplugin::Plugin::Registration< UnusedDefaultParams > X("unuseddefaultparams", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
