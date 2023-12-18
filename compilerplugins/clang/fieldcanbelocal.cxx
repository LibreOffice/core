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

#include "clang/AST/ParentMapContext.h"

/**
 Look for fields on objects that can be local variables.
 Not a particularly smart plugin, generates a lot of false positives, and requires review of the output.
 Mostly looks for fields that are only accessed within a single method.
*/

namespace
{
struct MyFuncInfo
{
    std::string returnType;
    std::string nameAndParams;
    std::string sourceLocation;
};

struct MyFieldInfo
{
    std::string parentClass;
    std::string fieldName;
    std::string fieldType;
    std::string sourceLocation;
};

// try to limit the voluminous output a little
// if the value is nullptr, that indicates that we touched that field from more than one function
static std::unordered_map<const FieldDecl*, const FunctionDecl*> touchedMap;

class FieldCanBeLocal : public loplugin::FilteringPlugin<FieldCanBeLocal>
{
public:
    explicit FieldCanBeLocal(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override;

    bool shouldVisitTemplateInstantiations() const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool TraverseCXXConstructorDecl(CXXConstructorDecl*);
    bool TraverseCXXMethodDecl(CXXMethodDecl*);
    bool TraverseFunctionDecl(FunctionDecl*);

    bool VisitMemberExpr(const MemberExpr*);
    bool VisitDeclRefExpr(const DeclRefExpr*);
    bool VisitInitListExpr(const InitListExpr*);
    bool VisitCXXConstructorDecl(const CXXConstructorDecl*);

private:
    MyFieldInfo niceName(const FieldDecl*);
    MyFuncInfo niceName(const FunctionDecl*);
    std::string toString(SourceLocation loc);
    void checkTouched(const FieldDecl* fieldDecl, const FunctionDecl*);
    bool isSomeKindOfConstant(const Expr* arg);

    RecordDecl* insideMoveOrCopyOrCloneDeclParent = nullptr;
    RecordDecl* insideStreamOutputOperator = nullptr;
    // For reasons I do not understand, parentFunctionDecl() is not reliable, so
    // we store the parent function on the way down the AST.
    FunctionDecl* insideFunctionDecl = nullptr;
};

void FieldCanBeLocal::run()
{
    handler.enableTreeWideAnalysisMode();

    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    if (!isUnitTestMode())
    {
        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        output.reserve(64 * 1024);
        for (const auto& pair : touchedMap)
        {
            if (pair.first->getParent()->isLambda())
                continue;
            MyFieldInfo s = niceName(pair.first);
            output += "definition:\t" + s.parentClass //
                      + "\t" + s.fieldName //
                      + "\t" + s.fieldType //
                      + "\t" + s.sourceLocation //
                      + "\n";
            // we have to output a negative, in case, in some other file, it is touched only once
            if (!pair.second)
                output += "touched:\t" + s.parentClass //
                          + "\t" + s.fieldName //
                          + "\tNegative" //
                          + "\tnowhere.cxx" //
                          + "\n";
            else
            {
                MyFuncInfo s2 = niceName(pair.second);
                output += "touched:\t" + s.parentClass //
                          + "\t" + s.fieldName //
                          + "\t" + s2.returnType + " " + s2.nameAndParams //
                          + "\t" + s2.sourceLocation //
                          + "\n";
            }
        }
        std::ofstream myfile;
        myfile.open(WORKDIR "/loplugin.fieldcanbelocal.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }
    else
    {
        //        for (const MyFieldInfo & s : readFromSet)
        //            report(
        //                DiagnosticsEngine::Warning,
        //                "read %0",
        //                s.parentRecord->getBeginLoc())
        //                << s.fieldName;
    }
}

MyFieldInfo FieldCanBeLocal::niceName(const FieldDecl* fieldDecl)
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
    {
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

    return aInfo;
}

MyFuncInfo FieldCanBeLocal::niceName(const FunctionDecl* functionDecl)
{
    if (functionDecl->getInstantiatedFromMemberFunction())
        functionDecl = functionDecl->getInstantiatedFromMemberFunction();
    else if (functionDecl->getTemplateInstantiationPattern())
        functionDecl = functionDecl->getTemplateInstantiationPattern();

    MyFuncInfo aInfo;
    if (!isa<CXXConstructorDecl>(functionDecl))
    {
        aInfo.returnType = functionDecl->getReturnType().getCanonicalType().getAsString();
    }
    else
    {
        aInfo.returnType = "";
    }

    if (isa<CXXMethodDecl>(functionDecl))
    {
        const CXXRecordDecl* recordDecl = dyn_cast<CXXMethodDecl>(functionDecl)->getParent();
        aInfo.nameAndParams += recordDecl->getQualifiedNameAsString();
        aInfo.nameAndParams += "::";
    }
    aInfo.nameAndParams += functionDecl->getNameAsString() + "(";
    bool bFirst = true;
    for (const ParmVarDecl* pParmVarDecl : functionDecl->parameters())
    {
        if (bFirst)
            bFirst = false;
        else
            aInfo.nameAndParams += ",";
        aInfo.nameAndParams += pParmVarDecl->getType().getCanonicalType().getAsString();
    }
    aInfo.nameAndParams += ")";
    if (isa<CXXMethodDecl>(functionDecl) && dyn_cast<CXXMethodDecl>(functionDecl)->isConst())
    {
        aInfo.nameAndParams += " const";
    }

    aInfo.sourceLocation = toString(functionDecl->getLocation());

    return aInfo;
}

std::string FieldCanBeLocal::toString(SourceLocation loc)
{
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc(loc);
    StringRef name = getFilenameOfLocation(expansionLoc);
    std::string sourceLocation
        = std::string(name.substr(strlen(SRCDIR) + 1)) + ":"
          + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(sourceLocation);
    return sourceLocation;
}

bool FieldCanBeLocal::TraverseCXXConstructorDecl(CXXConstructorDecl* cxxConstructorDecl)
{
    auto copy = insideMoveOrCopyOrCloneDeclParent;
    if (!ignoreLocation(cxxConstructorDecl->getBeginLoc())
        && cxxConstructorDecl->isThisDeclarationADefinition())
    {
        if (cxxConstructorDecl->isCopyOrMoveConstructor())
            insideMoveOrCopyOrCloneDeclParent = cxxConstructorDecl->getParent();
    }
    bool ret = RecursiveASTVisitor::TraverseCXXConstructorDecl(cxxConstructorDecl);
    insideMoveOrCopyOrCloneDeclParent = copy;
    return ret;
}

bool FieldCanBeLocal::TraverseCXXMethodDecl(CXXMethodDecl* cxxMethodDecl)
{
    auto copy1 = insideMoveOrCopyOrCloneDeclParent;
    auto copy2 = insideFunctionDecl;
    if (!ignoreLocation(cxxMethodDecl->getBeginLoc())
        && cxxMethodDecl->isThisDeclarationADefinition())
    {
        if (cxxMethodDecl->isCopyAssignmentOperator() || cxxMethodDecl->isMoveAssignmentOperator()
            || (cxxMethodDecl->getIdentifier()
                && (compat::starts_with(cxxMethodDecl->getName(), "Clone")
                    || compat::starts_with(cxxMethodDecl->getName(), "clone")
                    || compat::starts_with(cxxMethodDecl->getName(), "createClone"))))
            insideMoveOrCopyOrCloneDeclParent = cxxMethodDecl->getParent();
        // these are similar in that they tend to simply enumerate all the fields of an object without putting
        // them to some useful purpose
        auto op = cxxMethodDecl->getOverloadedOperator();
        if (op == OO_EqualEqual || op == OO_ExclaimEqual)
            insideMoveOrCopyOrCloneDeclParent = cxxMethodDecl->getParent();
    }
    insideFunctionDecl = cxxMethodDecl;
    bool ret = RecursiveASTVisitor::TraverseCXXMethodDecl(cxxMethodDecl);
    insideMoveOrCopyOrCloneDeclParent = copy1;
    insideFunctionDecl = copy2;
    return ret;
}

bool FieldCanBeLocal::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    auto copy1 = insideStreamOutputOperator;
    auto copy2 = insideFunctionDecl;
    auto copy3 = insideMoveOrCopyOrCloneDeclParent;
    if (functionDecl->getLocation().isValid() && !ignoreLocation(functionDecl->getBeginLoc())
        && functionDecl->isThisDeclarationADefinition())
    {
        auto op = functionDecl->getOverloadedOperator();
        if (op == OO_LessLess && functionDecl->getNumParams() == 2)
        {
            QualType qt = functionDecl->getParamDecl(1)->getType();
            insideStreamOutputOperator
                = qt.getNonReferenceType().getUnqualifiedType()->getAsCXXRecordDecl();
        }
        // these are similar in that they tend to simply enumerate all the fields of an object without putting
        // them to some useful purpose
        if (op == OO_EqualEqual || op == OO_ExclaimEqual)
        {
            QualType qt = functionDecl->getParamDecl(1)->getType();
            insideMoveOrCopyOrCloneDeclParent
                = qt.getNonReferenceType().getUnqualifiedType()->getAsCXXRecordDecl();
        }
    }
    insideFunctionDecl = functionDecl;
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
    insideStreamOutputOperator = copy1;
    insideFunctionDecl = copy2;
    insideMoveOrCopyOrCloneDeclParent = copy3;
    return ret;
}

bool FieldCanBeLocal::VisitMemberExpr(const MemberExpr* memberExpr)
{
    const ValueDecl* decl = memberExpr->getMemberDecl();
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    if (!fieldDecl)
    {
        return true;
    }
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation(fieldDecl->getBeginLoc()))
    {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation())))
    {
        return true;
    }

