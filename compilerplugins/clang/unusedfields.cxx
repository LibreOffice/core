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
#include <algorithm>
#include "plugin.hxx"
#include "compat.hxx"

/**
This performs two analyses:
 (1) look for unused fields
 (2) look for fields that are write-only

We dmp a list of calls to methods, and a list of field definitions.
Then we will post-process the 2 lists and find the set of unused methods.

Be warned that it produces around 5G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unusedfields' check
  $ ./compilerplugins/clang/unusedfields.py

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
    std::string fieldType;
    std::string sourceLocation;
    std::string access;
};
bool operator < (const MyFieldInfo &lhs, const MyFieldInfo &rhs)
{
    return std::tie(lhs.parentClass, lhs.fieldName)
         < std::tie(rhs.parentClass, rhs.fieldName);
}


// try to limit the voluminous output a little
static std::set<MyFieldInfo> touchedFromInsideSet;
static std::set<MyFieldInfo> touchedFromConstructorSet;
static std::set<MyFieldInfo> touchedFromOutsideSet;
static std::set<MyFieldInfo> readFromSet;
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
        for (const MyFieldInfo & s : touchedFromInsideSet)
            output += "inside:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : touchedFromConstructorSet)
            output += "constructor:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : touchedFromOutsideSet)
            output += "outside:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : readFromSet)
            output += "read:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : definitionSet)
        {
            output += "definition:\t" + s.access + "\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.fieldType + "\t" + s.sourceLocation + "\n";
        }
        ofstream myfile;
        myfile.open( SRCDIR "/loplugin.unusedfields.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitFieldDecl( const FieldDecl* );
    bool VisitMemberExpr( const MemberExpr* );
    bool VisitDeclRefExpr( const DeclRefExpr* );
private:
    MyFieldInfo niceName(const FieldDecl*);
    void checkTouched(const FieldDecl* fieldDecl, const Expr* memberExpr);
};

MyFieldInfo UnusedFields::niceName(const FieldDecl* fieldDecl)
{
    MyFieldInfo aInfo;

    const RecordDecl* recordDecl = fieldDecl->getParent();

    if (const CXXRecordDecl* cxxRecordDecl = dyn_cast<CXXRecordDecl>(recordDecl))
    {
        if (cxxRecordDecl->getTemplateInstantiationPattern())
            cxxRecordDecl = cxxRecordDecl->getTemplateInstantiationPattern();
        aInfo.parentClass = cxxRecordDecl->getQualifiedNameAsString();
    }
    else
        aInfo.parentClass = recordDecl->getQualifiedNameAsString();

    aInfo.fieldName = fieldDecl->getNameAsString();
    // sometimes the name (if it's anonymous thing) contains the full path of the build folder, which we don't need
    size_t idx = aInfo.fieldName.find(SRCDIR);
    if (idx != std::string::npos) {
        aInfo.fieldName = aInfo.fieldName.replace(idx, strlen(SRCDIR), "");
    }
    aInfo.fieldType = fieldDecl->getType().getAsString();

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( fieldDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    normalizeDotDotInFilePath(aInfo.sourceLocation);

    switch (fieldDecl->getAccess())
    {
    case AS_public: aInfo.access = "public"; break;
    case AS_private: aInfo.access = "private"; break;
    case AS_protected: aInfo.access = "protected"; break;
    default: aInfo.access = "unknown"; break;
    }

    return aInfo;
}

bool UnusedFields::VisitFieldDecl( const FieldDecl* fieldDecl )
{
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation( fieldDecl )) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation()))) {
        return true;
    }

    QualType type = fieldDecl->getType();
    // unwrap array types
    while (type->isArrayType())
        type = type->getAsArrayTypeUnsafe()->getElementType();
/*
    if( CXXRecordDecl* recordDecl = type->getAsCXXRecordDecl() )
    {
        bool warn_unused = recordDecl->hasAttr<WarnUnusedAttr>();
        if( !warn_unused )
        {
            string n = recordDecl->getQualifiedNameAsString();
            // Check some common non-LO types.
            if( n == "std::string" || n == "std::basic_string"
                || n == "std::list" || n == "std::__debug::list"
                || n == "std::vector" || n == "std::__debug::vector" )
                warn_unused = true;
        }
        if (!warn_unused)
            return true;
    }
*/
    definitionSet.insert(niceName(fieldDecl));
    return true;
}

