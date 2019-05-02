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
#include <unordered_set>
#include <map>

#include "clang/AST/Attr.h"

#include "plugin.hxx"

/**
What we are looking for here are methods that are not reachable from any of the program
entry points.
"Entry points" includes main, and various binary API

Mostly that means we end up finding cycles of methods i.e. methods that refer to each
other, but are not reachable.

It does so, by dumping various call/definition info to a log file.
Be warned that it produces around 20G of log file.

Then we will post-process the log file with a python script, which takes about
15min to run on a fast machine.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='methodcycles' check
  $ ./compilerplugins/clang/methodcycles.py

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

*/

namespace
{
struct MyFuncInfo
{
    std::string returnType;
    std::string nameAndParams;
    std::string sourceLocation;
};
bool operator<(const MyFuncInfo& lhs, const MyFuncInfo& rhs)
{
    return std::tie(lhs.returnType, lhs.nameAndParams)
           < std::tie(rhs.returnType, rhs.nameAndParams);
}

// try to limit the voluminous output a little
static std::multimap<const FunctionDecl*, const FunctionDecl*> callMap;
static std::set<MyFuncInfo> definitionSet;

class MethodCycles : public RecursiveASTVisitor<MethodCycles>, public loplugin::Plugin
{
public:
    explicit MethodCycles(loplugin::InstantiationData const& data)
        : Plugin(data)
    {
    }

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile

        std::string output;
        for (const MyFuncInfo& s : definitionSet)
            output += "definition:\t" + s.returnType + "\t" + s.nameAndParams + "\t"
                      + s.sourceLocation + "\n";
        for (const std::pair<const FunctionDecl*, const FunctionDecl*>& pair : callMap)
        {
            if (!isLocationMine(pair.first->getLocation())
                || !isLocationMine(pair.second->getLocation()))
                continue;
            auto niceNameFrom = niceName(pair.first);
            auto niceNameTo = niceName(pair.second);
            output += "call:\t" + niceNameFrom.returnType + "\t" + niceNameFrom.nameAndParams + "\t"
                      + niceNameTo.returnType + "\t" + niceNameTo.nameAndParams + "\n";
        }
        std::ofstream myfile;
        myfile.open(WORKDIR "/loplugin.methodcycles.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations() const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitCallExpr(CallExpr*);
    bool VisitFunctionDecl(const FunctionDecl* decl);
    bool VisitDeclRefExpr(const DeclRefExpr*);
    bool VisitCXXConstructExpr(const CXXConstructExpr*);

    bool TraverseFunctionDecl(FunctionDecl*);
    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool TraverseCXXConstructorDecl(CXXConstructorDecl*);
    bool TraverseCXXConversionDecl(CXXConversionDecl*);
    bool TraverseCXXDestructorDecl(CXXDestructorDecl*);
    bool TraverseCXXDeductionGuideDecl(CXXDeductionGuideDecl*);

private:
    void logCallToRootMethods(const FunctionDecl* functionDeclFrom,
                              const FunctionDecl* functionDeclTo);
    void findRoots(const FunctionDecl* functionDecl,
                   std::unordered_set<const FunctionDecl*>& roots);
    MyFuncInfo niceName(const FunctionDecl* functionDecl);
    bool isLocationMine(SourceLocation loc);
    std::string toString(SourceLocation loc);
    FunctionDecl const* currentFunctionDecl = nullptr;
};

MyFuncInfo MethodCycles::niceName(const FunctionDecl* functionDecl)
{
    if (functionDecl->getInstantiatedFromMemberFunction())
        functionDecl = functionDecl->getInstantiatedFromMemberFunction();
#if CLANG_VERSION < 90000
    else if (functionDecl->getClassScopeSpecializationPattern())
        functionDecl = functionDecl->getClassScopeSpecializationPattern();
#endif
    else if (functionDecl->getTemplateInstantiationPattern())
        functionDecl = functionDecl->getTemplateInstantiationPattern();

    MyFuncInfo aInfo;
    if (!isa<CXXConstructorDecl>(functionDecl))
    {
        aInfo.returnType = functionDecl->getReturnType().getCanonicalType().getAsString();
    }
    else
    {
        aInfo.returnType = "";
    }

    if (auto methodDecl = dyn_cast<CXXMethodDecl>(functionDecl))
    {
        const CXXRecordDecl* recordDecl = methodDecl->getParent();
        aInfo.nameAndParams
            = recordDecl->getQualifiedNameAsString() + "::" + functionDecl->getNameAsString() + "(";
    }
    else
    {
        aInfo.nameAndParams = functionDecl->getQualifiedNameAsString() + "(";
    }
    bool bFirst = true;
    for (const ParmVarDecl* pParmVarDecl : functionDecl->parameters())
    {
        if (bFirst)
            bFirst = false;
        else
            aInfo.nameAndParams += ",";
        aInfo.nameAndParams += pParmVarDecl->getType().getCanonicalType().getAsString();
    }
    aInfo.nameAndParams += ")";
    if (isa<CXXMethodDecl>(functionDecl) && dyn_cast<CXXMethodDecl>(functionDecl)->isConst())
    {
        aInfo.nameAndParams += " const";
    }

    aInfo.sourceLocation = toString(functionDecl->getLocation());

    return aInfo;
}

std::string MethodCycles::toString(SourceLocation loc)
{
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc(loc);
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    std::string sourceLocation
        = std::string(name.substr(strlen(SRCDIR) + 1)) + ":"
          + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(sourceLocation);
    return sourceLocation;
}

bool MethodCycles::isLocationMine(SourceLocation loc)
{
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc(loc);
    if (compiler.getSourceManager().isInSystemHeader(expansionLoc))
        return false;
    const char* bufferName = compiler.getSourceManager().getPresumedLoc(expansionLoc).getFilename();
    if (bufferName == NULL)
        return false;
    if (loplugin::hasPathnamePrefix(bufferName, WORKDIR "/")
        || loplugin::hasPathnamePrefix(bufferName, BUILDDIR "/")
        || loplugin::hasPathnamePrefix(bufferName, SRCDIR "/"))
        return true; // ok
    return false;
}

void MethodCycles::logCallToRootMethods(const FunctionDecl* functionDeclFrom,
                                        const FunctionDecl* functionDeclTo)
{
    if (!functionDeclFrom)
    {
        // template magic mostly, but also things called from initialisers
        return;
    }
    functionDeclFrom = functionDeclFrom->getCanonicalDecl();
    functionDeclTo = functionDeclTo->getCanonicalDecl();

    std::unordered_set<const FunctionDecl*> fromRoots;
    findRoots(functionDeclFrom, fromRoots);
    std::unordered_set<const FunctionDecl*> toRoots;
    findRoots(functionDeclTo, toRoots);

    for (auto const& from : fromRoots)
        for (auto const& to : toRoots)
            callMap.insert({ from, to });
}

void MethodCycles::findRoots(const FunctionDecl* functionDecl,
                             std::unordered_set<const FunctionDecl*>& roots)
{
    bool bCalledSuperMethod = false;
    if (auto methodDecl = dyn_cast<CXXMethodDecl>(functionDecl))
    {
        // For virtual/overriding methods, we need to pretend we called from/to root method(s),
        // so that they get marked as used.
        for (auto it = methodDecl->begin_overridden_methods();
             it != methodDecl->end_overridden_methods(); ++it)
        {
            findRoots(*it, roots);
            bCalledSuperMethod = true;
        }
    }
    if (!bCalledSuperMethod)
    {
        while (functionDecl->getTemplateInstantiationPattern())
            functionDecl = functionDecl->getTemplateInstantiationPattern();
        if (functionDecl->getLocation().isValid())
            roots.insert(functionDecl);
    }
}

bool MethodCycles::VisitCallExpr(CallExpr* expr)
{
    // Note that I don't ignore ANYTHING here, because I want to get calls to my code that result
    // from template instantiation deep inside the STL and other external code

    FunctionDecl* calleeFunctionDecl = expr->getDirectCallee();
    if (calleeFunctionDecl == nullptr)
    {
        Expr* callee = expr->getCallee()->IgnoreParenImpCasts();
        DeclRefExpr* dr = dyn_cast<DeclRefExpr>(callee);
        if (dr)
        {
            calleeFunctionDecl = dyn_cast<FunctionDecl>(dr->getDecl());
            if (calleeFunctionDecl)
                goto gotfunc;
        }
        return true;
    }

gotfunc:

    if (currentFunctionDecl != calleeFunctionDecl)
        // ignore recursive calls
        logCallToRootMethods(currentFunctionDecl, calleeFunctionDecl);

    return true;
}

bool MethodCycles::VisitCXXConstructExpr(const CXXConstructExpr* constructExpr)
{
    // Note that I don't ignore ANYTHING here, because I want to get calls to my code that result
    // from template instantiation deep inside the STL and other external code

    const CXXConstructorDecl* constructorDecl = constructExpr->getConstructor();
    constructorDecl = constructorDecl->getCanonicalDecl();

    if (!constructorDecl->getLocation().isValid())
    {
        return true;
    }

    logCallToRootMethods(currentFunctionDecl, constructorDecl);

    return true;
}

bool MethodCycles::VisitFunctionDecl(const FunctionDecl* functionDecl)
{
    const FunctionDecl* canonicalFunctionDecl = functionDecl->getCanonicalDecl();
    if (functionDecl->isDeleted())
        return true;
    // don't care about compiler-generated functions
    if (functionDecl->isImplicit())
        return true;
    if (!canonicalFunctionDecl->getLocation().isValid())
        return true;
    // ignore method overrides, since the call will show up as being directed to the root method
    const CXXMethodDecl* methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
    if (methodDecl
        && (methodDecl->size_overridden_methods() != 0 || methodDecl->hasAttr<OverrideAttr>()))
        return true;
    if (!isLocationMine(canonicalFunctionDecl->getLocation()))
        return true;

    MyFuncInfo funcInfo = niceName(canonicalFunctionDecl);
    definitionSet.insert(funcInfo);
    return true;
}

bool MethodCycles::VisitDeclRefExpr(const DeclRefExpr* declRefExpr)
{
    const FunctionDecl* functionDecl = dyn_cast<FunctionDecl>(declRefExpr->getDecl());
    if (!functionDecl)
    {
        return true;
    }
    logCallToRootMethods(currentFunctionDecl, functionDecl->getCanonicalDecl());

    return true;
}

bool MethodCycles::TraverseFunctionDecl(FunctionDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(f);
    currentFunctionDecl = copy;
    return ret;
}
bool MethodCycles::TraverseCXXMethodDecl(CXXMethodDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(f);
    currentFunctionDecl = copy;
    return ret;
}
bool MethodCycles::TraverseCXXConversionDecl(CXXConversionDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseCXXConversionDecl(f);
    currentFunctionDecl = copy;
    return ret;
}
bool MethodCycles::TraverseCXXDeductionGuideDecl(CXXDeductionGuideDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseCXXDeductionGuideDecl(f);
    currentFunctionDecl = copy;
    return ret;
}
bool MethodCycles::TraverseCXXConstructorDecl(CXXConstructorDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(f);
    currentFunctionDecl = copy;
    return ret;
}
bool MethodCycles::TraverseCXXDestructorDecl(CXXDestructorDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseCXXDestructorDecl(f);
    currentFunctionDecl = copy;
    return ret;
}

loplugin::Plugin::Registration<MethodCycles> X("methodcycles", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
