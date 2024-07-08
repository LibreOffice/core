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
#include "config_clang.h"
#include "plugin.hxx"

/**
This looks for unused enum constants

We search for 3 things
(a) constants that are declared but never used
(b) constants only used in a "read" fashion i.e. we compare stuff against them, but we never store a value anywhere
(c) constants only used in a "write" fashion i.e. we store a value, but never check for that value

(a) is fairly reliable but (b) and (c) will need some checking before acting on.

Be warned that it produces around 5G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE=all COMPILER_PLUGIN_TOOL='unusedenumconstants' check
  $ ./compilerplugins/clang/unusedenumconstants.py

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

*/

namespace {

struct MyFieldInfo
{
    std::string parentClass;
    std::string fieldName;
    std::string sourceLocation;
    SourceLocation loc;
};
bool operator < (const MyFieldInfo &lhs, const MyFieldInfo &rhs)
{
    return std::tie(lhs.parentClass, lhs.fieldName)
         < std::tie(rhs.parentClass, rhs.fieldName);
}


// try to limit the voluminous output a little
static std::set<MyFieldInfo> definitionSet;
static std::set<MyFieldInfo> writeSet;
static std::set<MyFieldInfo> readSet;


class UnusedEnumConstants:
    public loplugin::FilteringPlugin<UnusedEnumConstants>
{
public:
    explicit UnusedEnumConstants(loplugin::InstantiationData const & data): FilteringPlugin(data) {}

    virtual void run() override
    {
        handler.enableTreeWideAnalysisMode();

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        if (!isUnitTestMode())
        {
            // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
            // writing to the same logfile
            std::string output;
            for (const MyFieldInfo & s : definitionSet)
                output += "definition:\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.sourceLocation + "\n";
            for (const MyFieldInfo & s : writeSet)
                output += "write:\t" + s.parentClass + "\t" + s.fieldName + "\n";
            for (const MyFieldInfo & s : readSet)
                output += "read:\t" + s.parentClass + "\t" + s.fieldName + "\n";
            std::ofstream myfile;
            myfile.open( WORKDIR "/loplugin.unusedenumconstants.log", std::ios::app | std::ios::out);
            myfile << output;
            myfile.close();
        }
        else
        {
            for (const MyFieldInfo& s : writeSet)
                report(DiagnosticsEngine::Warning, "write %0", s.loc)
                    << s.fieldName;
            for (const MyFieldInfo& s : readSet)
                report(DiagnosticsEngine::Warning, "read %0", s.loc)
                    << s.fieldName;
        }
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitEnumConstantDecl( const EnumConstantDecl * );
    bool VisitDeclRefExpr( const DeclRefExpr * );
private:
    MyFieldInfo niceName(const EnumConstantDecl*);
};

MyFieldInfo UnusedEnumConstants::niceName(const EnumConstantDecl* enumConstantDecl)
{
    MyFieldInfo aInfo;

    aInfo.parentClass = enumConstantDecl->getType().getAsString();
    aInfo.fieldName = enumConstantDecl->getNameAsString();
    // sometimes the name (if it's anonymous thing) contains the full path of the build folder, which we don't need
    size_t idx = aInfo.fieldName.find(SRCDIR);
    if (idx != std::string::npos) {
        aInfo.fieldName = aInfo.fieldName.replace(idx, strlen(SRCDIR), "");
    }

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( enumConstantDecl->getLocation() );
    StringRef name = getFilenameOfLocation(expansionLoc);
    aInfo.loc = expansionLoc;
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(aInfo.sourceLocation);

    return aInfo;
}

bool UnusedEnumConstants::VisitEnumConstantDecl( const EnumConstantDecl* enumConstantDecl )
{
    enumConstantDecl = enumConstantDecl->getCanonicalDecl();
    if (ignoreLocation( enumConstantDecl )) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(enumConstantDecl->getLocation()))) {
        return true;
    }

    definitionSet.insert(niceName(enumConstantDecl));
    return true;
}

bool UnusedEnumConstants::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    auto enumConstantDecl = dyn_cast<EnumConstantDecl>(declRefExpr->getDecl());
    if (!enumConstantDecl) {
        return true;
    }
    enumConstantDecl = enumConstantDecl->getCanonicalDecl();
    if (ignoreLocation(enumConstantDecl)) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(enumConstantDecl->getLocation()))) {
        return true;
    }

    const Stmt * parent = declRefExpr;
    const Stmt * child = nullptr;

