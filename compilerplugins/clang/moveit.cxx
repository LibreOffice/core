/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <unordered_set>
#include "config_clang.h"
#include "plugin.hxx"
#include "check.hxx"

/*
Look for local variables that can be std::move'd into parameters.

TODO
(*) Ideally we would use a proper data-flow analysis, to detect that the var is dead after this point,
     like the one in clang at include/clang/Analysis/CFG.h
(*) we could expand the set of approved/interesting types
*/

namespace
{
class MoveIt : public loplugin::FilteringPlugin<MoveIt>
{
public:
    explicit MoveIt(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        //        std::string fn(handler.getMainFileName());
        //        loplugin::normalizeDotDotInFilePath(fn);
        //        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/filter/source/msfilter/escherex.cxx"))
        //            return false;
        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr*);
    bool VisitCXXConstructExpr(const CXXConstructExpr*);

private:
    bool isInterestingType(QualType);
};

bool MoveIt::VisitCXXMemberCallExpr(const CXXMemberCallExpr* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    const CXXMethodDecl* methodDecl = callExpr->getMethodDecl();
    if (!methodDecl)
        return true;

    unsigned len = std::min(callExpr->getNumArgs(), methodDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i)
    {
        // check if the parameter is a moveable type
        const ParmVarDecl* parmVarDecl = methodDecl->getParamDecl(i);
        if (!parmVarDecl->getType()->isRecordType())
            continue;
        const CXXRecordDecl* recordDecl
            = dyn_cast<CXXRecordDecl>(parmVarDecl->getType()->getAsRecordDecl());
        if (!recordDecl || !recordDecl->hasMoveConstructor() || recordDecl->isTriviallyCopyable())
            continue;
        if (!isInterestingType(parmVarDecl->getType()))
            continue;

        // check if (a) we're making a copy to pass to the param and (b) we're making a copy of a local var
        const Expr* argExpr = callExpr->getArg(i);
        if (!argExpr)
            continue;
        const CXXConstructExpr* argSubExpr = dyn_cast<CXXConstructExpr>(argExpr->IgnoreImplicit());
        if (!argSubExpr)
            continue;
        const DeclRefExpr* dre = dyn_cast<DeclRefExpr>(argSubExpr->getArg(0)->IgnoreImplicit());
        if (!dre)
            continue;
        const VarDecl* varDecl = dyn_cast<VarDecl>(dre->getDecl());
        if (!varDecl || varDecl->getType()->isReferenceType() || varDecl->getType()->isPointerType()
            || !varDecl->hasLocalStorage())
            continue;

        report(DiagnosticsEngine::Warning, "can std::move this var into this param",
               argExpr->getBeginLoc());
        report(DiagnosticsEngine::Note, "passing to this param", parmVarDecl->getBeginLoc());
        parmVarDecl->getType()->dump();
    }

    //    StringRef aFileName = getFilenameOfLocation(
    //        compiler.getSourceManager().getSpellingLoc(parmVarDecl->getBeginLoc()));
    //    if (loplugin::hasPathnamePrefix(aFileName,
    //                                    SRCDIR "/svx/source/sidebar/line/LineWidthValueSet.cxx"))
    //        return true;

    return true;
}

bool MoveIt::VisitCXXConstructExpr(const CXXConstructExpr* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    const CXXConstructorDecl* methodDecl = callExpr->getConstructor();
    if (!methodDecl)
        return true;

    unsigned len = std::min(callExpr->getNumArgs(), methodDecl->getNumParams());
    for (unsigned i = 0; i < len; ++i)
    {
        // check if the parameter is a moveable type
        const ParmVarDecl* parmVarDecl = methodDecl->getParamDecl(i);
        if (!parmVarDecl->getType()->isRecordType())
            continue;
        const CXXRecordDecl* recordDecl
            = dyn_cast<CXXRecordDecl>(parmVarDecl->getType()->getAsRecordDecl());
        if (!recordDecl || !recordDecl->hasMoveConstructor() || recordDecl->isTriviallyCopyable())
            continue;
        if (!isInterestingType(parmVarDecl->getType()))
            continue;

        // check if (a) we're making a copy to pass to the param and (b) we're making a copy of a local var
        const Expr* argExpr = callExpr->getArg(i);
        if (!argExpr)
            continue;
        const CXXConstructExpr* argSubExpr = dyn_cast<CXXConstructExpr>(argExpr->IgnoreImplicit());
        if (!argSubExpr || argSubExpr->getNumArgs() == 0)
            continue;
        const DeclRefExpr* dre = dyn_cast<DeclRefExpr>(argSubExpr->getArg(0)->IgnoreImplicit());
        if (!dre)
            continue;
        const VarDecl* varDecl = dyn_cast<VarDecl>(dre->getDecl());
        if (!varDecl || varDecl->getType()->isReferenceType() || varDecl->getType()->isPointerType()
            || !varDecl->hasLocalStorage())
            continue;

        report(DiagnosticsEngine::Warning, "can std::move this var into this param",
               argExpr->getBeginLoc());
        report(DiagnosticsEngine::Note, "passing to this param", parmVarDecl->getBeginLoc());
        parmVarDecl->getType()->dump();
    }

    //    StringRef aFileName = getFilenameOfLocation(
    //        compiler.getSourceManager().getSpellingLoc(parmVarDecl->getBeginLoc()));
    //    if (loplugin::hasPathnamePrefix(aFileName,
    //                                    SRCDIR "/svx/source/sidebar/line/LineWidthValueSet.cxx"))
    //        return true;

    return true;
}

/// Exclude boring types, so that we don't generate too many low-value conversions
bool MoveIt::isInterestingType(QualType qt)
{
    auto tc = loplugin::TypeCheck(qt);

    return !tc.ClassOrStruct("iterator") && !tc.ClassOrStruct("const_iterator")
           && !tc.Typedef("iterator") && !tc.Typedef("const_iterator") && !tc.Typedef("string")
           && !tc.ClassOrStruct("shared_ptr").StdNamespace()
           && !tc.ClassOrStruct("shared_ptr").Namespace("boost")
           && !tc.Class("B2DHomMatrix").Namespace("basegfx").GlobalNamespace()
           && !tc.Class("ByteSequence").Namespace("rtl").GlobalNamespace()
           && !tc.Class("OUString").Namespace("rtl").GlobalNamespace()
           && !tc.Class("OString").Namespace("rtl").GlobalNamespace()
           && !tc.Class("Any").Namespace("uno") && !tc.Class("Reference")
           && !tc.Class("SvRef").Namespace("tools").GlobalNamespace()
           && !tc.ClassOrStruct("sk_sp") // skia shared pointer
           && !tc.ClassOrStruct("VclPtr");
}

loplugin::Plugin::Registration<MoveIt> moveit("moveit");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