    if (insideMoveOrCopyOrCloneDeclParent || insideStreamOutputOperator)
    {
        RecordDecl const* cxxRecordDecl1 = fieldDecl->getParent();
        // we don't care about reads from a field when inside the copy/move constructor/operator= for that field
        if (cxxRecordDecl1 && (cxxRecordDecl1 == insideMoveOrCopyOrCloneDeclParent))
            return true;
        // we don't care about reads when the field is being used in an output operator, this is normally
        // debug stuff
        if (cxxRecordDecl1 && (cxxRecordDecl1 == insideStreamOutputOperator))
            return true;
    }

    checkTouched(fieldDecl, insideFunctionDecl);

    return true;
}

bool FieldCanBeLocal::VisitDeclRefExpr(const DeclRefExpr* declRefExpr)
{
    const Decl* decl = declRefExpr->getDecl();
    const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    if (!fieldDecl)
    {
        return true;
    }
    fieldDecl = fieldDecl->getCanonicalDecl();
    if (ignoreLocation(fieldDecl->getBeginLoc()))
    {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getLocation())))
    {
        return true;
    }

    checkTouched(fieldDecl, insideFunctionDecl);

    return true;
}

// fields that are assigned via member initialisers do not get visited in VisitDeclRef, so
// have to do it here
bool FieldCanBeLocal::VisitCXXConstructorDecl(const CXXConstructorDecl* cxxConstructorDecl)
{
    if (ignoreLocation(cxxConstructorDecl->getBeginLoc()))
    {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(
            compiler.getSourceManager().getSpellingLoc(cxxConstructorDecl->getLocation())))
    {
        return true;
    }

    // templates make EvaluateAsInt crash inside clang
    if (cxxConstructorDecl->isDependentContext())
        return true;

    // we don't care about writes to a field when inside the copy/move constructor/operator= for that field
    if (insideMoveOrCopyOrCloneDeclParent
        && cxxConstructorDecl->getParent() == insideMoveOrCopyOrCloneDeclParent)
        return true;

    for (auto it = cxxConstructorDecl->init_begin(); it != cxxConstructorDecl->init_end(); ++it)
    {
        const CXXCtorInitializer* init = *it;
        const FieldDecl* fieldDecl = init->getMember();
        if (!fieldDecl)
            continue;
        if (init->getInit() && isSomeKindOfConstant(init->getInit()))
            checkTouched(fieldDecl, cxxConstructorDecl);
        else
            touchedMap[fieldDecl] = nullptr;
    }
    return true;
}

