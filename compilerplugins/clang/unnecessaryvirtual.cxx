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
  $ ./compilerplugins/clang/unnecessaryvirtual.py
  $ for dir in *; do make FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='removevirtuals' $dir; done

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

TODO some boost bind stuff appears to confuse it, notably in the xmloff module
TODO does not find destructors that don't need to be virtual
*/

namespace {

struct MyFuncInfo
{
    std::string name;
    std::string sourceLocation;

};
bool operator < (const MyFuncInfo &lhs, const MyFuncInfo &rhs)
{
    return lhs.name < rhs.name;
}

// try to limit the voluminous output a little
static std::set<MyFuncInfo> definitionSet;
static std::set<std::string> overridingSet;

class UnnecessaryVirtual:
    public RecursiveASTVisitor<UnnecessaryVirtual>, public loplugin::Plugin
{
public:
    explicit UnnecessaryVirtual(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyFuncInfo & s : definitionSet)
            output += "definition:\t" + s.name + "\t" + s.sourceLocation + "\n";
        for (const std::string & s : overridingSet)
            output += "overriding:\t" + s + "\n";
        std::ofstream myfile;
        myfile.open( SRCDIR "/loplugin.unnecessaryvirtual.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }
    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitCXXMethodDecl( const CXXMethodDecl* decl );
private:
    std::string toString(SourceLocation loc);
};

std::string niceName(const CXXMethodDecl* functionDecl)
{
    std::string s =
           functionDecl->getParent()->getQualifiedNameAsString() + "::"
           + functionDecl->getReturnType().getAsString() + "-"
           + functionDecl->getNameAsString() + "(";
    for (const ParmVarDecl *pParmVarDecl : compat::parameters(*functionDecl)) {
        s += pParmVarDecl->getType().getAsString();
        s += ",";
    }
    s += ")";
    if (functionDecl->isConst()) {
        s += "const";
    }
    return s;
}

bool UnnecessaryVirtual::VisitCXXMethodDecl( const CXXMethodDecl* methodDecl )
{
    if (ignoreLocation(methodDecl)) {
        return true;
    }
    if (!methodDecl->isThisDeclarationADefinition() ||
        !methodDecl->isVirtual() ||
        methodDecl->isDeleted())
    {
        return true;
    }
    methodDecl = methodDecl->getCanonicalDecl();
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(methodDecl)) {
        return true;
    }

    std::string aNiceName = niceName(methodDecl);

    // for destructors, we need to check if any of the superclass' destructors are virtual
    if (isa<CXXDestructorDecl>(methodDecl)) {
        const CXXRecordDecl* cxxRecordDecl = methodDecl->getParent();
        if (cxxRecordDecl->getNumBases() == 0) {
            definitionSet.insert( { aNiceName, toString( methodDecl->getLocation() ) } );
            return true;
        }
        for(auto baseSpecifier = cxxRecordDecl->bases_begin();
            baseSpecifier != cxxRecordDecl->bases_end(); ++baseSpecifier)
        {
            if (baseSpecifier->getType()->isRecordType())
            {
                const CXXRecordDecl* superclassCXXRecordDecl = baseSpecifier->getType()->getAsCXXRecordDecl();
                std::string aOverriddenNiceName = niceName(superclassCXXRecordDecl->getDestructor());
                overridingSet.insert(aOverriddenNiceName);
            }
        }
        return true;
    }

    if (methodDecl->size_overridden_methods() == 0) {
        definitionSet.insert( { aNiceName, toString( methodDecl->getLocation() ) } );
    } else {
       for (auto iter = methodDecl->begin_overridden_methods();
            iter != methodDecl->end_overridden_methods(); ++iter)
       {
           const CXXMethodDecl *overriddenMethod = *iter;
           // we only care about the first level override to establish that a virtual qualifier was useful.
           if (overriddenMethod->isPure() || overriddenMethod->size_overridden_methods() == 0)
           {
               std::string aOverriddenNiceName = niceName(overriddenMethod);
               overridingSet.insert(aOverriddenNiceName);
           }
        }
    }
    return true;
}

std::string UnnecessaryVirtual::toString(SourceLocation loc)
{
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( loc );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    std::string sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(sourceLocation);
    return sourceLocation;
}


loplugin::Plugin::Registration< UnnecessaryVirtual > X("unnecessaryvirtual", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
