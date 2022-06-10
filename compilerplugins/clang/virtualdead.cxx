/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "check.hxx"
#include "compat.hxx"

#include <cassert>
#include <string>
#include <iostream>
#include <set>
#include <fstream>

/**
Look for virtual methods where all of the overrides either
(a) do nothing
(b) all return the same value

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE=all COMPILER_PLUGIN_TOOL='VirtualDead' check
  $ ./compilerplugins/clang/VirtualDead.py
  $ for dir in *; do make FORCE_COMPILE=all UPDATE_FILES=$dir COMPILER_PLUGIN_TOOL='removevirtuals' $dir; done

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)
*/

namespace
{
struct MyFuncInfo
{
    std::string name;
    std::string sourceLocation;
    std::string returnValue;
};
bool operator<(const MyFuncInfo& lhs, const MyFuncInfo& rhs)
{
    return std::tie(lhs.name, lhs.returnValue) < std::tie(rhs.name, rhs.returnValue);
}
struct MyParamInfo
{
    std::string funcName;
    std::string paramBitField;
};
bool operator<(const MyParamInfo& lhs, const MyParamInfo& rhs)
{
    return std::tie(lhs.funcName, lhs.paramBitField) < std::tie(rhs.funcName, rhs.paramBitField);
}

// try to limit the voluminous output a little
static std::set<MyFuncInfo> definitionSet;
static std::set<MyParamInfo> paramUsedSet;

class VirtualDead : public RecursiveASTVisitor<VirtualDead>, public loplugin::Plugin
{
public:
    explicit VirtualDead(loplugin::InstantiationData const& data)
        : Plugin(data)
    {
    }

    virtual void run() override
    {
        handler.enableTreeWideAnalysisMode();

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyFuncInfo& s : definitionSet)
            output += "virtual:\t" + s.name + "\t" + s.sourceLocation + "\t" + s.returnValue + "\n";
        for (const MyParamInfo& s : paramUsedSet)
            output += "param:\t" + s.funcName + "\t" + s.paramBitField + "\n";
        std::ofstream myfile;
        myfile.open(WORKDIR "/loplugin.virtualdead.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }
    bool shouldVisitTemplateInstantiations() const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitCXXMethodDecl(const CXXMethodDecl* decl);

private:
    std::string getCallValue(const Expr* arg);
    std::string toString(SourceLocation loc);
    void markSuperclassMethods(const CXXMethodDecl* methodDecl, const std::string& returnValue,
                               const std::string& paramBitField);
};

std::string niceName(const CXXMethodDecl* cxxMethodDecl)
{
    while (cxxMethodDecl->getTemplateInstantiationPattern())
        cxxMethodDecl = dyn_cast<CXXMethodDecl>(cxxMethodDecl->getTemplateInstantiationPattern());
    while (cxxMethodDecl->getInstantiatedFromMemberFunction())
        cxxMethodDecl = dyn_cast<CXXMethodDecl>(cxxMethodDecl->getInstantiatedFromMemberFunction());
    std::string s = cxxMethodDecl->getReturnType().getCanonicalType().getAsString() + " "
                    + cxxMethodDecl->getQualifiedNameAsString() + "(";
    for (const ParmVarDecl* pParmVarDecl : cxxMethodDecl->parameters())
    {
        s += pParmVarDecl->getType().getCanonicalType().getAsString();
        s += ",";
    }
    s += ")";
    if (cxxMethodDecl->isConst())
    {
        s += "const";
    }
    return s;
}

bool VirtualDead::VisitCXXMethodDecl(const CXXMethodDecl* methodDecl)
{
    if (!methodDecl->isVirtual() || methodDecl->isDeleted())
        return true;
    if (isa<CXXDestructorDecl>(methodDecl))
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(methodDecl->getCanonicalDecl()))
        return true;

    if (!methodDecl->isThisDeclarationADefinition())
        return true;

    std::string returnValue;

    auto body = methodDecl->getBody();
    if (body)
    {
        auto compoundStmt = dyn_cast<CompoundStmt>(body);
        if (!compoundStmt)
            returnValue = "empty";
        else if (compoundStmt->size() == 0)
            returnValue = "empty";
        else
        {
            if (auto returnStmt = dyn_cast<ReturnStmt>(*compoundStmt->body_begin()))
            {
                if (!returnStmt->getRetValue())
                    returnValue = "empty";
                else
                    returnValue = getCallValue(returnStmt->getRetValue());
            }
            else
                returnValue = "unknown-stmt";
        }
    }
    else
        returnValue = "empty";

