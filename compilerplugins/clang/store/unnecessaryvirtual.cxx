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
#include "plugin.hxx"
#include "compat.hxx"

/**
Dump a list of virtual methods and a list of methods overriding virtual methods.
Then we will post-process the 2 lists and find the set of virtual methods which don't need to be virtual.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unnecessaryvirtual' check > log.txt
  $ grep 'definition' log.txt | cut -f 2 | sort -u > definition.txt
  $ grep 'overriding' log.txt | cut -f 2 | sort -u > overriding.txt
  $ cat definition.txt overriding.txt | sort | uniq -u > result.txt
  $ echo "\n" >> result.txt
  $ for dir in *; do make  FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='removevirtuals' $dir; done

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)
Notably templates tend to confuse it into removing stuff that is still needed.
*/

namespace {

class UnnecessaryVirtual:
    public RecursiveASTVisitor<UnnecessaryVirtual>, public loplugin::Plugin
{
public:
    explicit UnnecessaryVirtual(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXMethodDecl( const CXXMethodDecl* var );

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

bool UnnecessaryVirtual::VisitCXXMethodDecl( const CXXMethodDecl* functionDecl )
{
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    functionDecl = functionDecl->getCanonicalDecl();
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getNameInfo().getLoc()))) {
        return true;
    }
    if (!functionDecl->isVirtual()) {
        return true;
    }
    // ignore UNO interface definitions, cannot change those
    static const char cssPrefix[] = "com::sun::star";
    if (functionDecl->getParent()->getQualifiedNameAsString().compare(0, strlen(cssPrefix), cssPrefix) == 0) {
        return true;
    }
    std::string aNiceName = niceName(functionDecl);
    // Ignore virtual destructors for now.
    // I cannot currently detect the case where we are overriding a pure virtual destructor.
    if (dyn_cast<CXXDestructorDecl>(functionDecl)) {
        return true;
    }
    if (functionDecl->size_overridden_methods() == 0) {
        // ignore definition of virtual functions in templates
//        if (functionDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate
//           && functionDecl->getParent()->getDescribedClassTemplate() == nullptr)
//        {
           cout << "definition\t" << aNiceName << endl;
//        }
    } else {
       for (CXXMethodDecl::method_iterator iter = functionDecl->begin_overridden_methods(); iter != functionDecl->end_overridden_methods(); ++iter) {
           const CXXMethodDecl *pOverriddenMethod = *iter;
           // we only care about the first level override to establish that a virtual qualifier was useful.
           if (pOverriddenMethod->size_overridden_methods() == 0) {
               // ignore UNO interface definitions, cannot change those
               if (pOverriddenMethod->getParent()->getQualifiedNameAsString().compare(0, strlen(cssPrefix), cssPrefix) != 0) {
                   std::string aOverriddenNiceName = niceName(pOverriddenMethod);
                   cout << "overriding\t" << aOverriddenNiceName << endl;
               }
           }
      }
    }
    return true;
}



loplugin::Plugin::Registration< UnnecessaryVirtual > X("unnecessaryvirtual", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
