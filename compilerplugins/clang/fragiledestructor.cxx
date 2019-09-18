/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include <string>
#include <iostream>

#include "plugin.hxx"
#include "compat.hxx"
#include "clang/AST/CXXInheritance.h"


// Check for calls to virtual methods from destructors. These are dangerous because intention might be to call
// a method on a subclass, while in actual fact, it only calls the method on the current or super class.
//

namespace {

class FragileDestructor:
    public loplugin::FilteringPlugin<FragileDestructor>
{
public:
    explicit FragileDestructor(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}

    virtual bool preRun() override
    {
        StringRef fn(handler.getMainFileName());

        // TODO, these all need fixing

        if (loplugin::isSamePathname(fn, SRCDIR "/comphelper/source/misc/proxyaggregation.cxx"))
             return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/svdraw/svdpntv.cxx")) // ~SdrPaintView calling ClearPageView
             return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/svdraw/svdobj.cxx")) // ~SdrObject calling GetLastBoundRect
             return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/svdraw/svdedxv.cxx")) // ~SdrObjEditView calling SdrEndTextEdit
             return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/connectivity/source/drivers/file/FStatement.cxx")) // ~OStatement_Base calling disposing
             return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/core/CustomAnimationEffect.cxx")) // ~EffectSequenceHelper calling reset
             return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/view/sdview.cxx")) // ~View calling DeleteWindowFromPaintView
             return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/layout/ssfrm.cxx")) // ~SwFrame calling IsDeleteForbidden
             return false;

        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool PreTraverseCXXDestructorDecl(CXXDestructorDecl*);
    bool PostTraverseCXXDestructorDecl(CXXDestructorDecl*, bool);
    bool TraverseCXXDestructorDecl(CXXDestructorDecl*);
    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr *);

private:
    std::vector<CXXDestructorDecl*>  m_vDestructors;
};

bool FragileDestructor::PreTraverseCXXDestructorDecl(CXXDestructorDecl* cxxDestructorDecl)
{
    if (ignoreLocation(cxxDestructorDecl))
        return true;
    if (!cxxDestructorDecl->isThisDeclarationADefinition())
        return true;
    if (cxxDestructorDecl->getParent()->hasAttr<FinalAttr>())
        return true;
//    // ignore this for now, too tricky for me to work out
//    StringRef aFileName = getFileNameOfSpellingLoc(
//            compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(pCXXDestructorDecl)));
//    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/comphelper/")
//        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/cppuhelper/")
//        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/cppuhelper/")
//        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/comphelper/")
//        // don't know how to detect this in clang - it is making an explicit call to its own method, so presumably OK
//        || loplugin::isSamePathname(aFileName, SRCDIR "/basic/source/sbx/sbxvalue.cxx")
//       )
//        return RecursiveASTVisitor::TraverseCXXDestructorDecl(pCXXDestructorDecl);
    m_vDestructors.push_back(cxxDestructorDecl);
    return true;
}

bool FragileDestructor::PostTraverseCXXDestructorDecl(CXXDestructorDecl* cxxDestructorDecl, bool)
{
    if (!m_vDestructors.empty() && m_vDestructors.back() == cxxDestructorDecl)
        m_vDestructors.pop_back();
    return true;
}

bool FragileDestructor::TraverseCXXDestructorDecl(CXXDestructorDecl* cxxDestructorDecl)
{
    PreTraverseCXXDestructorDecl(cxxDestructorDecl);
    auto ret = FilteringPlugin::TraverseCXXDestructorDecl(cxxDestructorDecl);
    PostTraverseCXXDestructorDecl(cxxDestructorDecl, ret);
    return ret;
}

bool FragileDestructor::VisitCXXMemberCallExpr(const CXXMemberCallExpr* callExpr)
{
    if (m_vDestructors.empty() || ignoreLocation(callExpr))
        return true;
    const CXXMethodDecl* methodDecl = callExpr->getMethodDecl();
    if (!methodDecl->isVirtual() || methodDecl->hasAttr<FinalAttr>())
        return true;
    const CXXRecordDecl* parentRecordDecl = methodDecl->getParent();
    if (parentRecordDecl->hasAttr<FinalAttr>())
        return true;
    if (!callExpr->getImplicitObjectArgument()->IgnoreImpCasts()->isImplicitCXXThis())
        return true;

    // if we see an explicit call to its own method, that's OK
    auto s1 = compiler.getSourceManager().getCharacterData(compat::getBeginLoc(callExpr));
    auto s2 = compiler.getSourceManager().getCharacterData(compat::getEndLoc(callExpr));
    std::string tok(s1, s2-s1);
    if (tok.find("::") != std::string::npos)
        return true;

    // Very common pattern that we call acquire/dispose in destructors of UNO objects
    // to make sure they are cleaned up.
    if (methodDecl->getName() == "acquire" || methodDecl->getName() == "dispose")
        return true;

    // e.g. osl/thread.hxx and cppuhelper/compbase.hxx
//    StringRef aFileName = getFileNameOfSpellingLoc(
//        compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(methodDecl)));
//    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/osl/")
//        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/comphelper/")
//        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/cppuhelper/"))
//        return true;
    report(
        DiagnosticsEngine::Warning,
        "calling virtual method from destructor, either make the virtual method final, or make this class final",
        compat::getBeginLoc(callExpr))
      << callExpr->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "callee method here",
        compat::getBeginLoc(methodDecl))
      << methodDecl->getSourceRange();
    return true;
}


loplugin::Plugin::Registration<FragileDestructor> fragiledestructor("fragiledestructor");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
