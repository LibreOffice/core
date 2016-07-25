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
  Find params on methods where the param is only ever passed as a single constant value.

 The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='constantparam' check
  $ ./compilerplugins/clang/constantparam.py
*/

namespace {

struct MyCallSiteInfo
{
    std::string returnType;
    std::string nameAndParams;
    std::string paramName;
    int paramIndex; // because in some declarations the names are empty
    std::string callValue;
    std::string sourceLocation;
};
bool operator < (const MyCallSiteInfo &lhs, const MyCallSiteInfo &rhs)
{
    return std::tie(lhs.sourceLocation, lhs.paramIndex, lhs.callValue)
         < std::tie(rhs.sourceLocation, rhs.paramIndex, rhs.callValue);
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
        // there is a crash here that I can't seem to workaround
        //  clang-3.8: /home/noel/clang/src/tools/clang/lib/AST/Type.cpp:1878: bool clang::Type::isConstantSizeType() const: Assertion `!isDependentType() && "This doesn't make sense for dependent types"' failed.
        FileID mainFileID = compiler.getSourceManager().getMainFileID();
        static const char* prefix = SRCDIR "/oox/source";
        const FileEntry * fe = compiler.getSourceManager().getFileEntryForID(mainFileID);
        if (strncmp(prefix, fe->getDir()->getName(), strlen(prefix)) == 0) {
            return;
        }
        if (strcmp(fe->getDir()->getName(), SRCDIR "/sw/source/filter/ww8") == 0)
        {
            return;
        }
        if (strcmp(fe->getDir()->getName(), SRCDIR "/sc/source/filter/oox") == 0)
        {
            return;
        }
        if (strcmp(fe->getDir()->getName(), SRCDIR "/sd/source/filter/eppt") == 0)
        {
            return;
        }

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile

        std::string output;
        for (const MyCallSiteInfo & s : callSet)
            output += s.returnType + "\t" + s.nameAndParams + "\t" + s.sourceLocation + "\t"
                        + s.paramName + "\t" + s.callValue + "\n";
        ofstream myfile;
        myfile.open( SRCDIR "/loplugin.constantparam.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode () const { return true; }

    bool VisitCallExpr( const CallExpr* );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitCXXConstructExpr( const CXXConstructExpr* );
private:
    MyCallSiteInfo niceName(const FunctionDecl* functionDecl, int paramIndex, const ParmVarDecl* parmVarDecl, const std::string& callValue);
    std::string getCallValue(const Expr* arg);
};

MyCallSiteInfo ConstantParam::niceName(const FunctionDecl* functionDecl, int paramIndex, const ParmVarDecl* parmVarDecl, const std::string& callValue)
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
    aInfo.paramName = parmVarDecl->getName();
    aInfo.paramIndex = paramIndex;
    aInfo.callValue = callValue;

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( functionDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));

    return aInfo;
}

std::string ConstantParam::getCallValue(const Expr* arg)
{
    arg = arg->IgnoreParenCasts();
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg)) {
        return "unknown";
    }
    APSInt x1;
    if (arg->EvaluateAsInt(x1, compiler.getASTContext()))
    {
        return x1.toString(10);
    }
    return "unknown";
}

bool ConstantParam::VisitCallExpr(const CallExpr * callExpr) {
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
        const Expr* arg = callExpr->getArg(i);
        std::string callValue = getCallValue(arg);
        const ParmVarDecl* parmVarDecl = functionDecl->getParamDecl(i);
        MyCallSiteInfo funcInfo = niceName(functionDecl, i, parmVarDecl, callValue);
        callSet.insert(funcInfo);
    }
    return true;
}

// this catches places that take the address of a method
bool ConstantParam::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const Decl* decl = declRefExpr->getDecl();
    if (!isa<FunctionDecl>(decl)) {
        return true;
    }
    const FunctionDecl* functionDecl = dyn_cast<FunctionDecl>(decl);
    for (unsigned i = 0; i < functionDecl->getNumParams(); ++i)
    {
        MyCallSiteInfo funcInfo = niceName(functionDecl, i, functionDecl->getParamDecl(i), "unknown");
        callSet.insert(funcInfo);
    }
    return true;
}

bool ConstantParam::VisitCXXConstructExpr( const CXXConstructExpr* constructExpr )
{
    const CXXConstructorDecl* constructorDecl = constructExpr->getConstructor();
    constructorDecl = constructorDecl->getCanonicalDecl();

    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              constructorDecl->getNameInfo().getLoc()))) {
        return true;
    }
    if (constructorDecl->getNameInfo().getLoc().isValid() && ignoreLocation(constructorDecl)) {
        return true;
    }

    for (unsigned i = 0; i < constructExpr->getNumArgs(); ++i)
    {
        if (i >= constructorDecl->getNumParams()) // can happen in template code
            break;
        const Expr* arg = constructExpr->getArg(i);
        std::string callValue = getCallValue(arg);
        const ParmVarDecl* parmVarDecl = constructorDecl->getParamDecl(i);
        MyCallSiteInfo funcInfo = niceName(constructorDecl, i, parmVarDecl, callValue);
        callSet.insert(funcInfo);
    }
    return true;
}


loplugin::Plugin::Registration< ConstantParam > X("constantparam", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
