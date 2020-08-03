/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if !defined _WIN32 //TODO, #include <sys/file.h>

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <sys/file.h>
#include <unistd.h>

#include "config_clang.h"

#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"

#if CLANG_VERSION >= 110000
#include "clang/AST/ParentMapContext.h"
#endif

/**
Look for fields that are only assigned to in the constructor using field-init, and can therefore be const.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='constfields' check
  $ ./compilerplugins/clang/constfields.py

and then
  $ for dir in *; do make $dir FORCE_COMPILE_ALL=1 UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='constfieldsrewrite' $dir; done
to auto-remove the method declarations

*/

namespace
{
struct MyFieldInfo
{
    const RecordDecl* parentRecord;
    std::string parentClass;
    std::string fieldName;
    std::string fieldType;
    std::string sourceLocation;
    std::string access;
};
bool operator<(const MyFieldInfo& lhs, const MyFieldInfo& rhs)
{
    return std::tie(lhs.parentClass, lhs.fieldName) < std::tie(rhs.parentClass, rhs.fieldName);
}

// try to limit the voluminous output a little
static std::set<MyFieldInfo> cannotBeConstSet;
static std::set<MyFieldInfo> definitionSet;

/**
 * Wrap the different kinds of callable and callee objects in the clang AST so I can define methods that handle everything.
 */
class CallerWrapper
{
    const CallExpr* m_callExpr;
    const CXXConstructExpr* m_cxxConstructExpr;

public:
    CallerWrapper(const CallExpr* callExpr)
        : m_callExpr(callExpr)
        , m_cxxConstructExpr(nullptr)
    {
    }
    CallerWrapper(const CXXConstructExpr* cxxConstructExpr)
        : m_callExpr(nullptr)
        , m_cxxConstructExpr(cxxConstructExpr)
    {
    }
    unsigned getNumArgs() const
    {
        return m_callExpr ? m_callExpr->getNumArgs() : m_cxxConstructExpr->getNumArgs();
    }
    const Expr* getArg(unsigned i) const
    {
        return m_callExpr ? m_callExpr->getArg(i) : m_cxxConstructExpr->getArg(i);
    }
};
class CalleeWrapper
{
    const FunctionDecl* m_calleeFunctionDecl = nullptr;
    const CXXConstructorDecl* m_cxxConstructorDecl = nullptr;
    const FunctionProtoType* m_functionPrototype = nullptr;

public:
    explicit CalleeWrapper(const FunctionDecl* calleeFunctionDecl)
        : m_calleeFunctionDecl(calleeFunctionDecl)
    {
    }
    explicit CalleeWrapper(const CXXConstructExpr* cxxConstructExpr)
        : m_cxxConstructorDecl(cxxConstructExpr->getConstructor())
    {
    }
    explicit CalleeWrapper(const FunctionProtoType* functionPrototype)
        : m_functionPrototype(functionPrototype)
    {
    }
    unsigned getNumParams() const
    {
        if (m_calleeFunctionDecl)
            return m_calleeFunctionDecl->getNumParams();
        else if (m_cxxConstructorDecl)
            return m_cxxConstructorDecl->getNumParams();
        else if (m_functionPrototype->param_type_begin() == m_functionPrototype->param_type_end())
            // FunctionProtoType will assert if we call getParamTypes() and it has no params
            return 0;
        else
            return m_functionPrototype->getParamTypes().size();
    }
    const QualType getParamType(unsigned i) const
    {
        if (m_calleeFunctionDecl)
            return m_calleeFunctionDecl->getParamDecl(i)->getType();
        else if (m_cxxConstructorDecl)
            return m_cxxConstructorDecl->getParamDecl(i)->getType();
        else
            return m_functionPrototype->getParamTypes()[i];
    }
    std::string getNameAsString() const
    {
        if (m_calleeFunctionDecl)
            return m_calleeFunctionDecl->getNameAsString();
        else if (m_cxxConstructorDecl)
            return m_cxxConstructorDecl->getNameAsString();
        else
            return "";
    }
    CXXMethodDecl const* getAsCXXMethodDecl() const
    {
        if (m_calleeFunctionDecl)
            return dyn_cast<CXXMethodDecl>(m_calleeFunctionDecl);
        return nullptr;
    }
};

class ConstFields : public RecursiveASTVisitor<ConstFields>, public loplugin::Plugin
{
public:
    explicit ConstFields(loplugin::InstantiationData const& data)
        : Plugin(data)
    {
    }

    virtual void run() override;

