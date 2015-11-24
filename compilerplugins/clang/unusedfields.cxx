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
Dump a list of calls to methods, and a list of field definitions.
Then we will post-process the 2 lists and find the set of unused methods.

Be warned that it produces around 5G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unusedfields' check
  $ ./compilerplugins/clang/unusedfields.py unusedfields.log > result.txt

and then
  $ for dir in *; do make FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='unusedfieldsremove' $dir; done
to auto-remove the method declarations

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

*/

namespace {

struct MyFieldInfo
{
    std::string parentClass;
    std::string fieldName;
    std::string sourceLocation;

    bool operator < (const MyFieldInfo &other) const
    {
        if (parentClass < other.parentClass)
            return true;
        else if (parentClass == other.parentClass)
            return fieldName < other.fieldName;
        else
            return false;
    }
};


// try to limit the voluminous output a little
static std::set<MyFieldInfo> touchedSet;
static std::set<MyFieldInfo> definitionSet;


class UnusedFields:
    public RecursiveASTVisitor<UnusedFields>, public loplugin::Plugin
{
public:
    explicit UnusedFields(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyFieldInfo & s : touchedSet)
            output += "touch:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : definitionSet)
        {
            output += "definition:\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.sourceLocation + "\n";
        }
        ofstream myfile;
        myfile.open( SRCDIR "/unusedfields.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitCallExpr(CallExpr* );
    bool VisitFieldDecl( const FieldDecl* );
    bool VisitMemberExpr( const MemberExpr* );
    bool VisitDeclRefExpr( const DeclRefExpr* );
private:
    MyFieldInfo niceName(const FieldDecl*);
    std::string fullyQualifiedName(const FunctionDecl*);
};

MyFieldInfo UnusedFields::niceName(const FieldDecl* fieldDecl)
{
    MyFieldInfo aInfo;
    aInfo.parentClass = fieldDecl->getParent()->getQualifiedNameAsString();
    aInfo.fieldName = fieldDecl->getNameAsString();

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( fieldDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));

    return aInfo;
}

std::string UnusedFields::fullyQualifiedName(const FunctionDecl* functionDecl)
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

// prevent recursive templates from blowing up the stack
static std::set<std::string> traversedFunctionSet;

bool UnusedFields::VisitCallExpr(CallExpr* expr)
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

bool UnusedFields::VisitFieldDecl( const FieldDecl* fieldDecl )
{
    fieldDecl = fieldDecl->getCanonicalDecl();
    const FieldDecl* canonicalDecl = fieldDecl;

    if( ignoreLocation( fieldDecl ))
        return true;

    QualType type = fieldDecl->getType();
    // unwrap array types
    while (type->isArrayType())
        type = type->getAsArrayTypeUnsafe()->getElementType();

    if( CXXRecordDecl* recordDecl = type->getAsCXXRecordDecl() )
    {
        bool warn_unused = false;
        if( recordDecl->hasAttrs())
        {
                // Clang currently has no support for custom attributes, but
                // the annotate attribute comes close, so check for __attribute__((annotate("lo_warn_unused")))
                for( specific_attr_iterator<AnnotateAttr> i = recordDecl->specific_attr_begin<AnnotateAttr>(),
                 e = recordDecl->specific_attr_end<AnnotateAttr>();
                 i != e;
                 ++i )
                {
                    if( (*i)->getAnnotation() == "lo_warn_unused" )
                    {
                        warn_unused = true;
                        break;
                    }
                }
        }
        if( !warn_unused )
        {
            string n = recordDecl->getQualifiedNameAsString();
            if( n == "rtl::OUString" )
                warn_unused = true;
            // Check some common non-LO types.
            if( n == "std::string" || n == "std::basic_string"
                || n == "std::list" || n == "std::__debug::list"
                || n == "std::vector" || n == "std::__debug::vector" )
                warn_unused = true;
        }
        if (!warn_unused)
            return true;
    }

    definitionSet.insert(niceName(canonicalDecl));
    return true;
}

bool UnusedFields::VisitMemberExpr( const MemberExpr* memberExpr )
{
    const ValueDecl* decl = memberExpr->getMemberDecl();
    if (!isa<FieldDecl>(decl)) {
        return true;
    }
    touchedSet.insert(niceName(dyn_cast<FieldDecl>(decl)));
    return true;
}

bool UnusedFields::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const Decl* decl = declRefExpr->getDecl();
    if (!isa<FieldDecl>(decl)) {
        return true;
    }
    touchedSet.insert(niceName(dyn_cast<FieldDecl>(decl)));
    return true;
}

loplugin::Plugin::Registration< UnusedFields > X("unusedfields", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
