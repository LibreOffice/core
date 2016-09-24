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
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='mergeclasses' check
  $ ./compilerplugins/clang/mergeclasses.py

FIXME exclude 'static-only' classes, which some people may use/have used instead of a namespace to tie together a bunch of functions

*/

namespace {

// try to limit the voluminous output a little
static std::set<std::string> instantiatedSet;
static std::set<std::pair<std::string,std::string> > childToParentClassSet; // childClassName -> parentClassName
static std::map<std::string,std::string> definitionMap;  // className -> filename

class MergeClasses:
    public RecursiveASTVisitor<MergeClasses>, public loplugin::Plugin
{
public:
    explicit MergeClasses(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const std::string & s : instantiatedSet)
            output += "instantiated:\t" + s + "\n";
        for (const std::pair<std::string,std::string> & s : childToParentClassSet)
            output += "has-subclass:\t" + s.first + "\t" + s.second + "\n";
        for (const std::pair<std::string,std::string> & s : definitionMap)
            output += "definition:\t" + s.first + "\t" + s.second + "\n";
        ofstream myfile;
        myfile.open( SRCDIR "/loplugin.mergeclasses.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool VisitVarDecl(const VarDecl *);
    bool VisitFieldDecl(const FieldDecl *);
    bool VisitCXXConstructExpr( const CXXConstructExpr* var );
    bool VisitCXXRecordDecl( const CXXRecordDecl* decl);
    bool VisitFunctionDecl( const FunctionDecl* decl);
    bool VisitCallExpr(const CallExpr* decl);
};

bool startsWith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

void addToInstantiatedSet(const std::string& s)
{
    // ignore stuff in the standard library, and UNO stuff we can't touch.
    if (startsWith(s, "rtl::") || startsWith(s, "sal::") || startsWith(s, "com::sun::")
        || startsWith(s, "std::") || startsWith(s, "boost::")
        || s == "OString" || s == "OUString" || s == "bad_alloc")
    {
        return;
    }
    instantiatedSet.insert(s);
}

// check for implicit construction
bool MergeClasses::VisitVarDecl( const VarDecl* pVarDecl )
{
    if (ignoreLocation(pVarDecl)) {
        return true;
    }
    addToInstantiatedSet(pVarDecl->getType().getAsString());
    return true;
}

// check for implicit construction
bool MergeClasses::VisitFieldDecl( const FieldDecl* pFieldDecl )
{
    if (ignoreLocation(pFieldDecl)) {
        return true;
    }
    addToInstantiatedSet(pFieldDecl->getType().getAsString());
    return true;
}

bool MergeClasses::VisitCXXConstructExpr( const CXXConstructExpr* pCXXConstructExpr )
{
    if (ignoreLocation(pCXXConstructExpr)) {
        return true;
    }
    // ignore calls when a sub-class is constructing its superclass
    if (pCXXConstructExpr->getConstructionKind() != CXXConstructExpr::ConstructionKind::CK_Complete) {
        return true;
    }
    const CXXConstructorDecl* pCXXConstructorDecl = pCXXConstructExpr->getConstructor();
    const CXXRecordDecl* pParentCXXRecordDecl = pCXXConstructorDecl->getParent();
    std::string s = pParentCXXRecordDecl->getQualifiedNameAsString();
    addToInstantiatedSet(s);
    return true;
}

bool MergeClasses::VisitCXXRecordDecl(const CXXRecordDecl* decl)
{
    if (ignoreLocation(decl)) {
        return true;
    }
    if (decl->hasDefinition())
    {
        SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(decl->getCanonicalDecl()->getLocStart());
        std::string filename = compiler.getSourceManager().getFilename(spellingLocation);
        filename = filename.substr(strlen(SRCDIR));
        definitionMap.insert( std::pair<std::string,std::string>(decl->getQualifiedNameAsString(), filename) );
        for (auto it = decl->bases_begin(); it != decl->bases_end(); ++it)
        {
            const CXXBaseSpecifier spec = *it;
            // need to look through typedefs, hence the getUnqualifiedDesugaredType
            QualType baseType = spec.getType().getDesugaredType(compiler.getASTContext());
            childToParentClassSet.insert( std::pair<std::string,std::string>(decl->getQualifiedNameAsString(),  baseType.getAsString()) );
        }
    }
    return true;
}

bool MergeClasses::VisitFunctionDecl(const FunctionDecl* decl)
{
    if (ignoreLocation(decl)) {
        return true;
    }
    return true;
}

bool startswith(const std::string& s, const std::string& prefix)
{
    return s.rfind(prefix,0) == 0;
}

bool endswith(const std::string& s, const std::string& suffix)
{
    return s.rfind(suffix) == (s.size()-suffix.size());
}

bool MergeClasses::VisitCallExpr(const CallExpr* decl)
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


loplugin::Plugin::Registration< MergeClasses > X("mergeclasses", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