static char easytolower(char in)
{
    if (in<='Z' && in>='A')
        return in-('Z'-'z');
    return in;
}
bool startswith(const std::string& rStr, const char* pSubStr)
{
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool UnusedFields::VisitMemberExpr( const MemberExpr* memberExpr )
{
    const ValueDecl* decl = memberExpr->getMemberDecl();
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    if (!fieldDecl) {
        return true;
    }
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation()))) {
        return true;
    }

    MyFieldInfo fieldInfo = niceName(fieldDecl);

  // for the touched-from-outside analysis

    checkTouched(fieldDecl, memberExpr);

  // for the write-only analysis

    const Stmt* child = memberExpr;
    const Stmt* parent = parentStmt(memberExpr);
    // walk up the tree until we find something interesting
    bool bPotentiallyReadFrom = false;
    bool bDump = false;
    do {
        if (!parent) {
            return true;
        }
        if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent) || isa<ParenListExpr>(parent)
             || isa<ExprWithCleanups>(parent) || isa<UnaryOperator>(parent))
        {
            child = parent;
            parent = parentStmt(parent);
        }
        else if (isa<CaseStmt>(parent))
        {
            bPotentiallyReadFrom = dyn_cast<CaseStmt>(parent)->getLHS() == child
                                  || dyn_cast<CaseStmt>(parent)->getRHS() == child;
            break;
        }
        else if (isa<IfStmt>(parent))
        {
            bPotentiallyReadFrom = dyn_cast<IfStmt>(parent)->getCond() == child;
            break;
        }
        else if (isa<DoStmt>(parent))
        {
            bPotentiallyReadFrom = dyn_cast<DoStmt>(parent)->getCond() == child;
            break;
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            // check for calls to ReadXXX() type methods and the operator>>= methods on Any.
            const FunctionDecl * calleeFunctionDecl = callExpr->getDirectCallee();
            if (calleeFunctionDecl && calleeFunctionDecl->getIdentifier())
            {
                std::string name = calleeFunctionDecl->getNameAsString();
                std::transform(name.begin(), name.end(), name.begin(), easytolower);
                if (startswith(name, "read") || name.find(">>=") != std::string::npos)
                    // this is a write-only call
                    ;
                else
                    bPotentiallyReadFrom = true;
            }
            else
                bPotentiallyReadFrom = true;
            break;
        }
        else if (isa<ReturnStmt>(parent) || isa<CXXConstructExpr>(parent)
                 || isa<ConditionalOperator>(parent) || isa<SwitchStmt>(parent) || isa<ArraySubscriptExpr>(parent)
                 || isa<DeclStmt>(parent) || isa<WhileStmt>(parent) || isa<CXXNewExpr>(parent)
                 || isa<ForStmt>(parent) || isa<InitListExpr>(parent)
                 || isa<BinaryOperator>(parent) || isa<CXXDependentScopeMemberExpr>(parent)
                 || isa<UnresolvedMemberExpr>(parent)
                 || isa<MaterializeTemporaryExpr>(parent))  //???
        {
            bPotentiallyReadFrom = true;
            break;
        }
        else if (isa<CXXDeleteExpr>(parent)
                  || isa<UnaryExprOrTypeTraitExpr>(parent)
                 || isa<CXXUnresolvedConstructExpr>(parent) || isa<CompoundStmt>(parent)
                 || isa<CXXTypeidExpr>(parent) || isa<DefaultStmt>(parent))
        {
            break;
        }
        else {
            bPotentiallyReadFrom = true;
            bDump = true;
            break;
        }
    } while (true);
    if (bDump)
    {
        report(
             DiagnosticsEngine::Warning,
             "oh dear, what can the matter be?",
              memberExpr->getLocStart())
              << memberExpr->getSourceRange();
        parent->dump();
    }
    if (bPotentiallyReadFrom)
        readFromSet.insert(fieldInfo);
    return true;
}

bool UnusedFields::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const Decl* decl = declRefExpr->getDecl();
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    if (!fieldDecl) {
        return true;
    }
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation()))) {
        return true;
    }
    checkTouched(fieldDecl, declRefExpr);
    return true;
}

void UnusedFields::checkTouched(const FieldDecl* fieldDecl, const Expr* memberExpr) {
    const FunctionDecl* memberExprParentFunction = parentFunctionDecl(memberExpr);
    const CXXMethodDecl* methodDecl = dyn_cast_or_null<CXXMethodDecl>(memberExprParentFunction);

    MyFieldInfo fieldInfo = niceName(fieldDecl);

    // it's touched from somewhere outside a class
    if (!methodDecl) {
        touchedFromOutsideSet.insert(fieldInfo);
        return;
    }

    auto constructorDecl = dyn_cast<CXXConstructorDecl>(methodDecl);
    if (methodDecl->isCopyAssignmentOperator() || methodDecl->isMoveAssignmentOperator()) {
        // ignore move/copy operator, it's self->self
    } else if (constructorDecl && (constructorDecl->isCopyConstructor() || constructorDecl->isMoveConstructor())) {
        // ignore move/copy constructor, it's self->self
    } else if (constructorDecl && memberExprParentFunction->getParent() == fieldDecl->getParent()) {
        // if the field is touched from inside it's parent class constructor
        touchedFromConstructorSet.insert(fieldInfo);
    } else {
        if (memberExprParentFunction->getParent() == fieldDecl->getParent()) {
            touchedFromInsideSet.insert(fieldInfo);
        } else {
           touchedFromOutsideSet.insert(fieldInfo);
        }
    }
}

loplugin::Plugin::Registration< UnusedFields > X("unusedfields", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
