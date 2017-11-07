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
#include <unordered_map>


#include "clang/AST/Attr.h"

#include "plugin.hxx"
#include "compat.hxx"

/**
 Find methods that are only called from inside their own class, and are only called from one spot.
 They are candidates to be removed and have their code inlined into the call site.


 TODO if a method has only one call-site, and that call site is inside a constructor
     then it's probably worth inlining, since it's probably an old method that was intended to be shared amongst
     multiple constructors
*/

namespace {

struct MyFuncInfo
{
    std::string access;
    std::string returnType;
    std::string nameAndParams;
    std::string sourceLocation;

};
bool operator < (const MyFuncInfo &lhs, const MyFuncInfo &rhs)
{
    return std::tie(lhs.returnType, lhs.nameAndParams)
         < std::tie(rhs.returnType, rhs.nameAndParams);
}

// try to limit the voluminous output a little

static std::set<std::pair<std::string, MyFuncInfo>> calledFromSet;
static std::set<MyFuncInfo> definitionSet;
static std::set<MyFuncInfo> calledFromOutsideSet;
static std::set<MyFuncInfo> largeFunctionSet;
static std::set<MyFuncInfo> addressOfSet;


class ExpandableMethods:
    public RecursiveASTVisitor<ExpandableMethods>, public loplugin::Plugin
{
public:
    explicit ExpandableMethods(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile

        std::string output;
        for (const MyFuncInfo & s : definitionSet)
            output += "definition:\t" + s.access + "\t" + s.returnType + "\t" + s.nameAndParams + "\t" + s.sourceLocation + "\n";
        for (const MyFuncInfo & s : calledFromOutsideSet)
            output += "outside:\t" + s.returnType + "\t" + s.nameAndParams + "\n";
        for (const std::pair<std::string,MyFuncInfo> & s : calledFromSet)
            output += "calledFrom:\t" + s.first
                       + "\t" + s.second.returnType + "\t" + s.second.nameAndParams + "\n";
        for (const MyFuncInfo & s : largeFunctionSet)
            output += "large:\t" + s.returnType + "\t" + s.nameAndParams + "\n";
        for (const MyFuncInfo & s : addressOfSet)
            output += "addrof:\t" + s.returnType + "\t" + s.nameAndParams + "\n";
        std::ofstream myfile;
        myfile.open( SRCDIR "/loplugin.expandablemethods.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitFunctionDecl( const FunctionDecl* );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool VisitMemberExpr( const MemberExpr* );
    // interception methods for FunctionDecl and all its subclasses
    bool TraverseFunctionDecl( FunctionDecl* );
    bool TraverseCXXMethodDecl( CXXMethodDecl* );
    bool TraverseCXXConstructorDecl( CXXConstructorDecl* );
    bool TraverseCXXConversionDecl( CXXConversionDecl* );
    bool TraverseCXXDestructorDecl( CXXDestructorDecl* );

private:
    MyFuncInfo niceName(const FunctionDecl* functionDecl);
    std::string toString(SourceLocation loc);
    void functionTouchedFromExpr( const FunctionDecl* calleeFunctionDecl, const Expr* expr );
    bool isCalleeFunctionInteresting( const FunctionDecl* );

    // I use traverse and a member variable because I cannot find a reliable way of walking back up the AST tree using the parentStmt() stuff
    std::vector<const FunctionDecl*> maTraversingFunctions;
};

MyFuncInfo ExpandableMethods::niceName(const FunctionDecl* functionDecl)
{
    if (functionDecl->getInstantiatedFromMemberFunction())
        functionDecl = functionDecl->getInstantiatedFromMemberFunction();
    else if (functionDecl->getClassScopeSpecializationPattern())
        functionDecl = functionDecl->getClassScopeSpecializationPattern();
// workaround clang-3.5 issue
#if CLANG_VERSION >= 30600
    else if (functionDecl->getTemplateInstantiationPattern())
        functionDecl = functionDecl->getTemplateInstantiationPattern();
#endif

    MyFuncInfo aInfo;
    switch (functionDecl->getAccess())
    {
    case AS_public: aInfo.access = "public"; break;
    case AS_private: aInfo.access = "private"; break;
    case AS_protected: aInfo.access = "protected"; break;
    default: aInfo.access = "unknown"; break;
    }
    if (!isa<CXXConstructorDecl>(functionDecl)) {
        aInfo.returnType = compat::getReturnType(*functionDecl).getCanonicalType().getAsString();
    } else {
        aInfo.returnType = "";
    }

    if (isa<CXXMethodDecl>(functionDecl)) {
        const CXXRecordDecl* recordDecl = dyn_cast<CXXMethodDecl>(functionDecl)->getParent();
        aInfo.nameAndParams += recordDecl->getQualifiedNameAsString();
        aInfo.nameAndParams += "::";
    }
    aInfo.nameAndParams += functionDecl->getNameAsString() + "(";
    bool bFirst = true;
    for (const ParmVarDecl *pParmVarDecl : compat::parameters(*functionDecl)) {
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

    aInfo.sourceLocation = toString( functionDecl->getLocation() );

    return aInfo;
}

std::string ExpandableMethods::toString(SourceLocation loc)
{
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( loc );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    std::string sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(sourceLocation);
    return sourceLocation;
}

bool ExpandableMethods::VisitFunctionDecl( const FunctionDecl* functionDecl )
{
    const FunctionDecl* canonicalFunctionDecl = functionDecl->getCanonicalDecl();
    if (!isCalleeFunctionInteresting(canonicalFunctionDecl)) {
        return true;
    }
    definitionSet.insert(niceName(canonicalFunctionDecl));

    if (functionDecl->doesThisDeclarationHaveABody()) {
        bool bLargeFunction = false;
        if (const CompoundStmt* compoundStmt = dyn_cast<CompoundStmt>(functionDecl->getBody())) {
            if (compoundStmt->size() > 1) {
                bLargeFunction = true;
            }
            if (!bLargeFunction) {
                 auto s1 = compiler.getSourceManager().getCharacterData(compoundStmt->getLBracLoc());
                 auto s2 = compiler.getSourceManager().getCharacterData(compoundStmt->getRBracLoc());
                 bLargeFunction = (s2 - s1) > 40;
                 // any function that uses a parameter more than once
                 if (!bLargeFunction) {
                     StringRef bodyText(s1, s2-s1);
                     for (const ParmVarDecl* param : compat::parameters(*functionDecl)) {
                         StringRef name = param->getName();
                         if (name.empty())
                             continue;
                         size_t idx = bodyText.find(name);
                         if (idx != StringRef::npos && bodyText.find(name, idx+1) != StringRef::npos) {
                             bLargeFunction = true;
                             break;
                         }
                     }
                 }
            }
        }
        if (bLargeFunction) {
            largeFunctionSet.insert(niceName(canonicalFunctionDecl));
        }
    }
    return true;
}

bool ExpandableMethods::TraverseFunctionDecl( FunctionDecl* p )
{
    maTraversingFunctions.push_back(p);
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(p);
    maTraversingFunctions.pop_back();
    return ret;
}
bool ExpandableMethods::TraverseCXXMethodDecl( CXXMethodDecl* p )
{
    maTraversingFunctions.push_back(p);
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(p);
    maTraversingFunctions.pop_back();
    return ret;
}
bool ExpandableMethods::TraverseCXXConstructorDecl( CXXConstructorDecl* p )
{
    maTraversingFunctions.push_back(p);
    bool ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(p);
    maTraversingFunctions.pop_back();
    return ret;
}
bool ExpandableMethods::TraverseCXXConversionDecl( CXXConversionDecl* p )
{
    maTraversingFunctions.push_back(p);
    bool ret = RecursiveASTVisitor::TraverseCXXConversionDecl(p);
    maTraversingFunctions.pop_back();
    return ret;
}
bool ExpandableMethods::TraverseCXXDestructorDecl( CXXDestructorDecl* p )
{
    maTraversingFunctions.push_back(p);
    bool ret = RecursiveASTVisitor::TraverseCXXDestructorDecl(p);
    maTraversingFunctions.pop_back();
    return ret;
}

bool ExpandableMethods::VisitMemberExpr( const MemberExpr* memberExpr )
{
    const FunctionDecl* functionDecl = dyn_cast<FunctionDecl>(memberExpr->getMemberDecl());
    if (functionDecl) {
        functionTouchedFromExpr(functionDecl, memberExpr);
    }
    return true;
}

bool ExpandableMethods::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const FunctionDecl* functionDecl = dyn_cast<FunctionDecl>(declRefExpr->getDecl());
    if (functionDecl) {
        functionTouchedFromExpr(functionDecl, declRefExpr);
    }
    return true;
}

void ExpandableMethods::functionTouchedFromExpr( const FunctionDecl* calleeFunctionDecl, const Expr* expr )
{
    const FunctionDecl* canonicalFunctionDecl = calleeFunctionDecl->getCanonicalDecl();
    if (!isCalleeFunctionInteresting(canonicalFunctionDecl)) {
        return;
    }

    calledFromSet.emplace(toString(expr->getLocStart()), niceName(canonicalFunctionDecl));

    if (const UnaryOperator* unaryOp = dyn_cast_or_null<UnaryOperator>(getParentStmt(expr))) {
        if (unaryOp->getOpcode() == UO_AddrOf) {
            addressOfSet.insert(niceName(canonicalFunctionDecl));
        }
    }

    const CXXMethodDecl* calleeMethodDecl = dyn_cast<CXXMethodDecl>(calleeFunctionDecl);
    if (maTraversingFunctions.empty())
    {
        calledFromOutsideSet.insert(niceName(canonicalFunctionDecl));
    }
    else
    {
        const CXXMethodDecl* callsiteParentMethodDecl = dyn_cast<CXXMethodDecl>(maTraversingFunctions.back());
        if (!callsiteParentMethodDecl
            || calleeMethodDecl->getParent() != callsiteParentMethodDecl->getParent())
        {
            calledFromOutsideSet.insert(niceName(canonicalFunctionDecl));
        }
    }
}

bool ExpandableMethods::isCalleeFunctionInteresting(const FunctionDecl* functionDecl)
{
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl)) {
        return false;
    }
    if (isa<CXXDestructorDecl>(functionDecl)) {
        return false;
    }
    if (functionDecl->isDeleted() || functionDecl->isDefaulted()) {
        return false;
    }
    if (isa<CXXConstructorDecl>(functionDecl)
        && dyn_cast<CXXConstructorDecl>(functionDecl)->isCopyOrMoveConstructor())
    {
        return false;
    }
    if (!functionDecl->getLocation().isValid() || ignoreLocation(functionDecl)) {
        return false;
    }
    const CXXMethodDecl* methodDecl = dyn_cast<CXXMethodDecl>(functionDecl);
    if (!methodDecl || methodDecl->isVirtual()) {
        return false;
    }
    return true;
}

loplugin::Plugin::Registration< ExpandableMethods > X("expandablemethods", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
