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

TODO ignore calls from a method to itself, so we can eliminate unused recursive methods
TODO deal with calls to superclass/member constructors from other constructors, so
     we can find unused constructors
TODO need to handle places where the code takes the address of a method, that needs to count
     as a use-site.
TODO deal with free functions and static methods
TODO track instantiations of template class constructor methods
*/

namespace {

// try to limit the volumninous output a little
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
        myfile.open("/home/noel/libo4/unusedmethods.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool VisitCallExpr(CallExpr* );
    bool VisitCXXMethodDecl( const CXXMethodDecl* decl );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool TraverseCXXMethodDecl(CXXMethodDecl * decl) { return RecursiveASTVisitor::TraverseCXXMethodDecl(decl); }
};

static std::string niceName(const CXXMethodDecl* functionDecl)
{
    std::string s =
        compat::getReturnType(*functionDecl).getCanonicalType().getAsString()
        + " " + functionDecl->getParent()->getQualifiedNameAsString()
        + "::" + functionDecl->getNameAsString()
        + "(";
    bool bFirst = true;
    for (const ParmVarDecl *pParmVarDecl : functionDecl->params()) {
        if (bFirst)
            bFirst = false;
        else
            s += ",";
        s += pParmVarDecl->getType().getCanonicalType().getAsString();
    }
    s += ")";
    if (functionDecl->isConst()) {
        s += " const";
    }
    return s;
}

static void logCallToRootMethods(const CXXMethodDecl* decl)
{
    // For virtual/overriding methods, we need to pretend we called the root method(s),
    // so that they get marked as used.
    decl = decl->getCanonicalDecl();
    bool bPrinted = false;
    for(CXXMethodDecl::method_iterator it = decl->begin_overridden_methods();
        it != decl->end_overridden_methods(); ++it)
    {
        logCallToRootMethods(*it);
        bPrinted = true;
    }
    if (!bPrinted)
    {
        std::string s = niceName(decl);
        callSet.insert(s);
    }
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
          || startsWith(s, "std::") || startsWith(s, "boost::") || startsWith(s, "class boost::") || startsWith(s, "__gnu_debug::")
          // can't change our rtl layer
          || startsWith(s, "rtl::")
          // ignore anonymous namespace stuff, it is compilation-unit-local and the compiler will detect any
          // unused code there
          || startsWith(s, "(anonymous namespace)::");
}

// prevent recursive templates from blowing up the stack
static std::set<std::string> traversedTemplateFunctionSet;

bool UnusedMethods::VisitCallExpr(CallExpr* expr)
{
    if (ignoreLocation(expr)) {
        return true;
    }
    FunctionDecl* calleeFunctionDecl = expr->getDirectCallee();
    if (calleeFunctionDecl == nullptr) {
        return true;
    }
    // if we see a call to a templated function, it effectively creates new code,
    // so we need to examine it's interior to see if it, in turn, calls anything else
    if (calleeFunctionDecl->getTemplatedKind() != FunctionDecl::TemplatedKind::TK_NonTemplate
        || calleeFunctionDecl->isFunctionTemplateSpecialization())
    {
        if (traversedTemplateFunctionSet.insert(calleeFunctionDecl->getQualifiedNameAsString()).second)
            TraverseFunctionDecl(calleeFunctionDecl);
    }

    CXXMethodDecl* calleeMethodDecl = dyn_cast_or_null<CXXMethodDecl>(calleeFunctionDecl);
    if (calleeMethodDecl == nullptr) {
        return true;
    }
    logCallToRootMethods(calleeMethodDecl);
    return true;
}

bool UnusedMethods::VisitCXXMethodDecl( const CXXMethodDecl* functionDecl )
{
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    functionDecl = functionDecl->getCanonicalDecl();
    // ignore method overrides, since the call will show up as being directed to the root method
    if (functionDecl->size_overridden_methods() != 0 || functionDecl->hasAttr<OverrideAttr>()) {
        return true;
    }
    // ignore static's for now. Would require generalising this plugin a little
    if (functionDecl->isStatic()) {
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
    if (isa<CXXDestructorDecl>(functionDecl)) {
        return true;
    }
    if (isa<CXXConstructorDecl>(functionDecl)) {
        return true;
    }
    if (functionDecl->isDeleted()) {
        return true;
    }

    definitionSet.insert(niceName(functionDecl));
    return true;
}

// this catches places that take the address of a method
bool UnusedMethods::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    if (ignoreLocation(declRefExpr)) {
        return true;
    }
    const Decl* functionDecl = declRefExpr->getDecl();
    if (!isa<CXXMethodDecl>(functionDecl)) {
        return true;
    }
    logCallToRootMethods(dyn_cast<CXXMethodDecl>(functionDecl));
    return true;
}

loplugin::Plugin::Registration< UnusedMethods > X("unusedmethods", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
