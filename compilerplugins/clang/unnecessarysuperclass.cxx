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
#include <fstream>

/**

Idea from Norbert (shm_get) - look for classes that are
(a) not instantiated
(b) have zero or one subclasses
and warn about them - would allow us to remove a bunch of abstract classes
that can be merged into one class and simplified.

Dump a list of
- unique classes that exist (A)
- unique classes that are instantiated (B)
- unique class-subclass relationships (C)
Then
   let D = A minus B
   for each class in D, look in C and count the entries, then dump it if no-entries == 1

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unnecessarysuperclass' check > log.txt
  $ ./compilerplugins/clang/unnecessarysuperclass.py > result.txt

*/

namespace {

// try to limit the voluminous output a little
static std::set<std::string> instantiatedSet;
static std::map<std::string,std::string> hasSubclassMap; // childClassName -> parentClassName
static std::map<std::string,std::string> definitionMap;  // className -> filename

class UnnecessarySuperclass:
    public RecursiveASTVisitor<UnnecessarySuperclass>, public loplugin::Plugin
{
public:
    explicit UnnecessarySuperclass(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const std::string & s : instantiatedSet)
            output += "instantiated:\t" + s + "\n";
        for (const std::pair<std::string,std::string> & s : hasSubclassMap)
            output += "has-subclass:\t" + s.first + "\t" + s.second + "\n";
        for (const std::pair<std::string,std::string> & s : definitionMap)
            output += "definition:\t" + s.first + "\t" + s.second + "\n";
        ofstream myfile;
        myfile.open( SRCDIR "/unnecessarysuperclass.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool VisitCXXConstructExpr( const CXXConstructExpr* var );
    bool VisitCXXRecordDecl( const CXXRecordDecl* decl);
    bool VisitFunctionDecl( const FunctionDecl* decl);
    bool VisitCallExpr(const CallExpr* decl);
};

static bool startsWith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool UnnecessarySuperclass::VisitCXXConstructExpr( const CXXConstructExpr* pCXXConstructExpr )
{
    if (ignoreLocation(pCXXConstructExpr)) {
        return true;
    }
    const CXXConstructorDecl* pCXXConstructorDecl = pCXXConstructExpr->getConstructor();
    const CXXRecordDecl* pParentCXXRecordDecl = pCXXConstructorDecl->getParent();
    if (ignoreLocation(pParentCXXRecordDecl)) {
        return true;
    }
    std::string s = pParentCXXRecordDecl->getQualifiedNameAsString();
    // ignore stuff in the standard library, and UNO stuff we can't touch.
    if (startsWith(s, "rtl::") || startsWith(s, "sal::") || startsWith(s, "com::sun::")
        || startsWith(s, "std::") || startsWith(s, "boost::")
        || s == "OString" || s == "OUString" || s == "bad_alloc")
    {
        return true;
    }
    instantiatedSet.insert(s);
    return true;
}

bool UnnecessarySuperclass::VisitCXXRecordDecl(const CXXRecordDecl* decl)
{
    if (ignoreLocation(decl)) {
        return true;
    }
    if (decl->hasDefinition())
    {
        SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(decl->getCanonicalDecl()->getLocStart());
        std::string filename = compiler.getSourceManager().getFilename(spellingLocation);
        definitionMap.insert( std::pair<std::string,std::string>(decl->getQualifiedNameAsString(), filename) );
        for (auto it = decl->bases_begin(); it != decl->bases_end(); ++it)
        {
            const CXXBaseSpecifier spec = *it;
            // need to look through typedefs, hence the getUnqualifiedDesugaredType
            QualType baseType = spec.getType().getDesugaredType(compiler.getASTContext());
            hasSubclassMap.insert( std::pair<std::string,std::string>(decl->getQualifiedNameAsString(),  baseType.getAsString()) );
        }
    }
    return true;
}

bool UnnecessarySuperclass::VisitFunctionDecl(const FunctionDecl* decl)
{
    if (ignoreLocation(decl)) {
        return true;
    }
    return true;
}

static bool startswith(const std::string& s, const std::string& prefix)
{
    return s.rfind(prefix,0) == 0;
}

static bool endswith(const std::string& s, const std::string& suffix)
{
    return s.rfind(suffix) == (s.size()-suffix.size());
}

bool UnnecessarySuperclass::VisitCallExpr(const CallExpr* decl)
{
    if (ignoreLocation(decl)) {
        return true;
    }
    // VclPtr<T>::Create using a forwarding constructor, so we need to handle it differently in order
    // to pick up the instantiation via it.
    if (decl->getCalleeDecl() && isa<CXXMethodDecl>(decl->getCalleeDecl()))
    {
        const CXXMethodDecl * pMethod = dyn_cast<CXXMethodDecl>(decl->getCalleeDecl());
        std::string s = pMethod->getQualifiedNameAsString();
        if (startswith(s, "VclPtr<") && endswith(s, ">::Create"))
        {
            const ClassTemplateSpecializationDecl *pTemplateDecl = dyn_cast<ClassTemplateSpecializationDecl>(pMethod->getParent());
            QualType windowType = pTemplateDecl->getTemplateArgs()[0].getAsType();
            instantiatedSet.insert(windowType.getAsString());
        }
    }
    return true;
}


loplugin::Plugin::Registration< UnnecessarySuperclass > X("unnecessarysuperclass", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
