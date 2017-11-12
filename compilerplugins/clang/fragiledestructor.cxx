/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
    public RecursiveASTVisitor<FragileDestructor>, public loplugin::Plugin
{
public:
    explicit FragileDestructor(loplugin::InstantiationData const & data):
        Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool TraverseCXXDestructorDecl(CXXDestructorDecl *);

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr *);

private:
    bool mbChecking = false;
};

bool FragileDestructor::TraverseCXXDestructorDecl(CXXDestructorDecl* pCXXDestructorDecl)
{
    if (ignoreLocation(pCXXDestructorDecl)) {
        return RecursiveASTVisitor::TraverseCXXDestructorDecl(pCXXDestructorDecl);
    }
    if (!pCXXDestructorDecl->isThisDeclarationADefinition()) {
        return RecursiveASTVisitor::TraverseCXXDestructorDecl(pCXXDestructorDecl);
    }
    // ignore this for now, too tricky for me to work out
    StringRef aFileName = compiler.getSourceManager().getFilename(
            compiler.getSourceManager().getSpellingLoc(pCXXDestructorDecl->getLocStart()));
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/comphelper/")
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/cppuhelper/")
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/cppuhelper/")
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/comphelper/")
        // don't know how to detect this in clang - it is making an explicit call to its own method, so presumably OK
        || loplugin::isSamePathname(aFileName, SRCDIR "/basic/source/sbx/sbxvalue.cxx")
       )
        return RecursiveASTVisitor::TraverseCXXDestructorDecl(pCXXDestructorDecl);
    mbChecking = true;
    bool ret = RecursiveASTVisitor::TraverseCXXDestructorDecl(pCXXDestructorDecl);
    mbChecking = false;
    return ret;
}

bool FragileDestructor::VisitCXXMemberCallExpr(const CXXMemberCallExpr* callExpr)
{
    if (!mbChecking || ignoreLocation(callExpr)) {
        return true;
    }
    const CXXMethodDecl* methodDecl = callExpr->getMethodDecl();
    if (!methodDecl->isVirtual() || methodDecl->hasAttr<FinalAttr>()) {
        return true;
    }
    const CXXRecordDecl* parentRecordDecl = methodDecl->getParent();
    if (parentRecordDecl->hasAttr<FinalAttr>()) {
        return true;
    }
    if (!callExpr->getImplicitObjectArgument()->IgnoreImpCasts()->isImplicitCXXThis()) {
        return true;
    }
    // if we see an explicit call to its own method, that's OK
    auto s1 = compiler.getSourceManager().getCharacterData(callExpr->getLocStart());
    auto s2 = compiler.getSourceManager().getCharacterData(callExpr->getLocEnd());
    std::string tok(s1, s2-s1);
    if (tok.find("::") != std::string::npos) {
        return true;
    }
    // e.g. osl/thread.hxx and cppuhelper/compbase.hxx
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(methodDecl->getLocStart()));
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/osl/")
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/comphelper/")
        || loplugin::hasPathnamePrefix(aFileName, SRCDIR "/include/cppuhelper/"))
        return true;
    report(
        DiagnosticsEngine::Warning,
        "calling virtual method from destructor, either make the virtual method SAL_FINAL, or make this class SAL_FINAL",
        callExpr->getLocStart())
      << callExpr->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "callee method here",
        methodDecl->getLocStart())
      << methodDecl->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< FragileDestructor > X("fragiledestructor", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
