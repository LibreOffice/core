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
#include <set>
#include "plugin.hxx"
#include "compat.hxx"
#include <fstream>

/**
Look for virtual methods where we never call the defining virtual method, and only call the overriding virtual
methods, which indicates a places where the virtual-ness is unwarranted, normally a result of premature abstraction.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='VirtualDown' check
  $ ./compilerplugins/clang/VirtualDown.py

@TODO for some reason, we get false+ for operator== methods
@TODO some templates confuse it and we get false+

*/

namespace
{
struct MyFuncInfo
{
    std::string name;
    std::string sourceLocation;
};
bool operator<(const MyFuncInfo& lhs, const MyFuncInfo& rhs) { return lhs.name < rhs.name; }

// try to limit the voluminous output a little
static std::set<MyFuncInfo> definitionSet;
static std::set<std::string> callSet;

class VirtualDown : public loplugin::FilteringPlugin<VirtualDown>
{
public:
    explicit VirtualDown(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyFuncInfo& s : definitionSet)
            output += "definition:\t" + s.name + "\t" + s.sourceLocation + "\n";
        for (const std::string& s : callSet)
            output += "call:\t" + s + "\n";
        std::ofstream myfile;
        myfile.open(WORKDIR "/loplugin.virtualdown.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }
    bool shouldVisitTemplateInstantiations() const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitCXXMethodDecl(CXXMethodDecl const*);
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr const*);
    bool TraverseFunctionDecl(FunctionDecl*);
    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool TraverseCXXConversionDecl(CXXConversionDecl*);
#if CLANG_VERSION >= 50000
    bool TraverseCXXDeductionGuideDecl(CXXDeductionGuideDecl*);
#endif
private:
    std::string toString(SourceLocation loc);
    std::string niceName(const CXXMethodDecl* functionDecl);
    FunctionDecl const* currentFunctionDecl = nullptr;
};

bool VirtualDown::VisitCXXMethodDecl(const CXXMethodDecl* methodDecl)
{
    if (ignoreLocation(methodDecl))
    {
        return true;
    }
    if (!methodDecl->isThisDeclarationADefinition() || !methodDecl->isVirtual()
        || methodDecl->isDeleted())
    {
        return true;
    }
    methodDecl = methodDecl->getCanonicalDecl();
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(methodDecl))
    {
        return true;
    }

    std::string aNiceName = niceName(methodDecl);

    if (isa<CXXDestructorDecl>(methodDecl))
        return true;
    if (isa<CXXConstructorDecl>(methodDecl))
        return true;

    if (methodDecl->size_overridden_methods() == 0)
        definitionSet.insert({ aNiceName, toString(methodDecl->getLocation()) });

    return true;
}

bool VirtualDown::VisitCXXMemberCallExpr(CXXMemberCallExpr const* expr)
{
    // Note that I don't ignore ANYTHING here, because I want to get calls to my code that result
    // from template instantiation deep inside the STL and other external code

    FunctionDecl const* calleeFunctionDecl = expr->getDirectCallee();
    if (calleeFunctionDecl == nullptr)
    {
        Expr const* callee = expr->getCallee()->IgnoreParenImpCasts();
        DeclRefExpr const* dr = dyn_cast<DeclRefExpr>(callee);
        if (dr)
        {
            calleeFunctionDecl = dyn_cast<FunctionDecl>(dr->getDecl());
            if (calleeFunctionDecl)
                goto gotfunc;
        }
        return true;
    }

gotfunc:

    // ignore recursive calls
    if (currentFunctionDecl == calleeFunctionDecl)
        return true;

    auto cxxMethodDecl = dyn_cast<CXXMethodDecl>(calleeFunctionDecl);
    if (!cxxMethodDecl)
        return true;

    while (cxxMethodDecl->getTemplateInstantiationPattern())
        cxxMethodDecl = dyn_cast<CXXMethodDecl>(cxxMethodDecl->getTemplateInstantiationPattern());
    while (cxxMethodDecl->getInstantiatedFromMemberFunction())
        cxxMethodDecl = dyn_cast<CXXMethodDecl>(cxxMethodDecl->getInstantiatedFromMemberFunction());
    if (cxxMethodDecl->getLocation().isValid() && !ignoreLocation(cxxMethodDecl))
        callSet.insert(niceName(cxxMethodDecl));

    return true;
}

bool VirtualDown::TraverseFunctionDecl(FunctionDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(f);
    currentFunctionDecl = copy;
    return ret;
}
bool VirtualDown::TraverseCXXMethodDecl(CXXMethodDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(f);
    currentFunctionDecl = copy;
    return ret;
}
bool VirtualDown::TraverseCXXConversionDecl(CXXConversionDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseCXXConversionDecl(f);
    currentFunctionDecl = copy;
    return ret;
}
#if CLANG_VERSION >= 50000
bool VirtualDown::TraverseCXXDeductionGuideDecl(CXXDeductionGuideDecl* f)
{
    auto copy = currentFunctionDecl;
    currentFunctionDecl = f;
    bool ret = RecursiveASTVisitor::TraverseCXXDeductionGuideDecl(f);
    currentFunctionDecl = copy;
    return ret;
}
#endif

std::string VirtualDown::niceName(const CXXMethodDecl* cxxMethodDecl)
{
    std::string s = cxxMethodDecl->getReturnType().getCanonicalType().getAsString() + " "
                    + cxxMethodDecl->getQualifiedNameAsString() + "(";
    for (const ParmVarDecl* pParmVarDecl : compat::parameters(*cxxMethodDecl))
    {
        s += pParmVarDecl->getType().getCanonicalType().getAsString();
        s += ",";
    }
    s += ")";
    if (cxxMethodDecl->isConst())
    {
        s += "const";
    }
    return s;
}

std::string VirtualDown::toString(SourceLocation loc)
{
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc(loc);
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    std::string sourceLocation
        = std::string(name.substr(strlen(SRCDIR) + 1)) + ":"
          + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(sourceLocation);
    return sourceLocation;
}

loplugin::Plugin::Registration<VirtualDown> X("virtualdown", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
