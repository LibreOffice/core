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
  Count call sites that are actually using the defaulted values on params on methods that declare such.

  The process goes something like this:
    $ make check
    $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='countusersofdefaultparams' check
    $ ./compilerplugins/clang/countusersofdefaultparams.py
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
struct MyCallInfo : public MyFuncInfo
{
    std::string sourceLocationOfCall;
};
bool operator < (const MyCallInfo &lhs, const MyCallInfo &rhs)
{
    return std::tie(lhs.returnType, lhs.nameAndParams, lhs.sourceLocationOfCall)
         < std::tie(rhs.returnType, rhs.nameAndParams, rhs.sourceLocationOfCall);
}


// try to limit the voluminous output a little
static std::set<MyCallInfo> callSet;
static std::set<MyFuncInfo> definitionSet;

class CountUsersOfDefaultParams:
    public RecursiveASTVisitor<CountUsersOfDefaultParams>, public loplugin::Plugin
{
public:
    explicit CountUsersOfDefaultParams(loplugin::InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile

        std::string output;
        for (const MyFuncInfo & s : definitionSet)
            output += "defn:\t" + s.access + "\t" + s.returnType + "\t" + s.nameAndParams + "\t" + s.sourceLocation + "\n";
        for (const MyCallInfo & s : callSet)
            output += "call:\t" + s.returnType + "\t" + s.nameAndParams + "\t" + s.sourceLocationOfCall + "\n";
        std::ofstream myfile;
        myfile.open( SRCDIR "/loplugin.countusersofdefaultparams.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitCallExpr( const CallExpr * );
    bool VisitFunctionDecl( const FunctionDecl* );
    bool VisitCXXConstructExpr( const CXXConstructExpr * );
private:
    void niceName(const FunctionDecl* functionDecl, MyFuncInfo&);
    std::string locationToString(const SourceLocation&);
};

void CountUsersOfDefaultParams::niceName(const FunctionDecl* functionDecl, MyFuncInfo& aInfo)
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

    aInfo.sourceLocation = locationToString(functionDecl->getLocation());
    loplugin::normalizeDotDotInFilePath(aInfo.sourceLocation);
}

bool CountUsersOfDefaultParams::VisitCallExpr(const CallExpr * callExpr) {
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
    // work our way up to the root method
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
    int n = functionDecl->getNumParams() - 1;
    if (n < 0 || !functionDecl->getParamDecl(n)->hasDefaultArg()) {
        return true;
    }
    while (n > 0 && functionDecl->getParamDecl(n-1)->hasDefaultArg()) {
        --n;
    }
    // look for callsites that are actually using the defaulted values
    if ( n < (int)callExpr->getNumArgs() && callExpr->getArg(n)->isDefaultArgument()) {
        MyCallInfo callInfo;
        niceName(functionDecl, callInfo);
        callInfo.sourceLocationOfCall = locationToString(callExpr->getLocStart());
        callSet.insert(callInfo);
    }
    return true;
}

bool CountUsersOfDefaultParams::VisitCXXConstructExpr(const CXXConstructExpr * constructExpr) {
    if (ignoreLocation(constructExpr)) {
        return true;
    }
    const CXXConstructorDecl* constructorDecl = constructExpr->getConstructor()->getCanonicalDecl();
    // work our way back to the root definition for template methods
    if (constructorDecl->getInstantiatedFromMemberFunction())
        constructorDecl = dyn_cast<CXXConstructorDecl>(constructorDecl->getInstantiatedFromMemberFunction());
    else if (constructorDecl->getClassScopeSpecializationPattern())
        constructorDecl = dyn_cast<CXXConstructorDecl>(constructorDecl->getClassScopeSpecializationPattern());
// workaround clang-3.5 issue
#if CLANG_VERSION >= 30600
    else if (constructorDecl->getTemplateInstantiationPattern())
        constructorDecl = dyn_cast<CXXConstructorDecl>(constructorDecl->getTemplateInstantiationPattern());
#endif
    int n = constructorDecl->getNumParams() - 1;
    if (n < 0 || !constructorDecl->getParamDecl(n)->hasDefaultArg()) {
        return true;
    }
    while (n > 0 && constructorDecl->getParamDecl(n-1)->hasDefaultArg()) {
        --n;
    }
    // look for callsites that are actually using the defaulted values
    if ( n < (int)constructExpr->getNumArgs() && constructExpr->getArg(n)->isDefaultArgument()) {
        MyCallInfo callInfo;
        niceName(constructorDecl, callInfo);
        callInfo.sourceLocationOfCall = locationToString(constructExpr->getLocStart());
        callSet.insert(callInfo);
    }
    return true;
}

std::string CountUsersOfDefaultParams::locationToString(const SourceLocation& sourceLoc)
{
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( sourceLoc );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    return std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
}

bool CountUsersOfDefaultParams::VisitFunctionDecl( const FunctionDecl* functionDecl )
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
    if (isInUnoIncludeFile(functionDecl)) {
        return true;
    }
    if (isa<CXXDestructorDecl>(functionDecl)) {
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
        MyFuncInfo funcInfo;
        niceName(functionDecl, funcInfo);
        definitionSet.insert(funcInfo);
    }
    return true;
}

loplugin::Plugin::Registration< CountUsersOfDefaultParams > X("countusersofdefaultparams", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
