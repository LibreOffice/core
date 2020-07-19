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
if a field is
- a pointer
- only assigned to in the constructor via a new expression
- unconditionally deleted in the destructor
then it can probably just be allocated inline in the parent object

TODO check for cases where the pointer is passed by non-const reference

Be warned that it produces around 5G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='inlinefields' check
  $ ./compilerplugins/clang/inlinefields.py

and then
  $ for dir in *; do make FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='inlinefieldsremove' $dir; done
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
};
bool operator < (const MyFieldInfo &lhs, const MyFieldInfo &rhs)
{
    return std::tie(lhs.parentClass, lhs.fieldName)
         < std::tie(rhs.parentClass, rhs.fieldName);
}


// try to limit the voluminous output a little
static std::set<MyFieldInfo> excludedSet;
static std::set<MyFieldInfo> definitionSet;
static std::set<MyFieldInfo> deletedInDestructorSet;
static std::set<MyFieldInfo> newedInConstructorSet;

class InlineFields:
    public RecursiveASTVisitor<InlineFields>, public loplugin::Plugin
{
public:
    explicit InlineFields(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyFieldInfo & s : definitionSet)
            output += "definition:\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.sourceLocation + "\n";
        for (const MyFieldInfo & s : excludedSet)
            output += "excluded:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : deletedInDestructorSet)
            output += "deletedInDestructor:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : newedInConstructorSet)
            output += "newedInConstructor:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        std::ofstream myfile;
        myfile.open( WORKDIR "/loplugin.inlinefields.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitFieldDecl( const FieldDecl* );
    bool VisitCXXConstructorDecl( const CXXConstructorDecl* );
    bool VisitCXXDeleteExpr( const CXXDeleteExpr* );
    bool VisitBinaryOperator( const BinaryOperator* );
private:
    MyFieldInfo niceName(const FieldDecl*);
    void checkTouched(const FieldDecl* fieldDecl, const Expr* memberExpr);
};

MyFieldInfo InlineFields::niceName(const FieldDecl* fieldDecl)
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

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( fieldDecl->getLocation() );
    StringRef name = getFilenameOfLocation(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(aInfo.sourceLocation);

    return aInfo;
}

bool InlineFields::VisitFieldDecl( const FieldDecl* fieldDecl )
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
    if (!type->isPointerType())
        return true;
    definitionSet.insert(niceName(fieldDecl));
    return true;
}

bool InlineFields::VisitCXXConstructorDecl( const CXXConstructorDecl* decl )
{
    if( ignoreLocation( decl ) )
        return true;
    if (decl->isCopyOrMoveConstructor())
        return true;
    for(auto it = decl->init_begin(); it != decl->init_end(); ++it)
    {
        const CXXCtorInitializer* init = *it;
        const FieldDecl* fieldDecl = init->getMember();
        if( !fieldDecl || !fieldDecl->getType()->isPointerType() )
            continue;
        auto e = init->getInit();
        if (auto parentListExpr = dyn_cast<ParenListExpr>(e))
            e = parentListExpr->getExpr(0);
        e = e->IgnoreParenImpCasts();
        if( isa<CXXNewExpr>(e) )
            newedInConstructorSet.insert(niceName(fieldDecl));
        else if( isa<CXXNullPtrLiteralExpr>(e) || isa<GNUNullExpr>(e))
            ; // ignore
        else
            excludedSet.insert(niceName(fieldDecl));
    }
    return true;
}

static bool isSameParent(const CXXMethodDecl* cxxMethodDecl, const FieldDecl* fieldDecl)
{
    return cxxMethodDecl->getParent() == dyn_cast<CXXRecordDecl>(fieldDecl->getParent());
}

bool InlineFields::VisitBinaryOperator(const BinaryOperator * binaryOp)
{
    if (binaryOp->getOpcode() != BO_Assign) {
        return true;
    }
    if( ignoreLocation( binaryOp ) )
        return true;
    auto memberExpr = dyn_cast<MemberExpr>(binaryOp->getLHS());
    if (!memberExpr)
        return true;
    auto fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
    if (!fieldDecl || !fieldDecl->getType()->isPointerType()) {
        return true;
    }
    const FunctionDecl* parentFunction = getParentFunctionDecl(binaryOp);
    if (!parentFunction) {
        return true;
    }
    // if the field is being assigned from outside its own constructor or destructor, exclude
    auto constructorDecl = dyn_cast<CXXConstructorDecl>(parentFunction);
    if (constructorDecl && isSameParent(constructorDecl, fieldDecl)) {
        if( isa<CXXNewExpr>(binaryOp->getRHS()) )
            newedInConstructorSet.insert(niceName(fieldDecl));
        else {
            excludedSet.insert(niceName(fieldDecl));
            std::cout << "assign in constructor:" << std::endl;
            binaryOp->getRHS()->dump();
        }
        return true;
    }
    auto destructorDecl = dyn_cast<CXXDestructorDecl>(parentFunction);
    if (destructorDecl && isSameParent(destructorDecl, fieldDecl)) {
        auto e = binaryOp->getRHS()->IgnoreParenImpCasts();
        if( !isa<CXXNullPtrLiteralExpr>(e) && !isa<GNUNullExpr>(e)) {
            excludedSet.insert(niceName(fieldDecl));
            std::cout << "assign in destructor:" << std::endl;
            e->dump();
        }
        return true;
    }
    excludedSet.insert(niceName(fieldDecl));
    return true;
}

bool InlineFields::VisitCXXDeleteExpr(const CXXDeleteExpr * deleteExpr)
{
    if( ignoreLocation( deleteExpr ) )
        return true;
    auto memberExpr = dyn_cast<MemberExpr>(deleteExpr->getArgument()->IgnoreParenImpCasts());
    if (!memberExpr)
        return true;
    auto fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
    if (!fieldDecl || !fieldDecl->getType()->isPointerType()) {
        return true;
    }
    // TODO for some reason, this part is not working properly, it doesn't find the parent
    // function for delete statements properly
    const FunctionDecl* parentFunction = getParentFunctionDecl(deleteExpr);
    if (!parentFunction) {
        return true;
    }
    auto destructorDecl = dyn_cast<CXXDestructorDecl>(parentFunction);
    if (destructorDecl && isSameParent(destructorDecl, fieldDecl)) {
        deletedInDestructorSet.insert(niceName(fieldDecl));
        return true;
    }
    excludedSet.insert(niceName(fieldDecl));
    return true;
}

loplugin::Plugin::Registration< InlineFields > X("inlinefields", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