walk_up:
    child = parent;
    parent = getParentStmt(parent);
    bool bWrite = false;
    bool bRead = false;
    bool bDump = false;
    if (!parent)
    {
        // Could probably do better here.
        // Sometimes this is a constructor-initialiser-expression, so just make a pessimistic assumption.
        bWrite = true;
    }
    else if (const CXXOperatorCallExpr * operatorCall = dyn_cast<CXXOperatorCallExpr>(parent))
    {
        auto oo = operatorCall->getOperator();
        if (oo == OO_AmpEqual)
        {
            // Ignore a common pattern that does not introduce any new information, merely removes
            // information:     foo &= ~Enum6::Top
            bool found = false;
            if (auto innerOperatorCall = dyn_cast<CXXOperatorCallExpr>(operatorCall->getArg(1)->IgnoreImplicit()->IgnoreParens()->IgnoreImplicit()))
            {
                found = innerOperatorCall->getOperator() == OO_Tilde;
            }
            if (!found)
                bWrite = true;
        }
            // if assignment op
        else if (oo == OO_Equal || oo == OO_StarEqual || oo == OO_SlashEqual || oo == OO_PercentEqual
           || oo == OO_PlusEqual || oo == OO_MinusEqual || oo == OO_LessLessEqual
           || oo == OO_CaretEqual || oo == OO_PipeEqual)
            bWrite = true;
            // else if comparison op
        else if (oo == OO_AmpAmp || oo == OO_PipePipe || oo == OO_Subscript
            || oo == OO_Less || oo == OO_Greater || oo == OO_LessEqual || oo == OO_GreaterEqual || oo == OO_EqualEqual || oo == OO_ExclaimEqual)
            bRead = true;
        else
            goto walk_up;
    }
    else if (const CXXMemberCallExpr * memberCall = dyn_cast<CXXMemberCallExpr>(parent))
    {
        // happens a lot with o3tl::typed_flags
        if (*memberCall->child_begin() == child)
        {
            if (auto conversionDecl = dyn_cast<CXXConversionDecl>(memberCall->getMethodDecl()))
            {
                if (conversionDecl->getConversionType()->isSpecificBuiltinType(clang::BuiltinType::Bool))
                    bRead = true;
                else
                    goto walk_up;
            }
            else
                goto walk_up;
        }
        else
            bWrite = true;
    }
    else if (isa<CallExpr>(parent) || isa<InitListExpr>(parent) || isa<ArraySubscriptExpr>(parent)
             || isa<ReturnStmt>(parent) || isa<DeclStmt>(parent)
             || isa<CXXConstructExpr>(parent)
             || isa<CXXThrowExpr>(parent))
    {
        bWrite = true;
    }
    else if (isa<CaseStmt>(parent) || isa<SwitchStmt>(parent) || isa<IfStmt>(parent)
            || isa<WhileStmt>(parent) || isa<DoStmt>(parent) || isa<ForStmt>(parent) || isa<DefaultStmt>(parent))
    {
        bRead = true;
    }
    else if (const BinaryOperator * binaryOp = dyn_cast<BinaryOperator>(parent))
    {
        if (BinaryOperator::isAssignmentOp(binaryOp->getOpcode())) {
            bWrite = true;
        } else if (BinaryOperator::isComparisonOp(binaryOp->getOpcode())) {
            bRead = true;
        } else {
            goto walk_up;
        }
    }
    else if (isa<ConditionalOperator>(parent))
    {
        goto walk_up;
    }
    else if (isa<CastExpr>(parent) || isa<UnaryOperator>(parent)
                || isa<ParenExpr>(parent)
                || isa<MaterializeTemporaryExpr>(parent)
                || isa<ExprWithCleanups>(parent)
                || isa<ConstantExpr>(parent)
                || isa<CXXBindTemporaryExpr>(parent))
    {
        goto walk_up;
    }
    else if (isa<CXXDefaultArgExpr>(parent))
    {
        // TODO this could be improved
        bWrite = true;
    }
    else if (isa<DeclRefExpr>(parent))
    {
        // slightly weird case I saw in basegfx where the enum is being used as a template param
        bWrite = true;
    }
    else if (isa<MemberExpr>(parent))
    {
        goto walk_up;
    }
    else if (isa<ParenListExpr>(parent))
    {
        goto walk_up;
    }
    else if (isa<CXXDefaultInitExpr>(parent))
    {
        goto walk_up;
    }
    else if (isa<UnresolvedLookupExpr>(parent)
            || isa<CompoundStmt>(parent))
    {
        bRead = true;
        bWrite = true;
    }
    else
    {
        bDump = true;
    }

    // to let me know if I missed something
    if (bDump) {
        parent->dump();
        declRefExpr->dump();
        report( DiagnosticsEngine::Warning,
                "unhandled clang AST node type",
                parent->getBeginLoc());
    }

    if (bWrite) {
        writeSet.insert(niceName(enumConstantDecl));
    }
    if (bRead) {
        readSet.insert(niceName(enumConstantDecl));
    }
    return true;
}

loplugin::Plugin::Registration< UnusedEnumConstants > X("unusedenumconstants", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