    bool shouldVisitTemplateInstantiations() const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitFieldDecl(const FieldDecl*);
    bool VisitMemberExpr(const MemberExpr*);
    bool TraverseCXXConstructorDecl(CXXConstructorDecl*);
    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool TraverseFunctionDecl(FunctionDecl*);
    bool TraverseIfStmt(IfStmt*);

private:
    MyFieldInfo niceName(const FieldDecl*);
    void check(const FieldDecl* fieldDecl, const Expr* memberExpr);
    bool isSomeKindOfZero(const Expr* arg);
    bool IsPassedByNonConst(const FieldDecl* fieldDecl, const Stmt* child, CallerWrapper callExpr,
                            CalleeWrapper calleeFunctionDecl);
    llvm::Optional<CalleeWrapper> getCallee(CallExpr const*);

    RecordDecl* insideMoveOrCopyDeclParent = nullptr;
    // For reasons I do not understand, parentFunctionDecl() is not reliable, so
    // we store the parent function on the way down the AST.
    FunctionDecl* insideFunctionDecl = nullptr;
    std::vector<FieldDecl const*> insideConditionalCheckOfMemberSet;
};

void ConstFields::run()
{
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    if (!isUnitTestMode())
    {
        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyFieldInfo& s : cannotBeConstSet)
            output += "write-outside-constructor:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo& s : definitionSet)
            output += "definition:\t" + s.access + "\t" + s.parentClass + "\t" + s.fieldName + "\t"
                      + s.fieldType + "\t" + s.sourceLocation + "\n";
        std::ofstream myfile;
        myfile.open(WORKDIR "/loplugin.constfields.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }
    else
    {
        for (const MyFieldInfo& s : cannotBeConstSet)
            report(DiagnosticsEngine::Warning, "notconst %0", compat::getBeginLoc(s.parentRecord))
                << s.fieldName;
    }
}

MyFieldInfo ConstFields::niceName(const FieldDecl* fieldDecl)
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
    if (idx != std::string::npos)
    {
        aInfo.fieldName = aInfo.fieldName.replace(idx, strlen(SRCDIR), "");
    }
    aInfo.fieldType = fieldDecl->getType().getAsString();

    SourceLocation expansionLoc
        = compiler.getSourceManager().getExpansionLoc(fieldDecl->getLocation());
    StringRef name = getFilenameOfLocation(expansionLoc);
    aInfo.sourceLocation
        = std::string(name.substr(strlen(SRCDIR) + 1)) + ":"
          + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(aInfo.sourceLocation);

    switch (fieldDecl->getAccess())
    {
        case AS_public:
            aInfo.access = "public";
            break;
        case AS_private:
            aInfo.access = "private";
            break;
        case AS_protected:
            aInfo.access = "protected";
            break;
        default:
            aInfo.access = "unknown";
            break;
    }

    return aInfo;
}

bool ConstFields::VisitFieldDecl(const FieldDecl* fieldDecl)
{
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation(fieldDecl))
    {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation())))
    {
        return true;
    }
    definitionSet.insert(niceName(fieldDecl));
    return true;
}

bool ConstFields::TraverseCXXConstructorDecl(CXXConstructorDecl* cxxConstructorDecl)
{
    auto copy = insideMoveOrCopyDeclParent;
    if (!ignoreLocation(cxxConstructorDecl) && cxxConstructorDecl->isThisDeclarationADefinition())
    {
        if (cxxConstructorDecl->isCopyOrMoveConstructor())
            insideMoveOrCopyDeclParent = cxxConstructorDecl->getParent();
    }
    bool ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(cxxConstructorDecl);
    insideMoveOrCopyDeclParent = copy;
    return ret;
}

bool ConstFields::TraverseCXXMethodDecl(CXXMethodDecl* cxxMethodDecl)
{
    auto copy1 = insideMoveOrCopyDeclParent;
    auto copy2 = insideFunctionDecl;
    if (!ignoreLocation(cxxMethodDecl) && cxxMethodDecl->isThisDeclarationADefinition())
    {
        if (cxxMethodDecl->isCopyAssignmentOperator() || cxxMethodDecl->isMoveAssignmentOperator())
            insideMoveOrCopyDeclParent = cxxMethodDecl->getParent();
    }
    insideFunctionDecl = cxxMethodDecl;
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(cxxMethodDecl);
    insideMoveOrCopyDeclParent = copy1;
    insideFunctionDecl = copy2;
    return ret;
}

bool ConstFields::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    auto copy2 = insideFunctionDecl;
    insideFunctionDecl = functionDecl;
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
    insideFunctionDecl = copy2;
    return ret;
}

