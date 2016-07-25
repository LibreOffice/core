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

#include "clang/AST/Attr.h"

#include "plugin.hxx"
#include "compat.hxx"

/**
This plugin performs 3 different analyses:

(1) Find unused methods
(2) Find methods whose return types are never evaluated
(3) Find methods which are public, but are never called from outside the class i.e. they can be private

It does so, by dumping various call/definition/use info to a log file.
Then we will post-process the various lists and find the set of unused methods.

Be warned that it produces around 15G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unusedmethods' check
  $ ./compilerplugins/clang/unusedmethods.py

and then
  $ for dir in *; do make FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='unusedmethodsremove' $dir; done
to auto-remove the method declarations

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

TODO deal with calls to superclass/member constructors from other constructors, so
     we can find unused constructors
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

// for the "unused method" analysis
static std::set<MyFuncInfo> callSet;
static std::set<MyFuncInfo> definitionSet;
// for the "unused return type" analysis
static std::set<MyFuncInfo> usedReturnSet;
// for the "unnecessary public" analysis
static std::set<MyFuncInfo> publicDefinitionSet;
static std::set<MyFuncInfo> calledFromOutsideSet;


class UnusedMethods:
    public RecursiveASTVisitor<UnusedMethods>, public loplugin::Plugin
{
public:
    explicit UnusedMethods(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile

        std::string output;
        for (const MyFuncInfo & s : definitionSet)
            output += "definition:\t" + s.access + "\t" + s.returnType + "\t" + s.nameAndParams + "\t" + s.sourceLocation + "\n";
        // for the "unused method" analysis
        for (const MyFuncInfo & s : callSet)
            output += "call:\t" + s.returnType + "\t" + s.nameAndParams + "\n";
        // for the "unused return type" analysis
        for (const MyFuncInfo & s : usedReturnSet)
            output += "usedReturn:\t" + s.returnType + "\t" + s.nameAndParams + "\n";
        // for the "unnecessary public" analysis
        for (const MyFuncInfo & s : calledFromOutsideSet)
            output += "outside:\t" + s.returnType + "\t" + s.nameAndParams + "\n";
        ofstream myfile;
        myfile.open( SRCDIR "/loplugin.unusedmethods.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitCallExpr(CallExpr* );
    bool VisitFunctionDecl( const FunctionDecl* decl );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitCXXConstructExpr( const CXXConstructExpr* );
private:
    void logCallToRootMethods(const FunctionDecl* functionDecl, std::set<MyFuncInfo>& funcSet);
    MyFuncInfo niceName(const FunctionDecl* functionDecl);
    std::string fullyQualifiedName(const FunctionDecl* functionDecl);
};

MyFuncInfo UnusedMethods::niceName(const FunctionDecl* functionDecl)
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
    if (!isa<CXXConstructorDecl>(functionDecl)) {
        aInfo.returnType = compat::getReturnType(*functionDecl).getCanonicalType().getAsString();
    } else {
        aInfo.returnType = "";
    }

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

std::string UnusedMethods::fullyQualifiedName(const FunctionDecl* functionDecl)
{
    std::string ret = compat::getReturnType(*functionDecl).getCanonicalType().getAsString();
    ret += " ";
    if (isa<CXXMethodDecl>(functionDecl)) {
        const CXXRecordDecl* recordDecl = dyn_cast<CXXMethodDecl>(functionDecl)->getParent();
        ret += recordDecl->getQualifiedNameAsString();
        ret += "::";
    }
    ret += functionDecl->getNameAsString() + "(";
    bool bFirst = true;
    for (const ParmVarDecl *pParmVarDecl : compat::parameters(*functionDecl)) {
        if (bFirst)
            bFirst = false;
        else
            ret += ",";
        ret += pParmVarDecl->getType().getCanonicalType().getAsString();
    }
    ret += ")";
    if (isa<CXXMethodDecl>(functionDecl) && dyn_cast<CXXMethodDecl>(functionDecl)->isConst()) {
        ret += " const";
    }

    return ret;
}

void UnusedMethods::logCallToRootMethods(const FunctionDecl* functionDecl, std::set<MyFuncInfo>& funcSet)
{
    functionDecl = functionDecl->getCanonicalDecl();
    bool bCalledSuperMethod = false;
    if (isa<CXXMethodDecl>(functionDecl)) {
        // For virtual/overriding methods, we need to pretend we called the root method(s),
        // so that they get marked as used.
        const CXXMethodDecl* methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
        for(CXXMethodDecl::method_iterator it = methodDecl->begin_overridden_methods();
            it != methodDecl->end_overridden_methods(); ++it)
        {
            logCallToRootMethods(*it, funcSet);
            bCalledSuperMethod = true;
        }
    }
    if (!bCalledSuperMethod)
    {
        while (functionDecl->getTemplateInstantiationPattern())
            functionDecl = functionDecl->getTemplateInstantiationPattern();
        if (functionDecl->getLocation().isValid() && !ignoreLocation( functionDecl )
             && !functionDecl->isExternC())
            funcSet.insert(niceName(functionDecl));
    }
}

// prevent recursive templates from blowing up the stack
static std::set<std::string> traversedFunctionSet;

bool UnusedMethods::VisitCallExpr(CallExpr* expr)
{
    // Note that I don't ignore ANYTHING here, because I want to get calls to my code that result
    // from template instantiation deep inside the STL and other external code

    FunctionDecl* calleeFunctionDecl = expr->getDirectCallee();
    if (calleeFunctionDecl == nullptr) {
        Expr* callee = expr->getCallee()->IgnoreParenImpCasts();
        DeclRefExpr* dr = dyn_cast<DeclRefExpr>(callee);
        if (dr) {
            calleeFunctionDecl = dyn_cast<FunctionDecl>(dr->getDecl());
            if (calleeFunctionDecl)
                goto gotfunc;
        }
        /*
        expr->dump();
        throw "Can't touch this";
        */
        return true;
    }

