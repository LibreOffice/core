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
#include "check.hxx"
#include "functionaddress.hxx"

/*
  Find params on methods where the param is only ever passed as a single constant value.

 The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='constantparam' check
  $ ./compilerplugins/clang/constantparam.py

  TODO look for OUString and OString params and check for call-params that are always either "" or default constructed

  FIXME this plugin manages to trigger crashes inside clang, when calling EvaluateAsInt, so I end up disabling it for a handful of files
     here and there.
*/

namespace {

struct MyCallSiteInfo
{
    std::string returnType;
    std::string nameAndParams;
    std::string paramName;
    std::string paramType;
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
    public loplugin::FunctionAddress<ConstantParam>
{
public:
    explicit ConstantParam(loplugin::InstantiationData const & data): loplugin::FunctionAddress<ConstantParam>(data) {}

    virtual void run() override
    {
        // ignore some files that make clang crash inside EvaluateAsInt
        std::string fn( compiler.getSourceManager().getFileEntryForID(
                        compiler.getSourceManager().getMainFileID())->getName() );
        loplugin::normalizeDotDotInFilePath(fn);
        if (loplugin::isSamePathname(fn, SRCDIR "/basegfx/source/matrix/b2dhommatrix.cxx")
            || loplugin::isSamePathname(fn, SRCDIR "/basegfx/source/matrix/b3dhommatrix.cxx"))
             return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // this catches places that take the address of a method
        for (auto functionDecl : getFunctionsWithAddressTaken())
        {
            for (unsigned i = 0; i < functionDecl->getNumParams(); ++i)
                addToCallSet(functionDecl, i, functionDecl->getParamDecl(i)->getName(), "unknown3");
        }

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile

        std::string output;
        for (const MyCallSiteInfo & s : callSet)
            output += s.returnType + "\t" + s.nameAndParams + "\t" + s.sourceLocation + "\t"
                        + s.paramName + "\t" + s.paramType + "\t" + s.callValue + "\n";
        std::ofstream myfile;
        myfile.open( SRCDIR "/loplugin.constantparam.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode () const { return true; }

    bool VisitCallExpr( const CallExpr* );
    bool VisitCXXConstructExpr( const CXXConstructExpr* );
private:
    void addToCallSet(const FunctionDecl* functionDecl, int paramIndex, llvm::StringRef paramName, const std::string& callValue);
    std::string getCallValue(const Expr* arg);
};

void ConstantParam::addToCallSet(const FunctionDecl* functionDecl, int paramIndex, llvm::StringRef paramName, const std::string& callValue)
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


    if (!functionDecl->getNameInfo().getLoc().isValid())
        return;
    if (functionDecl->isVariadic())
        return;
    if (ignoreLocation(functionDecl))
        return;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl))
        return;
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( functionDecl->getLocation() );
    StringRef filename = compiler.getSourceManager().getFilename(expansionLoc);
    if (!loplugin::hasPathnamePrefix(filename, SRCDIR))
        return;
    filename = filename.substr(strlen(SRCDIR)+1);


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
    aInfo.paramName = paramName;
    aInfo.paramIndex = paramIndex;
    if (paramIndex < (int)functionDecl->getNumParams())
        aInfo.paramType = functionDecl->getParamDecl(paramIndex)->getType().getCanonicalType().getAsString();
    aInfo.callValue = callValue;

    aInfo.sourceLocation = filename.str() + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(aInfo.sourceLocation);

