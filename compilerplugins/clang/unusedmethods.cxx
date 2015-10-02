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

Be warned that it produces around 4G of log file.

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

struct MyFuncInfo
{
    std::string returnType;
    std::string nameAndParams;
    std::string sourceLocation;

    bool operator < (const MyFuncInfo &other) const
    {
        if (returnType < other.returnType)
            return true;
        else if (returnType == other.returnType)
            return nameAndParams < other.nameAndParams;
        else
            return false;
    }
};


// try to limit the voluminous output a little
static std::set<MyFuncInfo> callSet;
static std::set<MyFuncInfo> definitionSet;


static bool startswith(const std::string& s, const std::string& prefix)
{
    return s.rfind(prefix,0) == 0;
}

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
        for (const MyFuncInfo & s : callSet)
            output += "call:\t" + s.returnType + "\t" + s.nameAndParams + "\n";
        for (const MyFuncInfo & s : definitionSet)
        {
            //treat all UNO interfaces as having been called, since they are part of our external ABI
            if (!startswith(s.nameAndParams, "com::sun::star::"))
                output += "definition:\t" + s.returnType + "\t" + s.nameAndParams + "\t" + s.sourceLocation + "\n";
        }
        ofstream myfile;
        myfile.open( SRCDIR "/unusedmethods.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitCallExpr(CallExpr* );
    bool VisitFunctionDecl( const FunctionDecl* decl );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitCXXConstructExpr( const CXXConstructExpr* );
    bool VisitVarDecl( const VarDecl* );
    bool VisitCXXRecordDecl( CXXRecordDecl* );
private:
    void logCallToRootMethods(const FunctionDecl* functionDecl);
    MyFuncInfo niceName(const FunctionDecl* functionDecl);
    std::string fullyQualifiedName(const FunctionDecl* functionDecl);
};

MyFuncInfo UnusedMethods::niceName(const FunctionDecl* functionDecl)
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

    MyFuncInfo aInfo;
    aInfo.returnType = compat::getReturnType(*functionDecl).getCanonicalType().getAsString();

    if (isa<CXXMethodDecl>(functionDecl)) {
        const CXXRecordDecl* recordDecl = dyn_cast<CXXMethodDecl>(functionDecl)->getParent();
        aInfo.nameAndParams += recordDecl->getQualifiedNameAsString();
        aInfo.nameAndParams += "::";
    }
    aInfo.nameAndParams += functionDecl->getNameAsString() + "(";
    bool bFirst = true;
    for (const ParmVarDecl *pParmVarDecl : functionDecl->params()) {
        if (bFirst)
            bFirst = false;
        else
            aInfo.nameAndParams += ",";
        aInfo.nameAndParams += pParmVarDecl->getType().getCanonicalType().getAsString();
    }
    aInfo.nameAndParams += ")";
    if (isa<CXXMethodDecl>(functionDecl) && dyn_cast<CXXMethodDecl>(functionDecl)->isConst()) {
        aInfo.nameAndParams += " const";
    }

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( functionDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));

    return aInfo;
}

std::string UnusedMethods::fullyQualifiedName(const FunctionDecl* functionDecl)
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

void UnusedMethods::logCallToRootMethods(const FunctionDecl* functionDecl)
{
    functionDecl = functionDecl->getCanonicalDecl();
    bool bCalledSuperMethod = false;
    if (isa<CXXMethodDecl>(functionDecl)) {
        // For virtual/overriding methods, we need to pretend we called the root method(s),
        // so that they get marked as used.
        const CXXMethodDecl* methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
        for(CXXMethodDecl::method_iterator it = methodDecl->begin_overridden_methods();
            it != methodDecl->end_overridden_methods(); ++it)
        {
            logCallToRootMethods(*it);
            bCalledSuperMethod = true;
        }
    }
    if (!bCalledSuperMethod)
    {
        while (functionDecl->getTemplateInstantiationPattern())
            functionDecl = functionDecl->getTemplateInstantiationPattern();
        callSet.insert(niceName(functionDecl));
    }
}