gotfunc:
    // if we see a call to a function, it may effectively create new code,
    // if the function is templated. However, if we are inside a template function,
    // calling another function on the same template, the same problem occurs.
    // Rather than tracking all of that, just traverse anything we have not already traversed.
    if (traversedFunctionSet.insert(fullyQualifiedName(calleeFunctionDecl)).second)
        TraverseFunctionDecl(calleeFunctionDecl);

    logCallToRootMethods(calleeFunctionDecl, callSet);

    const Stmt* parent = parentStmt(expr);

    // Now do the checks necessary for the "unnecessary public" analysis
    CXXMethodDecl* calleeMethodDecl = dyn_cast<CXXMethodDecl>(calleeFunctionDecl);
    if (calleeMethodDecl && calleeMethodDecl->getAccess() == AS_public)
    {
        const FunctionDecl* parentFunction = parentFunctionDecl(expr);
        if (parentFunction && parentFunction != calleeFunctionDecl) {
            calledFromOutsideSet.insert(niceName(parentFunction));
        }
    }

    // Now do the checks necessary for the "unused return value" analysis
    if (compat::getReturnType(*calleeFunctionDecl)->isVoidType()) {
        return true;
    }
    if (!parent) {
        // we will get null parent if it's under a CXXConstructExpr node
        logCallToRootMethods(calleeFunctionDecl, usedReturnSet);
        return true;
    }
    if (isa<Expr>(parent) || isa<ReturnStmt>(parent) || isa<DeclStmt>(parent)
        || isa<IfStmt>(parent) || isa<SwitchStmt>(parent) || isa<ForStmt>(parent)
        || isa<WhileStmt>(parent) || isa<DoStmt>(parent)
        || isa<CXXForRangeStmt>(parent))
    {
        logCallToRootMethods(calleeFunctionDecl, usedReturnSet);
        return true;
    }
    if (isa<CompoundStmt>(parent) || isa<DefaultStmt>(parent) || isa<CaseStmt>(parent)
        || isa<LabelStmt>(parent))
    {
        return true;
    }
    parent->dump();
    return true;
}

bool UnusedMethods::VisitCXXConstructExpr( const CXXConstructExpr* constructExpr )
{
    // Note that I don't ignore ANYTHING here, because I want to get calls to my code that result
    // from template instantiation deep inside the STL and other external code

    const CXXConstructorDecl* constructorDecl = constructExpr->getConstructor();
    constructorDecl = constructorDecl->getCanonicalDecl();

    logCallToRootMethods(constructorDecl, callSet);

    return true;
}

bool UnusedMethods::VisitFunctionDecl( const FunctionDecl* functionDecl )
{
    functionDecl = functionDecl->getCanonicalDecl();
    const CXXMethodDecl* methodDecl = dyn_cast_or_null<CXXMethodDecl>(functionDecl);

    // ignore method overrides, since the call will show up as being directed to the root method
    if (methodDecl && (methodDecl->size_overridden_methods() != 0 || methodDecl->hasAttr<OverrideAttr>())) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getCanonicalDecl()->getNameInfo().getLoc()))) {
        return true;
    }
    if (isa<CXXDestructorDecl>(functionDecl)) {
        return true;
    }
    if (functionDecl->isDeleted() || functionDecl->isDefaulted()) {
        return true;
    }
    if (isa<CXXConstructorDecl>(functionDecl) && dyn_cast<CXXConstructorDecl>(functionDecl)->isCopyConstructor()) {
        return true;
    }

    if( functionDecl->getLocation().isValid() && !ignoreLocation( functionDecl )
        && !functionDecl->isExternC())
    {
        MyFuncInfo funcInfo = niceName(functionDecl);
        definitionSet.insert(funcInfo);
        if (functionDecl->getAccess() == AS_public) {
            publicDefinitionSet.insert(funcInfo);
        }
    }
    return true;
}

// this catches places that take the address of a method
bool UnusedMethods::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const Decl* functionDecl = declRefExpr->getDecl();
    if (!isa<FunctionDecl>(functionDecl)) {
        return true;
    }
    logCallToRootMethods(dyn_cast<FunctionDecl>(functionDecl)->getCanonicalDecl(), callSet);
    logCallToRootMethods(dyn_cast<FunctionDecl>(functionDecl)->getCanonicalDecl(), usedReturnSet);
    return true;
}

loplugin::Plugin::Registration< UnusedMethods > X("unusedmethods", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
