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

#include "plugin.hxx"
#include "compat.hxx"

/*
 Find methods with default params, where the callers never specify the default param i.e.
 might as well remove it.

 The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unuseddefaultparams' check
  $ ./compilerplugins/clang/unuseddefaultparams.py unuseddefaultparams.log
*/

namespace {

struct MyCallSiteInfo
{
    std::string returnType;
    std::string nameAndParams;
    std::string paramName;
    std::string callValue;
    std::string sourceLocation;
};
bool operator < (const MyCallSiteInfo &lhs, const MyCallSiteInfo &rhs)
{
    if (lhs.sourceLocation < rhs.sourceLocation)
        return true;
    else if (lhs.sourceLocation > rhs.sourceLocation)
        return false;
    else return lhs.callValue < rhs.callValue;
}


// try to limit the voluminous output a little
static std::set<MyCallSiteInfo> callSet;

class ConstantParam:
    public RecursiveASTVisitor<ConstantParam>, public loplugin::Plugin
{
public:
    explicit ConstantParam(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile

        std::string output;
        for (const MyCallSiteInfo & s : callSet)
            output += s.returnType + "\t" + s.nameAndParams + "\t" + s.sourceLocation + "\t"
                        + s.paramName + "\t" + s.callValue + "\n";
        ofstream myfile;
        myfile.open( SRCDIR "/constantparam.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitCallExpr(CallExpr * callExpr);
    bool VisitDeclRefExpr( const DeclRefExpr* declRefExpr );
private:
    MyCallSiteInfo niceName(const FunctionDecl* functionDecl, const ParmVarDecl* parmVarDecl, const std::string& callValue);
};

MyCallSiteInfo ConstantParam::niceName(const FunctionDecl* functionDecl, const ParmVarDecl* parmVarDecl, const std::string& callValue)
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

    MyCallSiteInfo aInfo;
    aInfo.returnType = compat::getReturnType(*functionDecl).getCanonicalType().getAsString();

    if (isa<CXXMethodDecl>(functionDecl)) {
        const CXXRecordDecl* recordDecl = dyn_cast<CXXMethodDecl>(functionDecl)->getParent();
        aInfo.nameAndParams += recordDecl->getQualifiedNameAsString();
        aInfo.nameAndParams += "::";
    }
    aInfo.nameAndParams += functionDecl->getNameAsString() + "(";
    bool bFirst = true;
    for (const ParmVarDecl *pParmVarDecl : functionDecl->params()) {
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
    aInfo.paramName = parmVarDecl->getName();
    aInfo.callValue = callValue;

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( functionDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));

    return aInfo;
}

bool ConstantParam::VisitCallExpr(CallExpr * callExpr) {
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

    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getNameInfo().getLoc()))) {
        return true;
    }
    if (functionDecl->getNameInfo().getLoc().isValid() && ignoreLocation(functionDecl)) {
        return true;
    }

    for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
        if (i >= functionDecl->getNumParams()) // can happen in template code
            break;
        Expr* arg = callExpr->getArg(i);
        bool found = false;
        std::string callValue;
        // ignore this, it seems to trigger an infinite recursion
        if (isa<UnaryExprOrTypeTraitExpr>(arg->IgnoreParenCasts())) {
            found = true;
            callValue = "unknown";
        }
        if (!found)
        {
            APSInt x1;
            if (arg->EvaluateAsInt(x1, compiler.getASTContext()))
            {
                found = true;
                callValue = x1.toString(10);
            }
        }
        if (!found)
            callValue = "unknown";
        const ParmVarDecl* parmVarDecl = functionDecl->getParamDecl(i);
        MyCallSiteInfo funcInfo = niceName(functionDecl, parmVarDecl, callValue);
        callSet.insert(funcInfo);
        break;
    }
    return true;
}

// this catches places that take the address of a method
bool ConstantParam::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const Decl* functionDecl = declRefExpr->getDecl();
    if (!isa<FunctionDecl>(functionDecl)) {
        return true;
    }
// TODO
//    MyCallSiteInfo funcInfo = niceName(dyn_cast<FunctionDecl>(functionDecl));
//    callSet.insert(funcInfo);
    return true;
}

loplugin::Plugin::Registration< ConstantParam > X("constantparam", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
