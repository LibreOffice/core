/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>
#include "config_clang.h"
#include "compat.hxx"
#include "plugin.hxx"
#include <fstream>

/**
Look for methods that are final i.e. nothing overrides them

Making the method final means the compiler can devirtualise
some method calls.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE=all COMPILER_PLUGIN_TOOL='finalmethods' check
  $ ./compilerplugins/clang/finalmethods.py

*/

namespace
{
// try to limit the voluminous output a little
static std::unordered_set<std::string> overriddenSet;
static std::unordered_map<std::string, std::string> definitionMap; // methodName -> location

class FinalMethods : public RecursiveASTVisitor<FinalMethods>, public loplugin::Plugin
{
public:
    explicit FinalMethods(loplugin::InstantiationData const& data)
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
        for (const std::string& s : overriddenSet)
            output += "overridden:\t" + s + "\n";
        for (const auto& s : definitionMap)
            output += "definition:\t" + s.first + "\t" + s.second + "\n";
        std::ofstream myfile;
        myfile.open(WORKDIR "/loplugin.finalmethods.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations() const { return true; }

    bool shouldVisitImplicitCode() const { return true; }

    bool VisitCXXMethodDecl(const CXXMethodDecl*);

private:
    std::string toString(SourceLocation loc);
    std::string niceName(const CXXMethodDecl*);
};

bool FinalMethods::VisitCXXMethodDecl(const CXXMethodDecl* methodDecl)
{
    if (ignoreLocation(methodDecl))
        return true;
    if (!methodDecl->isThisDeclarationADefinition())
        return true;
    // don't care about compiler-generated functions
    if (methodDecl->isImplicit())
        return true;
    if (!methodDecl->isVirtual())
        return true;
    if (!methodDecl->getLocation().isValid())
        return true;
    if (isa<CXXDestructorDecl>(methodDecl) || isa<CXXConstructorDecl>(methodDecl))
        return true;

    methodDecl = methodDecl->getCanonicalDecl();
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(methodDecl->getLocation())))
        return true;

    std::string methodNiceName = niceName(methodDecl);

    // If the containing class/struct is final, then the method is effectively final too.
    if (!methodDecl->hasAttr<FinalAttr>() && !methodDecl->getParent()->hasAttr<FinalAttr>())
        definitionMap.insert({ methodNiceName, toString(methodDecl->getBeginLoc()) });

    for (auto it = methodDecl->begin_overridden_methods();
         it != methodDecl->end_overridden_methods(); ++it)
        overriddenSet.insert(niceName(*it));

    return true;
}

std::string FinalMethods::niceName(const CXXMethodDecl* methodDecl)
{
    if (methodDecl->getInstantiatedFromMemberFunction())
        methodDecl = dyn_cast<CXXMethodDecl>(methodDecl->getInstantiatedFromMemberFunction());
    else if (methodDecl->getTemplateInstantiationPattern())
        methodDecl = dyn_cast<CXXMethodDecl>(methodDecl->getTemplateInstantiationPattern());

    std::string returnType = methodDecl->getReturnType().getCanonicalType().getAsString();

    const CXXRecordDecl* recordDecl = methodDecl->getParent();
    std::string nameAndParams
        = recordDecl->getQualifiedNameAsString() + "::" + methodDecl->getNameAsString() + "(";

    bool bFirst = true;
    for (const ParmVarDecl* pParmVarDecl : methodDecl->parameters())
    {
        if (bFirst)
            bFirst = false;
        else
            nameAndParams += ",";
        nameAndParams += pParmVarDecl->getType().getCanonicalType().getAsString();
    }
    nameAndParams += ")";
    if (methodDecl->isConst())
        nameAndParams += " const";

    return returnType + " " + nameAndParams + " " + returnType;
}

std::string FinalMethods::toString(SourceLocation loc)
{
    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc(loc);
    StringRef name = getFilenameOfLocation(expansionLoc);
    std::string sourceLocation
        = std::string(name.substr(strlen(SRCDIR) + 1)) + ":"
          + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(sourceLocation);
    return sourceLocation;
}

loplugin::Plugin::Registration<FinalMethods> X("finalmethods", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