bool ConstFields::TraverseIfStmt(IfStmt* ifStmt)
{
    FieldDecl const* memberFieldDecl = nullptr;
    Expr const* cond = ifStmt->getCond()->IgnoreParenImpCasts();
    if (auto memberExpr = dyn_cast<MemberExpr>(cond))
    {
        if ((memberFieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl())))
            insideConditionalCheckOfMemberSet.push_back(memberFieldDecl);
    }
    bool ret = RecursiveASTVisitor::TraverseIfStmt(ifStmt);
    if (memberFieldDecl)
        insideConditionalCheckOfMemberSet.pop_back();
    return ret;
}

bool ConstFields::VisitMemberExpr(const MemberExpr* memberExpr)
{
    const ValueDecl* decl = memberExpr->getMemberDecl();
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    if (!fieldDecl)
    {
        return true;
    }
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation(fieldDecl))
    {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation())))
    {
        return true;
    }

    check(fieldDecl, memberExpr);

    return true;
}

void ConstFields::check(const FieldDecl* fieldDecl, const Expr* memberExpr)
{
    auto parentsRange = compiler.getASTContext().getParents(*memberExpr);
    const Stmt* child = memberExpr;
    const Stmt* parent
        = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    // walk up the tree until we find something interesting
    bool bCannotBeConst = false;
    bool bDump = false;
    auto walkUp = [&]() {
        child = parent;
        auto parentsRange = compiler.getASTContext().getParents(*parent);
        parent = parentsRange.begin() == parentsRange.end() ? nullptr
                                                            : parentsRange.begin()->get<Stmt>();
    };
    do
    {
        if (!parent)
        {
            // check if we have an expression like
            //    int& r = m_field;
            auto parentsRange = compiler.getASTContext().getParents(*child);
            if (parentsRange.begin() != parentsRange.end())
            {
                auto varDecl = dyn_cast_or_null<VarDecl>(parentsRange.begin()->get<Decl>());
                // The isImplicit() call is to avoid triggering when we see the vardecl which is part of a for-range statement,
                // which is of type 'T&&' and also an l-value-ref ?
                if (varDecl && !varDecl->isImplicit()
                    && loplugin::TypeCheck(varDecl->getType()).LvalueReference().NonConst())
                {
                    bCannotBeConst = true;
                }
            }
            break;
        }
        if (isa<CXXReinterpretCastExpr>(parent))
        {
            // once we see one of these, there is not much useful we can know
            bCannotBeConst = true;
            break;
        }
        else if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent)
                 || isa<ParenListExpr>(parent) || isa<ArrayInitLoopExpr>(parent)
                 || isa<ExprWithCleanups>(parent))
        {
            walkUp();
        }
        else if (auto unaryOperator = dyn_cast<UnaryOperator>(parent))
        {
            UnaryOperator::Opcode op = unaryOperator->getOpcode();
            if (op == UO_AddrOf || op == UO_PostInc || op == UO_PostDec || op == UO_PreInc
                || op == UO_PreDec)
            {
                bCannotBeConst = true;
            }
            walkUp();
        }
        else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent))
        {
            auto callee = getCallee(operatorCallExpr);
            if (callee)
            {
                // if calling a non-const operator on the field
                auto calleeMethodDecl = callee->getAsCXXMethodDecl();
                if (calleeMethodDecl && operatorCallExpr->getArg(0) == child
                    && !calleeMethodDecl->isConst())
                {
                    bCannotBeConst = true;
                }
                else if (IsPassedByNonConst(fieldDecl, child, operatorCallExpr, *callee))
                {
                    bCannotBeConst = true;
                }
            }
            else
                bCannotBeConst = true; // conservative, could improve
            walkUp();
        }
        else if (auto cxxMemberCallExpr = dyn_cast<CXXMemberCallExpr>(parent))
        {
            const CXXMethodDecl* calleeMethodDecl = cxxMemberCallExpr->getMethodDecl();
            if (calleeMethodDecl)
            {
                // if calling a non-const method on the field
                const Expr* tmp = dyn_cast<Expr>(child);
                if (tmp->isBoundMemberFunction(compiler.getASTContext()))
                {
                    tmp = dyn_cast<MemberExpr>(tmp)->getBase();
                }
                if (cxxMemberCallExpr->getImplicitObjectArgument() == tmp
                    && !calleeMethodDecl->isConst())
                {
                    bCannotBeConst = true;
                    break;
                }
                if (IsPassedByNonConst(fieldDecl, child, cxxMemberCallExpr,
                                       CalleeWrapper(calleeMethodDecl)))
                    bCannotBeConst = true;
            }
            else
                bCannotBeConst = true; // can happen in templates
            walkUp();
        }
        else if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(parent))
        {
            if (IsPassedByNonConst(fieldDecl, child, cxxConstructExpr,
                                   CalleeWrapper(cxxConstructExpr)))
                bCannotBeConst = true;
            walkUp();
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            auto callee = getCallee(callExpr);
            if (callee)
            {
                if (IsPassedByNonConst(fieldDecl, child, callExpr, *callee))
                    bCannotBeConst = true;
            }
            else
                bCannotBeConst = true; // conservative, could improve
            walkUp();
        }
        else if (auto binaryOp = dyn_cast<BinaryOperator>(parent))
        {
            BinaryOperator::Opcode op = binaryOp->getOpcode();
            const bool assignmentOp = op == BO_Assign || op == BO_MulAssign || op == BO_DivAssign
                                      || op == BO_RemAssign || op == BO_AddAssign
                                      || op == BO_SubAssign || op == BO_ShlAssign
                                      || op == BO_ShrAssign || op == BO_AndAssign
                                      || op == BO_XorAssign || op == BO_OrAssign;
            if (assignmentOp)
            {
                if (binaryOp->getLHS() == child)
                    bCannotBeConst = true;
                else if (loplugin::TypeCheck(binaryOp->getLHS()->getType())
                             .LvalueReference()
                             .NonConst())
                    // if the LHS is a non-const reference, we could write to the field later on
                    bCannotBeConst = true;
            }
            walkUp();
        }
        else if (isa<ReturnStmt>(parent))
        {
            if (insideFunctionDecl)
            {
                auto tc = loplugin::TypeCheck(insideFunctionDecl->getReturnType());
                if (tc.LvalueReference().NonConst())
                    bCannotBeConst = true;
            }
            break;
        }
        else if (isa<SwitchStmt>(parent) || isa<WhileStmt>(parent) || isa<ForStmt>(parent)
                 || isa<IfStmt>(parent) || isa<DoStmt>(parent) || isa<CXXForRangeStmt>(parent)
                 || isa<DefaultStmt>(parent))
        {
            break;
        }
        else
        {
            walkUp();
        }
    } while (true);

    if (bDump)
    {
        report(DiagnosticsEngine::Warning, "oh dear, what can the matter be? writtenTo=%0",
               compat::getBeginLoc(memberExpr))
            << bCannotBeConst << memberExpr->getSourceRange();
        if (parent)
        {
            report(DiagnosticsEngine::Note, "parent over here", compat::getBeginLoc(parent))
                << parent->getSourceRange();
            parent->dump();
        }
        memberExpr->dump();
        fieldDecl->getType()->dump();
    }

    if (bCannotBeConst)
    {
        cannotBeConstSet.insert(niceName(fieldDecl));
    }
}

