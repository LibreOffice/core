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
Dump a list of calls to methods, and a list of method definitions.
Then we will post-process the 2 lists and find the set of unused methods.

Be warned that it produces around 2.4G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unusedmethods' check
  $ ./compilerplugins/clang/unusedmethods.py unusedmethods.log > result.txt

and then
  $ for dir in *; do make FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='unusedmethodsremove' $dir; done
to auto-remove the method declarations

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

TODO deal with calls to superclass/member constructors from other constructors, so
     we can find unused constructors
*/

namespace {

// try to limit the voluminous output a little
static std::set<std::string> callSet;
static std::set<std::string> definitionSet;


class UnusedMethods:
    public RecursiveASTVisitor<UnusedMethods>, public loplugin::Plugin
{
public:
    explicit UnusedMethods(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const std::string & s : callSet)
            output += "call:\t" + s + "\t\n";
        for (const std::string & s : definitionSet)
            output += "definition:\t" + s + "\t\n";
        ofstream myfile;
        myfile.open( SRCDIR "/unusedmethods.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool VisitCallExpr(CallExpr* );
    bool VisitFunctionDecl( const FunctionDecl* decl );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitCXXConstructExpr( const CXXConstructExpr* );
    bool VisitVarDecl( const VarDecl* );
};

static std::string niceName(const FunctionDecl* functionDecl)
{
    if (functionDecl->getInstantiatedFromMemberFunction())
        functionDecl = functionDecl->getInstantiatedFromMemberFunction();
    else if (functionDecl->getClassScopeSpecializationPattern())
        functionDecl = functionDecl->getClassScopeSpecializationPattern();
// workaround clang-3.5 issue
#if __clang_major__ > 3 || ( __clang_major__ == 3 && __clang_minor__ >= 6 )
    else if (functionDecl->getTemplateInstantiationPattern())
        functionDecl = functionDecl->getTemplateInstantiationPattern();
#endif

    std::string s =
        compat::getReturnType(*functionDecl).getCanonicalType().getAsString()
        + " ";
    if (isa<CXXMethodDecl>(functionDecl)) {
        const CXXRecordDecl* recordDecl = dyn_cast<CXXMethodDecl>(functionDecl)->getParent();
        s += recordDecl->getQualifiedNameAsString();
        s += "::";
    }
    s += functionDecl->getNameAsString() + "(";
    bool bFirst = true;
    for (const ParmVarDecl *pParmVarDecl : functionDecl->params()) {
        if (bFirst)
            bFirst = false;
        else
            s += ",";
        s += pParmVarDecl->getType().getCanonicalType().getAsString();
    }
    s += ")";
    if (isa<CXXMethodDecl>(functionDecl) && dyn_cast<CXXMethodDecl>(functionDecl)->isConst()) {
        s += " const";
    }
    return s;
}

static void logCallToRootMethods(const FunctionDecl* functionDecl)
{
    functionDecl = functionDecl->getCanonicalDecl();
    bool bPrinted = false;
    if (isa<CXXMethodDecl>(functionDecl)) {
        // For virtual/overriding methods, we need to pretend we called the root method(s),
        // so that they get marked as used.
        const CXXMethodDecl* methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
        for(CXXMethodDecl::method_iterator it = methodDecl->begin_overridden_methods();
            it != methodDecl->end_overridden_methods(); ++it)
        {
            logCallToRootMethods(*it);
            bPrinted = true;
        }
    }
    if (!bPrinted)
    {
        std::string s = niceName(functionDecl);
        callSet.insert(s);
    }
}

static bool startsWith(const std::string& s, const char* other)
{
    return s.compare(0, strlen(other), other) == 0;
}

static bool isStandardStuff(const std::string& input)
{
    std::string s = input;
    if (startsWith(s,"class "))
        s = s.substr(6);
    else if (startsWith(s,"struct "))
        s = s.substr(7);
    // ignore UNO interface definitions, cannot change those
    return startsWith(s, "com::sun::star::")
          // ignore stuff in the C++ stdlib and boost
          || startsWith(s, "std::") || startsWith(s, "boost::") || startsWith(s, "class boost::") || startsWith(s, "__gnu_debug::")
          // external library
          || startsWith(s, "mdds::")
          // can't change our rtl layer
          || startsWith(s, "rtl::")
          // ignore anonymous namespace stuff, it is compilation-unit-local and the compiler will detect any
          // unused code there
          || startsWith(s, "(anonymous namespace)::");
}

// prevent recursive templates from blowing up the stack
static std::set<const FunctionDecl*> traversedFunctionSet;

bool UnusedMethods::VisitCallExpr(CallExpr* expr)
{
    // I don't use the normal ignoreLocation() here, because I __want__ to include files that are
    // compiled in the $WORKDIR since they may refer to normal code
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( expr->getLocStart() );
    if( compiler.getSourceManager().isInSystemHeader( expansionLoc ))
        return true;

    FunctionDecl* calleeFunctionDecl = expr->getDirectCallee();
    if (calleeFunctionDecl == nullptr) {
        return true;
    }
    // if we see a call to a function, it may effectively create new code,
    // if the function is templated. However, if we are inside a template function,
    // calling another function on the same template, the same problem occurs.
    // Rather than tracking all of that, just traverse anything we have not already traversed.
    if (traversedFunctionSet.insert(calleeFunctionDecl).second)
        TraverseFunctionDecl(calleeFunctionDecl);

    logCallToRootMethods(calleeFunctionDecl);
    return true;
}

bool UnusedMethods::VisitCXXConstructExpr(const CXXConstructExpr* expr)
{
    // I don't use the normal ignoreLocation() here, because I __want__ to include files that are
    // compiled in the $WORKDIR since they may refer to normal code
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( expr->getLocStart() );
    if( compiler.getSourceManager().isInSystemHeader( expansionLoc ))
        return true;

    const CXXConstructorDecl *consDecl = expr->getConstructor();
    consDecl = consDecl->getCanonicalDecl();
    if (consDecl->getTemplatedKind() == FunctionDecl::TemplatedKind::TK_NonTemplate
        && !consDecl->isFunctionTemplateSpecialization()) {
        return true;
    }
    // if we see a call to a constructor, it may effectively create a whole new class,
    // if the constructor's class is templated.
    if (!traversedFunctionSet.insert(consDecl).second)
        return true;

    const CXXRecordDecl* parent = consDecl->getParent();
    for( CXXRecordDecl::ctor_iterator it = parent->ctor_begin(); it != parent->ctor_end(); ++it)
        TraverseCXXConstructorDecl(*it);
    for( CXXRecordDecl::method_iterator it = parent->method_begin(); it != parent->method_end(); ++it)
        TraverseCXXMethodDecl(*it);

    return true;
}

bool UnusedMethods::VisitFunctionDecl( const FunctionDecl* functionDecl )
{
    if (ignoreLocation(functionDecl)) {
        return true;
    }

    functionDecl = functionDecl->getCanonicalDecl();
    const CXXMethodDecl* methodDecl = dyn_cast_or_null<CXXMethodDecl>(functionDecl);

    // ignore method overrides, since the call will show up as being directed to the root method
    if (methodDecl && (methodDecl->size_overridden_methods() != 0 || methodDecl->hasAttr<OverrideAttr>())) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getCanonicalDecl()->getNameInfo().getLoc()))) {
        return true;
    }
    if (methodDecl && isStandardStuff(methodDecl->getParent()->getQualifiedNameAsString())) {
        return true;
    }
    if (isa<CXXDestructorDecl>(functionDecl)) {
        return true;
    }
    if (isa<CXXConstructorDecl>(functionDecl)) {
        return true;
    }
    if (methodDecl && methodDecl->isDeleted()) {
        return true;
    }

    definitionSet.insert(niceName(functionDecl));
    return true;
}