// prevent recursive templates from blowing up the stack
static std::set<std::string> traversedFunctionSet;

bool UnusedMethods::VisitCallExpr(CallExpr* expr)
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
        /*
        // ignore case where we can't determine the target of the call because we're inside a template
        if (isa<CXXDependentScopeMemberExpr>(callee))
            return true;
        if (isa<UnresolvedLookupExpr>(callee))
            return true;
        if (isa<UnresolvedMemberExpr>(callee))
            return true;
        if (isa<DependentScopeDeclRefExpr>(callee))
            return true;
        // ignore this, doesn't really exist (side-effect of template expansion on scalar types)
        if (isa<CXXPseudoDestructorExpr>(callee))
            return true;
        expr->dump();
        std::string name = compiler.getSourceManager().getFilename(expansionLoc);
        std::string sourceLocation = name + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
        cout << sourceLocation << endl;
        throw "Cant touch this";
        */
        return true;
    }

gotfunc:
    // if we see a call to a function, it may effectively create new code,
    // if the function is templated. However, if we are inside a template function,
    // calling another function on the same template, the same problem occurs.
    // Rather than tracking all of that, just traverse anything we have not already traversed.
    if (traversedFunctionSet.insert(fullyQualifiedName(calleeFunctionDecl)).second)
        TraverseFunctionDecl(calleeFunctionDecl);

    logCallToRootMethods(calleeFunctionDecl);
    return true;
}

bool UnusedMethods::VisitCXXConstructExpr(const CXXConstructExpr* expr)
{
    const CXXConstructorDecl *consDecl = expr->getConstructor();
    consDecl = consDecl->getCanonicalDecl();
    if (consDecl->getTemplatedKind() == FunctionDecl::TemplatedKind::TK_NonTemplate
        && !consDecl->isFunctionTemplateSpecialization()) {
        return true;
    }
    // if we see a call to a constructor, it may effectively create a whole new class,
    // if the constructor's class is templated.
    if (!traversedFunctionSet.insert(fullyQualifiedName(consDecl)).second)
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
    if (isa<CXXDestructorDecl>(functionDecl)) {
        return true;
    }
    if (isa<CXXConstructorDecl>(functionDecl)) {
        return true;
    }
    if (methodDecl && methodDecl->isDeleted()) {
        return true;
    }

    if( !ignoreLocation( functionDecl ))
        definitionSet.insert(niceName(functionDecl));
    return true;
}

// this catches places that take the address of a method
bool UnusedMethods::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
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

// Sometimes a class will inherit from something, and in the process invoke a template,
// which can create new methods.
//
bool UnusedMethods::VisitCXXRecordDecl( CXXRecordDecl* recordDecl )
{
    recordDecl = recordDecl->getCanonicalDecl();
    if (!recordDecl->hasDefinition())
        return true;
// workaround clang-3.5 issue
#if __clang_major__ > 3 || ( __clang_major__ == 3 && __clang_minor__ >= 6 )
    for(CXXBaseSpecifier* baseSpecifier = recordDecl->bases_begin();
        baseSpecifier != recordDecl->bases_end(); ++baseSpecifier)
    {
        const Type *baseType = baseSpecifier->getType().getTypePtr();
        if (isa<TypedefType>(baseSpecifier->getType())) {
            baseType = dyn_cast<TypedefType>(baseType)->desugar().getTypePtr();
        }
        if (isa<RecordType>(baseType)) {
            const RecordType *baseRecord = dyn_cast<RecordType>(baseType);
            CXXRecordDecl* baseRecordDecl = dyn_cast<CXXRecordDecl>(baseRecord->getDecl());
            if (baseRecordDecl && baseRecordDecl->getTemplateInstantiationPattern()) {
                TraverseCXXRecordDecl(baseRecordDecl);
            }
        }
    }
#endif
    return true;
}

loplugin::Plugin::Registration< UnusedMethods > X("unusedmethods", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
