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
#include <fstream>
#include <set>
#include "plugin.hxx"
#include "compat.hxx"

/**
Looks for enums with unused constants.

Be warned that it produces around 5G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unusedenumconstants' check
  $ ./compilerplugins/clang/unusedenumconstants.py

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

TODO ignore constants when we are only testing for them, not actually storing them somewhere

*/

namespace {

struct MyEnumValueInfo
{
    std::string parentClass;
    std::string constantName;
    std::string sourceLocation;
};
bool operator < (const MyEnumValueInfo &lhs, const MyEnumValueInfo &rhs)
{
    return std::tie(lhs.parentClass, lhs.constantName)
         < std::tie(rhs.parentClass, rhs.constantName);
}


// try to limit the voluminous output a little
static std::set<MyEnumValueInfo> touchedSet;
static std::set<MyEnumValueInfo> definitionSet;


class UnusedEnumValues:
    public RecursiveASTVisitor<UnusedEnumValues>, public loplugin::Plugin
{
public:
    explicit UnusedEnumValues(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyEnumValueInfo & s : touchedSet)
            output += "touch:\t" + s.parentClass + "\t" + s.constantName + "\n";
        for (const MyEnumValueInfo & s : definitionSet)
        {
            output += "definition:\t" + s.parentClass + "\t" + s.constantName + "\t" + s.sourceLocation + "\n";
        }
        ofstream myfile;
        myfile.open( SRCDIR "/loplugin.unusedenumvalues.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitEnumDecl( const EnumDecl* );
    bool VisitDeclRefExpr( const DeclRefExpr* );
private:
    MyEnumValueInfo niceName(const EnumConstantDecl*);
};

MyEnumValueInfo UnusedEnumValues::niceName(const EnumConstantDecl* enumDecl)
{
    MyEnumValueInfo aInfo;
    const EnumType* enumType = dyn_cast<EnumType>(enumDecl->getType());
    if (enumType && enumType->getDecl())
    {
        aInfo.parentClass = enumType->getDecl()->getNameAsString();
    } else {
        aInfo.parentClass = "unknown";
    }
    aInfo.constantName = enumDecl->getNameAsString();

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( enumDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));

    return aInfo;
}

bool UnusedEnumValues::VisitEnumDecl( const EnumDecl* enumDecl )
{
    enumDecl = enumDecl->getCanonicalDecl();

    if( ignoreLocation( enumDecl ))
        return true;

    for (auto it = enumDecl->enumerator_begin(); it != enumDecl->enumerator_end(); ++it)
        definitionSet.insert(niceName(*it));
    return true;
}

bool UnusedEnumValues::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const Decl* decl = declRefExpr->getDecl();
    if (!isa<EnumConstantDecl>(decl)) {
        return true;
    }
    touchedSet.insert(niceName(dyn_cast<EnumConstantDecl>(decl)));
    return true;
}

loplugin::Plugin::Registration< UnusedEnumValues > X("unusedenumvalues", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