// this catches places that take the address of a method
bool UnusedMethods::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    // I don't use the normal ignoreLocation() here, because I __want__ to include files that are
    // compiled in the $WORKDIR since they may refer to normal code
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( declRefExpr->getLocStart() );
    if( compiler.getSourceManager().isInSystemHeader( expansionLoc ))
        return true;

    const Decl* functionDecl = declRefExpr->getDecl();
    if (!isa<FunctionDecl>(functionDecl)) {
        return true;
    }
    logCallToRootMethods(dyn_cast<FunctionDecl>(functionDecl)->getCanonicalDecl());
    return true;
}

// this is for declarations of static variables that involve a template
bool UnusedMethods::VisitVarDecl( const VarDecl* varDecl )
{
    varDecl = varDecl->getCanonicalDecl();
    // I don't use the normal ignoreLocation() here, because I __want__ to include files that are
    // compiled in the $WORKDIR since they may refer to normal code
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( varDecl->getLocStart() );
    if( compiler.getSourceManager().isInSystemHeader( expansionLoc ))
        return true;

    if (varDecl->getStorageClass() != SC_Static)
        return true;
    const CXXRecordDecl* recordDecl = varDecl->getType()->getAsCXXRecordDecl();
    if (!recordDecl)
        return true;
// workaround clang-3.5 issue
#if __clang_major__ > 3 || ( __clang_major__ == 3 && __clang_minor__ >= 6 )
    if (!recordDecl->getTemplateInstantiationPattern())
        return true;
#endif
    for( CXXRecordDecl::ctor_iterator it = recordDecl->ctor_begin(); it != recordDecl->ctor_end(); ++it)
        TraverseCXXConstructorDecl(*it);
    for( CXXRecordDecl::method_iterator it = recordDecl->method_begin(); it != recordDecl->method_end(); ++it)
        TraverseCXXMethodDecl(*it);
    return true;
}

loplugin::Plugin::Registration< UnusedMethods > X("unusedmethods", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