    std::string paramBitfield;
    for (auto it = methodDecl->param_begin(); it != methodDecl->param_end(); ++it)
    {
        auto param = *it;
        paramBitfield += param->getName().empty() ? "0" : "1";
    }

    markSuperclassMethods(methodDecl, returnValue, paramBitfield);

    return true;
}

void VirtualDead::markSuperclassMethods(const CXXMethodDecl* methodDecl,
                                        const std::string& returnValue,
                                        std::string const& paramBitField)
{
    if (methodDecl->size_overridden_methods() == 0)
    {
        std::string aNiceName = niceName(methodDecl);
        definitionSet.insert(
            { aNiceName, toString(methodDecl->getCanonicalDecl()->getLocation()), returnValue });
        paramUsedSet.insert({ aNiceName, paramBitField });
        return;
    }

    for (auto iter = methodDecl->begin_overridden_methods();
         iter != methodDecl->end_overridden_methods(); ++iter)
    {
        const CXXMethodDecl* overriddenMethod = *iter;
        markSuperclassMethods(overriddenMethod, returnValue, paramBitField);
    }
}

std::string VirtualDead::getCallValue(const Expr* arg)
{
    arg = arg->IgnoreParenCasts();
    if (isa<CXXDefaultArgExpr>(arg))
    {
        arg = dyn_cast<CXXDefaultArgExpr>(arg)->getExpr();
    }
    arg = arg->IgnoreParenCasts();
    // ignore this, it seems to trigger an infinite recursion
    if (isa<UnaryExprOrTypeTraitExpr>(arg))
        return "unknown1";
    if (arg->isValueDependent())
        return "unknown2";
    APSInt x1;
    if (compat::EvaluateAsInt(arg, x1, compiler.getASTContext()))
    {
        return compat::toString(x1, 10);
    }
    if (isa<CXXNullPtrLiteralExpr>(arg))
    {
        return "0";
    }
    if (isa<MaterializeTemporaryExpr>(arg))
    {
        const CXXBindTemporaryExpr* strippedArg
            = dyn_cast_or_null<CXXBindTemporaryExpr>(arg->IgnoreParenCasts());
        if (strippedArg)
        {
            auto temp = dyn_cast<CXXTemporaryObjectExpr>(strippedArg->getSubExpr());
            if (temp->getNumArgs() == 0)
            {
                if (loplugin::TypeCheck(temp->getType())
                        .Class("OUString")
                        .Namespace("rtl")
                        .GlobalNamespace())
                {
                    return "\"\"";
                }
                if (loplugin::TypeCheck(temp->getType())
                        .Class("OString")
                        .Namespace("rtl")
                        .GlobalNamespace())
                {
                    return "\"\"";
                }
                return "defaultConstruct";
            }
        }
    }

    // Get the expression contents.
    // This helps us find params which are always initialised with something like "OUString()".
    SourceManager& SM = compiler.getSourceManager();
    SourceLocation startLoc = arg->getBeginLoc();
    SourceLocation endLoc = arg->getEndLoc();
    const char* p1 = SM.getCharacterData(startLoc);
    const char* p2 = SM.getCharacterData(endLoc);
    if (!p1 || !p2 || (p2 - p1) < 0 || (p2 - p1) > 40)
    {
        return "unknown3";
    }
    unsigned n = Lexer::MeasureTokenLength(endLoc, SM, compiler.getLangOpts());
    std::string s(p1, p2 - p1 + n);
    // strip linefeed and tab characters so they don't interfere with the parsing of the log file
    std::replace(s.begin(), s.end(), '\r', ' ');
    std::replace(s.begin(), s.end(), '\n', ' ');
    std::replace(s.begin(), s.end(), '\t', ' ');

    // now normalize the value. For some params, like OUString, we can pass it as OUString() or "" and they are the same thing
    if (s == "OUString()")
        s = "\"\"";
    else if (s == "OString()")
        s = "\"\"";

    if (s[0] == '"' || s[0] == '\'')
        return s;
    return "unknown4";
}

std::string VirtualDead::toString(SourceLocation loc)
{
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc(loc);
    StringRef name = getFilenameOfLocation(expansionLoc);
    std::string sourceLocation
        = std::string(name.substr(strlen(SRCDIR) + 1)) + ":"
          + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(sourceLocation);
    return sourceLocation;
}

loplugin::Plugin::Registration<VirtualDead> X("virtualdead", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