// Fields that are assigned via init-list-expr do not get visited in VisitDeclRef, so
// have to do it here.
bool FieldCanBeLocal::VisitInitListExpr(const InitListExpr* initListExpr)
{
    if (ignoreLocation(initListExpr->getBeginLoc()))
        return true;

    QualType varType = initListExpr->getType().getDesugaredType(compiler.getASTContext());
    auto recordType = varType->getAs<RecordType>();
    if (!recordType)
        return true;

    auto recordDecl = recordType->getDecl();
    for (auto it = recordDecl->field_begin(); it != recordDecl->field_end(); ++it)
    {
        checkTouched(*it, insideFunctionDecl);
    }

    return true;
}

void FieldCanBeLocal::checkTouched(const FieldDecl* fieldDecl, const FunctionDecl* functionDecl)
{
    auto methodDecl = dyn_cast_or_null<CXXMethodDecl>(functionDecl);
    if (!methodDecl)
    {
        touchedMap[fieldDecl] = nullptr;
        return;
    }
    if (methodDecl->getParent() != fieldDecl->getParent())
    {
        touchedMap[fieldDecl] = nullptr;
        return;
    }
    auto it = touchedMap.find(fieldDecl);
    if (it == touchedMap.end())
        touchedMap.emplace(fieldDecl, functionDecl);
    else if (it->second != functionDecl)
        it->second = nullptr;
}

bool FieldCanBeLocal::isSomeKindOfConstant(const Expr* arg)
{
    assert(arg);
    if (arg->isValueDependent())
        return false;
    return arg->isCXX11ConstantExpr(compiler.getASTContext());
}

loplugin::Plugin::Registration<FieldCanBeLocal> X("fieldcanbelocal", false);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
