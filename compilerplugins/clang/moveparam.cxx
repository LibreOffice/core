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
#include "plugin.hxx"
#include "check.hxx"

/*
Look for places where we can pass by move && param and so avoid
unnecessary copies.
Empirically, when we are passing a container type to a function, 80% of the time,
we are passing a local temporary that can be moved instead of being copied.

TODO this could be a lot smarter, with ignoring false+ e.g. when copying a param
in a loop
*/

namespace
{
class MoveParam : public loplugin::FilteringPlugin<MoveParam>
{
public:
    explicit MoveParam(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/filter/source/msfilter/escherex.cxx"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/ui/docshell/docfunc.cxx"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sc/source/ui/view/viewfunc.cxx"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/basegfx/source/polygon/b2dpolygontools.cxx"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/basegfx/source/polygon/b3dpolygontools.cxx"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/connectivity/source/commontools/dbtools.cxx"))
            return false;
        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXOperatorCallExpr(const CXXOperatorCallExpr*);
    bool VisitCXXConstructExpr(const CXXConstructExpr*);

    bool isContainerType(QualType qt);
};

bool MoveParam::VisitCXXOperatorCallExpr(const CXXOperatorCallExpr* callExpr)
{
    if (ignoreLocation(callExpr))
        return true;
    if (!callExpr->isAssignmentOp())
        return true;
    auto qt = callExpr->getType();
    if (!isContainerType(qt))
        return true;
    auto declRef = dyn_cast<DeclRefExpr>(callExpr->getArg(1)->IgnoreParenImpCasts());
    if (!declRef)
        return true;

    auto parmVarDecl = dyn_cast_or_null<ParmVarDecl>(declRef->getDecl());
    if (!parmVarDecl)
        return true;

    if (!loplugin::TypeCheck(parmVarDecl->getType()).LvalueReference().Const())
        return true;

    StringRef aFileName = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(parmVarDecl)));
    if (loplugin::hasPathnamePrefix(aFileName,
                                    SRCDIR "/svx/source/sidebar/line/LineWidthValueSet.cxx"))
        return true;

    report(DiagnosticsEngine::Warning, "rather use move && param1", compat::getBeginLoc(callExpr));

    return true;
}

bool MoveParam::VisitCXXConstructExpr(const CXXConstructExpr* constructExpr)
{
    if (ignoreLocation(compat::getBeginLoc(constructExpr)))
        return true;
    if (isInUnoIncludeFile(compat::getBeginLoc(constructExpr)))
        return true;

    auto qt = constructExpr->getType();
    if (!isContainerType(qt))
        return true;

    if (constructExpr->getNumArgs() != 1)
        return true;

    auto declRef = dyn_cast<DeclRefExpr>(constructExpr->getArg(0)->IgnoreParenImpCasts());
    if (!declRef)
        return true;

    auto parmVarDecl = dyn_cast_or_null<ParmVarDecl>(declRef->getDecl());
    if (!parmVarDecl)
        return true;

    if (!loplugin::TypeCheck(parmVarDecl->getType()).LvalueReference().Const())
        return true;

    StringRef aFileName = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(parmVarDecl)));
    if (loplugin::hasPathnamePrefix(aFileName,
                                    SRCDIR "/include/drawinglayer/primitive3d/baseprimitive3d.hxx"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/svx/source/svdraw/svdmrkv.cxx"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/editeng/swafopt.hxx"))
        return true;
    if (loplugin::hasPathnamePrefix(
            aFileName, SRCDIR "/drawinglayer/source/primitive2d/textdecoratedprimitive2d.cxx"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName,
                                    SRCDIR "/chart2/source/tools/InternalDataProvider.cxx"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sc/source/core/data/attrib.cxx"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sw/source/core/doc/docfmt.cxx"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/configmgr/source/modifications.cxx"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/svx/source/dialog/srchdlg.cxx"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName,
                                    SRCDIR "/stoc/source/servicemanager/servicemanager.cxx"))
        return true;

    report(DiagnosticsEngine::Warning, "rather use move && param3",
           compat::getBeginLoc(constructExpr));

    return true;
}

bool MoveParam::isContainerType(QualType qt)
{
    auto tc = loplugin::TypeCheck(qt);
    return tc.Class("Primitive2DContainer")
               .Namespace("primitive2d")
               .Namespace("drawinglayer")
               .GlobalNamespace()
           || tc.ClassOrStruct("sorted_vector").Namespace("o3tl").GlobalNamespace()
           || tc.ClassOrStruct("array").StdNamespace() || tc.ClassOrStruct("vector").StdNamespace()
           || tc.ClassOrStruct("deque").StdNamespace()
           || tc.ClassOrStruct("forward_list").StdNamespace()
           || tc.ClassOrStruct("list").StdNamespace() || tc.ClassOrStruct("set").StdNamespace()
           || tc.ClassOrStruct("map").StdNamespace() || tc.ClassOrStruct("multiset").StdNamespace()
           || tc.ClassOrStruct("multimap").StdNamespace()
           || tc.ClassOrStruct("unordered_set").StdNamespace()
           || tc.ClassOrStruct("unordered_map").StdNamespace()
           || tc.ClassOrStruct("unordered_multiset").StdNamespace()
           || tc.ClassOrStruct("unordered_multimap").StdNamespace()
           || tc.ClassOrStruct("stack").StdNamespace() || tc.ClassOrStruct("queue").StdNamespace()
           || tc.ClassOrStruct("priority_queue").StdNamespace();
}

/** off by default because it needs some hand-holding */
loplugin::Plugin::Registration<MoveParam> moveparam("moveparam", false);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
