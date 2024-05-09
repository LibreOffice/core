/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <set>
#include <string>
#include <iostream>
#include "config_clang.h"
#include "compat.hxx"
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
  $ make FORCE_COMPILE=all COMPILER_PLUGIN_TOOL='mergeclasses' check
  $ ./compilerplugins/clang/mergeclasses.py

FIXME exclude 'static-only' classes, which some people may use/have used instead of a namespace to tie together a bunch of functions

*/

namespace {

// try to limit the voluminous output a little
static std::set<std::string> instantiatedSet;
static std::set<std::pair<std::string,std::string> > childToParentClassSet; // childClassName -> parentClassName
static std::map<std::string,std::string> definitionMap;  // className -> filename

class MergeClasses:
    public loplugin::FilteringPlugin<MergeClasses>
{
public:
    explicit MergeClasses(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    virtual void run() override
    {
        handler.enableTreeWideAnalysisMode();

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const std::string & s : instantiatedSet)
            output += "instantiated:\t" + s + "\n";
        for (const std::pair<std::string,std::string> & s : childToParentClassSet)
            output += "has-subclass:\t" + s.first + "\t" + s.second + "\n";
        for (const auto & s : definitionMap)
            output += "definition:\t" + s.first + "\t" + s.second + "\n";
        std::ofstream myfile;
        myfile.open( WORKDIR "/loplugin.mergeclasses.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitVarDecl(const VarDecl *);
    bool VisitFieldDecl(const FieldDecl *);
    bool VisitCXXConstructExpr( const CXXConstructExpr* var );
    bool VisitCXXRecordDecl( const CXXRecordDecl* decl);
};

bool ignoreClass(StringRef s)
{
    // ignore stuff in the standard library, and UNO stuff we can't touch.
    if (compat::starts_with(s, "rtl::") || compat::starts_with(s, "sal::")
        || compat::starts_with(s, "com::sun::") || compat::starts_with(s, "std::")
        || compat::starts_with(s, "boost::")
        || s == "OString" || s == "OUString" || s == "bad_alloc")
    {
        return true;
    }
    // ignore instantiations of pointers and arrays
    if (compat::ends_with(s, "*") || compat::ends_with(s, "]")) {
        return true;
    }
    return false;
}

// check for implicit construction
bool MergeClasses::VisitVarDecl( const VarDecl* pVarDecl )
{
    if (ignoreLocation(pVarDecl)) {
        return true;
    }
    if (pVarDecl->getType()->isReferenceType())
        return true;
    std::string s = pVarDecl->getType().getUnqualifiedType().getAsString();
    if (!ignoreClass(s))
        instantiatedSet.insert(s);
    return true;
}

// check for implicit construction
bool MergeClasses::VisitFieldDecl( const FieldDecl* pFieldDecl )
{
    if (ignoreLocation(pFieldDecl)) {
        return true;
    }
    if (pFieldDecl->getType()->isReferenceType())
        return true;
    std::string s = pFieldDecl->getType().getUnqualifiedType().getAsString();
    if (!ignoreClass(s))
        instantiatedSet.insert(s);
    return true;
}

bool MergeClasses::VisitCXXConstructExpr( const CXXConstructExpr* pCXXConstructExpr )
{
    if (ignoreLocation(pCXXConstructExpr)) {
        return true;
    }
    // ignore calls when a sub-class is constructing its superclass
    if (pCXXConstructExpr->getConstructionKind() != compat::CXXConstructionKind::Complete) {
        return true;
    }
    const CXXConstructorDecl* pCXXConstructorDecl = pCXXConstructExpr->getConstructor();
    const CXXRecordDecl* pParentCXXRecordDecl = pCXXConstructorDecl->getParent();
    std::string s = pParentCXXRecordDecl->getQualifiedNameAsString();
    if (!ignoreClass(s))
        instantiatedSet.insert(s);
    return true;
}

bool MergeClasses::VisitCXXRecordDecl(const CXXRecordDecl* decl)
{
    if (ignoreLocation(decl)) {
        return true;
    }
    if (decl->isThisDeclarationADefinition())
    {
        SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(decl->getBeginLoc());
        auto filename = getFilenameOfLocation(spellingLocation);
        filename = filename.substr(strlen(SRCDIR));
        std::string s = decl->getQualifiedNameAsString();
        if (ignoreClass(s))
            return true;
        definitionMap.insert( std::pair<std::string,std::string>(s, filename.str()) );
        for (auto it = decl->bases_begin(); it != decl->bases_end(); ++it)
        {
            const CXXBaseSpecifier spec = *it;
            // need to look through typedefs, hence the getUnqualifiedDesugaredType
            QualType baseType = spec.getType().getDesugaredType(compiler.getASTContext());
            childToParentClassSet.insert( std::pair<std::string,std::string>(s, baseType.getAsString()) );
        }
    }
    return true;
}

loplugin::Plugin::Registration< MergeClasses > X("mergeclasses", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