    callSet.insert(aInfo);
}

std::string ConstantParam::getCallValue(const Expr* arg)
{
    arg = arg->IgnoreParenCasts();
    if (isa<CXXDefaultArgExpr>(arg)) {
        arg = dyn_cast<CXXDefaultArgExpr>(arg)->getExpr();
    }
    arg = arg->IgnoreParenCasts();
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg)) {
        return "unknown1";
    }
    APSInt x1;
    if (arg->EvaluateAsInt(x1, compiler.getASTContext()))
    {
        return x1.toString(10);
    }
    if (isa<CXXNullPtrLiteralExpr>(arg)) {
        return "0";
    }
    if (isa<MaterializeTemporaryExpr>(arg))
    {
        const CXXBindTemporaryExpr* strippedArg = dyn_cast_or_null<CXXBindTemporaryExpr>(arg->IgnoreParenCasts());
        if (strippedArg)
        {
            auto temp = dyn_cast<CXXTemporaryObjectExpr>(strippedArg->getSubExpr());
            if (temp->getNumArgs() == 0)
            {
                if (loplugin::TypeCheck(temp->getType()).Class("OUString").Namespace("rtl").GlobalNamespace()) {
                    return "\"\"";
                }
                if (loplugin::TypeCheck(temp->getType()).Class("OString").Namespace("rtl").GlobalNamespace()) {
                    return "\"\"";
                }
                return "defaultConstruct";
            }
        }
    }

    // Get the expression contents.
    // This helps us find params which are always initialised with something like "OUString()".
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = arg->getLocStart();
    SourceLocation endLoc = arg->getLocEnd();
    const char *p1 = SM.getCharacterData( startLoc );
    const char *p2 = SM.getCharacterData( endLoc );
    if (!p1 || !p2 || (p2 - p1) < 0 || (p2 - p1) > 40) {
        return "unknown";
    }
    unsigned n = Lexer::MeasureTokenLength( endLoc, SM, compiler.getLangOpts());
    std::string s( p1, p2 - p1 + n);
    // strip linefeed and tab characters so they don't interfere with the parsing of the log file
    std::replace( s.begin(), s.end(), '\r', ' ');
    std::replace( s.begin(), s.end(), '\n', ' ');
    std::replace( s.begin(), s.end(), '\t', ' ');

    // now normalize the value. For some params, like OUString, we can pass it as OUString() or "" and they are the same thing
    if (s == "OUString()")
        s = "\"\"";
    else if (s == "OString()")
        s = "\"\"";
    else if (s == "aEmptyOUStr") //sw
        s = "\"\"";
    else if (s == "EMPTY_OUSTRING")//sc
        s = "\"\"";
    else if (s == "GetEmptyOUString()") //sc
        s = "\"\"";
    return s;
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
    if (!functionDecl)
        return true;
    functionDecl = functionDecl->getCanonicalDecl();
    // method overrides don't always specify the same default params (although they probably should)
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

    unsigned len = std::max(callExpr->getNumArgs(), functionDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i) {
        const Expr* valExpr;
        if (i < callExpr->getNumArgs())
            valExpr = callExpr->getArg(i);
        else if (i < functionDecl->getNumParams() && functionDecl->getParamDecl(i)->hasDefaultArg())
            valExpr = functionDecl->getParamDecl(i)->getDefaultArg();
        else
            // can happen in template code
            continue;
        std::string callValue = getCallValue(valExpr);
        std::string paramName = i < functionDecl->getNumParams()
                                ? functionDecl->getParamDecl(i)->getName()
                                : llvm::StringRef("###" + std::to_string(i));
        addToCallSet(functionDecl, i, paramName, callValue);
    }
    return true;
}

bool ConstantParam::VisitCXXConstructExpr( const CXXConstructExpr* constructExpr )
{
    const CXXConstructorDecl* constructorDecl = constructExpr->getConstructor();
    constructorDecl = constructorDecl->getCanonicalDecl();

    unsigned len = std::max(constructExpr->getNumArgs(), constructorDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i) {
        const Expr* valExpr;
        if (i < constructExpr->getNumArgs())
            valExpr = constructExpr->getArg(i);
        else if (i < constructorDecl->getNumParams() && constructorDecl->getParamDecl(i)->hasDefaultArg())
            valExpr = constructorDecl->getParamDecl(i)->getDefaultArg();
        else
            // can happen in template code
            continue;
        std::string callValue = getCallValue(valExpr);
        std::string paramName = i < constructorDecl->getNumParams()
                                ? constructorDecl->getParamDecl(i)->getName()
                                : llvm::StringRef("###" + std::to_string(i));
        addToCallSet(constructorDecl, i, paramName, callValue);
    }
    return true;
}


loplugin::Plugin::Registration< ConstantParam > X("constantparam", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
