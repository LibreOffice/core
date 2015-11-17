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
Dump a list of virtual methods and a list of methods overriding virtual methods.
Then we will post-process the 2 lists and find the set of virtual methods which don't need to be virtual.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unnecessaryvirtual' check
  $ ./compilerplugins/clang/unnecessaryvirtual.py unnecessaryvirtual.log > result.txt
  $ for dir in *; do make FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='removevirtuals' $dir; done

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

TODO some boost bind stuff appears to confuse it, notably in the xmloff module
TODO does not find destructors that don't need to be virtual
*/

namespace {

// try to limit the voluminous output a little
static std::set<std::string> definitionSet;
static std::set<std::string> overridingSet;

class UnnecessaryVirtual:
    public RecursiveASTVisitor<UnnecessaryVirtual>, public loplugin::Plugin
{
public:
    explicit UnnecessaryVirtual(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const std::string & s : definitionSet)
            output += "definition:\t" + s + "\n";
        for (const std::string & s : overridingSet)
            output += "overriding:\t" + s + "\n";
        ofstream myfile;
        myfile.open( SRCDIR "/unnecessaryvirtual.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }
    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitCXXMethodDecl( const CXXMethodDecl* decl );
    bool VisitCallExpr(CallExpr* );
private:
    std::string fullyQualifiedName(const FunctionDecl* functionDecl);
};

static std::string niceName(const CXXMethodDecl* functionDecl)
{
    std::string s =
           functionDecl->getParent()->getQualifiedNameAsString() + "::"
           + compat::getReturnType(*functionDecl).getAsString() + "-"
           + functionDecl->getNameAsString() + "(";
    for (const ParmVarDecl *pParmVarDecl : functionDecl->params()) {
        s += pParmVarDecl->getType().getAsString();
        s += ",";
    }
    s += ")";
    if (functionDecl->isConst()) {
        s += "const";
    }
    return s;
}

std::string UnnecessaryVirtual::fullyQualifiedName(const FunctionDecl* functionDecl)
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
    for (const ParmVarDecl *pParmVarDecl : functionDecl->params()) {
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

bool UnnecessaryVirtual::VisitCXXMethodDecl( const CXXMethodDecl* methodDecl )
{
    methodDecl = methodDecl->getCanonicalDecl();

    std::string aNiceName = niceName(methodDecl);

    // for destructors, we need to check if any of the superclass' destructors are virtual
    if (isa<CXXDestructorDecl>(methodDecl)) {
    /* TODO I need to check if the base class has any virtual functions, since overriding
            classes will simply get a compiler-provided virtual destructor by default.

        if (!methodDecl->isVirtual() && !methodDecl->isPure()) {
           return true;
        }
        std::set<std::string> overriddenSet;
        const CXXRecordDecl *pRecordDecl = methodDecl->getParent();
        for(auto baseSpecifier = pRecordDecl->bases_begin();
            baseSpecifier != pRecordDecl->bases_end(); ++baseSpecifier)
        {
            if (baseSpecifier->getType()->isRecordType())
            {
                const CXXRecordDecl *pSuperclassCXXRecordDecl = baseSpecifier->getType()->getAsCXXRecordDecl();
                if (pSuperclassCXXRecordDecl->getDestructor())
                {
                   std::string aOverriddenNiceName = niceName(pSuperclassCXXRecordDecl->getDestructor());
                   overriddenSet.insert(aOverriddenNiceName);
                }
            }
        }
        if (overriddenSet.empty()) {
            cout << "definition:\t" << aNiceName << endl;
        } else {
            for(std::string s : overriddenSet)
               cout << "overriding:\t" << s << endl;
        }*/
        return true;
    }

    if (!methodDecl->isVirtual()) {
        return true;
    }
    if (methodDecl->size_overridden_methods() == 0) {
        // ignore stuff that forms part of the stable URE interface
        if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                                  methodDecl->getNameInfo().getLoc()))) {
            return true;
        }
        // ignore templates and template instantiations,
        // I just cannot get clang to give me decent overriding method data out of them
        if (methodDecl->getParent()->getDescribedClassTemplate()
            || methodDecl->getParent()->getTemplateInstantiationPattern())
            return true;
        if (aNiceName.find("processOpCode2") != std::string::npos)
        {
            methodDecl->dump();
            cout << "definition " << aNiceName << endl;
        }
        definitionSet.insert(aNiceName);
    } else {
       for (auto iter = methodDecl->begin_overridden_methods();
            iter != methodDecl->end_overridden_methods(); ++iter)
       {
           const CXXMethodDecl *overriddenMethod = *iter;
           // we only care about the first level override to establish that a virtual qualifier was useful.
           if (overriddenMethod->isPure() || overriddenMethod->size_overridden_methods() == 0) {
               std::string aOverriddenNiceName = niceName(overriddenMethod);
               overridingSet.insert(aOverriddenNiceName);
               if (aNiceName.find("processOpCode2") != std::string::npos)
               {
                    methodDecl->dump();
                    cout << "overriding " << aNiceName << endl;
                }
           }
      }
    }
    return true;
}

// prevent recursive templates from blowing up the stack
static std::set<std::string> traversedFunctionSet;

bool UnnecessaryVirtual::VisitCallExpr(CallExpr* expr)
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
        return true;
    }

gotfunc:
    // if we see a call to a function, it may effectively create new code,
    // if the function is templated. However, if we are inside a template function,
    // calling another function on the same template, the same problem occurs.
    // Rather than tracking all of that, just traverse anything we have not already traversed.
    if (traversedFunctionSet.insert(fullyQualifiedName(calleeFunctionDecl)).second)
        TraverseFunctionDecl(calleeFunctionDecl);

    return true;
}



loplugin::Plugin::Registration< UnnecessaryVirtual > X("unnecessaryvirtual", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
