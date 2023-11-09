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
  Look for class fields that are always cast to some subtype,
  which indicates that they should probably just be declared to be that subtype.

  TODO add checking for dynamic_cast/static_cast on
      unique_ptr
      shared_ptr
*/

namespace
{
struct MyFieldInfo
{
    std::string parentClass;
    std::string fieldName;
    std::string fieldType;
    std::string sourceLocation;
};

// try to limit the voluminous output a little
static std::unordered_multimap<const FieldDecl*, const CXXRecordDecl*> castMap;

class FieldCast : public loplugin::FilteringPlugin<FieldCast>
{
public:
    explicit FieldCast(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override;

    bool VisitCXXStaticCastExpr(const CXXStaticCastExpr*);
    bool VisitCXXDynamicCastExpr(const CXXDynamicCastExpr*);
    bool VisitCXXReinterpretCastExpr(const CXXReinterpretCastExpr*);

private:
    MyFieldInfo niceName(const FieldDecl*);
    void checkCast(const CXXNamedCastExpr*);
};

void FieldCast::run()
{
    handler.enableTreeWideAnalysisMode();

    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    if (!isUnitTestMode())
    {
        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        output.reserve(64 * 1024);
        for (const auto& pair : castMap)
        {
            MyFieldInfo s = niceName(pair.first);
            output += "cast:\t" + s.parentClass //
                      + "\t" + s.fieldName //
                      + "\t" + s.fieldType //
                      + "\t" + s.sourceLocation //
                      + "\t" + pair.second->getQualifiedNameAsString() //
                      + "\n";
        }
        std::ofstream myfile;
        myfile.open(WORKDIR "/loplugin.fieldcast.log", std::ios::app | std::ios::out);
        myfile << output;
        myfile.close();
    }
    else
    {
        for (const auto& pair : castMap)
            report(DiagnosticsEngine::Warning, "cast %0", pair.first->getBeginLoc())
                << pair.second->getQualifiedNameAsString();
    }
}

MyFieldInfo FieldCast::niceName(const FieldDecl* fieldDecl)
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

bool FieldCast::VisitCXXDynamicCastExpr(const CXXDynamicCastExpr* expr)
{
    checkCast(expr);
    return true;
}

bool FieldCast::VisitCXXStaticCastExpr(const CXXStaticCastExpr* expr)
{
    checkCast(expr);
    return true;
}

bool FieldCast::VisitCXXReinterpretCastExpr(const CXXReinterpretCastExpr* expr)
{
    checkCast(expr);
    return true;
}

void FieldCast::checkCast(const CXXNamedCastExpr* expr)
{
    if (ignoreLocation(expr))
        return;
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(expr->getBeginLoc())))
        return;
    auto castToType = expr->getTypeAsWritten()->getPointeeCXXRecordDecl();
    if (!castToType)
        return;
    const Expr* subExpr = compat::getSubExprAsWritten(expr);
    const FieldDecl* fieldDecl = nullptr;
    if (const MemberExpr* memberExpr = dyn_cast_or_null<MemberExpr>(subExpr->IgnoreImplicit()))
    {
        fieldDecl = dyn_cast_or_null<FieldDecl>(memberExpr->getMemberDecl());
    }
    else if (const CXXMemberCallExpr* memberCallExpr
             = dyn_cast_or_null<CXXMemberCallExpr>(subExpr->IgnoreImplicit()))
    {
        if (!memberCallExpr->getMethodDecl()->getIdentifier()
            || memberCallExpr->getMethodDecl()->getName() != "get")
            return;
        const MemberExpr* memberExpr = dyn_cast_or_null<MemberExpr>(
            memberCallExpr->getImplicitObjectArgument()->IgnoreImplicit());
        if (!memberExpr)
            return;
        fieldDecl = dyn_cast_or_null<FieldDecl>(memberExpr->getMemberDecl());
    }
    if (!fieldDecl)
        return;
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(fieldDecl->getBeginLoc())))
        return;

    // ignore casting to a less specific type
    auto castFromType = subExpr->getType()->getPointeeCXXRecordDecl();
    if (castFromType && castFromType->isDerivedFrom(castToType))
        return;

    castMap.emplace(fieldDecl, castToType);
}

loplugin::Plugin::Registration<FieldCast> X("fieldcast", false);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
