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

/**
Dump a list of virtual methods and a list of methods overriding virtual methods.
Then we will post-process the 2 lists and find the set of virtual methods which don't need to be virtual.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unnecessaryvirtual' check > log.txt
  $ ./compilerplugins/clang/unnecessaryvirtual.py log.txt > result.txt
  $ for dir in *; do make FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='removevirtuals' $dir; done

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

TODO function template instantiations are not handled
TODO some boost bind stuff appears to confuse it, notably in the xmloff module
*/

namespace {

class UnnecessaryVirtual:
    public RecursiveASTVisitor<UnnecessaryVirtual>, public loplugin::Plugin
{
public:
    explicit UnnecessaryVirtual(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXRecordDecl( const CXXRecordDecl* decl );
    bool VisitCXXMethodDecl( const CXXMethodDecl* decl );
    bool VisitCXXConstructExpr( const CXXConstructExpr* expr );
    void printTemplateInstantiations( const CXXRecordDecl *decl );
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

static bool startsWith(const std::string& s, const char* other)
{
    return s.compare(0, strlen(other), other) == 0;
}

static bool isStandardStuff(const std::string& s)
{
    // ignore UNO interface definitions, cannot change those
    return startsWith(s, "com::sun::star::")
          // ignore stuff in the C++ stdlib and boost
          || startsWith(s, "std::") || startsWith(s, "boost::") || startsWith(s, "__gnu_debug::")
          // can't change our rtl layer
          || startsWith(s, "rtl::");
}

void UnnecessaryVirtual::printTemplateInstantiations( const CXXRecordDecl *recordDecl )
{
    for(auto functionDecl = recordDecl->method_begin();
        functionDecl != recordDecl->method_end(); ++functionDecl)
    {
        if (!functionDecl->isUserProvided() || !functionDecl->isVirtual()) {
            continue;
        }
        if (isa<CXXDestructorDecl>(*functionDecl)) {
            continue;
        }
        std::string aNiceName = niceName(*functionDecl);
        if (isStandardStuff(aNiceName)) {
            continue;
        }
        if (functionDecl->size_overridden_methods() == 0) {
           cout << "definition:\t" << aNiceName << endl;
        } else {
           for (auto iter = functionDecl->begin_overridden_methods();
                iter != functionDecl->end_overridden_methods(); ++iter)
           {
               const CXXMethodDecl *pOverriddenMethod = *iter;
               // we only care about the first level override to establish that a virtual qualifier was useful.
               if (pOverriddenMethod->isPure() || pOverriddenMethod->size_overridden_methods() == 0) {
                   std::string aOverriddenNiceName = niceName(pOverriddenMethod);
                   if (isStandardStuff(aOverriddenNiceName)) {
                       continue;
                   }
                   cout << "overriding:\t" << aOverriddenNiceName << endl;
               }
          }
        }
    }
    for(auto baseSpecifier = recordDecl->bases_begin();
        baseSpecifier != recordDecl->bases_end(); ++baseSpecifier)
    {
        QualType qt = baseSpecifier->getType().getDesugaredType(compiler.getASTContext());
        if (!qt->isRecordType()) {
            continue;
        }
        const CXXRecordDecl *pSuperclassCXXRecordDecl = qt->getAsCXXRecordDecl();
        std::string aNiceName = pSuperclassCXXRecordDecl->getQualifiedNameAsString();
        if (isStandardStuff(aNiceName)) {
            continue;
        }
        printTemplateInstantiations(pSuperclassCXXRecordDecl);
    }
}

// I need to check construct expressions to see if we are instantiating any templates
// which will effectively generate new methods
bool UnnecessaryVirtual::VisitCXXConstructExpr( const CXXConstructExpr* constructExpr )
{
    if (ignoreLocation(constructExpr)) {
        return true;
    }
    const CXXConstructorDecl* pConstructorDecl = constructExpr->getConstructor();
    const CXXRecordDecl* recordDecl = pConstructorDecl->getParent();
    printTemplateInstantiations(recordDecl);
    return true;
}

// I need to visit class definitions, so I can scan through the classes they extend to check if
// we have any template instantiations that will create new methods
bool UnnecessaryVirtual::VisitCXXRecordDecl( const CXXRecordDecl* recordDecl )
{
    if (ignoreLocation(recordDecl)) {
        return true;
    }
    if(!recordDecl->hasDefinition()) {
        return true;
    }
    // ignore uninstantiated templates
    if (recordDecl->getTemplateInstantiationPattern()) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              recordDecl->getLocation()))) {
        return true;
    }
    for(auto baseSpecifier = recordDecl->bases_begin();
        baseSpecifier != recordDecl->bases_end(); ++baseSpecifier)
    {
        QualType qt = baseSpecifier->getType().getDesugaredType(compiler.getASTContext());
        if (!qt->isRecordType()) {
            continue;
        }
        const CXXRecordDecl *pSuperclassCXXRecordDecl = qt->getAsCXXRecordDecl();
        printTemplateInstantiations(pSuperclassCXXRecordDecl);
    }
    return true;
}

bool UnnecessaryVirtual::VisitCXXMethodDecl( const CXXMethodDecl* functionDecl )
{
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    functionDecl = functionDecl->getCanonicalDecl();
    // ignore uninstantiated template methods
    if (functionDecl->getTemplatedKind() != FunctionDecl::TemplatedKind::TK_NonTemplate
        || functionDecl->getParent()->getDescribedClassTemplate() != nullptr) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getNameInfo().getLoc()))) {
        return true;
    }
    if (isStandardStuff(functionDecl->getParent()->getQualifiedNameAsString())) {
        return true;
    }

    std::string aNiceName = niceName(functionDecl);

    // for destructors, we need to check if any of the superclass' destructors are virtual
    if (isa<CXXDestructorDecl>(functionDecl)) {
    /* TODO I need to check if the base class has any virtual functions, since overriding
            classes will simply get a compiler-provided virtual destructor by default.

        if (!functionDecl->isVirtual() && !functionDecl->isPure()) {
           return true;
        }
        std::set<std::string> overriddenSet;
        const CXXRecordDecl *pRecordDecl = functionDecl->getParent();
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

    if (!functionDecl->isVirtual()) {
        return true;
    }
    if (isStandardStuff(aNiceName)) {
        return true;
    }
    if (functionDecl->size_overridden_methods() == 0) {
           cout << "definition:\t" << aNiceName << endl;
    } else {
       for (auto iter = functionDecl->begin_overridden_methods();
            iter != functionDecl->end_overridden_methods(); ++iter)
       {
           const CXXMethodDecl *pOverriddenMethod = *iter;
           // we only care about the first level override to establish that a virtual qualifier was useful.
           if (pOverriddenMethod->isPure() || pOverriddenMethod->size_overridden_methods() == 0) {
               std::string aOverriddenNiceName = niceName(pOverriddenMethod);
               if (isStandardStuff(aOverriddenNiceName)) {
                   continue;
               }
               cout << "overriding:\t" << aOverriddenNiceName << endl;
           }
      }
    }
    return true;
}



loplugin::Plugin::Registration< UnnecessaryVirtual > X("unnecessaryvirtual", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
