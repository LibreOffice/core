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
    const RecordDecl* parentRecord;
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

    virtual void run() override;

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitFieldDecl( const FieldDecl* );
    bool VisitMemberExpr( const MemberExpr* );
    bool VisitDeclRefExpr( const DeclRefExpr* );
    bool TraverseCXXConstructorDecl( CXXConstructorDecl* );
    bool TraverseCXXMethodDecl( CXXMethodDecl* );
    bool TraverseFunctionDecl( FunctionDecl* );
private:
    MyFieldInfo niceName(const FieldDecl*);
    void checkTouchedFromOutside(const FieldDecl* fieldDecl, const Expr* memberExpr);
    void checkWriteOnly(const FieldDecl* fieldDecl, const Expr* memberExpr);
    RecordDecl * insideMoveOrCopyDeclParent;
    RecordDecl * insideStreamOutputOperator;
};

void UnusedFields::run()
{
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    if (!isUnitTestMode())
    {
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
        std::ofstream myfile;
        myfile.open( SRCDIR "/loplugin.unusedfields.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }
    else
    {
        for (const MyFieldInfo & s : readFromSet)
        {
            report(
                DiagnosticsEngine::Warning,
                "read %0",
                s.parentRecord->getLocStart())
                << s.fieldName;
        }
    }
}


MyFieldInfo UnusedFields::niceName(const FieldDecl* fieldDecl)
{
    MyFieldInfo aInfo;

    const RecordDecl* recordDecl = fieldDecl->getParent();

    if (const CXXRecordDecl* cxxRecordDecl = dyn_cast<CXXRecordDecl>(recordDecl))
    {
        if (cxxRecordDecl->getTemplateInstantiationPattern())
            cxxRecordDecl = cxxRecordDecl->getTemplateInstantiationPattern();
        aInfo.parentRecord = cxxRecordDecl;
        aInfo.parentClass = cxxRecordDecl->getQualifiedNameAsString();
    }
    else
    {
        aInfo.parentRecord = recordDecl;
        aInfo.parentClass = recordDecl->getQualifiedNameAsString();
    }

    aInfo.fieldName = fieldDecl->getNameAsString();
    // sometimes the name (if it's an anonymous thing) contains the full path of the build folder, which we don't need
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

bool UnusedFields::TraverseCXXConstructorDecl(CXXConstructorDecl* cxxConstructorDecl)
{
    if (!ignoreLocation(cxxConstructorDecl) && cxxConstructorDecl->isThisDeclarationADefinition())
    {
        if (cxxConstructorDecl->isCopyOrMoveConstructor())
            insideMoveOrCopyDeclParent = cxxConstructorDecl->getParent();
    }
    bool ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(cxxConstructorDecl);
    insideMoveOrCopyDeclParent = nullptr;
    return ret;
}

bool UnusedFields::TraverseCXXMethodDecl(CXXMethodDecl* cxxMethodDecl)
{
    if (!ignoreLocation(cxxMethodDecl) && cxxMethodDecl->isThisDeclarationADefinition())
    {
        if (cxxMethodDecl->isCopyAssignmentOperator() || cxxMethodDecl->isMoveAssignmentOperator())
            insideMoveOrCopyDeclParent = cxxMethodDecl->getParent();
    }
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(cxxMethodDecl);
    insideMoveOrCopyDeclParent = nullptr;
    return ret;
}

bool UnusedFields::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    if (functionDecl->getLocation().isValid() && !ignoreLocation(functionDecl) && functionDecl->isThisDeclarationADefinition())
    {
        if (functionDecl->getOverloadedOperator() == OO_LessLess
            && functionDecl->getNumParams() == 2)
        {
            QualType qt = functionDecl->getParamDecl(1)->getType();
            insideStreamOutputOperator = qt.getNonReferenceType().getUnqualifiedType()->getAsCXXRecordDecl();
        }
    }
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
    insideStreamOutputOperator = nullptr;
    return ret;
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

    checkTouchedFromOutside(fieldDecl, memberExpr);

    checkWriteOnly(fieldDecl, memberExpr);

    return true;
}

void UnusedFields::checkWriteOnly(const FieldDecl* fieldDecl, const Expr* memberExpr)
{
    if (insideMoveOrCopyDeclParent || insideStreamOutputOperator)
    {
        RecordDecl const * cxxRecordDecl1 = fieldDecl->getParent();
        // we don't care about reads from a field when inside the copy/move constructor/operator= for that field
        if (cxxRecordDecl1 && (cxxRecordDecl1 == insideMoveOrCopyDeclParent))
            return;
        // we don't care about reads when the field is being used in an output operator, this is normally
        // debug stuff
        if (cxxRecordDecl1 && (cxxRecordDecl1 == insideStreamOutputOperator))
            return;
    }

    auto parentsRange = compiler.getASTContext().getParents(*memberExpr);
    const Stmt* child = memberExpr;
    const Stmt* parent = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    // walk up the tree until we find something interesting
    bool bPotentiallyReadFrom = false;
    bool bDump = false;
    auto walkupUp = [&]() {
       child = parent;
       auto parentsRange = compiler.getASTContext().getParents(*parent);
       parent = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    };
    do
    {
        if (!parent)
        {
            // check if we're inside a CXXCtorInitializer or a VarDecl
            auto parentsRange = compiler.getASTContext().getParents(*child);
            if ( parentsRange.begin() != parentsRange.end())
            {
                const Decl* decl = parentsRange.begin()->get<Decl>();
                if (decl && (isa<CXXConstructorDecl>(decl) || isa<VarDecl>(decl)))
                    bPotentiallyReadFrom = true;
            }
            if (!bPotentiallyReadFrom)
                return;
            break;
        }
        if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent) || isa<ParenListExpr>(parent)
#if CLANG_VERSION >= 40000
             || isa<ArrayInitLoopExpr>(parent)
#endif
             || isa<ExprWithCleanups>(parent))
        {
            walkupUp();
        }
        else if (auto unaryOperator = dyn_cast<UnaryOperator>(parent))
        {
            UnaryOperator::Opcode op = unaryOperator->getOpcode();
            if (memberExpr->getType()->isArrayType() && op == UO_Deref)
            {
                // ignore, deref'ing an array does not count as a read
            }
            else if (op == UO_AddrOf || op == UO_Deref
                || op == UO_Plus || op == UO_Minus
                || op == UO_Not || op == UO_LNot
                || op == UO_PreInc || op == UO_PostInc
                || op == UO_PreDec || op == UO_PostDec)
            {
                bPotentiallyReadFrom = true;
                break;
            }
            walkupUp();
        }
        else if (auto caseStmt = dyn_cast<CaseStmt>(parent))
        {
            bPotentiallyReadFrom = caseStmt->getLHS() == child || caseStmt->getRHS() == child;
            break;
        }
        else if (auto ifStmt = dyn_cast<IfStmt>(parent))
        {
            bPotentiallyReadFrom = ifStmt->getCond() == child;
            break;
        }
        else if (auto doStmt = dyn_cast<DoStmt>(parent))
        {
            bPotentiallyReadFrom = doStmt->getCond() == child;
            break;
        }
        else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(parent))
        {
            if (arraySubscriptExpr->getIdx() == child)
            {
                bPotentiallyReadFrom = true;
                break;
            }
            walkupUp();
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
                else if (name == "clear" || name == "dispose" || name == "clearAndDispose" || name == "swap")
                    // we're abusing the write-only analysis here to look for fields which don't have anything useful
                    // being done to them, so we're ignoring things like std::vector::clear, std::vector::swap,
                    // and VclPtr::clearAndDispose
                    ;
                else
                    bPotentiallyReadFrom = true;
            }
            else
                bPotentiallyReadFrom = true;
            break;
        }
        else if (auto binaryOp = dyn_cast<BinaryOperator>(parent))
        {
            BinaryOperator::Opcode op = binaryOp->getOpcode();
            // If the child is on the LHS and it is an assignment op, we are obviously not reading from it
            const bool assignmentOp = op == BO_Assign || op == BO_MulAssign
                || op == BO_DivAssign || op == BO_RemAssign || op == BO_AddAssign
                || op == BO_SubAssign || op == BO_ShlAssign || op == BO_ShrAssign
                || op == BO_AndAssign || op == BO_XorAssign || op == BO_OrAssign;
            if (!(binaryOp->getLHS() == child && assignmentOp)) {
                bPotentiallyReadFrom = true;
            }
            break;
        }
        else if (isa<ReturnStmt>(parent)
                 || isa<CXXConstructExpr>(parent)
                 || isa<ConditionalOperator>(parent)
                 || isa<SwitchStmt>(parent)
                 || isa<DeclStmt>(parent)
                 || isa<WhileStmt>(parent)
                 || isa<CXXNewExpr>(parent)
                 || isa<ForStmt>(parent)
                 || isa<InitListExpr>(parent)
                 || isa<CXXDependentScopeMemberExpr>(parent)
                 || isa<UnresolvedMemberExpr>(parent)
                 || isa<MaterializeTemporaryExpr>(parent))
        {
            bPotentiallyReadFrom = true;
            break;
        }
        else if (isa<CXXDeleteExpr>(parent)
                 || isa<UnaryExprOrTypeTraitExpr>(parent)
                 || isa<CXXUnresolvedConstructExpr>(parent)
                 || isa<CompoundStmt>(parent)
                 || isa<LabelStmt>(parent)
                 || isa<CXXForRangeStmt>(parent)
                 || isa<CXXTypeidExpr>(parent)
                 || isa<DefaultStmt>(parent))
        {
            break;
        }
        else
        {
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
        report(
             DiagnosticsEngine::Note,
             "parent over here",
              parent->getLocStart())
              << parent->getSourceRange();
        parent->dump();
        memberExpr->dump();
    }

    MyFieldInfo fieldInfo = niceName(fieldDecl);
    if (bPotentiallyReadFrom)
        readFromSet.insert(fieldInfo);
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
    checkTouchedFromOutside(fieldDecl, declRefExpr);
    return true;
}

void UnusedFields::checkTouchedFromOutside(const FieldDecl* fieldDecl, const Expr* memberExpr) {
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

loplugin::Plugin::Registration< UnusedFields > X("unusedfields", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
