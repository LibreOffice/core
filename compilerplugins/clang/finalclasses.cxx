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
#include "plugin.hxx"
#include <fstream>

/**
Look for classes that are final i.e. nothing extends them, and have either
(a) protected fields or members.
or
(b) virtual members

In the case of (a), those members/fields can be made private.
In the case of (b), making the class final means the compiler can devirtualise
some method calls

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE=all COMPILER_PLUGIN_TOOL='finalclasses' check
  $ ./compilerplugins/clang/finalclasses.py

*/

namespace {

// try to limit the voluminous output a little
static std::set<std::string> inheritedFromSet;
static std::map<std::string,std::string> definitionMap;  // className -> filename

class FinalClasses:
    public RecursiveASTVisitor<FinalClasses>, public loplugin::Plugin
{
public:
    explicit FinalClasses(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        handler.enableTreeWideAnalysisMode();

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const std::string & s : inheritedFromSet)
            output += "inherited-from:\t" + s + "\n";
        for (const auto & s : definitionMap)
            output += "definition:\t" + s.first + "\t" + s.second + "\n";
        std::ofstream myfile;
        myfile.open( WORKDIR "/loplugin.finalclasses.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool shouldVisitImplicitCode() const { return true; }

    bool VisitCXXRecordDecl( const CXXRecordDecl* decl);
private:
    void checkBase(QualType qt);
};

bool ignoreClass(StringRef s)
{
    // ignore stuff in the standard library, and UNO stuff we can't touch.
    if (s.startswith("rtl::") || s.startswith("sal::") || s.startswith("com::sun::")
        || s.startswith("std::") || s.startswith("boost::")
        || s == "OString" || s == "OUString" || s == "bad_alloc")
    {
        return true;
    }
    return false;
}

bool FinalClasses::VisitCXXRecordDecl(const CXXRecordDecl* decl)
{
    if (ignoreLocation(decl))
        return true;
    if (!decl->hasDefinition())
        return true;

    for (auto it = decl->bases_begin(); it != decl->bases_end(); ++it)
    {
        const CXXBaseSpecifier spec = *it;
        checkBase(spec.getType());
    }
    for (auto it = decl->vbases_begin(); it != decl->vbases_end(); ++it)
    {
        const CXXBaseSpecifier spec = *it;
        checkBase(spec.getType());
    }

    if (decl->hasAttr<FinalAttr>())
        return true;
    bool bFoundVirtual = false;
    bool bFoundProtected = false;
    for (auto it = decl->method_begin(); it != decl->method_end(); ++it) {
        auto i = *it;
        // ignore methods that are overriding base-class methods, making them private
        // isn't useful
        if ( !i->hasAttr<OverrideAttr>() && i->getAccess() == AS_protected )
            bFoundProtected = true;
        if ( i->isVirtual() )
            bFoundVirtual = true;
    }

    if (!bFoundProtected)
    {
        for (auto it = decl->field_begin(); it != decl->field_end(); ++it) {
            auto i = *it;
            if ( i->getAccess() == AS_protected ) {
                bFoundProtected = true;
                break;
            }
        }
    }
    if (!bFoundProtected && !bFoundVirtual)
        return true;

    std::string s = decl->getQualifiedNameAsString();
    if (ignoreClass(s))
        return true;

    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(decl->getBeginLoc());
    auto const filename = getFilenameOfLocation(spellingLocation);
    auto sourceLocation = filename.substr(strlen(SRCDIR)).str() + ":"
        + std::to_string(compiler.getSourceManager().getSpellingLineNumber(spellingLocation));
    definitionMap.insert( std::pair<std::string,std::string>(s, sourceLocation) );
    return true;
}

void FinalClasses::checkBase(QualType baseType)
{
    // need to look through typedefs, hence the getUnqualifiedDesugaredType
    baseType = baseType.getDesugaredType(compiler.getASTContext());
    std::string x;
    // so that we get just the template name, excluding the template parameters
    if (baseType->isRecordType())
        x = baseType->getAsCXXRecordDecl()->getQualifiedNameAsString();
    else if (auto templateType = baseType->getAs<TemplateSpecializationType>())
        x = templateType->getTemplateName().getAsTemplateDecl()->getQualifiedNameAsString();
    else
        x = baseType.getAsString();
    inheritedFromSet.insert( x );
}

loplugin::Plugin::Registration< FinalClasses > X("finalclasses", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