bool ConstFields::IsPassedByNonConst(const FieldDecl* fieldDecl, const Stmt* child,
                                     CallerWrapper callExpr, CalleeWrapper calleeFunctionDecl)
{
    unsigned len = std::min(callExpr.getNumArgs(), calleeFunctionDecl.getNumParams());
    // if it's an array, passing it by value to a method typically means the
    // callee takes a pointer and can modify the array
    if (fieldDecl->getType()->isConstantArrayType())
    {
        for (unsigned i = 0; i < len; ++i)
            if (callExpr.getArg(i) == child)
                if (loplugin::TypeCheck(calleeFunctionDecl.getParamType(i)).Pointer().NonConst())
                    return true;
    }
    else
    {
        for (unsigned i = 0; i < len; ++i)
            if (callExpr.getArg(i) == child)
                if (loplugin::TypeCheck(calleeFunctionDecl.getParamType(i))
                        .LvalueReference()
                        .NonConst())
                    return true;
    }
    return false;
}

llvm::Optional<CalleeWrapper> ConstFields::getCallee(CallExpr const* callExpr)
{
    FunctionDecl const* functionDecl = callExpr->getDirectCallee();
    if (functionDecl)
        return CalleeWrapper(functionDecl);

    // Extract the functionprototype from a type
    clang::Type const* calleeType = callExpr->getCallee()->getType().getTypePtr();
    if (auto pointerType = calleeType->getUnqualifiedDesugaredType()->getAs<clang::PointerType>())
    {
        if (auto prototype = pointerType->getPointeeType()
                                 ->getUnqualifiedDesugaredType()
                                 ->getAs<FunctionProtoType>())
        {
            return CalleeWrapper(prototype);
        }
    }

    return llvm::Optional<CalleeWrapper>();
}

loplugin::Plugin::Registration<ConstFields> X("constfields", false);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
